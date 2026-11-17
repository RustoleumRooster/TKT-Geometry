#include <vulkan/vulkan.h>
#include "vkSunlightModule.h"
#include "LightMaps.h"
#include "vkUtilModules.h"
#include <math.h>

Sunlight_Module::Sunlight_Module()
	: Vulkan_Module()
{
	set_ptrs();
}

void Sunlight_Module::initialize(Vulkan_App* vulkan)
{
	Vulkan_Module::initialize(vulkan);
	configuration = vulkan->configuration;

	Geometry_Module* geo_mod = vulkan->geo_module;
	Geometry_Assets* geo_assets = vulkan->geo_module->geo_assets;

	indices_soa = &geo_assets->indices_soa;
	vertices_soa = &geo_assets->vertices_soa;
	n_indices = geo_assets->indices_soa.data.size();
	n_vertices = geo_assets->vertices_soa.data0.size();
	n_nodes = geo_assets->bvh.node_count;
	n_sky_triangles = geo_assets->triangle_lists[GPU_TRIS_SKY].count();

	materials = &configuration->get_materials();

	reflect::connect(&geo_mod->vertices, &this->vertices);
	reflect::connect(&geo_mod->indices, &this->indices);
	reflect::connect(&geo_mod->lm_uvs, &this->lm_uvs);
	reflect::connect(&geo_mod->bvh_nodes, &this->bvh_nodes);
	reflect::connect(&geo_mod->edges, &this->edges);
}

void Sunlight_Module::createDescriptorSets(int lightmap_n)
{
	MyDescriptorWriter writer(*descriptorSetLayout, *m_DescriptorPool);

	descriptorSets.resize(1);

	writer.writeBuffer(0, ubo.getDescriptorBufferInfo());
	writer.writeBuffer(1, indices.X->getDescriptorBufferInfo());
	writer.writeBuffer(2, vertices.X->getDescriptorBufferInfo());
	writer.writeBuffer(3, lm_uvs.X->getDescriptorBufferInfo());
	writer.writeBuffer(4, bvh_nodes.X->getDescriptorBufferInfo());
	writer.writeBuffer(5, edges.X->getDescriptorBufferInfo());
	writer.writeImage(6, images_in.X->getDescriptorBufferInfo(lightmap_n));
	writer.build(descriptorSets[0]);
}

void Sunlight_Module::createDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.resize(7);

	bindings[0] = ubo.getDescriptorSetLayout(0);
	bindings[1] = indices.X->getDescriptorSetLayout(1);
	bindings[2] = vertices.X->getDescriptorSetLayout(2);
	bindings[3] = lm_uvs.X->getDescriptorSetLayout(3);
	bindings[4] = bvh_nodes.X->getDescriptorSetLayout(4);
	bindings[5] = edges.X->getDescriptorSetLayout(5);
	bindings[6] = images_out.X->getDescriptorSetLayout(6);

	descriptorSetLayout = new MyDescriptorSetLayout(m_device, bindings);
}

void Sunlight_Module::createUBO()
{
	ubo.range = sizeof(SunTraceInfo);

	m_device->createBuffer(ubo.range, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, ubo.Buffer,
		ubo.BufferMemory);
}

void Sunlight_Module::writeUBO(int info_n)
{
	SunTraceInfo info{};

	info.n_vertices = n_vertices;
	info.n_triangles = n_indices / 3;
	info.n_nodes = n_nodes;

	info.face_index_offset = indices_soa->offset[info_n];
	info.face_n_indices = indices_soa->len[info_n];
	info.face_vertex_offset = vertices_soa->offset[info_n];

	info.lightmap_width = materials->data()[info_n].lightmap_size;
	info.lightmap_height = materials->data()[info_n].lightmap_size;

	//=================================
	// Sun cone, with an orthonormal basis around the sun direction
	//
	vector3df dir = sun_direction;
	dir.normalize();

	vector3df up = fabs(dir.Y) > 0.99f ? vector3df(1, 0, 0) : vector3df(0, 1, 0);
	vector3df tan = dir.crossProduct(up);
	tan.normalize();
	vector3df bitan = tan.crossProduct(dir);
	bitan.normalize();

	f32 half_angle = 0.5f * sun_angle * core::DEGTORAD;

	info.sun_dir = dir;
	info.sun_tan = tan;
	info.sun_bitan = bitan;
	info.cos_max = cosf(half_angle);
	info.intensity = intensity;
	info.n_passes = std::max(1u, n_passes);
	info.miss_is_sky = miss_is_sky ? 1 : 0;
	info.seed = info_n * 0x9E3779B9u;

	ubo.writeToBuffer(m_device->getDevice(), (void*)&info);
}

void Sunlight_Module::runMaterial(int mat_n)
{
	writeUBO(mat_n);

	createDescriptorSets(materials->data()[mat_n].lightmap_no);

	VkCommandBuffer commandBuffer = m_device->beginSingleTimeCommands();

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipeline->getPipeline());

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1,
		&descriptorSets[0], 0, 0);

	uint32_t n_WorkGroups_x = indices_soa->len[mat_n] / 3;

	std::cout << "sunlight: execute " << n_WorkGroups_x << " workgroups\n";

	m_device->dispatch(commandBuffer, n_WorkGroups_x, 1, 1);

	m_device->endSingleTimeCommands(commandBuffer);

	m_DescriptorPool->freeDescriptorsSets(descriptorSets);

	vkDeviceWaitIdle(m_device->getDevice());

	estimateRays(mat_n);
}

void Sunlight_Module::estimateRays(int mat_n)
{
	//Per texel every thread (256) traces one ray per pass. Unlike area lights there's
	//no per-light loop, so the count only depends on the texels covered.
	const u32 threads_per_workgroup = 32 * 8;

	u32 lm_size = materials->data()[mat_n].lightmap_size;
	u32 first = indices_soa->offset[mat_n];
	u32 n_triangles = indices_soa->len[mat_n] / 3;

	for (u32 t = 0; t < n_triangles; t++)
	{
		u32 texels = triangle_trans->count_texels(first + t * 3, lm_size);

		texels_shaded += texels;
		rays_traced += (u64)texels * threads_per_workgroup * std::max(1u, n_passes);
	}
}

void Sunlight_Module::run()
{
	if (n_sky_triangles == 0 && !miss_is_sky)
	{
		cout << "sunlight: no sky triangles in the scene (and miss_is_sky is off), nothing can be lit\n";
		images_out.X = images_in.X;
		images_out.ready = true;
		return;
	}

	createDescriptorSetLayout();
	createComputePipeline("shaders/sunlight.spv");
	createUBO();

	triangle_trans = new Triangle_Transformer(*vertices_soa, *indices_soa);

	for (int i = 0; i < materials->size(); i++)
	{
		if (materials->data()[i].has_lightmap)
			runMaterial(i);
	}

	delete triangle_trans;
	triangle_trans = NULL;

	cleanup();

	images_out.X = images_in.X;
	images_out.ready = true;
}

void Sunlight_Module::cleanup()
{
	descriptorSetLayout->cleanup();

	ubo.destroy(m_device->getDevice());

	pipeline->cleanup();

	vkDestroyPipelineLayout(m_device->getDevice(), pipelineLayout, nullptr);
}

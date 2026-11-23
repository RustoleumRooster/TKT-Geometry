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

	has_selected_triangle = geo_assets->has_selected_triangle;
	selected_triangle_index = geo_assets->selected_triangle_index;
	selected_triangle_mg = geo_assets->selected_triangle_mg;
	selected_triangle_bary_coords = geo_assets->selected_triangle_bary_coords;

	materials = &configuration->get_materials();

	//a Sunlight node in the scene overrides the default direction
	if (geo_assets->has_sun_node)
		sun_direction = geo_assets->sun_direction;

	reflect::connect(&geo_mod->vertices, &this->vertices);
	reflect::connect(&geo_mod->indices, &this->indices);
	reflect::connect(&geo_mod->lm_uvs, &this->lm_uvs);
	reflect::connect(&geo_mod->bvh_nodes, &this->bvh_nodes);
	reflect::connect(&geo_mod->edges, &this->edges);
	reflect::connect(&geo_mod->scratchpad, &this->scratchpad);
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
	writer.writeBuffer(7, scratchpad.X->getDescriptorBufferInfo());
	writer.build(descriptorSets[0]);
}

void Sunlight_Module::createDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.resize(8);

	bindings[0] = ubo.getDescriptorSetLayout(0);
	bindings[1] = indices.X->getDescriptorSetLayout(1);
	bindings[2] = vertices.X->getDescriptorSetLayout(2);
	bindings[3] = lm_uvs.X->getDescriptorSetLayout(3);
	bindings[4] = bvh_nodes.X->getDescriptorSetLayout(4);
	bindings[5] = edges.X->getDescriptorSetLayout(5);
	bindings[6] = images_out.X->getDescriptorSetLayout(6);
	bindings[7] = scratchpad.X->getDescriptorSetLayout(7);

	descriptorSetLayout = new MyDescriptorSetLayout(m_device, bindings);
}

void Sunlight_Module::createUBO()
{
	ubo.range = sizeof(SunTraceInfo);

	m_device->createBuffer(ubo.range, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, ubo.Buffer,
		ubo.BufferMemory);
}

void Sunlight_Module::writeUBO(int info_n, int debug, int triangle_offset)
{
	SunTraceInfo info{};

	info.n_vertices = n_vertices;
	info.n_triangles = n_indices / 3;
	info.n_nodes = n_nodes;

	if (triangle_offset >= 0)
	{
		//single triangle: one workgroup, starting at the triangle's first index
		info.face_index_offset = triangle_offset;
		info.face_n_indices = 3;
	}
	else
	{
		info.face_index_offset = indices_soa->offset[info_n];
		info.face_n_indices = indices_soa->len[info_n];
	}
	info.face_vertex_offset = vertices_soa->offset[info_n];

	info.debug_mode = debug;
	info.debug_ray_len = std::max(0.0f, debug_ray_length);
	info.in_coords = vector3df(0, 0, 0);

	if (triangle_offset >= 0)
	{
		//lightmap uv of the clicked point (data1 is the lightmap uv, as the shader sees it)
		u32 idx0 = indices_soa->data[triangle_offset].x;
		u32 idx1 = indices_soa->data[triangle_offset + 1].x;
		u32 idx2 = indices_soa->data[triangle_offset + 2].x;

		info.in_coords = vertices_soa->data1[idx0].V * selected_triangle_bary_coords.X +
			vertices_soa->data1[idx1].V * selected_triangle_bary_coords.Y +
			vertices_soa->data1[idx2].V * selected_triangle_bary_coords.Z;
	}

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

	//settings come straight from the editor, so keep them in a sane range here
	f32 half_angle = 0.5f * core::clamp(sun_angle, 0.0f, 180.0f) * core::DEGTORAD;

	info.sun_dir = dir;
	info.sun_tan = tan;
	info.sun_bitan = bitan;
	info.cos_max = cosf(half_angle);
	info.intensity = std::max(0.0f, intensity);
	info.n_passes = std::max(1u, n_passes);
	info.min_coverage = core::clamp(min_coverage, 0.0f, 1.0f);
	info.miss_is_sky = miss_is_sky ? 1 : 0;

	//seed 0 gives the original per-material streams
	info.seed = (info_n * 0x9E3779B9u) ^ (seed * 0x85EBCA6Bu);

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

void Sunlight_Module::runTriangle(int mat_n, int triangle_offset)
{
	writeUBO(mat_n, debug_mode.value, triangle_offset);

	createDescriptorSets(materials->data()[mat_n].lightmap_no);

	VkCommandBuffer commandBuffer = m_device->beginSingleTimeCommands();

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipeline->getPipeline());

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1,
		&descriptorSets[0], 0, 0);

	std::cout << "sunlight: debug run of material " << mat_n << ", triangle index " << triangle_offset << "\n";

	m_device->dispatch(commandBuffer, 1, 1, 1);

	m_device->endSingleTimeCommands(commandBuffer);

	m_DescriptorPool->freeDescriptorsSets(descriptorSets);

	vkDeviceWaitIdle(m_device->getDevice());

	read_debug_results(mat_n, triangle_offset);
}

void Sunlight_Module::read_debug_results(int mat_n, int triangle_offset)
{
	const u32 n = 512;

	std::vector<aligned_vec3> hits(n);

	MyBufferObject stagingBuffer(m_device, sizeof(aligned_vec3), n, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	m_device->copyBuffer(scratchpad.X->Buffer, stagingBuffer.getBuffer(), sizeof(aligned_vec3) * n);

	stagingBuffer.readFromBuffer((void*)hits.data());

	LineHolder& graph = vulkan->geo_assets->debug_graph;

	u32 lm_size = materials->data()[mat_n].lightmap_size;

	//triangle in world space and in texels, for reference
	cout << "sunlight debug: lightmap " << materials->data()[mat_n].lightmap_no << ", size " << lm_size << "\n";
	for (int k = 0; k < 3; k++)
	{
		u32 idx = indices_soa->data[triangle_offset + k].x;
		vector3df P = vertices_soa->data0[idx].V;
		vector3df uv = vertices_soa->data1[idx].V * (f32)lm_size;

		cout << "  v" << k << " pos (" << P.X << ", " << P.Y << ", " << P.Z << ")  uv in texels ("
			<< uv.X << ", " << uv.Y << ")\n";

		vector3df P1 = vertices_soa->data0[indices_soa->data[triangle_offset + (k + 1) % 3].x].V;
		graph.lines.push_back(core::line3df(P, P1));
	}

	if (debug_mode.value == SUN_DEBUG_RAYS)
	{
		u32 n_rays = 0;

		for (u32 t = 0; t < 256; t++)
		{
			if (hits[t].V == hits[256 + t].V)
				continue;

			graph.lines.push_back(core::line3df(hits[t].V, hits[256 + t].V));
			n_rays++;
		}

		vector3df uv = vector3df(0, 0, 0);
		{
			u32 idx0 = indices_soa->data[triangle_offset].x;
			u32 idx1 = indices_soa->data[triangle_offset + 1].x;
			u32 idx2 = indices_soa->data[triangle_offset + 2].x;
			uv = (vertices_soa->data1[idx0].V * selected_triangle_bary_coords.X +
				vertices_soa->data1[idx1].V * selected_triangle_bary_coords.Y +
				vertices_soa->data1[idx2].V * selected_triangle_bary_coords.Z) * (f32)lm_size;
		}

		cout << "  clicked texel (" << floor(uv.X) << ", " << floor(uv.Y) << "): " << n_rays
			<< " of 256 threads had a valid sample\n";
	}
	else if (debug_mode.value == SUN_DEBUG_TEXELS)
	{
		u32 n_visited = (u32)hits[0].V.X;
		u32 n_written = (u32)hits[256].V.X;
		bool degenerate = hits[0].V.Y != 0;
		u32 n_rows = (u32)hits[0].V.Z;

		cout << "  rows walked " << n_rows << ", texels visited " << n_visited << ", written " << n_written
			<< (degenerate ? "  (UV DEGENERATE: zero area in lightmap space)" : "") << "\n";
		//same test as the shader: ray_count > min_coverage * 256 * n_passes
		u32 samples_max = 256 * std::max(1u, n_passes);
		f32 samples_needed = core::clamp(min_coverage, 0.0f, 1.0f) * samples_max;

		cout << "  (written needs more than " << samples_needed << " valid samples)\n";

		//normal length for the written texels: a texel's width in world units would be ideal,
		//this is a fixed fraction of the triangle's size instead
		vector3df a = vertices_soa->data0[indices_soa->data[triangle_offset].x].V;
		vector3df b = vertices_soa->data0[indices_soa->data[triangle_offset + 1].x].V;
		vector3df c = vertices_soa->data0[indices_soa->data[triangle_offset + 2].x].V;
		vector3df N = (b - a).crossProduct(c - b);
		N.normalize();

		f32 len = 8.0f;

		for (u32 k = 0; k < std::min(n_visited, 255u); k++)
		{
			vector3df texel = hits[1 + k].V;
			vector3df centre = hits[257 + k].V;
			bool written = texel.Z > samples_needed;

			cout << "    texel (" << (int)texel.X << ", " << (int)texel.Y << ")  samples " << (int)texel.Z
				<< " / " << samples_max << (written ? "  written" : "  -- not written") << "\n";

			if (written)
				graph.lines.push_back(core::line3df(centre, centre + N * len));
			else
				graph.points.push_back(centre);
		}

		if (n_visited > 255)
			cout << "    (" << n_visited - 255 << " more not recorded)\n";
	}
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

	{
		vector3df d = sun_direction;
		d.normalize();
		cout << "sunlight: direction toward sun (" << d.X << ", " << d.Y << ", " << d.Z << ")"
			<< (vulkan->geo_assets->has_sun_node ? " from Sunlight node\n" : " (default, no Sunlight node)\n");
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

	//==================================
	// Single triangle debug run, on top of the full bake
	//
	if (debug_mode.value != SUN_DEBUG_OFF)
	{
		if (!has_selected_triangle)
			cout << "sunlight: debug mode is on but no face is selected (click a face first)\n";
		else if (selected_triangle_mg < 0 || selected_triangle_mg >= (int)materials->size() ||
			!materials->data()[selected_triangle_mg].has_lightmap)
			cout << "sunlight: selected triangle's material " << selected_triangle_mg << " has no lightmap\n";
		else
			runTriangle(selected_triangle_mg, selected_triangle_index);
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

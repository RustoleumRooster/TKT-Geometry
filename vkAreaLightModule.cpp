#include <vulkan/vulkan.h>
#include "vkAreaLightModule.h"
#include "LightMaps.h"
#include "vkUtilModules.h"

#define PRINTV(x) << x.X <<","<<x.Y<<","<<x.Z<<" "

REFLECT_STRUCT3_BEGIN(AreaLight_Module)
	REFLECT_STRUCT_MEMBER(indices)
	REFLECT_STRUCT_MEMBER(vertices)
	REFLECT_STRUCT_MEMBER(lm_uvs)
	REFLECT_STRUCT_MEMBER(bvh_nodes)
	REFLECT_STRUCT_MEMBER(edges)
	REFLECT_STRUCT_MEMBER(scratchpad)
	REFLECT_STRUCT_MEMBER(area_lights)
	REFLECT_STRUCT_MEMBER(images_in)
	REFLECT_STRUCT_MEMBER(images_out)
	REFLECT_STRUCT_MEMBER_FORWARD(images_in, images_out)
REFLECT_STRUCT3_END()

AreaLight_Module::AreaLight_Module(Vulkan_App* vulkan, Geometry_Module* geo_mod, Lightmap_Configuration* configuration) :
	Vulkan_Module(vulkan)
{
	set_ptrs();

	Geometry_Assets* geo_assets = geo_mod->geo_assets;

	indices_soa = &geo_assets->indices_soa;
	vertices_soa = &geo_assets->vertices_soa;
	lightsource_indices_soa = &geo_assets->area_light_indices;
	n_indices = geo_assets->indices_soa.data.size();
	n_vertices = geo_assets->vertices_soa.data0.size();
	n_nodes = geo_assets->bvh.node_count;
	selected_triangle_mg = geo_assets->selected_triangle_mg;
	selected_triangle_index = geo_assets->selected_triangle_index;
	selected_triangle_bary_coords = geo_assets->selected_triangle_bary_coords;

	materials = &configuration->get_materials();

	reflect::connect(&geo_mod->vertices,&this->vertices);
	reflect::connect(&geo_mod->indices,&this->indices);
	reflect::connect(&geo_mod->lm_uvs,&this->lm_uvs);
	reflect::connect(&geo_mod->bvh_nodes,&this->bvh_nodes);
	reflect::connect(&geo_mod->edges, &this->edges);
	reflect::connect(&geo_mod->scratchpad, &this->scratchpad);
	reflect::connect(&geo_mod->area_lights, &this->area_lights);
}

void AreaLight_Module::createDescriptorSets(int lightmap_n)
{
	MyDescriptorWriter writer(*descriptorSetLayout, *m_DescriptorPool);

	descriptorSets.resize(1);

	writer.writeBuffer(0, ubo.getDescriptorBufferInfo());
	writer.writeBuffer(1, indices.X->getDescriptorBufferInfo());
	writer.writeBuffer(2, vertices.X->getDescriptorBufferInfo());
	writer.writeBuffer(3, lm_uvs.X->getDescriptorBufferInfo());
	writer.writeBuffer(4, bvh_nodes.X->getDescriptorBufferInfo());
	writer.writeBuffer(5, edges.X->getDescriptorBufferInfo());
	writer.writeBuffer(6, area_lights.X->getDescriptorBufferInfo());
	writer.writeBuffer(7, scratchpad.X->getDescriptorBufferInfo());
	writer.writeImage(8, images_in.X->getDescriptorBufferInfo(lightmap_n));
	writer.build(descriptorSets[0]);
}

void AreaLight_Module::createDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.resize(9);

	bindings[0] = ubo.getDescriptorSetLayout(0);
	bindings[1] = indices.X->getDescriptorSetLayout(1);
	bindings[2] = vertices.X->getDescriptorSetLayout(2);
	bindings[3] = lm_uvs.X->getDescriptorSetLayout(3);
	bindings[4] = bvh_nodes.X->getDescriptorSetLayout(4);
	bindings[5] = edges.X->getDescriptorSetLayout(5);
	bindings[6] = area_lights.X->getDescriptorSetLayout(6);
	bindings[7] = scratchpad.X->getDescriptorSetLayout(7);
	bindings[8] = images_out.X->getDescriptorSetLayout(8);

	descriptorSetLayout = new MyDescriptorSetLayout(m_device, bindings);
}

void AreaLight_Module::createUBO()
{
	ubo.range = sizeof(RayTraceInfo);

	m_device->createBuffer(ubo.range, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, ubo.Buffer,
		ubo.BufferMemory);
}

void AreaLight_Module::writeUBO2(int mg, int triangle_offset)
{
	RayTraceInfo info{};

	info.n_vertices = n_vertices;
	info.n_triangles = n_indices / 3;
	info.n_rays = 256;
	info.n_nodes = n_nodes;

	info.face_index_offset = triangle_offset;
	info.face_n_indices = 3;
	info.face_vertex_offset = 0; //unused

	//std::cout << "Vertex offset " << info.face_vertex_offset << "\n";
	//std::cout << "index offset " << info.face_index_offset << "\n";
	//std::cout << "n indices " << info.face_n_indices << "\n";

	info.lightmap_width = materials->data()[mg].lightmap_size;
	info.lightmap_height = materials->data()[mg].lightmap_size;

	int idx0, idx1, idx2;
	idx0 = indices_soa->data[triangle_offset].x;
	idx1 = indices_soa->data[triangle_offset + 1].x;
	idx2 = indices_soa->data[triangle_offset + 2].x;
	//info.in_coords = selected_triangle_bary_coords;
	
	info.in_coords = vertices_soa->data1[idx0].V * selected_triangle_bary_coords.X +
		vertices_soa->data1[idx1].V * selected_triangle_bary_coords.Y +
		vertices_soa->data1[idx2].V * selected_triangle_bary_coords.Z;
	
	//info.in_coords = vector3df(0.120613, 0.906483, 0);
	//info.in_coords = vector3df(0.125543, 0.793006, 0);
		

	//cout PRINTV(vertices_soa->data1[idx0].V) << "\n";
	//cout PRINTV(vertices_soa->data1[idx1].V) << "\n";
	//cout PRINTV(vertices_soa->data1[idx2].V) << "\n";
	//cout << "bary coords = " PRINTV(info.in_coords) << "\n";

	//======================================
	// Struct is paired uints: index,intensity
	//
	info.n_lights = lightsource_indices_soa->size()/2;

	ubo.writeToBuffer(m_device->getDevice(),(void*)&info);
}

void AreaLight_Module::writeUBO(int info_n)
{
	RayTraceInfo info{};

	info.n_vertices = n_vertices;
	info.n_triangles = n_indices / 3;
	info.n_rays = 256;
	info.n_nodes = n_nodes;

	info.face_index_offset = indices_soa->offset[info_n];
	info.face_n_indices = indices_soa->len[info_n];
	info.face_vertex_offset = vertices_soa->offset[info_n];

	//std::cout << "Vertex offset " << info.face_vertex_offset << "\n";
	//std::cout << "index offset " << info.face_index_offset << "\n";
	//std::cout << "n indices " << info.face_n_indices << "\n";

	info.lightmap_width = materials->data()[info_n].lightmap_size;
	info.lightmap_height = materials->data()[info_n].lightmap_size;

	//======================================
	// Struct is paired uints: index,intensity
	//
	info.n_lights = lightsource_indices_soa->size()/2;

	ubo.writeToBuffer(m_device->getDevice(), (void*)&info);
}

void AreaLight_Module::runMaterial(int mat_n)
{
	writeUBO(mat_n);

	createDescriptorSets(materials->data()[mat_n].lightmap_no);

	VkCommandBuffer commandBuffer = m_device->beginSingleTimeCommands();

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipeline->getPipeline());

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1,
		&descriptorSets[0], 0, 0);

	uint32_t n_WorkGroups_x = indices_soa->len[mat_n] / 3;

	std::cout << "execute " << n_WorkGroups_x << " workgroups\n";

	vkCmdDispatch(commandBuffer, n_WorkGroups_x, 1, 1);

	m_device->endSingleTimeCommands(commandBuffer);

	m_DescriptorPool->freeDescriptorsSets(descriptorSets);

	vkDeviceWaitIdle(m_device->getDevice());

}

void AreaLight_Module::runTriangle(int mat_n, int triangle_offset)
{
	//int lm_size = materials->data()[selected_triangle_mg].lightmap_size;
	//triangle_trans->get_uvs_for_triangle(selected_triangle_index, lm_size, w0, w1, w2);

	writeUBO2(mat_n, triangle_offset);

	createDescriptorSets(materials->data()[mat_n].lightmap_no);

	VkCommandBuffer commandBuffer = m_device->beginSingleTimeCommands();

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipeline->getPipeline());

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1,
		&descriptorSets[0], 0, 0);

	std::cout << "execute...\n";
	vkCmdDispatch(commandBuffer, 1, 1, 1);

	m_device->endSingleTimeCommands(commandBuffer);

	m_DescriptorPool->freeDescriptorsSets(descriptorSets);

	vkDeviceWaitIdle(m_device->getDevice());
	
}

void AreaLight_Module::run()
{
	n_lightsource_indices = lightsource_indices_soa->size();

	if (n_lightsource_indices == 0)
	{
		cout << "no lights present, cannot run shader\n";
		return;
	}

	createDescriptorSetLayout();
	createComputePipeline("shaders/arealight.spv");
	createUBO();

	triangle_trans = new Triangle_Transformer(*vertices_soa, *indices_soa);

	cout << "Running material " << selected_triangle_mg << ", triangle " << selected_triangle_index << "\n";

	vector3df w0, w1, w2;

	//runTriangle(selected_triangle_mg, selected_triangle_index);
	
	for (int i = 0; i < materials->size(); i++)
	{
		if (materials->data()[i].has_lightmap)
			runMaterial(i);
	}

	read_results();

	delete triangle_trans;

	cleanup();

	images_out.X = images_in.X;
	images_out.ready = true;
}

void AreaLight_Module::read_results()
{
	aligned_vec3* hit_results = NULL;

	uint16_t bSize = 256 * 2;
	VkDeviceSize bufferSize = sizeof(aligned_vec3) * bSize;

	hit_results = new aligned_vec3[bSize];
	for (int i = 0; i < bSize; i++) {
		hit_results[i].V = vector3df{ 0,0,0 };
	}

	MyBufferObject stagingBuffer(m_device, sizeof(aligned_vec3), 256 * 2, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	m_device->copyBuffer(scratchpad.X->Buffer, stagingBuffer.getBuffer(), sizeof(aligned_vec3) * 256 * 2);

	stagingBuffer.readFromBuffer((void*)hit_results);

	for (int i = 0; i <256; i++)
	{
		//cout PRINTV(hit_results[i].V) << " ";
		//graph.lines.push_back(line3df(hit_results[i].V, hit_results[256 + i].V));
	}

	for (int i = 0; i < 10; i++)
	{
		//cout PRINTV(hit_results[i].V) << "\n";
	}

	delete[] hit_results;

}

void AreaLight_Module::cleanup()
{
	descriptorSetLayout->cleanup();

	ubo.destroy(m_device->getDevice());

	pipeline->cleanup();

	vkDestroyPipelineLayout(m_device->getDevice(), pipelineLayout, nullptr);
}



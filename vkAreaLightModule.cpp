#include <vulkan/vulkan.h>
#include "vkAreaLightModule.h"
#include "LightMaps.h"

#define PRINTV(x) << x.X <<","<<x.Y<<","<<x.Z<<" "

void AreaLight_Module::createDescriptorSets(int lightmap_n)
{
	MyDescriptorWriter writer(*descriptorSetLayout, *m_DescriptorPool);

	descriptorSets.resize(1);

	VkDescriptorBufferInfo raytraceInfo{};
	raytraceInfo.buffer = raytraceBufferObject->getBuffer();
	raytraceInfo.offset = 0;
	raytraceInfo.range = sizeof(RayTraceInfo);

	VkDescriptorBufferInfo indexBufferInfo{};
	indexBufferInfo.buffer = indexBuffer;
	indexBufferInfo.offset = 0;
	indexBufferInfo.range = sizeof(aligned_uint) * n_indices;

	VkDescriptorBufferInfo vertexBufferInfo{};
	vertexBufferInfo.buffer = vertexBuffer;
	vertexBufferInfo.offset = 0;
	vertexBufferInfo.range = sizeof(aligned_vec3) * n_vertices;

	VkDescriptorBufferInfo uvBufferInfo{};
	uvBufferInfo.buffer = uvBuffer;
	uvBufferInfo.offset = 0;
	uvBufferInfo.range = sizeof(aligned_vec3) * n_vertices;

	VkDescriptorBufferInfo nodeBufferInfo{};
	nodeBufferInfo.buffer = nodeBuffer;
	nodeBufferInfo.offset = 0;
	nodeBufferInfo.range = sizeof(BVH_node_gpu) * n_nodes;

	VkDescriptorBufferInfo edgeBufferInfo{};
	edgeBufferInfo.buffer = edgeBuffer;
	edgeBufferInfo.offset = 0;
	edgeBufferInfo.range = sizeof(aligned_uint) * n_indices;

	VkDescriptorBufferInfo lightSourceBufferInfo{};
	lightSourceBufferInfo.buffer = lightSourceBuffer;
	lightSourceBufferInfo.offset = 0;
	lightSourceBufferInfo.range = sizeof(aligned_uint) * n_lightsource_indices;

	VkDescriptorBufferInfo outputBufferInfo{};
	outputBufferInfo.buffer = hitResultsBufferObject->getBuffer();
	outputBufferInfo.offset = 0;
	outputBufferInfo.range = sizeof(aligned_vec3) * 512;

	VkDescriptorImageInfo imageStorageInfo{};
	imageStorageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageStorageInfo.imageView = lightmapImageViews[lightmap_n];

	writer.writeBuffer(0, raytraceInfo);
	writer.writeBuffer(1, indexBufferInfo);
	writer.writeBuffer(2, vertexBufferInfo);
	writer.writeBuffer(3, uvBufferInfo);
	writer.writeBuffer(4, nodeBufferInfo);
	writer.writeBuffer(5, edgeBufferInfo);
	writer.writeBuffer(6, lightSourceBufferInfo);
	writer.writeBuffer(7, outputBufferInfo);
	writer.writeImage(8, imageStorageInfo);
	writer.build(descriptorSets[0]);
}


void AreaLight_Module::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding raytraceBufferBinding{};
	raytraceBufferBinding.binding = 0;
	raytraceBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	raytraceBufferBinding.descriptorCount = 1;
	raytraceBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	raytraceBufferBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding indexBufferBinding{};
	indexBufferBinding.binding = 1;
	indexBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	indexBufferBinding.descriptorCount = 1;
	indexBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	indexBufferBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding vertexBufferBinding{};
	vertexBufferBinding.binding = 2;
	vertexBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	vertexBufferBinding.descriptorCount = 1;
	vertexBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	vertexBufferBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding uvBufferBinding{};
	uvBufferBinding.binding = 3;
	uvBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	uvBufferBinding.descriptorCount = 1;
	uvBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	uvBufferBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding nodeBufferBinding{};
	nodeBufferBinding.binding = 4;
	nodeBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	nodeBufferBinding.descriptorCount = 1;
	nodeBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	nodeBufferBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding edgeBufferBinding{};
	edgeBufferBinding.binding = 5;
	edgeBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	edgeBufferBinding.descriptorCount = 1;
	edgeBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	edgeBufferBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding lightsourceBufferBinding{};
	lightsourceBufferBinding.binding = 6;
	lightsourceBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	lightsourceBufferBinding.descriptorCount = 1;
	lightsourceBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	lightsourceBufferBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding outputBufferBinding{};
	outputBufferBinding.binding = 7;
	outputBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	outputBufferBinding.descriptorCount = 1;
	outputBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	outputBufferBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding imageStorageBinding{};
	imageStorageBinding.binding = 8;
	imageStorageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	imageStorageBinding.descriptorCount = 1;
	imageStorageBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	imageStorageBinding.pImmutableSamplers = nullptr; // Optional

	std::vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.resize(9);
	bindings = { raytraceBufferBinding, indexBufferBinding, vertexBufferBinding, 
		uvBufferBinding, nodeBufferBinding, edgeBufferBinding, lightsourceBufferBinding, outputBufferBinding, imageStorageBinding };

	descriptorSetLayout = new MyDescriptorSetLayout(m_device, bindings);
}

void AreaLight_Module::createComputePipeline()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &(descriptorSetLayout->getDescriptorSetLayout());

	if (vkCreatePipelineLayout(m_device->getDevice(), &pipelineLayoutInfo, nullptr,
		&pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create compute pipeline layout!");
	}

	pipeline = new ComputePipeline(m_device, "shaders/arealight.spv", pipelineLayout);
}

void AreaLight_Module::createRaytraceInfoBuffer()
{
	raytraceBufferObject = new MyBufferObject(m_device, sizeof(RayTraceInfo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 16);
}

void AreaLight_Module::writeRaytraceInfoBuffer2(int mg, int triangle_offset)
{
	RayTraceInfo info{};

	info.n_vertices = n_vertices;
	info.n_triangles = n_indices / 3;
	info.n_rays = 256;
	info.n_nodes = n_nodes;

	info.face_index_offset = triangle_offset;
	info.face_n_indices = 3;
	info.face_vertex_offset = 0; //unused

	std::cout << "Vertex offset " << info.face_vertex_offset << "\n";
	std::cout << "index offset " << info.face_index_offset << "\n";
	std::cout << "n indices " << info.face_n_indices << "\n";

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
		

	cout PRINTV(vertices_soa->data1[idx0].V) << "\n";
	cout PRINTV(vertices_soa->data1[idx1].V) << "\n";
	cout PRINTV(vertices_soa->data1[idx2].V) << "\n";
	cout << "bary coords = " PRINTV(info.in_coords) << "\n";

	info.n_lights = lightsource_indices_soa->size();


	raytraceBufferObject->writeToBuffer((void*)&info);
}

void AreaLight_Module::writeRaytraceInfoBuffer(int info_n)
{
	RayTraceInfo info{};

	info.n_vertices = n_vertices;
	info.n_triangles = n_indices / 3;
	info.n_rays = 256;
	info.n_nodes = n_nodes;

	info.face_index_offset = indices_soa->offset[info_n];
	info.face_n_indices = indices_soa->len[info_n];
	info.face_vertex_offset = vertices_soa->offset[info_n];

	std::cout << "Vertex offset " << info.face_vertex_offset << "\n";
	std::cout << "index offset " << info.face_index_offset << "\n";
	std::cout << "n indices " << info.face_n_indices << "\n";

	info.lightmap_width = materials->data()[info_n].lightmap_size;
	info.lightmap_height = materials->data()[info_n].lightmap_size;

	info.n_lights = lightsource_indices_soa->size();


	raytraceBufferObject->writeToBuffer((void*)&info);
}

void AreaLight_Module::runMaterial(int mat_n)
{
	writeRaytraceInfoBuffer(mat_n);
	createDescriptorSets(materials->data()[mat_n].lightmap_no);

	VkCommandBuffer commandBuffer = m_device->beginSingleTimeCommands();

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipeline->getPipeline());

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1,
		&descriptorSets[0], 0, 0);

	uint32_t n_WorkGroups_x = indices_soa->len[mat_n] / 3;

	std::cout << n_WorkGroups_x << " workgroups\n";

	std::cout << "execute...\n";
	vkCmdDispatch(commandBuffer, n_WorkGroups_x, 1, 1);

	m_device->endSingleTimeCommands(commandBuffer);

	m_DescriptorPool->freeDescriptorsSets(descriptorSets);

	vkDeviceWaitIdle(m_device->getDevice());

}

void AreaLight_Module::runTriangle(int mat_n, int triangle_offset)
{
	//int lm_size = materials->data()[selected_triangle_mg].lightmap_size;

	//triangle_trans->get_uvs_for_triangle(selected_triangle_index, lm_size, w0, w1, w2);

	
	writeRaytraceInfoBuffer2(mat_n, triangle_offset);

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
	createComputePipeline();
	createRaytraceInfoBuffer();
	createHitResultsBuffer();

	triangle_trans = new Triangle_Transformer(*vertices_soa, *indices_soa);

	cout << "Running material " << selected_triangle_mg << ", triangle " << selected_triangle_index << "\n";

	vector3df w0, w1, w2;

	

	//runTriangle(selected_triangle_mg, selected_triangle_index);
	
	for (int i = 0; i < materials->size(); i++)
	{
		if (materials->data()[i].has_lightmap)
			runMaterial(i);
	}

	//read_results();

	delete triangle_trans;

	cleanup();
}

void AreaLight_Module::createHitResultsBuffer() {

	uint16_t bSize = 256 * 2;
	VkDeviceSize bufferSize = sizeof(aligned_vec3) * bSize;

	hit_results = new aligned_vec3[bSize];
	for (int i = 0; i < bSize; i++) {
		hit_results[i].V = vector3df{ 0,0,0 };
	}

	hitResultsBufferObject = new MyBufferObject(m_device, sizeof(aligned_vec3), bSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 16);
}

void AreaLight_Module::read_results()
{
	MyBufferObject stagingBuffer(m_device, sizeof(aligned_vec3), 256 * 2, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	m_device->copyBuffer(hitResultsBufferObject->getBuffer(), stagingBuffer.getBuffer(), sizeof(aligned_vec3) * 256 * 2);

	stagingBuffer.readFromBuffer((void*)hit_results);

	for (int i = 0; i <256; i++)
	{
		//cout PRINTV(hit_results[i].V) << " ";
		graph.lines.push_back(line3df(hit_results[i].V, hit_results[256 + i].V));
	}

	for (int i = 0; i < 10; i++)
	{
		//cout PRINTV(hit_results[i].V) << "\n";
	}
	//cout PRINTV(hit_results[0].V) << "\n";
	//cout PRINTV(hit_results[1].V) << "\n";
	//cout PRINTV(hit_results[2].V) << "\n";
	//cout PRINTV(hit_results[3].V) << "\n";
	//cout PRINTV(hit_results[4].V) << "\n";
	//cout PRINTV(hit_results[5].V) << "\n";
	
	delete hitResultsBufferObject;

	delete[] hit_results;

}

void AreaLight_Module::cleanup()
{
	descriptorSetLayout->cleanup();

	delete raytraceBufferObject;

	pipeline->cleanup();

	vkDestroyPipelineLayout(m_device->getDevice(), pipelineLayout, nullptr);
}



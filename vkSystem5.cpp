#include <sstream>

#include "vkSystem5.h"
#include "vkDescriptors.h"
#include "vkComputePipeline.h"
#include "vkBufferObject.h"

#include "vkSystem4.h"
#include "utils.h"
#include "BufferManager.h"

void System5::cleanup() {
	descriptorSetLayout->cleanup();

	vkDestroyBuffer(device->getDevice(), indexBuffer, nullptr);
	vkFreeMemory(device->getDevice(), indexBufferMemory, nullptr);

	vkDestroyBuffer(device->getDevice(), vertexBuffer, nullptr);
	vkFreeMemory(device->getDevice(), vertexBufferMemory, nullptr);

	vkDestroyBuffer(device->getDevice(), uvBuffer, nullptr);
	vkFreeMemory(device->getDevice(), uvBufferMemory, nullptr);

	delete raytraceBufferObject;
	delete hitResultsBufferObject;

	pipeline->cleanup();

	delete[] hit_results;

	for (auto& imgView : lightmapImageViews)
		vkDestroyImageView(device->getDevice(), imgView, nullptr);

	for (auto& img : lightmapImages)
		vkDestroyImage(device->getDevice(), img, nullptr);

	for (auto& imgMem : lightmapsMemory)
		vkFreeMemory(device->getDevice(), imgMem, nullptr);

	vkDestroyPipelineLayout(device->getDevice(), pipelineLayout, nullptr);
}

void System5::loadModel(MeshNode_Interface_Final* meshnode)
{

	writeLightmapsInfo(meshnode->getMaterialsUsed(), lightmaps_info);

	createLightmapImages();

	fill_vertex_struct(meshnode->getMesh(), vertices_soa);
	fill_index_struct(meshnode->getMesh(), indices_soa);

	n_indices = indices_soa.data.size();
	n_vertices = vertices_soa.data0.size();

	std::cout << "model has " << n_vertices << " vertices, " << n_indices << " indexes\n";
}

void System5::createDescriptorSets(int lightmap_n) 
{
	MyDescriptorWriter writer(*descriptorSetLayout, *descriptorPool);

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
	/*
	VkDescriptorBufferInfo normalBufferInfo{};
	normalBufferInfo.buffer = normalBuffer;
	normalBufferInfo.offset = 0;
	normalBufferInfo.range = sizeof(aligned_vec3) * ( n_indices / 3 );
	*/
	VkDescriptorBufferInfo hitResultsInfo{};
	hitResultsInfo.buffer = hitResultsBufferObject->getBuffer();
	hitResultsInfo.offset = 0;
	hitResultsInfo.range = sizeof(aligned_vec3) * N_RAYS * 2;

	VkDescriptorImageInfo imageStorageInfo{};
	imageStorageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageStorageInfo.imageView = lightmapImageViews[lightmap_n];

	writer.writeBuffer(0, raytraceInfo);
	writer.writeBuffer(1, indexBufferInfo);
	writer.writeBuffer(2, vertexBufferInfo); 
	writer.writeBuffer(3, uvBufferInfo);
	//writer.writeBuffer(4, normalBufferInfo);
	writer.writeBuffer(4, hitResultsInfo);
	writer.writeImage(5, imageStorageInfo);
	writer.build(descriptorSets[0]);
}


void System5::createDescriptorSetLayout()
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

	/*
	VkDescriptorSetLayoutBinding normalBufferBinding{};
	normalBufferBinding.binding = 4;
	normalBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	normalBufferBinding.descriptorCount = 1;
	normalBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	normalBufferBinding.pImmutableSamplers = nullptr; // Optional
	*/

	VkDescriptorSetLayoutBinding hitResultsBinding{};
	hitResultsBinding.binding = 4;
	hitResultsBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	hitResultsBinding.descriptorCount = 1;
	hitResultsBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	hitResultsBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding imageStorageBinding{};
	imageStorageBinding.binding = 5;
	imageStorageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	imageStorageBinding.descriptorCount = 1;
	imageStorageBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	imageStorageBinding.pImmutableSamplers = nullptr; // Optional

	std::vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.resize(6);
	bindings = { raytraceBufferBinding, indexBufferBinding, vertexBufferBinding, uvBufferBinding, /*normalBufferBinding, */hitResultsBinding, imageStorageBinding};

	descriptorSetLayout = new MyDescriptorSetLayout(device, bindings);
}

void System5::createComputePipeline()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &(descriptorSetLayout->getDescriptorSetLayout());

	if (vkCreatePipelineLayout(device->getDevice(), &pipelineLayoutInfo, nullptr,
		&pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create compute pipeline layout!");
	}

	pipeline = new ComputePipeline(device, "shaders/mesh_triangles.spv", pipelineLayout);
}

void System5::createLightmapImages()
{
	for (int i = 0; i < lightmaps_info.size(); i++)
	{
		VkDeviceSize lm_size = lightmaps_info[i].size;

		VkImage img;
		VkDeviceMemory imgMemory;
		VkImageView imgView;

		device->createImage(lm_size, lm_size, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, img,
			imgMemory);

		imgView = device->createImageView(img, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

		device->transitionImageLayout(img, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

		lightmapImages.push_back(img);
		lightmapsMemory.push_back(imgMemory);
		lightmapImageViews.push_back(imgView);
	}
}

void System5::createRaytraceInfoBuffer()
{
	raytraceBufferObject = new MyBufferObject(device, sizeof(RayTraceInfo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 16);
}

void System5::createHitResultsBuffer() {

	uint16_t bSize = 256 * 2;
	VkDeviceSize bufferSize = sizeof(aligned_vec3) * bSize;

	hit_results = new aligned_vec3[bSize];
	for (int i = 0; i < bSize; i++) {
		hit_results[i].V = vector3df{ 0,0,0 };
	}

	hitResultsBufferObject = new MyBufferObject(device, sizeof(aligned_vec3), bSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 16);
}

void System5::createIndexBuffer() {

	VkDeviceSize bufferSize = sizeof(aligned_uint) * n_indices;

	MyBufferObject stagingBuffer(device, sizeof(aligned_uint), n_indices, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	stagingBuffer.writeToBuffer((void*)indices_soa.data.data());

	device->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer,
		indexBufferMemory);

	device->copyBuffer(stagingBuffer.getBuffer(), indexBuffer, bufferSize);
}


void System5::createVertexBuffer() {

	VkDeviceSize bufferSize = sizeof(aligned_vec3) * n_vertices;

	MyBufferObject stagingBuffer(device, sizeof(aligned_vec3), n_vertices, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	stagingBuffer.writeToBuffer((void*)vertices_soa.data0.data());

	device->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer,
		vertexBufferMemory);

	device->copyBuffer(stagingBuffer.getBuffer(), vertexBuffer, bufferSize);
}

void System5::createUVBuffer()
{
	VkDeviceSize bufferSize = sizeof(aligned_vec3) * n_vertices;

	MyBufferObject stagingBuffer(device, sizeof(aligned_vec3), n_vertices, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	stagingBuffer.writeToBuffer((void*)vertices_soa.data1.data());

	device->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uvBuffer,
		uvBufferMemory);

	device->copyBuffer(stagingBuffer.getBuffer(), uvBuffer, bufferSize);
}

void System5::writeRaytraceInfoBuffer(int lightmap_n)
{
	RayTraceInfo info{};


	info.n_vertices = n_vertices;
	info.n_triangles = n_indices / 3;
	info.n_rays = N_RAYS;

	info.face_index_offset = indices_soa.offset[lightmap_n];
	info.face_n_indices = indices_soa.len[lightmap_n];
	info.face_vertex_offset = vertices_soa.offset[lightmap_n];

	std::cout << "Vertex offset " << info.face_vertex_offset << "\n";
	std::cout << "index offset " << info.face_index_offset << "\n";
	std::cout << "n indices " << info.face_n_indices <<  "\n";
	
	info.lightmap_width = lightmaps_info[lightmap_n].size;
	info.lightmap_height = lightmaps_info[lightmap_n].size;

	raytraceBufferObject->writeToBuffer((void*)&info);
}

void System5::buildLightmap(int lightmap_n)
{
	writeRaytraceInfoBuffer(lightmap_n);
	createDescriptorSets(lightmap_n);

	VkCommandBuffer commandBuffer = device->beginSingleTimeCommands();

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipeline->getPipeline());

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1,
		&descriptorSets[0], 0, 0);

	uint32_t n_WorkGroups_x = indices_soa.len[lightmap_n] / 3;
	std::cout << n_WorkGroups_x << " workgroups\n";

	std::cout << "execute...\n";
	vkCmdDispatch(commandBuffer, n_WorkGroups_x, 1, 1);


	device->endSingleTimeCommands(commandBuffer);

	descriptorPool->freeDescriptorsSets(descriptorSets);

	vkDeviceWaitIdle(device->getDevice());

	MyBufferObject stagingBuffer(device, sizeof(aligned_vec3), N_RAYS * 2, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	device->copyBuffer(hitResultsBufferObject->getBuffer(), stagingBuffer.getBuffer(), sizeof(aligned_vec3) * N_RAYS * 2);

	stagingBuffer.readFromBuffer((void*)hit_results);

	for (int i = 0; i < 256; i++)
	{
		//std::cout << hit_results[i].V.X << ", ";
		//std::cout << hit_results[i].V.X << "," << hit_results[i].V.Y << "  ";
		//std::cout << hit_results[i].V.X << "," << hit_results[i].V.Y << "," << hit_results[i].V.Z << ",  ";
		//std::cout << hit_results[256+i].V.X << "," << hit_results[256+i].V.Y << "," << hit_results[256+ i].V.Z << "\n";
	}
	//std::cout << "\n";
}

void System5::executeComputeShader() 
{
	for (int i = 0; i < lightmaps_info.size(); i++)
	{
		buildLightmap(i);
	}

	System4* edges_compute_shader = new System4(device);

	edges_compute_shader->setDescriptorPool(descriptorPool);
	edges_compute_shader->createDescriptorSetLayout();
	edges_compute_shader->createComputePipeline();

	for (int i = 0; i < lightmapImages.size(); i++)
	{
		uint16_t width = lightmaps_info[i].size;
		VkDeviceSize imgSize = width * width * 4;
		/*
		MyBufferObject stagingBuffer(device, imgSize, 1, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

		device->transitionImageLayout(lightmapImages[i], VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		device->copyImageToBuffer(stagingBuffer.getBuffer(), lightmapImages[i], static_cast<uint32_t>(width), static_cast<uint32_t>(width));

		unsigned char* pixels = (unsigned char*) malloc(imgSize);

		stagingBuffer.readFromBuffer(pixels);

		std::stringstream ss;
		ss << "lightmap_" << i << ".bmp";

		generateBitmapImage(pixels, width, width, ss.str().c_str());

		free(pixels);
		*/

		//device->transitionImageLayout(lightmapImages[i], VK_FORMAT_R8G8B8A8_UNORM,
		//	VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		////////////////////////////

		
		VkDeviceSize lm_size = lightmaps_info[i].size;

		VkImage img;
		VkDeviceMemory imgMemory;
		VkImageView imgView;

		device->createImage(lm_size, lm_size, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, img,
			imgMemory);

		imgView = device->createImageView(img, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

		device->transitionImageLayout(img, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

		edges_compute_shader->setLightmapIn(lightmapImages[i], lightmapsMemory[i], lightmapImageViews[i], lm_size);
		edges_compute_shader->setLightmapOut(img, imgMemory, imgView);
		edges_compute_shader->execute();


		////////////////////////////////////////
		/*
		MyBufferObject stagingBuffer(device, imgSize, 1, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

		device->transitionImageLayout(img, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		device->copyImageToBuffer(stagingBuffer.getBuffer(), img, static_cast<uint32_t>(width), static_cast<uint32_t>(width));

		unsigned char* pixels = (unsigned char*)malloc(imgSize);

		stagingBuffer.readFromBuffer(pixels);

		std::stringstream ss;
		ss << "bmp_test.bmp";

		generateBitmapImage(pixels, width, width, ss.str().c_str());

		free(pixels);

		device->transitionImageLayout(lightmapImages[i], VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			*/
		/////////////////////////////////

		
		vkDestroyImageView(device->getDevice(), lightmapImageViews[i], nullptr);
		vkDestroyImage(device->getDevice(), lightmapImages[i], nullptr);
		vkFreeMemory(device->getDevice(), lightmapsMemory[i], nullptr);

		lightmapImages[i] = img;
		lightmapImageViews[i] = imgView;
		lightmapsMemory[i] = imgMemory;
		

		///////////////////////////////
		
		MyBufferObject stagingBuffer(device, imgSize, 1, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

		device->transitionImageLayout(lightmapImages[i], VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		device->copyImageToBuffer(stagingBuffer.getBuffer(), lightmapImages[i], static_cast<uint32_t>(width), static_cast<uint32_t>(width));

		unsigned char* pixels = (unsigned char*)malloc(imgSize);

		stagingBuffer.readFromBuffer(pixels);

		std::stringstream ss;
		ss << "../projects/export/lightmap_" << i << ".bmp";

		generateBitmapImage(pixels, width, width, ss.str().c_str());

		free(pixels);
		

		device->transitionImageLayout(lightmapImages[i], VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	edges_compute_shader->cleanup();

	delete edges_compute_shader;
}
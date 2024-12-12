#include <sstream>

#include "vkCompute_Light_Multipass.h"
#include "vkDescriptors.h"
#include "vkComputePipeline.h"
#include "vkBufferObject.h"

#include "vkSystem4.h"
#include "utils.h"
#include "BufferManager.h"
#include "geometry_scene.h"
#include "custom_nodes.h"

void System_Light_Multipass::cleanup() {
	descriptorSetLayout->cleanup();

	vkDestroyBuffer(device->getDevice(), indexBuffer, nullptr);
	vkFreeMemory(device->getDevice(), indexBufferMemory, nullptr);

	vkDestroyBuffer(device->getDevice(), vertexBuffer, nullptr);
	vkFreeMemory(device->getDevice(), vertexBufferMemory, nullptr);

	vkDestroyBuffer(device->getDevice(), uvBuffer, nullptr);
	vkFreeMemory(device->getDevice(), uvBufferMemory, nullptr);

	vkDestroyBuffer(device->getDevice(), nodeBuffer, nullptr);
	vkFreeMemory(device->getDevice(), nodeBufferMemory, nullptr);

	vkDestroyBuffer(device->getDevice(), lightSourceBuffer, nullptr);
	vkFreeMemory(device->getDevice(), lightSourceBufferMemory, nullptr);

	vkDestroyBuffer(device->getDevice(), quadsBuffer, nullptr);
	vkFreeMemory(device->getDevice(), quadsBufferMemory, nullptr);

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

	for (auto& imgView : workingImageViews)
		vkDestroyImageView(device->getDevice(), imgView, nullptr);

	for (auto& img : workingImages)
		vkDestroyImage(device->getDevice(), img, nullptr);

	for (auto& imgMem : workingImagesMemory)
		vkFreeMemory(device->getDevice(), imgMem, nullptr);

	vkDestroyPipelineLayout(device->getDevice(), pipelineLayout, nullptr);
}

typedef CMeshBuffer<video::S3DVertex2TCoords> mesh_buffer_type;


bool System_Light_Multipass::verify_inputs(geometry_scene* geo_scene)
{/*
	std::vector<Reflected_SceneNode*> scene_nodes = geo_scene->getSceneNodes();

	int count = 0;
	for (Reflected_SceneNode* node : scene_nodes)
	{
		if (strcmp(node->GetDynamicReflection()->name, "Reflected_LightSceneNode") == 0)
		{
			Reflected_LightSceneNode* lsnode = dynamic_cast<Reflected_LightSceneNode*>(node);

			if (lsnode)
			{
				count++;
			}
		}
	}

	if (count == 0)
		return false;

	MeshNode_Interface_Final* meshnode = &geo_scene->final_meshnode_interface;

	if (meshnode->getMesh() == NULL || meshnode->getMesh()->getMeshBufferCount() == 0 ||
		meshnode->getMesh()->MeshBuffers[0]->getIndexCount() == 0)
		return false;
		*/
	return true;
}

void System_Light_Multipass::loadLights(geometry_scene* geo_scene)
{
	//std::vector<Reflected_SceneNode*> scene_nodes = geo_scene->getSceneNodes();

	lightSources.clear();

	//for (Reflected_SceneNode* node : scene_nodes)
	for (ISceneNode* it : geo_scene->EditorNodes()->getChildren())
	{
		Reflected_SceneNode* node = (Reflected_SceneNode*)it;

		if (strcmp(node->GetDynamicReflection()->name, "Reflected_LightSceneNode") == 0)
		{
			Reflected_LightSceneNode* lsnode = dynamic_cast<Reflected_LightSceneNode*>(node);

			if (lsnode)
			{
				LightSource_struct ls{ lsnode->getPosition(),lsnode->light_radius };
				lightSources.push_back(ls);
			}
		}
	}
	n_lights = lightSources.size();
}

void System_Light_Multipass::loadModel(MeshNode_Interface_Final* meshnode)
{

	writeLightmapsInfo(meshnode->getMaterialsUsed(), lightmaps_info, meshnode);

	createLightmapImages();
	
	fill_vertex_struct(meshnode->getMesh(), vertices_soa);
	fill_index_struct(meshnode->getMesh(), indices_soa);

	for (int i = 0; i < vertices_soa.offset.size(); i++)
	{
		for (int j = 0; j < indices_soa.len[i]; j++)
		{
			indices_soa.data[indices_soa.offset[i] + j].x += vertices_soa.offset[i];
		}
	}

	n_indices = indices_soa.data.size();
	n_vertices = vertices_soa.data0.size();
	master_triangle_list.resize(indices_soa.data.size() / 3);

	for (int i = 0; i < master_triangle_list.size(); i++)
	{
		master_triangle_list[i].set((u16)(indices_soa.data[i * 3].x),(u16)(indices_soa.data[(i * 3) + 1].x),(u16)(indices_soa.data[(i * 3) + 2].x) );
	}

	faces_offset.resize(lightmaps_info.size());

	for (int i=0; i < lightmaps_info.size(); i++)
	{
		faces_offset[i] = total_n_faces;
		total_n_faces += lightmaps_info[i].faces.size();
	}

	bounding_quads.resize(total_n_faces * 4);

	int c = 0;
	for (LightMaps_Info_Struct info : lightmaps_info)
	{
		for (int i = 0; i < info.faces.size(); i++)
		{
			bounding_quads[c * 4].V = info.quads[i].verts[0];
			bounding_quads[c * 4 + 1].V = info.quads[i].verts[1];
			bounding_quads[c * 4 + 2].V = info.quads[i].verts[2];
			bounding_quads[c * 4 + 3].V = info.quads[i].verts[3];

			c++;
		}
	}

	//==========================================================
	// Create a BVH and store it for raytracing calculations in the shader
	//

	std::cout << "building BVH...\n";
	my_bvh.build(vertices_soa.data0.data(), master_triangle_list.data(), master_triangle_list.size(), NULL);

	n_nodes = my_bvh.node_count;

	for (int i = 0; i < n_nodes; i++)
	{
		if (my_bvh.nodes[i].left_node == 0xFFFF && my_bvh.nodes[i].right_node == 0xFFFF)
		{
			if (my_bvh.nodes[i].n_prims > 2)
				std::cout << "WARNING: node " << i << " has " << my_bvh.nodes[i].n_prims << " triangles, max 2\n";

			for (int j = 0; j < std::min(4u,my_bvh.nodes[i].n_prims); j++)
			{
				my_bvh.nodes[i].packing[j] = static_cast<f32> (my_bvh.indices[my_bvh.nodes[i].first_prim + j]);
			}
		}
		else
			my_bvh.nodes[i].n_prims = 0;
	}
}


void System_Light_Multipass::createDescriptorSets(int lightmap_n) 
{
	MyDescriptorWriter writer(*descriptorSetLayout, *descriptorPool);

	descriptorSets.resize(1);

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

	VkDescriptorBufferInfo lightBufferInfo{};
	lightBufferInfo.buffer = lightSourceBuffer;
	lightBufferInfo.offset = 0;
	lightBufferInfo.range = sizeof(LightSource_struct) * n_lights;

	VkDescriptorBufferInfo quadBufferInfo{};
	quadBufferInfo.buffer = quadsBuffer;
	quadBufferInfo.offset = 0;
	quadBufferInfo.range = sizeof(aligned_vec3) * total_n_faces * 4;
	
	VkDescriptorBufferInfo hitResultsInfo{};
	hitResultsInfo.buffer = hitResultsBufferObject->getBuffer();
	hitResultsInfo.offset = 0;
	hitResultsInfo.range = sizeof(aligned_vec3) * N_RAYS * 2;

	VkDescriptorBufferInfo raytraceInfo{};
	raytraceInfo.buffer = raytraceBufferObject->getBuffer();
	raytraceInfo.offset = 0;
	raytraceInfo.range = sizeof(RayTraceInfo);

	VkDescriptorBufferInfo firstTriangleBufferInfo{};
	firstTriangleBufferInfo.buffer = firstTriangleBuffer;
	firstTriangleBufferInfo.offset = 0;
	firstTriangleBufferInfo.range = sizeof(aligned_uint) * lightmaps_info[lightmap_n].faces.size();

	VkDescriptorBufferInfo nTrianglesBufferInfo{};
	nTrianglesBufferInfo.buffer = nTrianglesBuffer;
	nTrianglesBufferInfo.offset = 0;
	nTrianglesBufferInfo.range = sizeof(aligned_uint) * lightmaps_info[lightmap_n].faces.size();

	VkDescriptorImageInfo workingImageInfo{};
	workingImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	workingImageInfo.imageView = workingImageViews[lightmap_n];

	VkDescriptorImageInfo imageStorageInfo{};
	imageStorageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageStorageInfo.imageView = lightmapImageViews[lightmap_n];

	
	writer.writeBuffer(0, indexBufferInfo);
	writer.writeBuffer(1, vertexBufferInfo); 
	writer.writeBuffer(2, uvBufferInfo);
	writer.writeBuffer(3, nodeBufferInfo);
	writer.writeBuffer(4, lightBufferInfo);
	writer.writeBuffer(5, quadBufferInfo);
	writer.writeBuffer(6, hitResultsInfo);

	writer.writeBuffer(7, raytraceInfo);
	writer.writeBuffer(8, firstTriangleBufferInfo);
	writer.writeBuffer(9, nTrianglesBufferInfo);
	writer.writeImage(10, workingImageInfo);
	writer.writeImage(11, imageStorageInfo);

	writer.build(descriptorSets[0]);
}


void System_Light_Multipass::createDescriptorSetLayout()
{

	VkDescriptorSetLayoutBinding indexBufferBinding{};
	indexBufferBinding.binding = 0;
	indexBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	indexBufferBinding.descriptorCount = 1;
	indexBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	indexBufferBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding vertexBufferBinding{};
	vertexBufferBinding.binding = 1;
	vertexBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	vertexBufferBinding.descriptorCount = 1;
	vertexBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	vertexBufferBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding uvBufferBinding{};
	uvBufferBinding.binding = 2;
	uvBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	uvBufferBinding.descriptorCount = 1;
	uvBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	uvBufferBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding nodeBufferBinding{};
	nodeBufferBinding.binding = 3;
	nodeBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	nodeBufferBinding.descriptorCount = 1;
	nodeBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	nodeBufferBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding lightBufferBinding{};
	lightBufferBinding.binding = 4;
	lightBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	lightBufferBinding.descriptorCount = 1;
	lightBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	lightBufferBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding quadBufferBinding{};
	quadBufferBinding.binding = 5;
	quadBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	quadBufferBinding.descriptorCount = 1;
	quadBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	quadBufferBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding hitResultsBinding{};
	hitResultsBinding.binding = 6;
	hitResultsBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	hitResultsBinding.descriptorCount = 1;
	hitResultsBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	hitResultsBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding raytraceBufferBinding{};
	raytraceBufferBinding.binding = 7;
	raytraceBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	raytraceBufferBinding.descriptorCount = 1;
	raytraceBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	raytraceBufferBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding firstTriangleBufferBinding{};
	firstTriangleBufferBinding.binding = 8;
	firstTriangleBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	firstTriangleBufferBinding.descriptorCount = 1;
	firstTriangleBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	firstTriangleBufferBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding nTrianglesBufferBinding{};
	nTrianglesBufferBinding.binding = 9;
	nTrianglesBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	nTrianglesBufferBinding.descriptorCount = 1;
	nTrianglesBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	nTrianglesBufferBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding workingImageBinding{};
	workingImageBinding.binding = 10;
	workingImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	workingImageBinding.descriptorCount = 1;
	workingImageBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	workingImageBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding imageStorageBinding{};
	imageStorageBinding.binding = 11;
	imageStorageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	imageStorageBinding.descriptorCount = 1;
	imageStorageBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	imageStorageBinding.pImmutableSamplers = nullptr; // Optional

	std::vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.resize(12);
	bindings = {  indexBufferBinding, vertexBufferBinding, uvBufferBinding, nodeBufferBinding, lightBufferBinding, quadBufferBinding, hitResultsBinding, raytraceBufferBinding, firstTriangleBufferBinding, nTrianglesBufferBinding, workingImageBinding, imageStorageBinding};

	descriptorSetLayout = new MyDescriptorSetLayout(device, bindings);
}

void System_Light_Multipass::createComputePipeline()
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

	pipeline = new ComputePipeline(device, "shaders/multipasslight.spv", pipelineLayout);
}

void System_Light_Multipass::createLightmapImages()
{
	for (int i = 0; i < lightmaps_info.size(); i++)
	{
		VkDeviceSize lm_size = lightmaps_info[i].size;

		{
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

		{
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

			workingImages.push_back(img);
			workingImagesMemory.push_back(imgMemory);
			workingImageViews.push_back(imgView);
		}
	}
}

void System_Light_Multipass::createRaytraceInfoBuffer()
{
	raytraceBufferObject = new MyBufferObject(device, sizeof(RayTraceInfo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 16);
}

void System_Light_Multipass::createHitResultsBuffer() {

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

void System_Light_Multipass::createIndexBuffer() {

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


void System_Light_Multipass::createVertexBuffer() {

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

void System_Light_Multipass::createUVBuffer()
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

void System_Light_Multipass::createQuadBuffer() {

	VkDeviceSize bufferSize = sizeof(aligned_vec3) * total_n_faces * 4;

	MyBufferObject stagingBuffer(device, sizeof(aligned_vec3), total_n_faces * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	stagingBuffer.writeToBuffer((void*)bounding_quads.data());

	device->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, quadsBuffer,
		quadsBufferMemory);

	device->copyBuffer(stagingBuffer.getBuffer(), quadsBuffer, bufferSize);
}

void System_Light_Multipass::createLightsBuffer() {

	VkDeviceSize bufferSize = sizeof(LightSource_struct) * n_lights;

	MyBufferObject stagingBuffer(device, sizeof(LightSource_struct), n_lights, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	stagingBuffer.writeToBuffer((void*)lightSources.data());

	device->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, lightSourceBuffer,
		lightSourceBufferMemory);

	device->copyBuffer(stagingBuffer.getBuffer(), lightSourceBuffer, bufferSize);
}

void System_Light_Multipass::createNodeBuffer()
{
	VkDeviceSize bufferSize = sizeof(BVH_node_gpu) * n_nodes;

	MyBufferObject stagingBuffer(device, sizeof(BVH_node_gpu), n_nodes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	stagingBuffer.writeToBuffer((void*)my_bvh.nodes.data());

	device->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nodeBuffer,
		nodeBufferMemory);

	device->copyBuffer(stagingBuffer.getBuffer(), nodeBuffer, bufferSize);
}

void System_Light_Multipass::writeRaytraceInfoBuffer(int lightmap_n, int f_j)
{
	RayTraceInfo info{};

	info.light_pos = light_pos;
	info.n_vertices = n_vertices;
	info.n_triangles = n_indices / 3;
	info.n_rays = N_RAYS;
	info.n_nodes = n_nodes;

	//info.face_index_offset = indices_soa.offset[lightmap_n];
	info.face_triangle_offset = indices_soa.offset[lightmap_n] / 3 + lightmaps_info[lightmap_n].first_triangle[f_j];

	//info.face_n_indices = indices_soa.len[lightmap_n];
	info.face_n_triangles = lightmaps_info[lightmap_n].n_triangles[f_j];
	
	
	info.face_vertex_offset = vertices_soa.offset[lightmap_n]; //unused atm

	//std::cout << "Vertex offset " << info.face_vertex_offset << "\n";
	//std::cout << "t0 offset " << info.face_triangle_offset << "\n";
	//std::cout << "n triangles " << info.face_n_triangles <<  "\n";
	
	info.lightmap_width = lightmaps_info[lightmap_n].size;
	info.lightmap_height = lightmaps_info[lightmap_n].size;



	//cout<<"width "<< lightmaps_info[lightmap_n].size <<"\n";
	//cout<<"height " << lightmaps_info[lightmap_n].size << "\n";

	raytraceBufferObject->writeToBuffer((void*)&info);
}

void System_Light_Multipass::writeRaytraceInfoBuffer2(int lightmap_n)
{
	RayTraceInfo info{};

	info.light_pos = light_pos;
	info.n_vertices = n_vertices;
	info.n_triangles = n_indices / 3;
	info.n_rays = N_RAYS;
	info.n_nodes = n_nodes;
	info.n_lights = n_lights;

	//info.face_index_offset = indices_soa.offset[lightmap_n];
	//info.face_triangle_offset = indices_soa.offset[lightmap_n] / 3 + lightmaps_info[lightmap_n].first_triangle[f_j];

	//info.face_n_indices = indices_soa.len[lightmap_n];
	//info.face_n_triangles = lightmaps_info[lightmap_n].n_triangles[f_j];


	info.face_vertex_offset = vertices_soa.offset[lightmap_n]; //unused atm

	//std::cout << "Vertex offset " << info.face_vertex_offset << "\n";
	//std::cout << "t0 offset " << info.face_triangle_offset << "\n";
	//std::cout << "n triangles " << info.face_n_triangles << "\n";

	info.lightmap_width = lightmaps_info[lightmap_n].size;
	info.lightmap_height = lightmaps_info[lightmap_n].size;

	//cout<<"width "<< lightmaps_info[lightmap_n].size <<"\n";
	//cout<<"height " << lightmaps_info[lightmap_n].size << "\n";

	info.faces_offset = faces_offset[lightmap_n];

	raytraceBufferObject->writeToBuffer((void*)&info);

	std::cout << "offset = " << info.faces_offset << "\n";


	////////////////////////////////////

	u16 nFaces = lightmaps_info[lightmap_n].faces.size();

	nTrianglesFace.resize(nFaces);
	firstTriangleFace.resize(nFaces);

	for (int i = 0; i < nFaces; i++)
	{
		nTrianglesFace[i].x = lightmaps_info[lightmap_n].n_triangles[i];
		firstTriangleFace[i].x = indices_soa.offset[lightmap_n] / 3 + lightmaps_info[lightmap_n].first_triangle[i];
	}

	{
		VkDeviceSize bufferSize = sizeof(aligned_uint) * nFaces;

		MyBufferObject stagingBuffer(device, sizeof(aligned_uint), nFaces, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

		stagingBuffer.writeToBuffer((void*)nTrianglesFace.data());

		device->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nTrianglesBuffer,
			nTrianglesBufferMemory);

		device->copyBuffer(stagingBuffer.getBuffer(), nTrianglesBuffer, bufferSize);
	}

	{
		VkDeviceSize bufferSize = sizeof(aligned_uint) * nFaces;

		MyBufferObject stagingBuffer(device, sizeof(aligned_uint), nFaces, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

		stagingBuffer.writeToBuffer((void*)firstTriangleFace.data());

		device->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, firstTriangleBuffer,
			firstTriangleBufferMemory);

		device->copyBuffer(stagingBuffer.getBuffer(), firstTriangleBuffer, bufferSize);
	}
}

void System_Light_Multipass::buildLightmaps_async()
{
	descriptorSets.resize(1);

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

	VkDescriptorBufferInfo lightBufferInfo{};
	lightBufferInfo.buffer = lightSourceBuffer;
	lightBufferInfo.offset = 0;
	lightBufferInfo.range = sizeof(LightSource_struct) * n_lights;

	VkDescriptorBufferInfo quadBufferInfo{};
	quadBufferInfo.buffer = quadsBuffer;
	quadBufferInfo.offset = 0;
	quadBufferInfo.range = sizeof(aligned_vec3) * total_n_faces * 4;

	VkDescriptorBufferInfo hitResultsInfo{};
	hitResultsInfo.buffer = hitResultsBufferObject->getBuffer();
	hitResultsInfo.offset = 0;
	hitResultsInfo.range = sizeof(aligned_vec3) * N_RAYS * 2;

	for (int i = 0; i < lightmaps_info.size(); i++)
	//int i = 1;
	{
		int nFaces = lightmaps_info[i].faces.size();

		writeRaytraceInfoBuffer2(i);

		VkDescriptorBufferInfo raytraceInfo{};
		raytraceInfo.buffer = raytraceBufferObject->getBuffer();
		raytraceInfo.offset = 0;
		raytraceInfo.range = sizeof(RayTraceInfo);

		VkDescriptorBufferInfo firstTriangleBufferInfo{};
		firstTriangleBufferInfo.buffer = firstTriangleBuffer;
		firstTriangleBufferInfo.offset = 0;
		firstTriangleBufferInfo.range = sizeof(aligned_uint) * nFaces;

		VkDescriptorBufferInfo nTrianglesBufferInfo{};
		nTrianglesBufferInfo.buffer = nTrianglesBuffer;
		nTrianglesBufferInfo.offset = 0;
		nTrianglesBufferInfo.range = sizeof(aligned_uint) * nFaces;

		VkDescriptorImageInfo workingImageInfo{};
		workingImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		workingImageInfo.imageView = workingImageViews[i];

		VkDescriptorImageInfo imageStorageInfo{};
		imageStorageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageStorageInfo.imageView = lightmapImageViews[i];

		MyDescriptorWriter writer(*descriptorSetLayout, *descriptorPool);

		writer.writeBuffer(0, indexBufferInfo);
		writer.writeBuffer(1, vertexBufferInfo);
		writer.writeBuffer(2, uvBufferInfo);
		writer.writeBuffer(3, nodeBufferInfo);
		writer.writeBuffer(4, lightBufferInfo);
		writer.writeBuffer(5, quadBufferInfo);
		writer.writeBuffer(6, hitResultsInfo);
		writer.writeBuffer(7, raytraceInfo);
		writer.writeBuffer(8, firstTriangleBufferInfo);
		writer.writeBuffer(9, nTrianglesBufferInfo);
		writer.writeImage(10, workingImageInfo);
		writer.writeImage(11, imageStorageInfo);

		writer.build(descriptorSets[0]);

		//=====================================
		// Begin Command Buffer
		//
		VkCommandBufferAllocateInfo allocInfo{};

		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = device->getCommandPool();
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device->getDevice(), &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		//=====================================
		// Bind
		//

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
			pipeline->getPipeline());

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1,
			&descriptorSets[0], 0, 0);

		//=====================================
		// Dispatch and end Command Buffer
		//
		uint32_t n_WorkGroups_x = nFaces;
		//uint32_t n_WorkGroups_x = 1;
		std::cout << n_WorkGroups_x << " workgroups\n";

		std::cout << "execute...\n";
		vkCmdDispatch(commandBuffer, n_WorkGroups_x, 1, 1);

		vkEndCommandBuffer(commandBuffer);

		//=====================================
		// Queue Submit, wait for Queue Idle, free Command Buffers and Descriptor sets
		//
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);

		vkQueueWaitIdle(device->getGraphicsQueue());


		vkFreeCommandBuffers(device->getDevice(), device->getCommandPool(), 1, &commandBuffer);

		descriptorPool->freeDescriptorsSets(descriptorSets);

		//=====================================
		// Wait for Device Idle, Free Buffers for reuse
		//
		vkDeviceWaitIdle(device->getDevice());

		vkDestroyBuffer(device->getDevice(), firstTriangleBuffer, nullptr);
		vkFreeMemory(device->getDevice(), firstTriangleBufferMemory, nullptr);

		vkDestroyBuffer(device->getDevice(), nTrianglesBuffer, nullptr);
		vkFreeMemory(device->getDevice(), nTrianglesBufferMemory, nullptr);

		//=====================================
		// Print output
		//
		/*
		MyBufferObject stagingBuffer(device, sizeof(aligned_vec3), N_RAYS * 2, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

		device->copyBuffer(hitResultsBufferObject->getBuffer(), stagingBuffer.getBuffer(), sizeof(aligned_vec3)* N_RAYS * 2);

		stagingBuffer.readFromBuffer((void*)hit_results);

		for (int i = 0; i < 32; i += 1)
		{
			//line3df line0 = line3df(vector3df(hit_results[i].V), vector3df(hit_results[i].V + hit_results[i + 1].V));
			//line3df line1 = line3df(vector3df(hit_results[i].V), vector3df(hit_results[i].V + hit_results[i + 2].V));
			//line3df line2 = line3df(vector3df(hit_results[i].V + hit_results[i + 1].V), vector3df(hit_results[i].V + hit_results[i + 2].V));
			//line3df line0 = line3df(vector3df(hit_results[i].V), vector3df(hit_results[i].V + N));
			//line3df line1 = line3df(vector3df(hit_results[i].V), vector3df(hit_results[i + 2].V));
			//line3df line2 = line3df(vector3df(hit_results[i + 1].V), vector3df(hit_results[i + 2].V));

			//line3df line0 = line3df(vector3df(hit_results[i].V), vector3df(hit_results[256 + i].V));
			//m_graph.lines.push_back(line0);

			//std::cout << hit_results[i].V.X << "," << hit_results[i].V.Y << "," << hit_results[i].V.Z << "\n";
			//std::cout << hit_results[i].V.X << "," << hit_results[i].V.Y << "\n ";

			//m_graph.lines.push_back(line1);
			//m_graph.lines.push_back(line2);
		}
		std::cout << "\n";
		*/
	}
}

void System_Light_Multipass::buildLightmap(int lightmap_n, int f_j)
{
	writeRaytraceInfoBuffer(lightmap_n, f_j);
	createDescriptorSets(lightmap_n);

	VkCommandBuffer commandBuffer = device->beginSingleTimeCommands();

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipeline->getPipeline());

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1,
		&descriptorSets[0], 0, 0);

	//uint32_t n_WorkGroups_x = indices_soa.len[lightmap_n] / 3;
	uint32_t n_WorkGroups_x = 1;
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

	for (int i = 0; i < 32; i++)
	{
		//std::cout << hit_results[i].V.X << ", ";
		std::cout << hit_results[i].V.X << "," << hit_results[i].V.Y << "  ";
		//std::cout << hit_results[i].V.X << "," << hit_results[i].V.Y << "," << hit_results[i].V.Z << "\n  ";
		//std::cout << hit_results[256+i].V.X << "," << hit_results[256+i].V.Y << "," << hit_results[256+ i].V.Z << "\n";
	}

	for (int i = 0; i < 256; i += 1)
	{
		//line3df line0 = line3df(vector3df(hit_results[i].V), vector3df(hit_results[i].V + hit_results[i + 1].V));
		//line3df line1 = line3df(vector3df(hit_results[i].V), vector3df(hit_results[i].V + hit_results[i + 2].V));
		//line3df line2 = line3df(vector3df(hit_results[i].V + hit_results[i + 1].V), vector3df(hit_results[i].V + hit_results[i + 2].V));
		//line3df line0 = line3df(vector3df(hit_results[i].V), vector3df(hit_results[i].V + N));
		//line3df line1 = line3df(vector3df(hit_results[i].V), vector3df(hit_results[i + 2].V));
		//line3df line2 = line3df(vector3df(hit_results[i + 1].V), vector3df(hit_results[i + 2].V));

		//line3df line0 = line3df(vector3df(hit_results[i].V), vector3df(hit_results[256 + i].V));
		//m_graph.lines.push_back(line0);

		//m_graph.lines.push_back(line1);
		//m_graph.lines.push_back(line2);
	}
	std::cout << "\n";
}

void System_Light_Multipass::writeDrawLines(LineHolder& graph)
{
	graph.lines = m_graph.lines;
}

void System_Light_Multipass::executeComputeShader() 
{
	/*
	for (int i = 0; i < lightmaps_info.size(); i++)
	{
		//if(i==0)
		for (int j = 0; j < lightmaps_info[i].faces.size(); j++)
		{
			std::cout << i << "," << j << ":\n";
			//if(j==0)
				buildLightmap(i,j);
		}
	}*/

	buildLightmaps_async();
	/*
	vkDeviceWaitIdle(device->getDevice());

	MyBufferObject stagingBuffer(device, sizeof(aligned_vec3), N_RAYS * 2, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	device->copyBuffer(hitResultsBufferObject->getBuffer(), stagingBuffer.getBuffer(), sizeof(aligned_vec3) * N_RAYS * 2);

	stagingBuffer.readFromBuffer((void*)hit_results);

	for (int i = 0; i < 32; i += 1)
	{
		//line3df line0 = line3df(vector3df(hit_results[i].V), vector3df(hit_results[i].V + hit_results[i + 1].V));
		//line3df line1 = line3df(vector3df(hit_results[i].V), vector3df(hit_results[i].V + hit_results[i + 2].V));
		//line3df line2 = line3df(vector3df(hit_results[i].V + hit_results[i + 1].V), vector3df(hit_results[i].V + hit_results[i + 2].V));
		//line3df line0 = line3df(vector3df(hit_results[i].V), vector3df(hit_results[i].V + N));
		//line3df line1 = line3df(vector3df(hit_results[i].V), vector3df(hit_results[i + 2].V));
		//line3df line2 = line3df(vector3df(hit_results[i + 1].V), vector3df(hit_results[i + 2].V));

		//line3df line0 = line3df(vector3df(hit_results[i].V), vector3df(hit_results[256 + i].V));
		//m_graph.lines.push_back(line0);

		//std::cout << hit_results[i].V.X << "," << hit_results[i].V.Y << "," << hit_results[i].V.Z << "\n";
		std::cout << hit_results[i].V.X << " ";

		//m_graph.lines.push_back(line1);
		//m_graph.lines.push_back(line2);
	}
	std::cout << "\n";*/
	
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

void System_Light_Multipass::addDrawLinesPrims(int node_i, LineHolder& graph) const
{
	my_bvh.addDrawLinesPrims(vertices_soa.data0.data(), indices_soa.data.data(), node_i, graph);
}

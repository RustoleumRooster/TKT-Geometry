#include <sstream>

#include "vkCompute_TestMC.h"
#include "vkDescriptors.h"
#include "vkComputePipeline.h"
#include "vkBufferObject.h"

#include "vkSystem4.h"
#include "utils.h"
#include "BufferManager.h"
#include "geometry_scene.h"

void System_Test_MC::cleanup() {
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

bool System_Test_MC::verify_inputs(geometry_scene* geo_scene)
{
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

	count = 0;
	for (Reflected_SceneNode* node : scene_nodes)
	{
		if (strcmp(node->GetDynamicReflection()->name, "Reflected_PointNode") == 0)
		{
			Reflected_PointNode* pnode = dynamic_cast<Reflected_PointNode*>(node);

			if (pnode)
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

	return true;
}

typedef CMeshBuffer<video::S3DVertex2TCoords> mesh_buffer_type;

void System_Test_MC::loadLights(geometry_scene* geo_scene)
{
	std::vector<Reflected_SceneNode*> scene_nodes = geo_scene->getSceneNodes();

	lightSources.clear();

	for (Reflected_SceneNode* node : scene_nodes)
	{
		if (strcmp(node->GetDynamicReflection()->name, "Reflected_LightSceneNode") == 0)
		{
			Reflected_LightSceneNode* lsnode = dynamic_cast<Reflected_LightSceneNode*>(node);

			if (lsnode)
			{
				
				LightSource_struct ls{ lsnode->getPosition(),lsnode->light_radius };
				lightSources.push_back(ls);

				light_pos = ls.pos;
			}
		}
	}
	n_lights = lightSources.size();

	for (Reflected_SceneNode* node : scene_nodes)
	{
		if (strcmp(node->GetDynamicReflection()->name, "Reflected_PointNode") == 0)
		{
			Reflected_PointNode* lsnode = dynamic_cast<Reflected_PointNode*>(node);

			if (lsnode)
			{
				eye_pos = lsnode->getPosition();
				break;
			}
		}
	}
}

void System_Test_MC::loadModel(MeshNode_Interface_Final* meshnode)
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
		master_triangle_list[i].set((u16)(indices_soa.data[i * 3].x), (u16)(indices_soa.data[(i * 3) + 1].x), (u16)(indices_soa.data[(i * 3) + 2].x));
	}

	my_bvh.build(vertices_soa.data0.data(), master_triangle_list.data(), master_triangle_list.size(), NULL);

	n_nodes = my_bvh.node_count;

	for (int i = 0; i < n_nodes; i++)
	{
		if (my_bvh.nodes[i].left_node == 0xFFFF && my_bvh.nodes[i].right_node == 0xFFFF)
		{
			if (my_bvh.nodes[i].n_prims > 2)
				std::cout << "WARNING: node " << i << " has " << my_bvh.nodes[i].n_prims << " triangles, max 2\n";

			for (int j = 0; j < std::min(4u, my_bvh.nodes[i].n_prims); j++)
			{
				my_bvh.nodes[i].packing[j] = static_cast<f32> (my_bvh.indices[my_bvh.nodes[i].first_prim + j]);
			}
		}
		else
			my_bvh.nodes[i].n_prims = 0;
	}

}

void System_Test_MC::createDescriptorSets(int lightmap_n)
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

	VkDescriptorBufferInfo nodeBufferInfo{};
	nodeBufferInfo.buffer = nodeBuffer;
	nodeBufferInfo.offset = 0;
	nodeBufferInfo.range = sizeof(BVH_node_gpu) * n_nodes;

	VkDescriptorBufferInfo lightBufferInfo{};
	lightBufferInfo.buffer = lightSourceBuffer;
	lightBufferInfo.offset = 0;
	lightBufferInfo.range = sizeof(LightSource_struct) * n_lights;

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
	writer.writeBuffer(4, nodeBufferInfo);
	writer.writeBuffer(5, lightBufferInfo);
	writer.writeBuffer(6, hitResultsInfo);
	writer.writeImage(7, imageStorageInfo);
	writer.build(descriptorSets[0]);
}


void System_Test_MC::createDescriptorSetLayout()
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

	VkDescriptorSetLayoutBinding lightBufferBinding{};
	lightBufferBinding.binding = 5;
	lightBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	lightBufferBinding.descriptorCount = 1;
	lightBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	lightBufferBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding hitResultsBinding{};
	hitResultsBinding.binding = 6;
	hitResultsBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	hitResultsBinding.descriptorCount = 1;
	hitResultsBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	hitResultsBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding imageStorageBinding{};
	imageStorageBinding.binding = 7;
	imageStorageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	imageStorageBinding.descriptorCount = 1;
	imageStorageBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	imageStorageBinding.pImmutableSamplers = nullptr; // Optional

	std::vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.resize(8);
	bindings = { raytraceBufferBinding, indexBufferBinding, vertexBufferBinding, uvBufferBinding, nodeBufferBinding, lightBufferBinding, hitResultsBinding, imageStorageBinding };

	descriptorSetLayout = new MyDescriptorSetLayout(device, bindings);
}

void System_Test_MC::createComputePipeline()
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

	pipeline = new ComputePipeline(device, "shaders/testmc.spv", pipelineLayout);
}

void System_Test_MC::createLightmapImages()
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

void System_Test_MC::createRaytraceInfoBuffer()
{
	raytraceBufferObject = new MyBufferObject(device, sizeof(RayTraceInfo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 16);
}

void System_Test_MC::createHitResultsBuffer() {

	uint16_t bSize = 256 * 2;
	VkDeviceSize bufferSize = sizeof(aligned_vec3) * bSize;

	hit_results = new aligned_vec3[bSize];
	for (int i = 0; i < bSize; i++) {
		hit_results[i].V = vector3df{ 0,0,0 };
	}

	hitResultsBufferObject = new MyBufferObject(device, sizeof(aligned_vec3), bSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 16);
}

void System_Test_MC::createIndexBuffer() {

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

void System_Test_MC::createLightsBuffer() {

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


void System_Test_MC::createVertexBuffer() {

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

void System_Test_MC::createUVBuffer()
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

void System_Test_MC::createNodeBuffer()
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

void System_Test_MC::writeRaytraceInfoBuffer(int lightmap_n)
{
	RayTraceInfo info{};
	/*
	vector3df center = vector3df(0, 0, 0);
	for (int i = 0; i < n_vertices; i++)
	{
		center += vertices_soa.data0[i].V;
	}
	center /= n_vertices;

	if (lightSources.size() > 0)
	{
		info.eye_pos = lightSources[0].pos;
		info.lightradius = lightSources[0].radius;
	}
	else
	{
		info.eye_pos = center;
		info.lightradius = 500;
	}
	*/

	info.eye_pos = eye_pos;
	info.light_pos = light_pos;

	info.n_vertices = n_vertices;
	info.n_triangles = n_indices / 3;
	info.n_rays = N_RAYS;
	info.n_nodes = n_nodes;
	info.n_lights = n_lights;

	info.face_index_offset = indices_soa.offset[lightmap_n];
	info.face_n_indices = indices_soa.len[lightmap_n];
	info.face_vertex_offset = vertices_soa.offset[lightmap_n];

	std::cout << "Vertex offset " << info.face_vertex_offset << "\n";
	std::cout << "index offset " << info.face_index_offset << "\n";
	std::cout << "n indices " << info.face_n_indices << "\n";

	info.lightmap_width = lightmaps_info[lightmap_n].size;
	info.lightmap_height = lightmaps_info[lightmap_n].size;

	raytraceBufferObject->writeToBuffer((void*)&info);
}

bool System_Test_MC::triangle_sees_light(uint32_t triangle_i, vector3df pos, uint32_t radius)
{
	vector3df v0, v1, v2;

	v0 = vertices_soa.data0[indices_soa.data[(triangle_i) + 0].x].V;
	v1 = vertices_soa.data0[indices_soa.data[(triangle_i) + 1].x].V;
	v2 = vertices_soa.data0[indices_soa.data[(triangle_i) + 2].x].V;

	vector3df N = vector3df(v1 - v0).crossProduct(vector3df(v2 - v1));
	N.normalize();

	plane3df plane = plane3df(v0, v1, v2);
	plane.Normal.normalize();

	if (plane.getDistanceTo(pos) > radius)
		return false;

	if (plane.classifyPointRelation(pos) != ISREL3D_FRONT)
		return false;

	if (vector3df(pos - v0).getLength() <= radius ||
		vector3df(pos - v1).getLength() <= radius ||
		vector3df(pos - v2).getLength() <= radius)
		return true;

	line3df edge0, edge1, edge2;

	edge0 = line3df(v0, v1);
	edge1 = line3df(v0, v2);
	edge2 = line3df(v2, v1);

	f64 dist;

	if (edge0.getIntersectionWithSphere(pos, radius, dist) ||
		edge1.getIntersectionWithSphere(pos, radius, dist) ||
		edge2.getIntersectionWithSphere(pos, radius, dist))
		return true;

	vector3df ray = plane.Normal;
	ray.normalize();
	
	if (trace_triangle_hit(triangle_i, pos, -ray) < radius)
		return true;

	return false;
}

float System_Test_MC::trace_triangle_hit(uint32_t triangle_i, vector3df eye, vector3df ray)
{
	vector3df edge1, edge2, rayVecXe2, s, sXe1;
	float det, invDet, u, v;

	u32 idx = triangle_i;

	edge1 = vertices_soa.data0[indices_soa.data[(idx) + 1].x].V - vertices_soa.data0[indices_soa.data[idx].x].V;
	edge2 = vertices_soa.data0[indices_soa.data[(idx) + 2].x].V - vertices_soa.data0[indices_soa.data[idx].x].V;
	rayVecXe2 = ray.crossProduct(edge2);
	det = edge1.dotProduct(rayVecXe2);

	if (det > -0.00001 && det < 0.00001)
		return 0xFFFF;

	invDet = 1.0 / det;
	s = eye - vertices_soa.data0[indices_soa.data[idx].x].V;
	u = invDet * s.dotProduct(rayVecXe2);

	if (u < 0.0 || u > 1.0)
		return 0xFFFF;

	sXe1 = s.crossProduct(edge1);
	v = invDet * ray.dotProduct(sXe1);

	if (v < 0.0 || u + v > 1.0)
		return 0xFFFF;

	float t = invDet * edge2.dotProduct(sXe1);

	if (t > 0)
	{
		return t;
	}

	return 0xFFFF;
}

void System_Test_MC::buildLightmap(int lightmap_n)
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
	//vkCmdDispatch(commandBuffer, n_WorkGroups_x, 1, 1);
	vkCmdDispatch(commandBuffer, 1, 1, 1);


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


	for (int i = 0; i < 128; i += 1)
	{
		//line3df line0 = line3df(vector3df(hit_results[i].V), vector3df(hit_results[i].V + hit_results[i + 1].V));
		//line3df line1 = line3df(vector3df(hit_results[i].V), vector3df(hit_results[i].V + hit_results[i + 2].V));
		//line3df line2 = line3df(vector3df(hit_results[i].V + hit_results[i + 1].V), vector3df(hit_results[i].V + hit_results[i + 2].V));
		//line3df line0 = line3df(vector3df(hit_results[i].V), vector3df(hit_results[i].V + N));
		//line3df line1 = line3df(vector3df(hit_results[i].V), vector3df(hit_results[i + 2].V));
		//line3df line2 = line3df(vector3df(hit_results[i + 1].V), vector3df(hit_results[i + 2].V));

		line3df line0 = line3df(vector3df(hit_results[i].V), vector3df(hit_results[128 + i].V));
		m_graph.lines.push_back(line0);
		m_graph.points.push_back(vector3df(hit_results[128 + i].V));

		//line0 = line3df(vector3df(hit_results[128 + i].V), vector3df(hit_results[256 + i].V));
		//m_graph.lines.push_back(line0);

		//m_graph.lines.push_back(line1);
		//m_graph.lines.push_back(line2);
	}
}



void System_Test_MC::executeComputeShader()
{
	m_graph.lines.clear();

	for (int i = 0; i < lightmaps_info.size(); i++)
	{
		if(i==0)
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

void System_Test_MC::writeDrawLines(LineHolder& graph)
{
	graph.lines = m_graph.lines;
	graph.points = m_graph.points;
}

void System_Test_MC::addDrawLinesPrims(int node_i, LineHolder& graph) const
{
	my_bvh.addDrawLinesPrims(vertices_soa.data0.data(), indices_soa.data.data(), node_i, graph);
}

#include <sstream>

#include "vkCompute_SoftLight.h"
#include "vkDescriptors.h"
#include "vkComputePipeline.h"
#include "vkBufferObject.h"

#include "vkSystem4.h"
#include "utils.h"
#include "BufferManager.h"
#include "geometry_scene.h"
#include "custom_nodes.h"

void System_Soft_Light::cleanup() {
	descriptorSetLayout->cleanup();

	vkDestroyBuffer(device->getDevice(), indexBuffer, nullptr);
	vkFreeMemory(device->getDevice(), indexBufferMemory, nullptr);

	vkDestroyBuffer(device->getDevice(), vertexBuffer, nullptr);
	vkFreeMemory(device->getDevice(), vertexBufferMemory, nullptr);

	vkDestroyBuffer(device->getDevice(), uvBuffer, nullptr);
	vkFreeMemory(device->getDevice(), uvBufferMemory, nullptr);

	vkDestroyBuffer(device->getDevice(), nodeBuffer, nullptr);
	vkFreeMemory(device->getDevice(), nodeBufferMemory, nullptr);

	vkDestroyBuffer(device->getDevice(), edgeBuffer, nullptr);
	vkFreeMemory(device->getDevice(), edgeBufferMemory, nullptr);

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

typedef CMeshBuffer<video::S3DVertex2TCoords> mesh_buffer_type;


bool System_Soft_Light::verify_inputs(geometry_scene* geo_scene)
{
	//std::vector<Reflected_SceneNode*> scene_nodes = geo_scene->getSceneNodes();

	int count = 0;
	//for (Reflected_SceneNode* node : scene_nodes)
	for (ISceneNode* it : geo_scene->EditorNodes()->getChildren())
	{
		Reflected_SceneNode* node = (Reflected_SceneNode*)it;
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

	return true;
}

void System_Soft_Light::loadLights(geometry_scene* geo_scene)
{
	//std::vector<Reflected_SceneNode*> scene_nodes = geo_scene->getSceneNodes();

	//lightSources.clear();

	//for (Reflected_SceneNode* node : scene_nodes)
	for (ISceneNode* it : geo_scene->EditorNodes()->getChildren())
	{
		Reflected_SceneNode* node = (Reflected_SceneNode*)it;
		if (strcmp(node->GetDynamicReflection()->name, "Reflected_LightSceneNode") == 0)
		{
			Reflected_LightSceneNode* lsnode = dynamic_cast<Reflected_LightSceneNode*>(node);

			if (lsnode)
			{

				//LightSource_struct ls{ lsnode->getPosition(),lsnode->light_radius };
				//lightSources.push_back(ls);

				light_pos = lsnode->getPosition();
				break;
			}
		}
	}
}

void System_Soft_Light::loadModel(MeshNode_Interface_Final* meshnode)
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

	//==========================================================
	// Store references to the adjacent triangles for each edge, in order to properly calculate the lighting at the edges
	//

	triangle_edges.resize(master_triangle_list.size() * 3);
	
	for (int i = 0; i < master_triangle_list.size(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			vector<u16> hits;

			triangle_edge edge;
			u16 v_0 = master_triangle_list[i].v_i[j];
			edge.v0 = vertices_soa.data0.operator[](v_0).V;

			u16 v_1 = master_triangle_list[i].v_i[(j+1)%3];
			edge.v1 = vertices_soa.data0.operator[](v_1).V;

			vector3df N = master_triangle_list[i].normal(vertices_soa.data0);

			vector3df edgeTan = vector3df(edge.v1 - edge.v0).crossProduct(N);

			my_bvh.intersect(edge, hits);
			bool ok = false;

			for (u16 k : hits)
			{
				if (k != i && master_triangle_list[k].find_edge(v_1,v_0, vertices_soa.data0))
				{
					//Only consider the adjacent triangles which create a concave angle
					if (master_triangle_list[k].normal(vertices_soa.data0).crossProduct(N).dotProduct(edgeTan.crossProduct(N)) < -0.001)
					{
						triangle_edges[i * 3 + j].x = k;
					}
					else
					{
						triangle_edges[i * 3 + j].x = 0xFFFF;
					}
					ok = true;
					break;
				}
				else if (k != i)
				{
					//std::cout << " ???";
				}
			}

			if (!ok)
			{
				triangle_edges[i].x = 0xFFFF;
				std::cout << "warning: triangle missing adjacent \n";
			}
		}
	}
}

void System_Soft_Light::createDescriptorSets(int lightmap_n) 
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

	VkDescriptorBufferInfo edgeBufferInfo{};
	edgeBufferInfo.buffer = edgeBuffer;
	edgeBufferInfo.offset = 0;
	edgeBufferInfo.range = sizeof(aligned_uint) * n_indices;
	
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
	writer.writeBuffer(5, edgeBufferInfo);
	writer.writeBuffer(6, hitResultsInfo);
	writer.writeImage(7, imageStorageInfo);
	writer.build(descriptorSets[0]);
}


void System_Soft_Light::createDescriptorSetLayout()
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
	bindings = { raytraceBufferBinding, indexBufferBinding, vertexBufferBinding, uvBufferBinding, nodeBufferBinding, edgeBufferBinding, hitResultsBinding, imageStorageBinding};

	descriptorSetLayout = new MyDescriptorSetLayout(device, bindings);
}

void System_Soft_Light::createComputePipeline()
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

	pipeline = new ComputePipeline(device, "shaders/softlight.spv", pipelineLayout);
}

void System_Soft_Light::createLightmapImages()
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

void System_Soft_Light::createRaytraceInfoBuffer()
{
	raytraceBufferObject = new MyBufferObject(device, sizeof(RayTraceInfo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 16);
}

void System_Soft_Light::createHitResultsBuffer() {

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

void System_Soft_Light::createIndexBuffer() {

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


void System_Soft_Light::createVertexBuffer() {

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

void System_Soft_Light::createUVBuffer()
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

void System_Soft_Light::createEdgeBuffer()
{
	VkDeviceSize bufferSize = sizeof(aligned_uint) * n_indices;

	MyBufferObject stagingBuffer(device, sizeof(aligned_uint), n_indices, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	stagingBuffer.writeToBuffer((void*)triangle_edges.data());

	device->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, edgeBuffer,
		edgeBufferMemory);

	device->copyBuffer(stagingBuffer.getBuffer(), edgeBuffer, bufferSize);
}

void System_Soft_Light::createNodeBuffer()
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

void System_Soft_Light::writeRaytraceInfoBuffer(int lightmap_n, int f_j)
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
	std::cout << "t0 offset " << info.face_triangle_offset << "\n";
	std::cout << "n triangles " << info.face_n_triangles <<  "\n";
	
	info.lightmap_width = lightmaps_info[lightmap_n].size;
	info.lightmap_height = lightmaps_info[lightmap_n].size;

	//cout<<"width "<< lightmaps_info[lightmap_n].size <<"\n";
	//cout<<"height " << lightmaps_info[lightmap_n].size << "\n";

	raytraceBufferObject->writeToBuffer((void*)&info);
}

void System_Soft_Light::buildLightmap(int lightmap_n, int f_j)
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

	for (int i = 0; i < 256; i++)
	{
		//std::cout << hit_results[i].V.X << ", ";
		//std::cout << hit_results[i].V.X << "," << hit_results[i].V.Y << " \n  ";
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
	//std::cout << "\n";
}

void System_Soft_Light::writeDrawLines(LineHolder& graph)
{
	graph.lines = m_graph.lines;
}

void System_Soft_Light::executeComputeShader() 
{
	for (int i = 0; i < lightmaps_info.size(); i++)
	{
		//if(i==0)
		for (int j = 0; j < lightmaps_info[i].faces.size(); j++)
		{
			std::cout << i << "," << j << ":\n";
			//if(j==0)
				buildLightmap(i,j);
		}
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

void System_Soft_Light::addDrawLinesPrims(int node_i, LineHolder& graph) const
{
	my_bvh.addDrawLinesPrims(vertices_soa.data0.data(), indices_soa.data.data(), node_i, graph);
}

#pragma once

#include <vulkan/vulkan.h>
#include <irrlicht.h>

#include <vector>
#include <stdexcept>
#include "vkDevice.h"
#include "vkModel.h"
#include "Reflection.h"
#include "vk_BVH.h"

class MyDescriptorSetLayout;
class SwapChain;
class MyTextures;
class MyDescriptorPool;
class ComputePipeline;
class MyBufferObject;

class MeshNode_Interface_Final;




class System_Amb_Occlusion {

	struct RayTraceInfo {
		uint32_t n_rays;

		//scene info
		uint32_t n_triangles;
		uint32_t n_vertices;
		uint32_t n_nodes;

		//face info
		uint32_t face_vertex_offset;
		uint32_t face_index_offset;
		uint32_t face_n_indices;

		//lightmap info
		uint32_t lightmap_width;
		uint32_t lightmap_height;
	};

public:
	System_Amb_Occlusion(MyDevice* device,
		const std::vector<VkBuffer>& uniformBuffers) :

		device{ device },
		uniformBuffers{ uniformBuffers } {
	}
	uint32_t num_lightmaps_used() { return lightmapImages.size(); }
	void cleanup();

	void loadModel(MeshNode_Interface_Final* meshnode);

	void initialize_step2(MyDescriptorPool* descriptorPool)
	{
		setDescriptorPool(descriptorPool);
		createDescriptorSetLayout();
		createComputePipeline();
		createVertexBuffer();
		createIndexBuffer();
		createRaytraceInfoBuffer();
		createHitResultsBuffer();
		createUVBuffer();
		createNodeBuffer();
		createEdgeBuffer();
	}
	
	void executeComputeShader(std::string filename_base);
	void createLightmapImages();
	VkImageView getImageView(int n) { return lightmapImageViews[n]; }

	void writeDrawLines(LineHolder& graph);

private:

	void addDrawLinesPrims(int node_i, LineHolder& graph) const;

	void setDescriptorPool(MyDescriptorPool* descriptorPool) {
		this->descriptorPool = descriptorPool;
	}

	void createDescriptorSetLayout();
	void createIndexBuffer();
	void createVertexBuffer();
	void createUVBuffer();
	void createEdgeBuffer();
	void createRaytraceInfoBuffer();
	void createHitResultsBuffer();
	void createComputePipeline();
	
	void createNodeBuffer();
	void createDescriptorSets(int face_n);
	void writeRaytraceInfoBuffer(int face_n);
	void buildLightmap(int face_n);

	VkPipelineLayout pipelineLayout;
	std::vector<VkDescriptorSet> descriptorSets;
	std::vector<VkBuffer> uniformBuffers;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VkBuffer normalBuffer;
	VkDeviceMemory normalBufferMemory;

	VkBuffer uvBuffer;
	VkDeviceMemory uvBufferMemory;

	VkBuffer faceIndexBuffer;
	VkDeviceMemory faceIndexBufferMemory;

	VkBuffer nodeBuffer;
	VkDeviceMemory nodeBufferMemory;

	VkBuffer edgeBuffer;
	VkDeviceMemory edgeBufferMemory;

	MyBufferObject* raytraceBufferObject = NULL;
	MyBufferObject* hitResultsBufferObject = NULL;

	MyDescriptorSetLayout* descriptorSetLayout = NULL;
	ComputePipeline* pipeline = NULL;
	MyDevice* device = NULL;
	MyDescriptorPool* descriptorPool = NULL;

	uint16_t N_RAYS = 256;

	//scene
	aligned_vec3* hit_results = NULL;

	soa_struct_2<aligned_vec3, aligned_vec3> vertices_soa;
	soa_struct<aligned_uint> indices_soa;

	std::vector<aligned_uint> triangle_edges;

	uint16_t n_indices = 0;
	uint16_t n_vertices = 0;

	//faces
	uint32_t n_faces = 0;

	std::vector<LightMaps_Info_Struct> lightmaps_info;

	std::vector<VkImage> lightmapImages;
	std::vector<VkDeviceMemory> lightmapsMemory;
	std::vector<VkImageView> lightmapImageViews;

	std::vector<triangle_b> master_triangle_list;
	BVH_structure_triangles my_bvh;

	uint32_t n_nodes;

	LineHolder m_graph;


};
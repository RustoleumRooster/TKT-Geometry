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


class System_Light_Multipass {

	struct LightSource_struct {
		alignas(16) vector3df pos;
		uint32_t radius;
	};

	struct RayTraceInfo {

		alignas(16) vector3df light_pos;
		uint32_t n_rays;

		//scene info
		uint32_t n_triangles;
		uint32_t n_vertices;
		uint32_t n_nodes;

		//face info
		uint32_t face_vertex_offset;
		uint32_t face_triangle_offset;
		uint32_t face_n_triangles;

		//lightmap info
		uint32_t lightmap_width;
		uint32_t lightmap_height;

		uint32_t faces_offset;

		/*uint32_t block_x0;
		uint32_t block_y0;
		uint32_t block_width;
		uint32_t block_id;*/
	};

public:
	System_Light_Multipass(MyDevice* device,
		const std::vector<VkBuffer>& uniformBuffers) :

		device{ device },
		uniformBuffers{ uniformBuffers } {
	}
	uint32_t num_lightmaps_used() { return lightmapImages.size(); }
	void cleanup();

	void loadLights(geometry_scene* geo_scene);
	void loadModel(MeshNode_Interface_Final* meshnode);
	static bool verify_inputs(geometry_scene* geo_scene);

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
		createLightsBuffer();
		createQuadBuffer();
	}
	
	void executeComputeShader();
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
	void createLightsBuffer();
	void createRaytraceInfoBuffer();
	void createHitResultsBuffer();
	void createComputePipeline();
	
	void createQuadBuffer();
	void createNodeBuffer();
	void createDescriptorSets(int face_n);
	void writeRaytraceInfoBuffer(int face_n, int f_j);
	void writeRaytraceInfoBuffer2(int lightmap_n);
	void buildLightmap(int face_n, int f_j);

	void buildLightmaps_async();


	VkPipelineLayout pipelineLayout;
	std::vector<VkDescriptorSet> descriptorSets;
	std::vector<VkBuffer> uniformBuffers;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VkBuffer uvBuffer;
	VkDeviceMemory uvBufferMemory;

	VkBuffer nodeBuffer;
	VkDeviceMemory nodeBufferMemory;

	VkBuffer lightSourceBuffer;
	VkDeviceMemory lightSourceBufferMemory;

	VkBuffer firstTriangleBuffer;
	VkDeviceMemory firstTriangleBufferMemory;

	VkBuffer nTrianglesBuffer;
	VkDeviceMemory nTrianglesBufferMemory;

	VkBuffer quadsBuffer;
	VkDeviceMemory quadsBufferMemory;

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

	vector<aligned_uint> nTrianglesFace;
	vector<aligned_uint> firstTriangleFace;

	vector<aligned_vec3> bounding_quads;
	//std::vector<aligned_uint> triangle_edges;

	uint16_t n_indices = 0;
	uint16_t n_vertices = 0;

	//faces
	uint32_t total_n_faces = 0;
	vector<u16> faces_offset;

	std::vector<LightMaps_Info_Struct> lightmaps_info;

	std::vector<VkImage> lightmapImages;
	std::vector<VkDeviceMemory> lightmapsMemory;
	std::vector<VkImageView> lightmapImageViews;

	std::vector<VkImage> workingImages;
	std::vector<VkDeviceMemory> workingImagesMemory;
	std::vector<VkImageView> workingImageViews;

	std::vector<triangle_b> master_triangle_list;
	BVH_structure_triangles my_bvh;

	std::vector<LightSource_struct> lightSources;

	uint32_t n_nodes = 0;

	uint32_t n_lights = 0;

	LineHolder m_graph;
	vector3df light_pos;

};
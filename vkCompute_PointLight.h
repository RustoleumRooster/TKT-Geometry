#pragma once

#include <vulkan/vulkan.h>
#include <irrlicht.h>

#include <vector>
#include <stdexcept>
#include "vkDevice.h"
#include "vkModel.h"
#include "Reflection.h"
#include "vk_BVH.h"
#include "csg_classes.h"

class MyDescriptorSetLayout;
class SwapChain;
class MyTextures;
class MyDescriptorPool;
class ComputePipeline;
class MyBufferObject;
class MeshNode_Interface_Final;



class System_Point_Light {

	struct LightSource_struct {
		alignas(16) vector3df pos;
		uint32_t radius;
	};

	struct RayTraceInfo {
		alignas(16) vector3df eye_pos;
		uint32_t lightradius;
		uint32_t n_rays;
		uint32_t n_lights;

		//scene info
		uint32_t n_triangles;
		uint32_t n_vertices;
		uint32_t n_nodes;

		//face info
		uint32_t face_vertex_offset;
		uint32_t face_index_offset;
		uint32_t face_n_indices;

		//uint32_t face_light_offset;
		//uint32_t face_n_lights;

		//lightmap info
		uint32_t lightmap_width;
		uint32_t lightmap_height;
	};

public:
	System_Point_Light(MyDevice* device,
		const std::vector<VkBuffer>& uniformBuffers) :

		device{ device },
		uniformBuffers{ uniformBuffers } {
	}
	uint32_t num_lightmaps_used() { return lightmapImages.size(); }
	void cleanup();

	void loadLights(geometry_scene* geo_scene);
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
		createLightsBuffer();
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

	void createNodeBuffer();
	void createDescriptorSets(int face_n);
	void writeRaytraceInfoBuffer(int face_n);
	void buildLightmap(int face_n);
	bool triangle_sees_light(uint32_t triangle_i, vector3df pos, uint32_t radius);

	f32 trace_triangle_hit(uint32_t triangle_i, vector3df eye, vector3df ray);

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

	VkBuffer lightSourceBuffer;
	VkDeviceMemory lightSourceBufferMemory;

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

	uint16_t n_indices = 0;
	uint16_t n_vertices = 0;

	//faces
	uint32_t n_faces = 0;

	std::vector<LightMaps_Info_Struct> lightmaps_info;

	std::vector<VkImage> lightmapImages;
	std::vector<VkDeviceMemory> lightmapsMemory;
	std::vector<VkImageView> lightmapImageViews;

	std::vector<LightSource_struct> lightSources;

	std::vector<triangle_b> master_triangle_list;
	BVH_structure_triangles my_bvh;

	LineHolder m_graph;

	uint32_t n_nodes = 0;
	uint32_t n_lights = 0;

};
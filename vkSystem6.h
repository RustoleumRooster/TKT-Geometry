#pragma once

#include <vulkan/vulkan.h>
#include <irrlicht.h>

#include <vector>
#include <stdexcept>
#include "vkDevice.h"
#include "vkModel.h"
#include "Reflection.h"
#include "BVH.h"

class MyDescriptorSetLayout;
class SwapChain;
class MyTextures;
class MyDescriptorPool;
class ComputePipeline;
class MyBufferObject;

class MeshNode_Interface_Final;


struct BVH_node_gpu
{
	alignas(16) f32 aabbMin[4];
	alignas(16) f32 aabbMax[4];
	alignas(16) u32 packing[4];

	u32 left_node = 0xFFFF;
	u32 right_node = 0xFFFF;

	u32 first_prim;
	u32 n_prims;
	
	
	//u32 id = 0;

	template<typename T>
	void grow(T* obj) const
	{
		obj->aabbMin[0] = fmin(obj->aabbMin[0], aabbMin[0]);
		obj->aabbMin[1] = fmin(obj->aabbMin[1], aabbMin[1]);
		obj->aabbMin[2] = fmin(obj->aabbMin[2], aabbMin[2]);

		obj->aabbMax[0] = fmax(obj->aabbMax[0], aabbMax[0]);
		obj->aabbMax[1] = fmax(obj->aabbMax[1], aabbMax[1]);
		obj->aabbMax[2] = fmax(obj->aabbMax[2], aabbMax[2]);
	}

	f32 surface_area()
	{
		return calc_surface_area(aabbMin, aabbMax);
	}

	f32 length(int axis)
	{
		return aabbMax[axis] - aabbMin[axis];
	}

	bool isLeafNode() const {
		return (left_node == 0xFFFF && right_node == 0xFFFF);
	}
	void addDrawLines(LineHolder& graph) const;
	void addDrawLines_Recursive(int depth, const std::vector<BVH_node_gpu>& nodes, int current_depth, LineHolder& graph) const;
	
};

//template<typename T>
struct BVH_structure_triangles : BVH_structure_base<triangle_b,BVH_node_gpu,u32>
{
	std::vector<core::vector3df> positions;
	std::vector<aabb_struct> aabbs;

	virtual void grow_prim(const triangle_b* data, u16 i, aabb_struct& aabb)
	{
		aabbs[i].grow(&aabb);
	}

	virtual core::vector3df prim_position(const triangle_b* data, u16 i)
	{
		return positions[i];
	}

	virtual void assign_node_id(u16 n_i) {}

	//void build(poly_vert* verts, const T* data, int n_prims, bool(*func)(const T*) = NULL) {
	void build(const aligned_vec3* verts, const triangle_b* data, int n_prims, bool(*func)(const triangle_b*) = NULL) {

		positions.resize(n_prims);
		aabbs.resize(n_prims);

		for (int i = 0; i < n_prims; i++)
		{
			positions[i] = data[i].position(verts);

			aabbs[i].clear();
			data[i].grow(&aabbs[i], verts);
		}

		BVH_structure_base<triangle_b, BVH_node_gpu, u32>::construct<2>(data, n_prims, func);
	}

	void addDrawLines(int depth, LineHolder& graph) const
	{
		if (nodes.size() > 0)
		{
			nodes[0].addDrawLines_Recursive(depth, nodes, 0, graph);
		}
	}

	void addDrawLinesPrims(const aligned_vec3* verts, const aligned_uint* indices, int node_i, LineHolder& graph) const;
};

class System6 {

	struct RayTraceInfo {
		alignas(16) vector3df eye_pos;
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
	System6(MyDevice* device,
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
	//void createNormalBuffer();
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

	//std::vector<aligned_vec3> master_triangle_vertices;
	//std::vector<aligned_uint> master_triangle_indices;
	std::vector<triangle_b> master_triangle_list;
	BVH_structure_triangles my_bvh;

	uint32_t n_nodes;

};
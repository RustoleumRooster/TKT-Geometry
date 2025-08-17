#pragma once

#ifndef _LIGHTMAP_MODULES_H_
#define _LIGHTMAP_MODULES_H_

#include <irrlicht.h>
#include <vector>
#include "BufferManager.h"
#include <iterator>
#include "vkDevice.h"
#include "vkModel.h"
#include "vk_BVH.h"
#include <vulkan/vulkan.h>
#include "vkDescriptors.h"
#include "vkBufferObject.h"
#include "vkComputePipeline.h"


class GeometryStack;
class geometry_scene;
class MyDescriptorPool;
class Lightmap_Configuration;

void Lightmap_Routine(geometry_scene*, Lightmap_Configuration*, std::vector<irr::video::ITexture*>&, Lightmap_Configuration*);
void Lightmap_Routine2(geometry_scene* g_scene, Lightmap_Configuration* configuration, std::vector<irr::video::ITexture*>& textures, Lightmap_Configuration* configuration1);

class Geometry_Assets
{
public:
	Geometry_Assets(geometry_scene* g_scene, Lightmap_Configuration*);

	soa_struct_2<aligned_vec3, aligned_vec3> vertices_soa;
	soa_struct<aligned_uint> indices_soa;
	soa_struct<MeshIndex_Chunk> surface_index;
	std::vector<triangle_b> master_triangle_list;
	std::vector<aligned_uint> triangle_edges;
	BVH_structure_triangles bvh;
};

class Vulkan_App
{
public:

	Vulkan_App()
	{
		initVulkan();
	}

	void initVulkan();
	void initBuffers();

	void cleanup();
	void createDescriptorPool();
	void createCommandBuffers();

	void createIndexBuffer();
	void createVertexBuffer();
	void createUVBuffer();
	void createEdgeBuffer();
	void createNodeBuffer();

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VkBuffer uvBuffer;
	VkDeviceMemory uvBufferMemory;

	VkBuffer nodeBuffer;
	VkDeviceMemory nodeBufferMemory;

	VkBuffer edgeBuffer;
	VkDeviceMemory edgeBufferMemory;

	std::vector<VkCommandBuffer> commandBuffers;
	MyDescriptorPool* m_DescriptorPool = NULL;
	MyDevice* m_device = NULL;

	uint16_t n_indices = 0;
	uint16_t n_vertices = 0;
	uint32_t n_nodes = 0;

	soa_struct_2<aligned_vec3, aligned_vec3>* vertices_soa = NULL;
	soa_struct<aligned_uint>* indices_soa = NULL;
	std::vector<aligned_uint>* triangle_edges = NULL;
	BVH_structure_triangles* bvh = NULL;
};

class Vulkan_Module
{
public:

	Vulkan_Module(Vulkan_App* vulkan);

	virtual void run() = 0;

	Vulkan_App* vulkan = NULL;
	MyDevice* m_device = NULL;
	MyDescriptorPool* m_DescriptorPool = NULL;
};

class Create_Images_Module : public Vulkan_Module
{
public:

	Create_Images_Module(Vulkan_App* vulkan, Lightmap_Configuration* configuration)
	: Vulkan_Module(vulkan), configuration{ configuration }
	{}

	virtual void run() override;
	void createImages();
	void destroyImages();

	Lightmap_Configuration* configuration = NULL;
	
	std::vector<VkImage> lightmapImages;
	std::vector<VkDeviceMemory> lightmapsMemory;
	std::vector<VkImageView> lightmapImageViews;
};

class Lightmap_Edges_Module : public Vulkan_Module
{
	struct ShaderParamsInfo {
		uint32_t lightmap_width;
		uint32_t lightmap_height;
	};

public:

	Lightmap_Edges_Module(Vulkan_App* vulkan, Lightmap_Configuration* configuration)
		: Vulkan_Module(vulkan), configuration{ configuration }
	{}

	void createDescriptorSetLayout();
	void createDescriptorSets(int);
	void createComputePipeline();

	virtual void run() override;
	void execute(int);
	void destroyImages();

	Lightmap_Configuration* configuration = NULL;

	std::vector<VkImage> lightmapImages_in;
	std::vector<VkDeviceMemory> lightmapsMemory_in;
	std::vector<VkImageView> lightmapImageViews_in;

	std::vector<VkImage> lightmapImages_out;
	std::vector<VkDeviceMemory> lightmapsMemory_out;
	std::vector<VkImageView> lightmapImageViews_out;

	VkPipelineLayout pipelineLayout;
	std::vector<VkDescriptorSet> descriptorSets;

	MyDescriptorSetLayout* descriptorSetLayout = NULL;
	ComputePipeline* pipeline = NULL;
	MyBufferObject* shaderParamsBufferObject = NULL;
};

class Load_Textures_Module : public Vulkan_Module
{
public:
	Load_Textures_Module(Vulkan_App* vulkan, video::IVideoDriver* driver, Lightmap_Configuration* configuration)
		: Vulkan_Module(vulkan), driver{ driver }, configuration{ configuration }
	{}

	virtual void run() override;
	void createImages();
	void destroyImages();

	Lightmap_Configuration* configuration = NULL;

	std::vector<VkImage> lightmapImages;
	std::vector<VkDeviceMemory> lightmapsMemory;
	std::vector<VkImageView> lightmapImageViews;

	std::vector<video::ITexture*> textures;
	video::IVideoDriver* driver;
};

class Download_Textures_Module : public Vulkan_Module
{
public:
	Download_Textures_Module(Vulkan_App* vulkan, video::IVideoDriver* driver, Lightmap_Configuration* configuration)
		: Vulkan_Module(vulkan), driver{driver}, configuration{configuration}
	{}

	virtual void run() override;

	std::vector<VkImage> lightmapImages;
	std::vector<VkDeviceMemory> lightmapsMemory;
	std::vector<VkImageView> lightmapImageViews;

	std::vector<video::ITexture*> textures;

	video::IVideoDriver* driver;
	Lightmap_Configuration* configuration;

	bool bFlip = false;
};

class Copy_Lightmaps_Module : public Vulkan_Module
{
	struct ShaderParamsInfo {
		uint32_t lightmap0_width;
		uint32_t lightmap0_height;

		uint32_t lightmap1_width;
		uint32_t lightmap1_height;

		uint32_t face_vertex_offset; //not used in shader
		uint32_t face_index_offset;
		uint32_t face_n_indices;
		uint32_t intensity;
	};

public:
	Copy_Lightmaps_Module(Vulkan_App* vulkan,
		Lightmap_Configuration* configuration0, Lightmap_Configuration* configuration1)
		: Vulkan_Module(vulkan), configuration0{ configuration0 }, configuration1{ configuration1 }
	{}

	void createDescriptorSetLayout();
	void createDescriptorSets(int,int);
	void createComputePipeline();
	void createImageViews();
	void createImages();
	void createUVBuffer();

	VkSampler createDefaultSampler();

	virtual void run() override;
	void execute(int,int,int,int,int);
	void destroyImages();

	std::vector<aligned_vec3>* uv_struct_0;
	std::vector<aligned_vec3>* uv_struct_1;
	int n_vertices = 0;

	Lightmap_Configuration* configuration0;
	Lightmap_Configuration* configuration1;

	std::vector<int>* element_by_element_id = NULL;
	soa_struct<MeshIndex_Chunk>* surface_index = NULL;

	std::vector<VkImage> lightmapImages_in;
	std::vector<VkDeviceMemory> lightmapsMemory_in;
	std::vector<VkImageView> lightmapImageViews_in;

	std::vector<VkImage> lightmapImages_out;
	std::vector<VkDeviceMemory> lightmapsMemory_out;
	std::vector<VkImageView> lightmapImageViews_out;

	VkBuffer uvBuffer_0;
	VkDeviceMemory uvBufferMemory_0;

	VkBuffer uvBuffer_1;
	VkDeviceMemory uvBufferMemory_1;

	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	int n_indices = 0;

	VkSampler mySampler;

	VkPipelineLayout pipelineLayout;
	std::vector<VkDescriptorSet> descriptorSets;

	MyDescriptorSetLayout* descriptorSetLayout = NULL;
	ComputePipeline* pipeline = NULL;
	MyBufferObject* shaderParamsBufferObject = NULL;
	
};

#endif
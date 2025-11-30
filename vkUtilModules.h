#pragma once
#ifndef _VK_UTIL_MODULES_H_
#define _VK_UTIL_MODULES_H_

#include <irrlicht.h>
#include <vector>
#include "BufferManager.h"
#include <iterator>
#include "vkDevice.h"
#include "vkModel.h"
#include "vkModules.h"
#include "vk_BVH.h"
#include <vulkan/vulkan.h>
#include "vkDescriptors.h"
#include "vkBufferObject.h"
#include "vkComputePipeline.h"
#include "reflect_custom_types.h"


class GeometryStack;
class geometry_scene;
class MyDescriptorPool;
class Lightmap_Configuration;


class Geometry_Module : public Vulkan_Module
{
public:

	Geometry_Module(Vulkan_App* vulkan, Geometry_Assets* geo_assets);

	void initBuffers();
	void init_area_light_buffer();

	void createIndexBuffer();
	void createVertexBuffer();
	void createUVBuffer();
	void createEdgeBuffer();
	void createNodeBuffer();
	void createOutputBuffer();
	virtual void run();

	reflect::output<vkBufferResource> vertices;
	reflect::output<vkBufferResource> indices;
	reflect::output<vkBufferResource> lm_uvs;
	reflect::output<vkBufferResource> bvh_nodes;
	reflect::output<vkBufferResource> edges;
	reflect::output<vkBufferResource> scratchpad;
	reflect::output<vkBufferResource> area_lights;

	uint16_t n_indices = 0;
	uint16_t n_vertices = 0;
	uint32_t n_nodes = 0;

	soa_struct_2<aligned_vec3, aligned_vec3>* vertices_soa = NULL;
	soa_struct<aligned_uint>* indices_soa = NULL;
	std::vector<aligned_uint>* triangle_edges = NULL;
	std::vector<aligned_uint>* area_light_indices = NULL;
	BVH_structure_triangles* bvh = NULL;

	Geometry_Assets* geo_assets;

	REFLECT3()
};

class MultiImage_To_ImageArray_Module : public Vulkan_Module
{
public:

	MultiImage_To_ImageArray_Module(Vulkan_App* vulkan, Lightmap_Configuration* configuration)
		: Vulkan_Module(vulkan), configuration{ configuration }
	{
		set_ptrs();
	}

	virtual void run();
	void createImages();

	Lightmap_Configuration* configuration = NULL;

	reflect::input<vkMultiImageResource> images_in;
	reflect::output<vkImageArrayResource> images_out;

	REFLECT3()
};

class MultiImage_Copy_Module : public Vulkan_Module
{
public:

	MultiImage_Copy_Module(Vulkan_App* vulkan, Lightmap_Configuration* configuration)
		: Vulkan_Module(vulkan), configuration{ configuration }
	{
		set_ptrs();
	}

	virtual void run();
	void createImages();

	Lightmap_Configuration* configuration = NULL;

	reflect::input<vkMultiImageResource> images_in;
	reflect::output<vkMultiImageResource> images_out;

	REFLECT3()
};

class Merge_Images_Module : public Vulkan_Module
{
	struct ShaderParamsInfo {
		uint32_t lightmap_width;
		uint32_t lightmap_height;
	};

public:

	Merge_Images_Module(Vulkan_App* vulkan, Lightmap_Configuration* configuration)
		: Vulkan_Module(vulkan), configuration{ configuration }
	{
		set_ptrs();
	}

	void createDescriptorSetLayout();
	void createDescriptorSets(int);

	virtual void run();
	virtual void execute(u32);
	void createImages();

	Lightmap_Configuration* configuration = NULL;

	vkUniformBufferResource ubo;

	reflect::input<vkMultiImageResource> images_in_0;
	reflect::input<vkMultiImageResource> images_in_1;
	reflect::output<vkMultiImageResource> images_out;

	REFLECT3()
};

class Create_Lightmap_Images_Module : public Vulkan_Module
{
public:

	Create_Lightmap_Images_Module(Vulkan_App* vulkan, Lightmap_Configuration* configuration)
		: Vulkan_Module(vulkan), configuration{ configuration }
	{
		set_ptrs();
	}

	virtual void run();
	void createImages();

	Lightmap_Configuration* configuration = NULL;

	reflect::output<vkMultiImageResource> images_out;

	REFLECT3()
};

class Create_Texture_Images_Module : public Vulkan_Module
{
public:

	Create_Texture_Images_Module(Vulkan_App* vulkan, Lightmap_Configuration* configuration)
		: Vulkan_Module(vulkan), configuration{ configuration }
	{
		set_ptrs();
	}
	~Create_Texture_Images_Module();

	virtual void run();
	void index_materials(Lightmap_Configuration* configuration);

	Lightmap_Configuration* configuration = NULL;

	reflect::output<vkMultiImageResource> images_out;

	std::vector<video::ITexture*> textures;

	video::IVideoDriver* driver;

	REFLECT3()
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
	{
		set_ptrs();
	}

	void createDescriptorSetLayout();
	void createDescriptorSets(int);
	void createComputePipeline();

	virtual void run() override;
	void execute(int);

	Lightmap_Configuration* configuration = NULL;

	MyBufferObject* shaderParamsBufferObject = NULL;

	reflect::input<vkMultiImageResource> images_in;
	reflect::output<vkMultiImageResource> images_out;

	REFLECT3()
};

class Load_Textures_Module : public Vulkan_Module
{
public:
	Load_Textures_Module(Vulkan_App* vulkan, video::IVideoDriver* driver, Lightmap_Configuration* configuration)
		: Vulkan_Module(vulkan), driver{ driver }, configuration{ configuration }
	{
		set_ptrs();
	}

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
		: Vulkan_Module(vulkan), driver{ driver }, configuration{ configuration }
	{
		set_ptrs();
	}

	virtual void run() override;

	reflect::input<vkMultiImageResource> images_in;

	std::vector<video::ITexture*> textures;

	video::IVideoDriver* driver;
	Lightmap_Configuration* configuration;

	bool bFlip = false;

	REFLECT3()
};

class Download_TextureArray_Module : public Vulkan_Module
{
public:

	Download_TextureArray_Module(Vulkan_App* vulkan, video::IVideoDriver* driver, Lightmap_Configuration* configuration)
		: Vulkan_Module(vulkan), driver{ driver }, configuration{ configuration }
	{
		set_ptrs();
	}

	virtual void run() override;

	reflect::input<vkImageArrayResource> images_in;

	std::vector<video::ITexture*> textures;

	video::IVideoDriver* driver;
	Lightmap_Configuration* configuration;

	bool bFlip = false;

	REFLECT3()
};


#endif
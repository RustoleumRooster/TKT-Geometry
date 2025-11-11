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
#include "reflect_custom_types.h"


class GeometryStack;
class geometry_scene;
class MyDescriptorPool;
class Lightmap_Configuration;

void Lightmap_Routine(geometry_scene*, Lightmap_Configuration*, std::vector<irr::video::ITexture*>&, Lightmap_Configuration*);

struct vkResource
{
	
};

struct vkRaytraceResource
{

};

struct vkUniformBufferResource
{
	VkBuffer Buffer;
	VkDeviceMemory BufferMemory;
	u32 range = 0;

	VkDescriptorSetLayoutBinding getDescriptorSetLayout(u32);
	VkDescriptorBufferInfo getDescriptorBufferInfo();

	void destroy(VkDevice);
	void writeToBuffer(VkDevice device, void* data, VkDeviceSize = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

};

struct vkBufferResource
{
	VkBuffer Buffer;
	VkDeviceMemory BufferMemory;
	u32 range = 0;

	VkDescriptorSetLayoutBinding getDescriptorSetLayout(u32);
	VkDescriptorBufferInfo getDescriptorBufferInfo();

	void destroy(VkDevice);

	REFLECT()
};

struct vkImageResource
{
	VkImage Image;
	VkDeviceMemory ImageMemory;
	VkImageView ImageView;

	void destroy(VkDevice);

	REFLECT()
};

struct vkMultiImageResource 
{
	std::vector<vkImageResource> Images;

	VkDescriptorSetLayoutBinding getDescriptorSetLayout(u32);
	VkDescriptorImageInfo getDescriptorBufferInfo(u32);

	REFLECT()
};

class Vulkan_Module;
std::vector<Vulkan_Module*>* get_all_vk_modules();

Vulkan_Module* get_module_by_uid(std::vector<Vulkan_Module*>*, u64 uid);

namespace reflect
{
#define REFLECT3() \
        virtual reflect::TypeDescriptor_Struct* GetDynamicReflection(); \
        friend struct reflect::DefaultResolver; \
        static reflect::TypeDescriptor_Struct Reflection; \
        static void initReflection(reflect::TypeDescriptor_Struct*); \

#define REFLECT_STRUCT3_BEGIN(type) \
        reflect::TypeDescriptor_Struct type::Reflection{type::initReflection}; \
        reflect::TypeDescriptor_Struct* type::GetDynamicReflection() {\
            return &type::Reflection;\
            }\
        void type::initReflection(reflect::TypeDescriptor_Struct* typeDesc) { \
            using T = type; \
            typeDesc->name = #type; \
            typeDesc->size = sizeof(T); \
            typeDesc->inherited_type = NULL; \
            typeDesc->name_func = NULL; \
            typeDesc->alias = typeDesc->name;

#define REFLECT_STRUCT3_END() \
    }

	template <typename TY> struct input;
	template <typename TY> struct output;

	template <typename TY>
	void connect(input<TY>* in, output<TY>* out);

	struct input_type
	{
		u64 my_uid = 0;
		virtual bool load() = 0;
		REFLECT()
	};

	struct output_type
	{
		u64 my_uid = 0;
		REFLECT()
	};

	template <typename TY>
	struct input : public input_type
	{
		struct attributes
		{
		};

		u64 input_uid = 0;
		std::string input_member;
		u64 old_uid;	//not reflected
		TY X;			//not reflected

		virtual bool load() override;

		REFLECT_CUSTOM_STRUCT()
	};

	template <typename TY>
	struct output : public output_type
	{
		struct attributes
		{
		};

		std::vector<u64> output_uids;
		std::vector<u64> old_uids;	//not reflected
		TY X;						//not reflected
		bool ready = false;			//not reflected

		REFLECT_CUSTOM_STRUCT()
	};

	template <typename TY>
	void connect( output<TY>* out, input<TY>* in)
	{
		in->input_uid = out->my_uid;
		out->output_uids.push_back(in->my_uid);

		std::vector<Vulkan_Module*>* all_vulkan_modules = get_all_vk_modules();

		if (!all_vulkan_modules)
			return;

		Vulkan_Module* out_module = get_module_by_uid(all_vulkan_modules, out->my_uid);

		if (!out_module)
			return;

		reflect::TypeDescriptor_Struct* tD = out_module->GetDynamicReflection();
		for (reflect::Member m : tD->members)
		{
			if ((char*)out_module + m.offset == (char*)out)
			{
				in->input_member = m.name;
			}
		}

	}

	template <typename TY>
	void disconnect(input<TY>* in, output<TY>* out)
	{

	}

	template <typename TY>
	bool input<TY>::load()
	{
		std::vector<Vulkan_Module*>* all_vulkan_modules = get_all_vk_modules();

		Vulkan_Module* module = get_module_by_uid(all_vulkan_modules, this->input_uid);
		
		if (!module)
			return false;

		TypeDescriptor_Struct* td = module->GetDynamicReflection();

		if (!td)
			return false;

		for (reflect::Member& m : td->members)
		{
			if (strcmp(this->input_member.c_str(), m.name) == 0)
			{
				output<TY>* src = (output<TY>*)((char*)module + m.offset);

				if (!src->ready)
					return false;

				this->X = src->X;

				return true;
			}
		}

		return false;
	}
}

struct AreaLightInfoStruct
{
	vector<u32> indices;
	u32 element_id;
	u32 face_id;
	u32 intensity;
};

class Triangle_Transformer
{
public:

	Triangle_Transformer(soa_struct_2<aligned_vec3, aligned_vec3>& verts, soa_struct<aligned_uint>& indx)
		: vertices_soa{ verts }, indices_soa{ indx } {}

	u16 indexed(u16 v_i)
	{
		return indices_soa.data[v_i].x;
	}

	u16 indexed(u16 v_i, u16 offset)
	{
		return indices_soa.data[v_i + offset].x;
	}

	float floor_y_value(float y, float lm_height)
	{
		return (floor(lm_height * y) + 0.5) / lm_height;
	}

	float floor_x_value(float x, float lm_width)
	{
		return (floor(lm_width * x) + 0.5) / lm_width;
	}

	bool get_uvs_for_triangle(int triangle_no, int lm_size, vector3df& w0, vector3df& w1, vector3df& w2);

	const soa_struct_2<aligned_vec3, aligned_vec3>& vertices_soa;
	const soa_struct<aligned_uint>& indices_soa;
};

class Geometry_Assets
{
public:
	Geometry_Assets(geometry_scene* g_scene, Lightmap_Configuration*);

	soa_struct_2<aligned_vec3, aligned_vec3> vertices_soa;
	soa_struct<aligned_uint> indices_soa;
	soa_struct<MeshIndex_Chunk> surface_index;
	std::vector<triangle_b> master_triangle_list;
	std::vector<aligned_uint> triangle_edges;
	std::vector<aligned_uint> area_light_indices;
	std::vector<AreaLightInfoStruct> area_lights;
	BVH_structure_triangles bvh;
	int selected_triangle_index = 0;
	int selected_triangle_mg = 0;
	vector3df selected_triangle_bary_coords{ 0,0,0 };

};

class Vulkan_App;

class Vulkan_Module
{
public:

	Vulkan_Module(Vulkan_App* vulkan);

	void createComputePipeline(const char* shader_path);

	virtual void run() = 0;
	bool load_resources();
	void set_uids();

	Vulkan_App* vulkan = NULL;
	MyDevice* m_device = NULL;
	MyDescriptorPool* m_DescriptorPool = NULL;

	VkPipelineLayout pipelineLayout;
	std::vector<VkDescriptorSet> descriptorSets;
	MyDescriptorSetLayout* descriptorSetLayout = NULL;
	ComputePipeline* pipeline = NULL;

	u64 m_uid;

	REFLECT3()
};

class Vulkan_App
{
public:

	Vulkan_App(Geometry_Assets* geo_assets);

	void initVulkan();
	void cleanup();

	void createDescriptorPool();
	void createCommandBuffers();

	std::vector<VkCommandBuffer> commandBuffers;
	MyDescriptorPool* m_DescriptorPool = NULL;
	MyDevice* m_device = NULL;
	std::vector<Vulkan_Module*> all_modules;
};

class Geometry_Module : public Vulkan_Module
{
public:

	Geometry_Module(Vulkan_App* vulkan, Geometry_Assets* geo_assets);

	void initBuffers();
	void init_area_light_buffer();

	void cleanup();
	
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



class Create_Images_Module : public Vulkan_Module
{
public:

	Create_Images_Module(Vulkan_App* vulkan, Lightmap_Configuration* configuration)
	: Vulkan_Module(vulkan), configuration{ configuration }
	{
		set_uids();
		//images_out.my_uid = m_uid;
	}

	virtual void run();
	void createImages();

	Lightmap_Configuration* configuration = NULL;

	reflect::output<vkMultiImageResource> images_out;

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
		set_uids();
		//images_in.my_uid = m_uid;
		//images_out.my_uid = m_uid;
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
		set_uids();
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
		: Vulkan_Module(vulkan), driver{driver}, configuration{configuration}
	{
		set_uids();
	}

	virtual void run() override;

	reflect::input<vkMultiImageResource> images_in;

	std::vector<video::ITexture*> textures;

	video::IVideoDriver* driver;
	Lightmap_Configuration* configuration;

	bool bFlip = false;

	REFLECT3()
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
	{
		set_uids();
	}

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

	MyBufferObject* shaderParamsBufferObject = NULL;
	
};

#endif
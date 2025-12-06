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


namespace reflect 
{

	struct input_type;
	struct output_type;

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

#define REFLECT_STRUCT_MEMBER_FORWARD(name0,name1) \
		int a = offsetof(T,name0);int b = offsetof(T,name1);\
        for(int i=0;i<typeDesc->members.size();i++) {\
			if(strcmp(typeDesc->members[i].name,#name0)==0) {\
				for(int j=0;j<typeDesc->members.size();j++) {\
					if(strcmp(typeDesc->members[j].name,#name1)==0) { \
						typeDesc->members[i].forward_output = j; \
						std::cout << #name0 <<" FWD DST = "<<j<<", "<<typeDesc->members[j].name<<"\n";\
					}\
				}\
			}\
		}

#define REFLECT_STRUCT3_END() \
		}
}

enum {
	RESOURCE_UNK,
	RESOURCE_VALID,
	RESOURCE_DESTROYED
};

struct vkMemoryResource
{
	vkMemoryResource(reflect::output_type*);
	void find_consumers(reflect::output_type*);
	void consume(reflect::input_type*);

	u32 reference_count = 0;
	std::vector<reflect::input_type*> consumers;
	virtual void destroy(VkDevice) = 0;
	reflect::output_type* owner = NULL;
	int status = RESOURCE_UNK;

	virtual reflect::TypeDescriptor_Struct* GetDynamicReflection() = 0;
};

struct vkRaytraceResource
{
	
};

struct vkUniformBufferResource //: public vkMemoryResource
{

	VkBuffer Buffer;
	VkDeviceMemory BufferMemory;
	u32 range = 0;

	VkDescriptorSetLayoutBinding getDescriptorSetLayout(u32);
	VkDescriptorBufferInfo getDescriptorBufferInfo();

	void destroy(VkDevice);
	void writeToBuffer(VkDevice device, void* data, VkDeviceSize = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

};

struct vkBufferResource : public vkMemoryResource
{
	VkBuffer Buffer;
	VkDeviceMemory BufferMemory;
	u32 range = 0;

	vkBufferResource(reflect::output_type* out) : vkMemoryResource(out) {}
	VkDescriptorSetLayoutBinding getDescriptorSetLayout(u32);
	VkDescriptorBufferInfo getDescriptorBufferInfo();

	void destroy(VkDevice);

	REFLECT3()
};

struct vkImageResource : public vkMemoryResource
{
	VkImage Image;
	VkDeviceMemory ImageMemory;
	VkImageView ImageView;

	vkImageResource(reflect::output_type* out) : vkMemoryResource(out) {}
	void destroy(VkDevice);
	void create_and_load_texture(MyDevice* device, video::IVideoDriver* driver, video::ITexture* tex);

	REFLECT3()
};

struct vkImageSubresource
{
	VkImage Image;
	VkDeviceMemory ImageMemory;
	VkImageView ImageView;

	void destroy(VkDevice);
	void create_and_load_texture(MyDevice* device, video::IVideoDriver* driver, video::ITexture* tex);

	REFLECT3()
};

struct vkImageArrayResource : public vkMemoryResource
{
	VkImage Image;
	VkDeviceMemory ImageMemory;
	VkImageView ImageView;

	int n_images = 0;

	vkImageArrayResource(reflect::output_type* out) : vkMemoryResource(out) {}

	void destroy(VkDevice);
	VkDescriptorSetLayoutBinding getDescriptorSetLayout(u32);
	VkDescriptorImageInfo* getDescriptorBufferInfo();
	void initializeDescriptorInfo();

	std::vector<VkDescriptorImageInfo> imageStorageInfo;

	REFLECT3()
};

struct vkMultiImageResource : public vkMemoryResource
{
	std::vector<vkImageSubresource> Images;

	vkMultiImageResource(reflect::output_type* out) : vkMemoryResource(out) {}

	VkDescriptorSetLayoutBinding getDescriptorSetLayout(u32);
	VkDescriptorImageInfo getDescriptorBufferInfo(u32);

	void destroy(VkDevice);

	REFLECT3()
};

class Vulkan_Module;
std::vector<Vulkan_Module*>* get_all_vk_modules();

Vulkan_Module* get_module_by_uid(std::vector<Vulkan_Module*>*, u64 uid);

namespace reflect
{

	template <typename TY> struct input;
	template <typename TY> struct output;

	template <typename TY>
	void connect(input<TY>* in, output<TY>* out);

	struct output_type;

	enum {
		INPUT_NOT_CONNECTED,
		INPUT_WAITING,
		INPUT_FINISHED
	};

	struct input_type
	{
		//u64 my_uid = 0;
		
		u64 input_uid = 0;
		std::string input_member;

		//not reflected
		//

		Vulkan_Module* owner = NULL;
		output_type* src_output = NULL;		
		output_type* forward_output = NULL;	
		bool ready = false;				
		int status = INPUT_NOT_CONNECTED;

		u64 forward_uid = 0;
		

		REFLECT3()
	};

	struct output_type
	{
		//u64 my_uid = 0;
		std::vector<u64> output_uids;
		std::vector<std::string> output_member;

		//not reflected
		//
		virtual void consume(input_type*) = 0;
		Vulkan_Module* owner = NULL;
		std::vector<input_type*> dest_inputs;	
		bool ready = false;						

		virtual void signal() = 0;
		virtual void push() = 0;

		REFLECT3()
	};

	template <typename TY>
	struct input : public input_type
	{
		struct attributes
		{
		};

		input() {}

		u64 old_uid;	//not reflected
		TY* X = NULL;			//not reflected

		//virtual bool load() override;

		virtual reflect::TypeDescriptor_Struct* GetDynamicReflection() { return &input<TY>::Reflection; }
		REFLECT_CUSTOM_STRUCT()
	};

	template <typename TY>
	struct output : public output_type
	{
		struct attributes
		{
		};

		output() {}

		virtual void consume(input_type*);

		std::vector<u64> old_uids;	//not reflected
		TY* X = NULL;						

		virtual void signal() override;
		virtual void push() override;

		virtual reflect::TypeDescriptor_Struct* GetDynamicReflection() { return &output<TY>::Reflection; }
		REFLECT_CUSTOM_STRUCT()
	};


	//void connect(output<vkImageArrayResource>* out, input<vkMultiImageResource>* in)
	//{
	//}

	template <typename TY>
	void connect( output<TY>* out, input<TY>* in)
	{
		//in->input_uid = out->my_uid;
		in->input_uid = out->owner->m_uid;
		//out->output_uids.push_back(in->my_uid);
		out->output_uids.push_back(in->owner->m_uid);

		Vulkan_Module* out_module = out->owner;
		Vulkan_Module* in_module = in->owner;

		if (!out_module || !in_module)
			return;

		reflect::TypeDescriptor_Struct* out_tD = out_module->GetDynamicReflection();
		for (int i=0;i< out_tD->members.size();i++)
		{
			reflect::Member& m = out_tD->members[i];

			if ((char*)out_module + m.offset == (char*)out)
			{
				in->input_member = m.name;
				in->src_output = (reflect::output_type*)m.get(out_module);
				in->status = reflect::INPUT_WAITING;
			}
		}

		reflect::TypeDescriptor_Struct* in_tD = in_module->GetDynamicReflection();
		for (int i = 0; i < in_tD->members.size(); i++)
		{
			reflect::Member& m = in_tD->members[i];

			if ((char*)in_module + m.offset == (char*)in)
			{
				out->output_member.push_back(m.name);
				out->dest_inputs.push_back((reflect::input_type*)m.get(in_module));
			}
		}

	}

	template <typename TY>
	void disconnect(input<TY>* in, output<TY>* out)
	{

	}

	template <typename TY>
	void output<TY>::consume(input_type* in)
	{
		X->consume(in);
	}

	template <typename TY>
	void output<TY>::push()
	{
		std::vector<Vulkan_Module*>* all_vulkan_modules = get_all_vk_modules();

		for (int i = 0; i < this->output_uids.size(); i++)
		{
			Vulkan_Module* module = get_module_by_uid(all_vulkan_modules, this->output_uids[i]);

			if (!module)
				return;

			TypeDescriptor_Struct* in_td = module->GetDynamicReflection();

			if (!in_td)
				return;

			input<TY>* in = (input<TY>*)this->dest_inputs[i];
			in->X = this->X;
			in->ready = true;
		}
	}

	template <typename TY>
	void output<TY>::signal()
	{
		std::vector<Vulkan_Module*>* all_vulkan_modules = get_all_vk_modules();

		for (int i = 0; i < this->output_uids.size(); i++)
		{
			Vulkan_Module* module = get_module_by_uid(all_vulkan_modules, this->output_uids[i]);

			if (!module)
				return;

			TypeDescriptor_Struct* in_td = module->GetDynamicReflection();

			if (!in_td)
				return;

			input<TY>* in = (input<TY>*)this->dest_inputs[i];
			in->X = this->X;
			in->ready = true;

			module->signaled();
		}
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

enum
{
	VK_MODULE_NOT_RAN = 0,
	VK_MODULE_RAN
};

class Vulkan_Module
{
public:

	Vulkan_Module(Vulkan_App* vulkan);

	void createComputePipeline(const char* shader_path);

	virtual void run() = 0;
	//bool load_resources();
	bool all_resources_ready();
	bool signaled();
	void run_and_push();
	void set_ptrs();

	Vulkan_App* vulkan = NULL;
	MyDevice* m_device = NULL;
	MyDescriptorPool* m_DescriptorPool = NULL;

	VkPipelineLayout pipelineLayout;
	std::vector<VkDescriptorSet> descriptorSets;
	MyDescriptorSetLayout* descriptorSetLayout = NULL;
	ComputePipeline* pipeline = NULL;

	u64 m_uid;
	int my_status = VK_MODULE_NOT_RAN;
	bool enabled = true;

	REFLECT3()
};

class Geometry_Module;

class Vulkan_App
{
public:

	Vulkan_App(Geometry_Assets* geo_assets, Lightmap_Configuration* configuration, video::IVideoDriver* driver);

	void initVulkan();
	void cleanup();
	void status();
	void run_workflow();

	void createDescriptorPool();
	void createCommandBuffers();

	vkImageArrayResource* create_imageArray(int n_layers, int width, int height, VkImageUsageFlags flags, reflect::output_type*);
	vkMultiImageResource* create_multiImage(int n_layers, int width, int height, VkImageUsageFlags flags, reflect::output_type*);
	vkImageResource* create_image(int width, int height, VkImageUsageFlags flags, reflect::output_type*);
	vkBufferResource* create_buffer(VkDeviceSize bufferSize, VkBufferUsageFlags flags, reflect::output_type*);

	template<typename T>
	Vulkan_Module* create_module();


	std::vector<VkCommandBuffer> commandBuffers;
	MyDescriptorPool* m_DescriptorPool = NULL;
	MyDevice* m_device = NULL;
	std::vector<Vulkan_Module*> all_modules;
	std::vector<vkMemoryResource*> resources;

	video::IVideoDriver* driver;
	Lightmap_Configuration* configuration;
	Geometry_Module* geo_module = NULL;
	Geometry_Assets* geo_assets;
};

template<typename T>
Vulkan_Module* Vulkan_App::create_module()
{
	reflect::TypeDescriptor_Struct* descriptor = &T::Reflection;

	void* mem = malloc(descriptor->size);
	T* module = (T*)mem;

	new(module) T(this);

	return module;
}

#endif
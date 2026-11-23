#include <irrlicht.h>
#include "LightMaps.h"
#include "vkModules.h"
#include "vkUtilModules.h"
#include "csg_classes.h"
#include "utils.h"
#include "geometry_scene.h"
#include "soa.h"
#include "vkSunlightModule.h"
#include "vkAreaLightModule.h"
#include "my_reflected_nodes.h"
#include "material_groups.h"
#include <vulkan/vulkan.h>
#include "reflect_custom_types.h"
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <cfloat>

using namespace irr;
using namespace core;
using namespace std;

extern IrrlichtDevice* device;
extern VkDevice vk_device = NULL;

#define PRINTV(x) << x.X <<","<<x.Y<<","<<x.Z<<" "


reflect::Vulkan_Reflection_Factory Vulkan_Module::factory{};

/*
REFLECT_CUSTOM_STRUCT_BEGIN_TEMPLATE(int, reflect::input)
	REFLECT_STRUCT_MEMBER(my_uid)
	REFLECT_STRUCT_MEMBER(input_uids)
REFLECT_STRUCT_END()*/

REFLECT_STRUCT3_BEGIN(vkBufferResource)
	REFLECT_STRUCT_MEMBER(Buffer)
	REFLECT_STRUCT_MEMBER(BufferMemory)
REFLECT_STRUCT_END()

REFLECT_STRUCT3_BEGIN(vkImageArrayResource)
	REFLECT_STRUCT_MEMBER(Image)
REFLECT_STRUCT_END()

REFLECT_STRUCT3_BEGIN(vkMultiImageResource)
	REFLECT_STRUCT_MEMBER(Images)
REFLECT_STRUCT_END()

REFLECT_STRUCT3_BEGIN(vkImageResource)
	REFLECT_STRUCT_MEMBER(Image)
	REFLECT_STRUCT_MEMBER(ImageMemory)
	REFLECT_STRUCT_MEMBER(ImageView)
REFLECT_STRUCT_END()

REFLECT_STRUCT3_BEGIN(vkImageSubresource)
	REFLECT_STRUCT_MEMBER(Image)
	REFLECT_STRUCT_MEMBER(ImageMemory)
	REFLECT_STRUCT_MEMBER(ImageView)
REFLECT_STRUCT_END()

REFLECT_CUSTOM_STRUCT_BEGIN_TEMPLATE(vkImageArrayResource, reflect::input)
	INHERIT_FROM(reflect::input_type)
	REFLECT_STRUCT_MEMBER(input_uid)
	REFLECT_STRUCT_MEMBER(input_member)
REFLECT_STRUCT_END()

REFLECT_CUSTOM_STRUCT_BEGIN_TEMPLATE(vkImageArrayResource, reflect::output)
	INHERIT_FROM(reflect::output_type)
	REFLECT_STRUCT_MEMBER(output_uids)
	REFLECT_STRUCT_MEMBER(output_member);
REFLECT_STRUCT_END()

REFLECT_CUSTOM_STRUCT_BEGIN_TEMPLATE(vkMultiImageResource, reflect::input)
	INHERIT_FROM(reflect::input_type)
	REFLECT_STRUCT_MEMBER(input_uid)
	REFLECT_STRUCT_MEMBER(input_member)
REFLECT_STRUCT_END()

REFLECT_CUSTOM_STRUCT_BEGIN_TEMPLATE(vkMultiImageResource, reflect::output)
	INHERIT_FROM(reflect::output_type)
	REFLECT_STRUCT_MEMBER(output_uids)
	REFLECT_STRUCT_MEMBER(output_member);
REFLECT_STRUCT_END()

REFLECT_CUSTOM_STRUCT_BEGIN_TEMPLATE(vkBufferResource, reflect::output)
	INHERIT_FROM(reflect::output_type)
	REFLECT_STRUCT_MEMBER(output_uids)
	REFLECT_STRUCT_MEMBER(output_member);
REFLECT_STRUCT_END()

REFLECT_CUSTOM_STRUCT_BEGIN_TEMPLATE(vkBufferResource, reflect::input)
	INHERIT_FROM(reflect::input_type)
	REFLECT_STRUCT_MEMBER(input_uid)
	REFLECT_STRUCT_MEMBER(input_member)
REFLECT_STRUCT_END()

//ports are graph wiring, edited in the node graph: editing their uids in a form
//would leave the live connections (dest_inputs / src_output) out of step
REFLECT_STRUCT3_BEGIN(reflect::input_type)
	HIDE_IN_FORMS()
REFLECT_STRUCT_END()

REFLECT_STRUCT3_BEGIN(reflect::output_type)
	HIDE_IN_FORMS()
REFLECT_STRUCT_END()

namespace reflect
{
	template <>
	TypeDescriptor* getPrimitiveDescriptor<VkImage>() {
		static TypeDescriptor_U64 typeDesc;
		return &typeDesc;
	}
	template <>
	TypeDescriptor* getPrimitiveDescriptor<VkImageView>() {
		static TypeDescriptor_U64 typeDesc;
		return &typeDesc;
	}
	template <>
	TypeDescriptor* getPrimitiveDescriptor<VkDeviceMemory>() {
		static TypeDescriptor_U64 typeDesc;
		return &typeDesc;
	}
	template <>
	TypeDescriptor* getPrimitiveDescriptor<VkBuffer>() {
		static TypeDescriptor_U64 typeDesc;
		return &typeDesc;
	}
	/*
	void connect(output<vkImageArrayResource>* in, input<vkMultiImageResource>* out)
	{
		ImageArray_To_MultiImage_Module* link_module = (ImageArray_To_MultiImage_Module* )in->owner->vulkan->create_module<ImageArray_To_MultiImage_Module>();
		
		connect(in, &link_module->images_in);
		connect(&link_module->images_out,out);
	}

	void connect(output<vkMultiImageResource>* in, input<vkImageArrayResource>* out)
	{
		MultiImage_To_ImageArray_Module* link_module = (MultiImage_To_ImageArray_Module*)in->owner->vulkan->create_module<MultiImage_To_ImageArray_Module>();
	
		connect(in, &link_module->images_in);
		connect(&link_module->images_out, out);
	}*/

}

Vulkan_App::Vulkan_App(Geometry_Assets* geo_assets, Lightmap_Configuration* configuration, video::IVideoDriver* driver)
: driver{ driver }, configuration{ configuration }, geo_assets{ geo_assets }
{
	initVulkan();
}

void Vulkan_App::initVulkan()
{
	m_device = new MyDevice();

	vk_device = m_device->getDevice();

	createDescriptorPool();
	createCommandBuffers();

	geo_module = (Geometry_Module*)create_module<Geometry_Module>();

}

void Vulkan_App::cleanup()
{
	m_DescriptorPool->cleanup();
	m_device->cleanup();

	for (Vulkan_Module* module : all_modules)
	{
		//delete module;
	}
}

Vulkan_Module::Vulkan_Module()
	: vulkan{ vulkan }
{
	m_uid = random_number();
}

void Vulkan_Module::initialize(Vulkan_App* vulkan)
{
	this->vulkan = vulkan;
	vulkan->all_modules.push_back(this);
	m_device = vulkan->m_device;
	m_DescriptorPool = vulkan->m_DescriptorPool;
}

bool Vulkan_Module::all_resources_ready()
{
	reflect::TypeDescriptor_Struct* tD = GetDynamicReflection();

	for (reflect::Member& m : tD->members)
	{
		reflect::TypeDescriptor_Struct* m_tD = (reflect::TypeDescriptor_Struct*)m.type;

		if (m_tD->inherited_type == &reflect::input_type::Reflection)
		{
			reflect::input_type* in = (reflect::input_type*)m.get(this);
			if (in->ready == false)
				return false;
		}
	}
	return true;
}

//Runs the module and records its wall time and the compute dispatches it issued
void Vulkan_Module::timed_run()
{
	DispatchStats before = m_device->getDispatchStats();
	texels_shaded = 0;
	rays_traced = 0;
	auto t0 = std::chrono::high_resolution_clock::now();

	run();

	auto t1 = std::chrono::high_resolution_clock::now();
	const DispatchStats& after = m_device->getDispatchStats();

	Module_Run_Stats stats;
	stats.name = GetDynamicReflection()->name;
	stats.wall_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
	stats.dispatches.n_dispatches = after.n_dispatches - before.n_dispatches;
	stats.dispatches.n_workgroups = after.n_workgroups - before.n_workgroups;
	stats.dispatches.gpu_ms = after.gpu_ms - before.gpu_ms;
	stats.texels_shaded = texels_shaded;
	stats.rays_traced = rays_traced;

	vulkan->run_stats.push_back(stats);
}

bool Vulkan_Module::signaled()
{
	if (my_status != VK_MODULE_NOT_RAN)
		return false;

	if(all_resources_ready())
	{
		reflect::TypeDescriptor_Struct* tD = GetDynamicReflection();

		my_status = VK_MODULE_RAN;

		cout << tD->name << ": running \n";

		timed_run();

		for (reflect::Member& m : tD->members)
		{
			reflect::TypeDescriptor_Struct* m_tD = (reflect::TypeDescriptor_Struct*)m.type;

			if (m_tD->inherited_type == &reflect::input_type::Reflection)
			{
				reflect::input_type* in = (reflect::input_type*)m.get(this);
				in->src_output->consume(in);
			}
			else if (m_tD->inherited_type == &reflect::output_type::Reflection)
			{
				reflect::output_type* out = (reflect::output_type*)m.get(this);
				if (out->ready)
				{
					out->signal();
				}
			}
		}
		return true;
	}
	return false;
}

void Vulkan_Module::run_and_push()
{
	if (my_status != VK_MODULE_NOT_RAN)
		return;

	if (all_resources_ready())
	{
		reflect::TypeDescriptor_Struct* tD = GetDynamicReflection();

		my_status = VK_MODULE_RAN;

		cout << tD->name << ": running \n";

		timed_run();

		for (reflect::Member& m : tD->members)
		{
			reflect::TypeDescriptor_Struct* m_tD = (reflect::TypeDescriptor_Struct*)m.type;

			if (m_tD->inherited_type == &reflect::output_type::Reflection)
			{
				reflect::output_type* out = (reflect::output_type*)m.get(this);
				if (out->ready)
				{
					out->push();
				}
			}
		}
	}
}

void Vulkan_Module::set_ptrs()
{
	reflect::TypeDescriptor_Struct* tD = GetDynamicReflection();

	reflect::TypeDescriptor* input_tD = reflect::TypeResolver<reflect::input_type>::get();
	reflect::TypeDescriptor* output_tD = reflect::TypeResolver<reflect::output_type>::get();

	for (reflect::Member& m : tD->members)
	{
		reflect::TypeDescriptor_Struct* m_tD = (reflect::TypeDescriptor_Struct*)m.type;

		if(m_tD->inherited_type == input_tD)
		{
			reflect::input_type* in = (reflect::input_type*)m.get(this);
			in->owner = this;

			if (m.forward_output != 0xFF)
			{
				reflect::Member& forward_m = tD->members[m.forward_output];
				in->forward_output = (reflect::output_type*)forward_m.get(this);
			}
		}
		else if (m_tD->inherited_type == output_tD)
		{
			reflect::output_type* out = (reflect::output_type*)m.get(this);
			out->owner = this;
		}
	}
}

reflect::input_type* Vulkan_Module::get_input_by_name(std::string str)
{
	reflect::TypeDescriptor_Struct* tD = GetDynamicReflection();

	//tD->dump((void*)this, 0);

	reflect::TypeDescriptor* input_tD = reflect::TypeResolver<reflect::input_type>::get();
	reflect::TypeDescriptor* output_tD = reflect::TypeResolver<reflect::output_type>::get();
	
	for (reflect::Member& m : tD->members)
	{
		reflect::TypeDescriptor_Struct* m_tD = (reflect::TypeDescriptor_Struct*)m.type;

		if (strcmp(m.name, str.c_str()) == 0)
		{
			return (reflect::input_type*)m.get(this);
		}
		/*
		if (m_tD->inherited_type == input_tD)
		{
			reflect::input_type* in = (reflect::input_type*)m.get(this);
			in->owner = this;

			if (m.forward_output != 0xFF)
			{
				reflect::Member& forward_m = tD->members[m.forward_output];
				in->forward_output = (reflect::output_type*)forward_m.get(this);
			}
		}
		else if (m_tD->inherited_type == output_tD)
		{
			reflect::output_type* out = (reflect::output_type*)m.get(this);
			out->owner = this;
		}*/
	}
	return NULL;
}

void Vulkan_Module::createComputePipeline(const char* shader_path)
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &(descriptorSetLayout->getDescriptorSetLayout());

	if (vkCreatePipelineLayout(m_device->getDevice(), &pipelineLayoutInfo, nullptr,
		&pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create compute pipeline layout!");
	}

	pipeline = new ComputePipeline(m_device, shader_path, pipelineLayout);
}

//==================================================
// Vulkan App
//

vkImageArrayResource* Vulkan_App::create_imageArray(int n_layers, int width, int height, VkImageUsageFlags flags, reflect::output_type* output_binding)
{
	vkImageArrayResource* imgArray = new vkImageArrayResource(output_binding);

	m_device->createImageArray(n_layers, width, height, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, imgArray->Image,
		imgArray->ImageMemory);

	imgArray->ImageView = m_device->createImageArrayView(n_layers, imgArray->Image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

	m_device->transitionImageArrayLayout(n_layers, imgArray->Image, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	imgArray->n_images = n_layers;
	imgArray->initializeDescriptorInfo();
	imgArray->status = RESOURCE_VALID;

	resources.push_back(imgArray);

	return imgArray;
}

vkMultiImageResource* Vulkan_App::create_multiImage(int n_layers, int width, int height, VkImageUsageFlags flags, reflect::output_type* output_binding)
{
	vkMultiImageResource* img = new vkMultiImageResource(output_binding);

	img->Images.resize(n_layers);

	for (int i = 0; i < n_layers; i++)
	{
		m_device->createImage(width, height, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			flags,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, img->Images[i].Image,
			img->Images[i].ImageMemory);

		img->Images[i].ImageView = m_device->createImageView(img->Images[i].Image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	img->status = RESOURCE_VALID;

	resources.push_back(img);

	return img;
}

vkImageResource* Vulkan_App::create_image(int width, int height, VkImageUsageFlags flags, reflect::output_type* output_binding)
{
	vkImageResource* img = new vkImageResource(output_binding);

	m_device->createImage(width, height, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		flags,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, img->Image,
		img->ImageMemory);

	img->status = RESOURCE_VALID;

	resources.push_back(img);

	return img;
}

vkBufferResource* Vulkan_App::create_buffer(VkDeviceSize bufferSize, VkBufferUsageFlags flags, reflect::output_type* output_binding)
{
	vkBufferResource* buffer = new vkBufferResource(output_binding);
	/* VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT*/

	m_device->createBuffer(bufferSize, flags,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer->Buffer,
		buffer->BufferMemory);

	buffer->status = RESOURCE_VALID;

	resources.push_back(buffer);

	return buffer;
}

void Vulkan_App::createCommandBuffers() {
	commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_device->getCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if (vkAllocateCommandBuffers(m_device->getDevice(), &allocInfo, commandBuffers.data()) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

void Vulkan_App::createDescriptorPool() {

	std::vector<VkDescriptorPoolSize> poolSizes{};
	poolSizes.resize(4);

	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = 1;

	poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[1].descriptorCount = 6;

	poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	poolSizes[2].descriptorCount = 2;

	poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[3].descriptorCount = 1;

	m_DescriptorPool = new MyDescriptorPool(m_device, 6, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, poolSizes);
}


void Vulkan_App::status()
{
	for (Vulkan_Module* vk : all_modules)
	{
		if (vk->my_status == VK_MODULE_NOT_RAN)
		{
			cout << vk->GetDynamicReflection()->name << " did not run\n";
			
			reflect::TypeDescriptor_Struct* tD = vk->GetDynamicReflection();

			for (reflect::Member& m : tD->members)
			{
				reflect::TypeDescriptor_Struct* m_tD = (reflect::TypeDescriptor_Struct*)m.type;

				if (m_tD->inherited_type == &reflect::input_type::Reflection)
				{
					reflect::input_type* in = (reflect::input_type*)m.get(vk);
					if (!in->ready)
					{
						cout << "  " << m.name << " is not ready\n";
					}
				}
			}
		}
	}
}

void Vulkan_App::run_workflow()
{
	for (Vulkan_Module* vk : all_modules)
	{
		reflect::TypeDescriptor_Struct* tD = vk->GetDynamicReflection();

		for (reflect::Member& m : tD->members)
		{
			reflect::TypeDescriptor_Struct* m_tD = (reflect::TypeDescriptor_Struct*)m.type;

			//only inputs have a connection status; other members aren't input_type
			if (m_tD->inherited_type != &reflect::input_type::Reflection)
				continue;

			reflect::input_type* in = (reflect::input_type*)m.get(vk);
			if (in->status == reflect::INPUT_NOT_CONNECTED)
			{
				std::cout << tD->name << " disabled\n";
				vk->enabled = false;
			}
		}
	}

	run_stats.clear();
	m_device->resetDispatchStats();
	auto t0 = std::chrono::high_resolution_clock::now();

	geo_module->run_and_push();

	bool still_running = true;
	while (still_running)
	{
		still_running = false;
		for (Vulkan_Module* vk : all_modules)
		{
			if (vk->enabled && vk->my_status == VK_MODULE_NOT_RAN)
			{
				if(vk->signaled() == true)
					still_running = true;
			}
		}
	}

	for (Vulkan_Module* vk : all_modules)
	{
		reflect::TypeDescriptor_Struct* tD = vk->GetDynamicReflection();

		if (vk->my_status == VK_MODULE_NOT_RAN)
		{
			//std::cout << tD->name << " did not run\n";
		}
	}
	bool UnusedResources = false;
	for (vkMemoryResource* res : resources)
	{
		if (res->status != RESOURCE_DESTROYED)
		{	
			if (!UnusedResources)
			{
				UnusedResources = true;
				cout << "Resources not consumed:\n";
			}
			reflect::TypeDescriptor_Struct* res_tD = res->GetDynamicReflection();
			reflect::TypeDescriptor_Struct* res_owner_tD = res->owner->owner->GetDynamicReflection();
			cout <<"  "<< res_owner_tD->name << "::" << res_tD->name << "\n";

			for (reflect::input_type* in : res->consumers)
			{
				reflect::TypeDescriptor_Struct* tD = in->owner->GetDynamicReflection();
				cout << "    -> " << tD->name << "\n";
			}
			res->status = RESOURCE_DESTROYED;
			res->destroy(m_device->getDevice());
		}
	}
	if (!UnusedResources)
	{
		cout << "All resources consumed\n";
	}
	cout << "Finished running workflow\n";

	auto t1 = std::chrono::high_resolution_clock::now();
	print_run_stats(std::chrono::duration<double, std::milli>(t1 - t0).count());
}

void Vulkan_App::print_run_stats(double total_ms)
{
	const DispatchStats& total = m_device->getDispatchStats();

	cout << "\n===== Lightmap workflow performance =====\n";
	cout << "Triangles:   " << geo_assets->master_triangle_list.size() << "\n";
	cout << "BVH nodes:   " << geo_assets->bvh.nodes.size() << "\n";
	cout << "Area lights: " << geo_assets->area_lights.size()
		<< " (" << geo_assets->triangle_lists[GPU_TRIS_AREA_LIGHT].count() << " light triangles)\n";
	cout << "Sky:         " << geo_assets->triangle_lists[GPU_TRIS_SKY].count() << " triangles\n\n";

	u64 total_texels = 0;
	u64 total_rays = 0;
	double raytrace_gpu_ms = 0;

	cout << std::fixed << std::setprecision(2);
	cout << std::left << std::setw(32) << "Module"
		<< std::right << std::setw(11) << "Dispatches" << std::setw(12) << "Workgroups"
		<< std::setw(12) << "Texels" << std::setw(15) << "Rays" << std::setw(11) << "Rays/texel"
		<< std::setw(11) << "GPU ms" << std::setw(11) << "Wall ms" << "\n";

	for (const Module_Run_Stats& s : run_stats)
	{
		cout << std::left << std::setw(32) << s.name
			<< std::right << std::setw(11) << s.dispatches.n_dispatches << std::setw(12) << s.dispatches.n_workgroups
			<< std::setw(12) << s.texels_shaded << std::setw(15) << s.rays_traced
			<< std::setw(11) << (s.texels_shaded ? (double)s.rays_traced / s.texels_shaded : 0.0)
			<< std::setw(11) << s.dispatches.gpu_ms << std::setw(11) << s.wall_ms << "\n";

		total_texels += s.texels_shaded;
		total_rays += s.rays_traced;
		if (s.rays_traced > 0)
			raytrace_gpu_ms += s.dispatches.gpu_ms;
	}

	cout << std::left << std::setw(32) << "TOTAL"
		<< std::right << std::setw(11) << total.n_dispatches << std::setw(12) << total.n_workgroups
		<< std::setw(12) << total_texels << std::setw(15) << total_rays << std::setw(11) << ""
		<< std::setw(11) << total.gpu_ms << std::setw(11) << total_ms << "\n";

	if (raytrace_gpu_ms > 0)
		cout << "\nRaytracing throughput: " << (total_rays / (raytrace_gpu_ms * 1e-3)) / 1e6 << " Mrays/s (GPU time)\n";

	cout << "Texel and ray counts are CPU estimates: texels = texel visits per triangle (shared edge texels count once per triangle),\n"
		<< "rays = all 256 threads per texel, including samples rejected as off-triangle\n";

	if (!m_device->timestampsSupported())
		cout << "(GPU timestamps unavailable, GPU ms not measured)\n";

	cout << "=========================================\n\n";
	cout << std::defaultfloat;
}

Geometry_Assets::Geometry_Assets(geometry_scene* g_scene, Lightmap_Configuration* config)
{
	MeshNode_Interface_Final* meshnode = &g_scene->geoNode()->final_meshnode_interface;
	SMesh* mesh = g_scene->geoNode()->final_meshnode_interface.getMesh();

	fill_vertex_struct(mesh, vertices_soa);
	fill_index_struct_with_offsets(mesh, indices_soa);

	surface_index = g_scene->geoNode()->final_meshnode_interface.surface_index;

	master_triangle_list.resize(indices_soa.data.size() / 3);
	for (int i = 0; i < master_triangle_list.size(); i++)
	{
		master_triangle_list[i].set((u16)(indices_soa.data[i * 3].x), (u16)(indices_soa.data[(i * 3) + 1].x), (u16)(indices_soa.data[(i * 3) + 2].x));
	}

	//==========================================================
	// Store the material type of each triangle
	//
	struct triangle_gpu_info_struct
	{
		u16 material_type = 0;
		u16 lightmap_no = 0;
		u16 gpu_list = GPU_TRIS_NONE_PACKED;
	};

	vector<triangle_gpu_info_struct> triangle_gpu_info;
	triangle_gpu_info.resize(indices_soa.data.size() / 3);

	//vector<u16> triangle_material_type;
	//triangle_material_type.resize(indices_soa.data.size() / 3);

	Material_Groups_Base* mg_base = Material_Groups_Tool::get_base();

	auto gpu_list_of = [&](int material_group) -> int {
		if (material_group < 0 || material_group >= mg_base->material_groups.size())
			return GPU_TRIS_NONE;
		return mg_base->material_groups[material_group].gpu_triangle_list;
	};

	for (const TextureMaterial& tm : config->get_materials())
	{
		int gpu_list = gpu_list_of(tm.materialGroup);
		u16 gpu_list_packed = (gpu_list == GPU_TRIS_NONE) ? GPU_TRIS_NONE_PACKED : static_cast<u16>(gpu_list);

		//cout << "material: " << tm.materialGroup << "\n";
		for (int f_i : tm.faces)
		{
			int b_offset = indices_soa.offset[meshnode->get_buffer_index(f_i)] / 3;
			int t_0 = meshnode->get_first_triangle(f_i) / 3;
			//cout << "  face: " << f_i << ", index: " << b_offset + t_0 << "\n";
			for (int i = 0; i < meshnode->get_n_triangles(f_i); i++)
			{
				//triangle_material_type[b_offset + t_0 + i] = tm.materialGroup;
				triangle_gpu_info[b_offset + t_0 + i].material_type = tm.materialGroup;
				triangle_gpu_info[b_offset + t_0 + i].lightmap_no = tm.lightmap_no;
				triangle_gpu_info[b_offset + t_0 + i].gpu_list = gpu_list_packed;
			}
		}
	}

	//==========================================================
	// Selected triangle, for the single triangle debug paths
	//
	// The final mesh buffer index doubles as the material index (indices_soa.offset[i]
	// and config->get_materials()[i] line up). The click's triangle_n / bary_coords come
	// from the edit mesh, so the triangle is picked again here in the final mesh, from
	// the click position.
	//
	//selected face may have no final geometry (eg No Geometry material group)
	if (g_scene->getSelectedFaces().size() > 0 &&
		meshnode->get_buffer_index_by_face(g_scene->getSelectedFaces()[0]) != -1)
	{
		int f_i = g_scene->getSelectedFaces()[0];
		int buffer_index = meshnode->get_buffer_index_by_face(f_i);

		MeshBuffer_Chunk chunk = meshnode->get_mesh_buffer_by_face(f_i);
		vector3df p = g_scene->get_last_click().hitvec;

		f32 best_score = -FLT_MAX;

		for (int i = (int)chunk.begin_i; i + 2 < (int)chunk.end_i; i += 3)
		{
			int i_0 = indices_soa.offset[buffer_index] + i;

			vector3df a = vertices_soa.data0[indices_soa.data[i_0].x].V;
			vector3df b = vertices_soa.data0[indices_soa.data[i_0 + 1].x].V;
			vector3df c = vertices_soa.data0[indices_soa.data[i_0 + 2].x].V;

			//barycentrics of p projected onto the triangle's plane
			vector3df v0 = b - a, v1 = c - a, v2 = p - a;
			f64 d00 = v0.dotProduct(v0), d01 = v0.dotProduct(v1), d11 = v1.dotProduct(v1);
			f64 d20 = v2.dotProduct(v0), d21 = v2.dotProduct(v1);
			f64 denom = d00 * d11 - d01 * d01;

			if (fabs(denom) < 1e-12)
				continue;

			f32 v = (f32)((d11 * d20 - d01 * d21) / denom);
			f32 w = (f32)((d00 * d21 - d01 * d20) / denom);
			f32 u = 1.0f - v - w;

			//the most inside triangle wins (min bary is >= 0 for the one containing p)
			f32 score = std::min(u, std::min(v, w));
			if (score > best_score)
			{
				best_score = score;
				selected_triangle_index = i_0;
				selected_triangle_bary_coords = vector3df(u, v, w);
			}
		}

		if (best_score > -FLT_MAX)
		{
			selected_triangle_mg = buffer_index;
			has_selected_triangle = true;
		}
	}

	bvh.build(vertices_soa.data0.data(), master_triangle_list.data(), master_triangle_list.size(), NULL);

	for (int i = 0; i < bvh.node_count; i++)
	{
		if (bvh.nodes[i].left_node == 0xFFFF && bvh.nodes[i].right_node == 0xFFFF)
		{
			if (bvh.nodes[i].n_prims > 2)
				std::cout << "WARNING: node " << i << " has " << bvh.nodes[i].n_prims << " triangles, max 2\n";

			for (int j = 0; j < std::min(2u, bvh.nodes[i].n_prims); j++)
			{
				u32 prim_no = static_cast<f32> (bvh.indices[bvh.nodes[i].first_prim + j]);

				//Pack data about each triangle into the packing space
				//	packing[0] (padding.x): prim 0 triangle no | lightmap no << 16
				//	packing[1] (padding.y): prim 0 material group | gpu triangle list << 16
				//	packing[2] (padding.z): prim 1 triangle no | lightmap no << 16
				//	packing[3] (padding.w): prim 1 material group | gpu triangle list << 16
				//gpu triangle list is GPU_TRIS_NONE_PACKED (0xFFFF) for triangles not in any list

				const triangle_gpu_info_struct& info = triangle_gpu_info[prim_no];

				if (j == 0)
				{
					bvh.nodes[i].pack(0, prim_no);
					bvh.nodes[i].pack(1, info.lightmap_no);
					bvh.nodes[i].pack(2, info.material_type);
					bvh.nodes[i].pack(3, info.gpu_list);
				}
				else if (j == 1)
				{
					bvh.nodes[i].pack(4, prim_no);
					bvh.nodes[i].pack(5, info.lightmap_no);
					bvh.nodes[i].pack(6, info.material_type);
					bvh.nodes[i].pack(7, info.gpu_list);
				}
			}
		}
		else
			bvh.nodes[i].n_prims = 0;
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

			u16 v_1 = master_triangle_list[i].v_i[(j + 1) % 3];
			edge.v1 = vertices_soa.data0.operator[](v_1).V;

			vector3df N = master_triangle_list[i].normal(vertices_soa.data0);

			vector3df edgeTan = vector3df(edge.v1 - edge.v0).crossProduct(N);

			bvh.intersect(edge, hits);
			bool ok = false;

			for (u16 k : hits)
			{
				if (k != i && master_triangle_list[k].find_edge(v_1, v_0, vertices_soa.data0))
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
				triangle_edges[i * 3 + j].x = 0xFFFF;
				std::cout << "warning: triangle missing adjacent \n";
			}
		}
	}

	//==========================================================
	// GPU triangle lists
	// Every material group with a gpu_triangle_list channel sends its triangles
	// to that channel as [triangle_idx, payload] pairs (see gpu_triangle_lists.h)
	//

	auto triangle_area = [&](u32 tri) -> f32 {
		vector3df v0 = vertices_soa.data0[indices_soa.data[tri * 3].x].V;
		vector3df v1 = vertices_soa.data0[indices_soa.data[tri * 3 + 1].x].V;
		vector3df v2 = vertices_soa.data0[indices_soa.data[tri * 3 + 2].x].V;
		return 0.5f * (v1 - v0).crossProduct(v2 - v0).getLength();
	};

	//area lights carry per-light settings on their meshbuffer nodes, so collect those first
	reflect::TypeDescriptor_Struct* AreaLightNode_tD =
		(reflect::TypeDescriptor_Struct*)(reflect::TypeResolver<Reflected_MeshBuffer_AreaLight_SceneNode>::get());

	vector<Reflected_MeshBuffer_AreaLight_SceneNode*> light_source_nodes;

	core::list<ISceneNode*> nodes = g_scene->EditorNodes()->getChildren();
	core::list<scene::ISceneNode*>::Iterator it = nodes.begin();
	for (; it != nodes.end(); ++it)
	{
		Reflected_SceneNode* node = (Reflected_SceneNode*)*it;
		if (node->GetDynamicReflection()->isOfType(AreaLightNode_tD))
		{
			Reflected_MeshBuffer_AreaLight_SceneNode* light_node = dynamic_cast<Reflected_MeshBuffer_AreaLight_SceneNode*>(node);
			AreaLightInfoStruct light_info;

			light_info.element_id = light_node->get_element_id();
			light_info.face_id = light_node->get_face_id();
			light_info.intensity = light_node->intensity;

			area_lights.push_back(light_info);
			light_source_nodes.push_back(light_node);
		}
		else if (Reflected_SunlightNode* sun_node = dynamic_cast<Reflected_SunlightNode*>(node))
		{
			if (has_sun_node)
				std::cout << "warning: more than one Sunlight node in the scene, using the first\n";
			else
			{
				has_sun_node = true;
				sun_direction = sun_node->get_sun_direction();

				if (sun_direction.Y <= 0.01f)
					std::cout << "warning: Sunlight node's arrow isn't pointing downward (the sun is at or below the horizon),"
					             " floors will be unlit. The arrow is the direction the light travels.\n";
			}
		}
	}

	for (const TextureMaterial& tm : config->get_materials())
	{
		int channel = gpu_list_of(tm.materialGroup);

		if (channel == GPU_TRIS_NONE)
			continue;

		for (const pair<int, int>& surf : tm.surfaces)
		{
			int element_n = g_scene->geoNode()->element_by_element_id[surf.first];
			u32 offset = meshnode->surface_index.offset[element_n] + surf.second;

			u32 first_tri = meshnode->surface_index.data[offset].begin_i / 3;
			u32 n_tris = (meshnode->surface_index.data[offset].end_i - meshnode->surface_index.data[offset].begin_i) / 3;

			switch (channel)
			{
			case GPU_TRIS_AREA_LIGHT:
			{
				//grouped per light below, so the buffer keeps each light's triangles together
				for (AreaLightInfoStruct& light : area_lights)
				{
					if (surf.first == light.element_id && surf.second == light.face_id)
					{
						light.surface_normal = g_scene->geoNode()->get_element_by_id(light.element_id)->brush.faces[light.face_id].m_normal;

						for (u32 i = 0; i < n_tris; i++)
							light.indices.push_back(first_tri + i);
					}
				}
			} break;
			case GPU_TRIS_SKY:
			{
				for (u32 i = 0; i < n_tris; i++)
					triangle_lists[GPU_TRIS_SKY].push(first_tri + i, static_cast<u32>(triangle_area(first_tri + i)));
			} break;
			default:
				cout << "warning: no payload defined for gpu triangle list " << channel << "\n";
				break;
			}
		}
	}

	//area light payload: intensity * area
	for (int i = 0; i < area_lights.size(); i++)
	{
		const AreaLightInfoStruct& a = area_lights[i];

		u32 power = 0;
		for (u32 idx : a.indices)
		{
			u32 intensity = a.intensity * triangle_area(idx);

			power += intensity;

			triangle_lists[GPU_TRIS_AREA_LIGHT].push(idx, intensity);
		}

		light_source_nodes[i]->power = power;
		light_source_nodes[i]->normal = area_lights[i].surface_normal;
	}

	g_scene->calc_lighting_vectors();
}

bool Triangle_Transformer::get_uvs_for_triangle(int triangle_no, int lm_size, vector3df& w0, vector3df& w1, vector3df& w2)
{
	int i_0 = triangle_no;

	const vector<aligned_vec3>& map_uvs = vertices_soa.data1;

	int v_0, v_1, v_2;

	if (map_uvs[indexed(i_0)].V.Y < map_uvs[indexed(i_0, 1)].V.Y)
		v_0 = 0;
	else
		v_0 = 1;

	if (map_uvs[indexed(i_0, v_0)].V.Y < map_uvs[indexed(i_0, 2)].V.Y)
		v_0 = v_0;
	else
		v_0 = 2;

	if (map_uvs[indexed(i_0, ((v_0 + 1) % 3))].V.Y > map_uvs[indexed(i_0, ((v_0 + 2) % 3))].V.Y)
	{
		v_1 = (v_0 + 1) % 3;
		v_2 = (v_0 + 2) % 3;
	}
	else
	{
		v_1 = (v_0 + 2) % 3;
		v_2 = (v_0 + 1) % 3;
	}

	v_0 = indexed(i_0, v_0);
	v_1 = indexed(i_0, v_1);
	v_2 = indexed(i_0, v_2);

	w0 = vector3df(floor_x_value(map_uvs[v_0].V.X, lm_size), floor_y_value(map_uvs[v_0].V.Y, lm_size), 0);
	w1 = vector3df(floor_x_value(map_uvs[v_1].V.X, lm_size), floor_y_value(map_uvs[v_1].V.Y, lm_size), 0);
	w2 = vector3df(floor_x_value(map_uvs[v_2].V.X, lm_size), floor_y_value(map_uvs[v_2].V.Y, lm_size), 0);

	return true;
}

u32 Triangle_Transformer::count_texels(u32 index_offset, u32 lm_size)
{
	//Mirrors main() in shader_arealight.comp / shader_bounced_light.comp, using the same float math
	const vector<aligned_vec3>& map_uvs = vertices_soa.data1;

	vector3df w0, w1, w2;
	get_uvs_for_triangle(index_offset, lm_size, w0, w1, w2);

	//get_uvs_for_triangle sorts the vertices but doesn't return the indices; the raw uvs are needed for b0/b1
	f32 y_raw[3] = { map_uvs[indexed(index_offset)].V.Y, map_uvs[indexed(index_offset, 1)].V.Y, map_uvs[indexed(index_offset, 2)].V.Y };
	std::sort(y_raw, y_raw + 3);

	f32 W = (f32)lm_size;
	f32 H = (f32)lm_size;

	//Degenerate in v: the shader divides by zero here, treat it as doing no work
	if (w1.Y - w0.Y == 0)
		return 0;

	f32 m0 = (w1.X - w0.X) / (w1.Y - w0.Y);
	f32 m1 = (w2.X - w0.X) / (w2.Y - w0.Y);
	f32 m2 = (w1.X - w2.X) / (w1.Y - w2.Y);

	bool b0 = y_raw[1] > y_raw[0] + 0.0001f;
	bool b1 = y_raw[2] > y_raw[1] + 0.0001f;

	auto MajorX = [&](f32 y) { return w0.X + (m0 * (y - w0.Y)); };
	bool bNegativeX = MajorX(w2.Y) > w2.X;

	auto MinorX = [&](f32 y) -> f32 {
		if (b0 && b1)
		{
			if (bNegativeX)
				return std::max(w0.X + (m1 * (y - w0.Y)), w2.X + (m2 * (y - w2.Y)));
			else
				return std::min(w0.X + (m1 * (y - w0.Y)), w2.X + (m2 * (y - w2.Y)));
		}
		else if (b0)
			return w0.X + (m1 * (y - w0.Y));
		else if (b1)
			return w2.X + (m2 * (y - w2.Y));
		return 0;
		};

	auto to_uint = [](f32 x) -> s64 { return x < 0 ? 0 : (s64)x; };

	u32 count = 0;
	f32 ystep = 1.0f / H;

	for (f32 y0 = w0.Y; y0 < w1.Y + 0.001f; y0 += ystep)
	{
		s64 x0, x1;
		if (!bNegativeX)
		{
			x0 = to_uint(W * MajorX(y0));
			x1 = to_uint(W * MinorX(y0));
		}
		else
		{
			x0 = to_uint(W * MinorX(y0));
			x1 = to_uint(W * MajorX(y0));
		}

		if (x1 >= x0)
			count += (u32)(x1 - x0 + 1);
	}

	return count;
}

void vkImageSubresource::destroy(VkDevice device)
{
	vkDestroyImageView(device, ImageView, nullptr);
	vkDestroyImage(device, Image, nullptr);
	vkFreeMemory(device, ImageMemory, nullptr);
}

void vkImageResource::destroy(VkDevice device)
{
	vkDestroyImageView(device, ImageView, nullptr);
	vkDestroyImage(device, Image, nullptr);
	vkFreeMemory(device, ImageMemory, nullptr);
}

void vkImageResource::create_and_load_texture(MyDevice* device, video::IVideoDriver* driver, video::ITexture* tex)
{
	VkDeviceSize width = tex->getOriginalSize().Width;
	VkDeviceSize height = tex->getOriginalSize().Height;
	VkDeviceSize imgSize = width * height * 4;

	MyBufferObject stagingBuffer(device, imgSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	irr::video::IImage* pImage = driver->createImage(tex, core::vector2di(0, 0), tex->getOriginalSize());
	pImage->flip(true, false);

	irr::u8* imgDataPtr = (irr::u8*)pImage->lock();

	stagingBuffer.writeToBuffer(imgDataPtr);

	pImage->unlock();
	pImage->drop();

	device->createImage(width, height, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Image,
		ImageMemory);

	device->transitionImageLayout(Image, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	ImageView = device->createImageView(Image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

	device->copyBufferToImage(stagingBuffer.getBuffer(), Image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

	device->transitionImageLayout(Image, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
}


void vkImageSubresource::create_and_load_texture(MyDevice* device, video::IVideoDriver* driver, video::ITexture* tex)
{
	VkDeviceSize width = tex->getOriginalSize().Width;
	VkDeviceSize height = tex->getOriginalSize().Height;
	VkDeviceSize imgSize = width * height * 4;

	MyBufferObject stagingBuffer(device, imgSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	irr::video::IImage* pImage = driver->createImage(tex, core::vector2di(0, 0), tex->getOriginalSize());
	pImage->flip(true, false);

	irr::u8* imgDataPtr = (irr::u8*)pImage->lock();

	stagingBuffer.writeToBuffer(imgDataPtr);

	pImage->unlock();
	pImage->drop();

	device->createImage(width, height, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Image,
		ImageMemory);

	device->transitionImageLayout(Image, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	ImageView = device->createImageView(Image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

	device->copyBufferToImage(stagingBuffer.getBuffer(), Image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

	device->transitionImageLayout(Image, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
}

VkDescriptorSetLayoutBinding vkMultiImageResource::getDescriptorSetLayout(u32 binding_no)
{
	VkDescriptorSetLayoutBinding imageStorageBinding{};
	imageStorageBinding.binding = binding_no;
	imageStorageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	imageStorageBinding.descriptorCount = 1;
	imageStorageBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	imageStorageBinding.pImmutableSamplers = nullptr; // Optional

	return imageStorageBinding;
}

VkDescriptorImageInfo vkMultiImageResource::getDescriptorBufferInfo(u32 image_no)
{
	VkDescriptorImageInfo imageStorageInfo{};
	imageStorageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageStorageInfo.imageView = Images[image_no].ImageView;

	return imageStorageInfo;
}

void vkMultiImageResource::destroy(VkDevice device)
{
	for (auto& img : Images)
		img.destroy(device);
}

VkDescriptorSetLayoutBinding vkBufferResource::getDescriptorSetLayout(u32 binding_no)
{
	VkDescriptorSetLayoutBinding Binding{};
	Binding.binding = binding_no;
	Binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	Binding.descriptorCount = 1;
	Binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	Binding.pImmutableSamplers = nullptr; // Optional

	return Binding;
}

VkDescriptorBufferInfo vkBufferResource::getDescriptorBufferInfo()
{
	VkDescriptorBufferInfo BufferInfo{};
	BufferInfo.buffer = Buffer;
	BufferInfo.offset = 0;
	BufferInfo.range = range;

	return BufferInfo;
}

void vkBufferResource::destroy(VkDevice device)
{
	vkDestroyBuffer(device, Buffer, nullptr);
	vkFreeMemory(device, BufferMemory, nullptr);
}

VkDescriptorSetLayoutBinding vkUniformBufferResource::getDescriptorSetLayout(u32 binding_no)
{
	VkDescriptorSetLayoutBinding Binding{};
	Binding.binding = binding_no;
	Binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	Binding.descriptorCount = 1;
	Binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	Binding.pImmutableSamplers = nullptr; // Optional

	return Binding;
}

VkDescriptorBufferInfo vkUniformBufferResource::getDescriptorBufferInfo()
{
	VkDescriptorBufferInfo BufferInfo{};
	BufferInfo.buffer = Buffer;
	BufferInfo.offset = 0;
	BufferInfo.range = range;

	return BufferInfo;
}

void vkUniformBufferResource::destroy(VkDevice device)
{
	vkDestroyBuffer(device, Buffer, nullptr);
	vkFreeMemory(device, BufferMemory, nullptr);
}

void vkUniformBufferResource::writeToBuffer(VkDevice device, void* data, VkDeviceSize, VkDeviceSize offset)
{
	void* mapped_data = nullptr;

	vkMapMemory(device, BufferMemory, 0, range, 0, &mapped_data);

	memcpy(mapped_data, data, range);

	vkUnmapMemory(device, BufferMemory);
}

void vkImageArrayResource::destroy(VkDevice device)
{
	vkDestroyImageView(device, ImageView, nullptr);
	vkDestroyImage(device, Image, nullptr);
	vkFreeMemory(device, ImageMemory, nullptr);
}

VkDescriptorSetLayoutBinding vkImageArrayResource::getDescriptorSetLayout(u32 binding_no)
{
	VkDescriptorSetLayoutBinding Binding{};
	Binding.binding = binding_no;
	Binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	Binding.descriptorCount = n_images;
	Binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	Binding.pImmutableSamplers = nullptr; // Optional

	return Binding;
}

VkDescriptorImageInfo* vkImageArrayResource::getDescriptorBufferInfo()
{
	return imageStorageInfo.data();
}

void vkImageArrayResource::initializeDescriptorInfo()
{

	imageStorageInfo.resize(n_images);

	for (int i = 0; i < n_images; i++)
	{
		imageStorageInfo[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageStorageInfo[i].imageView = ImageView;
	}
}

vkMemoryResource::vkMemoryResource(reflect::output_type* out)
{
	owner = out;
	find_consumers(out);
}

void vkMemoryResource::find_consumers(reflect::output_type* out)
{
	if (out == NULL)
		return;

	for (reflect::input_type* dest : out->dest_inputs)
	{
		if (dest->owner->enabled == false)
			continue;

		consumers.push_back(dest);

		if (dest->forward_output != NULL)
		{
			find_consumers(dest->forward_output);
		}
	}
}

void vkMemoryResource::consume(reflect::input_type* in)
{
	vector<reflect::input_type*> new_consumers;
	for (reflect::input_type* it : consumers)
	{
		if (it != in)
			new_consumers.push_back(it);
	}

	if (new_consumers.size() > 0)
	{
		consumers = new_consumers;
	}
	else
	{
		consumers.clear();
		//reflect::TypeDescriptor_Struct* owner_tD = (reflect::TypeDescriptor_Struct*)owner->owner->GetDynamicReflection();
		//reflect::TypeDescriptor_Struct* m_tD = (reflect::TypeDescriptor_Struct*)GetDynamicReflection();
		//cout << "  " << owner_tD->name << "::" << m_tD->name << "  destroyed\n";
		status = RESOURCE_DESTROYED;
		destroy(vk_device);
	}
}
namespace reflect
{
	template <>
	TypeDescriptor* getPrimitiveDescriptor<VkMod_Reference>() {
		static TypeDescriptor_VkMod_Reference typeDesc;
		return &typeDesc;
	}
}


namespace reflect
{
	void TypeDescriptor_VkWorkflow_Struct::serialize(std::ofstream& f, const void* obj)
	{
		TypeDescriptor_Struct::serialize(f, obj);
	}

	void TypeDescriptor_VkWorkflow_Struct::deserialize(std::ifstream& f, void* obj)
	{
		TypeDescriptor_Struct::deserialize(f, obj);

		Vulkan_Workflow* workflow = (Vulkan_Workflow*)obj;

		//Set input/output pointers
		for (Vulkan_Module* mod : workflow->items)
		{
			reflect::TypeDescriptor_Struct* tD = mod->GetDynamicReflection();

			//tD->dump((void*)mod,0);

			for (reflect::Member& m : tD->members)
			{
				reflect::TypeDescriptor_Struct* m_tD = (reflect::TypeDescriptor_Struct*)m.type;

				if (m_tD->inherited_type == &reflect::output_type::Reflection)
				{
					reflect::output_type* out = (reflect::output_type*)m.get(mod);
					
					for (int i=0; i<out->output_uids.size() ; i++)
					{
						Vulkan_Module* mod2 = workflow->find(out->output_uids[i]);
						if (mod2)
						{
							//reflect::input_type* in = (reflect::input_type *)mod2->get_inout_by_name(out->output_member[i]);
							//reflect::inout_gui_type* in_ = mod2->get_input_by_name(out->output_member[i]);
							reflect::input_type* in = mod2->get_input_by_name(out->output_member[i]);
							if (in)
							{
								out->dest_inputs.push_back(in);
								in->src_output = out;
								in->status = reflect::INPUT_WAITING; //as reflect::connect does; not serialized
							}
							else
							{
								cout << "could not find " << out->output_member[i] << "\n";
							}
						}
						else
						{
							cout << "could not find " << out->output_uids[i] << "\n";
						}
					}
				}
			}
		}
	}
}


//============================================================================
//  Add type to factory at initialization


namespace reflect
{
	TypeDescriptor_VkMod_Struct::TypeDescriptor_VkMod_Struct(void (*init)(TypeDescriptor_Struct*)) : reflect::TypeDescriptor_Struct(init)
	{
		Vulkan_Module::factory.addType(this);
	}
}

REFLECT_VKMOD_BEGIN(Vulkan_Module)
	REFLECT_STRUCT_MEMBER(pos);
	REFLECT_STRUCT_MEMBER(m_uid);
	REFLECT_STRUCT_MEMBER_FLAG(FLAG_HIDDEN)	//identity; ports refer to it
REFLECT_STRUCT3_END()

REFLECT_VK_WORKFLOW_BEGIN(Vulkan_Workflow)
	REFLECT_STRUCT_MEMBER(items) //tkt_owning_set::items; was "Modules" (same file format)
REFLECT_STRUCT_END()
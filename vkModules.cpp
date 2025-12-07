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
#include <vulkan/vulkan.h>
#include "reflect_custom_types.h"

using namespace irr;
using namespace core;
using namespace std;

extern IrrlichtDevice* device;
extern VkDevice vk_device = NULL;

#define PRINTV(x) << x.X <<","<<x.Y<<","<<x.Z<<" "


static std::vector<Vulkan_Module*>* all_vulkan_modules = NULL;

vector<Vulkan_Module*>* get_all_vk_modules()
{
	return all_vulkan_modules;
}

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
REFLECT_STRUCT_END()

REFLECT_CUSTOM_STRUCT_BEGIN_TEMPLATE(vkMultiImageResource, reflect::input)
	INHERIT_FROM(reflect::input_type)
	REFLECT_STRUCT_MEMBER(input_uid)
	REFLECT_STRUCT_MEMBER(input_member)
REFLECT_STRUCT_END()

REFLECT_CUSTOM_STRUCT_BEGIN_TEMPLATE(vkMultiImageResource, reflect::output)
	INHERIT_FROM(reflect::output_type)
	REFLECT_STRUCT_MEMBER(output_uids)
REFLECT_STRUCT_END()

REFLECT_CUSTOM_STRUCT_BEGIN_TEMPLATE(vkBufferResource, reflect::output)
	INHERIT_FROM(reflect::output_type)
	REFLECT_STRUCT_MEMBER(output_uids)
REFLECT_STRUCT_END()

REFLECT_CUSTOM_STRUCT_BEGIN_TEMPLATE(vkBufferResource, reflect::input)
	INHERIT_FROM(reflect::input_type)
	REFLECT_STRUCT_MEMBER(input_uid)
	REFLECT_STRUCT_MEMBER(input_member)
REFLECT_STRUCT_END()

REFLECT_STRUCT3_BEGIN(reflect::input_type)
REFLECT_STRUCT_END()

REFLECT_STRUCT3_BEGIN(reflect::output_type)
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
	}

}

Vulkan_App::Vulkan_App(Geometry_Assets* geo_assets, Lightmap_Configuration* configuration, video::IVideoDriver* driver)
: driver{ driver }, configuration{ configuration }, geo_assets{ geo_assets }
{
	initVulkan();

	all_vulkan_modules = &all_modules;
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
		delete module;
	}
}

REFLECT_STRUCT3_BEGIN(Vulkan_Module)
REFLECT_STRUCT3_END()

Vulkan_Module::Vulkan_Module(Vulkan_App* vulkan)
	: vulkan{ vulkan }
{
	m_device = vulkan->m_device;
	m_DescriptorPool = vulkan->m_DescriptorPool;
	m_uid = random_number();
	vulkan->all_modules.push_back(this);
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

bool Vulkan_Module::signaled()
{
	if (my_status != VK_MODULE_NOT_RAN)
		return false;

	if(all_resources_ready())
	{
		reflect::TypeDescriptor_Struct* tD = GetDynamicReflection();

		my_status = VK_MODULE_RAN;

		cout << tD->name << ": running \n";

		run();

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

		run();

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

Vulkan_Module* get_module_by_uid(std::vector<Vulkan_Module*>* modules, u64 uid)
{
	for (Vulkan_Module* m : *modules)
	{
		if (m->m_uid == uid)
			return m;
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
			reflect::input_type* in = (reflect::input_type*)m.get(vk);
			if (in->status == reflect::INPUT_NOT_CONNECTED)
			{
				std::cout << tD->name << " disabled\n";
				vk->enabled = false;
			}
		}
	}

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
		u16 material_type;
		u16 lightmap_no;
	};

	vector<triangle_gpu_info_struct> triangle_gpu_info;
	triangle_gpu_info.resize(indices_soa.data.size() / 3);

	//vector<u16> triangle_material_type;
	//triangle_material_type.resize(indices_soa.data.size() / 3);

	for (const TextureMaterial& tm : config->get_materials())
	{
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
			}
		}
	}

	if (g_scene->getSelectedFaces().size() > 0)
	{
		GeometryStack* geo_node = g_scene->geoNode();
		int f_i = g_scene->getSelectedFaces()[0];

		int mb_i = g_scene->geoNode()->edit_meshnode_interface.get_buffer_index_by_face(f_i);
		selected_triangle_mg = g_scene->geoNode()->edit_meshnode_interface.get_material_group_by_face(mb_i);
		
		int lm_size = config->get_materials()[selected_triangle_mg].lightmap_size;

		MeshBuffer_Chunk chunk = meshnode->get_mesh_buffer_by_face(f_i);

		int buffer_index = meshnode->get_buffer_index_by_face(f_i);
		int face_offset = indices_soa.offset[buffer_index] + chunk.begin_i;
		int tri_sel = g_scene->get_last_click().triangle_n;

		selected_triangle_index = face_offset +tri_sel * 3;
		selected_triangle_bary_coords = g_scene->get_last_click().bary_coords;
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
				
				
				/*
				bvh.nodes[i].pack(j*4 + 1, static_cast<u16> (triangle_gpu_info[prim_no].material_type));
				bvh.nodes[i].pack(j * 4 + 2, static_cast<u16> (triangle_gpu_info[prim_no].lightmap_no));
				*/
				//bvh.nodes[i].packing[3] = 5;

				if (j == 0)
				{
					bvh.nodes[i].pack(0, prim_no);
					bvh.nodes[i].pack(1, static_cast<u16> (triangle_gpu_info[prim_no].lightmap_no));
				}
				else if (j == 1)
				{
					bvh.nodes[i].pack(4, prim_no);
					bvh.nodes[i].pack(5, static_cast<u16> (triangle_gpu_info[prim_no].lightmap_no));
				}
				

				

				//cout << prim_no << ", " << static_cast<u16> (triangle_gpu_info[prim_no].lightmap_no) << "\n";
			}
		}
		else
			bvh.nodes[i].n_prims = 0;
	}

	for (int i = 0; i < bvh.node_count; i++)
	{
		if (bvh.nodes[i].left_node == 0xFFFF && bvh.nodes[i].right_node == 0xFFFF)
		{

			for (int j = 0; j < std::min(2u, bvh.nodes[i].n_prims); j++)
			{

				//for (int k = 0; k < 8; k++)
				//	bvh.nodes[i].check_packing(k, k + 1);

				//cout << prim_no << ", " << static_cast<u16> (triangle_gpu_info[prim_no].lightmap_no) << "\n";
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
	// Make a list of area lights
	//

	vector<pair<u32, u32>> intensities;

	reflect::TypeDescriptor_Struct* MeshBufferNode_tD =
		(reflect::TypeDescriptor_Struct*)(reflect::TypeResolver<Reflected_MeshBuffer_AreaLight_SceneNode>::get());

	core::list<ISceneNode*> nodes = g_scene->EditorNodes()->getChildren();
	core::list<scene::ISceneNode*>::Iterator it = nodes.begin();
	for (; it != nodes.end(); ++it)
	{
		Reflected_SceneNode* node = (Reflected_SceneNode*)*it;
		if (node->GetDynamicReflection()->isOfType(MeshBufferNode_tD))
		{
			Reflected_MeshBuffer_AreaLight_SceneNode* light_node = dynamic_cast<Reflected_MeshBuffer_AreaLight_SceneNode*>(node);
			AreaLightInfoStruct light_info;

			light_info.element_id = light_node->get_element_id();
			light_info.face_id = light_node->get_face_id();
			light_info.intensity = light_node->intensity;

			area_lights.push_back(light_info);
		}
	}

	for (const TextureMaterial& tm : config->get_materials())
	{
		if (tm.materialGroup == 7)
		{
			for (AreaLightInfoStruct& light : area_lights)
			{
				for (const pair<int, int>& face_id : tm.surfaces)
				{
					if (face_id.first == light.element_id && face_id.second == light.face_id)
					{
						AreaLightInfoStruct* light_info = &light;
						int element_n = g_scene->geoNode()->element_by_element_id[light.element_id];

						u32 offset = meshnode->surface_index.offset[element_n] + light.face_id;

						int begin_i = meshnode->surface_index.data[offset].begin_i;
						int end_i = meshnode->surface_index.data[offset].end_i;
						int len = (end_i - begin_i) / 3;
						for (int i = 0; i < len; i++)
						{
							light_info->indices.push_back(begin_i / 3 + i);
						}
					}
				}
			}
		}
	}

	for (const AreaLightInfoStruct& a : area_lights)
	{
		for (u32 idx : a.indices)
		{
			area_light_indices.push_back(aligned_uint{ idx });

			u32 i0 = indices_soa.data[idx * 3].x;
			u32 i1 = indices_soa.data[idx * 3 + 1].x;
			u32 i2 = indices_soa.data[idx * 3 + 2].x;

			vector3df v0 = vertices_soa.data0[i0].V;
			vector3df v1 = vertices_soa.data0[i1].V;
			vector3df v2 = vertices_soa.data0[i2].V;

			vector3df e0 = v1 - v0;
			vector3df e1 = v2 - v0;

			f32 area = (0.5 * e0.crossProduct(e1).getLength());

			u32 intensity = a.intensity * area;

			area_light_indices.push_back(aligned_uint{ intensity });
		}
	}
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
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
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
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
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

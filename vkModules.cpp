#include <irrlicht.h>
#include "LightMaps.h"
#include "vkModules.h"
#include "csg_classes.h"
#include "utils.h"
#include "geometry_scene.h"
#include "soa.h"
#include "vkSunlightModule.h"
#include "vkAreaLightModule.h"
#include <vulkan/vulkan.h>

using namespace irr;
using namespace core;
using namespace std;

extern IrrlichtDevice* device;

#define PRINTV(x) << x.X <<","<<x.Y<<","<<x.Z<<" "


void Vulkan_App::initVulkan() 
{
	m_device = new MyDevice();

	createDescriptorPool();
	createCommandBuffers();
	//createSyncObjects();
}

void Vulkan_App::initBuffers()
{
	createIndexBuffer();
	createVertexBuffer();
	createUVBuffer();
	createEdgeBuffer();
	createNodeBuffer();
}

void Vulkan_App::cleanup()
{
	vkDestroyBuffer(m_device->getDevice(), indexBuffer, nullptr);
	vkFreeMemory(m_device->getDevice(), indexBufferMemory, nullptr);

	vkDestroyBuffer(m_device->getDevice(), vertexBuffer, nullptr);
	vkFreeMemory(m_device->getDevice(), vertexBufferMemory, nullptr);

	vkDestroyBuffer(m_device->getDevice(), uvBuffer, nullptr);
	vkFreeMemory(m_device->getDevice(), uvBufferMemory, nullptr);

	vkDestroyBuffer(m_device->getDevice(), nodeBuffer, nullptr);
	vkFreeMemory(m_device->getDevice(), nodeBufferMemory, nullptr);

	vkDestroyBuffer(m_device->getDevice(), edgeBuffer, nullptr);
	vkFreeMemory(m_device->getDevice(), edgeBufferMemory, nullptr);

	if (outputBuffer)
	{
		vkDestroyBuffer(m_device->getDevice(), outputBuffer, nullptr);
		vkFreeMemory(m_device->getDevice(), outputBufferMemory, nullptr);
	}

	if (AreaLightSourceBuffer)
	{
		vkDestroyBuffer(m_device->getDevice(), AreaLightSourceBuffer, nullptr);
		vkFreeMemory(m_device->getDevice(), AreaLightSourceBufferMemory, nullptr);
	}

	m_DescriptorPool->cleanup();
	m_device->cleanup();
}

Vulkan_Module::Vulkan_Module(Vulkan_App* vulkan)
	: vulkan{ vulkan }
{
	m_device = vulkan->m_device;
	m_DescriptorPool = vulkan->m_DescriptorPool;
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


void Vulkan_App::createIndexBuffer() {


	VkDeviceSize bufferSize = sizeof(aligned_uint) * n_indices;

	MyBufferObject stagingBuffer(m_device, sizeof(aligned_uint), n_indices, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	stagingBuffer.writeToBuffer((void*)indices_soa->data.data());

	m_device->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer,
		indexBufferMemory);

	m_device->copyBuffer(stagingBuffer.getBuffer(), indexBuffer, bufferSize);
}


void Vulkan_App::createVertexBuffer() {

	VkDeviceSize bufferSize = sizeof(aligned_vec3) * n_vertices;

	MyBufferObject stagingBuffer(m_device, sizeof(aligned_vec3), n_vertices, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	stagingBuffer.writeToBuffer((void*)vertices_soa->data0.data());

	m_device->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer,
		vertexBufferMemory);

	m_device->copyBuffer(stagingBuffer.getBuffer(), vertexBuffer, bufferSize);
}

void Vulkan_App::init_area_light_buffer() {

	int n = area_light_indices->size();

	VkDeviceSize bufferSize = sizeof(aligned_uint) * n;

	MyBufferObject stagingBuffer(m_device, sizeof(aligned_uint), n, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	stagingBuffer.writeToBuffer((void*)area_light_indices->data());

	m_device->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, AreaLightSourceBuffer,
		AreaLightSourceBufferMemory);

	m_device->copyBuffer(stagingBuffer.getBuffer(), AreaLightSourceBuffer, bufferSize);
}

void Vulkan_App::createUVBuffer()
{
	VkDeviceSize bufferSize = sizeof(aligned_vec3) * n_vertices;

	MyBufferObject stagingBuffer(m_device, sizeof(aligned_vec3), n_vertices, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	stagingBuffer.writeToBuffer((void*)vertices_soa->data1.data());

	m_device->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uvBuffer,
		uvBufferMemory);

	m_device->copyBuffer(stagingBuffer.getBuffer(), uvBuffer, bufferSize);
}

void Vulkan_App::createEdgeBuffer()
{
	VkDeviceSize bufferSize = sizeof(aligned_uint) * n_indices;

	MyBufferObject stagingBuffer(m_device, sizeof(aligned_uint), n_indices, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	stagingBuffer.writeToBuffer((void*)triangle_edges->data());

	m_device->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, edgeBuffer,
		edgeBufferMemory);

	m_device->copyBuffer(stagingBuffer.getBuffer(), edgeBuffer, bufferSize);
}

void Vulkan_App::createNodeBuffer()
{
	VkDeviceSize bufferSize = sizeof(BVH_node_gpu) * n_nodes;

	MyBufferObject stagingBuffer(m_device, sizeof(BVH_node_gpu), n_nodes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	stagingBuffer.writeToBuffer((void*)bvh->nodes.data());

	m_device->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nodeBuffer,
		nodeBufferMemory);

	m_device->copyBuffer(stagingBuffer.getBuffer(), nodeBuffer, bufferSize);
}

void Vulkan_App::createOutputBuffer()
{
	VkDeviceSize bufferSize = sizeof(aligned_vec3) * 512;

	/*
	MyBufferObject stagingBuffer(m_device, sizeof(aligned_vec3), 512, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	stagingBuffer.writeToBuffer((void*)bvh->nodes.data());
	*/

	m_device->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, outputBuffer,
		outputBufferMemory);

	//m_device->copyBuffer(stagingBuffer.getBuffer(), nodeBuffer, bufferSize);
}

void Create_Images_Module::run()
{
	createImages();
}

void Create_Images_Module::createImages()
{
	for (int i = 0; i < configuration->lightmap_dimensions.size(); i++)
	{
		VkDeviceSize width = configuration->lightmap_dimensions[i].Width;
		VkDeviceSize height = configuration->lightmap_dimensions[i].Height;

		VkImage img;
		VkDeviceMemory imgMemory;
		VkImageView imgView;

		m_device->createImage(width, height, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, img,
			imgMemory);

		imgView = m_device->createImageView(img, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

		m_device->transitionImageLayout(img, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

		lightmapImages.push_back(img);
		lightmapsMemory.push_back(imgMemory);
		lightmapImageViews.push_back(imgView);
	}
}

void Create_Images_Module::destroyImages()
{
	for (auto& imgView : lightmapImageViews)
		vkDestroyImageView(m_device->getDevice(), imgView, nullptr);

	for (auto& img : lightmapImages)
		vkDestroyImage(m_device->getDevice(), img, nullptr);

	for (auto& imgMem : lightmapsMemory)
		vkFreeMemory(m_device->getDevice(), imgMem, nullptr);
}

void Load_Textures_Module::run()
{
	createImages();
}

void Load_Textures_Module::createImages()
{
	for (int i = 0; i < configuration->lightmap_dimensions.size(); i++)
	{
		VkDeviceSize width = configuration->lightmap_dimensions[i].Width;
		VkDeviceSize height = configuration->lightmap_dimensions[i].Height;
		VkDeviceSize imgSize = width * height * 4;

		VkImage img;
		VkDeviceMemory imgMemory;
		VkImageView imgView;

		MyBufferObject stagingBuffer(m_device, imgSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

		irr::video::IImage* pImage = driver->createImage(textures[i],core::vector2di(0,0), configuration->lightmap_dimensions[i]);
		pImage->flip(true, false);

		irr::u8* imgDataPtr = (irr::u8*)pImage->lock();

		stagingBuffer.writeToBuffer(imgDataPtr);

		pImage->unlock();
		pImage->drop();
		/*
		m_device->createImage(width, height, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, img,
			imgMemory);*/

		m_device->createImage(width, height, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, img,
			imgMemory);

		m_device->transitionImageLayout(img, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		//VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL

		imgView = m_device->createImageView(img, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

		lightmapImages.push_back(img);
		lightmapsMemory.push_back(imgMemory);
		lightmapImageViews.push_back(imgView);

		m_device->copyBufferToImage(stagingBuffer.getBuffer(), lightmapImages[i], static_cast<uint32_t>(width), static_cast<uint32_t>(height));

		m_device->transitionImageLayout(img, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	}
}

void Load_Textures_Module::destroyImages()
{
	for (auto& imgView : lightmapImageViews)
		vkDestroyImageView(m_device->getDevice(), imgView, nullptr);

	for (auto& img : lightmapImages)
		vkDestroyImage(m_device->getDevice(), img, nullptr);

	for (auto& imgMem : lightmapsMemory)
		vkFreeMemory(m_device->getDevice(), imgMem, nullptr);
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

	vector<u16> triangle_material_type;
	triangle_material_type.resize(indices_soa.data.size() / 3);

	for (const TextureMaterial& tm : config->get_materials())
	{
		//cout << "material: " << tm.materialGroup << "\n";
		for (int f_i : tm.faces)
		{
			int b_offset = indices_soa.offset[meshnode->get_buffer_index(f_i)] / 3;
			int t_0 = meshnode->get_first_triangle(f_i);
			//cout << "  face: " << f_i << ", index: " << b_offset + t_0 << "\n";
			for (int i = 0; i < meshnode->get_n_triangles(f_i); i++)
			{
				
				triangle_material_type[b_offset + t_0 + i] = tm.materialGroup;
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
				bvh.nodes[i].packing[j] = prim_no;
				bvh.nodes[i].packing[j + 2] = static_cast<f32> (triangle_material_type[prim_no]);
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

	//cout << "Area lights: ";
	for (const TextureMaterial& tm : config->get_materials())
	{
		if (tm.materialGroup == 7)
		{
			//cout << "material: " << tm.materialGroup << "\n";
			for (int f_i : tm.faces)
			{

				unsigned int b_offset = indices_soa.offset[meshnode->get_buffer_index(f_i)] / 3;
				int t_0 = meshnode->get_first_triangle(f_i);

				//area_light_indices.push_back(aligned_uint{ 59 });
				//cout << "face offset: " << b_offset + t_0 << "\n";

				for (int i = 0; i < meshnode->get_n_triangles(f_i); i++)
				{
					area_light_indices.push_back(aligned_uint{ b_offset + t_0 + i});
					//cout << b_offset + t_0 + i << " ";
				}

				//for (u32 i = 0; i < meshnode->get_n_triangles(f_i); i++)
				{
					//area_light_indices.push_back(aligned_uint{ b_offset + t_0 + i });
					//area_light_indices.push_back(aligned_uint{ b_offset + t_0 + i });
					//cout << b_offset + t_0 + i << ", ";
					//cout << b_offset + t_0 + i << ", ";
				}
			}
		}
	}
	cout << "\n" << area_light_indices.size() << " (triangles) area lights\n";
	cout << "\n";
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

void Download_Textures_Module::run()
{
	for (int i = 0; i < configuration->lightmap_dimensions.size(); i++)
	{
		VkDeviceSize width = configuration->lightmap_dimensions[i].Width;
		VkDeviceSize height = configuration->lightmap_dimensions[i].Height;
		VkDeviceSize imgSize = width * height * 4;

		MyBufferObject stagingBuffer(m_device, imgSize, 1, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

		m_device->transitionImageLayout(lightmapImages[i], VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		m_device->copyImageToBuffer(stagingBuffer.getBuffer(), lightmapImages[i], static_cast<uint32_t>(width), static_cast<uint32_t>(height));

		irr::video::IImage* pImage = driver->createImage(irr::video::ECF_A8R8G8B8, irr::core::dimension2du(width, height));

		irr::u8* imgDataPtr = (irr::u8*)pImage->lock();

		stagingBuffer.readFromBuffer(imgDataPtr);

		if(bFlip)
			pImage->flip(true, false);

		irr::video::ITexture* tex = driver->addTexture(irr::io::path("image name"), pImage);
		textures.push_back(tex);

		pImage->unlock();
		pImage->drop();

		m_device->transitionImageLayout(lightmapImages[i], VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
}

void Lightmap_Edges_Module::run()
{
	createDescriptorSetLayout();
	createComputePipeline();

	int n_maps = configuration->lightmap_dimensions.size();

	lightmapImages_out.resize(n_maps);
	lightmapImageViews_out.resize(n_maps);
	lightmapsMemory_out.resize(n_maps);

	shaderParamsBufferObject = new MyBufferObject(m_device, sizeof(ShaderParamsInfo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 16);

	for (int i = 0; i < n_maps; i++)
	{
		VkDeviceSize width = configuration->lightmap_dimensions[i].Width;
		VkDeviceSize height = configuration->lightmap_dimensions[i].Height;
		VkDeviceSize imgSize = width * height * 4;

		m_device->createImage(width, height, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, lightmapImages_out[i],
			lightmapsMemory_out[i]);

		lightmapImageViews_out[i] = m_device->createImageView(lightmapImages_out[i], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

		m_device->transitionImageLayout(lightmapImages_out[i], VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);


		ShaderParamsInfo info;
		info.lightmap_height = height;
		info.lightmap_width = width;

		shaderParamsBufferObject->writeToBuffer((void*)&info);

		execute(i);
	}

	delete shaderParamsBufferObject;

	for (auto& imgView : lightmapImageViews_in)
		vkDestroyImageView(m_device->getDevice(), imgView, nullptr);

	for (auto& img : lightmapImages_in)
		vkDestroyImage(m_device->getDevice(), img, nullptr);

	for (auto& imgMem : lightmapsMemory_in)
		vkFreeMemory(m_device->getDevice(), imgMem, nullptr);

	descriptorSetLayout->cleanup();
	pipeline->cleanup();

	vkDestroyPipelineLayout(m_device->getDevice(), pipelineLayout, nullptr);
}

void Lightmap_Edges_Module::execute(int n)
{
	createDescriptorSets(n);

	std::cout << "executing compute shader ";

	VkCommandBuffer commandBuffer = m_device->beginSingleTimeCommands();

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipeline->getPipeline());

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1,
		&descriptorSets[0], 0, 0);

	uint32_t work_length = 1;
	uint32_t work_height = 1;

	uint32_t n_WorkGroups_x = 1 + configuration->lightmap_dimensions[n].Width / (32 * work_length);
	uint32_t n_WorkGroups_y = 1 + configuration->lightmap_dimensions[n].Height / (8 * work_height);

	vkCmdDispatch(commandBuffer, n_WorkGroups_x, n_WorkGroups_y, 1);

	m_device->endSingleTimeCommands(commandBuffer);

	m_DescriptorPool->freeDescriptorsSets(descriptorSets);

	vkDeviceWaitIdle(m_device->getDevice());
	std::cout << "...execution complete\n";
}

void Lightmap_Edges_Module::destroyImages()
{
	for (auto& imgView : lightmapImageViews_out)
		vkDestroyImageView(m_device->getDevice(), imgView, nullptr);

	for (auto& img : lightmapImages_out)
		vkDestroyImage(m_device->getDevice(), img, nullptr);

	for (auto& imgMem : lightmapsMemory_out)
		vkFreeMemory(m_device->getDevice(), imgMem, nullptr);
}


void Lightmap_Edges_Module::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding paramsBufferBinding{};
	paramsBufferBinding.binding = 0;
	paramsBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	paramsBufferBinding.descriptorCount = 1;
	paramsBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	paramsBufferBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	VkDescriptorSetLayoutBinding imageStorageBinding{};
	imageStorageBinding.binding = 2;
	imageStorageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	imageStorageBinding.descriptorCount = 1;
	imageStorageBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	imageStorageBinding.pImmutableSamplers = nullptr; // Optional

	std::vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.resize(3);
	bindings = { paramsBufferBinding, samplerLayoutBinding, imageStorageBinding };

	descriptorSetLayout = new MyDescriptorSetLayout(m_device, bindings);
}

void Lightmap_Edges_Module::createDescriptorSets(int n)
{
	MyDescriptorWriter writer(*descriptorSetLayout, *m_DescriptorPool);

	descriptorSets.resize(1);

	VkDescriptorBufferInfo paramsInfo{};
	paramsInfo.buffer = shaderParamsBufferObject->getBuffer();
	paramsInfo.offset = 0;
	paramsInfo.range = sizeof(ShaderParamsInfo);

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageInfo.imageView = lightmapImageViews_in[n];
	//imageInfo.sampler = defaultSampler;

	VkDescriptorImageInfo imageStorageInfo{};
	imageStorageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageStorageInfo.imageView = lightmapImageViews_out[n];

	writer.writeBuffer(0, paramsInfo);
	writer.writeImage(1, imageInfo);
	writer.writeImage(2, imageStorageInfo);
	writer.build(descriptorSets[0]);
}

void Lightmap_Edges_Module::createComputePipeline()
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

	pipeline = new ComputePipeline(m_device, "shaders/compute_lm_edges.spv", pipelineLayout);

}

VkSampler Copy_Lightmaps_Module::createDefaultSampler()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;

	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	samplerInfo.anisotropyEnable = VK_TRUE;

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(m_device->getPhysicalDevice(), &properties);
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	VkSampler sampler;

	if (vkCreateSampler(m_device->getDevice(), &samplerInfo, nullptr,
		&sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}

	return sampler;
}



void Copy_Lightmaps_Module::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding paramsBufferBinding{};
	paramsBufferBinding.binding = 0;
	paramsBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	paramsBufferBinding.descriptorCount = 1;
	paramsBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	paramsBufferBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding indexBufferBinding{};
	indexBufferBinding.binding = 1;
	indexBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	indexBufferBinding.descriptorCount = 1;
	indexBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	indexBufferBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding uvBufferBinding0{};
	uvBufferBinding0.binding = 2;
	uvBufferBinding0.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	uvBufferBinding0.descriptorCount = 1;
	uvBufferBinding0.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	uvBufferBinding0.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding uvBufferBinding1{};
	uvBufferBinding1.binding = 3;
	uvBufferBinding1.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	uvBufferBinding1.descriptorCount = 1;
	uvBufferBinding1.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	uvBufferBinding1.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 4;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	VkDescriptorSetLayoutBinding imageStorageBinding{};
	imageStorageBinding.binding = 5;
	imageStorageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	imageStorageBinding.descriptorCount = 1;
	imageStorageBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	imageStorageBinding.pImmutableSamplers = nullptr; // Optional

	std::vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.resize(6);
	bindings = { paramsBufferBinding, indexBufferBinding, uvBufferBinding0, uvBufferBinding1, samplerLayoutBinding, imageStorageBinding };

	descriptorSetLayout = new MyDescriptorSetLayout(m_device, bindings);
}

void Copy_Lightmaps_Module::createDescriptorSets(int i, int j)
{
	MyDescriptorWriter writer(*descriptorSetLayout, *m_DescriptorPool);

	descriptorSets.resize(1);

	VkDescriptorBufferInfo paramsInfo{};
	paramsInfo.buffer = shaderParamsBufferObject->getBuffer();
	paramsInfo.offset = 0;
	paramsInfo.range = sizeof(ShaderParamsInfo);

	VkDescriptorBufferInfo indexBufferInfo{};
	indexBufferInfo.buffer = indexBuffer;
	indexBufferInfo.offset = 0;
	indexBufferInfo.range = sizeof(aligned_uint) * n_indices;

	VkDescriptorBufferInfo uvBufferInfo0{};
	uvBufferInfo0.buffer = uvBuffer_0;
	uvBufferInfo0.offset = 0;
	uvBufferInfo0.range = sizeof(aligned_vec3) * n_vertices;

	VkDescriptorBufferInfo uvBufferInfo1{};
	uvBufferInfo1.buffer = uvBuffer_1;
	uvBufferInfo1.offset = 0;
	uvBufferInfo1.range = sizeof(aligned_vec3) * n_vertices;

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = lightmapImageViews_in[i];
	imageInfo.sampler = mySampler;

	VkDescriptorImageInfo imageStorageInfo{};
	imageStorageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageStorageInfo.imageView = lightmapImageViews_out[j];

	writer.writeBuffer(0, paramsInfo);
	writer.writeBuffer(1, indexBufferInfo);
	writer.writeBuffer(2, uvBufferInfo0);
	writer.writeBuffer(3, uvBufferInfo1);
	writer.writeImage(4, imageInfo);
	writer.writeImage(5, imageStorageInfo);
	writer.build(descriptorSets[0]);
}

void Copy_Lightmaps_Module::createComputePipeline()
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

	pipeline = new ComputePipeline(m_device, "shaders/compute_lm_copy.spv", pipelineLayout);

}

void Copy_Lightmaps_Module::createImageViews()
{
	for (int i = 0; i < configuration0->lightmap_dimensions.size(); i++)
	{
		//VkDeviceSize width = configuration0->lightmap_dimensions[i].Width;
		//VkDeviceSize height = configuration0->lightmap_dimensions[i].Height;
		//VkDeviceSize imgSize = width * height * 4;

		VkImageView imgView;

		//int a = VK_IMAGE_ASPECT_STENCIL_BIT;

		imgView = m_device->createImageView(lightmapImages_in[i], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

		lightmapImageViews_in.push_back(imgView);
	}
}

void Copy_Lightmaps_Module::createImages()
{
	lightmapImages_out.resize(configuration1->lightmap_dimensions.size());
	lightmapImageViews_out.resize(configuration1->lightmap_dimensions.size());
	lightmapsMemory_out.resize(configuration1->lightmap_dimensions.size());

	for (int i = 0; i < configuration1->lightmap_dimensions.size(); i++)
	{
		VkDeviceSize width = configuration1->lightmap_dimensions[i].Width;
		VkDeviceSize height = configuration1->lightmap_dimensions[i].Height;
		VkDeviceSize imgSize = width * height * 4;

		m_device->createImage(width, height, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, lightmapImages_out[i],
			lightmapsMemory_out[i]);

		lightmapImageViews_out[i] = m_device->createImageView(lightmapImages_out[i], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

		m_device->transitionImageLayout(lightmapImages_out[i], VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
	}
}

void Copy_Lightmaps_Module::createUVBuffer()
{
	n_vertices = uv_struct_0->size();

	{
		

		VkDeviceSize bufferSize = sizeof(aligned_vec3) * n_vertices;

		MyBufferObject stagingBuffer(m_device, sizeof(aligned_vec3), n_vertices, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

		stagingBuffer.writeToBuffer((void*)uv_struct_0->data());

		m_device->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uvBuffer_0,
			uvBufferMemory_0);

		m_device->copyBuffer(stagingBuffer.getBuffer(), uvBuffer_0, bufferSize);
	}

	{

		VkDeviceSize bufferSize = sizeof(aligned_vec3) * n_vertices;

		MyBufferObject stagingBuffer(m_device, sizeof(aligned_vec3), n_vertices, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

		stagingBuffer.writeToBuffer((void*)uv_struct_1->data());

		m_device->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uvBuffer_1,
			uvBufferMemory_1);

		m_device->copyBuffer(stagingBuffer.getBuffer(), uvBuffer_1, bufferSize);
	}
}

void Copy_Lightmaps_Module::run()
{
	createDescriptorSetLayout();
	createComputePipeline();
	//createImageViews();
	createImages();
	createUVBuffer();
	mySampler = createDefaultSampler();

	int n_maps = configuration1->lightmap_dimensions.size();

	lightmapImages_out.resize(n_maps);
	lightmapImageViews_out.resize(n_maps);
	lightmapsMemory_out.resize(n_maps);

	shaderParamsBufferObject = new MyBufferObject(m_device, sizeof(ShaderParamsInfo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 16);

	for (int i = 0; i < configuration0->lightmap_dimensions.size(); i++)
	{
		std::cout << i << " to\n";
		for (int j = 0; j < configuration1->lightmap_dimensions.size(); j++)
		{
			std::cout << "   " << j << ":\n";
			for (tex_block& a : configuration0->bl_combined[i].blocks)
			{
				for (tex_block& b : configuration1->bl_combined[j].blocks)
				{
					if (a.element_id == b.element_id && a.surface_no == b.surface_no)
					{
						
						ShaderParamsInfo info;
						info.lightmap0_height = configuration0->lightmap_dimensions[i].Height;
						info.lightmap0_width = configuration0->lightmap_dimensions[i].Width;

						info.lightmap1_height = configuration1->lightmap_dimensions[j].Height;
						info.lightmap1_width = configuration1->lightmap_dimensions[j].Width;
						//info.intensity = random_number() % 255;
						info.intensity = 255;

						int element_no = element_by_element_id->data()[a.element_id];
						int surface_no = a.surface_no;


						u16 begin = surface_index->data[surface_index->offset[element_no] + surface_no].begin_i;
						u16 end = surface_index->data[surface_index->offset[element_no] + surface_no].end_i;

						std::cout << "       "<<element_no << "," << surface_no << ", " << (end - begin) / 3 << " triangles ";

						info.face_index_offset = begin;
						info.face_n_indices = end - begin;
						//info.f

						shaderParamsBufferObject->writeToBuffer((void*)&info);

						execute(i, j, a.element_id, a.surface_no, (end-begin)/3);
					}
				}
			}
			
		}
	}

	delete shaderParamsBufferObject;

	vkDestroySampler(m_device->getDevice(), mySampler, NULL);

	descriptorSetLayout->cleanup();
	pipeline->cleanup();

	vkDestroyPipelineLayout(m_device->getDevice(), pipelineLayout, nullptr);

	vkDestroyBuffer(m_device->getDevice(), uvBuffer_0, nullptr);
	vkFreeMemory(m_device->getDevice(), uvBufferMemory_0, nullptr);

	vkDestroyBuffer(m_device->getDevice(), uvBuffer_1, nullptr);
	vkFreeMemory(m_device->getDevice(), uvBufferMemory_1, nullptr);

	//for (auto& imgView : lightmapImageViews_in)
	//	vkDestroyImageView(m_device->getDevice(), imgView, nullptr);
}

void Copy_Lightmaps_Module::execute(int i,int j,int element_id,int surface_no, int n_triangles)
{

	createDescriptorSets(i,j);

	std::cout << "  compute shader "<<n_triangles<<" workgroups";

	VkCommandBuffer commandBuffer = m_device->beginSingleTimeCommands();

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipeline->getPipeline());

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1,
		&descriptorSets[0], 0, 0);

	uint32_t work_length = 1;
	uint32_t work_height = 1;

	uint32_t n_WorkGroups_x = n_triangles;// +configuration->lightmap_dimensions[n].Width / (32 * work_length);
	uint32_t n_WorkGroups_y = 1;// +configuration->lightmap_dimensions[n].Height / (8 * work_height);

	vkCmdDispatch(commandBuffer, n_WorkGroups_x, n_WorkGroups_y, 1);

	m_device->endSingleTimeCommands(commandBuffer);

	m_DescriptorPool->freeDescriptorsSets(descriptorSets);

	vkDeviceWaitIdle(m_device->getDevice());
	std::cout << "...execution complete\n";
}

void Copy_Lightmaps_Module::destroyImages()
{
	for (auto& imgView : lightmapImageViews_out)
		vkDestroyImageView(m_device->getDevice(), imgView, nullptr);

	for (auto& img : lightmapImages_out)
		vkDestroyImage(m_device->getDevice(), img, nullptr);

	for (auto& imgMem : lightmapsMemory_out)
		vkFreeMemory(m_device->getDevice(), imgMem, nullptr);
}

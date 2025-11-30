#include <irrlicht.h>
#include "LightMaps.h"
#include "vkModules.h"
#include "vkUtilModules.h"
#include "csg_classes.h"
#include "utils.h"
#include "geometry_scene.h"
#include "soa.h"
#include "my_reflected_nodes.h"
#include <vulkan/vulkan.h>
#include "reflect_custom_types.h"

using namespace irr;
using namespace core;
using namespace std;

extern IrrlichtDevice* device;

//==========================================
// Geometry
//

REFLECT_STRUCT3_BEGIN(Geometry_Module)
	REFLECT_STRUCT_MEMBER(indices)
	REFLECT_STRUCT_MEMBER(vertices)
	REFLECT_STRUCT_MEMBER(lm_uvs)
	REFLECT_STRUCT_MEMBER(bvh_nodes)
	REFLECT_STRUCT_MEMBER(edges)
	REFLECT_STRUCT_MEMBER(scratchpad)
	REFLECT_STRUCT_MEMBER(area_lights)
REFLECT_STRUCT3_END()

Geometry_Module::Geometry_Module(Vulkan_App* vulkan, Geometry_Assets* geo_assets) : Vulkan_Module(vulkan),
geo_assets{ geo_assets }
{
	set_ptrs();

	indices_soa = &geo_assets->indices_soa;
	vertices_soa = &geo_assets->vertices_soa;
	bvh = &geo_assets->bvh;
	n_indices = geo_assets->indices_soa.data.size();
	n_vertices = geo_assets->vertices_soa.data0.size();
	n_nodes = geo_assets->bvh.node_count;
	triangle_edges = &geo_assets->triangle_edges;
	area_light_indices = &geo_assets->area_light_indices;
}

void Geometry_Module::initBuffers()
{
	createIndexBuffer();
	createVertexBuffer();
	createUVBuffer();
	createEdgeBuffer();
	createNodeBuffer();
}

void Geometry_Module::createIndexBuffer() {

	VkDeviceSize bufferSize = sizeof(aligned_uint) * n_indices;

	MyBufferObject stagingBuffer(m_device, sizeof(aligned_uint), n_indices, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	stagingBuffer.writeToBuffer((void*)indices_soa->data.data());

	indices.X = vulkan->create_buffer(bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &indices);

	m_device->copyBuffer(stagingBuffer.getBuffer(), indices.X->Buffer, bufferSize);

	indices.X->range = bufferSize;
	indices.ready = true;
}


void Geometry_Module::createVertexBuffer() {

	VkDeviceSize bufferSize = sizeof(aligned_vec3) * n_vertices;

	MyBufferObject stagingBuffer(m_device, sizeof(aligned_vec3), n_vertices, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	stagingBuffer.writeToBuffer((void*)vertices_soa->data0.data());

	vertices.X = vulkan->create_buffer(bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &vertices);

	m_device->copyBuffer(stagingBuffer.getBuffer(), vertices.X->Buffer, bufferSize);

	vertices.X->range = bufferSize;
	vertices.ready = true;
}

void Geometry_Module::init_area_light_buffer() {

	int n = area_light_indices->size();

	VkDeviceSize bufferSize = sizeof(aligned_uint) * n;

	MyBufferObject stagingBuffer(m_device, sizeof(aligned_uint), n, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	stagingBuffer.writeToBuffer((void*)area_light_indices->data());

	area_lights.X = vulkan->create_buffer(bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &area_lights);

	m_device->copyBuffer(stagingBuffer.getBuffer(), area_lights.X->Buffer, bufferSize);

	area_lights.X->range = bufferSize;
	area_lights.ready = true;
}

void Geometry_Module::createUVBuffer()
{
	VkDeviceSize bufferSize = sizeof(aligned_vec3) * n_vertices;

	MyBufferObject stagingBuffer(m_device, sizeof(aligned_vec3), n_vertices, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	stagingBuffer.writeToBuffer((void*)vertices_soa->data1.data());

	lm_uvs.X = vulkan->create_buffer(bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &lm_uvs);

	m_device->copyBuffer(stagingBuffer.getBuffer(), lm_uvs.X->Buffer, bufferSize);

	lm_uvs.X->range = bufferSize;
	lm_uvs.ready = true;
}

void Geometry_Module::createEdgeBuffer()
{
	VkDeviceSize bufferSize = sizeof(aligned_uint) * n_indices;

	MyBufferObject stagingBuffer(m_device, sizeof(aligned_uint), n_indices, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	stagingBuffer.writeToBuffer((void*)triangle_edges->data());

	edges.X = vulkan->create_buffer(bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &edges);

	m_device->copyBuffer(stagingBuffer.getBuffer(), edges.X->Buffer, bufferSize);

	edges.X->range = bufferSize;
	edges.ready = true;
}

void Geometry_Module::createNodeBuffer()
{
	VkDeviceSize bufferSize = sizeof(BVH_node_gpu) * n_nodes;

	MyBufferObject stagingBuffer(m_device, sizeof(BVH_node_gpu), n_nodes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	stagingBuffer.writeToBuffer((void*)bvh->nodes.data());

	bvh_nodes.X = vulkan->create_buffer(bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &bvh_nodes);

	m_device->copyBuffer(stagingBuffer.getBuffer(), bvh_nodes.X->Buffer, bufferSize);

	bvh_nodes.X->range = bufferSize;
	bvh_nodes.ready = true;
}

void Geometry_Module::createOutputBuffer()
{
	VkDeviceSize bufferSize = sizeof(aligned_vec3) * 512;

	scratchpad.X = vulkan->create_buffer(bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &scratchpad);

	scratchpad.X->range = bufferSize;
	scratchpad.ready = true;
	//m_device->copyBuffer(stagingBuffer.getBuffer(), nodeBuffer, bufferSize);
}

void Geometry_Module::run()
{
	initBuffers();
	init_area_light_buffer();
	createOutputBuffer();
}


//==========================================
// Create Lightmap Images
//

REFLECT_STRUCT3_BEGIN(Create_Lightmap_Images_Module)
	REFLECT_STRUCT_MEMBER(images_out)
REFLECT_STRUCT3_END()

void Create_Lightmap_Images_Module::run()
{
	//if (!load_resources())
	//	return;

	createImages();

	images_out.ready = true;
}

void Create_Lightmap_Images_Module::createImages()
{
	int n_layers = configuration->lightmap_dimensions.size();
	VkDeviceSize width = configuration->lightmap_dimensions[0].Width;
	VkDeviceSize height = configuration->lightmap_dimensions[0].Height;

	images_out.X = vulkan->create_multiImage(n_layers, width, height, 
		VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		&images_out);

	for (int i = 0; i < n_layers; i++)
	{
		m_device->transitionImageLayout(images_out.X->Images[i].Image, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
	}
}

//==========================================
// Create Texture Images
//

REFLECT_STRUCT3_BEGIN(MultiImage_To_ImageArray_Module)
	REFLECT_STRUCT_MEMBER(images_in)
	REFLECT_STRUCT_MEMBER(images_out)
REFLECT_STRUCT3_END()

void MultiImage_To_ImageArray_Module::run()
{
	createImages();
}

void MultiImage_To_ImageArray_Module::createImages()
{
		VkDeviceSize width = configuration->lightmap_dimensions[0].Width;
		VkDeviceSize height = configuration->lightmap_dimensions[0].Height;
		uint32_t n_layers = images_in.X->Images.size();
		//VkImage img;
		//VkDeviceMemory imgMemory;
		//VkImageView imgView;
		/*
		m_device->createImageArray(n_layers, width, height, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, images_out.X->Image,
			images_out.X->ImageMemory);

		images_out.X->ImageView = m_device->createImageArrayView(n_layers, images_out.X->Image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
		*/

		images_out.X = vulkan->create_imageArray(n_layers, width, height,
			VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			&images_out);

		m_device->transitionImageArrayLayout( n_layers, images_out.X->Image, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		for (int i = 0; i < n_layers; i++)
		{
			m_device->transitionImageLayout(images_in.X->Images[i].Image, VK_FORMAT_R8G8B8A8_UNORM,
				VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

			m_device->copyImageToImageLayer(i, images_out.X->Image, images_in.X->Images[i].Image, width, height);

			m_device->transitionImageLayout(images_in.X->Images[i].Image, VK_FORMAT_R8G8B8A8_UNORM,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
		}

		m_device->transitionImageArrayLayout(n_layers, images_out.X->Image, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);

		vkDeviceWaitIdle(m_device->getDevice());

		//m_device->transitionImageArrayLayout(n_layers, img, VK_FORMAT_R8G8B8A8_UNORM,
		//	VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		//images_out.X->Image = img;
		//images_out.X->ImageView = imgView;
		//images_out.X->ImageMemory = imgMemory;
		images_out.X->n_images = n_layers;

		images_out.X->initializeDescriptorInfo();
		images_out.ready = true;
}

//==========================================
// MultiImage Copy
//

REFLECT_STRUCT3_BEGIN(MultiImage_Copy_Module)
	REFLECT_STRUCT_MEMBER(images_in)
	REFLECT_STRUCT_MEMBER(images_out)
REFLECT_STRUCT3_END()

void MultiImage_Copy_Module::run()
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
			VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, img,
			imgMemory);

		imgView = m_device->createImageView(img, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

		m_device->transitionImageLayout(img, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		m_device->transitionImageLayout(images_in.X->Images[i].Image, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		m_device->copyImage(img, images_in.X->Images[i].Image, width, height);

		m_device->transitionImageLayout(img, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);

		images_out.X->Images.push_back(vkImageSubresource{ img,imgMemory,imgView });
	}

	//for (auto& img : images_in.X->Images)
	//	img.destroy(m_device->getDevice());

	images_out.ready = true;
}

//==========================================
// Create Texture Images
//

REFLECT_STRUCT3_BEGIN(Create_Texture_Images_Module)
	REFLECT_STRUCT_MEMBER(images_out)
REFLECT_STRUCT3_END()

void Create_Texture_Images_Module::run()
{
	//if (!load_resources())
	//	return;
}

Create_Texture_Images_Module::~Create_Texture_Images_Module()
{
	//for (auto& img : images_out.X->Images)
	//	img.destroy(m_device->getDevice());
}

void Create_Texture_Images_Module::index_materials(Lightmap_Configuration* configuration)
{
	std::vector<TextureMaterial>& materials = configuration->materials;

	std::vector<video::ITexture*> textures_used;

	for (int i = 0; i < materials.size(); i++)
	{
		bool b = false;

		for (int j = 0; j < textures_used.size(); j++)
		{
			if (materials[i].texture == textures_used[j])
			{
				b = true;
				materials[i].texture_idx = j;

				break;
			}
		}
		if (!b)
		{
			textures_used.push_back(materials[i].texture);

			materials[i].texture_idx = textures_used.size() - 1;
		}
	}

	images_out.X->Images.resize(textures.size());

	for (int i = 0; i < textures.size(); i++)
	{
		images_out.X->Images[i].create_and_load_texture(m_device, driver, textures[i]);
	}

	images_out.ready = true;
}

//==========================================
// Load Textures
//

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

		irr::video::IImage* pImage = driver->createImage(textures[i], core::vector2di(0, 0), configuration->lightmap_dimensions[i]);
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
{/*
	for (auto& imgView : lightmapImageViews)
		vkDestroyImageView(m_device->getDevice(), imgView, nullptr);

	for (auto& img : lightmapImages)
		vkDestroyImage(m_device->getDevice(), img, nullptr);

	for (auto& imgMem : lightmapsMemory)
		vkFreeMemory(m_device->getDevice(), imgMem, nullptr);*/
}

//==========================================
// Download Textures
//

REFLECT_STRUCT3_BEGIN(Download_Textures_Module)
	REFLECT_STRUCT_MEMBER(images_in)
REFLECT_STRUCT3_END()

void Download_Textures_Module::run()
{
	//if (!load_resources())
	//	return;

	for (int i = 0; i < configuration->lightmap_dimensions.size(); i++)
	{
		VkDeviceSize width = configuration->lightmap_dimensions[i].Width;
		VkDeviceSize height = configuration->lightmap_dimensions[i].Height;
		VkDeviceSize imgSize = width * height * 4;

		MyBufferObject stagingBuffer(m_device, imgSize, 1, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

		m_device->transitionImageLayout(images_in.X->Images[i].Image, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		m_device->copyImageToBuffer(stagingBuffer.getBuffer(), images_in.X->Images[i].Image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

		irr::video::IImage* pImage = driver->createImage(irr::video::ECF_A8R8G8B8, irr::core::dimension2du(width, height));

		irr::u8* imgDataPtr = (irr::u8*)pImage->lock();

		stagingBuffer.readFromBuffer(imgDataPtr);

		if (bFlip)
			pImage->flip(true, false);

		irr::video::ITexture* tex = driver->addTexture(irr::io::path("image name"), pImage);
		textures.push_back(tex);

		pImage->unlock();
		pImage->drop();

		m_device->transitionImageLayout(images_in.X->Images[i].Image, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	//for (auto& img : images_in.X->Images)
	//	img.destroy(m_device->getDevice());
}

//==========================================
// Download Textures
//

REFLECT_STRUCT3_BEGIN(Download_TextureArray_Module)
	REFLECT_STRUCT_MEMBER(images_in)
REFLECT_STRUCT3_END()

void Download_TextureArray_Module::run()
{

	m_device->transitionImageArrayLayout(images_in.X->n_images,images_in.X->Image, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	for (int i = 0; i < configuration->lightmap_dimensions.size(); i++)
	{
		VkDeviceSize width = configuration->lightmap_dimensions[i].Width;
		VkDeviceSize height = configuration->lightmap_dimensions[i].Height;
		VkDeviceSize imgSize = width * height * 4;

		MyBufferObject stagingBuffer(m_device, imgSize, 1, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

		m_device->copyImageLayerToBuffer(i,stagingBuffer.getBuffer(), images_in.X->Image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));

		irr::video::IImage* pImage = driver->createImage(irr::video::ECF_A8R8G8B8, irr::core::dimension2du(width, height));

		irr::u8* imgDataPtr = (irr::u8*)pImage->lock();

		stagingBuffer.readFromBuffer(imgDataPtr);

		if (bFlip)
			pImage->flip(true, false);

		irr::video::ITexture* tex = driver->addTexture(irr::io::path("image name"), pImage);
		textures.push_back(tex);

		pImage->unlock();
		pImage->drop();

		//m_device->transitionImageLayout(images_in.X->Image, VK_FORMAT_R8G8B8A8_UNORM,
		//	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	//images_in.X->destroy(m_device->getDevice());
}


//==========================================
// Lightmap Edge/Corner Correction
//

REFLECT_STRUCT3_BEGIN(Lightmap_Edges_Module)
	REFLECT_STRUCT_MEMBER(images_in)
	REFLECT_STRUCT_MEMBER(images_out)
REFLECT_STRUCT3_END()

void Lightmap_Edges_Module::run()
{
	//if (!load_resources())
	//	return;

	createDescriptorSetLayout();
	createComputePipeline();
	int n_layers = images_in.X->Images.size();

	shaderParamsBufferObject = new MyBufferObject(m_device, sizeof(ShaderParamsInfo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 16);

	VkDeviceSize width = configuration->lightmap_dimensions[0].Width;
	VkDeviceSize height = configuration->lightmap_dimensions[0].Height;
	VkDeviceSize imgSize = width * height * 4;

	images_out.X = vulkan->create_multiImage(n_layers, width, height,
		VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, &images_out);

	for (int i = 0; i < n_layers; i++)
	{
		/*
		m_device->createImage(width, height, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, images_out.X->Images[i].Image,
			images_out.X->Images[i].ImageMemory);

		images_out.X->Images[i].ImageView = m_device->createImageView(images_out.X->Images[i].Image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
		*/
		m_device->transitionImageLayout(images_out.X->Images[i].Image, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

		ShaderParamsInfo info;
		info.lightmap_height = height;
		info.lightmap_width = width;

		shaderParamsBufferObject->writeToBuffer((void*)&info);

		execute(i);
	}

	delete shaderParamsBufferObject;


	//for (auto& img : images_in.X->Images)
	//	img.destroy(m_device->getDevice());

	descriptorSetLayout->cleanup();
	pipeline->cleanup();

	vkDestroyPipelineLayout(m_device->getDevice(), pipelineLayout, nullptr);

	images_out.ready = true;
}

void Lightmap_Edges_Module::execute(int n)
{
	createDescriptorSets(n);

	VkCommandBuffer commandBuffer = m_device->beginSingleTimeCommands();

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipeline->getPipeline());

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1,
		&descriptorSets[0], 0, 0);

	uint32_t work_length = 1;
	uint32_t work_height = 1;

	uint32_t n_WorkGroups_x = 1 + configuration->lightmap_dimensions[n].Width / (32 * work_length);
	uint32_t n_WorkGroups_y = 1 + configuration->lightmap_dimensions[n].Height / (8 * work_height);

	std::cout << "executing compute shader (" << n_WorkGroups_x << " / " << n_WorkGroups_y << ")\n";

	vkCmdDispatch(commandBuffer, n_WorkGroups_x, n_WorkGroups_y, 1);

	m_device->endSingleTimeCommands(commandBuffer);

	m_DescriptorPool->freeDescriptorsSets(descriptorSets);

	vkDeviceWaitIdle(m_device->getDevice());
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

	std::vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.resize(3);

	//bindings = { paramsBufferBinding, samplerLayoutBinding, imageStorageBinding };

	bindings[0] = paramsBufferBinding;
	bindings[1] = images_in.X->getDescriptorSetLayout(1);
	bindings[2] = images_out.X->getDescriptorSetLayout(2);

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
	imageInfo.imageView = images_in.X->Images[n].ImageView;
	//imageInfo.sampler = defaultSampler;

	VkDescriptorImageInfo imageStorageInfo{};
	imageStorageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageStorageInfo.imageView = images_out.X->Images[n].ImageView;

	writer.writeBuffer(0, paramsInfo);
	writer.writeImage(1, images_in.X->getDescriptorBufferInfo(n));
	writer.writeImage(2, images_out.X->getDescriptorBufferInfo(n));
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

//==========================================
// Lightmap Edge/Corner Correction
//

REFLECT_STRUCT3_BEGIN(Merge_Images_Module)
	REFLECT_STRUCT_MEMBER(images_in_0)
	REFLECT_STRUCT_MEMBER(images_in_1)
	REFLECT_STRUCT_MEMBER(images_out)
	REFLECT_STRUCT_MEMBER_FORWARD(images_in_1, images_out)
REFLECT_STRUCT3_END()


void Merge_Images_Module::run()
{
	createDescriptorSetLayout();
	createComputePipeline("shaders/merge_images.spv");

	images_out.X->Images.resize(images_in_0.X->Images.size());

	ubo.range = sizeof(ShaderParamsInfo);

	m_device->createBuffer(ubo.range, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, ubo.Buffer,
		ubo.BufferMemory);

	for (int i = 0; i < images_in_0.X->Images.size(); i++)
	{
		VkDeviceSize width = configuration->lightmap_dimensions[i].Width;
		VkDeviceSize height = configuration->lightmap_dimensions[i].Height;

		ShaderParamsInfo info;
		info.lightmap_height = height;
		info.lightmap_width = width;

		ubo.writeToBuffer(m_device->getDevice(), (void*)&info);

		execute(i);
	}

	//for (auto& img : images_in_0.X->Images)
	//	img.destroy(m_device->getDevice());

	images_out.X = images_in_1.X;
	images_out.ready = true;
}

void Merge_Images_Module::createDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.resize(3);

	bindings[0] = ubo.getDescriptorSetLayout(0);
	bindings[1] = images_in_0.X->getDescriptorSetLayout(1);
	bindings[2] = images_in_1.X->getDescriptorSetLayout(2);
	//bindings[2] = images_out.X->getDescriptorSetLayout(2);

	descriptorSetLayout = new MyDescriptorSetLayout(m_device, bindings);
}

void Merge_Images_Module::createDescriptorSets(int layer)
{
	MyDescriptorWriter writer(*descriptorSetLayout, *m_DescriptorPool);

	descriptorSets.resize(1);

	writer.writeBuffer(0, ubo.getDescriptorBufferInfo());
	writer.writeImage(1, images_in_0.X->getDescriptorBufferInfo(layer));
	writer.writeImage(2, images_in_1.X->getDescriptorBufferInfo(layer));
	//writer.writeImage(2, images_out.X->getDescriptorBufferInfo(layer));
	writer.build(descriptorSets[0]);
}

void Merge_Images_Module::execute(u32 n)
{
	createDescriptorSets(n);

	VkCommandBuffer commandBuffer = m_device->beginSingleTimeCommands();

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipeline->getPipeline());

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1,
		&descriptorSets[0], 0, 0);

	uint32_t work_length = 1;
	uint32_t work_height = 1;

	uint32_t n_WorkGroups_x = 1 + configuration->lightmap_dimensions[n].Width / (32 * work_length);
	uint32_t n_WorkGroups_y = 1 + configuration->lightmap_dimensions[n].Height / (8 * work_height);

	std::cout << "executing compute shader (" << n_WorkGroups_x << " / " << n_WorkGroups_y << ")\n";

	vkCmdDispatch(commandBuffer, n_WorkGroups_x, n_WorkGroups_y, 1);

	m_device->endSingleTimeCommands(commandBuffer);

	m_DescriptorPool->freeDescriptorsSets(descriptorSets);

	vkDeviceWaitIdle(m_device->getDevice());
}

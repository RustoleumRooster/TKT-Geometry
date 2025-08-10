#pragma once
#ifndef _VK_SUNLIGHT_MOD_H_
#define _VK_SUNLIGHT_MOD_H_

#include <vulkan/vulkan.h>
#include <irrlicht.h>
#include "vkModules.h"
#include "Reflection.h"
#include "vk_BVH.h"
#include "vkDevice.h"
#include "vkDescriptors.h"
#include "vkBufferObject.h"
#include "vkComputePipeline.h"

class Lightmap_Configuration;

class Sunlight_Module : public Vulkan_Module
{

	struct RayTraceInfo {
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

		aligned_vec3 sun_dir;
		aligned_vec3 sun_tan;
		aligned_vec3 sun_bitan;
	};

public:
	Sunlight_Module(Vulkan_App* vulkan) : 
		Vulkan_Module(vulkan)
	{
	}

	void createComputePipeline();
	void createDescriptorSetLayout();
	void createDescriptorSets(int);
	void createRaytraceInfoBuffer();

	virtual void run() override;
	void cleanup();


	void runMaterial(int);
	void writeRaytraceInfoBuffer(int);


	const std::vector<TextureMaterial>* materials = NULL;

	VkPipelineLayout pipelineLayout;
	std::vector<VkDescriptorSet> descriptorSets;
	std::vector<VkBuffer> uniformBuffers;

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

	MyBufferObject* raytraceBufferObject = NULL;

	MyDescriptorSetLayout* descriptorSetLayout = NULL;
	ComputePipeline* pipeline = NULL;

	vector3df sun_direction{ 0, 0, 0 };

	uint16_t n_indices = 0;
	uint16_t n_vertices = 0;
	uint32_t n_nodes = 0;

	soa_struct_2<aligned_vec3, aligned_vec3>* vertices_soa = NULL;
	soa_struct<aligned_uint>* indices_soa = NULL;

	std::vector<VkImage> lightmapImages;
	std::vector<VkDeviceMemory> lightmapsMemory;
	std::vector<VkImageView> lightmapImageViews;

};
#endif
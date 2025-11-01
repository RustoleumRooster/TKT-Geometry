#pragma once

#ifndef _VK_AREALIGHT_MOD_H_
#define _VK_AREALIGHT_MOD_H_

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

class AreaLight_Module : public Vulkan_Module
{

	struct RayTraceInfo {
		//Lightmap coordinates
		alignas(16) vector3df in_coords;

		uint32_t n_rays;

		//scene info
		uint32_t n_triangles;
		uint32_t n_vertices;
		uint32_t n_nodes;

		//face info
		uint32_t face_vertex_offset; //not used in shader
		uint32_t face_index_offset;
		uint32_t face_n_indices;

		//lightmap info
		uint32_t lightmap_width;
		uint32_t lightmap_height;

		uint32_t n_lights;

	};

public:
	AreaLight_Module(Vulkan_App* vulkan) :
		Vulkan_Module(vulkan)
	{
	}

	void createComputePipeline();
	void createDescriptorSetLayout();
	void createDescriptorSets(int);
	void createRaytraceInfoBuffer();

	virtual void run() override;

	void createHitResultsBuffer();
	void read_results();
	void cleanup();


	void runMaterial(int);
	void runTriangle(int mat_n, int triangle_offset);
	void writeRaytraceInfoBuffer(int);
	void writeRaytraceInfoBuffer2(int mg , int triangle_n);


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

	VkBuffer lightSourceBuffer;
	VkDeviceMemory lightSourceBufferMemory;

	VkBuffer outputBuffer;
	VkDeviceMemory outputBufferMemory;

	MyBufferObject* raytraceBufferObject = NULL;
	MyBufferObject* hitResultsBufferObject = NULL;
	aligned_vec3* hit_results = NULL;

	MyDescriptorSetLayout* descriptorSetLayout = NULL;
	ComputePipeline* pipeline = NULL;

	Triangle_Transformer* triangle_trans = NULL;

	uint16_t n_indices = 0;
	uint16_t n_vertices = 0;
	uint32_t n_nodes = 0;

	uint16_t n_lightsource_indices = 0;

	uint16_t selected_triangle_index = 0;
	uint16_t selected_triangle_mg = 0;
	vector3df selected_triangle_bary_coords{ 0,0,0 };

	soa_struct_2<aligned_vec3, aligned_vec3>* vertices_soa = NULL;
	soa_struct<aligned_uint>* indices_soa = NULL;
	std::vector<aligned_uint>* lightsource_indices_soa = NULL;

	std::vector<VkImage> lightmapImages;
	std::vector<VkDeviceMemory> lightmapsMemory;
	std::vector<VkImageView> lightmapImageViews;

	LineHolder graph;

};
#endif
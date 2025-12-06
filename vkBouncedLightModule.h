#pragma once

#ifndef _VK_BOUNCEDLIGHT_MOD_H_
#define _VK_BOUNCEDLIGHT_MOD_H_

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
class Geometry_Module;

class BouncedLight_Module : public Vulkan_Module
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
		uint32_t lightmap_no;

	};

public:
	BouncedLight_Module(Vulkan_App* vulkan);

	void createDescriptorSetLayout();
	void createDescriptorSets(int);
	void createRaytraceInfoBuffer();

	virtual void run() override;

	void createImages();
	void read_results();
	void cleanup();

	void runMaterial(int);
	void runTriangle(int mat_n, int triangle_offset);
	void writeUBO(int);
	void writeUBO2(int mg, int triangle_n);

	const std::vector<TextureMaterial>* materials = NULL;

	vkUniformBufferResource ubo;

	reflect::input<vkBufferResource> vertices;
	reflect::input<vkBufferResource> indices;
	reflect::input<vkBufferResource> lm_uvs;
	reflect::input<vkBufferResource> bvh_nodes;
	reflect::input<vkBufferResource> edges;
	reflect::input<vkBufferResource> scratchpad;
	reflect::input<vkBufferResource> area_lights;

	reflect::input<vkImageArrayResource> images_in;
	reflect::output<vkImageArrayResource> images_out;

	Triangle_Transformer* triangle_trans = NULL;

	uint16_t n_indices = 0;
	uint16_t n_vertices = 0;
	uint32_t n_nodes = 0;

	uint16_t selected_triangle_index = 0;
	uint16_t selected_triangle_mg = 0;
	vector3df selected_triangle_bary_coords{ 0,0,0 };

	soa_struct_2<aligned_vec3, aligned_vec3>* vertices_soa = NULL;
	soa_struct<aligned_uint>* indices_soa = NULL;

	Lightmap_Configuration* configuration = NULL;

	LineHolder graph;

	REFLECT3()

};
#endif

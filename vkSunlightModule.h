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
class Geometry_Module;

//=====================================================================
// Sunlight
//
// Monte Carlo estimate of direct sunlight for every lightmapped triangle.
// Each texel traces one ray per thread (256 per pass) from a random point in the
// texel, in a random direction inside the sun's cone (its apparent size in the sky).
// A ray is lit when its closest hit is a sky triangle (GPU_TRIS_SKY, read from the
// BVH packing), or when it hits nothing and miss_is_sky is set.
//
//		texel = intensity * mean( visible * cos(ray, surface normal) )
//

class Sunlight_Module : public Vulkan_Module
{
	//std140: each vec3 shares its 16 byte slot with the scalar after it
	struct SunTraceInfo {
		alignas(16) vector3df sun_dir;	//unit vector pointing TOWARD the sun
		float cos_max;					//cos of the cone half angle

		alignas(16) vector3df sun_tan;
		float intensity;

		alignas(16) vector3df sun_bitan;
		uint32_t n_passes;				//rays per texel = 256 * n_passes

		//scene info
		uint32_t n_triangles;
		uint32_t n_vertices;
		uint32_t n_nodes;
		uint32_t face_vertex_offset;	//not used in shader

		//face info
		uint32_t face_index_offset;
		uint32_t face_n_indices;

		//lightmap info
		uint32_t lightmap_width;
		uint32_t lightmap_height;

		uint32_t miss_is_sky;
		uint32_t seed;
	};

public:
	Sunlight_Module();

	virtual void initialize(Vulkan_App* vulkan);

	void createDescriptorSetLayout();
	void createDescriptorSets(int);
	void createUBO();

	virtual void run() override;

	void cleanup();

	void runMaterial(int);
	void estimateRays(int);
	void writeUBO(int);

	//=================================
	// Sun parameters (not reflected yet)
	//
	vector3df sun_direction{ 0.3f, 1.0f, 0.2f };	//toward the sun, normalized in writeUBO
	f32 sun_angle = 0.53f;		//apparent diameter of the sun in degrees (real sun ~0.53)
	f32 intensity = 1.0f;		//texel value for a fully lit surface facing the sun
	u32 n_passes = 1;			//rays per texel = 256 * n_passes
	bool miss_is_sky = false;	//treat rays that escape the level as reaching the sky (leaky / unsealed test scenes)

	const std::vector<TextureMaterial>* materials = NULL;

	vkUniformBufferResource ubo;

	reflect::input<vkBufferResource> vertices;
	reflect::input<vkBufferResource> indices;
	reflect::input<vkBufferResource> lm_uvs;
	reflect::input<vkBufferResource> bvh_nodes;
	reflect::input<vkBufferResource> edges;
	reflect::input<vkMultiImageResource> images_in;

	reflect::output<vkMultiImageResource> images_out;

	Triangle_Transformer* triangle_trans = NULL;

	uint32_t n_indices = 0;
	uint32_t n_vertices = 0;
	uint32_t n_nodes = 0;
	uint32_t n_sky_triangles = 0;

	soa_struct_2<aligned_vec3, aligned_vec3>* vertices_soa = NULL;
	soa_struct<aligned_uint>* indices_soa = NULL;

	Lightmap_Configuration* configuration = NULL;

	REFLECT_VKMOD()
};
#endif

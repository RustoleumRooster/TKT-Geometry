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

		//single triangle debugging (shader_sunlight.comp "Debug output")
		uint32_t debug_mode;
		alignas(16) vector3df in_coords;	//lightmap uv of the clicked point
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
	void runTriangle(int mat_n, int triangle_offset);
	void estimateRays(int);
	void writeUBO(int mat_n, int debug = SUN_DEBUG_OFF, int triangle_offset = -1);
	void read_debug_results(int mat_n, int triangle_offset);

	//=================================
	// Sun parameters (not reflected yet)
	//
	vector3df sun_direction{ 0.3f, 1.0f, 0.2f };	//toward the sun, normalized in writeUBO
	f32 sun_angle = 0.53f;		//apparent diameter of the sun in degrees (real sun ~0.53)
	f32 intensity = 1.0f;		//texel value for a fully lit surface facing the sun
	u32 n_passes = 1;			//rays per texel = 256 * n_passes
	bool miss_is_sky = false;	//treat rays that escape the level as reaching the sky (leaky / unsealed test scenes)

	//=================================
	// Single triangle debugging
	//
	// With a face selected (the last click picks the triangle and texel), the selected
	// triangle is run once more after the full bake with debug output on. The results
	// are printed and drawn in the 3D view (Triangles view, special_graph).
	//
	//	SUN_DEBUG_RAYS:   the clicked texel's sample points and rays (to their hit, or toward the sun)
	//	SUN_DEBUG_TEXELS: every texel the rasteriser visits for the triangle; written texels are
	//	                  drawn as a normal, unwritten ones as a cross, plus a console table
	//
	enum { SUN_DEBUG_OFF = 0, SUN_DEBUG_RAYS = 1, SUN_DEBUG_TEXELS = 2 };
	int debug_mode = SUN_DEBUG_OFF;

	const std::vector<TextureMaterial>* materials = NULL;

	vkUniformBufferResource ubo;

	reflect::input<vkBufferResource> vertices;
	reflect::input<vkBufferResource> indices;
	reflect::input<vkBufferResource> lm_uvs;
	reflect::input<vkBufferResource> bvh_nodes;
	reflect::input<vkBufferResource> edges;
	reflect::input<vkBufferResource> scratchpad;
	reflect::input<vkMultiImageResource> images_in;

	reflect::output<vkMultiImageResource> images_out;

	Triangle_Transformer* triangle_trans = NULL;

	uint32_t n_indices = 0;
	uint32_t n_vertices = 0;
	uint32_t n_nodes = 0;
	uint32_t n_sky_triangles = 0;

	bool has_selected_triangle = false;
	int selected_triangle_index = 0;
	int selected_triangle_mg = 0;
	vector3df selected_triangle_bary_coords{ 0,0,0 };

	soa_struct_2<aligned_vec3, aligned_vec3>* vertices_soa = NULL;
	soa_struct<aligned_uint>* indices_soa = NULL;

	Lightmap_Configuration* configuration = NULL;

	REFLECT_VKMOD()
};
#endif

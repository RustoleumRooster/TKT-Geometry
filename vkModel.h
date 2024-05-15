#pragma once

#include <vulkan/vulkan.h>
#include <irrlicht.h>

#include <vector>
#include <array>

#include "Reflection.h"
#include "utils.h"

#include "soa.h"

using namespace std;
using namespace irr;
using namespace core;

struct aligned_uint {
	alignas(16) uint32_t x;
};

struct aligned_vec3 {
	alignas(16) vector3df V;
};

class TextureMaterial;

void writeLightmapsInfo(const vector<TextureMaterial>& materials_used, std::vector<LightMaps_Info_Struct>& dest);
void fill_vertex_struct(SMesh* mesh, soa_struct_2<aligned_vec3, aligned_vec3>& dest);
void fill_index_struct(SMesh* mesh, soa_struct<aligned_uint>& dest);
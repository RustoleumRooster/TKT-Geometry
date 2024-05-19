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

struct triangle_b {

    triangle_b() {};

    triangle_b(u16 x, u16 y, u16 z)
    {
        v_i[0] = x;
        v_i[1] = y;
        v_i[2] = z;
    }

    void set(u16 x, u16 y, u16 z)
    {
        v_i[0] = x;
        v_i[1] = y;
        v_i[2] = z;
    }

	u16 v_i[3];

    vector3df position(const aligned_vec3* vertices) const {
        vector3df center = vertices[v_i[0]].V + vertices[v_i[1]].V + vertices[v_i[2]].V;
        center /= 3.0f;
        return center;
    }

    template<typename T>
    void grow(T* obj, const aligned_vec3* vertices) const
    {
        for (int i = 0; i < 3; i++)
        {
            obj->aabbMin[0] = fmin(obj->aabbMin[0], vertices[v_i[i]].V.X);
            obj->aabbMin[1] = fmin(obj->aabbMin[1], vertices[v_i[i]].V.Y);
            obj->aabbMin[2] = fmin(obj->aabbMin[2], vertices[v_i[i]].V.Z);

            obj->aabbMax[0] = fmax(obj->aabbMax[0], vertices[v_i[i]].V.X);
            obj->aabbMax[1] = fmax(obj->aabbMax[1], vertices[v_i[i]].V.Y);
            obj->aabbMax[2] = fmax(obj->aabbMax[2], vertices[v_i[i]].V.Z);
        }
    }
};

class TextureMaterial;

void writeLightmapsInfo(const vector<TextureMaterial>& materials_used, std::vector<LightMaps_Info_Struct>& dest);
void fill_vertex_struct(SMesh* mesh, soa_struct_2<aligned_vec3, aligned_vec3>& dest);
void fill_index_struct(SMesh* mesh, soa_struct<aligned_uint>& dest);

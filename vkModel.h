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

struct triangle_edge
{
    vector3df v0;
    vector3df v1;
    /*
    template<typename T>
    void grow(T* obj) const
    {
        obj->aabbMin[0] = fmin(obj->aabbMin[0], v0.X);
        obj->aabbMin[1] = fmin(obj->aabbMin[1], v0.Y);
        obj->aabbMin[2] = fmin(obj->aabbMin[2], v0.Z);

        obj->aabbMax[0] = fmax(obj->aabbMax[0], v0.X);
        obj->aabbMax[1] = fmax(obj->aabbMax[1], v0.Y);
        obj->aabbMax[2] = fmax(obj->aabbMax[2], v0.Z);
    }*/
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

    vector3df normal(const vector<aligned_vec3>& vertices)
    {
        vector3df N = vector3df(vertices[v_i[1]].V - vertices[v_i[0]].V).crossProduct(vector3df(vertices[v_i[2]].V - vertices[v_i[1]].V));
        N.normalize();
        return N;
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

    bool find_edge(u16 v0, u16 v1, const vector<aligned_vec3>& vertices)
    {
        if (vertices[v_i[0]].V == vertices[v0].V && vertices[v_i[1]].V == vertices[v1].V)
            return true;

        if (vertices[v_i[0]].V == vertices[v1].V && vertices[v_i[1]].V == vertices[v0].V)
            return true;

        if (vertices[v_i[1]].V == vertices[v0].V && vertices[v_i[2]].V == vertices[v1].V)
            return true;

        if (vertices[v_i[1]].V == vertices[v1].V && vertices[v_i[2]].V == vertices[v0].V)
            return true;

        if (vertices[v_i[2]].V == vertices[v0].V && vertices[v_i[0]].V == vertices[v1].V)
            return true;

        if (vertices[v_i[2]].V == vertices[v1].V && vertices[v_i[0]].V == vertices[v0].V)
            return true;

        return false;
    }

};

class TextureMaterial;
class MeshNode_Interface_Final;

void writeLightmapsInfo(const vector<TextureMaterial>& materials_used, std::vector<LightMaps_Info_Struct>& dest, MeshNode_Interface_Final* meshnode);
void fill_vertex_struct(SMesh* mesh, soa_struct_2<aligned_vec3, aligned_vec3>& dest);
void fill_index_struct(SMesh* mesh, soa_struct<aligned_uint>& dest);

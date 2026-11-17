#ifndef _GPU_TRIANGLE_LISTS_H_
#define _GPU_TRIANGLE_LISTS_H_

#include <vector>
#include "soa.h"

//=====================================================================
// GPU triangle lists
//
// A Material_Group can route its final-mesh triangles to a GPU buffer by
// setting gpu_triangle_list to one of these channels. Geometry_Assets gathers
// every channel in one pass and Geometry_Module uploads each to its own port.
//
// Every channel uses the same layout: pairs of uints, one pair per triangle
//      [ triangle index into the master index buffer, payload ]
// The payload depends on the channel:
//      GPU_TRIS_AREA_LIGHT : node intensity * triangle area
//      GPU_TRIS_SKY        : triangle area
//
// The channel is also packed into the BVH leaves (see Geometry_Assets) so a
// shader can tell what kind of surface a ray hit (eg: a sun ray reaching sky).
//
// BVH leaf packing (Node.padding in the shaders, two triangles per leaf):
//      padding.x : prim 0 triangle no  | lightmap no << 16
//      padding.y : prim 0 material grp | gpu list    << 16
//      padding.z : prim 1 triangle no  | lightmap no << 16
//      padding.w : prim 1 material grp | gpu list    << 16
//
// eg, did a ray hit sky:
//      uint gpu_list = (floatBitsToUint(global_nodes[node_i].padding.y) >> 16) & 0xFFFF;
//      bool hit_sky = (gpu_list == 1);     //GPU_TRIS_SKY
//
// To add a channel: add an enum value, set it on a Material_Group, add a
// payload case in Geometry_Assets, and an output port in Geometry_Module.
//

enum GPU_Triangle_List
{
    GPU_TRIS_NONE = -1,
    GPU_TRIS_AREA_LIGHT = 0,
    GPU_TRIS_SKY,
    GPU_TRIS_COUNT
};

//value packed into the BVH for triangles not in any list
#define GPU_TRIS_NONE_PACKED 0xFFFF

struct GPU_Triangle_List_Data
{
    std::vector<aligned_uint> data;     //[triangle_idx, payload] pairs

    unsigned int count() const { return (unsigned int)(data.size() / 2); }
    bool empty() const { return data.empty(); }

    void push(unsigned int triangle_idx, unsigned int payload)
    {
        data.push_back(aligned_uint{ triangle_idx });
        data.push_back(aligned_uint{ payload });
    }
};

#endif

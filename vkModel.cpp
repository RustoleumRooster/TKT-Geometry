
#include <irrlicht.h>
#include <vulkan/vulkan.h>

#include "vkModel.h"
#include "Reflection.h"
#include "vkTextures.h"

#include <fstream>
#include <vector>
#include <iostream>

#include "BufferManager.h"

using namespace std;

typedef CMeshBuffer<video::S3DVertex2TCoords> mesh_buffer_type;

void writeLightmapsInfo(const vector<TextureMaterial>& materials_used, std::vector<LightMaps_Info_Struct>& dest, MeshNode_Interface_Final* meshnode)
{
    dest.resize(materials_used.size());

    for (int i = 0; i < materials_used.size(); i++)
    {
        int n_faces = materials_used[i].faces.size();

        dest[i].faces.resize(n_faces);
        dest[i].first_triangle.resize(n_faces);
        dest[i].n_triangles.resize(n_faces);
       // dest[i].quads.resize(n_faces);
        dest[i].type = (u16)materials_used[i].materialGroup;
        dest[i].has_lightmap_coords = materials_used[i].has_lightmap;
        dest[i].lightmap_no = materials_used[i].lightmap_no;

        dest[i].size = materials_used[i].lightmap_size;

        for (int j = 0; j < n_faces; j++)
        {
            int f_i = materials_used[i].faces[j];
            dest[i].faces[j] = f_i;

            MeshBuffer_Chunk chunk = meshnode->get_mesh_buffer(f_i);

            dest[i].first_triangle[j] = chunk.begin_i / 3;
            dest[i].n_triangles[j] = (chunk.end_i - chunk.begin_i) / 3;
            
            //cout << " " << j << ", zero = " << dest[i].first_triangle[j] << ", len is " << dest[i].n_triangles[j] << "\n";

           // dest[i].quads[j].verts[0] = materials_used[i].records[j].bounding_verts[0];
           // dest[i].quads[j].verts[1] = materials_used[i].records[j].bounding_verts[1];
           // dest[i].quads[j].verts[2] = materials_used[i].records[j].bounding_verts[2];
           // dest[i].quads[j].verts[3] = materials_used[i].records[j].bounding_verts[3];
        }
    }
    
}

void fill_vertex_struct(SMesh* mesh, soa_struct_2<aligned_vec3, aligned_vec3>& dest)
{
    u32 n = mesh->getMeshBufferCount();

    u32(*length)(SMesh*, u32);
    aligned_vec3 (*item0)(SMesh*, u32,u32);
    aligned_vec3 (*item1)(SMesh*, u32,u32);

    length = [](SMesh* mesh_, u32 index) -> u32 {
        return ((mesh_buffer_type*)mesh_->getMeshBuffer(index))->getVertexCount();
        };

    item0 = [](SMesh* mesh_, u32 i, u32 j) -> aligned_vec3 {
        return aligned_vec3{ ((mesh_buffer_type*)mesh_->getMeshBuffer(i))->Vertices[j].Pos };
        };

    item1 = [](SMesh* mesh_, u32 i, u32 j) -> aligned_vec3 {
        return aligned_vec3{ vector3df{ ((mesh_buffer_type*)mesh_->getMeshBuffer(i))->Vertices[j].TCoords2.X,
                                        (1.0f - ((mesh_buffer_type*)mesh_->getMeshBuffer(i))->Vertices[j].TCoords2.Y), 0} };
        };

    dest.fill_data(mesh, n, length, item0, item1);

}

void fill_index_struct(SMesh* mesh, soa_struct<aligned_uint>& dest)
{
    u32 n = mesh->getMeshBufferCount();

    u32(*length)(SMesh*, u32);
    aligned_uint(*item)(SMesh*, u32, u32);

    length = [](SMesh* mesh_, u32 index) -> u32 {
        return ((mesh_buffer_type*)mesh_->getMeshBuffer(index))->getIndexCount();
        };

    item = [](SMesh* mesh_, u32 i, u32 j) -> aligned_uint {
        return aligned_uint{ ((mesh_buffer_type*)mesh_->getMeshBuffer(i))->Indices[j]};
        };

    dest.fill_data(mesh, n, length, item);
}


void fill_vertex_struct(SMesh* mesh, soa_struct_2<aligned_vec3, aligned_vec3>& dest, const vector<bool>& include)
{
    u32 n = mesh->getMeshBufferCount();

    u32(*length)(SMesh*, u32);
    aligned_vec3(*item0)(SMesh*, u32, u32);
    aligned_vec3(*item1)(SMesh*, u32, u32);

    length = [](SMesh* mesh_, u32 index) -> u32 {
        return ((mesh_buffer_type*)mesh_->getMeshBuffer(index))->getVertexCount();
        };

    item0 = [](SMesh* mesh_, u32 i, u32 j) -> aligned_vec3 {
        return aligned_vec3{ ((mesh_buffer_type*)mesh_->getMeshBuffer(i))->Vertices[j].Pos };
        };

    item1 = [](SMesh* mesh_, u32 i, u32 j) -> aligned_vec3 {
        return aligned_vec3{ vector3df{ ((mesh_buffer_type*)mesh_->getMeshBuffer(i))->Vertices[j].TCoords2.X,
                                        (1.0f - ((mesh_buffer_type*)mesh_->getMeshBuffer(i))->Vertices[j].TCoords2.Y), 0} };
        };

    dest.fill_data(mesh, n, length, item0, item1, include);

}

void fill_index_struct(SMesh* mesh, soa_struct<aligned_uint>& dest, const vector<bool>& include)
{
    u32 n = mesh->getMeshBufferCount();

    u32(*length)(SMesh*, u32);
    aligned_uint(*item)(SMesh*, u32, u32);

    length = [](SMesh* mesh_, u32 index) -> u32 {
        return ((mesh_buffer_type*)mesh_->getMeshBuffer(index))->getIndexCount();
        };

    item = [](SMesh* mesh_, u32 i, u32 j) -> aligned_uint {
        return aligned_uint{ ((mesh_buffer_type*)mesh_->getMeshBuffer(i))->Indices[j] };
        };

    dest.fill_data(mesh, n, length, item, include);
}


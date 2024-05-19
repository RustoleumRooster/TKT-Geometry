#include <irrlicht.h>
#include "BVH.h"
#include "csg_classes.h"

using namespace irr;
using namespace scene;

std::vector<core::vector3df> vertices;
std::vector<u16> indices;
//std::vector<BVH_struct> nodes;

f32 calc_surface_area(f32* aabbMin, f32* aabbMax)
{
    return ((0.5 + aabbMax[0] - aabbMin[0]) * (0.5 + aabbMax[1] - aabbMin[1]) *0.001) +
        ((0.5 + aabbMax[0] - aabbMin[0]) * (0.5 + aabbMax[2] - aabbMin[2]) * 0.001)+
        ((0.5 + aabbMax[2] - aabbMin[2]) * (0.5 + aabbMax[1] - aabbMin[1]) * 0.001);
}

void BVH_node::addDrawLines(LineHolder& graph) const
{
    vector3df v0(aabbMin[0], aabbMin[1], aabbMin[2]);
    vector3df v1(aabbMax[0], aabbMin[1], aabbMin[2]);
    vector3df v2(aabbMin[0], aabbMax[1], aabbMin[2]);
    vector3df v3(aabbMin[0], aabbMin[1], aabbMax[2]);

    vector3df v4(aabbMax[0], aabbMax[1], aabbMax[2]);
    vector3df v5(aabbMin[0], aabbMax[1], aabbMax[2]);
    vector3df v6(aabbMax[0], aabbMin[1], aabbMax[2]);
    vector3df v7(aabbMax[0], aabbMax[1], aabbMin[2]);


    graph.lines.push_back(core::line3df(v0, v1));
    graph.lines.push_back(core::line3df(v0, v2));
    graph.lines.push_back(core::line3df(v0, v3));

    graph.lines.push_back(core::line3df(v4, v5));
    graph.lines.push_back(core::line3df(v4, v6));
    graph.lines.push_back(core::line3df(v4, v7));

    graph.lines.push_back(core::line3df(v1, v7));
    graph.lines.push_back(core::line3df(v2, v5));
    graph.lines.push_back(core::line3df(v3, v6));

    graph.lines.push_back(core::line3df(v1, v6));
    graph.lines.push_back(core::line3df(v2, v7));
    graph.lines.push_back(core::line3df(v3, v5));

}


void make_BVH(SMesh* mesh)
{
    u16 nVerts = 0;

    for (int i = 0; i < mesh->getMeshBufferCount(); i++)
        nVerts += mesh->getMeshBuffer(i)->getVertexCount();

    vertices.resize(nVerts);

    int n = 0;
    for (int i = 0; i < mesh->getMeshBufferCount(); i++) 
    {
        CMeshBuffer<video::S3DVertex2TCoords>* mesh_buffer = (CMeshBuffer<video::S3DVertex2TCoords>*)mesh->getMeshBuffer(i);

        for (int j = 0; j < mesh_buffer->getVertexCount(); j++) 
        {
            vertices[n] = mesh_buffer->Vertices[j].Pos;
            n++;
        }
    }

    uint16_t nIndices = 0;

    for (int i = 0; i < mesh->getMeshBufferCount(); i++)
        nIndices += mesh->getMeshBuffer(i)->getIndexCount();

    indices.resize(nIndices);

    n = 0;
    for (int i = 0; i < mesh->getMeshBufferCount(); i++) 
    {
        CMeshBuffer<video::S3DVertex2TCoords>* mesh_buffer = (CMeshBuffer<video::S3DVertex2TCoords>*)mesh->getMeshBuffer(i);

        for (int j = 0; j < mesh_buffer->getIndexCount(); j++) 
        {
            indices[n] = mesh_buffer->Indices[j];
            n++;
        }
    }

    /*BVH_struct bvh;
    bvh.bbox.set(vertices[0]);

    for (int i = 0; i < nIndices / 3; i++)
    {

        bvh.bbox.add_point(vertices[indices[3*i]]);
        bvh.bbox.add_point(vertices[indices[(3*i)+1]]);
        bvh.bbox.add_point(vertices[indices[(3*i)+2]]);

        bvh.triangles.push_back(i);
    }

    nodes.push_back(bvh);*/
}

void aabb_struct::textdump()
{
    std::cout << aabbMin[0] << "/" << aabbMax[0] << " " << aabbMin[1] << "/" << aabbMax[1] << " " << aabbMin[2] << "/" << aabbMax[2] << "\n";
}

void aabb_struct::addDrawLines(LineHolder& graph) const
{
    vector3df v0(aabbMin[0], aabbMin[1], aabbMin[2]);
    vector3df v1(aabbMax[0], aabbMin[1], aabbMin[2]);
    vector3df v2(aabbMin[0], aabbMax[1], aabbMin[2]);
    vector3df v3(aabbMin[0], aabbMin[1], aabbMax[2]);

    vector3df v4(aabbMax[0], aabbMax[1], aabbMax[2]);
    vector3df v5(aabbMin[0], aabbMax[1], aabbMax[2]);
    vector3df v6(aabbMax[0], aabbMin[1], aabbMax[2]);
    vector3df v7(aabbMax[0], aabbMax[1], aabbMin[2]);

    graph.lines.push_back(core::line3df(v0, v1));
    graph.lines.push_back(core::line3df(v0, v2));
    graph.lines.push_back(core::line3df(v0, v3));

    graph.lines.push_back(core::line3df(v4, v5));
    graph.lines.push_back(core::line3df(v4, v6));
    graph.lines.push_back(core::line3df(v4, v7));

    graph.lines.push_back(core::line3df(v1, v7));
    graph.lines.push_back(core::line3df(v2, v5));
    graph.lines.push_back(core::line3df(v3, v6));

    graph.lines.push_back(core::line3df(v1, v6));
    graph.lines.push_back(core::line3df(v2, v7));
    graph.lines.push_back(core::line3df(v3, v5));
}

#include <irrlicht.h>
#include <iostream>
#include "csg_classes.h"
#include "tolerances.h"
#include "geometry_scene.h"
#include "build_meshes.h"

using namespace irr;

#define PRINTV(x) x.X <<","<<x.Y<<","<<x.Z<<" "

void make_meshbuffer_from_triangles(const std::vector<triangle_holder>& triangles, scene::IMeshBuffer* buffer)
{
    if (!buffer)
        return;

    f32 pi = 3.141592653;

    core::array<video::S3DVertex2TCoords>* Vertices = (core::array<video::S3DVertex2TCoords>*) & ((scene::CMeshBuffer<video::S3DVertex2TCoords>*)buffer)->Vertices;
    core::array<u16>* Indices = (core::array<u16>*) & ((scene::CMeshBuffer<video::S3DVertex2TCoords>*)buffer)->Indices;

    Vertices->clear();
    Indices->clear();

    std::vector<core::vector3df> vecs;
    std::vector<u16> inds;

    int index_count = 0;
    int vertex_total = 0;

    for (const triangle_holder& t_h : triangles)
    {
        for (core::vector3df v : t_h.vertices)
        {
            vecs.push_back(v);
        }

        for (triangle T : t_h.triangles)
        {
            inds.push_back(T.A + vertex_total);
            inds.push_back(T.B + vertex_total);
            inds.push_back(T.C + vertex_total);
        }
        vertex_total = vecs.size();
    }

    Vertices->reallocate(vecs.size());
    Indices->reallocate(inds.size());

    for (core::vector3df v : vecs)
    {
        video::S3DVertex2TCoords vtx;
        vtx.Color.set(255, 255, 255, 255);
        vtx.Pos = v;
        Vertices->push_back(vtx);
    }

    for (u16 index : inds)
    {
        Indices->push_back(index);
    }

    buffer->recalculateBoundingBox();
}

void make_meshbuffer_from_triangles(const triangle_holder& t_h, scene::IMeshBuffer* buffer)
{
    if (!buffer)
        return;

    f32 pi = 3.141592653;

    core::array<video::S3DVertex2TCoords>* Vertices = (core::array<video::S3DVertex2TCoords>*) & ((scene::CMeshBuffer<video::S3DVertex2TCoords>*)buffer)->Vertices;
    core::array<u16>* Indices = (core::array<u16>*) & ((scene::CMeshBuffer<video::S3DVertex2TCoords>*)buffer)->Indices;

    Vertices->clear();
    Indices->clear();

    std::vector<core::vector3df> vecs;
    std::vector<u16> inds;

    int index_count = 0;
    int vertex_total = 0;

    for (core::vector3df v : t_h.vertices)
    {
        vecs.push_back(v);
    }

    for (triangle T : t_h.triangles)
    {
        inds.push_back(T.A + vertex_total);
        inds.push_back(T.B + vertex_total);
        inds.push_back(T.C + vertex_total);
    }
    vertex_total = vecs.size();

    Vertices->reallocate(vecs.size());
    Indices->reallocate(inds.size());

    for (core::vector3df v : vecs)
    {
        video::S3DVertex2TCoords vtx;
        vtx.Color.set(255, 255, 255, 255);
        vtx.Pos = v;
        Vertices->push_back(vtx);
    }

    for (u16 index : inds)
    {
        Indices->push_back(index);
    }

    buffer->recalculateBoundingBox();
}


void calculate_meshbuffer_uvs_cylinder(GeometryStack* g_scene, int e_i, int f_i, scene::IMeshBuffer* buffer)
{
    calculate_meshbuffer_uvs_cylinder(g_scene, e_i, f_i, buffer, 0, buffer->getVertexCount());
}

void calculate_meshbuffer_uvs_cylinder(GeometryStack* g_scene, int e_i, int f_i, scene::IMeshBuffer* buffer, int v_start, int v_end)
{
    polyfold* brush = &g_scene->get_element_by_id(e_i)->brush;

    poly_face* face = &brush->faces[f_i];

    surface_group sfg = brush->surface_groups[face->surface_group];

    video::S3DVertex2TCoords* vtx;

    core::vector3df iY = sfg.vec.crossProduct(sfg.vec1);
    iY.normalize();
    core::vector2df k, k0;

    for (int i = v_start; i < v_end; i++)
    {
        u16 idx = buffer->getIndices()[i];
        vtx = &((video::S3DVertex2TCoords*)buffer->getVertices())[idx];

        core::vector3df r = vtx->Pos - sfg.point;

        k.X = r.dotProduct(sfg.vec1);
        k.Y = r.dotProduct(iY);

        f32 theta = k.getAngleTrig();

        if (fabs(theta - 360.0) < 0.001 || fabs(theta) < 0.001)
        {
            if (face->m_normal.dotProduct(iY) * r.dotProduct(face->m_normal) > 0)
                theta = 0.0;
            else
                theta = 360.0;
        }

        core::vector3df uv(theta / 90, vtx->Pos.dotProduct(sfg.vec) / 128, 0);
        face->uv_mat.transformVect(uv);
        face->uv_mat.translateVect(uv);
        vtx->TCoords.set(-uv.X, uv.Y);
    }
    /*
        for(int i=0; i<buffer->getVertexCount(); i++)
        {
            vtx = &((video::S3DVertex*)buffer->getVertices())[i];
            core::vector3df r = vtx->Pos - sfg.point;
            f32 h = r.dotProduct(sfg.vec);
            core::vector3df R = vtx->Pos - (sfg.point + sfg.vec*h);

            R.normalize();
            vtx->Normal = R;

            f32 theta = acos(R.dotProduct(sfg.vec1));
            //f32 theta = a.getAngleTrig();
            core::vector3df m = R.crossProduct(sfg.vec1);
            //m.normalize();
            bool b = m.dotProduct(sfg.vec) > 0;

            if(b)
                theta =  -theta;

            //std::cout<<i<<" "<<R.X<<" "<<R.Z<<" "<<theta<<" "<<b<<"\n";
            std::cout<<i<<" "<<theta<<" "<<b<<"\n";

            core::vector3df uv(theta *4  / 3.1459 , h/128,0);

            face->uv_mat.transformVect(uv);
            face->uv_mat.translateVect(uv);

            vtx->TCoords.set(uv.X,uv.Y);
        }
        */
}

void calculate_meshbuffer_uvs_dome(GeometryStack* g_scene, int e_i, int f_i, scene::IMeshBuffer* buffer)
{
    calculate_meshbuffer_uvs_dome(g_scene, e_i, f_i, buffer, 0, buffer->getVertexCount());
}

void calculate_meshbuffer_uvs_dome(GeometryStack* g_scene, int e_i, int f_i, scene::IMeshBuffer* buffer, int v_start, int v_end)
{
    polyfold* brush = &g_scene->elements[e_i].brush;

    poly_face* face = &brush->faces[f_i];

    surface_group sfg = brush->surface_groups[face->surface_group];

    video::S3DVertex2TCoords* vtx;

    core::vector3df vec3 = sfg.vec.crossProduct(sfg.vec1);
    vec3.normalize();

    for (int i = v_start; i < v_end; i++)
    {
        u16 idx = buffer->getIndices()[i];
        vtx = &((video::S3DVertex2TCoords*)buffer->getVertices())[idx];

        core::vector3df r = vtx->Pos - sfg.point;
        r.normalize();
        core::vector3df rr = sfg.vec.crossProduct(r);
        rr.normalize();

        f32 alpha = acos(r.dotProduct(sfg.vec));
        f32 theta = rr.dotProduct(sfg.vec1);
        f32 theta1 = rr.dotProduct(vec3);
        core::vector3df uv = core::vector3df(alpha * theta, alpha * theta1, 0);

        face->uv_mat.transformVect(uv);
        face->uv_mat.translateVect(uv);

        vtx->TCoords.set(uv.X, uv.Y);
    }
}

void calculate_meshbuffer_uvs_sphere(GeometryStack* g_scene, int e_i, int f_i, scene::IMeshBuffer* buffer)
{
    calculate_meshbuffer_uvs_sphere(g_scene, e_i, f_i, buffer, 0, buffer->getVertexCount());
}

void calculate_meshbuffer_uvs_sphere(GeometryStack* g_scene, int e_i, int f_i, scene::IMeshBuffer* buffer, int v_start, int v_end)
{
    polyfold* brush = &g_scene->elements[e_i].brush;

    poly_face* face = &brush->faces[f_i];

    surface_group sfg = brush->surface_groups[face->surface_group];
    core::vector3df N = face->m_normal;

    video::S3DVertex2TCoords* vtx;

    //core::vector3df vec3 = sfg.vec.crossProduct(sfg.vec1);
    //vec3.normalize();

    for (int i = v_start; i < v_end; i++)
    {
        u16 idx = buffer->getIndices()[i];
        vtx = &((video::S3DVertex2TCoords*)buffer->getVertices())[idx];

        core::vector3df r = vtx->Pos - sfg.point;
        r.normalize();
        core::vector3df rr = sfg.vec.crossProduct(r);
        rr.normalize();

        f32 alpha = acos(r.dotProduct(sfg.vec));
        f32 theta = acos(rr.dotProduct(sfg.vec1));
        /*
                if(fabs(alpha)<0.001 || fabs(alpha-pi)<0.001)
                    {
                        core::vector3df centerv(0,0,0);
                        for(int j=0;j<t_h.vertices.size();j++)
                        {
                            centerv += t_h.vertices[j];
                        }
                        centerv /= t_h.vertices.size();

                        r = centerv-t_h.surface_info.point;
                        r.normalize();
                        rr = t_h.surface_info.vec.crossProduct(r);
                        rr.normalize();
                        theta = acos(rr.dotProduct(t_h.surface_info.vec1));
                    }
        */
        core::vector3df uv = core::vector3df(theta * 4.0 / 3.1459, alpha * 4.0 / 3.1459, 0);

        face->uv_mat.transformVect(uv);
        face->uv_mat.translateVect(uv);

        vtx->TCoords.set(uv.X, uv.Y);
    }
}


void calculate_meshbuffer_uvs_cube(GeometryStack* g_scene, int e_i, int f_i, scene::IMeshBuffer* buffer)
{
    calculate_meshbuffer_uvs_cube(g_scene, e_i, f_i, buffer, 0, buffer->getVertexCount());
}


void calculate_meshbuffer_uvs_canonical(GeometryStack* g_scene, int e_i, int f_i, scene::IMeshBuffer* buffer, int v_start, int v_end)
{
    polyfold* brush = &g_scene->elements[e_i].brush;

    poly_face* face = &brush->faces[f_i];

    surface_group sfg = brush->surface_groups[face->surface_group];
    core::vector3df N = face->m_normal;

    aligned_vec3 w{ { 0, 0, 0 } };

    video::S3DVertex2TCoords* vtx;

    for (int i = v_start; i < v_end; i++)
    {
        u16 idx = buffer->getIndices()[i];
        vtx = &((video::S3DVertex2TCoords*)buffer->getVertices())[idx];

        if (sfg.c_brush.map_point(brush, face->face_id, vtx->Pos, w) == false)
            cout << "mapping error\n";

        face->uv_mat.transformVect(w.V);
        face->uv_mat.translateVect(w.V);

        vtx->TCoords.set(w.V.X, w.V.Y);
        vtx->Normal = N;
    }
}

void calculate_meshbuffer_uvs_cube(GeometryStack* g_scene, int e_i, int f_i, scene::IMeshBuffer* buffer, int v_start, int v_end)
{
    polyfold* brush = &g_scene->elements[e_i].brush;

    poly_face* face = &brush->faces[f_i];

    surface_group sfg = brush->surface_groups[face->surface_group];
    core::vector3df N = face->m_normal;

    core::vector3df w(0, 0, 0);
    core::vector3df iZ = is_parallel_normal(sfg.vec, N) ? sfg.vec1.crossProduct(N) : sfg.vec.crossProduct(N);
    iZ.normalize();

    core::vector3df w0(0, 0, 0);
    core::vector3df iX = iZ.crossProduct(N);

    w0.X = face->uv_origin.dotProduct(iZ) / 128;
    w0.Y = face->uv_origin.dotProduct(iX) / 128;

    core::vector3df scale;
    scale.X = face->uv_mat.getScale().X;
    scale.Y = face->uv_mat.getScale().Y;
    scale.X = fmax(0.01, scale.X);
    scale.Y = fmax(0.01, scale.Y);
    scale.Z = 1;

    video::S3DVertex2TCoords* vtx;

    for (int i = v_start; i < v_end; i++)
    {
        u16 idx = buffer->getIndices()[i];
        vtx = &((video::S3DVertex2TCoords*)buffer->getVertices())[idx];

        w.X = -vtx->Pos.dotProduct(iZ) / 128;
        w.Y = vtx->Pos.dotProduct(iX) / 128;

        face->uv_mat.transformVect(w);
        face->uv_mat.translateVect(w);

        w = (w - w0);
        vtx->TCoords.set(w.X, w.Y);
        vtx->Normal = N;
    }
}


void calculate_meshbuffer_uvs_custom(GeometryStack* g_scene, int e_i, int f_i, scene::IMeshBuffer* buffer)
{
    calculate_meshbuffer_uvs_custom(g_scene, e_i, f_i, buffer, 0, buffer->getVertexCount());
}

void calculate_meshbuffer_uvs_custom(GeometryStack* g_scene, int e_i, int f_i, scene::IMeshBuffer* buffer, int v_start, int v_end)
{
    polyfold* brush = &g_scene->elements[e_i].brush;
    polyfold* geometry = &g_scene->elements[e_i].geometry;

    poly_face* brush_face = &brush->faces[f_i];
    surface_group sfg = brush->surface_groups[brush_face->surface_group];
    core::vector3df N = brush_face->m_normal;

    core::vector3df w(0, 0, 0);
    core::vector3df iZ = is_parallel_normal(sfg.vec, N) ? sfg.vec1.crossProduct(N) : sfg.vec.crossProduct(N);
    iZ.normalize();

    core::vector3df w0(0, 0, 0);
    core::vector3df iX = iZ.crossProduct(N);

    w0.X = brush_face->uv_origin.dotProduct(iZ) / 128;
    w0.Y = brush_face->uv_origin.dotProduct(iX) / 128;

    core::vector3df scale;
    scale.X = brush_face->uv_mat.getScale().X;
    scale.Y = brush_face->uv_mat.getScale().Y;
    scale.X = fmax(0.01, scale.X);
    scale.Y = fmax(0.01, scale.Y);
    scale.Z = 1;

    video::S3DVertex2TCoords* vtx;

    for (int i = v_start; i < v_end; i++)
    {
        u16 idx = buffer->getIndices()[i];
        vtx = &((video::S3DVertex2TCoords*)buffer->getVertices())[idx];

        int v_i;

        if (sfg.type == SURFACE_GROUP_CUSTOM_UVS_GEOMETRY)
            v_i = geometry->find_point(vtx->Pos);
        else
            v_i = brush->find_point(vtx->Pos);

        if (v_i != -1)
        {
            for (int i = 0; i < sfg.texcoords.size(); i++)
            {
                if (sfg.texcoords[i].vertex == v_i)
                {
                    w.X = sfg.texcoords[i].texcoord.X;
                    w.Y = sfg.texcoords[i].texcoord.Y;

                    //std::cout << v_i << " " << idx << " " << w.X << "," << w.Y << "\n";
                    break;
                }
            }
        }

        brush_face->uv_mat.transformVect(w);
        brush_face->uv_mat.translateVect(w);

        vtx->TCoords.set(w.X, w.Y);
        vtx->Normal = N;
    }
}
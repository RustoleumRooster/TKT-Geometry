#ifndef _CLIP_FUNCTIONS_3_H_
#define _CLIP_FUNCTIONS_3_H_

#include <irrlicht.h>
#include "csg_classes.h"
#include "tolerances.h"
#include "BVH.h"

template<bool bAccelerate>
int do_intersections_and_bisect(polyfold& pf4, polyfold& pf5, LineHolder& graph);

int sync_vertices(polyfold& pf4, polyfold& pf5);


inline void do_self_intersections_inner_loop(polyfold& pf, u16 face_i, u16 face_j)
{
    poly_face& f = pf.faces[face_i];
    poly_face& f2 = pf.faces[face_j];

    core::vector3df i_point;
    core::vector3df i_vec;

    core::plane3df f_plane = core::plane3df(f.m_center, f.m_normal);
    core::plane3df f2_plane = core::plane3df(f2.m_center, f2.m_normal);

    if (f_plane.getIntersectionWithPlane(f2_plane, i_point, i_vec) && is_parallel_normal(f.m_normal, f2.m_normal) == false)
    {
        for (int e_i : pf.faces[face_i].edges)
        {
            bool b1 = false;
            bool b2 = false;

            for (int v_j : pf.faces[face_j].vertices)
                if (v_j == pf.edges[e_i].v0)
                    b1 = true;

            for (int v_j : pf.faces[face_j].vertices)
                if (v_j == pf.edges[e_i].v1)
                    b2 = true;

            if (b1 && b2)
            {
                core::vector3df v0 = pf.getVertex(e_i, 0).V + (pf.getVertex(e_i, 1).V - pf.getVertex(e_i, 0).V) * 0.5;

                if (pf.is_point_on_face(face_j, v0))
                {
                    pf.faces[face_j].addEdge(e_i);
                }
            }
        }

        for (int e_j : pf.faces[face_j].edges)
        {
            bool b1 = false;
            bool b2 = false;

            for (int v_i : pf.faces[face_i].vertices)
                if (v_i == pf.edges[e_j].v0)
                    b1 = true;

            for (int v_i : pf.faces[face_i].vertices)
                if (v_i == pf.edges[e_j].v1)
                    b2 = true;

            if (b1 && b2)
            {
                core::vector3df v0 = pf.getVertex(e_j, 0).V + (pf.getVertex(e_j, 1).V - pf.getVertex(e_j, 0).V) * 0.5;

                if (pf.is_point_on_face(face_i, v0))
                {
                    pf.faces[face_i].addEdge(e_j);
                }
            }
        }
    }
}

template<bool bAccelerate>
void do_self_intersections(polyfold& pf, const std::vector<BVH_intersection_struct>& faces_faces)
{
    if (pf.faces.size() == 0)
        return;

    if (bAccelerate)
        for (const BVH_intersection_struct& hit : faces_faces)
        {
            u16 face_i = hit.X;
            u16 face_j = hit.Y;

            if(face_i > face_j)
                do_self_intersections_inner_loop(pf, face_i, face_j);
        }
    else
        for (int face_i = 0; face_i < pf.faces.size() - 1; face_i++)
            for (int face_j = face_i + 1; face_j < pf.faces.size(); face_j++)
            {

                do_self_intersections_inner_loop(pf, face_i, face_j);
            }
}

template<bool bAccelerate>
void polyfold::classify_edge(int e_0)
{
    edges[e_0].conv = EDGE_UNDEF;
    std::vector<u16> nfaces;
    static std::vector<u16> hits;

    if (bAccelerate)
    {
        hits.clear();

        aabb_struct aabb;
        edges[e_0].grow(&aabb, vertices.data());

        faces_BVH.intersect(aabb, hits);

        for (u16 f_i : hits)
        {
            for (int e_i : this->faces[f_i].edges)
            {
                if (e_i == e_0)
                    nfaces.push_back(f_i);
            }
        }
    }
    else
    {
        for (int f_i = 0; f_i < this->faces.size(); f_i++)
        {
            for (int e_i : this->faces[f_i].edges)
            {
                if (e_i == e_0)
                    nfaces.push_back(f_i);
            }
        }
    }

    if (nfaces.size() == 1)
    {
        this->edges[e_0].conv = EDGE_SIMPLE;
        return;
    }
    else if (nfaces.size() > 2)
    {
        this->edges[e_0].conv = EDGE_COMPLEX;
        return;
    }
    else if (nfaces.size() == 0)
        return;

    std::vector<core::vector3df> face_vecs;

    core::vector3df v1 = this->getVertex(e_0, 0).V - this->getVertex(e_0, 1).V;

    for (int f_i : nfaces)
    {
        core::vector3df r = v1.crossProduct(this->faces[f_i].m_normal);
        r.normalize();

        if (this->is_point_on_face(f_i, this->getVertex(e_0, 1).V + v1 * 0.5 + r))
            face_vecs.push_back(r);
        else
            face_vecs.push_back(-r);
    }

    core::matrix4 R;
    v1.normalize();
    R.buildRotateFromTo(v1, core::vector3df(0, 1, 0));

    core::vector3df m1;
    core::vector3df m2;
    core::vector3df r1 = this->getVertex(e_0, 1).V + v1 * 0.5 + face_vecs[0];
    core::vector3df r2 = this->getVertex(e_0, 1).V + v1 * 0.5 + face_vecs[1];

    LineHolder nograph;

    R.rotateVect(m1, this->faces[nfaces[0]].m_normal);
    R.rotateVect(m2, this->faces[nfaces[1]].m_normal);

    R.rotateVect(r1);
    R.rotateVect(r2);
    m1.normalize();
    m2.normalize();

    core::line2df line1 = core::line2df(r1.X, r1.Z, r1.X + m1.X, r1.Z + m1.Z);
    core::line2df line2 = core::line2df(r2.X, r2.Z, r2.X + m2.X, r2.Z + m2.Z);

    core::vector2df ipoint;

    if (line1.intersectWith(line2, ipoint, false))
    {
        if (core::vector2df(m1.X, m1.Z).dotProduct(ipoint - core::vector2df(r1.X, r1.Z)) < 0
            && core::vector2df(m2.X, m2.Z).dotProduct(ipoint - core::vector2df(r2.X, r2.Z)) < 0)
        {
            this->edges[e_0].conv = EDGE_CONVEX;
        }
        else
        {
            this->edges[e_0].conv = EDGE_CONCAVE;
        }
    }
}

template<bool bAccelerate>
void polyfold::classify_edges(LineHolder& graph)
{
    for (int e_0 = 0; e_0 < this->edges.size(); e_0++)
    {
        classify_edge<bAccelerate>(e_0);
    }
}
#endif
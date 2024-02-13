
#include <irrlicht.h>
#include <iostream>
#include "csg_classes.h"
#include "clip_functions.h"
#include "clip_functions3.h"
#include "tolerances.h"

using namespace irr;

void common_topology_inner_loop_1(polyfold& pf, polyfold& pf2, polyfold& pf4, polyfold& pf5, u16 face_i, u16 face_j, LineHolder& graph)
{
    poly_face& f = pf.faces[face_i];
    poly_face& f2 = pf2.faces[face_j];

    core::vector3df i_point;
    core::vector3df i_vec;

    core::plane3df f_plane = core::plane3df(f.m_center, f.m_normal);
    core::plane3df f2_plane = core::plane3df(f2.m_center, f2.m_normal);

    if (f_plane.getIntersectionWithPlane(f2_plane, i_point, i_vec) && is_parallel_normal(f.m_normal, f2.m_normal) == false)
    {
        polyfold verts;
        for (int v_i : pf4.faces[face_i].vertices)
        {
            core::vector3df v = pf4.vertices[v_i].V;
            if (is_coplanar_point(f2_plane, v) && pf5.is_point_on_face(face_j, v))
            {
                verts.get_point_or_add(v);
                //graph.points.push_back(v);
            }
        }

        for (int v_j : pf5.faces[face_j].vertices)
        {
            core::vector3df v = pf5.vertices[v_j].V;
            if (is_coplanar_point(f_plane, v) && pf.is_point_on_face(face_i, v))
            {
                verts.get_point_or_add(v);
                //graph.points.push_back(v);
            }
        }

        if (verts.vertices.size() > 1)
        {
            sort_inline_vertices(verts);
            for (int i = 0; i < verts.vertices.size() - 1; i++)
            {
                //graph.points.push_back(verts.vertices[i+1].V);
                graph.lines.push_back(core::line3df(verts.vertices[i].V, verts.vertices[i + 1].V));

                core::vector3df r = verts.vertices[i].V + (verts.vertices[i + 1].V - verts.vertices[i].V) * 0.47;

                if (pf4.is_point_on_face(face_i, r))
                {
                    // graph.points.push_back(r);
                }

                if (pf.is_point_on_face(face_i, r) &&
                    (pf5.is_point_on_face(face_j, r)))
                {
                    int v0 = pf4.get_point_or_add(verts.vertices[i].V);
                    int v1 = pf4.get_point_or_add(verts.vertices[i + 1].V);
                    int w0 = pf5.get_point_or_add(verts.vertices[i].V);
                    int w1 = pf5.get_point_or_add(verts.vertices[i + 1].V);

                    // graph.points.push_back(verts.vertices[i].V);
                    // graph.points.push_back(verts.vertices[i+1].V);
                     //graph.lines.push_back(core::line3df(verts.vertices[i].V,verts.vertices[i+1].V));

                    if (pf4.find_edge(v0, v1) != -1)
                    {

                        // graph.lines.push_back(core::line3df(verts.vertices[i].V,verts.vertices[i+1].V));
                    }

                    int new_e;

                    if (pf4.find_edge(v0, v1) == -1)
                    {
                        new_e = pf4.get_edge_or_add(v0, v1, 3);
                        pf4.faces[face_i].addEdge(new_e);
                    }
                    else
                    {
                        new_e = pf4.find_edge(v0, v1);
                        pf4.edges[new_e].topo_group = 3;
                    }

                    if (pf5.find_edge(w0, w1) == -1)
                    {
                        new_e = pf5.get_edge_or_add(w0, w1, 3);
                        pf5.faces[face_j].addEdge(new_e);
                    }
                    else
                    {
                        new_e = pf5.find_edge(w0, w1);
                        pf5.edges[new_e].topo_group = 3;
                        pf5.faces[face_j].addEdge(new_e);
                    }

                }
            }
        }
    }
}

void common_topology_inner_loop_2(polyfold& pf, polyfold& pf2, polyfold& pf4, polyfold& pf5, u16 face_i, u16 face_j, LineHolder& graph)
{
    poly_face& f = pf.faces[face_i];
    poly_face& f2 = pf2.faces[face_j];

    core::plane3df f_plane = core::plane3df(f.m_center, f.m_normal);
    core::plane3df f2_plane = core::plane3df(f2.m_center, f2.m_normal);

    if (is_coplanar_point(f_plane, pf5.faces[face_j].m_center) && is_parallel_normal(pf4.faces[face_i].m_normal, pf2.faces[face_j].m_normal))
    {
        for (int e_i : pf5.faces[face_j].edges)
        {
            core::vector3df v0 = pf5.getVertex(e_i, 0).V;
            core::vector3df v1 = pf5.getVertex(e_i, 1).V;
            core::vector3df r = v0 + (v1 - v0) * 0.5;
            int v_i0 = pf4.find_point(v0);
            int v_i1 = pf4.find_point(v1);

            if (v_i0 != -1 && v_i1 != -1 && pf.is_point_on_face(face_i, r))
            {
                int new_e = pf4.get_edge_or_add(v_i0, v_i1, 3);
                pf4.edges[new_e].topo_group = 3;
                pf4.faces[face_i].addEdge(new_e);
            }
            //graph.lines.push_back(core::line3df(v0,v1));
        }
        for (int e_i : pf4.faces[face_i].edges)
        {
            core::vector3df v0 = pf4.getVertex(e_i, 0).V;
            core::vector3df v1 = pf4.getVertex(e_i, 1).V;
            core::vector3df r = v0 + (v1 - v0) * 0.5;
            int v_i0 = pf5.find_point(v0);
            int v_i1 = pf5.find_point(v1);

            if (v_i0 != -1 && v_i1 != -1 && pf2.is_point_on_face(face_j, r))
            {
                int new_e = pf5.get_edge_or_add(v_i0, v_i1, 3);
                pf5.edges[new_e].topo_group = 3;
                pf5.faces[face_j].addEdge(new_e);
            }
        }
    }
}

int do_intersections_and_bisect(polyfold& pf, polyfold& pf2, polyfold& pf4, polyfold& pf5, poly_intersection_info& intersect_info, LineHolder& graph)
{
    int n_intersections=0;

    for(poly_edge &edge : pf4.edges)
    {
        edge.topo_group=2;
    }

    for(poly_edge &edge : pf5.edges)
    {
        edge.topo_group=2;
    }

    //intersections and bisect - pf4
#ifdef BVH_OPTIMIZE
    for(int i=0;i<intersect_info.edges_faces.size();i++)
    {
        u16 e_i = intersect_info.edges_faces[i].X;
        u16 face_j = intersect_info.edges_faces[i].Y;
#else
    for(int face_j=0;face_j<pf5.faces.size();face_j++)
    {
#endif

        poly_face& f2=pf5.faces[face_j];
        core::plane3df f2_plane = core::plane3df(f2.m_center,f2.m_normal);
        core::vector3df ipoint;

#ifndef BVH_OPTIMIZE
        for(int e_i=0;e_i<pf4.edges.size();e_i++)
#endif
        {

            core::vector3df v0=pf4.getVertex(e_i,0).V;
            core::vector3df v1=pf4.getVertex(e_i,1).V;

            if(line_intersects_plane(f2_plane,v0,v1,ipoint) && pf4.edges[e_i].topo_group != -1
               && pf5.is_point_on_face(face_j,ipoint))
            {
                //core::vector3df linevec = v0-v1;
                //linevec.normalize();
                int new_v = pf4.get_point_or_add(ipoint);
                if (pf4.bisect_edge(e_i, new_v, 2, 2))
                {
#ifdef BVH_OPTIMIZE
                    for (int j = 0; j < intersect_info.edges_faces.size(); j++)
                    {
                        if (intersect_info.edges_faces[j].X == e_i)
                        {
                            intersect_info.edges_faces[j].X = static_cast<u16>(pf4.edges.size() - 1);
                            intersect_info.edges_faces.push_back(BVH_intersection_struct{ static_cast<u16>(pf4.edges.size() - 2),intersect_info.edges_faces[j].Y });
                        }
                    }
#endif
                }
                n_intersections++;
            }
        }
    }

    //intersections and bisect - pf5
#ifdef BVH_OPTIMIZE
    for(int i=0;i< intersect_info.faces_edges.size();i++)
    {
        u16 face_j = intersect_info.faces_edges[i].X;
        u16 e_i = intersect_info.faces_edges[i].Y;
#else
    for(u16 face_j=0;face_j<pf.faces.size();face_j++)
    {
#endif
        poly_face& f=pf.faces[face_j];
        core::plane3df f_plane = core::plane3df(f.m_center,f.m_normal);
        core::vector3df ipoint;

#ifndef BVH_OPTIMIZE
        for(u16 e_i=0; e_i<pf5.edges.size(); e_i++)
#endif
        {
            
            core::vector3df v0=pf5.getVertex(e_i,0).V;
            core::vector3df v1=pf5.getVertex(e_i,1).V;
            if(line_intersects_plane(f_plane,v0,v1,ipoint) && pf5.edges[e_i].topo_group != -1
               && pf.is_point_on_face(face_j,ipoint))
            {
                //    graph.lines.push_back(core::line3df(pf5.getVertex(e_i, 0).V, pf5.getVertex(e_i, 1).V));
                int new_v = pf5.get_point_or_add(ipoint);
                if (pf5.bisect_edge(e_i, new_v, 2, 2))
                {
#ifdef BVH_OPTIMIZE
                    for (int j = 0; j < intersect_info.faces_edges.size(); j++)
                    {
                        if (intersect_info.faces_edges[j].Y == e_i)
                        {
                            intersect_info.faces_edges[j].Y = static_cast<u16>(pf5.edges.size() - 1);
                            intersect_info.faces_edges.push_back(BVH_intersection_struct{ intersect_info.faces_edges[j].X,static_cast<u16>(pf5.edges.size() - 2) });
                        }
                    }
#endif
                }
                n_intersections++;
            }
        }
    }
    
    for(int v_i=0; v_i<pf5.vertices.size(); v_i++)
    {
        if(pf4.find_point(pf5.vertices[v_i].V) == -1)
           {
            int new_v = -1;
            for(int f_i=0; f_i<pf4.faces.size(); f_i++)
                {
                    if(pf4.point_is_coplanar(f_i,pf5.vertices[v_i].V) &&
                       pf4.is_point_on_face(f_i,pf5.vertices[v_i].V))
                    {
                        new_v = pf4.get_point_or_add(pf5.vertices[v_i].V);
                        pf4.faces[f_i].addVertex(new_v);
                    }
                }
            if(new_v != -1)
                for(int e_i=0; e_i<pf4.edges.size(); e_i++)
                    {
                        if(pf4.point_is_on_edge(e_i,pf5.vertices[v_i].V))
                        {
                            pf4.bisect_edge(e_i,new_v,2,2);
                            n_intersections++;
                        }
                    }
           }
    }

    for(int v_i=0; v_i<pf4.vertices.size(); v_i++)
    {
        if(pf5.find_point(pf4.vertices[v_i].V) == -1)
        {
        int new_v=-1;
        for(int f_i=0; f_i<pf5.faces.size(); f_i++)
            {
                if(pf5.point_is_coplanar(f_i,pf4.vertices[v_i].V) &&
                   pf5.is_point_on_face(f_i,pf4.vertices[v_i].V))
                {
                    new_v = pf5.get_point_or_add(pf4.vertices[v_i].V);
                    pf5.faces[f_i].addVertex(new_v);
                }
            }

        if(new_v != -1)
            for(int e_i=0; e_i<pf5.edges.size(); e_i++)
            {
                if(pf5.point_is_on_edge(e_i,pf4.vertices[v_i].V))
                {
                    pf5.bisect_edge(e_i,new_v,2,2);
                    n_intersections++;
                }
            }
        }
    }

    return n_intersections;
}

void do_self_topology_loops(polyfold& pf, const polyfold& pf0, LineHolder &graph)
{
    LineHolder nograph;

    for (int f_i = 0; f_i < pf.faces.size(); f_i++)
    {
        if (pf.faces[f_i].temp_b == FACE_GEOMETRY_CHANGED)
        {
            for (int p_i = 0; p_i < pf.faces[f_i].loops.size(); p_i++)
            {
                core::vector3df v1;

                pf.set_loop_solid(f_i, p_i);
                pf.faces[f_i].loops[p_i].topo_group = LOOP_UNDEF;

                if (pf.faces[f_i].loops[p_i].type == LOOP_INNER && pf.get_point_in_loop(f_i, p_i, v1, nograph))
                {
                    graph.points.push_back(v1);
                    if (pf0.is_point_on_face(f_i, v1))
                    {
                        pf.faces[f_i].loops[p_i].topo_group = LOOP_SOLID;
                    }
                    else
                    {
                        pf.faces[f_i].loops[p_i].topo_group = LOOP_HOLLOW;

                    }
                }
                else if (pf.faces[f_i].loops[p_i].type == LOOP_INNER)
                {
                    std::cout << "*warning pf " << f_i << ", " << p_i << "\n";
                }
            }
        }
    }
}

void do_clear_redundant_inner_loops(polyfold& pf, int f_i)
{
    pf.sort_loops_inner(f_i);

    int depth=0;

    for(int p=0;p<pf.faces[f_i].loops.size();p++)
    {
        if(pf.faces[f_i].loops[p].vertices.size() > 0 && pf.faces[f_i].loops[p].depth > depth)
            depth = pf.faces[f_i].loops[p].depth;
    }

    while(depth>0)
    {
        for(int p_i=0;p_i<pf.faces[f_i].loops.size();p_i++)
        {
            if(pf.faces[f_i].loops[p_i].vertices.size() > 0 && pf.faces[f_i].loops[p_i].depth == depth)
            {
                for(int p_j=0;p_j<pf.faces[f_i].loops.size();p_j++)
                {
                    if(p_j != p_i && pf.faces[f_i].loops[p_j].depth == depth-1 &&
                       pf.faces[f_i].loops[p_i].topo_group == pf.faces[f_i].loops[p_j].topo_group &&
                       pf.is_inner_loop(f_i,p_i,p_j))
                    {
                        pf.faces[f_i].loops[p_i].vertices.clear();
                    }
                }
            }
        }
        depth--;
    }
}

bool is_identical_loop(const polyfold& pf, const poly_loop& loop_a, const polyfold& pf2, const poly_loop& loop_b)
{
    if (loop_a.vertices.size() == loop_b.vertices.size())
    {
        for (int v_i : loop_a.vertices)
        {
            bool b = false;
            for (int v_j : loop_b.vertices)
            {
                if (is_same_point(pf.vertices[v_i].V, pf2.vertices[v_j].V) == true)
                    b = true;
            }
            if (!b)
                return false;
        }
        return true;
    }
    return false;
}

void polyfold::remove_empty_faces()
{
    std::vector<poly_face> new_faces;
    for(int f_i=0;f_i<this->faces.size();f_i++)
    {
        if(this->faces[f_i].loops.size() > 0)
        {
            new_faces.push_back(this->faces[f_i]);
        }
    }
    this->faces = new_faces;
}

void polyfold::sort_loops_inner(int f_i)
{
    poly_face* face = &this->faces[f_i];

    for (int i = 0; i < face->loops.size(); i++)
    {
        if (face->loops[i].vertices.size() > 0)
        {
            this->set_loop_solid(f_i, i);
            face->loops[i].depth = 0;
        }
    }

    for (int i = 0; i < face->loops.size(); i++)
        for (int j = i + 1; j < face->loops.size(); j++)
        {
            if (face->loops[i].vertices.size() == 0 || face->loops[j].vertices.size() == 0)
                continue;

            if (this->is_inner_loop(f_i, i, j))
                face->loops[i].depth++;
            else if (this->is_inner_loop(f_i, j, i))
                face->loops[j].depth++;
        }
}

bool polyfold::bisect_edge(int edge_no, int v_no, int g0, int g1)
{
    if (this->edges[edge_no].v0 == v_no)
    {
        return false;
    }
    else if (this->edges[edge_no].v1 == v_no)
    {
        return false;
    }
    else if (this->edges[edge_no].topo_group == -1)
        return false;

    this->edges.push_back(poly_edge(this->edges[edge_no].v0, v_no, g0));
    this->edges.push_back(poly_edge(this->edges[edge_no].v1, v_no, g1));

    int new_edge0 = this->edges.size() - 2;
    int new_edge1 = this->edges.size() - 1;

    this->edges[edge_no].topo_group = -1;
    core::vector3df v0 = this->getVertex(new_edge0, 0).V;
    core::vector3df v1 = this->getVertex(new_edge0, 1).V;

    v0 = this->getVertex(new_edge1, 0).V;
    v1 = this->getVertex(new_edge1, 1).V;

    for (poly_face& face : this->faces)
    {
        for (int i = 0; i < face.edges.size(); i++)
        {
            if (face.edges[i] == edge_no)
            {
                face.edges[i] = new_edge0;
                face.edges.push_back(new_edge1);
                face.addVertex(v_no);
            }
        }
    }
    return true;
}

int sync_vertices(polyfold& pf4, polyfold& pf5)
{
    int n_intersections = 0;

    for (int v_i = 0; v_i < pf5.vertices.size(); v_i++)
    {
        if (pf4.find_point(pf5.vertices[v_i].V) == -1)
        {
            int new_v = -1;
            for (int f_i = 0; f_i < pf4.faces.size(); f_i++)
            {
                if (pf4.point_is_coplanar(f_i, pf5.vertices[v_i].V) &&
                    pf4.is_point_on_face(f_i, pf5.vertices[v_i].V))
                {
                    new_v = pf4.get_point_or_add(pf5.vertices[v_i].V);
                    pf4.faces[f_i].addVertex(new_v);
                }
            }
            if (new_v != -1)
                for (int e_i = 0; e_i < pf4.edges.size(); e_i++)
                {
                    if (pf4.point_is_on_edge(e_i, pf5.vertices[v_i].V))
                    {
                        pf4.bisect_edge(e_i, new_v, 2, 2);
                        n_intersections++;
                    }
                }
        }
    }

    return n_intersections;
}

template<>
int do_intersections_and_bisect<true>(polyfold& pf4, polyfold& pf5, LineHolder& graph)
{
    int n_intersections = 0;

    for (poly_edge& edge : pf4.edges)
    {
        edge.topo_group = 2;
    }

    std::vector<core::plane3df> pf5_planes;
    pf5_planes.resize(pf5.faces.size());

    for (int face_j = 0; face_j < pf5.faces.size(); face_j++)
    {
        const poly_face& f = pf5.faces[face_j];
        pf5_planes[face_j] = core::plane3df(f.m_center, f.m_normal);
    }

    std::vector<u16> hits;
    core::vector3df ipoint;

    for (int e_i = 0; e_i < pf4.edges.size(); e_i++)
    {
        hits.clear();

        aabb_struct aabb;
        pf4.edges[e_i].grow(&aabb, pf4.vertices.data());
        pf5.faces_BVH.intersect(aabb, hits);

        for (u16 face_j : hits)
        {

            core::vector3df v0 = pf4.getVertex(e_i, 0).V;
            core::vector3df v1 = pf4.getVertex(e_i, 1).V;

            if (line_intersects_plane(pf5_planes[face_j], v0, v1, ipoint) && pf4.edges[e_i].topo_group != -1
                && pf5.is_point_on_face(face_j, ipoint))
            {
                int new_v = pf4.get_point_or_add(ipoint);
                if (pf4.bisect_edge(e_i, new_v, 2, 2))
                {

                }
                n_intersections++;
            }
        }
    }
    return n_intersections;
}

template<>
int do_intersections_and_bisect<false>(polyfold& pf4, polyfold& pf5, LineHolder& graph)
{
    int n_intersections = 0;

    for (poly_edge& edge : pf4.edges)
    {
        edge.topo_group = 2;
    }

    for (int face_j = 0; face_j < pf5.faces.size(); face_j++)
    {
        poly_face& f2 = pf5.faces[face_j];
        core::plane3df f2_plane = core::plane3df(f2.m_center, f2.m_normal);
        core::vector3df ipoint;
        for (int e_i = 0; e_i < pf4.edges.size(); e_i++)
        {
            core::vector3df v0 = pf4.getVertex(e_i, 0).V;
            core::vector3df v1 = pf4.getVertex(e_i, 1).V;

            if (line_intersects_plane(f2_plane, v0, v1, ipoint) && pf4.edges[e_i].topo_group != -1
                && pf5.is_point_on_face(face_j, ipoint))
            {
                int new_v = pf4.get_point_or_add(ipoint);
                if (pf4.bisect_edge(e_i, new_v, 2, 2))
                {

                }
                n_intersections++;
            }
        }
    }
    return n_intersections;
}

void meld_loops(polyfold& pf, int f_i)
{
    for (int e_i : pf.faces[f_i].edges)
        pf.edges[e_i].p2 = 0;

    int meld_count = 0;

    for (int p = 0; p < pf.faces[f_i].loops.size(); p++)
    {
        pf.set_loop_solid(f_i, p);

        std::vector<int> tempv = pf.faces[f_i].loops[p].vertices;
        tempv.push_back(tempv[0]);

        for (int i = 0; i < tempv.size() - 1; i++)
        {
            int e_i = pf.find_edge(f_i, tempv[i], tempv[i + 1]);

            if (tempv[i] == pf.edges[e_i].v0) //v0 to v1
            {
                if (pf.edges[e_i].p2 == 0)
                    pf.edges[e_i].p2 = 1;
                else if (pf.edges[e_i].p2 == 2)
                {
                    meld_count++;
                    pf.edges[e_i].p2 = 3;
                }
            }
            else //v1 to v0
            {
                if (pf.edges[e_i].p2 == 0)
                    pf.edges[e_i].p2 = 2;
                else if (pf.edges[e_i].p2 == 1)
                {
                    meld_count++;
                    pf.edges[e_i].p2 = 3;
                }
            }
        }
    }

    if (meld_count > 0)
    {
        std::vector<int> new_edges;

        for (int e_i : pf.faces[f_i].edges)
        {
            if (pf.edges[e_i].p2 == 1 || pf.edges[e_i].p2 == 2)
                new_edges.push_back(e_i);
        }

        pf.faces[f_i].edges = new_edges;

        for (int e_0 : pf.faces[f_i].edges)
        {
            if (pf.edges[e_0].p2 == 1)
                pf.edges[e_0].p2 = 2;
            else if (pf.edges[e_0].p2 == 2)
                pf.edges[e_0].p2 = 1;
        }

        pf.faces[f_i].loops.clear();

        LineHolder nograph;
        for (int e_0 : pf.faces[f_i].edges)
        {
            if (pf.edges[e_0].p2 < 3 && pf.edges[e_0].p2 != -1 && pf.edges[e_0].topo_group != -1)
            {
                pf.do_loops(f_i, e_0, nograph);
            }
        }

        //if (cull_inner_loops)
        for (int p = 0; p < pf.faces[f_i].loops.size(); p++)
        {
            if (pf.faces[f_i].loops[p].type == LOOP_OUTER)
            {
                pf.faces[f_i].loops[p].vertices.clear();
            }
        }
    }
}


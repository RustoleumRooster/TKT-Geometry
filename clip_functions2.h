#ifndef _CLIP_FUNCTIONS_2_H_
#define _CLIP_FUNCTIONS_2_H_

#include <irrlicht.h>
#include "csg_classes.h"
#include "clip_functions3.h"

void common_topology_inner_loop_1(polyfold& pf, polyfold& pf2, polyfold& pf4, polyfold& pf5, u16 face_i, u16 face_j, LineHolder& graph);
void common_topology_inner_loop_2(polyfold& pf, polyfold& pf2, polyfold& pf4, polyfold& pf5, u16 face_i, u16 face_j, LineHolder& graph);

template<bool bAccelerate>
void do_common_topology(polyfold& pf, polyfold& pf2, polyfold& pf4, polyfold& pf5, const std::vector<BVH_intersection_struct>& faces_faces, LineHolder& graph)
{

    if (bAccelerate)
        for (const BVH_intersection_struct& hit : faces_faces)
        {
            u16 face_i = hit.X;
            u16 face_j = hit.Y;

            common_topology_inner_loop_1(pf, pf2, pf4, pf5, face_i, face_j, graph);
        }
    else
        for (int face_i = 0; face_i < pf.faces.size(); face_i++)
            for (int face_j = 0; face_j < pf2.faces.size(); face_j++)
            {
                common_topology_inner_loop_1(pf, pf2, pf4, pf5, face_i, face_j, graph);
            }

    if (bAccelerate)
        for (const BVH_intersection_struct& hit : faces_faces)
        {
            u16 face_i = hit.X;
            u16 face_j = hit.Y;

            common_topology_inner_loop_2(pf, pf2, pf4, pf5, face_i, face_j, graph);
        }
    else
        for (int face_i = 0; face_i < pf.faces.size(); face_i++)
            for (int face_j = 0; face_j < pf2.faces.size(); face_j++)
            {

                common_topology_inner_loop_2(pf, pf2, pf4, pf5, face_i, face_j, graph);
            }
}

inline void initial_topology_inner_loop(polyfold& pf4, polyfold& pf5, const core::plane3df& f2_plane, u16 e_i, u16 face_j)
{
    static LineHolder nograph;
    core::vector3df ipoint;
    core::vector3df v0 = pf4.getVertex(e_i, 0).V;
    core::vector3df v1 = pf4.getVertex(e_i, 1).V;
    if (line_intersects_plane(f2_plane, v0, v1, ipoint) && pf4.edges[e_i].topo_group != -1
        && pf5.is_point_on_face(face_j, ipoint))
    {
        int new_v = pf4.find_point(ipoint);

        if (new_v == pf4.edges[e_i].v0)
        {
            v0 = pf4.getVertex(e_i, 0).V;
            v1 = pf4.getVertex(e_i, 1).V;
        }
        else
        {
            v1 = pf4.getVertex(e_i, 0).V;
            v0 = pf4.getVertex(e_i, 1).V;
        }

        if (pf4.edges[e_i].topo_group == 2)
        {
            if (pf5.is_point_on_face(face_j, ipoint))
            {
                int RES = pf5.classify_point(face_j, v0, v1, nograph);

                if (RES == TOP_FRONT)
                    pf4.edges[e_i].topo_group = 0;
                else if (RES == TOP_BEHIND)
                    pf4.edges[e_i].topo_group = 1;
            }
        }
    }
}

template<bool bAccelerate>
void do_initial_topology(polyfold& pf4, polyfold& pf5, const std::vector<BVH_intersection_struct>& edges_faces, LineHolder& graph)
{
    if (bAccelerate)
    {
        for (const BVH_intersection_struct& hit : edges_faces)
        {
            u16 e_i = hit.X;
            u16 face_j = hit.Y;

            poly_face& f2 = pf5.faces[face_j];
            core::plane3df f2_plane = core::plane3df(f2.m_center, f2.m_normal);

            initial_topology_inner_loop(pf4, pf5, f2_plane, e_i, face_j);
        }
    }
    else
    {
        for (int face_j = 0; face_j < pf5.faces.size(); face_j++)
        {
            poly_face& f2 = pf5.faces[face_j];
            core::plane3df f2_plane = core::plane3df(f2.m_center, f2.m_normal);

            for (int e_i = 0; e_i < pf4.edges.size(); e_i++)
            {
                initial_topology_inner_loop(pf4, pf5, f2_plane, e_i, face_j);
            }
        }
    }
}

template<bool bAccelerate>
void propagate_topo_group(polyfold& pf, int e_0)
{
    struct edge_point_pair {
        int edge;
        int vertex;
    };

    std::vector<edge_point_pair> edges_to_explore;

    edges_to_explore.push_back(edge_point_pair{ e_0, pf.edges[e_0].v0 });
    edges_to_explore.push_back(edge_point_pair{ e_0, pf.edges[e_0].v1 });

    auto no_common_topology = [&](const std::vector<int>& edges) -> bool 
    {
        for (int e_j : edges)
        {
            if (pf.edges[e_j].topo_group == 3)
                return false;
        }
        return true;
    };

    std::vector<int> c_edges;
    std::vector<edge_point_pair> all_new_edges;

    while (edges_to_explore.size() > 0)
    {
        all_new_edges.clear();
        
        for (edge_point_pair e : edges_to_explore)
        {
            c_edges.clear();

            if (bAccelerate)
                pf.get_all_edges_from_point_accelerated(e.edge, e.vertex, c_edges);
            else
                pf.get_all_edges_from_point(e.edge, e.vertex, c_edges);

            if(no_common_topology(c_edges))
                for (int e_j : c_edges)
                {
                    if (pf.edges[e_j].topo_group == 2)
                    {
                        pf.edges[e_j].topo_group = pf.edges[e_0].topo_group;
                        int new_vertex = pf.get_opposite_end(e_j, e.vertex);

                        all_new_edges.push_back(edge_point_pair{ e_j, new_vertex });
                    }
                }
        }

        edges_to_explore = all_new_edges;
    }
}

template<bool bAccelerate>
bool apply_topology_groups(polyfold& pf, polyfold& pf2, int default_group, LineHolder& graph)
{

    for (int e_i = 0; e_i < pf.edges.size(); e_i++)
    {
        if (pf.edges[e_i].topo_group == 0 || pf.edges[e_i].topo_group == 1)
        {
            propagate_topo_group<bAccelerate>(pf, e_i);
        }
    }
    
    bool found_one;
    do
    {
        found_one = false;
        for (int e_i = 0; e_i < pf.edges.size(); e_i++)
        {
            if (pf.edges[e_i].topo_group == 2)
            {
                found_one = true;

                core::vector3df v0 = pf.getVertex(e_i, 0).V;
                core::vector3df v1 = pf.getVertex(e_i, 1).V;

                int RES = pf2.classify_point(v0 + (v1 - v0) * 0.5, graph);

                if (RES == TOP_FRONT)
                {
                    pf.edges[e_i].topo_group = 0;
                }
                else if (RES == TOP_BEHIND)
                {
                    pf.edges[e_i].topo_group = 1;
                }
                else if (RES == TOP_UNDEF)
                {
                    if (pf2.topology == TOP_CONCAVE)
                    {
                        pf.edges[e_i].topo_group = TOP_BEHIND;
                    }
                    else if (pf2.topology == TOP_CONVEX)
                    {
                        pf.edges[e_i].topo_group = TOP_FRONT;
                    }
                    else //plane geometry
                    {
                        //TODO
                       // pf.edges[e_i].topo_group = TOP_BEHIND;
                    }
                }

                if (pf.edges[e_i].topo_group != 2)
                    propagate_topo_group<bAccelerate>(pf, e_i);
            }
        }
    } while (found_one == true);

    return true;
}

template<bool bAccelerate>
void do_topology_groups(polyfold& pf4, polyfold& pf2, int rule, int rule2, LineHolder& graph)
{
    apply_topology_groups<bAccelerate>(pf4, pf2, rule2, graph);

    if (rule == GEO_SUBTRACT)
        for (int e_i = 0; e_i < pf4.edges.size(); e_i++)
        {
            if (pf4.edges[e_i].topo_group == 1 || pf4.edges[e_i].topo_group == 0)
            {
                pf4.edges[e_i].topo_group = !pf4.edges[e_i].topo_group;
            }
        }
}



#endif
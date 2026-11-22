#include <irrlicht.h>
#include <iostream>
#include "csg_classes.h"
#include "geometry_scene.h"
#include "clip_functions.h"
#include "clip_functions2.h"
#include "clip_functions3.h"

#include "tolerances.h"
#include <chrono>
/*
#define START_TIMER() startTime = std::chrono::high_resolution_clock::now();
#define PRINT_TIMER(text) currentTime = std::chrono::high_resolution_clock::now(); \
    time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count(); \
    std::cout << "---------time (" <<#text<< "): " << time << "\n";
#define PRINT_TOTAL_TIME() currentTime = std::chrono::high_resolution_clock::now(); \
    time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - timeZero).count(); \
    std::cout << "---------total time: " << time << "\n";
    */



#define START_TIMER() startTime = std::chrono::high_resolution_clock::now();
#define START_TIMER_2() startTime2 = std::chrono::high_resolution_clock::now();

#define PRINT_TIMER(text) currentTime = std::chrono::high_resolution_clock::now(); \
    time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count(); \
    std::cout << "---------time (" <<#text<< "): " << time << "\n";

#define PRINT_TOTAL_TIME() currentTime = std::chrono::high_resolution_clock::now(); \
    time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - timeZero).count(); \
    std::cout << "---------total time: " << time << "\n";

#define INC_TIMER(base,thingy) currentTime = std::chrono::high_resolution_clock::now(); \
    base.thingy += std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count(); \
    base.total_time +=  std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

#define INC_TIMER_2(base,thingy) currentTime = std::chrono::high_resolution_clock::now(); \
    base.thingy += std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime2).count(); \
    base.total_time +=  std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime2).count();

#define RET_LOOP_UNKNOWN -1
#define RET_LOOP_POSITIVE 0
#define RET_LOOP_NEGATIVE 1

inline int make_result_inner_loop(const polyfold& pf4, polyfold& pf5, polyfold& pf2, u16 f_i, u16 f_j, u16 p_i, poly_face& f_p, poly_face& f_n, int rule, bool overwrite)
{
    LineHolder nograph;

    for (int p_j = 0; p_j < pf5.faces[f_j].loops.size(); p_j++)
    {
        if (pf5.faces[f_j].loops[p_j].vertices.size() > 0 &&
            pf5.faces[f_j].loops[p_j].topo_group != LOOP_UNDEF &&
            is_identical_loop(pf4, pf4.faces[f_i].loops[p_i], pf5, pf5.faces[f_j].loops[p_j]))
        {
            if (pf4.faces[f_i].m_normal.dotProduct(pf5.faces[f_j].m_normal) > 0)
            {
                if (pf5.faces[f_j].loops[p_j].topo_group == LOOP_SOLID && pf4.faces[f_i].loops[p_i].topo_group == LOOP_SOLID)
                {
                    if (pf4.faces[f_i].loops[p_i].flags == 0)
                    {
                        pf5.faces[f_j].loops[p_j].flags = 1;

                        if (overwrite)
                            return RET_LOOP_POSITIVE;
                        else
                            return RET_LOOP_NEGATIVE;
                    }
                    else
                    {
                        if (overwrite)
                            return RET_LOOP_NEGATIVE;
                        else
                            return RET_LOOP_POSITIVE;
                    }         
                }
                else if (pf5.faces[f_j].loops[p_j].topo_group == LOOP_HOLLOW && pf4.faces[f_i].loops[p_i].topo_group == LOOP_SOLID)
                {
                    core::vector3df v1;
                    if (pf5.get_facial_point(f_j, p_j, v1, nograph))
                    {
                        if (rule == GEO_ADD)
                        {
                            if (pf2.classify_point(v1, nograph) == TOP_FRONT)
                                return RET_LOOP_POSITIVE;
                            else
                                return RET_LOOP_NEGATIVE;
                        }
                        else
                        {
                            if (pf2.classify_point(v1, nograph) == TOP_FRONT)
                                return RET_LOOP_NEGATIVE;
                            else
                                return RET_LOOP_POSITIVE;
                        }
                    }
                }
                else if (pf5.faces[f_j].loops[p_j].topo_group == LOOP_SOLID && pf4.faces[f_i].loops[p_i].topo_group == LOOP_HOLLOW)
                {
                    return RET_LOOP_NEGATIVE;
                }
                else if (pf5.faces[f_j].loops[p_j].topo_group == LOOP_HOLLOW && pf4.faces[f_i].loops[p_i].topo_group == LOOP_HOLLOW)
                {
                    return RET_LOOP_NEGATIVE;
                }
            }
            else
            {
                if (pf5.faces[f_j].loops[p_j].topo_group == LOOP_SOLID && pf4.faces[f_i].loops[p_i].topo_group == LOOP_SOLID)
                {
                    return RET_LOOP_NEGATIVE;
                }
                else if (pf5.faces[f_j].loops[p_j].topo_group == LOOP_HOLLOW && pf4.faces[f_i].loops[p_i].topo_group == LOOP_SOLID)
                {
                    core::vector3df v1;
                    if (pf5.get_facial_point(f_j, p_j, v1, nograph))
                    {
                        if (rule == GEO_ADD)
                        {
                            if (pf2.classify_point(v1, nograph) == TOP_FRONT)
                                return RET_LOOP_POSITIVE;
                            else
                                return RET_LOOP_NEGATIVE;
                        }
                        else
                        {
                            if (pf2.classify_point(v1, nograph) == TOP_FRONT)
                                return RET_LOOP_NEGATIVE;
                            else
                                return RET_LOOP_POSITIVE;
                        }
                    }
                }
                else if (pf5.faces[f_j].loops[p_j].topo_group == LOOP_SOLID && pf4.faces[f_i].loops[p_i].topo_group == LOOP_HOLLOW)
                {
                    return RET_LOOP_NEGATIVE;
                }
                else if (pf5.faces[f_j].loops[p_j].topo_group == LOOP_HOLLOW && pf4.faces[f_i].loops[p_i].topo_group == LOOP_HOLLOW)
                {
                    return RET_LOOP_NEGATIVE;
                }
            }
        }
    }

    return RET_LOOP_UNKNOWN;
}

template<bool bAccelerateEdges, bool bAccelerateVertices>
void make_result8(polyfold& pf4, polyfold& pf5, polyfold& pf, polyfold& pf2, int rule, int rule2, bool overwrite, polyfold& result, clip_results& results, LineHolder& graph, const std::vector<BVH_intersection_struct>& faces_faces)
{

    LineHolder nograph;

    polyfold positive;
    polyfold negative;

    positive.vertices = pf4.vertices;
    negative.vertices = pf4.vertices;
    positive.edges = pf4.edges;
    negative.edges = pf4.edges;

    positive.faces.resize(pf4.faces.size());
    negative.faces.resize(pf4.faces.size());

    for (int f_i = 0; f_i < pf4.faces.size(); f_i++)
    {
        core::plane3df f_plane = core::plane3df(pf4.faces[f_i].m_center, pf4.faces[f_i].m_normal);

        poly_face& f_p = positive.faces[f_i];
        poly_face& f_n = negative.faces[f_i];

        f_p.m_center = pf4.faces[f_i].m_center;
        f_p.m_normal = pf4.faces[f_i].m_normal;
        f_n.m_center = pf4.faces[f_i].m_center;
        f_n.m_normal = pf4.faces[f_i].m_normal;

        if (pf4.faces[f_i].temp_b == FACE_GEOMETRY_CHANGED)
        {
            if (pf4.faces[f_i].loops.size() > 0)
            {
                for (int p_i = 0; p_i < pf4.faces[f_i].loops.size(); p_i++)
                {
                    if (pf4.faces[f_i].loops[p_i].vertices.size() > 0 && pf4.faces[f_i].loops[p_i].topo_group != LOOP_UNDEF)
                    {
                        for (int f_j = 0; f_j < pf5.faces.size(); f_j++)
                        {
                            if (is_coplanar_point(f_plane, pf5.faces[f_j].m_center) &&
                                is_parallel_normal(pf4.faces[f_i].m_normal, pf5.faces[f_j].m_normal))
                            {
                                int loop_res = make_result_inner_loop(pf4, pf5, pf2, f_i, f_j, p_i, f_p, f_n, rule, overwrite);

                                if(loop_res == RET_LOOP_POSITIVE)
                                {
                                    f_p.loops.push_back(pf4.faces[f_i].loops[p_i]);
                                    goto FinishedWithLoop;
                                }
                                else if (loop_res == RET_LOOP_NEGATIVE)
                                {
                                    f_n.loops.push_back(pf4.faces[f_i].loops[p_i]);
                                    goto FinishedWithLoop;
                                }
                            }
                        }

                        int RES = classify_loop<bAccelerateEdges>(pf4, f_i, p_i, pf5, rule, rule2, graph);

                        if (RES == TOP_FRONT)
                        {
                            if (pf4.faces[f_i].loops[p_i].topo_group == LOOP_SOLID)
                                f_p.loops.push_back(pf4.faces[f_i].loops[p_i]);
                            else if (pf4.faces[f_i].loops[p_i].topo_group == LOOP_HOLLOW)
                                f_n.loops.push_back(pf4.faces[f_i].loops[p_i]);
                        }
                        else if (RES == TOP_BEHIND)
                        {
                            f_n.loops.push_back(pf4.faces[f_i].loops[p_i]);
                        }
                    }
                FinishedWithLoop:
                    int unused_int = 0;
                }
            }
        }
        else
        {
            if (pf4.faces[f_i].loops.size() > 0)
            {
                for (int p_i = 0; p_i < pf4.faces[f_i].loops.size(); p_i++)
                {
                    int RES = classify_loop<bAccelerateEdges>(pf4, f_i, p_i, pf5, rule, rule2, graph);

                    if (RES == TOP_BEHIND)
                    {
                        pf.faces[f_i].loops.clear();
                    }
                }
            }
        }
    }

    positive.rebuild_faces_from_loops<bAccelerateEdges>(pf4);
    negative.rebuild_faces_from_loops<bAccelerateEdges>(pf4);

    for (int f_i = 0; f_i < positive.faces.size(); f_i++)
    {
        if (positive.faces[f_i].loops.size() > 1)
            meld_loops(positive, f_i);
    }

    for (int f_i = 0; f_i < negative.faces.size(); f_i++)
    {
        if (negative.faces[f_i].loops.size() > 1)
            meld_loops(negative, f_i);
    }

    pf.rebuild_loops_copy_verts<bAccelerateEdges && bAccelerateVertices>(pf4);

    pf.vertices = pf4.vertices;
    pf.edges = pf4.edges;

    int nPositive = 0;
    for (int i = 0; i < pf4.faces.size(); i++)
    {
        if (pf4.faces[i].temp_b == FACE_GEOMETRY_UNCHANGED)
            continue;

        poly_face& f = pf.faces[i];
        f.loops.clear();
       
        for (int p = 0; p < positive.faces[i].loops.size(); p++)
        {
            f.loops.push_back(positive.faces[i].loops[p]);
            f.loops[f.loops.size() - 1].topo_group = LOOP_SOLID;
        }

        for (int p = 0; p < negative.faces[i].loops.size(); p++)
        {
            f.loops.push_back(negative.faces[i].loops[p]);
            f.loops[f.loops.size() - 1].topo_group = LOOP_HOLLOW;
        }

    }

    if(bAccelerateEdges && bAccelerateVertices && pf.vertices.size() > 175)
        pf.finalize_clipped_poly<true>(pf4, graph);
    else
        pf.finalize_clipped_poly<false>(pf4, graph);
}


void clip_poly_accelerated2(polyfold& pf, polyfold& pf2, int rule, int base_type, clip_results& results, LineHolder& graph)
{
    LineHolder nograph;

    polyfold pf4 = pf;
    polyfold pf5 = pf2;

    auto startTime = std::chrono::high_resolution_clock::now();
    auto startTime2 = std::chrono::high_resolution_clock::now();
    auto timeZero = startTime;
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time;


        pf4.build_faces_BVH();


        pf5.build_faces_BVH();


        pf4.classify_edges<true>(nograph);


        pf5.classify_edges<true>(nograph);


        std::vector<BVH_intersection_struct> faces_faces_pf4;
        std::vector<BVH_intersection_struct> faces_faces_pf5;


        pf4.faces_BVH.intersect_2(pf4.faces_BVH, faces_faces_pf4);


        pf5.faces_BVH.intersect_2(pf5.faces_BVH, faces_faces_pf5);

        do_self_intersections<true>(pf4, faces_faces_pf4);

        do_self_intersections<true>(pf5, faces_faces_pf5);


        poly_intersection_info intersect_info;

    //START_TIMER()

        pf4.faces_BVH.intersect_2(pf5.faces_BVH, intersect_info.faces_faces);

    //INC_TIMER(intersect_bvh_time)

        int n_intersections = 0;


        n_intersections += do_intersections_and_bisect<true>(pf4, pf5, graph);



        n_intersections += do_intersections_and_bisect<true>(pf5, pf4, graph);


        n_intersections += sync_vertices(pf4, pf5);
        n_intersections += sync_vertices(pf5, pf4);

        results.n_intersections = n_intersections;

        if (n_intersections == 0)
        {
            return;
        }

        do_common_topology<true>(pf, pf2, pf4, pf5, intersect_info.faces_faces, graph);


        int N = pf4.edges.size() * pf5.faces.size();
        int M = pf5.edges.size() * pf4.faces.size();

        bool accelerate_left = false;
        bool accelerate_right = false;
    
        if ((N > 5000 && pf4.vertices.size() < pf5.vertices.size() * 0.5) || (N > 10000) || (pf4.vertices.size() > 300))
            accelerate_left = true;

        if ((M > 5000 && pf5.vertices.size() < pf4.vertices.size() * 0.5) || (M > 10000) || (pf5.vertices.size() > 300))
            accelerate_right = true;
        

        if (accelerate_left)
        {
            pf4.build_edges_BVH(); //cutoff is ~300 vertices
        }


        if (accelerate_right)
        {
            pf5.build_edges_BVH();
        }


        std::vector<BVH_intersection_struct> edges_faces_45;
        std::vector<BVH_intersection_struct> edges_faces_54;


        if (accelerate_left)
        {
            pf4.edges_BVH.intersect_2(pf5.faces_BVH, edges_faces_45);
            do_initial_topology<true>(pf4, pf5, edges_faces_45, nograph);
        }
        else
        {
            do_initial_topology<false>(pf4, pf5, edges_faces_45, nograph);
        }

        if (accelerate_right)
        {
            pf5.edges_BVH.intersect_2(pf4.faces_BVH, edges_faces_54);
            do_initial_topology<true>(pf5, pf4, edges_faces_54, nograph);
        }
        else
        {
            do_initial_topology<false>(pf5, pf4, edges_faces_54, nograph);
        }

        if (accelerate_left)
            do_topology_groups<true>(pf4, pf5, rule, base_type, nograph);
        else
            do_topology_groups<false>(pf4, pf5, rule, base_type, nograph);


        if (accelerate_right)
           do_topology_groups<true>(pf5, pf4, rule, base_type, nograph);
        else
            do_topology_groups<false>(pf5, pf4, rule, base_type, nograph);
    
        for (int f_i = 0; f_i < pf4.faces.size(); f_i++)
            pf4.faces[f_i].temp_b = FACE_GEOMETRY_UNCHANGED;

        for (int f_i = 0; f_i < pf5.faces.size(); f_i++)
            pf5.faces[f_i].temp_b = FACE_GEOMETRY_UNCHANGED;

        for (BVH_intersection_struct hit : intersect_info.faces_faces)
        {
            pf4.faces[hit.X].temp_b = FACE_GEOMETRY_CHANGED;
            pf5.faces[hit.Y].temp_b = FACE_GEOMETRY_CHANGED;
        }


        for (int f_i = 0; f_i < pf4.faces.size(); f_i++)
        {
            if (pf4.faces[f_i].temp_b == FACE_GEOMETRY_CHANGED)
                pf4.calc_loops(f_i, nograph);
        }

        for (int f_i = 0; f_i < pf5.faces.size(); f_i++)
        {
            if (pf5.faces[f_i].temp_b == FACE_GEOMETRY_CHANGED)
                pf5.calc_loops(f_i, nograph);
        }


        do_self_topology_loops(pf4, pf, nograph);


        do_self_topology_loops(pf5, pf2, nograph);


        //pf = pf4;
        //pf2 = pf5;
        //return;

        for (int f_i = 0; f_i < pf4.faces.size(); f_i++)
            for (int p_i = 0; p_i < pf4.faces[f_i].loops.size(); p_i++)
                pf4.faces[f_i].loops[p_i].flags = 0;

        for (int f_i = 0; f_i < pf5.faces.size(); f_i++)
            for (int p_i = 0; p_i < pf5.faces[f_i].loops.size(); p_i++)
                pf5.faces[f_i].loops[p_i].flags = 0;

        polyfold result1;
        polyfold result2;

        bool accelerate_left_vertices = (accelerate_left && pf4.vertices.size() > 80);
        bool accelerate_right_vertices = (accelerate_right && pf5.vertices.size() > 80);

        if (accelerate_left_vertices)
        {
            pf4.build_vertices_BVH();
        }


        if (accelerate_left_vertices)
        {
            make_result8<true, true>(pf4, pf5, pf, pf2, rule, true, base_type, result1, results, nograph, intersect_info.faces_faces);
        }
        else
        {
            if (accelerate_left)
                make_result8<true, false>(pf4, pf5, pf, pf2, rule, true, base_type, result1, results, nograph, intersect_info.faces_faces);
            else
                make_result8<false, false>(pf4, pf5, pf, pf2, rule, true, base_type, result1, results, nograph, intersect_info.faces_faces);
        }


        if (accelerate_right_vertices)
        {
            pf5.build_vertices_BVH();
        }

        if (accelerate_right_vertices)
        {
            make_result8<true, true>(pf5, pf4, pf2, pf, rule, true, base_type, result2, results, nograph, intersect_info.faces_faces);
        }
        else
        {
            if (accelerate_right)
                make_result8<true, false>(pf5, pf4, pf2, pf, rule, true, base_type, result2, results, nograph, intersect_info.faces_faces);
            else
                make_result8<false, false>(pf5, pf4, pf2, pf, rule, true, base_type, result2, results, nograph, intersect_info.faces_faces);
        }
}
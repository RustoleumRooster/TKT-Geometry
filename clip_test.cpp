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
struct time_measure {
    u16 nVerts = 0;
    float build_bvh_time = 0.0;
    float intersect_bvh_faces_time = 0.0;
    float self_intersections_time = 0.0;
    float intersect_bvh_time = 0.0;
    float intersections_and_bisect_time = 0.0;
    float classify_edges_time = 0.0;
    float common_topology_time = 0.0;
    float build_vertices_and_edges_BVH_time = 0.0;
    float initial_topology_time = 0.0;
    float topology_groups_time = 0.0;
    float calc_loops_time = 0.0;
    float self_topology_loops_time = 0.0;
    float remainder_time = 0.0;
    float total_time = 0.0;

    float inner_classify_loop_time = 0.0;
    float inner_rebuild_faces_from_loops_time = 0.0;
    float inner_rebuild_loops_copy_verts_time = 0.0;
    float inner_finalize_poly_time = 0.0;

    void operator-=(const time_measure& other)
    {
        build_bvh_time -= other.build_bvh_time;
        intersect_bvh_faces_time -= other.intersect_bvh_faces_time;
        self_intersections_time -= other.self_intersections_time;
        intersect_bvh_time -= other.intersect_bvh_time;
        intersections_and_bisect_time -= other.intersections_and_bisect_time;
        classify_edges_time -= other.classify_edges_time;
        common_topology_time -= other.common_topology_time;
        build_vertices_and_edges_BVH_time -= other.build_vertices_and_edges_BVH_time;
        initial_topology_time -= other.initial_topology_time;
        topology_groups_time -= other.topology_groups_time;
        calc_loops_time -= other.calc_loops_time;
        self_topology_loops_time -= other.self_topology_loops_time;
        remainder_time -= other.remainder_time;
        total_time -= other.total_time;

        inner_classify_loop_time -= other.inner_classify_loop_time;
        inner_rebuild_faces_from_loops_time -= other.inner_rebuild_faces_from_loops_time;
        inner_rebuild_loops_copy_verts_time -= other.inner_rebuild_loops_copy_verts_time;
        inner_finalize_poly_time -= other.inner_finalize_poly_time;
    }
};

time_measure left_simple, left_accel, right_simple, right_accel;


void reset_testclip_timers(time_measure& t)
{
    t.build_bvh_time = 0.0;
    t.intersect_bvh_faces_time = 0.0;
    t.self_intersections_time = 0.0;
    t.intersect_bvh_time = 0.0;
    t.intersections_and_bisect_time = 0.0;
    t.classify_edges_time = 0.0;
    t.common_topology_time = 0.0;
    t.build_vertices_and_edges_BVH_time = 0.0;
    t.initial_topology_time = 0.0;
    t.topology_groups_time = 0.0;
    t.calc_loops_time = 0.0;
    t.self_topology_loops_time = 0.0;
    t.remainder_time = 0.0;
    

    t.inner_classify_loop_time = 0.0;
    t.inner_finalize_poly_time = 0.0;
    t.inner_rebuild_faces_from_loops_time = 0.0;
    t.inner_rebuild_loops_copy_verts_time = 0.0;

    t.total_time = 0.0;
}

void reset_testclip_timers()
{
    reset_testclip_timers(left_simple);
    reset_testclip_timers(right_simple);
    reset_testclip_timers(left_accel);
    reset_testclip_timers(right_accel);
}

#define CLEAN_THINGY(thingy) thingy = fabs(thingy) < 0.001 ? 0 : thingy;

void clean_timer(time_measure& t)
{
    CLEAN_THINGY(t.build_bvh_time)
    CLEAN_THINGY(t.intersect_bvh_faces_time)
    CLEAN_THINGY(t.self_intersections_time)
    CLEAN_THINGY(t.intersect_bvh_time)
    CLEAN_THINGY(t.intersections_and_bisect_time)
    CLEAN_THINGY(t.classify_edges_time)
    CLEAN_THINGY(t.common_topology_time)
    CLEAN_THINGY(t.build_vertices_and_edges_BVH_time)
    CLEAN_THINGY(t.initial_topology_time)
    CLEAN_THINGY(t.topology_groups_time)
    CLEAN_THINGY(t.calc_loops_time)
    CLEAN_THINGY(t.self_topology_loops_time)
    CLEAN_THINGY(t.remainder_time)
    CLEAN_THINGY(t.total_time)

    CLEAN_THINGY(t.inner_classify_loop_time)
    CLEAN_THINGY(t.inner_rebuild_faces_from_loops_time)
    CLEAN_THINGY(t.inner_rebuild_loops_copy_verts_time)
    CLEAN_THINGY(t.inner_finalize_poly_time)
}

#define PRINT_THINGY(thingy)std::cout<<#thingy<<": "<<thingy<<"\n";

void print_testclip_timers()
{
    /*
    PRINT_THINGY(build_bvh_time)
    PRINT_THINGY(intersect_bvh_faces_time)
    PRINT_THINGY(self_intersections_time)
    PRINT_THINGY(intersect_bvh_time)
    PRINT_THINGY(intersections_and_bisect_time)
    PRINT_THINGY(classify_edges_time)
    PRINT_THINGY(common_topology_time)*/
    /*
    PRINT_THINGY(build_vertices_and_edges_BVH_time)
    PRINT_THINGY(initial_topology_time)
    PRINT_THINGY(topology_groups_time)
    PRINT_THINGY(calc_loops_time)
    PRINT_THINGY(self_topology_loops_time)
    PRINT_THINGY(remainder_time)*/

    time_measure left_time_diff = left_simple;
    left_time_diff -= left_accel;

    time_measure right_time_diff = right_simple;
    right_time_diff -= right_accel;

    clean_timer(left_time_diff);
    clean_timer(right_time_diff);

   
    PRINT_THINGY(left_time_diff.nVerts)
    PRINT_THINGY(left_time_diff.build_bvh_time)
    PRINT_THINGY(left_time_diff.intersect_bvh_faces_time)
    PRINT_THINGY(left_time_diff.self_intersections_time)
    PRINT_THINGY(left_time_diff.intersect_bvh_time)
    PRINT_THINGY(left_time_diff.intersections_and_bisect_time)
    PRINT_THINGY(left_time_diff.classify_edges_time)
    PRINT_THINGY(left_time_diff.common_topology_time)
    PRINT_THINGY(left_time_diff.build_vertices_and_edges_BVH_time)
    PRINT_THINGY(left_time_diff.initial_topology_time)
    PRINT_THINGY(left_time_diff.topology_groups_time)
    PRINT_THINGY(left_time_diff.calc_loops_time)
    PRINT_THINGY(left_time_diff.self_topology_loops_time)
    //PRINT_THINGY(left_time_diff.remainder_time)
    
    PRINT_THINGY(left_time_diff.inner_classify_loop_time)
    PRINT_THINGY(left_time_diff.inner_rebuild_faces_from_loops_time)
    PRINT_THINGY(left_time_diff.inner_rebuild_loops_copy_verts_time)
    PRINT_THINGY(left_time_diff.inner_finalize_poly_time)

    PRINT_THINGY(left_time_diff.total_time)

    std::cout << "\n";

    PRINT_THINGY(right_time_diff.nVerts)
        PRINT_THINGY(right_time_diff.build_bvh_time)
        PRINT_THINGY(right_time_diff.intersect_bvh_faces_time)
        PRINT_THINGY(right_time_diff.self_intersections_time)
        PRINT_THINGY(right_time_diff.intersect_bvh_time)
        PRINT_THINGY(right_time_diff.intersections_and_bisect_time)
        PRINT_THINGY(right_time_diff.classify_edges_time)
        PRINT_THINGY(right_time_diff.common_topology_time)
        PRINT_THINGY(right_time_diff.build_vertices_and_edges_BVH_time)
        PRINT_THINGY(right_time_diff.initial_topology_time)
        PRINT_THINGY(right_time_diff.topology_groups_time)
        PRINT_THINGY(right_time_diff.calc_loops_time)
        PRINT_THINGY(right_time_diff.self_topology_loops_time)
        // PRINT_THINGY(right_time_diff.remainder_time)

        PRINT_THINGY(right_time_diff.inner_classify_loop_time)
        PRINT_THINGY(right_time_diff.inner_rebuild_faces_from_loops_time)
        PRINT_THINGY(right_time_diff.inner_rebuild_loops_copy_verts_time)
        PRINT_THINGY(right_time_diff.inner_finalize_poly_time)

        PRINT_THINGY(right_time_diff.total_time)

     //   PRINT_THINGY(right_simple.topology_groups_time);
   // PRINT_THINGY(right_accel.topology_groups_time);

    /*std::cout << "\n";
    
    PRINT_THINGY(left_simple.nVerts)
    PRINT_THINGY(left_simple.build_vertices_and_edges_BVH_time)
    PRINT_THINGY(left_simple.initial_topology_time)
    PRINT_THINGY(left_simple.topology_groups_time)
    PRINT_THINGY(left_simple.calc_loops_time)
    PRINT_THINGY(left_simple.self_topology_loops_time)
    PRINT_THINGY(left_simple.remainder_time)
    PRINT_THINGY(left_simple.total_time)

    std::cout << "\n";

    PRINT_THINGY(right_simple.nVerts)
    PRINT_THINGY(right_simple.build_vertices_and_edges_BVH_time)
    PRINT_THINGY(right_simple.initial_topology_time)
    PRINT_THINGY(right_simple.topology_groups_time)
    PRINT_THINGY(right_simple.calc_loops_time)
    PRINT_THINGY(right_simple.self_topology_loops_time)
    PRINT_THINGY(right_simple.remainder_time)
    PRINT_THINGY(right_simple.total_time)

    std::cout << "\n";

    PRINT_THINGY(left_accel.nVerts)
    PRINT_THINGY(left_accel.build_vertices_and_edges_BVH_time)
    PRINT_THINGY(left_accel.initial_topology_time)
    PRINT_THINGY(left_accel.topology_groups_time)
    PRINT_THINGY(left_accel.calc_loops_time)
    PRINT_THINGY(left_accel.self_topology_loops_time)
    PRINT_THINGY(left_accel.remainder_time)
    PRINT_THINGY(left_accel.total_time)

    std::cout << "\n";

    PRINT_THINGY(right_accel.nVerts)
    PRINT_THINGY(right_accel.build_vertices_and_edges_BVH_time)
    PRINT_THINGY(right_accel.initial_topology_time)
    PRINT_THINGY(right_accel.topology_groups_time)
    PRINT_THINGY(right_accel.calc_loops_time)
    PRINT_THINGY(right_accel.self_topology_loops_time)
    PRINT_THINGY(right_accel.remainder_time)
    PRINT_THINGY(right_accel.total_time)*/
    
}

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

  



template<bool bAccelerateEdges, bool bAccelerateVertices>
void make_result7(polyfold& pf4, polyfold& pf5, polyfold& pf, polyfold& pf2, int rule, int rule2, bool overwrite, polyfold& result, clip_results& results, LineHolder& graph, time_measure& my_timer)
{
    auto startTime = std::chrono::high_resolution_clock::now();
    auto startTime2 = std::chrono::high_resolution_clock::now();
    auto timeZero = startTime;
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time;

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

#ifdef OPTIMIZE_LOOPS
        if (pf4.faces[f_i].temp_b == FACE_GEOMETRY_CHANGED)
#endif
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
                                                    if (overwrite)
                                                        f_p.loops.push_back(pf4.faces[f_i].loops[p_i]);
                                                    else
                                                        f_n.loops.push_back(pf4.faces[f_i].loops[p_i]);

                                                    pf5.faces[f_j].loops[p_j].flags = 1;
                                                }
                                                else
                                                {
                                                    if (overwrite)
                                                        f_n.loops.push_back(pf4.faces[f_i].loops[p_i]);
                                                    else
                                                        f_p.loops.push_back(pf4.faces[f_i].loops[p_i]);
                                                }

                                                goto FinishedWithLoop2;
                                            }
                                            else if (pf5.faces[f_j].loops[p_j].topo_group == LOOP_HOLLOW && pf4.faces[f_i].loops[p_i].topo_group == LOOP_SOLID)
                                            {
                                                core::vector3df v1;
                                                if (pf5.get_facial_point(f_j, p_j, v1, graph))
                                                {
                                                    if (rule == GEO_ADD)
                                                    {
                                                        if (pf2.classify_point(v1, graph) == TOP_FRONT)
                                                            f_p.loops.push_back(pf4.faces[f_i].loops[p_i]);
                                                        else
                                                            f_n.loops.push_back(pf4.faces[f_i].loops[p_i]);
                                                    }
                                                    else
                                                    {
                                                        if (pf2.classify_point(v1, graph) == TOP_FRONT)
                                                            f_n.loops.push_back(pf4.faces[f_i].loops[p_i]);
                                                        else
                                                            f_p.loops.push_back(pf4.faces[f_i].loops[p_i]);
                                                    }
                                                }
                                                goto FinishedWithLoop2;
                                            }
                                            else if (pf5.faces[f_j].loops[p_j].topo_group == LOOP_SOLID && pf4.faces[f_i].loops[p_i].topo_group == LOOP_HOLLOW)
                                            {
                                                f_n.loops.push_back(pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop2;
                                            }
                                            else if (pf5.faces[f_j].loops[p_j].topo_group == LOOP_HOLLOW && pf4.faces[f_i].loops[p_i].topo_group == LOOP_HOLLOW)
                                            {
                                                f_n.loops.push_back(pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop2;
                                            }
                                        }
                                        else
                                        {
                                            if (pf5.faces[f_j].loops[p_j].topo_group == LOOP_SOLID && pf4.faces[f_i].loops[p_i].topo_group == LOOP_SOLID)
                                            {
                                                f_n.loops.push_back(pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop2;
                                            }
                                            else if (pf5.faces[f_j].loops[p_j].topo_group == LOOP_HOLLOW && pf4.faces[f_i].loops[p_i].topo_group == LOOP_SOLID)
                                            {
                                                core::vector3df v1;
                                                if (pf5.get_facial_point(f_j, p_j, v1, nograph))
                                                {
                                                    if (rule == GEO_ADD)
                                                    {
                                                        if (pf2.classify_point(v1, graph) == TOP_FRONT)
                                                            f_p.loops.push_back(pf4.faces[f_i].loops[p_i]);
                                                        else
                                                            f_n.loops.push_back(pf4.faces[f_i].loops[p_i]);
                                                    }
                                                    else
                                                    {
                                                        if (pf2.classify_point(v1, graph) == TOP_FRONT)
                                                            f_n.loops.push_back(pf4.faces[f_i].loops[p_i]);
                                                        else
                                                            f_p.loops.push_back(pf4.faces[f_i].loops[p_i]);
                                                    }
                                                }
                                                goto FinishedWithLoop2;
                                            }
                                            else if (pf5.faces[f_j].loops[p_j].topo_group == LOOP_SOLID && pf4.faces[f_i].loops[p_i].topo_group == LOOP_HOLLOW)
                                            {
                                                f_n.loops.push_back(pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop2;
                                            }
                                            else if (pf5.faces[f_j].loops[p_j].topo_group == LOOP_HOLLOW && pf4.faces[f_i].loops[p_i].topo_group == LOOP_HOLLOW)
                                            {
                                                f_n.loops.push_back(pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop2;
                                            }
                                        }
                                    }
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
                FinishedWithLoop2:
                    int z = 0;
                }
            }
        }
#ifdef OPTIMIZE_LOOPS
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
#endif
    }
    START_TIMER()
    positive.rebuild_faces_from_loops<bAccelerateEdges>(pf4);
    negative.rebuild_faces_from_loops<bAccelerateEdges>(pf4);
    INC_TIMER(my_timer,inner_rebuild_faces_from_loops_time)

   // pf = positive;
   // return;

    //positive.reduce_edges_vertices();
   // negative.reduce_edges_vertices();

    for (int f_i = 0; f_i < positive.faces.size(); f_i++)
    {
        if (positive.faces[f_i].loops.size() > 1)
            positive.meld_loops(f_i);
    }

    for (int f_i = 0; f_i < negative.faces.size(); f_i++)
    {
        if (negative.faces[f_i].loops.size() > 1)
            negative.meld_loops(f_i);
    }

   // pf = negative;
   // return;
    START_TIMER()
    pf.rebuild_loops_copy_verts<bAccelerateEdges && bAccelerateVertices>(pf4);
    INC_TIMER(my_timer,inner_rebuild_loops_copy_verts_time)

    pf.vertices = pf4.vertices;
    pf.edges = pf4.edges;

    int nPositive = 0;
    for (int i = 0; i < pf4.faces.size(); i++)
    {
        //if (pf4.faces[i].temp_b == false)
       //     continue;
#ifdef OPTIMIZE_LOOPS
        if (pf4.faces[i].temp_b == FACE_GEOMETRY_UNCHANGED)
            continue;
#endif

        poly_face& f = pf.faces[i];
        f.loops.clear();
        //f.vertices.clear();
        //f.edges.clear();
        /*
        f.m_center = pf4.faces[i].m_center;
        f.m_normal = pf4.faces[i].m_normal;
        f.bFlippedNormal = pf4.faces[i].bFlippedNormal;
        f.surface_group = pf4.faces[i].surface_group;
        f.material_group = pf4.faces[i].material_group;
        f.original_brush = pf4.faces[i].original_brush;
        f.original_face = pf4.faces[i].original_face;
        f.uv_mat = pf4.faces[i].uv_mat;
        f.uv_origin = pf4.faces[i].uv_origin;
        */
        for (int p = 0; p < positive.faces[i].loops.size(); p++)
        {
            f.loops.push_back(positive.faces[i].loops[p]);
            f.loops[f.loops.size() - 1].topo_group = LOOP_SOLID;
            /*
            {
                poly_loop loop;
                for(int v_i=0;v_i<positive.faces[i].loops[p].vertices.size();v_i++)
                {
                    core::vector3df V = positive.vertices[positive.faces[i].loops[p].vertices[v_i]].V;
                    //int v = pf.get_point_or_add(V);
                    int v;

                    if(bAccelerate)
                        v = pf4.find_point_accelerated(V);
                    else
                        v = pf.find_point(V);

                    loop.vertices.push_back(v);
                }
                loop.topo_group=LOOP_SOLID;
                f.loops.push_back(loop);
            }*/
        }

        for (int p = 0; p < negative.faces[i].loops.size(); p++)
        {
            f.loops.push_back(negative.faces[i].loops[p]);
            f.loops[f.loops.size() - 1].topo_group = LOOP_HOLLOW;
            /*
            poly_loop loop;
            for(int v_i=0;v_i<negative.faces[i].loops[p].vertices.size();v_i++)
            {
                core::vector3df V = negative.vertices[negative.faces[i].loops[p].vertices[v_i]].V;
                //int v = pf.get_point_or_add(V);

                int v;
                if (bAccelerate)
                    v = pf4.find_point_accelerated(V);
                else
                    v = pf.find_point(V);

                loop.vertices.push_back(v);
            }
            loop.topo_group=LOOP_HOLLOW;
            f.loops.push_back(loop);*/
        }

    }
    // pf.vertices_BVH = pf4.vertices_BVH;
    // pf.edges_BVH = pf4.edges_BVH;

    START_TIMER()

    if(bAccelerateEdges && bAccelerateVertices && pf.vertices.size() > 175)
        pf.finalize_clipped_poly<true>(pf4, graph);
    else
        pf.finalize_clipped_poly<false>(pf4, graph);

    INC_TIMER(my_timer,inner_finalize_poly_time)

}

void clip_poly_no_acceleration(polyfold& pf, polyfold& pf2, int rule, int base_type, clip_results& results, LineHolder& graph)
{
    LineHolder nograph;

    polyfold pf4 = pf;
    polyfold pf5 = pf2;
    polyfold ret;

    auto startTime = std::chrono::high_resolution_clock::now();
    auto startTime2 = std::chrono::high_resolution_clock::now();
    auto timeZero = startTime;
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time;


#define bAccelerate_a false
#define bAccelerate_b false
#define bVertices_a false
#define bVertices_b false


    time_measure& left_timer = left_simple;
    time_measure& right_timer = right_simple;
    left_timer.nVerts = pf4.vertices.size();
    right_timer.nVerts = pf5.vertices.size();

    START_TIMER()

    //pf4.build_edges_BVH();
    pf4.build_faces_BVH();

    INC_TIMER(left_timer, build_bvh_time)

        START_TIMER()

        //pf5.build_edges_BVH();
        pf5.build_faces_BVH();

    INC_TIMER(right_timer, build_bvh_time)

        START_TIMER()

        pf4.classify_edges<false>(nograph);

    INC_TIMER(left_timer, build_bvh_time)

        START_TIMER()

        pf5.classify_edges<false>(nograph);

    INC_TIMER(right_timer, build_bvh_time)

        std::vector<BVH_intersection_struct> faces_faces_pf4;
    std::vector<BVH_intersection_struct> faces_faces_pf5;

    START_TIMER()

        pf4.faces_BVH.intersect_2(pf4.faces_BVH, faces_faces_pf4);

    INC_TIMER(left_timer, intersect_bvh_faces_time)

        START_TIMER()

        pf5.faces_BVH.intersect_2(pf5.faces_BVH, faces_faces_pf5);

    INC_TIMER(right_timer, intersect_bvh_faces_time)

        START_TIMER()

        do_self_intersections<false>(pf4, faces_faces_pf4);

    INC_TIMER(left_timer, self_intersections_time)

        START_TIMER()

        do_self_intersections<false>(pf5, faces_faces_pf5);

    INC_TIMER(right_timer, self_intersections_time)

        poly_intersection_info intersect_info;

    //START_TIMER()

   // pf4.faces_BVH.intersect_2(pf5.edges_BVH, intersect_info.faces_edges);
   // pf4.edges_BVH.intersect_2(pf5.faces_BVH, intersect_info.edges_faces);
    pf4.faces_BVH.intersect_2(pf5.faces_BVH, intersect_info.faces_faces);

    //INC_TIMER(intersect_bvh_time)
    int n_intersections = 0;

    START_TIMER()

        n_intersections += do_intersections_and_bisect<false>(pf4, pf5, intersect_info, graph);

    INC_TIMER(left_timer, intersections_and_bisect_time)

        START_TIMER()

        n_intersections += do_intersections_and_bisect<false>(pf5, pf4, intersect_info, graph);

    INC_TIMER(right_timer, intersections_and_bisect_time)
        //pf = pf4;
// pf2 = pf5;
 //return;

        n_intersections += sync_vertices(pf4, pf5);
    n_intersections += sync_vertices(pf5, pf4);

        START_TIMER()

        results.n_intersections = n_intersections;
    if (n_intersections == 0)
    {
        return;
    }

    START_TIMER()

        do_common_topology<false>(pf, pf2, pf4, pf5, intersect_info.faces_faces, graph);

    INC_TIMER(left_timer, common_topology_time)
        INC_TIMER(right_timer, common_topology_time)

    //
    //========================================
    //


    START_TIMER()

        if (bAccelerate_a)
        {
            pf4.build_edges_BVH(); //cutoff is ~300 vertices
        }

    INC_TIMER(left_timer,build_vertices_and_edges_BVH_time)

        START_TIMER()

        if (bAccelerate_b)
        {
            pf5.build_edges_BVH();
        }

    INC_TIMER(right_timer,build_vertices_and_edges_BVH_time)

        std::vector<BVH_intersection_struct> edges_faces_45;
    std::vector<BVH_intersection_struct> edges_faces_54;

    pf4.edges_BVH.intersect_2(pf5.faces_BVH, edges_faces_45);
    pf5.edges_BVH.intersect_2(pf4.faces_BVH, edges_faces_54);

    START_TIMER()

        do_initial_topology<false>(pf4, pf5, edges_faces_45, nograph);

    INC_TIMER(left_timer,initial_topology_time)

        START_TIMER()

        do_initial_topology<false>(pf5, pf4, edges_faces_54, nograph);

    INC_TIMER(right_timer,initial_topology_time)


        START_TIMER()

        do_topology_groups<false>(pf4, pf5, rule, base_type, nograph);

    INC_TIMER(left_timer,topology_groups_time)

        START_TIMER()
        
        do_topology_groups<false>(pf5, pf4, rule, base_type, nograph);

    INC_TIMER(right_timer, topology_groups_time)

    

#ifdef OPTIMIZE_LOOPS

        for (int f_i = 0; f_i < pf4.faces.size(); f_i++)
            pf4.faces[f_i].temp_b = FACE_GEOMETRY_UNCHANGED;

    for (int f_i = 0; f_i < pf5.faces.size(); f_i++)
        pf5.faces[f_i].temp_b = FACE_GEOMETRY_UNCHANGED;

    for (BVH_intersection_struct hit : intersect_info.faces_faces)
    {
        pf4.faces[hit.X].temp_b = FACE_GEOMETRY_CHANGED;
        pf5.faces[hit.Y].temp_b = FACE_GEOMETRY_CHANGED;
    }

    int saved = 0;

    START_TIMER()

        for (int f_i = 0; f_i < pf4.faces.size(); f_i++)
        {
            if (pf4.faces[f_i].temp_b == FACE_GEOMETRY_CHANGED)
                pf4.calc_loops(f_i, nograph);
        }

    INC_TIMER(left_timer,calc_loops_time)

        START_TIMER()

        for (int f_i = 0; f_i < pf5.faces.size(); f_i++)
        {
            if (pf5.faces[f_i].temp_b == FACE_GEOMETRY_CHANGED)
                pf5.calc_loops(f_i, nograph);
        }

    INC_TIMER(right_timer,calc_loops_time)

#else
        for (int f_i = 0; f_i < pf4.faces.size(); f_i++)
        {
            pf4.calc_loops(f_i, nograph);
        }

    for (int f_i = 0; f_i < pf5.faces.size(); f_i++)
    {
        pf5.calc_loops(f_i, nograph);
    }
#endif
    START_TIMER()

        do_self_topology_loops(pf4, pf, nograph);

    INC_TIMER(left_timer,self_topology_loops_time)

        START_TIMER()

        do_self_topology_loops(pf5, pf2, nograph);

    INC_TIMER(right_timer,self_topology_loops_time)



        for (int f_i = 0; f_i < pf4.faces.size(); f_i++)
            for (int p_i = 0; p_i < pf4.faces[f_i].loops.size(); p_i++)
                pf4.faces[f_i].loops[p_i].flags = 0;

    for (int f_i = 0; f_i < pf5.faces.size(); f_i++)
        for (int p_i = 0; p_i < pf5.faces[f_i].loops.size(); p_i++)
            pf5.faces[f_i].loops[p_i].flags = 0;

    polyfold result1;
    polyfold result2;

    START_TIMER()

    if (bVertices_a)
    {
        pf4.build_vertices_BVH();
    }

    INC_TIMER(left_timer, build_vertices_and_edges_BVH_time)

    make_result7<bAccelerate_a, bVertices_a>(pf4, pf5, pf, pf2, rule, true, base_type, result1, results, nograph,left_timer);

    START_TIMER()

        if (bVertices_b)
        {
            pf5.build_vertices_BVH();
        }

    INC_TIMER(right_timer, build_vertices_and_edges_BVH_time)

    make_result7<bAccelerate_b, bVertices_b>(pf5, pf4, pf2, pf, rule, true, base_type, result2, results, nograph,right_timer);

}

void clip_poly_accelerated(polyfold& pf, polyfold& pf2, int rule, int base_type, clip_results& results, LineHolder& graph)
{
    LineHolder nograph;

    polyfold pf4 = pf;
    polyfold pf5 = pf2;

    auto startTime = std::chrono::high_resolution_clock::now();
    auto startTime2 = std::chrono::high_resolution_clock::now();
    auto timeZero = startTime;
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time;

    
#define bAccelerate_a true
#define bAccelerate_b true


    time_measure& left_timer = left_accel;
    time_measure& right_timer = right_accel;
    left_timer.nVerts = pf4.vertices.size();
    right_timer.nVerts = pf5.vertices.size();

    START_TIMER()

        //pf4.build_edges_BVH();
    pf4.build_faces_BVH();

    INC_TIMER(left_timer, build_bvh_time)

        START_TIMER()

        //pf5.build_edges_BVH();
        pf5.build_faces_BVH();

    INC_TIMER(right_timer, build_bvh_time)

        START_TIMER()

        pf4.classify_edges<true>(nograph);

    INC_TIMER(left_timer, build_bvh_time)

        START_TIMER()

        pf5.classify_edges<true>(nograph);

    INC_TIMER(right_timer, build_bvh_time)

        std::vector<BVH_intersection_struct> faces_faces_pf4;
    std::vector<BVH_intersection_struct> faces_faces_pf5;

    START_TIMER()

        pf4.faces_BVH.intersect_2(pf4.faces_BVH, faces_faces_pf4);

    INC_TIMER(left_timer, intersect_bvh_faces_time)

        START_TIMER()

        pf5.faces_BVH.intersect_2(pf5.faces_BVH, faces_faces_pf5);

    INC_TIMER(right_timer, intersect_bvh_faces_time)

        START_TIMER()
        /*
        std::cout << faces_faces_pf4.size() << " faces faces\n";
        for (int i = 0; i < faces_faces_pf4.size(); i++)
        {
        if(faces_faces_pf4[i].X > faces_faces_pf4[i].Y)
            std::cout << faces_faces_pf4[i].X << " " << faces_faces_pf4[i].Y << "\n";
        }*/

        do_self_intersections<true>(pf4, faces_faces_pf4);

    INC_TIMER(left_timer, self_intersections_time)

        START_TIMER()

        do_self_intersections<true>(pf5, faces_faces_pf5);

    INC_TIMER(right_timer, self_intersections_time)

        poly_intersection_info intersect_info;

    //START_TIMER()

    //pf4.faces_BVH.intersect_2(pf5.edges_BVH, intersect_info.faces_edges);
    //pf4.edges_BVH.intersect_2(pf5.faces_BVH, intersect_info.edges_faces);
    pf4.faces_BVH.intersect_2(pf5.faces_BVH, intersect_info.faces_faces);

    //INC_TIMER(intersect_bvh_time)

    //std::cout << "faces edges: " << intersect_info.faces_edges.size() << "\n";
    int n_intersections = 0;

    START_TIMER()

        n_intersections += do_intersections_and_bisect<true>(pf4, pf5, intersect_info, graph);

    INC_TIMER(left_timer, intersections_and_bisect_time)

        START_TIMER()

        n_intersections += do_intersections_and_bisect<true>(pf5, pf4, intersect_info, graph);

    INC_TIMER(right_timer, intersections_and_bisect_time)
        //pf = pf4;
// pf2 = pf5;
 //return;

        n_intersections += sync_vertices(pf4, pf5);
        n_intersections += sync_vertices(pf5, pf4);

        START_TIMER()

        results.n_intersections = n_intersections;
    if (n_intersections == 0)
    {
        return;
    }

        START_TIMER()

        do_common_topology<true>(pf, pf2, pf4, pf5, intersect_info.faces_faces, graph);

    INC_TIMER(left_timer, common_topology_time)
        INC_TIMER(right_timer, common_topology_time)


    //pf4.build_vertices_BVH();
    //pf5.build_vertices_BVH();// ~200 vertices //if we are already building edges, cutoff at 80-100 


    int N = pf4.edges.size() * pf5.faces.size();
    int M = pf5.edges.size() * pf4.faces.size();

    bool accelerate_left = false;
    bool accelerate_right = false;
    
    if ((N > 5000 && pf4.vertices.size() < pf5.vertices.size() * 0.5) || (N > 10000) || (pf4.vertices.size() > 300))
        accelerate_left = true;

    if ((M > 5000 && pf5.vertices.size() < pf4.vertices.size() * 0.5) || (M > 10000) || (pf5.vertices.size() > 300))
        accelerate_right = true;
        
    START_TIMER()

        if (accelerate_left)
        {
            pf4.build_edges_BVH(); //cutoff is ~300 vertices
        }

    INC_TIMER(left_timer, build_vertices_and_edges_BVH_time)

        START_TIMER()

        if (accelerate_right)
        {
            pf5.build_edges_BVH();
        }

    INC_TIMER(right_timer, build_vertices_and_edges_BVH_time)

        std::vector<BVH_intersection_struct> edges_faces_45;
    std::vector<BVH_intersection_struct> edges_faces_54;

    START_TIMER()
        if (accelerate_left)
        {
            pf4.edges_BVH.intersect_2(pf5.faces_BVH, edges_faces_45);
            do_initial_topology<true>(pf4, pf5, edges_faces_45, nograph);
        }
        else
        {
            do_initial_topology<false>(pf4, pf5, edges_faces_45, nograph);
        }

    INC_TIMER(left_timer, initial_topology_time)

        START_TIMER()
        if (accelerate_right)
        {
            pf5.edges_BVH.intersect_2(pf4.faces_BVH, edges_faces_54);
            do_initial_topology<true>(pf5, pf4, edges_faces_54, nograph);
        }
        else
        {
            do_initial_topology<false>(pf5, pf4, edges_faces_54, nograph);
        }

    INC_TIMER(right_timer, initial_topology_time)


        START_TIMER()
        if (accelerate_left)
            do_topology_groups<true>(pf4, pf5, rule, base_type, nograph);
        else
            do_topology_groups<false>(pf4, pf5, rule, base_type, nograph);

    INC_TIMER(left_timer, topology_groups_time)

        START_TIMER()

        if (accelerate_right)
           do_topology_groups<true>(pf5, pf4, rule, base_type, nograph);
        else
            do_topology_groups<false>(pf5, pf4, rule, base_type, nograph);
        INC_TIMER(right_timer, topology_groups_time)
           
    

#ifdef OPTIMIZE_LOOPS

        for (int f_i = 0; f_i < pf4.faces.size(); f_i++)
            pf4.faces[f_i].temp_b = FACE_GEOMETRY_UNCHANGED;

    for (int f_i = 0; f_i < pf5.faces.size(); f_i++)
        pf5.faces[f_i].temp_b = FACE_GEOMETRY_UNCHANGED;

    for (BVH_intersection_struct hit : intersect_info.faces_faces)
    {
        pf4.faces[hit.X].temp_b = FACE_GEOMETRY_CHANGED;
        pf5.faces[hit.Y].temp_b = FACE_GEOMETRY_CHANGED;
    }

    int saved = 0;

    START_TIMER()

        for (int f_i = 0; f_i < pf4.faces.size(); f_i++)
        {
            if (pf4.faces[f_i].temp_b == FACE_GEOMETRY_CHANGED)
                pf4.calc_loops(f_i, nograph);
        }

    INC_TIMER(left_timer, calc_loops_time)

        START_TIMER()

        for (int f_i = 0; f_i < pf5.faces.size(); f_i++)
        {
            if (pf5.faces[f_i].temp_b == FACE_GEOMETRY_CHANGED)
                pf5.calc_loops(f_i, nograph);
        }

    INC_TIMER(right_timer, calc_loops_time)

#else
        for (int f_i = 0; f_i < pf4.faces.size(); f_i++)
        {
            pf4.calc_loops(f_i, nograph);
        }

    for (int f_i = 0; f_i < pf5.faces.size(); f_i++)
    {
        pf5.calc_loops(f_i, nograph);
    }
#endif
    START_TIMER()

        do_self_topology_loops(pf4, pf, nograph);

    INC_TIMER(left_timer, self_topology_loops_time)

        START_TIMER()

        do_self_topology_loops(pf5, pf2, nograph);

    INC_TIMER(right_timer, self_topology_loops_time)

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

    START_TIMER()

        if (accelerate_left_vertices)
        {
            pf4.build_vertices_BVH();
        }

    INC_TIMER(left_timer, build_vertices_and_edges_BVH_time)

        if (accelerate_left_vertices)
        {
            make_result7<true, true>(pf4, pf5, pf, pf2, rule, true, base_type, result1, results, nograph, left_timer);
        }
        else
        {
            if (accelerate_left)
                make_result7<true, false>(pf4, pf5, pf, pf2, rule, true, base_type, result1, results, nograph, left_timer);
            else
                make_result7<false, false>(pf4, pf5, pf, pf2, rule, true, base_type, result1, results, nograph, left_timer);
        }

    START_TIMER()

        if (accelerate_right_vertices)
        {
            pf5.build_vertices_BVH();
        }

    INC_TIMER(right_timer, build_vertices_and_edges_BVH_time)

        if (accelerate_right_vertices)
        {
            make_result7<true, true>(pf5, pf4, pf2, pf, rule, true, base_type, result2, results, nograph, right_timer);
        }
        else
        {
            if (accelerate_right)
                make_result7<true, false>(pf5, pf4, pf2, pf, rule, true, base_type, result2, results, nograph, right_timer);
            else
                make_result7<false, false>(pf5, pf4, pf2, pf, rule, true, base_type, result2, results, nograph, right_timer);
        }

}


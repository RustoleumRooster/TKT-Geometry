
#include <irrlicht.h>
#include <iostream>
#include "csg_classes.h"
#include "geometry_scene.h"
#include "clip_functions.h"
#include "clip_functions2.h"
#include "clip_functions3.h"
#include "tolerances.h"
#include <chrono>

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
float remainder_time=0.0;

void reset_clipcpp_timers()
{
    build_bvh_time = 0.0;
    intersect_bvh_faces_time = 0.0;
    self_intersections_time = 0.0;
    intersect_bvh_time = 0.0;
    intersections_and_bisect_time = 0.0;
    classify_edges_time = 0.0;
    common_topology_time = 0.0;
    build_vertices_and_edges_BVH_time = 0.0;
    initial_topology_time = 0.0;
    topology_groups_time = 0.0;
    calc_loops_time = 0.0;
    self_topology_loops_time = 0.0;
    remainder_time = 0.0;
}

#define PRINT_THINGY(thingy)std::cout<<#thingy<<": "<<thingy<<"\n";

void print_clipcpp_timers()
{
    PRINT_THINGY(build_bvh_time)
    PRINT_THINGY(intersect_bvh_faces_time)
    PRINT_THINGY(self_intersections_time)
    PRINT_THINGY(intersect_bvh_time)
    PRINT_THINGY(intersections_and_bisect_time)
    PRINT_THINGY(classify_edges_time)
    PRINT_THINGY(common_topology_time)
    PRINT_THINGY(build_vertices_and_edges_BVH_time)
    PRINT_THINGY(initial_topology_time)
    PRINT_THINGY(topology_groups_time)
    PRINT_THINGY(calc_loops_time)
    PRINT_THINGY(self_topology_loops_time)
    PRINT_THINGY(remainder_time)
}


#define START_TIMER() startTime = std::chrono::high_resolution_clock::now();
#define PRINT_TIMER(text) currentTime = std::chrono::high_resolution_clock::now(); \
    time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count(); \
    std::cout << "---------time (" <<#text<< "): " << time << "\n";
#define PRINT_TOTAL_TIME() currentTime = std::chrono::high_resolution_clock::now(); \
    time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - timeZero).count(); \
    std::cout << "---------total time: " << time << "\n";

#define INC_TIMER(thingy) currentTime = std::chrono::high_resolution_clock::now(); \
    thingy += std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count(); \

template<bool bAccelerate>
void make_result(polyfold& pf4, polyfold& pf5, polyfold& pf, polyfold& pf2, int rule, int rule2, bool overwrite, polyfold& result, clip_results& results, LineHolder &graph)
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

    for(int f_i=0; f_i < pf4.faces.size(); f_i++)
    {
        core::plane3df f_plane = core::plane3df(pf4.faces[f_i].m_center,pf4.faces[f_i].m_normal);

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

                        int RES = classify_loop<bAccelerate>(pf4, f_i, p_i, pf5, rule, rule2, graph);

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
                    int RES = classify_loop<bAccelerate>(pf4, f_i, p_i, pf5, rule, rule2, graph);

                    if (RES == TOP_BEHIND)
                    {
                        pf.faces[f_i].loops.clear();
                    }
                }
            }
        }
#endif
    }

    positive.rebuild_faces_from_loops<bAccelerate>(pf4);
    negative.rebuild_faces_from_loops<bAccelerate>(pf4);

    for (int f_i = 0; f_i < positive.faces.size(); f_i++)
    {
        if(positive.faces[f_i].loops.size() > 1)
            positive.meld_loops(f_i);
    }

    for (int f_i = 0; f_i < negative.faces.size(); f_i++)
    {
        if (negative.faces[f_i].loops.size() > 1)
            negative.meld_loops(f_i);
    }

    pf.rebuild_loops_copy_verts<bAccelerate>(pf4);
    pf.vertices = pf4.vertices;
    pf.edges = pf4.edges;

    int nPositive=0;
    for(int i=0;i<pf4.faces.size();i++)
    {

#ifdef OPTIMIZE_LOOPS
        if (pf4.faces[i].temp_b == FACE_GEOMETRY_UNCHANGED)
            continue;
#endif

        poly_face& f = pf.faces[i];
        f.loops.clear();

        for(int p=0;p<positive.faces[i].loops.size();p++)
        {
            f.loops.push_back(positive.faces[i].loops[p]);
            f.loops[f.loops.size() - 1].topo_group = LOOP_SOLID; 
        }

        for(int p=0;p<negative.faces[i].loops.size();p++)
        {
            f.loops.push_back(negative.faces[i].loops[p]);
            f.loops[f.loops.size() - 1].topo_group = LOOP_HOLLOW;
        }

    }

    pf.finalize_clipped_poly<bAccelerate>(pf4,graph);

}

void clip_poly(polyfold& pf, polyfold& pf2, int rule, int base_type, clip_results& results, LineHolder &graph)
{
    LineHolder nograph;

    polyfold pf4 = pf;
    polyfold pf5 = pf2;
    polyfold ret;

    auto startTime = std::chrono::high_resolution_clock::now();
    auto timeZero = startTime;
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time;

    START_TIMER()

    pf4.build_edges_BVH();
    pf4.build_faces_BVH();
    //pf5.build_edges_BVH();
    pf5.build_faces_BVH();

    INC_TIMER(build_bvh_time)

    pf4.classify_edges<true>(nograph);
    pf5.classify_edges<true>(nograph);

    

    std::vector<BVH_intersection_struct> faces_faces_pf4;
    std::vector<BVH_intersection_struct> faces_faces_pf5;

    START_TIMER()

    pf4.faces_BVH.intersect_2(pf4.faces_BVH, faces_faces_pf4);
    pf5.faces_BVH.intersect_2(pf5.faces_BVH, faces_faces_pf5);

    INC_TIMER(intersect_bvh_faces_time)

    START_TIMER()

    do_self_intersections<true>(pf4, faces_faces_pf4);
    do_self_intersections<true>(pf5, faces_faces_pf5);

    INC_TIMER(self_intersections_time)

    poly_intersection_info intersect_info;

    START_TIMER()

    pf4.faces_BVH.intersect_2(pf5.edges_BVH, intersect_info.faces_edges);
    pf4.edges_BVH.intersect_2(pf5.faces_BVH, intersect_info.edges_faces);
    pf4.faces_BVH.intersect_2(pf5.faces_BVH, intersect_info.faces_faces);
    
    INC_TIMER(intersect_bvh_time)

    START_TIMER()

    int n_intersections = do_intersections_and_bisect(pf,pf2,pf4,pf5,intersect_info, graph);

    INC_TIMER(intersections_and_bisect_time)

    START_TIMER()

    results.n_intersections = n_intersections;
    if(n_intersections == 0)
    {
        return;
    }

    START_TIMER()

    //pf4.classify_edges(nograph);
    //pf5.classify_edges(nograph);

    INC_TIMER(classify_edges_time)

    START_TIMER()

    do_common_topology<true>(pf,pf2,pf4,pf5,intersect_info.faces_faces,graph);

    INC_TIMER(common_topology_time)

    START_TIMER()

    pf4.build_vertices_BVH();
    pf5.build_vertices_BVH();
    pf4.build_edges_BVH();
    pf5.build_edges_BVH();

    std::vector<BVH_intersection_struct> edges_faces_45;
    std::vector<BVH_intersection_struct> edges_faces_54;

    pf4.edges_BVH.intersect_2(pf5.faces_BVH, edges_faces_45);
    pf5.edges_BVH.intersect_2(pf4.faces_BVH, edges_faces_54);

    INC_TIMER(build_vertices_and_edges_BVH_time)

    START_TIMER()

    do_initial_topology<true>(pf4,pf5,edges_faces_45,nograph);
    do_initial_topology<true>(pf5,pf4,edges_faces_54,nograph);

    INC_TIMER(initial_topology_time)

    START_TIMER()

    do_topology_groups<true>(pf4,pf5,rule,base_type,nograph);
    do_topology_groups<true>(pf5,pf4,rule,base_type,nograph);

    INC_TIMER(topology_groups_time)

    START_TIMER()

    
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

    INC_TIMER(calc_loops_time)


    START_TIMER()

    do_self_topology_loops(pf4,pf,nograph);
    do_self_topology_loops(pf5,pf2,nograph);

    INC_TIMER(self_topology_loops_time)

    START_TIMER()

    for(int f_i=0; f_i < pf4.faces.size(); f_i++)
        for(int p_i=0; p_i < pf4.faces[f_i].loops.size(); p_i++)
            pf4.faces[f_i].loops[p_i].flags=0;


    for(int f_i=0; f_i < pf5.faces.size(); f_i++)
        for(int p_i=0; p_i < pf5.faces[f_i].loops.size(); p_i++)
            pf5.faces[f_i].loops[p_i].flags=0;

    polyfold result1;
    polyfold result2;

    make_result<false>(pf4,pf5,pf,pf2,rule,true,base_type,result1, results,nograph);
    make_result<false>(pf5,pf4,pf2,pf,rule,true,base_type,result2, results,nograph);

    //result1.surface_groups = pf.surface_groups;
    //result2.surface_groups = pf2.surface_groups;

    //pf = result1;
    //pf2 = result2;

    INC_TIMER(remainder_time)
    //PRINT_TOTAL_TIME()
}

void clip_poly_single(polyfold& pf, polyfold& pf2, int rule, int base_type, clip_results& results, LineHolder &graph)
{
    LineHolder nograph;

    polyfold pf4 = pf;
    polyfold pf5 = pf2;
    polyfold ret;

    pf4.build_edges_BVH();
    pf4.build_faces_BVH();
    pf5.build_edges_BVH();
    pf5.build_faces_BVH();

    std::vector<BVH_intersection_struct> faces_faces_pf4;
    std::vector<BVH_intersection_struct> faces_faces_pf5;

    do_self_intersections<true>(pf4, faces_faces_pf4);
    do_self_intersections<true>(pf5, faces_faces_pf5);

    poly_intersection_info intersect_info;

    pf4.faces_BVH.intersect_2(pf5.edges_BVH, intersect_info.faces_edges);
    pf4.edges_BVH.intersect_2(pf5.faces_BVH, intersect_info.edges_faces);
    pf4.faces_BVH.intersect_2(pf5.faces_BVH, intersect_info.faces_faces);

    int n_intersections = do_intersections_and_bisect(pf,pf2,pf4,pf5, intersect_info, graph);

    results.n_intersections = n_intersections;
    if(n_intersections == 0)
    {
        return;
    }

    pf4.classify_edges<true>(nograph);
    pf5.classify_edges<true>(nograph);

    do_common_topology<true>(pf,pf2,pf4,pf5,intersect_info.faces_faces, graph);

    pf4.build_vertices_BVH();
    pf5.build_vertices_BVH();
    pf4.build_edges_BVH();
    //pf5.build_edges_BVH();

    std::vector<BVH_intersection_struct> edges_faces;

    pf4.edges_BVH.intersect_2(pf5.faces_BVH, edges_faces);

    do_initial_topology<true>(pf4,pf5,edges_faces,nograph);

    do_topology_groups<true>(pf4,pf5,rule,base_type,nograph);

    for(int f_i=0; f_i<pf4.faces.size();f_i++)
    {
        pf4.calc_loops(f_i,nograph);
    }

    for(int f_i=0; f_i<pf5.faces.size();f_i++)
    {
        pf5.calc_loops(f_i,nograph);
    }

    do_self_topology_loops(pf4,pf,nograph);
    do_self_topology_loops(pf5,pf2,nograph);

    for(int f_i=0; f_i < pf4.faces.size(); f_i++)
        for(int p_i=0; p_i < pf4.faces[f_i].loops.size(); p_i++)
            pf4.faces[f_i].loops[p_i].flags=0;

    for(int f_i=0; f_i < pf5.faces.size(); f_i++)
        for(int p_i=0; p_i < pf5.faces[f_i].loops.size(); p_i++)
            pf5.faces[f_i].loops[p_i].flags=0;

    polyfold result;

    make_result<true>(pf4,pf5,pf,pf2,rule,false,base_type,result, results,nograph);

    result.surface_groups = pf.surface_groups;

    pf = result;
}


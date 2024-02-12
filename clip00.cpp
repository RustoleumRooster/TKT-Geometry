
#include <irrlicht.h>
#include <iostream>
#include "csg_classes.h"
#include "geometry_scene.h"
#include "clip_functions.h"
#include "tolerances.h"


polyfold clip_poly1(polyfold& pf, polyfold& pf2, int rule, int rule2, clip_results& results, LineHolder &graph)
{
    std::cout<<"Creating new geometry (1) ";
    std::cout<<pf.topology<<","<<pf2.topology<<","<<rule<<","<<rule2<<"\n";

    LineHolder nograph;

    polyfold pf4 = pf;
    polyfold pf5 = pf2;
    polyfold ret;

    do_self_intersections(pf5);

    int n_intersections = do_intersections_and_bisect2(pf,pf2,pf4,pf5,graph);

    results.n_intersections = n_intersections;
    if(n_intersections == 0)
    {
       // return pf;
    }

    pf5.classify_edges3(nograph);

    do_common_topology(pf,pf2,pf4,pf5,graph);

    do_initial_topology(pf,pf2,pf4,pf5,nograph);

    do_topology_groups(pf4,pf5,rule,rule2,nograph);

    for(int f_i=0; f_i<pf4.faces.size();f_i++)
    {
        pf4.calc_loops4(f_i,nograph);
        for(int p_i=0; p_i< pf4.faces[f_i].loops.size(); p_i++)
        {
            pf4.set_loop_solid(f_i,p_i);
        }
    }

    for(int f_i=0; f_i<pf5.faces.size();f_i++)
    {
        pf5.calc_loops4(f_i,nograph);
    }

    for(int f_i=0; f_i < pf4.faces.size(); f_i++)
    {
        pf4.sort_loops(f_i);
    }

    do_self_topology_loops(pf4,pf,nograph);
    do_self_topology_loops(pf5,pf2,nograph);

    polyfold positive;
    polyfold negative;
    polyfold ghost;
    polyfold ghost_negative;

    positive.vertices = pf4.vertices;
    negative.vertices = pf4.vertices;
    ghost.vertices = pf4.vertices;
    ghost_negative.vertices = pf4.vertices;

    //Compare Loops pf4 vs pf5


    for(int f_i=0; f_i < pf4.faces.size(); f_i++)
    {
        core::plane3df f_plane = core::plane3df(pf4.faces[f_i].m_center,pf4.faces[f_i].m_normal);
        poly_face f_p;
        poly_face f_n;
        poly_face f_g;
        poly_face f_gn;

        f_p.m_center = pf4.faces[f_i].m_center;
        f_p.m_normal = pf4.faces[f_i].m_normal;
        f_n.m_center = pf4.faces[f_i].m_center;
        f_n.m_normal = pf4.faces[f_i].m_normal;
        f_g.m_center = pf4.faces[f_i].m_center;
        f_g.m_normal = pf4.faces[f_i].m_normal;
        f_gn.m_center = pf4.faces[f_i].m_center;
        f_gn.m_normal = pf4.faces[f_i].m_normal;
        f_p.bFlippedNormal = pf4.faces[f_i].bFlippedNormal;
        f_n.bFlippedNormal = pf4.faces[f_i].bFlippedNormal;
        f_g.bFlippedNormal = pf4.faces[f_i].bFlippedNormal;
        f_gn.bFlippedNormal = pf4.faces[f_i].bFlippedNormal;

        if(pf4.faces[f_i].loops.size()>0)
        {
            for(int p_i=0; p_i < pf4.faces[f_i].loops.size(); p_i++)
            {
                if(pf4.faces[f_i].loops[p_i].vertices.size()>0 && pf4.faces[f_i].loops[p_i].topo_group != LOOP_UNDEF)
                {
                    for(int f_j=0; f_j < pf5.faces.size(); f_j++)
                    {
                            if(is_coplanar_point(f_plane,pf5.faces[f_j].m_center) &&
                               is_parallel_normal(pf4.faces[f_i].m_normal,pf2.faces[f_j].m_normal))
                            {
                                for(int p_j=0; p_j < pf5.faces[f_j].loops.size(); p_j++)
                                {
                                    if(pf5.faces[f_j].loops[p_j].vertices.size() > 0 &&
                                       pf5.faces[f_j].loops[p_j].topo_group != LOOP_UNDEF &&
                                       is_identical_loop(pf4, pf4.faces[f_i].loops[p_i],pf5, pf5.faces[f_j].loops[p_j]))
                                    {
                                         //f_p.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                         //goto FinishedWithLoop;

                                        if(pf4.faces[f_i].m_normal.dotProduct(pf5.faces[f_j].m_normal) > 0)
                                        {
                                            if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_SOLID && pf4.faces[f_i].loops[p_i].topo_group==LOOP_SOLID)
                                            {
                                                f_g.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop;
                                            }
                                            else if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_HOLLOW && pf4.faces[f_i].loops[p_i].topo_group==LOOP_SOLID)
                                            {
                                                if(rule==GEO_ADD)
                                                {
                                                    f_g.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                    goto FinishedWithLoop;
                                                }
                                            }
                                            else if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_SOLID && pf4.faces[f_i].loops[p_i].topo_group==LOOP_HOLLOW)
                                            {
                                                f_gn.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop;
                                            }
                                            else if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_HOLLOW && pf4.faces[f_i].loops[p_i].topo_group==LOOP_HOLLOW)
                                            {
                                                f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop;
                                            }
                                        }
                                        else
                                        {
                                            if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_SOLID && pf4.faces[f_i].loops[p_i].topo_group==LOOP_SOLID)
                                             {
                                                f_g.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop;
                                             }
                                             else if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_HOLLOW && pf4.faces[f_i].loops[p_i].topo_group==LOOP_SOLID)
                                             {
                                                 if(rule==GEO_ADD)
                                                 {
                                                    f_p.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                    goto FinishedWithLoop;
                                                 }
                                                 //else
                                                 //   f_g.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                 //goto FinishedWithLoop;
                                             }
                                             else if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_SOLID && pf4.faces[f_i].loops[p_i].topo_group==LOOP_HOLLOW)
                                             {
                                                f_gn.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop;
                                             }
                                             else if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_HOLLOW && pf4.faces[f_i].loops[p_i].topo_group==LOOP_HOLLOW)
                                             {
                                                f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop;
                                             }
                                         }
                                    }
                                }
                            }
                    }
                    int RES = classify_loop(pf4,f_i,p_i,pf2,rule,rule2,graph);
                    if(RES==TOP_FRONT)
                    {
                        if(pf4.faces[f_i].loops[p_i].topo_group==LOOP_SOLID)
                            f_p.loops.push_back( pf4.faces[f_i].loops[p_i]);
                        else if(pf4.faces[f_i].loops[p_i].topo_group==LOOP_HOLLOW)
                            f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                    }
                    else if(RES==TOP_BEHIND)
                    {
                        if(pf4.faces[f_i].loops[p_i].topo_group==LOOP_SOLID)
                            f_g.loops.push_back( pf4.faces[f_i].loops[p_i]);
                        else if(pf4.faces[f_i].loops[p_i].topo_group==LOOP_HOLLOW)
                            f_gn.loops.push_back( pf4.faces[f_i].loops[p_i]);
                    }
                }
                FinishedWithLoop:
                int z=0;
            }
        }
        positive.faces.push_back(f_p);
        negative.faces.push_back(f_n);
        ghost.faces.push_back(f_g);
        ghost_negative.faces.push_back(f_gn);
    }

    positive.reduce_edges_vertices2();
    negative.reduce_edges_vertices2();
    ghost.reduce_edges_vertices2();
    ghost_negative.reduce_edges_vertices2();

    polyfold result;

    for(int i=0;i<pf4.faces.size();i++)
    {
        poly_face f;
        f.m_center = pf4.faces[i].m_center;
        f.m_normal = pf4.faces[i].m_normal;
        f.bFlippedNormal = pf4.faces[i].bFlippedNormal;
        f.surface_group = pf4.faces[i].surface_group;
        f.original_face = pf4.faces[i].original_face;

        for(int p=0;p<positive.faces[i].loops.size();p++)
        {
            {
                poly_loop loop;
                for(int v_i=0;v_i<positive.faces[i].loops[p].vertices.size();v_i++)
                {
                    core::vector3df V = positive.vertices[positive.faces[i].loops[p].vertices[v_i]].V;
                    int v = result.get_point_or_add(V);
                    loop.vertices.push_back(v);
                }
                loop.topo_group=LOOP_SOLID;
                f.loops.push_back(loop);
            }
        }
        for(int p=0;p<negative.faces[i].loops.size();p++)
        {
            {
                poly_loop loop;
                for(int v_i=0;v_i<negative.faces[i].loops[p].vertices.size();v_i++)
                {
                    core::vector3df V = negative.vertices[negative.faces[i].loops[p].vertices[v_i]].V;
                    int v = result.get_point_or_add(V);
                    loop.vertices.push_back(v);
                }
                loop.topo_group=LOOP_HOLLOW;
                f.loops.push_back(loop);
            }
        }
         for(int p=0;p<ghost.faces[i].loops.size();p++)
        {
            {
                poly_loop loop;
                for(int v_i=0;v_i<ghost.faces[i].loops[p].vertices.size();v_i++)
                {
                    core::vector3df V = ghost.vertices[ghost.faces[i].loops[p].vertices[v_i]].V;
                    int v = result.get_point_or_add(V);
                    loop.vertices.push_back(v);
                }
                loop.topo_group=LOOP_GHOST_SOLID;
                f.loops.push_back(loop);
            }
        }
        for(int p=0;p<ghost_negative.faces[i].loops.size();p++)
        {
            {
                poly_loop loop;
                for(int v_i=0;v_i<ghost_negative.faces[i].loops[p].vertices.size();v_i++)
                {
                    core::vector3df V = ghost_negative.vertices[ghost_negative.faces[i].loops[p].vertices[v_i]].V;
                    int v = result.get_point_or_add(V);
                    loop.vertices.push_back(v);
                }
                loop.topo_group=LOOP_GHOST_HOLLOW;
                f.loops.push_back(loop);
            }
        }
        result.faces.push_back(f);
    }

    result.reduce_edges_vertices2();

    for(int i=0;i<result.faces.size();i++)
    {
        for(int p=0;p<result.faces[i].loops.size();p++)
        {
            if(result.faces[i].loops[p].vertices.size()>0)
            {
                if(result.faces[i].loops[p].topo_group==LOOP_HOLLOW || result.faces[i].loops[p].topo_group==LOOP_GHOST_HOLLOW)
                    result.set_loop_open(i,p);
                else
                    result.set_loop_solid(i,p);
            }
        }
    }

    result.recalc_bbox();

    result.topology = pf.topology;
    result.surface_groups = pf.surface_groups;

    return result;
}

//=====================================================================================
//
//
//

polyfold clip_poly2(polyfold& pf, polyfold& pf2, int rule, int rule2, clip_results& results, LineHolder &graph)
{
    LineHolder nograph;

    std::cout<<"Creating new geometry (2) ";
    std::cout<<pf.topology<<","<<pf2.topology<<","<<rule<<","<<rule2<<"\n";

    polyfold pf4 = pf;
    polyfold pf5 = pf2;
    polyfold ret;

    do_self_intersections(pf5);

    int n_intersections = do_intersections_and_bisect2(pf,pf2,pf4,pf5,graph);

    results.n_intersections = n_intersections;
    if(n_intersections == 0)
    {
        return pf;
    }

    pf5.classify_edges3(nograph);

    do_common_topology(pf,pf2,pf4,pf5,graph);

    do_initial_topology(pf,pf2,pf4,pf5,nograph);

    do_topology_groups(pf4,pf5,rule,rule2,nograph);

    for(int f_i=0; f_i<pf4.faces.size();f_i++)
    {
        pf4.calc_loops4(f_i,nograph);
        for(int p_i=0; p_i< pf4.faces[f_i].loops.size(); p_i++)
            pf4.set_loop_solid(f_i,p_i);
    }

    for(int f_i=0; f_i<pf5.faces.size();f_i++)
    {
        pf5.calc_loops4(f_i,nograph);
    }

    for(int f_i=0; f_i < pf4.faces.size(); f_i++)
    {
        pf4.sort_loops(f_i);
    }


    do_self_topology_loops(pf4,pf,nograph);
    do_self_topology_loops(pf5,pf2,nograph);

    polyfold positive;
    polyfold negative;

    positive.vertices = pf4.vertices;
    negative.vertices = pf4.vertices;

    for(int f_i=0; f_i < pf4.faces.size(); f_i++)
    {
        core::plane3df f_plane = core::plane3df(pf4.faces[f_i].m_center,pf4.faces[f_i].m_normal);
        poly_face f_p;
        poly_face f_n;
        f_p.m_center = pf4.faces[f_i].m_center;
        f_p.m_normal = pf4.faces[f_i].m_normal;
        f_n.m_center = pf4.faces[f_i].m_center;
        f_n.m_normal = pf4.faces[f_i].m_normal;
        f_p.bFlippedNormal = pf4.faces[f_i].bFlippedNormal;
        f_n.bFlippedNormal = pf4.faces[f_i].bFlippedNormal;

        if(pf4.faces[f_i].loops.size()>0)
        {
            for(int p_i=0; p_i < pf4.faces[f_i].loops.size(); p_i++)
            {
                if(pf4.faces[f_i].loops[p_i].vertices.size()>0 && pf4.faces[f_i].loops[p_i].topo_group != LOOP_UNDEF)
                {
                    for(int f_j=0; f_j < pf5.faces.size(); f_j++)
                    {
                        if(is_coplanar_point(f_plane,pf5.faces[f_j].m_center) &&
                           is_parallel_normal(pf4.faces[f_i].m_normal,pf2.faces[f_j].m_normal))
                            {
                                for(int p_j=0; p_j < pf5.faces[f_j].loops.size(); p_j++)
                                {
                                    if(pf5.faces[f_j].loops[p_j].vertices.size() > 0 &&
                                       pf5.faces[f_j].loops[p_j].topo_group != LOOP_UNDEF &&
                                       is_identical_loop(pf4, pf4.faces[f_i].loops[p_i],pf5, pf5.faces[f_j].loops[p_j]))
                                    {
                                         if(pf4.faces[f_i].m_normal.dotProduct(pf5.faces[f_j].m_normal) > 0)
                                         {
                                             if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_SOLID && pf4.faces[f_i].loops[p_i].topo_group==LOOP_SOLID)
                                             {
                                                f_p.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop2;
                                             }
                                             else if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_HOLLOW && pf4.faces[f_i].loops[p_i].topo_group==LOOP_SOLID)
                                             {
                                                f_p.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop2;
                                             }
                                             else if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_SOLID && pf4.faces[f_i].loops[p_i].topo_group==LOOP_HOLLOW)
                                             {
                                                 if(rule==GEO_ADD)
                                                 {
                                                    core::vector3df v1;
                                                    int e_i = pf4.find_edge(pf4.faces[f_i].loops[p_i].vertices[0],pf4.faces[f_i].loops[p_i].vertices[1]);
                                                    if(pf4.get_facial_point(f_i,e_i,p_i,v1,nograph))
                                                    {
                                                        if(pf4.classify_point(v1,graph) == TOP_FRONT)
                                                            f_p.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                        else
                                                            f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                    }

                                                 }
                                                 //if(rule==GEO_ADD)
                                                 //   f_p.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                 else
                                                    f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop2;
                                             }
                                             else if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_HOLLOW && pf4.faces[f_i].loops[p_i].topo_group==LOOP_HOLLOW)
                                             {
                                                f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop2;
                                             }
                                         }
                                         else
                                         {
                                            if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_SOLID && pf4.faces[f_i].loops[p_i].topo_group==LOOP_SOLID)
                                             {
                                                f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop2;
                                             }
                                             else if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_HOLLOW && pf4.faces[f_i].loops[p_i].topo_group==LOOP_SOLID)
                                             {
                                                f_p.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop2;
                                             }
                                             else if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_SOLID && pf4.faces[f_i].loops[p_i].topo_group==LOOP_HOLLOW)
                                             {
                                                if(rule==GEO_ADD)
                                                    f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                 else
                                                    f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop2;
                                             }
                                             else if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_HOLLOW && pf4.faces[f_i].loops[p_i].topo_group==LOOP_HOLLOW)
                                             {
                                                f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop2;
                                             }
                                         }
                                    }
                                }
                            }
                    }

                    int RES = classify_loop(pf4,f_i,p_i,pf2,rule,rule2,graph);

                    if(RES==TOP_FRONT)
                    {
                        if(pf4.faces[f_i].loops[p_i].topo_group==LOOP_SOLID)
                            f_p.loops.push_back( pf4.faces[f_i].loops[p_i]);
                        else if(pf4.faces[f_i].loops[p_i].topo_group==LOOP_HOLLOW)
                            f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                    }
                    else if(RES==TOP_BEHIND)
                    {
                        f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                     //   if(pf4.faces[f_i].loops[p_i].topo_group==LOOP_SOLID)
                     //       f_g.loops.push_back( pf4.faces[f_i].loops[p_i]);
                    //    else if(pf4.faces[f_i].loops[p_i].topo_group==LOOP_HOLLOW)
                     //       f_gn.loops.push_back( pf4.faces[f_i].loops[p_i]);
                    }

                }
                FinishedWithLoop2:
                int z=0;
            }
        }
        positive.faces.push_back(f_p);
        negative.faces.push_back(f_n);
    }

    positive.reduce_edges_vertices2();
    negative.reduce_edges_vertices2();

    for(int f_i=0;f_i<positive.faces.size();f_i++)
        positive.meld_loops(f_i);

    for(int f_i=0;f_i<negative.faces.size();f_i++)
        negative.meld_loops(f_i);

    polyfold result;

    for(int i=0;i<pf4.faces.size();i++)
    {
        poly_face f;
        f.m_center = pf4.faces[i].m_center;
        f.m_normal = pf4.faces[i].m_normal;
        f.bFlippedNormal = pf4.faces[i].bFlippedNormal;
        f.surface_group = pf4.faces[i].surface_group;
        f.original_brush = pf4.faces[i].original_brush;
        f.original_face = pf4.faces[i].original_face;

        for(int p=0;p<positive.faces[i].loops.size();p++)
        {
            {
                poly_loop loop;
                for(int v_i=0;v_i<positive.faces[i].loops[p].vertices.size();v_i++)
                {
                    core::vector3df V = positive.vertices[positive.faces[i].loops[p].vertices[v_i]].V;
                    int v = result.get_point_or_add(V);
                    loop.vertices.push_back(v);
                }
                loop.topo_group=LOOP_SOLID;
                f.loops.push_back(loop);
            }
        }

        for(int p=0;p<negative.faces[i].loops.size();p++)
        {
            poly_loop loop;
            for(int v_i=0;v_i<negative.faces[i].loops[p].vertices.size();v_i++)
            {
                core::vector3df V = negative.vertices[negative.faces[i].loops[p].vertices[v_i]].V;
                int v = result.get_point_or_add(V);
                loop.vertices.push_back(v);
            }
            loop.topo_group=LOOP_HOLLOW;
            f.loops.push_back(loop);
        }
        result.faces.push_back(f);
    }

    result.finalize_clipped_poly();

    result.topology = pf.topology;
    result.surface_groups = pf.surface_groups;

    return result;
}

//=====================================================================================
//
//
//


polyfold clip_poly3(polyfold& pf, polyfold& pf2, int rule, int rule2, clip_results& results, LineHolder &graph)
{
    LineHolder nograph;

    std::cout<<"Creating new geometry (3)...\n";
    std::cout<<pf.topology<<","<<pf2.topology<<","<<rule<<","<<rule2<<"\n";

    polyfold pf4 = pf;
    polyfold pf5 = pf2;
    polyfold ret;

    do_self_intersections(pf5);

    int n_intersections = do_intersections_and_bisect2(pf,pf2,pf4,pf5,nograph);

    results.n_intersections = n_intersections;
    if(n_intersections == 0)
    {
        std::cout<<"no intersections...\n";
        return pf;
    }

    pf5.classify_edges3(nograph);

    do_common_topology(pf,pf2,pf4,pf5,nograph);

    do_initial_topology(pf,pf2,pf4,pf5,nograph);

    do_topology_groups(pf4,pf5,rule,rule2,nograph);

    for(int f_i=0; f_i<pf4.faces.size();f_i++)
    {

        pf4.calc_loops4(f_i,nograph);
        for(int p_i=0; p_i< pf4.faces[f_i].loops.size(); p_i++)
            pf4.set_loop_solid(f_i,p_i);
    }

    for(int f_i=0; f_i<pf5.faces.size();f_i++)
    {
        pf5.calc_loops4(f_i,nograph);
    }

    for(int f_i=0; f_i < pf4.faces.size(); f_i++)
    {
        pf4.sort_loops(f_i);
    }

    do_self_topology_loops(pf4,pf,nograph);

    do_self_topology_loops(pf5,pf2,nograph);

    polyfold positive;
    polyfold negative;

    positive.vertices = pf4.vertices;
    negative.vertices = pf4.vertices;

    for(int f_i=0; f_i < pf4.faces.size(); f_i++)
    {
        core::plane3df f_plane = core::plane3df(pf4.faces[f_i].m_center,pf4.faces[f_i].m_normal);
        poly_face f_p;
        poly_face f_n;
        f_p.m_center = pf4.faces[f_i].m_center;
        f_p.m_normal = pf4.faces[f_i].m_normal;
        f_n.m_center = pf4.faces[f_i].m_center;
        f_n.m_normal = pf4.faces[f_i].m_normal;
        f_p.bFlippedNormal = pf4.faces[f_i].bFlippedNormal;
        f_n.bFlippedNormal = pf4.faces[f_i].bFlippedNormal;

        if(pf4.faces[f_i].loops.size()>0)
        {
            for(int p_i=0; p_i < pf4.faces[f_i].loops.size(); p_i++)
            {
                if(pf4.faces[f_i].loops[p_i].vertices.size()>0 && pf4.faces[f_i].loops[p_i].topo_group != LOOP_UNDEF)
                {
                    for(int f_j=0; f_j < pf5.faces.size(); f_j++)
                    {
                        if(is_coplanar_point(f_plane,pf5.faces[f_j].m_center) &&
                           is_parallel_normal(pf4.faces[f_i].m_normal,pf2.faces[f_j].m_normal))
                            {
                                for(int p_j=0; p_j < pf5.faces[f_j].loops.size(); p_j++)
                                {
                                    if(pf5.faces[f_j].loops[p_j].vertices.size() > 0 &&
                                       pf5.faces[f_j].loops[p_j].topo_group != LOOP_UNDEF &&
                                       is_identical_loop(pf4, pf4.faces[f_i].loops[p_i],pf5, pf5.faces[f_j].loops[p_j]))
                                    {
                                         if(pf4.faces[f_i].m_normal.dotProduct(pf5.faces[f_j].m_normal) > 0)
                                         {
                                             if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_SOLID && pf4.faces[f_i].loops[p_i].topo_group==LOOP_SOLID)
                                             {
                                                f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop2;
                                             }
                                             else if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_HOLLOW && pf4.faces[f_i].loops[p_i].topo_group==LOOP_SOLID)
                                             {
                                                f_p.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop2;
                                             }
                                             else if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_SOLID && pf4.faces[f_i].loops[p_i].topo_group==LOOP_HOLLOW)
                                             {
                                                 if(rule==GEO_ADD)
                                                    f_p.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                 else
                                                    f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop2;
                                             }
                                             else if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_HOLLOW && pf4.faces[f_i].loops[p_i].topo_group==LOOP_HOLLOW)
                                             {
                                                f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop2;
                                             }
                                         }
                                         else
                                         {
                                            if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_SOLID && pf4.faces[f_i].loops[p_i].topo_group==LOOP_SOLID)
                                             {
                                                f_p.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop2;
                                             }
                                             else if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_HOLLOW && pf4.faces[f_i].loops[p_i].topo_group==LOOP_SOLID)
                                             {
                                                f_p.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop2;
                                             }
                                             else if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_SOLID && pf4.faces[f_i].loops[p_i].topo_group==LOOP_HOLLOW)
                                             {
                                                if(rule==GEO_ADD)
                                                    f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                 else
                                                    f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop2;
                                             }
                                             else if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_HOLLOW && pf4.faces[f_i].loops[p_i].topo_group==LOOP_HOLLOW)
                                             {
                                                f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop2;
                                             }
                                         }
                                    }
                                }
                            }
                    }

                    int RES = classify_loop(pf4,f_i,p_i,pf2,rule,rule2,graph);

                    if(RES==TOP_FRONT)
                    {
                        if(pf4.faces[f_i].loops[p_i].topo_group==LOOP_SOLID)
                            f_p.loops.push_back( pf4.faces[f_i].loops[p_i]);
                        else if(pf4.faces[f_i].loops[p_i].topo_group==LOOP_HOLLOW)
                            f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                    }
                    else if(RES==TOP_BEHIND)
                    {
                        f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                     //   if(pf4.faces[f_i].loops[p_i].topo_group==LOOP_SOLID)
                     //       f_g.loops.push_back( pf4.faces[f_i].loops[p_i]);
                    //    else if(pf4.faces[f_i].loops[p_i].topo_group==LOOP_HOLLOW)
                     //       f_gn.loops.push_back( pf4.faces[f_i].loops[p_i]);
                    }

                }
                FinishedWithLoop2:
                int z=0;
            }
        }
        positive.faces.push_back(f_p);
        negative.faces.push_back(f_n);
    }

    positive.reduce_edges_vertices2();
    negative.reduce_edges_vertices2();

    for(int f_i=0;f_i<positive.faces.size();f_i++)
        positive.meld_loops(f_i);

    for(int f_i=0;f_i<negative.faces.size();f_i++)
        negative.meld_loops(f_i);

    polyfold result;

    for(int i=0;i<pf4.faces.size();i++)
    {
        poly_face f;
        f.m_center = pf4.faces[i].m_center;
        f.m_normal = pf4.faces[i].m_normal;
        f.bFlippedNormal = pf4.faces[i].bFlippedNormal;
        f.surface_group = pf4.faces[i].surface_group;
        f.original_face = pf4.faces[i].original_face;

        for(int p=0;p<positive.faces[i].loops.size();p++)
        {
            {
                poly_loop loop;
                for(int v_i=0;v_i<positive.faces[i].loops[p].vertices.size();v_i++)
                {
                    core::vector3df V = positive.vertices[positive.faces[i].loops[p].vertices[v_i]].V;
                    int v = result.get_point_or_add(V);
                    loop.vertices.push_back(v);
                }
                loop.topo_group=LOOP_SOLID;
                f.loops.push_back(loop);
            }
        }

        for(int p=0;p<negative.faces[i].loops.size();p++)
        {
            poly_loop loop;
            for(int v_i=0;v_i<negative.faces[i].loops[p].vertices.size();v_i++)
            {
                core::vector3df V = negative.vertices[negative.faces[i].loops[p].vertices[v_i]].V;
                int v = result.get_point_or_add(V);
                loop.vertices.push_back(v);
            }
            loop.topo_group=LOOP_HOLLOW;
            f.loops.push_back(loop);
        }
        result.faces.push_back(f);
    }


    result.finalize_clipped_poly();

    result.topology = pf.topology;
    result.surface_groups = pf.surface_groups;

    return result;
}


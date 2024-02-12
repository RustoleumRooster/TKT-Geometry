
#include <irrlicht.h>
#include <iostream>
#include "csg_classes.h"
#include "clip_functions.h"
#include "tolerances.h"




void do_topology_groups(polyfold& pf4, polyfold& pf2, int rule, int rule2, LineHolder& graph)
{
    if(pf4.apply_topology_groups2(pf2, rule2) == false)
    {
        std::cout<<"(error) unresolved topology... using default value\n";

        for(int e_i = 0; e_i<pf4.edges.size(); e_i++)
        {
            if(pf4.edges[e_i].topo_group==2)
            {
                if(pf2.topology == TOP_CONCAVE)
                    pf4.edges[e_i].topo_group = TOP_BEHIND;
                else if(pf2.topology == TOP_CONVEX)
                    pf4.edges[e_i].topo_group = TOP_FRONT;
            }
        }
    }

    if(rule == GEO_SUBTRACT)
        for(int e_i = 0; e_i<pf4.edges.size(); e_i++)
        {
            if(pf4.edges[e_i].topo_group == 1 || pf4.edges[e_i].topo_group == 0)
            {
                pf4.edges[e_i].topo_group = !pf4.edges[e_i].topo_group;
            }
        }
}


void do_intersections_and_bisect(polyfold& pf, polyfold& pf2, polyfold& pf4, polyfold& pf5,LineHolder& graph)
{

    for(poly_edge &edge : pf4.edges)
    {
     edge.topo_group=2;
    }

    for(poly_edge &edge : pf5.edges)
    {
     edge.topo_group=2;
    }

    //intersections and bisect - pf4

    for(int face_j=0;face_j<pf5.faces.size();face_j++)
    {
        poly_face f2=pf5.faces[face_j];
        core::plane3df f2_plane = core::plane3df(f2.m_center,f2.m_normal);
        core::vector3df ipoint;

        for(int e_i=0; e_i<pf4.edges.size(); e_i++)
        {
            core::vector3df v0=pf4.getVertex(e_i,0).V;
            core::vector3df v1=pf4.getVertex(e_i,1).V;
            if(line_intersects_plane(f2_plane,v0,v1,ipoint) && pf4.edges[e_i].topo_group != -1
               && pf5.is_point_on_face_any_loop(face_j,ipoint))
            {
                int new_v = pf4.get_point_or_add(ipoint);
                pf4.bisect_edge(e_i,new_v,2,2);
            }
        }
    }

    //intersections and bisect - pf5
    for(int face_j=0;face_j<pf.faces.size();face_j++)
    {
        poly_face f=pf.faces[face_j];
        core::plane3df f_plane = core::plane3df(f.m_center,f.m_normal);
        core::vector3df ipoint;

        for(int e_i=0; e_i<pf5.edges.size(); e_i++)
        {
            core::vector3df v0=pf5.getVertex(e_i,0).V;
            core::vector3df v1=pf5.getVertex(e_i,1).V;
            if(line_intersects_plane(f_plane,v0,v1,ipoint) && pf5.edges[e_i].topo_group != -1
               && pf.is_point_on_face_any_loop(face_j,ipoint))
            {
                int new_v = pf5.get_point_or_add(ipoint);
                pf5.bisect_edge(e_i,new_v,2,2);
            }
        }
    }

    for(int v_i=0; v_i<pf5.vertices.size(); v_i++)
    {
        if(pf4.find_point(pf5.vertices[v_i].V) == -1)
           {
               //graph.points.push_back(pf5.vertices[v_i].V);
            for(int e_i=0; e_i<pf4.edges.size(); e_i++)
                {
                    if(pf4.point_is_on_edge(e_i,pf5.vertices[v_i].V))
                    {
                        //graph.points.push_back(pf5.vertices[v_i].V);
                        int new_v = pf4.get_point_or_add(pf5.vertices[v_i].V);
                        pf4.bisect_edge(e_i,new_v,2,2);
                    }
                }
           }
    }

    for(int v_i=0; v_i<pf4.vertices.size(); v_i++)
    {
        if(pf5.find_point(pf4.vertices[v_i].V) == -1)
           {
            for(int e_i=0; e_i<pf5.edges.size(); e_i++)
                {
                    if(pf5.point_is_on_edge(e_i,pf4.vertices[v_i].V))
                    {
                        int new_v = pf5.get_point_or_add(pf4.vertices[v_i].V);
                        pf5.bisect_edge(e_i,new_v,2,2);
                    }
                }
           }
    }

    //initial topology - pf4
    for(int face_j=0;face_j<pf2.faces.size();face_j++)
    {
        poly_face f2=pf2.faces[face_j];
        core::plane3df f2_plane = core::plane3df(f2.m_center,f2.m_normal);
        core::vector3df ipoint;

        for(int e_i=0; e_i<pf4.edges.size(); e_i++)
        {
            core::vector3df v0 = pf4.getVertex(e_i,0).V;
            core::vector3df v1 = pf4.getVertex(e_i,1).V;
            if(line_intersects_plane(f2_plane,v0,v1,ipoint) && pf4.edges[e_i].topo_group != -1
               && pf5.is_point_on_face_any_loop(face_j,ipoint))
            {
                int new_v = pf4.get_point_or_add(ipoint);

                if(new_v == pf4.edges[e_i].v0)
                {
                    v0 = pf4.getVertex(e_i,0).V;
                    v1 = pf4.getVertex(e_i,1).V;
                }
                else
                {
                    v1 = pf4.getVertex(e_i,0).V;
                    v0 = pf4.getVertex(e_i,1).V;
                }

               // std::cout<<"v0: "<<v0.X<<","<<v0.Y<<","<<v0.Z<<"\n";
               // std::cout<<"v1: "<<v1.X<<","<<v1.Y<<","<<v1.Z<<"\n";
               // graph.points.push_back(ipoint);

                if(pf5.is_point_on_face(face_j,ipoint))
                {
                    int RES = pf2.classify_point(face_j,v0,v1);

                    if(RES == TOP_FRONT)
                        pf4.edges[e_i].topo_group=0;
                    else if(RES == TOP_BEHIND)
                        pf4.edges[e_i].topo_group=1;
                }

                new_v = pf5.get_point_or_add(ipoint);
                pf5.faces[face_j].addVertex(new_v);
            }
        }
    }

    //initial topology - pf5

    std::cout<<"note: pf5 initial topology disabled\n";
    if(false)
    for(int face_j=0;face_j<pf.faces.size();face_j++)
    {
        poly_face f=pf.faces[face_j];
        core::plane3df f2_plane = core::plane3df(f.m_center,f.m_normal);
        core::vector3df ipoint;

        for(int e_i=0; e_i<pf5.edges.size(); e_i++)
        {
            core::vector3df v0 = pf5.getVertex(e_i,0).V;
            core::vector3df v1 = pf5.getVertex(e_i,1).V;
            if(line_intersects_plane(f2_plane,v0,v1,ipoint) && pf5.edges[e_i].topo_group != -1
               && pf.is_point_on_face_any_loop(face_j,ipoint))
            {
                int new_v = pf5.get_point_or_add(ipoint);

                if(new_v == pf5.edges[e_i].v0)
                {
                    v0 = pf5.getVertex(e_i,0).V;
                    v1 = pf5.getVertex(e_i,1).V;
                }
                else
                {
                    v1 = pf5.getVertex(e_i,0).V;
                    v0 = pf5.getVertex(e_i,1).V;
                }

                if(pf.is_point_on_face(face_j,ipoint))
                {
                    int RES = pf.classify_point(face_j,v0,v1);

                    if(RES == TOP_FRONT)
                        pf5.edges[e_i].topo_group=0;
                    else if(RES == TOP_BEHIND)
                        pf5.edges[e_i].topo_group=1;
                }

                new_v = pf4.get_point_or_add(ipoint);
                pf4.faces[face_j].addVertex(new_v);
            }
        }
    }
}


void do_common_topology(polyfold& pf, polyfold& pf2, polyfold& pf4, polyfold& pf5, LineHolder& graph)
{
    for(int face_i=0;face_i<pf.faces.size();face_i++)
        for(int face_j=0;face_j<pf2.faces.size();face_j++)
        {
            poly_face f=pf.faces[face_i];
            poly_face f2=pf2.faces[face_j];

            plane res;

            core::plane3df f_plane = core::plane3df(f.m_center,f.m_normal);
            core::plane3df f2_plane = core::plane3df(f2.m_center,f2.m_normal);

            //if(f_plane.getIntersectionWithPlane(f2_plane,res.point,res.norm) && fabs(f.m_normal.dotProduct(f2.m_normal))<PLANE_IS_COPLANAR_DOT)
            if(f_plane.getIntersectionWithPlane(f2_plane,res.point,res.norm) && is_parallel_normal(f.m_normal,f2.m_normal) == false)
            {
                polyfold verts;
                for(int v_i : pf4.faces[face_i].vertices)
                {
                    core::vector3df v = pf4.vertices[v_i].V;
                   // if(fabs(f2_plane.getDistanceTo(v))<POINT_IS_COPLANAR_DIST && pf5.is_point_on_face_any_loop(face_j,v))
                    if(is_coplanar_point(f2_plane,v) && pf5.is_point_on_face_any_loop(face_j,v))
                    {
                        verts.get_point_or_add(v);
                        //if(face_i==4)
                       // graph.points.push_back(v);

                    }


                }

                for(int v_j : pf5.faces[face_j].vertices)
                {
                    core::vector3df v = pf5.vertices[v_j].V;
                    //if(fabs(f_plane.getDistanceTo(v))<POINT_IS_COPLANAR_DIST && pf.is_point_on_face_any_loop(face_i,v))
                    if(is_coplanar_point(f_plane,v) && pf.is_point_on_face_any_loop(face_i,v))
                    {
                        verts.get_point_or_add(v);
                    }
                    //if(face_j==4)
                    //    graph.points.push_back(v);
                }


                if(verts.vertices.size()>1)
                {
                    sort_inline_vertices(verts);
                    for(int i=0;i<verts.vertices.size()-1;i++)
                    {
                            //if(face_i==0 && face_j==1)
                            //if(verts.vertices.size()==3)
                            {

                        //graph.points.push_back(verts.vertices[i+1].V);
                        //graph.lines.push_back(core::line3df(verts.vertices[i].V,verts.vertices[i+1].V));
                            }

                        core::vector3df r = verts.vertices[i].V + (verts.vertices[i+1].V - verts.vertices[i].V)*0.5;
                        if(pf.is_point_on_face_any_loop(face_i,r) &&
                          ( pf5.is_point_on_face_any_loop(face_j,r)  ))
                        {
                            int v0 = pf4.get_point_or_add(verts.vertices[i].V);
                            int v1 = pf4.get_point_or_add(verts.vertices[i+1].V);
                            int w0 = pf5.get_point_or_add(verts.vertices[i].V);
                            int w1 = pf5.get_point_or_add(verts.vertices[i+1].V);

                            if(face_j==4)//&& i==2)
                            {
                            //graph.points.push_back(verts.vertices[i].V);
                            //graph.points.push_back(verts.vertices[i+1].V);
                            //graph.lines.push_back(core::line3df(verts.vertices[i].V,verts.vertices[i+1].V));
                            }

                            int new_e;

                            if(pf4.find_edge(v0,v1)==-1)
                            {
                                new_e = pf4.get_edge_or_add(v0,v1,3);
                                pf4.faces[face_i].addEdge(new_e);
                            }
                            else
                            {
                                new_e = pf4.find_edge(v0,v1);
                                pf4.edges[new_e].topo_group=3;
                            }

                            if(pf5.find_edge(w0,w1)==-1)
                            {
                                new_e = pf5.get_edge_or_add(w0,w1,3);
                                std::cout<<new_e<<" (common top)\n";
                                pf5.faces[face_j].addEdge(new_e);
                            }
                            else
                            {
                                new_e = pf5.find_edge(w0,w1);
                                pf5.edges[new_e].topo_group=3;
                                pf5.faces[face_j].addEdge(new_e);
                            }

                        }
                    }
                }
            }
        }

     for(int face_i=0;face_i<pf.faces.size();face_i++)
        for(int face_j=0;face_j<pf2.faces.size();face_j++)
        {
            poly_face f=pf.faces[face_i];
            poly_face f2=pf2.faces[face_j];

            plane res;

            core::plane3df f_plane = core::plane3df(f.m_center,f.m_normal);
            core::plane3df f2_plane = core::plane3df(f2.m_center,f2.m_normal);

           // if(fabs(f_plane.getDistanceTo(pf5.faces[face_j].m_center))<POINT_IS_COPLANAR_DIST &&
           //                fabs(pf4.faces[face_i].m_normal.dotProduct(pf2.faces[face_j].m_normal))>PLANE_IS_COPLANAR_DOT)
           if(is_coplanar_point(f_plane,pf5.faces[face_j].m_center) && is_parallel_normal(pf4.faces[face_i].m_normal,pf2.faces[face_j].m_normal))
            {
                for(int e_i: pf5.faces[face_j].edges)
                {
                   core::vector3df v0 = pf5.getVertex(e_i,0).V;
                   core::vector3df v1 = pf5.getVertex(e_i,1).V;
                   int v_i0 = pf4.find_point(v0);
                   int v_i1 = pf4.find_point(v1);
                   if( v_i0 != -1 && v_i1 !=-1 /*&&
                      pf4.find_edge(v_i0,v_i1)==-1*/ && pf.is_point_on_face_any_loop(face_i,v0) && pf.is_point_on_face_any_loop(face_i,v1))
                        {
                            int new_e = pf4.get_edge_or_add(v_i0,v_i1,3);
                            pf4.edges[new_e].topo_group=3;
                            pf4.faces[face_i].addEdge(new_e);
                           //graph.lines.push_back(core::line3df(v0,v1));
                        }
                     //graph.lines.push_back(core::line3df(v0,v1));
                }
                for(int e_i: pf4.faces[face_i].edges)
                {
                   core::vector3df v0 = pf4.getVertex(e_i,0).V;
                   core::vector3df v1 = pf4.getVertex(e_i,1).V;
                   int v_i0 = pf5.find_point(v0);
                   int v_i1 = pf5.find_point(v1);
                   if( v_i0 != -1 && v_i1 !=-1 &&
                      pf5.find_edge(v_i0,v_i1)==-1 && pf2.is_point_on_face_any_loop(face_j,v0) && pf2.is_point_on_face_any_loop(face_j,v1))
                        {
                            int new_e = pf5.get_edge_or_add(v_i0,v_i1,3);
                            pf5.edges[new_e].topo_group=3;
                            pf5.faces[face_i].addEdge(new_e);
                           graph.lines.push_back(core::line3df(v0,v1));
                        }
                    //if( v_i0 != -1 && v_i1 !=-1)
                   // graph.lines.push_back(core::line3df(v0,v1));
                }
            }

        }

}


polyfold clip_poly2(polyfold& pf, polyfold& pf2, int rule, int rule2, LineHolder &graph)
{
    LineHolder nograph;
    pf.classify_edges(nograph);
    pf2.classify_edges(nograph);

    polyfold pf4 = pf;
    polyfold pf5 = pf2;

    std::cout<<"Creating new geometry (1)...  \n";

    do_intersections_and_bisect(pf,pf2,pf4,pf5,graph);


    do_common_topology(pf,pf2,pf4,pf5,graph);

    do_topology_groups(pf4,pf2,rule,rule2,graph);

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

    do_self_topology_loops(pf4,pf);
    do_self_topology_loops(pf5,pf2);

    polyfold positive;
    polyfold negative;
    polyfold ghost;
    polyfold ghost_negative;

    positive.vertices = pf4.vertices;
    negative.vertices = pf4.vertices;
    ghost.vertices = pf4.vertices;
    ghost_negative.vertices = pf4.vertices;

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
                                         if(pf4.faces[f_i].m_normal.dotProduct(pf5.faces[f_j].m_normal) > 0)
                                         {
                                             if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_SOLID && pf4.faces[f_i].loops[p_i].topo_group==LOOP_SOLID)
                                             {
                                                f_g.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop;
                                             }
                                             else if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_HOLLOW && pf4.faces[f_i].loops[p_i].topo_group==LOOP_SOLID)
                                             {
                                                f_g.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop;
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
                                                    f_p.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                 else
                                                    f_g.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                goto FinishedWithLoop;
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
                    int RES = classify_loop(pf4,f_i,p_i,pf2,rule,rule2,false,graph);

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

    return result;
}

//==================================================================================================================================================
//
//
//
//
//

//==================================================================================================================================================

/*
polyfold clip_poly3(polyfold& pf, polyfold& pf2, int rule, int rule2, LineHolder &graph)
{
    LineHolder nograph;

    //edge topology
    pf.classify_edges(nograph);
    pf2.classify_edges(nograph);

    polyfold pf4 = pf;
    polyfold pf5 = pf2;

    std::cout<<"Creating new geometry...\n";

    do_intersections_and_bisect(pf,pf2,pf4,pf5,graph);

    do_common_topology(pf,pf2,pf4,pf5,graph);

    do_topology_groups(pf4,pf2,rule,rule2,graph);

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

    do_self_topology_loops(pf4,pf);
    do_self_topology_loops(pf5,pf2);

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

        if(pf4.faces[f_i].loops.size()>0)
        {
            for(int p_i=0; p_i < pf4.faces[f_i].loops.size(); p_i++)
            {
                if(pf4.faces[f_i].loops[p_i].vertices.size()>0 && pf4.faces[f_i].loops[p_i].topo_group != LOOP_UNDEF)
                {
                    for(int f_j=0; f_j < pf5.faces.size(); f_j++)
                    {
                       //if(fabs(f_plane.getDistanceTo(pf5.faces[f_j].m_center))<POINT_IS_COPLANAR_DIST &&
                       //    fabs(pf4.faces[f_i].m_normal.dotProduct(pf2.faces[f_j].m_normal))>PLANE_IS_COPLANAR_DOT)
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
                                       // goto FinishedWithLoop2;
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

                    int RES = classify_loop(pf4,f_i,p_i,pf2,rule,rule2,false,graph);

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

    polyfold result;

    for(int i=0;i<pf4.faces.size();i++)
    {
        poly_face f;
        f.m_center = pf4.faces[i].m_center;
        f.m_normal = pf4.faces[i].m_normal;

        positive.calc_loops4(i,nograph);
        positive.sort_loops(i);

        negative.calc_loops4(i,nograph);
        negative.sort_loops(i);

        for(int p=0;p<positive.faces[i].loops.size();p++)
        {
            if(positive.faces[i].loops[p].type==LOOP_OUTER)
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
            if(negative.faces[i].loops[p].type==LOOP_OUTER)
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
        result.faces.push_back(f);
    }

    result.finalize_clipped_poly();

    return result;
}*/


polyfold clip_poly3(polyfold& pf, polyfold& pf2, int rule, int rule2, LineHolder &graph)
{
    LineHolder nograph;

    pf.classify_edges(nograph);
    pf2.classify_edges(nograph);

    polyfold pf4 = pf;
    polyfold pf5 = pf2;
    polyfold ret;

    std::cout<<"Creating new geometry (2)...\n";

    do_intersections_and_bisect(pf,pf2,pf4,pf5,graph);

    do_common_topology(pf,pf2,pf4,pf5,graph);

    do_topology_groups(pf4,pf2,rule,rule2,graph);

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

    do_self_topology_loops(pf4,pf);
    do_self_topology_loops(pf5,pf2);

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

                    int RES = classify_loop(pf4,f_i,p_i,pf2,rule,rule2,false,graph);

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
/*
    for(int i=0;i<pf4.faces.size();i++)
    {
        negative.calc_loops4(i,nograph);
        for(int p=0;p<negative.faces[i].loops.size();p++)
        {
            if(negative.faces[i].loops[p].type==LOOP_INNER)
            {
                negative.faces[i].loops[p].vertices.clear();
            }
        }
    }

    negative.reduce_edges_vertices2();

    for(int i=0;i<pf4.faces.size();i++)
    {
        negative.calc_loops4(i,nograph);
        for(int p=0;p<negative.faces[i].loops.size();p++)
        {
            if(negative.faces[i].loops[p].type==LOOP_OUTER)
            {
                negative.faces[i].loops[p].vertices.clear();
            }
        }
    }

    negative.reduce_edges_vertices2();

    for(int i=0;i<pf4.faces.size();i++)
    {
        poly_face f;
        f.m_center = pf4.faces[i].m_center;
        f.m_normal = pf4.faces[i].m_normal;

        positive.calc_loops4(i,nograph);

        for(int p=0;p<positive.faces[i].loops.size();p++)
        {
            if(positive.faces[i].loops[p].type==LOOP_OUTER)
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
*/
    result.finalize_clipped_poly();

    return result;
}

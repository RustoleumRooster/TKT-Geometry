
#include <irrlicht.h>
#include <iostream>
#include "csg_classes.h"

using namespace irr;



//=====================================================


polyfold clip_poly1(polyfold& pf, polyfold& pf2, int rule, int rule2, LineHolder &graph, int pass_no)
{
    LineHolder nograph;
    pf.classify_edges(nograph);
    pf2.classify_edges(nograph);


    polyfold pf4 = pf;
    polyfold pf5 = pf2;
    polyfold ghost_loops;
    polyfold ret;

    std::cout<<"Creating new geometry...\n";

    //if(false)
    if(pass_no==CLIP_SECOND_PASS)
    {
        ghost_loops.vertices=pf5.vertices;
        for(int f_i=0;f_i<pf5.faces.size();f_i++)
        {
            poly_face face;
            face.m_center = pf5.faces[f_i].m_center;
            face.m_normal = pf5.faces[f_i].m_normal;

            for(poly_loop &p : pf5.faces[f_i].loops)
                if(p.topo_group == LOOP_HOLLOW && p.depth%2 ==0)
                {
                    face.loops.push_back(p);
                    p.vertices.clear();
                }

            ghost_loops.faces.push_back(face);

        }
        //pf5.clear_ghost_loops();
        pf5.reduce_edges_vertices2();
        pf5.recalc_bbox_and_loops();
        ghost_loops.reduce_edges_vertices2();
        ghost_loops.recalc_bbox_and_loops();
    }

   // return ghost_loops;

    for(poly_edge &edge : pf4.edges)
    {
     edge.topo_group=2;
    }

    for(poly_edge &edge : pf5.edges)
    {
     edge.topo_group=2;
    }
    int num=0;

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
               && pf5.is_point_on_face(face_j,ipoint))
            {
                int new_v = pf4.get_point_or_add(ipoint);
                pf4.bisect_edge(e_i,new_v,2,2);
                //graph.points.push_back(ipoint);
               // graph.points.push_back(v1);
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
               && pf.is_point_on_face(face_j,ipoint))
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
            for(int e_i=0; e_i<pf4.edges.size(); e_i++)
                {
                    if(pf4.point_is_on_edge(e_i,pf5.vertices[v_i].V))
                    {
                        //std::cout<<"bonus vertex!\n";
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
                        //std::cout<<"bonus vertex!\n";
                        int new_v = pf5.get_point_or_add(pf4.vertices[v_i].V);
                        pf5.bisect_edge(e_i,new_v,2,2);
                    }
                }
           }
    }

    for(poly_edge &edge : pf4.edges)
        edge.perimeter=0;

    for(poly_edge &edge : pf5.edges)
        edge.perimeter=0;

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
               && pf5.is_point_on_face(face_j,ipoint))
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

                int RES = pf2.classify_point(face_j,v0,v1);


                if(RES == TOP_FRONT)
                    pf4.edges[e_i].topo_group=0;
                else if(RES == TOP_BEHIND)
                    pf4.edges[e_i].topo_group=1;

                new_v = pf5.get_point_or_add(ipoint);
                pf5.faces[face_j].addVertex(new_v);
            }
        }
    }

    //initial topology - pf5
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
               && pf.is_point_on_face(face_j,ipoint))
            {
                int new_v = pf5.get_point_or_add(ipoint);

                //graph.points.push_back(ipoint);
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

                int RES = pf.classify_point(face_j,v0,v1);

                if(RES == TOP_FRONT)
                    pf5.edges[e_i].topo_group=0;
                else if(RES == TOP_BEHIND)
                    pf5.edges[e_i].topo_group=1;

                new_v = pf4.get_point_or_add(ipoint);
                pf4.faces[face_j].addVertex(new_v);
            }
        }
    }

    //return pf4;

    //common topology
    for(int face_i=0;face_i<pf.faces.size();face_i++)
        for(int face_j=0;face_j<pf2.faces.size();face_j++)
        {
            poly_face f=pf.faces[face_i];
            poly_face f2=pf2.faces[face_j];

            plane res;

            core::plane3df f_plane = core::plane3df(f.m_center,f.m_normal);
            core::plane3df f2_plane = core::plane3df(f2.m_center,f2.m_normal);

            if(f_plane.getIntersectionWithPlane(f2_plane,res.point,res.norm))
            {

                polyfold verts;
                for(int v_i : pf4.faces[face_i].vertices)
                {
                    core::vector3df v = pf4.vertices[v_i].V;
                    if(fabs(f2_plane.getDistanceTo(v))<0.01 && (pf5.is_point_on_face(face_j,v) || (pass_no == CLIP_SECOND_PASS && ghost_loops.is_point_on_face(face_j,v))))
                    {
                        verts.get_point_or_add(v);
                    }
                }

                for(int v_j : pf5.faces[face_j].vertices)
                {
                    core::vector3df v = pf5.vertices[v_j].V;
                    if(fabs(f_plane.getDistanceTo(v))<0.01 && pf.is_point_on_face(face_i,v))
                    {
                        verts.get_point_or_add(v);
                    }
                }

                if(verts.vertices.size()>0)
                {
                    sort_inline_vertices(verts);
                    for(int i=0;i<verts.vertices.size()-1;i++)
                    {
                        //graph.points.push_back(verts.vertices[i].V);
                       // graph.points.push_back(verts.vertices[i+1].V);
                       // graph.lines.push_back(core::line3df(verts.vertices[i].V,verts.vertices[i+1].V));

                        core::vector3df r = verts.vertices[i].V + (verts.vertices[i+1].V - verts.vertices[i].V)*0.5;
                        if(pf.is_point_on_face(face_i,r) &&
                          ( pf5.is_point_on_face(face_j,r)  ))
                        {
                            int v0 = pf4.get_point_or_add(verts.vertices[i].V);
                            int v1 = pf4.get_point_or_add(verts.vertices[i+1].V);
                            int w0 = pf5.get_point_or_add(verts.vertices[i].V);
                            int w1 = pf5.get_point_or_add(verts.vertices[i+1].V);

                            //if(face_j==1)
                            {
                            //graph.points.push_back(verts.vertices[i].V);
                            //graph.points.push_back(verts.vertices[i+1].V);
                           // graph.lines.push_back(core::line3df(verts.vertices[i].V,verts.vertices[i+1].V));
                            }

                            int new_e;

                            if(pf4.find_edge(v0,v1)==-1)
                            {
                            //std::cout<<"add "<<v0<<","<<v1<<"\n";
                            new_e = pf4.get_edge_or_add(v0,v1,3);
                            pf4.edges[new_e].perimeter = 1;
                            pf4.faces[face_i].addEdge(new_e);

                            //graph.points.push_back(verts.vertices[i].V);
                            //graph.points.push_back(verts.vertices[i+1].V);

                            //new_e = pf5.get_edge_or_add(w0,w1,3);
                            //pf5.faces[face_j].addEdge(new_e);
                            }
                            else if(pf4.find_edge(v0,v1) != -1)
                            {
                            new_e = pf4.find_edge(v0,v1);
                           // if(pf4.edges[new_e].topo_group==2)
                                pf4.edges[new_e].topo_group=3;
                            }

                            if(pf5.find_edge(w0,w1)==-1)
                            {
                            new_e = pf5.get_edge_or_add(w0,w1,3);
                            pf5.edges[new_e].perimeter = 1;
                            pf5.faces[face_j].addEdge(new_e);
                            }
                            else
                            {
                            new_e = pf5.get_edge_or_add(w0,w1,3);
                            pf5.faces[face_j].addEdge(new_e);
                            }

                        }
                        else if (pf.is_point_on_face(face_i,r) && pass_no==CLIP_SECOND_PASS && ghost_loops.is_point_on_face(face_j,r))
                        {
                            std::cout<<"Using improvement Z5 \n";
                            int new_e = pf4.find_edge(verts.vertices[i].V,verts.vertices[i+1].V);
                            pf4.edges[new_e].topo_group=3;
                        }
                        //TESTING
                        else if ((pf.find_edge(verts.vertices[i].V,verts.vertices[i+1].V) != -1 ))// || pass_no==CLIP_SECOND_PASS&&ghost_loops.is_point_on_face(face_j,r))
                        {
                            std::cout<<"Using improvement Z1 "<<face_i<<" vs "<<face_j<<"\n";
                            int new_e = pf4.find_edge(verts.vertices[i].V,verts.vertices[i+1].V);
                           // graph.lines.push_back(core::line3df(verts.vertices[i].V,verts.vertices[i+1].V));
                           // if(pf4.edges[new_e].topo_group==2)
                            pf4.edges[new_e].topo_group=3;
                          //  std::cout<<new_e<<"\n";
                        }
                        else if((pf5.find_edge(verts.vertices[i].V,verts.vertices[i+1].V) != -1)&&
                                pass_no == CLIP_FIRST_PASS)
                        {
                            std::cout<<"Using improvement Z2\n";
                            int v0 = pf4.get_point_or_add(verts.vertices[i].V);
                            int v1 = pf4.get_point_or_add(verts.vertices[i+1].V);
                            //graph.points.push_back(r);
                            //graph.lines.push_back(core::line3df(verts.vertices[i].V,verts.vertices[i+1].V));
                            int new_e = pf4.get_edge_or_add(v0,v1,3);
                            //pf4.edges[new_e].perimeter = 1;
                            pf4.faces[face_i].addEdge(new_e);

                        }
                        //graph.lines.push_back(core::line3df(verts.vertices[i].V,verts.vertices[i+1].V));
                    }
                }
            }
            else if(pass_no == CLIP_SECOND_PASS &&
                    fabs(f_plane.getDistanceTo(pf5.faces[face_j].m_center))<0.001 &&
                    fabs(pf4.faces[face_i].m_normal.dotProduct(pf5.faces[face_j].m_normal))>0.999)
                    {
                         std::cout<<"Using Improvement Z4 "<<face_i<<" vs "<<face_j<<"\n";
                         for(int e_i=0;e_i<pf4.faces[face_i].edges.size();e_i++)
                         {
                             core::vector3df v0 = pf4.getVertex(face_i,e_i,0).V;
                             core::vector3df v1 = pf4.getVertex(face_i,e_i,1).V;
                             //graph.lines.push_back(core::line3df(v0,v1));

                             if ((pf4.edges[pf4.faces[face_i].edges[e_i]].topo_group!=3) &&
                                ((pf5.is_point_on_face(face_j,v0) && pf5.is_point_on_face(face_j,v1)) ||
                                (ghost_loops.is_point_on_face(face_j,v0) && ghost_loops.is_point_on_face(face_j,v1))))
                                {
                                    //graph.lines.push_back(core::line3df(v0,v1));
                                    pf4.edges[pf4.faces[face_i].edges[e_i]].topo_group=3;
                                    std::cout<<pf4.faces[face_i].edges[e_i]<<"\n";
                                }
                         }

                    }
        }

    //========================================

    //return pf4;

    //if(false)
    if(pf4.apply_topology_groups2(pf2, rule2) == false)
    {
        std::cout<<"unresolved topology... using default value\n";
        for(int e_i = 0; e_i<pf4.edges.size(); e_i++)
        {
            if(pf4.edges[e_i].topo_group==2)
                //pf4.edges[e_i].topo_group= rule2==rule ? !rule : rule;
                pf4.edges[e_i].topo_group= rule2==rule ? 1 : 0;
        }
    }


   // return pf4;

   // pf5.apply_topology_groups(!rule);




    //LineHolder nograph;
    //if(false)
    for(int f_i=0; f_i<pf4.faces.size();f_i++)
    {
        pf4.calc_loops4(f_i,nograph);
        for(int p_i=0; p_i< pf4.faces[f_i].loops.size(); p_i++)
            pf4.set_loop_solid(f_i,p_i);
       // pf4.sort_loops(pf4.faces[f_i]);
    }

    //std::cout<<"2\n";

    //std::cout<<"3\n";
    //return pf5;
    //if(false)
    for(int f_i=0; f_i<pf5.faces.size();f_i++)
    {
       // pf2.calc_loops4(f_i,nograph);
        pf5.calc_loops4(f_i,nograph);
    }
   // return pf5;
   // std::cout<<"returning\n";

    /*
        for(int p_i=0; p_i< pf5.faces[f_i].loops.size(); p_i++)
            pf5.set_loop_solid(f_i,p_i);
        //pf5.sort_loops(pf5.faces[f_i]);
    }*/


    for(int f_i=0; f_i < pf4.faces.size(); f_i++)
    {
        pf4.sort_loops(f_i);
    }



    //std::cout<<"4\n";
     for(int f_i=0; f_i < pf4.faces.size(); f_i++)
        for(int p_i=0; p_i < pf4.faces[f_i].loops.size(); p_i++)
        {
            pf4.faces[f_i].loops[p_i].topo_group = LOOP_UNDEF;
        }

    num=0;
   // return pf4;

    //ghost_loops.vertices = pf5.vertices;


    //return pf4;
    //std::cout<<"B\n";
     //if(false)
    {
        //cull loops which are kissing
        for(int f_i=0; f_i < pf4.faces.size(); f_i++)
        {
            core::plane3df f_plane = core::plane3df(pf4.faces[f_i].m_center,pf4.faces[f_i].m_normal);

            if(pf4.faces[f_i].loops.size()>0)
            {
                for(int p_i=0; p_i < pf4.faces[f_i].loops.size(); p_i++)
                {
                    if(pf4.faces[f_i].loops[p_i].vertices.size()>0)
                    {
                        for(int f_j=0; f_j < pf5.faces.size(); f_j++)
                        {
                            if(fabs(f_plane.getDistanceTo(pf5.faces[f_j].m_center))<0.001 &&
                               fabs(pf4.faces[f_i].m_normal.dotProduct(pf2.faces[f_j].m_normal))>0.999)
                                {
                                    for(int p_j=0; p_j < pf5.faces[f_j].loops.size(); p_j++)
                                    {
                                        //std::cout<<"loop "<<f_i<<" "<<p_i<<" vs "<<f_j<<" "<<p_j<<"\n";
                                        if(pass_no == CLIP_FIRST_PASS)
                                        {
                                            if(is_identical_loop(pf4, pf4.faces[f_i].loops[p_i],pf5, pf5.faces[f_j].loops[p_j]))
                                         //  && pf4.faces[f_i].loops[p_i].type==LOOP_INNER)
                                            {

                                                if(pf4.faces[f_i].m_normal.dotProduct(pf5.faces[f_j].m_normal) > 0 && pf4.faces[f_i].loops[p_i].type==LOOP_OUTER)
                                                {
                                                    /*
                                                    std::cout<<"1 solid loop "<<f_i<<" "<<p_i<<" vs "<<f_j<<" "<<p_j<<"\n";
                                                    pf4.faces[f_i].loops[p_i].topo_group = LOOP_SOLID;
                                                    num++;
                                                    goto LoopHasBeenCulled;
                                                    */
                                                }
                                                else if(pf4.faces[f_i].loops[p_i].type==LOOP_INNER)
                                                {
                                                    std::cout<<"1 hollow loop "<<f_i<<" "<<p_i<<" vs "<<f_j<<" "<<p_j<<"\n";
                                                    pf4.faces[f_i].loops[p_i].topo_group = LOOP_HOLLOW;
                                                    num++;
                                                    goto LoopHasBeenCulled;
                                                }
                                            }
                                        }
                                        else if(pass_no == CLIP_SECOND_PASS)
                                        {
                                            if( pf4.faces[f_i].loops[p_i].type==LOOP_INNER &&
                                                (is_identical_loop(pf4, pf4.faces[f_i].loops[p_i],pf5, pf5.faces[f_j].loops[p_j]) ||
                                                 is_contained_loop(pf4, pf4.faces[f_i].loops[p_i],pf5,f_j,p_j)))
                                            {
                                                if(pf4.faces[f_i].m_normal.dotProduct(pf5.faces[f_j].m_normal) > 0)
                                                {
                                                    std::cout<<"2 solid loop "<<f_i<<" "<<p_i<<" vs "<<f_j<<" "<<p_j<<"\n";
                                                    pf4.faces[f_i].loops[p_i].topo_group = LOOP_SOLID;
                                                    num++;
                                                    goto LoopHasBeenCulled;
                                                }
                                                else
                                                {
                                                    std::cout<<"2 hollow loop "<<f_i<<" "<<p_i<<" vs "<<f_j<<" "<<p_j<<"\n";
                                                    pf4.faces[f_i].loops[p_i].topo_group = LOOP_HOLLOW;
                                                    num++;
                                                    goto LoopHasBeenCulled;
                                                }
                                            }
                                            else if( pf4.faces[f_i].loops[p_i].type==LOOP_OUTER && pf4.faces[f_i].loops[p_i].depth%2 == 1 &&
                                                    (is_identical_loop(pf4, pf4.faces[f_i].loops[p_i],pf5, pf5.faces[f_j].loops[p_j]) ||
                                                 is_contained_loop(pf4, pf4.faces[f_i].loops[p_i],pf5,f_j,p_j)) &&
                                                    pf4.faces[f_i].m_normal.dotProduct(pf5.faces[f_j].m_normal) > 0)
                                            {
                                                    std::cout<<"Using improvement Z3\n";
                                                    std::cout<<"2 solid outer loop "<<f_i<<" "<<p_i<<" vs "<<f_j<<" "<<p_j<<"\n";
                                                    pf4.faces[f_i].loops[p_i].topo_group = LOOP_SOLID;
                                                    num++;
                                                    goto LoopHasBeenCulled;
                                            }
                                        }
                                    }
                                    if(pass_no == CLIP_SECOND_PASS)
                                    {
                                        for(int p_j=0; p_j < ghost_loops.faces[f_j].loops.size(); p_j++)
                                        {
                                          if( pf4.faces[f_i].loops[p_i].type==LOOP_INNER &&
                                            (is_identical_loop(pf4, pf4.faces[f_i].loops[p_i],ghost_loops, ghost_loops.faces[f_j].loops[p_j]) ||
                                             is_contained_loop(pf4, pf4.faces[f_i].loops[p_i],ghost_loops,f_j,p_j)))
                                            {
                                                if(pf4.faces[f_i].m_normal.dotProduct(pf5.faces[f_j].m_normal) > 0)
                                                {
                                                    std::cout<<"2 G solid loop "<<f_i<<" "<<p_i<<" vs "<<f_j<<" "<<p_j<<"\n";
                                                    pf4.faces[f_i].loops[p_i].topo_group = LOOP_SOLID;
                                                    num++;
                                                    goto LoopHasBeenCulled;
                                                }
                                                else
                                                {
                                                    std::cout<<"2 G hollow loop "<<f_i<<" "<<p_i<<" vs "<<f_j<<" "<<p_j<<"\n";
                                                    pf4.faces[f_i].loops[p_i].topo_group = LOOP_HOLLOW;
                                                    num++;
                                                    goto LoopHasBeenCulled;
                                                }
                                            }
                                        }
                                    }
                                }
                        }

                    }
                    LoopHasBeenCulled:
                    int z=0;
                }
            }
        }
        std::cout<<" "<<num<<" kissing loops\n";
    }

    //return pf4;

    //return object
    //if(false)
    {
        for(int f_i=0; f_i < pf4.faces.size(); f_i++)
        {
            poly_face f;
            f.m_normal = pf4.faces[f_i].m_normal;
            f.bFlippedNormal = pf4.faces[f_i].bFlippedNormal;
            //std::cout<<f_i<<"\n";

            for(int p_i=0; p_i<pf4.faces[f_i].loops.size(); p_i++)
            {
                if(pf4.faces[f_i].loops[p_i].vertices.size() > 0 && pf4.faces[f_i].loops[p_i].type == LOOP_INNER)
                {
                    poly_loop new_loop;
                    for(int v_i : pf4.faces[f_i].loops[p_i].vertices)
                    {
                        int v = ret.get_point_or_add(pf4.vertices[v_i].V);
                        new_loop.vertices.push_back(v);
                        f.addVertex(v);
                    }

                    if(pf4.faces[f_i].loops[p_i].topo_group == LOOP_UNDEF)
                    {
                        std::vector<int> tempv = pf4.faces[f_i].loops[p_i].vertices;
                        tempv.push_back(tempv[0]);

                        bool exclude=false;
                        bool include=false;


                        for(int i=0; i < tempv.size()-1; i++)
                        {
                            core::vector3df v0=pf4.vertices[tempv[i]].V;
                            core::vector3df v1=pf4.vertices[tempv[i+1]].V;

                            if(pf4.edges[pf4.get_edge_or_add(tempv[i],tempv[i+1],!rule)].topo_group == !rule )
                            {
                                exclude = true;
                            }
                            else if(pf4.edges[pf4.get_edge_or_add(tempv[i],tempv[i+1],!rule)].topo_group == rule )
                            {
                                include = true;
                            }
                           // else if(pf4.edges[pf4.get_edge_or_add(tempv[i],tempv[i+1],!rule)].topo_group == rule )
                            int new_e = ret.get_edge_or_add(ret.get_point_or_add(v0),ret.get_point_or_add(v1),0);
                            f.addEdge(new_e);
                        }

                        if(exclude && !include )
                        {
                        //    std::cout<<"hollow\n";
                            pf4.faces[f_i].loops[p_i].topo_group = LOOP_HOLLOW;
                        }
                        else if(include && !exclude )
                        {
                        //    std::cout<<"solid\n";
                            pf4.faces[f_i].loops[p_i].topo_group = LOOP_SOLID;
                        }
                        else if((!exclude && !include ) || (exclude && include ))
                        {
                            std::cout<<"odd loop: "<<f_i<<", "<<p_i<<" ";
                            core::vector3df v0=pf4.vertices[tempv[0]].V;
                            core::vector3df v1;
                            int e_i = pf4.find_edge(tempv[0],tempv[1]);

                            if(e_i != -1 && pf4.get_facial_point3(f_i,e_i,p_i,v1,nograph))
                            {
                                //graph.points.push_back(v1);
                                int res = pf2.classify_point(v0,v1);
                                if (res == rule)//  || res == TOP_UNDEF)
                                {
                                    pf4.faces[f_i].loops[p_i].topo_group = LOOP_SOLID;
                                    std::cout<<" solid\n";
                                    exclude = false;
                                }
                                else
                                {
                                    pf4.faces[f_i].loops[p_i].topo_group = LOOP_HOLLOW;
                                    std::cout<<" hollow\n";
                                    exclude = false;
                                }
                            }
                            else  pf4.faces[f_i].loops[p_i].topo_group = LOOP_HOLLOW;
                        }
                    }

                    new_loop.type = pf4.faces[f_i].loops[p_i].type;
                    new_loop.depth = pf4.faces[f_i].loops[p_i].depth;
                    new_loop.topo_group = pf4.faces[f_i].loops[p_i].topo_group;
                    ret.calc_loop_bbox(f,new_loop);

                    if(new_loop.topo_group != LOOP_UNDEF)
                    {/*
                        if(pass_no == CLIP_SECOND_PASS && new_loop.topo_group == LOOP_SOLID)
                        {
                            bool b=false;
                            for(int p_j=0;p_j<pf4.faces[f_i].loops.size();p_j++)
                                if(pf4.faces[f_i].loops[p_j].type==LOOP_OUTER  && pf4.faces[f_i].loops[p_j].topo_group==LOOP_SOLID &&
                                   is_identical_loop(pf4,pf4.faces[f_i].loops[p_j],ret,new_loop))
                                   {
                                        b=true;
                                   }
                            if(!b)
                                f.loops.push_back(new_loop);
                        }*/
                        if(pass_no == CLIP_SECOND_PASS && new_loop.topo_group == LOOP_SOLID)
                        {
                            core::vector3df v1;
                            int e_i = pf4.find_edge(pf4.faces[f_i].loops[p_i].vertices[0],pf4.faces[f_i].loops[p_i].vertices[1]);
                            if(pf4.get_facial_point3(f_i,e_i,p_i,v1,nograph))
                            {
                                //graph.points.push_back(v1);
                                if(pf.is_point_on_face(f_i,v1))
                                {
                                    //std::cout<<"yes\n";
                                    f.loops.push_back(new_loop);
                                }
                                //else std::cout<<"no\n";
                            }
                        }
                        else if(pass_no == CLIP_FIRST_PASS)
                        {
                            f.loops.push_back(new_loop);
                            //if(new_loop.topo_group == LOOP_SOLID)
                            //    std::cout<<"added solid loop\n";
                            if(new_loop.topo_group == LOOP_HOLLOW)
                                std::cout<<"added hollow loop\n";
                        }
                    }
                    //else std::cout<<"no loop!\n";
                }
                else if(pf4.faces[f_i].loops[p_i].vertices.size() > 0 && pf4.faces[f_i].loops[p_i].type == LOOP_OUTER
                        && pf4.faces[f_i].loops[p_i].depth%2 == 1 && pf4.faces[f_i].loops[p_i].topo_group != LOOP_SOLID)
                {
                    poly_loop new_loop;
                    for(int v_i : pf4.faces[f_i].loops[p_i].vertices)
                    {
                        int v = ret.get_point_or_add(pf4.vertices[v_i].V);
                        new_loop.vertices.push_back(v);
                        f.addVertex(v);
                    }

                    new_loop.type = pf4.faces[f_i].loops[p_i].type;
                    new_loop.depth = pf4.faces[f_i].loops[p_i].depth;
                    new_loop.topo_group = LOOP_HOLLOW;
                    new_loop.type = LOOP_INNER;
                    ret.calc_loop_bbox(f,new_loop);
                    //std::cout<<"hollow loop! "<<f_i<<","<<p_i<<"\n";\
                    //new_loop.reverse();
                    f.loops.push_back(new_loop);
                }
            }

            ret.calc_center(f);
            if(f.loops.size() > 0)
            {
                ret.faces.push_back(f);
            }
        }

        //return ret;

        //ret.merge_loops();

        if(pass_no == CLIP_SECOND_PASS)
        {
            ret.reduce_edges_vertices2();
            ret.recalc_bbox_and_loops();
        }
        else
        {
            ret.reduce_edges_vertices2();

            if(ret.vertices.size()>0)
            {
                ret.bbox.reset(ret.vertices[0].V);
                for(poly_vert v : ret.vertices)
                    ret.bbox.addInternalPoint(v.V);
            }
        }
    }

    return ret;
}

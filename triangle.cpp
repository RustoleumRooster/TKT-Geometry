#include <irrlicht.h>
#include <iostream>
#include "csg_classes.h"
#include "tolerances.h"

using namespace irr;

bool vec_is_left_from(const core::vector3df& v0_, const core::vector3df& v1_)
{
    core::vector3df v0 = v0_;
    core::vector3df v1 = v1_;
    v0.normalize();
    v1.normalize();
    return (v0.crossProduct(v1).Y > 0.001);
}

bool vec_is_right_from(const core::vector3df& v0_, const core::vector3df& v1_)
{
    core::vector3df v0 = v0_;
    core::vector3df v1 = v1_;
    v0.normalize();
    v1.normalize();

    return (v0.crossProduct(v1).Y < -0.001);
}


bool line_intersects_line(const core::line2df& line_1, const core::line2df& line_2, core::vector2df& outvec)
{
    if (line_1.intersectWith(line_2, outvec))
        return true;
    else
    {
        core::vector2df r1 = line_1.getClosestPoint(line_2.start) - line_2.start;
        core::vector2df r2 = line_1.getClosestPoint(line_2.end) - line_2.end;

        if (r1.getLength() < 0.1)
        {
            outvec = line_2.start;
            return true;
        }
        else if (r2.getLength() < 0.1)
        {
            outvec = line_2.end;
            return true;
        }
    }
    return false;
}

std::vector<poly_edge> get_all_connections(polyfold pf, int p_i, int vert)
{
    std::vector<int> v_list;
    std::vector<poly_edge> ret;

    poly_face f = pf.faces[0];

    v_list.push_back(f.loops[p_i].vertices[f.loops[p_i].vertices.size()-1]);
    for(int v_i : f.loops[p_i].vertices)
    {
        v_list.push_back(v_i);
    }
    v_list.push_back(v_list[1]);

    int my_v=-1;

    for(int i=1;i<v_list.size()-1;i++)
    {
        if(v_list[i]==vert)
            my_v=i;
    }

    bool bReverse = pf.faces[0].loops[p_i].topo_group == LOOP_HOLLOW;

    if(my_v== -1)std::cout<<"triangle, get_all_connections... error, point not on loop!\n";

    for(int i=0; i<f.vertices.size(); i++)
    {
        if (i == vert || pf.find_edge(0, vert, i) != -1)
            continue;

        core::vector3df v0      = pf.vertices[v_list[my_v]].V;
        core::vector3df v1      = pf.vertices[i].V;
        core::vector3df v_left;
        core::vector3df v_right;

        if (bReverse)
        {
            v_right = pf.vertices[v_list[my_v + 1]].V;
            v_left = pf.vertices[v_list[my_v - 1]].V;
        }
        else
        {
            v_left = pf.vertices[v_list[my_v + 1]].V;
            v_right = pf.vertices[v_list[my_v - 1]].V;
        }

        core::vector3df r       = v1 - v0;
        core::vector3df left_r  = v_left - v0;
        core::vector3df right_r = v_right - v0;

        int v_0                 = v_list[my_v];
        int v_1                 = i;

        if(vec_is_left_from(- left_r, right_r))
            {
                if (vec_is_left_from(r,right_r) || vec_is_right_from(r,left_r))
                {

                    core::line2df line_1 = core::line2df(v0.X,v0.Z,v1.X,v1.Z);

                    bool b=false;

                    for(int e_i=0; e_i <pf.edges.size();e_i++)
                    {
                        core::line2df line_2 = core::line2df(pf.getVertex(e_i,0).V.X,pf.getVertex(e_i,0).V.Z,pf.getVertex(e_i,1).V.X,pf.getVertex(e_i,1).V.Z);
                       core::vector2df ipoint;
                       if(line_intersects_line(line_1,line_2,ipoint))
                          {
                            if(!is_same_point(line_1.start,ipoint) && !is_same_point(line_1.end,ipoint))
                                b=true;
                          }
                    }
                    if(!b)
                        {
                        ret.push_back(poly_edge(v_0,v_1));
                        }
                }
            }
        else
                if (vec_is_left_from(r,right_r) && vec_is_right_from(r,left_r))
                {

                    core::line2df line_1 = core::line2df(v0.X,v0.Z,v1.X,v1.Z);

                    bool b=false;

                    for(int e_i=0; e_i< pf.edges.size();e_i++)
                    {
                        core::line2df line_2 = core::line2df(pf.getVertex(e_i,0).V.X,pf.getVertex(e_i,0).V.Z,pf.getVertex(e_i,1).V.X,pf.getVertex(e_i,1).V.Z);
                       core::vector2df ipoint;
                       if(line_intersects_line(line_1,line_2,ipoint))
                          {
                            if(!is_same_point(line_1.start,ipoint) && !is_same_point(line_1.end,ipoint))
                                b=true;
                          }
                    }
                    if(!b)
                        {
                        ret.push_back(poly_edge(v_0,v_1));
                        }
                }
    }
    return ret;
}

std::vector<poly_edge> get_all_connections2(const polyfold& pf, int vert)
{
    std::vector<int> v_list;
    std::vector<poly_edge> ret;

    std::vector<int> loop_nos = pf.get_vert_loop_no(0,vert);

    if (loop_nos.size() == 1)
    {
        return get_all_connections(pf, loop_nos[0], vert);
    }
    else if(loop_nos.size() >1)
    {
        std::vector<poly_edge>* links = new std::vector<poly_edge>[loop_nos.size()];

        for (int i = 0; i < loop_nos.size(); i++)
        {
            links[i] = get_all_connections(pf, loop_nos[i], vert);
        }

        for(int i=0; i<links[0].size();i++)
        {
            bool b=true;
            for(int j=1;j<loop_nos.size();j++)
            {
                bool bb=false;
                for(int ii=0;ii<links[j].size();ii++)
                {
                    if  ((links[j][ii].v0 == links[0][i].v0 && links[j][ii].v1 == links[0][i].v1) ||
                        (links[j][ii].v1 == links[0][i].v1 && links[j][ii].v0 == links[0][i].v0) ||
                        (links[j][ii].v1 == links[0][i].v0 && links[j][ii].v0 == links[0][i].v1) ||
                        (links[j][ii].v0 == links[0][i].v1 && links[j][ii].v1 == links[0][i].v0))
                        bb=true;
                }
                if(bb==false)
                    b=false;
            }
            if(b)
                ret.push_back(links[0][i]);
        }

        delete[] links;
    }

    return ret;
}


std::vector<triangle> get_adjacent_triangles(const polyfold& pf, int e_i, LineHolder& graph)
{
    std::vector<int> set0 =  pf.get_edges_from_point(0,pf.edges[e_i].v0);
    std::vector<int> set1 =  pf.get_edges_from_point(0,pf.edges[e_i].v1);
    std::vector<triangle> ret;

    int v_0 = pf.edges[e_i].v0;
    int v_1 = pf.edges[e_i].v1;

    for(int e_ii : set0)
        for(int e_jj : set1)
    {
        if ((e_ii != e_i && e_jj != e_i) && (pf.edges[e_ii].topo_group != -1 && pf.edges[e_jj].topo_group != -1) &&
            (pf.edges[e_ii].v0 == pf.edges[e_jj].v0 ||
             pf.edges[e_ii].v0 == pf.edges[e_jj].v1 ||
             pf.edges[e_ii].v1 == pf.edges[e_jj].v0 ||
             pf.edges[e_ii].v1 == pf.edges[e_jj].v1 ))
        {
            triangle T;
            T.A = v_0;
            T.B = v_1;
            if(pf.edges[e_ii].v0 == pf.edges[e_jj].v0)
            {
                T.C = pf.edges[e_ii].v0;
                if(T.C != T.A && T.C != T.B)
                    ret.push_back(T);
            }
            else if(pf.edges[e_ii].v1 == pf.edges[e_jj].v1)
            {
                T.C = pf.edges[e_ii].v1;
                if(T.C != T.A && T.C != T.B)
                    ret.push_back(T);
            }
            else if(pf.edges[e_ii].v1 == pf.edges[e_jj].v0)
            {
                T.C = pf.edges[e_ii].v1;
                if(T.C != T.A && T.C != T.B)
                    ret.push_back(T);
            }
            else if(pf.edges[e_ii].v0 == pf.edges[e_jj].v1)
            {
                T.C = pf.edges[e_ii].v0;
                if(T.C != T.A && T.C != T.B)
                    ret.push_back(T);
            }
        }
    }

    if(ret.size() > 2)
    {
        f32 bestleft=0xFFFF;
        f32 bestright=0xFFFF;
        triangle left;
        triangle right;

        for(triangle T : ret)
        {
            core::line2df line_ab = core::line2df(pf.vertices[T.A].V.X,pf.vertices[T.A].V.Z,pf.vertices[T.B].V.X,pf.vertices[T.B].V.Z);
            core::vector2df mid = line_ab.getMiddle();

            core::vector3df v1 = pf.vertices[T.C].V - pf.vertices[T.A].V;
            core::vector3df v2 = pf.vertices[T.A].V - pf.vertices[T.B].V;

        if(vec_is_left_from(v1,v2))
            {
                core::vector2df r = mid - core::vector2df(pf.vertices[T.C].V.X,pf.vertices[T.C].V.Z);
                if(r.getLength() < bestleft)
                {
                    bestleft = r.getLength();
                    left = T;
                }
            }
        else
            {
                core::vector2df r = mid - core::vector2df(pf.vertices[T.C].V.X,pf.vertices[T.C].V.Z);
                if(r.getLength() < bestright)
                {
                    bestright = r.getLength();
                    right = T;
                }
            }
        }
            ret.clear();
            ret.push_back(left);
            ret.push_back(right);
    }

    return ret;
}

bool improve_triangle(polyfold& pf, int e_i, LineHolder& graph)
{
    if(pf.edges[e_i].topo_group == -1) return false;

    std::vector<triangle> adj;
    adj = get_adjacent_triangles(pf,e_i,graph);

    if(adj.size() == 2)
    {
        core::vector3df v1 = pf.vertices[adj[0].C].V - pf.vertices[adj[0].A].V;
        core::vector3df v2 = pf.vertices[adj[0].A].V - pf.vertices[adj[1].C].V;
        core::vector3df v3 = pf.vertices[adj[1].C].V - pf.vertices[adj[0].B].V;
        core::vector3df v4 = pf.vertices[adj[0].B].V - pf.vertices[adj[0].C].V;

        if ((vec_is_left_from(v1,v2)&&vec_is_left_from(v2,v3)&&vec_is_left_from(v3,v4)&&vec_is_left_from(v4,v1)) ||
            (vec_is_right_from(v1,v2)&&vec_is_right_from(v2,v3)&&vec_is_right_from(v3,v4)&&vec_is_right_from(v4,v1)))
            {
                triangle T = adj[0];

                core::line2df line_ab = core::line2df(pf.vertices[T.A].V.X,pf.vertices[T.A].V.Z,pf.vertices[T.B].V.X,pf.vertices[T.B].V.Z);
                core::line2df line_bc = core::line2df(pf.vertices[T.B].V.X,pf.vertices[T.B].V.Z,pf.vertices[T.C].V.X,pf.vertices[T.C].V.Z);
                core::line2df line_ac = core::line2df(pf.vertices[T.A].V.X,pf.vertices[T.A].V.Z,pf.vertices[T.C].V.X,pf.vertices[T.C].V.Z);

                f32 ab = line_ab.getAngleWith(line_bc);
                f32 bc = line_ab.getAngleWith(line_ac);
                f32 mina = std::min(ab,bc);

                T = adj[1];

                line_ab = core::line2df(pf.vertices[T.A].V.X,pf.vertices[T.A].V.Z,pf.vertices[T.B].V.X,pf.vertices[T.B].V.Z);
                line_bc = core::line2df(pf.vertices[T.B].V.X,pf.vertices[T.B].V.Z,pf.vertices[T.C].V.X,pf.vertices[T.C].V.Z);
                line_ac = core::line2df(pf.vertices[T.A].V.X,pf.vertices[T.A].V.Z,pf.vertices[T.C].V.X,pf.vertices[T.C].V.Z);

                ab = line_ab.getAngleWith(line_bc);
                bc = line_ab.getAngleWith(line_ac);
                f32 mina2 = std::min(mina,std::min(ab,bc));

                T.A = adj[1].C;
                T.B = adj[0].C;
                T.C = adj[0].A;

                line_ab = core::line2df(pf.vertices[T.A].V.X,pf.vertices[T.A].V.Z,pf.vertices[T.B].V.X,pf.vertices[T.B].V.Z);
                line_bc = core::line2df(pf.vertices[T.B].V.X,pf.vertices[T.B].V.Z,pf.vertices[T.C].V.X,pf.vertices[T.C].V.Z);
                line_ac = core::line2df(pf.vertices[T.A].V.X,pf.vertices[T.A].V.Z,pf.vertices[T.C].V.X,pf.vertices[T.C].V.Z);

                ab = line_ab.getAngleWith(line_bc);
                bc = line_ab.getAngleWith(line_ac);
                f32 mina3 = std::min(ab,bc);

                T.A = adj[1].C;
                T.B = adj[0].C;
                T.C = adj[0].B;

                line_ab = core::line2df(pf.vertices[T.A].V.X,pf.vertices[T.A].V.Z,pf.vertices[T.B].V.X,pf.vertices[T.B].V.Z);
                line_bc = core::line2df(pf.vertices[T.B].V.X,pf.vertices[T.B].V.Z,pf.vertices[T.C].V.X,pf.vertices[T.C].V.Z);
                line_ac = core::line2df(pf.vertices[T.A].V.X,pf.vertices[T.A].V.Z,pf.vertices[T.C].V.X,pf.vertices[T.C].V.Z);

                ab = line_ab.getAngleWith(line_bc);
                bc = line_ab.getAngleWith(line_ac);
                f32 mina4 = std::min(mina3,std::min(ab,bc));

                if(mina2 < mina4)
                {
                    pf.edges[e_i].topo_group=-1;
                    int new_e = pf.get_edge_or_add(adj[0].C,adj[1].C,3);
                    pf.edges[new_e].topo_group = 3;
                    pf.faces[0].addEdge(new_e);

                    return true;
                }
            }
    }
    return false;
}

triangle_holder get_triangles_for_loop(polyfold& pf, LineHolder& graph)
{
    triangle_holder t_holder;

    pf.edges.clear();
    pf.faces[0].edges.clear();
    std::vector<int> fixed_edges;

    for (poly_vert v : pf.vertices)
    {
        t_holder.vertices.push_back(v.V);
    }

    for (int p_i = 0; p_i < pf.faces[0].loops.size(); p_i++)
    {
        std::vector<int> v_list;

        v_list.push_back(pf.faces[0].loops[p_i].vertices[pf.faces[0].loops[p_i].vertices.size() - 1]);
        for (int v_i : pf.faces[0].loops[p_i].vertices)
        {
            v_list.push_back(v_i);
        }
        v_list.push_back(v_list[1]);

        if (pf.faces[0].loops[p_i].topo_group == LOOP_SOLID)
            for (int i = 1; i < v_list.size() - 1; i++)
            {
                int new_e = pf.get_edge_or_add(v_list[i], v_list[i + 1], 0);
                pf.faces[0].addEdge(new_e);
                graph.lines.push_back(core::line3df(pf.getVertex(new_e, 0).V, pf.getVertex(new_e, 1).V));
            }
        else
        {
            if (pf.faces[0].loops[p_i].vertices.size() == 3)
                for (int i = 1; i < v_list.size() - 1; i++)
                {
                    int new_e = pf.get_edge_or_add(v_list[i], v_list[i + 1], 3);
                    pf.faces[0].addEdge(new_e);
                    graph.lines.push_back(core::line3df(pf.getVertex(new_e, 0).V, pf.getVertex(new_e, 1).V));
                }
            else
                for (int i = 1; i < v_list.size() - 1; i++)
                {
                    int new_e = pf.get_edge_or_add(v_list[i], v_list[i + 1], 0);
                    pf.faces[0].addEdge(new_e);
                    graph.lines.push_back(core::line3df(pf.getVertex(new_e, 0).V, pf.getVertex(new_e, 1).V));
                }
        }
    }

    //special handling for 3 sided holes
    for (int p_i = 0; p_i < pf.faces[0].loops.size(); p_i++)
    {
        if (pf.faces[0].loops[p_i].vertices.size() == 3 && pf.faces[0].loops[p_i].topo_group == LOOP_HOLLOW)
        {
            int a = pf.faces[0].loops[p_i].vertices[0];
            int b = pf.faces[0].loops[p_i].vertices[1];
            int c = pf.faces[0].loops[p_i].vertices[2];

            int new_e0 = pf.find_edge(a, b);
            int new_e1 = pf.find_edge(b, c);
            int new_e2 = pf.find_edge(c, a);

            if (new_e0 != -1)
                fixed_edges.push_back(new_e0);
            if (new_e1 != -1)
                fixed_edges.push_back(new_e1);
            if (new_e2 != -1)
                fixed_edges.push_back(new_e2);
        }
    }

    bool bKeepGoing = true;

    bool* tracker = new bool[pf.vertices.size()];
    for (int i = 0; i < pf.vertices.size(); i++)
        tracker[i] = true;

    while(bKeepGoing)
    { 
        bKeepGoing = false;
        for (int v_i = 0; v_i < pf.vertices.size(); v_i++)
        {
            if (tracker[v_i] == false)
                continue;

            std::vector<poly_edge> links = get_all_connections2(pf, v_i);

            if (links.size() <= 1)
                tracker[v_i] = false;

            if (links.size() > 0)
            {
                bKeepGoing = true;

                f32 len;
                int best = 0;
                
                if (links.size() > 1)
                {
                    const poly_edge& edge0 = links[0];
                    len = vector3df(pf.vertices[edge0.v0].V - pf.vertices[edge0.v1].V).getLength();

                    for (int i = 1; i < links.size(); i++)
                    {
                        const poly_edge& edge = links[i];
                        f32 new_len = vector3df(pf.vertices[edge.v0].V - pf.vertices[edge.v1].V).getLength();
                        if (new_len < len)
                        {
                            len = new_len;
                            best = i;
                        }
                    }
                }

                const poly_edge& best_edge = links[best];

                if (pf.find_edge(best_edge.v0, best_edge.v1) == -1)
                {
                    int new_e = pf.get_edge_or_add(best_edge.v0, best_edge.v1, 3);
                    pf.faces[0].addEdge(new_e);
                }
            }
        }
    }

    delete[] tracker;

    //===============
    LineHolder nograph;
    int num=0;

    try_again:
    for(int e_i : pf.faces[0].edges)
    {
        if(pf.edges[e_i].topo_group != -1)
        {

            bool b = false;

            for(int e_j : fixed_edges)
                if(e_j == e_i)
                    b=true;

            if(!b && improve_triangle(pf, e_i, graph))
            {
                num++;
                goto try_again;
            }
        }
    }
    
    num=0;

    for(int g=0;g<2;g++)
    {
        bKeepGoing = false;
        for(int e_i : pf.faces[0].edges)
        {
            if(pf.edges[e_i].topo_group==0)
            {
                std::vector<int> connex0 =  pf.get_edges_from_point(0,pf.edges[e_i].v0);
                std::vector<int> connex1 =  pf.get_edges_from_point(0,pf.edges[e_i].v1);

                core::vector3df r = pf.getVertex(e_i,0).V+(pf.getVertex(e_i,1).V-pf.getVertex(e_i,0).V)*0.5;
                f32 d = 99999;
                f32 dd;
                int v_k=-1;
                int e_J;
                int e_K;

                for( int e_j: connex0)
                    for( int e_k : connex1)
                    {

                        int v_i = pf.get_opposite_end(e_j,pf.edges[e_i].v0);
                        int v_j = pf.get_opposite_end(e_k,pf.edges[e_i].v1);
                        if(v_i == v_j && (dd = pf.vertices[v_i].V.getDistanceFrom(r))<d)
                        {
                            if(pf.edges[e_j].topo_group==0 && pf.edges[e_k].topo_group==0)
                            {
                            // TODO - bug fix (see below)
                            //    std::cout<<"careful!\n";
                            }

                            e_J = e_j;
                            e_K = e_k;
                            v_k = v_i;
                            d = dd;
                        }
                    }

                bKeepGoing = true;

                if(v_k != -1)
                {
                    pf.edges[e_i].topo_group=-1;

                    if(pf.edges[e_J].topo_group==0)
                        pf.edges[e_J].topo_group=-1;
                    else
                        pf.edges[e_J].topo_group=0;

                    if(pf.edges[e_K].topo_group==0)
                        pf.edges[e_K].topo_group=-1;
                    else
                        pf.edges[e_K].topo_group=0;


                    int v_a = t_holder.get_point_or_add(pf.getVertex(e_i,0).V);
                    int v_b = t_holder.get_point_or_add(pf.getVertex(e_i,1).V);
                    int v_c = t_holder.get_point_or_add(pf.vertices[v_k].V);


                    //TODO- fix bug where a triangle "hole" touching the outside, with sides topo group = 0 causes incorrect triangling
                    if(num==0)
                    {
                        //graph.points.push_back(pf.vertices[v_i].V);
                       // graph.lines.push_back(core::line3df(pf.vertices[v_a].V,pf.vertices[v_b].V));
                       // graph.lines.push_back(core::line3df(pf.vertices[v_b].V,pf.vertices[v_c].V));
                       // graph.lines.push_back(core::line3df(pf.vertices[v_c].V,pf.vertices[v_a].V));
                    }
                    num++;

                    triangle T;
                    T.A = v_a;
                    T.B = v_b;
                    T.C = v_c;

                    t_holder.triangles.push_back(T);

                    if(num > 360)
                    {
                        std::cout<<"\n*trianglize high count warning... \n";
                    }
                }
            }
        }

    }

    return t_holder;
}

void polyfold::trianglize(int face_i, triangle_holder& t_holder, scene::SMeshBuffer* buffer, LineHolder &graph, LineHolder &graph2)
{
    core::matrix4 R = this->faces[face_i].get2Dmat();

    polyfold pf;

    std::vector<triangle> ret;
    std::vector<triangle> no_triangle;
    std::vector<int> fixed_edges;

    {
        poly_face f;
        for(int v_i:this->faces[face_i].vertices)
        {
            core::vector3df r = this->vertices[v_i].V;
            R.rotateVect(r);
            int new_v = pf.get_point_or_add(r);
            f.addVertex(new_v);
        }

        pf.faces.push_back(f);
    }

    for(poly_vert v: pf.vertices)
    {
        t_holder.vertices.push_back(v.V);
    }

    //special case: open loops with just 3 vertices need special treatment
    for(int p_i =0; p_i<this->faces[face_i].loops.size(); p_i++)
    {
        if(this->faces[face_i].loops[p_i].vertices.size()==3 && this->is_closed_loop(face_i,p_i)==false)
        {
            int a = this->faces[face_i].loops[p_i].vertices[0];
            int b = this->faces[face_i].loops[p_i].vertices[1];
            int c = this->faces[face_i].loops[p_i].vertices[2];

            core::vector3df v0 = this->vertices[a].V;
            core::vector3df v1 = this->vertices[b].V;
            core::vector3df v2 = this->vertices[c].V;

            R.rotateVect(v0); R.rotateVect(v1); R.rotateVect(v2);

            int new_a = t_holder.get_point_or_add(v0);
            int new_b = t_holder.get_point_or_add(v1);
            int new_c = t_holder.get_point_or_add(v2);

            if(new_a != -1 && new_b != -1 && new_c != -1)
            {
                triangle T;
                T.A = new_a;
                T.B = new_b;
                T.C = new_c;

                no_triangle.push_back(T);
            }
        }
    }

    for(int p_i =0; p_i<this->faces[face_i].loops.size(); p_i++)
    {
        if(this->is_closed_loop(face_i,p_i))
        {
            pf.faces[0].loops.clear();

            poly_loop loop;
            for(int v_i : this->faces[face_i].loops[p_i].vertices)
            {
                core::vector3df r = this->vertices[v_i].V;
                R.rotateVect(r);
                int new_v = pf.get_point_or_add(r);
                loop.vertices.push_back(new_v);
            }
            loop.topo_group = LOOP_SOLID;
            pf.faces[0].loops.push_back(loop);

            for(int p_j =0; p_j<this->faces[face_i].loops.size(); p_j++)
            {
                if(p_j != p_i && this->is_closed_loop(face_i,p_j)== false && this->is_inner_loop(face_i,p_j,p_i))
                {
                    poly_loop loop;
                    for(int v_i : this->faces[face_i].loops[p_j].vertices)
                    {
                        core::vector3df r = this->vertices[v_i].V;
                        R.rotateVect(r);
                        int new_v = pf.get_point_or_add(r);
                        loop.vertices.push_back(new_v);
                    }
                    loop.topo_group = LOOP_HOLLOW;
                    pf.faces[0].loops.push_back(loop);
                }
            }

            triangle_holder t_h = get_triangles_for_loop(pf,graph);

            for(triangle T: t_h.triangles)
            {
                bool b = false;
                for(triangle Z : no_triangle)
                {
                    if ((Z.A == T.A || Z.A == T.B || Z.A == T.C) &&
                        (Z.B == T.A || Z.B == T.B || Z.B == T.C) &&
                        (Z.C == T.A || Z.C == T.B || Z.C == T.C))
                        b = true;
                }

                if(!b)
                    t_holder.triangles.push_back(T);
            }
        }
    }

   core::matrix4 R_inv;
   R_inv = R;
   R_inv.makeInverse();

   for(core::vector3df& v : t_holder.vertices)
   {
        R_inv.rotateVect(v);
        int v_i = this->find_point(v);
        if( v_i != -1)
            v = this->vertices[v_i].V;
   }

   for(triangle &T : t_holder.triangles)
   {
        core::vector3df v1 = t_holder.vertices[T.B] - t_holder.vertices[T.A];
        core::vector3df v2 = t_holder.vertices[T.C] - t_holder.vertices[T.B];
        f32 dp = this->faces[face_i].m_normal.dotProduct(v1.crossProduct(v2));
        if(dp < 0)
        {
             int temp= T.A;
             T.A = T.B;
             T.B = temp;
        }
   }

   if(buffer)
   {
        buffer->Vertices.clear();
        buffer->Indices.clear();

        buffer->Vertices.reallocate(t_holder.vertices.size());
        buffer->Indices.reallocate(3 * t_holder.triangles.size());

        video::S3DVertex vtx;
        vtx.Color.set(255,255,255,255);
        vtx.Normal = this->faces[face_i].m_normal;

        for(const core::vector3df &v : t_holder.vertices)
        {
            vtx.Pos = v;

            core::vector3df rot_v;
            R.rotateVect(rot_v,v);
            vtx.TCoords.set(rot_v.X/100, rot_v.Z/100);

            buffer->Vertices.push_back(vtx);
        }
        for(triangle T : t_holder.triangles)
        {
            buffer->Indices.push_back(T.A);
            buffer->Indices.push_back(T.B);
            buffer->Indices.push_back(T.C);
        }
        buffer->recalculateBoundingBox();
   }

}

int triangle_holder::get_point_or_add(core::vector3df point)
{
    for( int i=0; i<this->vertices.size(); i++)
    {
        core::vector3df vert=this->vertices[i];
        if(abs(vert.X-point.X) < 0.01 &&
           abs(vert.Y-point.Y) < 0.01 &&
           abs(vert.Z-point.Z) < 0.01)
           {
            return i;
           }
    }
    this->vertices.push_back(point);
    return this->vertices.size()-1;
}




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
               //graph.points.push_back(pf5.vertices[v_i].V);
            if(new_v != -1)
                for(int e_i=0; e_i<pf4.edges.size(); e_i++)
                    {
                        if(pf4.point_is_on_edge(e_i,pf5.vertices[v_i].V))
                        {
                            pf4.bisect_edge(e_i,new_v,2,2);
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

    return;

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
           // if(line_intersects_plane(f2_plane,v0,v1,ipoint))
               {
            //    if(face_j==3)
            //    graph.points.push_back(ipoint);
               }

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


void polyfold::clear_ghost_loops()
{
    for(poly_face &f : this->faces)
        for(poly_loop &p : f.loops)
            if(p.topo_group == LOOP_HOLLOW && p.depth%2 ==0)
                p.vertices.clear();
}


bool is_contained_loop(polyfold &pf, poly_loop &loop_a , polyfold &pf2, int f_j, int p_j)
{
    std::vector<int> tempv = loop_a.vertices;
    tempv.push_back(tempv[0]);

    for(int i=0; i < tempv.size()-1; i++)
        {
            bool b = false;
            core::vector3df v0 = pf.vertices[tempv[i]].V;
            core::vector3df v1 = pf.vertices[tempv[i+1]].V;
            core::vector3df V = v0+(v1-v0)*0.5;

            if(pf2.is_point_in_loop(f_j,p_j,V))
                b= true;

            if(!b)
            {
                return false;
            }
        }
    return true;
}


void polyfold::classify_edges(LineHolder& graph)
{
    for(poly_edge& edge : this->edges)
    {
        edge.conv = TOP_UNDEF;
    }

    for(int f_i=0;f_i<this->faces.size();f_i++)
    {
       // std::cout<<f_i<<"\n";
        for(int p_i=0; p_i < this->faces[f_i].loops.size(); p_i++)
        {
            std::vector<int> edges_i = this->get_all_loop_edges(f_i,p_i);
            //for(int e_i: edges_i)
            //     std::cout<<e_i<<" ";
            //std::cout<<"\n";
            for(int f_j=f_i+1;f_j<this->faces.size();f_j++)
            {
                for(int p_j=0; p_j < this->faces[f_j].loops.size(); p_j++)
                {
                    std::vector<int> edges_j = this->get_all_loop_edges(f_j,p_j);
                    std::vector<int> common_edges;
                    for(int e_i: edges_i)
                        for(int e_j: edges_j)
                    {
                        if(e_i==e_j)
                        {
                            common_edges.push_back(e_i);
                           // std::cout<<e_i<<" ";
                        }
                    }
                    for(int e_i : common_edges)
                    {
                        //if(!(f_i == 3 && e_i == 21)) continue;

                        core::vector3df cp = this->faces[f_i].m_normal.crossProduct(this->faces[f_j].m_normal);
                        core::matrix4 R;
                        cp.normalize();
                        R.buildRotateFromTo(cp,core::vector3df(0,1,0));

                        core::vector3df m1;
                        core::vector3df m2;
                        core::vector3df r1;
                        core::vector3df r2;

                        LineHolder nograph;

                        R.rotateVect(m1, this->faces[f_i].m_normal);
                        R.rotateVect(m2, this->faces[f_j].m_normal);
                        //R.rotateVect(r1, this->faces[sides[0]].m_center);
                        //R.rotateVect(r2, this->faces[sides[1]].m_center);

                        bool bOk = this->get_facial_point(f_i,e_i,p_i,r1,graph);
                        if(!bOk) continue;

                        bOk = this->get_facial_point(f_j,e_i,p_j,r2,graph);
                        if(!bOk) continue;

                        R.rotateVect(r1);
                        R.rotateVect(r2);
                        m1.normalize();
                        m2.normalize();

                        //m1*=25;
                        //m2*=25;

                        //std::cout<<r1.X<<","<<r1.Z<<"\n";
                        //std::cout<<r2.X<<","<<r2.Z<<"\n";

                        //graph.points.push_back(core::vector3df(r1.X,0,r1.Z));
                        //graph.points.push_back(core::vector3df(r2.X,0,r2.Z));

                        core::line2df line1 = core::line2df(r1.X,r1.Z,r1.X+m1.X,r1.Z+m1.Z);
                        core::line2df line2 = core::line2df(r2.X,r2.Z,r2.X+m2.X,r2.Z+m2.Z);

                        core::vector2df ipoint;

                        if(line1.intersectWith(line2,ipoint,false))
                        {
                            if(   core::vector2df(m1.X,m1.Z).dotProduct(ipoint-core::vector2df(r1.X,r1.Z)) < 0
                               && core::vector2df(m2.X,m2.Z).dotProduct(ipoint-core::vector2df(r2.X,r2.Z)) < 0)
                                this->edges[e_i].conv = TOP_CONVEX;
                            else
                                this->edges[e_i].conv = TOP_CONCAVE;
                        }
                    }
                }
            }
        }
    }
}


/*
bool polyfold::get_facial_point(int face_i, int edge_i, int loop_i, core::vector3df & ret, LineHolder &graph)
{
    core::matrix4 R = this->faces[face_i].get2Dmat();

    polyfold pf;
    poly_face f;

    std::cout<<"face: "<<face_i<<" "<<"edge: "<<edge_i<<"\n";

   for(int v_i : this->faces[face_i].vertices)
    {
        core::vector3df r = this->vertices[v_i].V;
        R.rotateVect(r);
        int new_v = pf.get_point_or_add(r);
        f.addVertex(new_v);
        //graph.points.push_back(r);
    }

    poly_loop loop;
    for(int v_i : this->faces[face_i].loops[loop_i].vertices)
    {
        core::vector3df r = this->vertices[v_i].V;
        R.rotateVect(r);
        //graph.points.push_back(r);
        int new_v = pf.get_point_or_add(r);
        f.addVertex(new_v);
        loop.vertices.push_back(new_v);
    }

    if(loop.vertices.size() >0)
        f.loops.push_back(loop);
    else return false;



    std::vector<int> edges_i = this->get_all_loop_edges(face_i,loop_i);

    for(int e_i : edges_i)
    {
        if(this->edges[e_i].topo_group == -1)
            continue;

        core::vector3df v0 = this->getVertex(e_i,0).V;
        core::vector3df v1 = this->getVertex(e_i,1).V;
        R.rotateVect(v0);
        R.rotateVect(v1);
        int v_0 = pf.find_point(v0);
        int v_1 = pf.find_point(v1);
        int new_e = pf.get_edge_or_add(v_0,v_1,0);
        f.addEdge(new_e);
        graph.lines.push_back(core::line3df(v0,v1));
    }

    //graph.lines.push_back(core::line3df(this->getVertex(edge_i,0).V,this->getVertex(edge_i,1).V));


    pf.faces.push_back(f);

    core::vector3df ev0 = this->getVertex(edge_i,0).V;
    core::vector3df ev1 = this->getVertex(edge_i,1).V;
    R.rotateVect(ev0);
    R.rotateVect(ev1);

    int my_edge = pf.find_edge(ev0,ev1);

    if(my_edge == -1)
    {
        std::cout<<"Edge not on face: Error\n";
        return false;
    }

    //std::cout<<loop_i<<", "<<f.loops.size()<<" ...\n";
    //std::cout<<"searching "<<pf.edges[my_edge].v0<<","<<pf.edges[my_edge].v1<<"\n";
    {

        std::vector<int> v_list;
        v_list.push_back(f.loops[0].vertices[f.loops[0].vertices.size()-1]);
        //std::cout<<"0\n";

        for(int v_i : f.loops[0].vertices)
        {
            v_list.push_back(v_i);
        }
        v_list.push_back(v_list[1]);

       // for(int i : v_list)
       //     std::cout<<i<<" ";
       // std::cout<<" ("<<pf.edges[my_edge].v0<<","<<pf.edges[my_edge].v1<<")\n";
       // std::cout<<"\n";


        std::vector<int> edge_ends;

       // for(int i=1; i< v_list.size()-1; i++)
       // {
       //     if(v_list[i] == pf.edges[my_edge].v0)
       //         edge_ends.push_back(i);
       //     else if(v_list[i] == pf.edges[my_edge].v1)
       //         edge_ends.push_back(i);

       //     //if(v_list[i] == pf.edges[my_edge].v1)
       // }

        int end_n = v_list.size()-2;

        if((pf.edges[my_edge].v0 == v_list[1] && pf.edges[my_edge].v1 == v_list[end_n]) ||
           (pf.edges[my_edge].v1 == v_list[1] && pf.edges[my_edge].v0 == v_list[end_n]))
           {
            //std::cout<<"YES!\n";
            edge_ends.push_back(1);
            edge_ends.push_back(end_n);
           }
        else
            for(int i=1; i< v_list.size()-1; i++)
            {
                if(v_list[i] == pf.edges[my_edge].v0 && v_list[i+1] == pf.edges[my_edge].v1)
                {
                    edge_ends.push_back(i);
                    edge_ends.push_back(i+1);
                }
                else if(v_list[i] == pf.edges[my_edge].v0 && v_list[i-1] == pf.edges[my_edge].v1)
                {
                    edge_ends.push_back(i-1);
                    edge_ends.push_back(i);
                }

            }

       // std::cout<<"--> "<<edge_ends[0]<<","<<edge_ends[1]<<"\n";

        if(edge_ends.size() != 2)
        {
            std::cout<<"*warning* edge not found in v_list\n";
            return false;
        }



        std::vector<int> res[2];

        res[0].push_back(v_list[edge_ends[0]-1]);
        res[1].push_back(v_list[edge_ends[1]+1]);

        res[0].push_back(v_list[edge_ends[0]+1]);
        res[1].push_back(v_list[edge_ends[1]-1]);

        for(int j=0; j<2; j++ )
        {
            int my_v = edge_ends[j];
            for(int i=0; i<f.vertices.size(); i++)
            {
                core::vector3df v0      = pf.vertices[v_list[my_v]].V;
                core::vector3df v1      = pf.vertices[i].V;
                core::vector3df v_left  = pf.vertices[v_list[my_v+1]].V;
                core::vector3df v_right = pf.vertices[v_list[my_v-1]].V;

                core::vector3df r = v1 - v0;
                core::vector3df left_r = v_left - v0;
                core::vector3df right_r = v_right - v0;

               // std::vector<int> edges0 = pf.get_edges_from_point(0,v_list[my_v]);
               // std::vector<int> edges1 = pf.get_edges_from_point(0,i);

               // std::cout<<j<<": \n";
               // for(int i : edges0)
               //     std::cout<<i<<" ";
               // std::cout<<"...."
               // for(int i : edges1)
               //     std::cout<<i<<" ";
               // std::cout<<"\n";

                if(vec_is_left_from(- left_r, right_r))
                    {
                        if(vec_is_left_from(r,right_r) || vec_is_right_from(r,left_r))
                        {

                            core::line2df line_1 = core::line2df(v0.X,v0.Z,v1.X,v1.Z);

                            bool b=false;

                            //graph.lines.push_back(core::line3df(v0,v1));

                            for(int e_i=0; e_i <pf.edges.size();e_i++)
                            {
                                core::vector2df ipoint;
                                core::line2df line_2 = core::line2df(pf.getVertex(e_i,0).V.X,pf.getVertex(e_i,0).V.Z,pf.getVertex(e_i,1).V.X,pf.getVertex(e_i,1).V.Z);
                                if(line_intersects_line(line_1,line_2,ipoint))
                                  {
                                    if(!is_same_point(line_1.start,ipoint) && !is_same_point(line_1.end,ipoint))
                                        b=true;
                                  }
                            }
                            if(!b)
                                {
                                graph.lines.push_back(core::line3df(v0,v1));
                                res[j].push_back(i);
                                }
                        }
                    }
                else
                        if (vec_is_left_from(r,right_r) && vec_is_right_from(r,left_r))
                        {
                            //graph.lines.push_back(core::line3df(v0,v1));

                            core::line2df line_1 = core::line2df(v0.X,v0.Z,v1.X,v1.Z);

                            bool b=false;

                            for(int e_i=0; e_i< pf.edges.size();e_i++)
                            {
                                core::vector2df ipoint;
                                core::line2df line_2 = core::line2df(pf.getVertex(e_i,0).V.X,pf.getVertex(e_i,0).V.Z,pf.getVertex(e_i,1).V.X,pf.getVertex(e_i,1).V.Z);

                                if(line_intersects_line(line_1,line_2,ipoint))
                                  {
                                    if(!is_same_point(line_1.start,ipoint) && !is_same_point(line_1.end,ipoint))
                                        b=true;
                                  }
                            }
                            if(!b)
                                {
                                res[j].push_back(i);
                                graph.lines.push_back(core::line3df(v0,v1));
                                }
                        }
            }
        }//for j

        core::matrix4 R_inv = R;
        R_inv.makeInverse();

        int cc=0;
        for(int v_i : res[0])
            for(int v_j : res[1])
            {
                if(v_i == v_j)
                {
                    cc++;
                }
            }

        for(int v_i : res[0])
            for(int v_j : res[1])
            {
                if(v_i == v_j)
                {
                core::vector3df v2 = pf.vertices[v_i].V;
                core::vector3df v0 = pf.getVertex(my_edge,0).V;
                core::vector3df v1 = pf.getVertex(my_edge,1).V;

                core::vector3df w0 = v2-v1;
                core::vector3df w1 = v2-v0;
                w0.normalize();
                w1.normalize();
                if(is_parallel_normal(w0,w1)==false)
                {
                    R_inv.rotateVect(v2);
                    R_inv.rotateVect(v0);
                    R_inv.rotateVect(v1);

                    graph.points.push_back(v1);
                    graph.points.push_back(v0);
                    graph.lines.push_back(core::line3df(v0,v2));
                    graph.lines.push_back(core::line3df(v1,v2));

                    //ret.X = (v0.X + v1.X + v2.X)/3.0;
                    //ret.Y = (v0.Y + v1.Y + v2.Y)/3.0;
                    //ret.Z = (v0.Z + v1.Z + v2.Z)/3.0;
                    core::vector3df v3 = core::vector3df(v0+(v1-v0)*0.5);
                    ret = v3 + (v2-v3)*0.05;

                    //std::cout<<ret.X<<","<<ret.Y<<","<<ret.Z<<"\n";
                    return true;
                }
                }
            }
    }
    std::cout<<"*ERROR could not find any point on face "<<face_i<<" for edge "<<edge_i<<", loop "<<loop_i<<" *\n";
    return false;
}
*/


/*
void polyfold::merge_faces()
{
    int n_faces_merged=0;
    try_again_merge_faces:
    for(int f_i = 0; f_i <this->faces.size()-1; f_i++)
        for(int f_j = f_i+1; f_j <this->faces.size(); f_j++)
    {
        if(this->faces[f_i].m_normal.dotProduct(this->faces[f_j].m_normal) > 0.999)
        {
            bool bMerge = false;
            for(int e_i: this->faces[f_i].edges)
                 for(int e_j: this->faces[f_j].edges)
            {
                if(e_i == e_j)
                {
                    this->edges[e_i].topo_group = -1;
                    bMerge=true;
                }
            }
            if(bMerge)
            {
                //std::cout<<"Merging faces "<<f_i<<" and "<<f_j<<"\n";
                n_faces_merged++;

                for(int e_j : this->faces[f_j].edges)
                    this->faces[f_i].addEdge(e_j);

                for(int v_j : this->faces[f_j].vertices)
                    this->faces[f_i].addVertex(v_j);

                //this->faces[f_i].loops.clear();
                //LineHolder nograph;
                //this->calc_loops4(f_i,nograph);

                //for(poly_loop &loop : this->faces[f_i].loops)
                //    this->set_loop_solid(this->faces[f_i],loop);
                //this->sort_loops(this->faces[f_i]);

                std::vector<poly_face> fvec;
                for(int f_k = 0; f_k<this->faces.size(); f_k++)
                {
                    if(f_k != f_j)
                        fvec.push_back(this->faces[f_k]);
                }
                this->faces = fvec;
                goto try_again_merge_faces;
            }
        }
    }
    if(n_faces_merged>0) std::cout<<" merged "<<n_faces_merged<<" faces\n";
   // std::cout<<"ok..\n";
   // this->reduce_edges_vertices2();
   // this->simplify_edges();
    this->recalc_bbox_and_loops();
    this->reduce_edges_vertices2();

}
*/


void polyfold::merge_loops()
{/*
    for(int f_i = 0; f_i <this->faces.size()-1; f_i++)
    {
        for(int p_i=0; p_i<this->faces[f_i].loops.size(); p_i++)
            for(int p_j=0; p_j<p_i; p_j++)
            {
                poly_loop new_loop;
                for(int v_i : this->faces[f_i].loops[p_i].vertices)
                {
                    int v = ret.get_point_or_add(pf.vertices[v_i].V);
                    new_loop.vertices.push_back(v);
                    f.addVertex(v);
                }

                std::vector<int> tempv = loop.vertices;
                tempv.push_back(tempv[0]);

                for(int i=0; i < tempv.size()-1; i++)
                   {



                   }
            }
    }*/
}


bool is_left_from(core::vector3df v0, core::vector3df v1, core::vector3df straight, core::vector3df N)
{
    core::vector3df cp0 = v0.crossProduct(straight);
    core::vector3df cp1 = v1.crossProduct(straight);

    if(cp0.dotProduct(N) > 0.0001 && cp1.dotProduct(N) < -0.0001)
        return true;

    core::vector3df cp = v0.crossProduct(v1);
    if(cp.dotProduct(N) > 0.0001)
        return true;
    return false;
}


bool polyfold::apply_topology_groups(int default_group)
{
    std::vector<int> c_edges;
    int counts=0;
    bool found_one;
    while(this->exists_topology_group(2))
    {
        found_one=false;
        for(int e_i = 0; e_i<this->edges.size(); e_i++)
        {
            poly_edge edge = this->edges[e_i];

            if(edge.topo_group==2)
            {
                int v0 = edge.v0;
                int v1 = edge.v1;
                bool safe = true;
                bool found = false;

                c_edges = this->get_all_edges_from_point(e_i,v0);
                for(int e_j : c_edges)
                {
                    if(this->edges[e_j].topo_group == 3)
                        safe=false;
                }

                if(safe)
                    for(int e_j : c_edges)
                    {
                        if(this->edges[e_j].topo_group == 0 || this->edges[e_j].topo_group == 1)
                        {
                            this->edges[e_i].topo_group = this->edges[e_j].topo_group;
                            found=true;
                            found_one=true;
                        }
                    }

                if(!found)
                {
                    safe=true;
                    c_edges = this->get_all_edges_from_point(e_i,v1);
                    for(int e_j : c_edges)
                    {
                        if(this->edges[e_j].topo_group == 3)
                            safe=false;
                    }

                    if(safe)
                        for(int e_j : c_edges)
                        {
                            if(this->edges[e_j].topo_group == 0 || this->edges[e_j].topo_group == 1)
                            {
                                this->edges[e_i].topo_group = this->edges[e_j].topo_group;
                                found_one=true;
                            }
                        }
                }
            }
        }
        if(!found_one)
        {
            std::cout<<" no intersections... using default value\n";
            return false;
        }
    }
    return true;
}


bool polyfold::apply_topology_groups2(polyfold& pf2, int default_group)
{
    std::vector<int> c_edges;
    int counts=0;
    bool found_one;
    while(this->exists_topology_group(2))
    {
        found_one=false;
        for(int e_i = 0; e_i<this->edges.size(); e_i++)
        {
            poly_edge edge = this->edges[e_i];

            if(edge.topo_group==2)
            {
                int v0 = edge.v0;
                int v1 = edge.v1;
                bool safe = true;
                bool found = false;

                c_edges = this->get_all_edges_from_point(e_i,v0);
                for(int e_j : c_edges)
                {
                    if(this->edges[e_j].topo_group == 3)
                        safe=false;
                }

                if(safe)
                    for(int e_j : c_edges)
                    {
                        if(this->edges[e_j].topo_group == 0 || this->edges[e_j].topo_group == 1)
                        {
                            this->edges[e_i].topo_group = this->edges[e_j].topo_group;
                            found=true;
                            found_one=true;
                        }
                    }

                if(!found)
                {
                    safe=true;
                    c_edges = this->get_all_edges_from_point(e_i,v1);
                    for(int e_j : c_edges)
                    {
                        if(this->edges[e_j].topo_group == 3)
                            safe=false;
                    }

                    if(safe)
                        for(int e_j : c_edges)
                        {
                            if(this->edges[e_j].topo_group == 0 || this->edges[e_j].topo_group == 1)
                            {
                                this->edges[e_i].topo_group = this->edges[e_j].topo_group;
                                found_one=true;
                            }
                        }
                }
            }
        }
        if(!found_one)
        {
            for(int e_i = 0; e_i<this->edges.size(); e_i++)
            {
                poly_edge edge = this->edges[e_i];

                if(edge.topo_group==2)
                {
                    core::vector3df v0 = this->getVertex(e_i,0).V;
                    core::vector3df v1 = this->getVertex(e_i,1).V;

                    LineHolder nograph;
                    int RES = pf2.classify_point(v0+(v1-v0)*0.5,3,nograph);
                    if(RES == TOP_FRONT)
                    {
                        this->edges[e_i].topo_group=0;
                        found_one=true;
                        std::cout<<"FRONT\n";
                    }
                    else if(RES == TOP_BEHIND)
                    {
                        this->edges[e_i].topo_group=1;
                        found_one=true;
                        std::cout<<"BEHIND\n";
                    }
                    else if(RES == TOP_UNDEF)
                    {
                        if(pf2.topology == TOP_CONCAVE)
                        {
                            this->edges[e_i].topo_group = TOP_BEHIND;
                            std::cout<<"BEHIND (default)\n";
                        }
                        else if(pf2.topology == TOP_CONVEX)
                        {
                            this->edges[e_i].topo_group = TOP_FRONT;
                            std::cout<<"FRONT (default)\n";
                        }
                        found_one=true;
                    }
                    //else std::cout<<"UNDEF\n";
                }
                if(found_one)
                    break;
            }
        if(!found_one)
            {
               // std::cout<<" no intersections... using default value\n";
                return false;
            }
        }
    }
    return true;
}


std::vector<triangle> get_alternate_single_triangles(polyfold pf, int v_i,LineHolder& graph)
{
    std::vector<int> connex = pf.get_edges_from_point(0,v_i);

    std::vector<int> loop_nos = pf.get_vert_loop_no(0,v_i);
    int p_i = loop_nos[0];

    std::vector<triangle> ret;

    if(connex.size() < 2)
        return ret;

    for(int i=0; i< connex.size()-1; i++)
        for(int j=i+1; j< connex.size(); j++)
    {
        int e_i = connex[i];
        int e_j = connex[j];

        int v_1 = pf.get_opposite_end(e_i,v_i);
        int v_2 = pf.get_opposite_end(e_j,v_i);

        loop_nos = pf.get_vert_loop_no(0,v_1);
        int p_i2 = loop_nos[0];
        std::vector<poly_edge> con_edges = get_all_connections(pf,p_i2,v_1);

        for(poly_edge edge : con_edges)
            if(pf.get_opposite_end(edge,v_1) == v_2)
            {
                triangle T;
                T.A = v_1;
                T.B = v_2;
                T.C = v_i;
                //std::cout<<T.A<<","<<T.B<<","<<T.C<<"\n";
                //graph.lines.push_back(core::line3df(pf.vertices[v_i].V,pf.vertices[v_1].V));
               // graph.lines.push_back(core::line3df(pf.vertices[v_1].V,pf.vertices[v_2].V));
               // graph.lines.push_back(core::line3df(pf.vertices[v_2].V,pf.vertices[v_i].V));
               if(pf.find_edge(T.A,T.B) == -1)
                    ret.push_back(T);
            }
    }
    return ret;
}


std::vector<triangle> get_all_single_triangles(polyfold pf, int v_i,LineHolder& graph)
{
    std::vector<int> loop_nos = pf.get_vert_loop_no(0,v_i);

    std::vector<poly_edge> connex;
    for(int p_i: loop_nos)
    {

        std::vector<poly_edge> loop_connex = get_all_connections(pf,p_i,v_i);
        for(poly_edge edge : loop_connex)
            connex.push_back(edge);
    }

    std::vector<triangle> ret;

    for(poly_edge edge : connex)
    {
        int v_0 = edge.v0;
        int v_1 = edge.v1;

        std::vector<int> con_edges0 = pf.get_edges_from_point(0,v_0);
        std::vector<int> con_edges1 = pf.get_edges_from_point(0,v_1);

        for(int v_ii : con_edges0)
            for(int v_jj : con_edges1)
            {
                triangle T;
                T.A = v_0;
                T.B = v_1;
                if(pf.find_edge(T.A,T.B) == -1)
                {
                    if(pf.edges[v_ii].v0 == pf.edges[v_jj].v0)
                    {
                        T.C = pf.edges[v_ii].v0;
                        if(T.C != T.A && T.C != T.B)
                            ret.push_back(T);
                    }
                    else if(pf.edges[v_ii].v1 == pf.edges[v_jj].v1)
                    {
                        T.C = pf.edges[v_ii].v1;
                        if(T.C != T.A && T.C != T.B)
                            ret.push_back(T);
                    }
                    else if(pf.edges[v_ii].v1 == pf.edges[v_jj].v0)
                    {
                        T.C = pf.edges[v_ii].v1;
                        if(T.C != T.A && T.C != T.B)
                            ret.push_back(T);
                    }
                    else if(pf.edges[v_ii].v0 == pf.edges[v_jj].v1)
                    {
                        //graph.lines.push_back(core::line3df(pf.vertices[v_0].V,pf.vertices[v_1].V));
                        //graph.lines.push_back(core::line3df(pf.getVertex(v_ii,0).V,pf.getVertex(v_ii,1).V));
                        //graph.lines.push_back(core::line3df(pf.getVertex(v_jj,0).V,pf.getVertex(v_jj,1).V));
                        T.C = pf.edges[v_ii].v0;
                        if(T.C != T.A && T.C != T.B)
                            ret.push_back(T);
                    }
                }
            }
    }
    return ret;
}


std::vector<triangle> get_all_double_triangles2(polyfold pf, int v_i, LineHolder& graph)
{
    std::vector<triangle> ret;

    //int p_i = pf.get_vert_loop_no(0,v_i);
    std::vector<int> loop_nos = pf.get_vert_loop_no(0,v_i);

    std::vector<poly_edge> connex0, connexL, connexR;
    for(int p_i: loop_nos)
    {
        connex0 = get_all_connections(pf,p_i,v_i);

        std::vector<int> left_right_vecs = getLeftRightVertices(pf,p_i,v_i);

        if(left_right_vecs.size() != 2)
            return ret;

        connexL = get_all_connections(pf,p_i,left_right_vecs[0]);

        connexR = get_all_connections(pf,p_i,left_right_vecs[1]);

        int v_i0= v_i;
        int v_j0= left_right_vecs[0];
        int v_k0= left_right_vecs[1];

        //graph.points.push_back(pf.vertices[v_i0].V);
        //graph.points.push_back(pf.vertices[v_j0].V);
        //graph.points.push_back(pf.vertices[v_k0].V);

        for(poly_edge edge_i : connex0)
        {
            int v_i1 = pf.get_opposite_end(edge_i, v_i0);

            core::vector3df v0 = pf.vertices[v_i0].V;
            core::vector3df v3 = pf.vertices[v_i1].V;
            //graph.lines.push_back(core::line3df(v0,v3));

            for(poly_edge edge_j : connexL)
            {
                int v_j1 = pf.get_opposite_end(edge_j, v_j0);
                if(v_j1 == v_i1)
                {
                    core::vector3df v0 = pf.vertices[v_i0].V;
                    core::vector3df v1 = pf.vertices[v_j0].V;
                    core::vector3df v2 = pf.vertices[v_j1].V;

                    core::vector3df r0 = v1-v0;
                    core::vector3df r1 = v2-v1;

                    if(vec_is_right_from(r1,r0))
                    {
                        triangle T;
                        T.A = v_i0;
                        T.B = v_j0;
                        T.C = v_j1;

                        if(pf.find_edge(T.C,T.B) == -1 || pf.find_edge(T.A,T.C) == -1)
                        {
                            ret.push_back(T);
                        //graph.lines.push_back(core::line3df(v0,v2));
                        //graph.lines.push_back(core::line3df(v1,v2));
                        }
                    }
                }
            }

            for(poly_edge edge_k : connexR)
            {
                int v_k1 = pf.get_opposite_end(edge_k, v_k0);
                if(v_k1 == v_i1)
                {
                    core::vector3df v0 = pf.vertices[v_i0].V;
                    core::vector3df v1 = pf.vertices[v_k0].V;
                    core::vector3df v2 = pf.vertices[v_k1].V;

                    core::vector3df r0 = v1-v0;
                    core::vector3df r1 = v2-v1;

                    if(vec_is_left_from(r1,r0))
                    {
                        triangle T;
                        T.A = v_i0;
                        T.B = v_k0;
                        T.C = v_k1;

                        if(pf.find_edge(T.A,T.C) == -1 || pf.find_edge(T.C,T.B) == -1)
                            ret.push_back(T);
                        //graph.lines.push_back(core::line3df(v0,v2));
                        //graph.lines.push_back(core::line3df(v1,v2));
                    }
                }
            }
        }
    }

    return ret;
}


std::vector<triangle> get_all_double_triangles(polyfold pf, int v_i)
{
    std::vector<triangle> ret;

    //int p_i = pf.get_vert_loop_no(0,v_i);
    std::vector<int> loop_nos = pf.get_vert_loop_no(0,v_i);

    std::vector<poly_edge> connex0;
    for(int p_i: loop_nos)
    {
        std::vector<poly_edge> loop_connex = get_all_connections(pf,p_i,v_i);
        for(poly_edge edge : loop_connex)
            connex0.push_back(edge);
    }

    //std::vector<poly_edge> connex0 = get_all_connections(pf,p_i,v_i);
    for(poly_edge edge : connex0)
    {
        int v_0= v_i;
        int v_1;
        if(edge.v0==v_0)
            v_1 = edge.v1;
        else
            v_1 = edge.v0;

        //int p_j = pf.get_vert_loop_no(0,v_1);
        loop_nos = pf.get_vert_loop_no(0,v_1);
        int p_j = loop_nos[0];

        std::vector<poly_edge> connex1 = get_all_connections(pf,p_j,v_1);

        for(poly_edge edge1 : connex1)
        {
            int v_2;
            if(edge1.v0 == v_1)
                v_2 = edge1.v1;
            else
                v_2 = edge1.v0;

            if(v_2 != v_0)
            {
                std::vector<int> con_edges = pf.get_edges_from_point(0,v_0);

                for(int v_ii : con_edges)
                    {
                        if(pf.edges[v_ii].v0 == v_2 || pf.edges[v_ii].v1 == v_2)
                        {
                            //graph.lines.push_back(core::line3df(pf.vertices[v_0].V,pf.vertices[v_1].V));
                            //graph.lines.push_back(core::line3df(pf.vertices[v_1].V,pf.vertices[v_2].V));
                            //graph.lines.push_back(core::line3df(pf.vertices[v_0].V,pf.vertices[v_2].V));

                            core::vector3df v0 = pf.vertices[v_0].V;
                            core::vector3df v1 = pf.vertices[v_1].V;
                            core::vector3df v2 = pf.vertices[v_2].V;

                            core::vector3df r0 = v1-v0;
                            core::vector3df r1 = v2-v1;
                            core::vector3df r2 = v0-v2;
                            std::cout<<"a\n";
                            bool b=false;
                            /*
                            if(vec_is_left_from(r1,r0) && vec_is_left_from(r2,r1))
                            {
                                std::cout<<"left!\n";
                                for(int v_j=0;v_j<pf.vertices.size();v_j++)
                                {
                                    core::vector3df w0 = pf.vertices[v_j].V - core::vector3df v0;
                                    core::vector3df w1 = pf.vertices[v_j].V - core::vector3df v1;
                                    core::vector3df w2 = pf.vertices[v_j].V - core::vector3df v2;
                                }
                            }
                            if(vec_is_right_from(r1,r0) && vec_is_right_from(r2,r1))
                            {
                                std::cout<<"right!\n";
                            }
*/
                            if(!b)
                            {
                                triangle T;
                                T.A = v_0;
                                T.B = v_1;
                                T.C = v_2;
                                ret.push_back(T);
                            }

                            //int new_e = pf.get_edge_or_add(v_0,v_1,3);
                            //pf.faces[0].addEdge(new_e);
                        }
                    }
            }
        }
    }
    return ret;
}

triangle get_best_2triangle(polyfold pf)
{
    triangle ret;
    f32 best = 0.0;
    for(int i=0;i<pf.vertices.size();i++)
    {
        std::vector<triangle> vT = get_all_double_triangles(pf,i);
        for(triangle T : vT)
        {
            core::line2df line_a = core::line2df(pf.vertices[T.A].V.X,pf.vertices[T.A].V.Z,pf.vertices[T.B].V.X,pf.vertices[T.B].V.Z);
            core::line2df line_b = core::line2df(pf.vertices[T.B].V.X,pf.vertices[T.B].V.Z,pf.vertices[T.C].V.X,pf.vertices[T.C].V.Z);
            core::line2df line_c = core::line2df(pf.vertices[T.A].V.X,pf.vertices[T.A].V.Z,pf.vertices[T.C].V.X,pf.vertices[T.C].V.Z);

            f32 aa = line_a.getAngleWith(line_b);
            f32 ab = line_b.getAngleWith(line_c);
            f32 ac = line_a.getAngleWith(line_c);

            f32 mina = std::min(aa,std::min(ab,ac));
            if(mina > best)
            {
                best = mina;
                ret = T;

            }
        }
    }
    return ret;
}

triangle get_shortest_triangle(polyfold pf, std::vector<triangle> tV)
{
    triangle ret;

    if(tV.size()==1)
        return tV[0];

    f32 best=99999;

    for(triangle T : tV)
    {
        core::line2df line_a = core::line2df(pf.vertices[T.A].V.X,pf.vertices[T.A].V.Z,pf.vertices[T.B].V.X,pf.vertices[T.B].V.Z);
        core::line2df line_b = core::line2df(pf.vertices[T.B].V.X,pf.vertices[T.B].V.Z,pf.vertices[T.C].V.X,pf.vertices[T.C].V.Z);
        core::line2df line_c = core::line2df(pf.vertices[T.A].V.X,pf.vertices[T.A].V.Z,pf.vertices[T.C].V.X,pf.vertices[T.C].V.Z);

        f32 lena = line_a.getLength();
        f32 lenb = line_b.getLength();
        f32 lenc = line_c.getLength();

        f32 maxl = std::max(lena,std::max(lenb,lenc));
        if(( pf.find_edge(T.A,T.B)== -1 || pf.find_edge(T.B,T.C)== -1 || pf.find_edge(T.A,T.C)== -1)
           &&(maxl < best))
        {
            ret = T;
            best = maxl;
        }
    }
    return ret;
}



int start_triangles_from(polyfold& pf, int v_i, int steps_left0, LineHolder& graph)
{
    //graph.points.push_back(pf.vertices[v_i].V);


    if(steps_left0 != -1)
    std::cout<<steps_left0<<"\n";

    std::vector<triangle> tV = get_alternate_single_triangles(pf,v_i, graph);
    if(tV.size()==0)
         tV = get_all_single_triangles(pf,v_i, graph);
    triangle T;

    std::vector<int> next_points;

    int n_triangles=0;
    //if(false)
    if(tV.size()>0)
    {
        std::cout<<"1 line  ";
        T = get_shortest_triangle(pf, tV);

        std::cout<<"     "<<T.A<<" "<<T.B<<" "<<T.C<<"\n";

        if( T.A != -1 && pf.find_edge(T.A,T.B) == -1)
        {
            int new_e = pf.get_edge_or_add(T.A,T.B,3);
            //graph.lines.push_back(core::line3df(pf.vertices[T.A].V,pf.vertices[T.B].V));
            //graph.lines.push_back(core::line3df(pf.vertices[T.B].V,pf.vertices[T.C].V));
            //graph.lines.push_back(core::line3df(pf.vertices[T.C].V,pf.vertices[T.A].V));
            pf.faces[0].addEdge(new_e);
            std::cout<<"new edge "<<T.A<<" "<<T.B<<" \n";//<<T.C<<"\n";
            n_triangles++;
            //n_triangles+= start_triangles_from(pf,T.B,graph);
            //n_triangles+= start_triangles_from(pf,T.A,graph);
            next_points.push_back(T.A);
            next_points.push_back(T.B);
        }
    }
    //if(false)
    {

        tV = get_all_double_triangles(pf,v_i);
        if(tV.size()>0)
        {

            T = get_shortest_triangle(pf, tV);

            std::cout<<"2 lines ";
            std::cout<<"      "<<T.A<<" "<<T.B<<" "<<T.C<<"\n";

            if(T.A != -1)
            {
                bool new_ac = (pf.find_edge(T.A,T.C)==-1);
                bool new_bc = (pf.find_edge(T.B,T.C)==-1);
                bool new_ab = (pf.find_edge(T.A,T.B)==-1);

                //graph.lines.push_back(core::line3df(pf.vertices[T.A].V,pf.vertices[T.B].V));
                //graph.lines.push_back(core::line3df(pf.vertices[T.B].V,pf.vertices[T.C].V));
                //graph.lines.push_back(core::line3df(pf.vertices[T.C].V,pf.vertices[T.A].V));
                if(new_ab)
                {
                    int new_e = pf.get_edge_or_add(T.A,T.B,3);
                    //graph.lines.push_back(core::line3df(pf.vertices[T.A].V,pf.vertices[T.B].V));
                    pf.faces[0].addEdge(new_e);
                    std::cout<<"new edge (2) "<<T.A<<" "<<T.B<<"\n";//<<T.C<<"\n";
                }

                if(new_ac)
                {
                    int new_e = pf.get_edge_or_add(T.A,T.C,3);
                    //graph.lines.push_back(core::line3df(pf.vertices[T.A].V,pf.vertices[T.B].V));
                    pf.faces[0].addEdge(new_e);
                    std::cout<<"new edge (2) "<<T.A<<" "<<T.C<<"\n";//<<T.C<<"\n";
                }

                if(new_bc)
                {
                    int new_e = pf.get_edge_or_add(T.B,T.C,3);
                    //graph.lines.push_back(core::line3df(pf.vertices[T.B].V,pf.vertices[T.C].V));
                    pf.faces[0].addEdge(new_e);
                    std::cout<<"new edge (2) "<<T.B<<" "<<T.C<<"\n";//<<T.C<<"\n";
                }

                n_triangles+=2;

                if(new_ac)
                    next_points.push_back(T.B);
                    //n_triangles += start_triangles_from(pf,T.B,graph);
                if(new_bc)
                    next_points.push_back(T.C);
                    //n_triangles += start_triangles_from(pf,T.C,graph);


            }
        }
    }

    int steps_left = steps_left0;
    for(int next_v : next_points)
    {
        if(steps_left>0)
            steps_left--;

        if(steps_left !=0)
            start_triangles_from(pf,next_v,steps_left,graph);
    }

    return n_triangles;
}

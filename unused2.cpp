/*

void polyfold::calc_loops(poly_face& face, std::vector<int> rules)
{
        //Assign vertices on each face to a loop
        std::vector<int> temp1;
        std::vector<int> temp2;
        std::vector<int> temp3;

       // face.loops.clear();
        bool still_has_edges = true;
        bool keep_trying = true;
        bool not_done = true;

        int cur_edge;

        int first_v;
        int cur_v;

        poly_loop new_loop;

        if(rules.size() == 0 )
            temp1=face.edges;
        else
            for(int e_i : face.edges)
                for(int rule : rules)
                    if(this->edges[e_i].topo_group == rule)
                    {
                        temp1.push_back(e_i);
                    }

        if(temp1.size()<3)
        {
            //std::cout<<"not enough elements ("<<temp1.size()<<")\n";
            return;
        }

        //std::cout<<"\n size is "<<temp1.size()<<"\n";

        //std::cout<<"new face\n";
        while(keep_trying)
        {
            first_v = this->edges[temp1[0]].v0;
            cur_v = this->edges[temp1[0]].v1;
            cur_edge = temp1[0];

            new_loop.vertices.clear();
            new_loop.vertices.push_back(first_v);
            new_loop.vertices.push_back(cur_v);

            //std::cout<<"new loop, size is "<<temp1.size()<<"\n";
            //for(int e_i : temp1)
            //    std::cout<<e_i<<" ";
            //std::cout<<"\n";
            if(temp1.size()>2)
            {
                //std::cout<<" add "<<first_v<<"\n";
                //std::cout<<" add "<<cur_v<<"\n";
                temp2.clear();
                for(int i=1; i<temp1.size(); i++)
                    temp2.push_back(temp1[i]);
                temp1=temp2;
                //std::cout<<"A\n";
                do
                {
                    not_done=false;
                    keep_trying=false;
                    temp2.clear();
                    //std::cout<<"B\n";
                    for(int i=0;i<temp1.size();i++)
                    {
                        if(edges[temp1[i]].v0 == cur_v && temp1[i] != cur_edge)
                        {
                            //temp3.push_back(temp1[i]);
                            cur_v = edges[temp1[i]].v1;
                            if(cur_v == first_v)
                            {
                                not_done = false;
                                keep_trying = true;
                                face.loops.push_back(new_loop);
                                //std::cout<<"created new loop with "<<new_loop.vertices.size()<<" vertices\n";
                            }
                            else
                            {
                                //std::cout<<temp1[i]<<" links "<<edges[temp1[i]].v1<<" "<<edges[temp1[i]].v0<<"\n";
                                new_loop.vertices.push_back(edges[temp1[i]].v1);
                                not_done=true;
                                keep_trying=true;
                                cur_edge = temp1[i];
                            }
                        }
                        else if(edges[temp1[i]].v1 == cur_v && temp1[i] != cur_edge)
                        {
                            //temp3.push_back(temp1[i]);
                            cur_v = edges[temp1[i]].v0;
                            if(cur_v == first_v)
                            {
                                //i=temp1.size();
                                not_done = false;
                                keep_trying = true;
                                face.loops.push_back(new_loop);
                               // std::cout<<"created new loop with "<<new_loop.vertices.size()<<" vertices\n";
                            }
                            else
                            {
                               // std::cout<<" add "<<edges[temp1[i]].v0<<"\n";
                               // std::cout<<temp1[i]<<" links "<<edges[temp1[i]].v1<<" "<<edges[temp1[i]].v0<<"\n";
                                new_loop.vertices.push_back(edges[temp1[i]].v0);
                                not_done=true;
                                keep_trying=true;
                                cur_edge = temp1[i];

                            }
                        }
                        else {
                                temp2.push_back(temp1[i]);
                        }
                    }
                    temp1 = temp2;
                    //std::cout<<"still searching "<<temp2.size()<<"\n";
                } while(not_done);
            }
            else keep_trying = false;
        }

        if(temp1.size()>0)
        {
            for(int v_i : new_loop.vertices)
            {
           //         std::cout<<v_i<<" ";
            }
        //std::cout<<"\n";
          for(int e_i : temp1)
          {
              //  std::cout<<"could not allocate "<<e_i<<" edge\n";
             //   std::cout<<edges[e_i].v0<<" "<<edges[e_i].v1<<"\n";
          }
        }
      //  if(face.loops.size()==0)
      //      std::cout<<"warning: face has 0 loops\n";
}
*/

/*

int polyfold::point_is_behind_point2(int point, core::vector3df v )
{

    std::vector<int> convex_edges;
    std::vector<int> concave_edges;
    std::vector<int> undef_edges;

   // std::cout<<point<<" point\n ";

    for(int e_i =0; e_i< this->edges.size(); e_i++)
    {
        if(this->edges[e_i].v0 == point || this->edges[e_i].v1 == point)
        {
            if(this->classify_edge(e_i)==TOP_CONVEX)
                convex_edges.push_back(e_i);
            else if(this->classify_edge(e_i)==TOP_CONCAVE)
                concave_edges.push_back(e_i);
            else
                undef_edges.push_back(e_i);
        }
    }
    //std::cout<<convex_edges.size()<<","<<concave_edges.size()<<"  ("<<undef_edges.size()<<") edges\n ";

    if(undef_edges.size()>0)
        return TOP_UNDEF;

    bool bIsBehindConvex = true;

    for(int e_i : convex_edges)
        if(this->point_is_behind_edge(e_i,v)==false)
            bIsBehindConvex = false;

    bool bIsBehindConcave = true;
    if(concave_edges.size() > 0)
    {
        bIsBehindConcave = false;
        for(int e_i : concave_edges)
            if(this->point_is_behind_edge(e_i,v)==true)
               bIsBehindConcave = true;
    }

    if (bIsBehindConvex && bIsBehindConcave)
        return TOP_BEHIND;
    else
        return TOP_FRONT;
}
*/

/*

void polyfold::apply_topology_groups2(int default_group)
{
    std::vector<int> c_edges;
    int counts=0;
    while(this->exists_topology_group(2))
    {
        for(int e_i = 0; e_i<this->edges.size(); e_i++)
        {
            poly_edge edge = this->edges[e_i];

            if(edge.topo_group==2)
            {
                c_edges = this->get_connecting_edges(e_i);

                bool b=true;
                for(int e_j : c_edges)
                {
                    if(this->edges[e_j].topo_group == 3 || this->edges[e_j].topo_group == 5)
                        b=false;
                }

                if(b)
                for(int e_j : c_edges)
                {
                    if(this->edges[e_j].topo_group == 0 || this->edges[e_j].topo_group == 1)
                    {
                        this->edges[e_i].topo_group = this->edges[e_j].topo_group;
                    }
                }
            }
        }
    counts++;
    if(counts > 200)
        {
            for(int e_i = 0; e_i<this->edges.size(); e_i++)
            {
                if(this->edges[e_i].topo_group==2)
                    this->edges[e_i].topo_group=default_group;
            }
         std::cout<<"no intersections... using default value\n";
         break;
        }
    }
}
*/
/*

polyfold clip_poly(polyfold& pf, polyfold& pf2, int rule, int rule2, LineHolder &graph)
{
    pf.classify_edges(graph);
    pf2.classify_edges(graph);

    polyfold pf4 = pf;
    polyfold pf5 = pf2;
    polyfold ret;

    std::cout<<"Creating new geometry...\n";

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
    for(int face_j=0;face_j<pf2.faces.size();face_j++)
    {
        poly_face f2=pf2.faces[face_j];
        core::plane3df f2_plane = core::plane3df(f2.m_center,f2.m_normal);
        core::vector3df ipoint;

        for(int e_i=0; e_i<pf4.edges.size(); e_i++)
        {
            core::vector3df v0=pf4.getVertex(e_i,0).V;
            core::vector3df v1=pf4.getVertex(e_i,1).V;
            if(line_intersects_plane(f2_plane,v0,v1,ipoint) && pf4.edges[e_i].topo_group != -1
               && pf2.is_point_on_face(&f2,ipoint))
            {
                int new_v = pf4.get_point_or_add(ipoint);
                pf4.bisect_edge(e_i,new_v,2,2);
               // graph.points.push_back(ipoint);
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
               && pf.is_point_on_face(&f,ipoint))
            {
                int new_v = pf5.get_point_or_add(ipoint);
                pf5.bisect_edge(e_i,new_v,2,2);
               // graph.points.push_back(ipoint);
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
               && pf2.is_point_on_face(&f2,ipoint))
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

    //catch special cases
    for(int e_i=0; e_i<pf4.edges.size(); e_i++)
        {
            if(pf4.edges[e_i].topo_group == 2)
                {
                core::vector3df v0 = pf4.getVertex(e_i,0).V;
                core::vector3df v1 = pf4.getVertex(e_i,1).V;
                int RES = TOP_UNDEF;
                int v_0 = pf2.find_point(v0);
                int v_1 = pf2.find_point(v1);

                if(v_0 != -1)
                {
                    RES = pf2.classify_point_point_special(v_0,v1);
                }
                if(v_1 != -1)
                {
                    RES = pf2.classify_point_point_special(v_1,v0);
                }

                if(RES == TOP_FRONT)
                    pf4.edges[e_i].topo_group=0;
                else if(RES == TOP_BEHIND)
                    pf4.edges[e_i].topo_group=1;
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
               && pf.is_point_on_face(&f,ipoint))
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
                    if(abs(f2_plane.getDistanceTo(v))<0.01 && pf2.is_point_on_face(&f2,v))
                    {
                        verts.get_point_or_add(v);
                    }
                }

                for(int v_j : pf5.faces[face_j].vertices)
                {
                    core::vector3df v = pf5.vertices[v_j].V;
                    if(abs(f_plane.getDistanceTo(v))<0.01 && pf.is_point_on_face(&f,v))
                    {
                        verts.get_point_or_add(v);
                    }
                }
                if(verts.vertices.size()>0)
                {
                    sort_inline_vertices(verts);
                    for(int i=0;i<verts.vertices.size()-1;i++)
                    {
                        core::vector3df r = verts.vertices[i].V + (verts.vertices[i+1].V - verts.vertices[i].V)*0.5;
                        if(pf.is_point_on_face(&f,r) && pf2.is_point_on_face(&f2,r))
                        {
                            int v0 = pf4.get_point_or_add(verts.vertices[i].V);
                            int v1 = pf4.get_point_or_add(verts.vertices[i+1].V);
                            int w0 = pf5.get_point_or_add(verts.vertices[i].V);
                            int w1 = pf5.get_point_or_add(verts.vertices[i+1].V);

                            int new_e;

                            if(pf4.find_edge(v0,v1)==-1)
                            {
                            new_e = pf4.get_edge_or_add(v0,v1,3);
                            pf4.faces[face_i].addEdge(new_e);

                            //new_e = pf5.get_edge_or_add(w0,w1,3);
                            //pf5.faces[face_j].addEdge(new_e);
                            }
                            else if(pf4.find_edge(v0,v1) != -1 && pf5.find_edge(w0,w1) != -1)
                            {
                            //new_e = pf4.find_edge(v0,v1);
                            //if(pf4.edges[new_e].topo_group==2)
                            //    pf4.edges[new_e].topo_group=!rule;
                            }

                            new_e = pf5.get_edge_or_add(w0,w1,3);
                            pf5.faces[face_j].addEdge(new_e);
                        }
                        //graph.lines.push_back(core::line3df(verts.vertices[i].V,verts.vertices[i+1].V));
                    }
                }
            }
        }

    //========================================

    pf4.apply_topology_groups(rule);
    pf5.apply_topology_groups(rule2);

    //if(false)
    //remove superfluous edges from faces
    {
        for(int f_i=0; f_i < pf4.faces.size(); f_i++)
        {
            pf4.faces[f_i].loops.clear();

            //std::cout<<pf4.faces[f_i].edges.size()<<"\n";

            std::vector<int> rules;
            rules.push_back(!rule);
            rules.push_back(3);
            pf4.calc_loops2(pf4.faces[f_i],rules);

            //cull loops
            for(poly_loop loop : pf4.faces[f_i].loops)
            {
                //std::cout<<"vol loop with "<<loop.vertices.size()<<"\n";
                std::vector<int> tevec;
                std::vector<int> tevec2;

                //if(!keep)
                {
                    for(int i = 0; i<loop.vertices.size()-1; i++)
                        tevec.push_back(pf4.find_edge(loop.vertices[i],loop.vertices[i+1]));
                    tevec.push_back(pf4.find_edge(loop.vertices[loop.vertices.size()-1],loop.vertices[0]));

                    for(int e_i : tevec)
                        if(pf4.edges[e_i].topo_group==!rule)
                        {
                            tevec2.push_back(e_i);
                        }
                    tevec.clear();
                    for(int e_i : pf4.faces[f_i].edges)
                    {
                        bool b = false;
                        for(int e_j : tevec2)
                        {
                            if(e_i == e_j)
                                b=true;
                        }
                        if(!b)
                            tevec.push_back(e_i);
                    }
                    pf4.faces[f_i].edges=tevec;
                }
            }
            pf4.faces[f_i].loops.clear();
        }

        for(int e_i = 0; e_i <pf4.edges.size(); e_i++)
        {
            if(pf4.edges[e_i].topo_group==!rule)
            {
                pf4.edges[e_i].topo_group=3;
            }
        }

        //initial loops - pf4
        for(int f_i=0; f_i < pf4.faces.size(); f_i++)
        {
            pf4.faces[f_i].loops.clear();

            std::vector<int> rules;
            rules.push_back(rule);
            rules.push_back(3);
            pf4.calc_loops2(pf4.faces[f_i],rules);
            pf4.sort_loops(pf4.faces[f_i]);

            std::cout<<"face "<<f_i<<": "<<pf4.faces[f_i].loops.size()<<" loops ";
            for(poly_loop loop: pf4.faces[f_i].loops)
                std::cout<<loop.vertices.size()<<" ";
            std::cout<<"\n";
        }

        std::cout<<"...\n";

        //initial loops - pf5
        for(int f_i=0; f_i < pf5.faces.size(); f_i++)
        {
            pf5.faces[f_i].loops.clear();

            std::vector<int> rules;
            rules.push_back(rule2);
            rules.push_back(3);
            pf5.calc_loops2(pf5.faces[f_i],rules);
            pf5.sort_loops(pf5.faces[f_i]);

            std::cout<<"face "<<f_i<<": "<<pf5.faces[f_i].loops.size()<<" loops ";
            for(poly_loop loop: pf5.faces[f_i].loops)
                std::cout<<loop.vertices.size()<<" ";
            std::cout<<"\n";
        }

        //cull loops which are kissing
        num=0;
        for(int f_i=0; f_i < pf4.faces.size(); f_i++)
        {
            if(pf4.faces[f_i].loops.size()>0)
            {
                for(int p_i=0; p_i < pf4.faces[f_i].loops.size(); p_i++)
                {
                    if(pf4.faces[f_i].loops[p_i].vertices.size()>0)
                    {
                        for(int f_j=0; f_j < pf5.faces.size(); f_j++)
                        for(int p_j=0; p_j < pf5.faces[f_j].loops.size(); p_j++)
                        {
                            bool bRemoveLoop=true;
                            for(v_i : pf4.faces[f_i].loops[p_i].vertices)
                            {
                                bool bIsDouble=false;
                                for(v_j : pf5.faces[f_j].loops[p_j].vertices)
                                {
                                    if(is_same_point(pf4.vertices[v_i].V,pf5.vertices[v_j].V)==true)
                                        bIsDouble = true;
                                }
                                if(!bIsDouble)
                                    bRemoveLoop = false;
                            }
                            if(bRemoveLoop)
                            {
                                std::cout<<"cull loop "<<f_i<<" "<<p_i<<" vs "<<f_j<<" "<<p_j<<"\n";
                                pf4.faces[f_i].loops[p_i].vertices.clear();
                                num++;
                                goto LoopHasBeenCulled;
                                //pf5.faces[f_j].loops[p_j].vertices.clear();
                            }
                        }
                    }
                    LoopHasBeenCulled:
                    int z=0;
                }
            }
        }
        std::cout<<"culled "<<num<<" loops\n";

        //return object
        //if(false)
        for(int f_i=0; f_i < pf4.faces.size(); f_i++)
        {
            poly_face f;
            f.m_normal = pf4.faces[f_i].m_normal;
            f.bFlippedNormal = pf4.faces[f_i].bFlippedNormal;

            for(poly_loop loop : pf4.faces[f_i].loops)
            {
                if(loop.vertices.size() > 0)
                {
                    poly_loop new_loop;
                    for(int v_i : loop.vertices)
                    {
                        int v = ret.get_point_or_add(pf4.vertices[v_i].V);
                        new_loop.vertices.push_back(v);
                        f.addVertex(v);
                    }

                    std::vector<int> tempv = loop.vertices;
                    tempv.push_back(tempv[0]);

                    for(int i=0; i < tempv.size()-1; i++)
                       {
                        core::vector3df v0=pf4.vertices[tempv[i]].V;
                        core::vector3df v1=pf4.vertices[tempv[i+1]].V;
                        int new_e = ret.get_edge_or_add(ret.get_point_or_add(v0),ret.get_point_or_add(v1),0);
                        f.addEdge(new_e);
                       }

                    new_loop.type = loop.type;
                    ret.calc_loop_bbox(f,new_loop);
                    f.loops.push_back(new_loop);
                }
            }

            ret.calc_center(f);
            if(f.edges.size()>2 && f.loops.size() > 0)
            {
                ret.sort_loops(f);
                ret.faces.push_back(f);
            }
        }
        //ret.recalc_bbox_and_loops();

        if(ret.vertices.size()>0)
        {
            ret.bbox.reset(ret.vertices[0].V);
            for(poly_vert v : ret.vertices)
                ret.bbox.addInternalPoint(v.V);
        }

    }//=======


    //ret.topology = pf4.topology;

    //return pf5;
    return ret;
}*/


/*
void polyfold::calc_loops3(int f_i, LineHolder& graph)
{
    for(poly_edge &edge: this->edges)
        edge.p2= -1;//edge.perimeter;

    LineHolder nograph;

    this->faces[f_i].loops.clear();

    for(int e_0 : this->faces[f_i].edges)
    {
        if(this->edges[e_0].p2 == -1 && this->edges[e_0].topo_group != -1)
        {
            std::vector<int> evec = this->get_all_connected_edges(f_i,e_0);
            std::cout<<"found "<<evec.size()<<" more edges\n";
            for(int e_i : evec)
            {
                //std::cout<<e_i;
                std::vector<int> links0 = this->get_edges_from_point(f_i,e_i,this->edges[e_i].v0);
                std::vector<int> links1 = this->get_edges_from_point(f_i,e_i,this->edges[e_i].v1);
                if(links0.size()>1 && links1.size() >1)
                {
                    this->edges[e_i].p2 = 1;
                    //std::cout<<"! ";
                }
                else
                {
                    this->edges[e_i].p2 = 0;
                   // std::cout<<" ";
                }
            }
           // std::cout<<"\n";
        }
    }

    for(int e_0 : this->faces[f_i].edges)
    {
        if(this->edges[e_0].p2 == 0 && this->edges[e_0].topo_group != -1)
        {
            //std::cout<<"(loops) f: "<<f_i<<" e: "<<e_0<<"\n";
            std::vector<int> res;

           // if(e_0==17)
                res = this->get_loop_from_edge(f_i, e_0, graph);
           // else
           //     res = this->get_loop_from_edge(f_i, e_0, nograph);

            //for(int i : res)
            //    std::cout<<i<<" ";
            //std::cout<<"\n";

            // for(int i : res)
            //    std::cout<<this->edges[i].v0<<","<<this->edges[i].v1<<" ";
            //std::cout<<"\n";

            if(res.size()>0)
            {
                poly_loop loop;
                int v0;
                if(this->edges[res[0]].v0 == this->edges[res[1]].v0 || this->edges[res[0]].v0 == this->edges[res[1]].v1)
                    v0 = this->edges[res[0]].v1;
                else
                    v0 = this->edges[res[0]].v0;
                for(int e_i : res)
                {
                    //std::cout<<v0<<" ";
                    loop.vertices.push_back(v0);
                    v0 = get_opposite_end(e_i,v0);
                    //std::cout<<v0<<" ";
                    if(this->edges[e_i].p2 == 0)
                        this->edges[e_i].p2 = -1;
                    else if(this->edges[e_i].p2 == 1)
                        this->edges[e_i].p2 = 0;
                }
                //std::cout<<"\n";

                this->faces[f_i].loops.push_back(loop);
            }

        }
    }

    std::cout<<"face "<<f_i<<": ";
    for(int i = 0; i< this->faces[f_i].loops.size(); i++)
        std::cout<<this->faces[f_i].loops[i].vertices.size()<<" ";
    std::cout<<"\n";

}
*/

/*
core::vector3df polyfold::get_test_vec(int face_i, int p_i)
{
    std::vector<int> v_list;

    poly_face f = this->faces[face_i];

    v_list.push_back(f.loops[p_i].vertices[f.loops[p_i].vertices.size()-1]);
    for(int v_i : f.loops[p_i].vertices)
    {
        v_list.push_back(v_i);
    }
    v_list.push_back(v_list[1]);

    int my_v = 1;

    core::vector3df v0      = this->vertices[v_list[my_v]].V;
    core::vector3df v_left  = this->vertices[v_list[my_v+1]].V;
    core::vector3df v_right = this->vertices[v_list[my_v-1]].V;

    bool rev = this->is_closed_loop(f,f.loops[p_i]);

    //if(rev)
    core::line3df r = core::line3df(v_left, v_right);
    core::vector3df ret = r.getMiddle();

    //if(!rev)
    //    ret = v0+ (v0 - ret);

    //ret.normalize();
   // ret*=16;
    return ret;
}
*/


/*
void polyfold::apply_topology_groups2(int default_group)
{
    std::vector<int> c_edges;
    int counts=0;
    while(this->exists_topology_group(2))
    {
        for(int e_i = 0; e_i<this->edges.size(); e_i++)
        {
            poly_edge edge = this->edges[e_i];

            if(edge.topo_group==2)
            {
                c_edges = this->get_connecting_edges(e_i);
                for(int e_j : c_edges)
                {
                    if(this->edges[e_j].topo_group == default_group)
                    {
                        this->edges[e_i].topo_group = this->edges[e_j].topo_group;
                    }
                }
            }
        }
    counts++;
    if(counts > 100)
        {
            for(int e_i = 0; e_i<this->edges.size(); e_i++)
            {
              //  if(this->edges[e_i].topo_group==2)
              //      this->edges[e_i].topo_group=default_group;
            }
         std::cout<<"no intersections... using default value\n";
         break;
        }
    }
}
*/


/*
void polyfold::reduce_edges_vertices()
{
    for(poly_edge& edge: this->edges)
    {
        edge.topo_group=-1;
    }

    for(int f_i=0;f_i<this->faces.size();f_i++)
    {
        this->faces[f_i].edges.clear();

        for(int p_i = 0; p_i< this->faces[f_i].loops.size(); p_i++)
        {
            std::vector<int> tempv = this->faces[f_i].loops[p_i].vertices;
            tempv.push_back(tempv[0]);

            for(int i=0; i < tempv.size()-1; i++)
               {
                //core::vector3df v0=this->vertices[tempv[i]].V;
                //core::vector3df v1=this->vertices[tempv[i+1]].V;

                int e_i = this->find_edge(tempv[i],tempv[i+1]);
                this->faces[f_i].addEdge(e_i);
                this->edges[e_i].topo_group=0;
               }
        }
    }

    std::vector<poly_edge> new_edges;
    for(poly_edge edge: this->edges)
    {
        poly_edge new_e(0,0);
        if(edge.topo_group==0)
        {
            new_e=edge;
            new_edges.push_back(new_e);
        }
    }
    std::cout<<this->edges.size()<<" edges reduce to "<<new_edges.size()<<"\n";

    int* renumber = new int[this->edges.size()*2];
    int cc=0;
    for(int i=0; i<this->edges.size(); i++)
    {
        renumber[i*2]=i;
        if(this->edges[i].topo_group==0)
        {
            renumber[i*2+1]=cc;
            cc++;
        }
        else renumber[i*2+1]=-1;
       // std::cout<<renumber[i*2]<<","<<renumber[i*2+1]<<"\n";
    }

    for(int f_i=0;f_i<this->faces.size();f_i++)
    {
        for(int e_i=0;e_i<this->faces[f_i].edges.size();e_i++)
        {
            int index = this->faces[f_i].edges[e_i];
          //  std::cout<<index<<" -> "<<renumber[index*2+1]<<"\n";
            this->faces[f_i].edges[e_i] = renumber[index*2+1];
        }
    }

    this->edges = new_edges;

    delete[] renumber;

    //-----vertices

    int* a_vertlist = new int[this->vertices.size()];

    for(int i=0; i<this->vertices.size(); i++)
        a_vertlist[i]=0;

    for(int i=0; i<this->edges.size(); i++)
    {
        a_vertlist[this->edges[i].v0]=1;
        a_vertlist[this->edges[i].v1]=1;
    }

    std::vector<poly_vert> new_verts;

    for(int i=0; i<this->vertices.size(); i++)
    {
        poly_vert avert(0,0,0);
        if(a_vertlist[i]==1)
        {
            avert = this->vertices[i];
            new_verts.push_back(avert);
        }
    }

    std::cout<<this->vertices.size()<<" vertices reduce to "<<new_verts.size()<<"\n";


    renumber = new int[this->vertices.size()*2];
    cc=0;
    for(int i=0; i<this->vertices.size(); i++)
    {
        renumber[i*2]=i;
        if(a_vertlist[i]==1)
        {
            renumber[i*2+1]=cc;
            cc++;
        }
        else renumber[i*2+1]=-1;
       // std::cout<<renumber[i*2]<<","<<renumber[i*2+1]<<"\n";
    }

    for(poly_edge& edge : this->edges)
    {
        int index = edge.v0;
        edge.v0 = renumber[index*2+1];
        //std::cout<<index<<" -> "<<renumber[index*2+1]<<"\n";
        index = edge.v1;
        edge.v1 = renumber[index*2+1];
        //std::cout<<index<<" -> "<<renumber[index*2+1]<<"\n";
    }

    for(int f_i=0;f_i<this->faces.size();f_i++)
    {
        this->faces[f_i].vertices.clear();
        for(int p_i = 0; p_i< this->faces[f_i].loops.size(); p_i++)
        {
            for(int i=0; i< this->faces[f_i].loops[p_i].vertices.size();i++)
            {
                int index = this->faces[f_i].loops[p_i].vertices[i];
                this->faces[f_i].loops[p_i].vertices[i]=renumber[index*2+1];
                //std::cout<<index<<" -> "<<renumber[index*2+1]<<"\n";
                this->faces[f_i].addVertex(renumber[index*2+1]);
            }
        }
    }

    this->vertices = new_verts;

    delete[] a_vertlist;
    delete[] renumber;
}
*/
//
//
//
/*
void polyfold::classify_loop(polyfold &pf4, polyfold &pf5, int loop_i)
{

    if(pf4.faces[f_i].loops[loop_i].vertices.size()>0)
    {
        for(int f_j=0; f_j < pf5.faces.size(); f_j++)
        for(int p_j=0; p_j < pf5.faces[f_j].loops.size(); p_j++)
        {
            if(is_identical_loop(pf4, pf4.faces[f_i].loops[loop_i],pf5, pf5.faces[f_j].loops[p_j]))
            {
                if(pf4.faces[f_i].m_normal.dotProduct(pf5.faces[f_j].m_normal) > 0)
                {
                    std::cout<<"cull loop "<<f_i<<" "<<loop_i<<" vs "<<f_j<<" "<<p_j<<"\n";
                    pf4.faces[f_i].loops[loop_i].vertices.clear();
                    //
                }
                else if(pf4.faces[f_i].loops[loop_i].type==LOOP_INNER)
                {
                    std::cout<<"cull loop "<<f_i<<" "<<loop_i<<" vs "<<f_j<<" "<<p_j<<"\n";
                    pf4.faces[f_i].loops[loop_i].vertices.clear();
                    //
                }
                //pf5.faces[f_j].loops[p_j].vertices.clear();
            }
        }
    }

      std::vector<int> tempv = pf4.faces[f_i].loops[p_i].vertices;
        tempv.push_back(tempv[0]);

        bool exclude=false;
        bool include=false;

        for(int i=0; i < tempv.size()-1; i++)
           {
            core::vector3df v0=pf4.vertices[tempv[i]].V;
            core::vector3df v1=pf4.vertices[tempv[i+1]].V;
            if(pf4.edges[pf4.get_edge_or_add(tempv[i],tempv[i+1],!rule)].topo_group == !rule )
                exclude = true;
            else if(pf4.edges[pf4.get_edge_or_add(tempv[i],tempv[i+1],!rule)].topo_group == rule )
                include = true;
            int new_e = ret.get_edge_or_add(ret.get_point_or_add(v0),ret.get_point_or_add(v1),0);
            f.addEdge(new_e);
           }

        if(!exclude && !include)
        {
            std::cout<<"odd loop: "<<f_i<<", "<<p_i<<" ";
            core::vector3df v0=pf4.vertices[tempv[0]].V;
            core::vector3df v1;//=pf4.vertices[tempv[i+1]].V;
            int e_i = pf4.find_edge(tempv[0],tempv[1]);


            if(e_i != -1 && pf4.get_facial_point3(f_i,e_i,p_i,v1,nograph))
            {
                int res = pf2.classify_point(v0,v1);
                //graph.lines.push_back(core::line3df(v0,v1));
                //std::cout<<"res "<<res<<"\n";
                if    ((pf4.faces[f_i].loops[p_i].type == LOOP_INNER && res == rule )
                   || ( pf4.faces[f_i].loops[p_i].type == LOOP_OUTER && rule != rule2 )
                   || ( pf4.faces[f_i].loops[p_i].type == LOOP_INNER && cull_kissing == false))
                {
                    std::cout<<" keep\n";
                    exclude = false;
                }
                else
                {
                    std::cout<<" cull\n";
                    exclude = true;
                }
            }
            else exclude = true;
        }
}
*/


/*
bool polyfold::is_edge_loop(int face_i, int loop_i)
{
    //std::cout<<"face "<<face_i<<"\n";
    for(int p_i = 0; p_i< this->faces[face_i].loops.size(); p_i++)
    {
        if(this->faces[face_i].loops[p_i].depth == this->faces[face_i].loops[loop_i].depth &&
            this->faces[face_i].loops[p_i].type == LOOP_OUTER)
        {
           // std::cout<<"outer "<<p_i<<" inner "<<loop_i<<"\n";
            std::vector<int> e_vec0 = this->get_all_loop_edges(face_i,loop_i);
            std::vector<int> e_vec1 = this->get_all_loop_edges(face_i,p_i);
            for(int e_i : e_vec0)
                for(int e_j : e_vec1)
                    {
                        if(e_i == e_j)
                            return true;
                    }
        }
    }
    return false;
}
*/

/*
std::vector<int> polyfold::get_loop_from_edge(int f_i, int e_0, LineHolder& graph)
{
    std::vector<int> e_loop;
    e_loop.push_back(e_0);
    int v_0 = this->edges[e_0].v0;
    int v_i = this->edges[e_0].v1;
    int e_i = e_0;

    while(true)
        {
            std::vector<int> links_ = get_edges_from_point(f_i,e_i,v_i);

            std::vector<int> links;
            for(int i: links_)
                if(this->edges[i].p2 != -1)
                    links.push_back(i);

            if(links.size()==1)
            {
                e_i = links[0];
                v_i = this->get_opposite_end(e_i,v_i);
                e_loop.push_back(e_i);
                //std::cout<<e_i<<".";
                graph.lines.push_back(core::line3df(this->getVertex(e_i,0).V,this->getVertex(e_i,1).V));
                if(v_i == v_0)
                {
                    //std::cout<<"single loop\n";
                    return e_loop;
                }
            }
            else break;
        };

    std::vector<int> links_ = get_edges_from_point(f_i,e_i,v_i);

    std::vector<int> links;
    for(int i: links_)
        if(this->edges[i].p2 != -1)
            links.push_back(i);

    if(links.size() > 1)
    {
        //std::cout<<"branch...";
        std::vector<core::vector3df> vecs;
        for(int e_j : links)
        {
            int v_j = this->get_opposite_end(e_j,v_i);
            vecs.push_back(this->vertices[v_j].V - this->vertices[v_i].V);
        }

        core::vector3df straight_ahead = this->vertices[v_i].V - this->vertices[this->get_opposite_end(e_i,v_i)].V;

        for(int i=0; i<links.size()-1; i++)
            for(int j=i+1; j<links.size(); j++)
            {
                if(is_left_from(vecs[j],vecs[i],straight_ahead, this->faces[f_i].m_normal))
                {
                    core::vector3df tv = vecs[i];
                    vecs[i]=vecs[j];
                    vecs[j]= tv;

                    int ti = links[i];
                    links[i]=links[j];
                    links[j]=ti;
                }
            }
        if(this->edges[links[0]].p2 ==0)
        {
            //std::cout<<"going right\n";
            e_i = links[links.size()-1];
            e_loop = this->get_loop_going_right(f_i,v_i,e_i,this->get_opposite_end(e_i,v_i),graph);
            return e_loop;
        }
        else if(this->edges[links[links.size()-1]].p2 ==0)
        {
            //std::cout<<"going left\n";
            e_i = links[0];
            e_loop = this->get_loop_going_left(f_i,v_i,e_i,this->get_opposite_end(e_i,v_i),graph);
            return e_loop;
        }
        else std::cout<<"warning: loop search error\n";
    }

}
*/
/*
int polyfold::cw_test(int f_i)
{
    core::vector3df up = core::vector3df(1,2,3);
    core::vector3df left = core::vector3df(-1,0,0);
    core::vector3df right = core::vector3df(0,0,1);

    core::matrix4 R = this->faces[f_i].get2Dmat();
    R.rotateVect(up);
    R.rotateVect(left);
    R.rotateVect(right);

    std::cout<<up.X<<","<<up.Y<<","<<up.Z<<"\n";
    return 1;
}
*/

/*
bool polyfold::is_duplicate_loop(int f_i,poly_loop loop)
{
    for(poly_loop p : this->faces[f_i].loops)
    {
        if(p.vertices.size() == loop.vertices.size())
        {
            bool bDuplicate = true;
            for(int v_i : p.vertices)
            {
                bool b = false;
                for(int v_j : loop.vertices)
                {
                    if(v_i==v_j) b= true;
                }
                if(!b)
                    bDuplicate = false;
            }
            if(bDuplicate) return true;
        }
    }
    return false;
}
*/


/*
int polyfold::classify_edge2(int edge)
{
    std::vector<int> sides;

    for(int f_i=0; f_i<this->faces.size(); f_i++)
        {
            for(int e_i : this->faces[f_i].edges)
            {
                if(e_i == edge)
                {
                    sides.push_back(f_i);
                    std::cout<<f_i<<"\n";
                }
            }
        }
    if(sides.size() ==2)
    {
        core::vector3df cp = this->faces[sides[0]].m_normal.crossProduct(this->faces[sides[1]].m_normal);
        core::matrix4 R;
        cp.normalize();
        R.buildRotateFromTo(cp,core::vector3df(0,1,0));

        core::vector3df m1;
        core::vector3df m2;
        core::vector3df r1;
        core::vector3df r2;

        LineHolder nograph;

        R.rotateVect(m1, this->faces[sides[0]].m_normal);
        R.rotateVect(m2, this->faces[sides[1]].m_normal);
        //R.rotateVect(r1, this->faces[sides[0]].m_center);
        //R.rotateVect(r2, this->faces[sides[1]].m_center);
        std::cout<<"go:\n";
        core::vector3df fp0,fp1;

        bool res0 = this->get_facial_point2(sides[0],edge, fp0, nograph);
        if(!res0)
        {
            std::cout<<"undef\n";
            return TOP_UNDEF;
        }

        std::cout<<"....\n";

        bool res1 = this->get_facial_point2(sides[1],edge, fp1, nograph);
        if(!res1)
        {
            std::cout<<"undef\n";
            return TOP_UNDEF;
        }

        std::cout<<"ok\n";

        R.rotateVect(r1, fp0);
        R.rotateVect(r2, fp1);
        m1.normalize();
        m2.normalize();

        //m1*=25;
        //m2*=25;

      //  std::cout<<r1.X<<","<<r1.Z<<"\n";
      //  std::cout<<r2.X<<","<<r2.Z<<"\n";

        //graph.points.push_back(core::vector3df(r1.X,0,r1.Z));
        //graph.points.push_back(core::vector3df(r2.X,0,r2.Z));

        core::line2df line1 = core::line2df(r1.X,r1.Z,r1.X+m1.X,r1.Z+m1.Z);
        core::line2df line2 = core::line2df(r2.X,r2.Z,r2.X+m2.X,r2.Z+m2.Z);

        core::vector2df ipoint;
        //graph.lines.push_back(core::line3df(core::vector3df(line1.start.X,0,line1.start.Y),core::vector3df(line1.end.X,0,line1.end.Y)));
        //graph.lines.push_back(core::line3df(core::vector3df(line2.start.X,0,line2.start.Y),core::vector3df(line2.end.X,0,line2.end.Y)));

        std::cout<<"returning...\n";
        if(line1.intersectWith(line2,ipoint,false))
        {
            //graph.points.push_back(core::vector3df(ipoint.X,0,ipoint.Y));

            if(   core::vector2df(m1.X,m1.Z).dotProduct(ipoint-core::vector2df(r1.X,r1.Z)) < 0
               && core::vector2df(m2.X,m2.Z).dotProduct(ipoint-core::vector2df(r2.X,r2.Z)) < 0)
                return TOP_CONVEX;
            else return TOP_CONCAVE;
        }
        else return TOP_UNDEF;

    }
    return TOP_UNDEF;
}

*/


/*
bool polyfold::get_facial_point6(int face_i, int edge_i, core::vector3df & ret, LineHolder &graph)
{
    core::matrix4 R = this->faces[face_i].get2Dmat();

    polyfold pf;
    poly_face f;

    std::cout<<"face: "<<face_i<<" "<<"edge: "<<edge_i<<"\n";

    for(int p_i =0; p_i<this->faces[face_i].loops.size(); p_i++)
    {
        poly_loop loop;
        for(int v_i : this->faces[face_i].loops[p_i].vertices)
        {
            core::vector3df r = this->vertices[v_i].V;
            R.rotateVect(r);
            graph.points.push_back(r);
            int new_v = pf.get_point_or_add(r);
            f.addVertex(new_v);
            loop.vertices.push_back(new_v);
        }

        if(loop.vertices.size() >0)
            f.loops.push_back(loop);
    }


    for(int e_i : this->faces[face_i].edges)
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
        //graph.lines.push_back(core::line3df(v0,v1));
    }


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


    int p_i = this->get_edge_loop_no(face_i,edge_i);

    if(p_i == -1)
        std::cout<<"error\n";
    std::cout<<p_i<<", "<<f.loops.size()<<" ...\n";
    std::cout<<"searching "<<pf.edges[my_edge].v0<<","<<pf.edges[my_edge].v1<<"\n";
    {

        std::vector<int> v_list;

        v_list.push_back(f.loops[p_i].vertices[f.loops[p_i].vertices.size()-1]);
        //std::cout<<"0\n";

        for(int v_i : f.loops[p_i].vertices)
        {
            v_list.push_back(v_i);
        }
        v_list.push_back(v_list[1]);


        std::vector<int> edge_ends;
        for(int i=1; i< v_list.size()-1; i++)
        {
            if(v_list[i] == pf.edges[my_edge].v0)
                edge_ends.push_back(i);
            if(v_list[i] == pf.edges[my_edge].v1)
                edge_ends.push_back(i);
        }

        if(edge_ends.size() != 2)
        {
            std::cout<<"edge is not in any loop on face "<<face_i<<" for edge "<<edge_i<<"\n";
            return false;
        }

        for(int i : v_list)
            std::cout<<i<<" ";
        std::cout<<"\n";

        std::vector<int> res[2];

        res[0].push_back(v_list[edge_ends[0]-1]);
        res[1].push_back(v_list[edge_ends[1]+1]);

        res[0].push_back(v_list[edge_ends[0]+1]);
        res[1].push_back(v_list[edge_ends[1]-1]);

        std::cout<<"a";
        //std::cout<<edge_ends.size()<<" should be 2\n";


        std::cout<<"BANANA\n";
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

                if(vec_is_left_from(- left_r, right_r))
                    {
                        if(vec_is_left_from(r,right_r) || vec_is_right_from(r,left_r))
                        {

                            core::line2df line_1 = core::line2df(v0.X,v0.Z,v1.X,v1.Z);

                            bool b=false;

                           // graph.lines.push_back(core::line3df(v0,v1));

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
                            graph.lines.push_back(core::line3df(v0,v1));

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

       // std::cout<<"b";

        core::matrix4 R_inv = R;
        R_inv.makeInverse();
        for(int v_i : res[0])
            for(int v_j : res[1])
            {
                std::cout<<v_i<<" "<<v_j<<"\n";
                if(v_i == v_j)
                {
                core::vector3df v1 = pf.vertices[v_i].V;
                core::vector3df v0 = pf.getVertex(my_edge,0).V;
                graph.lines.push_back(core::line3df(v0,v1));

                v0 = pf.getVertex(my_edge,1).V;
                graph.lines.push_back(core::line3df(v0,v1));

                R_inv.rotateVect(v1);
                ret = v1;
                return true;
                }
            }
        //std::cout<<"c\n";

    }
    std::cout<<"could not find any point on face "<<face_i<<" for edge "<<edge_i<<"\n";
    return false;
}
*/

/*
core::vector3df polyfold::get_facial_point(int face_i, int edge_i,LineHolder &graph)
{
    core::matrix4 R = this->faces[face_i].get2Dmat();

    polyfold pf;
    poly_face f;

    for(int p_i =0; p_i<this->faces[face_i].loops.size(); p_i++)
    {
        poly_loop loop;
        for(int v_i : this->faces[face_i].loops[p_i].vertices)
        {
            core::vector3df r = this->vertices[v_i].V;
            R.rotateVect(r);
            graph.points.push_back(r);
            int new_v = pf.get_point_or_add(r);
            f.addVertex(new_v);
            loop.vertices.push_back(new_v);
        }

        if(loop.vertices.size() >0)
            f.loops.push_back(loop);
    }

    for(int e_i : this->faces[face_i].edges)
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
        //graph.lines.push_back(core::line3df(v0,v1));
    }


    pf.faces.push_back(f);

    core::vector3df ev0 = this->getVertex(edge_i,0).V;
    core::vector3df ev1 = this->getVertex(edge_i,1).V;
    R.rotateVect(ev0);
    R.rotateVect(ev1);

    int my_edge = pf.find_edge(ev0,ev1);

    if(my_edge == -1)
    {
        std::cout<<"Edge not on face: Error\n";
        return core::vector3df(0,0,0);
    }


    int p_i = this->get_edge_loop_no(face_i,edge_i);

    if(p_i == -1)
        std::cout<<"error\n";

    {

        std::vector<int> v_list;

        v_list.push_back(f.loops[p_i].vertices[f.loops[p_i].vertices.size()-1]);
        for(int v_i : f.loops[p_i].vertices)
        {
            v_list.push_back(v_i);
        }
        v_list.push_back(v_list[1]);


        std::vector<int> edge_ends;
        for(int i=1; i< v_list.size()-1; i++)
        {
            if(v_list[i] == pf.edges[my_edge].v0)
                edge_ends.push_back(i);
            if(v_list[i] == pf.edges[my_edge].v1)
                edge_ends.push_back(i);
        }

       // for(int i : v_list)
       //     std::cout<<i<<" ";
       // std::cout<<"\n";

        std::vector<int> res[2];

        res[0].push_back(v_list[edge_ends[0]-1]);
        res[1].push_back(v_list[edge_ends[1]+1]);

        res[0].push_back(v_list[edge_ends[0]+1]);
        res[1].push_back(v_list[edge_ends[1]-1]);

        //std::cout<<"a";

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

                std::vector<int> edges0 = pf.get_edges_from_point(0,v_list[my_v]);
                std::vector<int> edges1 = pf.get_edges_from_point(0,i);


                //for(int i : edges0)
                //    std::cout<<i<<" ";
                //for(int i : edges1)
                //    std::cout<<i<<" ";
                //std::cout<<"\n";

                if(vec_is_left_from(- left_r, right_r))
                    {
                        if(vec_is_left_from(r,right_r) || vec_is_right_from(r,left_r))
                        {

                            core::line2df line_1 = core::line2df(v0.X,v0.Z,v1.X,v1.Z);

                            bool b=false;

                           // graph.lines.push_back(core::line3df(v0,v1));

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
                            graph.lines.push_back(core::line3df(v0,v1));

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

        //std::cout<<"b\n";

        core::matrix4 R_inv = R;
        R_inv.makeInverse();
        for(int v_i : res[0])
            for(int v_j : res[1])
            {
                if(v_i == v_j)
                {
                core::vector3df v1 = pf.vertices[v_i].V;

                core::vector3df v0 = pf.getVertex(my_edge,0).V;
                graph.lines.push_back(core::line3df(v0,v1));

                v0 = pf.getVertex(my_edge,1).V;
                graph.lines.push_back(core::line3df(v0,v1));

                R_inv.rotateVect(v1);
                return v1;
                }
            }

    }
    std::cout<<"could not find any point on face "<<face_i<<" for edge "<<edge_i<<"\n";
    return core::vector3df(0,0,0);
}
*/

/*
int polyfold::classify_edge(int edge)
{
    return this->edges[edge].conv;


    std::vector<int> sides;

    for(int f_i=0; f_i<this->faces.size(); f_i++)
        {
            for(int e_i : this->faces[f_i].edges)
            {
                if(e_i == edge)
                {
                    sides.push_back(f_i);
                    //std::cout<<f_i<<"\n";
                }
            }
        }
    if(sides.size() ==2)
    {
        core::vector3df cp = this->faces[sides[0]].m_normal.crossProduct(this->faces[sides[1]].m_normal);
        core::matrix4 R;
        cp.normalize();
        R.buildRotateFromTo(cp,core::vector3df(0,1,0));

        core::vector3df m1;
        core::vector3df m2;
        core::vector3df r1;
        core::vector3df r2;

        LineHolder nograph;

        R.rotateVect(m1, this->faces[sides[0]].m_normal);
        R.rotateVect(m2, this->faces[sides[1]].m_normal);
        //R.rotateVect(r1, this->faces[sides[0]].m_center);
        //R.rotateVect(r2, this->faces[sides[1]].m_center);
        R.rotateVect(r1, this->get_facial_point(sides[0],edge,nograph));
        R.rotateVect(r2, this->get_facial_point(sides[1],edge,nograph));
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
        //graph.lines.push_back(core::line3df(core::vector3df(line1.start.X,0,line1.start.Y),core::vector3df(line1.end.X,0,line1.end.Y)));
        //graph.lines.push_back(core::line3df(core::vector3df(line2.start.X,0,line2.start.Y),core::vector3df(line2.end.X,0,line2.end.Y)));

        if(line1.intersectWith(line2,ipoint,false))
        {
            //graph.points.push_back(core::vector3df(ipoint.X,0,ipoint.Y));

            if(   core::vector2df(m1.X,m1.Z).dotProduct(ipoint-core::vector2df(r1.X,r1.Z)) < 0
               && core::vector2df(m2.X,m2.Z).dotProduct(ipoint-core::vector2df(r2.X,r2.Z)) < 0)
                return TOP_CONVEX;
            else return TOP_CONCAVE;
        }
        else return TOP_UNDEF;

    }
    return TOP_UNDEF;

}
*/
/*
polyfold make_poly_cube3(int height,int length,int width)
{
    polyfold pf;

    pf.vertices.push_back(poly_vert(0,0,0)); //0
    pf.vertices.push_back(poly_vert(length,0,0));
    pf.vertices.push_back(poly_vert(0,0,width));
    pf.vertices.push_back(poly_vert(length,0,width));

    pf.vertices.push_back(poly_vert(0,height,0)); //4
    pf.vertices.push_back(poly_vert(length,height,0));
    pf.vertices.push_back(poly_vert(0,height,width));
    pf.vertices.push_back(poly_vert(length,height,width));

    pf.edges.push_back(poly_edge(0,1));//0
    pf.edges.push_back(poly_edge(2,3)); //2,4,6,10
    pf.edges.push_back(poly_edge(0,2)); //3,5,7,11
    pf.edges.push_back(poly_edge(1,3));

    pf.edges.push_back(poly_edge(0,4));//4
    pf.edges.push_back(poly_edge(1,5));
    pf.edges.push_back(poly_edge(2,6));
    pf.edges.push_back(poly_edge(3,7));

    pf.edges.push_back(poly_edge(4,5));//8
    pf.edges.push_back(poly_edge(6,7));
    pf.edges.push_back(poly_edge(4,6));
    pf.edges.push_back(poly_edge(5,7));

    poly_face f;

    f.clear();
    f.addEdge(0);
    f.addEdge(1);
    f.addEdge(2);
    f.addEdge(3);
    pf.faces.push_back(f);

    f.clear();
    f.addEdge(0);
    f.addEdge(8);
    f.addEdge(4);
    f.addEdge(5);
    pf.faces.push_back(f);

    f.clear();
    f.addEdge(1);
    f.addEdge(9);
    f.addEdge(6);
    f.addEdge(7);
    pf.faces.push_back(f);

    f.clear();
    f.addEdge(2);
    f.addEdge(4);
    f.addEdge(6);
    f.addEdge(10);
    pf.faces.push_back(f);

    f.clear();
    f.addEdge(3);
    f.addEdge(5);
    f.addEdge(7);
    f.addEdge(11);
    pf.faces.push_back(f);

    f.clear();
    f.addEdge(8);
    f.addEdge(9);
    f.addEdge(10);
    f.addEdge(11);
    pf.faces.push_back(f);

    pf.calc_vertices(true);

    pf.faces[4].flip_normal();
    pf.faces[1].flip_normal();
    pf.faces[5].flip_normal();

    pf.topology = 1;

    return pf;
}
*/


/*
void polyfold::calc_vertices(bool bRecalcNormals)
    {
        std::cout<<"Calculating Vertices... ";
        for(int f_i = 0; f_i<faces.size(); f_i++)
        {
            for(int e_i : faces[f_i].edges)
            {
                edges[e_i].addFace(f_i);
            }
        }

        for(poly_face& face : this->faces)
        {
            face.loops.clear();
            std::vector<int> rules;
            calc_loops2(face, rules);

        }

        LineHolder nograph;

        bool b;
        for(poly_face &face : this->faces)
        {
            face.vertices.clear();

            for(poly_loop &loop : face.loops)
            {
                for (int v_i : loop.vertices)
                {
                face.vertices.push_back(v_i);
                }
            }
        }


        //Add Vertices to the face



         //Normals and Centers
        for(poly_face &face : this->faces)
        {

            this->calc_center(face);

            if(bRecalcNormals)
            {
                if(face.vertices.size()>2)
                {
                    core::vector3df v1 = vertices[face.vertices[0]].V - vertices[face.vertices[1]].V;
                    core::vector3df v2 = vertices[face.vertices[1]].V - vertices[face.vertices[2]].V;
                    core::vector3df cross = v1.crossProduct(v2);
                    cross.normalize();
                    face.m_normal = cross;
                }
                else std::cout<<"error... not enough vertices to calculate normal\n";
            }
        }

        //this->recalc_bbox_and_loops();

        for(poly_face& face:this->faces)
            for(poly_loop& loop : face.loops)
                {
                this->calc_loop_bbox(face,loop);
               // this->sort_loops(face);
                this->set_loop_solid(face,loop);
                }


        bbox.reset(vertices[0].V);
        for(poly_vert v : vertices)
            bbox.addInternalPoint(v.V);

        std::cout<<"... Done!\n";
        std::cout<<"polyfold has "<<this->faces.size()<<" faces\n";
        for(int i=0; i< faces.size(); i++)
        {
         if(faces[i].loops.size() != 1)
                std::cout<<"face "<<i<<" has "<<faces[i].loops.size()<<" loops\n";
        }
    }
    */


/*
void polyfold::calc_loops2(poly_face& face, std::vector<int> rules)
{
    //Assign vertices on each face to a loop
    std::vector<int> temp1;
    std::vector<int> temp2;

   // face.loops.clear();
    bool keep_trying = true;

    int cur_edge;

    int first_v;
    int cur_v;

    if(rules.size() == 0 )
        temp1=face.edges;
    else
        for(int e_i : face.edges)
            for(int rule : rules)
                if(this->edges[e_i].topo_group == rule)
                {
                    temp1.push_back(e_i);
                }
    keep_trying = true;
     //std::cout<<"search space is "<<temp1.size()<<" edges\n";

    //pruning
    while(keep_trying)
    {
        temp2.clear();
        for(int e_i : temp1)
        {
            bool left = false;
            bool right = false;
            for(int e_j : temp1)
            {
                if(e_i != e_j && (this->edges[e_i].v1==this->edges[e_j].v0 ||
                                  this->edges[e_i].v1==this->edges[e_j].v1 ))
                   left = true;
                else if(e_i != e_j && ( this->edges[e_i].v0==this->edges[e_j].v0 ||
                                        this->edges[e_i].v0==this->edges[e_j].v1 ))
                    right = true;
            }
            if(left == true && right == true)
                temp2.push_back(e_i);
        }
        if(temp1.size() == temp2.size())
            keep_trying = false;
        else
            temp1 = temp2;
    }

    //std::cout<<"search space "<<temp1.size()<<" edges\n";
    //for(int e_i : temp1)
    //    std::cout<<this->edges[e_i].v0<<","<<this->edges[e_i].v1<<" ";
   // std::cout<<"\n";

    if(temp1.size() < 3)
    {
    //    std::cout<<"not enough search space...\n";
        return;
    }
    keep_trying =true;

    while(keep_trying)
    {
        std::vector<int> edge;
        edge.push_back(temp1[0]);
        first_v = this->edges[temp1[0]].v0;
        cur_v = this->edges[temp1[0]].v1;

        std::vector<int> res = find_loop_path(temp1, first_v,cur_v,edge);

        if(res.size()==0)
            keep_trying = false;
        else
        {
            //std::cout<<"result: ";
            //for(int e_i : res)
            //    std::cout<<this->edges[e_i].v0<<","<<this->edges[e_i].v1<<" ";
            //std::cout<<"\n";

            poly_loop loop;
            loop.vertices.push_back(this->edges[res[0]].v0);
            for(int e_i : res)
            {
                if(this->edges[e_i].v0 == loop.vertices[loop.vertices.size()-1])
                    loop.vertices.push_back(this->edges[e_i].v1);
                else
                    loop.vertices.push_back(this->edges[e_i].v0);
            }
            loop.vertices.pop_back();
            face.loops.push_back(loop);

            //for(int v_i : loop.vertices)
            //    std::cout<<v_i<<" ";
            //std::cout<<"\n";

            temp2.clear();
            for(int e_i : temp1)
            {
                bool b = false;
                for(int e_j : res)
                {
                    if(e_j == e_i)
                        b = true;
                }
                if(!b)
                    temp2.push_back(e_i);
            }
            temp1 = temp2;
        }
    }
}
*/


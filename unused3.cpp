
/*
void polyfold::recalc_bbox_and_loops()
{

    for(poly_face &face : this->faces)
    {
        for(poly_loop &loop : face.loops)
            this->calc_loop_bbox(face,loop);
    }

    //order loops
    for(poly_face &face : this->faces)
        for(poly_loop &loop  : face.loops)
            this->set_loop_solid(face,loop);

    //Check for nested loops
    for(poly_face &face : this->faces)
        for(int i = 0; i<face.loops.size(); i++)
            for(int j = i+1; j<face.loops.size(); j++)
        {
            core::vector3df v0,v1;
            v0 = this->vertices[face.loops[i].vertices[0]].V;
            v1 = this->vertices[face.loops[j].vertices[0]].V;
            if(this->is_point_in_loop(&face,&face.loops[j],v0))
               {
                   face.loops[i].reverse();
               }
            else if(this->is_point_in_loop(&face,&face.loops[i],v1))
                {
                    face.loops[j].reverse();
                }
        }

    bbox.reset(vertices[0].V);
    for(poly_vert v : vertices)
        bbox.addInternalPoint(v.V);
}
*/


/*
std::vector<int> polyfold::find_loop_path(std::vector<int> search_space, int goal, int v0, std::vector<int> path_so_far)
{
    std::vector<int> links;

    for(int e_i: search_space)
    {
        bool bIsNew = true;
        for(int e_j : path_so_far)
        {
            if(e_i==e_j)
                bIsNew = false;
        }
        if(bIsNew)
        {
            if(this->edges[e_i].v1 == v0 || this->edges[e_i].v0 == v0)
                links.push_back(e_i);
        }
    }

    //std::cout<<"searching "<<v0<<", depth is "<<path_so_far.size()<<", "<<links.size()<<" paths\n";

    //if(links.size() == 0)
    //    std::cout<<"dead end..\n";

    for(int e_i : links)
    {
        int next_v;
        if(this->edges[e_i].v0 == v0)
            next_v = this->edges[e_i].v1;
        else
            next_v = this->edges[e_i].v0;


        std::vector<int> path = path_so_far;
        path.push_back(e_i);

        if(next_v == goal)
            {
            //std::cout<<"goal reached\n";
            return path;
            }
        //std::cout<<" go: ";
        std::vector<int> searchRes = find_loop_path(search_space, goal,next_v,path);

        if(searchRes.size() > 0) return searchRes;
    }

    std::vector<int> empt;
    return empt;
}
*/

/*
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
                            if(is_identical_loop(pf4, pf4.faces[f_i].loops[p_i],pf5, pf5.faces[f_j].loops[p_j])
                               && pf4.faces[f_i].loops[p_i].type==LOOP_INNER)
                           // if(is_contained_loop(pf4, pf4.faces[f_i].loops[p_i],pf5, pf5.faces[f_j], pf5.faces[f_j].loops[p_j])
                           //    && pf4.faces[f_i].loops[p_i].type==LOOP_INNER)
                            {
                                if(pf4.faces[f_i].m_normal.dotProduct(pf5.faces[f_j].m_normal) > 0)
                                {
                                   // std::cout<<"solid loop "<<f_i<<" "<<p_i<<" vs "<<f_j<<" "<<p_j<<"\n";
                                    if(pass_no == CLIP_FIRST_PASS)
                                        pf4.faces[f_i].loops[p_i].topo_group = LOOP_HOLLOW;
                                    else
                                        pf4.faces[f_i].loops[p_i].topo_group = LOOP_SOLID;
                                    num++;
                                    goto LoopHasBeenCulled;
                                }
                                else
                                {
                                   // std::cout<<"hollow loop "<<f_i<<" "<<p_i<<" vs "<<f_j<<" "<<p_j<<"\n";
                                    pf4.faces[f_i].loops[p_i].topo_group = LOOP_HOLLOW;
                                    num++;
                                    goto LoopHasBeenCulled;
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
*/

/*
if(pf4.faces[f_i].loops[p_i].vertices.size() > 0 && pf4.faces[f_i].loops[p_i].type == LOOP_INNER)
                {
                    poly_loop new_loop;
                    for(int v_i : pf4.faces[f_i].loops[p_i].vertices)
                    {
                        int v = ret.get_point_or_add(pf4.vertices[v_i].V);
                        new_loop.vertices.push_back(v);
                        f.addVertex(v);
                    }

                    std::vector<int> tempv = pf4.faces[f_i].loops[p_i].vertices;
                    tempv.push_back(tempv[0]);

                    bool exclude=false;
                    bool include=false;

                    //std::cout<<"loop "<<f_i<<": ";
                    for(int i=0; i < tempv.size()-1; i++)
                       {
                        core::vector3df v0=pf4.vertices[tempv[i]].V;
                        core::vector3df v1=pf4.vertices[tempv[i+1]].V;
                       // std::cout<<pf4.edges[pf4.get_edge_or_add(tempv[i],tempv[i+1],!rule)].topo_group<<" ";
                        if(pf4.edges[pf4.get_edge_or_add(tempv[i],tempv[i+1],!rule)].topo_group == !rule )
                        {
                            exclude = true;
                        }
                        else if(pf4.edges[pf4.get_edge_or_add(tempv[i],tempv[i+1],!rule)].topo_group == rule )
                        {
                            include = true;
                        }
                        int new_e = ret.get_edge_or_add(ret.get_point_or_add(v0),ret.get_point_or_add(v1),0);
                        f.addEdge(new_e);
                       }
                 //   std::cout<<"\n";

                    if(exclude && !include && pf4.faces[f_i].loops[p_i].topo_group == LOOP_UNDEF)
                    {
                    //    std::cout<<"hollow\n";
                        pf4.faces[f_i].loops[p_i].topo_group = LOOP_HOLLOW;
                    }
                    else if(include && !exclude && pf4.faces[f_i].loops[p_i].topo_group == LOOP_UNDEF)
                    {
                    //    std::cout<<"solid\n";
                        pf4.faces[f_i].loops[p_i].topo_group = LOOP_SOLID;
                    }

                    if((!exclude && !include && pf4.faces[f_i].loops[p_i].topo_group == LOOP_UNDEF ) ||
                      (exclude && include && pf4.faces[f_i].loops[p_i].topo_group == LOOP_UNDEF ))
                    {
                        std::cout<<"odd loop: "<<f_i<<", "<<p_i<<" ";
                        core::vector3df v0=pf4.vertices[tempv[0]].V;
                        core::vector3df v1;
                        int e_i = pf4.find_edge(tempv[0],tempv[1]);

                        if(e_i != -1 && pf4.get_facial_point3(f_i,e_i,p_i,v1,nograph))
                        {
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

                    new_loop.type = pf4.faces[f_i].loops[p_i].type;
                    new_loop.topo_group = pf4.faces[f_i].loops[p_i].topo_group;
                    ret.calc_loop_bbox(f,new_loop);
                    if(new_loop.topo_group != LOOP_UNDEF)
                    {
                        if(new_loop.topo_group == LOOP_SOLID || pass_no == CLIP_FIRST_PASS)
                        {
                            f.loops.push_back(new_loop);
                        }
                        //else if(new_loop.topo_group == LOOP_HOLLOW)
                        //    std::cout<<"added hollow loop\n";
                        //f.loops.push_back(new_loop);
                    }
                }
*/

/*
    core::vector3df N(0,1,0);

   // core::vector3df S(1,0,0);
   // core::vector3df v0(-60,0,-98);
   // core::vector3df v1(-20,0,98);

    core::vector3df S(20,0,-98);
    core::vector3df v1(-1,0,0);
    core::vector3df v0(1,0,1);

    S.normalize();
    v0.normalize();
    v1.normalize();

    std::cout<<v0.crossProduct(S).Y<<"\n";
    std::cout<<v1.crossProduct(S).Y<<"\n";
    std::cout<<v0.crossProduct(v1).Y<<"\n";

    graph2.lines.push_back(core::line3df(core::vector3df(0,0,0),S*100));
    graph2.lines.push_back(core::line3df(core::vector3df(0,0,0),N*100));
    graph.lines.push_back(core::line3df(core::vector3df(0,0,0),v0*100));
    graph3.lines.push_back(core::line3df(core::vector3df(0,0,0),v1*100));

    if(is_left_from2(v0,v1,S,N))
        std::cout<<"left\n";
   else std::cout<<"right\n";
    std::cout<<"\n\n\n";



     for(poly_edge &edge: pf.edges)
        edge.p2= 0;//edge.perimeter;
    pf.faces[5].loops.clear();

    int e_0 = pf.faces[5].edges[0];
    int vv0 = pf.edges[e_0].v1;
    int vv1 = pf.edges[e_0].v0;
    std::cout<<"N= "<<pf.faces[0].getOrientingNormal().Y<<"\n";

    std::vector<int> res = pf.get_loop_going_left(5,6,8,7,graph);
    std::cout<<"got ";
    for(int aa : res)
        std::cout<<aa<<" ";
    std::cout<<"\n";
    //core::vector3df v0 = pf.faces[0].getOrientingNormal();
    //std::cout<<v0.X<<","<<v0.Y<<","<<v0.Z<<"\n";

    //pf.calc_loops4(5,graph);
*/

//LR TEST
/*
                    poly_loop loop = pf.faces[f_i].loops[p_i];

                    //core::vector3df r = pf.vertices[5].V;
                    core::vector3df r(256,0,-496);
                    core::matrix4 R = pf.faces[f_i].get2Dmat();
                    //core::vector3df V = v;
                    R.rotateVect(r);

                    core::vector3df z(25,0,-25);
                    //r+=z;

                    if(pf.left_right_test(f_i,p_i,r.X,r.Z))
                        std::cout<<"in\n";
                    else std::cout<<"out\n";

                  graph2.points.push_back(core::vector3df(r.X,0,r.Z));

                    if(pf.left_right_test2(f_i,p_i,r.X,r.Z,graph))
                        std::cout<<"in\n";
                    else std::cout<<"out\n";

                    pf.left_right_test3(f_i,p_i,r.X,r.Z,graph);

                  if(false)
                    for(int i=-128;i<100;i+=16)
                        for(int j=-128;j<100;j+=16)
                        {
                            z=core::vector3df(i,0,j);
                            core::vector3df rr = r+z;
                            if(pf.left_right_test2(f_i,p_i,rr.X,rr.Z,nograph))
                                graph2.points.push_back(core::vector3df(rr.X,0,rr.Z));
                            else
                                graph.points.push_back(core::vector3df(rr.X,0,rr.Z));
                        }

                   // graph.points.push_back(r);
*/

//TESTING CODE FROM MAIN
/*
   for(int e_i : pf.faces[1].edges)
            {
                std::cout<<e_i<<"\n";
               // core::vector3df v0=pf.getVertex(e_i,0).V;
               // core::vector3df v1=pf.getVertex(e_i,1).V;
               // graph.lines.push_back(core::line3df(v0,v1));
            }

   for(int f_i=0; f_i < pf.faces.size(); f_i++)
        {
            if(f_i==1)
            for(int e_i : pf.faces[1].edges)
            {
              //  std::cout<<e_i<<"\n";
                core::vector3df v0=pf.getVertex(e_i,0).V;
                core::vector3df v1=pf.getVertex(e_i,1).V;
                graph.lines.push_back(core::line3df(v0,v1));
            }
        }
*/

  // std::cout<<pf.faces[f_0].m_normal.X<<","<<pf.faces[f_0].m_normal.Y<<","<<pf.faces[f_0].m_normal.Z<<"\n";
  // std::cout<<pf.faces[f_0].getOrientingNormal().X<<","<<pf.faces[f_0].getOrientingNormal().Y<<","<<pf.faces[f_0].getOrientingNormal().Z<<"\n";
   //if(false)
/*
    core::vector3df V(-15.9683,0,-253.898);
    core::vector3df z(0,0,0);
    V+=z;

    pf=pf2;

    core::matrix4 R = pf.faces[0].get2Dmat();
    //core::vector3df V = v;
    R.rotateVect(V);

    graph.points.push_back(V);

   //if(pf.is_point_in_loop2(&pf.faces[0],&pf.faces[0].loops[0],R))
  //  if(pf.is_closed_loop(pf.faces[0],pf.faces[0].loops[0]))
    if(pf.left_right_test2(&pf.faces[0],&pf.faces[0].loops[0],V.X,V.Z,graph))
        std::cout<<"OK\n";
    else std::cout<<"Uhhh\n";
*/

    //is_identical_loop(pf4, pf4.faces[f_i].loops[p_i],pf5, pf5.faces[f_j].loops[p_j])

    //pf.calc_loops4(0,nograph);
// SOME L/R TEST STUFF (from f_i/p_i loop)
/*
                    core::matrix4 R = pf.faces[f_i].get2Dmat();
                    for(int v_i : pf.faces[f_i].loops[p_i].vertices)
                    {
                    core::vector3df v = pf.vertices[v_i].V;
                    R.rotateVect(v);
                     graph.points.push_back(pf.vertices[v_i].V);
                     std::cout<<v.X<<","<<v.Y<<","<<v.Z<<"\n";
                    }
                    std::cout<<loop.min_x<<" to "<<loop.max_x<<", "<<loop.min_z<<" to "<<loop.max_z<<"\n";
                    f32 testz = (loop.max_z - loop.min_z)*0.5 + loop.min_z;
                    f32 testx = loop.min_x - 10;
                    //testz-=1;
                    std::cout<<"test point = "<<testx<<","<<testz<<"\n";
                    graph.points.push_back(core::vector3df(testx,0,testz));
                    std::cout<<"res = "<<pf.left_right_test2(&pf.faces[f_i],&loop,testx,testz,nograph);
*/
                    //graph.points.push_back(pf.vertices[0].V);
                    //graph.lines.push_back(core::line3df(pf.vertices[0].V,pf.vertices[1].V));
                    /*
                    f32 testz = (loop.max_z - loop.min_z)*0.5 + loop.min_z;
                    f32 testx = loop.min_x - 10;
                    graph.points.push_back(core::vector3d(testx,0,testz));
                    graph.lines.push_back(core::line3df(core::vector3df(loop.min_x,0,loop.max_x))
                    graph.lines.push_back(
                    */

/*

int polyfold::left_right_test3(int f_i, int p_i, f32 vx, f32 vz, LineHolder &graph)
{
    core::vector3df point;
    point.X = vx;
    point.Y=0;
    point.Z = vz;

    std::vector<core::vector3df> verts;
    int size_minus_one = this->faces[f_i].loops[p_i].vertices.size()-1;

    verts.push_back(this->vertices[this->faces[f_i].loops[p_i].vertices[size_minus_one]].V);
    for(int v_i : this->faces[f_i].loops[p_i].vertices)
    {
        verts.push_back(this->vertices[v_i].V);
    }
    verts.push_back(verts[1]);

    core::matrix4 R = this->faces[f_i].get2Dmat();
    for(core::vector3df &v : verts)
    {
        R.rotateVect(v);
        v.Y=0;
    }

    //point.Y = verts[1].Y;
    //graph.points.push_back(point);

    f32 dist=999999;
    int my_v;
    for(int i=1;i<verts.size()-1;i++)
    {
        core::line3df line(verts[i],verts[i+1]);
        core::vector3df v0 = line.getClosestPoint(point);
        graph.lines.push_back(core::line3df(verts[i],verts[i+1]));
        f32 d = point.getDistanceFrom(v0);
        if(d<dist)
        {
            dist=d;
            my_v=i;
        }
    }
    graph.points.push_back(verts[my_v]);

    core::vector3df v0      = verts[my_v];
    core::vector3df v1      = point;
    core::vector3df v_left  = verts[my_v+1];
    core::vector3df v_right = verts[my_v-1];

    core::vector3df r       = v1 - v0;
    core::vector3df left_r  = v_left - v0;
    core::vector3df right_r = v_right - v0;

    graph.points.push_back(verts[2]);

    if(vec_is_left_from(- left_r, right_r) )
       {
        if (vec_is_left_from(r,right_r) || vec_is_right_from(r,left_r))
            return 1;
        }
    else if (vec_is_left_from(r,right_r) && vec_is_right_from(r,left_r))
        {
            return 1;
        }
    else
        {
            core::line3df line1(v0,v_left);
            core::line3df line0(v0,v_right);
            if(fabs((line0.getClosestPoint(v1).getDistanceFrom(v1))<0.001) ||
                fabs(line1.getClosestPoint(v1).getDistanceFrom(v1))<0.001)
                return 1;
        }
    return 0;
}
*/

/* //THE ORIGINAL UNIMPROVED L/R test
//int polyfold::left_right_test(poly_face* face, poly_loop* loop, f32 vx, f32 vz)
int polyfold::left_right_test2(int f_i, int p_i, f32 vx, f32 vz)
{
    LineHolder nograph;
   // return left_right_test2(f_i,p_i,vx,vz,nograph);

   // std::cout<<"l/r test: "<<loop->vertices.size()<<"   "<<vx<<","<<vz<<"\n";
    core::vector3df point;
    point.X = vx;
    point.Z = vz;

    f32 closest_x=99000;
    f32 closest_z=99000;
    int status=-1; //-1 = undefined, 0 = left, 1 = right
    std::vector<core::vector3df> verts;
    for(int v_i : this->faces[f_i].loops[p_i].vertices)
    {
        verts.push_back(this->vertices[v_i].V);
    }
    verts.push_back(verts[0]);

    core::matrix4 R = this->faces[f_i].get2Dmat();
    for(core::vector3df &v : verts)
    {
        R.rotateVect(v);
    }
    for(int i = 0 ; i<verts.size()-1; i++)
    {
        core::vector3df leftv,rightv;
        if(verts[i].X < verts[i+1].X)
        {
            leftv = verts[i];
            rightv = verts[i+1];
        }
        else
        {
            leftv = verts[i+1];
            rightv = verts[i];
        }
        f32 small_number = 0.0001;
        if(point.X > leftv.X  - small_number &&
           point.X < rightv.X + small_number)
        {
            f32 m = (rightv.Z - leftv.Z)/ (rightv.X - leftv.X);
            f32 z_intersect = m*(point.X-leftv.X)+leftv.Z;

            if(verts[i].X < verts[i+1].X
               && point.Z > z_intersect - small_number
               && fabs(point.Z - z_intersect)<closest_z)
            {
                status = 0; //Left
                closest_z = fabs(point.Z - z_intersect);
            }
            else if(verts[i].X < verts[i+1].X
               && point.Z < z_intersect + small_number
               && fabs(point.Z - z_intersect)<closest_z)
            {
                status = 1; //Right
                closest_z = fabs(point.Z - z_intersect);
            }
            else if(verts[i].X > verts[i+1].X
               && point.Z > z_intersect - small_number
               && fabs(point.Z - z_intersect)<closest_z)
            {
                status = 1; //Right
                closest_z = fabs(point.Z - z_intersect);
            }
            else if(verts[i].X > verts[i+1].X
               && point.Z < z_intersect + small_number
               && fabs(point.Z - z_intersect)<closest_z)
            {
                status = 0; //Left
                closest_z = fabs(point.Z - z_intersect);
            }
        }
        if (point.Z > std::min(verts[i].Z,verts[i+1].Z) - small_number &&
           point.Z < std::max(verts[i].Z,verts[i+1].Z) + small_number &&
           fabs(rightv.Z - leftv.Z) > small_number)
        {
            f32 m = (rightv.Z - leftv.Z)/ (rightv.X - leftv.X);
            f32 x_intersect = ((point.Z-leftv.Z))/m+leftv.X;
            if(verts[i].Z < verts[i+1].Z
               && point.X < x_intersect + small_number
               && fabs(point.X - x_intersect)<closest_x)
            {
                status = 0; //Left
                closest_x = fabs(point.X - x_intersect);
            }
            else if(verts[i].Z < verts[i+1].Z
               && point.X > x_intersect - small_number
               && fabs(point.X - x_intersect)<closest_x)
            {
                status = 1; //Right
                closest_x = fabs(point.X - x_intersect);
            }
            else if(verts[i].Z > verts[i+1].Z
               && point.X < x_intersect + small_number
               && fabs(point.X - x_intersect)<closest_x)
            {
                status = 1; //Right
                closest_x = fabs(point.X - x_intersect);
            }
            else if(verts[i].Z > verts[i+1].Z
               && point.X > x_intersect - small_number
               && fabs(point.X - x_intersect)<closest_x)
            {
                status = 0; //Left
                closest_x = fabs(point.X - x_intersect);
            }
        }
    }
    //std::cout<<"closest approach "<<closest_x<<" / "<<closest_z<<"\n";
    if(closest_x < 0.1 || closest_z <0.1)
    {
        //std::cout<<"l/r test: point on line "<<closest_x<<" / "<<closest_z<<"\n";
        status = 1;
    }

    if(status == -1)
    {
        std::cout<<"warning - undefined left right test\n";
      //  std::cout<<"bounds: "<<loop->min_x<<","<<loop->max_x<<"  "<<loop->min_z<<","<<loop->max_z<<"\n";
        for(core::vector3df v : verts)
        {
            std::cout<<v.X<<","<<v.Z<<" ";
        }
        std::cout<<"("<<vx<<","<<vz<<")";
        std::cout<<"\n";
        status = 1;
    }

//    LineHolder nograph;
   int t3 = left_right_test2(f_i,p_i,vx,vz);
    if(t3 != status)
    {
    //      std::cout<<"ERR- left right mismatch\n";
    //std::cout<<"mismatch f="<<f_i<<" p="<<p_i<<" "<<vx<<","<<vz<<"\n";
    std::cout<<" using improved l/r test\n";
    return t3;
    }

   // return t3;
   return status;
}
*/



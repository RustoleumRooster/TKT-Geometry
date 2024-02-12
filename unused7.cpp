
/*
bool geometry_scene::WriteGeometryToFile(std::string fname)
{
    ofstream wf(fname,ios::out | ios::binary);

    if(!wf)
    {
        cout<<"Cannot open file\n";
        return false;
    }

    wf<<"747\n";

    std::vector<video::ITexture*> textures_used;
    std::vector<std::string> texture_paths;

    for(int i=1;i<this->elements.size();i++)
	{
        for(int f_i =0 ;f_i<this->elements[i].brush.faces.size(); f_i++)
        {
           // std::cout<<this->elements[i].brush.faces.size()<<" / "<<this->elements[i].geometry.faces.size()<<"\n";
            video::ITexture* tex_j = this->driver->getTexture(this->elements[i].brush.faces[f_i].texture_name.c_str());

            bool b=false;
            for(video::ITexture* tex_i : textures_used)
            {
                if(tex_j == tex_i)
                    b=true;
            }
            if(!b)
            {
                textures_used.push_back(tex_j);
                texture_paths.push_back(std::string(this->elements[i].brush.faces[f_i].texture_name.c_str()));
                std::cout<<this->elements[i].brush.faces[f_i].texture_name.c_str()<<"\n";
            }
        }
	}

	wf<<textures_used.size()<<"\n";

	for(int i=0;i<textures_used.size();i++)
        wf<<texture_paths[i]<<"\n";

    int e = this->elements.size();
    wf.write((char*)&e,sizeof(int));
    for(int i=0;i<this->elements.size();i++)
    {
        int type = this->elements[i].type;
        wf.write((char*)&type,sizeof(int));

        int top = this->elements[i].brush.topology;
        wf.write((char*)&top,sizeof(int));

        //Surface Groups
        int n_s = this->elements[i].brush.surface_groups.size();
        wf.write((char*)&n_s,sizeof(int));
        for(int j=0;j<n_s;j++)
        {
            wf.write((char*)&this->elements[i].brush.surface_groups[j].type,sizeof(int));
            wf.write((char*)&this->elements[i].brush.surface_groups[j].point,sizeof(core::vector3df));
            wf.write((char*)&this->elements[i].brush.surface_groups[j].vec,sizeof(core::vector3df));
            wf.write((char*)&this->elements[i].brush.surface_groups[j].vec1,sizeof(core::vector3df));
        }

        //Vertices
        int n_v = this->elements[i].brush.vertices.size();
        wf.write((char*)&n_v,sizeof(int));

        for(int j=0;j<n_v;j++)
        {
            wf.write((char*)&this->elements[i].brush.vertices[j].V,sizeof(core::vector3df));
        }

        //Faces
        int n_f = this->elements[i].brush.faces.size();
        wf.write((char*)&n_f,sizeof(int));
        for(int j=0;j<n_f;j++)
        {
            core::vector3df norm = this->elements[i].brush.faces[j].m_normal;
            bool bFlip = this->elements[i].brush.faces[j].bFlippedNormal;
            wf.write((char*)&norm,sizeof(core::vector3df));
            wf.write((char*)&bFlip,sizeof(bool));
            wf.write((char*)&this->elements[i].brush.faces[j].surface_group,sizeof(int));
            wf.write((char*)&this->elements[i].brush.faces[j].material_group,sizeof(int));

            int tex_num=0;
            video::ITexture* tex_j = this->driver->getTexture(this->elements[i].brush.faces[j].texture_name.c_str());
            if(tex_j)
            {
                for(int ii=0;ii<textures_used.size();ii++)
                    if(tex_j == textures_used[ii])
                        tex_num=ii;
            }
            wf.write((char*)&tex_num,sizeof(int));

            //Loops
            int n_p = this->elements[i].brush.faces[j].loops.size();
            wf.write((char*)&n_p,sizeof(int));
            for(int y=0;y<n_p;y++)
            {
                int tg = this->elements[i].brush.faces[j].loops[y].topo_group;
                wf.write((char*)&tg,sizeof(int));

                int n_vv = this->elements[i].brush.faces[j].loops[y].vertices.size();
                wf.write((char*)&n_vv,sizeof(int));
                for(int vv=0;vv<n_vv;vv++)
                {
                    wf.write((char*)&this->elements[i].brush.faces[j].loops[y].vertices[vv],sizeof(int));
                }
            }
        }
    }

    wf.close();
    if(!wf.good())
    {
        cout<<"error writing file\n";
        return false;
    }
    return true;
}

bool geometry_scene::ReadGeometryFromFile(io::path fname)
{

    ifstream rf(fname.c_str(),ios::in | ios::binary);

    if(!rf)
    {
        cout<<"Cannot open file\n";
        return false;
    }
    std::string line;

    int magic;
    getline(rf,line);
    magic = core::strtoul10(line.c_str());

    if(magic != 747)
    {
        std::cout<<fname.c_str()<<" is not a valid poly file\n";
        rf.close();
        return false;
    }

    int n_textures;
    getline(rf,line);
    n_textures = core::strtoul10(line.c_str());

    std::cout<<n_textures<<" textures used:\n";

    std::vector<std::string> texture_paths;

    for(int i=0;i<n_textures;i++)
    {
        getline(rf,line);
        std::cout<<line<<"\n";
        texture_paths.push_back(line.c_str());
       // else std::cout<<" ** could not load "<<line<<"\n";
    }

    int n_elements;

    rf.read((char*)&n_elements,sizeof(int));
    std::cout<<n_elements<<" elements\n";

    this->elements.clear();
    for(int m_i=0;m_i<n_elements;m_i++)
    {
        geo_element new_element;

        int type;
        rf.read((char*)&type,sizeof(int));
        new_element.type = type;

        int top;
        rf.read((char*)&top,sizeof(int));
        new_element.brush.topology = top;

        //Surface Groups
        int n_sg;
        rf.read((char*)&n_sg,sizeof(int));
        for(int j=0;j<n_sg;j++)
        {
            surface_group sg;
            rf.read((char*)&sg.type,sizeof(int));
            rf.read((char*)&sg.point,sizeof(core::vector3df));
            rf.read((char*)&sg.vec,sizeof(core::vector3df));
            rf.read((char*)&sg.vec1,sizeof(core::vector3df));

            new_element.brush.surface_groups.push_back(sg);
        }

        //Vertices
        int n_vertices;
        rf.read((char*)&n_vertices,sizeof(int));
        for(int v_i=0;v_i<n_vertices;v_i++)
        {
            core::vector3df V;
            rf.read((char*)&V,sizeof(core::vector3df));
            poly_vert vert(V.X,V.Y,V.Z);

            new_element.brush.vertices.push_back(vert);
        }

        //Faces
        int n_faces;
        rf.read((char*)&n_faces,sizeof(int));
        for(int f_i=0;f_i<n_faces;f_i++)
        {
            poly_face face;
            core::vector3df N;
            rf.read((char*)&N,sizeof(core::vector3df));
            face.m_normal=N;

            bool bFlip;
            rf.read((char*)&bFlip,sizeof(bool));
            face.bFlippedNormal=bFlip;

            rf.read((char*)&face.surface_group,sizeof(int));
            rf.read((char*)&face.material_group,sizeof(int));

            int tex_num;
            rf.read((char*)&tex_num,sizeof(int));
            if(tex_num < texture_paths.size())
                face.texture_name = texture_paths[tex_num];
            else
                face.texture_name="wall.bmp";

            //Loops
            int n_loops;
            rf.read((char*)&n_loops,sizeof(int));
            for(int p_i=0;p_i<n_loops;p_i++)
            {
                poly_loop loop;

                rf.read((char*)&loop.topo_group,sizeof(int));

                rf.read((char*)&n_vertices,sizeof(int));
                for(int v_i=0;v_i<n_vertices;v_i++)
                {
                    int v_0;
                    rf.read((char*)&v_0,sizeof(int));
                    loop.vertices.push_back(v_0);
                }
                face.loops.push_back(loop);
            }
            new_element.brush.faces.push_back(face);
        }
        this->elements.push_back(new_element);
    }

    for(int i=0;i<this->elements.size();i++)
    {

        this->elements[i].brush.reduce_edges_vertices2();
        //this->elements[i].brush.recalc_bbox_and_loops();

        this->elements[i].brush.recalc_bbox();

        for(int f_i=0;f_i<this->elements[i].brush.faces.size();f_i++)
        {
            for(int p_i=0;p_i<this->elements[i].brush.faces[f_i].loops.size();p_i++)
                this->elements[i].brush.calc_loop_bbox(f_i,p_i);
        }

        for(poly_face &f :this->elements[i].brush.faces)
        {
            this->elements[i].brush.calc_center(f);
            //f.texture_name="wall.bmp";
        }
    }


    rf.close();
    if(!rf.good())
    {
        cout<<"error reading file\n";
        return false;
    }
    return true;
}
*/


/*
void polyfold::repair_clipped_poly()
{
    LineHolder nograph;
    polyfold negative;
    polyfold positive;

    negative.vertices = this->vertices;
    positive.vertices = this->vertices;

    for(int f_i=0;f_i<faces.size();f_i++)
    {
        poly_face f;
        poly_face f_p;

        f.m_center = faces[f_i].m_center;
        f.m_normal = faces[f_i].m_normal;

        f_p.m_center = faces[f_i].m_center;
        f_p.m_normal = faces[f_i].m_normal;

        f_p.bFlippedNormal = faces[f_i].bFlippedNormal;
        f.bFlippedNormal = faces[f_i].bFlippedNormal;

        for(int p=0;p<faces[f_i].loops.size();p++)
        {
            if(faces[f_i].loops[p].topo_group == LOOP_HOLLOW ||
               faces[f_i].loops[p].topo_group == LOOP_GHOST_HOLLOW ||
               faces[f_i].loops[p].topo_group == LOOP_GHOST_SOLID)
                {
                    f.loops.push_back(faces[f_i].loops[p]);
                    faces[f_i].loops[p].vertices.clear();
                }
        }
        negative.faces.push_back(f);

        for(int p=0;p<faces[f_i].loops.size();p++)
        {
            if(faces[f_i].loops[p].topo_group == LOOP_SOLID)
                {
                    f_p.loops.push_back(faces[f_i].loops[p]);
                    faces[f_i].loops[p].vertices.clear();
                }
        }
        positive.faces.push_back(f_p);
    }

    positive.reduce_edges_vertices2();
    negative.reduce_edges_vertices2();

    for(int f_i=0;f_i<negative.faces.size();f_i++)
        negative.meld_loops(f_i);

    for(int f_i=0;f_i<positive.faces.size();f_i++)
        positive.meld_loops(f_i);

    for(int f_i=0;f_i<faces.size();f_i++)
    {
        for(int p=0;p<negative.faces[f_i].loops.size();p++)
        {
            poly_loop loop;
            for(int v_i=0;v_i<negative.faces[f_i].loops[p].vertices.size();v_i++)
            {
                core::vector3df V = negative.vertices[negative.faces[f_i].loops[p].vertices[v_i]].V;
                int v = this->get_point_or_add(V);
                loop.vertices.push_back(v);
            }
            loop.topo_group=LOOP_HOLLOW;
            this->faces[f_i].loops.push_back(loop);
        }

        for(int p=0;p<positive.faces[f_i].loops.size();p++)
        {
            poly_loop loop;
            for(int v_i=0;v_i<positive.faces[f_i].loops[p].vertices.size();v_i++)
            {
                core::vector3df V = positive.vertices[positive.faces[f_i].loops[p].vertices[v_i]].V;
                int v = this->get_point_or_add(V);
                loop.vertices.push_back(v);
            }
            loop.topo_group=LOOP_SOLID;
            this->faces[f_i].loops.push_back(loop);
        }
    }

    this->reduce_edges_vertices2();
}
*/

/*
int do_classify_point(polyfold pf, core::vector3df v0)
{
    std::vector<core::vector3df> ipoints;
    std::vector<int> face_n;

    for(int f_i=0; f_i<pf.faces.size();f_i++)
    {
        poly_face f=pf.faces[f_i];
        core::plane3df f_plane = core::plane3df(f.m_center,f.m_normal);
        core::vector3df ipoint;

        if(f_plane.getIntersectionWithLine(v0,core::vector3df(0,-1,0),ipoint))
        {
            int n=0;
            for(int p=0;p<pf.faces[f_i].loops.size();p++)
            {
                if(pf.faces[f_i].loops[p].topo_group!=LOOP_UNDEF && pf.is_point_in_loop(f_i,p,ipoint))
                   {
                        if(pf.faces[f_i].loops[p].topo_group==LOOP_SOLID)
                            n++;
                        else if(pf.faces[f_i].loops[p].topo_group==LOOP_HOLLOW)
                            n--;
                   }
            }
            if(n>0)
            {
            ipoints.push_back(ipoint);
            face_n.push_back(f_i);
            }
        }
    }
    //std::cout<<"ipoints: "<<ipoints.size()<<"\n";

    if(ipoints.size()>1)
    {
        int res;
        f32 d=99999;
        for(int i=0;i<ipoints.size();i++)
        {
            f32 r=v0.getDistanceFrom(ipoints[i]);
            if(r<d)
            {
                res = i;
                d=r;
            }
        }
        return pf.classify_point_face(face_n[res],v0);
    }
    return TOP_UNDEF;
}
*/

/*

polyfold make_result(polyfold& pf4, polyfold& pf5, polyfold pf, polyfold pf2, int rule, int rule2, clip_results& results, LineHolder &graph)
{
    LineHolder nograph;

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
                           is_parallel_normal(pf4.faces[f_i].m_normal,pf5.faces[f_j].m_normal))
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
                                                if(pf4.faces[f_i].loops[p_i].flags==0)
                                                {
                                                    f_p.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                    pf5.faces[f_j].loops[p_j].flags=1;
                                                }
                                                else
                                                    f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);

                                                goto FinishedWithLoop2;
                                            }
                                            else if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_HOLLOW && pf4.faces[f_i].loops[p_i].topo_group==LOOP_SOLID)
                                            {
                                                core::vector3df v1;
                                                int e_j = pf5.find_edge(pf5.faces[f_j].loops[p_j].vertices[0],pf5.faces[f_j].loops[p_j].vertices[1]);
                                                if(pf5.get_facial_point(f_j,e_j,p_j,v1,graph))
                                                {
                                                    if(rule==GEO_ADD)
                                                    {
                                                        if(pf2.classify_point(v1,graph) == TOP_FRONT)
                                                            f_p.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                        else
                                                            f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                    }
                                                    else
                                                    {
                                                        if(pf2.classify_point(v1,graph) == TOP_FRONT)
                                                            f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                        else
                                                            f_p.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                    }
                                                }
                                                goto FinishedWithLoop2;
                                            }
                                            else if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_SOLID && pf4.faces[f_i].loops[p_i].topo_group==LOOP_HOLLOW)
                                            {
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
                                                core::vector3df v1;
                                                int e_j = pf5.find_edge(pf5.faces[f_j].loops[p_j].vertices[0],pf5.faces[f_j].loops[p_j].vertices[1]);
                                                if(pf5.get_facial_point(f_j,e_j,p_j,v1,nograph))
                                                {
                                                    if(rule==GEO_ADD)
                                                    {
                                                        if(pf2.classify_point(v1,graph) == TOP_FRONT)
                                                            f_p.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                        else
                                                            f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                    }
                                                    else
                                                    {
                                                        if(pf2.classify_point(v1,graph) == TOP_FRONT)
                                                            f_n.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                        else
                                                            f_p.loops.push_back( pf4.faces[f_i].loops[p_i]);
                                                    }
                                                }
                                                goto FinishedWithLoop2;
                                            }
                                            else if(pf5.faces[f_j].loops[p_j].topo_group==LOOP_SOLID && pf4.faces[f_i].loops[p_i].topo_group==LOOP_HOLLOW)
                                            {
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

                    int RES = classify_loop(pf4,f_i,p_i,pf5,rule,rule2,graph);

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
        f.bFlippedNormal = pf4.faces[i].bFlippedNormal;
        f.surface_group = pf4.faces[i].surface_group;
        f.original_brush = pf4.faces[i].original_brush;
        f.original_face = pf4.faces[i].original_face;
        f.uv_mat = pf4.faces[i].uv_mat;
        f.uv_origin = pf4.faces[i].uv_origin;

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

    return result;
}
*/

/*
bool point_is_inside_angle(polyfold pf, int p_i, int vert, core::vector3df v1)
{
    int my_v;
    int left_v;
    int right_v;

    int n = pf.faces[0].loops[p_i].vertices.size()-1;
    for(int i=0;i<pf.faces[0].loops[p_i].vertices.size();i++)
    {
        if(pf.faces[0].loops[p_i].vertices[i]==vert)
        {
            my_v=pf.faces[0].loops[p_i].vertices[i];
            if(i==0)
            {
                left_v = pf.faces[0].loops[p_i].vertices[1];
                right_v = pf.faces[0].loops[p_i].vertices[n];
            }
            else if(i==n)
            {
                left_v = pf.faces[0].loops[p_i].vertices[0];
                right_v = pf.faces[0].loops[p_i].vertices[n-1];
            }
            else
            {
                left_v = pf.faces[0].loops[p_i].vertices[i+1];
                right_v = pf.faces[0].loops[p_i].vertices[i-1];
            }
        }
    }

    core::vector3df v0      = pf.vertices[my_v].V;
    core::vector3df v_left  = pf.vertices[left_v].V;
    core::vector3df v_right = pf.vertices[right_v].V;

    core::vector3df r       = v1 - v0;
    core::vector3df left_r  = v_left - v0;
    core::vector3df right_r = v_right - v0;

     if(vec_is_left_from(- left_r, right_r))
            {
                if (vec_is_left_from(r,right_r) || vec_is_right_from(r,left_r))
                {
                    return true;
                }
            }
    else
        if (vec_is_left_from(r,right_r) && vec_is_right_from(r,left_r))
            return true;

    return false;
}
*/
/* 
class NodePropertiesEditArea : public irr::gui::IGUIElement
{
public:
    NodePropertiesEditArea(gui::IGUIEnvironment* env, gui::IGUIElement* parent,NodeProperties_Base*,s32 id,core::rect<s32> rect);
    ~NodePropertiesEditArea();

    virtual void draw();

    static std::vector<reflect::TypeDescriptor_Struct*> GetTypeDescriptors(geometry_scene*);

    void show(std::vector<reflect::TypeDescriptor_Struct*> typeDescriptors, void* obj);
    void refresh();
    virtual void click_OK();
    virtual bool OnEvent(const SEvent& event);
    virtual void write(void* obj);

    int scroll_pos;
    int scroll_offset=0;
    int old_forms_height=0;
    void setEditAreaScrollPos(int pos);

    int getMaxFormsHeight();
    int getFormsHeight();
    int getEditAreaHeight();
    FormField* getFieldFromId(int id);
    int getFormFromId(int id);

    int OK_BUTTON_ID=0;

    std::vector<Reflected_GUI_Edit_Form*> forms;
    std::vector<reflect::TypeDescriptor_Struct*> my_typeDescriptors;
    std::vector<int> expand_button_Ids;
    std::vector<bool> readwrite;

    gui::IGUIElement* edit_panel=NULL;
    gui::IGUIScrollBar* my_scrollbar=NULL;
    geometry_scene* g_scene=NULL;
    NodeProperties_Base* my_base;

    void* temp_object=NULL;
};


std::vector<int> getLeftRightVertices(polyfold pf, int p_i, int vert)
{
    std::vector<int> v_list;
    std::vector<int> ret;

    v_list.push_back(pf.faces[0].loops[p_i].vertices[pf.faces[0].loops[p_i].vertices.size()-1]);
    for(int v_i : pf.faces[0].loops[p_i].vertices)
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

    if(my_v != -1)
    {
        int v_left  = v_list[my_v+1];
        int v_right = v_list[my_v-1];

        ret.push_back(v_left);
        ret.push_back(v_right);
    }

    return ret;
}
*/


/*
void NodePropertiesEditArea::show(std::vector<reflect::TypeDescriptor_Struct*> typeDescriptors, void* obj)
{
    int NEXT_ID = GUI_ID_REFLECTED_BASE;
    int ypos = 4;
    int x_offset = 16;

    core::rect<s32> pr(0, 0, getRelativePosition().getWidth(), getMaxFormsHeight());
    edit_panel = new gui::IGUIElement(gui::EGUIET_ELEMENT, Environment, this, -1, pr);

    for (reflect::TypeDescriptor_Struct* typeDesc : typeDescriptors)
    {
        // if(typeDesc->members.size()==0)
        //     continue;

        std::wstring txt0 = typeDesc->expanded ? L"-" : L"+";

        Environment->addButton(core::rect<s32>(core::vector2di(24, ypos), core::vector2di(24 + 16, ypos + 16)),
            edit_panel, NEXT_ID, txt0.c_str());

        expand_button_Ids.push_back(NEXT_ID);

        NEXT_ID++;

        std::wstring txt(typeDesc->alias, typeDesc->alias + strlen(typeDesc->alias));
        Environment->addStaticText(txt.c_str(), core::rect<s32>(core::vector2di(x_offset + 26, ypos), core::dimension2du(120, ypos + 28)), false, false, edit_panel, -1);

        ypos += 24;

        Reflected_GUI_Edit_Form* form = new Reflected_GUI_Edit_Form(Environment, edit_panel, g_scene, NEXT_ID,
            core::rect<s32>(core::vector2di(x_offset + 16, ypos), core::dimension2du(96, ypos + 100)));

        //form->column_left_begin = 0;//28;
        //form->column_left_end = 160;
        //form->column_middle_begin = 160;
        //form->column_middle_end = 160;
        //form->column_right_begin = 150;
        ////form->column_right_end = 220;

        //form->column_right_end = getRelativePosition().getWidth()-x_offset-32;

        NEXT_ID++;

        typeDesc->addFormWidget(form, NULL, NULL, std::vector<int>{}, 0, typeDesc->expanded, true, 0);

        form->calculateSize();

        NEXT_ID = form->ShowWidgets(NEXT_ID);

        ypos += form->getTotalHeight();

        forms.push_back(form);
        bringToFront(form);
    }

    f32 lr = (f32)getEditAreaHeight() / (f32)getFormsHeight();

    if (lr < 1.0)
    {
        my_scrollbar = new CGUIScrollBar2(false, Environment, edit_panel, GUI_ID_TEXTURE_SCROLL_BAR, core::rect<s32>(0, 0, 16, getEditAreaHeight()));
        my_scrollbar->drop();

        my_scrollbar->setMax(getFormsHeight());
        my_scrollbar->setSmallStep(24);

        int old_offset = scroll_offset;

        my_scrollbar->setPos(scroll_offset);

        scroll_offset = 0;
        setEditAreaScrollPos(old_offset);
    }
    else
    {
        my_scrollbar = NULL;
    }

    if (typeDescriptors.size() > 0)
    {
        int n = typeDescriptors.size() - 1;
        std::wstring txt(typeDescriptors[n]->alias, typeDescriptors[n]->alias + strlen(typeDescriptors[n]->alias));
        txt += std::wstring(L" properties");
        my_base->setName(txt);
    }
    else
        my_base->setName(std::wstring(L"Properties"));

    OK_BUTTON_ID = NEXT_ID;

    ypos += 40;

    core::rect<s32> button_r(core::vector2di(x_offset + 160, getEditAreaHeight() + 4),
        core::vector2di(edit_panel->getRelativePosition().LowerRightCorner.X, getEditAreaHeight() + 38));
    //Environment->addButton(),this,OK_BUTTON_ID,L"Apply");

    Flat_Button* ok_button = new Flat_Button(Environment, this, OK_BUTTON_ID, button_r);
    ok_button->setText(L"Apply");
    ok_button->drop();

    my_typeDescriptors = typeDescriptors;

    //================Initial Pass
    if (typeDescriptors.size() > 0 && temp_object == NULL)
    {
        readwrite.clear();

        Reflected_SceneNode* node_0 = g_scene->getSelectedSceneNode(0);

        for (int n_i : g_scene->getSelectedNodes())
        {
            g_scene->getSceneNodes()[n_i]->preEdit();
        }

        if (g_scene->getSelectedNodes().size() > 1)
        {
            for (int i = 0; i < typeDescriptors.size(); i++)
            {
                Reflected_GUI_Edit_Form* form = forms[i];

                FormField* f = form->edit_fields;
                while (f)
                {
                    bool b = true;

                    for (int n_i : g_scene->getSelectedNodes())
                    {
                        Reflected_SceneNode* node = g_scene->getSceneNodes()[n_i];
                        if (f->is_equal(node, node_0) == false)
                        {
                            b = false;
                        }
                    }

                    readwrite.push_back(b);

                    f = f->next;
                }
            }
        }
        else
        {
            for (int i = 0; i < typeDescriptors.size(); i++)
            {
                Reflected_GUI_Edit_Form* form = forms[i];

                FormField* f = form->edit_fields;
                while (f)
                {
                    readwrite.push_back(true);
                    f = f->next;
                }
            }
        }

        //if(obj == NULL)
        {
            temp_object = malloc(typeDescriptors[typeDescriptors.size() - 1]->size);

            for (reflect::TypeDescriptor_Struct* typeDesc : typeDescriptors)
            {
                typeDesc->copy(temp_object, node_0);
            }
        }
    }

    int f_c = 0;
    for (int i = 0; i < typeDescriptors.size(); i++)
    {
        Reflected_GUI_Edit_Form* form = forms[i];

        FormField* f = form->edit_fields;
        while (f)
        {
            f->bWrite = readwrite[f_c];

            f_c++;
            f = f->next;
        }
    }
    
    for (int i = 0; i < forms.size(); i++)
    {
        Reflected_GUI_Edit_Form* form = forms[i];

        FormField* f = form->edit_fields;
        while (f)
        {
            if (f->bWrite)
            {
                f->setActive(true);
                f->readValue(temp_object);
            }
            else
            {
                f->setActive(false);
            }
            f = f->next;
        }
    }
}

int NodePropertiesEditArea::getMaxFormsHeight()
{
    return getRelativePosition().getHeight() - 40;
    //return 400;
}

int NodePropertiesEditArea::getEditAreaHeight()
{
    return std::min(getFormsHeight(), getMaxFormsHeight());
}

void NodePropertiesEditArea::refresh()
{
    core::list<gui::IGUIElement*> child_list = this->getChildren();
    core::list<gui::IGUIElement*>::Iterator it = child_list.begin();

    for (; it != child_list.end(); ++it)
    {
        //        if(*it != this->getCloseButton())
        this->removeChild(*it);
    }

    if (this->temp_object)
    {
        write(this->temp_object);
    }

    for (Reflected_GUI_Edit_Form* form : forms)
    {
        // form->remove();
        form->drop();
    }

    forms.clear();
    expand_button_Ids.clear();

    //IGUIElement* ok_button = getElementFromId(OK_BUTTON_ID);

    //if(ok_button)
    //    ok_button->remove();
    std::vector<reflect::TypeDescriptor_Struct*> common_types = NodePropertiesEditArea::GetTypeDescriptors(g_scene);
    show(common_types, this->temp_object);
}

int NodePropertiesEditArea::getFormsHeight()
{
    int y = forms.size() * 24 + 8;

    for (Reflected_GUI_Edit_Form* f : forms)
        y += f->getTotalHeight();

    return y;
}

void NodePropertiesEditArea::draw()
{
    core::rect<s32> r = getAbsolutePosition();
    r.LowerRightCorner.Y = r.UpperLeftCorner.Y + getEditAreaHeight();
    gui::IGUISkin* skin = Environment->getSkin();

    r.LowerRightCorner.X -= 1;
    skin->draw3DSunkenPane(this, skin->getColor(EGDC_3D_FACE), true, true, r);

    gui::IGUIElement::draw();
}


NodePropertiesEditArea::NodePropertiesEditArea(gui::IGUIEnvironment* env, gui::IGUIElement* parent, NodeProperties_Base* base_, s32 id, core::rect<s32> rect)
    : gui::IGUIElement(gui::EGUIET_ELEMENT, env, parent, id, rect), my_base(base_)
{
    MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
    receiver->Register(this);
}

NodePropertiesEditArea::~NodePropertiesEditArea()
{
    std::cout << "Out of scope (Node Properties)\n";

    MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
    receiver->UnRegister(this);

    for (Reflected_GUI_Edit_Form* form : forms)
    {
        if (form)
            form->drop();
    }

    if (temp_object)
    {
        free(temp_object);
    }
}

void NodePropertiesEditArea::click_OK()
{
    for (int n_i : g_scene->getSelectedNodes())
    {

        Reflected_SceneNode* node = g_scene->getSceneNodes()[n_i];

        for (int i = 0; i < forms.size(); i++)
        {
            reflect::TypeDescriptor_Struct* typeDesc = my_typeDescriptors[i];
            Reflected_GUI_Edit_Form* form = forms[i];

            FormField* f = form->edit_fields;
            while (f)
            {
                //reflect::Member* M = typeDesc->getTreeNode(f->tree_pos);
                if (f->bWrite)
                    f->copy(node, temp_object);
                f = f->next;
            }
        }

        g_scene->getSceneNodes()[n_i]->postEdit();
    }

    //this->remove();
}

void NodePropertiesEditArea::write(void* obj)
{
    for (Reflected_GUI_Edit_Form* form : forms)
    {
        if (form && obj)
            form->write(obj);
    }
}

FormField* NodePropertiesEditArea::getFieldFromId(int id)
{
    for (Reflected_GUI_Edit_Form* form : forms)
    {
        FormField* ff = form->getFieldFromId(id);
        if (ff)
            return ff;
    }
    return NULL;
}


int NodePropertiesEditArea::getFormFromId(int id)
{
    for (int i = 0; i < forms.size(); i++)
    {
        FormField* ff = forms[i]->getFieldFromId(id);
        if (ff)
            return i;
    }
    return -1;
}

void NodePropertiesEditArea::setEditAreaScrollPos(int pos)
{
    core::list<gui::IGUIElement*> child_list = edit_panel->getChildren();
    core::list<gui::IGUIElement*>::Iterator it = child_list.begin();

    int move_dist = scroll_offset - pos;
    scroll_offset -= move_dist;

    for (; it != child_list.end(); ++it)
    {
        if (*it != my_scrollbar)
        {
            (*it)->move(core::vector2di(0, (move_dist)));
        }
    }
}

bool NodePropertiesEditArea::OnEvent(const SEvent& event)
{
    if (event.EventType == EET_GUI_EVENT)
    {
        s32 id = event.GUIEvent.Caller->getID();

        switch (event.GUIEvent.EventType)
        {
        case EGET_SCROLL_BAR_CHANGED:
        {
            if (id == GUI_ID_TEXTURE_SCROLL_BAR)
            {
                setEditAreaScrollPos(my_scrollbar->getPos());

                return true;
            }
        } break;
        case EGET_ELEMENT_FOCUS_LOST:
        {
            //std::cout<<Environment->getFocus()<<"\n";
            //if(event.GUIEvent.Caller->getType() == gui::EGUIET_EDIT_BOX)
            //{
            //    refresh();
            //    return false;
            //}

            return false;
        } break;
        case EGET_ELEMENT_FOCUSED:
        {
            if (event.GUIEvent.Element &&
                (event.GUIEvent.Element->getType() == gui::EGUIET_EDIT_BOX))// ||
                // (event.GUIEvent.Element->getType() == gui::EGUIET_COMBO_BOX)    )
            {
                s32 lost_focus_id = event.GUIEvent.Element->getID();
                int form_i = getFormFromId(lost_focus_id);


                //FormField* ff = forms[form_i]->edit_fields;
                //while(ff)
                //{
                // for(int i=0;i<ff->tree_pos.size();i++)
                //    std::cout<<ff->tree_pos[i]<<" ";
                // std::cout<<"\n";
                // ff = ff->next;
                //}

                FormField* field = getFieldFromId(lost_focus_id);
                //std::cout<<"---\n";
                if (field && field->bWrite)
                {
                    while (field)
                    {
                        field->readValue(temp_object);
                        //for(int i=0;i<field->tree_pos.size();i++)
                        //    std::cout<<field->tree_pos[i]<<" ";
                        //std::cout<<"\n";
                        field = forms[form_i]->getParentField(field);
                    }
                }
            }
            return false;

        } break;
        case EGET_BUTTON_CLICKED:
        {
            if (id == OK_BUTTON_ID)
            {
                click_OK();
                return true;
            }
            else
            {
                for (int i = 0; i < forms.size(); i++)
                {
                    if (id == expand_button_Ids[i])
                    {
                        my_typeDescriptors[i]->expanded = !my_typeDescriptors[i]->expanded;
                        refresh();
                        return true;
                    }
                    else
                    {
                        FormField* field = forms[i]->getFieldFromId(id);
                        if (field)
                        {
                            if (field->getButtonType() == FORM_FIELD_EXPAND_BUTTON)
                            {
                                ((ExButton_FormField*)field)->toggle(my_typeDescriptors[i]);
                                refresh();
                            }
                            else if (field->getButtonType() == FORM_FIELD_BUTTON)
                            {
                                field->clickButton();
                                field->bWrite = true;
                            }
                            return true;
                        }
                    }
                }
            }
            break;
        } break;
        case EGET_COMBO_BOX_CHANGED:
        case EGET_CHECKBOX_CHANGED:
        case EGET_EDITBOX_CHANGED:
        {
            FormField* field = getFieldFromId(id);
            if (field)
            {
                field->setActive(true);
                field->bWrite = true;

                int f_c = 0;
                for (int i = 0; i < forms.size(); i++)
                {
                    Reflected_GUI_Edit_Form* form = forms[i];

                    FormField* f = form->edit_fields;
                    while (f)
                    {
                        if (f == field)
                            readwrite[f_c] = true;
                        f_c++;
                        f = f->next;
                    }
                }

                field->writeValue(temp_object);
            }
        } break;
        }
    }
    else if (event.EventType == EET_MOUSE_INPUT_EVENT)
    {
        switch (event.MouseInput.Event)
        {
        case EMIE_LMOUSE_PRESSED_DOWN:
            std::cout << "click\n";
            break;
        }
    }
    else if (event.EventType == irr::EET_USER_EVENT)
    {
        switch (event.UserEvent.UserData1)
        {
        case USER_EVENT_SELECTION_CHANGED:
        {
            //std::cout<<"Node Properties: selection changed\n";
            if (temp_object)
            {
                free(temp_object);
                temp_object = NULL;
            }

            refresh();
            return true;
        }
        }
    }

    return false;
    //gui::IGUIElement::OnEvent(event);
}
*/


vector3df center_X2(const vector3df* points, vector3df v0, vector3df Up, vector3df center, f32& FOVout, LineHolder& graph)
{

    vector3df r = points[0] - v0;

    //r -= Up * (r.dotProduct(Up));

    //vector3df X = r.crossProduct(Up);
    //X.normalize();

    //vector3df Z = r;
    //Z.normalize();

    f32 tmin0 = 9;
    f32 tmax0 = -9;

    for (int i = 0; i < 4; i++)
    {
        vector3df rr = points[i] - v0;
        vector3df g;

        g = rr - (Up * (rr.dotProduct(Up)));

        f32 y = rr.dotProduct(Up);
        f32 x = g.getLength();

        f32 th = atan2f(y, x);

        tmin0 = fmin(tmin0, th);
        tmax0 = fmax(tmax0, th);
    }

    std::cout << tmin0 << " to " << tmax0 << "\n";

    FOVout = tmax0 - tmin0;

    f32 res = tmin0 + (tmax0 - tmin0) * 0.5;

    std::cout << "res is " << res << "\n";

    vector3df rr = center;
    vector3df g = rr - (Up * (rr.dotProduct(Up)));

    f32 y = rr.dotProduct(Up);
    f32 x = g.getLength();

    f32 th = atan2f(y, x);
    std::cout << "center angle is " << th << "\n";

    vector3df resv = g + Up * (g.getLength() * tan(res));

    //vector3df resv = cos(res) * Z + sin(res) * X;
    /*
    f32 ymin = 99999;
    f32 ymax = -99999;
    for (int i = 0; i < 4; i++)
    {
        f32 y = vector3df(points[i] - v0).dotProduct(Up);
        ymin = fmin(y, ymin);
        ymax = fmax(y, ymax);
    }

    std::cout << ymin << " to " << ymax << "\n";

    {
        ymin = fmin(fabs(ymin), fabs(ymax));
    }

    std::cout << ymin << " = y min\n";

    r = points[0] - v0;

    r -= Up * (r.dotProduct(Up));
    r += Up * ymin;

    X = r.crossProduct(Up);
    X.normalize();

    Z = r;
    Z.normalize();

    f32 tmin = 9;
    f32 tmax = -9;

    for (int i = 0; i < 4; i++)
        {

        vector3df g = points[i] - v0;

        g -= Up * (g.dotProduct(Up));
        g += Up * ymin;

        f32 y = g.dotProduct(X);
        f32 x = g.dotProduct(Z);

        f32 th = atan2f(y, x);

        tmin = fmin(tmin, th);
        tmax = fmax(tmax, th);
    }

    FOVout = tmax - tmin;

    std::cout << tmin << " to " << tmax << "\n";

    //
    */

    //graph.lines.push_back(line3df(v0, v0 + resv * 1000));

    return resv;
}

vector3df center_X(const vector3df* points, vector3df v0, vector3df center, vector3df Up)
{
    vector3df X = vector3df(center - v0).crossProduct(Up);
    X.normalize();

    vector3df Level = Up.crossProduct(X);
    Level.normalize();

    vector3df Y = vector3df(center - v0).crossProduct(X);
    Y.normalize();

    std::cout << "X:\n";

    f32 ah = vector3df(center - v0).dotProduct(X);
    ah /= vector3df(center - v0).getLength();

    f32 th0 = 1.57079639 - acosf(ah);

    //std::cout << th0 << "\n";

    f32 thmax = -9;
    f32 thmin = 9;

    for (int i = 0; i < 4; i++)
    {
        f32 ylen = vector3df(points[i] - v0).dotProduct(Y);
        vector3df g = vector3df(points[i] - v0) - Y * ylen;
        ah = vector3df(g).dotProduct(X);
        ah /= vector3df(g).getLength();
        f32 th = 1.5707963 - acosf(ah);
        std::cout << th << "\n";
        thmax = fmax(th, thmax);
        thmin = fmin(th, thmin);
    }
    std::cout << thmin << " to " << thmax << "\n";
    f32 th_delta = (thmax - thmin) * 0.5 + thmin;

    //std::cout << th_delta << " = delta \n";

    f32 x = cos(th_delta);
    f32 y = sin(th_delta);

    vector3df new_center = Level * x + X * y;
    new_center *= vector3df(center - v0).getLength();

    return new_center + v0;
}

f32 get_Y_FOV(const vector3df* points, vector3df v0, vector3df center, vector3df Up)
{
    vector3df X = vector3df(center - v0).crossProduct(Up);
    X.normalize();

    vector3df Level = Up.crossProduct(X);
    Level.normalize();

    f32 thmax = -9;
    f32 thmin = 9;

    for (int i = 0; i < 4; i++)
    {
        f32 xlen = vector3df(points[i] - v0).dotProduct(X);
        vector3df g = vector3df(points[i] - v0) - X * xlen;
        f32 ah = vector3df(g).dotProduct(Up);
        ah /= vector3df(g).getLength();
        f32 th = 1.5707963 - acosf(ah);
        //std::cout << th << "\n";
        thmax = fmax(th, thmax);
        thmin = fmin(th, thmin);
    }

    return thmax - thmin;
}

vector3df center_Y(const vector3df* points, vector3df v0, vector3df center, vector3df Up)
{
    vector3df X = vector3df(center - v0).crossProduct(Up);
    X.normalize();

    vector3df Level = Up.crossProduct(X);
    Level.normalize();

    vector3df Y = vector3df(center - v0).crossProduct(X);
    Y.normalize();

    std::cout << "Y:\n";

    f32 ah = vector3df(center - v0).dotProduct(Y);
    ah /= vector3df(center - v0).getLength();

    f32 th0 = 1.57079639 - acosf(ah);

    //std::cout << th0 << "\n";

    f32 thmax = -9;
    f32 thmin = 9;


    for (int i = 0; i < 4; i++)
    {
        f32 xlen = vector3df(points[i] - v0).dotProduct(X);
        vector3df g = vector3df(points[i] - v0) - X * xlen;
        ah = vector3df(g).dotProduct(Y);
        ah /= vector3df(g).getLength();
        f32 th = 1.5707963 - acosf(ah);
        std::cout << th << "\n";
        thmax = fmax(th, thmax);
        thmin = fmin(th, thmin);
    }
    std::cout << thmin << " to " << thmax << "\n";

    f32 th_delta = (thmax - thmin) * 0.5 + thmin;


    std::cout << th_delta << " new theta\n";


    f32 x = cos(th_delta);
    f32 y = sin(th_delta);


    vector3df new_center = Level * x + Y * y;
    new_center *= vector3df(center - v0).getLength();

    return new_center + v0;
    //return center;

}

/*

vector3df center_Y(const vector3df* points, vector3df v0, vector3df center, vector3df Up)
{
    vector3df X = vector3df(center - v0).crossProduct(Up);
    X.normalize();

    vector3df Level = Up.crossProduct(X);
    Level.normalize();

    //vector3df Y = vector3df(center - v0).crossProduct(X);
    //Y.normalize();

    f32 ah = vector3df(center - v0).dotProduct(Up);
    ah /= vector3df(center - v0).getLength();

    f32 th0 = 1.57079639 - acosf(ah);

    std::cout << th0 << "\n";

    f32 thmax = -9;
    f32 thmin = 9;


    for (int i = 0; i < 4; i++)
    {
        f32 xlen = vector3df(points[i] - v0).dotProduct(X);
        vector3df g = vector3df(points[i] - v0) - X * xlen;
        ah = vector3df(g).dotProduct(Up);
        ah /= vector3df(g).getLength();
        f32 th = 1.5707963 - acosf(ah);
        std::cout << th << "\n";
        thmax = fmax(th, thmax);
        thmin = fmin(th, thmin);
    }
    std::cout << thmin << " to " << thmax << "\n";

    f32 th_delta = (thmax - thmin) * 0.5 + thmin;


    std::cout << th_delta << " new theta\n";


    f32 x = cos(th_delta);
    f32 y = sin(th_delta);


    vector3df new_center = Level * x + Up * y;
    new_center *= vector3df(center - v0).getLength();

    return new_center+v0;

}
*/
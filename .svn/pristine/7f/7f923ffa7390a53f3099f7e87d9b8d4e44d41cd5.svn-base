/*
std::vector<int> polyfold::get_loop_going_right2(int f_i, int v_0, int e_0, int v_1, LineHolder& graph)
{
    std::vector<int> e_loop;
    e_loop.push_back(e_0);
    //graph.lines.push_back(core::line3df(this->getVertex(e_0,0).V,this->getVertex(e_0,1).V));
    int v_i = v_1;
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
                //graph.lines.push_back(core::line3df(this->getVertex(e_i,0).V,this->getVertex(e_i,1).V));
                if(v_i == v_0)
                    return e_loop;
            }
            else if(links.size() > 1)
            {
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
                        if(is_left_from2(vecs[j],vecs[i],straight_ahead, this->faces[f_i].getOrientingNormal()))
                        {
                            core::vector3df tv = vecs[i];
                            vecs[i]=vecs[j];
                            vecs[j]= tv;

                            int ti = links[i];
                            links[i]=links[j];
                            links[j]=ti;
                           // std::cout<<"x\n";
                        }
                    }
                //std::cout<<"S "<<straight_ahead.X<<" , "<<straight_ahead.Z<<"\n";
                //std::cout<<"choices: ";
                for(int i=0; i<links.size(); i++)
                {
                    //std::cout<<links[i]<<", ";
                    //graph.lines.push_back(core::line3df(this->getVertex(links[i],0).V,this->getVertex(links[i],1).V));
                    //std::cout<<vecs[i].X<<","<<vecs[i].Z<<"  ";
                }
                //std::cout<<"\n";

                e_i = links[links.size()-1];

                v_i = this->get_opposite_end(e_i,v_i);
                e_loop.push_back(e_i);
                graph.lines.push_back(core::line3df(this->getVertex(e_i,0).V,this->getVertex(e_i,1).V));
                if(v_i == v_0)
                    return e_loop;
            }
            else
            {
               std::cout<<"warning: search space size zero, could not find loop f="<<f_i<<"\n";
               return e_loop;
            }
        };
}
*/

/*
std::vector<int> polyfold::get_loop_going_left2(int f_i, int v_0, int e_0, int v_1,LineHolder& graph)
{
    std::vector<int> e_loop;
    e_loop.push_back(e_0);
    //graph.lines.push_back(core::line3df(this->getVertex(e_0,0).V,this->getVertex(e_0,1).V));
    int v_i = v_1;
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
                //graph.lines.push_back(core::line3df(this->getVertex(e_i,0).V,this->getVertex(e_i,1).V));
                //std::cout<<v_i<<"\n";
                if(v_i == v_0)
                {
                    //for(int i=0;i<e_loop.size();i++)
                    //    std::cout<<e_loop[i]<<",";
                    //std::cout<<"\n";
                    return e_loop;
                }
            }
            else if(links.size()>1)
            {
                std::vector<core::vector3df> vecs;
                for(int e_j : links)
                {
                    int v_j = this->get_opposite_end(e_j,v_i);
                    vecs.push_back(this->vertices[v_j].V - this->vertices[v_i].V);
                }

                graph.lines.push_back(core::line3df(this->getVertex(e_i,0).V,this->getVertex(e_i,1).V));

                core::vector3df straight_ahead = this->vertices[v_i].V - this->vertices[this->get_opposite_end(e_i,v_i)].V;

                //std::cout<<". "<<v_i<<"\n";
                for(int i=0; i<links.size()-1; i++)
                    for(int j=i+1; j<links.size(); j++)
                    {
                        if(is_left_from2(vecs[j],vecs[i],straight_ahead, this->faces[f_i].getOrientingNormal()))
                        {
                            //std::cout<<"left\n";
                            core::vector3df tv = vecs[i];
                            vecs[i]=vecs[j];
                            vecs[j]= tv;

                            int ti = links[i];
                            links[i]=links[j];
                            links[j]=ti;
                            //std::cout<<"x\n";
                        }
                        //else
                    }
                //std::cout<<

               // std::cout<<"S "<<straight_ahead.X<<" , "<<straight_ahead.Z<<"\n";
                //std::cout<<"choices: ";
                //for(int i=0; i<links.size(); i++)
                //{
                //    std::cout<<links[i]<<", ";
                //    graph.lines.push_back(core::line3df(this->getVertex(links[i],0).V,this->getVertex(links[i],1).V));
                //    std::cout<<vecs[i].X<<","<<vecs[i].Z<<"  ";
                //}
                //std::cout<<"\n";
                e_i = links[0];
                v_i = this->get_opposite_end(e_i,v_i);
                //std::cout<<"going to "<<v_i<<"\n";
                e_loop.push_back(e_i);
                //graph.lines.push_back(core::line3df(this->getVertex(e_i,0).V,this->getVertex(e_i,1).V));
                if(v_i == v_0)
                {
                   // for(int i=0;i<e_loop.size();i++)
                   //     std::cout<<e_loop[i]<<",";
                   // std::cout<<"\n";
                    return e_loop;
                }
            }
            else
            {
               std::cout<<"warning: search space size zero, could not find loop f="<<f_i<<"\n";
               return e_loop;
            }
        };
}
*/
/*
class MakeCylinderWindow : public gui::CGUIWindow
{
public:
    MakeCylinderWindow(gui::IGUIEnvironment* env, gui::IGUIElement* parent,s32 id, core::rect<s32> rect)
    : gui::CGUIWindow(env,parent,id,rect)
    {
        this->setText(L"Make Cylinder\n");
        env->addEditBox(L"128",core::rect<s32>(core::vector2di(48,32),core::vector2di(96,32+24)),true,this,GUI_ID_MAKE_CYLINDER_EDITBOX_HEIGHT);
        env->addEditBox(L"64",core::rect<s32>(core::vector2di(48,64),core::vector2di(96,64+24)),true,this,GUI_ID_MAKE_CYLINDER_EDITBOX_RADIUS);
        env->addEditBox(L"16",core::rect<s32>(core::vector2di(48,96),core::vector2di(96,96+24)),true,this,GUI_ID_MAKE_CYLINDER_EDITBOX_SIDES);
        env->addStaticText(L"Height",core::rect<s32>(core::vector2di(8,36),core::vector2di(48,36+24)),false,false,this,-1);
        env->addStaticText(L"Radius",core::rect<s32>(core::vector2di(8,68),core::vector2di(48,68+24)),false,false,this,-1);
        env->addStaticText(L"# Sides",core::rect<s32>(core::vector2di(8,100),core::vector2di(48,100+24)),false,false,this,-1);
        env->addButton(core::rect<s32>(core::vector2di(64,140),core::vector2di(128,140+32)),this,GUI_ID_MAKE_CYLINDER_BUTTON_OK,L"Ok");
    }

    void do_cylinder(int h,int r, int n)
    {
        if(g_scene)
        {
        core::matrix4 M;
        M.setTranslation(g_scene->elements[0].brush.vertices[0].V);
        g_scene->elements[0].brush = make_cylinder(h,r,n);
        g_scene->elements[0].brush.translate(M);
        }
    }

    bool OnEvent(const SEvent& event)
    {
        if(event.EventType == EET_GUI_EVENT)
        {
            s32 id = event.GUIEvent.Caller->getID();
            gui::IGUIEnvironment* env = device->getGUIEnvironment();

            switch(event.GUIEvent.EventType)
            {
                case EGET_BUTTON_CLICKED:
                {
                    switch(id)
                    {
                        case GUI_ID_MAKE_CYLINDER_BUTTON_OK:
                            int radius=100;
                            int height=100;
                            int sides=16;

                            gui::IGUIEditBox* editbox =  (gui::IGUIEditBox*)(this->getElementFromId(GUI_ID_MAKE_CYLINDER_EDITBOX_HEIGHT));
                            if(editbox)
                            {
                                height = core::strtoul10(core::string<char>(editbox->getText()).c_str());
                            }
                            editbox =  (gui::IGUIEditBox*)(this->getElementFromId(GUI_ID_MAKE_CYLINDER_EDITBOX_RADIUS));
                            if(editbox)
                            {
                                radius = core::strtoul10(core::string<char>(editbox->getText()).c_str());
                            }
                            editbox =  (gui::IGUIEditBox*)(this->getElementFromId(GUI_ID_MAKE_CYLINDER_EDITBOX_SIDES));
                            if(editbox)
                            {
                                sides = core::strtoul10(core::string<char>(editbox->getText()).c_str());
                            }

                            do_cylinder(height,radius,sides);

                            //std::cout<<"making active cube "<<width<<","<<height<<","<<length<<" ";

                            this->remove();
                            break;
                    }
                }
                break;
            }

        }
    gui::CGUIWindow::OnEvent(event);
    };

    ~MakeCylinderWindow()
    {
        //edit_width->drop();
        //edit_length->drop();
       // edit_height->drop();
    }

private:
    //gui::IGUIEditBox* edit_height;
    //gui::IGUIEditBox* edit_radius;
    //gui::IGUIEditBox* edit_sides;

};


class MakeCubeWindow : public gui::CGUIWindow
{
public:
    MakeCubeWindow(gui::IGUIEnvironment* env, gui::IGUIElement* parent,s32 id, core::rect<s32> rect)
    : gui::CGUIWindow(env,parent,id,rect)
    {
        this->setText(L"Make Cube\n");
        edit_width = env->addEditBox(L"256",core::rect<s32>(core::vector2di(48,32),core::vector2di(96,32+24)),true,this,GUI_ID_MAKE_CUBE_EDITBOX_WIDTH);
        //edit_width->grab();
        edit_height = env->addEditBox(L"64",core::rect<s32>(core::vector2di(48,64),core::vector2di(96,64+24)),true,this,GUI_ID_MAKE_CUBE_EDITBOX_HEIGHT);
        //edit_height->grab();
        edit_length = env->addEditBox(L"64",core::rect<s32>(core::vector2di(48,96),core::vector2di(96,96+24)),true,this,GUI_ID_MAKE_CUBE_EDITBOX_LENGTH);
        //edit_length->grab();
        env->addStaticText(L"Width",core::rect<s32>(core::vector2di(8,36),core::vector2di(48,36+24)),false,false,this,-1);
        env->addStaticText(L"Height",core::rect<s32>(core::vector2di(8,68),core::vector2di(48,68+24)),false,false,this,-1);
        env->addStaticText(L"Length",core::rect<s32>(core::vector2di(8,100),core::vector2di(48,100+24)),false,false,this,-1);
        env->addButton(core::rect<s32>(core::vector2di(64,140),core::vector2di(128,140+32)),this,GUI_ID_MAKE_CUBE_BUTTON_OK,L"Ok");
    }

    void do_cube(int h,int w, int el)
    {
        if(g_scene)
        {
        core::matrix4 M;
        M.setTranslation(g_scene->elements[0].brush.vertices[0].V);
        g_scene->elements[0].brush = make_poly_cube(h,el,w);
        g_scene->elements[0].brush.translate(M);
        std::cout<<"new cube "<< g_scene->elements[0].brush.surface_groups.size()<<" surface groups\n";
        }
    }

    bool OnEvent(const SEvent& event)
    {
        if(event.EventType == EET_GUI_EVENT)
        {
            s32 id = event.GUIEvent.Caller->getID();
            gui::IGUIEnvironment* env = device->getGUIEnvironment();

            switch(event.GUIEvent.EventType)
            {
                case EGET_BUTTON_CLICKED:
                {
                    switch(id)
                    {
                        case GUI_ID_MAKE_CUBE_BUTTON_OK:
                            int width=100;
                            int height=100;
                            int length=100;

                            gui::IGUIEditBox* editbox =  (gui::IGUIEditBox*)(this->getElementFromId(GUI_ID_MAKE_CUBE_EDITBOX_WIDTH));
                            if(editbox)
                            {
                                width = core::strtoul10(core::string<char>(editbox->getText()).c_str());
                            }
                            editbox =  (gui::IGUIEditBox*)(this->getElementFromId(GUI_ID_MAKE_CUBE_EDITBOX_HEIGHT));
                            if(editbox)
                            {
                                height = core::strtoul10(core::string<char>(editbox->getText()).c_str());
                            }
                            editbox =  (gui::IGUIEditBox*)(this->getElementFromId(GUI_ID_MAKE_CUBE_EDITBOX_LENGTH));
                            if(editbox)
                            {
                                length = core::strtoul10(core::string<char>(editbox->getText()).c_str());
                            }

                            do_cube(height,width,length);

                            std::cout<<"making active cube "<<width<<","<<height<<","<<length<<" ";

                            this->remove();
                            break;
                    }
                }
                break;
            }

        }
    gui::CGUIWindow::OnEvent(event);
    };

    ~MakeCubeWindow()
    {
        //edit_width->drop();
        //edit_length->drop();
       // edit_height->drop();
    }

private:
    gui::IGUIEditBox* edit_width;
    gui::IGUIEditBox* edit_length;
    gui::IGUIEditBox* edit_height;

};
*/


/*
void geometry_scene::generate_edit_mesh()
{
    std::cout<<"generating edit mesh...\n";

    if(this->mesh_node_edit)
        this->mesh_node_edit->remove();

    scene::SMeshBuffer* buffer;
    scene::SMesh* mesh = new scene::SMesh;
    LineHolder nograph;

    polyfold pf = *this->get_total_geometry();

    //pf.merge_faces2();
    //pf.simplify_edges();
    //pf.reduce_edges_vertices2();

    //buffer = new scene::SMeshBuffer();
	//buffer->Material.Lighting = false;
	//buffer->Material.setTexture(0, driver->getTexture("wall.bmp"));


	int b_count=0;
	int v_count=0;
	int i_count=0;

	this->edit_mesh_buffer_faces.clear();
	//std::cout<<pf.surface_groups.size()<<" surface groups\n";

    for(int f_i =0; f_i<pf.faces.size(); f_i++)
    {
        if(pf.faces[f_i].loops.size()==0)
        {
            this->edit_mesh_buffer_faces.push_back(-1);
            continue;
        }

        this->edit_mesh_buffer_faces.push_back(b_count);

        std::vector<triangle_holder> triangles;

        triangle_holder th = pf.trianglize(f_i,NULL,nograph,nograph);
       // th.surface_info = pf.surface_groups[pf.faces[f_i].surface_group];
        th.surface_info.type = SURFACE_GROUP_STANDARD;
        triangles.push_back(th);


        buffer = new scene::SMeshBuffer();
        buffer->Material.Lighting = false;
        buffer->Material.setTexture(0,driver->getTexture(pf.faces[f_i].texture_name.c_str()));

        make_meshbuffer_from_triangles(triangles,buffer);

        mesh->addMeshBuffer(buffer);

        b_count++;
        i_count+=buffer->Indices.size();
        v_count+=buffer->Vertices.size();
        //std::cout<<"buffer "<<b_count<<" has "<<buffer->Vertices.size()<<" vertices and "<<buffer->Indices.size()/3<<" triangles\n";

        pf.faces[f_i].temp_b = false;
    }

    std::cout<<b_count <<" buffers containing "<<v_count<<" vertices and "<<i_count/3<<" triangles\n";

    this->mesh_node_edit = new scene::CMeshSceneNode(mesh,smgr->getRootSceneNode(),smgr,747);
    this->mesh_node_edit->SetBox(pf.bbox);
    mesh->drop();


    for(int f_i = 0; f_i<pf.faces.size(); f_i++)
    {

        int brush_j = pf.faces[f_i].original_brush;
        int face_j = pf.faces[f_i].original_face;

        if(pf.faces[f_i].loops.size()>0)
        {
            poly_face* f = &elements[brush_j].brush.faces[face_j];
            f->temp_b = false;
        }
    }

    for(int f_i = 0; f_i<pf.faces.size(); f_i++)
    {

        int brush_j = pf.faces[f_i].original_brush;
        int face_j = pf.faces[f_i].original_face;

        if(pf.faces[f_i].loops.size()>0)
        {

            poly_face* f = &elements[brush_j].brush.faces[face_j];

            std::vector<int> surface;

            if(f->temp_b==false)
            {
                switch(elements[brush_j].brush.surface_groups[ f->surface_group ].type)
                {
                case SURFACE_GROUP_STANDARD:
                    {
                        scene::IMeshBuffer* buff = get_edit_mesh_buffer_by_face(f_i);

                        if(buff)
                        {
                            calculate_meshbuffer_uvs_cube(&elements[brush_j].brush,f, buff);
                        }
                        f->temp_b = true;
                    } break;
                case SURFACE_GROUP_CYLINDER:
                    {
                        surface = getSurfaceFromFace(f_i);
                        std::cout<<"CYLINDER\n";
                        for(int b_i : surface)
                        {
                            int brush_i = pf.faces[b_i].original_brush;
                            int face_i = pf.faces[b_i].original_face;

                            poly_face* ff = &elements[brush_i].brush.faces[face_i];

                            if(ff->temp_b == false)
                            {
                                scene::IMeshBuffer* buff = get_edit_mesh_buffer_by_face(b_i);

                                if(buff)
                                {
                                    calculate_meshbuffer_uvs_cylinder(&elements[brush_j].brush,ff, buff);
                                }

                                ff->temp_b = true;
                            }
                        }

                    } break;
                case SURFACE_GROUP_SPHERE:
                    {
                        surface = getSurfaceFromFace(f_i);
                        std::cout<<"SPHERE\n";
                        for(int b_i : surface)
                        {
                            int brush_i = pf.faces[b_i].original_brush;
                            int face_i = pf.faces[b_i].original_face;

                            poly_face* ff = &elements[brush_i].brush.faces[face_i];

                            if(ff->temp_b == false)
                            {
                                scene::IMeshBuffer* buff = get_edit_mesh_buffer_by_face(b_i);

                                if(buff)
                                {
                                    calculate_meshbuffer_uvs_dome(&elements[brush_j].brush,ff, buff);
                                }

                                ff->temp_b = true;
                            }
                        }

                    } break;
                }//switch
            }//if(f->temp_b==false)
        }
    }

    std::cout<<"testing....\n\n";
    edit_meshnode_interface.generate_mesh_node(this);
    final_meshnode_interface.generate_mesh_node(this);
    std::cout<<"DONE\n";
}


void geometry_scene::generate_final_mesh()
{
    std::cout<<"generating final mesh...\n";

    if(this->mesh_node_final)
        this->mesh_node_final->remove();

    scene::SMeshBuffer* buffer;
    scene::SMesh* mesh = new scene::SMesh;
    LineHolder nograph;

    polyfold pf = *this->get_total_geometry();

    //pf.merge_faces2();
    //pf.simplify_edges();
    //pf.reduce_edges_vertices2();

    //buffer = new scene::SMeshBuffer();
	//buffer->Material.Lighting = false;
	//buffer->Material.setTexture(0, driver->getTexture("wall.bmp"));
	std::vector<video::ITexture*> textures_used;
	{
        for(int f_i =0 ;f_i<pf.faces.size(); f_i++)
        {
            video::ITexture* tex_j = driver->getTexture(pf.faces[f_i].texture_name.c_str());

            bool b=false;
            for(video::ITexture* tex_i : textures_used)
            {
                if(tex_j == tex_i)
                    b=true;
            }
            if(!b)
                textures_used.push_back(tex_j);
        }
	}

	std::cout<<"geometry uses "<<textures_used.size()<<" textures\n";

	for(int f_i = 0; f_i<pf.faces.size(); f_i++)
    {
        pf.faces[f_i].temp_b=false;
    }

    std::cout<< pf.faces.size()<<" faces\n\n";
	for(int t_i=0; t_i<textures_used.size(); t_i++)
    {
        std::vector<triangle_holder> triangle_groups;
        for(int f_i =0 ;f_i<pf.faces.size(); f_i++)
        {
            if(pf.faces[f_i].loops.size() > 0 && pf.faces[f_i].temp_b==false)
            {
                video::ITexture* tex_j = driver->getTexture(pf.faces[f_i].texture_name.c_str());
                if(tex_j == textures_used[t_i])
                {
                    switch(pf.getFaceSurfaceGroup(f_i).type)
                    {
                     case SURFACE_GROUP_STANDARD:
                        {
                            triangle_holder th = pf.trianglize(f_i,NULL,nograph,nograph);
                            th.surface_info.type = SURFACE_GROUP_STANDARD;
                            triangle_groups.push_back(th);
                            pf.faces[f_i].temp_b=true;
                        } break;
                    case SURFACE_GROUP_SPHERE:
                        {
                            std::vector<int> sfg = getSurfaceFromFace(f_i);
                            for(int b_i : sfg)
                            {
                                pf.faces[b_i].temp_b=true;
                            }
                            triangle_holder th = pf.trianglize(sfg);
                            std::cout<<sfg.size()<<" faces in sphere\n";
                            //triangle_holder th = pf.trianglize(f_i,NULL,nograph,nograph);
                            th.surface_info.type = SURFACE_GROUP_SPHERE;
                            triangle_groups.push_back(th);
                            //pf.faces[f_i].temp_b=true;
                        } break;
                    case SURFACE_GROUP_CYLINDER:
                        {
                            std::vector<int> sfg = getSurfaceFromFace(f_i);
                            for(int b_i : sfg)
                            {
                                pf.faces[b_i].temp_b=true;
                            }
                            triangle_holder th = pf.trianglize(sfg);
                            std::cout<<sfg.size()<<" faces in cylinder\n";
                            //triangle_holder th = pf.trianglize(f_i,NULL,nograph,nograph);
                            th.surface_info.type = SURFACE_GROUP_CYLINDER;
                            triangle_groups.push_back(th);
                            //pf.faces[f_i].temp_b=true;
                        } break;
                    }
                }
            }
        }

        buffer = new scene::SMeshBuffer();
        buffer->Material.Lighting = false;
        buffer->Material.setTexture(0,textures_used[t_i]);

       // surface_group sg;
        make_meshbuffer_from_triangles(triangle_groups,buffer);

        mesh->addMeshBuffer(buffer);

        std::cout<<"buffer "<<t_i<<" has "<<buffer->Vertices.size()<<" vertices and "<<buffer->Indices.size()/3<<" triangles\n";
    }

    this->mesh_node_final = new scene::CMeshSceneNode(mesh,smgr->getRootSceneNode(),smgr,747);
    this->mesh_node_final->SetBox(pf.bbox);
    this->mesh_node_final->setVisible(false);
    mesh->drop();
}
*/


/*
scene::IMeshBuffer* geometry_scene::get_edit_mesh_buffer_by_face(int i)
{
    if(i<edit_mesh_buffer_faces.size())
    {
        int idx = edit_mesh_buffer_faces[i];
        if(idx !=-1 )
        {
            scene::IMeshBuffer* ret = this->mesh_node_edit->getMesh()->getMeshBuffer(idx);
            return ret;
        }
    }
    return NULL;
}

scene::IMeshBuffer* geometry_scene::get_edit_mesh_buffer(int i)
{
    return this->mesh_node_edit->getMesh()->getMeshBuffer(i);
}
*/
  /*
        for(int i=0; i<this->geo_scene->elements.size(); i++)
        {
            if(this->geo_scene->elements[i].has_geometry())
                for(int j=0; j<this->geo_scene->elements[i].geometry.faces.size(); j++)
                {
                    core::plane3df f_plane(this->geo_scene->elements[i].geometry.faces[j].m_center,
                                   this->geo_scene->elements[i].geometry.faces[j].m_normal);

                if(GetAnyPlaneClickVector(this->Texture->getOriginalSize(),this->getCamera(),f_plane, clickx,clicky,hitvec) &&
                   this->geo_scene->elements[i].geometry.is_point_on_face(j,hitvec))
                    {

                     f32 d = hitvec.getDistanceFrom(cam_pos);
                     //std::cout<<"hit face "<<i<<","<<j<<" "<<d<<"\n";
                     if(d<dist)
                        {
                            //std::cout<<"a\n";
                             dist = d;
                             selected_poly = i;
                             selected_face = j;
                        }
                    }
                }
        }*/
        /*
            for(int j=0; j<this->geo_scene->get_total_geometry()->faces.size(); j++)
            {
                core::plane3df f_plane(this->geo_scene->get_total_geometry()->faces[j].m_center,
                               this->geo_scene->get_total_geometry()->faces[j].m_normal);

                if(GetAnyPlaneClickVector(this->Texture->getOriginalSize(),this->getCamera(),f_plane, clickx,clicky,hitvec) &&
                   this->geo_scene->get_total_geometry()->is_point_on_face(j,hitvec))
                    {

                     f32 d = hitvec.getDistanceFrom(cam_pos);
                     //std::cout<<"hit face "<<i<<","<<j<<" "<<d<<"\n";
                     if(d<dist && d > near_dist &&
                        this->geo_scene->get_total_geometry()->faces[j].m_normal.dotProduct(cam_pos-hitvec) > 0 &&
                        (hitvec-cam_pos).dotProduct(this->getCamera()->getTarget()-cam_pos) > 0)
                        {
                            //std::cout<<"a\n";
                             dist = d;
                             selected_poly = 0;
                             selected_face = j;
                        }
                    }
            }
        */

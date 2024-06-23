#include "GUI_tools.h"
#include <irrlicht.h>
#include "Reflection.h"
#include "uv_tool.h"
#include "CGUIWindow.h"
#include "geometry_scene.h"
#include "BufferManager.h"
#include "utils.h"
#include "uv_mapping.h"
#include "LightMaps.h"

using namespace std;

UV_Editor_Base* UV_Editor_Tool::base = NULL;
multi_tool_panel* UV_Editor_Tool::panel = NULL;

extern IrrlichtDevice* device;

extern irr::video::ITexture* small_circle_tex_add_selected;
extern irr::video::ITexture* small_circle_tex_add_not_selected;
extern irr::video::ITexture* small_circle_tex_sub_selected;
extern irr::video::ITexture* small_circle_tex_sub_not_selected;
extern irr::video::ITexture* small_circle_tex_red_selected;
extern irr::video::ITexture* small_circle_tex_red_not_selected;

extern irr::video::ITexture* med_circle_tex_add_selected;
extern irr::video::ITexture* med_circle_tex_add_not_selected;
extern irr::video::ITexture* med_circle_tex_sub_selected;
extern irr::video::ITexture* med_circle_tex_sub_not_selected;
extern irr::video::ITexture* med_circle_tex_red_selected;
extern irr::video::ITexture* med_circle_tex_red_not_selected;


//======================================================
// UV Editor Window
//


UV_Editor_Window::UV_Editor_Window(gui::IGUIEnvironment* env, gui::IGUIElement* parent, UV_Editor_Base* base, geometry_scene* g_scene_, s32 id, core::rect<s32> rect)
    : gui::CGUIWindow(env, parent, id, rect), my_base(base)
{
    //MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
    //receiver->Register(this);
    uv_scene = new geometry_scene(device->getVideoDriver(), (MyEventReceiver*)device->getEventReceiver());
}

UV_Editor_Window::~UV_Editor_Window()
{
    //MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
    //receiver->UnRegister(this);

    if(uv_scene)
        delete uv_scene;

    if (my_image)
        delete my_image;

    my_base->window_closed();
}

void UV_Editor_Window::show()
{
    
    my_panel = new UV_Editor_Panel(Environment, device->getVideoDriver(), this, this,
        GUI_ID_PANEL_UV, core::rect<s32>(core::position2d<s32>(8, 316), core::dimension2d<u32>(450, 300)));
    //test_panel3->resize(core::position2d<s32>(8,316), core::dimension2d<u32>(450,300));
    //my_panel->resize(core::vector2di(16, 32), core::dimension2d<u32>(512, 384));

    my_panel->Initialize(device->getSceneManager(), uv_scene);
    
    my_panel->setGridSnap(32);

    OK_Button = Environment->addButton(core::rect<s32>(core::position2d<s32>(580, 360), core::dimension2d<u32>(64, 32)), this, -1, L"OK");

    if(my_texture)
        my_image = new TextureImage(my_texture);

    my_panel->setAxis(CAMERA_Y_AXIS);

}

bool UV_Editor_Window::OnEvent(const SEvent& event)
{
    if (event.EventType == EET_GUI_EVENT)
    {
        if (event.GUIEvent.EventType == gui::EGET_BUTTON_CLICKED)
        {

            if (event.GUIEvent.Caller == OK_Button)
            {
                
                click_OK();
                remove();
                return true;
            }
        }
    }
    return gui::CGUIWindow::OnEvent(event);
}

void UV_Editor_Window::draw()
{
    if (my_panel)
        my_panel->render();

    gui::CGUIWindow::draw();
}

void UV_Editor_Window::set_poly(polyfold pf)
{
    if (uv_scene)
    {
        geo_element em;
        em.brush = pf;
        uv_scene->elements.push_back(em);
    }

}

void UV_Editor_Window::set_texture(video::ITexture* tex)
{
    my_texture = tex;

    if (my_texture && !my_image)
    {
        my_image = new TextureImage(my_texture);

        core::dimension2du texture_size = my_texture->getOriginalSize();

        my_panel->setGridSpacing(std::min(texture_size.Height, texture_size.Width));
        my_panel->setAxis(CAMERA_Y_AXIS);
    }
}

void UV_Editor_Window::click_OK()
{
    if(uv_scene->elements.size() > 0)
        my_base->make_custom_surface_group(uv_scene->elements[0].brush);
}

TextureImage* UV_Editor_Window::getTextureImage()
{
    return my_image;
}


//======================================================
// UV Editor Base
//


void UV_Editor_Base::show()
{
    UV_Editor_Window* win = new UV_Editor_Window(env, env->getRootGUIElement(), this, g_scene, -1, core::rect<s32>(150, 64, 150 + 680, 64 + 436));

    win->setText(L"UV Editor");

    

    win->show();
    win->drop();

    if (g_scene->getSelectedFaces().size() > 0)
    {
       // int f_no = g_scene->getSelectedFaces()[0];
        polyfold* pf = g_scene->get_total_geometry();

        core::stringw texname = g_scene->get_original_brush_face(g_scene->getSelectedFaces()[0])->texture_name;
        my_texture = device->getVideoDriver()->getTexture(texname);

       // clear_poly();

        for (int f_i : g_scene->getSelectedFaces())
        {
          //  make_face(pf, f_i, my_texture);
        }

        win->set_poly(uv_poly);

        

        win->set_texture(device->getVideoDriver()->getTexture(texname));
        
        
    }
    else
        std::cout << "no face selected\n";



    //core::rect<s32> client_rect(core::vector2di(0, 0),
    //    core::dimension2du(this->panel->getClientRect()->getAbsolutePosition().getWidth(),
    //        win->getClientRect()->getAbsolutePosition().getHeight()));

    //core::rect<s32> client_rect = win->getClientRect();
    //client_rect.UpperLeftCorner.Y += 24;

   // File_Open_Widget* widget = new File_Open_Widget(env, win, g_scene, this, my_ID, win->getClientRect());

   // widget->show();
   // widget->drop();
}

void UV_Editor_Base::initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel* panel_)
{
    tool_base::initialize(name_, my_id, env_, g_scene_, panel_);
}

void UV_Editor_Base::widget_closing(Reflected_Widget_EditArea*)
{
}

void UV_Editor_Base::click_OK()
{
}

void UV_Editor_Base::window_closed()
{
}

void UV_Editor_Base::make_custom_surface_group(polyfold pf)
{
    if (original_brush != -1 && original_face != -1)
    {
        polyfold* og_brush = &g_scene->elements[original_brush].brush;
        poly_face* face = &(og_brush->faces[original_face]);

        surface_group* sfg = &(og_brush->surface_groups[face->surface_group]);

        int new_type;

        core::dimension2du texture_size = my_texture->getOriginalSize();

        if (use_geometry_brush)
        {
            new_type = SURFACE_GROUP_CUSTOM_UVS_GEOMETRY;
           // std::cout << "using geometry vertexes\n";
        }
        else
        {
            new_type = SURFACE_GROUP_CUSTOM_UVS_BRUSH;
          //  std::cout << "using brush vertexes\n";
        }

        if (sfg->type != SURFACE_GROUP_CUSTOM_UVS_GEOMETRY && 
            sfg->type != SURFACE_GROUP_CUSTOM_UVS_BRUSH)
        {
            surface_group new_g;
            new_g.type = new_type;
            og_brush->surface_groups.push_back(new_g);
            int sfg_i = og_brush->surface_groups.size() - 1;
            sfg = &og_brush->surface_groups[sfg_i];
            face->surface_group = sfg_i;
        }

        sfg->texcoords.clear();
        sfg->type = new_type;

        //for (int i = 0; i < pf.vertices.size(); i++)
        for(int i=0; i<vertex_index.size(); i++)
        {
            if (vertex_index[i] != -1)
            {
                core::vector2df pos;
                pos.Y = -pf.vertices[vertex_index[i]].V.X / texture_size.Width;
                pos.X = -pf.vertices[vertex_index[i]].V.Z / texture_size.Height;
                point_texcoord tc;
                tc.vertex = i;
                tc.texcoord = pos;
                sfg->texcoords.push_back(tc);

               // std::cout << "  " << pos.X << "," << pos.Y << "\n";
            }

            //sfg->texcoords.push_back(pos);
            //std::cout << "  " << pos.X << "," << pos.Y << "\n";
        }

        //std::cout << sfg->texcoords.size() << " texture coords\n";

        g_scene->edit_meshnode_interface.recalc_uvs_for_face_custom(g_scene,original_brush, original_face, my_face_no);
        g_scene->final_meshnode_interface.recalc_uvs_for_face_custom(g_scene,original_brush, original_face, my_face_no);
    }
}


//======================================================
// UV Editor Camera Panel
//


UV_Editor_Panel::UV_Editor_Panel(IGUIEnvironment* environment, video::IVideoDriver* driver, IGUIElement* parent, UV_Editor_Window* win, s32 id, core::rect<s32> rectangle)
    : TestPanel_2D(environment, driver, parent, id, rectangle)
{
    my_window = win;
    vHorizontal = core::vector3df(0, 0, 1);
    vVertical = core::vector3df(1, 0, 0);
    vAxis = core::vector3df(0, 1, 0);

    uv_scene = new geometry_scene(device->getVideoDriver(), (MyEventReceiver*)device->getEventReceiver());
}

UV_Editor_Panel::~UV_Editor_Panel()
{
    if (uv_scene)
        delete uv_scene;
}

void UV_Editor_Panel::resize(core::dimension2d<u32> new_size)
{
    
    f32 ratio = (f32)this->Texture->getOriginalSize().Width / (f32)this->Texture->getOriginalSize().Height;
    core::matrix4 M;

    if (ratio >= 1.0)
    {
        M.buildProjectionMatrixOrthoLH(516 * ratio, 516, 0, 10000);

        viewSize = dimension2du(516 * ratio, 516);
    }
    else
    {
        M.buildProjectionMatrixOrthoLH(516, 516 / ratio, 0, 10000);

        viewSize = dimension2du(516, 516 / ratio);
    }

    getCamera()->setProjectionMatrix(M, true);
}

void UV_Editor_Panel::showTriangles(bool bShowTriangles)
{
    bShowBrushes = bShowTriangles;
}

void UV_Editor_Panel::showLightmap(bool bShowLightmap)
{
    bRenderLightmap = bShowLightmap;
}

scene::ICameraSceneNode* UV_Editor_Panel::getCamera()
{
    if (this->camera == NULL)
    {
        if (this->smgr)
        {
            core::dimension2di texture_size(512, 512);
            core::matrix4 M;

            M.buildProjectionMatrixOrthoLH(512, 512, 0, 10000);
            this->camera = smgr->addCameraSceneNode(0, core::vector3df(-texture_size.Width*0.5, 1000, -texture_size.Height * 0.5), core::vector3df(-texture_size.Width * 0.5, 0, -texture_size.Height * 0.5), -1, false);
            this->camera->setProjectionMatrix(M, true);
        }
    }
    return this->camera;
}

void UV_Editor_Panel::showMaterialGroup(int mg_n)
{
    if (uv_scene)
    {
        uv_scene->elements.clear();

        polyfold* pf = geo_scene->get_total_geometry();

        const vector<TextureMaterial>& mat_groups = geo_scene->final_meshnode_interface.getMaterialsUsed();
        if (mg_n < mat_groups.size())
        {
            current_mat_group = mg_n;

            for (int f_i : mat_groups[mg_n].faces)
            {
                make_face(pf, f_i, NULL);
            }
            
            if (bRenderLightmap)
            {
                
                if (my_image)
                    delete my_image;
                my_image = NULL;

                vector<video::ITexture*>& lm_textures = geo_scene->getLightmapManager()->lightmap_textures;

                if (mg_n < lm_textures.size())
                {
                    my_image = new TextureImage(lm_textures[mg_n]);
                }
            }
        }

        
    }
}


bool UV_Editor_Panel::OnEvent(const SEvent& event)
{
    switch (event.EventType)
    {
    case EET_USER_EVENT:
        switch (event.UserEvent.UserData1)
        {
            case USER_EVENT_MATERIAL_GROUP_SELECTION_CHANGED:
            {
                int mg = geo_scene->get_selected_material_group();

                if (mg >= 0)
                {
                    showMaterialGroup(mg);
                }
                else
                {
                    if (uv_scene)
                        uv_scene->elements.clear();
                }
                
            }break;

            case USER_EVENT_SELECTION_CHANGED:
            {
                if (geo_scene->getSelectedFaces().size() != 0)
                {
                    /*
                    clear_poly();

                    polyfold* pf = geo_scene->get_total_geometry();

                    for (int f_i : geo_scene->getSelectedFaces())
                    {
                        make_face(pf, f_i, NULL);
                    }

                    if (uv_scene)
                    {
                        uv_scene->elements.clear();
                        geo_element em;
                        em.brush = uv_poly;
                        uv_scene->elements.push_back(em);
                    }*/

                }
                else
                {
                    /*
                    clear_poly();
                    if (uv_scene)
                    {
                        uv_scene->elements.clear();
                    }*/
                }
                return true;
            }break;

        } break;
    }

    return TestPanel_2D::OnEvent(event);
}

void UV_Editor_Panel::make_face(polyfold* pf_0, int f_no, video::ITexture* face_texture2)
{
    MeshBuffer_Chunk chunk = geo_scene->edit_meshnode_interface.get_mesh_buffer_by_face(f_no);

    my_face_no = f_no;

    original_face = pf_0->faces[f_no].original_face;
    original_brush = pf_0->faces[f_no].original_brush;

    polyfold uv_poly;

    scene::IMeshBuffer* buffer = chunk.buffer;
    if (buffer)
    {
        polyfold* source_brush = &geo_scene->elements[original_brush].brush;
        //poly_face* face = &(og_brush->faces[original_face]);

        u32 n_points = source_brush->vertices.size();
        vertex_index.assign(n_points, -1);

        // std::cout << buffer->getIndexCount() << " indices\n";

         //u32* indices = (u32*)buffer->getIndices();
        // video::S3DVertex* vertices = (video::S3DVertex2TCoords*)buffer->getVertices();

        int c = 0;

        bool use_geometry_brush = false;

        for (int i = chunk.begin_i; i < chunk.end_i; i++)
        {
            u32 v0 = buffer->getIndices()[i];
            video::S3DVertex* vtx0 = &((video::S3DVertex2TCoords*)buffer->getVertices())[v0];

            if (source_brush->find_point(vtx0->Pos) == -1)
            {
                use_geometry_brush = true;
                break;
            }
        }

        if (use_geometry_brush)
        {
            source_brush = &geo_scene->elements[original_brush].geometry;

            n_points = source_brush->vertices.size();
            vertex_index.assign(n_points, -1);
        }
        //std::cout << "chunks:\n";
        for (int i = chunk.begin_i; i < chunk.end_i; i += 3)
        {
            // core::dimension2du texture_size = face_texture->getOriginalSize();

            u32 v0 = buffer->getIndices()[i];
            u32 v1 = buffer->getIndices()[i + 1];
            u32 v2 = buffer->getIndices()[i + 2];

            // std::cout << v0 << "," << v1 << "," << v2 << "\n";

            video::S3DVertex2TCoords* vtx0 = &((video::S3DVertex2TCoords*)buffer->getVertices())[v0];
            video::S3DVertex2TCoords* vtx1 = &((video::S3DVertex2TCoords*)buffer->getVertices())[v1];
            video::S3DVertex2TCoords* vtx2 = &((video::S3DVertex2TCoords*)buffer->getVertices())[v2];

            core::vector3df pos(0, 0, 0);

            pos.Z = -vtx0->TCoords2.X * 512.0f;// *texture_size.Width;
            pos.X = -vtx0->TCoords2.Y * 512.0f;// *texture_size.Height;
            int a = uv_poly.get_point_or_add(pos);

            int a_indx = source_brush->find_point(vtx0->Pos);
            vertex_index[a_indx] = a;


            pos.Z = -vtx1->TCoords2.X * 512.0f;// *texture_size.Width;
            pos.X = -vtx1->TCoords2.Y * 512.0f;// *texture_size.Height;
            int b = uv_poly.get_point_or_add(pos);

            int b_indx = source_brush->find_point(vtx1->Pos);
            vertex_index[b_indx] = b;

            pos.Z = -vtx2->TCoords2.X * 512.0f;// *texture_size.Width;
            pos.X = -vtx2->TCoords2.Y * 512.0f;// *texture_size.Height;
            int c = uv_poly.get_point_or_add(pos);

            int c_indx = source_brush->find_point(vtx2->Pos);
            vertex_index[c_indx] = c;

            int e0 = uv_poly.get_edge_or_add(a, b, 0);
            int e1 = uv_poly.get_edge_or_add(b, c, 0);
            int e2 = uv_poly.get_edge_or_add(a, c, 0);
        }

        //std::cout << uv_poly.edges.size() << " edges \n";

        for (int i = 0; i < uv_poly.edges.size(); i++)
        {
            int v_i0 = uv_poly.edges[i].v0;
            int v_i1 = uv_poly.edges[i].v1;

            core::vector3df v0 = uv_poly.vertices[v_i0].V;
            core::vector3df v1 = uv_poly.vertices[v_i1].V;

            if (pf_0->find_edge(v0, v1) != -1)
            {
                uv_poly.edges[i].topo_group = 2;
            }
        }

        c = 0;
        for (int i = 0; i < uv_poly.edges.size(); i++)
        {
            if (uv_poly.edges[i].topo_group == 2)
                c++;
        }

        if (uv_scene)
        {
            geo_element em;
            em.brush = std::move(uv_poly);
            uv_scene->elements.push_back(em);
        }
    }
}


void UV_Editor_Panel::drawGrid(video::IVideoDriver* driver, const video::SMaterial material)
{
    //driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
    //driver->setMaterial(material);

    int far_value = this->getCamera()->getFarValue();
    far_value *= -1;

    //if (m_axis == CAMERA_Y_AXIS)
    {
        int interval = gridSpace;
        while (viewSize.Width / interval > 36 || viewSize.Height / interval > 36)
        {
            interval = interval << 1;
        }
        int h_lines = viewSize.Width / interval;
        int v_lines = viewSize.Height / interval;
        core::vector3df vDownLeft = this->getCamera()->getAbsolutePosition();
        vDownLeft.Z -= viewSize.Width / 2;
        vDownLeft.X -= viewSize.Height / 2;
        int start_x = interval * ((int)vDownLeft.Z / interval);
        int start_y = interval * ((int)vDownLeft.X / interval);

        video::SColor col = video::SColor(255, 128, 128, 128);
        /*
        for (int i = -1; i < h_lines + 2; i++)
        {
            if (start_x + i * interval >= 0)
                driver->draw3DLine(core::vector3df(0, far_value, start_x + i * interval), core::vector3df(vDownLeft.X + viewSize.Height, far_value, start_x + i * interval), col);
        }
        for (int i = -1; i < v_lines + 2; i++)
        {
            if (start_y + i * interval >= 0)
                driver->draw3DLine(core::vector3df(start_y + i * interval, far_value, 0), core::vector3df(start_y + i * interval, far_value, vDownLeft.Z + viewSize.Width), col);
        }*/
        
        driver->draw3DLine(core::vector3df(0, 0, 0), core::vector3df(0, 0, -512), col);
        driver->draw3DLine(core::vector3df(0, 0, 0), core::vector3df(-512, 0, 0), col);
        driver->draw3DLine(core::vector3df(-512, 0, -512), core::vector3df(0, 0, -512), col);
        driver->draw3DLine(core::vector3df(-512, 0, -512), core::vector3df(-512, 0, 0), col);
    }
}

void UV_Editor_Panel::render()
{

    driver->setRenderTarget(getImage(), true, true, video::SColor(255, 4, 4, 4));
    smgr->setActiveCamera(getCamera());

    getCamera()->render();

    video::SMaterial someMaterial;
    someMaterial.Lighting = false;
    someMaterial.Thickness = 1.0;
    someMaterial.MaterialType = video::EMT_SOLID;

    

   // if (my_window && my_window->getTextureImage())
   // {
   //     my_window->getTextureImage()->render();
   //     //driver->draw2DImage(uv_texture, core::vector2di(0, 0), false);
   // }

    if (bRenderLightmap && my_image)
    {
        my_image->render();
    }

    driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
    driver->setMaterial(someMaterial);

    //if (bShowGrid)
        this->drawGrid(driver, someMaterial);

    if (this->uv_scene)
    {
        if (bShowBrushes)
        {
            for (int e_i = 0; e_i < this->uv_scene->elements.size(); e_i++)
            {
                this->uv_scene->elements[e_i].draw_brush(driver, someMaterial);
            }

            for (int e_i = 0; e_i < this->uv_scene->elements.size(); e_i++)
            {
                geo_element* geo = &this->uv_scene->elements[e_i];

                if (geo->bSelected)
                {

                    core::vector2di coords;
                    for (int i = 0; i < geo->brush.vertices.size(); i++)
                    {
                        GetScreenCoords(geo->brush.vertices[i].V, coords);
                        coords.X -= 4;
                        coords.Y -= 4;
                        if (uv_scene->selected_brush_vertex_editing == e_i && geo->selected_vertex == i)
                        {
                            if (geo->type == GEO_ADD)
                                driver->draw2DImage(med_circle_tex_add_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                            else if (geo->type == GEO_SUBTRACT)
                                driver->draw2DImage(med_circle_tex_sub_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                            else if (geo->type == GEO_RED)
                                driver->draw2DImage(med_circle_tex_red_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                        }
                        else
                        {
                            if (geo->type == GEO_ADD)
                                driver->draw2DImage(small_circle_tex_add_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                            else if (geo->type == GEO_SUBTRACT)
                                driver->draw2DImage(small_circle_tex_sub_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                            else if (geo->type == GEO_RED)
                                driver->draw2DImage(small_circle_tex_red_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                        }
                    }
                }
            }
        }

        if (bShowGeometry)
            for (geo_element geo : this->uv_scene->elements)
                geo.draw_geometry(driver, someMaterial);
    }

    driver->setRenderTarget(0, true, true, video::SColor(0, 0, 0, 0));
}



void UV_Editor_Panel::OnMenuItemSelected(IGUIContextMenu* menu)
{
}




void UV_Editor_Panel::left_click(core::vector2di pos)
{
    std::vector<int> old_sel_faces = geo_scene->getSelectedFaces();
    std::vector<int> old_sel_nodes = geo_scene->getSelectedNodes();
    std::vector<int> old_sel_brushes = geo_scene->getBrushSelection();

    if (click_hits_poly(&geo_scene->elements[0].brush, core::vector2di(clickx, clicky)))
    {
        geo_scene->setBrushSelection(std::vector<int>{0});
    }
    else
    {
        geo_scene->setBrushSelection(std::vector<int>{});
    }
 
    if (old_sel_faces == geo_scene->getSelectedFaces() &&
        old_sel_nodes == geo_scene->getSelectedNodes() &&
        old_sel_brushes == geo_scene->getBrushSelection())
    {
        return;
    }

    geo_scene->selectionChanged();
}


void UV_Editor_Panel::right_click(core::vector2di pos)
{
    if (geo_scene && geo_scene->getBrushSelection().size() > 0)
    {
        bool bVertexClick = false;
        for (int p_i : geo_scene->getBrushSelection())
        {
            if (click_hits_poly(&geo_scene->elements[p_i].brush, core::vector2di(clickx, clicky)))
            {
                for (int v_i = 0; v_i < geo_scene->elements[p_i].brush.vertices.size(); v_i++)
                {
                    core::vector2di coords;
                    GetScreenCoords(geo_scene->elements[p_i].brush.vertices[v_i].V, coords);
                    if (core::vector2di(clickx, clicky).getDistanceFrom(coords) < 4)
                    {
                        geo_scene->selected_brush_vertex_editing = p_i;
                        geo_scene->elements[p_i].selected_vertex = v_i;
                        bVertexClick = true;
                    }
                }
            }
        }
    }
}


void UV_Editor_Panel::setGridSpacing(int spacing)
{
    gridSpace = spacing;
}

/*
void UV_Editor_Panel::centerCamera()
{
    this->getCamera()->setPosition(core::vector3df((int)texture_size.Width * 0.5, 5000, (int)texture_size.Height * 0.5));
    this->getCamera()->setTarget(core::vector3df((int)texture_size.Width * 0.5, 0, (int)texture_size.Height * 0.5));
}
*/

void UV_Editor_Panel::setAxis(int axis)
{
    if (axis == CAMERA_Y_AXIS)
    {
        core::dimension2du texture_size(128, 128);

        this->getCamera()->setPosition(core::vector3df((int)texture_size.Width * 0.5, 5000, (int)texture_size.Height * 0.5));
        this->getCamera()->setTarget(core::vector3df((int)texture_size.Width * 0.5, 0, (int)texture_size.Height * 0.5));

        this->getCamera()->updateAbsolutePosition();
        this->vHorizontal = core::vector3df(0, 0, 1);
        this->vVertical = core::vector3df(1, 0, 0);
        this->vAxis = core::vector3df(0, 1, 0);
    }
}

//======================================================
// Texture Image for UV Editor
//


TextureImage::TextureImage(video::ITexture* texture)
{
    m_texture = texture;

    Buffer = new scene::SMeshBuffer();

    Buffer->Material.Lighting = false;
    Buffer->Material.setTexture(0, m_texture);
    //Buffer->Material.MaterialType = video::EMT_TRANSPARENT_ADD_COLOR;
    Buffer->Material.MaterialType = video::EMT_SOLID;
    //MaterialType = video::EMT_SOLID;

    //core::dimension2du size = m_texture->getOriginalSize();
    core::dimension2du size = core::dimension2du(512, 512);

    Buffer->Vertices.set_used(4);
    Buffer->Indices.set_used(6);

    Buffer->Indices[0] = (u16)0;
    Buffer->Indices[1] = (u16)2;
    Buffer->Indices[2] = (u16)1;
    Buffer->Indices[3] = (u16)0;
    Buffer->Indices[4] = (u16)3;
    Buffer->Indices[5] = (u16)2;

   

    Buffer->Vertices[0].Pos = core::vector3df(0,0,0);
    Buffer->Vertices[0].Color = video::SColor(255, 255, 255, 255);
    Buffer->Vertices[0].Normal = core::vector3df(0,1,0);
    Buffer->Vertices[0].TCoords = core::vector2df(0, 0);

    Buffer->Vertices[1].Pos = core::vector3df(-(int)size.Width, 0, 0);
    Buffer->Vertices[1].Color = video::SColor(255, 255, 255, 255);
    Buffer->Vertices[1].Normal = core::vector3df(0, 1, 0);
    Buffer->Vertices[1].TCoords = core::vector2df(0, 1);

    Buffer->Vertices[2].Pos = core::vector3df(-(int)size.Width, 0, -(int)size.Height);
    Buffer->Vertices[2].Color = video::SColor(255, 255, 255, 255);
    Buffer->Vertices[2].Normal = core::vector3df(0, 1, 0);
    Buffer->Vertices[2].TCoords = core::vector2df(1, 1);

    Buffer->Vertices[3].Pos = core::vector3df(0, 0, -(int)size.Height);
    Buffer->Vertices[3].Color = video::SColor(255, 255, 255, 255);
    Buffer->Vertices[3].Normal = core::vector3df(0, 1, 0);
    Buffer->Vertices[3].TCoords = core::vector2df(1, 0);
}

TextureImage::~TextureImage()
{
    if (Buffer)
        delete Buffer;
}

void TextureImage::render()
{
    video::IVideoDriver* driver = device->getVideoDriver();

    if (!driver)
        return;

    core::matrix4 mat;

    driver->setTransform(video::ETS_WORLD, mat);
    driver->setMaterial(Buffer->Material);

    driver->drawVertexPrimitiveList(Buffer->getVertices(), 4,
        Buffer->getIndices(), 2, video::EVT_STANDARD, EPT_TRIANGLES, Buffer->getIndexType());
}

core::dimension2du TextureImage::getDimensions()
{
    if (m_texture)
        return m_texture->getOriginalSize();

    return core::dimension2du();
}

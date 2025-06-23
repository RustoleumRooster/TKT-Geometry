#include "GUI_tools.h"
#include <irrlicht.h>
#include "Reflection.h"
#include "CGUIWindow.h"
#include "geometry_scene.h"
#include "uv_tool.h"
#include "GUI_tools.h"
#include "edit_classes.h"
#include "ex_gui_elements.h"
#include <sstream>
#include <algorithm>
#include "edit_env.h"

using namespace std;
using namespace irr;
using namespace gui;

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

#define MIN_OF(out,a,b,c) out = fmin(out,fmin(a,fmin(b,c)));
#define MAX_OF(out,a,b,c) out = fmax(out,fmax(a,fmax(b,c)));


//======================================================
// UV Editor Camera Panel
//

UV_Editor_Panel::UV_Editor_Panel(IGUIEnvironment* environment, video::IVideoDriver* driver, IGUIElement* parent, s32 id, core::rect<s32> rectangle)
    : TestPanel_2D(environment, driver, parent, id, rectangle)
{

    vHorizontal = core::vector3df(0, 0, 1);
    vVertical = core::vector3df(1, 0, 0);
    vAxis = core::vector3df(0, 1, 0);

    geo_scene = new geometry_scene(device->getVideoDriver(), (MyEventReceiver*)device->getEventReceiver());
    geo_scene->geoNode()->setRenderType(true, false, false, false);
    //geo_scene->geoNode()->setVisible(false);
}

UV_Editor_Panel::~UV_Editor_Panel()
{
    if (geo_scene)
        delete geo_scene;
}

void UV_Editor_Panel::Initialize(geometry_scene* real_geo_scene)
{
    //Unfortunate Name Collisions :(
    //Be careful !!!

    smgr = real_geo_scene->get_smgr();
    real_g_scene = real_geo_scene;

    geo_scene->geoNode()->initialize(real_geo_scene->get_smgr(), (MyEventReceiver*)device->getEventReceiver());
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

void UV_Editor_Panel::showTexture(bool b)
{
    show_texture = b;
}

void UV_Editor_Panel::showGrid(bool b)
{
    show_grid = b;
}

void UV_Editor_Panel::setGroupSelect(bool b)
{
    group_select = b;
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


bool UV_Editor_Panel::OnEvent(const SEvent& event)
{
    GeometryStack* real_geo_node = real_g_scene->geoNode();

    switch (event.EventType)
    {
    case EET_USER_EVENT:
        switch (event.UserEvent.UserData1)
        {
            case USER_EVENT_TEXTURE_UVS_MODIFIED:
            case USER_EVENT_SELECTION_CHANGED:
            {
                vector<int> selection = real_g_scene->getSelectedFaces();

                faces_shown = 0;

                if (selection.size() > 0)
                {
                    int brush_j = real_geo_node->get_total_geometry()->faces[selection[0]].original_brush;
                    int face_j = real_geo_node->get_total_geometry()->faces[selection[0]].original_face;

                    poly_face* f = &real_geo_node->elements[brush_j].brush.faces[face_j];

                    if (group_select)
                    {
                        if ((real_geo_node->elements[brush_j].brush.surface_groups[f->surface_group].type == SURFACE_GROUP_CYLINDER ||
                            real_geo_node->elements[brush_j].brush.surface_groups[f->surface_group].type == SURFACE_GROUP_SPHERE ||
                            real_geo_node->elements[brush_j].brush.surface_groups[f->surface_group].type == SURFACE_GROUP_DOME
                            ))
                        {
                            real_g_scene->selectSurfaceGroup();
                        }
                        selection = real_g_scene->getSelectedFaces();
                    }
                    
                    geo_scene->geoNode()->elements.clear();

                    polyfold* pf = real_geo_node->get_total_geometry();

                    first_texcoord = true;

                    faces_shown = selection.size();

                    original_brushes.clear();
                    brush_vertices_index.clear();
                    surface_groups.clear();
                    brush_faces_index.clear();
                    faces_index.clear();

                    for (int f_i : selection)
                    {
                        int og_brush = pf->faces[f_i].original_brush;
                        int og_face = pf->faces[f_i].original_face;
                        int og_sfg = real_geo_node->elements[og_brush].geometry.faces[og_face].surface_group;

                        surface_group* sfg = &real_geo_node->elements[og_brush].geometry.surface_groups[og_sfg];

                        bool b = false;

                        if(sfg->type != SURFACE_GROUP_STANDARD)
                            for (int i=0;i<original_brushes.size();i++)
                            {
                                if (original_brushes[i] == og_brush && surface_groups[i] == og_sfg )
                                {
                                    b = true;
                                    brush_faces_index[i].push_back(og_face);
                                    faces_index[i].push_back(f_i);
                                }
                            }

                        if (!b)
                        {
                            original_brushes.push_back(og_brush);
                            surface_groups.push_back(og_sfg);

                            geo_scene->geoNode()->elements.push_back(geo_element{});

                            int n_brush_vertices = real_geo_node->elements[og_brush].geometry.vertices.size();

                            brush_vertices_index.push_back(vector<int>{});
                            brush_vertices_index[brush_vertices_index.size() - 1].assign(n_brush_vertices, -1);

                            brush_faces_index.push_back(vector<int>{og_face});
                            faces_index.push_back(vector<int>{f_i});
                        }
                    }

                    for (int f_i : selection)
                    {
                        make_face(pf, f_i, NULL);
                    }

                    if (my_image)
                        delete my_image;
                    my_image = NULL;

                    core::stringw texname = real_geo_node->get_original_brush_face(selection[0])->texture_name;
                    video::ITexture* my_texture = device->getVideoDriver()->getTexture(texname);

                    my_image = new TextureImage(my_texture, true);


                    //===================================================
                    // Center the Camera
                    //
                    if (faces_shown > 0 && m_viewPanel != NULL)
                    {
                        f32 window_ratio = (f32)this->Texture->getOriginalSize().Width / (f32)this->Texture->getOriginalSize().Height;
                        core::matrix4 M;

                        f32 uv_width = BR_texcoord.X - UL_texcoord.X;
                        f32 uv_height = BR_texcoord.Y - UL_texcoord.Y;
                        
                        u16 pixel_height;

                        if (uv_width / uv_height < window_ratio)
                            pixel_height = (uv_height) * 512;
                        else
                            pixel_height = ((uv_width) * 512) / (window_ratio);

                        pixel_height *= 1.05;

                        if (window_ratio >= 1.0)
                        {
                            M.buildProjectionMatrixOrthoLH(pixel_height * window_ratio, pixel_height, 0, 10000);

                            viewSize = dimension2du(pixel_height * window_ratio, pixel_height);
                        }
                        else
                        {
                            M.buildProjectionMatrixOrthoLH(pixel_height, pixel_height / window_ratio, 0, 10000);

                            viewSize = dimension2du(pixel_height, pixel_height / window_ratio);
                        }

                        vector3df move_pos = vector3df((UL_texcoord.Y + 0.5 * (BR_texcoord.Y - UL_texcoord.Y)) * 512,
                            5000,
                            (UL_texcoord.X + 0.5 * (BR_texcoord.X - UL_texcoord.X)) * 512);

                        vector3df look_pos = vector3df((UL_texcoord.Y + 0.5 * (BR_texcoord.Y - UL_texcoord.Y)) * 512,
                            0,
                            (UL_texcoord.X + 0.5 * (BR_texcoord.X - UL_texcoord.X)) * 512);

                        getCamera()->setProjectionMatrix(M, true);

                        this->getCamera()->setPosition(move_pos);
                        this->getCamera()->setTarget(look_pos);
                        this->getCamera()->updateAbsolutePosition();
                        
                    }
                }
                else
                {
                    geo_scene->setBrushSelection(std::vector<int>{});
                    geo_scene->geoNode()->elements.clear();
                    if (geo_scene)
                    {
                        geo_scene->geoNode()->elements.clear();
                    }
                }
                return true;
            }break;

        } break;
    }

    return TestPanel_2D::OnEvent(event);
}

void UV_Editor_Panel::make_face(polyfold* pf_0, int f_no, video::ITexture* face_texture2)
{
    MeshBuffer_Chunk chunk = real_g_scene->geoNode()->edit_meshnode_interface.get_mesh_buffer_by_face(f_no);

    int original_face = pf_0->faces[f_no].original_face;
    int original_brush = pf_0->faces[f_no].original_brush;
    int original_sfg = real_g_scene->geoNode()->elements[original_brush].geometry.faces[original_face].surface_group;

    polyfold* uv_poly = NULL;
    vector<int>* vertex_index = NULL;

    for (int i = 0; i < original_brushes.size(); i++)
    {
        if (original_brushes[i] == original_brush && surface_groups[i] == original_sfg && 
            std::any_of(brush_faces_index[i].begin(), brush_faces_index[i].end(), [&](int n) {return n == original_face; }))
        {
            uv_poly = &geo_scene->geoNode()->elements[i].brush;
            vertex_index = &brush_vertices_index[i];
        }
    }

    scene::IMeshBuffer* buffer = chunk.buffer;
    if (buffer)
    {
        polyfold* source_brush;

        int c = 0;
        
        bool use_geometry_brush = true;

        /*
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
        */

        if (use_geometry_brush)
        {
            source_brush = &real_g_scene->geoNode()->elements[original_brush].geometry;
        }

        for (int i = chunk.begin_i; i < chunk.end_i; i += 3)
        {

            u32 v0 = buffer->getIndices()[i];
            u32 v1 = buffer->getIndices()[i + 1];
            u32 v2 = buffer->getIndices()[i + 2];

            video::S3DVertex2TCoords* vtx0 = &((video::S3DVertex2TCoords*)buffer->getVertices())[v0];
            video::S3DVertex2TCoords* vtx1 = &((video::S3DVertex2TCoords*)buffer->getVertices())[v1];
            video::S3DVertex2TCoords* vtx2 = &((video::S3DVertex2TCoords*)buffer->getVertices())[v2];

            if (first_texcoord)
            {
                UL_texcoord = -vtx0->TCoords;
                BR_texcoord = -vtx0->TCoords;
                first_texcoord = false;
            }

            MIN_OF(UL_texcoord.X, -vtx0->TCoords.X, -vtx1->TCoords.X, -vtx2->TCoords.X);
            MIN_OF(UL_texcoord.Y, -vtx0->TCoords.Y, -vtx1->TCoords.Y, -vtx2->TCoords.Y);

            MAX_OF(BR_texcoord.X, -vtx0->TCoords.X, -vtx1->TCoords.X, -vtx2->TCoords.X);
            MAX_OF(BR_texcoord.Y, -vtx0->TCoords.Y, -vtx1->TCoords.Y, -vtx2->TCoords.Y);

            core::vector3df pos(0, 0, 0);

            pos.Z = -vtx0->TCoords.X * 512.0f;// *texture_size.Width;
            pos.X = -vtx0->TCoords.Y * 512.0f;// *texture_size.Height;
            int a = uv_poly->get_point_or_add(pos);

            int a_indx = source_brush->find_point(vtx0->Pos);
            (*vertex_index)[a_indx] = a;


            pos.Z = -vtx1->TCoords.X * 512.0f;// *texture_size.Width;
            pos.X = -vtx1->TCoords.Y * 512.0f;// *texture_size.Height;
            int b = uv_poly->get_point_or_add(pos);

            int b_indx = source_brush->find_point(vtx1->Pos);
            (*vertex_index)[b_indx] = b;


            pos.Z = -vtx2->TCoords.X * 512.0f;// *texture_size.Width;
            pos.X = -vtx2->TCoords.Y * 512.0f;// *texture_size.Height;
            int c = uv_poly->get_point_or_add(pos);

            int c_indx = source_brush->find_point(vtx2->Pos);
            (*vertex_index)[c_indx] = c;


            int e0 = uv_poly->get_edge_or_add(a, b, 0);
            int e1 = uv_poly->get_edge_or_add(b, c, 0);
            int e2 = uv_poly->get_edge_or_add(a, c, 0);
        }
    }
}


void UV_Editor_Panel::make_custom_surface_group()
{
    vector<int> selection = geo_scene->getBrushSelection();

    if(selection.size() > 0)
    {
        int p_i = selection[0];
        int brush_j = original_brushes[p_i];
        int sfg_j = surface_groups[p_i];
        vector<int> faces = brush_faces_index[p_i];

        polyfold* pf = &real_g_scene->geoNode()->elements[brush_j].brush;
        polyfold* uv_poly = &geo_scene->geoNode()->elements[p_i].brush;

        surface_group* sfg = &pf->surface_groups[sfg_j];

        if (sfg->type != SURFACE_GROUP_CUSTOM_UVS_GEOMETRY)
        {
            surface_group sg;
            sg.type = SURFACE_GROUP_CUSTOM_UVS_GEOMETRY;
            pf->surface_groups.push_back(sg);
            int new_sfg = pf->surface_groups.size() - 1;

            for (int f_i : faces)
            {
                poly_face* f = &pf->faces[f_i];
                f->surface_group = new_sfg;
            }

            sfg = &pf->surface_groups[new_sfg];
        }

        sfg->texcoords.clear();

        for (int i = 0; i < brush_vertices_index[p_i].size(); i++)
        {
            int v_i = brush_vertices_index[p_i][i];

            if (v_i != -1)
            {
                core::vector2df pos;
                pos.Y = -uv_poly->vertices[v_i].V.X / 512.0;
                pos.X = -uv_poly->vertices[v_i].V.Z / 512.0;
                point_texcoord tc;
                tc.vertex = i;
                tc.texcoord = pos;
                sfg->texcoords.push_back(tc);
            }

        }

        real_g_scene->geoNode()->elements[brush_j].geometry.surface_groups = real_g_scene->geoNode()->elements[brush_j].brush.surface_groups;

        for (int i=0; i<faces.size(); i++)
        {
            real_g_scene->geoNode()->edit_meshnode_interface.recalc_uvs_for_face_custom(real_g_scene->geoNode(), brush_j, brush_faces_index[p_i][i], faces_index[p_i][i]);
            real_g_scene->geoNode()->final_meshnode_interface.recalc_uvs_for_face_custom(real_g_scene->geoNode(), brush_j, brush_faces_index[p_i][i], faces_index[p_i][i]);
        }
       
    }
}


void UV_Editor_Panel::drawGrid(video::IVideoDriver* driver, const video::SMaterial material)
{
    //driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
    //driver->setMaterial(material);

    int far_value = this->getCamera()->getFarValue();
    far_value *= -1;

    int gridSpace = 512;

    {
        int interval = gridSpace;
        while (viewSize.Width / interval > 36 || viewSize.Height / interval > 36)
        {
          //  interval = interval << 1;
        }
        int h_lines = viewSize.Width / interval;
        int v_lines = viewSize.Height / interval;
        core::vector3df vDownLeft = this->getCamera()->getAbsolutePosition();
        vDownLeft.Z -= viewSize.Width / 2;
        vDownLeft.X -= viewSize.Height / 2;
        int start_x = interval * ((int)vDownLeft.Z / interval);
        int start_y = interval * ((int)vDownLeft.X / interval);

        video::SColor col = video::SColor(96, 96, 96, 96);
        
        for (int i = -1; i < h_lines + 2; i++)
        {
            driver->draw3DLine(core::vector3df(vDownLeft.X, far_value, start_x + i * interval), core::vector3df(vDownLeft.X + viewSize.Height, far_value, start_x + i * interval), col);
        }
        for (int i = -1; i < v_lines + 2; i++)
        {
            driver->draw3DLine(core::vector3df(start_y + i * interval, far_value, vDownLeft.Z), core::vector3df(start_y + i * interval, far_value, vDownLeft.Z + viewSize.Width), col);
        }
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

    if (show_texture && my_image)
    {
        my_image->render();
    }

    driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
    driver->setMaterial(someMaterial);

    if (show_grid)
        this->drawGrid(driver, someMaterial);

    if (this->geo_scene)
    {
        geo_scene->geoNode()->render();
    }

    driver->setRenderTarget(0, true, true, video::SColor(0, 0, 0, 0));
}



void UV_Editor_Panel::OnMenuItemSelected(IGUIContextMenu* menu)
{
}


void UV_Editor_Panel::left_click(core::vector2di pos)
{
    vector<int> selection = std::vector<int>{};
    GeometryStack* geo_node = geo_scene->geoNode();

    geo_scene->setBrushSelection(std::vector<int>{});

    for (int i = 0; i < geo_node->elements.size(); i++)
    {
        if (click_hits_poly(&geo_node->elements[i].brush, core::vector2di(clickx, clicky)))
        {
            geo_scene->setBrushSelection(std::vector<int>{i});
        }
    }
}


void UV_Editor_Panel::right_click(core::vector2di pos)
{
    GeometryStack* geo_node = geo_scene->geoNode();

    if (geo_scene && geo_scene->getBrushSelection().size() > 0)
    {
        bool bVertexClick = false;
        for (int p_i : geo_scene->getBrushSelection())
        {
            if (click_hits_poly(&geo_node->elements[p_i].brush, core::vector2di(clickx, clicky)))
            {
                for (int v_i = 0; v_i < geo_node->elements[p_i].brush.vertices.size(); v_i++)
                {
                    core::vector2di coords;
                    GetScreenCoords(geo_node->elements[p_i].brush.vertices[v_i].V, coords);
                    if (core::vector2di(clickx, clicky).getDistanceFrom(coords) < 4)
                    {
                        //geo_scene->selected_brush_vertex_editing = p_i;
                        geo_scene->set_selected_brush_vertex_editing(p_i);
                        geo_node->elements[p_i].selected_vertex = v_i;
                        bVertexClick = true;
                    }
                }
            }
        }
    }
}


void UV_Editor_Panel::setGridSpacing(int spacing)
{
    //gridSpace = spacing;
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

//=============================================================
// UV Editor Widget
//


UV_Editor_Widget::UV_Editor_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene* g_scene_, UV_Editor_Base* base_, s32 id, core::rect<s32> rect)
    : gui::IGUIElement(gui::EGUIET_ELEMENT, env, parent, id, rect), my_base(base_), g_scene(g_scene_), my_ID(id)
{
    MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
    receiver->Register(this);
}


UV_Editor_Widget::~UV_Editor_Widget()
{
    //std::cout << "Out of scope (Material Buffers Widget)\n";

    MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
    receiver->UnRegister(this);

    my_base->close_uv_panel();

}


void UV_Editor_Widget::show()
{
    core::rect<s32> pr(0, 0, getRelativePosition().getWidth(), getRelativePosition().getHeight());
    edit_panel = new gui::IGUIElement(gui::EGUIET_ELEMENT, Environment, this, -1, pr);

    OK_BUTTON_ID = my_ID + 1;

    my_widget = new Reflected_Widget_EditArea(Environment, edit_panel, g_scene, my_base, my_ID + 2, pr);
    my_widget->show(true, my_base->getObj());

    int ypos = my_widget->getEditAreaHeight() + 8;

    core::rect<s32> pr2(0, ypos, getRelativePosition().getWidth(), ypos + 120);

    core::rect<s32> br = core::rect<s32>(getRelativePosition().getWidth() - 80, ypos, getRelativePosition().getWidth() - 8, ypos + 36);

    my_button = new Flat_Button(Environment, this, OK_BUTTON_ID, br);
    my_button->setText(L"Apply");

    edit_panel->drop();
    my_widget->drop();

}

void UV_Editor_Widget::onRefresh()
{
    int ypos = my_widget->getEditAreaHeight() + 8;

    //Button
    if (my_button)
        my_button->remove();

    core::rect<s32> br = core::rect<s32>(getRelativePosition().getWidth() - 80, ypos, getRelativePosition().getWidth() - 8, ypos + 36);

    my_button = new Flat_Button(Environment, this, OK_BUTTON_ID, br);
    my_button->setText(L"Apply");

    //Text
    if (my_text)
        my_text->remove();

    core::rect<s32> tr = core::rect<s32>(32, ypos, getRelativePosition().getWidth() - 80, ypos + 24);

    std::wstringstream ss;
    ss << my_base->GetSelectedString().c_str();
    my_text = Environment->addStaticText(ss.str().c_str(), tr, false, false, this);

    ypos += 32;

    //Image
    if (my_image)
        my_image->remove();

    core::rect<s32> ir = core::rect<s32>(32, ypos, 32 + 128, ypos + 128);

}

bool UV_Editor_Widget::OnEvent(const SEvent& event)
{
    if (event.EventType == EET_USER_EVENT)
    {
        if (event.UserEvent.UserData1 == USER_EVENT_REFLECTED_FORM_REFRESHED)
        {
            onRefresh();
            return true;
        }
        else if (event.UserEvent.UserData1 == USER_EVENT_SELECTION_CHANGED)
        {
            my_base->m_struct.nFaces = g_scene->getSelectedFaces().size();

            if (my_widget)
            {
                my_widget->remove();
            }

            show();

            onRefresh();
            return true;
        }
    }
    else if (event.EventType == EET_GUI_EVENT)
    {
        s32 id = event.GUIEvent.Caller->getID();

        if (event.GUIEvent.EventType == EGET_BUTTON_CLICKED)
        {
            if (event.GUIEvent.Caller == my_button)
            {
                click_OK();
                return true;
            }
            else
            {
                FormField* field = my_widget->form->getFieldFromId(id);

                if (field)
                {
                    if (field->getButtonType() == FORM_FIELD_LABEL)
                    {

                        std::vector<int> leaf = field->tree_pos;
                        leaf.push_back(0); //node_class_item.id

                        reflect::Member* m = my_widget->m_typeDesc->getTreeNode(leaf);
                        size_t offset = my_widget->m_typeDesc->getTreeNodeOffset(leaf);

                        int sel;

                        m->type->copy(&sel, (char*)((char*)my_widget->temp_object) + offset);

                        my_base->select(sel);

                        my_widget->refresh();
                        onRefresh();

                        g_scene->set_selected_material_group(sel);

                        MyEventReceiver* event_receiver = (MyEventReceiver*)device->getEventReceiver();
                        SEvent event;
                        event.EventType = EET_USER_EVENT;
                        event.UserEvent.UserData1 = USER_EVENT_MATERIAL_GROUP_SELECTION_CHANGED;
                        event_receiver->OnEvent(event);

                        return true;
                    }
                }
            }
            return true;
        }
        else if (event.GUIEvent.EventType == EGET_CHECKBOX_CHANGED)
        {
            my_widget->write();
            my_base->refresh_panel_view();
        }
        else if (event.GUIEvent.EventType == EGET_ELEMENT_FOCUS_LOST)
        {
            FormField* field = my_widget->form->getFieldFromId(id);

            if (field)
            {
                if(strcmp(field->text.c_str(),"snap_dist") == 0)
                {
                    my_widget->write();
                    my_base->refresh_panel_view();
                }
            }
        }
    }

    return IGUIElement::OnEvent(event);
}

void UV_Editor_Widget::click_OK()
{
    my_base->write_uvs();
}


//============================================================
// UV Editor Base
//

UV_Editor_Base::UV_Editor_Base(std::wstring name, int my_id, gui::IGUIEnvironment* env, multi_tool_panel* panel)
    :simple_reflected_tool_base(name,my_id,env,panel)
{
    m_typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<uv_editor_struct>::get();
}

UV_Editor_Base::~UV_Editor_Base()
{
    if (uv_edit)
        delete uv_edit;
}

void UV_Editor_Base::set_scene(geometry_scene* g_scene_)
{
    tool_base::set_scene(g_scene_);

    if (uv_edit)
        delete uv_edit;

    uv_edit = new UV_Editor_Panel(env, device->getVideoDriver(), NULL, 0, core::recti());
    uv_edit->Initialize(g_scene);
}

void UV_Editor_Base::show()
{
    core::rect<s32> client_rect(core::vector2di(0, 0),
        core::dimension2du(this->panel->getClientRect()->getAbsolutePosition().getWidth(),
            this->panel->getClientRect()->getAbsolutePosition().getHeight()));

    UV_Editor_Widget* widget = new UV_Editor_Widget(env, this->panel->getClientRect(), g_scene, this, GUI_ID_MAT_BUFFERS_BASE, client_rect);

    widget->show();
    widget->drop();

    SEvent event;
    event.EventType = EET_USER_EVENT;
    event.UserEvent.UserData1 = USER_EVENT_SELECTION_CHANGED;
    widget->OnEvent(event);

    refresh_panel_view();
    uv_edit->OnEvent(event);
}

void UV_Editor_Base::select(int sel)
{
    selection = sel;
}

void UV_Editor_Base::close_uv_panel()
{
    if (uv_edit && uv_edit->getViewPanel())
        uv_edit->getViewPanel()->disconnect();
}

void UV_Editor_Base::refresh_panel_view()
{
    if (cameraQuad && uv_edit)
    {
        if (m_struct.show_uv_view && uv_edit->hooked_up() == false)
        {
            cameraQuad->hookup_aux_panel(uv_edit);
        }
        else if (m_struct.show_uv_view == false && uv_edit->hooked_up() == true)
        {
            uv_edit->getViewPanel()->disconnect();
        }

        if (uv_edit->hooked_up() == true)
        {
            uv_edit->showTexture(m_struct.show_texture);
            uv_edit->showGrid(m_struct.show_grid);
            uv_edit->setGridSnap(m_struct.snap_dist);
            uv_edit->setGroupSelect(m_struct.group_select);
        }
    }
}

void UV_Editor_Base::init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos)
{
    reflect::Member* m = flat_typeDescriptor->getTreeNode(tree_pos);
    //material_group_struct::my_typeDesc* m_type = (material_group_struct::my_typeDesc*)(m->type);
    m->readwrite = true;

    if (tree_pos.size() > 1)
    {
        //m_type->my_attributes.selected = m_struct.material_groups[tree_pos[1]].selected;
    }
}

void UV_Editor_Base::write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor)
{
    /*
    for (int i = 0; i < m_struct.material_groups.size(); i++)
    {
        std::vector<int> tree_pos{ 1, i };

        reflect::Member* m = flat_typeDescriptor->getTreeNode(tree_pos);

        material_group_struct::my_typeDesc* m_type = (material_group_struct::my_typeDesc*)(m->type);
        m_type->my_attributes.selected = m_struct.material_groups[i].selected;
    }*/
}

std::string UV_Editor_Base::GetSelectedString()
{
    /*
    if (selection != -1)
    {
        int mg = m_struct.material_groups[selection].material_group;
        return Material_Groups_Tool::get_base()->getMaterialGroup(mg).name;
    }*/
    return std::string();
}

video::ITexture* UV_Editor_Base::GetSelectedTexture()
{
    return nullptr;
}

void UV_Editor_Base::write_uvs()
{
    uv_edit->make_custom_surface_group();
}


//======================================================
// Texture Image for UV Editor
//


TextureImage::TextureImage(video::ITexture* texture, bool repeat)
{
    m_texture = texture;

    Buffer = new scene::SMeshBuffer();

    Buffer->Material.Lighting = false;
    Buffer->Material.setTexture(0, m_texture);
    Buffer->Material.MaterialType = video::EMT_TRANSPARENT_ADD_COLOR;
    //Buffer->Material.MaterialType = video::EMT_SOLID;
    //MaterialType = video::EMT_SOLID;

    //core::dimension2du size = m_texture->getOriginalSize();
    core::dimension2du size = repeat ? core::dimension2du(5120, 5120) : core::dimension2du(512,512);

    Buffer->Vertices.set_used(4);
    Buffer->Indices.set_used(6);

    Buffer->Indices[0] = (u16)0;
    Buffer->Indices[1] = (u16)2;
    Buffer->Indices[2] = (u16)1;
    Buffer->Indices[3] = (u16)0;
    Buffer->Indices[4] = (u16)3;
    Buffer->Indices[5] = (u16)2;

    f32 t_scale = repeat ? 10.0 : 1.0;
    vector3df pos_offset = repeat? vector3df(2560.0,0,2560.0) : vector3df(0,0,0);

    Buffer->Vertices[0].Pos = core::vector3df(0,0,0) + pos_offset;
    Buffer->Vertices[0].Color = video::SColor(255, 255, 255, 255);
    Buffer->Vertices[0].Normal = core::vector3df(0,t_scale,0);
    Buffer->Vertices[0].TCoords = core::vector2df(0, 0);

    Buffer->Vertices[1].Pos = core::vector3df(-(int)size.Width, 0, 0) + pos_offset;
    Buffer->Vertices[1].Color = video::SColor(255, 255, 255, 255);
    Buffer->Vertices[1].Normal = core::vector3df(0, t_scale, 0);
    Buffer->Vertices[1].TCoords = core::vector2df(0, t_scale);

    Buffer->Vertices[2].Pos = core::vector3df(-(int)size.Width, 0, -(int)size.Height) + pos_offset;
    Buffer->Vertices[2].Color = video::SColor(255, 255, 255, 255);
    Buffer->Vertices[2].Normal = core::vector3df(0, t_scale, 0);
    Buffer->Vertices[2].TCoords = core::vector2df(t_scale, t_scale);

    Buffer->Vertices[3].Pos = core::vector3df(0, 0, -(int)size.Height) + pos_offset;
    Buffer->Vertices[3].Color = video::SColor(255, 255, 255, 255);
    Buffer->Vertices[3].Normal = core::vector3df(0, t_scale, 0);
    Buffer->Vertices[3].TCoords = core::vector2df(t_scale, 0);
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
        Buffer->getIndices(), 2, video::EVT_STANDARD, scene::EPT_TRIANGLES, Buffer->getIndexType());
}

core::dimension2du TextureImage::getDimensions()
{
    if (m_texture)
        return m_texture->getOriginalSize();

    return core::dimension2du();
}


//===========================================================
// Reflection
//

REFLECT_STRUCT_BEGIN(uv_editor_struct)
    REFLECT_STRUCT_MEMBER(nFaces)
        REFLECT_STRUCT_MEMBER_FLAG(FLAG_NON_EDITABLE)
    REFLECT_STRUCT_MEMBER(show_uv_view)
    REFLECT_STRUCT_MEMBER(show_texture)
    REFLECT_STRUCT_MEMBER(show_grid)
    REFLECT_STRUCT_MEMBER(group_select)
    REFLECT_STRUCT_MEMBER(snap_dist)
REFLECT_STRUCT_END()

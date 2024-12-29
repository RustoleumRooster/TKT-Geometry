#include "GUI_tools.h"
#include <irrlicht.h>
#include "Reflection.h"
#include "lightmaps_tool.h"
#include "CGUIWindow.h"
#include "geometry_scene.h"
#include "BufferManager.h"
#include "utils.h"
#include "uv_mapping.h"
#include "LightMaps.h"
#include "ex_gui_elements.h"
#include <sstream>
#include "reflect_custom_types.h"
#include "material_groups.h"
#include "edit_env.h"
//#include "edit_env.h"

using namespace std;
using namespace gui;

//LM_Viewer_Base* LM_Viewer_Tool::base = NULL;
//multi_tool_panel* LM_Viewer_Tool::panel = NULL;

Material_Buffers_Base* Material_Buffers_Tool::base = NULL;
multi_tool_panel* Material_Buffers_Tool::panel = NULL;

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
// UV Editor Camera Panel
//

LM_Viewer_Panel::LM_Viewer_Panel(IGUIEnvironment* environment, video::IVideoDriver* driver, IGUIElement* parent, LM_Viewer_Window* win, s32 id, core::rect<s32> rectangle)
    : TestPanel_2D(environment, driver, parent, id, rectangle)
{
    my_window = win;
    vHorizontal = core::vector3df(0, 0, 1);
    vVertical = core::vector3df(1, 0, 0);
    vAxis = core::vector3df(0, 1, 0);

   // uv_scene = new geometry_scene(device->getVideoDriver(), (MyEventReceiver*)device->getEventReceiver());
   // uv_scene = new GeometryStack(NULL,Scen)
}

void LM_Viewer_Panel::Initialize(geometry_scene* geo_scene)
{
    uv_scene = new GeometryStack();// (NULL, smgr, (MyEventReceiver*)device->getEventReceiver());
    uv_scene->initialize(geo_scene->get_smgr(), (MyEventReceiver*)device->getEventReceiver());
    TestPanel::Initialize(geo_scene);
}

LM_Viewer_Panel::~LM_Viewer_Panel()
{
    if (uv_scene)
        delete uv_scene;
}

void LM_Viewer_Panel::resize(core::dimension2d<u32> new_size)
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

void LM_Viewer_Panel::showTriangles(bool bShowTriangles)
{
    bShowBrushes = bShowTriangles;
}

void LM_Viewer_Panel::showLightmap(bool bShowLightmap)
{
    bRenderLightmap = bShowLightmap;
}

scene::ICameraSceneNode* LM_Viewer_Panel::getCamera()
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

void LM_Viewer_Panel::showMaterialGroup(int mg_n)
{
    if (uv_scene)
    {
        uv_scene->elements.clear();
        faces.clear();

        polyfold* pf = geo_scene->geoNode()->get_total_geometry();

        const vector<TextureMaterial>& mat_groups = geo_scene->geoNode()->final_meshnode_interface.getMaterialsUsed();

        if (mg_n < mat_groups.size())
        {
            current_mat_group = mg_n;
            //current_material = mat_groups[mg_n];

            for (int f_i : mat_groups[mg_n].faces)
            {
                make_face(pf, f_i, NULL);
                faces.push_back(f_i);
            }

            vector<int> selection = geo_scene->getSelectedFaces();
            for (int i = 0; i < faces.size(); i++)
            {
                for (int f_j : selection)
                {
                    if (f_j == faces[i] && i < uv_scene->elements.size())
                    {
                        uv_scene->elements[i].bSelected = true;
                    }
                }
            }

            
            if (bRenderLightmap)
            {
                
                if (my_image)
                    delete my_image;
                my_image = NULL;

                vector<video::ITexture*>& lm_textures = Lightmaps_Tool::get_manager()->lightmap_textures;

                if (mg_n < lm_textures.size())
                {
                    my_image = new TextureImage(lm_textures[mg_n]);
                }
            }
        }

        
    }
}


bool LM_Viewer_Panel::OnEvent(const SEvent& event)
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

            case USER_EVENT_CLEAR_LIGHTMAP_TEXTURES:
            {
                my_image = NULL;

            } break;

            case USER_EVENT_SELECTION_CHANGED:
            {
                vector<int> selection = geo_scene->getSelectedFaces();

                for (int i = 0; i < faces.size(); i++)
                {
                    uv_scene->elements[i].bSelected = false;
                    for (int f_j : selection)
                    {
                        if (f_j == faces[i] && i < uv_scene->elements.size())
                        {
                            uv_scene->elements[i].bSelected = true;
                        }
                    }
                }

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

void LM_Viewer_Panel::make_face(polyfold* pf_0, int f_no, video::ITexture* face_texture2)
{
    MeshBuffer_Chunk chunk = geo_scene->geoNode()->edit_meshnode_interface.get_mesh_buffer_by_face(f_no);

    my_face_no = f_no;

    original_face = pf_0->faces[f_no].original_face;
    original_brush = pf_0->faces[f_no].original_brush;

    polyfold uv_poly;

    scene::IMeshBuffer* buffer = chunk.buffer;
    if (buffer)
    {
        polyfold* source_brush = &geo_scene->geoNode()->elements[original_brush].brush;
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
            source_brush = &geo_scene->geoNode()->elements[original_brush].geometry;

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


void LM_Viewer_Panel::drawGrid(video::IVideoDriver* driver, const video::SMaterial material)
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

void LM_Viewer_Panel::render()
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
                    int f_i = faces[e_i];

                    //for (int i = 0; i < current_material.records.size(); i++)
                    {
                    //    if (f_i == current_material.faces[i])
                        {
                            //....
                        }
                    }

                    core::vector2di coords;
                    for (int i = 0; i < geo->brush.vertices.size(); i++)
                    {
                        GetScreenCoords(geo->brush.vertices[i].V, coords);
                        coords.X -= 4;
                        coords.Y -= 4;/*
                        if (uv_scene->selected_brush_vertex_editing == e_i && geo->selected_vertex == i)
                        {
                            if (geo->type == GEO_ADD)
                                driver->draw2DImage(med_circle_tex_add_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                            else if (geo->type == GEO_SUBTRACT)
                                driver->draw2DImage(med_circle_tex_sub_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                            else if (geo->type == GEO_RED)
                                driver->draw2DImage(med_circle_tex_red_selected, coords, core::rect<int>(0, 0, 8, 8), 0, video::SColor(255, 255, 255, 255), true);
                        }
                        else*/
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



void LM_Viewer_Panel::OnMenuItemSelected(IGUIContextMenu* menu)
{
}




void LM_Viewer_Panel::left_click(core::vector2di pos)
{
    /*
    std::vector<int> old_sel_faces = geo_scene->getSelectedFaces();
    std::vector<Reflected_SceneNode*> old_sel_nodes = geo_scene->getSelectedNodes();
    std::vector<int> old_sel_brushes = geo_scene->getBrushSelection();

    if (click_hits_poly(&geo_scene->geoNode()->elements[0].brush, core::vector2di(clickx, clicky)))
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

    geo_scene->selectionChanged();*/
}


void LM_Viewer_Panel::right_click(core::vector2di pos)
{/*
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
    }*/
}


void LM_Viewer_Panel::setGridSpacing(int spacing)
{
    gridSpace = spacing;
}

/*
void LM_Viewer_Panel::centerCamera()
{
    this->getCamera()->setPosition(core::vector3df((int)texture_size.Width * 0.5, 5000, (int)texture_size.Height * 0.5));
    this->getCamera()->setTarget(core::vector3df((int)texture_size.Width * 0.5, 0, (int)texture_size.Height * 0.5));
}
*/

void LM_Viewer_Panel::setAxis(int axis)
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

//============================================================
//
//

REFLECT_CUSTOM_STRUCT_BEGIN(material_group_struct)
REFLECT_STRUCT_MEMBER(id)
REFLECT_STRUCT_MEMBER(nFaces)
REFLECT_STRUCT_MEMBER(nTriangles)
REFLECT_STRUCT_MEMBER(texture)
REFLECT_STRUCT_MEMBER(material_group)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(material_buffers_struct)
REFLECT_STRUCT_MEMBER(nBuffers)
REFLECT_STRUCT_MEMBER(material_groups)
REFLECT_STRUCT_END()


void material_group_struct::my_typeDesc::addFormWidget(Reflected_GUI_Edit_Form* win, TypeDescriptor_Struct* type_struct, std::vector<int> tree, size_t offset_base, bool bVisible, bool bEditable, int tab)
{

    int m_i = tree[tree.size() - 1];
    std::string name = type_struct->members[m_i].name;
    size_t offset = type_struct->members[m_i].offset + offset_base;

    if (m_i == 0)
    {
        Text_StaticField* f = new Text_StaticField();
        f->initInline("faces", tree, offset, 1, bVisible);
        win->addEditField(f);

        f = new Text_StaticField();
        f->initInline("triangles", tree, offset, 2, bVisible);
        win->addEditField(f);

        f = new Text_StaticField();
        f->init(" ", tree, offset, tab, bVisible);
        win->addEditField(f);
    }


    {
        Int_StaticField* f = new Int_StaticField();
        f->initInline("", tree, offset + 8, 1, bVisible);
        f->bBorder = true;
        win->addEditField(f);

        f = new Int_StaticField();
        f->initInline("", tree, offset + 16, 2, bVisible);
        f->bBorder = true;
        win->addEditField(f);
    }

    Text_StaticField* f = new Text_StaticLabel();

    //String_StaticField* f = new String_StaticField();
    f->bCanSelect = true;

    std::stringstream ss;
    ss << "buffer " << m_i;
    f->setText(ss.str().c_str());

    f->tab = (tab == -1 ? 0 : tab);
    f->setVisible(bVisible);
    f->tree_pos = tree;

    if (my_attributes.selected)
    {
        f->bHighlight = true;
    }

    win->addEditField(f);

    //TypeDescriptor_Struct::addFormWidget(win, type_struct, obj, tree, offset_base, bVisible, bEditable, tab);

}



Material_Buffers_Widget::Material_Buffers_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene* g_scene_, Material_Buffers_Base* base_, s32 id, core::rect<s32> rect)
    : gui::IGUIElement(gui::EGUIET_ELEMENT, env, parent, id, rect), my_base(base_), g_scene(g_scene_), my_ID(id)
{
    MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
    receiver->Register(this);
}


Material_Buffers_Widget::~Material_Buffers_Widget()
{
    //std::cout << "Out of scope (Material Buffers Widget)\n";

    MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
    receiver->UnRegister(this);

    my_base->close_uv_panel();

}


void Material_Buffers_Widget::show()
{
    core::rect<s32> pr(0, 0, getRelativePosition().getWidth(), getRelativePosition().getHeight());
    edit_panel = new gui::IGUIElement(gui::EGUIET_ELEMENT, Environment, this, -1, pr);

    OK_BUTTON_ID = my_ID + 1;

    my_widget = new Reflected_Widget_EditArea(Environment, edit_panel, g_scene, my_base, my_ID + 2, pr);
    //my_widget->setName("material groups widget");

    //reflect::TypeDescriptor_Struct* typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<material_buffers_struct>::get();

    my_widget->show(false, my_base->getObj());

    int ypos = my_widget->getEditAreaHeight() + 8;

    core::rect<s32> pr2(0, ypos, getRelativePosition().getWidth(), ypos + 120);

    options_form = new Reflected_GUI_Edit_Form(Environment, edit_panel, g_scene, my_ID + 3, pr2);
    reflect::TypeDescriptor_Struct* options_td = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<mb_tool_options_struct>::get();

    options_td->addFormWidget(options_form, NULL, vector<int>{0}, 0, true, true, 0);
    options_form->ShowWidgets(my_ID + 4);
    options_form->read(my_base->getOptions());

    ypos += my_widget->getFormsHeight();
    core::rect<s32> br = core::rect<s32>(getRelativePosition().getWidth() - 80, ypos, getRelativePosition().getWidth() - 8, ypos + 36);

    my_button = new Flat_Button(Environment, this, OK_BUTTON_ID, br);
    my_button->setText(L"Ok");

    edit_panel->drop();
    my_widget->drop();

}

void Material_Buffers_Widget::onRefresh()
{
    //Display options
    options_form->remove();

    int ypos = my_widget->getEditAreaHeight() + 8;

    core::rect<s32> pr2(0, ypos, getRelativePosition().getWidth(), ypos + 120);

    options_form = new Reflected_GUI_Edit_Form(Environment, edit_panel, g_scene, my_ID + 3, pr2);
    reflect::TypeDescriptor_Struct* options_td = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<mb_tool_options_struct>::get();

    options_td->addFormWidget(options_form, NULL, vector<int>{0}, 0, true, true, 0);
    options_form->ShowWidgets(my_ID + 4);
    options_form->read(my_base->getOptions());

    ypos += options_form->getTotalHeight() + 8;

    //Button
    if (my_button)
        my_button->remove();

    core::rect<s32> br = core::rect<s32>(getRelativePosition().getWidth() - 80, ypos, getRelativePosition().getWidth() - 8, ypos + 36);

    my_button = new Flat_Button(Environment, this, OK_BUTTON_ID, br);
    my_button->setText(L"Ok");

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

    video::ITexture* texture = my_base->GetSelectedTexture();

    if (texture)
    {
        my_image = Environment->addImage(ir, this, -1);
        my_image->setScaleImage(true);
        my_image->setImage(texture);
    }
}

bool Material_Buffers_Widget::OnEvent(const SEvent& event)
{
    if (event.EventType == EET_USER_EVENT)
    {
        if (event.UserEvent.UserData1 == USER_EVENT_REFLECTED_FORM_REFRESHED)
        {
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
            options_form->write(my_base->getOptions());
            my_base->refresh_panel_view();
        }
    }

    return IGUIElement::OnEvent(event);
}

void Material_Buffers_Widget::click_OK()
{
    //reflect::TypeDescriptor_Struct* td = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<geo_settings_struct>::get();

    //my_widget->write(td, my_base->getObj());
}

//============================================================
//
//

void Material_Buffers_Base::initialize()
{
    m_typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<material_buffers_struct>::get();

    if (uv_edit)
        delete uv_edit;

    uv_edit = new LM_Viewer_Panel(env, device->getVideoDriver(), NULL, NULL, 0, core::recti());
    uv_edit->Initialize(g_scene);
}

Material_Buffers_Base::~Material_Buffers_Base()
{
    if (uv_edit)
        delete uv_edit;
}

void Material_Buffers_Base::show()
{
    refresh_panel_view();

    std::vector<TextureMaterial> materials = g_scene->geoNode()->final_meshnode_interface.getMaterialsUsed();
    m_struct.nBuffers = materials.size();

    m_struct.material_groups.clear();
    for (int i = 0; i < m_struct.nBuffers; i++)
    {
        material_group_struct mgs;

        mgs.id = i;
        mgs.nTriangles = materials[i].n_triangles;
        mgs.nFaces = materials[i].n_faces;
        mgs.texture = materials[i].texture;
        mgs.material_group = materials[i].materialGroup;
        mgs.selected = false;
        m_struct.material_groups.push_back(mgs);
    }

    core::rect<s32> client_rect(core::vector2di(0, 0),
        core::dimension2du(this->panel->getClientRect()->getAbsolutePosition().getWidth(),
            this->panel->getClientRect()->getAbsolutePosition().getHeight()));

    Material_Buffers_Widget* widget = new Material_Buffers_Widget(env, this->panel->getClientRect(), g_scene, this, GUI_ID_MAT_BUFFERS_BASE, client_rect);

    widget->show();
    widget->drop();
}

void Material_Buffers_Base::select(int sel)
{
    selection = sel;

    for (int i = 0; i < m_struct.material_groups.size(); i++)
    {
        if (i == sel)
            m_struct.material_groups[i].selected = true;
        else
            m_struct.material_groups[i].selected = false;
    }

}

void Material_Buffers_Base::set_scene(geometry_scene* gs)
{
    tool_base::set_scene(gs);

    if (uv_edit)
        uv_edit->set_scene(gs);
}

void Material_Buffers_Base::close_uv_panel()
{
    if (uv_edit && uv_edit->getViewPanel())
        uv_edit->getViewPanel()->disconnect();
}

void Material_Buffers_Base::refresh_panel_view()
{
    if (cameraQuad && uv_edit)
    {
        if (mb_options.show_uv_view && uv_edit->hooked_up() == false)
        {
            cameraQuad->hookup_aux_panel(uv_edit);
        }
        else if (mb_options.show_uv_view == false && uv_edit->hooked_up() == true)
        {
            uv_edit->getViewPanel()->disconnect();
        }

        if (uv_edit->hooked_up() == true)
        {
            uv_edit->showTriangles(mb_options.show_triangles);
            uv_edit->showLightmap(mb_options.show_lightmap);

            MyEventReceiver* event_receiver = (MyEventReceiver*)device->getEventReceiver();
            SEvent event;
            event.EventType = EET_USER_EVENT;
            event.UserEvent.UserData1 = USER_EVENT_MATERIAL_GROUP_SELECTION_CHANGED;
            event_receiver->OnEvent(event);
        }

    }
}

void Material_Buffers_Base::init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos)
{
    reflect::Member* m = flat_typeDescriptor->getTreeNode(tree_pos);
    //material_group_struct::my_typeDesc* m_type = (material_group_struct::my_typeDesc*)(m->type);
    m->readwrite = true;

    if (tree_pos.size() > 1)
    {
        //m_type->my_attributes.selected = m_struct.material_groups[tree_pos[1]].selected;
    }
}

void Material_Buffers_Base::write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor)
{
    for (int i = 0; i < m_struct.material_groups.size(); i++)
    {
        std::vector<int> tree_pos{ 1, i };

        reflect::Member* m = flat_typeDescriptor->getTreeNode(tree_pos);

        material_group_struct::my_typeDesc* m_type = (material_group_struct::my_typeDesc*)(m->type);
        m_type->my_attributes.selected = m_struct.material_groups[i].selected;
    }
}

std::string Material_Buffers_Base::GetSelectedString()
{
    if (selection != -1)
    {
        int mg = m_struct.material_groups[selection].material_group;
        return Material_Groups_Tool::get_base()->getMaterialGroup(mg).name;
    }
    return std::string();
}

video::ITexture* Material_Buffers_Base::GetSelectedTexture()
{
    if (selection != -1)
    {
        return m_struct.material_groups[selection].texture;

    }

    return nullptr;
}

REFLECT_STRUCT_BEGIN(mb_tool_options_struct)
REFLECT_STRUCT_MEMBER(show_uv_view)
REFLECT_STRUCT_MEMBER(show_triangles)
REFLECT_STRUCT_MEMBER(show_lightmap)
REFLECT_STRUCT_END()


#include "texture_adjust.h"
#include "edit_classes.h"
#include "edit_env.h"
#include "geometry_scene.h"
#include <irrlicht.h>

using namespace irr;
using namespace gui;

extern IrrlichtDevice* device;

geometry_scene* Texture_Adjust_Tool::g_scene = NULL;
irr::gui::IGUIEnvironment* Texture_Adjust_Tool::env = NULL;
Texture_Adjust_Window::TextureAlignment Texture_Adjust_Window::tex_struct{0.01,0,0,1,1,0};
Texture_Adjust_Window::nSelected_struct Texture_Adjust_Window::sel_struct{2};

REFLECT_STRUCT_BEGIN(Texture_Adjust_Window::nSelected_struct)
    REFLECT_STRUCT_MEMBER(nSelected)
REFLECT_STRUCT_END()

REFLECT_MULTI_STRUCT_BEGIN(Texture_Adjust_Window::ChooseUVType)
    REFLECT_MULTI_STRUCT_LABEL("sphere")
    REFLECT_MULTI_STRUCT_LABEL("dome")
REFLECT_MULTI_STRUCT_END()


REFLECT_STRUCT_BEGIN(Texture_Adjust_Window::TextureAlignment)
    REFLECT_STRUCT_MEMBER(snap_dist)
    REFLECT_STRUCT_MEMBER(offset_x)
    REFLECT_STRUCT_MEMBER(offset_y)
    REFLECT_STRUCT_MEMBER(scale_x)
    REFLECT_STRUCT_MEMBER(scale_y)
    REFLECT_STRUCT_MEMBER(rotation)
    REFLECT_STRUCT_MEMBER(style)
REFLECT_STRUCT_END()

void Texture_Adjust_Window::refresh()
{
    if(form)
    {
        form->remove();
    }
    core::vector2di form_pos(8,60);
    form = new Reflected_GUI_Edit_Form(Environment,this,NULL,GUI_ID_REFLECTED_BASE,
                                            core::rect<s32>(form_pos,core::dimension2du(96,128)));
    bool editable = this->sel_struct.nSelected > 0;

    reflect::TypeDescriptor_Struct* typeDesc = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<TextureAlignment>::get();

    typeDesc->addFormWidget(form,NULL,std::vector<int>{},0,true,editable,-1);

    int next_ID = form->ShowWidgets(GUI_ID_REFLECTED_BASE+2);

    form->read(&tex_struct);
    form2->read(&sel_struct);

    bool bShowCombobox = false;

    GeometryStack* geo_node = g_scene->geoNode();

    if(editable && this->g_scene->getSelectedFaces().size() > 0 )
    {
        int b_i = this->g_scene->getSelectedFaces()[0];
        int brush_j = geo_node->get_total_geometry()->faces[b_i].original_brush;
        int face_j = geo_node->get_total_geometry()->faces[b_i].original_face;

        poly_face* f = &geo_node->elements[brush_j].brush.faces[face_j];

        int st = geo_node->elements[brush_j].brush.surface_groups[ f->surface_group ].type;

        if(st == SURFACE_GROUP_DOME || st == SURFACE_GROUP_SPHERE)
            bShowCombobox=true;
    }

    if(bShowCombobox == false)
    {
        form->getFieldFromId(STYLE_ID)->setActive(false);
        gui::IGUIComboBox* box = (gui::IGUIComboBox*)form->getFieldFromId(STYLE_ID)->getEditElement(0);
        box->setEnabled(false);
    }
}

void Texture_Adjust_Window::show()
{
    core::vector2di form_pos(8,60);
    core::vector2di form_pos2(8,28);

    form = new Reflected_GUI_Edit_Form(Environment,this,NULL,GUI_ID_REFLECTED_BASE,
                                            core::rect<s32>(form_pos,core::dimension2du(96,128)));
    form2 = new Reflected_GUI_Edit_Form(Environment,this,NULL,GUI_ID_REFLECTED_BASE+1,
                                            core::rect<s32>(form_pos2,core::dimension2du(96,128)));

    reflect::TypeDescriptor_Struct* typeDesc = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<TextureAlignment>::get();
   // reflect::TypeDescriptor* typeDesc2 = reflect::TypeResolver<nSelected_struct>::get();
    //typeDesc->addFormWidgets("", form,true);
    typeDesc->addFormWidget(form,NULL,std::vector<int>{},0,true,true,-1);
    //typeDesc2->addFormWidget("", form2,obj2,false);

    SNAP_ID = GUI_ID_REFLECTED_BASE+2;
    int next_ID = form->ShowWidgets(GUI_ID_REFLECTED_BASE+2);

    STYLE_ID = next_ID-1;

    //form->write_on_focus_lost=true;
    //next_ID = form2->ShowWidgets(next_ID);

    Int_StaticField* f = new Int_StaticField();

    f->setText("selected faces");

    //*obj2 = g_scene->getSelectedFaces().size();

    //f->obj = (int*)&sel_struct;
    f->offset=0;

    form2->addEditField(f);

    next_ID = form2->ShowWidgets(next_ID);

    sel_struct.nSelected = g_scene->getSelectedFaces().size();

    //form->read();
    //form2->read();

    //core::vector2di pos = getRelativePosition().UpperLeftCorner;
    //this->DesiredRect = core::rect<s32>(pos,core::dimension2d<u32>(196,form->getTotalHeight()+86));
    //this->recalculateAbsolutePosition(true);

    int ypos = form->getTotalHeight()+10;

    OK_BUTTON_ID=next_ID;
    ALIGN_BUTTON_ID = OK_BUTTON_ID + 1;

    Environment->addButton(core::rect<s32>(form_pos+core::vector2di(120,ypos),form_pos+core::vector2di(180,ypos+28)),this,OK_BUTTON_ID,L"Apply");

    ypos += 32;

    Environment->addButton(core::rect<s32>(form_pos + core::vector2di(0, ypos), form_pos + core::vector2di(60, ypos + 28)), this, ALIGN_BUTTON_ID, L"Align");
    
    bringToFront(form);

    IEventReceiver* receiver = device->getEventReceiver();

    SEvent event;
    event.EventType = EET_USER_EVENT;
    event.UserEvent.UserData1=USER_EVENT_TEXTURE_EDIT_MODE_BEGIN;
    receiver->OnEvent(event);

    event.EventType = EET_USER_EVENT;
    event.UserEvent.UserData1=USER_EVENT_SELECTION_CHANGED;
    this->OnEvent(event);
}

Texture_Adjust_Window::Texture_Adjust_Window(gui::IGUIEnvironment* env, gui::IGUIElement* parent,geometry_scene* g_scene_,s32 id,core::rect<s32> rect)
    : gui::CGUIWindow(env,parent,id,rect), g_scene(g_scene_)
{
    MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
    receiver->Register(this);
}

Texture_Adjust_Window::~Texture_Adjust_Window()
{
    MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
    receiver->UnRegister(this);

    SEvent event;
    event.EventType = EET_USER_EVENT;
    event.UserEvent.UserData1=USER_EVENT_TEXTURE_EDIT_MODE_END;
    receiver->OnEvent(event);
}

void  Texture_Adjust_Window::click_OK()
{
    std::vector<int> selection = this->g_scene->getSelectedFaces();

    this->form->write(&tex_struct);

    GeometryStack* geo_node = g_scene->geoNode();

    for(int b_i: selection)
    {
        int brush_j = geo_node->get_total_geometry()->faces[b_i].original_brush;
        int face_j = geo_node->get_total_geometry()->faces[b_i].original_face;

        poly_face* f = &geo_node->elements[brush_j].brush.faces[face_j];

        core::vector3df trans;
        trans.X=tex_struct.offset_x;
        trans.Y=tex_struct.offset_y;

        f32 pi = 3.141592653;

        core::vector3df scale;
        scale.X = tex_struct.scale_x;
        scale.Y = tex_struct.scale_y;
        scale.Z = 1;

        f->uv_mat.buildTextureTransform(tex_struct.rotation*pi/180.0,core::vector2df(0.5,0.5),
                                      core::vector2df(tex_struct.offset_x,tex_struct.offset_y),
                                      core::vector2df(tex_struct.scale_x,tex_struct.scale_y));
        trans.X*=tex_struct.scale_x;
        trans.Y*=tex_struct.scale_y;

        trans.rotateXYBy(tex_struct.rotation);
        f->uv_mat.setTranslation(trans);

        gui::IGUIComboBox* box = (gui::IGUIComboBox*)form->getFieldFromId(STYLE_ID)->getEditElement(0);
        if(box->isEnabled())
        {
            if(tex_struct.style.value==0)
                geo_node->elements[brush_j].brush.surface_groups[ f->surface_group ].type = SURFACE_GROUP_SPHERE;
            else if(tex_struct.style.value==1)
                geo_node->elements[brush_j].brush.surface_groups[ f->surface_group ].type = SURFACE_GROUP_DOME;
        }

        geo_node->edit_meshnode_interface.recalc_uvs_for_face(geo_node, brush_j, face_j, b_i);
        geo_node->final_meshnode_interface.recalc_uvs_for_face(geo_node, brush_j, face_j, b_i);
    }
    refresh();
}

void Texture_Adjust_Window::click_Align()
{
    GeometryStack* geo_node = g_scene->geoNode();

    if (g_scene->getSelectedFaces().size() > 1)
    {
        polyfold* pf = geo_node->get_total_geometry();

        int f_0 = g_scene->getSelectedFaces()[0];

        int brush_0 = pf->faces[f_0].original_brush;
        int face_0 = pf->faces[f_0].original_face;

        int sg_type = g_scene->getFaceSurfaceGroup(f_0).type;

        if (sg_type == SURFACE_GROUP_STANDARD)
        {
            core::vector3df v0 = geo_node->elements[brush_0].brush.faces[face_0].uv_origin;

            for (int i = 1; i < g_scene->getSelectedFaces().size(); i++)
            {
                int f_i = g_scene->getSelectedFaces()[i];

                int brush_i = geo_node->get_total_geometry()->faces[f_i].original_brush;
                int face_i = geo_node->get_total_geometry()->faces[f_i].original_face;

                geo_node->elements[brush_i].brush.faces[face_i].uv_origin = v0;

                geo_node->edit_meshnode_interface.recalc_uvs_for_face(geo_node, brush_i, face_i, f_i);
                geo_node->final_meshnode_interface.recalc_uvs_for_face(geo_node, brush_i, face_i, f_i);
            }
        }
        else if (sg_type == SURFACE_GROUP_CYLINDER)
        {
            int sg_0 = geo_node->elements[brush_0].brush.faces[face_0].surface_group;
            core::vector3df r0 = geo_node->elements[brush_0].brush.surface_groups[sg_0].point;
            core::vector3df v0 = geo_node->elements[brush_0].brush.surface_groups[sg_0].vec;
            core::vector3df v1 = geo_node->elements[brush_0].brush.surface_groups[sg_0].vec1;

            for (int i = 1; i < g_scene->getSelectedFaces().size(); i++)
            {
                int f_i = g_scene->getSelectedFaces()[i];

                int brush_i = geo_node->get_total_geometry()->faces[f_i].original_brush;
                int face_i = geo_node->get_total_geometry()->faces[f_i].original_face;

                int sg_i = geo_node->elements[brush_i].brush.faces[face_i].surface_group;

                geo_node->elements[brush_i].brush.surface_groups[sg_i].point = r0;
                geo_node->elements[brush_i].brush.surface_groups[sg_i].vec = v0;
                geo_node->elements[brush_i].brush.surface_groups[sg_i].vec1 = v1;

                geo_node->edit_meshnode_interface.recalc_uvs_for_face(geo_node, brush_i, face_i, f_i);
                geo_node->final_meshnode_interface.recalc_uvs_for_face(geo_node, brush_i, face_i, f_i);
            }
        }
    }
}

bool Texture_Adjust_Window::OnEvent(const SEvent& event)
{
    MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
    GeometryStack* geo_node = g_scene->geoNode();

    if(event.EventType == irr::EET_USER_EVENT)
    {
        switch(event.UserEvent.UserData1)
        {
            case USER_EVENT_SELECTION_CHANGED:
            {
                this->sel_struct.nSelected = g_scene->getSelectedFaces().size();

                if(g_scene->getSelectedFaces().size() > 0)
                {
                    SEvent event;
                    event.EventType = EET_USER_EVENT;
                    event.UserEvent.UserData1=USER_EVENT_TEXTURE_EDIT_MODE_BEGIN;
                    receiver->OnEvent(event);

                    int b_i = g_scene->getSelectedFaces()[0];

                    int brush_j = geo_node->get_total_geometry()->faces[b_i].original_brush;
                    int face_j = geo_node->get_total_geometry()->faces[b_i].original_face;

                    poly_face* f = &geo_node->elements[brush_j].brush.faces[face_j];

                    if((geo_node->elements[brush_j].brush.surface_groups[ f->surface_group ].type == SURFACE_GROUP_CYLINDER ||
                        geo_node->elements[brush_j].brush.surface_groups[ f->surface_group ].type == SURFACE_GROUP_SPHERE ||
                        geo_node->elements[brush_j].brush.surface_groups[ f->surface_group ].type == SURFACE_GROUP_DOME
                        )
                       && !dont_sg_select)
                    {
                        dont_sg_select=true; //avoid an infinite loop
                        g_scene->selectSurfaceGroup();
                    }
                    dont_sg_select=false;

                    if(geo_node->elements[brush_j].brush.surface_groups[ f->surface_group ].type == SURFACE_GROUP_SPHERE)
                        tex_struct.style.value=0;
                    else if(geo_node->elements[brush_j].brush.surface_groups[ f->surface_group ].type == SURFACE_GROUP_DOME)
                        tex_struct.style.value=1;

                    tex_struct.rotation = f->uv_mat.getRotationDegrees().Z;
                    //tex_struct.scale = core::vector2df(f->uv_mat.getScale().X,f->uv_mat.getScale().Y);
                    tex_struct.scale_x = f->uv_mat.getScale().X;
                    tex_struct.scale_y = f->uv_mat.getScale().Y;

                    core::vector3df trans = f->uv_mat.getTranslation();
                    trans.rotateXYBy(-tex_struct.rotation);
                    trans.X /= tex_struct.scale_x;
                    trans.Y /= tex_struct.scale_y;
                    //tex_struct.offset = core::vector2df(trans.X,trans.Y);
                    tex_struct.offset_x = trans.X;
                    tex_struct.offset_y = trans.Y;

                }

                refresh();
                //form2->read();
            }
            break;
            case USER_EVENT_TEXTURE_PLANE_DRAG:
            {
                if(!bDragTexture)
                {
                    bDragTexture=true;
                    vDragOrigin=g_scene->getDragVec();

                    std::vector<int> selection = this->g_scene->getSelectedFaces();
                    for(int b_i: selection)
                    {
                        int brush_j = geo_node->get_total_geometry()->faces[b_i].original_brush;
                        int face_j = geo_node->get_total_geometry()->faces[b_i].original_face;
                        poly_face* f = &geo_node->elements[brush_j].brush.faces[face_j];
                        f->uv_mat0 = f->uv_mat;
                    }
                }
                else
                {
                    core::vector3df drag = g_scene->getDragVec();
                    std::vector<int> selection = this->g_scene->getSelectedFaces();

                    if(selection.size() > 0)
                    {
                        int b_0 = selection[0];
                        int brush_0 = geo_node->get_total_geometry()->faces[b_0].original_brush;
                        int face_0 = geo_node->get_total_geometry()->faces[b_0].original_face;

                        poly_face* f0 = &geo_node->elements[brush_0].brush.faces[face_0];
                        int sg = geo_node->elements[brush_0].brush.surface_groups[ f0->surface_group ].type;

                        if(sg == SURFACE_GROUP_CYLINDER)
                        {

                            core::vector3df iV;
                            core::vector3df jV;
                            geo_node->elements[brush_0].brush.getSurfaceVectors(face_0,iV,jV);

                            f32 snap_dist = tex_struct.snap_dist;
                            f32 X_ = snap_dist * (int)(drag.dotProduct(iV)/256 / snap_dist);
                            f32 Y_ = snap_dist * (int)(drag.dotProduct(jV)/256 / snap_dist);

                            X_*=tex_struct.scale_x;
                            Y_*=tex_struct.scale_y;

                            for(int b_i: selection)
                            {
                                int brush_j = geo_node->get_total_geometry()->faces[b_i].original_brush;
                                int face_j = geo_node->get_total_geometry()->faces[b_i].original_face;

                                poly_face* f = &geo_node->elements[brush_j].brush.faces[face_j];

                                core::vector3df trans = f->uv_mat.getTranslation();
                                core::vector3df trans0 = f->uv_mat0.getTranslation();

                                trans.X=-X_;
                                trans.Y=Y_;

                                trans.rotateXYBy(tex_struct.rotation);

                                trans.X+=trans0.X;
                                trans.Y+=trans0.Y;

                                f->uv_mat.setTranslation(trans);

                                geo_node->edit_meshnode_interface.recalc_uvs_for_face_cylinder(geo_node, brush_j, face_j, b_i);
                                geo_node->final_meshnode_interface.recalc_uvs_for_face_cylinder(geo_node, brush_j, face_j, b_i);

                            }
                        }
                        else if(sg == SURFACE_GROUP_STANDARD || sg == SURFACE_GROUP_CUSTOM_UVS_GEOMETRY || 
                            sg == SURFACE_GROUP_CUSTOM_UVS_BRUSH)
                        {
                            for(int b_i: selection)
                            {

                                int brush_j = geo_node->get_total_geometry()->faces[b_i].original_brush;
                                int face_j = geo_node->get_total_geometry()->faces[b_i].original_face;

                                poly_face* f = &geo_node->elements[brush_j].brush.faces[face_j];

                                core::vector3df iV;
                                core::vector3df jV;
                                geo_node->elements[brush_j].brush.getSurfaceVectors(face_j,iV,jV);

                                core::vector3df trans = f->uv_mat.getTranslation();
                                core::vector3df trans0 = f->uv_mat0.getTranslation();

                                f32 snap_dist = tex_struct.snap_dist;
                                f32 X_ = snap_dist * (int)(drag.dotProduct(iV)/256 / snap_dist);
                                f32 Y_ = snap_dist * (int)(drag.dotProduct(jV)/256 / snap_dist);

                                X_*=tex_struct.scale_x;
                                Y_*=tex_struct.scale_y;

                                trans.X=X_;
                                trans.Y=-Y_;

                                trans.rotateXYBy(tex_struct.rotation);

                                trans.X+=trans0.X;
                                trans.Y+=trans0.Y;

                                f->uv_mat.setTranslation(trans);

                                if (sg == SURFACE_GROUP_STANDARD)
                                {
                                    geo_node->edit_meshnode_interface.recalc_uvs_for_face_cube(geo_node, brush_j, face_j, b_i);
                                    geo_node->final_meshnode_interface.recalc_uvs_for_face_cube(geo_node, brush_j, face_j, b_i);
                                }
                                else if (sg == SURFACE_GROUP_CUSTOM_UVS_BRUSH || 
                                         sg == SURFACE_GROUP_CUSTOM_UVS_GEOMETRY)
                                {
                                    geo_node->edit_meshnode_interface.recalc_uvs_for_face_custom(geo_node, brush_j, face_j, b_i);
                                    geo_node->final_meshnode_interface.recalc_uvs_for_face_custom(geo_node, brush_j, face_j, b_i);
                                }
                            }
                        }
                    }
                    if(selection.size()>0)
                    {
                        int b_i = g_scene->getSelectedFaces()[0];

                        int brush_j = geo_node->get_total_geometry()->faces[b_i].original_brush;
                        int face_j = geo_node->get_total_geometry()->faces[b_i].original_face;

                        poly_face* f = &geo_node->elements[brush_j].brush.faces[face_j];

                        core::vector3df trans = f->uv_mat.getTranslation();
                        trans.rotateXYBy(-tex_struct.rotation);
                        trans.X /= tex_struct.scale_x;
                        trans.Y /= tex_struct.scale_y;
                        //tex_struct.offset = core::vector2df(trans.X,trans.Y);
                        tex_struct.offset_x = trans.X;
                        tex_struct.offset_y = trans.Y;
                        if(form)
                        {
                            form->read(&tex_struct);
                        }

                        SEvent event;
                        event.EventType = EET_USER_EVENT;
                        event.UserEvent.UserData1 = USER_EVENT_TEXTURE_UVS_MODIFIED;
                        receiver->OnEvent(event);
                    }
                }

            }
            break;
            case USER_EVENT_TEXTURE_PLANE_DRAG_END:
            {
                if(bDragTexture)
                {
                    bDragTexture=false;
                }
            }
            break;
        }
        return true;
    }
    else if(event.EventType == EET_GUI_EVENT)
    {
        s32 id = event.GUIEvent.Caller->getID();
        gui::IGUIEnvironment* env = device->getGUIEnvironment();

        switch(event.GUIEvent.EventType)
        {
            case EGET_BUTTON_CLICKED:
            {
                if(id==OK_BUTTON_ID)
                {
                    click_OK();

                    SEvent event;
                    event.EventType = EET_USER_EVENT;
                    event.UserEvent.UserData1 = USER_EVENT_TEXTURE_UVS_MODIFIED;
                    receiver->OnEvent(event);

                    return true;
                }
                else if (id == ALIGN_BUTTON_ID)
                {
                    click_Align();

                    SEvent event;
                    event.EventType = EET_USER_EVENT;
                    event.UserEvent.UserData1 = USER_EVENT_TEXTURE_UVS_MODIFIED;
                    receiver->OnEvent(event);

                    return true;
                }
                break;
            }
            case EGET_ELEMENT_FOCUS_LOST:
            {
                if(id == SNAP_ID)
                {
                    FormField* f = form->getFieldFromId(id);
                    if(f)
                    {
                        f->writeValue(&tex_struct);
                    }
                }

            }
            break;
        }
    }

    return gui::CGUIWindow::OnEvent(event);
}

void Texture_Adjust_Tool::show()
{
    Texture_Adjust_Window* win = new Texture_Adjust_Window(env,env->getRootGUIElement(),g_scene,-1,core::rect<s32>(500,64,500+230,64+340));
    win->setText(L"Texture Adjust");

    win->show();
    win->drop();
}

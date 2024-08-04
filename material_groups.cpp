


#include <irrlicht.h>
#include "material_groups.h"
#include "edit_env.h"
#include "edit_classes.h"
#include "geometry_scene.h"
#include "utils.h"

using namespace irr;

extern IrrlichtDevice* device;

Material_Groups_Base* Material_Groups_Tool::base = NULL;
gui::IGUIEnvironment* Material_Groups_Tool::env = NULL;
geometry_scene* Material_Groups_Tool::g_scene = NULL;
multi_tool_panel* Material_Groups_Tool::panel = NULL;

//Material_Groups_Widget::mGroups_struct Material_Groups_Widget::mg_struct{0};
Material_Groups_Widget::nSelected_struct Material_Groups_Widget::sel_struct{2};


REFLECT_STRUCT_BEGIN(Material_Groups_Widget::nSelected_struct)
    REFLECT_STRUCT_MEMBER(nSelected)
REFLECT_STRUCT_END()


Material_Groups_Widget::Material_Groups_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent,geometry_scene* g_scene_,Material_Groups_Base* base_, s32 id,core::rect<s32> rect)
    : gui::IGUIElement(gui::EGUIET_ELEMENT,env,parent,id,rect), g_scene(g_scene_), my_ID(id), base(base_)
{
    MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
    receiver->Register(this);

}

Material_Groups_Widget::~Material_Groups_Widget()
{
    //std::cout << "Out of scope (Material Groups Widget)\n";
    MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
    receiver->UnRegister(this);
}

void Material_Groups_Widget::show()
{
    core::vector2di form_pos(0,0);
    core::vector2di form_pos2(0,0);

    LISTBOX_ID = my_ID + 1;
    VISUALIZE_BUTTON_ID = my_ID + 2;
    COMBOBOX_ID = my_ID + 3;

    //form = new Reflected_GUI_Edit_Form(Environment,this,NULL,OK_BUTTON_ID+1,
    //                                        core::rect<s32>(form_pos,core::dimension2du(128,128)));
    form2 = new Reflected_GUI_Edit_Form(Environment,this,NULL,VISUALIZE_BUTTON_ID+1,
                                            core::rect<s32>(form_pos2,core::dimension2du(128,128)));

    //reflect::TypeDescriptor_Struct* typeDesc = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<mGroups_struct>::get();
   // reflect::TypeDescriptor* typeDesc2 = reflect::TypeResolver<nSelected_struct>::get();
    //typeDesc->addFormWidgets("", form,true);
    //typeDesc->addFormWidget(form,NULL,std::vector<int>{},0,true,true,0);

    //int next_ID = form->ShowWidgets(GUI_ID_REFLECTED_BASE+3);

    Int_StaticField* f = new Int_StaticField();
    f->setText("selected faces");
    f->offset = 0;
    f->bVisible = true;

    form2->addEditField(f);

    int next_ID = form2->ShowWidgets(GUI_ID_REFLECTED_BASE + 3);

    
    gui::IGUISkin* skin = Environment->getSkin();
    gui::IGUIFont* font = skin->getFont();
    int itemheight = font->getDimension(L"A").Height + 4;

    core::rect<s32> r(core::vector2di(0, form2->getTotalHeight()+8), core::dimension2di(getRelativePosition().getWidth(), base->material_groups.size() * itemheight + 4));

    if (m_listbox)
        m_listbox->remove();

    m_listbox = Environment->addListBox(r, this, LISTBOX_ID);
    m_listbox->setDrawBackground(false);
    
    for (Material_Group mg : base->material_groups)
    {
       
        //std::wstring txt(typeDesc->alias, typeDesc->alias + strlen(typeDesc->alias));
        std::wstring txt(mg.name.begin(),mg.name.end());
        m_listbox->addItem(txt.c_str());
    }

    m_listbox->setSelected(-1);

    

    //*obj2 = g_scene->getSelectedFaces().size();

    //f->obj = (int*)&sel_struct;
    

    sel_struct.nSelected = g_scene->getSelectedFaces().size();

    int ypos = form2->getTotalHeight()+32 + base->material_groups.size() * itemheight + 4;

    //VISUALIZE_BUTTON_ID=next_ID;

    Environment->addButton(core::rect<s32>(form_pos+core::vector2di(100,ypos),form_pos+core::vector2di(160,ypos+28)),this,VISUALIZE_BUTTON_ID,L"Visualize");


    r = core::rect<s32>(core::vector2di(getRelativePosition().getWidth()-96, ypos+48), core::dimension2di(96, 24));

    m_combobox_lmres = Environment->addComboBox(r, this, COMBOBOX_ID);
    //m_combobox_lmres->setDrawBackground(false);

    m_combobox_lmres->addItem(L"None");
    m_combobox_lmres->addItem(L"16");
    m_combobox_lmres->addItem(L"32");
    m_combobox_lmres->addItem(L"64");
    m_combobox_lmres->addItem(L"128");
    m_combobox_lmres->addItem(L"256");
    m_combobox_lmres->addItem(L"512");


    m_combobox_lmres->setSelected(0);

    //bringToFront(form2);

    SEvent event;
    event.EventType = EET_USER_EVENT;
    event.UserEvent.UserData1=USER_EVENT_SELECTION_CHANGED;
    this->OnEvent(event);

}

void Material_Groups_Widget::refresh()
{
    /*
    if(form)
    {
        form->remove();
    }*/

    core::vector2di form_pos(8,60);
    //form = new Reflected_GUI_Edit_Form(Environment,this,NULL,GUI_ID_REFLECTED_BASE,
    //                                        core::rect<s32>(form_pos,core::dimension2du(96,128)));

    //bool editable = this->sel_struct.nSelected > 0;

   // reflect::TypeDescriptor_Struct* typeDesc = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<mGroups_struct>::get();
//    typeDesc->addFormWidgets("", form,editable);
   // typeDesc->addFormWidget(form,NULL,std::vector<int>{},0,true,editable,0);

    //int next_ID = form->ShowWidgets(GUI_ID_REFLECTED_BASE+2);

    //form->read(&mg_struct);
    form2->read(&sel_struct);

    //FormField* f = form->edit_fields;

    if( g_scene->getSelectedFaces().size() > 1)
    {
        int f_0 = g_scene->getSelectedFaces()[0];
        int brush_0 = g_scene->get_total_geometry()->faces[f_0].original_brush;
        int face_0 = g_scene->get_total_geometry()->faces[f_0].original_face;

        for( int f_i : g_scene->getSelectedFaces())
        {
            int b_i = g_scene->getSelectedFaces()[0];

            int brush_j = g_scene->get_total_geometry()->faces[f_i].original_brush;
            int face_j = g_scene->get_total_geometry()->faces[f_i].original_face;

            bool b = (g_scene->elements[brush_j].brush.faces[face_j].material_group ==
                        g_scene->elements[brush_0].brush.faces[face_0].material_group);

            if(b)
            {
                //mg_struct.group_no.value = g_scene->elements[brush_0].brush.faces[face_0].material_group;
                //form->read(&mg_struct);
                bWrite=true;
            }
            else
            {
                //f->setActive(false);
                bWrite=false;
            }
        }
    }
    else if( g_scene->getSelectedFaces().size() == 1)
    {
        int f_0 = g_scene->getSelectedFaces()[0];
        int brush_0 = g_scene->get_total_geometry()->faces[f_0].original_brush;
        int face_0 = g_scene->get_total_geometry()->faces[f_0].original_face;

        //mg_struct.group_no.value = g_scene->elements[brush_0].brush.faces[face_0].material_group;
        //form->read(&mg_struct);
        bWrite=true;
    }
}


void Material_Groups_Widget::click_OK()
{
    if(!bWrite)
        return;

    std::vector<int> selection = this->g_scene->getSelectedFaces();

    //this->form->write(&mg_struct);

    for(int b_i: selection)
    {
        int brush_j = g_scene->get_total_geometry()->faces[b_i].original_brush;
        int face_j = g_scene->get_total_geometry()->faces[b_i].original_face;

        poly_face* f = &g_scene->elements[brush_j].brush.faces[face_j];

        //f->material_group = mg_struct.group_no.value;
        //this->g_scene->get_total_geometry()->faces[b_i].material_group == mg_struct.group_no.value;
        f->material_group = 1;
        this->g_scene->get_total_geometry()->faces[b_i].material_group = 1;
    }
}


bool Material_Groups_Widget::OnEvent(const SEvent& event)
{

    if(event.EventType == irr::EET_USER_EVENT)
    {
        switch(event.UserEvent.UserData1)
        {
            case USER_EVENT_SELECTION_CHANGED:
            {
                this->sel_struct.nSelected = g_scene->getSelectedFaces().size();

                if (g_scene->getSelectedFaces().size() > 0)
                {
                    int b_i = g_scene->getSelectedFaces()[0];

                    int brush_j = g_scene->get_total_geometry()->faces[b_i].original_brush;
                    int face_j = g_scene->get_total_geometry()->faces[b_i].original_face;

                    poly_face* f = &g_scene->elements[brush_j].brush.faces[face_j];

                    if ((g_scene->elements[brush_j].brush.surface_groups[f->surface_group].type == SURFACE_GROUP_CYLINDER ||
                        g_scene->elements[brush_j].brush.surface_groups[f->surface_group].type == SURFACE_GROUP_SPHERE ||
                        g_scene->elements[brush_j].brush.surface_groups[f->surface_group].type == SURFACE_GROUP_DOME
                        )
                        && !dont_sg_select)
                    {
                        dont_sg_select = true; //avoid an infinite loop
                        g_scene->selectSurfaceGroup();
                    }
                    dont_sg_select = false;

                    int mg0 = g_scene->get_total_geometry()->faces[b_i].material_group;

                    bool b = true;

                    for (int f_i : g_scene->getSelectedFaces())
                    {
                        int mg1 = g_scene->get_total_geometry()->faces[f_i].material_group;
                        if (mg0 != mg1)
                            b = false;
                    }

                    if (m_listbox)
                    {
                        if (b)
                        {
                            m_listbox->setSelected(mg0);
                            base->selected = mg0;
                        }
                        else
                        {
                            m_listbox->setSelected(-1);
                            base->selected = -1;
                        }

                    }

                }
                refresh();
            } break;
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
                if(id==VISUALIZE_BUTTON_ID)
                {
                   // std::cout << "visualize!\n";
                    g_scene->visualizeMaterialGroups();
                    //click_OK();
                    return true;
                }
                break;
            }
            case EGET_COMBO_BOX_CHANGED:
            {
                bWrite = true;
                return true;
                break;
            }
            case EGET_LISTBOX_CHANGED:
            {
                if (id == LISTBOX_ID)
                {
                    gui::IGUIListBox* listbox = (gui::IGUIListBox*)event.GUIEvent.Caller;
                    int sel = listbox->getSelected();

                    this->base->selected = sel;

                    if (sel != -1)
                        g_scene->MaterialGroupToSelectedFaces();

                    return true;
                }
            }break;
        }
    }

    return gui::IGUIElement::OnEvent(event);
}

void Material_Groups_Base::refreshTextures()
{
    /*
    for (Material_Group &mg: material_groups)
    {
        if (mg.texture)
            env->getVideoDriver()->removeTexture(mg.texture);

        irr::video::IImage* img = makeSolidColorImage(env->getVideoDriver(), mg.color);
        video::ITexture* tex = env->getVideoDriver()->addTexture("txtr", img);
        img->drop();
        mg.texture = tex;
    }
    */
}

Material_Groups_Base::~Material_Groups_Base()
{
    /*
    for (Material_Group& mg : material_groups)
    {
        if (mg.texture)
            env->getVideoDriver()->removeTexture(mg.texture);
    }*/
}

void Material_Groups_Base::show()
{
    core::rect<s32> client_rect(core::vector2di(0,0),
                                core::dimension2du(this->panel->getClientRect()->getAbsolutePosition().getWidth(),
                                                   this->panel->getClientRect()->getAbsolutePosition().getHeight()));

    Material_Groups_Widget* widget = new Material_Groups_Widget(env,this->panel->getClientRect(),g_scene,this,GUI_ID_MATERIAL_GROUPS_BASE,client_rect);
    widget->setName("material groups widget");

    widget->show();

    widget->drop();
}

void Material_Groups_Base::apply_material_to_buffer(scene::IMeshBuffer* buffer, int material_no, int lighting, bool selected)
{
    if (material_no >= material_groups.size())
        return;

    Material_Group mg = material_groups[material_no];

    if (mg.lightmap && lighting)
    {
        if(selected)
            buffer->getMaterial().MaterialType = LightingMaterial_Selected_Type;
        else
            buffer->getMaterial().MaterialType = LightingMaterial_Type;
    }
    else
    {
        if (selected)
            buffer->getMaterial().MaterialType = SolidMaterial_Selected_Type;
        else
            buffer->getMaterial().MaterialType = SolidMaterial_Type;
    }

    if (mg.two_sided)
    {
        buffer->getMaterial().BackfaceCulling = false;
    }
    else
    {
        buffer->getMaterial().BackfaceCulling = true;
    }

    if (mg.transparent)
    {
        buffer->getMaterial().MaterialType = video::EMT_TRANSPARENT_ADD_COLOR;
    }

}


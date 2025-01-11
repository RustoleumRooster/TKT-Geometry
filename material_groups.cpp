#include <irrlicht.h>
#include "material_groups.h"
#include "edit_env.h"
#include "edit_classes.h"
#include "geometry_scene.h"
#include "utils.h"

using namespace irr;
using namespace gui;

extern IrrlichtDevice* device;

Material_Groups_Base* Material_Groups_Tool::base = NULL;
IGUIEnvironment* Material_Groups_Tool::env = NULL;
multi_tool_panel* Material_Groups_Tool::panel = NULL;

Material_Groups_Widget::nSelected_struct Material_Groups_Widget::sel_struct{2};

REFLECT_STRUCT_BEGIN(Material_Groups_Widget::nSelected_struct)
    REFLECT_STRUCT_MEMBER(nSelected)
REFLECT_STRUCT_END()


Material_Groups_Widget::Material_Groups_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene* scene, Material_Groups_Base* base_, s32 id, core::rect<s32> rect)
    : IGUIElement(EGUIET_ELEMENT, env, parent, id, rect), my_ID(id), base(base_), g_scene(scene)
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

    form2 = new Reflected_GUI_Edit_Form(Environment,this,NULL,VISUALIZE_BUTTON_ID+1,
                                            core::rect<s32>(form_pos2,core::dimension2du(128,128)));

    Int_StaticField* f = new Int_StaticField();
    f->setText("selected faces");
    f->offset = 0;
    f->bVisible = true;

    form2->addEditField(f);

    int next_ID = form2->ShowWidgets(GUI_ID_REFLECTED_BASE + 3);

    
    IGUISkin* skin = Environment->getSkin();
    IGUIFont* font = skin->getFont();
    int itemheight = font->getDimension(L"A").Height + 4;

    core::rect<s32> r(core::vector2di(0, form2->getTotalHeight()+8), core::dimension2di(getRelativePosition().getWidth(), base->material_groups.size() * itemheight + 4));

    if (m_listbox)
        m_listbox->remove();

    m_listbox = Environment->addListBox(r, this, LISTBOX_ID);
    m_listbox->setDrawBackground(false);
    
    for (Material_Group mg : base->material_groups)
    {
        std::wstring txt(mg.name.begin(),mg.name.end());
        m_listbox->addItem(txt.c_str());
    }

    m_listbox->setSelected(-1);

    sel_struct.nSelected = g_scene->getSelectedFaces().size();

    int ypos = form2->getTotalHeight()+32 + base->material_groups.size() * itemheight + 4;

    Environment->addButton(core::rect<s32>(form_pos+core::vector2di(100,ypos),form_pos+core::vector2di(160,ypos+28)),this,VISUALIZE_BUTTON_ID,L"Visualize");

    r = core::rect<s32>(core::vector2di(getRelativePosition().getWidth()-96, ypos+48), core::dimension2di(96, 24));

    m_combobox_lmres = Environment->addComboBox(r, this, COMBOBOX_ID);

    m_combobox_lmres->addItem(L"None");
    m_combobox_lmres->addItem(L"16");
    m_combobox_lmres->addItem(L"32");
    m_combobox_lmres->addItem(L"64");
    m_combobox_lmres->addItem(L"128");
    m_combobox_lmres->addItem(L"256");
    m_combobox_lmres->addItem(L"512");


    m_combobox_lmres->setSelected(0);

    SEvent event;
    event.EventType = EET_USER_EVENT;
    event.UserEvent.UserData1=USER_EVENT_SELECTION_CHANGED;
    this->OnEvent(event);

}

void Material_Groups_Widget::refresh()
{
    core::vector2di form_pos(8,60);

    form2->read(&sel_struct);
}

void Material_Groups_Widget::click_OK()
{

}

bool Material_Groups_Widget::OnEvent(const SEvent& event)
{
    GeometryStack* geo_node = g_scene->geoNode();

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

                    int brush_j = geo_node->get_total_geometry()->faces[b_i].original_brush;
                    int face_j = geo_node->get_total_geometry()->faces[b_i].original_face;

                    poly_face* f = &geo_node->elements[brush_j].brush.faces[face_j];

                    if ((geo_node->elements[brush_j].brush.surface_groups[f->surface_group].type == SURFACE_GROUP_CYLINDER ||
                        geo_node->elements[brush_j].brush.surface_groups[f->surface_group].type == SURFACE_GROUP_SPHERE ||
                        geo_node->elements[brush_j].brush.surface_groups[f->surface_group].type == SURFACE_GROUP_DOME
                        )
                        && !dont_sg_select)
                    {
                        dont_sg_select = true; //avoid an infinite loop
                        g_scene->selectSurfaceGroup();
                    }
                    dont_sg_select = false;

                    int mg0 = geo_node->get_total_geometry()->faces[b_i].material_group;

                    bool b = true;

                    for (int f_i : g_scene->getSelectedFaces())
                    {
                        int mg1 = geo_node->get_total_geometry()->faces[f_i].material_group;
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
        IGUIEnvironment* env = device->getGUIEnvironment();

        switch(event.GUIEvent.EventType)
        {
            case EGET_BUTTON_CLICKED:
            {
                if(id==VISUALIZE_BUTTON_ID)
                {

                    g_scene->visualizeMaterialGroups();

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
                    IGUIListBox* listbox = (IGUIListBox*)event.GUIEvent.Caller;
                    int sel = listbox->getSelected();

                    this->base->selected = sel;

                    if (sel != -1)
                        g_scene->MaterialGroupToSelectedFaces();

                    return true;
                }
            }break;
        }
    }

    return IGUIElement::OnEvent(event);
}


Material_Groups_Base::Material_Groups_Base(std::wstring name, int my_id, gui::IGUIEnvironment* env, multi_tool_panel* panel)
    :tool_base(name,my_id,env,panel)
{
}

Material_Groups_Base::~Material_Groups_Base()
{
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

void Material_Groups_Base::apply_material_to_buffer(scene::IMeshBuffer* buffer, int material_no, int lighting, bool selected, bool final_view)
{
    if (material_no > 3)
    {
        if(final_view)
            return;
        else
        {
            if (selected)
                buffer->getMaterial().MaterialType = SolidMaterial_Selected_Type;
            else
                buffer->getMaterial().MaterialType = SolidMaterial_Type;
            return;
        }
    }

    Material_Group mg = material_groups[material_no];

    if (lighting != -1)
        lighting_mode = lighting;

    if (mg.lightmap && lighting_mode == LIGHTING_LIGHTMAP)
    {
        if(selected)
            buffer->getMaterial().MaterialType = LightingMaterial_Selected_Type;
        else
            buffer->getMaterial().MaterialType = LightingMaterial_Type;
    }
    else if (mg.lightmap && lighting_mode == LIGHTING_LIGHT_ONLY)
    {
        if (selected)
            buffer->getMaterial().MaterialType = LightingOnlyMaterial_Selected_Type;
        else
            buffer->getMaterial().MaterialType = LightingOnlyMaterial_Type;
    }
    else
    {
        buffer->getMaterial().Lighting = false;

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


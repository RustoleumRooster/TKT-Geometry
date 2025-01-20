
#include "node_properties.h"
#include <irrlicht.h>
#include "reflected_nodes.h"
#include "edit_classes.h"
#include "geometry_scene.h"
#include "Reflection.h"
#include "scrollbar2.h"
#include "ex_gui_elements.h"
#include "edit_env.h"
#include "NodeInstancesTool.h"

using namespace irr;

extern IrrlichtDevice* device;

Node_Properties_Base* NodeProperties_Tool::base = NULL;
multi_tool_panel* NodeProperties_Tool::panel = NULL;

//====================================================
// Node Properties Widget
//
//
//

Node_Properties_Widget::Node_Properties_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene* g_scene_, Node_Properties_Base* base_, s32 id, core::rect<s32> rect)
    : gui::IGUIElement(gui::EGUIET_ELEMENT, env, parent, id, rect), my_base(base_), g_scene(g_scene_), my_ID(id)
{
    MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
    receiver->Register(this);
}

Node_Properties_Widget::~Node_Properties_Widget()
{
   // std::cout << "Out of scope (Node Properties Widget)\n";

    if (my_widget)
        my_widget->remove();

    MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
    receiver->UnRegister(this);

}

void Node_Properties_Widget::show()
{
    core::rect<s32> pr(0, 0, getRelativePosition().getWidth(), getRelativePosition().getHeight());
    edit_panel = new gui::IGUIElement(gui::EGUIET_ELEMENT, Environment, this, -1, pr);

    OK_BUTTON_ID = my_ID + 1;

    my_widget = new Reflected_Widget_EditArea(Environment, edit_panel, g_scene, my_base, my_ID + 2, pr);

    my_widget->show(true, my_base->getObj());

    int ypos = my_widget->getEditAreaHeight() + 8;
    core::rect<s32> br = core::rect<s32>(getRelativePosition().getWidth() - 80, ypos, getRelativePosition().getWidth() - 8, ypos + 36);

    my_button = new Flat_Button(Environment, this, OK_BUTTON_ID, br);
    my_button->setText(L"Apply");

    my_widget->drop();
    edit_panel->drop();
}

void Node_Properties_Widget::onRefresh()
{
    if (my_button)
        my_button->remove();

    int ypos = my_widget->getEditAreaHeight() + 8;
    core::rect<s32> br = core::rect<s32>(getRelativePosition().getWidth() - 80, ypos, getRelativePosition().getWidth() - 8, ypos + 36);

    my_button = new Flat_Button(Environment, this, OK_BUTTON_ID, br);
    my_button->setText(L"Apply");
}

bool Node_Properties_Widget::OnEvent(const SEvent& event)
{
    if (event.EventType == EET_USER_EVENT)
    {
        if (event.UserEvent.UserData1 == USER_EVENT_REFLECTED_FORM_REFRESHED)
        {
            onRefresh();
            return true;
        }
        else if (event.UserEvent.UserData1 == USER_EVENT_SELECTION_CHANGED && my_base->waiting_for_node_selection == false)
        {
            if (my_widget)
            {
                my_widget->remove();
            }

            my_base->refresh_types();
            my_base->preEdit();

            core::rect<s32> pr(0, 0, getRelativePosition().getWidth(), getRelativePosition().getHeight());
            my_widget = new Reflected_Widget_EditArea(Environment, edit_panel, g_scene, my_base, my_ID + 2, pr);

            my_widget->show(true, my_base->getObj());
            my_widget->drop();

            onRefresh();

            return true;
        }
        else if (event.UserEvent.UserData1 == USER_EVENT_NODES_SELECTED)
        {
            my_base->node_selection_made();

            std::vector<int> tree_pos = my_base->node_selection_member;
            reflect::Member* m = my_widget->m_typeDesc->getTreeNode(tree_pos);
            size_t offset = my_widget->m_typeDesc->getTreeNodeOffset(tree_pos);

            m->modified = true;
            m->type->copy((char*)my_widget->temp_object + offset, &my_base->saved_uids);

            FormField* f = my_widget->form->edit_fields;
            while (f)
            {
                if (f->tree_pos == tree_pos)
                {
                    f->readValue((char*)my_widget->temp_object);
                    break;
                }
                f = f->next;
            }

            onRefresh();

            return true;
        }
        else if (event.UserEvent.UserData1 == USER_EVENT_NODE_SELECTION_ENDED)
        {
            my_base->node_selection_over();

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
        }
    }

    return IGUIElement::OnEvent(event);
}

void Node_Properties_Widget::click_OK()
{
    my_widget->write_by_field();
}

//====================================================
// Node Properties Base
//
//
//

Node_Properties_Base::Node_Properties_Base(std::wstring name, int my_id, gui::IGUIEnvironment* env, multi_tool_panel* panel)
    : reflected_tool_base(name,my_id,env,panel)
{
}

Node_Properties_Base::~Node_Properties_Base()
{
}

std::vector<reflect::TypeDescriptor_Struct*> Node_Properties_Base::GetTypeDescriptors(geometry_scene* geo_scene)
{
    if(geo_scene->getSelectedNodes().size() == 0)
        return std::vector<reflect::TypeDescriptor_Struct*>{};

    Reflected_SceneNode* node_0 = geo_scene->getSelectedNodes()[0];

    std::vector<reflect::TypeDescriptor_Struct*> common_types;

    reflect::TypeDescriptor_Struct* typeDescriptor = node_0->GetDynamicReflection();

     auto IsOfType = [](Reflected_SceneNode* node,reflect::TypeDescriptor_Struct* typeDesc) -> bool{
        reflect::TypeDescriptor_Struct* td = node->GetDynamicReflection();
        while(td != NULL)
        {
            if(td == typeDesc)
                return true;
            td = td->inherited_type;
        }
        return false;
    };

    while(typeDescriptor != NULL)
    {
        bool b = true;

        for(Reflected_SceneNode* n : geo_scene->getSelectedNodes())
        {
            if(IsOfType(n,typeDescriptor) == false)
                b=false;
        }

        if(b)
        {
            common_types.push_back(typeDescriptor);
        }

        typeDescriptor = typeDescriptor->inherited_type;
    }
    std::vector<reflect::TypeDescriptor_Struct*> ret;

    for(int i=common_types.size()-1;i>=0;i--)
    {
        if(common_types[i]->members.size() > 0)
            ret.push_back(common_types[i]);
    }

   return ret;
}

void Node_Properties_Base::show()
{
    core::rect<s32> client_rect(core::vector2di(0, 0),
        core::dimension2du(this->panel->getClientRect()->getAbsolutePosition().getWidth(),
            this->panel->getClientRect()->getAbsolutePosition().getHeight()));
  
    refresh_types();

    Node_Properties_Widget* widget = new Node_Properties_Widget(env, this->panel->getClientRect(), g_scene, this, GUI_ID_GEO_SETTINGS_BASE, client_rect);
    
    for (Reflected_SceneNode* node : g_scene->getSelectedNodes())
    {
        node->preEdit();
    }

    widget->show();
    widget->drop();
}

void Node_Properties_Base::refresh_types()
{
    my_typeDescriptors = Node_Properties_Base::GetTypeDescriptors(g_scene);
}

void Node_Properties_Base::choose_node_selection(std::vector<int> tree_pos)
{
    preEdit();

    saved_selection = g_scene->getSelectedNodes();

    waiting_for_node_selection = true;
    node_selection_member = tree_pos;

    Node_Selector_Tool::show();
}

void Node_Properties_Base::clear_selection(std::vector<int> tree_pos)
{
    node_selection_member = tree_pos;

    saved_uids.clear();

    preEdit();

    MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();

    SEvent event;
    event.EventType = EET_USER_EVENT;
    event.UserEvent.UserData1 = USER_EVENT_NODES_SELECTED;

    receiver->OnEvent(event);
}

void Node_Properties_Base::node_selection_made()
{
    std::vector<int> tree_pos = node_selection_member;

    if (waiting_for_node_selection)
    {
        std::vector<int> branch;
        for (int i = 1; i < tree_pos.size(); i++)
            branch.push_back(tree_pos[i]);

        if (branch.size() > 0)
        {
            reflect::TypeDescriptor_Struct* typeDesc = my_typeDescriptors[tree_pos[0]];

            reflect::Member* m = typeDesc->getTreeNode(branch);
            size_t offset = typeDesc->getTreeNodeOffset(branch);

            std::vector<Reflected_SceneNode*> nodes = g_scene->getSelectedNodes();
            
            saved_uids.clear();

            for (Reflected_SceneNode* n : nodes)
                saved_uids.push_back(n->UID());
        }
    }
}

void Node_Properties_Base::node_selection_over()
{
    if (waiting_for_node_selection)
    {
        waiting_for_node_selection = false;
        g_scene->setSelectedNodes(saved_selection);
    }
}

void Node_Properties_Base::preEdit()
{
    for (Reflected_SceneNode* node : g_scene->getSelectedNodes())
    {
        node->preEdit();
    }
}

reflect::TypeDescriptor_Struct* Node_Properties_Base::new_node_properties_flat_typedescriptor(std::vector<reflect::TypeDescriptor_Struct*> typeDescriptors)
{
    reflect::TypeDescriptor_Struct* ret = new reflect::TypeDescriptor_Struct(reflect::EmptyStruct::initReflection);
    ret->size = 0;

    size_t p_inc = 0;

    if(g_scene->getSelectedNodes().size() > 0)
    {
        Reflected_SceneNode* node = g_scene->getSelectedNodes()[0];

        size_t size_inc = 0;
        
        for (reflect::TypeDescriptor_Struct* typeDesc : typeDescriptors)
        {
            reflect::TypeDescriptor* flat = typeDesc->get_flat_copy(node,0);

            reflect::Member m{typeDesc->alias,size_inc,flat};
            m.expanded = ((reflect::TypeDescriptor_Struct *) flat)->expanded;
            ret->members.push_back(m);
            size_inc += flat->size;
        }
        ret->size = size_inc; 
    }
    
    return ret;
}

reflect::TypeDescriptor_Struct* Node_Properties_Base::getTypeDescriptor()
{
    reflect::TypeDescriptor_Struct* ret = new reflect::TypeDescriptor_Struct(reflect::EmptyStruct::initReflection);

    for (reflect::TypeDescriptor_Struct* typeDesc : my_typeDescriptors)
    {
        reflect::Member m{ typeDesc->alias,0,typeDesc };
        m.expanded = typeDesc->expanded;
        ret->members.push_back(m);

        ret->size = typeDesc->size;
    }

    return ret;
}

void Node_Properties_Base::read_obj(void* obj)
{
    if (g_scene->getSelectedNodes().size() > 0)
    {
        Reflected_SceneNode* node = g_scene->getSelectedNodes()[0];

        for (reflect::TypeDescriptor_Struct* typeDesc : my_typeDescriptors)
        {
            typeDesc->copy((char*)obj, node);
        }
    }
}

/*
void Node_Properties_Base::write_obj(void* obj)
{
    if (g_scene->getSelectedNodes().size() > 0)
    {
        Reflected_SceneNode* node = g_scene->getSelectedNodes()[0];

        for (reflect::TypeDescriptor_Struct* typeDesc : my_typeDescriptors)
        {
            typeDesc->copy(node, (char*)obj);
        }
    }
}*/

void Node_Properties_Base::post_edit()
{
    for (Reflected_SceneNode* node : g_scene->getSelectedNodes())
    {
        node->postEdit();
    }
}

void Node_Properties_Base::write_obj_by_field(reflect::TypeDescriptor_Struct* flat_typeDescriptor /*unused*/, std::vector<int> tree_pos, void* obj)
{
    std::vector<int> branch;
    for (int i = 1; i < tree_pos.size(); i++)
        branch.push_back(tree_pos[i]);
    
    if (branch.size() > 0)
    {
        reflect::TypeDescriptor_Struct* typeDesc = my_typeDescriptors[tree_pos[0]];

        reflect::Member* m = typeDesc->getTreeNode(branch);
        
        if (m->modified)
        {
            size_t offset = typeDesc->getTreeNodeOffset(branch);

            for (Reflected_SceneNode* node : g_scene->getSelectedNodes())
            {
                m->type->copy((char*)node + offset, (char*)obj + offset);
                int a = 0;
            }

            m->modified = false;
        }
    }
}

void Node_Properties_Base::init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos)
{
    std::vector<int> branch;
    for (int i = 1; i < tree_pos.size(); i++)
        branch.push_back(tree_pos[i]);
    
    if (branch.size() > 0)
    {
        reflect::Member* m = flat_typeDescriptor->getTreeNode(tree_pos);
        
        reflect::TypeDescriptor_Struct* typeDesc = my_typeDescriptors[tree_pos[0]];

        reflect::Member* m2 = typeDesc->getTreeNode(branch);

        size_t offset = typeDesc->getTreeNodeOffset(branch);

        if (g_scene->getSelectedNodes().size() > 1)
        {
            Reflected_SceneNode* node_0 = g_scene->getSelectedNodes()[0];
            bool b = true;

            if(m->modified)
                m->readwrite = true;
            else
            {
                for (Reflected_SceneNode* node : g_scene->getSelectedNodes())
                {
                    if (m2->type->isEqual((char*)node + offset, (char*)node_0 + offset) == false)
                    {
                        b = false;
                    }
                }
                m->readwrite = b;
            }
        }
        else if (g_scene->getSelectedNodes().size() == 1)
        {
            m->readwrite = true;
        }
    }
}

void Node_Properties_Base::widget_closing(Reflected_Widget_EditArea* widget)
{
    widget->save_expanded_status();
}

void Node_Properties_Base::save_expanded_status(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos)
{
    std::vector<int> branch;
    for (int i = 1; i < tree_pos.size(); i++)
        branch.push_back(tree_pos[i]);

    reflect::TypeDescriptor_Struct* typeDesc = my_typeDescriptors[tree_pos[0]];
    reflect::Member* m = flat_typeDescriptor->getTreeNode(tree_pos);

    if (branch.size() > 0 && m)
    {
        reflect::Member* m2 =typeDesc->getTreeNode(branch);

        if (m2)
            m2->expanded = m->expanded;
    }
    else if(m)
    {
        typeDesc->expanded = m->expanded;
    }
}

void* Node_Properties_Base::getObj()
{
    if (g_scene->getSelectedNodes().size() > 0)
    {
        Reflected_SceneNode* node = g_scene->getSelectedNodes()[0];
        return node;
    }
    return NULL;
}
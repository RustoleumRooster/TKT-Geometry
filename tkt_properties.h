#pragma once
#ifndef _TKT_PROPERTIES_H_
#define _TKT_PROPERTIES_H_

#include <irrlicht.h>
#include <vector>
#include "tkt_set.h"
#include "edit_classes.h"
#include "GUI_tools.h"
#include "Reflection.h"
#include "edit_env.h"
#include "ex_gui_elements.h"
#include "geometry_scene.h"

using namespace irr;
using namespace gui;

extern IrrlichtDevice* device;

template<typename T> class tkt_properties_widget;

//====================================================
// tkt_properties_base<T>
//
// Generic property editor base for any tkt_set<T>.
// Ported from Node_Properties_Base; all scene-node-specific
// calls replaced with m_source->getSelected() / T duck-typed calls.
//

template<typename T>
class tkt_properties_base : public reflected_tool_base
{
public:
    tkt_properties_base(
        std::wstring name, int my_id,
        gui::IGUIEnvironment* env, multi_tool_panel* panel,
        tkt_set<T>* source
    ) : reflected_tool_base(name, my_id, env, panel), m_source(source)
    {
    }

    ~tkt_properties_base() {}

    // If g_scene_ is also a tkt_set<T>, update m_source automatically.
    // For T = Reflected_SceneNode this fires because geometry_scene IS-A tkt_set<Reflected_SceneNode>.
    virtual void set_scene(geometry_scene* g_scene_) override
    {
        tool_base::set_scene(g_scene_);
        if (auto* src = dynamic_cast<tkt_set<T>*>(g_scene_))
            m_source = src;
    }

    static std::vector<reflect::TypeDescriptor_Struct*> GetTypeDescriptors(tkt_set<T>* source)
    {
        if (source->getSelected().size() == 0)
            return std::vector<reflect::TypeDescriptor_Struct*>{};

        T* item_0 = source->getSelected()[0];

        std::vector<reflect::TypeDescriptor_Struct*> common_types;

        reflect::TypeDescriptor_Struct* typeDescriptor = item_0->GetDynamicReflection();

        auto IsOfType = [](T* item, reflect::TypeDescriptor_Struct* typeDesc) -> bool {
            reflect::TypeDescriptor_Struct* td = item->GetDynamicReflection();
            while (td != NULL)
            {
                if (td == typeDesc)
                    return true;
                td = td->inherited_type;
            }
            return false;
        };

        while (typeDescriptor != NULL)
        {
            bool b = true;

            for (T* n : source->getSelected())
            {
                if (IsOfType(n, typeDescriptor) == false)
                    b = false;
            }

            if (b)
                common_types.push_back(typeDescriptor);

            typeDescriptor = typeDescriptor->inherited_type;
        }

        std::vector<reflect::TypeDescriptor_Struct*> ret;

        for (int i = (int)common_types.size() - 1; i >= 0; i--)
        {
            if (common_types[i]->members.size() > 0)
                ret.push_back(common_types[i]);
        }

        return ret;
    }

    void refresh_types()
    {
        my_typeDescriptors = GetTypeDescriptors(m_source);
    }

    void preEdit()
    {
        for (T* item : m_source->getSelected())
            item->preEdit();
    }

    void choose_item_selection(std::vector<int> tree_pos)
    {
        preEdit();
        saved_selection = m_source->getSelected();
        waiting_for_item_selection = true;
        item_selection_member = tree_pos;
        // TODO: Node_Selector_Tool::show(); — deferred pending UID abstraction
    }

    void clear_item_selection(std::vector<int> tree_pos)
    {
        item_selection_member = tree_pos;
        saved_uids.clear();
        preEdit();

        MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
        SEvent event;
        event.EventType = EET_USER_EVENT;
        event.UserEvent.UserData1 = USER_EVENT_NODES_SELECTED;
        receiver->OnEvent(event);
    }

    void item_selection_made()
    {
        std::vector<int> tree_pos = item_selection_member;

        if (waiting_for_item_selection)
        {
            std::vector<int> branch;
            for (int i = 1; i < (int)tree_pos.size(); i++)
                branch.push_back(tree_pos[i]);

            if (branch.size() > 0)
            {
                reflect::TypeDescriptor_Struct* typeDesc = my_typeDescriptors[tree_pos[0]];
                reflect::Member* m = typeDesc->getTreeNode(branch);
                size_t offset = typeDesc->getTreeNodeOffset(branch);

                std::vector<T*> items = m_source->getSelected();
                saved_uids.clear();
                for (T* n : items)
                    saved_uids.push_back(n->UID());
            }
        }
    }

    void item_selection_over()
    {
        if (waiting_for_item_selection)
        {
            waiting_for_item_selection = false;
            m_source->setSelected(saved_selection);
        }
    }

    // reflected_tool_base interface

    virtual void show() override
    {
        core::rect<s32> client_rect(core::vector2di(0, 0),
            core::dimension2du(this->panel->getClientRect()->getAbsolutePosition().getWidth(),
                this->panel->getClientRect()->getAbsolutePosition().getHeight()));

        refresh_types();

        tkt_properties_widget<T>* widget = new tkt_properties_widget<T>(env, this->panel->getClientRect(), g_scene, this, GUI_ID_GEO_SETTINGS_BASE, client_rect);

        for (T* item : m_source->getSelected())
            item->preEdit();

        widget->show();
        widget->drop();
    }

    virtual void widget_closing(Reflected_Widget_EditArea* widget) override
    {
        widget->save_expanded_status();
    }

    virtual reflect::TypeDescriptor_Struct* getTypeDescriptor() override
    {
        reflect::TypeDescriptor_Struct* ret = new reflect::TypeDescriptor_Struct(reflect::EmptyStruct::initReflection);

        for (reflect::TypeDescriptor_Struct* typeDesc : my_typeDescriptors)
        {
            reflect::Member m{ typeDesc->alias, 0, typeDesc };
            m.expanded = typeDesc->expanded;
            ret->members.push_back(m);
            ret->size = typeDesc->size;
        }

        return ret;
    }

    virtual void read_obj(void* obj) override
    {
        if (m_source->getSelected().size() > 0)
        {
            T* item = m_source->getSelected()[0];
            for (reflect::TypeDescriptor_Struct* typeDesc : my_typeDescriptors)
                typeDesc->copy((char*)obj, item);
        }
    }

    virtual void post_edit() override
    {
        for (T* item : m_source->getSelected())
            item->postEdit();
    }

    virtual void write_obj_by_field(reflect::TypeDescriptor_Struct* /*flat_typeDescriptor*/, std::vector<int> tree_pos, void* obj) override
    {
        std::vector<int> branch;
        for (int i = 1; i < (int)tree_pos.size(); i++)
            branch.push_back(tree_pos[i]);

        if (branch.size() > 0)
        {
            reflect::TypeDescriptor_Struct* typeDesc = my_typeDescriptors[tree_pos[0]];
            reflect::Member* m = typeDesc->getTreeNode(branch);

            if (m->modified)
            {
                size_t offset = typeDesc->getTreeNodeOffset(branch);

                for (T* item : m_source->getSelected())
                    m->type->copy((char*)item + offset, (char*)obj + offset);

                m->modified = false;
            }
        }
    }

    virtual void save_expanded_status(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos) override
    {
        std::vector<int> branch;
        for (int i = 1; i < (int)tree_pos.size(); i++)
            branch.push_back(tree_pos[i]);

        reflect::TypeDescriptor_Struct* typeDesc = my_typeDescriptors[tree_pos[0]];
        reflect::Member* m = flat_typeDescriptor->getTreeNode(tree_pos);

        if (branch.size() > 0 && m)
        {
            reflect::Member* m2 = typeDesc->getTreeNode(branch);
            if (m2)
                m2->expanded = m->expanded;
        }
        else if (m)
        {
            typeDesc->expanded = m->expanded;
        }
    }

    virtual void init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos) override
    {
        std::vector<int> branch;
        for (int i = 1; i < (int)tree_pos.size(); i++)
            branch.push_back(tree_pos[i]);

        if (branch.size() > 0)
        {
            reflect::Member* m = flat_typeDescriptor->getTreeNode(tree_pos);

            reflect::TypeDescriptor_Struct* typeDesc = my_typeDescriptors[tree_pos[0]];
            reflect::Member* m2 = typeDesc->getTreeNode(branch);
            size_t offset = typeDesc->getTreeNodeOffset(branch);

            std::vector<T*> selected = m_source->getSelected();

            if (selected.size() > 1)
            {
                T* item_0 = selected[0];
                bool b = true;

                if (m->modified)
                    m->readwrite = true;
                else
                {
                    for (T* item : selected)
                    {
                        if (m2->type->isEqual((char*)item + offset, (char*)item_0 + offset) == false)
                            b = false;
                    }
                    m->readwrite = b;
                }
            }
            else if (selected.size() == 1)
            {
                m->readwrite = true;
            }
        }
    }

    virtual void write_attributes(reflect::TypeDescriptor_Struct* /*flat_typeDescriptor*/) override {}

    virtual void* getObj() override
    {
        if (m_source->getSelected().size() > 0)
            return m_source->getSelected()[0];
        return NULL;
    }

    // Unused pending investigation — kept for parity with original new_node_properties_flat_typedescriptor
    reflect::TypeDescriptor_Struct* new_flat_typedescriptor(std::vector<reflect::TypeDescriptor_Struct*> typeDescriptors)
    {
        reflect::TypeDescriptor_Struct* ret = new reflect::TypeDescriptor_Struct(reflect::EmptyStruct::initReflection);
        ret->size = 0;

        if (m_source->getSelected().size() > 0)
        {
            T* item = m_source->getSelected()[0];
            size_t size_inc = 0;

            for (reflect::TypeDescriptor_Struct* typeDesc : typeDescriptors)
            {
                reflect::TypeDescriptor* flat = typeDesc->get_flat_copy(item, 0);
                reflect::Member m{ typeDesc->alias, size_inc, flat };
                m.expanded = ((reflect::TypeDescriptor_Struct*)flat)->expanded;
                ret->members.push_back(m);
                size_inc += flat->size;
            }
            ret->size = size_inc;
        }

        return ret;
    }

    // Public state (accessed by tkt_properties_widget<T>)
    std::vector<reflect::TypeDescriptor_Struct*> my_typeDescriptors;
    bool waiting_for_item_selection = false;
    std::vector<int> item_selection_member;
    std::vector<u64> saved_uids;
    std::vector<T*> saved_selection;

private:
    tkt_set<T>* m_source;
};

//====================================================
// tkt_properties_widget<T>
//
// Generic property editor widget for tkt_properties_base<T>.
// Ported directly from Node_Properties_Widget.
//

template<typename T>
class tkt_properties_widget : public gui::IGUIElement
{
public:
    tkt_properties_widget(
        gui::IGUIEnvironment* env, gui::IGUIElement* parent,
        geometry_scene* g_scene_,
        tkt_properties_base<T>* base_,
        s32 id, core::rect<s32> rect
    ) : gui::IGUIElement(gui::EGUIET_ELEMENT, env, parent, id, rect),
        my_base(base_), g_scene(g_scene_), my_ID(id)
    {
        MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
        receiver->Register(this);
    }

    ~tkt_properties_widget()
    {
        if (my_widget)
            my_widget->remove();

        MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
        receiver->UnRegister(this);
    }

    void show()
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

    void onRefresh()
    {
        if (my_button)
            my_button->remove();

        int ypos = my_widget->getEditAreaHeight() + 8;
        core::rect<s32> br = core::rect<s32>(getRelativePosition().getWidth() - 80, ypos, getRelativePosition().getWidth() - 8, ypos + 36);

        my_button = new Flat_Button(Environment, this, OK_BUTTON_ID, br);
        my_button->setText(L"Apply");
    }

    void click_OK()
    {
        my_widget->write_by_field();
    }

    virtual bool OnEvent(const SEvent& event) override
    {
        if (event.EventType == EET_USER_EVENT)
        {
            if (event.UserEvent.UserData1 == USER_EVENT_REFLECTED_FORM_REFRESHED)
            {
                onRefresh();
                return true;
            }
            else if (event.UserEvent.UserData1 == USER_EVENT_SELECTION_CHANGED && my_base->waiting_for_item_selection == false)
            {
                if (my_widget)
                    my_widget->remove();

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
                my_base->item_selection_made();

                std::vector<int> tree_pos = my_base->item_selection_member;
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
                my_base->item_selection_over();
                return true;
            }
        }
        else if (event.EventType == EET_GUI_EVENT)
        {
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

private:
    int OK_BUTTON_ID = 0;
    int my_ID;

    Reflected_Widget_EditArea* my_widget = NULL;
    Flat_Button* my_button = NULL;

    geometry_scene* g_scene = NULL;
    tkt_properties_base<T>* my_base = NULL;
    gui::IGUIElement* edit_panel = NULL;
};

#endif

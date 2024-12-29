#ifndef _NODE_PROPERTIES_H_
#define _NODE_PROPERTIES_H_

#include <irrlicht.h>
#include "edit_classes.h"
#include "GUI_tools.h"

using namespace irr;

class geometry_scene;
class Flat_Button;

namespace reflect {
    class TypeDescriptor_Struct;
}

class Reflected_SceneNode;
class Node_Properties_Base;

class Node_Properties_Widget : public gui::IGUIElement
{

public:
    Node_Properties_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene*, Node_Properties_Base*, s32 id, core::rect<s32> rect);
    ~Node_Properties_Widget();

    void show();
    void onRefresh();

    void click_OK();
    virtual bool OnEvent(const SEvent& event);

private:

    int OK_BUTTON_ID = 0;
    int my_ID;

    Reflected_Widget_EditArea* my_widget = NULL;
    Flat_Button* my_button = NULL;

    geometry_scene* g_scene = NULL;
    Node_Properties_Base* my_base = NULL;
    gui::IGUIElement* edit_panel = NULL;
};

class Node_Properties_Base : public reflected_tool_base
{
public:
    Node_Properties_Base(std::wstring name, int my_id, gui::IGUIEnvironment* env, multi_tool_panel* panel);

    static std::vector<reflect::TypeDescriptor_Struct*> GetTypeDescriptors(geometry_scene* geo_scene);

    void refresh_types();

    reflect::TypeDescriptor_Struct* new_node_properties_flat_typedescriptor(std::vector<reflect::TypeDescriptor_Struct*>);

    // Inherited via reflected_tool_base
    //virtual void initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel*);
    virtual void show();
    virtual void widget_closing(Reflected_Widget_EditArea*) override;
    virtual reflect::TypeDescriptor_Struct* getTypeDescriptor() override;
    virtual void read_obj(void* obj) override;
    virtual void post_edit() override;
    virtual void write_obj_by_field(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos, void* obj) override;
    virtual void save_expanded_status(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos) override;
    virtual void init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos) override;
    virtual void write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor) {}
    virtual void* getObj() override;

    std::vector<reflect::TypeDescriptor_Struct*> my_typeDescriptors;

};

class NodeProperties_Tool
{

public:

    static void initialize(Node_Properties_Base* base_, multi_tool_panel* panel_)
    {
        base = base_;
        panel = panel_;
    }

    static void show()
    {
        panel->add_tool(base);
    }

    static Node_Properties_Base* get_base()
    {
        return base;
    }

    static Node_Properties_Base* base;
    static multi_tool_panel* panel;
};













#endif

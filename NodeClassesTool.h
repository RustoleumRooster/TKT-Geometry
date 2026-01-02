#ifndef _NODE_CLASSES_TOOL_H_
#define _NODE_CLASSES_TOOL_H_

#include <irrlicht.h>
#include "Reflection.h"
#include "GUI_tools.h"
#include "tree_struct.h"
#include "reflect_custom_types.h"

using namespace irr;

class Node_Classes_Base;
class geometry_scene;
class Reflected_Widget_EditArea;
class Flat_Button;

namespace reflect
{
    struct TypeDescriptor_SN_Struct;
}

struct node_class_item
{
    //ATTRIBUTES:
    //display attributes contained in the reflected type descriptor 
    //initialized through write_attributes()
    //and accesible from addFormWidget()
    struct attributes
    {
        bool placeable;
        bool selected;
        std::string name;
    };
    
    //REFLECTED MEMBERS:
    int id;
    std::string class_name;

    //DEFINES:
    DECLARE_TREE_FUNCTIONS(node_class_item)
    REFLECT_CUSTOM_TREE_STRUCT()

    //NON REFLECTED MEMBERS:
    reflect::TypeDescriptor_SN_Struct* typeDescriptor;
    bool placeable;
    bool selected = false;
    node_class_item* find_type(reflect::TypeDescriptor_Struct* tD);
    void set_selected_recursive(Node_Classes_Base*);
};

class Node_Classes_Widget : public gui::IGUIElement
{

public:
    Node_Classes_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene*, Node_Classes_Base*, s32 id, core::rect<s32> rect);
    ~Node_Classes_Widget();

    void show();
    void refresh();
    void onRefresh();

    void click_OK();
    virtual bool OnEvent(const SEvent& event);

    int OK_BUTTON_ID = 0;
    int STATIC_TEXT_ID = 0;
    int my_ID;

    Reflected_Widget_EditArea* my_widget = NULL;
    Flat_Button* my_button = NULL;
    gui::IGUIStaticText* my_static_text = NULL;

    geometry_scene* g_scene = NULL;
    Node_Classes_Base* my_base = NULL;
    gui::IGUIElement* edit_panel = NULL;
};

class Node_Classes_Base : public simple_reflected_tool_base
{
public:
    Node_Classes_Base(std::wstring name, int my_id, gui::IGUIEnvironment* env, multi_tool_panel* panel);

    virtual void show();

    virtual reflect::TypeDescriptor_Struct* getFlatTypeDescriptor();

    virtual void widget_closing(Reflected_Widget_EditArea* widget) override;

    virtual void set_scene(geometry_scene* g_scene_) override;

    //virtual void initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel* panel_) override;

    virtual void init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos) override;

    virtual void write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor) override;

    virtual void toggle_expanded_struct(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos) override;

    void set_selected(node_class_item* item);

    std::string getStaticString();

    reflect::TypeDescriptor_Struct* getSelectedTypeDescriptor() { return selected_type; }

    void select(int sel);

    virtual void* getObj() {
        return &m_struct;
    }

    void build_struct();

private:
    
    node_class_item m_struct;
    reflect::TypeDescriptor_Struct* selected_type = NULL;
};

class Node_Classes_Tool
{
    static Node_Classes_Base* base;
    static multi_tool_panel* panel;

public:

    static void show()
    {
        panel->add_tool(base);
    }

    static void initialize(Node_Classes_Base* base_, multi_tool_panel* panel_)
    {
        base = base_;
        panel = panel_;
    }

    static Node_Classes_Base* get_base()
    {
        return base;
    }
};

#endif
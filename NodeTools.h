#ifndef _NODE_TOOLS_H_
#define _NODE_TOOLS_H_

#include <irrlicht.h>
#include "Reflection.h"
#include "GUI_tools.h"

using namespace irr;

class Node_Classes_Base;
class geometry_scene;
class Reflected_Widget_EditArea;
class Flat_Button;

class Node_Classes_Widget : public gui::IGUIElement
{

public:
    Node_Classes_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene*, Node_Classes_Base*, s32 id, core::rect<s32> rect);
    ~Node_Classes_Widget();

    void show();
    void onRefresh();

    void click_OK();
    virtual bool OnEvent(const SEvent& event);

    int OK_BUTTON_ID = 0;
    int my_ID;

    Reflected_Widget_EditArea* my_widget = NULL;
    Flat_Button* my_button = NULL;

    geometry_scene* g_scene = NULL;
    Node_Classes_Base* my_base = NULL;
    gui::IGUIElement* edit_panel = NULL;
};

struct node_class_item
{
    int id;
    std::string class_name;
    std::vector<node_class_item> sub_classes;
    bool expanded;

    REFLECT_CUSTOM()

    reflect::TypeDescriptor_Struct* typeDescriptor; //not reflected

    node_class_item* find_type(reflect::TypeDescriptor_Struct* tD)
    {
        if (this->typeDescriptor == tD)
            return this;
        
        else for (node_class_item& sub : sub_classes)
        {
            node_class_item* res = sub.find_type(tD);
            if (res)
                return res;
        }
        
        return NULL;
    }
};

struct node_classes_struct
{
    int nClasses;
    std::vector<node_class_item> classes;
    REFLECT()
};

class Node_Classes_Base : public simple_reflected_tool_base
{
public:

    virtual void show();

    virtual void initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel* panel_);

    virtual void init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos);

    virtual void* getObj() {
        return &m_struct;
    }

private:
    void build_struct();

    node_class_item m_struct;
   // node_classes_struct m_struct;
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
};

#endif
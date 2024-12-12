#ifndef _NODE_INSTANCES_TOOL_H_
#define _NODE_INSTANCES_TOOL_H_

#include <irrlicht.h>
#include "Reflection.h"
#include "GUI_tools.h"
#include "tree_struct.h"
#include "reflect_custom_types.h"

using namespace irr;

class Node_Instances_Base;
class geometry_scene;
class Reflected_Widget_EditArea;
class Flat_Button;


struct node_instance
{
    //ATTRIBUTES:
    struct attributes
    {
        bool selected;
    };

    //REFLECTED MEMBERS:
    int id;
    std::string name;
    char* node_ptr = NULL;
 
    //DEFINES:
    REFLECT_CUSTOM_STRUCT()

    //NON REFLECTED MEMBERS:
    
    bool selected = false;
    void write_attributes(reflect::Member* m_struct);
};

struct node_tree_item
{
    enum
    {
        ITEM_TYPE_CLASS,
        ITEM_TYPE_INSTANCE
    };

    //ATTRIBUTES:
    //display attributes contained in the reflected type descriptor 
    //initialized through write_attributes()
    //and accesible from addFormWidget()
    struct attributes
    {
        bool selected;
    };

    //REFLECTED MEMBERS:
    int item_type;
    int id;
    std::string class_name;
    std::vector<node_instance> instances;
    
    //DEFINES:
    DECLARE_TREE_FUNCTIONS(node_tree_item)
    REFLECT_CUSTOM_TREE_STRUCT()

    //NON REFLECTED MEMBERS:
    bool hasSubs;
    reflect::TypeDescriptor_Struct* typeDescriptor;
    node_tree_item* find_type(reflect::TypeDescriptor_Struct* tD);
    bool has_items_recursive();
};

class Node_Instances_Widget : public gui::IGUIElement
{

public:
    Node_Instances_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene*, Node_Instances_Base*, s32 id, core::rect<s32> rect);
    ~Node_Instances_Widget();

    void show();
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
    Node_Instances_Base* my_base = NULL;
    gui::IGUIElement* edit_panel = NULL;
};

class Node_Instances_Base : public simple_reflected_tool_base
{
public:

    virtual void show();

    virtual reflect::TypeDescriptor_Struct* getFlatTypeDescriptor();

    virtual void widget_closing(Reflected_Widget_EditArea* widget);

    virtual void initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel* panel_);

    virtual void init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos);

    virtual void write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor);

    std::string getTypesString();

    void select(Reflected_SceneNode* sel, bool shift);

    virtual void* getObj() {
        return &m_struct;
    }
    void build_struct();

private:
    
    void build_initial_struct();

    node_tree_item m_struct;
    node_tree_item m_initial_struct;
    reflect::TypeDescriptor_Struct* selected_type = NULL;
};

class Node_Instances_Tool
{
    static Node_Instances_Base* base;
    static multi_tool_panel* panel;

public:

    static void show()
    {
        panel->add_tool(base);
    }

    static void initialize(Node_Instances_Base* base_, multi_tool_panel* panel_)
    {
        base = base_;
        panel = panel_;
    }
};

#endif
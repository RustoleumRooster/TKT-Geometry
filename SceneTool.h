#ifndef _TKT_SCENE_TOOL
#define _TKT_SCENE_TOOL
#include "GUI_tools.h"
#include "tree_struct.h"

class Scene_Instances_Base;

struct scene_instance_item
{
    //ATTRIBUTES:
    //display attributes contained in the reflected type descriptor 
    //initialized through write_attributes()
    //and accesible from addFormWidget()
    struct attributes
    {
        bool selected;
        bool editable;
    };

    //REFLECTED MEMBERS:
    int id;
    std::string scene_name;

    //DEFINES:
    DECLARE_TREE_FUNCTIONS(scene_instance_item)
    REFLECT_CUSTOM_TREE_STRUCT()

    //NON REFLECTED MEMBERS:

    bool selected = false;
    bool editable = false;
    //node_class_item* find_type(reflect::TypeDescriptor_Struct* tD);
    void set_selected_recursive(Scene_Instances_Base*);
};

class Flat_Button;

class Scene_Instances_Widget : public gui::IGUIElement
{

public:
    Scene_Instances_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, Scene_Instances_Base*, s32 id, core::rect<s32> rect);
    ~Scene_Instances_Widget();

    void show();
    void refresh();
    void onRefresh();

    void click_OK();
    
    virtual bool OnEvent(const SEvent& event);
    int read_formfield_id(FormField*);

    int OK_BUTTON_ID = 0;
    //int STATIC_TEXT_ID = 0;
    int my_ID;
    int editing_element_id = 0;

    Reflected_Widget_EditArea* my_widget = NULL;
    Flat_Button* my_button = NULL;
    //gui::IGUIStaticText* my_static_text = NULL;

    //geometry_scene* g_scene = NULL;
    Scene_Instances_Base* my_base = NULL;
    gui::IGUIElement* edit_panel = NULL;
};

class Scene_Instances_Base : public simple_reflected_tool_base
{
public:
    Scene_Instances_Base(std::wstring name, int my_id, gui::IGUIEnvironment* env, multi_tool_panel* panel);

    virtual void show();

    virtual reflect::TypeDescriptor_Struct* getFlatTypeDescriptor();

    virtual void widget_closing(Reflected_Widget_EditArea* widget) override;

    //virtual void set_scene(geometry_scene* g_scene_) override;

    //virtual void initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel* panel_) override;

    virtual void init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos) override;

    virtual void write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor) override;

    virtual void toggle_expanded_struct(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos) override;

    virtual void write_obj_by_field(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos, void* obj) override;

    void set_selected(scene_instance_item* item);

    void delete_item();
    void rename_item();

    std::string getStaticString();

    int getSelection() { return selection; }

    void select(int sel);
    void right_click_item(core::vector2di pos, int sel);

    virtual void* getObj() {
        return &m_struct;
    }

    void build_struct();

private:

    scene_instance_item m_struct;
    //reflect::TypeDescriptor_Struct* selected_type = NULL;
    int selection=0;
    bool bNameEditing = false;
    int rightClickedItem = 0;
};

class Scene_Instances_Tool
{
    static Scene_Instances_Base* base;
    static multi_tool_panel* panel;

public:

    static void show()
    {
        panel->add_tool(base);
    }

    static void initialize(Scene_Instances_Base* base_, multi_tool_panel* panel_)
    {
        base = base_;
        panel = panel_;
    }

    static Scene_Instances_Base* get_base()
    {
        return base;
    }
};

#endif

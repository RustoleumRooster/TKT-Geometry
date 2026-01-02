#ifndef _GUI_TOOLS_H_
#define _GUI_TOOLS_H_

#include <irrlicht.h>
#include <vector>
#include <string>
#include "Reflection.h"
#include "tree_struct.h"

using namespace irr;

class tool_base;
class geometry_scene;
class tool_header;
class CGUIScrollBar2;
class reflected_tool_base;
class FormField;

class Reflected_Widget_EditArea : public irr::gui::IGUIElement
{
public:
    Reflected_Widget_EditArea(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene*, reflected_tool_base*, s32 id, core::rect<s32> rect);
    ~Reflected_Widget_EditArea();

    virtual void draw();
    virtual bool OnEvent(const SEvent& event);

    void show(bool editable, void* obj);
    void refresh();
    void write();
    void write_by_field();
    void save_expanded_status();
	void setColumns(std::vector<int>);

    int getMaxFormsHeight();
    int getFormsHeight();
    int getEditAreaHeight();
    void setEditAreaScrollPos(int pos);

	std::vector<int> override_columns;

    CGUIScrollBar2* my_scrollbar = NULL;
    gui::IGUIElement* edit_panel = NULL;
    Reflected_GUI_Edit_Form* form = NULL;
    reflect::TypeDescriptor_Struct* m_typeDesc = NULL;
    geometry_scene* g_scene = NULL;
    reflected_tool_base* my_base = NULL;
    bool b_editable = true;
    void* temp_object = NULL;

    int scroll_offset = 0;
    int my_ID;
    int scrollbar_ID = -1;

};

class multi_tool_panel : public gui::IGUIElement
{
public:

    multi_tool_panel(gui::IGUIEnvironment* env, gui::IGUIElement* parent,s32 id,core::rect<s32> rect);
    //TODO - destructor ?

    void resize(core::rect<s32> rect);

    void add_tool(tool_base*);
    void remove_tool(int tool_no);

    void show_tool(int tool_no);
    void do_layout();
    void nameChange(tool_base*);

    virtual void draw();
    virtual bool OnEvent(const SEvent& event);

    gui::IGUIElement* getClientRect();

    void updateClientRect();
    gui::IGUIElement* clientRect;
    std::vector<tool_base*> my_tools;
    std::vector<tool_header*> my_headers;
    int tool_header_height;
};

class tool_header : public gui::IGUIElement
{
public:
    tool_header(gui::IGUIEnvironment* env, gui::IGUIElement* parent,s32 id,core::rect<s32> rect);

    virtual void draw();
    virtual void setText(std::wstring txt);
    virtual bool OnEvent(const SEvent& event);

    gui::IGUIStaticText* my_text = NULL;
    gui::IGUIButton* CloseButton;
   // std::wstring text;
    int tool_no;
    bool can_select = true;
    bool is_selected = false;
    bool hovered = false;
};

class tool_base
{
public:
    tool_base(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, multi_tool_panel*);
    virtual void set_scene(geometry_scene* g_scene_);

    virtual void show() = 0;
    virtual std::wstring getName(){return name;}
    virtual void setName(std::wstring txt);
    virtual int getID() {return my_ID;}

protected:
    gui::IGUIEnvironment* env = NULL;
    geometry_scene* g_scene = NULL;
    multi_tool_panel* panel = NULL;
    std::wstring name;
    int my_ID;
};

class reflected_tool_base : public tool_base
{
public:
    reflected_tool_base(std::wstring name, int my_id, gui::IGUIEnvironment* env, multi_tool_panel* panel)
        : tool_base(name, my_id, env, panel) {}

    virtual void widget_closing(Reflected_Widget_EditArea*) = 0;
    virtual reflect::TypeDescriptor_Struct* getTypeDescriptor() = 0;
    virtual void read_obj(void* obj) = 0;
    virtual void write_obj(void* obj) {};
    virtual void post_edit() {};
    virtual void write_obj_by_field(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos, void* obj) = 0;
    virtual void save_expanded_status(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos) = 0;
    virtual void init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos) = 0;
    virtual void toggle_expanded_struct(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos);
    virtual void write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor) = 0;
    virtual void* getObj() = 0;

    //virtual void initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel*) = 0;
};

class simple_reflected_tool_base : public reflected_tool_base
{
public:
    simple_reflected_tool_base(std::wstring name, int my_id, gui::IGUIEnvironment* env, multi_tool_panel* panel) 
        : reflected_tool_base(name, my_id, env, panel) {}

    virtual void widget_closing(Reflected_Widget_EditArea*) override;
    virtual reflect::TypeDescriptor_Struct* getTypeDescriptor() override;
    virtual void read_obj(void* obj) override;
    virtual void write_obj(void* obj) override;
    virtual void write_obj_by_field(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos, void* obj) override;

    virtual reflect::TypeDescriptor_Struct* getFlatTypeDescriptor();
    virtual void deserialize_by_field(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos, void* flat_object) {};
    virtual void serialize_flat_obj(void* flat_object);
    virtual void deserialize_flat_obj(void* flat_object);
    virtual void save_expanded_status(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos);
    virtual void toggle_expanded_struct(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos);
    virtual void init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos);
    virtual void write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor) {}

    //virtual void initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel*) = 0;
    virtual void* getObj() = 0;

protected:
    reflect::TypeDescriptor_Struct* m_typeDescriptor=NULL;

};

template<typename T>
class simple_reflected_tree_tool_base : public simple_reflected_tool_base
{
public:
    simple_reflected_tree_tool_base(std::wstring name, int my_id, gui::IGUIEnvironment* env, multi_tool_panel* panel);

    virtual void widget_closing(Reflected_Widget_EditArea*);
    virtual void toggle_expanded_struct(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos);
    virtual void write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor);

    //virtual void initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel*);
    virtual void* getObj() {
        return &m_struct;
    }

protected:

    T m_struct;

};


struct folder_tree_item
{
    enum
    {
        ITEM_TYPE_CLASS,
        ITEM_TYPE_INSTANCE
    };

    //ATTRIBUTES:
    struct attributes
    {
        bool selected = false;
        bool isDirectory = false;
    };

    //REFLECTED MEMBERS:
    //int item_type;
    int id = 0;
    std::string name;

    //DEFINES:
    DECLARE_TREE_FUNCTIONS(folder_tree_item)
    REFLECT_CUSTOM_TREE_STRUCT()

    //NON REFLECTED MEMBERS:
    bool hasSubs = true;
    bool isDirectory = false;
    bool selected = false;

};

class TextureImage
{
public:
    TextureImage(video::ITexture*, bool repeat = false);
    ~TextureImage();

    void render();
    core::dimension2du getDimensions();

private:
    video::ITexture* m_texture = NULL;
    scene::SMeshBuffer* Buffer = NULL;

};

#endif

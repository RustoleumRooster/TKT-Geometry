#pragma once

#ifndef _LIGHT_SYSTEM_TOOL_H_
#define _LIGHT_SYSTEM_TOOL_H_

#include <string>
#include "GUI_tools.h"
#include "CameraPanel.h"
#include "fonts.h"

class CameraQuad;
class LM_Viewer_Window;
class Light_System_Base;
class Vulkan_Module;
class Module_GUI_Element;
class Vulkan_Workflow;

class System_GUI_Element : public gui::IGUIElement
{
public:
    //! constructor
    System_GUI_Element(Light_System_Base* base, gui::IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle);

    //! destructor
    virtual ~System_GUI_Element();

    virtual void draw();

    virtual bool OnEvent(const SEvent& event);

    void left_click();

    Vulkan_Workflow* m_workflow = NULL;
    Light_System_Base* my_base;

};

class Module_GUI_Element : public gui::IGUIElement
{
public:
    //! constructor
    Module_GUI_Element(Light_System_Base* base, gui::IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle);

    //! destructor
    virtual ~Module_GUI_Element();

    virtual void draw();
    virtual void move(vector2di d) override;

    virtual bool OnEvent(const SEvent& event);
    void resize(core::dimension2di);

    void left_click();

    std::wstring text;
    vector2di click_pos;
    vector2di drag_pos;
    bool bDragging = false;
    bool bMouseDown = false;
    Vulkan_Module* m_module = NULL;
    Light_System_Base* my_base;
    int module_index = 0;
};
namespace reflect
{
    class inout_gui_type;
}
class Hookup_GUI_Element : public gui::IGUIElement
{
public:
    //! constructor
    Hookup_GUI_Element(Light_System_Base* base, gui::IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle);

    Vulkan_Module* m_module = NULL;
    reflect::inout_gui_type* m_inout = NULL;
    Light_System_Base* my_base;
    int module_index = 0;
    int member_idx = 0;

    virtual void draw();
    //virtual void move(vector2di d) override;

    virtual bool OnEvent(const SEvent& event);

    void left_click();

    vector2di click_pos;
    bool bMouseDown = false;
};

class LM_System_Panel : public TestPanel_2D
{
public:
    LM_System_Panel(gui::IGUIEnvironment* environment, video::IVideoDriver* driver, gui::IGUIElement* parent, LM_Viewer_Window* win, s32 id, core::rect<s32> rectangle);
    ~LM_System_Panel();

    virtual scene::ICameraSceneNode* getCamera();
    virtual void Initialize(geometry_scene* geo_scene);
    virtual void setAxis(int);
    virtual void render();

    virtual bool OnEvent(const SEvent& event);
    //virtual void left_click(core::vector2di pos);
    virtual void drawGrid(video::IVideoDriver* driver, const video::SMaterial material);

    void setGridSpacing(int spacing);
    virtual void resize(core::dimension2d<u32> new_size);

    void clear_childern();
    virtual void startCameraPan(vector2di) override;
    virtual void cameraPan(vector2di) override;

    void showTriangles(bool);
    void showLightmap(bool);

    video::ITexture* uv_texture = NULL;
    void showMaterialGroup(int mg);

    void set_base(Light_System_Base* b) { m_base = b; }

    System_GUI_Element* system_gui_element = NULL;

protected:
    vector2di cameraDragStartPos;
    

    virtual void left_click(core::vector2di);
    virtual void right_click(core::vector2di);
    int gridSpace = 128;

    virtual void OnMenuItemSelected(gui::IGUIContextMenu* menu);

    Light_System_Base* m_base = NULL;
};

struct lm_system_struct
{
    bool aVariable = false;

    REFLECT()
};

struct lm_system_options_struct
{
    bool option_1;
};

class Flat_Button;

class Light_System_Widget : public gui::IGUIElement
{

public:
    Light_System_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, Light_System_Base*, s32 id, core::rect<s32> rect);
    ~Light_System_Widget();

    void show();
    void onRefresh();

    //u32 get_total_height() { return total_height; }

    void click_OK();
    virtual bool OnEvent(const SEvent& event);

    int OK_BUTTON_ID = 0;
    int my_ID;

    Reflected_Widget_EditArea* my_widget = NULL;

    //mini_header* header = NULL;
    Flat_Button* my_button = NULL;
    gui::IGUIImage* my_image = NULL;
    gui::IGUIStaticText* my_text = NULL;

    geometry_scene* g_scene = NULL;
    Light_System_Base* my_base = NULL;
    gui::IGUIElement* edit_panel = NULL;

    //u32 total_height = 0;
};

class Light_System_Base : public simple_reflected_tool_base
{
public:
    Light_System_Base(std::wstring name, int my_id, gui::IGUIEnvironment* env, multi_tool_panel* panel) :
        simple_reflected_tool_base(name, my_id, env, panel), lm_options{ true }
        {
        font = (AGG_TT_Font*)(env->getSkin()->getFont());
        }

    ~Light_System_Base();

    virtual void show();
    void refresh();
    void select(int sel);
    virtual void set_scene(geometry_scene* gs) override;

    virtual void initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel* panel_)
    {}

    void initialize();

    void setCameraQuad(CameraQuad* cameraQuad_) { cameraQuad = cameraQuad_; }
    void close_uv_panel();
    void refresh_panel_view();

    virtual void* getObj() {
        return &m_struct;
    }

    virtual void* getOptions() {
        return &lm_options;
    }

    virtual void init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos);
    virtual void write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor);

    void save_workflow();

    std::string GetSelectedString();
    TextureInfo* GetSelectedTexture();

    s32 getWidgetHeight();
    void setWidgetHeight(s32);

    void draw_text();
    void draw_lines(video::IVideoDriver*);
//private:


    AGG_TT_Font* font = NULL;

    CachedText_Manager* cached_texts[2] = { NULL,NULL };

    std::vector<std::pair<Hookup_GUI_Element*, Hookup_GUI_Element*>> gui_connections;

    Vulkan_Workflow* workflow = NULL;

    int selection = -1;

    LM_System_Panel* m_panel = NULL;
    CameraQuad* cameraQuad = NULL;

    lm_system_options_struct lm_options;
    lm_system_struct m_struct;

    s32 widget_total_height = 0;

};

class Light_System_Tool
{
    static Light_System_Base* base;
    static multi_tool_panel* panel;

public:

    static void show()
    {
        panel->add_tool(base);
    }

    static Light_System_Base* get_base()
    {
        return base;
    }

    static void initialize(Light_System_Base* base_,  multi_tool_panel* panel_)
    {
        base = base_;
        panel = panel_;
    }
};

#endif
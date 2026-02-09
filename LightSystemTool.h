#pragma once

#ifndef _LIGHT_SYSTEM_TOOL_H_
#define _LIGHT_SYSTEM_TOOL_H_

#include <string>
#include "GUI_tools.h"
#include "CameraPanel.h"

class CameraQuad;
class LM_Viewer_Window;
class Light_System_Base;
class Vulkan_Module;
class Module_GUI_Element;
class Vulkan_Workflow;

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
    virtual void cameraPan(vector2di);

    void showTriangles(bool);
    void showLightmap(bool);

    video::ITexture* uv_texture = NULL;
    void showMaterialGroup(int mg);

    void set_base(Light_System_Base* b) { m_base = b; }

protected:

    virtual void left_click(core::vector2di);
    virtual void right_click(core::vector2di);
    int gridSpace = 128;

    virtual void OnMenuItemSelected(gui::IGUIContextMenu* menu);

    Light_System_Base* m_base = NULL;
};

struct lm_system_struct
{

};

struct lm_system_options_struct
{
    bool option_1;
};

struct module_connector
{
    std::wstring name;
    module_connector* link_to = NULL;
    vector2di pos;
};

struct Reflected_Module
{
    std::wstring name;
    vector2di pos;
    dimension2du size;
    Vulkan_Module* m_module = NULL;
    std::vector<module_connector> inputs;
    std::vector<module_connector> outputs;

    void addComponents(reflect::TypeDescriptor_Struct*, Vulkan_Module* obj);

    Module_GUI_Element* get_GUI_Element(gui::IGUIEnvironment* environment, gui::IGUIElement* parent, s32 id);
};

class Light_System_Base : public simple_reflected_tool_base
{
public:
    Light_System_Base(std::wstring name, int my_id, gui::IGUIEnvironment* env, multi_tool_panel* panel) :
        simple_reflected_tool_base(name, my_id, env, panel), lm_options{ true } {}

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

    std::string GetSelectedString();
    TextureInfo* GetSelectedTexture();

    s32 getWidgetHeight();
    void setWidgetHeight(s32);

//private:

    Vulkan_Workflow* workflow = NULL;

    std::vector<Reflected_Module> modules;

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
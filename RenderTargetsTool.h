#pragma once

#ifndef _TKT_RENDER_TOOL_
#define _TKT_RENDER_TOOL_

#include <irrlicht.h>
#include "Reflection.h"
#include "CameraPanel.h"
#include "edit_env.h"
#include "GUI_tools.h"


class TwoTriangleSceneNode;


class RenderTargetPanel : public TestPanel
{
public:
    RenderTargetPanel(gui::IGUIEnvironment* environment, video::IVideoDriver* driver, gui::IGUIElement* parent, s32 id, core::rect<s32> rectangle);
    ~RenderTargetPanel();

    virtual scene::ICameraSceneNode* getCamera();
    virtual void Initialize(scene::ISceneManager* smgr, geometry_scene* geo_scene);
    virtual void render();
    virtual void resize(core::dimension2d<u32> new_size);
    virtual void SetMeshNodesVisible();
    virtual bool GetScreenCoords(core::vector3df V, core::vector2di& out_coords);
    void connect_image(ViewResizeObject* view);

    void setCameraQuad(CameraQuad* q) { cameraQuad = q; }

    void setMaterial(s32 someMaterial);

private:

    CameraQuad* cameraQuad = NULL;
    TwoTriangleSceneNode* render_node = NULL;
    video::E_MATERIAL_TYPE my_material = video::EMT_SOLID;

};

class Render_Tool_Base;

class Render_Tool_Widget : public gui::IGUIElement
{

public:
    Render_Tool_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene*, Render_Tool_Base*, s32 id, core::rect<s32> rect);
    ~Render_Tool_Widget();

    void show();

private:
    int my_ID;

    geometry_scene* g_scene = NULL;
    Render_Tool_Base* my_base = NULL;
};

class Render_Tool_Base : public simple_reflected_tool_base
{
    struct render_options_struct
    {
        int some_int;
        REFLECT()
    };

public:

    Render_Tool_Base(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, multi_tool_panel* panel_);

    ~Render_Tool_Base() {

        if (view_panel)
            delete view_panel;
    }

    virtual void show();

    void set_scene(geometry_scene* g_scene_, scene::ISceneManager* smgr_);
    //virtual void initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel* panel_) {}
    //virtual void initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel* panel_, scene::ISceneManager* smgr);

    void setCameraQuad(CameraQuad* cameraQuad_);
    void close_panel();
    void refresh_panel_view();

    void connect_image(ViewResizeObject* view);

    virtual void* getObj() {
        return &m_struct;
    }
    void setMaterial(s32 someMaterial)
    {
        my_material = video::E_MATERIAL_TYPE(someMaterial);

        if (view_panel)
            view_panel->setMaterial(someMaterial);
    }

    virtual void init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos);
    virtual void write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor);
    void setRenderList(RenderList* renderList_);

    video::ITexture* getRender();
    video::ITexture* getRTT();
    video::ITexture* getRTT2();
private:

    int selection = -1;

    scene::ISceneManager* smgr = NULL;
    RenderTargetPanel* view_panel = NULL;
    CameraQuad* cameraQuad = NULL;
    RenderList* renderList = NULL;
    video::E_MATERIAL_TYPE my_material = video::EMT_SOLID;

    //uv_editor_struct m_struct{ 0,true,true,true,true,16 };
    render_options_struct m_struct;

    ViewResizeObject* my_view = NULL;

    friend class UV_Editor_Widget;
};

class Render_Tool
{
    static Render_Tool_Base* base;
    static multi_tool_panel* panel;

public:

    static void show()
    {
        panel->add_tool(base);
    }

    static void initialize(Render_Tool_Base* base_, multi_tool_panel* panel_)
    {
        base = base_;
        panel = panel_;
    }

    static Render_Tool_Base* get_base()
    {
        return base;
    }

    static void connect_image(ViewResizeObject* view)
    {
        if (base)
            base->connect_image(view);
    }
    static void disconnect()
    {
        if (base)
            base->connect_image(NULL);
    }

    static video::ITexture* getRender()
    {
        if (base)
        {
            return base->getRender();
        }
        return NULL;
    }

    static video::ITexture* getRTT()
    {
        if (base)
        {
            return base->getRTT();
        }
        return NULL;
    }


};

#endif

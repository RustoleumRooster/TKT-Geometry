#ifndef _UV_TOOL_H_
#define _UV_TOOL_H_

#include <string>
#include "GUI_tools.h"
#include "CGUIWindow.h"
#include "CameraPanel.h"

class TextureImage
{
public:
    TextureImage(video::ITexture*);
    ~TextureImage();

    void render();
    core::dimension2du getDimensions();

private:
    video::ITexture* m_texture = NULL;
    scene::SMeshBuffer* Buffer = NULL;

};

class UV_Editor_Window;

class UV_Editor_Panel : public TestPanel_2D
{
public:
    UV_Editor_Panel(IGUIEnvironment* environment, video::IVideoDriver* driver, IGUIElement* parent, UV_Editor_Window* win, s32 id, core::rect<s32> rectangle);
    ~UV_Editor_Panel();

    virtual scene::ICameraSceneNode* getCamera();
    virtual void setAxis(int);
    virtual void render();
    virtual bool OnEvent(const SEvent& event);
    virtual void drawGrid(video::IVideoDriver* driver, const video::SMaterial material);

    void setGridSpacing(int spacing);
    virtual void resize(core::dimension2d<u32> new_size);

    void showTriangles(bool);
    void showLightmap(bool);

    video::ITexture* uv_texture = NULL;
    void showMaterialGroup(int mg);

protected:
    void make_face(polyfold* pf, int f_no, video::ITexture* face_texture);

    virtual void left_click(core::vector2di);
    virtual void right_click(core::vector2di);
    int gridSpace = 128;

    virtual void OnMenuItemSelected(IGUIContextMenu* menu);

    int current_mat_group = -1;
    int my_face_no = -1;

    int original_brush = -1;
    int original_face = -1;
    //polyfold uv_poly;
    std::vector<int> vertex_index;

    //video::ITexture* lm_tex = NULL;

    bool bRenderLightmap = true;
    
    TextureImage* my_image = NULL;

    UV_Editor_Window* my_window = NULL;
    //UV_Editor_Base* my_base = NULL;
    geometry_scene* uv_scene = NULL;
    //CameraQuad* my_camera_quad = NULL;
};


class UV_Editor_Base;
class polyfold;

class UV_Editor_Window : public gui::CGUIWindow
{
public:
    UV_Editor_Window(gui::IGUIEnvironment* env, gui::IGUIElement* parent, UV_Editor_Base* my_base, geometry_scene* g_scene_, s32 id, core::rect<s32> rect);
    ~UV_Editor_Window();

    void show();
    //void refresh();

    virtual bool OnEvent(const SEvent& event);

    virtual void draw();
    void set_poly(polyfold pf);
    void set_texture(video::ITexture* tex);
    void click_OK();
    TextureImage* getTextureImage();

private:

    UV_Editor_Base* my_base = NULL;
    UV_Editor_Panel* my_panel = NULL;
    geometry_scene* uv_scene = NULL;
    video::ITexture* my_texture = NULL;
    TextureImage* my_image = NULL;

    gui::IGUIButton* OK_Button = NULL;
};


struct uv_vertex
{
    f32 X;
    f32 Y;
};

struct uv_triangle
{
    //edges
    int A;
    int B;
    int C;

    //points
    int X;
    int Y;
    int Z;
};

struct uv_face
{
    std::vector<uv_vertex> vertexes;
    std::vector<uv_triangle> triangles;
};

class UV_Editor_Base : public tool_base
{
public:

    virtual void show();

    virtual void initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel* panel_);

    void widget_closing(Reflected_Widget_EditArea*);


    void click_OK();
    void window_closed();

    void make_custom_surface_group(polyfold pf);

private:

    
    video::ITexture* my_texture = NULL;
    std::vector<int> vertex_index;
    bool use_geometry_brush;

    int my_face_no = -1;

    int original_brush = -1;
    int original_face = -1;

    polyfold uv_poly;

    //friend class UV_Editor_Tool;
    friend class UV_Editor_Panel;
};

class UV_Editor_Tool
{
    static UV_Editor_Base* base;
    static multi_tool_panel* panel;

public:

    static void show()
    {
        base->show();
        //panel->add_tool(base);
    }

    static void initialize(UV_Editor_Base* base_, multi_tool_panel* panel_)
    {
        base = base_;
        panel = panel_;
    }


};

#endif
#ifndef _UV_TOOL_H_
#define _UV_TOOL_H_

#include <string>
#include "GUI_tools.h"
#include "CameraPanel.h"

class UV_Editor_Panel : public TestPanel_2D
{
public:
    UV_Editor_Panel(IGUIEnvironment* environment, video::IVideoDriver* driver, IGUIElement* parent, s32 id, core::rect<s32> rectangle);
    ~UV_Editor_Panel();

    virtual void Initialize(scene::ISceneManager* smgr, geometry_scene* geo_scene);
    virtual void SetMeshNodesVisible() {}

    virtual scene::ICameraSceneNode* getCamera();
    virtual void setAxis(int);
    virtual void render();
    virtual bool OnEvent(const SEvent& event);
    virtual void drawGrid(video::IVideoDriver* driver, const video::SMaterial material);

    
    virtual void resize(core::dimension2d<u32> new_size);

    void setGridSpacing(int spacing);
    void showTexture(bool);
    void showGrid(bool);
    void setGroupSelect(bool);

    void make_custom_surface_group();

protected:

    void make_face(polyfold* pf, int f_no, video::ITexture* face_texture);
    
    virtual void left_click(core::vector2di);
    virtual void right_click(core::vector2di);
    virtual void OnMenuItemSelected(IGUIContextMenu* menu);

    vector<int> original_brushes;
    vector<int> surface_groups;

    vector<vector<int>> brush_vertices_index;
    vector<vector<int>> brush_faces_index;
    vector<vector<int>> faces_index;
    
    TextureImage* my_image = NULL;

    geometry_scene* real_g_scene = NULL;

    vector<int> faces;

    vector2df UL_texcoord;
    vector2df BR_texcoord;

    bool first_texcoord;

    int faces_shown = 0;
    bool show_texture;
    bool show_grid;
    bool group_select = false;
};

class UV_Editor_Base;
class polyfold;

struct uv_editor_struct
{
    int nFaces;
    bool show_uv_view;
    bool show_texture;
    bool show_grid;
    bool group_select;
    int snap_dist;

    REFLECT()
};

class UV_Editor_Widget : public gui::IGUIElement
{

public:
    UV_Editor_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene*, UV_Editor_Base*, s32 id, core::rect<s32> rect);
    ~UV_Editor_Widget();

    void show();
    void onRefresh();

    void click_OK();
    virtual bool OnEvent(const SEvent& event);

    int OK_BUTTON_ID = 0;
    int my_ID;

    Reflected_Widget_EditArea* my_widget = NULL;
    Reflected_GUI_Edit_Form* options_form = NULL;

    Flat_Button* my_button = NULL;
    gui::IGUIImage* my_image = NULL;
    gui::IGUIStaticText* my_text = NULL;

    geometry_scene* g_scene = NULL;
    UV_Editor_Base* my_base = NULL;
    gui::IGUIElement* edit_panel = NULL;

};

class UV_Editor_Base : public simple_reflected_tool_base
{
public:

    ~UV_Editor_Base() {
        if (uv_edit)
            delete uv_edit;
    }

    virtual void show();
    void select(int sel);

    virtual void initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel* panel_) {}
    void initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel* panel_, scene::ISceneManager* smgr);

    void setCameraQuad(CameraQuad* cameraQuad_) { cameraQuad = cameraQuad_; }
    void close_uv_panel();
    void refresh_panel_view();

    virtual void* getObj() {
        return &m_struct;
    }

    virtual void init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos);
    virtual void write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor);

    std::string GetSelectedString();
    video::ITexture* GetSelectedTexture();

private:

    void write_uvs();

    int selection = -1;

    UV_Editor_Panel* uv_edit = NULL;
    CameraQuad* cameraQuad = NULL;

    uv_editor_struct m_struct{ 0,true,true,true,true,16 };

    friend class UV_Editor_Widget;
};

class UV_Editor_Tool
{
    static UV_Editor_Base* base;
    static multi_tool_panel* panel;

public:

    static void show()
    {
        panel->add_tool(base);
    }

    static void initialize(UV_Editor_Base* base_, multi_tool_panel* panel_)
    {
        base = base_;
        panel = panel_;
    }
};

#endif
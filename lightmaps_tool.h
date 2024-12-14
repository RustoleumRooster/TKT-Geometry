#ifndef _LIGHTMAP_VIEWER_TOOL_H
#define _LIGHTMAP_VIEWER_TOOL_H

#include <string>
#include "GUI_tools.h"
#include "CGUIWindow.h"
#include "CameraPanel.h"
#include "edit_classes.h"

class LM_Viewer_Window;

struct material_group_struct
{
    //ATTRIBUTES:
    struct attributes
    {
        bool selected;
    };

    //REFLECTED MEMBERS:
    int id;
    int nFaces;
    int nTriangles;
    int material_group;
    video::ITexture* texture;

    //NON REFLECTED MEMBERS:
    bool selected;

    REFLECT_CUSTOM_STRUCT()
};

struct material_buffers_struct
{
    int nBuffers;
    std::vector<material_group_struct> material_groups;
    REFLECT()
};

class LM_Viewer_Panel : public TestPanel_2D
{
public:
    LM_Viewer_Panel(IGUIEnvironment* environment, video::IVideoDriver* driver, IGUIElement* parent, LM_Viewer_Window* win, s32 id, core::rect<s32> rectangle);
    ~LM_Viewer_Panel();

    virtual scene::ICameraSceneNode* getCamera();
    virtual void Initialize(scene::ISceneManager* smgr, geometry_scene* geo_scene);
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

    //TextureMaterial& current_material;

    bool bRenderLightmap = true;
    
    TextureImage* my_image = NULL;

    LM_Viewer_Window* my_window = NULL;
    //LM_Viewer_Base* my_base = NULL;
    GeometryStack* uv_scene = NULL;
    vector<int> faces;
    //CameraQuad* my_camera_quad = NULL;
};

class polyfold;

//============================================================
//
//

class Material_Buffers_Base;

class Material_Buffers_Widget : public gui::IGUIElement
{

public:
    Material_Buffers_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene*, Material_Buffers_Base*, s32 id, core::rect<s32> rect);
    ~Material_Buffers_Widget();

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
    Material_Buffers_Base* my_base = NULL;
    gui::IGUIElement* edit_panel = NULL;

};

struct mb_tool_options_struct
{
    bool show_uv_view;
    bool show_triangles;
    bool show_lightmap;

    REFLECT()
};

class Material_Buffers_Base : public simple_reflected_tool_base
{
public:

    ~Material_Buffers_Base() {
        if (uv_edit)
            delete uv_edit;
    }

    virtual void show();
    void select(int sel);

    virtual void initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel* panel_)
    {}

    void initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel* panel_, scene::ISceneManager* smgr);

    void setCameraQuad(CameraQuad* cameraQuad_) { cameraQuad = cameraQuad_; }
    void close_uv_panel();
    void refresh_panel_view();

    virtual void* getObj() {
        return &m_struct;
    }

    virtual void* getOptions() {
        return &mb_options;
    }

    virtual void init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos);
    virtual void write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor);

    std::string GetSelectedString();
    video::ITexture* GetSelectedTexture();

private:

    int selection = -1;

    LM_Viewer_Panel* uv_edit = NULL;
    CameraQuad* cameraQuad = NULL;

    mb_tool_options_struct mb_options;

    material_buffers_struct m_struct;
};

class Material_Buffers_Tool
{
    static Material_Buffers_Base* base;
    static multi_tool_panel* panel;

public:

    static void show()
    {
        panel->add_tool(base);
    }

    static void initialize(Material_Buffers_Base* base_, multi_tool_panel* panel_)
    {
        base = base_;
        panel = panel_;
    }
};

#endif
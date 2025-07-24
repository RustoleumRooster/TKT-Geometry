#ifndef _LIGHTMAP_VIEWER_TOOL_H
#define _LIGHTMAP_VIEWER_TOOL_H

#include <string>
#include "GUI_tools.h"
#include "CGUIWindow.h"
#include "CameraPanel.h"
#include "edit_classes.h"
#include "reflect_custom_types.h"

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

class GeometryStack;

class LM_Viewer_Panel : public TestPanel_2D
{
public:
    LM_Viewer_Panel(gui::IGUIEnvironment* environment, video::IVideoDriver* driver, gui::IGUIElement* parent, LM_Viewer_Window* win, s32 id, core::rect<s32> rectangle);
    ~LM_Viewer_Panel();

    virtual scene::ICameraSceneNode* getCamera();
    virtual void Initialize(geometry_scene* geo_scene);
    virtual void setAxis(int);
    virtual void render();
    virtual bool OnEvent(const SEvent& event);
    //virtual void left_click(core::vector2di pos);
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

    virtual void OnMenuItemSelected(gui::IGUIContextMenu* menu);

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
    std::vector<int> faces;
    //CameraQuad* my_camera_quad = NULL;
};

class polyfold;

//============================================================
//
//

class Lightmap_Resize_Base;
class Flat_Button;

class Lightmap_Resize_Widget : public gui::IGUIElement
{

public:
    Lightmap_Resize_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene*, Lightmap_Resize_Base*, s32 id, core::rect<s32> rect);
    ~Lightmap_Resize_Widget();

    void show();
    void onRefresh();

    void click_OK();
    virtual bool OnEvent(const SEvent& event);

private:

    int OK_BUTTON_ID = 0;
    int my_ID;

    Reflected_Widget_EditArea* my_widget = NULL;
    Flat_Button* my_button = NULL;

    geometry_scene* g_scene = NULL;
    Lightmap_Resize_Base* my_base = NULL;
    gui::IGUIElement* edit_panel = NULL;
};

class Lightmap_Resize_Base : public simple_reflected_tool_base
{
public:
    Lightmap_Resize_Base(std::wstring name, int my_id, gui::IGUIEnvironment* env, multi_tool_panel* panel);

    ~Lightmap_Resize_Base();

    virtual void show() override;
    virtual void widget_closing(Reflected_Widget_EditArea*) override;
    virtual reflect::TypeDescriptor_Struct* getTypeDescriptor() override;
    //virtual void read_obj(void* obj) override;
    virtual void post_edit() override;
    virtual void write_obj_by_field(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos, void* obj) override;
    virtual void save_expanded_status(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos) override;
    virtual void init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos) override;
    virtual void write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor) {}
    virtual void* getObj() override;

    s32 getWidgetHeight();

    void initialize();

    void ApplyResize();
};


class mini_header : public gui::IGUIElement
{
public:
    mini_header(gui::IGUIEnvironment* env, gui::IGUIElement* parent, s32 id, core::rect<s32> rect);

    virtual void draw();
    virtual void setText(std::wstring txt);

    gui::IGUIStaticText* my_text = NULL;
};


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

    //u32 get_total_height() { return total_height; }

    void click_OK();
    virtual bool OnEvent(const SEvent& event);

    int OK_BUTTON_ID = 0;
    int my_ID;

    Reflected_Widget_EditArea* my_widget = NULL;
    Reflected_GUI_Edit_Form* options_form = NULL;
    Reflected_GUI_Edit_Form* dimensions_form = NULL;

    mini_header* header = NULL;
    Flat_Button* my_button = NULL;
    gui::IGUIImage* my_image = NULL;
    gui::IGUIStaticText* my_text = NULL;

    geometry_scene* g_scene = NULL;
    Material_Buffers_Base* my_base = NULL;
    gui::IGUIElement* edit_panel = NULL;

    //u32 total_height = 0;

    Lightmap_Resize_Widget* sub_widget = NULL; //user events will be sent to the next widget, for proper placement
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
    Material_Buffers_Base(std::wstring name, int my_id, gui::IGUIEnvironment* env, multi_tool_panel* panel) :
        simple_reflected_tool_base(name, my_id, env, panel) , mb_options{ true,true,true } {}

    ~Material_Buffers_Base();

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
        return &mb_options;
    }

    virtual void init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos);
    virtual void write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor);

    std::string GetSelectedString();
    video::ITexture* GetSelectedTexture();

    s32 getWidgetHeight();
    void setWidgetHeight(s32);

private:

    int selection = -1;

    LM_Viewer_Panel* uv_edit = NULL;
    CameraQuad* cameraQuad = NULL;

    mb_tool_options_struct mb_options;

    material_buffers_struct m_struct;

    s32 widget_total_height = 0;

};

class Material_Buffers_Tool
{
    static Material_Buffers_Base* base;
    static Lightmap_Resize_Base* base2;

    static multi_tool_panel* panel;

public:

    static void show()
    {
        panel->add_tool(base);
    }

    static Material_Buffers_Base* get_base()
    {
        return base;
    }

    static Lightmap_Resize_Base* get_base2()
    {
        return base2;
    }

    static void initialize(Material_Buffers_Base* base_, Lightmap_Resize_Base* base2_, multi_tool_panel* panel_)
    {
        base = base_;
        base2 = base2_;
        panel = panel_;
    }
};

#endif
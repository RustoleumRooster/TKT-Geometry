#include <irrlicht.h>
#include "GUI_tools.h"
#include "Reflection.h"
#include "edit_classes.h"
#include "reflect_custom_types.h"
#include "lightmaps_tool.h"

class Geo_Settings_Base;
class geometry_scene;
class Flat_Button;

struct drag_snap_sizes_struct
{
    int value;
    REFLECT_MULTI_CHOICE()
};

struct rotate_snap_sizes_struct
{
    int value;
    REFLECT_MULTI_CHOICE()
};

struct editor_settings_struct
{
    drag_snap_sizes_struct snap_dist;
    rotate_snap_sizes_struct rotate_snap;
    bool bAutoSave;
    REFLECT()
};

struct build_settings_struct
{
    bool bProgressiveBuild;
    bool bBuildFinal;
    bool bAutoRebuildMeshes;
   // bool bOptimizeTriangles;
    REFLECT()
};

struct geo_settings_struct
{
    editor_settings_struct editor_settings;
    build_settings_struct build_settings;
    std::vector<int> IntArray;
    std::vector<f32> FloatArray;

    REFLECT()
};

namespace reflect {
    
}

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


class Geo_Settings_Widget : public gui::IGUIElement
{

public:
    Geo_Settings_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene*, Geo_Settings_Base*, s32 id, core::rect<s32> rect);
    ~Geo_Settings_Widget();

    void show();
    void onRefresh();
   
    void click_OK();
    virtual bool OnEvent(const SEvent& event);

    int OK_BUTTON_ID = 0;
    int my_ID;

    Reflected_Widget_EditArea* my_widget = NULL;

    Flat_Button* my_button = NULL;
    
    geometry_scene* g_scene = NULL;
    Geo_Settings_Base* my_base = NULL;
    gui::IGUIElement* edit_panel = NULL;
};


class Geo_Settings_Base : public simple_reflected_tool_base
{
public:

    virtual void show();

    virtual void initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel* panel_)
    {
        tool_base::initialize(name_, my_id, env_, g_scene_, panel_);
        m_settings.editor_settings.rotate_snap.value = 3;
        m_settings.editor_settings.snap_dist.value = 3;
        m_settings.build_settings.bAutoRebuildMeshes = true;
        m_settings.build_settings.bBuildFinal = true;
       // m_settings.build_settings.bOptimizeTriangles = true;
        for (int i = 0; i < 10; i++)
        {
            m_settings.IntArray.push_back(0);
            m_settings.FloatArray.push_back(1.0);
        }

        m_typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<geo_settings_struct>::get();
    }

    virtual void* getObj() {
        return &m_settings;
    }

    

private:

    geo_settings_struct m_settings;

};


class Geo_Settings_Tool
{
    static Geo_Settings_Base* base;
    static multi_tool_panel* panel;

public:

    static void show()
    {
        panel->add_tool(base);
    }

    static void initialize(Geo_Settings_Base* base_, multi_tool_panel* panel_)
    {
        base = base_;
        panel = panel_;
    }
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
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


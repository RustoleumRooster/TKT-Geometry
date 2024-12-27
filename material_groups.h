#ifndef _MATERIAL_GROUPS_H_
#define _MATERIAL_GROUPS_H_

#include <irrlicht.h>
#include <iostream>
#include <vector>
#include "Reflection.h"
#include "GUI_tools.h"

using namespace irr;

struct Material_Group
{
    std::string name;
    bool two_sided;
    bool transparent;
    bool lightmap;
    bool isSky;
    video::SColor color;
};

class geometry_scene;
class Material_Groups_Base;

class Material_Groups_Widget : public gui::IGUIElement
{

     struct nSelected_struct
    {
        int nSelected;
        REFLECT()
    };


public:
    Material_Groups_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene* scene,Material_Groups_Base*,s32 id,core::rect<s32> rect);
    ~Material_Groups_Widget();

    void show();
    void refresh();

    void click_OK();
    virtual bool OnEvent(const SEvent& event);

    int VISUALIZE_BUTTON_ID=0;
    int LISTBOX_ID=0;
    int COMBOBOX_ID = 0;
    int my_ID;

    //Reflected_GUI_Edit_Form* form=NULL;
    Reflected_GUI_Edit_Form* form2=NULL;
    geometry_scene* g_scene = NULL;
    Material_Groups_Base* base = NULL;
    gui::IGUIListBox* m_listbox = NULL;
    gui::IGUIComboBox* m_combobox_lmres = NULL;
    bool bWrite=true;
    bool dont_sg_select=false;

    //static mGroups_struct mg_struct;
    static nSelected_struct sel_struct;
};

enum {
    LIGHTING_UNLIT = 0,
    LIGHTING_LIGHTMAP,
    LIGHTING_LIGHT_ONLY
};

class Material_Groups_Base : public tool_base
{
public:
    Material_Groups_Base(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, multi_tool_panel*);
    ~Material_Groups_Base();

    void show();

    /*
    void preinitialize(gui::IGUIEnvironment* env_, geometry_scene* g_scene_)
    {
        env=env_;
        g_scene= g_scene_;
    }
    */

    void apply_material_to_buffer(scene::IMeshBuffer*, int material_no, int lighting, bool selected, bool final_view);

    int getSelected() {
        return selected;
    }

    Material_Group getMaterialGroup(int i) {
        return material_groups[i];
    }

    std::vector<Material_Group> material_groups;

    video::E_MATERIAL_TYPE LightingMaterial_Type = video::EMT_SOLID;
    video::E_MATERIAL_TYPE LightingMaterial_Selected_Type = video::EMT_SOLID;
    video::E_MATERIAL_TYPE LightingOnlyMaterial_Type = video::EMT_SOLID;
    video::E_MATERIAL_TYPE LightingOnlyMaterial_Selected_Type = video::EMT_SOLID;
    video::E_MATERIAL_TYPE SolidMaterial_Type = video::EMT_SOLID;
    video::E_MATERIAL_TYPE SolidMaterial_Selected_Type = video::EMT_SOLID;
    video::E_MATERIAL_TYPE SolidMaterial_WaterSurface_Type = video::EMT_SOLID;
    video::E_MATERIAL_TYPE Material_Projection_Type = video::EMT_SOLID;

    friend int main();

    int lighting_mode = LIGHTING_UNLIT;
    int selected = -1;
};

class Material_Groups_Tool
{
    static Material_Groups_Base* base;
    static gui::IGUIEnvironment* env;
    static multi_tool_panel* panel;

public:

    static void show()
    {
        panel->add_tool(base);
    }

    static void initialize(gui::IGUIEnvironment* env_, Material_Groups_Base* base_ , multi_tool_panel* panel_)
    {
        base = base_;
        env=env_;
        panel = panel_;
    }

    static Material_Groups_Base* get_base()
    {
        return base;
    }
};




#endif

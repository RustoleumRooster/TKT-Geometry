#ifndef _MATERIAL_GROUPS_H_
#define _MATERIAL_GROUPS_H_

#include <irrlicht.h>
#include <iostream>
#include <vector>
//#include "CGUIWindow.h"
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
    Material_Groups_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent,geometry_scene*, Material_Groups_Base*,s32 id,core::rect<s32> rect);
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

class Material_Groups_Base : public tool_base
{
public:

    ~Material_Groups_Base();

    void show();
    void preinitialize(gui::IGUIEnvironment* env_, geometry_scene* g_scene_)
    {
        env=env_;
        g_scene= g_scene_;
    }

    void initialize(std::wstring name,int my_id, multi_tool_panel* panel)
    {
        tool_base::initialize(name,my_id,env,g_scene,panel);
        refreshTextures();
    }

    void apply_material_to_buffer(scene::IMeshBuffer*, int material_no, int lighting, bool selected);

    int getSelected() {
        return selected;
    }

    void refreshTextures();

    Material_Group getMaterialGroup(int i) {
        return material_groups[i];
    }

    std::vector<Material_Group> material_groups;

    video::E_MATERIAL_TYPE LightingMaterial_Type = video::EMT_SOLID;
    video::E_MATERIAL_TYPE LightingMaterial_Selected_Type = video::EMT_SOLID;
    video::E_MATERIAL_TYPE SolidMaterial_Type = video::EMT_SOLID;
    video::E_MATERIAL_TYPE SolidMaterial_Selected_Type = video::EMT_SOLID;

    friend int main();

    int selected = -1;
};

class Material_Groups_Tool
{
    static Material_Groups_Base* base;
    static gui::IGUIEnvironment* env;
    static geometry_scene* g_scene;
    static multi_tool_panel* panel;

public:

    static void show()
    {
        panel->add_tool(base);
    }

    static void initialize(gui::IGUIEnvironment* env_, geometry_scene* g_scene_,Material_Groups_Base* base_ , multi_tool_panel* panel_)
    {
        base = base_;
        env=env_;
        g_scene= g_scene_;
        panel = panel_;
    }

    static Material_Groups_Base* get_base()
    {
        return base;
    }
};




#endif

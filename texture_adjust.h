#ifndef _TEXTURE_ADJUST_H_
#define _TEXTURE_ADJUST_H_
#include <irrlicht.h>
#include "CGUIWindow.h"
#include "reflection.h"

using namespace irr;

class geometry_scene;

class Texture_Adjust_Window : public gui::CGUIWindow
{
    struct nSelected_struct
    {
        int nSelected;
        REFLECT()
    };

    struct ChooseUVType
    {
        int value;
        REFLECT_MULTI_CHOICE()
    };

    struct TextureAlignment
    {
        f32 snap_dist;
        //core::vector2df offset;
        f32 offset_x;
        f32 offset_y;
        //core::vector2df scale;
        f32 scale_x;
        f32 scale_y;
        f32 rotation;
        ChooseUVType style;

        REFLECT()
    };

    void click_OK();
    void click_Align();

public:
    Texture_Adjust_Window(gui::IGUIEnvironment* env, gui::IGUIElement* parent,geometry_scene* g_scene_,s32 id,core::rect<s32> rect);
    ~Texture_Adjust_Window();

    void show();
    void refresh();

    virtual bool OnEvent(const SEvent& event);

    int OK_BUTTON_ID=0;
    int ALIGN_BUTTON_ID = 0;
    int SNAP_ID=0;
    int STYLE_ID=0;

    Reflected_GUI_Edit_Form* form=NULL;
    Reflected_GUI_Edit_Form* form2=NULL;
    geometry_scene* g_scene;

    static TextureAlignment tex_struct;
    static nSelected_struct sel_struct;
    core::vector3df vDragOrigin;
    core::vector3df uv_trans_base;
    bool bDragTexture=false;
    bool dont_sg_select=false;
};

class Texture_Adjust_Tool
{

public:

    static gui::IGUIEnvironment* env;
    static geometry_scene* g_scene;

    static void show();
    static void initialize(gui::IGUIEnvironment* env_, geometry_scene* g_scene_)
    {
        env=env_;
        g_scene= g_scene_;
    }

    static void set_scene(geometry_scene* g_scene_)
    {
        g_scene = g_scene_;
    }
};



#endif

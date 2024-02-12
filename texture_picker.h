#ifndef _TEXTURE_PICKER_H_
#define _TEXTURE_PICKER_H_



#include <irrlicht.h>
#include "CGUIWindow.h"
#include <iostream>
#include "GUI_tools.h"

class geometry_scene;

using namespace irr;
using namespace gui;

struct TextureInfo
{
    video::ITexture* texture=NULL;
    core::stringw name;
    //core::stringw category;
    int category;
    int texture_number;
};

class TexturePicker_Base;

class TexturePicker : public gui::IGUIElement
{
public:

    //! constructor
    TexturePicker(IGUIEnvironment* environment,IGUIElement* parent,  video::IVideoDriver* driver, geometry_scene*, TexturePicker_Base* base, s32 id, core::rect<s32> rectangle);
    ~TexturePicker();

    virtual void draw();
    virtual bool OnEvent(const SEvent& event);

   // void addTexture(std::string);
    void do_layout();
    void selected(int);
    //video::ITexture* getCurrentTexture();
    void scrubList();

private:
    int getTotalLength();

    gui::IGUIScrollBar* my_scrollbar=NULL;
    std::vector<TextureInfo> texture_list;
    geometry_scene* geo_scene=NULL;

    video::IVideoDriver* driver=NULL;
    int texture_icon_size = 100;
    int layout_start=0;
    int selection = -1;

    int MY_SCROLLBAR_ID;

    s32 clickx;
    s32 clicky;
    s32 mousex;
    s32 mousey;
    bool bMouseDown=false;
    bool rMouseDown=false;

    TexturePicker_Base* my_base = NULL;

};

class Texture_Picker_Window : public gui::IGUIElement
{
public:
     Texture_Picker_Window(gui::IGUIEnvironment* env, gui::IGUIElement* parent,geometry_scene* g_scene_,TexturePicker_Base* texture_picker_base_,s32 id,core::rect<s32> rect)
     : gui::IGUIElement(gui::EGUIET_ELEMENT,env,parent,id,rect), g_scene(g_scene_), texture_picker_base(texture_picker_base_)
        {}
    ~Texture_Picker_Window();

    void show();
    virtual bool OnEvent(const SEvent& event);

private:

    int MY_COMBOBOX_ID;
    gui::IGUIComboBox* my_ComboBox=NULL;
    geometry_scene* g_scene=NULL;
    TexturePicker_Base* texture_picker_base=NULL;
    TexturePicker* texturePicker = NULL;

    friend class TexturePicker_Base;
};


class TexturePicker_Base : public tool_base
{
public:
    void initialize(std::wstring name_,int my_id,gui::IGUIEnvironment* env_, geometry_scene* g_scene_,multi_tool_panel* panel_)
    {
        tool_base::initialize(name_,my_id,env_,g_scene_,panel_);
        categories.push_back("All Textures");
    }

    void show();

    //void addTexture(std::string);
    void addTexture(std::string name,std::string category = std::string(""));
    void addTexture(video::ITexture* texture, std::string name, std::string category);

    video::ITexture* getCurrentTexture();

private:
    std::vector<TextureInfo> my_textures;
    std::vector<std::string> categories;
    int selection =-1;
    int selected_category=0;

    friend class Texture_Picker_Window;
    friend class TexturePicker;
};

class TexturePicker_Tool
{

public:
    static void initialize(TexturePicker_Base* base_, multi_tool_panel* panel_)
    {
        base = base_;
        panel = panel_;
    }
    static void show()
    {
        panel->add_tool(base);
    }

    static TexturePicker_Base* base;
    static multi_tool_panel* panel;
};


#endif

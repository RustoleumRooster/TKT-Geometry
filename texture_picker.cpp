
#include <irrlicht.h>
#include <iostream>
#include "edit_env.h"
#include "geometry_scene.h"
#include "texture_picker.h"
#include "texture_icon.h"
#include "scrollbar2.h"

using namespace irr;
using namespace gui;

TexturePicker_Base* TexturePicker_Tool::base = NULL;
multi_tool_panel* TexturePicker_Tool::panel = NULL;

extern IrrlichtDevice* device;

TexturePicker::TexturePicker(IGUIEnvironment* environment,  IGUIElement* parent, video::IVideoDriver* driver_, geometry_scene* g_scene_,TexturePicker_Base* base, s32 id, core::rect<s32> rectangle)
: IGUIElement(EGUIET_IMAGE,environment, parent, id, rectangle), driver(driver_), geo_scene(g_scene_),my_base(base)
{
	#ifdef _DEBUG
	setDebugName("TexturePicker");
	#endif

    MY_SCROLLBAR_ID = GUI_ID_TEXTURE_SCROLL_BAR;

	my_scrollbar = new CGUIScrollBar2(false,environment,this,GUI_ID_TEXTURE_SCROLL_BAR,core::rect<s32>(0,0,16,this->AbsoluteRect.getHeight()));
	my_scrollbar->drop();

    my_scrollbar->setMax(getTotalLength());
}

TexturePicker::~TexturePicker()
{
    //std::cout << "Out of scope (Texture Picker)\n";
}

int TexturePicker::getTotalLength()
{
    return ((texture_list.size()+1)/2)*(32+texture_icon_size);;
}

void TexturePicker::do_layout()
{
    int start_i =0;
    int end_i = texture_list.size();
    int scroll_pos = my_scrollbar->getPos();
    int total_length = getTotalLength();
    int view_length = RelativeRect.getHeight();
    int move_length=0;

    if(total_length > view_length)
    {
        move_length = total_length - view_length;
    }
    int start_pos = scroll_pos;

    core::list<gui::IGUIElement*> child_list = this->getChildren();

    core::list<gui::IGUIElement*>::Iterator it = child_list.begin();
    for(;it != child_list.end();++it)
    {
        int id = (*it)->getID();
        if( id != GUI_ID_TEXTURE_SCROLL_BAR )
            this->removeChild(*it);
    }

    int c=0;
    int j;
    int border =8;
    for(int i=start_i, j=0; i<end_i;i++,j++)
    {
        int xpos = 16+ 26 + (j%2) * (16+texture_icon_size);
        int ypos = 8 +(j/2) * (32+texture_icon_size);

        if(ypos + 32  + texture_icon_size > start_pos && ypos < start_pos + view_length)
        {
            {
                TextureIcon* img = new TextureIcon(Environment,this,-1, core::rect<s32>(xpos-border,ypos-start_pos-border,xpos+texture_icon_size+border,ypos-start_pos+texture_icon_size+16+border));
                img->setImage(texture_list[i].texture);
                img->setText(texture_list[i].name.c_str());
                img->setTextureID(i);

                if(i==selection)
                    img->setSelected(true);
            }

            c++;
        }
    }
}

void TexturePicker::selected(int sel)
{
    if(sel != -1)
    {
        selection=sel;
        my_base->selection=texture_list[sel].texture_number;

        do_layout();
        if(this->geo_scene && this->geo_scene->getSelectedFaces().size() > 0)
        {
            SEvent event;
            event.EventType = EET_USER_EVENT;
            event.UserEvent.UserData1=USER_EVENT_TEXTURE_SELECTED;
            device->getEventReceiver()->OnEvent(event);
        }
    }
}

bool TexturePicker::OnEvent(const SEvent& event)
{
	if (isEnabled())
	{
		switch(event.EventType)
		{
		    case EET_GUI_EVENT:
            {
                s32 id = event.GUIEvent.Caller->getID();

                switch(event.GUIEvent.EventType)
                {
                    case EGET_SCROLL_BAR_CHANGED:
                    {
                        if(id == GUI_ID_TEXTURE_SCROLL_BAR)
                        {
                            this->do_layout();
                            return true;
                        }
                    }
                }
                break;
            }
            case EET_MOUSE_INPUT_EVENT:
            {
                switch(event.MouseInput.Event)
                {
                    case EMIE_RMOUSE_PRESSED_DOWN:
                        {
                            //if (Environment->hasFocus(this))
                            if(!AbsoluteClippingRect.isPointInside( core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y ) ))
                            {
                                Environment->removeFocus(this);
                                return false;
                            }
                        }
                }
                break;
            }
		        /*
            case EET_MOUSE_INPUT_EVENT:
            {
                switch(event.MouseInput.Event)
                {
                    case EMIE_LMOUSE_LEFT_UP:
                        {
                            if(!AbsoluteClippingRect.isPointInside( core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y ) ))
                            {
                                Environment->removeFocus(this);
                            }
                            bMouseDown = false;
                            std::cout<<"lmouse up\n";
                        }
                        return true;
                    case EMIE_LMOUSE_PRESSED_DOWN:
                        {

                            std::cout<<"lmouse down\n";;
                            bMouseDown = true;
                        }
                        return true;
                }
                break;
            }*/
		}
	}
    return false;
}

void TexturePicker::scrubList()
{
    texture_list.clear();

    int sel_cat = my_base->selected_category;

    if( sel_cat == 0)
        texture_list = my_base->my_textures;
    else
    {
        for(TextureInfo texinfo : my_base->my_textures)
        {
            if(texinfo.category == sel_cat)
                texture_list.push_back(texinfo);
        }
    }


    f32 lr = (f32)(RelativeRect.getHeight()) / (f32)getTotalLength();
    if(lr < 1.0)
    {
        my_scrollbar->setVisible(true);
        my_scrollbar->setEnabled(true);

        my_scrollbar->setMax(getTotalLength());
        my_scrollbar->setPos(0);
    }
    else
    {
        my_scrollbar->setPos(0);
        my_scrollbar->setEnabled(false);
        my_scrollbar->setVisible(false);
    }
}

void TexturePicker::draw()
{
    if (!IsVisible)
		return;

	IGUISkin* skin = Environment->getSkin();
	video::IVideoDriver* driver = Environment->getVideoDriver();

    skin->draw2DRectangle(this, video::SColor(255,0,0,0),
                         AbsoluteRect,
                         &AbsoluteClippingRect);

    core::rect<s32> r = this->AbsoluteRect;
    r.LowerRightCorner.X = r.UpperLeftCorner.X+17;
    r.UpperLeftCorner.X = r.LowerRightCorner.X-1;


    skin->draw2DRectangle(this, video::SColor(255,24,24,24),
                         r,
                         &AbsoluteClippingRect);


    IGUIElement::draw();
}

void Texture_Picker_Window::show()
{
    core::rect<s32> client_rect(core::position2d<s32>(0,26),
                                core::dimension2du(AbsoluteRect.getWidth(),
                                                    AbsoluteRect.getHeight()-26));

    texturePicker = new TexturePicker(Environment, this, device->getVideoDriver(),g_scene,texture_picker_base, GUI_ID_TEXTURE_PICKER,
                                                      client_rect);
    MY_COMBOBOX_ID = GUI_ID_TEXTURE_PICKER+2;

    core::rect<s32> combo_rect(core::position2d<s32>(0,0),
                                core::dimension2du(AbsoluteRect.getWidth(),
                                                    24));

    my_ComboBox = Environment->addComboBox(combo_rect,this,MY_COMBOBOX_ID);

    for(std::string cat : texture_picker_base->categories)
    {
        std::wstring str(cat.begin(),cat.end());
        my_ComboBox->addItem(str.c_str());
    }

    texturePicker->scrubList();
    texturePicker->do_layout();
}

bool Texture_Picker_Window::OnEvent(const SEvent& event)
{
	if (isEnabled())
	{
		switch(event.EventType)
		{
		    case EET_GUI_EVENT:
		        {
                    s32 id = event.GUIEvent.Caller->getID();

                    switch(event.GUIEvent.EventType)
                    {
                        case EGET_COMBO_BOX_CHANGED:
                        {
                            if(id == MY_COMBOBOX_ID)
                            {
                                gui::IGUIComboBox* box = (gui::IGUIComboBox*)event.GUIEvent.Caller;
                                texture_picker_base->selected_category = box->getSelected();
                                texturePicker->scrubList();
                                texturePicker->do_layout();
                                return true;
                            }
                        }break;
                    }
                    break;
		        }
		}
	}
	return false;
	//gui::CGUIWindow::OnEvent(event);
}

Texture_Picker_Window::~Texture_Picker_Window()
{
    if(texturePicker)
        texturePicker->drop();
}

TexturePicker_Base::TexturePicker_Base(std::wstring name, int my_id, gui::IGUIEnvironment* env, multi_tool_panel* panel)
    : tool_base(name,my_id,env,panel)
{
    categories.push_back("All Textures");
}

void TexturePicker_Base::show()
{
    core::rect<s32> client_rect(core::vector2di(0,0),
                                core::dimension2du(this->panel->getClientRect()->getAbsolutePosition().getWidth(),
                                                   this->panel->getClientRect()->getAbsolutePosition().getHeight()));

    Texture_Picker_Window* widget = new Texture_Picker_Window(env,panel->getClientRect(),g_scene,this,GUI_ID_TEXTURES_BASE,client_rect);

    widget->show();
    widget->my_ComboBox->setSelected(this->selected_category);

    widget->drop();
}
/*
TextureInfo& TexturePicker_Base::addTexture(video::ITexture* texture, std::string name, std::string category)
{
    TextureInfo texinfo;
    texinfo.texture = texture;

    std::wstring wstr(name.begin(),name.end());
    core::stringw str(wstr.c_str());
    texinfo.name = str;

    if(category != std::string(""))
        {
        bool b = false;
        for(int i=0;i<categories.size();i++)
            {
                if(categories[i] == category)
                {
                    texinfo.category = i;
                    b=true;
                }
            }
        if(!b)
            {
                categories.push_back(category);
                texinfo.category=categories.size()-1;
            }
        }
    else
        texinfo.category=0;

    texinfo.texture_number =  my_textures.size();

    my_textures.push_back(texinfo);

    return texinfo;
}
*/

TextureInfo& TexturePicker_Base::addTexture(std::string tex_name, std::string fname, std::string category)
{
    TextureInfo texinfo;
    texinfo.name = tex_name.c_str();
    texinfo.texture = device->getVideoDriver()->getTexture(fname.c_str());

    io::path path = texinfo.texture->getName();
    core::deletePathFromFilename(path);
    core::cutFilenameExtension(path,path);
    core::stringw str = path;
    //texinfo.name = str;

    if(category != std::string(""))
        {
        bool b = false;
        for(int i=0;i<categories.size();i++)
            {
                if(categories[i] == category)
                {
                    texinfo.category = i;
                    b=true;
                }
            }
        if(!b)
            {
                categories.push_back(category);
                texinfo.category=categories.size()-1;
            }
        }
    else
        texinfo.category=0;

    texinfo.texture_number =  my_textures.size();

    my_textures.push_back(texinfo);

    return texinfo;
}

TextureInfo& TexturePicker_Base::addTexture(std::string fname, std::string category)
{
    io::path path = fname.c_str();
    core::deletePathFromFilename(path);
    core::cutFilenameExtension(path, path);

    return addTexture(path.c_str(), fname, category);
}

TextureInfo* TexturePicker_Base::getCurrentTexture()
{
    if(selection !=-1)
    {
        return &my_textures[selection];
    }
    else if(my_textures.size()>0)
    {
        return &my_textures[0];
    }
    else return NULL;
}

TextureInfo& TextureInfo::set_normal_map(std::string fname)
{
    normal_map = device->getVideoDriver()->getTexture(fname.c_str());
    return *this;
}

TextureInfo& TextureInfo::set_height_map(std::string fname)
{
    height_map = device->getVideoDriver()->getTexture(fname.c_str());
    return *this;
}

TextureInfo* TexturePicker_Tool::get_texture_info(core::stringw name)
{
    for (TextureInfo& ti : base->my_textures)
    {
        if (ti.name == name)
            return &ti;
    }

    return NULL;
}

video::ITexture* TexturePicker_Tool::get_texture(core::stringw name)
{
    for (const TextureInfo& ti : base->my_textures)
    {
        if (ti.name == name)
            return ti.texture;
    }

    return NULL;
}

video::ITexture* TexturePicker_Tool::get_height_map(core::stringw name)
{
    for (const TextureInfo& ti : base->my_textures)
    {
        if (ti.name == name)
            return ti.height_map;
    }

    return NULL;
}

video::ITexture* TexturePicker_Tool::get_normal_map(core::stringw name)
{
    for (const TextureInfo& ti : base->my_textures)
    {
        if (ti.name == name)
            return ti.normal_map;
    }

    return NULL;
}

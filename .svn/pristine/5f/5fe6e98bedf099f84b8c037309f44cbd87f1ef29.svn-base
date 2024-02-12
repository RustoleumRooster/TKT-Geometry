
#include <irrlicht.h>
#include <iostream>
#include "edit_env.h"
#include "texture_icon.h"
#include "texture_picker.h"


using namespace irr;
using namespace gui;


//! constructor
TextureIcon::TextureIcon(IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle)
: IGUIElement(EGUIET_IMAGE, environment, parent, id, rectangle), Texture(0), Color(255,255,255,255)
{
	#ifdef _DEBUG
	setDebugName("TextureIcon");
	#endif
}


//! destructor
TextureIcon::~TextureIcon()
{
	if (Texture)
		Texture->drop();
}

void TextureIcon::setText(std::wstring txt)
{
    //std::wstring s(txt.begin(),txt.end());
    gui::IGUIStaticText* stext = Environment->addStaticText(txt.c_str(),core::rect<s32>(core::vector2di(0,draw_size + 2),core::vector2di(draw_size,draw_size+24)),false,false,this,-1);
    stext->setOverrideColor(video::SColor(255,255,255,255));
}


//! sets an image
void TextureIcon::setImage(video::ITexture* image)
{
	if (image == Texture)
		return;

	if (Texture)
		Texture->drop();

	Texture = image;

	if (Texture)
		Texture->grab();
}

//! Gets the image texture
video::ITexture* TextureIcon::getImage() const
{
	return Texture;
}

//! sets the color of the image
void TextureIcon::setColor(video::SColor color)
{
	Color = color;
}

//! Gets the color of the image
video::SColor TextureIcon::getColor() const
{
	return Color;
}

void TextureIcon::setTextureID(int new_id)
{
    texture_id = new_id;
}


bool TextureIcon::OnEvent(const SEvent& event)
{
	if (isEnabled())
	{
		switch(event.EventType)
		{
            case EET_MOUSE_INPUT_EVENT:
                {
                    switch(event.MouseInput.Event)
                    {
                        case EMIE_LMOUSE_LEFT_UP:
                            {
                               // if(!AbsoluteClippingRect.isPointInside( core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y ) ))
                               // {
                               //     Environment->removeFocus(this);
                               // }
                               // bMouseDown = false;
                                //std::cout<<"lmouse up\n";
                            }
                            return true;
                            break;
                        case EMIE_LMOUSE_PRESSED_DOWN:
                            {
                                ((TexturePicker*)this->getParent())->selected(texture_id);
                                //std::cout<<"lmouse down\n";;
                               // bMouseDown = true;
                            }
                            return true;
                            break;
                        case EMIE_RMOUSE_PRESSED_DOWN:
                            {
                                if(!AbsoluteClippingRect.isPointInside( core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y ) ))
                                {
                                    Environment->removeFocus(this);
                                    return false;
                                }
                            }
                            break;
                    }
                }
                break;
        }
	}
    return false;
}

void TextureIcon:: setSelected(bool sel)
{
    selected = sel;
}

//! draws the element and its children
void TextureIcon::draw()
{
	if (!IsVisible)
		return;

	IGUISkin* skin = Environment->getSkin();
	video::IVideoDriver* driver = Environment->getVideoDriver();

	if(selected)
	{
		skin->draw2DRectangle(this, skin->getColor(EGDC_3D_DARK_SHADOW),
                         core::rect<s32>(AbsoluteRect.UpperLeftCorner,AbsoluteRect.UpperLeftCorner+core::vector2di(draw_size,draw_size)+core::vector2di(border,border)),
                         &AbsoluteClippingRect);
	}

	if (Texture)
	{
        const video::SColor Colors[] = {Color,Color,Color,Color};

        //core::rect<s32> rr;
        //rr.
        driver->draw2DImage(Texture, core::rect<s32>(AbsoluteRect.UpperLeftCorner+core::vector2di(border,border),AbsoluteRect.UpperLeftCorner+core::vector2di(draw_size,draw_size)),
            core::rect<s32>(core::position2d<s32>(0,0), core::dimension2di(Texture->getOriginalSize())),
            &AbsoluteClippingRect, Colors, false);
	}
	else
	{
	//	skin->draw2DRectangle(this, skin->getColor(EGDC_3D_DARK_SHADOW), AbsoluteRect, &AbsoluteClippingRect);
	}

	IGUIElement::draw();
}



#include <irrlicht.h>
#include "ex_gui_elements.h"
#include <iostream>

using namespace irr;
using namespace gui;

void MySkin::draw3DButtonPaneStandard(IGUIElement* element,
					const core::rect<s32>& r,
					const core::rect<s32>* clip)
{
/*
	core::rect<s32> rect = r;

	driver->draw2DRectangle(getColor(EGDC_3D_HIGH_LIGHT), rect, clip);

	rect.UpperLeftCorner.X += 1;
	rect.UpperLeftCorner.Y += 1;
	rect.LowerRightCorner.X -= 1;
	rect.LowerRightCorner.Y -= 1;

	driver->draw2DRectangle(getColor(EGDC_3D_SHADOW), rect, clip);
	*/
	draw3DSunkenPane(element,getColor(EGDC_EDITABLE),true,true,r,clip);
}


void MySkin::draw3DButtonPanePressed(IGUIElement* element,
					const core::rect<s32>& r,
					const core::rect<s32>* clip)
{/*
	core::rect<s32> rect = r;

	driver->draw2DRectangle(getColor(EGDC_3D_HIGH_LIGHT), rect, clip);

	rect.UpperLeftCorner.X += 1;
	rect.UpperLeftCorner.Y += 1;
	rect.LowerRightCorner.X -= 1;
	rect.LowerRightCorner.Y -= 1;

	driver->draw2DRectangle(getColor(EGDC_3D_DARK_SHADOW), rect, clip);
	*/
	draw3DSunkenPane(element,getColor(EGDC_3D_DARK_SHADOW),true,true,r,clip);
}

void MySkin::draw3DSunkenPane(IGUIElement* element, video::SColor bgcolor,
				bool flat, bool fillBackGround,
				const core::rect<s32>& r,
				const core::rect<s32>* clip)
{
    core::rect<s32> rect = r;

    if (fillBackGround)
		driver->draw2DRectangle(bgcolor, rect, clip);

    rect.LowerRightCorner.Y = rect.UpperLeftCorner.Y + 1;
		driver->draw2DRectangle(getColor(EGDC_3D_LIGHT), rect, clip);	// top

		++rect.UpperLeftCorner.Y;
		rect.LowerRightCorner.Y = r.LowerRightCorner.Y;
		rect.LowerRightCorner.X = rect.UpperLeftCorner.X + 1;
		driver->draw2DRectangle(getColor(EGDC_3D_LIGHT), rect, clip);	// left

		rect = r;
		++rect.UpperLeftCorner.Y;
		rect.UpperLeftCorner.X = rect.LowerRightCorner.X - 1;
		driver->draw2DRectangle(getColor(EGDC_3D_LIGHT), rect, clip);	// right

		rect = r;
		++rect.UpperLeftCorner.X;
		rect.UpperLeftCorner.Y = r.LowerRightCorner.Y - 1;
		--rect.LowerRightCorner.X;
		driver->draw2DRectangle(getColor(EGDC_3D_LIGHT), rect, clip);	// bottom

    /*
    core::rect<s32> rect = r;

	driver->draw2DRectangle(getColor(EGDC_3D_HIGH_LIGHT), rect, clip);

	rect.UpperLeftCorner.X += 1;
	rect.UpperLeftCorner.Y += 1;
	rect.LowerRightCorner.X -= 1;
	rect.LowerRightCorner.Y -= 1;

	driver->draw2DRectangle(bgcolor, rect, clip);*/
   // CGUISkin::draw3DSunkenPane(element,bgcolor,true,fillBackGround,r,clip);
}




Flat_Button::Flat_Button(gui::IGUIEnvironment* env, gui::IGUIElement* parent,s32 id,core::rect<s32> rect)
    : gui::IGUIElement(gui::EGUIET_BUTTON, env,parent,id,rect)
{

}


void Flat_Button::setSpriteBank(IGUISpriteBank* sprites)
{
	if (sprites)
		sprites->grab();

	if (SpriteBank)
		SpriteBank->drop();

	SpriteBank = sprites;
}


void Flat_Button::setSprite(EGUI_BUTTON_STATE state, s32 index, video::SColor color, bool loop)
{
	if (SpriteBank)
	{
		ButtonSprites[(u32)state].Index	= index;
		ButtonSprites[(u32)state].Color	= color;
		ButtonSprites[(u32)state].Loop	= loop;
	}
	else
	{
		ButtonSprites[(u32)state].Index = -1;
	}
}

bool Flat_Button::OnEvent(const SEvent& event)
{
    if (isEnabled())
	{
	    if(event.EventType == EET_MOUSE_INPUT_EVENT)
        {
            if(event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN)
            {
                if (Environment->hasFocus(this) &&
				!AbsoluteClippingRect.isPointInside(core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y)))
                {
					Environment->removeFocus(this);
					return false;
                }

                pressed = true;

                Environment->setFocus(this);
                return true;
            }
            else if(event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP)
            {
                if ( !AbsoluteClippingRect.isPointInside( core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y ) ) )
                {
                    pressed = false;
                    return true;
                }
                pressed = false;

                SEvent e;
                e.EventType = EET_GUI_EVENT;
                e.GUIEvent.EventType = gui::EGET_BUTTON_CLICKED;
                e.GUIEvent.Caller = this;
                Parent->OnEvent(e);

				Environment->removeFocus(this);

                return true;
            }

			return false;
		}
        else if(event.EventType == EET_GUI_EVENT)
        {
            gui::IGUISkin* skin = Environment->getSkin();
            if(event.GUIEvent.EventType == gui::EGET_ELEMENT_HOVERED)
            {
                hovered = true;
                //if(my_text)
                //    my_text->setOverrideColor(skin->getColor(gui::EGDC_HIGH_LIGHT_TEXT));
                return true;
            }
            else if(event.GUIEvent.EventType == gui::EGET_ELEMENT_LEFT)
            {
                hovered = false;
               // if(my_text)
                //    my_text->setOverrideColor(skin->getColor(gui::EGDC_BUTTON_TEXT));
                return true;
            }

        }
	}

	return gui::IGUIElement::OnEvent(event);
}

void Flat_Button::draw()
{
    /*
    if(hovered)
    {
    gui::IGUISkin* skin = Environment->getSkin();
    skin->draw2DRectangle(this,skin->getColor(gui::EGDC_3D_LIGHT),getAbsolutePosition());
    }
    gui::IGUIElement::draw();*/

    if (!IsVisible)
		return;

	IGUISkin* skin = Environment->getSkin();
	video::IVideoDriver* driver = Environment->getVideoDriver();

	// todo:	move sprite up and text down if the pressed state has a sprite
	const core::position2di spritePos = AbsoluteRect.getCenter();

	if (pressed)
	{
		//if (DrawBorder)
			//skin->draw3DButtonPanePressed(this, AbsoluteRect, &AbsoluteClippingRect);
			skin->draw3DSunkenPane(this,video::SColor(255,32,48,32),true,true,AbsoluteRect,&AbsoluteClippingRect);
	}
	else if(hovered)
	{
            //skin->draw3DSunkenPane(this,skin->getColor(EGDC_3D_LIGHT),true,true,AbsoluteRect,&AbsoluteClippingRect);
            skin->draw3DSunkenPane(this,video::SColor(255,32,48,32),true,true,AbsoluteRect,&AbsoluteClippingRect);
	}
	else
    {
        //if (DrawBorder)
			skin->draw3DButtonPaneStandard(this, AbsoluteRect, &AbsoluteClippingRect);
    }

	if (SpriteBank)
	{
		// pressed / unpressed animation
		u32 state = pressed ? (u32)EGBS_BUTTON_DOWN : (u32)EGBS_BUTTON_UP;
		if (ButtonSprites[state].Index != -1)
		{
			SpriteBank->draw2DSprite(ButtonSprites[state].Index, spritePos,
			 	&AbsoluteClippingRect, ButtonSprites[state].Color, 0, 0,
				ButtonSprites[state].Loop, true);
		}

		// focused / unfocused animation
		state = Environment->hasFocus(this) ? (u32)EGBS_BUTTON_FOCUSED : (u32)EGBS_BUTTON_NOT_FOCUSED;
		if (ButtonSprites[state].Index != -1)
		{
			SpriteBank->draw2DSprite(ButtonSprites[state].Index, spritePos,
			 	&AbsoluteClippingRect, ButtonSprites[state].Color, 0, 0,
				ButtonSprites[state].Loop, true);
		}

		// mouse over / off animation
		if (isEnabled())
		{
			state = Environment->getHovered() == this ? (u32)EGBS_BUTTON_MOUSE_OVER : (u32)EGBS_BUTTON_MOUSE_OFF;
			if (ButtonSprites[state].Index != -1)
			{
				SpriteBank->draw2DSprite(ButtonSprites[state].Index, spritePos,
				 	&AbsoluteClippingRect, ButtonSprites[state].Color, 0, 0,
					ButtonSprites[state].Loop, true);
			}
		}
	}

	if (Text.size())
	{
		IGUIFont* font = skin->getFont();//  skin->getActiveFont();

		core::rect<s32> rect = AbsoluteRect;
		if (pressed)
		{
			rect.UpperLeftCorner.X += skin->getSize(EGDS_BUTTON_PRESSED_TEXT_OFFSET_X);
			rect.UpperLeftCorner.Y += skin->getSize(EGDS_BUTTON_PRESSED_TEXT_OFFSET_Y);
		}

		if (font)
        {
            video::SColor col;
            //= skin->getColor(isEnabled() ? EGDC_BUTTON_TEXT : EGDC_GRAY_TEXT)

            if(hovered || pressed)
                col = skin->getColor(gui::EGDC_HIGH_LIGHT_TEXT);
            else if (isEnabled())
                col = skin->getColor(EGDC_BUTTON_TEXT);
            else
                col = skin->getColor(EGDC_GRAY_TEXT);

			font->draw(Text.c_str(), rect ,col ,true, true, &AbsoluteClippingRect);
        }
	}

	IGUIElement::draw();
}


// Based on Irrlicht stock toolbar
// (Copyright (C) 2002-2012 Nikolaus Gebhardt)

#include <irrlicht.h>
#include "vtoolbar.h"
#include <iostream>
//#include "CGUIButton.h"

//! constructor
VToolBar::VToolBar(IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle)
:IGUIToolBar(environment, parent, id, rectangle), ButtonY(5)
{

	// calculate position and find other menubars
	s32 y = 0;
	s32 parentheight = 100;
	s32 parentwidth;

	if (parent)
	{
		parentheight = Parent->getAbsolutePosition().getHeight();
		parentwidth = Parent->getAbsolutePosition().getWidth();

		const core::list<IGUIElement*>& children = parent->getChildren();
		core::list<IGUIElement*>::ConstIterator it = children.begin();
		for (; it != children.end(); ++it)
		{
			core::rect<s32> r = (*it)->getAbsolutePosition();
			if (r.UpperLeftCorner.X == 0 && r.UpperLeftCorner.Y <= y &&
				r.LowerRightCorner.X == parentwidth)
				y = r.LowerRightCorner.Y;
		}
	}
	y=20;

	core::rect<s32> rr;
	rr.UpperLeftCorner.X = 0;
	rr.UpperLeftCorner.Y = y;
	//s32 width = Environment->getSkin()->getSize ( EGDS_MENU_HEIGHT );
    s32 width = 32;

	rr.LowerRightCorner.X = width;
	rr.LowerRightCorner.Y = parentheight;
	setRelativePosition(rr);
}


//! called if an event happened.
bool VToolBar::OnEvent(const SEvent& event)
{
	if (isEnabled())
	{
		if (event.EventType == EET_MOUSE_INPUT_EVENT &&
			event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN)
		{
			if (AbsoluteClippingRect.isPointInside(core::position2di(event.MouseInput.X, event.MouseInput.Y)))
				return true;
		}
	}

	return IGUIElement::OnEvent(event);
}


//! draws the element and its children
void VToolBar::draw()
{
	if (!IsVisible)
		return;

	IGUISkin* skin = Environment->getSkin();
	if (!skin)
		return;

	core::rect<s32> rect = AbsoluteRect;
	core::rect<s32>* clip = &AbsoluteClippingRect;

	// draw frame
	skin->draw3DToolBar(this, rect, clip);

	IGUIElement::draw();
}


//! Updates the absolute position.
void VToolBar::updateAbsolutePosition()
{
	if (Parent)
	{
		//DesiredRect.UpperLeftCorner.X = 0;
		//DesiredRect.LowerRightCorner.X = Parent->getAbsolutePosition().getWidth();
	}

	IGUIElement::updateAbsolutePosition();
}


//! Adds a button to the tool bar
IGUIButton* VToolBar::addButton(s32 id, const wchar_t* text,const wchar_t* tooltiptext,
	video::ITexture* img, video::ITexture* pressed, bool isPushButton,
	bool useAlphaChannel)
{
	ButtonY += 3;

	core::rect<s32> rectangle(1,ButtonY,3,ButtonY+1);
	if ( img )
	{
		const core::dimension2du &size = img->getOriginalSize();
		rectangle.LowerRightCorner.X = rectangle.UpperLeftCorner.X + size.Width + 8;
		rectangle.LowerRightCorner.Y = rectangle.UpperLeftCorner.Y + size.Height + 6;
	}

	if ( text )
	{
		IGUISkin* skin = Environment->getSkin();
		IGUIFont * font = skin->getFont(EGDF_BUTTON);
		if ( font )
		{
			core::dimension2d<u32> dim = font->getDimension(text);
			if ( (s32)dim.Width > rectangle.getWidth() )
				rectangle.LowerRightCorner.X = rectangle.UpperLeftCorner.X + dim.Width + 8;
			if ( (s32)dim.Height > rectangle.getHeight() )
				rectangle.LowerRightCorner.Y = rectangle.UpperLeftCorner.Y + dim.Height + 6;
		}
	}

	ButtonY += rectangle.getHeight();

	IGUIButton* button = Environment->addButton(rectangle,this,id);

	if (text)
		button->setText(text);

	if (tooltiptext)
		button->setToolTipText(tooltiptext);

	if (img)
		button->setImage(img);

	if (pressed)
		button->setPressedImage(pressed);

	if (isPushButton)
		button->setIsPushButton(isPushButton);

	if (useAlphaChannel)
		button->setUseAlphaChannel(useAlphaChannel);

	return button;
}

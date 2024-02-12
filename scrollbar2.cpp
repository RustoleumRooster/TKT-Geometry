// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "scrollbar2.h"
#include <irrlicht.h>
#include "os.h"
#include "CGUIButton.h"
#include <iostream>

using namespace irr;
using namespace gui;

//! constructor
CGUIScrollBar2::CGUIScrollBar2(bool horizontal, IGUIEnvironment* environment,
				IGUIElement* parent, s32 id,
				core::rect<s32> rectangle, bool noclip)
	: IGUIScrollBar(environment, parent, id, rectangle), UpButton(0),
	DownButton(0), Horizontal(horizontal),
	DraggedBySlider(false), Pos(0), DrawPos(0),
	DrawHeight(0), Min(0), Max(100), SmallStep(24), LargeStep(96), DesiredPos(0),
	LastChange(0)
{
	#ifdef _DEBUG
	setDebugName("CGUIScrollBar2");
	#endif

	refreshControls();

	setNotClipped(noclip);

	// this element can be tabbed to
	setTabStop(true);
	setTabOrder(-1);

	if(horizontal)
        DrawHeight = RelativeRect.getHeight();
    else
        DrawHeight = RelativeRect.getWidth();

	setPos(0);
}


//! destructor
CGUIScrollBar2::~CGUIScrollBar2()
{
	//if (UpButton)
	//	UpButton->drop();

	//if (DownButton)
	//	DownButton->drop();
}


//! called if an event happened.
bool CGUIScrollBar2::OnEvent(const SEvent& event)
{
	if (isEnabled())
	{

		switch(event.EventType)
		{
		case EET_KEY_INPUT_EVENT:
			if (event.KeyInput.PressedDown)
			{
				const s32 oldPos = getPos();
				bool absorb = true;
				switch (event.KeyInput.Key)
				{
				case KEY_LEFT:
				case KEY_UP:
					setPos(getPos()-SmallStep);
					break;
				case KEY_RIGHT:
				case KEY_DOWN:
					setPos(getPos() +SmallStep);
					break;
				case KEY_HOME:
					setPos(Min);
					break;
				case KEY_PRIOR:
					setPos(getPos() -LargeStep);
					break;
				case KEY_END:
					setPos(Max);
					break;
				case KEY_NEXT:
					setPos(getPos() +LargeStep);
					break;
				default:
					absorb = false;
				}

				if (Pos != oldPos)
				{
					SEvent newEvent;
					newEvent.EventType = EET_GUI_EVENT;
					newEvent.GUIEvent.Caller = this;
					newEvent.GUIEvent.Element = 0;
					newEvent.GUIEvent.EventType = EGET_SCROLL_BAR_CHANGED;
					Parent->OnEvent(newEvent);
				}
				if (absorb)
					return true;
			}
			break;
		case EET_GUI_EVENT:
			if (event.GUIEvent.EventType == EGET_BUTTON_CLICKED)
			{
				if (event.GUIEvent.Caller == UpButton)
					setPos(getPos() - SmallStep);
				else
				if (event.GUIEvent.Caller == DownButton)
					setPos(getPos() + SmallStep);

				SEvent newEvent;
				newEvent.EventType = EET_GUI_EVENT;
				newEvent.GUIEvent.Caller = this;
				newEvent.GUIEvent.Element = 0;
				newEvent.GUIEvent.EventType = EGET_SCROLL_BAR_CHANGED;
				Parent->OnEvent(newEvent);

				return true;
			}
			else
			if (event.GUIEvent.EventType == EGET_ELEMENT_FOCUS_LOST)
			{
				if (event.GUIEvent.Caller == this)
					DraggedBySlider = false;
			}
			break;
		case EET_MOUSE_INPUT_EVENT:
		{
			
			const core::position2di p(event.MouseInput.X, event.MouseInput.Y);
			bool isInside = isPointInside ( p );
			switch(event.MouseInput.Event)
			{
			case EMIE_MOUSE_WHEEL:
			{
				setPos(	getPos() +
						( (event.MouseInput.Wheel < 0 ? -1 : 1) * SmallStep * (Horizontal ? 1 : -1 ) )
						);

				SEvent newEvent;
				newEvent.EventType = EET_GUI_EVENT;
				newEvent.GUIEvent.Caller = this;
				newEvent.GUIEvent.Element = 0;
				newEvent.GUIEvent.EventType = EGET_SCROLL_BAR_CHANGED;
				Parent->OnEvent(newEvent);
				return true;
				break;
			}
			case EMIE_LMOUSE_PRESSED_DOWN:
			{
				if (isInside)
				{
					
					DraggedBySlider = SliderRect.isPointInside(p);

					DragOffset = getPosFromMousePos(p) - getPos();

					DesiredPos = getPosFromMousePos(p) - DragOffset;

					Environment->setFocus ( this );
					return true;
				}
				break;
			}
			case EMIE_LMOUSE_LEFT_UP:
			{
				if (!DraggedBySlider)
				{
					int oldPos = getPos();
					int pos = getPosFromMousePos(p);

					if (pos < getPos())
						setPos(getPos() - LargeStep);
					else
						setPos(getPos() + LargeStep);

					if (getPos() != oldPos && Parent)
					{
						SEvent newEvent;
						newEvent.EventType = EET_GUI_EVENT;
						newEvent.GUIEvent.Caller = this;
						newEvent.GUIEvent.Element = 0;
						newEvent.GUIEvent.EventType = EGET_SCROLL_BAR_CHANGED;
						Parent->OnEvent(newEvent);
					}
				}
			}break;
			case EMIE_MOUSE_MOVED:
			{
				if (!event.MouseInput.isLeftPressed())
				{
					DraggedBySlider = false;
				}

				if (!DraggedBySlider)
					return isInside;

				const s32 newPos = getPosFromMousePos(p) - DragOffset;
				const s32 oldPos = getPos();

				if (DraggedBySlider)
				{
					setPos(newPos);
				}

				if (getPos() != oldPos && Parent)
				{
					SEvent newEvent;
					newEvent.EventType = EET_GUI_EVENT;
					newEvent.GUIEvent.Caller = this;
					newEvent.GUIEvent.Element = 0;
					newEvent.GUIEvent.EventType = EGET_SCROLL_BAR_CHANGED;
					Parent->OnEvent(newEvent);
				}
				return isInside;
			} break;

			default:
				break;
			}
		} break;
		default:
			break;
		}
	}

	return IGUIElement::OnEvent(event);
}

//! draws the element and its children
void CGUIScrollBar2::draw()
{
	if (!IsVisible)
		return;

	IGUISkin* skin = Environment->getSkin();
	if (!skin)
		return;


	video::SColor iconColor = skin->getColor(isEnabled() ? EGDC_WINDOW_SYMBOL : EGDC_GRAY_WINDOW_SYMBOL);
	if ( iconColor != CurrentIconColor )
	{
		refreshControls();
	}

	SliderRect = AbsoluteRect;

	skin->draw2DRectangle(this, skin->getColor(EGDC_SCROLLBAR), SliderRect, &AbsoluteClippingRect);

	if ( core::isnotzero ( range() ) )
	{
		if (Horizontal)
		{
			SliderRect.UpperLeftCorner.X = AbsoluteRect.UpperLeftCorner.X + DrawPos + RelativeRect.getHeight() - DrawHeight/2;
			SliderRect.LowerRightCorner.X = SliderRect.UpperLeftCorner.X + DrawHeight;
		}
		else
		{
		    SliderRect.UpperLeftCorner.Y = AbsoluteRect.UpperLeftCorner.Y + DrawPos + RelativeRect.getWidth() - DrawHeight/2;
			SliderRect.LowerRightCorner.Y = SliderRect.UpperLeftCorner.Y + DrawHeight;
		}

		skin->draw3DButtonPaneStandard(this, SliderRect, &AbsoluteClippingRect);
	}

	IGUIElement::draw();
}


void CGUIScrollBar2::updateAbsolutePosition()
{
	IGUIElement::updateAbsolutePosition();
	// todo: properly resize
	refreshControls();
	//setPos ( Pos );
}

//!
s32 CGUIScrollBar2::getPosFromMousePos(const core::position2di &pos) const
{
	f32 p;
	if (Horizontal)
	{
		p = (f32)(pos.X - RelativeRect.getHeight() - AbsoluteRect.UpperLeftCorner.X) / (RelativeRect.getWidth() - (DrawHeight + RelativeRect.getHeight() * 2.0f));
		p *= (Max - RelativeRect.getWidth());
	}
	else
	{
		p = (f32)(pos.Y - RelativeRect.getWidth() - AbsoluteRect.UpperLeftCorner.Y) / (RelativeRect.getHeight() - (DrawHeight + RelativeRect.getWidth() * 2.0f));
		p *= (Max - RelativeRect.getHeight());
	}
	
	return p;
}


//! sets the position of the scrollbar
void CGUIScrollBar2::setPos(s32 pos)
{
	if (Horizontal)
	{
		Pos = (f32)pos * Max / (f32)((Max - RelativeRect.getWidth()));
	}
	else
	{
		Pos = (f32)pos * Max / (f32)((Max - RelativeRect.getHeight()));
	}

	Pos = core::s32_clamp ( Pos, Min, Max );

	if (Horizontal)
	{
		f32 f = ((f32)RelativeRect.getWidth() - ((f32)DrawHeight + RelativeRect.getHeight()*2.0f)) / range();
		DrawPos = (s32)(f * ( Pos - Min ) + ((f32)DrawHeight * 0.5f));
	}
	else
	{
		f32 f = ((f32)RelativeRect.getHeight() - ((f32)DrawHeight + RelativeRect.getWidth()*2.0f)) / range();
		DrawPos = (s32)(f * ( Pos - Min ) + ((f32)DrawHeight * 0.5f));
	}
}


//! gets the small step value
s32 CGUIScrollBar2::getSmallStep() const
{
	return SmallStep;
}


//! sets the small step value
void CGUIScrollBar2::setSmallStep(s32 step)
{
	if (step > 0)
		SmallStep = step;
	else
		SmallStep = 10;
}


//! gets the small step value
s32 CGUIScrollBar2::getLargeStep() const
{
	return LargeStep;
}


//! sets the small step value
void CGUIScrollBar2::setLargeStep(s32 step)
{
	if (step > 0)
		LargeStep = step;
	else
		LargeStep = 50;
}


//! gets the maximum value of the scrollbar.
s32 CGUIScrollBar2::getMax() const
{
	return Max;
}


//! sets the maximum value of the scrollbar.
void CGUIScrollBar2::setMax(s32 max)
{
	Max = max;
	if ( Min > Max )
		Min = Max;

	s32 slider_len;
	f32 lr;
	
	if (Horizontal)
	{
		lr = (f32)RelativeRect.getWidth() / (f32)Max;
		slider_len = (RelativeRect.getWidth() - RelativeRect.getHeight() * 2) * lr;

		DrawHeight = std::max(slider_len, RelativeRect.getHeight());

		setLargeStep(RelativeRect.getWidth());
	}
	else
	{
		lr = (f32)RelativeRect.getHeight() / (f32)Max;
		slider_len = (RelativeRect.getHeight() - RelativeRect.getWidth() * 2) * lr;

		DrawHeight = std::max(slider_len, RelativeRect.getWidth());

		setLargeStep(RelativeRect.getHeight());
	}

	bool enable = core::isnotzero ( range() );
	UpButton->setEnabled(enable);
	DownButton->setEnabled(enable);
	setPos(Pos);
}

//! gets the minimum value of the scrollbar.
s32 CGUIScrollBar2::getMin() const
{
	return Min;
}

//! sets the minimum value of the scrollbar.
void CGUIScrollBar2::setMin(s32 min)
{
	Min = min;
	if ( Max < Min )
		Max = Min;

	bool enable = core::isnotzero ( range() );
	UpButton->setEnabled(enable);
	DownButton->setEnabled(enable);
	setPos(Pos);
}


//! gets the current position of the scrollbar
s32 CGUIScrollBar2::getPos() const
{
	if(Horizontal)
		return (range() - RelativeRect.getWidth()) * (f32)(Pos / range());
	else
		return (range() - RelativeRect.getHeight()) * (f32)(Pos / range());
}


//! refreshes the position and text on child buttons
void CGUIScrollBar2::refreshControls()
{
	CurrentIconColor = video::SColor(255,255,255,255);

	IGUISkin* skin = Environment->getSkin();
	IGUISpriteBank* sprites = 0;

	if (skin)
	{
		sprites = skin->getSpriteBank();
		CurrentIconColor = skin->getColor(isEnabled() ? EGDC_WINDOW_SYMBOL : EGDC_GRAY_WINDOW_SYMBOL);
	}

	if (Horizontal)
	{
		s32 h = RelativeRect.getHeight();
		if (!UpButton)
		{
			//UpButton = new CGUIButton(Environment, this, -1, core::rect<s32>(0,0, h, h), NoClip);
			UpButton = Environment->addButton(core::rect<s32>(0, 0, h, h), this, -1, L"", L"");
			UpButton->setSubElement(true);
			UpButton->setTabStop(false);
		}
		if (sprites)
		{
			UpButton->setSpriteBank(sprites);
			UpButton->setSprite(EGBS_BUTTON_UP, skin->getIcon(EGDI_CURSOR_LEFT), CurrentIconColor);
			UpButton->setSprite(EGBS_BUTTON_DOWN, skin->getIcon(EGDI_CURSOR_LEFT), CurrentIconColor);
		}
		UpButton->setRelativePosition(core::rect<s32>(0,0, h, h));
		UpButton->setAlignment(EGUIA_UPPERLEFT, EGUIA_UPPERLEFT, EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT);
		if (!DownButton)
		{
			//DownButton = new CGUIButton(Environment, this, -1, core::rect<s32>(RelativeRect.getWidth()-h, 0, RelativeRect.getWidth(), h), NoClip);
			DownButton = Environment->addButton(core::rect<s32>(RelativeRect.getWidth() - h, 0, RelativeRect.getWidth(), h), this, -1, L"", L"");
			DownButton->setSubElement(true);
			DownButton->setTabStop(false);
		}
		if (sprites)
		{
			DownButton->setSpriteBank(sprites);
			DownButton->setSprite(EGBS_BUTTON_UP, skin->getIcon(EGDI_CURSOR_RIGHT), CurrentIconColor);
			DownButton->setSprite(EGBS_BUTTON_DOWN, skin->getIcon(EGDI_CURSOR_RIGHT), CurrentIconColor);
		}
		DownButton->setRelativePosition(core::rect<s32>(RelativeRect.getWidth()-h, 0, RelativeRect.getWidth(), h));
		DownButton->setAlignment(EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT);
	}
	else
	{
		s32 w = RelativeRect.getWidth();
		if (!UpButton)
		{
			//UpButton = new CGUIButton(Environment, this, -1, core::rect<s32>(0,0, w, w), NoClip);
			UpButton = Environment->addButton(core::rect<s32>(0, 0, w, w), this, -1, L"", L"");
			UpButton->setSubElement(true);
			UpButton->setTabStop(false);
		}
		if (sprites)
		{
			UpButton->setSpriteBank(sprites);
			UpButton->setSprite(EGBS_BUTTON_UP, skin->getIcon(EGDI_CURSOR_UP), CurrentIconColor);
			UpButton->setSprite(EGBS_BUTTON_DOWN, skin->getIcon(EGDI_CURSOR_UP), CurrentIconColor);
		}
		UpButton->setRelativePosition(core::rect<s32>(0,0, w, w));
		UpButton->setAlignment(EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_UPPERLEFT);
		if (!DownButton)
		{
			//DownButton = new CGUIButton(Environment, this, -1, core::rect<s32>(0,RelativeRect.getHeight()-w, w, RelativeRect.getHeight()), NoClip);
			DownButton = Environment->addButton(core::rect<s32>(0, RelativeRect.getHeight() - w, w, RelativeRect.getHeight()), this, -1, L"", L"");
			DownButton->setSubElement(true);
			DownButton->setTabStop(false);
		}
		if (sprites)
		{
			DownButton->setSpriteBank(sprites);
			DownButton->setSprite(EGBS_BUTTON_UP, skin->getIcon(EGDI_CURSOR_DOWN), CurrentIconColor);
			DownButton->setSprite(EGBS_BUTTON_DOWN, skin->getIcon(EGDI_CURSOR_DOWN), CurrentIconColor);
		}
		DownButton->setRelativePosition(core::rect<s32>(0,RelativeRect.getHeight()-w, w, RelativeRect.getHeight()));
		DownButton->setAlignment(EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT);
	}
}


//! Writes attributes of the element.
void CGUIScrollBar2::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const
{
	IGUIScrollBar::serializeAttributes(out,options);

	out->addBool("Horizontal",	Horizontal);
	out->addInt ("Value",		Pos);
	out->addInt ("Min",			Min);
	out->addInt ("Max",			Max);
	out->addInt ("SmallStep",	SmallStep);
	out->addInt ("LargeStep",	LargeStep);
	// CurrentIconColor - not serialized as continuiously updated
}


//! Reads attributes of the element
void CGUIScrollBar2::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options=0)
{
	IGUIScrollBar::deserializeAttributes(in,options);

	Horizontal = in->getAttributeAsBool("Horizontal");
	setMin(in->getAttributeAsInt("Min"));
	setMax(in->getAttributeAsInt("Max"));
	setPos(in->getAttributeAsInt("Value"));
	setSmallStep(in->getAttributeAsInt("SmallStep"));
	setLargeStep(in->getAttributeAsInt("LargeStep"));
	// CurrentIconColor - not serialized as continuiously updated

	refreshControls();
}



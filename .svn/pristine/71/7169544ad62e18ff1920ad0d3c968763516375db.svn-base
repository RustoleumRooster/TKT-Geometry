
#ifndef _VTOOLBAR_H_
#define _VTOOLBAR_H_


#include "IGUIToolbar.h"
using namespace irr;
using namespace gui;

	//! Stays at the top of its parent like the menu bar and contains tool buttons
	class VToolBar : public IGUIToolBar
	{
	public:

		//! constructor
		VToolBar(IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle);

		//! called if an event happened.
		virtual bool OnEvent(const SEvent& event);

		//! draws the element and its children
		virtual void draw();

		//! Updates the absolute position.
		virtual void updateAbsolutePosition();

		//! Adds a button to the tool bar
		virtual IGUIButton* addButton(s32 id=-1, const wchar_t* text=0,const wchar_t* tooltiptext=0,
			video::ITexture* img=0, video::ITexture* pressed=0,
			bool isPushButton=false, bool useAlphaChannel=false);

	private:

		s32 ButtonY;
	};

#endif

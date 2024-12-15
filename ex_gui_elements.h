#include <irrlicht.h>
#include "CGUIButton.h"
#include "CGUISkin.h"
#include <string>

using namespace irr;

class MySkin : public gui::CGUISkin
{
public:
    MySkin(gui::EGUI_SKIN_TYPE type, video::IVideoDriver* driver_)
     : gui::CGUISkin(type, driver_), driver(driver_) {
     }

     virtual void draw3DButtonPaneStandard(gui::IGUIElement* element,
				const core::rect<s32>& rect,
				const core::rect<s32>* clip=0);

     virtual void draw3DButtonPanePressed(gui::IGUIElement* element,
				const core::rect<s32>& rect,
				const core::rect<s32>* clip=0);

    virtual void draw3DSunkenPane(gui::IGUIElement* element,
				video::SColor bgcolor, bool flat,
				bool fillBackGround,
				const core::rect<s32>& rect,
				const core::rect<s32>* clip=0);

private:
    video::IVideoDriver* driver;
};


class Flat_Button : public gui::IGUIElement
{
public:
    Flat_Button(gui::IGUIEnvironment* env, gui::IGUIElement* parent,s32 id,core::rect<s32> rect);

    virtual void draw();
    virtual bool OnEvent(const SEvent& event);

    virtual void setSpriteBank(gui::IGUISpriteBank* bank=0);
    virtual void setSprite(gui::EGUI_BUTTON_STATE state, s32 index,
				video::SColor color=video::SColor(255,255,255,255), bool loop=false);
   // virtual void setText(const wchar_t* text) {}

private:
    struct ButtonSprite
		{
			s32 Index;
			video::SColor Color;
			bool Loop;
		};

    ButtonSprite ButtonSprites[gui::EGBS_COUNT];

    gui::IGUISpriteBank* SpriteBank = NULL;
    bool hovered = false;
    bool pressed = false;
    bool DrawBorder = true;
};

#pragma once
#ifndef _TKT_FONT_H_
#define _TKT_FONT_H_

#include <irrlicht.h>
#include "RenderTargetsTool.h"

using namespace irr;

class FontDraw;

class AGG_TT_Font : public gui::IGUIFont
{
public:

	AGG_TT_Font(gui::IGUIEnvironment* env, gui::IGUISpriteBank* SpriteBank, s32 height, f32 weight, s32 kerning_wdith, s32 kerning_height);
	~AGG_TT_Font();

	virtual void draw(const core::stringw& text, const core::rect<s32>& position,
		video::SColor color, bool hcenter = false, bool vcenter = false,
		const core::rect<s32>* clip = 0);

	virtual core::dimension2d<u32> getDimension(const wchar_t* text) const;
	virtual s32 getCharacterFromPos(const wchar_t* text, s32 pixel_x) const;
	virtual gui::EGUI_FONT_TYPE getType() const { return gui::EGFT_CUSTOM; }
	virtual void setKerningWidth(s32 kerning);
	virtual void setKerningHeight(s32 kerning);
	virtual s32 getKerningWidth(const wchar_t* thisLetter = 0, const wchar_t* previousLetter = 0) const;
	virtual s32 getKerningHeight() const;

	//! Define which characters should not be drawn by the font.
	/** For example " " would not draw any space which is usually blank in
	most fonts.**/
	virtual void setInvisibleCharacters(const wchar_t* s);

	s32 getAreaFromCharacter(const wchar_t c) const;

	struct SFontArea
	{
		SFontArea() : underhang(0), overhang(0), width(0), height(0), spriteno(0) {}
		s32				underhang;
		s32				overhang;
		s32				width;
		s32				height;
		u32				spriteno;
	};

	core::array<SFontArea>		Areas;
	core::map<wchar_t, s32>		CharacterMap;
	video::IVideoDriver* Driver;
	gui::IGUISpriteBank* SpriteBank;
	gui::IGUIEnvironment* Environment;
	u32				WrongCharacter;
	s32				MaxHeight;
	s32				GlobalKerningWidth, GlobalKerningHeight;
	s32				Cap_Height;
	f32				weight;

	core::stringw Invisible;
};


class AGG_TT_Font_Renderer
{
public:

	AGG_TT_Font* Render_Font(gui::IGUIEnvironment* env,
		const char* typeface, u32 height, f32 weight, s32 kerning_width, s32 kerning_height, dimension2du image_dimension);

private:

	bool renderImage(AGG_TT_Font*, irr::video::IImage* pImage, irr::video::ITexture* imgtex, FontDraw& myFont, gui::IGUISpriteBank* SpriteBank, bool makeBMP = false);

	void pushTextureCreationFlags(bool(&flags)[3]);
	void popTextureCreationFlags(const bool(&flags)[3]);

	video::IVideoDriver* Driver = NULL;
};

#endif
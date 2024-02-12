#ifndef _TEXTURE_ICON_H_
#define _TEXTURE_ICON_H_
#include <irrlicht.h>

using namespace irr;
using namespace gui;


	class TextureIcon : public gui::IGUIElement
	{
	public:

		//! constructor
		TextureIcon(IGUIEnvironment* environment, IGUIElement* parent, s32 id, core::rect<s32> rectangle);

		//! destructor
		virtual ~TextureIcon();

		virtual void setColor(video::SColor color);
		virtual video::SColor getColor() const;

        virtual bool OnEvent(const SEvent& event);

		//! sets an image
		virtual void setImage(video::ITexture* image);
		void setText(std::wstring txt);
		void setTextureID(int);
		void setSelected(bool);

		//! Gets the image texture
		virtual video::ITexture* getImage() const;


		//! draws the element and its children
		virtual void draw();


	private:
		video::ITexture* Texture;
		video::SColor Color;
		int draw_size=100;
		int border=8;
		int texture_id=0;
		std::string text;
		bool selected = false;


	};

#endif

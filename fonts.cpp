
#include <agg_basics.h>
#include <agg_rendering_buffer.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include "agg_font_win32_tt.h"
#include <agg_path_storage.h>
#include "agg_conv_marker.h"
#include "agg_vcgen_markers_term.h"

// Specifically for the curve
#include <agg_conv_stroke.h>
#include <agg_conv_bspline.h>

#include <agg_conv_curve.h>
#include <agg_conv_contour.h>

#define AGG_ARGB32
#include <pixel_formats.h>

#include <irrlicht.h>
#include "fonts.h"
#include "RenderTargetsTool.h"
#include "utils.h"

using namespace irr;
using namespace video;
using namespace gui;

extern IrrlichtDevice* device;



class FontDraw
{
    typedef agg::font_engine_win32_tt_int32 font_engine_type;
    typedef agg::font_cache_manager<font_engine_type> font_manager_type;

    font_engine_type             m_feng;
    font_manager_type            m_fman;

    // Pipeline to process the vectors glyph paths (curves + contour)
    typedef agg::conv_curve<font_manager_type::path_adaptor_type> conv_curve_type;
    typedef agg::conv_contour<conv_curve_type> conv_contour_type;

    conv_curve_type m_curves;
    conv_contour_type m_contour;

    const char* typeface = NULL;
    int    m_rentype = 0;
    double m_weight = 2;
    double m_height = 30;
    double m_width = 20;
    agg::rgba m_color = agg::rgba(0.6 * 0.8, 0.8, 0.6, 1);

public:

    void size(double height, double width, double weight)
    {
        m_weight = weight;
        m_height = height;
        m_width = width;
    }

    void color(agg::rgba color)
    {
        m_color = color;
    }

    FontDraw(HDC dc, const char* typeface) : m_feng(dc),
        m_fman(m_feng),
        m_curves(m_fman.path_adaptor()),
        m_contour(m_curves),
        typeface(typeface)
    {}

    
    template<class Rasterizer, class Scanline, class RenSolid, class RenBin>
    unsigned draw_text(gui::IGUISpriteBank* SpriteBank, AGG_TT_Font* font, Rasterizer& ras, Scanline& sl,
        RenSolid& ren_solid, RenBin& ren_bin)
    {
        unsigned num_glyphs = 0;

        m_height = font->MaxHeight;
        m_feng.width(m_height / 2.0);
        m_feng.flip_y(true);

        ren_bin.color(agg::rgba(1, 1, 1, 1));

        //agg::glyph_rendering gren = agg::glyph_ren_outline;
        agg::glyph_rendering gren = agg::glyph_ren_agg_gray8;

        m_color = agg::rgba(1, 1, 1, 1);

        m_feng.hinting(true);
        m_feng.height(m_height);
        m_feng.weight(font->weight);

        m_contour.width(font->weight * 0.1 * m_height * 0.1);

        if (m_feng.create_font(typeface, gren))
        {
            m_fman.precache(' ', 127);
            {
                double x = 0;
                double y = 0;

                const agg::glyph_cache* glyph;

                ras.reset();

                ren_bin.color(m_color);
                ren_solid.color(m_color);

                int total_len=0;

                for (char ch = ' '; ch < 127; ch++)
                {
                    glyph = m_fman.glyph(ch);
                    if (glyph)
                    {
                        total_len += glyph->advance_x;
                    }
                }
                int glyph_count = 0;
                for(char ch = ' ' ; ch<127 ; ch++)
                {
                    glyph = m_fman.glyph(ch);

                    if (glyph)
                    {
                        if (x + glyph->advance_x + 4 > 255)
                        {
                            x = 0;
                            y += m_height + 2;
                        }

                        SpriteBank->getPositions().push_back(core::rect<s32>());
                        SpriteBank->getPositions()[glyph_count].UpperLeftCorner = core::dimension2di(x,y);

                        //m_fman.init_embedded_adaptors(glyph, x, y + m_height);
                        m_fman.init_embedded_adaptors(glyph, x, y + m_height * 0.75);

                        switch (glyph->data_type)
                        {
                        case agg::glyph_data_mono:
                            agg::render_scanlines(m_fman.mono_adaptor(),
                                m_fman.mono_scanline(),
                                ren_bin);
                            break;

                        case agg::glyph_data_gray8:
                            
                            agg::render_scanlines(m_fman.gray8_adaptor(),
                                m_fman.gray8_scanline(),
                                ren_solid);
                            break;

                        case agg::glyph_data_outline:
                            ras.reset();
                            if (fabs(m_weight) <= 0.01)
                            {
                                // For the sake of efficiency skip the
                                // contour converter if the weight is about zero.
                                //-----------------------
                                ras.add_path(m_curves);
                            }
                            else
                            {
                                ras.add_path(m_contour);
                            }
                            agg::render_scanlines(ras, sl, ren_solid);
                            break;
                        }

                        // increment pen position
                        x += glyph->advance_x + 4;
                        y += glyph->advance_y;

                        SpriteBank->getPositions()[glyph_count].LowerRightCorner = core::dimension2di(x - 4, (y + m_height));

                        // add frame to sprite bank
                        SGUISpriteFrame f;
                        f.rectNumber = glyph_count;
                        f.textureNumber = 0;
                        SGUISprite s;
                        s.Frames.push_back(f);
                        s.frameTime = 0;
                        SpriteBank->getSprites().push_back(s);

                        // add character to font
                        AGG_TT_Font::SFontArea a;
                        a.overhang = 0;
                        a.underhang = 0;
                        a.spriteno = glyph_count;
                        a.height = glyph->bounds.y2 - glyph->bounds.y1; //unused
                        if (ch == 'M')
                        {
                            font->Cap_Height = -glyph->bounds.y1;
                        }
                        a.width = SpriteBank->getPositions()[glyph_count].getWidth();
                        font->Areas.push_back(a);

                        // map letter to character
                        wchar_t ch_ = (wchar_t)(glyph_count + 32);
                        font->CharacterMap.set(ch_, glyph_count);

                        ++num_glyphs;
                        ++glyph_count;
                    }
                }
            }
        }//if font
        else cout << "no font\n";
        return num_glyphs;
    }
};

AGG_TT_Font::AGG_TT_Font(gui::IGUIEnvironment* env, IGUISpriteBank* SpriteBank, s32 height, f32 weight, s32 kerning_width, s32 kerning_height)
    : Driver(0), SpriteBank(SpriteBank), Environment(env), WrongCharacter(0),
    MaxHeight(height), GlobalKerningWidth(kerning_width), GlobalKerningHeight(kerning_height), weight(weight)
{
    //SpriteBank = Environment->addEmptySpriteBank("font_spritebank");

    if (SpriteBank)
        SpriteBank->grab();

    Driver = env->getVideoDriver();
}

AGG_TT_Font::~AGG_TT_Font()
{
    if (SpriteBank)
    {
        SpriteBank->drop();
    }
}
/*
void AGG_TT_Font::resizeView(core::dimension2du new_size)
{
    SpriteBank->clear();

    if (test_img)
        test_img->drop();

    if (test_tex)
        Driver->removeTexture(test_tex);

    bool mip_maps = Driver->getTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS);
    bool non_power_2 = Driver->getTextureCreationFlag(irr::video::ETCF_ALLOW_NON_POWER_2);

    Driver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);
    Driver->setTextureCreationFlag(irr::video::ETCF_ALLOW_NON_POWER_2, true);

    IImage* img = Driver->createImage(ECF_A8R8G8B8, new_size);

    ITexture* tex = Driver->addTexture("fonts", img);

    Driver->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, mip_maps);
    Driver->setTextureCreationFlag(irr::video::ETCF_ALLOW_NON_POWER_2, non_power_2);

    SpriteBank->addTexture(0);
    SpriteBank->setTexture(0,tex);

    //uncomment to use the Render Tool View
    //Render_Tool::connect_image(this);

    test_img = img;
    test_tex = tex;

    HDC dc = ::GetDC(0);
    FontDraw myFont(dc);

    renderImage(test_img, SpriteBank->getTexture(0), myFont, false);
}*/


int CachedText_Manager::cache(CachedText& text_, position2di pos_)
{
    text.push_back(text_);
    pos.push_back(pos_);
    return text.size() - 1;
}

CachedText_Manager::CachedText_Manager(AGG_TT_Font* font) : my_font(font), driver{device->getVideoDriver()},my_texture{font->SpriteBank->getTexture(0) }
{
}

void CachedText_Manager::fill_positions()
{
    u32 n = text.size();

    u32(*length)(vector<CachedText>*, u32);
    vector2di(*item0)(vector<CachedText>*, u32, u32);
    recti(*item1)(vector<CachedText>*, u32, u32);

    length = [](vector<CachedText>* text_, u32 index) -> u32 {
        return text_->data()[index].offsets.size();
        };

    item0 = [](vector<CachedText>* text_, u32 i, u32 j) -> vector2di {
        return text_->data()[i].offsets[j];
        };

    item1 = [](vector<CachedText>* text_, u32 i, u32 j) -> recti {
        return text_->data()[i].src_rects[j];
        };

    positions.fill_data(&text, n, length, item0);
    image_batch.fill_data(&text, n, length, item0, item1);
}

void CachedText_Manager::draw(vector2di origin, video::SColor color, const core::rect<s32>* clip)
{
    memcpy(image_batch.data0.data(), positions.data.data(), positions.data.size() * sizeof(vector2di));

    for (int i = 0; i < pos.size(); i++)
    {
        int offset = image_batch.offset[i];
        for (int j = 0; j < image_batch.len[i]; j++)
        {
            //image_batch.data0.data()[offset + j] = positions.data.data()[offset + j];
            image_batch.data0.data()[offset + j] += pos[i];
            image_batch.data0.data()[offset + j] += origin;
        }
    }
    
    core::array<position2di> r;
    core::array<recti> d;

    r.set_pointer(image_batch.data0.data(), image_batch.data0.size(), false, false);
    d.set_pointer(image_batch.data1.data(), image_batch.data1.size(), false, false);

    driver->draw2DImageBatch(my_texture, r, d, clip, color, true);

   // Driver->draw2DImageBatch(getTexture(i), drawBatches[i].positions,
  //      drawBatches[i].sourceRects, clip, color, true);
}

void CachedText_Manager::setInitialTextPos()
{
    for (int i = 0; i < pos.size(); i++)
    {
        int offset = image_batch.offset[i];
        for (int j = 0; j < image_batch.len[i]; j++)
        {
            //image_batch.data0.data()[offset + j] = positions.data.data()[offset + j];
            //positions.data.data()[offset + j] -= pos[i];
        }
    }
}

void AGG_TT_Font::draw(const core::stringw& text, const core::rect<s32>& position, video::SColor color, bool hcenter, bool vcenter, const core::rect<s32>* clip)
{
    if (!Driver || !SpriteBank)
        return;

    core::dimension2d<s32> textDimension;	// NOTE: don't make this u32 or the >> later on can fail when the dimension width is < position width
    core::position2d<s32> offset = position.UpperLeftCorner;

    if (hcenter || vcenter || clip)
        textDimension = getDimension(text.c_str());
    
    if (hcenter)
    {
        offset.X += (position.getWidth() - textDimension.Width) >> 1;
    }

    if (vcenter)
    {
        //offset.Y += (position.getHeight() - Cap_Height) >> 1;
        offset.Y += (position.getHeight() - textDimension.Height) >> 1;
    }

    if (clip)
    {
        core::rect<s32> clippedRect(offset, textDimension);
        clippedRect.clipAgainst(*clip);
        if (!clippedRect.isValid())
            return;
    }

    core::array<u32> indices(text.size());
    core::array<core::position2di> offsets(text.size());

    for (u32 i = 0; i < text.size(); i++)
    {
        wchar_t c = text[i];

        bool lineBreak = false;
        if (c == L'\r') // Mac or Windows breaks
        {
            lineBreak = true;
            if (text[i + 1] == L'\n') // Windows breaks
                c = text[++i];
        }
        else if (c == L'\n') // Unix breaks
        {
            lineBreak = true;
        }

        if (lineBreak)
        {
            offset.Y += MaxHeight;
            offset.X = position.UpperLeftCorner.X;

            if (hcenter)
            {
                offset.X += (position.getWidth() - textDimension.Width) >> 1;
            }
            continue;
        }

        SFontArea& area = Areas[getAreaFromCharacter(c)];

        offset.X += area.underhang;
        if (Invisible.findFirst(c) < 0)
        {
            indices.push_back(area.spriteno);
            offsets.push_back(offset);
        }

        offset.X += area.width + area.overhang + GlobalKerningWidth;
    }

    SpriteBank->draw2DSpriteBatch(indices, offsets, clip, color);
}

void AGG_TT_Font::getCachedText(CachedText& cache, const core::stringw& text, position2di pos, dimension2di rect, int alignment)
{
    //if (!Driver || !SpriteBank)
    //    return;

    //const core::rect<s32>& position{ 0,0 };

    core::dimension2d<s32> textDimension;	// NOTE: don't make this u32 or the >> later on can fail when the dimension width is < position width
    core::position2d<s32> offset = pos;

    if (alignment == TKT_ALIGN_RIGHT)
    {
        textDimension = getDimension(text.c_str());
        offset.X -= textDimension.Width - rect.Width;
    }

    /*
    if (hcenter || vcenter || clip)
        textDimension = getDimension(text.c_str());
    
    if (hcenter)
    {
        offset.X += (position.getWidth() - textDimension.Width) >> 1;
    }

    if (vcenter)
    {
        //offset.Y += (position.getHeight() - Cap_Height) >> 1;
        offset.Y += (position.getHeight() - textDimension.Height) >> 1;
    }*/

    /*
    if (clip)
    {
        core::rect<s32> clippedRect(offset, textDimension);
        clippedRect.clipAgainst(*clip);
        if (!clippedRect.isValid())
            return;
    }*/

    //core::array<u32> indices(text.size());
   // core::array<core::position2di> offsets(text.size());
    //core::array<recti> src_rects(text.size());

    int new_size = cache.src_rects.size() + text.size();

    cache.src_rects.reallocate(new_size);
    cache.offsets.reallocate(new_size);

    for (u32 i = 0; i < text.size(); i++)
    {
        wchar_t c = text[i];

        bool lineBreak = false;
        if (c == L'\r') // Mac or Windows breaks
        {
            lineBreak = true;
            if (text[i + 1] == L'\n') // Windows breaks
                c = text[++i];
        }
        else if (c == L'\n') // Unix breaks
        {
            lineBreak = true;
        }

        if (lineBreak)
        {
            offset.Y += MaxHeight;
            //offset.X = position.UpperLeftCorner.X;

            //if (hcenter)
            //{
            //    offset.X += (position.getWidth() - textDimension.Width) >> 1;
            //}
            continue;
        }

        SFontArea& area = Areas[getAreaFromCharacter(c)];

        offset.X += area.underhang;
        if (Invisible.findFirst(c) < 0)
        {
            //indices.push_back(area.spriteno);
            cache.src_rects.push_back(SpriteBank->getPositions()[area.spriteno]);
            cache.offsets.push_back(offset);
        }

        offset.X += area.width + area.overhang + GlobalKerningWidth;
    }

    //return CachedText{ src_rects, offsets, clip };
}

void AGG_TT_Font::draw(CachedText& text, video::SColor color)
{
    //SpriteBank->draw2DSpriteBatch(text.src_rects, text.offsets, text.clip, color);
}

core::dimension2d<u32> AGG_TT_Font::getDimension(const wchar_t* text) const
{
    core::dimension2d<u32> dim(0, 0);
    core::dimension2d<u32> thisLine(0, MaxHeight);

    for (const wchar_t* p = text; *p; ++p)
    {
        bool lineBreak = false;
        if (*p == L'\r') // Mac or Windows breaks
        {
            lineBreak = true;
            if (p[1] == L'\n') // Windows breaks
                ++p;
        }
        else if (*p == L'\n') // Unix breaks
        {
            lineBreak = true;
        }
        if (lineBreak)
        {
            dim.Height += thisLine.Height;
            if (dim.Width < thisLine.Width)
                dim.Width = thisLine.Width;
            thisLine.Width = 0;
            continue;
        }

        const SFontArea& area = Areas[getAreaFromCharacter(*p)];

        thisLine.Width += area.underhang;
        thisLine.Width += area.width + area.overhang + GlobalKerningWidth;
    }

    dim.Height += thisLine.Height;
    if (dim.Width < thisLine.Width)
        dim.Width = thisLine.Width;

    return dim;
}

s32 AGG_TT_Font::getAreaFromCharacter(const wchar_t c) const
{
    core::map<wchar_t, s32>::Node* n = CharacterMap.find(c);
    if (n)
        return n->getValue();
    else
        return WrongCharacter;
}

s32 AGG_TT_Font::getCharacterFromPos(const wchar_t* text, s32 pixel_x) const
{
    s32 x = 0;
    s32 idx = 0;

    while (text[idx])
    {
        const SFontArea& a = Areas[getAreaFromCharacter(text[idx])];

        x += a.width + a.overhang + a.underhang + GlobalKerningWidth;

        if (x >= pixel_x)
            return idx;

        ++idx;
    }

    return -1;
}

void AGG_TT_Font::setKerningWidth(s32 kerning)
{
}

void AGG_TT_Font::setKerningHeight(s32 kerning)
{
    
}

s32 AGG_TT_Font::getKerningWidth(const wchar_t* thisLetter, const wchar_t* previousLetter) const
{
    return s32();
}

s32 AGG_TT_Font::getKerningHeight() const
{
    return 0;
}

void AGG_TT_Font::setInvisibleCharacters(const wchar_t* s)
{
    Invisible = s;
}

AGG_TT_Font* AGG_TT_Font_Renderer::Render_Font(gui::IGUIEnvironment* env, const char* typeface, u32 height, f32 weight, s32 kerning_width, s32 kerning_height, dimension2du image_dimension)
{
    IGUISpriteBank* SpriteBank = env->addEmptySpriteBank("font_spritebank");

    if (SpriteBank)
        SpriteBank->grab();

    Driver = env->getVideoDriver();

    bool flags[3];
    pushTextureCreationFlags(flags);

    IImage* img = Driver->createImage(ECF_A8R8G8B8, dimension2du{ 256,256 });
    ITexture* tex = Driver->addTexture("fonts", img);

    popTextureCreationFlags(flags);

    SpriteBank->addTexture(0);
    SpriteBank->setTexture(0, tex);

    //for retrieving True Type fonts from Windows
    HDC dc = ::GetDC(0);
    FontDraw fontDraw(dc,typeface);

    AGG_TT_Font* font = new AGG_TT_Font(env, SpriteBank, height, weight, kerning_width, kerning_height);

    font->setInvisibleCharacters(L" ");

    renderImage(font, img, SpriteBank->getTexture(0), fontDraw, SpriteBank, false);

    return font;
}

bool AGG_TT_Font_Renderer::renderImage(AGG_TT_Font* font, irr::video::IImage* pImage, irr::video::ITexture* imgtex, FontDraw& FontDrawer, IGUISpriteBank* SpriteBank, bool makeBMP)
{
    // Yes, it's technically u32 data, but Anti-Grain treats it as pixels composed of 1 byte/8 bit colors.
    irr::u8* imgDataPtr = (irr::u8*)pImage->lock();
    irr::core::dimension2du imgSize = pImage->getDimension();

    agg::rendering_buffer renderingBuffer;
    renderingBuffer.attach(imgDataPtr, imgSize.Width, imgSize.Height, pImage->getPitch());

    agg::pixfmt_argb32 pixelFormat(renderingBuffer);
    agg::renderer_base<agg::pixfmt_argb32> rendererBase(pixelFormat);
    agg::scanline_p8 scanLine;
    agg::rasterizer_scanline_aa<> ras;

    renderingBuffer.clear(0);

    typedef agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_argb32> > renderer_solid;
    typedef agg::renderer_scanline_bin_solid<agg::renderer_base<agg::pixfmt_argb32> > renderer_bin;

    renderer_solid ren_solid(rendererBase);
    renderer_bin ren_bin(rendererBase);

    FontDrawer.draw_text(SpriteBank, font, ras, scanLine, ren_solid, ren_bin);

    if(makeBMP)
        generateBitmapImage(imgDataPtr, imgSize.Height, imgSize.Width, "bmpTest.bmp");

    // memcopy results directly to our texture
    //---------------------------
    if (imgtex != NULL)
    {
        irr::core::dimension2d<u32> tDim = imgtex->getSize();
        irr::core::dimension2d<u32> iDim = pImage->getDimension();

        u32 imgPitch = pImage->getBytesPerPixel() * iDim.Width;
        u32 txtPitch = imgtex->getPitch();

        char* txtPtr = (char*)imgtex->lock(irr::video::ETLM_WRITE_ONLY);
        for (u32 c = 0; c < iDim.Height; c++)
        {
            memcpy(txtPtr, imgDataPtr, imgPitch);
            txtPtr += txtPitch;
            imgDataPtr += imgPitch;
        }
        imgtex->unlock();
    }
    pImage->unlock();

    return true;
}

void AGG_TT_Font_Renderer::pushTextureCreationFlags(bool(&flags)[3])
{
    flags[0] = Driver->getTextureCreationFlag(video::ETCF_ALLOW_NON_POWER_2);
    flags[1] = Driver->getTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS);
    flags[2] = Driver->getTextureCreationFlag(video::ETCF_ALLOW_MEMORY_COPY);

    Driver->setTextureCreationFlag(video::ETCF_ALLOW_NON_POWER_2, true);
    Driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
    Driver->setTextureCreationFlag(video::ETCF_ALLOW_MEMORY_COPY, true);
}

void AGG_TT_Font_Renderer::popTextureCreationFlags(const bool(&flags)[3])
{
    Driver->setTextureCreationFlag(video::ETCF_ALLOW_NON_POWER_2, flags[0]);
    Driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, flags[1]);
    Driver->setTextureCreationFlag(video::ETCF_ALLOW_MEMORY_COPY, flags[2]);
}



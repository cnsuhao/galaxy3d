#include "Label.h"
#include "GTStringUTF32.h"
#include <ft2build.h>
#include FT_FREETYPE_H

namespace Galaxy3D
{
	static FT_Library g_ft_lib;

	void Label::InitFontLib()
	{
		FT_Init_FreeType(&g_ft_lib);
	}

	void Label::DoneFontLib()
	{
		FT_Done_FreeType(g_ft_lib);
	}

	std::shared_ptr<Label> Label::Create(const std::string &text, bool rich)
	{
		std::shared_ptr<Label> label(new Label());
		label->m_rich = rich;
		label->SetText(text);

		return label;
	}

	Label::Label():
		m_rich(false)
	{
	}

	void Label::SetText(const std::string &text)
	{
		m_text = text;
	}
}

/*
auto font_tex = Texture2D::Create(2048, 2048, TextureFormat::Alpha8, FilterMode::Point, TextureWrapMode::Clamp);
int font_size = 24;

FT_Face face;
err = FT_New_Face( library,
            (Application::GetDataPath() + "/Assets/font/STHeiti-Light.ttc").c_str(),
            0,
            &face );
err = FT_Set_Pixel_Sizes(
	face,
	0,
	font_size);

FT_GlyphSlot  slot = face->glyph;
auto pen_x = 0;
auto pen_y = 0;
unsigned char str[] = {0xe9, 0xbe, 0x99, 0xe9, 0xbe, 0x8d, 0};
GTStringUTF32 text(std::string((char *) str) + "AVxyija");
			
auto use_kerning = FT_HAS_KERNING( face );
FT_UInt previous = 0;
int origin_y = face->bbox.yMax * font_size / face->units_per_EM;

for (int n = 0; n < text.Size(); n++ )
{
	auto glyph_index = FT_Get_Char_Index( face, text[n] );
	err = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);
	if ( err )
		continue;

	if ( use_kerning && previous && glyph_index )
	{
		FT_Vector delta;
		FT_Get_Kerning( face, previous, glyph_index, FT_KERNING_DEFAULT, &delta );
		pen_x += delta.x >> 6;
	}

	font_tex->SetPixels(
		pen_x + slot->bitmap_left,
		pen_y + origin_y - slot->bitmap_top,
		slot->bitmap.width,
		slot->bitmap.rows,
		(char *) slot->bitmap.buffer);
				
	pen_x += slot->advance.x >> 6;

	previous = glyph_index;
}

font_tex->EncodeToPNG(Application::GetDataPath() + "/Assets/font/test.png");
*/
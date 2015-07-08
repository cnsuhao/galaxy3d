#include "Label.h"
#include "GTStringUTF32.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <unordered_map>

struct CharInfo
{
	int c;
	int size;
	int uv_pixel_x;
	int uv_pixel_y;
	int uv_pixel_w;
	int uv_pixel_h;
	int bearing_x;
	int bearing_y;
	int advance_x;
	int advance_y;
};

namespace Galaxy3D
{
	static const int TEXTURE_SIZE_MAX = 2048;
	static FT_Library g_ft_lib;
	static std::unordered_map<std::string, void *> g_fonts;
	static std::unordered_map<std::string, std::unordered_map<int, std::unordered_map<int, CharInfo>>> g_chars;
	static std::shared_ptr<Texture2D> g_font_texture;
	static int g_texture_x;
	static int g_texture_y;
	static int g_texture_line_h_max;

	void Label::InitFontLib()
	{
		FT_Init_FreeType(&g_ft_lib);

		g_font_texture = Texture2D::Create(TEXTURE_SIZE_MAX, TEXTURE_SIZE_MAX, TextureFormat::Alpha8, FilterMode::Point, TextureWrapMode::Clamp);
		g_texture_x = 0;
		g_texture_y = 0;
		g_texture_line_h_max = 0;
	}

	void Label::DoneFontLib()
	{
		for(auto i : g_fonts)
		{
			FT_Done_Face((FT_Face) i.second);
		}

		FT_Done_FreeType(g_ft_lib);
	}

	std::shared_ptr<Texture2D> Label::GetFontTexture()
	{
		return g_font_texture;
	}

	void Label::LoadFont(const std::string &name, const std::string &file)
	{
		FT_Face face;
		auto err = FT_New_Face(g_ft_lib, file.c_str(), 0, &face);
		if(!err)
		{
			auto find = g_fonts.find(name);
			if(find != g_fonts.end())
			{
				FT_Done_Face((FT_Face) find->second);
			}

			g_fonts[name] = face;
		}
	}

	std::shared_ptr<Label> Label::Create(const std::string &text, const std::string &font, int font_size)
	{
		std::shared_ptr<Label> label;

		auto find = g_fonts.find(font);
		if(find != g_fonts.end())
		{
			label = std::shared_ptr<Label>(new Label());
			label->m_text = text;
			label->m_font = font;
			label->m_font_size = font_size;

			label->ProcessText();
		}

		return label;
	}

	Label::Label():
		m_font_size(20),
		m_pixels_per_unit(100),
		m_char_space(0),
		m_line_space(0)
	{
	}

	void Label::SetText(const std::string &text)
	{
		if(m_text != text)
		{
			m_text = text;
			ProcessText();
		}
	}

	void Label::SetCharSpace(int space)
	{
		if(m_char_space != space)
		{
			m_char_space = space;
			ProcessText();
		}
	}

	void Label::SetLineSpace(int space)
	{
		if(m_line_space != space)
		{
			m_line_space = space;
			ProcessText();
		}
	}

	static CharInfo get_char_info(const std::string &font, int c, int size)
	{
		auto find_font = g_chars.find(font);
		if(find_font != g_chars.end())
		{
			auto &fon = find_font->second;
			auto find_c = fon.find(c);
			if(find_c != fon.end())
			{
				auto &cha = find_c->second;
				auto find_s = cha.find(size);
				if(find_s != cha.end())
				{
					return find_s->second;
				}
			}
			else
			{
				std::unordered_map<int, CharInfo> cha;
				fon[c] = cha;
			}
		}
		else
		{
			std::unordered_map<int, std::unordered_map<int, CharInfo>> fon;
			std::unordered_map<int, CharInfo> cha;
			fon[c] = cha;
			g_chars[font] = fon;
		}

		CharInfo info;
		info.c = c;
		info.size = size;

		auto find_face = g_fonts.find(font);
		if(find_face != g_fonts.end())
		{
			FT_Face face = (FT_Face) find_face->second;
			FT_Set_Pixel_Sizes(face, 0, size);

			FT_GlyphSlot slot = face->glyph;
			auto glyph_index = FT_Get_Char_Index(face, c);
			FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);

			_ASSERT(slot->bitmap.width == slot->bitmap.pitch);

			info.uv_pixel_w = slot->bitmap.width;
			info.uv_pixel_h = slot->bitmap.rows;
			info.bearing_x = slot->bitmap_left;
			info.bearing_y = slot->bitmap_top;
			info.advance_x = slot->advance.x >> 6;
			info.advance_y = slot->advance.y >> 6;

			if(g_texture_y + info.uv_pixel_h <= TEXTURE_SIZE_MAX)
			{
				if(g_texture_x + info.uv_pixel_w > TEXTURE_SIZE_MAX)
				{
					g_texture_y += g_texture_line_h_max;
					g_texture_x = 0;
					g_texture_line_h_max = 0;
				}

				if(g_texture_line_h_max < info.uv_pixel_h)
				{
					g_texture_line_h_max = info.uv_pixel_h;
				}

				g_font_texture->SetPixels(
					g_texture_x,
					g_texture_y,
					info.uv_pixel_w,
					info.uv_pixel_h,
					(char *) slot->bitmap.buffer);
				info.uv_pixel_x = g_texture_x;
				info.uv_pixel_y = g_texture_y;

				g_texture_x += info.uv_pixel_w;
			}
			else
			{
				_ASSERT(g_texture_y + info.uv_pixel_h <= TEXTURE_SIZE_MAX);
			}
		}

		g_chars[font][c][size] = info;

		return info;
	}

	void Label::ProcessText()
	{
		FT_Face face = nullptr;
		auto find_face = g_fonts.find(m_font);
		if(find_face != g_fonts.end())
		{
			face = (FT_Face) find_face->second;
		}
		else
		{
			return;
		}

		m_vertices.clear();
		m_uv.clear();
		m_colors.clear();
		m_indices.clear();

		GTStringUTF32 str(m_text);
		int pen_x = 0;
		int pen_y = 0;
		int origin_y = face->bbox.yMax * m_font_size / face->units_per_EM;
		float v_ppu = 1 / m_pixels_per_unit;
		float v_size = 1.0f / TEXTURE_SIZE_MAX;
		int vertex_count = 0;

		for(int i=0; i<str.Size(); i++)
		{
			int c = str[i];

			if(c == '\n')
			{
				pen_x = 0;
				pen_y -= m_font_size + m_line_space;
				continue;
			}
			
			CharInfo info = get_char_info(m_font, c, m_font_size);

			int x0 = pen_x + info.bearing_x;
			int y0 = pen_y - origin_y + info.bearing_y;
			int x1 = x0 + info.uv_pixel_w;
			int y1 = y0 - info.uv_pixel_h;

			int uv_x0 = info.uv_pixel_x;
			int uv_y0 = info.uv_pixel_y;
			int uv_x1 = uv_x0 + info.uv_pixel_w;
			int uv_y1 = uv_y0 + info.uv_pixel_h;

			pen_x += info.advance_x + m_char_space;

			m_vertices.push_back(Vector2(x0 * v_ppu, y0 * v_ppu));
			m_vertices.push_back(Vector2(x0 * v_ppu, y1 * v_ppu));
			m_vertices.push_back(Vector2(x1 * v_ppu, y1 * v_ppu));
			m_vertices.push_back(Vector2(x1 * v_ppu, y0 * v_ppu));
			m_uv.push_back(Vector2(uv_x0 * v_size, uv_y0 * v_size));
			m_uv.push_back(Vector2(uv_x0 * v_size, uv_y1 * v_size));
			m_uv.push_back(Vector2(uv_x1 * v_size, uv_y1 * v_size));
			m_uv.push_back(Vector2(uv_x1 * v_size, uv_y0 * v_size));
			m_colors.push_back(Color(1, 1, 1, 1));
			m_colors.push_back(Color(1, 1, 1, 1));
			m_colors.push_back(Color(1, 1, 1, 1));
			m_colors.push_back(Color(1, 1, 1, 1));
			m_indices.push_back(vertex_count + 0);
			m_indices.push_back(vertex_count + 1);
			m_indices.push_back(vertex_count + 2);
			m_indices.push_back(vertex_count + 0);
			m_indices.push_back(vertex_count + 2);
			m_indices.push_back(vertex_count + 3);

			vertex_count += 4;
		}

		g_font_texture->Apply();
		//g_font_texture->EncodeToPNG(Application::GetDataPath() + "/Assets/font/test.png");
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
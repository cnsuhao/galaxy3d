#include "Label.h"
#include "GTStringUTF32.h"
#include "GTString.h"
#include "Mathf.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <unordered_map>

struct CharInfo
{
	int c;
	int size;
	unsigned int glyph_index;
	int uv_pixel_x;
	int uv_pixel_y;
	int uv_pixel_w;
	int uv_pixel_h;
	int bearing_x;
	int bearing_y;
	int advance_x;
	int advance_y;
};

struct TagInfo
{
	std::string tag;
	std::string value;
	int begin;
	int end;
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
	static std::unordered_map<std::string, std::vector<std::shared_ptr<Texture2D>>> g_rich_images;

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

	void Label::LoadImage(const std::string &name, const std::string &file)
	{
		std::vector<const std::string> files;
		files.push_back(file);
		LoadImages(name, files);
	}

	void Label::LoadImages(const std::string &name, const std::vector<const std::string> &files)
	{
		std::vector<std::shared_ptr<Texture2D>> textures;
		for(size_t i=0; i<files.size(); i++)
		{
			auto tex = Texture2D::LoadImageFile(files[i], FilterMode::Point, TextureWrapMode::Clamp);
			textures.push_back(tex);
		}

		g_rich_images[name] = textures;
	}

	std::shared_ptr<Texture2D> Label::GetRichImageTexture(const std::string &name, int index)
	{
		std::shared_ptr<Texture2D> tex;

		auto find = g_rich_images.find(name);
		if(find != g_rich_images.end())
		{
			tex = find->second[index];
		}

		return tex;
	}

	std::shared_ptr<Label> Label::Create(const std::string &text, const std::string &font, int font_size, bool rich)
	{
		std::shared_ptr<Label> label;

		auto find = g_fonts.find(font);
		if(find != g_fonts.end())
		{
			label = std::shared_ptr<Label>(new Label());
			label->m_text = text;
			label->m_font = font;
			label->m_font_size = font_size;
			label->m_rich = rich;

			label->ProcessText();
		}

		return label;
	}

	Label::Label():
		m_font_size(20),
		m_pixels_per_unit(100),
		m_char_space(0),
		m_line_space(0),
		m_color(1, 1, 1, 1),
		m_width(0x7fffffff),
		m_height(0x7fffffff)
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

			info.glyph_index = glyph_index;
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

	static std::vector<TagInfo> parse_rich_tags(GTStringUTF32 &str)
	{
		std::vector<TagInfo> tags;

		std::vector<TagInfo> tag_find;
		for(int i=0; i<str.Size(); i++)
		{
			if(	str[i+0] == '<' &&
				str[i+1] == 'c' &&
				str[i+2] == 'o' &&
				str[i+3] == 'l' &&
				str[i+4] == 'o' &&
				str[i+5] == 'r' &&
				str[i+6] == '=' &&
				str[i+7] == '#')
			{
				auto value = str.Substr(i + 8, 8);

				TagInfo info;
				info.tag = "color";
				info.value = value.Utf8();
				info.begin = i;

				tag_find.push_back(info);

				str.Erase(i, 17);
				i--;
			}
			else if(
				str[i+0] == '<' &&
				str[i+1] == '/' &&
				str[i+2] == 'c' &&
				str[i+3] == 'o' &&
				str[i+4] == 'l' &&
				str[i+5] == 'o' &&
				str[i+6] == 'r' &&
				str[i+7] == '>')
			{
				for(int j=tag_find.size()-1; j>=0; j--)
				{
					auto &t = tag_find[j];
					if(t.tag == "color")
					{
						t.end = i;
						tags.push_back(t);
						tag_find.erase(tag_find.begin() + j);
						break;
					}
				}

				str.Erase(i, 8);
				i--;
			}
			else if(
				str[i+0] == '<' &&
				str[i+1] == 's' &&
				str[i+2] == 'h' &&
				str[i+3] == 'a' &&
				str[i+4] == 'd' &&
				str[i+5] == 'o' &&
				str[i+6] == 'w' &&
				str[i+7] == '=' &&
				str[i+8] == '#')
			{
				auto value = str.Substr(i + 9, 8);

				TagInfo info;
				info.tag = "shadow";
				info.value = value.Utf8();
				info.begin = i;

				tag_find.push_back(info);

				str.Erase(i, 18);
				i--;
			}
			else if(
				str[i+0] == '<' &&
				str[i+1] == 's' &&
				str[i+2] == 'h' &&
				str[i+3] == 'a' &&
				str[i+4] == 'd' &&
				str[i+5] == 'o' &&
				str[i+6] == 'w' &&
				str[i+7] == '>')
			{
				TagInfo info;
				info.tag = "shadow";
				info.value = "000000ff";
				info.begin = i;

				tag_find.push_back(info);

				str.Erase(i, 8);
				i--;
			}
			else if(
				str[i+0] == '<' &&
				str[i+1] == '/' &&
				str[i+2] == 's' &&
				str[i+3] == 'h' &&
				str[i+4] == 'a' &&
				str[i+5] == 'd' &&
				str[i+6] == 'o' &&
				str[i+7] == 'w' &&
				str[i+8] == '>')
			{
				for(int j=tag_find.size()-1; j>=0; j--)
				{
					auto &t = tag_find[j];
					if(t.tag == "shadow")
					{
						t.end = i;
						tags.push_back(t);
						tag_find.erase(tag_find.begin() + j);
						break;
					}
				}

				str.Erase(i, 9);
				i--;
			}
			else if(
				str[i+0] == '<' &&
				str[i+1] == 'o' &&
				str[i+2] == 'u' &&
				str[i+3] == 't' &&
				str[i+4] == 'l' &&
				str[i+5] == 'i' &&
				str[i+6] == 'n' &&
				str[i+7] == 'e' &&
				str[i+8] == '=' &&
				str[i+9] == '#')
			{
				auto value = str.Substr(i + 10, 8);

				TagInfo info;
				info.tag = "outline";
				info.value = value.Utf8();
				info.begin = i;

				tag_find.push_back(info);

				str.Erase(i, 19);
				i--;
			}
			else if(
				str[i+0] == '<' &&
				str[i+1] == 'o' &&
				str[i+2] == 'u' &&
				str[i+3] == 't' &&
				str[i+4] == 'l' &&
				str[i+5] == 'i' &&
				str[i+6] == 'n' &&
				str[i+7] == 'e' &&
				str[i+8] == '>')
			{
				TagInfo info;
				info.tag = "outline";
				info.value = "000000ff";
				info.begin = i;

				tag_find.push_back(info);

				str.Erase(i, 9);
				i--;
			}
			else if(
				str[i+0] == '<' &&
				str[i+1] == '/' &&
				str[i+2] == 'o' &&
				str[i+3] == 'u' &&
				str[i+4] == 't' &&
				str[i+5] == 'l' &&
				str[i+6] == 'i' &&
				str[i+7] == 'n' &&
				str[i+8] == 'e' &&
				str[i+9] == '>')
			{
				for(int j=tag_find.size()-1; j>=0; j--)
				{
					auto &t = tag_find[j];
					if(t.tag == "outline")
					{
						t.end = i;
						tags.push_back(t);
						tag_find.erase(tag_find.begin() + j);
						break;
					}
				}

				str.Erase(i, 10);
				i--;
			}
			else if(
				str[i+0] == '<' &&
				str[i+1] == 's' &&
				str[i+2] == 'i' &&
				str[i+3] == 'z' &&
				str[i+4] == 'e' &&
				str[i+5] == '=')
			{
				int value_size = str.Find('>', i+5) - (i+5) - 1;
				auto value = str.Substr(i + 6, value_size);

				TagInfo info;
				info.tag = "size";
				info.value = value.Utf8();
				info.begin = i;

				tag_find.push_back(info);

				str.Erase(i, 6 + value_size + 1);
				i--;
			}
			else if(
				str[i+0] == '<' &&
				str[i+1] == '/' &&
				str[i+2] == 's' &&
				str[i+3] == 'i' &&
				str[i+4] == 'z' &&
				str[i+5] == 'e' &&
				str[i+6] == '>')
			{
				for(int j=tag_find.size()-1; j>=0; j--)
				{
					auto &t = tag_find[j];
					if(t.tag == "size")
					{
						t.end = i;
						tags.push_back(t);
						tag_find.erase(tag_find.begin() + j);
						break;
					}
				}

				str.Erase(i, 7);
				i--;
			}
			else if(
				str[i+0] == '<' &&
				str[i+1] == 'f' &&
				str[i+2] == 'o' &&
				str[i+3] == 'n' &&
				str[i+4] == 't' &&
				str[i+5] == '=')
			{
				int value_size = str.Find('>', i+5) - (i+5) - 1;
				auto value = str.Substr(i + 6, value_size);

				TagInfo info;
				info.tag = "font";
				info.value = value.Utf8();
				info.begin = i;

				tag_find.push_back(info);

				str.Erase(i, 6 + value_size + 1);
				i--;
			}
			else if(
				str[i+0] == '<' &&
				str[i+1] == '/' &&
				str[i+2] == 'f' &&
				str[i+3] == 'o' &&
				str[i+4] == 'n' &&
				str[i+5] == 't' &&
				str[i+6] == '>')
			{
				for(int j=tag_find.size()-1; j>=0; j--)
				{
					auto &t = tag_find[j];
					if(t.tag == "font")
					{
						t.end = i;
						tags.push_back(t);
						tag_find.erase(tag_find.begin() + j);
						break;
					}
				}

				str.Erase(i, 7);
				i--;
			}
			else if(
				str[i+0] == '<' &&
				str[i+1] == 'i' &&
				str[i+2] == 'm' &&
				str[i+3] == 'a' &&
				str[i+4] == 'g' &&
				str[i+5] == 'e' &&
				str[i+6] == '=')
			{
				int value_size = str.Find('>', i+6) - (i+6) - 1;
				auto value = str.Substr(i + 7, value_size);

				TagInfo info;
				info.tag = "image";
				info.value = value.Utf8();
				info.begin = i;

				tag_find.push_back(info);

				str.Erase(i, 7 + value_size + 1);
				i--;
			}
			else if(
				str[i+0] == '<' &&
				str[i+1] == '/' &&
				str[i+2] == 'i' &&
				str[i+3] == 'm' &&
				str[i+4] == 'a' &&
				str[i+5] == 'g' &&
				str[i+6] == 'e' &&
				str[i+7] == '>')
			{
				for(int j=tag_find.size()-1; j>=0; j--)
				{
					auto &t = tag_find[j];
					if(t.tag == "image")
					{
						t.end = i;
						tags.push_back(t);
						tag_find.erase(tag_find.begin() + j);
						break;
					}
				}

				str.Erase(i, 8);
				i--;
			}
		}

		return tags;
	}

	void Label::ProcessText()
	{
		FT_Face face = nullptr;
		FT_Face face_old = nullptr;
		auto find_face = g_fonts.find(m_font);
		if(find_face != g_fonts.end())
		{
			face = (FT_Face) find_face->second;
		}
		else
		{
			return;
		}

		m_image_items.clear();
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

		std::vector<TagInfo> tags;
		if(m_rich)
		{
			tags = parse_rich_tags(str);
		}

		auto has_kerning = FT_HAS_KERNING(face);
		FT_UInt previous = 0;
		int font_size = m_font_size;
		std::string font = m_font;

		for(int i=0; i<str.Size(); i++)
		{
			int c = str[i];
			Color color(1, 1, 1, 1);
			bool shadow = false;
			Color color_shadow(0, 0, 0, 1);
			bool outline = false;
			Color color_outline(0, 0, 0, 1);
			int origin = origin_y;

			if(c == '\n')
			{
				pen_x = 0;
				pen_y += -(font_size + m_line_space);
				continue;
			}

			if(m_rich)
			{
				TagInfo *tag_color = 0;
				TagInfo *tag_shadow = 0;
				TagInfo *tag_outline = 0;
				TagInfo *tag_size = 0;
				TagInfo *tag_font = 0;
				int begin_max;

				//	color
				begin_max = 0;
				for(int j=0; j<(int) tags.size(); j++)
				{
					auto &t = tags[j];
					if(t.tag == "color" && i >= t.begin && i < t.end)
					{
						if(begin_max < t.begin)
						{
							tag_color = &t;
							begin_max = t.begin;
						}
					}
				}

				if(tag_color != 0)
				{
					auto &t = *tag_color;
					
					GTString v(t.value);
					v = v.ToLower();
					std::stringstream ss;
					ss << std::hex << v.str;
					unsigned int color_i = 0; 
					ss >> color_i;

					int r = (color_i & 0xff000000) >> 24;
					int g = (color_i & 0xff0000) >> 16;
					int b = (color_i & 0xff00) >> 8;
					int a = (color_i & 0xff);

					float div = 1/255.f;
					color = Color((float) r, (float) g, (float) b, (float) a) * div;
				}

				//	shadow
				begin_max = -1;
				for(int j=0; j<(int) tags.size(); j++)
				{
					auto &t = tags[j];
					if(t.tag == "shadow" && i >= t.begin && i < t.end)
					{
						if(begin_max < t.begin)
						{
							tag_shadow = &t;
							begin_max = t.begin;
						}
					}
				}

				if(tag_shadow != 0)
				{
					auto &t = *tag_shadow;
					
					GTString v(t.value);
					v = v.ToLower();
					std::stringstream ss;
					ss << std::hex << v.str;
					unsigned int color_i = 0; 
					ss >> color_i;

					int r = (color_i & 0xff000000) >> 24;
					int g = (color_i & 0xff0000) >> 16;
					int b = (color_i & 0xff00) >> 8;
					int a = (color_i & 0xff);

					float div = 1/255.f;
					color_shadow = Color((float) r, (float) g, (float) b, (float) a) * div;
					shadow = true;
				}

				//	outline
				begin_max = -1;
				for(int j=0; j<(int) tags.size(); j++)
				{
					auto &t = tags[j];
					if(t.tag == "outline" && i >= t.begin && i < t.end)
					{
						if(begin_max < t.begin)
						{
							tag_outline = &t;
							begin_max = t.begin;
						}
					}
				}

				if(tag_outline != 0)
				{
					auto &t = *tag_outline;
					
					GTString v(t.value);
					v = v.ToLower();
					std::stringstream ss;
					ss << std::hex << v.str;
					unsigned int color_i = 0; 
					ss >> color_i;

					int r = (color_i & 0xff000000) >> 24;
					int g = (color_i & 0xff0000) >> 16;
					int b = (color_i & 0xff00) >> 8;
					int a = (color_i & 0xff);

					float div = 1/255.f;
					color_outline = Color((float) r, (float) g, (float) b, (float) a) * div;
					outline = true;
				}

				//	size
				begin_max = -1;
				for(int j=0; j<(int) tags.size(); j++)
				{
					auto &t = tags[j];
					if(t.tag == "size" && i >= t.begin && i < t.end)
					{
						if(begin_max < t.begin)
						{
							tag_size = &t;
							begin_max = t.begin;
						}
					}
				}

				if(tag_size != 0)
				{
					auto &t = *tag_size;
					
					GTString v(t.value);
					std::stringstream ss;
					ss << v.str;
					int size;
					ss >> size;

					if(font_size != size)
					{
						font_size = size;
						previous = 0;
					}
				}
				else
				{
					if(font_size != m_font_size)
					{
						font_size = m_font_size;
						previous = 0;
					}
				}

				//	font
				begin_max = -1;
				for(int j=0; j<(int) tags.size(); j++)
				{
					auto &t = tags[j];
					if(t.tag == "font" && i >= t.begin && i < t.end)
					{
						if(begin_max < t.begin)
						{
							tag_font = &t;
							begin_max = t.begin;
						}
					}
				}

				if(tag_font != 0)
				{
					auto &t = *tag_font;

					if(font != t.value)
					{
						font = t.value;
						previous = 0;
					}
				}
				else
				{
					if(font != m_font)
					{
						font = m_font;
						previous = 0;
					}
				}

				if(font_size != m_font_size)
				{
					origin = face->bbox.yMax * font_size / face->units_per_EM;
				}

				if(font != m_font)
				{
					auto find = g_fonts.find(font);
					if(find != g_fonts.end())
					{
						face_old = face;
						face = (FT_Face) find->second;
					}
					else
					{
						font = m_font;
					}

					origin = face->bbox.yMax * font_size / face->units_per_EM;
				}
				else
				{
					if(face_old != nullptr)
					{
						face = face_old;
						face_old = nullptr;
					}
				}
			}
			
			CharInfo info = get_char_info(font, c, font_size);

			//limit width
			if(pen_x + info.bearing_x + info.uv_pixel_w > m_width)
			{
				pen_x = 0;
				pen_y += -(font_size + m_line_space);
				previous = 0;
			}

			//kerning
			if(has_kerning && previous && info.glyph_index)
			{
				FT_Vector delta;
				FT_Get_Kerning(face, previous, info.glyph_index, FT_KERNING_UNFITTED, &delta);
				pen_x += delta.x >> 6;
			}

			int x0 = pen_x + info.bearing_x;
			int y0 = pen_y - origin + info.bearing_y;
			int x1 = x0 + info.uv_pixel_w;
			int y1 = y0 - info.uv_pixel_h;

			int uv_x0 = info.uv_pixel_x;
			int uv_y0 = info.uv_pixel_y;
			int uv_x1 = uv_x0 + info.uv_pixel_w;
			int uv_y1 = uv_y0 + info.uv_pixel_h;

			pen_x += info.advance_x + m_char_space;

			if(m_rich)
			{
				//	image
				for(int j=0; j<(int) tags.size(); j++)
				{
					auto &t = tags[j];
					if(t.tag == "image" && i + 1 == t.begin)
					{
						LabelImageItem img;
						img.name = t.value;
						
						auto find = g_rich_images.find(img.name);
						if(find != g_rich_images.end())
						{
							auto tex = find->second.front();

							int w = tex->GetWidth();
							int h = tex->GetHeight();

							img.image_index = 0;
							img.image_count = find->second.size();
							
							int ix0 = pen_x;
							int iy0 = pen_y - origin + h;
							int ix1 = ix0 + w;
							int iy1 = iy0 - h;
							img.vertices.push_back(Vector2(ix0 * v_ppu, iy0 * v_ppu));
							img.vertices.push_back(Vector2(ix0 * v_ppu, iy1 * v_ppu));
							img.vertices.push_back(Vector2(ix1 * v_ppu, iy1 * v_ppu));
							img.vertices.push_back(Vector2(ix1 * v_ppu, iy0 * v_ppu));
							img.uv.push_back(Vector2(0, 0));
							img.uv.push_back(Vector2(0, 1));
							img.uv.push_back(Vector2(1, 1));
							img.uv.push_back(Vector2(1, 0));
							img.colors.push_back(color);
							img.colors.push_back(color);
							img.colors.push_back(color);
							img.colors.push_back(color);
							img.indices.push_back(0);
							img.indices.push_back(1);
							img.indices.push_back(2);
							img.indices.push_back(0);
							img.indices.push_back(2);
							img.indices.push_back(3);

							m_image_items.push_back(img);

							pen_x += w + m_char_space;
						}
					}
				}

				if(shadow)
				{
					Vector2 offset = Vector2(1, -1) * v_ppu;

					m_vertices.push_back(Vector2(x0 * v_ppu, y0 * v_ppu) + offset);
					m_vertices.push_back(Vector2(x0 * v_ppu, y1 * v_ppu) + offset);
					m_vertices.push_back(Vector2(x1 * v_ppu, y1 * v_ppu) + offset);
					m_vertices.push_back(Vector2(x1 * v_ppu, y0 * v_ppu) + offset);
					m_uv.push_back(Vector2(uv_x0 * v_size, uv_y0 * v_size));
					m_uv.push_back(Vector2(uv_x0 * v_size, uv_y1 * v_size));
					m_uv.push_back(Vector2(uv_x1 * v_size, uv_y1 * v_size));
					m_uv.push_back(Vector2(uv_x1 * v_size, uv_y0 * v_size));
					m_colors.push_back(color_shadow);
					m_colors.push_back(color_shadow);
					m_colors.push_back(color_shadow);
					m_colors.push_back(color_shadow);
					m_indices.push_back(vertex_count + 0);
					m_indices.push_back(vertex_count + 1);
					m_indices.push_back(vertex_count + 2);
					m_indices.push_back(vertex_count + 0);
					m_indices.push_back(vertex_count + 2);
					m_indices.push_back(vertex_count + 3);

					vertex_count += 4;
				}

				if(outline)
				{
					Vector2 offsets[4];
					offsets[0] = Vector2(-1, 1) * v_ppu;
					offsets[1] = Vector2(-1, -1) * v_ppu;
					offsets[2] = Vector2(1, -1) * v_ppu;
					offsets[3] = Vector2(1, 1) * v_ppu;

					for(int j=0; j<4; j++)
					{
						Vector2 offset = offsets[j];

						m_vertices.push_back(Vector2(x0 * v_ppu, y0 * v_ppu) + offset);
						m_vertices.push_back(Vector2(x0 * v_ppu, y1 * v_ppu) + offset);
						m_vertices.push_back(Vector2(x1 * v_ppu, y1 * v_ppu) + offset);
						m_vertices.push_back(Vector2(x1 * v_ppu, y0 * v_ppu) + offset);
						m_uv.push_back(Vector2(uv_x0 * v_size, uv_y0 * v_size));
						m_uv.push_back(Vector2(uv_x0 * v_size, uv_y1 * v_size));
						m_uv.push_back(Vector2(uv_x1 * v_size, uv_y1 * v_size));
						m_uv.push_back(Vector2(uv_x1 * v_size, uv_y0 * v_size));
						m_colors.push_back(color_outline);
						m_colors.push_back(color_outline);
						m_colors.push_back(color_outline);
						m_colors.push_back(color_outline);
						m_indices.push_back(vertex_count + 0);
						m_indices.push_back(vertex_count + 1);
						m_indices.push_back(vertex_count + 2);
						m_indices.push_back(vertex_count + 0);
						m_indices.push_back(vertex_count + 2);
						m_indices.push_back(vertex_count + 3);

						vertex_count += 4;
					}
				}
			}

			m_vertices.push_back(Vector2(x0 * v_ppu, y0 * v_ppu));
			m_vertices.push_back(Vector2(x0 * v_ppu, y1 * v_ppu));
			m_vertices.push_back(Vector2(x1 * v_ppu, y1 * v_ppu));
			m_vertices.push_back(Vector2(x1 * v_ppu, y0 * v_ppu));
			m_uv.push_back(Vector2(uv_x0 * v_size, uv_y0 * v_size));
			m_uv.push_back(Vector2(uv_x0 * v_size, uv_y1 * v_size));
			m_uv.push_back(Vector2(uv_x1 * v_size, uv_y1 * v_size));
			m_uv.push_back(Vector2(uv_x1 * v_size, uv_y0 * v_size));
			m_colors.push_back(color);
			m_colors.push_back(color);
			m_colors.push_back(color);
			m_colors.push_back(color);
			m_indices.push_back(vertex_count + 0);
			m_indices.push_back(vertex_count + 1);
			m_indices.push_back(vertex_count + 2);
			m_indices.push_back(vertex_count + 0);
			m_indices.push_back(vertex_count + 2);
			m_indices.push_back(vertex_count + 3);

			vertex_count += 4;
			previous = info.glyph_index;
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
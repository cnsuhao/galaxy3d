#ifndef __Label_h__
#define __Label_h__

#include "Object.h"
#include "Vector2.h"
#include "Color.h"
#include "Texture2D.h"
#include <vector>

namespace Galaxy3D
{
	class Label : public Object
	{
	public:
		static void InitFontLib();
		static void DoneFontLib();
		static std::shared_ptr<Texture2D> GetFontTexture();
		static void LoadFont(const std::string &name, const std::string &file);
		static std::shared_ptr<Label> Create(const std::string &text, const std::string &font, int font_size);
		void SetText(const std::string &text);
		std::string GetText() const {return m_text;}
		void SetCharSpace(int space);
		void SetLineSpace(int space);
		const std::vector<Vector2> &GetVertices() const {return m_vertices;}
		const std::vector<Vector2> &GetUV() const {return m_uv;}
		const std::vector<Color> &GetColors() const {return m_colors;}
		const std::vector<unsigned short> &GetIndices() const {return m_indices;}

	private:
		std::string m_text;
		std::string m_font;
		int m_font_size;
		int m_char_space;
		int m_line_space;
		float m_pixels_per_unit;
		std::vector<Vector2> m_vertices;
		std::vector<Vector2> m_uv;
		std::vector<Color> m_colors;
		std::vector<unsigned short> m_indices;

		Label();
		virtual void ProcessText();
	};
}

#endif
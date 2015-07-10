#ifndef __Label_h__
#define __Label_h__

#include "Object.h"
#include "Vector2.h"
#include "Color.h"
#include "Texture2D.h"
#include <vector>

#ifdef LoadImage
#undef LoadImage
#endif

namespace Galaxy3D
{
	struct LabelPivot
	{
		enum Enum
		{
			LeftTop,
			Left,
			LeftBottom,
			Top,
			Center,
			Bottom,
			RightTop,
			Right,
			RightBottom
		};
	};

	struct LabelImageItem
	{
		std::string name;
		int image_index;
		int image_count;
		int text_index;
		int line_index;
		std::vector<Vector2> vertices;
		std::vector<Vector2> uv;
		std::vector<Color> colors;
		std::vector<unsigned short> indices;
	};

	//	Supported rich text tags
	//
	//	<color=#ffffffff></color>
	//	<shadow></shadow>
	//	<shadow=#000000ff></shadow>
	//	<outline></outline>
	//	<outline=#000000ff></outline>
	//	<size=30></size>
	//	<font=msyh></font>
	//	<image=cool></image>;图片使用0xffffffff占一个字符位
	class Label : public Object
	{
	public:
		static void InitFontLib();
		static void DoneFontLib();
		static std::shared_ptr<Texture2D> GetFontTexture();
		static void LoadFont(const std::string &name, const std::string &file);
		static void LoadImage(const std::string &name, const std::string &file);
		static void LoadImages(const std::string &name, const std::vector<const std::string> &files);
		static std::shared_ptr<Texture2D> GetRichImageTexture(const std::string &name, int index);
		static std::shared_ptr<Label> Create(const std::string &text, const std::string &font, int font_size, LabelPivot::Enum pivot = LabelPivot::LeftTop, bool rich = false);
		void SetText(const std::string &text);
		std::string GetText() const {return m_text;}
		void SetCharSpace(int space);
		void SetLineSpace(int space);
		void SetColor(const Color &color) {m_color = color;}
		Color GetColor() const {return m_color;}
		void SetWidth(int width) {m_width = width;}
		int GetWidth() const {return m_width;}
		void SetHeight(int height) {m_height = height;}
		int GetHeight() const {return m_height;}
		int GetWidthActual() const {return m_width_actual;}
		int GetHeightActual() const {return m_height_actual;}
		LabelPivot::Enum GetPivot() const {return m_pivot;}
		float GetPixelsPerUnit() const {return m_pixels_per_unit;}
		std::vector<LabelImageItem> &GetImageItems() {return m_image_items;}
		const std::vector<Vector2> &GetVertices() const {return m_vertices;}
		const std::vector<Vector2> &GetUV() const {return m_uv;}
		const std::vector<Color> &GetColors() const {return m_colors;}
		const std::vector<unsigned short> &GetIndices() const {return m_indices;}

	private:
		std::string m_text;
		std::string m_font;
		int m_font_size;
		float m_pixels_per_unit;
		int m_char_space;
		int m_line_space;
		Color m_color;
		int m_width;
		int m_height;
		int m_width_actual;
		int m_height_actual;
		bool m_rich;
		LabelPivot::Enum m_pivot;
		std::vector<LabelImageItem> m_image_items;
		std::vector<Vector2> m_vertices;
		std::vector<Vector2> m_uv;
		std::vector<Color> m_colors;
		std::vector<unsigned short> m_indices;
		std::vector<int> m_heights;

		Label();
		virtual void ProcessText();
	};
}

#endif
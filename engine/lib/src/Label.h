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
            TopLeft,
            Top,
            TopRight,
            Left,
            Center,
            Right,
            BottomLeft,
            Bottom,
            BottomRight,
		};
	};

	struct LabelAlign
	{
		enum Enum
		{
			Auto,
			Left,
			Center,
			Right,
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

	struct LabelLine
	{
		int width;
		int height;
		std::vector<Vector2> vertices;
		std::vector<Vector2> uv;
		std::vector<Color> colors;
		std::vector<unsigned short> indices;
		std::vector<LabelImageItem> image_items;
		std::vector<int> heights;
	};

	//	Supported rich text tags
	//
	//	<color=#ffffffff></color>
	//	<shadow></shadow>
	//	<shadow=#000000ff></shadow>
	//	<outline></outline>
	//	<outline=#000000ff></outline>
	//	<underline></underline>
	//	<size=30></size>
	//	<font=msyh></font>
    //  <bold></bold>，粗体
    //  <italic></italic>，斜体
	//	<image=cool></image>，图片使用0xffffffff占一个字符位
	class Label : public Object
	{
	public:
		static void InitFontLib();
		static void DoneFontLib();
		static std::shared_ptr<Texture2D> GetFontTexture();
		static void LoadFont(const std::string &name, const std::string &file);
		static void LoadImage(const std::string &name, const std::string &file);
		static void LoadImages(const std::string &name, const std::vector<std::string> &files);
		static std::shared_ptr<Texture2D> GetRichImageTexture(const std::string &name, int index);
		static int FillVertexBuffer(char *buffer, const std::shared_ptr<Label> &label, Matrix4x4 *mat);
		static int FillVertexBuffer(char *buffer, LabelImageItem &item, const std::shared_ptr<Label> &label, const LabelLine &line);
		static int FillIndexBuffer(char *buffer, const std::shared_ptr<Label> &label);
		static std::shared_ptr<Label> Create(const std::string &text, const std::string &font, int font_size, LabelPivot::Enum pivot = LabelPivot::TopLeft, LabelAlign::Enum align = LabelAlign::Auto, bool rich = false);
		void SetText(const std::string &text);
		std::string GetText() const {return m_text;}
		void SetCharSpace(int space);
		void SetLineSpace(int space);
		void SetWidth(int width);
		int GetWidth() const {return m_width;}
		void SetHeight(int height);
		int GetHeight() const {return m_height;}
		int GetWidthActual() const {return m_width_actual;}
		int GetHeightActual() const {return m_height_actual;}
        int GetOffsetY() const {return m_offset_y;}
		LabelPivot::Enum GetPivot() const {return m_pivot;}
		LabelAlign::Enum GetAlign() const {return m_align;}
		float GetPixelsPerUnit() const {return m_pixels_per_unit;}
		std::vector<LabelLine> &GetLines() {return m_lines;}
		int GetVertexCount() const {return m_vertex_count;}
		int GetIndexCount() const {return m_vertex_count / 4 * 6;}
		int GetImageCount() const {return m_image_count;}
        bool IsDirty() const {return m_dirty;}
        void SetDirty(bool dirty) {m_dirty = dirty;}

	private:
		std::string m_text;
		std::string m_font;
		int m_font_size;
		float m_pixels_per_unit;
		int m_char_space;
		int m_line_space;
		int m_width;
		int m_height;
		int m_width_actual;
		int m_height_actual;
        int m_offset_y;
		bool m_rich;
		LabelPivot::Enum m_pivot;
		LabelAlign::Enum m_align;
		std::vector<LabelLine> m_lines;
		int m_vertex_count;
		int m_image_count;
        bool m_dirty;

		Label();
		virtual void ProcessText();
	};
}

#endif
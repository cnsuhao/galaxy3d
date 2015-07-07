#ifndef __Launcher_h__
#define __Launcher_h__

#include "Component.h"
#include "Application.h"
#include "Texture2D.h"
#include "Screen.h"

#include "SpriteNode.h"
#include "SpriteBatchRenderer.h"
#include "SpriteRenderer.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include "GTStringUTF32.h"

namespace Galaxy3D
{
	class Launcher :public Component
	{
	public:
		virtual ~Launcher()
		{
			FT_Done_FreeType(library);
		}

	protected:
		std::shared_ptr<SpriteNode> m_node;
		std::shared_ptr<Sprite> sps[20];
		std::shared_ptr<SpriteBatchRenderer> m_renderer;
		float index;

		FT_Library  library;

		virtual void Start()
		{
			auto font_tex = Texture2D::Create(2048, 2048, TextureFormat::Alpha8, FilterMode::Point, TextureWrapMode::Clamp);
			int font_size = 24;

			FT_Error err;
			err = FT_Init_FreeType( &library );

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




			index = 0;

			auto cam = GameObject::Create("camera")->AddComponent<Camera>();
			cam->SetOrthographicSize(Screen::GetHeight() / 200.f);
			auto bg = Sprite::Create(Application::GetDataPath() + "/Assets/texture/mustang.jpg");
			auto sr = GameObject::Create("renderer")->AddComponent<SpriteRenderer>();
			sr->SetSprite(bg);

			auto tex = Texture2D::LoadImageFile(Application::GetDataPath() + "/Assets/texture/test.png");

			float h = 384.f;
			for(int i=0; i<5; i++)
			{
				for(int j=0; j<4; j++)
				{
					sps[i*4 + j] = Sprite::Create(tex, Rect(192.f*i, h*j, 192, h), Vector2(96, h), 100, Vector4());
				}
			}

			auto renderer = GameObject::Create("renderer")->AddComponent<SpriteBatchRenderer>();
			renderer->SetSortingOrder(1);
			m_renderer = renderer;

			auto node = GameObject::Create("sprite")->AddComponent<SpriteNode>();
			node->GetTransform()->SetPosition(Vector3(-3, -2.f, 0));
			node->SetSprite(sps[0]);
			renderer->AddSprite(node);
			m_node = node;

			node = GameObject::Create("sprite")->AddComponent<SpriteNode>();
			node->GetTransform()->SetPosition(Vector3(-2, -2.f, 0));
			node->SetSprite(sps[1]);
			renderer->AddSprite(node);

			node = GameObject::Create("sprite")->AddComponent<SpriteNode>();
			node->GetTransform()->SetPosition(Vector3(-1, -2.f, 0));
			node->SetSprite(sps[2]);
			renderer->AddSprite(node);

			node = GameObject::Create("sprite")->AddComponent<SpriteNode>();
			node->GetTransform()->SetPosition(Vector3(0, -2.f, 0));
			node->SetSprite(sps[3]);
			renderer->AddSprite(node);

			node = GameObject::Create("sprite")->AddComponent<SpriteNode>();
			node->GetTransform()->SetPosition(Vector3(1, -2.f, 0));
			node->SetSprite(sps[4]);
			renderer->AddSprite(node);

			node = GameObject::Create("sprite")->AddComponent<SpriteNode>();
			node->GetTransform()->SetPosition(Vector3(2, -2.f, 0));
			node->SetSprite(sps[5]);
			renderer->AddSprite(node);

			node = GameObject::Create("sprite")->AddComponent<SpriteNode>();
			node->GetTransform()->SetPosition(Vector3(3, -2.f, 0));
			node->SetSprite(sps[6]);
			renderer->AddSprite(node);

			node = GameObject::Create("sprite")->AddComponent<SpriteNode>();
			node->GetTransform()->SetPosition(Vector3(4, -2.f, 0));
			node->SetSprite(sps[7]);
			renderer->AddSprite(node);
		}

		virtual void Update()
		{
			index += 0.07f;
			if(index > 19)
			{
				index = 0;
			}
			m_node->SetSprite(sps[(int) index]);
			m_renderer->UpdateSprites();
		}

	private:
	};
}

#endif
#ifndef __Launcher_h__
#define __Launcher_h__

#include "Component.h"
#include "Application.h"
#include "Texture2D.h"
#include "Screen.h"

#include "SpriteRenderer.h"
#include "SpriteNode.h"
#include "SpriteBatchRenderer.h"
#include "Label.h"
#include "TextRenderer.h"
#include "GTString.h"
#include "GTTime.h"

namespace Galaxy3D
{
	class Launcher :public Component
	{
	public:
		virtual ~Launcher()
		{
		}

	protected:
		std::shared_ptr<SpriteNode> m_node;
		std::shared_ptr<Sprite> sps[20];
		std::shared_ptr<SpriteBatchRenderer> m_renderer;
		std::shared_ptr<TextRenderer> m_fps;
		float index;

		virtual void Start()
		{
			Label::LoadFont("consola", Application::GetDataPath() + "/Assets/font/consola.ttf");
			unsigned char str[] = {0xe9, 0xbe, 0x99, 0xe9, 0xbe, 0x8d, 0};
			auto label = Label::Create(std::string((char *) str) + "AAV\nxy ija", "consola", 17);
			label->SetColor(Color(0, 0, 1, 1));
			auto tr = GameObject::Create("label")->AddComponent<TextRenderer>();
			tr->GetTransform()->SetPosition(Vector3(-4.8f, 3.2f, 0));
			tr->SetLabel(label);
			tr->SetSortingOrder(2);
			tr->UpdateLabel();
			m_fps = tr;

			auto cam = GameObject::Create("camera")->AddComponent<Camera>();
			cam->SetOrthographicSize(Screen::GetHeight() / 200.f);
			
			auto bg = Sprite::Create(Application::GetDataPath() + "/Assets/texture/mustang.jpg");
			auto sr = GameObject::Create("renderer")->AddComponent<SpriteRenderer>();
			sr->SetSprite(bg);
			sr->UpdateSprite();

			index = 0;
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

			m_fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::m_fps).str + "\n" +
				"drawcall:" + GTString::ToString(GTTime::m_draw_call).str);
			m_fps->UpdateLabel();
		}

	private:
	};
}

#endif
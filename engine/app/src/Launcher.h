#ifndef __Launcher_h__
#define __Launcher_h__

#include "Component.h"
#include "Application.h"
#include "Texture2D.h"
#include "Screen.h"

#include "SpriteNode.h"
#include "SpriteBatchRenderer.h"
#include "SpriteRenderer.h"

namespace Galaxy3D
{
	class Launcher :public Component
	{
	public:

	protected:
		std::shared_ptr<SpriteNode> m_node;
		std::shared_ptr<Sprite> sps[20];
		std::shared_ptr<SpriteBatchRenderer> m_renderer;
		float index;

		virtual void Start()
		{
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
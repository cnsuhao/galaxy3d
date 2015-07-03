#ifndef __Launcher_h__
#define __Launcher_h__

#include "Component.h"
#include "Sprite.h"
#include "Application.h"
#include "SpriteRenderer.h"
#include "Texture2D.h"
#include "Screen.h"

namespace Galaxy3D
{
	class Launcher :public Component
	{
	public:

	protected:
		virtual void Start()
		{
			auto cam = GameObject::Create("camera")->AddComponent<Camera>();
			cam->SetOrthographicSize(Screen::GetHeight() / 200.f);
			auto sprite = Sprite::Create(Application::GetDataPath() + "/Assets/texture/mustang.jpg");
			auto renderer = GameObject::Create("renderer")->AddComponent<SpriteRenderer>();
			renderer->SetSprite(sprite);

			auto tex = Texture2D::LoadImageFile(Application::GetDataPath() + "/Assets/texture/test.png");

			float h = 384.f;
			std::shared_ptr<Sprite> sps[20];
			for(int i=0; i<5; i++)
			{
				for(int j=0; j<4; j++)
				{
					sps[i*4 + j] = Sprite::Create(tex, Rect(192.f*i, h*j, 192, h), Vector2(96, h), 100, Vector4());
				}
			}

			renderer = GameObject::Create("renderer")->AddComponent<SpriteRenderer>();
			renderer->GetTransform()->SetPosition(Vector3(-3, -2.f, 0));
			renderer->SetSprite(sps[0]);

			renderer = GameObject::Create("renderer")->AddComponent<SpriteRenderer>();
			renderer->GetTransform()->SetPosition(Vector3(-2, -2.f, 0));
			renderer->SetSprite(sps[1]);

			renderer = GameObject::Create("renderer")->AddComponent<SpriteRenderer>();
			renderer->GetTransform()->SetPosition(Vector3(-1, -2.f, 0));
			renderer->SetSprite(sps[2]);

			renderer = GameObject::Create("renderer")->AddComponent<SpriteRenderer>();
			renderer->GetTransform()->SetPosition(Vector3(0, -2.f, 0));
			renderer->SetSprite(sps[3]);

			renderer = GameObject::Create("renderer")->AddComponent<SpriteRenderer>();
			renderer->GetTransform()->SetPosition(Vector3(1, -2.f, 0));
			renderer->SetSprite(sps[4]);

			renderer = GameObject::Create("renderer")->AddComponent<SpriteRenderer>();
			renderer->GetTransform()->SetPosition(Vector3(2, -2.f, 0));
			renderer->SetSprite(sps[5]);

			renderer = GameObject::Create("renderer")->AddComponent<SpriteRenderer>();
			renderer->GetTransform()->SetPosition(Vector3(3, -2.f, 0));
			renderer->SetSprite(sps[6]);

			renderer = GameObject::Create("renderer")->AddComponent<SpriteRenderer>();
			renderer->GetTransform()->SetPosition(Vector3(4, -2.f, 0));
			renderer->SetSprite(sps[7]);
		}

		virtual void Update()
		{
			
		}

	private:
	};
}

#endif
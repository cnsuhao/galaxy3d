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

#include "MirMap.h"
#include "MirHero.h"

namespace Galaxy3D
{
	class Launcher :public Component
	{
	protected:
		std::shared_ptr<SpriteNode> m_node;
		std::shared_ptr<Sprite> sps[20];
		std::shared_ptr<SpriteBatchRenderer> m_renderer;
		std::shared_ptr<TextRenderer> m_fps;
		float index;

		std::vector<MapTile> map_tiles;
		std::shared_ptr<MirHero> hero;

		virtual void Start()
		{
			Label::LoadFont("consola", Application::GetDataPath() + "/Assets/font/consola.ttf");

			auto cam = GameObject::Create("camera")->AddComponent<Camera>();
			cam->SetOrthographicSize(Screen::GetHeight() / 200.f);

			auto label = Label::Create("", "consola", 20, LabelPivot::LeftTop, LabelAlign::Auto, true);
			auto tr = GameObject::Create("label")->AddComponent<TextRenderer>();
			tr->GetTransform()->SetPosition(Vector3(-6.4f, 3.6f, 0));
			tr->SetLabel(label);
			tr->SetSortingOrder(1000, 0);
			m_fps = tr;
			m_fps->GetTransform()->SetParent(cam->GetTransform());

			//mir2
			int x0 = 297;
			int y0 = 299;
			int w = 31;
			int h = 61;
			std::vector<int> coords(w * h);
			for(int y=0; y<h; y++)
			{
				for(int x=0; x<w; x++)
				{
					coords[y * w + x] = ((x + x0 - w/2) << 16) | (y + y0 - h/2);
				}
			}

			//MirMap::LoadTiles(Application::GetDataPath() + "/Assets/mir/Map/0.map", coords, map_tiles);

			hero = std::shared_ptr<MirHero>(new MirHero(x0, y0, 3, 2, 24));

			cam->GetTransform()->SetPosition(Vector3(Mathf::Round((x0 + 0.5f) * 48), Mathf::Round(-(y0 + 0.5f) * 32), 0) * 0.01f);

			auto renderer = GameObject::Create("center")->AddComponent<SpriteRenderer>();
			renderer->SetSortingOrder(1, 0);
			renderer->SetSprite(Sprite::Create(Application::GetDataPath() + "/Assets/mir/center.png"));
			renderer->UpdateSprite();
			renderer->GetTransform()->SetPosition(cam->GetTransform()->GetPosition());
		}

		virtual void Update()
		{
			m_fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::m_fps).str + "\n" +
				"drawcall:" + GTString::ToString(GTTime::m_draw_call).str);
			m_fps->UpdateLabel();

			MirMap::UpdateTiles(map_tiles);
			hero->Update();
		}
	};
}

#endif
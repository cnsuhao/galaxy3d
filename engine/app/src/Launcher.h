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

		virtual void Start()
		{
			Label::LoadFont("consola", Application::GetDataPath() + "/Assets/font/consola.ttf");

			auto cam = GameObject::Create("camera")->AddComponent<Camera>();
			cam->SetOrthographicSize(Screen::GetHeight() / 200.f);

			auto label = Label::Create("", "consola", 20, LabelPivot::LeftTop, LabelAlign::Auto, true);
			auto tr = GameObject::Create("label")->AddComponent<TextRenderer>();
			tr->GetTransform()->SetPosition(Vector3(-4.8f, 3.2f, 0));
			tr->SetLabel(label);
			tr->SetSortingOrder(2);
			m_fps = tr;

			std::vector<MapTileInfo> tiles;
			std::vector<int> coords(35 * 65);
			for(int x=0; x<35; x++)
			{
				for(int y=0; y<65; y++)
				{
					coords[x * 65 + y] = ((x + 300) << 16) | (y + 300);
				}
			}

			MirMap::LoadTiles(Application::GetDataPath() + "/Assets/mir/Map/0.map", coords, tiles);
		}

		virtual void Update()
		{
			m_fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::m_fps).str + "\n" +
				"drawcall:" + GTString::ToString(GTTime::m_draw_call).str);
			m_fps->UpdateLabel();
		}
	};
}

#endif
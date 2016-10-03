#ifndef __Editor_hpp__
#define __Editor_hpp__

#include "Component.h"
#include "EditorWindowWorld.hpp"
#include "EditorWindowProperty.hpp"
#include "EditorWindowConsole.hpp"

class Editor : public Component
{
public:
	UICanvas *canvas;

protected:
	EditorWindow *m_win_world;
	EditorWindow *m_win_property;
	EditorWindow *m_win_console;

	virtual void Start()
	{
		GTUIManager::LoadFont("simsun", Application::GetDataPath() + "/Assets/font/simsun.ttf");
		auto atlas = GTUIManager::LoadAtlas("Editor", Application::GetDataPath() + "/Assets/editor/editor.json");

		float x0 = -Screen::GetWidth() / 2.0f;
		float y0 = Screen::GetHeight() / 2.0f - 20;
		float w0 = 300;
		float h0 = 726;
		float x1 = x0 + w0 + 2;
		float w1 = 1286;
		float x2 = x1 + w1 + 2;
		float w2 = Screen::GetWidth() - w0 - 2 - w1 - 2;
		float y3 = y0 - h0 - 20;
		float w3 = (float) Screen::GetWidth();
		float h3 = Screen::GetHeight() - 20 - h0 - 20;
		float x4 = x0 + w3 + 2;

		auto world = GameObject::Create("World")->AddComponent<EditorWindowWorld>();
		world->canvas = canvas;
		world->atlas = atlas.get();
		world->position = Vector2(x0, y0);
		world->size = Vector2(w0, h0);
		m_win_world = world.get();

		/*
		auto game = GameObject::Create("Game")->AddComponent<EditorWindowGame>();
		game->canvas = canvas.get();
		game->atlas = atlas.get();
		game->position = Vector2(x1, y0);
		game->size = Vector2(w1, h0);
		game->border = Vector2(3, 3);
		*/

		auto property = GameObject::Create("Property")->AddComponent<EditorWindowProperty>();
		property->canvas = canvas;
		property->atlas = atlas.get();
		property->position = Vector2(x2, y0);
		property->size = Vector2(w2, h0);
		m_win_property = property.get();

		auto console = GameObject::Create("Console")->AddComponent<EditorWindowConsole>();
		console->canvas = canvas;
		console->atlas = atlas.get();
		console->position = Vector2(x0, y3);
		console->size = Vector2(w3, h3);
		m_win_console = console.get();

		/*
		auto assets = GameObject::Create("Assets")->AddComponent<EditorWindowAssets>();
		assets->canvas = canvas.get();
		assets->atlas = atlas.get();
		assets->position = Vector2(x4, y3);
		assets->size = Vector2(w3, h3);
		*/
	}

	virtual void Update()
	{
		if(Input::GetKeyDown(KeyCode::F1))
		{
			m_win_world->HideOrShow();
			m_win_property->HideOrShow();
			m_win_console->HideOrShow();
		}
	}
};

#endif
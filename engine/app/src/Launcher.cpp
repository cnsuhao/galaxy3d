#include "Launcher.h"

static std::shared_ptr<TextRenderer> fps;
static std::shared_ptr<Camera> camera;
static std::shared_ptr<MirHero> hero;
static std::shared_ptr<MirHero> hero2;
static std::shared_ptr<MirHero> hero3;
static bool touch_down;
static Vector2 touch_pos;

void Launcher::Start()
{
	Label::LoadFont("consola", Application::GetDataPath() + "/Assets/font/consola.ttf");

	camera= GameObject::Create("camera")->AddComponent<Camera>();
	camera->SetOrthographicSize(Screen::GetHeight() / 200.f);

	auto label = Label::Create("", "consola", 20, LabelPivot::LeftTop, LabelAlign::Auto, true);
	auto tr = GameObject::Create("label")->AddComponent<TextRenderer>();
	tr->GetTransform()->SetPosition(Vector3(-6.4f, 3.6f, 0));
	tr->SetLabel(label);
	tr->SetSortingOrder(1000, 0);
	fps = tr;
	fps->GetTransform()->SetParent(camera->GetTransform());

	//mir2
	int x0 = 297;
	int y0 = 299;
	int w = 31;
	int h = 61;

	MirMap::Load("0", x0, y0, w, h);

	hero = std::shared_ptr<MirHero>(new MirHero(x0, y0, 3, 2, 24, 0));
	hero2 = std::shared_ptr<MirHero>(new MirHero(x0 + 1, y0, 6, 2, 26, 0));
	hero3 = std::shared_ptr<MirHero>(new MirHero(x0 + 2, y0, 11, 2, 37, 0));

	camera->GetTransform()->SetPosition(Vector3(Mathf::Round((x0 + 0.5f) * 48), Mathf::Round(-(y0 + 0.5f) * 32), 0) * 0.01f);
	camera->GetTransform()->SetParent(hero->GetGameObject()->GetTransform());
	hero->SetMain(true);

	touch_down = false;
}

void Launcher::Update()
{
	fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::m_fps).str + "\n" +
		"drawcall:" + GTString::ToString(GTTime::m_draw_call).str);
	fps->UpdateLabel();

	if(Input::GetTouchCount() > 0)
	{
		auto t = Input::GetTouch(0);

		if(t->phase == TouchPhase::Began)
		{
			touch_down = true;

			touch_pos = t->position;
		}
		else if(t->phase == TouchPhase::Ended ||
			t->phase == TouchPhase::Canceled)
		{
			touch_down = false;
		}
		else if(t->phase == TouchPhase::Moved)
		{
			if(touch_down)
			{
				touch_pos = t->position;
			}
		}
	}

	if(touch_down)
	{
		float center_x = Screen::GetWidth() / 2.0f;
		float center_y = Screen::GetHeight() / 2.0f;

		Vector2 dir = touch_pos - Vector2(center_x, center_y);
		float theta = atan2(dir.x, -dir.y) * Mathf::Rad2Deg + 22.5f;
		if(theta < 0)
		{
			theta += 360;
		}
				
		hero->ActionRun((int) (theta / 45));
	}

	MirMap::Update();
	hero->Update();
	camera->UpdateMatrix();

	hero2->Update();
	hero3->Update();
}
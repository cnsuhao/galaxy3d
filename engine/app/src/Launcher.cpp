#include "Launcher.h"

static const int CAMERA_OFFSET_Y = 2;
GameObject *pmesh;
float rot_y = 0;

void Launcher::Start()
{
	Label::LoadFont("consola", Application::GetDataPath() + "/Assets/font/consola.ttf");

	camera = GameObject::Create("camera")->AddComponent<Camera>();
	camera->SetOrthographicSize(Screen::GetHeight() / 200.f);
    camera->SetCullingMask(LayerMask::GetMask(Layer::UI));
    camera->SetDepth(1);
    camera->SetClearFlags(CameraClearFlags::Nothing);

	auto label = Label::Create("", "consola", 20, LabelPivot::LeftTop, LabelAlign::Auto, true);
	auto tr = GameObject::Create("label")->AddComponent<TextRenderer>();
	tr->GetTransform()->SetPosition(Vector3(-Screen::GetWidth()/2.0f, Screen::GetHeight()/2.0f, 0) * 0.01f);
	tr->SetLabel(label);
	tr->SetSortingOrder(1000, 0);
	fps = tr;
	fps->GetTransform()->SetParent(camera->GetTransform());
    fps->GetGameObject()->SetLayer(Layer::UI);

	//mir2
    /*
	int x0 = 297;
	int y0 = 299;
	MirMap::Load("0", x0, y0);

	hero = std::shared_ptr<MirHero>(new MirHero(x0, y0, 3, 2, 24, 0));
	hero2 = std::shared_ptr<MirHero>(new MirHero(x0 + 1, y0, 6, 2, 26, 0));
	hero3 = std::shared_ptr<MirHero>(new MirHero(x0 + 2, y0, 11, 2, 37, 0));

	hero2->ActionAttack(4);
	hero3->ActionAttack(5);

	mon = std::shared_ptr<MirMonster>(new MirMonster(12, 2, x0 + 1, y0 + 1, 4));
	
	camera->GetTransform()->SetPosition(Vector3(Mathf::Round((x0 + 0.5f) * MirMap::TILE_WIDTH), Mathf::Round(-(y0 + 0.5f + CAMERA_OFFSET_Y) * MirMap::TILE_HEIGHT), 0) * 0.01f);
	camera->GetTransform()->SetParent(hero->GetGameObject()->GetTransform());
	hero->SetMain(true);

	touch_down = false;

	//load main ui
	std::vector<int> uis_index;
	uis_index.push_back(1);
	auto uit = MirImage::LoadImages("ui", uis_index);
	
    auto uis = Sprite::Create(
        uit[0]->texture,
        Rect(0, 0, (float) uit[0]->texture->GetWidth(), (float) uit[0]->texture->GetHeight()),
        Vector2(0.5f, 1),
        100,
        Vector4(0, 0, 0, 0));

	std::shared_ptr<SpriteRenderer> uir = GameObject::Create("")->AddComponent<SpriteRenderer>();
	uir->GetTransform()->SetParent(camera->GetTransform());
	uir->GetTransform()->SetLocalPosition(Vector3(0, -Screen::GetHeight()/2.0f, 0) * 0.01f);

	auto mat = Material::Create("UI/Mir Sprite");
	mat->SetTexture("ColorTable", MirImage::GetColorTable());
	uir->SetSharedMaterial(mat);
	uir->SetSortingOrder(1000, 0);
	uir->SetSprite(uis);
	uir->UpdateSprite();
    */

    auto cam3d = GameObject::Create("camera")->AddComponent<Camera>();
    cam3d->SetOrthographic(false);
    cam3d->SetClipPlane(0.3f, 1000.0f);
    cam3d->SetCullingMask(LayerMask::GetMask(Layer::Default));
    cam3d->GetTransform()->SetPosition(Vector3(0, 1, -5));
    cam3d->SetDepth(0);

    auto mesh = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/Arthas.mesh");
    mesh->SetLayerRecursive(Layer::Default);
    mesh->GetTransform()->SetPosition(Vector3(-1.5f, 0, 0));
    pmesh = mesh.get();

    auto anim = Mesh::LoadSkinnedMesh(Application::GetDataPath() + "/Assets/mesh/Arthas.anim");
    anim->SetLayerRecursive(Layer::Default);
    anim->GetTransform()->SetPosition(Vector3(2, 0, 0));
    anim->GetTransform()->SetRotation(Quaternion::Euler(0, 180, 0));

    std::string clip = "Idle_Arthas_36896b399471f50409feff906777c5af.1.clip";
    auto anim_com = anim->GetComponent<Animation>();
    anim_com->GetAnimationState(clip)->wrap_mode = WrapMode::Loop;
    anim_com->Play(clip);
}

/*
bool Launcher::OnTouchDown(const Vector2 &pos)
{
	Vector3 cam_pos_pixel = camera->GetTransform()->GetPosition() * 100;
	cam_pos_pixel.y = -cam_pos_pixel.y;
	cam_pos_pixel += -Vector3(Screen::GetWidth() * 0.5f, Screen::GetHeight() * 0.5f);

	Vector2 pixel_world = pos + Vector2(cam_pos_pixel.x, cam_pos_pixel.y);

	if(mon->OnTouchDown(pixel_world))
	{
		auto offset = mon->GetPos() - hero->GetPos();
		int ox = (int) offset.x;
		int oy = (int) offset.y;

		if(ox >= -1 && ox<= 1 && oy >= -1 && oy<= 1)
		{
			float theta = atan2(offset.x, -offset.y) * Mathf::Rad2Deg + 22.5f;
			if(theta < 0)
			{
				theta += 360;
			}

			hero->ActionAttack((int) (theta / 45));
		}
		
		return true;
	}

	return false;
}
*/

void Launcher::Update()
{
	fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::m_fps).str + "\n" +
		"drawcall:" + GTString::ToString(GTTime::m_draw_call).str);
	fps->UpdateLabel();

    rot_y += 0.15f;
    pmesh->GetTransform()->SetRotation(Quaternion::Euler(0, rot_y, 0));

    /*
	if(Input::GetTouchCount() > 0)
	{
		auto t = Input::GetTouch(0);

		if(t->phase == TouchPhase::Began)
		{
			if(!OnTouchDown(t->position))
			{
				touch_down = true;

				touch_pos = t->position;
			}
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
		float center_y = Screen::GetHeight() / 2.0f - CAMERA_OFFSET_Y * MirMap::TILE_HEIGHT;

		Vector2 dir = touch_pos - Vector2(center_x, center_y);
		float theta = atan2(dir.x, -dir.y) * Mathf::Rad2Deg + 22.5f;
		if(theta < 0)
		{
			theta += 360;
		}
				
		hero->ActionMove((int) (theta / 45));
	}

	MirMap::Update();
	hero->Update();
    camera->UpdateMatrix();
	hero2->Update();
	hero3->Update();
    mon->Update();
    */
}

Launcher::~Launcher()
{
	//MirMap::Unload();
}
#include "Launcher.h"

using namespace Galaxy3D;

std::string clip_idle = "Idle_Arthas_36896b399471f50409feff906777c5af.1.clip";
std::string clip_move = "Move_Arthas_1586e0d40a0ba4545bd97991164aec42.1.clip";

void Launcher::Start()
{
	Label::LoadFont("consola", Application::GetDataPath() + "/Assets/font/consola.ttf");

    cam2d = GameObject::Create("camera")->AddComponent<Camera>();
    cam2d->SetOrthographicSize(Screen::GetHeight() / 200.f);
    cam2d->SetCullingMask(LayerMask::GetMask(Layer::UI));
    cam2d->SetDepth(1);
    cam2d->SetClearFlags(CameraClearFlags::Nothing);

	auto label = Label::Create("", "consola", 20, LabelPivot::LeftTop, LabelAlign::Auto, true);
	auto tr = GameObject::Create("label")->AddComponent<TextRenderer>();
	tr->GetTransform()->SetPosition(Vector3(-Screen::GetWidth()/2.0f, Screen::GetHeight()/2.0f, 0) * 0.01f);
	tr->SetLabel(label);
	tr->SetSortingOrder(1000, 0);
	fps = tr;
	fps->GetTransform()->SetParent(cam2d->GetTransform());
    fps->GetGameObject()->SetLayer(Layer::UI);

    cam3d = GameObject::Create("camera")->AddComponent<Camera>();
    cam3d->SetOrthographic(false);
    cam3d->SetFieldOfView(30);
    cam3d->SetClipPlane(0.3f, 1000.0f);
    cam3d->SetCullingMask(LayerMask::GetMask(Layer::Default));
    cam3d->SetDepth(0);
    cam3d->SetClearFlags(CameraClearFlags::SolidColor);
    cam3d->GetTransform()->SetPosition(Vector3(101, 7, 115));
    cam3d->GetTransform()->SetRotation(Quaternion::Euler(20, -140, 0));
    
    std::vector<std::string> terrain_texs;
    terrain_texs.push_back(Application::GetDataPath() + "/Assets/terrain/1.png");
    terrain_texs.push_back(Application::GetDataPath() + "/Assets/terrain/2.png");
    terrain_texs.push_back(Application::GetDataPath() + "/Assets/terrain/3.png");
    terrain_texs.push_back(Application::GetDataPath() + "/Assets/terrain/4.png");

    GameObject *terrain_obj = GameObject::Create("terrain").get();
    terrain_obj->SetLayer(Layer::Default);

    auto ter = terrain_obj->AddComponent<Terrain>();
    ter->SetCamera(cam3d);

    ter->LoadData(
        513,
        200.0f, 600.0f,
        Application::GetDataPath() + "/Assets/terrain/Terrain.raw",
        Application::GetDataPath() + "/Assets/terrain/Terrain.png",
        terrain_texs, 3);
    auto renderer = terrain_obj->AddComponent<TerrainRenderer>();
    renderer->SetTerrain(ter);
    renderer->SetSharedMaterial(ter->GetSharedMaterial());
    
    auto lightmap_ter = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/terrain/LightmapFar-1.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
    ter->GetSharedMaterial()->SetTexture("_Lightmap", lightmap_ter);

    auto lightmap = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/terrain/Objects/LightmapFar-0.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
    LightmapSettings::lightmaps.push_back(lightmap);
    /*
    auto mesh = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/terrain/Objects/Objects.mesh");
    mesh->SetLayerRecursive(Layer::Default);
    */
    auto anim_obj = Mesh::LoadSkinnedMesh(Application::GetDataPath() + "/Assets/mesh/Arthas.anim");
    anim_obj->SetLayerRecursive(Layer::Default);
    anim_obj->GetTransform()->SetPosition(Vector3(91, 0.05f, 103));
    anim_obj->GetTransform()->SetRotation(Quaternion::Euler(0, 0, 0));
    anim_obj->GetTransform()->SetScale(Vector3(1, 1, 1) * 0.5f);

    cam_offset = cam3d->GetTransform()->GetPosition() - anim_obj->GetTransform()->GetPosition();

    anim = anim_obj->GetComponent<Animation>();
    anim->GetAnimationState(clip_idle)->wrap_mode = WrapMode::Loop;
    anim->GetAnimationState(clip_move)->wrap_mode = WrapMode::Loop;
    anim->Play(clip_idle);
    
    RenderSettings::light_ambient = Color(1, 1, 1, 1) * 0.2f;
    RenderSettings::light_directional_color = Color(1, 1, 1, 1) * 0.6f;
    RenderSettings::light_directional_intensity = 1;
    RenderSettings::light_directional_direction = Vector3(0, -1, -1);

    Physics::Init();

    int file_size;
    void *terrain_data = GTFile::ReadAllBytes(Application::GetDataPath() + "/Assets/terrain/Terrain.raw", &file_size);
    if(terrain_data != NULL)
    {
        Physics::CreateTerrainRigidBody(513, (short *) terrain_data, 200.0f, 600.0f);
        free(terrain_data);
    }
}

void Launcher::OnTweenFinished(Component *tween, std::weak_ptr<Component> &target)
{
    if(!target.expired())
    {
        auto thiz = std::dynamic_pointer_cast<Launcher>(target.lock());
        thiz->anim->CrossFade(clip_idle);
    }
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
            Vector3 pos = t->position;
            Ray ray = cam3d->ScreenPointToRay(pos);

            Vector3 hit;
            Vector3 nor;
            //if(Physics::RarCast(Vector3(100, 5000, 100), Vector3(0, -1, 0), 10000, hit, nor))
            if(Physics::RarCast(ray.origin, ray.GetDirection(), 1000, hit, nor))
            {
                Debug::Log("%s", hit.ToString().c_str());

                Vector3 pos_old = anim->GetTransform()->GetPosition();
                Vector3 pos_new = hit + Vector3(0, 0.05f, 0);
                float speed = 3.0f;
                float move_time = (pos_new - pos_old).Magnitude() / speed;
                Vector3 dir = pos_new - pos_old;
                dir.y = 0;

                anim->CrossFade(clip_move);
                anim->GetTransform()->SetForward(dir);

                auto tp = anim->GetGameObject()->GetComponent<TweenPosition>();
                if(tp)
                {
                    tp->Reset();
                }
                else
                {
                    tp = anim->GetGameObject()->AddComponent<TweenPosition>();
                    tp->delay = 0;
                    tp->is_world = true;
                    tp->curve.keys.push_back(Keyframe(0, 0, 1, 1));
                    tp->curve.keys.push_back(Keyframe(1, 1, 1, 1));
                    tp->target = GetComponentPtr();
                    tp->on_finished = OnTweenFinished;
                }

                tp->from = pos_old;
                tp->to = pos_new;
                tp->duration = move_time;
            }
		}
		else if(
            t->phase == TouchPhase::Ended ||
			t->phase == TouchPhase::Canceled)
		{
		}
		else if(t->phase == TouchPhase::Moved)
		{
            Vector3 pos = t->position;
		}
	}

    Physics::Step();
}

void Launcher::LateUpdate()
{
    Vector3 pos = anim->GetTransform()->GetPosition();
    cam3d->GetTransform()->SetPosition(pos + cam_offset);
    cam3d->UpdateMatrix();
}

Launcher::~Launcher()
{
    Physics::Done();
}
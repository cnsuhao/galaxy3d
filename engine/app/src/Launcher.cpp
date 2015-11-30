#include "Launcher.h"

#define DEMO_0 0
#define DEMO_1 0
#define PS_TEST 1

using namespace Galaxy3D;

#if DEMO_0
std::string clip_idle = "Idle_Arthas_36896b399471f50409feff906777c5af.1.clip";
std::string clip_move = "Move_Arthas_1586e0d40a0ba4545bd97991164aec42.1.clip";
#endif

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

#if PS_TEST
    cam3d = GameObject::Create("camera")->AddComponent<Camera>();
    cam3d->SetOrthographic(false);
    cam3d->SetFieldOfView(60);
    cam3d->SetClipPlane(0.3f, 1000.0f);
    cam3d->SetCullingMask(LayerMask::GetMask(Layer::Default));
    cam3d->SetDepth(0);
    cam3d->SetClearColor(Color(12, 29, 54, 255) * (1.0f / 255));
    cam3d->SetClearFlags(CameraClearFlags::SolidColor);
    cam3d->GetTransform()->SetPosition(Vector3(0, 0, -10));

    auto ps = GameObject::Create("ps")->AddComponent<ParticleSystem>();
    ps->GetTransform()->SetRotation(Quaternion::Euler(-90, 0, 0));
    ps->SetTargetCamera(cam3d);
    auto psr = ps->GetGameObject()->GetComponent<ParticleSystemRenderer>();
    auto psm = Material::Create("Particles/Additive");
    psm->SetColor("_TintColor", Color(1, 1, 1, 1) * 0.5f);
    auto pst = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/texture/arthas.jpg", FilterMode::Bilinear, TextureWrapMode::Clamp);
    psm->SetMainTexture(pst);
    psr->SetSharedMaterial(psm);

#endif

#if DEMO_1
    cam3d = GameObject::Create("camera")->AddComponent<Camera>();
    cam3d->SetOrthographic(false);
    cam3d->SetFieldOfView(45);
    cam3d->SetClipPlane(0.3f, 1000.0f);
    cam3d->SetCullingMask(LayerMask::GetMask(Layer::Default));
    cam3d->SetDepth(0);
    cam3d->SetClearColor(Color(12, 29, 54, 255) * (1.0f / 255));
    cam3d->SetClearFlags(CameraClearFlags::SolidColor);
    cam3d->GetTransform()->SetPosition(Vector3(-65.13f, 5.47f, 4.76f));
    cam3d->GetTransform()->SetRotation(Quaternion::Euler(15.7f, 129.4f, 0));

    auto lightmap_0 = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/mesh/scene/Lightmap-0_comp_light.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
    auto lightmap_1 = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/mesh/scene/Lightmap-1_comp_light.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
    LightmapSettings::lightmaps.clear();
    LightmapSettings::lightmaps.push_back(lightmap_0);
    LightmapSettings::lightmaps.push_back(lightmap_1);

    auto scene = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/scene/Module_01.mesh");
    scene->SetLayerRecursive(Layer::Default);
    Renderer::BuildOctree(scene);
#endif

#if DEMO_0
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
    
    auto mesh = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/terrain/Objects/Objects.mesh");
    mesh->SetLayerRecursive(Layer::Default);
    Renderer::BuildOctree(mesh);
    
    auto anim_obj = Mesh::LoadSkinnedMesh(Application::GetDataPath() + "/Assets/mesh/anim/Arthas.anim");
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
    Physics::CreateTerrainRigidBody(ter.get());

    //set anim to ground
    Vector3 anim_pos = anim->GetTransform()->GetPosition();
    Vector3 hit;
    Vector3 nor;
    if(Physics::RarCast(Vector3(anim_pos.x, 1000, anim_pos.z), Vector3(0, -1, 0), 2000, hit, nor))
    {
        hit.y += 0.05f;
        anim->GetTransform()->SetPosition(hit);
    }
#endif
}

#if DEMO_0
void Launcher::OnTweenSetValue(Component *tween, std::weak_ptr<Component> &target, void *value)
{
    if(!target.expired())
    {
        auto thiz = std::dynamic_pointer_cast<Launcher>(target.lock());
        Vector3 *pos = (Vector3 *) value;

        Vector3 hit;
        Vector3 nor;
        if(Physics::RarCast(Vector3(pos->x, 1000, pos->z), Vector3(0, -1, 0), 2000, hit, nor))
        {
            hit.y += 0.05f;
            thiz->anim->GetTransform()->SetPosition(hit);
        }
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
#endif

void Launcher::Update()
{
	fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::m_fps).str + "\n" +
		"drawcall:" + GTString::ToString(GTTime::m_draw_call).str);
	fps->UpdateLabel();

#if DEMO_0
	if(Input::GetTouchCount() > 0)
	{
		auto t = Input::GetTouch(0);

		if(t->phase == TouchPhase::Began)
		{
            Vector3 pos = t->position;
            Ray ray = cam3d->ScreenPointToRay(pos);

            Vector3 hit;
            Vector3 nor;
            if(Physics::RarCast(ray.origin, ray.GetDirection(), 1000, hit, nor))
            {
                Vector3 pos_old = anim->GetTransform()->GetPosition();
                Vector3 pos_new = hit + Vector3(0, 0.05f, 0);
                float move_speed = 3.0f;
                float move_time = (pos_new - pos_old).Magnitude() / move_speed;
                Vector3 dir = pos_new - pos_old;
                dir.y = 0;

                anim->CrossFade(clip_move);

                {
                    // tween position
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
                        tp->on_set_value = OnTweenSetValue;
                        tp->on_finished = OnTweenFinished;
                    }

                    tp->from = pos_old;
                    tp->to = pos_new;
                    tp->duration = move_time;
                }

                {
                    Vector3 origin = anim->GetTransform()->GetForward();
                    Vector3 fn = Vector3::Normalize(dir);
                    
                    if(!Mathf::FloatEqual(fn.SqrMagnitude(), 0) && fn != origin)
                    {
                        // tween rotation
                        float deg = Vector3::Angle(origin, fn);
                        Vector3 axis = origin * fn;
                        Quaternion rot;

                        float rot_speed = 270.0f;
                        float rot_time = deg / rot_speed;

                        if(axis == Vector3(0, 0, 0))
                        {
                            rot = Quaternion::AngleAxis(deg, anim->GetTransform()->GetUp());
                        }
                        else
                        {
                            rot = Quaternion::AngleAxis(deg, axis);
                        }

                        auto tr = anim->GetGameObject()->GetComponent<TweenRotation>();
                        if(tr)
                        {
                            tr->Reset();
                        }
                        else
                        {
                            tr = anim->GetGameObject()->AddComponent<TweenRotation>();
                            tr->delay = 0;
                            tr->is_world = true;
                            tr->curve.keys.push_back(Keyframe(0, 0, 1, 1));
                            tr->curve.keys.push_back(Keyframe(1, 1, 1, 1));
                        }

                        Quaternion rot_now = anim->GetTransform()->GetRotation();

                        tr->from = rot_now;
                        tr->to = rot * rot_now;
                        tr->duration = rot_time;
                    }
                }
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
#endif
}

void Launcher::LateUpdate()
{
#if DEMO_0
    Vector3 pos = anim->GetTransform()->GetPosition();
    cam3d->GetTransform()->SetPosition(pos + cam_offset);
    cam3d->UpdateMatrix();
#endif
}

Launcher::~Launcher()
{
#if DEMO_0
    Physics::Done();
#endif
}
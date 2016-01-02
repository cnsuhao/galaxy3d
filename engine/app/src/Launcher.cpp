#include "Launcher.h"

#define DEMO_TERRAIN 0
#define DEMO_SCENE 0
#define DEMO_DEFERRED_SHADING 0
#define DEMO_DEF 1

using namespace Galaxy3D;

#if DEMO_TERRAIN
std::string clip_idle = "Idle_Arthas_36896b399471f50409feff906777c5af.1.clip";
std::string clip_move = "Move_Arthas_1586e0d40a0ba4545bd97991164aec42.1.clip";
Vector3 anim_rot(0, 0, 0);
#endif

#if DEMO_SCENE
static void add_fire_particles(std::shared_ptr<GameObject> &fire, std::shared_ptr<Camera> &cam3d);
static void add_lamp_particles(std::shared_ptr<GameObject> &lamp, std::shared_ptr<Camera> &cam3d);
static void add_dust_particles(std::shared_ptr<GameObject> &parent, std::shared_ptr<Camera> &cam3d, const Vector3 &local_pos);
#endif

void Launcher::Start()
{
	Label::LoadFont("consola", Application::GetDataPath() + "/Assets/font/consola.ttf");
    
    cam2d = GameObject::Create("camera")->AddComponent<Camera>();
    cam2d->SetOrthographic(true);
    cam2d->SetOrthographicSize(Screen::GetHeight() / 200.f);
    cam2d->SetClipPlane(-1, 1);
    cam2d->SetCullingMask(LayerMask::GetMask(Layer::UI));
    cam2d->SetDepth(1);
    cam2d->SetClearFlags(CameraClearFlags::Nothing);

	auto label = Label::Create("", "consola", 20, LabelPivot::LeftTop, LabelAlign::Auto, true);
	auto tr = GameObject::Create("label")->AddComponent<TextRenderer>();
	tr->GetTransform()->SetPosition(Vector3(-Screen::GetWidth()/2.0f, Screen::GetHeight()/2.0f, 0) * 0.01f);
	tr->SetLabel(label);
	tr->SetSortingOrder(1000, 0);
    tr->GetTransform()->SetParent(cam2d->GetTransform());
    tr->GetGameObject()->SetLayer(Layer::UI);
	fps = tr;

#if DEMO_DEFERRED_SHADING
    cam3d = GameObject::Create("camera")->AddComponent<Camera>();
    cam3d->SetFieldOfView(45);
    cam3d->SetCullingMask(LayerMask::GetMask(Layer::Default));
    cam3d->SetDepth(0);
    cam3d->SetClearColor(Color(12, 29, 54, 255) * (1.0f / 255));
    cam3d->GetTransform()->SetPosition(Vector3(-1, 6, -30));
    cam3d->GetTransform()->SetRotation(Quaternion::Euler(20, 0, 0));
    cam3d->SetClipPlane(0.05f, 300);
    cam3d->EnableDeferredShading(true);
    cam3d->EnableHDR(true);
    cam3d->GetGameObject()->AddComponent<ImageEffectBloom>();
    cam3d->GetGameObject()->AddComponent<ImageEffectToneMapping>();

    RenderSettings::light_ambient = Color(1, 1, 1, 1) * 0.0f;

    auto cube = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/primitive/Cube.mesh");
    cube->GetTransform()->SetPosition(Vector3(0, -1, 0));
    cube->GetTransform()->SetScale(Vector3(200, 2, 1000));
    auto rs = cube->GetComponentsInChildren<MeshRenderer>();
    for(auto i : rs)
    {
        i->CalculateBounds();
    }

    auto sphere = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/primitive/Sphere.mesh");
    sphere->GetTransform()->SetPosition(Vector3(10, 2, 50));
    sphere->GetTransform()->SetScale(Vector3(4, 4, 4));
    rs = sphere->GetComponentsInChildren<MeshRenderer>();
    for(auto i : rs)
    {
        i->CalculateBounds();
    }

    for(int i=0; i<20; i++)
    {
        cube = GameObject::Instantiate(cube);
        cube->GetTransform()->SetPosition(Vector3(-3, 1, -20.0f + i * 4));
        cube->GetTransform()->SetScale(Vector3(2, 2, 2));
        
        rs = cube->GetComponentsInChildren<MeshRenderer>();
        for(auto j : rs)
        {
            j->CalculateBounds();
        }
    }

    sphere = GameObject::Instantiate(sphere);
    sphere->GetTransform()->SetPosition(Vector3(-7, 1, -20));
    sphere->GetTransform()->SetScale(Vector3(2, 2, 2));
    rs = sphere->GetComponentsInChildren<MeshRenderer>();
    for(auto i : rs)
    {
        i->CalculateBounds();
    }

    auto light = GameObject::Create("light")->AddComponent<Light>();
    light->GetTransform()->SetRotation(Quaternion::Euler(45, 50, 0));
    light->SetType(LightType::Directional);
    light->SetColor(Color(1, 1, 1, 1) * 0.2f);
    light->EnableShadow(true);
    light->EnableCascade(true);

    RenderSettings::SetGlobalDirectionalLight(light);
    std::vector<float> cascade_splits;
    cascade_splits.push_back(0.05f);
    cascade_splits.push_back(0.15f);
    cascade_splits.push_back(0.35f);
    Light::SetCascadeSplits(cascade_splits);

    light = GameObject::Create("light")->AddComponent<Light>();
    light->GetTransform()->SetPosition(Vector3(0, 4, -15));
    light->GetTransform()->SetRotation(Quaternion::Euler(45, -90, 0));
    light->SetType(LightType::Spot);
    light->SetSpotAngle(120);
    light->SetRange(100);
    light->EnableShadow(true);
    light->SetColor(Color(0, 1, 0, 1));
    auto tp = light->GetGameObject()->AddComponent<TweenPosition>();
    tp->from = Vector3(0, 4, -18);
    tp->to = Vector3(0, 4, 50);
    tp->duration = 8;
    tp->curve = AnimationCurve();
    tp->curve.keys.push_back(Keyframe(0, 0, 1, 1));
    tp->curve.keys.push_back(Keyframe(0.5, 1, 1, -1));
    tp->curve.keys.push_back(Keyframe(1, 0, -1, -1));
    tp->loop = true;
    
    light = GameObject::Create("light")->AddComponent<Light>();
    light->GetTransform()->SetPosition(Vector3(3, 1, -18));
    light->SetRange(10);
    light->SetColor(Color(1, 0, 0, 1));
    tp = light->GetGameObject()->AddComponent<TweenPosition>();
    tp->from = Vector3(3, 1, -18);
    tp->to = Vector3(-5, 1, -18);
    tp->duration = 4;
    tp->curve = AnimationCurve();
    tp->curve.keys.push_back(Keyframe(0, 0, 1, 1));
    tp->curve.keys.push_back(Keyframe(0.5, 1, 1, -1));
    tp->curve.keys.push_back(Keyframe(1, 0, -1, -1));
    tp->loop = true;
    
    light = GameObject::Create("light")->AddComponent<Light>();
    light->GetTransform()->SetRotation(Quaternion::Euler(30, 150, 0));
    light->SetType(LightType::Directional);
    light->SetColor(Color(1, 1, 1, 1) * 0.2f);
    light->EnableShadow(true);
    light->EnableCascade(true);
    auto tro = light->GetGameObject()->AddComponent<TweenRotation>();
    tro->from = Vector3(30, 0, 0);
    tro->to = Vector3(30, 360, 0);
    tro->duration = 20;
    tro->curve = AnimationCurve();
    tro->curve.keys.push_back(Keyframe(0, 0, 1, 1));
    tro->curve.keys.push_back(Keyframe(1, 1, 1, 1));
    tro->loop = true;
#endif

#if DEMO_SCENE
    cam3d = GameObject::Create("camera")->AddComponent<Camera>();
    cam3d->SetFieldOfView(45);
    cam3d->SetCullingMask(LayerMask::GetMask(Layer::Default));
    cam3d->SetDepth(0);
    cam3d->SetClearColor(Color(12, 29, 54, 255) * (1.0f / 255));
    cam3d->GetTransform()->SetPosition(Vector3(-21.24f, 10.97f, 44.14f));
    cam3d->GetTransform()->SetRotation(Quaternion::Euler(24.58f, 137.26f, 0));
    cam3d->EnableHDR(true);
    cam3d->GetGameObject()->AddComponent<ImageEffectGlobalFog>();
    cam3d->GetGameObject()->AddComponent<ImageEffectBloom>();
    cam3d->GetGameObject()->AddComponent<ImageEffectToneMapping>();

    auto lightmap_0 = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/mesh/scene/Lightmap-0_comp_light.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
    auto lightmap_1 = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/mesh/scene/Lightmap-1_comp_light.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
    LightmapSettings::lightmaps.clear();
    LightmapSettings::lightmaps.push_back(lightmap_0);
    LightmapSettings::lightmaps.push_back(lightmap_1);
    
    auto scene = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/scene/Module_01.mesh");
    
    int cc = scene->GetTransform()->GetChildCount();
    for(int i=0; i<cc; i++)
    {
        auto child = scene->GetTransform()->GetChild(i);
        if("Furnace_A_01" == child->GetName())
        {
            add_fire_particles(child->GetGameObject(), cam3d);
        }
        else if("LampA_01" == child->GetName())
        {
            add_lamp_particles(child->GetGameObject(), cam3d);
        }
    }
    
    auto dusts = GameObject::Create("FX");
    dusts->GetTransform()->SetParent(scene->GetTransform());
    dusts->GetTransform()->SetLocalPosition(Vector3(28.68044f, -44.94969f, -16.97657f));
    add_dust_particles(dusts, cam3d, Vector3(-17.94812f, 41.30858f, 60.79422f));
    add_dust_particles(dusts, cam3d, Vector3(-7.215611f, 46.65371f, 44.56079f));
    add_dust_particles(dusts, cam3d, Vector3(-22.39325f, 39.16286f, 19.75619f));
    add_dust_particles(dusts, cam3d, Vector3(-47.34644f, 43.30156f, 15.99386f));
    
    Renderer::BuildOctree(scene);

    scene->SetStaticRecursively();
    Renderer::BuildStaticBatches();
#endif

#if DEMO_TERRAIN
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
    
    auto lightmap_ter = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/terrain/Lightmap-1_comp_light.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
    ter->GetSharedMaterial()->SetTexture("_Lightmap", lightmap_ter);

    auto lightmap = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/terrain/Lightmap-0_comp_light.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
    LightmapSettings::lightmaps.push_back(lightmap);

    auto mesh = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/terrain/Objects/Objects.mesh");
    mesh->SetLayerRecursively(Layer::Default);
    Renderer::BuildOctree(mesh);

    auto anim_obj = Mesh::LoadSkinnedMesh(Application::GetDataPath() + "/Assets/mesh/anim/Arthas/Arthas.anim");
    anim_obj->SetLayerRecursively(Layer::Default);
    anim_obj->GetTransform()->SetPosition(Vector3(91, 0.05f, 103));
    anim_obj->GetTransform()->SetRotation(Quaternion::Euler(anim_rot));
    anim_obj->GetTransform()->SetScale(Vector3(1, 1, 1) * 0.5f);

    cam_offset = cam3d->GetTransform()->GetPosition() - anim_obj->GetTransform()->GetPosition();

    anim = anim_obj->GetComponent<Animation>();
    anim->GetAnimationState(clip_idle)->wrap_mode = WrapMode::Loop;
    anim->GetAnimationState(clip_move)->wrap_mode = WrapMode::Loop;
    anim->Play(clip_idle);
    
    auto light = GameObject::Create("light")->AddComponent<Light>();
    light->GetTransform()->SetRotation(Quaternion::Euler(135, 0, 0));
    light->SetType(LightType::Directional);
    light->SetColor(Color(1, 1, 1, 1) * 0.6f);

    RenderSettings::SetGlobalDirectionalLight(light);
    RenderSettings::light_ambient = Color(1, 1, 1, 1) * 0.2f;

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

#if DEMO_DEF
    auto light = GameObject::Create("light")->AddComponent<Light>();
    light->GetTransform()->SetRotation(Quaternion::Euler(50, -150, 0));
    light->SetType(LightType::Directional);
    light->SetColor(Color(1, 1, 1, 1) * 1.0f);
    light->EnableShadow(true);
    light->EnableCascade(true);

    RenderSettings::SetGlobalDirectionalLight(light);
    std::vector<float> cascade_splits;
    cascade_splits.push_back(0.25f);
    cascade_splits.push_back(0.55f);
    cascade_splits.push_back(0.90f);
    Light::SetCascadeSplits(cascade_splits);
    RenderSettings::light_ambient = Color(1, 1, 1, 1) * 0.5f;

    cam3d = GameObject::Create("camera")->AddComponent<Camera>();
    cam3d->SetFieldOfView(45);
    cam3d->SetClipPlane(1.0f, 100.0f);
    cam3d->SetCullingMask(LayerMask::GetMask(Layer::Default));
    cam3d->GetTransform()->SetPosition(Vector3(0, 3.695f, 10.3f));
    cam3d->GetTransform()->SetRotation(Quaternion::Euler(10, -180, 0));
    cam3d->EnableDeferredShading(true);
    auto fog = cam3d->GetGameObject()->AddComponent<ImageEffectGlobalFog>();
    fog->EnableHeight(false);
    fog->SetFogMode(FogMode::ExponentialSquared);
    fog->SetFogColor(Color(1, 1, 1, 1) * 0.5f);
    fog->SetFogDensity(0.02f);

    auto mesh = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/LY/LY-1.mesh");

    auto anim_parent = GameObject::Create("anim_parent");
    auto anim_obj = Mesh::LoadSkinnedMesh(Application::GetDataPath() + "/Assets/mesh/anim/Warrior/warrior.anim");
    anim_obj->GetTransform()->SetPosition(Vector3(0, 0, 0));
    anim_obj->GetTransform()->SetRotation(Quaternion::Euler(0, 180, 0));
    anim_obj->GetTransform()->SetParent(anim_parent->GetTransform());
    cam3d->GetTransform()->SetParent(anim_parent->GetTransform());
    anim = anim_obj->GetComponent<Animation>();
    anim->GetAnimationState("idle")->wrap_mode = WrapMode::Loop;
    anim->GetAnimationState("run")->wrap_mode = WrapMode::Loop;
    anim->Play("idle");

    // navmesh
    NavMesh::LoadFromFile(Application::GetDataPath() + "/Assets/mesh/LY/navmesh.nav");
    anim_parent->GetTransform()->SetPosition(Vector3(1, 0, 0));
    auto agent = anim_parent->AddComponent<NavMeshAgent>();
    agent->SamplePosition();
    cam3d->UpdateMatrix();
#endif
}

#if DEMO_TERRAIN
void Launcher::OnTweenPositionSetValue(Component *tween, std::weak_ptr<Component> &target, void *value)
{
    if(!target.expired())
    {
        auto thiz = std::dynamic_pointer_cast<Launcher>(target.lock());
        Vector3 *pos = (Vector3 *) value;
        TweenPosition *tp = (TweenPosition *) tween;

        Vector3 hit;
        Vector3 nor;
        if(Physics::RarCast(Vector3(pos->x, 1000, pos->z), Vector3(0, -1, 0), 2000, hit, nor))
        {
            hit.y += 0.05f;

            thiz->anim->GetTransform()->SetPosition(hit);
        }
    }
}

void Launcher::OnTweenPositionFinished(Component *tween, std::weak_ptr<Component> &target)
{
    if(!target.expired())
    {
        auto thiz = std::dynamic_pointer_cast<Launcher>(target.lock());
        thiz->anim->CrossFade(clip_idle);
    }
}

void Launcher::OnTweenRotationSetValue(Component *tween, std::weak_ptr<Component> &target, void *value)
{
    if(!target.expired())
    {
        auto thiz = std::dynamic_pointer_cast<Launcher>(target.lock());
        Vector3 *rot = (Vector3 *) value;
        TweenRotation *tr = (TweenRotation *) tween;
        auto quat = Quaternion::Euler(*rot);

        thiz->anim->GetTransform()->SetRotation(quat);
        anim_rot = *rot;
    }
}
#endif

int g_key_down_count = 0;

static void move_key_down(std::shared_ptr<Animation> &anim)
{
    g_key_down_count++;

    if(g_key_down_count == 1)
    {
        anim->CrossFade("run");
    }
}

static void move_key_up(std::shared_ptr<Animation> &anim)
{
    g_key_down_count--;

    if(g_key_down_count == 0)
    {
        anim->CrossFade("idle");
    }
}

void Launcher::Update()
{
	fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::m_fps).str + "\n" +
		"drawcall:" + GTString::ToString(GTTime::m_draw_call).str);
	fps->UpdateLabel();

#if DEMO_DEF
    int key_down_count_old = g_key_down_count;

    if(Input::GetKeyDown(KeyCode::W)) move_key_down(anim);
    if(Input::GetKeyDown(KeyCode::S)) move_key_down(anim);
    if(Input::GetKeyDown(KeyCode::A)) move_key_down(anim);
    if(Input::GetKeyDown(KeyCode::D)) move_key_down(anim);

    if(Input::GetKeyUp(KeyCode::W)) move_key_up(anim);
    if(Input::GetKeyUp(KeyCode::S)) move_key_up(anim);
    if(Input::GetKeyUp(KeyCode::A)) move_key_up(anim);
    if(Input::GetKeyUp(KeyCode::D)) move_key_up(anim);

    Vector3 move_dir(0, 0, 0);
    if(Input::GetKey(KeyCode::W))
    {
        auto dir = cam3d->GetTransform()->GetForward();
        dir.y = 0;
        dir.Normalize();
        move_dir += dir;
    }
    if(Input::GetKey(KeyCode::S))
    {
        auto dir = -cam3d->GetTransform()->GetForward();
        dir.y = 0;
        dir.Normalize();
        move_dir += dir;
    }
    if(Input::GetKey(KeyCode::A))
    {
        auto dir = -cam3d->GetTransform()->GetRight();
        dir.y = 0;
        dir.Normalize();
        move_dir += dir;
    }
    if(Input::GetKey(KeyCode::D))
    {
        auto dir = cam3d->GetTransform()->GetRight();
        dir.y = 0;
        dir.Normalize();
        move_dir += dir;
    }

    if(move_dir != Vector3(0, 0, 0))
    {
        move_dir.Normalize();
        float speed = 6.0f;
        Vector3 offset = move_dir * speed * GTTime::GetDeltaTime();

        auto agent = anim->GetTransform()->GetParent().lock()->GetGameObject()->GetComponent<NavMeshAgent>();
        agent->Move(offset);
        cam3d->UpdateMatrix();

        anim->GetTransform()->SetForward(move_dir);
    }
#endif

#if DEMO_TERRAIN
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
                        tp->is_world = true;
                        tp->curve.keys.push_back(Keyframe(0, 0, 1, 1));
                        tp->curve.keys.push_back(Keyframe(1, 1, 1, 1));
                        tp->target = GetComponentPtr();
                        tp->on_set_value = OnTweenPositionSetValue;
                        tp->on_finished = OnTweenPositionFinished;
                    }

                    tp->from = pos_old;
                    tp->to = pos_new;
                    tp->duration = move_time;
                }

                {
                    float deg = atan2f(dir.x, dir.z) * Mathf::Rad2Deg;
                    if(deg < 0)
                    {
                        deg += 360;
                    }
                    if(deg > 360)
                    {
                        deg -= 360;
                    }
                    if(anim_rot.y < 0)
                    {
                        anim_rot.y += 360;
                    }
                    if(anim_rot.y > 360)
                    {
                        anim_rot.y -= 360;
                    }
                    if(fabs(anim_rot.y - deg) > 180)
                    {
                        if(anim_rot.y > deg)
                        {
                            anim_rot.y -= 360;
                        }
                        else
                        {
                            anim_rot.y += 360;
                        }
                    }
                    float rot_speed = 270.0f;
                    float rot_time = fabs(anim_rot.y - deg) / rot_speed;

                    auto tr = anim->GetGameObject()->GetComponent<TweenRotation>();
                    if(tr)
                    {
                        tr->Reset();
                    }
                    else
                    {
                        tr = anim->GetGameObject()->AddComponent<TweenRotation>();
                        tr->is_world = true;
                        tr->curve.keys.push_back(Keyframe(0, 0, 1, 1));
                        tr->curve.keys.push_back(Keyframe(1, 1, 1, 1));
                        tr->target = GetComponentPtr();
                        tr->on_set_value = OnTweenRotationSetValue;
                    }

                    tr->from = anim_rot;
                    tr->to = Vector3(0, deg, 0);
                    tr->duration = rot_time;
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
#if DEMO_TERRAIN
    Vector3 pos = anim->GetTransform()->GetPosition();
    cam3d->GetTransform()->SetPosition(pos + cam_offset);
    cam3d->UpdateMatrix();
#endif
}

Launcher::~Launcher()
{
#if DEMO_TERRAIN
    Physics::Done();
#endif
}

#if DEMO_SCENE
static void add_dust_particles(std::shared_ptr<GameObject> &parent, std::shared_ptr<Camera> &cam3d, const Vector3 &local_pos)
{
    auto ps = GameObject::Create("ps")->AddComponent<ParticleSystem>();
    ps->GetTransform()->SetParent(parent->GetTransform());
    ps->GetTransform()->SetLocalPosition(local_pos);
    ps->GetTransform()->SetLocalScale(Vector3(0.4237148f, 12.70362f, 0.3013902f));
    ps->duration = 32;
    ps->start_lifetime = 12;
    ps->start_speed = 0.2f;
    ps->start_size = 32;
    ps->start_color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0, 0));
    ps->start_color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0.3f, 25 / 255.0f));
    ps->start_color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(0, Color(1, 1, 1, 1)));
    ps->start_color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(1, Color(1, 1, 1, 1)));
    ps->emitter_shape = ParticleEmitterShape::Box;
    ps->emitter_shape_box_size = Vector3(25, 1, 39.9f);
    ps->emitter_random_direction = true;
    ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0, 0));
    ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0.471f, 1));
    ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(1, 0));
    ps->color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(0, Color(1, 1, 1, 1)));
    ps->color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(0.994f, Color(10, 18, 38, 255) * (1 / 255.0f)));
    ps->size_curve = AnimationCurve();
    ps->size_curve.keys.push_back(Keyframe(0, 0.5f, 0.5f, 0.5f));
    ps->size_curve.keys.push_back(Keyframe(1, 0.5f, 0.5f, 0.5f));
    ps->angular_velocity = 15;
    ps->max_particles = 1000;

    ps->SetTargetCamera(cam3d);
    auto psr = ps->GetGameObject()->GetComponent<ParticleSystemRenderer>();
    auto psm = Material::Create("Particles/AlphaBlended");
    psm->SetColor("_TintColor", Color(1, 1, 1, 1) * 0.5f);
    auto pst = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/texture/dust.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
    psm->SetMainTexture(pst);
    psr->SetSharedMaterial(psm);
}

static void add_lamp_particles(std::shared_ptr<GameObject> &lamp, std::shared_ptr<Camera> &cam3d)
{
    auto ps = GameObject::Create("lamp")->AddComponent<ParticleSystem>();
    ps->GetTransform()->SetParent(lamp->GetTransform());
    ps->GetTransform()->SetLocalPosition(Vector3(0.008f, 0.002f, -0.883f));
    ps->GetTransform()->SetLocalRotation(Quaternion::Euler(0.0001373291f, -90, -90));
    ps->duration = 5;
    ps->start_lifetime = 5;
    ps->start_speed = 0.01f;
    ps->start_size = 7;
    ps->max_particles = 1000;
    ps->emission_rate = 1;
    ps->emitter_shape = ParticleEmitterShape::Disable;
    ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0, 0));
    ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0.215f, 39 / 255.0f));
    ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0.765f, 69 / 255.0f));
    ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(1, 0));
    ps->color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(0, Color(152, 66, 0, 255) * (1.0f / 255)));
    ps->color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(1, Color(126, 81, 0, 255) * (1.0f / 255)));
    ps->size_curve = AnimationCurve();
    ps->size_curve.keys.push_back(Keyframe(0, 1, 0, 0));
    ps->size_curve.keys.push_back(Keyframe(0.235f, 0.212f, 0, 0));
    ps->size_curve.keys.push_back(Keyframe(0.458f, 0.995f, 0, 0));
    ps->size_curve.keys.push_back(Keyframe(0.744f, 0.46f, 0, 0));
    ps->size_curve.keys.push_back(Keyframe(1, 1, 0, 0));

    ps->SetTargetCamera(cam3d);
    auto psr = ps->GetGameObject()->GetComponent<ParticleSystemRenderer>();
    auto psm = Material::Create("Particles/AlphaBlendedPremultiply");
    auto pst = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/mesh/scene/particle.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
    psm->SetMainTexture(pst);
    psr->SetSharedMaterial(psm);
}

static void add_fire_particles(std::shared_ptr<GameObject> &fire, std::shared_ptr<Camera> &cam3d)
{
    {
        auto ps = GameObject::Create("fire")->AddComponent<ParticleSystem>();
        ps->GetTransform()->SetParent(fire->GetTransform());
        ps->GetTransform()->SetLocalPosition(Vector3(0, 0, 0.9835533f));
        ps->GetTransform()->SetLocalRotation(Quaternion::Euler(0, 0, 0));
        ps->duration = 5;
        ps->start_lifetime = 5;
        ps->start_speed = 0;
        ps->start_size = 0.7f;
        ps->start_color = Color(255, 255, 255, 255) * (1.0f / 255);
        ps->max_particles = 1;
        ps->emission_rate = 10;
        ps->emitter_shape = ParticleEmitterShape::Sphere;
        ps->emitter_shape_sphere_radius = 0.01f;
        ps->enable_texture_sheet_animation = true;
        ps->texture_sheet_animation_tile_x = 16;
        ps->texture_sheet_animation_tile_y = 4;
        ps->texture_sheet_animation_single_row = false;
        ps->texture_sheet_animation_frame_curve = AnimationCurve();
        ps->texture_sheet_animation_frame_curve.keys.push_back(Keyframe(0, 0, 64, 64));
        ps->texture_sheet_animation_frame_curve.keys.push_back(Keyframe(1, 64, 64, 64));
        ps->texture_sheet_animation_cycles = 4;

        ps->SetTargetCamera(cam3d);
        auto psr = ps->GetGameObject()->GetComponent<ParticleSystemRenderer>();
        auto psm = Material::Create("Particles/AlphaBlended");
        psm->SetColor("_TintColor", Color(255, 175, 152, 128) * (1.0f / 255));
        auto pst = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/mesh/scene/fire.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
        psm->SetMainTexture(pst);
        psr->SetSharedMaterial(psm);
    }

    {
        auto ps = GameObject::Create("smoke")->AddComponent<ParticleSystem>();
        ps->GetTransform()->SetParent(fire->GetTransform());
        ps->GetTransform()->SetLocalPosition(Vector3(0, 0, -0.8561009f));
        ps->GetTransform()->SetLocalRotation(Quaternion::Euler(0, 0, 0));
        ps->duration = 5;
        ps->start_lifetime = 5;
        ps->start_speed = 0.85f;
        ps->start_size = 2.56f;
        ps->max_particles = 1000;
        ps->emission_rate = 3.6f;
        ps->emitter_shape = ParticleEmitterShape::Cone;
        ps->emitter_shape_cone_angle = 6.42f;
        ps->emitter_shape_cone_radius = 0.2f;
        ps->emitter_shape_cone_from = EmitterShapeConeFrom::Base;
        ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0, 0));
        ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0.218f, 25 / 255.0f));
        ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0.826f, 39 / 255.0f));
        ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(1, 0));
        ps->color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(0, Color(1, 1, 1, 1)));
        ps->color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(1, Color(1, 1, 1, 1)));
        ps->size_curve = AnimationCurve();
        ps->size_curve.keys.push_back(Keyframe(0, 0.03f, 0, 0));
        ps->size_curve.keys.push_back(Keyframe(1, 1, 0, 0));
        ps->angular_velocity = 45;

        ps->SetTargetCamera(cam3d);
        auto psr = ps->GetGameObject()->GetComponent<ParticleSystemRenderer>();
        auto psm = Material::Create("Particles/Multiply");
        auto pst = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/mesh/scene/smoke.jpg", FilterMode::Bilinear, TextureWrapMode::Clamp);
        psm->SetMainTexture(pst);
        psr->SetSharedMaterial(psm);
    }

    {
        auto ps = GameObject::Create("spark")->AddComponent<ParticleSystem>();
        ps->GetTransform()->SetParent(fire->GetTransform());
        ps->GetTransform()->SetLocalPosition(Vector3(0, 0, 0.7186908f));
        ps->GetTransform()->SetLocalRotation(Quaternion::Euler(0, 0, 0));
        ps->duration = 1;
        ps->start_lifetime_type = ValueType::RandomConstants;
        ps->start_lifetime_random_contants = Vector2(0.1f, 0.35f);
        ps->start_speed_type = ValueType::RandomConstants;
        ps->start_speed_random_contants = Vector2(2, 5);
        ps->start_size = 0.14f;
        ps->start_color = Color(255, 192, 56, 255) * (1.0f / 255);
        ps->max_particles = 1000;
        ps->emission_rate = 30;
        ps->emitter_shape = ParticleEmitterShape::Cone;
        ps->emitter_shape_cone_angle = 25;
        ps->emitter_shape_cone_radius = 0.43f;
        ps->emitter_shape_cone_from = EmitterShapeConeFrom::Base;
        ps->emitter_random_direction = true;
        ps->velocity_type = ValueType::Constant;
        ps->velocity = Vector3(-0.66f, -1.11f, 0.17f);
        ps->force_type = ValueType::Constant;
        ps->force = Vector3(4, 0.03f, 1.32f);
        ps->size_curve = AnimationCurve();
        ps->size_curve.keys.push_back(Keyframe(0, 0.197f, 0, 0));
        ps->size_curve.keys.push_back(Keyframe(0.946f, 0.409f, 0, 0));

        ps->SetTargetCamera(cam3d);
        auto psr = ps->GetGameObject()->GetComponent<ParticleSystemRenderer>();
        psr->render_mode = ParticleSystemRenderMode::Stretch;
        psr->stretch_speed_scale = 0;
        psr->stretch_length_scale = 7.55f;
        auto psm = Material::Create("Particles/Additive");
        psm->SetColor("_TintColor", Color(1, 1, 1, 0.5f));
        auto pst = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/mesh/scene/spark.jpg", FilterMode::Bilinear, TextureWrapMode::Clamp);
        psm->SetMainTexture(pst);
        psr->SetSharedMaterial(psm);
    }

    {
        auto ps = GameObject::Create("halo")->AddComponent<ParticleSystem>();
        ps->GetTransform()->SetParent(fire->GetTransform());
        ps->GetTransform()->SetLocalPosition(Vector3(0, 0, 1.146898f));
        ps->GetTransform()->SetLocalRotation(Quaternion::Euler(90, 0, 0));
        ps->duration = 5;
        ps->start_lifetime = 5;
        ps->start_speed = 0.01f;
        ps->start_size = 5;
        ps->max_particles = 1000;
        ps->emission_rate = 1;
        ps->emitter_shape = ParticleEmitterShape::Disable;
        ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0, 0));
        ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0.215f, 39 / 255.0f));
        ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(0.765f, 69 / 255.0f));
        ps->color_gradient.key_alphas.push_back(ColorGradient::KeyAlpha(1, 0));
        ps->color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(0, Color(152, 66, 0, 255) * (1.0f / 255)));
        ps->color_gradient.key_rgbs.push_back(ColorGradient::KeyRGB(1, Color(126, 81, 0, 255) * (1.0f / 255)));
        ps->size_curve = AnimationCurve();
        ps->size_curve.keys.push_back(Keyframe(0, 1, 0, 0));
        ps->size_curve.keys.push_back(Keyframe(0.235f, 0.212f, 0, 0));
        ps->size_curve.keys.push_back(Keyframe(0.458f, 0.995f, 0, 0));
        ps->size_curve.keys.push_back(Keyframe(0.744f, 0.46f, 0, 0));
        ps->size_curve.keys.push_back(Keyframe(1, 1, 0, 0));

        ps->SetTargetCamera(cam3d);
        auto psr = ps->GetGameObject()->GetComponent<ParticleSystemRenderer>();
        auto psm = Material::Create("Particles/AlphaBlendedPremultiply");
        auto pst = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/mesh/scene/particle.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
        psm->SetMainTexture(pst);
        psr->SetSharedMaterial(psm);
    }
}
#endif
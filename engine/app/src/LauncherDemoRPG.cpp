#include "LauncherDemoRPG.h"
#include "LauncherDemoUI.h"

using namespace Galaxy3D;

static float g_unit_per_pixel = 0.01f;
int g_key_down_count = 0;
static bool g_key_down[4] = {false};
Vector3 g_mouse_down_pos;
bool g_mouse_down = false;
Vector3 g_cam_rot;
float g_cam_dis;
static bool g_init_pos = false;

void LauncherDemoRPG::Start()
{
    auto ui = GameObject::Create("ui")->AddComponent<LauncherDemoUI>();

    auto light = GameObject::Create("light")->AddComponent<Light>();
    light->GetTransform()->SetRotation(Quaternion::Euler(50, -150, 0));
    light->SetType(LightType::Directional);
    light->SetColor(Color(252, 181, 89, 255) / 255.0f);
    light->SetIntensity(3.4f);
    light->EnableShadow(true);
    light->EnableCascade(true);

    RenderSettings::light_ambient = Color(67, 128, 200, 255) / 255.0f;
    RenderSettings::SetGlobalDirectionalLight(light);
    std::vector<float> cascade_splits;
    cascade_splits.push_back(0.08f);
    cascade_splits.push_back(0.20f);
    cascade_splits.push_back(0.50f);
    Light::SetCascadeSplits(cascade_splits);

    cam3d = GameObject::Create("camera")->AddComponent<Camera>();
    cam3d->SetFieldOfView(45);
    cam3d->SetClipPlane(0.1f, 200.0f);
    cam3d->SetCullingMask(
        LayerMask::GetMask(Layer::Default) |
        LayerMask::GetMask(Layer::Scene) |
        LayerMask::GetMask(Layer::Terrain) |
        LayerMask::GetMask(Layer::Character));
    cam3d->EnableDeferredShading(true);

    std::vector<std::string> terrain_texs;
    terrain_texs.push_back(Application::GetDataPath() + "/Assets/terrain/t1/0.png");
    terrain_texs.push_back(Application::GetDataPath() + "/Assets/terrain/t1/1.png");
    terrain_texs.push_back(Application::GetDataPath() + "/Assets/terrain/t1/2.png");
    terrain_texs.push_back(Application::GetDataPath() + "/Assets/terrain/t1/3.png");
    terrain_texs.push_back(Application::GetDataPath() + "/Assets/terrain/t1/4.png");

    GameObject *terrain_obj = GameObject::Create("terrain").get();
    terrain_obj->SetLayer(Layer::Terrain);

    auto ter = terrain_obj->AddComponent<Terrain>();
    ter->SetCamera(cam3d);
    ter->LoadData(
        257,
        301, 120,
        Application::GetDataPath() + "/Assets/terrain/t1/Terrain.raw",
        Application::GetDataPath() + "/Assets/terrain/t1/Terrain.png",
        terrain_texs, 3);
    auto terrain_renderer = terrain_obj->AddComponent<TerrainRenderer>();
    terrain_renderer->SetCastShadow(false);
    auto tc = terrain_obj->AddComponent<TerrainCollider>();
    tc->SetTerrain(ter);

    /*
    auto scene = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/terrain/t1/static mesh/static mesh.mesh");
    scene->SetLayerRecursively(Layer::Scene);
    auto mrs = scene->GetComponentsInChildren<MeshRenderer>();
    for(auto &i : mrs)
    {
        auto m = i->GetMesh();
        if(m)
        {
            auto c = i->GetGameObject()->AddComponent<MeshCollider>();
            c->SetMesh(m);
        }
    }
    
    Renderer::BuildOctree(scene);
    scene->SetStaticRecursively();
    Renderer::BuildStaticBatches();
    
    auto static_skin = Mesh::LoadSkinnedMesh(Application::GetDataPath() + "/Assets/terrain/t1/skinned mesh/monster550.anim");
    static_skin->GetTransform()->SetPosition(Vector3(145.2373f, 56.40065f, 84.6097f));
    static_skin->GetTransform()->SetRotation(Quaternion::Euler(0, -79.45871f, 0));
    auto static_skin_anim = static_skin->GetComponent<Animation>();
    static_skin_anim->GetAnimationState("monster550")->wrap_mode = WrapMode::Loop;
    static_skin_anim->Play("monster550");

    static_skin = GameObject::Instantiate(static_skin);
    static_skin->GetTransform()->SetPosition(Vector3(144.8962f, 56.25666f, 82.47664f));
    static_skin->GetTransform()->SetRotation(Quaternion::Euler(0, -159.8306f, 0));
    auto static_skin_anim_state = static_skin_anim->GetAnimationState("monster550");
    static_skin_anim_state->time = Mathf::RandomRange(0.0f, static_skin_anim_state->length);
    static_skin->GetComponent<Animation>()->Play("monster550");

    static_skin = GameObject::Instantiate(static_skin);
    static_skin->GetTransform()->SetPosition(Vector3(146.788f, 56.25666f, 88.28624f));
    static_skin->GetTransform()->SetRotation(Quaternion::Euler(0, -4.007324f, 0));
    static_skin_anim_state = static_skin_anim->GetAnimationState("monster550");
    static_skin_anim_state->time = Mathf::RandomRange(0.0f, static_skin_anim_state->length);
    static_skin->GetComponent<Animation>()->Play("monster550");

    static_skin = Mesh::LoadSkinnedMesh(Application::GetDataPath() + "/Assets/terrain/t1/skinned mesh/qizi6.anim");
    static_skin->GetTransform()->SetPosition(Vector3(129.9555f, 59.13635f, 101.2854f));
    static_skin->GetTransform()->SetRotation(Quaternion::Euler(0, 180.0f, 0));
    static_skin_anim = static_skin->GetComponent<Animation>();
    static_skin_anim->GetAnimationState("c")->wrap_mode = WrapMode::Loop;
    static_skin_anim->Play("c");

    static_skin = GameObject::Instantiate(static_skin);
    static_skin->GetTransform()->SetPosition(Vector3(129.9555f, 59.13635f, 118.4536f));
    static_skin->GetTransform()->SetRotation(Quaternion::Euler(0, 180.0f, 0));
    static_skin_anim_state = static_skin_anim->GetAnimationState("c");
    static_skin_anim_state->time = Mathf::RandomRange(0.0f, static_skin_anim_state->length);
    static_skin->GetComponent<Animation>()->Play("c");

    static_skin = Mesh::LoadSkinnedMesh(Application::GetDataPath() + "/Assets/terrain/t1/skinned mesh/qizi7.anim");
    static_skin->GetTransform()->SetPosition(Vector3(110.3188f, 59.17264f, 116.552f));
    static_skin->GetTransform()->SetRotation(Quaternion::Euler(0, 180.0f, 0));
    static_skin_anim = static_skin->GetComponent<Animation>();
    static_skin_anim->GetAnimationState("S1_qizi07_zm_piao")->wrap_mode = WrapMode::Loop;
    static_skin_anim->Play("S1_qizi07_zm_piao");

    static_skin = GameObject::Instantiate(static_skin);
    static_skin->GetTransform()->SetPosition(Vector3(110.3188f, 59.17264f, 104.2874f));
    static_skin->GetTransform()->SetRotation(Quaternion::Euler(0, 180.0f, 0));
    static_skin_anim_state = static_skin_anim->GetAnimationState("S1_qizi07_zm_piao");
    static_skin_anim_state->time = Mathf::RandomRange(0.0f, static_skin_anim_state->length);
    static_skin->GetComponent<Animation>()->Play("S1_qizi07_zm_piao");

    static_skin = Mesh::LoadSkinnedMesh(Application::GetDataPath() + "/Assets/terrain/t1/skinned mesh/wujian.anim");
    static_skin->GetTransform()->SetPosition(Vector3(144.9908f, 58.20395f, 106.3395f));
    static_skin->GetTransform()->SetRotation(Quaternion::Euler(0, -90.0f, 0));
    static_skin_anim = static_skin->GetComponent<Animation>();
    static_skin_anim->GetAnimationState("cS1_wujian01_zm_piao")->wrap_mode = WrapMode::Loop;
    static_skin_anim->Play("cS1_wujian01_zm_piao");

    static_skin = GameObject::Instantiate(static_skin);
    static_skin->GetTransform()->SetPosition(Vector3(129.9706f, 57.00055f, 138.3009f));
    static_skin->GetTransform()->SetRotation(Quaternion::Euler(0, 90.0f, 0));
    static_skin_anim_state = static_skin_anim->GetAnimationState("cS1_wujian01_zm_piao");
    static_skin_anim_state->time = Mathf::RandomRange(0.0f, static_skin_anim_state->length);
    static_skin->GetComponent<Animation>()->Play("cS1_wujian01_zm_piao");
    */

    ui->cam3d = cam3d;

    auto fog = cam3d->GetGameObject()->AddComponent<ImageEffectGlobalFog>();
    fog->EnableHeight(false);
    fog->SetFogMode(FogMode::ExponentialSquared);
    fog->SetFogColor(Color(1, 1, 1, 1) * 0.5f);
    fog->SetFogDensity(0.02f);
    fog->ExcludeFarPixels(false);

    auto sky = cam3d->GetGameObject()->AddComponent<SkyBox>();
    std::vector<std::string> sky_textures;
    sky_textures.push_back(Application::GetDataPath() + "/Assets/texture/skybox/left.png");
    sky_textures.push_back(Application::GetDataPath() + "/Assets/texture/skybox/right.png");
    sky_textures.push_back(Application::GetDataPath() + "/Assets/texture/skybox/up.png");
    sky_textures.push_back(Application::GetDataPath() + "/Assets/texture/skybox/down.png");
    sky_textures.push_back(Application::GetDataPath() + "/Assets/texture/skybox/front.png");
    sky_textures.push_back(Application::GetDataPath() + "/Assets/texture/skybox/back.png");
    sky->SetCubemap(Cubemap::LoadFromFile(sky_textures));

    auto anim_parent = GameObject::Create("anim_parent");
    anim_parent->GetTransform()->SetPosition(Vector3(145.27f, 51.901f, 163.55f));

    auto anim_obj = Mesh::LoadSkinnedMesh(Application::GetDataPath() + "/Assets/mesh/anim/xiao_bie_li/xiao_bie_li.anim");
    anim_obj->GetTransform()->SetParent(anim_parent->GetTransform());
    anim_obj->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
    anim_obj->GetTransform()->SetLocalRotation(Quaternion::Euler(Vector3(0, 180, 0)));
    anim_obj->SetLayerRecursively(Layer::Character);
    anim = anim_obj->GetComponent<Animation>();
    anim->GetAnimationState("idle")->wrap_mode = WrapMode::Loop;
    anim->GetAnimationState("run")->wrap_mode = WrapMode::Loop;
    anim->Play("idle");
    auto rs = anim->GetGameObject()->GetComponentsInChildren<SkinnedMeshRenderer>();
    for(auto &i : rs)
    {
        i->GetSharedMaterial()->SetMainColor(Color(150, 150, 150, 255) / 255.0f);
    }

    cam3d->GetTransform()->SetParent(anim_parent->GetTransform());
    g_cam_rot = Vector3(10, -180, 0);
    cam3d->GetTransform()->SetLocalRotation(Quaternion::Euler(g_cam_rot));
    g_cam_dis = 12;
    cam3d->GetTransform()->SetLocalPosition(Vector3(0, 1.5f, 0) - cam3d->GetTransform()->GetForward() * g_cam_dis);

    cc = anim_parent->AddComponent<CharacterController>();

    // cursor
    Cursor::Load(Application::GetDataPath() + "/Assets/texture/cursor/Cursor.cur", 0);//normal
    Cursor::Load(Application::GetDataPath() + "/Assets/texture/cursor/Battle2.cur", 1);//attack
    Cursor::SetCursor(0);
}

static void move_key_down(std::shared_ptr<Animation> &anim, int index)
{
    g_key_down[index] = true;
    g_key_down_count++;

    if(g_key_down_count == 1)
    {
        anim->CrossFade("run");
    }
}

static void move_key_up(std::shared_ptr<Animation> &anim, int index)
{
    if(g_key_down[index])
    {
        g_key_down[index] = false;
        g_key_down_count--;

        if(g_key_down_count == 0)
        {
            anim->CrossFade("idle");
        }
    }
}

static Vector3 drag_cam_rot(std::shared_ptr<Camera> &cam3d)
{
    Vector3 mouse_pos = Input::GetMousePosition();
    Vector3 offset = mouse_pos - g_mouse_down_pos;
    Vector2 rot_scal(0.3f, 0.3f);
    Vector3 rot_offset = Vector3(-offset.y * rot_scal.x, offset.x * rot_scal.y, 0);
    Vector3 rot = g_cam_rot + rot_offset;
    rot.x = Mathf::Clamp(rot.x, -85.0f, 85.0f);
    rot_offset = rot - g_cam_rot;
    offset.y = - rot_offset.x / rot_scal.x;
    g_mouse_down_pos.y = mouse_pos.y - offset.y;

    cam3d->GetTransform()->SetLocalRotation(Quaternion::Euler(rot));
    cam3d->GetTransform()->SetLocalPosition(Vector3(0, 1.5f, 0) - cam3d->GetTransform()->GetForward() * g_cam_dis);

    Vector3 cam_target = cam3d->GetTransform()->GetPosition() + cam3d->GetTransform()->GetForward() * g_cam_dis;

    auto hits = Physics::RaycastAll(cam_target, -cam3d->GetTransform()->GetForward(), g_cam_dis, LayerMask::GetMask(Layer::Terrain));
    if(!hits.empty())
    {
        cam3d->GetTransform()->SetPosition(Vector3::Lerp(hits[0].point, cam_target, 0.1f / 1.5f));
    }

    return rot_offset;
}

void LauncherDemoRPG::Update()
{
    int key_down_count_old = g_key_down_count;

    if(Input::GetKeyDown(KeyCode::W)) move_key_down(anim, 0);
    if(Input::GetKeyDown(KeyCode::S)) move_key_down(anim, 1);
    if(Input::GetKeyDown(KeyCode::A)) move_key_down(anim, 2);
    if(Input::GetKeyDown(KeyCode::D)) move_key_down(anim, 3);

    if(Input::GetKeyUp(KeyCode::W)) move_key_up(anim, 0);
    if(Input::GetKeyUp(KeyCode::S)) move_key_up(anim, 1);
    if(Input::GetKeyUp(KeyCode::A)) move_key_up(anim, 2);
    if(Input::GetKeyUp(KeyCode::D)) move_key_up(anim, 3);

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

    if(!g_init_pos)
    {
        g_init_pos = true;

        auto agent = anim->GetTransform()->GetParent().lock()->GetGameObject();
        Vector3 from = agent->GetTransform()->GetPosition() + Vector3(0, 100, 0);
        auto hits = Physics::RaycastAll(from, Vector3(0, -1, 0), 200, LayerMask::GetMask(Layer::Terrain));
        if(!hits.empty())
        {
            agent->GetTransform()->SetPosition(hits[0].point);
        }
    }

    if(move_dir != Vector3(0, 0, 0))
    {
        move_dir.Normalize();
        float speed = 11;
        Vector3 offset = move_dir * speed * (1.0f / 60);
        cc->Move(offset);

        anim->GetTransform()->SetForward(move_dir);
    }

    if(g_key_down_count == 0)
    {
        cc->Move(Vector3(0, 0, 0));
    }

    if(Input::GetMouseButtonDown(1))
    {
        g_mouse_down = true;
        g_mouse_down_pos = Input::GetMousePosition();
    }

    if(Input::GetMouseButton(1))
    {
        drag_cam_rot(cam3d);
    }

    if(Input::GetMouseButtonUp(1))
    {
        auto rot_offset = drag_cam_rot(cam3d);

        g_cam_rot = g_cam_rot + rot_offset;
        g_mouse_down = false;
    }

    bool set_cursor = false;
    if(!g_mouse_down)
    {
        auto ray = cam3d->ScreenPointToRay(Input::GetMousePosition());

        auto hits = Physics::RaycastAll(ray.origin, ray.GetDirection(), 1000, LayerMask::GetMask(Layer::Character));
        if(!hits.empty())
        {
            auto c =  hits[0].collider.lock();
            if(c && c->GetGameObject() == anim->GetGameObject())
            {
                Cursor::SetCursor(1);
                set_cursor = true;
            }
        }
    }

    if(!set_cursor)
    {
        if(Cursor::GetCurent() != 0)
        {
            Cursor::SetCursor(0);
        }
    }
}
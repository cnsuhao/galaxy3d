#include "LauncherDemoDef.h"

using namespace Galaxy3D;

static float g_unit_per_pixel = 0.01f;
static int g_key_down_count = 0;
static bool g_key_down[4] = {false};
static Vector3 g_mouse_down_pos;
static bool g_mouse_down = false;
static Vector3 g_cam_rot;
static float g_cam_dis;

void LauncherDemoDef::Start()
{
    Label::LoadFont("consola", Application::GetDataPath() + "/Assets/font/consola.ttf");
    Label::LoadFont("heiti", Application::GetDataPath() + "/Assets/font/heiti.ttc");

    cam2d = GameObject::Create("camera")->AddComponent<Camera>();
    cam2d->SetOrthographic(true);
    cam2d->SetOrthographicSize(g_unit_per_pixel * Screen::GetHeight() / 2);
    cam2d->SetClipPlane(-1, 1);
    cam2d->SetCullingMask(LayerMask::GetMask(Layer::UI));
    cam2d->SetDepth(1);
    cam2d->SetClearFlags(CameraClearFlags::Nothing);
    cam2d->SetClearColor(Color(0.3f, 0.3f, 0.3f, 1));

    auto canvas = GameObject::Create("")->AddComponent<UICanvas>();
    canvas->GetTransform()->SetParent(cam2d->GetTransform());
    canvas->GetTransform()->SetScale(Vector3(1, 1, 1) * g_unit_per_pixel);

    auto label = Label::Create("", "heiti", 20, LabelPivot::Top, LabelAlign::Auto, true);
    auto tr = GameObject::Create("fps")->AddComponent<TextRenderer>();
    tr->SetLabel(label);
    tr->SetSortingOrder(1000, 0);
    tr->GetTransform()->SetParent(canvas->GetTransform());
    tr->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    tr->SetAnchor(Vector4(0.5f, 0, 0, 0));
    fps = tr;

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
    cam3d->SetClipPlane(0.1f, 100.0f);
    cam3d->SetCullingMask(LayerMask::GetMask(Layer::Default) | LayerMask::GetMask(Layer::Highlighting));
    cam3d->EnableDeferredShading(true);
    auto high_lihgting = cam3d->GetGameObject()->AddComponent<ImageEffectHighlighting>();

    /*
    auto fog = cam3d->GetGameObject()->AddComponent<ImageEffectGlobalFog>();
    fog->EnableHeight(false);
    fog->SetFogMode(FogMode::ExponentialSquared);
    fog->SetFogColor(Color(1, 1, 1, 1) * 0.5f);
    fog->SetFogDensity(0.02f);
    fog->ExcludeFarPixels(true);
    */

    auto sky = cam3d->GetGameObject()->AddComponent<SkyBox>();
    std::vector<std::string> sky_textures;
    sky_textures.push_back(Application::GetDataPath() + "/Assets/texture/skybox/left.png");
    sky_textures.push_back(Application::GetDataPath() + "/Assets/texture/skybox/right.png");
    sky_textures.push_back(Application::GetDataPath() + "/Assets/texture/skybox/up.png");
    sky_textures.push_back(Application::GetDataPath() + "/Assets/texture/skybox/down.png");
    sky_textures.push_back(Application::GetDataPath() + "/Assets/texture/skybox/front.png");
    sky_textures.push_back(Application::GetDataPath() + "/Assets/texture/skybox/back.png");
    sky->SetCubemap(Cubemap::LoadFromFile(sky_textures));

    auto mesh = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/LY/LY-1.mesh");

    auto anim_parent = GameObject::Create("anim_parent");
    auto anim_obj = Mesh::LoadSkinnedMesh(Application::GetDataPath() + "/Assets/mesh/anim/Warrior/warrior.anim");
    anim_obj->GetTransform()->SetParent(anim_parent->GetTransform());
    anim_obj->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
    anim_obj->GetTransform()->SetLocalRotation(Quaternion::Euler(Vector3(0, 180, 0)));
    anim_obj->SetLayerRecursively(Layer::Default);
    anim = anim_obj->GetComponent<Animation>();
    anim->GetAnimationState("idle")->wrap_mode = WrapMode::Loop;
    anim->GetAnimationState("run")->wrap_mode = WrapMode::Loop;
    anim->Play("idle");
    {
        anim_obj->SetLayerRecursively(Layer::Highlighting);
        auto rs = anim_obj->GetComponentsInChildren<SkinnedMeshRenderer>();
        for(auto &i : rs)
        {
            auto ho = i->GetGameObject()->AddComponent<HighlightingObject>();
            ho->SetColor(Color(0, 0, 0, 0.005f));
        }
    }

    {
        auto clone_obj = GameObject::Instantiate(anim_obj);
        clone_obj->GetTransform()->SetPosition(Vector3(-3, 0, 0));
        clone_obj->SetLayerRecursively(Layer::Highlighting);
        auto clone_anim = clone_obj->GetComponent<Animation>();
        clone_anim->Play("idle");
        clone_obj->AddComponent<NavMeshAgent>();

        auto rs = clone_obj->GetComponentsInChildren<SkinnedMeshRenderer>();
        for(auto &i : rs)
        {
            auto ho = i->GetGameObject()->AddComponent<HighlightingObject>();
            ho->SetColor(Color(1, 0, 0, 1));
        }
    }
    {
        auto clone_obj = GameObject::Instantiate(anim_obj);
        clone_obj->GetTransform()->SetPosition(Vector3(-6, 0, 0));
        clone_obj->SetLayerRecursively(Layer::Highlighting);
        auto clone_anim = clone_obj->GetComponent<Animation>();
        clone_anim->Play("idle");
        clone_obj->AddComponent<NavMeshAgent>();

        auto rs = clone_obj->GetComponentsInChildren<SkinnedMeshRenderer>();
        for(auto &i : rs)
        {
            auto ho = i->GetGameObject()->AddComponent<HighlightingObject>();
            ho->SetColor(Color(0, 1, 0, 1));
        }
    }

    cam3d->GetTransform()->SetParent(anim_parent->GetTransform());
    g_cam_rot = Vector3(10, -180, 0);
    cam3d->GetTransform()->SetLocalRotation(Quaternion::Euler(g_cam_rot));
    g_cam_dis = 12;
    cam3d->GetTransform()->SetLocalPosition(Vector3(0, 1.5f, 0) - cam3d->GetTransform()->GetForward() * g_cam_dis);

    // navmesh
    NavMesh::LoadFromFile(Application::GetDataPath() + "/Assets/mesh/LY/navmesh.nav");
    anim_parent->GetTransform()->SetPosition(Vector3(1, 0, 0));
    anim_parent->AddComponent<NavMeshAgent>();

    // collider
    auto rs = mesh->GetComponentsInChildren<MeshRenderer>();
    for(auto i : rs)
    {
        auto c = i->GetGameObject()->AddComponent<MeshCollider>();
        c->SetMesh(i->GetMesh());
    }

    auto bc = anim_obj->AddComponent<BoxCollider>();
    bc->SetCenter(Vector3(0, 1, 0));
    bc->SetSize(Vector3(1, 2, 1));

    // cursor
    Cursor::Load(Application::GetDataPath() + "/Assets/texture/cursor/Cursor.cur", 0);
    Cursor::Load(Application::GetDataPath() + "/Assets/texture/cursor/Battle2.cur", 1);
    Cursor::SetCursor(0);

    cam2d->GetGameObject()->SetLayerRecursively(Layer::UI);
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

    RaycastHit hit;
    if(Physics::Raycast(cam_target, -cam3d->GetTransform()->GetForward(), g_cam_dis, hit))
    {
        cam3d->GetTransform()->SetPosition(Vector3::Lerp(hit.point, cam_target, 0.1f / 1.5f));
    }

    return rot_offset;
}

void LauncherDemoDef::Update()
{
    fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::GetFPS()).str + "\n" +
        "drawcall:" + GTString::ToString(GTTime::GetDrawCall()).str);

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

    if(move_dir != Vector3(0, 0, 0))
    {
        move_dir.Normalize();
        float speed = 8.0f;
        Vector3 offset = move_dir * speed * GTTime::GetDeltaTime();

        auto agent = anim->GetTransform()->GetParent().lock()->GetGameObject()->GetComponent<NavMeshAgent>();
        agent->Move(offset);

        anim->GetTransform()->SetForward(move_dir);
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

        RaycastHit hit;
        if(Physics::Raycast(ray.origin, ray.GetDirection(), 1000, hit))
        {
            if(hit.collider.lock()->GetGameObject() == anim->GetGameObject())
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
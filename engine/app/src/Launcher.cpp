#include "Launcher.h"

#define DEMO_TERRAIN 0
#define DEMO_SCENE 0
#define DEMO_DEFERRED_SHADING 0
#define DEMO_DEF 0
#define DEMO_REWARD 0
#define DEMO_UI 1

using namespace Galaxy3D;

#if DEMO_DEF
int g_key_down_count = 0;
static bool g_key_down[4] = {false};
Vector3 g_mouse_down_pos;
bool g_mouse_down = false;
Vector3 g_cam_rot;
float g_cam_dis;
#endif

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

#if DEMO_REWARD
TextRenderer *g_reward = NULL;
TextRenderer *g_name = NULL;
SpriteRenderer *g_bg = NULL;
SpriteRenderer *g_state = NULL;
SpriteNode *g_stop = NULL;
SpriteNode *g_next = NULL;
SpriteRenderer *g_image = NULL;
std::vector<GTString> g_reward_name;
std::vector<int> g_reward_count;
std::vector<std::string> g_reward_image;
std::vector<std::string> g_name_txt;
std::vector<std::string> g_name_temp;
std::vector<std::string> g_name_rand;
std::vector<std::vector<std::string>> g_result;
int g_reward_index = 0;
bool g_rand = true;
int g_font_size = 100;
bool g_start = false;
#endif

void Launcher::Start()
{
	Label::LoadFont("heiti", Application::GetDataPath() + "/Assets/font/heiti.ttc");
    
    cam2d = GameObject::Create("camera")->AddComponent<Camera>();
    cam2d->SetOrthographic(true);
    cam2d->SetOrthographicSize(Screen::GetHeight() / 200.f);
    cam2d->SetClipPlane(-1, 1);
    cam2d->SetCullingMask(LayerMask::GetMask(Layer::UI));
    cam2d->SetDepth(1);
    cam2d->SetClearFlags(CameraClearFlags::Nothing);

	auto label = Label::Create("", "heiti", 20, LabelPivot::LeftTop, LabelAlign::Auto, true);
	auto tr = GameObject::Create("label")->AddComponent<TextRenderer>();
	tr->GetTransform()->SetPosition(Vector3(-Screen::GetWidth()/2.0f, Screen::GetHeight()/2.0f, 0) * 0.01f);
	tr->SetLabel(label);
	tr->SetSortingOrder(1000, 0);
    tr->GetTransform()->SetParent(cam2d->GetTransform());
    tr->GetGameObject()->SetLayer(Layer::UI);
	fps = tr;

#if DEMO_UI
    cam2d->SetClearFlags(CameraClearFlags::SolidColor);
    
    auto button_sprite = Sprite::Create(
        Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/texture/ui/NormalButton_Normal.png"),
        Rect(0, 0, 126, 50),
        Vector2(0.5f, 0.5f),
        100,
        Vector4(8, 10, 8, 8),
        Sprite::Type::Simple,
        Vector2(126, 50));

    auto button_sr = GameObject::Create("")->AddComponent<SpriteRenderer>();
    button_sr->GetGameObject()->SetLayer(Layer::UI);
    button_sr->SetSprite(button_sprite);

    /*
    auto node = GameObject::Create("")->AddComponent<SpriteNode>();
    node->SetSprite(button_sprite);
    auto button_sr = GameObject::Create("")->AddComponent<SpriteBatchRenderer>();
    button_sr->GetGameObject()->SetLayer(Layer::UI);
    button_sr->AddSprite(node);
    button_sr->UpdateSprites();
    */
#endif

#if DEMO_REWARD
    cam2d->SetClearFlags(CameraClearFlags::SolidColor);

    if(Screen::GetWidth() != 1920 || Screen::GetHeight() != 1080)
    {
        auto screen = RenderTexture::Create(1920, 1080, RenderTextureFormat::RGBA32, DepthBuffer::Depth_0, FilterMode::Trilinear);
        cam2d->SetRenderTexture(screen);
        cam2d->SetOrthographicSize(1080 / 200.f);

        auto cam2d_screen = GameObject::Create("")->AddComponent<Camera>();
        cam2d_screen->SetOrthographic(true);
        cam2d_screen->SetOrthographicSize(Screen::GetHeight() / 200.f);
        cam2d_screen->SetClipPlane(-1, 1);
        cam2d_screen->SetCullingMask(LayerMask::GetMask(Layer::Default));
        cam2d_screen->SetDepth(2);
        cam2d_screen->SetClearFlags(CameraClearFlags::SolidColor);

        auto screen_sprite = Sprite::Create(screen);
        auto screen_sr = GameObject::Create("bg")->AddComponent<SpriteRenderer>();
        screen_sr->GetGameObject()->SetLayer(Layer::Default);
        screen_sr->SetSprite(screen_sprite);
        float x = Screen::GetWidth() / (float) screen_sprite->GetTexture()->GetWidth();
        float y = Screen::GetHeight() / (float) screen_sprite->GetTexture()->GetHeight();
        screen_sr->GetTransform()->SetScale(Vector3(x, y, 1));
    }

    {
        auto bg_sprite = Sprite::LoadFromFile(Application::GetDataPath() + "/Assets/texture/start.png");
        auto bg_sr = GameObject::Create("bg")->AddComponent<SpriteRenderer>();
        bg_sr->GetGameObject()->SetLayer(Layer::UI);
        bg_sr->SetSprite(bg_sprite);
        float x = 1920 / (float) bg_sprite->GetTexture()->GetWidth();
        float y = 1080 / (float) bg_sprite->GetTexture()->GetHeight();
        bg_sr->GetTransform()->SetScale(Vector3(x, y, 1));
        g_bg = bg_sr.get();
    }

    auto state_sprite = Sprite::LoadFromFile(Application::GetDataPath() + "/Assets/texture/stop.png");
    auto state_sr = GameObject::Create("")->AddComponent<SpriteRenderer>();
    state_sr->GetGameObject()->SetLayer(Layer::UI);
    state_sr->SetSprite(state_sprite);
    state_sr->GetTransform()->SetPosition(Vector3(520, -420, 0) * 0.01f);
    state_sr->SetSortingOrder(1, 0);
    g_state = state_sr.get();
    g_state->Enable(false);

    auto stop = GameObject::Create("")->AddComponent<SpriteNode>();
    stop->SetSprite(state_sprite);
    g_stop = stop.get();

    state_sprite = Sprite::LoadFromFile(Application::GetDataPath() + "/Assets/texture/next.png");
    auto next = GameObject::Create("")->AddComponent<SpriteNode>();
    next->SetSprite(state_sprite);
    g_next = next.get();

    {
        std::string str;
        GTFile::ReadAllText(Application::GetDataPath() + "/Assets/text/reward.txt", str);
        GTString reward = str;
        reward = reward.Replace("\r\n", "\n");
        auto lines = reward.Split("\n", true);
        g_reward_name.resize(lines.size());
        g_reward_count.resize(lines.size());
        g_reward_image.resize(lines.size());
        for(size_t i=0; i<lines.size(); i++)
        {
            auto r = lines[i].Split("=", true);
            if(r.size() == 3)
            {
                g_reward_name[i] = r[0];
                g_reward_count[i] = GTString::ToType<int>(r[1].str);
                g_reward_image[i] = r[2].str;
            }
        }
    }

    auto image_sprite = Sprite::LoadFromFile(Application::GetDataPath() + "/Assets/texture/" + g_reward_image[0]);
    auto image_sr = GameObject::Create("")->AddComponent<SpriteRenderer>();
    image_sr->GetGameObject()->SetLayer(Layer::UI);
    image_sr->SetSprite(image_sprite);
    image_sr->GetTransform()->SetPosition(Vector3(-400, -70, 0) * 0.01f);
    image_sr->SetSortingOrder(1, 0);
    g_image = image_sr.get();
    g_image->Enable(false);

    {
        std::string str;
        GTFile::ReadAllText(Application::GetDataPath() + "/Assets/text/name.txt", str);
        GTString name = str;
        name = name.Replace("\r\n", "\n");
        auto lines = name.Split("\n", true);
        std::unordered_map<std::string, std::string> name_map;
        for(size_t i=0; i<lines.size(); i++)
        {
            auto n = lines[i].Split("=", true);
            for(auto i : n)
            {
                name_map[i.str] = i.str;
            }
        }
        std::vector<std::string> name_txt;
        for(auto &i : name_map)
        {
            name_txt.push_back(i.first);
        }
        while(!name_txt.empty())
        {
            int index = Mathf::RandomRange(0, name_txt.size());
            g_name_txt.push_back(name_txt[index]);
            name_txt.erase(name_txt.begin() + index);
        }
    }

    {
        std::string str;
        GTFile::ReadAllText(Application::GetDataPath() + "/Assets/text/config.txt", str);
        GTString config = str;
        config = config.Replace("\r\n", "\n");
        auto lines = config.Split("\n", true);
        for(size_t i=0; i<lines.size(); i++)
        {
            auto n = lines[i].Split("=", true);
            if(n.size() == 2 && n[0].str == "fontsize")
            {
                g_font_size = GTString::ToType<int>(n[1].str);
            }
        }
    }

    {
        auto label = Label::Create("", "heiti", 60, LabelPivot::Center, LabelAlign::Auto, true);
        auto tr = GameObject::Create("label")->AddComponent<TextRenderer>();
        tr->GetTransform()->SetPosition(Vector3(-300, 260, 0) * 0.01f);
        tr->SetLabel(label);
        tr->SetSortingOrder(1, 0);
        tr->GetTransform()->SetParent(cam2d->GetTransform());
        tr->GetGameObject()->SetLayer(Layer::UI);

        g_reward = tr.get();
    }

    {
        auto label = Label::Create("", "heiti", 120, LabelPivot::Center, LabelAlign::Auto, true);
        auto tr = GameObject::Create("label")->AddComponent<TextRenderer>();
        tr->GetTransform()->SetPosition(Vector3(520, -70, 0) * 0.01f);
        tr->SetLabel(label);
        tr->SetSortingOrder(1, 0);
        tr->GetTransform()->SetParent(cam2d->GetTransform());
        tr->GetGameObject()->SetLayer(Layer::UI);
        label->SetLineSpace(30);

        g_name = tr.get();
    }
#endif

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
    terrain_obj->AddComponent<TerrainRenderer>();
    auto tc = terrain_obj->AddComponent<TerrainCollider>();
    tc->SetTerrain(ter);
    
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
    
    //set anim to ground
    Vector3 anim_pos = anim->GetTransform()->GetPosition();
    RaycastHit hit;
    if(Physics::RayCast(Vector3(anim_pos.x, 1000, anim_pos.z), Vector3(0, -1, 0), 2000, hit))
    {
        hit.point.y += 0.05f;
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

        RaycastHit hit;
        if(Physics::RayCast(Vector3(pos->x, 1000, pos->z), Vector3(0, -1, 0), 2000, hit))
        {
            hit.point.y += 0.05f;

            thiz->anim->GetTransform()->SetPosition(hit.point);
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

#if DEMO_DEF
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
    if(Physics::RayCast(cam_target, -cam3d->GetTransform()->GetForward(), g_cam_dis, hit))
    {
        cam3d->GetTransform()->SetPosition(Vector3::Lerp(hit.point, cam_target, 0.1f / 1.5f));
    }

    return rot_offset;
}
#endif

void Launcher::Update()
{
#if !DEMO_REWARD
	fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::m_fps).str + "\n" +
		"drawcall:" + GTString::ToString(GTTime::m_draw_call).str);
	fps->UpdateLabel();
#endif

#if DEMO_REWARD
    bool first_frame = false;
    if(!g_start)
    {
        if(Input::GetKeyUp(KeyCode::Space) || Input::GetKeyUp(KeyCode::Return))
        {
            g_start = true;
            first_frame = true;

            auto bg_sprite = Sprite::LoadFromFile(Application::GetDataPath() + "/Assets/texture/bg.png");
            g_bg->SetSprite(bg_sprite);
            g_state->Enable(true);
            g_image->Enable(true);
        }
        else
        {
            return;
        }
    }

    g_reward->GetLabel()->SetText("<shadow=#000000ff>" + g_reward_name[g_reward_index].str + "</shadow>");
    g_reward->UpdateLabel();

    std::string names;

    if(g_rand)
    {
        size_t count = g_reward_count[g_reward_index];
        g_name_temp = g_name_txt;
        g_name_rand.clear();

        while(g_name_rand.size() < count && !g_name_temp.empty())
        {
            int index = Mathf::RandomRange(0, g_name_temp.size());
            g_name_rand.push_back(g_name_temp[index]);
            g_name_temp.erase(g_name_temp.begin() + index);
        }

        std::string name_text;
        int index = 0;
        for(auto i : g_name_rand)
        {
            name_text += "<shadow=#000000ff>";
            name_text += i;
            name_text += "</shadow>";

            if(index % 3 == 2)
            {
                name_text += "\n";
            }
            
            if((int) g_name_rand.size() >= 3)
            {
                if(index % 3 != 2)
                {
                    name_text += "        ";
                }
            }
            else
            {
                if(index + 1 < (int) g_name_rand.size())
                {
                    name_text += "        ";
                }
            }

            index++;
        }

        g_name->GetLabel()->SetText(name_text);
        g_name->UpdateLabel();

        if(!first_frame && (Input::GetKeyUp(KeyCode::Space) || Input::GetKeyUp(KeyCode::Return)))
        {
            g_rand = false;

            g_result.push_back(g_name_rand);
            g_name_txt = g_name_temp;
            g_state->SetSprite(g_next->GetSprite());

            std::string result;
            for(auto &i : g_result)
            {
                for(auto &j : i)
                {
                    result += j;
                    result += ",";
                }

                result += "\r\n";
            }
            GTFile::WriteAllBytes(Application::GetDataPath() + "/Assets/text/result.txt", (void *) result.c_str(), result.size());
        }
    }
    else
    {
        if(!first_frame && (Input::GetKeyUp(KeyCode::Space) || Input::GetKeyUp(KeyCode::Return)))
        {
            if(g_reward_index + 1 < (int) g_reward_name.size())
            {
                g_rand = true;

                g_reward_index++;

                g_state->SetSprite(g_stop->GetSprite());
                auto image_sprite = Sprite::LoadFromFile(Application::GetDataPath() + "/Assets/texture/" + g_reward_image[g_reward_index]);
                g_image->SetSprite(image_sprite);
            }
        }
    }
#endif

#if DEMO_DEF
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
        float speed = 6.0f;
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
        if(Physics::RayCast(ray.origin, ray.GetDirection(), 1000, hit))
        {
            if(hit.collider && hit.collider->GetGameObject() == anim->GetGameObject())
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
#endif

#if DEMO_TERRAIN
	if(Input::GetTouchCount() > 0)
	{
		auto t = Input::GetTouch(0);

		if(t->phase == TouchPhase::Began)
		{
            Vector3 pos = t->position;
            Ray ray = cam3d->ScreenPointToRay(pos);

            RaycastHit hit;
            if(Physics::RayCast(ray.origin, ray.GetDirection(), 1000, hit))
            {
                Vector3 pos_old = anim->GetTransform()->GetPosition();
                Vector3 pos_new = hit.point + Vector3(0, 0.05f, 0);
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
#endif
}

void Launcher::LateUpdate()
{
#if DEMO_TERRAIN
    Vector3 pos = anim->GetTransform()->GetPosition();
    cam3d->GetTransform()->SetPosition(pos + cam_offset);
#endif
}

Launcher::~Launcher()
{
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
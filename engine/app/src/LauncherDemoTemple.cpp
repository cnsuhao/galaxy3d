#include "LauncherDemoTemple.h"

using namespace Galaxy3D;

static float g_unit_per_pixel = 0.01f;

static void add_fire_particles(std::shared_ptr<GameObject> &fire, std::shared_ptr<Camera> &cam3d);
static void add_lamp_particles(std::shared_ptr<GameObject> &lamp, std::shared_ptr<Camera> &cam3d);
static void add_dust_particles(std::shared_ptr<GameObject> &parent, std::shared_ptr<Camera> &cam3d, const Vector3 &local_pos);

void LauncherDemoTemple::Start()
{
    Label::LoadFont("consola", Application::GetDataPath() + "/Assets/font/consola.ttf");
    Label::LoadFont("heiti", Application::GetDataPath() + "/Assets/font/heiti.ttc");

    cam2d = GameObject::Create("camera")->AddComponent<Camera>();
    cam2d->SetOrthographic(true);
    cam2d->SetOrthographicSize(1);
    cam2d->SetClipPlane(-1, 1);
    cam2d->SetCullingMask(LayerMask::GetMask(Layer::UI));
    cam2d->SetDepth(1);
    cam2d->SetClearFlags(CameraClearFlags::Nothing);
    cam2d->SetClearColor(Color(0.3f, 0.3f, 0.3f, 1));

	float ui_scale = 2.0f / Screen::GetHeight();
    auto canvas = GameObject::Create("")->AddComponent<UICanvas>();
    canvas->GetTransform()->SetParent(cam2d->GetTransform());
    canvas->GetTransform()->SetScale(Vector3(1, 1, 1) * ui_scale);
	canvas->SetCamera(cam2d);

    auto label = Label::Create("", "heiti", 20, LabelPivot::Top, LabelAlign::Auto, true);
    auto tr = GameObject::Create("fps")->AddComponent<TextRenderer>();
    tr->SetLabel(label);
    tr->SetSortingOrder(1000, 0);
    tr->GetTransform()->SetParent(canvas->GetTransform());
    tr->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    tr->SetAnchor(Vector4(0.5f, 0, 0, 0));
    fps = tr;

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

    cam2d->GetGameObject()->SetLayerRecursively(Layer::UI);
}

void LauncherDemoTemple::Update()
{
    fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::GetFPS()) + "\n" +
        "drawcall:" + GTString::ToString(GTTime::GetDrawCall()));
}

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
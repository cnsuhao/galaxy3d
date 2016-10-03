#include "LauncherDemoDeferredShading.h"

using namespace Galaxy3D;

static float g_unit_per_pixel = 0.01f;

void LauncherDemoDeferredShading::Start()
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

    cam2d->GetGameObject()->SetLayerRecursively(Layer::UI);
}

void LauncherDemoDeferredShading::Update()
{
    fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::GetFPS()) + "\n" +
        "drawcall:" + GTString::ToString(GTTime::GetDrawCall()));
}
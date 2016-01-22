#include "LauncherDemoUI.h"

using namespace Galaxy3D;

static float g_unit_per_pixel = 0.01f;

void LauncherDemoUI::Start()
{
    cam2d = GameObject::Create("camera")->AddComponent<Camera>();
    cam2d->SetOrthographic(true);
    cam2d->SetOrthographicSize(g_unit_per_pixel * Screen::GetHeight() / 2);
    cam2d->SetClipPlane(-1, 1);
    cam2d->SetCullingMask(LayerMask::GetMask(Layer::UI));
    cam2d->SetDepth(1);
    cam2d->SetClearFlags(CameraClearFlags::Nothing);

    auto canvas = GameObject::Create("")->AddComponent<UICanvas>();
    canvas->GetTransform()->SetParent(cam2d->GetTransform());
    canvas->GetTransform()->SetScale(Vector3(1, 1, 1) * g_unit_per_pixel);

    Label::LoadFont("consola", Application::GetDataPath() + "/Assets/font/consola.ttf");
    Label::LoadFont("heiti", Application::GetDataPath() + "/Assets/font/heiti.ttc");

	auto label = Label::Create("", "heiti", 20, LabelPivot::LeftTop, LabelAlign::Auto, true);
	auto tr = GameObject::Create("fps")->AddComponent<TextRenderer>();
	tr->SetLabel(label);
	tr->SetSortingOrder(1000, 0);
    tr->GetTransform()->SetParent(canvas->GetTransform());
    tr->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    tr->SetAnchor(Vector4(0, 1, 0, 0));
	fps = tr;

    cam2d->SetClearFlags(CameraClearFlags::SolidColor);
    cam2d->SetClearColor(Color(0.3f, 0.3f, 0.3f, 1));

    auto button_sprite = Sprite::Create(
        Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/texture/ui/RnM UI Atlas.png"),
        Rect(1343, 1536, 126, 50),
        Vector2(0.5f, 0.5f),
        100,
        Vector4(8, 10, 8, 8),
        Sprite::Type::Sliced,
        Vector2(0, 0));

    
    auto button = GameObject::Create("")->AddComponent<SpriteRenderer>();
    button->GetTransform()->SetParent(canvas->GetTransform());
    button->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    button->SetSprite(button_sprite);
    button->SetSortingOrder(0, 0);
    button->SetAnchor(Vector4(1, 0, -63, 25));
    auto collider = button->GetGameObject()->AddComponent<BoxCollider>();
    collider->SetSize(Vector3(126, 50, 0));
    auto event_listener = button->GetGameObject()->AddComponent<ButtonEventListener>();

    /*
    auto button_sr = GameObject::Create("")->AddComponent<SpriteBatchRenderer>();
    button_sr->GetTransform()->SetParent(canvas->GetTransform());
    button_sr->GetTransform()->SetLocalScale(Vector3(1, 1, 1));

    auto button = GameObject::Create("")->AddComponent<SpriteNode>();
    button->GetTransform()->SetParent(button_sr->GetTransform());
    button->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    button->SetSprite(button_sprite);
    button->SetAnchor(Vector4(1, 0, -63, 25));
    auto collider = button->GetGameObject()->AddComponent<BoxCollider>();
    collider->SetSize(Vector3(126, 50, 0));

    button_sr->AddSprite(button);
    button_sr->UpdateSprites();
    */

    label = Label::Create("Quit", "heiti", 20, LabelPivot::Center, LabelAlign::Auto, false);
    label->SetColor(Color(0.7f, 0.7f, 0.7f, 1));
    tr = GameObject::Create("")->AddComponent<TextRenderer>();
    tr->SetLabel(label);
    tr->UpdateLabel();
    tr->SetSortingOrder(0, 1);
    tr->GetTransform()->SetParent(button->GetTransform());
    tr->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
    tr->GetTransform()->SetLocalScale(Vector3(1, 1, 1));

    cam2d->GetGameObject()->SetLayerRecursively(Layer::UI);
}

void LauncherDemoUI::Update()
{
	fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::GetFPS()).str + "\n" +
		"drawcall:" + GTString::ToString(GTTime::GetDrawCall()).str);
	fps->UpdateLabel();

    if(Input::GetKeyUp(KeyCode::Escape))
    {
        Application::Quit();
    }
}

void LauncherDemoUI::LateUpdate()
{
}

LauncherDemoUI::~LauncherDemoUI()
{
}
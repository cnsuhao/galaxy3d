#include "LauncherDemoUI.h"

using namespace Galaxy3D;

static float g_unit_per_pixel = 0.01f;

void LauncherDemoUI::CreateButton(
    const std::shared_ptr<Texture2D> &atlas,
    const Rect &rect,
    const Vector4 &border,
    const Vector2 &size,
    const std::shared_ptr<UICanvas> &canvas,
    const std::string &text,
    const Vector4 &anchor,
    int layer,
    int order,
    const std::shared_ptr<SpriteBatchRenderer> &batch)
{
    auto sprite_button = Sprite::Create(
        atlas,
        rect,
        Vector2(0.5f, 0.5f),
        100,
        border,
        Sprite::Type::Sliced,
        size);

    auto button = GameObject::Create("")->AddComponent<SpriteNode>();
    button->GetTransform()->SetParent(batch->GetTransform());
    button->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    button->SetSprite(sprite_button);
    button->SetAnchor(anchor);
    button->SetSortingOrder(order);

    auto collider = button->GetGameObject()->AddComponent<BoxCollider>();
    collider->SetSize(Vector3(size.x, size.y, 0));
    auto event_listener = button->GetGameObject()->AddComponent<ButtonEventListener>();

    batch->AddSprite(button);

    if(!text.empty())
    {
        auto label = Label::Create(text, "heiti", 20, LabelPivot::Center, LabelAlign::Auto, false);
        label->SetColor(Color(0.7f, 0.7f, 0.7f, 1));
        auto tr = GameObject::Create("")->AddComponent<TextRenderer>();
        tr->GetTransform()->SetParent(button->GetTransform());
        tr->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
        tr->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
        tr->SetLabel(label);
        tr->UpdateLabel();
        tr->SetSortingOrder(layer, order + 1);
    }
}

void LauncherDemoUI::Start()
{
    Label::LoadFont("consola", Application::GetDataPath() + "/Assets/font/consola.ttf");
    Label::LoadFont("heiti", Application::GetDataPath() + "/Assets/font/heiti.ttc");

    cam2d = GameObject::Create("camera")->AddComponent<Camera>();
    cam2d->SetOrthographic(true);
    cam2d->SetOrthographicSize(g_unit_per_pixel * Screen::GetHeight() / 2);
    cam2d->SetClipPlane(-1, 1);
    cam2d->SetCullingMask(LayerMask::GetMask(Layer::UI));
    cam2d->SetDepth(1);
    cam2d->SetClearFlags(CameraClearFlags::SolidColor);
    cam2d->SetClearColor(Color(0.3f, 0.3f, 0.3f, 1));

    auto canvas = GameObject::Create("")->AddComponent<UICanvas>();
    canvas->GetTransform()->SetParent(cam2d->GetTransform());
    canvas->GetTransform()->SetScale(Vector3(1, 1, 1) * g_unit_per_pixel);

	auto label = Label::Create("", "heiti", 20, LabelPivot::LeftBottom, LabelAlign::Auto, true);
	auto tr = GameObject::Create("fps")->AddComponent<TextRenderer>();
	tr->SetLabel(label);
	tr->SetSortingOrder(1000, 0);
    tr->GetTransform()->SetParent(canvas->GetTransform());
    tr->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    tr->SetAnchor(Vector4(0, 1, 5, 10));
	fps = tr;

    auto batch = GameObject::Create("")->AddComponent<SpriteBatchRenderer>();
    batch->GetTransform()->SetParent(canvas->GetTransform());
    batch->GetTransform()->SetLocalScale(Vector3(1, 1, 1));

    auto atlas = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/texture/ui/RnM UI Atlas.png");

    auto sprite_top_bar = Sprite::Create(
        atlas,
        Rect(399, 1302, 512, 79),
        Vector2(0.5f, 0.5f),
        100,
        Vector4(150, 0, 150, 0),
        Sprite::Type::Sliced,
        Vector2((float) Screen::GetWidth(), 79));

    auto top_bar = GameObject::Create("")->AddComponent<SpriteNode>();
    top_bar->GetTransform()->SetParent(batch->GetTransform());
    top_bar->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    top_bar->SetSprite(sprite_top_bar);
    top_bar->SetAnchor(Vector4(0.5f, 0, 0, -40));
    top_bar->SetSortingOrder(0);

    batch->AddSprite(top_bar);

    CreateButton(
        atlas,
        Rect(1343, 1536, 126, 50),
        Vector4(8, 10, 8, 8),
        Vector2(126, 50),
        canvas,
        "Exit",
        Vector4(1, 1, -63, 25),
        0, 0,
        batch);
    
    batch->UpdateSprites();

    cam2d->GetGameObject()->SetLayerRecursively(Layer::UI);
}

void LauncherDemoUI::Update()
{
	fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::GetFPS()).str + "\n" +
		"drawcall:" + GTString::ToString(GTTime::GetDrawCall()).str);
	fps->UpdateLabel();

    if(Input::GetKeyUp(KeyCode::Escape))
    {
        //Application::Quit();
    }
}

void LauncherDemoUI::LateUpdate()
{
}

LauncherDemoUI::~LauncherDemoUI()
{
}
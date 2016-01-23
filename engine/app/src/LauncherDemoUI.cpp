#include "LauncherDemoUI.h"

using namespace Galaxy3D;

static float g_unit_per_pixel = 0.01f;
static UIAtlas *g_atlas = NULL;

struct NormalButtonEventListener : public UIEventListener
{
    std::shared_ptr<SpriteNode> m_button;
    std::shared_ptr<SpriteBatchRenderer> m_renderer;

    virtual void Start()
    {
        m_button = GetGameObject()->GetComponent<SpriteNode>();
        m_renderer = GetTransform()->GetParent().lock()->GetGameObject()->GetComponent<SpriteBatchRenderer>();
    }

    virtual void OnPress(bool press)
    {
        if(press)
        {
            g_atlas->SetSpriteData(m_button->GetSprite(), "NormalButton_Hover");
        }
        else
        {
            g_atlas->SetSpriteData(m_button->GetSprite(), "NormalButton_Normal");
        }

        m_renderer->UpdateSprites();
    }

    virtual void OnClick()
    {
        
    }
};

struct ExitEventListener : public NormalButtonEventListener
{
    virtual void OnClick()
    {
        Application::Quit();
    }
};

struct TopBarButtonEventListener : public UIEventListener
{
    std::shared_ptr<SpriteNode> m_button;
    std::shared_ptr<SpriteBatchRenderer> m_renderer;

    virtual void Start()
    {
        m_button = GetGameObject()->GetComponent<SpriteNode>();
        m_button->SetColor(Color(1, 1, 1, 0));
        m_renderer = GetTransform()->GetParent().lock()->GetGameObject()->GetComponent<SpriteBatchRenderer>();
        m_renderer->UpdateSprites();
    }

    virtual void OnPress(bool press)
    {
        if(press)
        {
            m_button->SetColor(Color(1, 1, 1, 1));
        }
        else
        {
            m_button->SetColor(Color(1, 1, 1, 0));
        }

        m_renderer->UpdateSprites();
    }
};

struct SettingEventListener : public TopBarButtonEventListener
{
    virtual void OnClick()
    {
        
    }
};

struct NewsEventListener : public TopBarButtonEventListener
{
    virtual void OnClick()
    {

    }
};

struct WorldEventListener : public TopBarButtonEventListener
{
    virtual void OnClick()
    {

    }
};

template<class T>
static void create_button(
    const std::shared_ptr<UIAtlas> &atlas,
    const std::string &sprite_name,
    const Vector2 &size,
    const std::string &text,
    bool use_anchor,
    const Vector4 &anchor,
    const Vector3 &local_pos,
    int layer,
    int order,
    const std::shared_ptr<SpriteBatchRenderer> &batch,
    std::vector<std::string> &sub_sprites,
    std::vector<Vector3> &sub_offsets)
{
    auto sprite_button = atlas->CreateSprite(
        sprite_name,
        Vector2(0.5f, 0.5f),
        100, 
        Sprite::Type::Sliced,
        size);

    auto button = GameObject::Create("button")->AddComponent<SpriteNode>();
    button->GetTransform()->SetParent(batch->GetTransform());
    button->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    button->SetSprite(sprite_button);
    if(use_anchor)
    {
        button->SetAnchor(anchor);
    }
    else
    {
        button->GetTransform()->SetLocalPosition(local_pos);
    }
    button->SetSortingOrder(order);

    auto collider = button->GetGameObject()->AddComponent<BoxCollider>();
    collider->SetSize(Vector3(size.x, size.y, 0));
    auto event_listener = button->GetGameObject()->AddComponent<T>();

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

    for(size_t i=0; i<sub_sprites.size(); i++)
    {
        auto sprite = atlas->CreateSprite(
            sub_sprites[i],
            Vector2(0.5f, 0.5f),
            100, 
            Sprite::Type::Simple,
            Vector2(0, 0));

        auto node = GameObject::Create(GTString::ToString(i).str)->AddComponent<SpriteNode>();
        node->GetTransform()->SetParent(button->GetTransform());
        node->GetTransform()->SetLocalPosition(sub_offsets[i]);
        node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
        node->SetSprite(sprite);
        node->SetSortingOrder(order + 2 + i);

        batch->AddSprite(node);
    }
}

static void create_top_bar(
    const std::shared_ptr<UIAtlas> &atlas,
    const std::shared_ptr<UICanvas> &canvas)
{
    auto batch = GameObject::Create("")->AddComponent<SpriteBatchRenderer>();
    batch->GetTransform()->SetParent(canvas->GetTransform());
    batch->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    batch->SetSortingOrder(0, 0);

    auto sprite_top_bar = atlas->CreateSprite(
        "TopBar_Background",
        Vector2(0.5f, 0.5f),
        100, 
        Sprite::Type::Sliced,
        Vector2((float) Screen::GetWidth(), 79));

    auto top_bar = GameObject::Create("")->AddComponent<SpriteNode>();
    top_bar->GetTransform()->SetParent(batch->GetTransform());
    top_bar->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    top_bar->SetSprite(sprite_top_bar);
    top_bar->SetAnchor(Vector4(0.5f, 0, 0, -40));
    top_bar->SetSortingOrder(0);

    batch->AddSprite(top_bar);

    std::vector<std::string> setting_sub_sprites;
    std::vector<Vector3> setting_sub_offsets;
    setting_sub_sprites.push_back("TopBar_Separator");
    setting_sub_offsets.push_back(Vector3(-30, 0, 0));
    setting_sub_sprites.push_back("UIIcon_Settings");
    setting_sub_offsets.push_back(Vector3(0, 0, 0));
    create_button<SettingEventListener>(
        atlas,
        "TopBar_Button_Hover",
        Vector2(58, 56),
        "",
        true,
        Vector4(1, 0, -31, -29),
        Vector3(),
        0, 1,
        batch,
        setting_sub_sprites,
        setting_sub_offsets);

    std::vector<std::string> news_sub_sprites;
    std::vector<Vector3> news_sub_offsets;
    news_sub_sprites.push_back("TopBar_Separator");
    news_sub_offsets.push_back(Vector3(-30, 0, 0));
    news_sub_sprites.push_back("UIIcon_News");
    news_sub_offsets.push_back(Vector3(0, 0, 0));
    create_button<NewsEventListener>(
        atlas,
        "TopBar_Button_Hover",
        Vector2(58, 56),
        "",
        true,
        Vector4(1, 0, -92, -29),
        Vector3(),
        0, 2,
        batch,
        news_sub_sprites,
        news_sub_offsets);

    std::vector<std::string> world_sub_sprites;
    std::vector<Vector3> world_sub_offsets;
    world_sub_sprites.push_back("TopBar_Separator");
    world_sub_offsets.push_back(Vector3(31, 0, 0));
    world_sub_sprites.push_back("UIIcon_World");
    world_sub_offsets.push_back(Vector3(0, 0, 0));
    create_button<WorldEventListener>(
        atlas,
        "TopBar_Button_Hover",
        Vector2(58, 56),
        "",
        true,
        Vector4(0, 0, 31, -29),
        Vector3(),
        0, 3,
        batch,
        world_sub_sprites,
        world_sub_offsets);

    batch->UpdateSprites();
}

static void create_window_setting(
    const std::shared_ptr<UIAtlas> &atlas,
    const std::shared_ptr<UICanvas> &canvas)
{
    auto batch = GameObject::Create("")->AddComponent<SpriteBatchRenderer>();
    batch->GetTransform()->SetParent(canvas->GetTransform());
    batch->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    batch->SetSortingOrder(1, 0);

    auto sprite_bg = atlas->CreateSprite(
        "Window_Background",
        Vector2(0.5f, 0.5f),
        100, 
        Sprite::Type::Sliced,
        Vector2(438, 584));

    auto bg = GameObject::Create("")->AddComponent<SpriteNode>();
    bg->GetTransform()->SetParent(batch->GetTransform());
    bg->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    bg->SetSprite(sprite_bg);
    bg->SetAnchor(Vector4(0.5f, 0.5f, 0, 0));
    bg->SetSortingOrder(0);

    batch->AddSprite(bg);

    create_button<NormalButtonEventListener>(
        atlas,
        "NormalButton_Normal",
        Vector2(350, 50),
        "Resume",
        false,
        Vector4(),
        Vector3(0, 220, 0),
        1, 1,
        batch,
        std::vector<std::string>(),
        std::vector<Vector3>());

    create_button<ExitEventListener>(
        atlas,
        "NormalButton_Normal",
        Vector2(350, 50),
        "Quit",
        false,
        Vector4(),
        Vector3(0, -220, 0),
        1, 1,
        batch,
        std::vector<std::string>(),
        std::vector<Vector3>());

    batch->UpdateSprites();
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

	auto label = Label::Create("", "heiti", 20, LabelPivot::Top, LabelAlign::Auto, true);
	auto tr = GameObject::Create("fps")->AddComponent<TextRenderer>();
	tr->SetLabel(label);
	tr->SetSortingOrder(1000, 0);
    tr->GetTransform()->SetParent(canvas->GetTransform());
    tr->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    tr->SetAnchor(Vector4(0.5f, 0, 0, 0));
	fps = tr;

    auto atlas = UIAtlas::LoadFromJsonFile(Application::GetDataPath() + "/Assets/texture/ui.json");
    g_atlas = atlas.get();
    //auto atlas = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/texture/RnM UI Atlas.png");

    create_top_bar(atlas, canvas);
    create_window_setting(atlas, canvas);

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
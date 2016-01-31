#include "LauncherDemoUI.h"

using namespace Galaxy3D;

static float g_unit_per_pixel = 0.01f;
static GameObject *g_win_setting = NULL;

struct ResumeEventListener : public UIButton
{
    virtual void OnClick()
    {
        if(g_win_setting->IsActiveSelf())
        {
            g_win_setting->SetActive(false);
        }
    }
};

struct ExitEventListener : public UIButton
{
    virtual void OnClick()
    {
        Application::Quit();
    }
};

struct TopBarButtonEventListener : public UIEventListener
{
    std::shared_ptr<Sprite> sprite;
    std::shared_ptr<SpriteNode> m_button;

    virtual void Start()
    {
        m_button = GetGameObject()->GetComponent<SpriteNode>();
        m_button->SetColor(Color(1, 1, 1, 0));
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
    }
};

struct SettingEventListener : public TopBarButtonEventListener
{
    virtual void OnClick()
    {
        if(!g_win_setting->IsActiveSelf())
        {
            g_win_setting->SetActive(true);
        }
        else
        {
            g_win_setting->SetActive(false);
        }
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
static std::shared_ptr<T> create_button(
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
    auto button = GameObject::Create("button")->AddComponent<SpriteNode>();
    button->GetTransform()->SetParent(batch->GetTransform());
    button->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    button->SetSprite(atlas->CreateSprite(
        sprite_name,
        Vector2(0.5f, 0.5f),
        100, 
        Sprite::Type::Sliced,
        size));
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
    event_listener->sprite = button->GetSprite();

    batch->AddSprite(button);

    if(!text.empty())
    {
        auto label = Label::Create(text, "heiti", 18, LabelPivot::Center, LabelAlign::Auto, false);
        auto tr = GameObject::Create("")->AddComponent<TextRenderer>();
        tr->GetTransform()->SetParent(button->GetTransform());
        tr->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
        tr->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
        tr->SetLabel(label);
        tr->SetColor(Color(0.7f, 0.7f, 0.7f, 1));
        tr->SetSortingOrder(layer, order + 1);
    }

    for(size_t i=0; i<sub_sprites.size(); i++)
    {
        auto node = GameObject::Create(GTString::ToString(i).str)->AddComponent<SpriteNode>();
        node->GetTransform()->SetParent(button->GetTransform());
        node->GetTransform()->SetLocalPosition(sub_offsets[i]);
        node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
        node->SetSprite(atlas->CreateSprite(
            sub_sprites[i],
            Vector2(0.5f, 0.5f),
            100, 
            Sprite::Type::Simple,
            Vector2(0, 0)));
        node->SetSortingOrder(order + 2 + i);
        batch->AddSprite(node);
    }

    return event_listener;
}

static void create_top_bar(
    const std::shared_ptr<UIAtlas> &atlas,
    const std::shared_ptr<UICanvas> &canvas)
{
    auto batch = GameObject::Create("")->AddComponent<SpriteBatchRenderer>();
    batch->GetTransform()->SetParent(canvas->GetTransform());
    batch->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    batch->SetSortingOrder(0, 0);

    auto top_bar = GameObject::Create("")->AddComponent<SpriteNode>();
    top_bar->GetTransform()->SetParent(batch->GetTransform());
    top_bar->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    top_bar->SetSprite(atlas->CreateSprite(
        "TopBar_Background",
        Vector2(0.5f, 0),
        100, 
        Sprite::Type::Sliced,
        Vector2((float) Screen::GetWidth(), 79)));
    top_bar->SetAnchor(Vector4(0.5f, 0, 0, 0));
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
}

static void create_action_bar(
    const std::shared_ptr<UIAtlas> &atlas,
    const std::shared_ptr<UICanvas> &canvas)
{
    auto batch = GameObject::Create("")->AddComponent<SpriteBatchRenderer>();
    batch->GetTransform()->SetParent(canvas->GetTransform());
    batch->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    batch->SetSortingOrder(0, 0);

    auto action_bar = GameObject::Create("")->AddComponent<SpriteNode>();
    action_bar->GetTransform()->SetParent(batch->GetTransform());
    action_bar->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    action_bar->SetSprite(atlas->CreateSprite(
        "ActionBar_Background",
        Vector2(0.5f, 1),
        100, 
        Sprite::Type::Sliced,
        Vector2(890, 98)));
    action_bar->SetAnchor(Vector4(0.5f, 1, 0, 0));
    action_bar->SetSortingOrder(0);
    batch->AddSprite(action_bar);

    auto left_globe_bg = GameObject::Create("")->AddComponent<SpriteNode>();
    left_globe_bg->GetTransform()->SetParent(action_bar->GetTransform());
    left_globe_bg->GetTransform()->SetLocalPosition(Vector3(-539, 172, 0));
    left_globe_bg->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    left_globe_bg->SetSprite(atlas->CreateSprite(
        "ActionBar_Globe_Background",
        Vector2(0, 0),
        100, 
        Sprite::Type::Simple,
        Vector2(0, 0)));
    left_globe_bg->SetSortingOrder(1);
    batch->AddSprite(left_globe_bg);

    auto left_globe_fill = GameObject::Create("")->AddComponent<SpriteNode>();
    left_globe_fill->GetTransform()->SetParent(left_globe_bg->GetTransform());
    left_globe_fill->GetTransform()->SetLocalPosition(Vector3(93, -94, 0));
    left_globe_fill->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    left_globe_fill->SetSprite(atlas->CreateSprite(
        "ActionBar_Globe_Fill",
        Vector2(0.5f, 0.5f),
        100, 
        Sprite::Type::Filled,
        Vector2(0, 0)));
    left_globe_fill->GetSprite()->SetFillDirection(Sprite::FillDirection::Vertical);
    left_globe_fill->GetSprite()->SetFillAmount(1);
    left_globe_fill->SetColor(Color(243, 30, 30, 255) / 255.0f);
    left_globe_fill->SetSortingOrder(2);
    batch->AddSprite(left_globe_fill);

    auto left_globe_overlay = GameObject::Create("")->AddComponent<SpriteNode>();
    left_globe_overlay->GetTransform()->SetParent(left_globe_bg->GetTransform());
    left_globe_overlay->GetTransform()->SetLocalPosition(Vector3(94, -94, 0));
    left_globe_overlay->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    left_globe_overlay->SetSprite(atlas->CreateSprite(
        "ActionBar_Globe_Overlay_D",
        Vector2(0.5f, 0.5f),
        100, 
        Sprite::Type::Simple,
        Vector2(0, 0)));
    left_globe_overlay->SetSortingOrder(3);
    batch->AddSprite(left_globe_overlay);

    auto left_globe_decoration = GameObject::Create("")->AddComponent<SpriteNode>();
    left_globe_decoration->GetTransform()->SetParent(left_globe_bg->GetTransform());
    left_globe_decoration->GetTransform()->SetLocalPosition(Vector3(93, -148, 0));
    left_globe_decoration->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    left_globe_decoration->SetSprite(atlas->CreateSprite(
        "ActionBar_Globe_Decoration",
        Vector2(0.5f, 0.5f),
        100, 
        Sprite::Type::Simple,
        Vector2(0, 0)));
    left_globe_decoration->SetSortingOrder(4);
    batch->AddSprite(left_globe_decoration);
    
    auto right_globe_bg = GameObject::Create("")->AddComponent<SpriteNode>();
    right_globe_bg->GetTransform()->SetParent(action_bar->GetTransform());
    right_globe_bg->GetTransform()->SetLocalPosition(Vector3(354, 172, 0));
    right_globe_bg->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    right_globe_bg->SetSprite(atlas->CreateSprite(
        "ActionBar_Globe_Background",
        Vector2(0, 0),
        100, 
        Sprite::Type::Simple,
        Vector2(0, 0)));
    right_globe_bg->SetSortingOrder(1);
    batch->AddSprite(right_globe_bg);

    auto right_globe_fill = GameObject::Create("")->AddComponent<SpriteNode>();
    right_globe_fill->GetTransform()->SetParent(right_globe_bg->GetTransform());
    right_globe_fill->GetTransform()->SetLocalPosition(Vector3(93, -94, 0));
    right_globe_fill->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    right_globe_fill->SetSprite(atlas->CreateSprite(
        "ActionBar_Globe_Fill",
        Vector2(0.5f, 0.5f),
        100, 
        Sprite::Type::Filled,
        Vector2(0, 0)));
    right_globe_fill->GetSprite()->SetFillDirection(Sprite::FillDirection::Vertical);
    right_globe_fill->GetSprite()->SetFillAmount(1);
    right_globe_fill->SetColor(Color(36, 157, 183, 255) / 255.0f);
    right_globe_fill->SetSortingOrder(2);
    batch->AddSprite(right_globe_fill);

    auto right_globe_overlay = GameObject::Create("")->AddComponent<SpriteNode>();
    right_globe_overlay->GetTransform()->SetParent(right_globe_bg->GetTransform());
    right_globe_overlay->GetTransform()->SetLocalPosition(Vector3(94, -94, 0));
    right_globe_overlay->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    right_globe_overlay->SetSprite(atlas->CreateSprite(
        "ActionBar_Globe_Overlay_D",
        Vector2(0.5f, 0.5f),
        100, 
        Sprite::Type::Simple,
        Vector2(0, 0)));
    right_globe_overlay->GetSprite()->SetFlip(Sprite::Flip::Horizontal);
    right_globe_overlay->SetSortingOrder(3);
    batch->AddSprite(right_globe_overlay);

    auto right_globe_decoration = GameObject::Create("")->AddComponent<SpriteNode>();
    right_globe_decoration->GetTransform()->SetParent(right_globe_bg->GetTransform());
    right_globe_decoration->GetTransform()->SetLocalPosition(Vector3(93, -148, 0));
    right_globe_decoration->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    right_globe_decoration->SetSprite(atlas->CreateSprite(
        "ActionBar_Globe_Decoration",
        Vector2(0.5f, 0.5f),
        100, 
        Sprite::Type::Simple,
        Vector2(0, 0)));
    right_globe_decoration->SetSortingOrder(4);
    batch->AddSprite(right_globe_decoration);

    for(int i=0; i<12; i++)
    {
        auto spell_slot = GameObject::Create("")->AddComponent<SpriteNode>();
        spell_slot->GetTransform()->SetParent(action_bar->GetTransform());
        spell_slot->GetTransform()->SetLocalPosition(Vector3(-330 + i * 60.0f, 38, 0));
        spell_slot->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
        spell_slot->SetSprite(atlas->CreateSprite(
            "ActionBar_SpellSlot",
            Vector2(0.5f, 0.5f),
            100, 
            Sprite::Type::Simple,
            Vector2(0, 0)));
        spell_slot->SetSortingOrder(1);
        batch->AddSprite(spell_slot);

        auto spell_cooldown = GameObject::Create("")->AddComponent<SpriteNode>();
        spell_cooldown->GetTransform()->SetParent(spell_slot->GetTransform());
        spell_cooldown->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
        spell_cooldown->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
        spell_cooldown->SetSprite(atlas->CreateSprite(
            "SpellSlot_Cooldown",
            Vector2(0.5f, 0.5f),
            100, 
            Sprite::Type::Filled,
            Vector2(0, 0)));
        spell_cooldown->SetSortingOrder(2);
        spell_cooldown->GetSprite()->SetFillDirection(Sprite::FillDirection::Radial_360);
        spell_cooldown->GetSprite()->SetFillAmount(0);
        batch->AddSprite(spell_cooldown);
    }

    auto xp_bar = GameObject::Create("")->AddComponent<SpriteNode>();
    xp_bar->GetTransform()->SetParent(action_bar->GetTransform());
    xp_bar->GetTransform()->SetLocalPosition(Vector3(0, 96, 0));
    xp_bar->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    xp_bar->SetSprite(atlas->CreateSprite(
        "ActionBar_XPBar_Background",
        Vector2(0.5f, 0.5f),
        100, 
        Sprite::Type::Sliced,
        Vector2(680, 43)));
    xp_bar->SetSortingOrder(1);
    batch->AddSprite(xp_bar);

    auto xp_fill = GameObject::Create("")->AddComponent<SpriteNode>();
    xp_fill->GetTransform()->SetParent(xp_bar->GetTransform());
    xp_fill->GetTransform()->SetLocalPosition(Vector3(1, -9, 0));
    xp_fill->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    xp_fill->SetSprite(atlas->CreateSprite(
        "ActionBar_XPBar_Fill",
        Vector2(0.5f, 0.5f),
        100, 
        Sprite::Type::Filled,
        Vector2(0, 0)));
    xp_fill->SetSortingOrder(2);
    xp_fill->GetSprite()->SetFillDirection(Sprite::FillDirection::Horizontal);
    xp_fill->GetSprite()->SetFillAmount(0);
    xp_fill->SetColor(Color(168, 255, 140, 255) / 255.0f);
    batch->AddSprite(xp_fill);
}

static void create_window_setting(
    const std::shared_ptr<UIAtlas> &atlas,
    const std::shared_ptr<UICanvas> &canvas)
{
    auto batch = GameObject::Create("")->AddComponent<SpriteBatchRenderer>();
    batch->GetTransform()->SetParent(canvas->GetTransform());
    batch->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    batch->SetSortingOrder(1, 0);

    auto bg = GameObject::Create("")->AddComponent<SpriteNode>();
    bg->GetTransform()->SetParent(batch->GetTransform());
    bg->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    bg->SetSprite(atlas->CreateSprite(
        "Window_Background",
        Vector2(0.5f, 0.5f),
        100, 
        Sprite::Type::Sliced,
        Vector2(438, 584)));
    bg->SetAnchor(Vector4(0.5f, 0.5f, 0, 0));
    bg->SetSortingOrder(0);
    batch->AddSprite(bg);

    auto header = GameObject::Create("")->AddComponent<SpriteNode>();
    header->GetTransform()->SetParent(bg->GetTransform());
    header->GetTransform()->SetLocalPosition(Vector3(0, 232, 0));
    header->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    header->SetSprite(atlas->CreateSprite(
        "Window_Header",
        Vector2(0.5f, 0.5f),
        100, 
        Sprite::Type::Sliced,
        Vector2(401, 81)));
    header->SetSortingOrder(1);
    batch->AddSprite(header);

    auto label = Label::Create("<bold>SETTINGS</bold>", "consola", 20, LabelPivot::Center, LabelAlign::Auto, true);
    auto tr = GameObject::Create("")->AddComponent<TextRenderer>();
    tr->GetTransform()->SetParent(header->GetTransform());
    tr->GetTransform()->SetLocalPosition(Vector3(0, 20, 0));
    tr->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
    tr->SetLabel(label);
    tr->SetColor(Color(185, 124, 43, 255) / 255.0f);
    tr->SetSortingOrder(1, 1);

    std::shared_ptr<UIButton> button;

    button = create_button<ResumeEventListener>(
        atlas,
        "NormalButton_Normal",
        Vector2(350, 50),
        "Back",
        false,
        Vector4(),
        Vector3(0, 160, 0),
        1, 2,
        batch,
        std::vector<std::string>(),
        std::vector<Vector3>());
    button->sprite_name_normal = "NormalButton_Normal";
    button->sprite_name_pressed = "NormalButton_Hover";

    button = create_button<UIButton>(
        atlas,
        "NormalButton_Normal",
        Vector2(350, 50),
        "Graphic",
        false,
        Vector4(),
        Vector3(0, 160 - 90, 0),
        1, 3,
        batch,
        std::vector<std::string>(),
        std::vector<Vector3>());
    button->sprite_name_normal = "NormalButton_Normal";
    button->sprite_name_pressed = "NormalButton_Hover";

    button = create_button<UIButton>(
        atlas,
        "NormalButton_Normal",
        Vector2(350, 50),
        "Audio",
        false,
        Vector4(),
        Vector3(0, 160 - 90 * 2, 0),
        1, 4,
        batch,
        std::vector<std::string>(),
        std::vector<Vector3>());
    button->sprite_name_normal = "NormalButton_Normal";
    button->sprite_name_pressed = "NormalButton_Hover";

    button = create_button<UIButton>(
        atlas,
        "NormalButton_Normal",
        Vector2(350, 50),
        "Input",
        false,
        Vector4(),
        Vector3(0, 160 - 90 * 3, 0),
        1, 5,
        batch,
        std::vector<std::string>(),
        std::vector<Vector3>());
    button->sprite_name_normal = "NormalButton_Normal";
    button->sprite_name_pressed = "NormalButton_Hover";

    button = create_button<ExitEventListener>(
        atlas,
        "NormalButton_Normal",
        Vector2(350, 50),
        "Quit",
        false,
        Vector4(),
        Vector3(0, 160 - 90 * 4, 0),
        1, 6,
        batch,
        std::vector<std::string>(),
        std::vector<Vector3>());
    button->sprite_name_normal = "NormalButton_Normal";
    button->sprite_name_pressed = "NormalButton_Hover";

    g_win_setting = batch->GetGameObject().get();
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

    auto atlas = UIAtlas::LoadFromJsonFile(Application::GetDataPath() + "/Assets/texture/ui.json");

    create_top_bar(atlas, canvas);
    create_window_setting(atlas, canvas);
    create_action_bar(atlas, canvas);

    g_win_setting->SetActive(false);

    cam2d->GetGameObject()->SetLayerRecursively(Layer::UI);
}

void LauncherDemoUI::Update()
{
	fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::GetFPS()).str + "\n" +
		"drawcall:" + GTString::ToString(GTTime::GetDrawCall()).str);

    if(Input::GetKeyUp(KeyCode::Escape))
    {
        if(!g_win_setting->IsActiveSelf())
        {
            g_win_setting->SetActive(true);
        }
        else
        {
            g_win_setting->SetActive(false);
        }
    }
}
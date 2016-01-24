#include "Launcher.h"

using namespace Galaxy3D;

static float g_unit_per_pixel = 0.01f;
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

void Launcher::Start()
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

    cam2d->SetClearFlags(CameraClearFlags::SolidColor);

    /*
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
    */

    {
        auto bg_sprite = Sprite::LoadFromFile(Application::GetDataPath() + "/Assets/texture/start.png");
        auto bg_sr = GameObject::Create("bg")->AddComponent<SpriteRenderer>();
        bg_sr->SetSprite(bg_sprite);
        bg_sr->GetTransform()->SetParent(canvas->GetTransform());
        float x = 1920 / (float) bg_sprite->GetTexture()->GetWidth();
        float y = 1080 / (float) bg_sprite->GetTexture()->GetHeight();
        bg_sr->GetTransform()->SetLocalScale(Vector3(x, y, 1));
        g_bg = bg_sr.get();
    }

    auto state_sprite = Sprite::LoadFromFile(Application::GetDataPath() + "/Assets/texture/stop.png");
    auto state_sr = GameObject::Create("")->AddComponent<SpriteRenderer>();
    state_sr->SetSprite(state_sprite);
    state_sr->GetTransform()->SetParent(canvas->GetTransform());
    state_sr->GetTransform()->SetLocalPosition(Vector3(520, -420, 0));
    state_sr->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
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
    image_sr->SetSprite(image_sprite);
    image_sr->GetTransform()->SetParent(canvas->GetTransform());
    image_sr->GetTransform()->SetLocalPosition(Vector3(-400, -70, 0));
    image_sr->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
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
        tr->GetTransform()->SetParent(canvas->GetTransform());
        tr->GetTransform()->SetLocalPosition(Vector3(-300, 260, 0));
        tr->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
        tr->SetLabel(label);
        tr->SetSortingOrder(1, 0);

        g_reward = tr.get();
    }

    {
        auto label = Label::Create("", "heiti", 120, LabelPivot::Center, LabelAlign::Auto, true);
        auto tr = GameObject::Create("label")->AddComponent<TextRenderer>();
        tr->GetTransform()->SetParent(canvas->GetTransform());
        tr->GetTransform()->SetLocalPosition(Vector3(520, -70, 0));
        tr->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
        tr->SetLabel(label);
        tr->SetSortingOrder(1, 0);
        label->SetLineSpace(30);

        g_name = tr.get();
    }

    cam2d->GetGameObject()->SetLayerRecursively(Layer::UI);
}

void Launcher::Update()
{
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

    if(Input::GetKeyUp(KeyCode::Escape))
    {
        Application::Quit();
    }
}
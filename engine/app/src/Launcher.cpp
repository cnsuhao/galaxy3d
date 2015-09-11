#include "Launcher.h"

using namespace Galaxy3D;

static const int CAMERA_OFFSET_Y = 2;
GameObject *pmesh;
float rot_y = 0;

void Launcher::Start()
{
	Label::LoadFont("consola", Application::GetDataPath() + "/Assets/font/consola.ttf");

	camera = GameObject::Create("camera")->AddComponent<Camera>();
	camera->SetOrthographicSize(Screen::GetHeight() / 200.f);
    camera->SetCullingMask(LayerMask::GetMask(Layer::UI));
    camera->SetDepth(1);
    camera->SetClearFlags(CameraClearFlags::Nothing);

	auto label = Label::Create("", "consola", 20, LabelPivot::LeftTop, LabelAlign::Auto, true);
	auto tr = GameObject::Create("label")->AddComponent<TextRenderer>();
	tr->GetTransform()->SetPosition(Vector3(-Screen::GetWidth()/2.0f, Screen::GetHeight()/2.0f, 0) * 0.01f);
	tr->SetLabel(label);
	tr->SetSortingOrder(1000, 0);
	fps = tr;
	fps->GetTransform()->SetParent(camera->GetTransform());
    fps->GetGameObject()->SetLayer(Layer::UI);

    /*
    auto cam3d = GameObject::Create("camera")->AddComponent<Camera>();
    cam3d->SetOrthographic(false);
    cam3d->SetClipPlane(0.3f, 1000.0f);
    cam3d->SetCullingMask(LayerMask::GetMask(Layer::Default));
    cam3d->GetTransform()->SetPosition(Vector3(0, 1, -5));
    cam3d->SetDepth(0);
    cam3d->SetClearFlags(CameraClearFlags::SolidColor);

    auto mesh = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/Arthas.mesh");
    mesh->SetLayerRecursive(Layer::Default);
    mesh->GetTransform()->SetPosition(Vector3(-1.5f, 0, 0));
    pmesh = mesh.get();

    auto anim = Mesh::LoadSkinnedMesh(Application::GetDataPath() + "/Assets/mesh/Arthas.anim");
    anim->SetLayerRecursive(Layer::Default);
    anim->GetTransform()->SetPosition(Vector3(2, 0, 0));
    anim->GetTransform()->SetRotation(Quaternion::Euler(0, 180, 0));

    std::string clip = "Idle_Arthas_36896b399471f50409feff906777c5af.1.clip";
    auto anim_com = anim->GetComponent<Animation>();
    anim_com->GetAnimationState(clip)->wrap_mode = WrapMode::Loop;
    anim_com->Play(clip);

    auto ui = GameObject::Create("")->AddComponent<SpriteRenderer>();
    ui->SetSprite(Sprite::LoadFromFile(Application::GetDataPath() + "/Assets/mesh/arthas.jpg"));
    ui->GetGameObject()->SetLayer(Layer::UI);
    */

    auto cam3d = GameObject::Create("camera")->AddComponent<Camera>();
    cam3d->SetOrthographic(false);
    cam3d->SetFieldOfView(30);
    cam3d->SetClipPlane(0.3f, 1000.0f);
    cam3d->SetCullingMask(LayerMask::GetMask(Layer::Default));
    cam3d->SetDepth(0);
    cam3d->SetClearFlags(CameraClearFlags::SolidColor);
    cam3d->GetTransform()->SetPosition(Vector3(100, 8, 115));
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
        200 / 512.0f, 600.0f,
        Application::GetDataPath() + "/Assets/terrain/Terrain.raw",
        Application::GetDataPath() + "/Assets/terrain/Terrain.png",
        terrain_texs, 3);
    auto renderer = terrain_obj->AddComponent<TerrainRenderer>();
    renderer->SetTerrain(ter);
    renderer->SetSharedMaterial(ter->GetSharedMaterial());

    auto lightmap_ter = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/terrain/LightmapFar-1.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
    ter->GetSharedMaterial()->SetTexture("_Lightmap", lightmap_ter);

    auto lightmap = Texture2D::LoadFromFile(Application::GetDataPath() + "/Assets/terrain/Objects/LightmapFar-0.png", FilterMode::Bilinear, TextureWrapMode::Clamp);
    LightmapSettings::lightmaps.push_back(lightmap);

    auto mesh = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/terrain/Objects/Objects.mesh");
    mesh->SetLayerRecursive(Layer::Default);

    auto anim = Mesh::LoadSkinnedMesh(Application::GetDataPath() + "/Assets/mesh/Arthas.anim");
    anim->SetLayerRecursive(Layer::Default);
    anim->GetTransform()->SetPosition(Vector3(91, 0, 103));
    anim->GetTransform()->SetRotation(Quaternion::Euler(0, 0, 0));
    anim->GetTransform()->SetScale(Vector3(1, 1, 1) * 0.5f);

    std::string clip = "Idle_Arthas_36896b399471f50409feff906777c5af.1.clip";
    auto anim_com = anim->GetComponent<Animation>();
    anim_com->GetAnimationState(clip)->wrap_mode = WrapMode::Loop;
    anim_com->Play(clip);

    RenderSettings::light_ambient = Color(1, 1, 1, 1) * 0.2f;
    RenderSettings::light_directional_color = Color(1, 1, 1, 1) * 0.6f;
    RenderSettings::light_directional_intensity = 1;
    RenderSettings::light_directional_direction = Vector3(0, -1, -1);
}

void Launcher::Update()
{
	fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::m_fps).str + "\n" +
		"drawcall:" + GTString::ToString(GTTime::m_draw_call).str);
	fps->UpdateLabel();

    /*
    rot_y += 0.15f;
    pmesh->GetTransform()->SetRotation(Quaternion::Euler(0, rot_y, 0));
    */

	if(Input::GetTouchCount() > 0)
	{
		auto t = Input::GetTouch(0);

		if(t->phase == TouchPhase::Began)
		{
			
		}
		else if(
            t->phase == TouchPhase::Ended ||
			t->phase == TouchPhase::Canceled)
		{
			
		}
		else if(t->phase == TouchPhase::Moved)
		{
			
		}
	}
}

Launcher::~Launcher()
{
}
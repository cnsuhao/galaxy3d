#include "Component.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "RenderSettings.h"
#include "Application.h"
#include "Animation.h"
#include "LabelBatchRenderer.h"
#include "GTUIManager.h"
#include "GTString.h"

using namespace Galaxy3D;

static const float g_pixel_per_unit = 100.f;

class LauncherFighting : public Component
{
	LabelNode *m_fps;

	void Init2D()
	{
		GTUIManager::LoadFont("heiti", Application::GetDataPath() + "/Assets/font/heiti.ttc");

		auto cam = GameObject::Create("")->AddComponent<Camera>();
		cam->SetOrthographic(true);
		cam->SetOrthographicSize(1 / g_pixel_per_unit * Screen::GetHeight() * 0.5f);
		cam->SetClipPlane(-1, 1);
		cam->SetCullingMask(LayerMask::GetMask(Layer::UI));
		cam->SetClearColor(Color(0, 0, 0, 1));
		cam->SetDepth(1);
		cam->SetClearFlags(CameraClearFlags::Depth);

		auto canvas = GameObject::Create("")->AddComponent<UICanvas>();
		canvas->GetTransform()->SetParent(cam->GetTransform());
		canvas->GetTransform()->SetLocalPosition(Vector3::Zero());
		canvas->GetTransform()->SetLocalScale(Vector3::One() * (1.0f / g_pixel_per_unit));

		auto batch = GameObject::Create("")->AddComponent<LabelBatchRenderer>();
		batch->GetTransform()->SetParent(canvas->GetTransform());
		batch->GetTransform()->SetLocalPosition(Vector3::Zero());
		batch->GetTransform()->SetLocalScale(Vector3::One());

		auto label = Label::Create("", "heiti", 20, LabelPivot::Top, LabelAlign::Auto, true);
		auto node = GameObject::Create("")->AddComponent<LabelNode>();
		node->GetTransform()->SetParent(batch->GetTransform());
		node->GetTransform()->SetLocalScale(Vector3::One());
		node->SetLabel(label);
		node->SetSortingOrder(0);
		node->SetAnchor(Vector4(0.5f, 0, 0, 0));
		m_fps = node.get();
		batch->AddLabel(node);

		cam->GetGameObject()->SetLayerRecursively(Layer::UI);
	}

	virtual void Start()
	{
		Init2D();

		auto cam = GameObject::Create("")->AddComponent<Camera>();
		cam->GetTransform()->SetPosition(Vector3(0, 3, -5));
		cam->GetTransform()->SetRotation(Quaternion::Euler(30, 0, 0));
		cam->SetClipPlane(0.3f, 50.f);
		cam->SetCullingMask(LayerMask::GetMask(Layer::Default));
		cam->SetClearColor(Color(0, 0, 1, 1));
		cam->SetDepth(0);

		RenderSettings::light_ambient = Color(1, 1, 1, 1) * 0.2f;
		RenderSettings::GetGlobalDirectionalLight()->GetTransform()->SetRotation(Quaternion::Euler(45, -45, 0));
		RenderSettings::GetGlobalDirectionalLight()->SetIntensity(0.8f);
		RenderSettings::GetGlobalDirectionalLight()->EnableShadow(true);
		RenderSettings::GetGlobalDirectionalLight()->EnableCascade(true);

		auto ground = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/primitive/Cube.mesh");
		ground->GetTransform()->SetPosition(Vector3(0, -0.5f, 0));
		ground->GetTransform()->SetScale(Vector3(2000, 1, 2000));
		
		auto sphere = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/primitive/Sphere.mesh");
		sphere->GetTransform()->SetPosition(Vector3(2.f, 0.5f, 0));

		auto cube = GameObject::Instantiate(ground);
		cube->GetTransform()->SetPosition(Vector3(-2, 0.5f, 0));

		sphere = GameObject::Instantiate(sphere);
		sphere->GetTransform()->SetPosition(Vector3(-2, 0.5f, 8));

		sphere = GameObject::Instantiate(sphere);
		sphere->GetTransform()->SetPosition(Vector3(2, 0.5f, 8));

		sphere = GameObject::Instantiate(sphere);
		sphere->GetTransform()->SetPosition(Vector3(0, 0.5f, 2));

		sphere = GameObject::Instantiate(sphere);
		sphere->GetTransform()->SetPosition(Vector3(0, 0.5f, -2));

		auto anim_obj = Mesh::LoadSkinnedMesh(Application::GetDataPath() + "/Assets/mesh/anim/xiao_bie_li/xiao_bie_li.anim");
		anim_obj->GetTransform()->SetRotation(Quaternion::Euler(0, 180, 0));
		auto anim = anim_obj->GetComponent<Animation>();
		anim->GetAnimationState("idle")->wrap_mode = WrapMode::Loop;
		anim->Play("idle");

		Renderer::SortAllBatches();
	}

	virtual void Update()
	{
		m_fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::GetFPS()) + "\n" +
			"draw call:" + GTString::ToString(GTTime::GetDrawCall()));
	}
};
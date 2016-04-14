#include "Component.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "RenderSettings.h"
#include "Application.h"
#include "Animation.h"
#include "LabelBatchRenderer.h"
#include "GTUIManager.h"
#include "GTString.h"
#include "LayerMask.h"
#include "Layer.h"
#include "UICanvas.h"
#include "Input.h"
#include "SkyBox.h"

using namespace Galaxy3D;

extern bool g_show_shadow_map;
static const float g_pixel_per_unit = 100.f;
static bool g_mouse_down = false;
static Vector3 g_cam_rot;
static Vector3 g_mouse_down_pos;
static Camera *g_cam;
static float g_cam_dis;
static GameObject *g_car;

static Vector3 drag_cam_rot(Camera *cam3d)
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
    cam3d->GetTransform()->SetLocalPosition(Vector3(0, 0, 0) - cam3d->GetTransform()->GetForward() * g_cam_dis);

    Vector3 cam_target = cam3d->GetTransform()->GetPosition() + cam3d->GetTransform()->GetForward() * g_cam_dis;

    return rot_offset;
}

class LauncherDemo : public Component
{
	LabelNode *m_fps;

	virtual void Start()
	{
		Init2D();

		auto cam = GameObject::Create("")->AddComponent<Camera>();
		cam->GetTransform()->SetPosition(Vector3(0, 0, -4));
		cam->GetTransform()->SetForward(Vector3::Zero() - cam->GetTransform()->GetPosition());
		cam->SetClipPlane(0.3f, 100.f);
		cam->SetCullingMask(LayerMask::GetMask(Layer::Default));
		cam->SetClearColor(Color(1, 1, 1, 1) * 0.3f);
		cam->SetDepth(0);
		g_cam = cam.get();
		g_cam_dis = 4;
		g_cam_rot = Vector3::Zero();

		RenderSettings::light_ambient = Color(1, 1, 1, 1) * 0.2f;
		RenderSettings::GetGlobalDirectionalLight()->GetTransform()->SetRotation(Quaternion::Euler(30, 45, 0));
		RenderSettings::GetGlobalDirectionalLight()->SetIntensity(0.8f);
		RenderSettings::GetGlobalDirectionalLight()->EnableShadow(true);
		RenderSettings::GetGlobalDirectionalLight()->EnableCascade(true);

		std::vector<std::string> sky_textures;
		for(int i=0; i<10; i++)
		{
			for(int j=0; j<6; j++)
			{
				auto name = "sphere map_" + GTString::ToString(i) + "_" +  GTString::ToString(j) + ".png";
				sky_textures.push_back(Application::GetDataPath() + "/Assets/mesh/car/sky/" + name);
			}
		}
		auto cubemap = Cubemap::LoadFromFile(sky_textures, FilterMode::Trilinear, TextureWrapMode::Clamp, true, false, 10);
		
		/*
		auto sky = cam->GetGameObject()->AddComponent<SkyBox>();
		sky->SetCubemap(cubemap);

		auto anim_obj = Mesh::LoadSkinnedMesh(Application::GetDataPath() + "/Assets/mesh/anim/xiao_bie_li/xiao_bie_li.anim");
		anim_obj->GetTransform()->SetPosition(Vector3(-3, 0, 0));
		anim_obj->GetTransform()->SetRotation(Quaternion::Euler(0, 180, 0));
		auto anim = anim_obj->GetComponent<Animation>();
		anim->GetAnimationState("idle")->wrap_mode = WrapMode::Loop;
		anim->Play("idle");
		
		
		auto sphere = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/primitive/Sphere.mesh");
		sphere->GetTransform()->SetPosition(Vector3(3.f, 0.5f, 0));
		*/
		auto ground = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/primitive/Cube.mesh", Vector3(20, 1, 20));
		ground->GetTransform()->SetPosition(Vector3(0, -0.5f, 0));

		auto car = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/car/car.mesh");
		auto rs = car->GetComponentsInChildren<MeshRenderer>();
		for(auto &i : rs)
		{
			auto mats = i->GetSharedMaterials();
			for(auto &j : mats)
			{
				if( j->GetShader()->GetName() == "PBR/Standard" ||
					j->GetShader()->GetName() == "PBR/StandardTransparent")
				{
					j->SetTexture("_ReflectionMap", cubemap);

					if(	i->GetName() == "body_MeshPart0" ||
						i->GetName() == "body_MeshPart1")
					{
						j->SetVector("_Metallic", Vector4(0, 0, 0, 0));
						j->SetVector("_Smoothness", Vector4(1, 0, 0, 0));
					}
				}
			}
		}
		g_car = car.get();

		/*
		{
			auto sphere = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/primitive/Sphere.mesh");
			sphere->GetTransform()->SetPosition(Vector3(-3.f, 0.5f, 0));
			auto rs = sphere->GetComponentsInChildren<MeshRenderer>();
			for(auto &i : rs)
			{
				auto mats = i->GetSharedMaterials();
				for(auto &j : mats)
				{
					j->SetShader(Shader::Find("PBR/Standard"));
					j->SetTexture("_ReflectionMap", cubemap);
					j->SetVector("_Metallic", Vector4(1, 0, 0, 0));
					j->SetVector("_Smoothness", Vector4(0.7f, 0, 0, 0));
				}
			}
		}*/
	}

	virtual void Update()
	{
		m_fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::GetFPS()) + "\n" +
			"draw call:" + GTString::ToString(GTTime::GetDrawCall())
			);

		if(Input::GetMouseButtonDown(0))
		{
			g_mouse_down = true;
			g_mouse_down_pos = Input::GetMousePosition();

			//g_show_shadow_map = true;
		}

		if(Input::GetMouseButton(0))
		{
			drag_cam_rot(g_cam);
		}

		if(Input::GetMouseButtonUp(0))
		{
			auto rot_offset = drag_cam_rot(g_cam);

			g_cam_rot = g_cam_rot + rot_offset;
			g_mouse_down = false;

			//g_show_shadow_map = false;
		}

		static float s_car_rot = 0;
		s_car_rot += 0.6f;
		if(s_car_rot > 360)
		{
			s_car_rot -= 360;
		}
		//g_car->GetTransform()->SetRotation(Quaternion::Euler(0, s_car_rot, 0));
	}

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
};
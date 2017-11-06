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
#include "CameraLookAroundController.hpp"
#include "Editor.hpp"

using namespace Galaxy3D;

#define EDITOR 1

extern bool g_show_shadow_map;
static const float g_pixel_per_unit = 100.f;
static Camera *g_cam;
static GameObject *g_car;

class LauncherDemo : public Component
{
	LabelNode *m_fps = NULL;
	Material *m_water_mat = NULL;
	Editor *m_editor = NULL;

	virtual void Start()
	{
		Init2D();

		auto cam = GameObject::Create("")->AddComponent<Camera>();
		cam->GetTransform()->SetPosition(Vector3(-104, 5, 50));
		cam->GetTransform()->SetRotation(Quaternion::Euler(0, 159, 0));
		cam->SetClipPlane(0.3f, 500.f);
		cam->SetCullingMask(LayerMask::GetMask(Layer::Default));
		cam->SetClearColor(Color(1, 1, 1, 1) * 0.3f);
		cam->SetDepth(0);
		cam->SetFieldOfView(45);
		g_cam = cam.get();

		auto cc = cam->GetGameObject()->AddComponent<CameraLookAroundController>();
		cc->cam_dis = 10;
		cc->cam_rot = Vector3(0, 159, 0);
		cc->cam_target = cam->GetTransform()->GetPosition() + cam->GetTransform()->GetForward() * cc->cam_dis;

		RenderSettings::light_ambient = Color(1, 1, 1, 1) * 0.2f;
		RenderSettings::GetGlobalDirectionalLight()->GetTransform()->SetRotation(Quaternion::Euler(10, 180, 0));
		RenderSettings::GetGlobalDirectionalLight()->SetIntensity(1.0f);
		//RenderSettings::GetGlobalDirectionalLight()->EnableShadow(true);
		//RenderSettings::GetGlobalDirectionalLight()->EnableCascade(true);

		std::vector<std::string> sky_textures;
		for (int i = 0; i < 12; i++)
		{
			for (int j = 0; j < 6; j++)
			{
				auto name = "cubemap_" + GTString::ToString(i) + "_" + GTString::ToString(j) + ".png";
				sky_textures.push_back(Application::GetDataPath() + "/Assets/mesh/scene/sky/" + name);
			}
		}
		auto cubemap = Cubemap::LoadFromFile(sky_textures, FilterMode::Trilinear, TextureWrapMode::Clamp, true, false, 10);

		auto sky = cam->GetGameObject()->AddComponent<SkyBox>();
		sky->SetCubemap(cubemap);

		auto water = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/scene/water.mesh");
		auto rs = water->GetComponentsInChildren<MeshRenderer>();
		for(auto &i : rs)
		{
			auto mat = i->GetSharedMaterial();
			mat->SetTexture("_ReflectionMap", cubemap);
			m_water_mat = mat.get();
			break;
		}

		auto scene = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/scene/scene.mesh");
		rs = scene->GetComponentsInChildren<MeshRenderer>();
		for(auto &i : rs)
		{
			auto mats = i->GetSharedMaterials();
			for(auto &j : mats)
			{
				j->SetTexture("_ReflectionMap", cubemap);
			}
		}

		/*
		auto anim_obj = Mesh::LoadSkinnedMesh(Application::GetDataPath() + "/Assets/mesh/anim/xiao_bie_li/xiao_bie_li.anim");
		anim_obj->GetTransform()->SetPosition(Vector3(-3, 0, 0));
		anim_obj->GetTransform()->SetRotation(Quaternion::Euler(0, 180, 0));
		auto anim = anim_obj->GetComponent<Animation>();
		anim->GetAnimationState("idle")->wrap_mode = WrapMode::Loop;
		anim->Play("idle");
		
		
		auto sphere = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/primitive/Sphere.mesh");
		sphere->GetTransform()->SetPosition(Vector3(3.f, 0.5f, 0));
		*/

		/*
		auto ground = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/primitive/Cube.mesh", Vector3(20, 1, 20));
		ground->GetTransform()->SetPosition(Vector3(0, -0.5f, 0));

		auto car = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/car/car.mesh");
		car->GetTransform()->SetPosition(Vector3(7, -3, 0));
		rs = car->GetComponentsInChildren<MeshRenderer>();
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
		
		auto cams = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/cam/cams.mesh");
		cams->GetTransform()->SetPosition(Vector3(-2, 1, 0));
		rs = cams->GetComponentsInChildren<MeshRenderer>();
		for(auto &i : rs)
		{
			auto mats = i->GetSharedMaterials();
			for(auto &j : mats)
			{
				j->SetTexture("_ReflectionMap", cubemap);
			}
		}


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
		if(m_fps)
		{
			m_fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::GetFPS()) + "\n" +
				"draw call:" + GTString::ToString(GTTime::GetDrawCall())
				);
		}
		
		static float s_car_rot = 0;
		s_car_rot += 0.6f;
		if(s_car_rot > 360)
		{
			s_car_rot -= 360;
		}
		//g_car->GetTransform()->SetRotation(Quaternion::Euler(0, s_car_rot, 0));

		if(m_water_mat)
		{
			m_water_mat->SetVector("_Time", GTTime::GetTime());
		}
	}

	void Init2D()
	{
		GTUIManager::LoadFont("heiti", Application::GetDataPath() + "/Assets/font/heiti.ttc");

		auto cam = GameObject::Create("")->AddComponent<Camera>();
		cam->SetOrthographic(true);
		cam->SetClipPlane(-1, 1);
		cam->SetCullingMask(LayerMask::GetMask(Layer::UI) | LayerMask::GetMask(Layer::Editor));
		cam->SetClearColor(Color(0, 0, 0, 1));
		cam->SetDepth(1);
		cam->SetClearFlags(CameraClearFlags::Depth);
		cam->SetOrthographicSize(1 / g_pixel_per_unit * cam->GetPixelHeight() * 0.5f);

		auto canvas = GameObject::Create("")->AddComponent<UICanvas>();
		canvas->GetTransform()->SetParent(cam->GetTransform());
		canvas->GetTransform()->SetLocalPosition(Vector3::Zero());
		canvas->GetTransform()->SetLocalScale(Vector3::One() * (1.0f / g_pixel_per_unit));
		canvas->SetCamera(cam);

		auto batch = GameObject::Create("")->AddComponent<LabelBatchRenderer>();
		batch->GetTransform()->SetParent(canvas->GetTransform());
		batch->GetTransform()->SetLocalPosition(Vector3::Zero());
		batch->GetTransform()->SetLocalScale(Vector3::One());
		batch->SetSortingOrder(0, 0);

		auto label = Label::Create("", "heiti", 12, LabelPivot::Top, LabelAlign::Auto, true);
		auto node = GameObject::Create("")->AddComponent<LabelNode>();
		node->GetTransform()->SetParent(batch->GetTransform());
		node->GetTransform()->SetLocalScale(Vector3::One());
		node->SetLabel(label);
		node->SetSortingOrder(0);
		node->SetAnchor(Vector4(0.5f, 0, 0, 0));
		m_fps = node.get();
		batch->AddLabel(node);

		auto editor = GameObject::Create("Editor")->AddComponent<Editor>();
		editor->canvas = canvas.get();

		cam->GetGameObject()->SetLayerRecursively(Layer::UI);
	}
};
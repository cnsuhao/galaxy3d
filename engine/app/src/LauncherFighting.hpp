#include "Component.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "RenderSettings.h"
#include "Application.h"
#include "Animation.h"

using namespace Galaxy3D;

class LauncherFighting : public Component
{
	virtual void Start()
	{
		auto cam = GameObject::Create("")->AddComponent<Camera>();
		cam->GetTransform()->SetPosition(Vector3(0, 3, -5));
		cam->GetTransform()->SetRotation(Quaternion::Euler(30, 0, 0));
		cam->SetClipPlane(1, 50.f);
		cam->SetClearColor(Color(0, 0, 1, 1));

		RenderSettings::light_ambient = Color(1, 1, 1, 1) * 0.2f;
		RenderSettings::GetGlobalDirectionalLight()->GetTransform()->SetRotation(Quaternion::Euler(45, -45, 0));
		RenderSettings::GetGlobalDirectionalLight()->SetIntensity(0.8f);
		RenderSettings::GetGlobalDirectionalLight()->EnableShadow(true);
		RenderSettings::GetGlobalDirectionalLight()->EnableCascade(true);

		auto ground = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/primitive/Cube.mesh");
		ground->GetTransform()->SetPosition(Vector3(0, -0.5f, 0));
		ground->GetTransform()->SetScale(Vector3(20, 1, 20));
		auto sphere = Mesh::LoadStaticMesh(Application::GetDataPath() + "/Assets/mesh/primitive/Sphere.mesh");
		sphere->GetTransform()->SetPosition(Vector3(2.f, 0.5f, 0));
		auto cube = GameObject::Instantiate(ground);
		cube->GetTransform()->SetPosition(Vector3(-2, 0.5f, 0));
		
		auto anim_obj = Mesh::LoadSkinnedMesh(Application::GetDataPath() + "/Assets/mesh/anim/xiao_bie_li/xiao_bie_li.anim");
		anim_obj->GetTransform()->SetRotation(Quaternion::Euler(0, 180, 0));
		auto anim = anim_obj->GetComponent<Animation>();
		anim->GetAnimationState("idle")->wrap_mode = WrapMode::Loop;
		anim->Play("idle");
	}
};
#include "Component.h"
#include "Screen.h"
#include "LayerMask.h"
#include "Layer.h"
#include "UICanvas.h"
#include "SpriteRenderer.h"
#include "Application.h"
#include "GTTime.h"
#include "TweenColor.h"
#include "LauncherPlant.hpp"

#define Launcher LauncherPlant

using namespace Galaxy3D;

class SplashScreen : public Component
{
	float m_pixel_per_unit = 100;

	virtual void Start()
	{
		auto cam = GameObject::Create("")->AddComponent<Camera>();
		cam->SetOrthographic(true);
		cam->SetOrthographicSize(1 / m_pixel_per_unit * Screen::GetHeight() / 2);
		cam->SetClipPlane(-1, 1);
		cam->SetCullingMask(LayerMask::GetMask(Layer::UI));
		cam->SetDepth(0);
		cam->SetClearColor(Color(0, 0, 0, 1));

		auto canvas = GameObject::Create("")->AddComponent<UICanvas>();
		canvas->GetTransform()->SetParent(cam->GetTransform());
		canvas->GetTransform()->SetLocalScale(Vector3::One() * (1.0f / m_pixel_per_unit));

		auto sprite = Sprite::LoadFromFile(Application::GetDataPath() + "/Assets/logo.png");
		auto renderer = GameObject::Create("")->AddComponent<SpriteRenderer>();
		renderer->GetTransform()->SetParent(canvas->GetTransform());
		if(Screen::GetHeight() >= 1440)
		{
			renderer->GetTransform()->SetLocalScale(Vector3::One() * 2);
		}
		else
		{
			renderer->GetTransform()->SetLocalScale(Vector3::One());
		}
		renderer->SetSprite(sprite);

		cam->GetGameObject()->SetLayerRecursively(Layer::UI);
		cam->GetTransform()->SetParent(GetTransform());

		auto tc = renderer->GetGameObject()->AddComponent<TweenColor>();
		tc->delay = 1;
		tc->duration = 0.8f;
		tc->from = Color(1, 1, 1, 1);
		tc->to = Color(1, 1, 1, 0);
		tc->curve = AnimationCurve();
		tc->curve.keys.push_back(Keyframe(0, 0, 1, 1));
		tc->curve.keys.push_back(Keyframe(0.7f, 1, 1, 0));
		tc->curve.keys.push_back(Keyframe(1, 1, 0, 0));
		tc->target = GetComponentPtr();
		tc->on_set_value =
			[renderer](Component *tween, std::weak_ptr<Component> &target, void *value)
			{
				Color c = *(Color *) value;
				renderer->SetColor(c);
			};
		tc->on_finished = 
			[this](Component *tween, std::weak_ptr<Component> &target)
			{
				GameObject::Create("")->AddComponent<Launcher>();
				auto obj = this->GetGameObject();
				GameObject::Destroy(obj);
			};
	}
};
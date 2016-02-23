#include "LauncherMerged.h"
#include "GameObject.h"
#include "Camera.h"
#include "Application.h"
#include "Sprite.h"
#include "SpriteRenderer.h"
#include "Layer.h"
#include "LayerMask.h"
#include "UICanvas.h"
#include "Screen.h"

namespace Galaxy3D
{
    void LauncherMerged::Start()
    {
		float pixel_per_unit = 100;

        auto cam = GameObject::Create("")->AddComponent<Camera>();
        cam->SetOrthographic(true);
		cam->SetOrthographicSize(1 / pixel_per_unit * Screen::GetHeight() / 2);
		cam->SetClipPlane(-1, 1);
		cam->SetCullingMask(LayerMask::GetMask(Layer::UI));
		cam->SetDepth(1);
		cam->SetClearColor(Color(0.3f, 0.3f, 0.3f, 1));

		auto canvas = GameObject::Create("")->AddComponent<UICanvas>();
		canvas->GetTransform()->SetParent(cam->GetTransform());
		canvas->GetTransform()->SetScale(Vector3(1, 1, 1) * (1 / pixel_per_unit));

		auto bg = GameObject::Create("")->AddComponent<SpriteRenderer>();
		auto s = Sprite::LoadFromFile(Application::GetDataPath() + "/Assets/image/bg.png");
		bg->SetSprite(s);
		bg->SetSortingOrder(0, 0);
		bg->GetTransform()->SetParent(canvas->GetTransform());
		bg->GetTransform()->SetLocalScale(Vector3(1, 1, 1) * 0.75f);

		cam->GetGameObject()->SetLayerRecursively(Layer::UI);
		cam->GetTransform()->SetParent(GetTransform());
    }
}
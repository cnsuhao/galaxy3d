#include "Component.h"
#include "LabelBatchRenderer.h"

class LauncherBasic : public Component
{
	LabelNode *m_fps;

	virtual void Start()
	{
		GTUIManager::LoadFont("heiti", Application::GetDataPath() + "/Assets/font/heiti.ttc");

		auto cam = GameObject::Create("")->AddComponent<Camera>();
		cam->SetOrthographic(true);
		cam->SetOrthographicSize(1 / g_pixel_per_unit * Screen::GetHeight() * 0.5f);
		cam->SetClipPlane(-1, 1);
		cam->SetCullingMask(LayerMask::GetMask(Layer::UI));
		cam->SetClearColor(Color(0, 0, 0, 1));

		auto canvas = GameObject::Create("")->AddComponent<UICanvas>();
		canvas->GetTransform()->SetParent(cam->GetTransform());
		canvas->GetTransform()->SetLocalPosition(Vector3::Zero());
		canvas->GetTransform()->SetLocalScale(Vector3::One() * (1.0f / g_pixel_per_unit));

		auto batch = GameObject::Create("")->AddComponent<LabelBatchRenderer>();
		batch->GetTransform()->SetParent(canvas->GetTransform());
		batch->GetTransform()->SetLocalPosition(Vector3::Zero());
		batch->GetTransform()->SetLocalScale(Vector3::One());

		auto label = Label::Create("Label", "heiti", 30, LabelPivot::Center, LabelAlign::Auto, true);
		auto node = GameObject::Create("")->AddComponent<LabelNode>();
		node->GetTransform()->SetParent(batch->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3::Zero());
		node->GetTransform()->SetLocalScale(Vector3::One());
		node->SetLabel(label);
		node->SetSortingOrder(0);
		batch->AddLabel(node);

		label = Label::Create("", "heiti", 20, LabelPivot::Top, LabelAlign::Auto, true);
		node = GameObject::Create("")->AddComponent<LabelNode>();
		node->GetTransform()->SetParent(batch->GetTransform());
		node->GetTransform()->SetLocalScale(Vector3::One());
		node->SetLabel(label);
		node->SetSortingOrder(0);
		node->SetAnchor(Vector4(0.5f, 0, 0, 0));
		m_fps = node.get();
		batch->AddLabel(node);

		cam->GetGameObject()->SetLayerRecursively(Layer::UI);
		cam->GetTransform()->SetParent(GetTransform());
	}

	virtual void Update()
	{
		m_fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::GetFPS()) + "\n" +
			"draw call:" + GTString::ToString(GTTime::GetDrawCall()));
	}
};
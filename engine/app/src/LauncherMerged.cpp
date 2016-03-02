#include "LauncherMerged.h"
#include "GameMerged.hpp"

static void on_pause_tween_color_set_value(Component *tween, std::weak_ptr<Component> &target, void *value)
{
	Color color = *((Color *) value);
		
	auto node = tween->GetGameObject()->GetComponent<SpriteNode>();
	node->SetColor(color);
}

static void on_pause_tween_scale_set_value(Component *tween, std::weak_ptr<Component> &target, void *value)
{
	Vector3 scale = *((Vector3 *) value);

	tween->GetTransform()->SetLocalScale(scale);
	auto cover = tween->GetTransform()->Find("cover");
	cover->SetScale(tween->GetTransform()->GetParent().lock()->GetScale());
}

static void on_pause_tween_scale_finish(Component *tween, std::weak_ptr<Component> &target)
{
	g_window_pause->SetActive(false);
}

static void write_score_best()
{
	auto path = Application::GetSavePath() + "/Save/score_best.bin";
	GTFile::WriteAllBytes(path, &g_score_best, 4);
}

static void read_score_best()
{
	auto path = Application::GetSavePath() + "/Save/score_best.bin";
	if(GTFile::Exist(path))
	{
		int size;
		auto bytes = GTFile::ReadAllBytes(path, &size);
		g_score_best = *(int *) bytes;
		free(bytes);
	}
}

static void on_lose()
{
	auto over = g_window_pause->GetTransform()->Find("over")->GetGameObject();
	over->GetComponent<TextRenderer>()->GetLabel()->SetText(
		"Game Over\n<size=60>" + 
		Localization::GetString("score") + " : " + GTString::ToString(g_score).str + "\n" +
		Localization::GetString("score_best") + " : " + GTString::ToString(g_score_best).str + "</size>");
	over->SetActive(true);
	g_window_pause->GetTransform()->Find("continue")->GetGameObject()->SetActive(false);
	g_window_pause->SetActive(true);
	g_window_pause->GetTransform()->SetLocalScale(Vector3(1, 1, 1) * 0.1f);
		
	auto t = g_window_pause->AddComponent<TweenScale>();
	t->duration = 0.2f;
	t->curve = AnimationCurve();
	t->curve.keys.push_back(Keyframe(0, 0, 0, 0));
	t->curve.keys.push_back(Keyframe(1, 1, 0, 0));
	t->from = t->GetTransform()->GetLocalScale();
	t->to = Vector3(1, 1, 1);
	t->target = t;
	t->on_set_value = on_pause_tween_scale_set_value;

	auto cover = g_window_pause->GetTransform()->Find("cover");
	auto tc = cover->GetGameObject()->AddComponent<TweenColor>();
	tc->duration = 0.2f;
	tc->curve = AnimationCurve();
	tc->curve.keys.push_back(Keyframe(0, 0, 0, 0));
	tc->curve.keys.push_back(Keyframe(1, 1, 0, 0));
	tc->from = Color(1, 1, 1, 0);
	tc->to = Color(1, 1, 1, 0.7f);
	tc->target = tc;
	tc->on_set_value = on_pause_tween_color_set_value;

	write_score_best();
}

struct PauseButtonEventListener : UIEventListener
{
	virtual void OnClick()
	{
		g_window_pause->GetTransform()->Find("over")->GetGameObject()->SetActive(false);
		g_window_pause->GetTransform()->Find("continue")->GetGameObject()->SetActive(true);
		g_window_pause->SetActive(true);
		g_window_pause->GetTransform()->SetLocalScale(Vector3(1, 1, 1) * 0.1f);
		
		auto t = g_window_pause->AddComponent<TweenScale>();
		t->duration = 0.2f;
		t->curve = AnimationCurve();
		t->curve.keys.push_back(Keyframe(0, 0, 0, 0));
		t->curve.keys.push_back(Keyframe(1, 1, 0, 0));
		t->from = t->GetTransform()->GetLocalScale();
		t->to = Vector3(1, 1, 1);
		t->target = t;
		t->on_set_value = on_pause_tween_scale_set_value;

		auto cover = g_window_pause->GetTransform()->Find("cover");
		auto tc = cover->GetGameObject()->AddComponent<TweenColor>();
		tc->duration = 0.2f;
		tc->curve = AnimationCurve();
		tc->curve.keys.push_back(Keyframe(0, 0, 0, 0));
		tc->curve.keys.push_back(Keyframe(1, 1, 0, 0));
		tc->from = Color(1, 1, 1, 0);
		tc->to = Color(1, 1, 1, 0.7f);
		tc->target = tc;
		tc->on_set_value = on_pause_tween_color_set_value;
	}
};

struct ContinueEventListener : UIEventListener
{
	virtual void OnClick()
	{
		auto t = g_window_pause->AddComponent<TweenScale>();
		t->duration = 0.2f;
		t->curve = AnimationCurve();
		t->curve.keys.push_back(Keyframe(0, 0, 0, 0));
		t->curve.keys.push_back(Keyframe(1, 1, 0, 0));
		t->from = t->GetTransform()->GetLocalScale();
		t->to = Vector3(1, 1, 1) * 0.1f;
		t->target = t;
		t->on_set_value = on_pause_tween_scale_set_value;
		t->on_finished = on_pause_tween_scale_finish;

		auto cover = g_window_pause->GetTransform()->Find("cover");
		auto tc = cover->GetGameObject()->AddComponent<TweenColor>();
		tc->duration = 0.2f;
		tc->curve = AnimationCurve();
		tc->curve.keys.push_back(Keyframe(0, 0, 0, 0));
		tc->curve.keys.push_back(Keyframe(1, 1, 0, 0));
		tc->from = Color(1, 1, 1, 0.7f);
		tc->to = Color(1, 1, 1, 0);
		tc->target = tc;
		tc->on_set_value = on_pause_tween_color_set_value;
	}
};

struct RestartEventListener : ContinueEventListener
{
	virtual void OnClick()
	{
		ContinueEventListener::OnClick();

		restart();
	}
};

static std::shared_ptr<TextRenderer> create_label(GameObject *parent, Vector3 &pos, int font_size, LabelPivot::Enum pivot, int order)
{
	auto label = Label::Create("", "heiti", font_size, pivot, LabelAlign::Auto, true);
	auto tr = GameObject::Create("")->AddComponent<TextRenderer>();
	tr->SetLabel(label);
	tr->SetSortingOrder(order, 0);
	tr->GetTransform()->SetParent(parent->GetTransform());
	tr->GetTransform()->SetLocalScale(Vector3(1, 1, 1) * (1.0f / g_scale));
	tr->GetTransform()->SetLocalPosition(pos);

	return tr;
}

static void create_pause_window(UICanvas *canvas, UIAtlas *atlas)
{
	auto batch = GameObject::Create("window_pause")->AddComponent<SpriteBatchRenderer>();
	batch->GetTransform()->SetParent(canvas->GetTransform());
	batch->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
	batch->SetSortingOrder(2, 0);
	g_window_pause = batch->GetGameObject().get();

	auto sprite = atlas->CreateSprite(
		"black",
		Vector2(0.5f, 0.5f),
		100,
		Sprite::Type::Simple,
		Vector2(3840, 3840));
	auto node = GameObject::Create("cover")->AddComponent<SpriteNode>();
	node->GetTransform()->SetParent(batch->GetTransform());
	node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
	node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
	node->SetSprite(sprite);
	node->SetColor(Color(1, 1, 1, 0.7f));
	node->SetSortingOrder(0);
	batch->AddSprite(node);
	auto collider = node->GetGameObject()->AddComponent<BoxCollider>();
	collider->SetSize(Vector3(3840, 3840, 0));

	sprite = atlas->CreateSprite(
		"window",
		Vector2(0.5f, 0.5f),
		100,
		Sprite::Type::Simple,
		Vector2(0, 0));
	node = GameObject::Create("")->AddComponent<SpriteNode>();
	node->GetTransform()->SetParent(batch->GetTransform());
	node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
	node->GetTransform()->SetLocalScale(Vector3(1, 1, 1) * g_scale * 1.15f);
	node->SetSprite(sprite);
	batch->AddSprite(node);

	sprite = atlas->CreateSprite(
		"button",
		Vector2(0.5f, 0.5f),
		100,
		Sprite::Type::Simple,
		Vector2(0, 0));
	node = GameObject::Create("continue")->AddComponent<SpriteNode>();
	node->GetTransform()->SetParent(batch->GetTransform());
	node->GetTransform()->SetLocalPosition(Vector3(0, 170, 0));
	node->GetTransform()->SetLocalScale(Vector3(1, 1, 1) * g_scale);
	node->SetSprite(sprite);
	node->SetSortingOrder(1);
	batch->AddSprite(node);
	collider = node->GetGameObject()->AddComponent<BoxCollider>();
	collider->SetSize(Vector3(1013, 253, 0));
	node->GetGameObject()->AddComponent<ContinueEventListener>();
	auto label = create_label(node->GetGameObject().get(), Vector3(Mathf::Round(0 / g_scale), Mathf::Round(0 / g_scale), 0), 60, LabelPivot::Center, 3);
	label->GetLabel()->SetText(Localization::GetString("continue"));

	node = GameObject::Create("restart")->AddComponent<SpriteNode>();
	node->GetTransform()->SetParent(batch->GetTransform());
	node->GetTransform()->SetLocalPosition(Vector3(0, -170, 0));
	node->GetTransform()->SetLocalScale(Vector3(1, 1, 1) * g_scale);
	node->SetSprite(sprite);
	node->SetSortingOrder(1);
	batch->AddSprite(node);
	collider = node->GetGameObject()->AddComponent<BoxCollider>();
	collider->SetSize(Vector3(1013, 253, 0));
	node->GetGameObject()->AddComponent<RestartEventListener>();
	label = create_label(node->GetGameObject().get(), Vector3(Mathf::Round(0 / g_scale), Mathf::Round(0 / g_scale), 0), 60, LabelPivot::Center, 3);
	label->GetLabel()->SetText(Localization::GetString("restart"));

	label = create_label(batch->GetGameObject().get(), Vector3(0, 170, 0), 100, LabelPivot::Center, 3);
	label->SetName("over");
	label->GetGameObject()->SetActive(false);

	g_window_pause->SetActive(false);
}

void LauncherMerged::Start()
{
	read_score_best();

	GTUIManager::LoadFont("heiti", Application::GetDataPath() + "/Assets/font/heiti.ttc");
	Localization::LoadStrings(Application::GetDataPath() + "/Assets/string.txt");

	float pixel_per_unit = 100;

    auto cam = GameObject::Create("")->AddComponent<Camera>();
    cam->SetOrthographic(true);
	cam->SetOrthographicSize(1 / pixel_per_unit * Screen::GetHeight() / 2);
	cam->SetClipPlane(-1, 1);
	cam->SetCullingMask(LayerMask::GetMask(Layer::UI));
	cam->SetDepth(1);
	cam->SetClearColor(Color(42, 42, 42, 255) / 255.0f);

	cam->GetGameObject()->AddComponent<AudioListener>();

	auto canvas = GameObject::Create("")->AddComponent<UICanvas>();
	canvas->GetTransform()->SetParent(cam->GetTransform());
	canvas->GetTransform()->SetScale(Vector3(1, 1, 1) * (1.0f / pixel_per_unit * Screen::GetHeight() / 1920));
	canvas->SetSize(1080, 1920);

	auto label = Label::Create("", "heiti", 40, LabelPivot::Top, LabelAlign::Auto, true);
	auto tr = GameObject::Create("fps")->AddComponent<TextRenderer>();
	tr->SetLabel(label);
	tr->SetSortingOrder(1000, 0);
	tr->GetTransform()->SetParent(canvas->GetTransform());
	tr->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
	tr->SetAnchor(Vector4(0.5f, 0, 0, 0));
	g_fps = tr.get();
	g_fps->GetGameObject()->SetActive(false);

	auto batch = GameObject::Create("")->AddComponent<SpriteBatchRenderer>();
	batch->GetTransform()->SetParent(canvas->GetTransform());
	batch->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
	batch->SetSortingOrder(0, 0);
	g_root_batch = batch.get();

	auto atlas = GTUIManager::LoadAtlas("merged", Application::GetDataPath() + "/Assets/image/merged.json");
	
	auto sprite = atlas->CreateSprite(
		"destroy",
		Vector2(0, 1),
		100,
		Sprite::Type::Simple,
		Vector2(0, 0));
	auto node = GameObject::Create("")->AddComponent<SpriteNode>();
	node->GetTransform()->SetParent(batch->GetTransform());
	node->GetTransform()->SetLocalScale(Vector3(1, 1, 1) * g_scale);
	node->SetSprite(sprite);
	node->SetAnchor(Vector4(0, 1, 0, 60));
	batch->AddSprite(node);
	node->GetGameObject()->SetActive(false);

	auto destroy_price = create_label(node->GetGameObject().get(), Vector3(Mathf::Round(135 / g_scale), Mathf::Round(34 / g_scale), 0), 40, LabelPivot::Center, 1);
	destroy_price->GetLabel()->SetText(GTString::ToString(g_coin_destroy_price).str);

	sprite = atlas->CreateSprite(
		"best_and_coin",
		Vector2(0, 0),
		100,
		Sprite::Type::Simple,
		Vector2(0, 0));
	node = GameObject::Create("")->AddComponent<SpriteNode>();
	node->GetTransform()->SetParent(batch->GetTransform());
	node->GetTransform()->SetLocalScale(Vector3(1, 1, 1) * g_scale);
	node->SetSprite(sprite);
	node->SetAnchor(Vector4(0, 0, 30, -100));
	batch->AddSprite(node);
	node->GetGameObject()->SetActive(false);

	auto score_best = create_label(node->GetGameObject().get(), Vector3(Mathf::Round(90 / g_scale), Mathf::Round(-30 / g_scale), 0), 40, LabelPivot::Left, 1);
	score_best->GetLabel()->SetText(GTString::ToString(g_score_best).str);
	g_score_best_label = score_best->GetLabel().get();

	auto coin = create_label(node->GetGameObject().get(), Vector3(Mathf::Round(90 / g_scale), Mathf::Round(-98 / g_scale), 0), 40, LabelPivot::Left, 1);
	coin->GetLabel()->SetText(GTString::ToString(g_coin).str);

	sprite = atlas->CreateSprite(
		"pause",
		Vector2(1, 0),
		100,
		Sprite::Type::Simple,
		Vector2(0, 0));
	node = GameObject::Create("button_pause")->AddComponent<SpriteNode>();
	node->GetTransform()->SetParent(batch->GetTransform());
	node->GetTransform()->SetLocalScale(Vector3(1, 1, 1) * g_scale);
	node->SetSprite(sprite);
	node->SetAnchor(Vector4(1, 0, -30, -100));
	batch->AddSprite(node);
	auto collider = node->GetGameObject()->AddComponent<BoxCollider>();
	collider->SetCenter(Vector3(-90, -90, 0));
	collider->SetSize(Vector3(180, 180, 0));
	node->GetGameObject()->AddComponent<PauseButtonEventListener>();
	
	auto score = create_label(node->GetGameObject().get(), Vector3(Mathf::Round(-180 / g_scale), Mathf::Round(-75 / g_scale), 0), 80, LabelPivot::Right, 1);
	score->GetLabel()->SetText(GTString::ToString(g_score).str);
	g_score_label = score->GetLabel().get();

	sprite = atlas->CreateSprite(
		"tile",
		Vector2(0.5f, 0.5f),
		100,
		Sprite::Type::Simple,
		Vector2(0, 0));
	g_tile = sprite;
	for(int i=0; i<5; i++)
	{
		for(int j=0; j<5; j++)
		{
			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(batch->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(175.0f * (i - 2), 175.0f * (j - 2), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1) * g_scale);
			node->SetSprite(sprite);
			node->SetColor(Color(58, 58, 58, 255) / 255.0f);
			batch->AddSprite(node);
		}
	}
	
	auto rotate_batch = GameObject::Create("")->AddComponent<SpriteBatchRenderer>();
	rotate_batch->GetTransform()->SetParent(canvas->GetTransform());
	rotate_batch->GetTransform()->SetLocalPosition(Vector3(0, -750, 0));
	rotate_batch->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
	rotate_batch->SetSortingOrder(1, 0);
	g_rotate_batch = rotate_batch.get();

	node = GameObject::Create("rotate_collider")->AddComponent<SpriteNode>();
	node->GetTransform()->SetParent(g_rotate_batch->GetTransform());
	node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
	node->GetTransform()->SetLocalScale(Vector3(1, 1, 1) * 1.4f);
	node->SetSprite(g_tile.lock());
	node->SetColor(Color(1, 1, 1, 0));
	g_rotate_batch->AddSprite(node);
	collider = node->GetGameObject()->AddComponent<BoxCollider>();
	collider->SetSize(Vector3(215, 215, 0));
	auto event_listener = node->GetGameObject()->AddComponent<RotateTileEventListener>();
	g_rotate_collider = node->GetGameObject().get();

	auto rotate = GameObject::Create("rotate_circle");
	rotate->GetTransform()->SetParent(rotate_batch->GetTransform());
	rotate->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
	rotate->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
	g_rotate = rotate.get();
	g_rotate->SetActive(false);

	sprite = atlas->CreateSprite(
		"rotate",
		Vector2(0.5f, 0.5f),
		100,
		Sprite::Type::Simple,
		Vector2(0, 0));
	node = GameObject::Create("")->AddComponent<SpriteNode>();
	node->GetTransform()->SetParent(rotate->GetTransform());
	node->GetTransform()->SetLocalPosition(Vector3(0, 175 * g_scale, 0));
	node->GetTransform()->SetLocalRotation(Quaternion::Euler(0, 0, 0));
	node->GetTransform()->SetLocalScale(Vector3(1, 1, 1) * g_scale);
	node->SetSprite(sprite);
	rotate_batch->AddSprite(node);

	node = GameObject::Create("")->AddComponent<SpriteNode>();
	node->GetTransform()->SetParent(rotate->GetTransform());
	node->GetTransform()->SetLocalPosition(Vector3(-175 * g_scale, 0, 0));
	node->GetTransform()->SetLocalRotation(Quaternion::Euler(0, 0, 90));
	node->GetTransform()->SetLocalScale(Vector3(1, 1, 1) * g_scale);
	node->SetSprite(sprite);
	rotate_batch->AddSprite(node);

	node = GameObject::Create("")->AddComponent<SpriteNode>();
	node->GetTransform()->SetParent(rotate->GetTransform());
	node->GetTransform()->SetLocalPosition(Vector3(0, -175 * g_scale, 0));
	node->GetTransform()->SetLocalRotation(Quaternion::Euler(0, 0, 180));
	node->GetTransform()->SetLocalScale(Vector3(1, 1, 1) * g_scale);
	node->SetSprite(sprite);
	rotate_batch->AddSprite(node);

	node = GameObject::Create("")->AddComponent<SpriteNode>();
	node->GetTransform()->SetParent(rotate->GetTransform());
	node->GetTransform()->SetLocalPosition(Vector3(175 * g_scale, 0, 0));
	node->GetTransform()->SetLocalRotation(Quaternion::Euler(0, 0, 270));
	node->GetTransform()->SetLocalScale(Vector3(1, 1, 1) * g_scale);
	node->SetSprite(sprite);
	rotate_batch->AddSprite(node);

	create_pause_window(canvas.get(), atlas.get());

	cam->GetGameObject()->SetLayerRecursively(Layer::UI);
	cam->GetTransform()->SetParent(GetTransform());

	memset(g_tiles, 0, sizeof(g_tiles));
	new_tile(2);
}

void LauncherMerged::Update()
{
	std::string hit_name;
    auto hit = UICanvas::GetRayHitObject().lock();
    if(hit)
    {
        hit_name = hit->GetName();
    }

	g_fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::GetFPS()).str + "\n" +
		"draw call:" + GTString::ToString(GTTime::GetDrawCall()).str + "\n" +
        "ray hit ui:" + hit_name);

	static float s_rot = 0;
	s_rot += -60 * GTTime::GetDeltaTime();
	g_rotate->GetTransform()->SetLocalRotation(Quaternion::Euler(0, 0, s_rot));
}
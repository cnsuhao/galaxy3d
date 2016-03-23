#include "Component.h"
#include "LayerMask.h"
#include "Layer.h"
#include "UICanvas.h"
#include "Label.h"
#include "TextRenderer.h"
#include "Transform.h"
#include "GTUIManager.h"
#include "Application.h"
#include "SpriteBatchRenderer.h"
#include "GTString.h"
#include "TweenScale.h"
#include "TweenPosition.h"
#include "TweenRotation.h"
#include "BoxCollider.h"
#include "UIEventListener.h"
#include "AudioListener.h"
#include "AudioClip.h"
#include "AudioSource.h"
#include "GTFile.h"
#include "RenderTexture.h"
#include "SpriteRenderer.h"
#include "Input.h"
#include <deque>

using namespace Galaxy3D;

struct Card
{
	int type_0;
	int type_1;
	std::string name;
	int price_base;
	int price_sale;
	int level;
	int planted;
	std::weak_ptr<TextRenderer> label_price;
	std::weak_ptr<TextRenderer> label_planted;

	Card():
		type_0(-1),
		type_1(-1),
		name(""),
		price_base(-1),
		level(-1),
		planted(-1)
	{}
	Card(int type_0, int type_1, const std::string &name, int price_base, int price_sale, int level, int planted):
		type_0(type_0),
		type_1(type_1),
		name(name),
		price_base(price_base),
		price_sale(price_sale),
		level(level),
		planted(planted)
	{}
};

struct Plant : public Component
{
	int type_0;
	int type_1;
	int pos;
	std::shared_ptr<SpriteNode> tree;
	std::shared_ptr<SpriteNode> fruit;

	void InitFruits(const Vector3 *pos, int count);
};

static GameObject *g_grass_node_0;
static GameObject *g_grass_node_0_copy;
static GameObject *g_grass_node_1;
static GameObject *g_grass_node_1_copy;
static GameObject *g_ground_node;
static GameObject *g_ground_node_copy;
static GameObject *g_map_limit_left;
static GameObject *g_map_limit_right;
static GameObject *g_wave_node_0;
static GameObject *g_wave_node_1;
static GameObject *g_bag;
static GameObject *g_win_settings;
static SpriteBatchRenderer *g_batch_game;
static float g_grass_pos_0_init;
static float g_grass_pos_1_init;
static float g_ground_pos_init;
static float g_wave_pos_0_init;
static float g_wave_pos_1_init;
static float g_map_pos = 0;
static float g_map_pos_limit = 0;
static float g_grass_pos_0;
static float g_grass_pos_1;
static float g_ground_pos = 0;
static float g_wave_pos_0 = 0;
static float g_wave_pos_1 = 0;
static bool g_ui_bag_up = true;
static int g_tab_current = 0;
static Card g_cards[25] = {
	Card(0, 0, "carrot", 3, 1, 0, 0),
	Card(0, 1, "turnip", 4, 2, 0, 0),
	Card(0, 2, "pumpkin", 5, 3, 0, 0),
	Card(0, 3, "radish", 6, 4, 0, 0),
	Card(0, 4, "cabbage", 7, 5, 0, 0),

	Card(1, 0, "blueberry", 8, 6, 0, 0),
	Card(1, 1, "raspberry", 9, 7, 0, 0),
	Card(1, 2, "tomato", 10, 8, 0, 0),
	Card(1, 3, "bell pepper", 11, 9, 0, 0),
	Card(1, 4, "eggplant", 12, 10, 0, 0),

	Card(2, 0, "apple", 30, 2, 0, 0),
	Card(2, 1, "pear", 80, 4, 0, 0),
	Card(2, 2, "orange", 250, 6, 0, 0),
	Card(2, 3, "cherry", 800, 8, 0, 0),
	Card(2, 4, "lemon", 2000, 8, 0, 0),
};
static std::deque<std::shared_ptr<Plant>> g_plants[3];
static int g_gold = 10;
static TextRenderer *g_label_gold;
static int g_exp = 0;
static SpriteNode *g_sprite_exp;
static int g_level = 1;
static TextRenderer *g_label_level;

static void set_gold(int gold)
{
	g_gold = gold;

	g_label_gold->GetLabel()->SetText("<outline>" + GTString::ToString(g_gold).str + "</outline>");
}

static void set_level(int level)
{
	g_level = level;

	g_label_level->GetLabel()->SetText("<outline>Lv." + GTString::ToString(g_level).str + "</outline>");
}

static void set_exp(int exp)
{
	g_exp = exp;
	int exp_full = g_level * 10;

	float w = Mathf::Round(295 * g_exp / (float) exp_full);
	g_sprite_exp->GetSprite()->SetSize(Vector2(w, 56));
}

static void add_exp(int exp)
{
	int exp_full = g_level * 10;
	g_exp += exp;

	while(g_exp >= exp_full)
	{
		g_exp -= exp_full;
		g_level += 1;
		exp_full = g_level * 10;
	}

	set_level(g_level);
	set_exp(g_exp);
}

static void on_grow_finished(Component *tween, std::weak_ptr<Component> &target)
{
	//auto plant = (Plant *) target.lock().get();
	auto fruit = tween->GetGameObject();

	auto collider = fruit->GetComponent<BoxCollider>();
	collider->Enable(true);

	auto tr = fruit->AddComponent<TweenRotation>();
	tr->duration = 2.0f;
	tr->loop = true;
	tr->curve = AnimationCurve();
	tr->curve.keys.push_back(Keyframe(0, 0, 1, 1));
	tr->curve.keys.push_back(Keyframe(0.5f, 1, 1, -1));
	tr->curve.keys.push_back(Keyframe(1, 0, -1, -1));
	tr->from = Vector3(0, 0, -5);
	tr->to = Vector3(0, 0, 5);

	tr->SetTime(0.5f);
}

struct FruitEventListener : public UIEventListener
{
	std::weak_ptr<Plant> plant;

	virtual void OnClick()
	{
		Sell();
		FreshNew();
	}

	void Sell()
	{
		auto p = plant.lock();
		int index = p->type_0 * 5 + p->type_1;

		int sale = g_cards[index].price_sale;

		set_gold(g_gold + sale);
		add_exp(sale);
	}

	void FreshNew()
	{
		auto collider = GetGameObject()->GetComponent<BoxCollider>();
		collider->Enable(false);

		auto tr = GetGameObject()->GetComponent<TweenRotation>();
		if(tr)
		{
			auto com = std::dynamic_pointer_cast<Component>(tr);
			Component::Destroy(com);
			GetTransform()->SetRotation(Quaternion::Identity());
		}

		GetTransform()->SetLocalScale(Vector3(1, 1, 1) * 0.01f);

		auto ts = GetGameObject()->AddComponent<TweenScale>();
		ts->duration = 10.0f;
		ts->curve = AnimationCurve();
		ts->curve.keys.push_back(Keyframe(0, 0, 1, 1));
		ts->curve.keys.push_back(Keyframe(1, 1, 1, 1));
		ts->from = Vector3(1, 1, 1) * 0.01f;
		ts->to = Vector3(1, 1, 1);
		ts->target = plant;
		ts->on_finished = on_grow_finished;
	}
};

void Plant::InitFruits(const Vector3 *pos, int count)
{
	for(int i=0; i<count; i++)
	{
		auto node = GameObject::Create("fruit_" + GTString::ToString(type_0).str + "_" + GTString::ToString(type_1).str)->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(GetTransform());
		node->GetTransform()->SetLocalPosition(pos[i]);
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1) * 0.01f);
		node->SetSprite(fruit->GetSprite());
		if(type_0 == 0)
		{
			node->SetSortingOrder(7);
		}
		else
		{
			node->SetSortingOrder(8 - type_0 * 2);
		}
		tree->GetBatch().lock()->AddSprite(node);

		auto ts = node->GetGameObject()->AddComponent<TweenScale>();
		ts->duration = 10.0f;
		ts->curve = AnimationCurve();
		ts->curve.keys.push_back(Keyframe(0, 0, 1, 1));
		ts->curve.keys.push_back(Keyframe(1, 1, 1, 1));
		ts->from = Vector3(1, 1, 1) * 0.01f;
		ts->to = Vector3(1, 1, 1);
		ts->target = GetComponentPtr();
		ts->on_finished = on_grow_finished;

		node->GetGameObject()->SetLayer(Layer::UI);
		auto collider = node->GetGameObject()->AddComponent<BoxCollider>();
		auto size = fruit->GetSprite()->GetSize();
		collider->SetSize(Vector3(size));
		if(type_0 == 0)
		{
			collider->SetCenter(Vector3(0, size.y * 0.5f, 0));
		}
		else
		{
			collider->SetCenter(Vector3(0, -size.y * 0.5f, 0));
		}
		collider->Enable(false);
		auto listener = node->GetGameObject()->AddComponent<FruitEventListener>();
		listener->plant = std::dynamic_pointer_cast<Plant>(GetComponentPtr());
	}
}

struct PlantBig : public Plant
{
	virtual void Start()
	{
		const Vector3 fruit_pos[5][5] = {
			{Vector3(-110, 280), Vector3(-50, 380), Vector3(0, 310), Vector3(70, 370), Vector3(110, 270)},
			{Vector3(-90, 230), Vector3(-50, 360), Vector3(0, 270), Vector3(70, 370), Vector3(110, 220)},
			{Vector3(-100, 240), Vector3(-50, 340), Vector3(10, 230), Vector3(60, 360), Vector3(110, 260)},
			{Vector3(-100, 360), Vector3(-50, 260), Vector3(10, 320), Vector3(80, 250), Vector3(110, 370)},
			{Vector3(0, 200), Vector3(0, 200), Vector3(0, 200), Vector3(0, 200), Vector3(0, 200)}
		};

		InitFruits(fruit_pos[type_1], 5);
	}
};

struct PlantMiddle : public Plant
{
	virtual void Start()
	{
		
	}
};

struct PlantSmall4 : public Plant
{
	virtual void Start()
	{
		const Vector3 fruit_pos[4] = {
			Vector3(-120.f, -15),
			Vector3(-41.f, -15),
			Vector3(38.f, -15),
			Vector3(117.f, -15)
		};

		InitFruits(fruit_pos, 4);
	}
};

struct PlantSmall2 : public Plant
{
	virtual void Start()
	{
		const Vector3 fruit_pos[2] = {
			Vector3(-120.f, -15),
			Vector3(-41.f, -15)
		};

		InitFruits(fruit_pos, 2);
	}
};

static void set_wave_pos(int x, GameObject *node)
{
	x %= 199;
	if(x < 0)
	{
		x += 199;
	}

	int child_count = node->GetTransform()->GetChildCount();
	for(int i=0; i<child_count; i++)
	{
		auto c = node->GetTransform()->GetChild(i);
		c->SetLocalPosition(Vector3(-1920 / 2.0f + (i - 1) * 199.0f + x, 0, 0));
	}
}

static void set_ground_pos(int x, GameObject *node, GameObject *node_copy)
{
	x += 1920 / 2;
	x = x % 1920;
	if(x < 0)
	{
		x += 1920;
	}

	float pos_x = 1920 / 2.0f - x;
	float pos_y = node->GetTransform()->GetLocalPosition().y;

	if(pos_x > 0)
	{
		node_copy->GetTransform()->SetLocalPosition(Vector3(pos_x - 1920, pos_y, 0));
		node_copy->SetActive(true);
	}
	else
	{
		node_copy->GetTransform()->SetLocalPosition(Vector3(pos_x + 1920, pos_y, 0));
		node_copy->SetActive(true);
	}

	node->GetTransform()->SetLocalPosition(Vector3(pos_x, pos_y, 0));
}

static void set_grass_pos(int x, GameObject *node, GameObject *node_copy)
{
	x %= 2050;
	if(x < 0)
	{
		x += 2050;
	}

	float pos_x = 1025.0f - x;
	float pos_y = node->GetTransform()->GetLocalPosition().y;

	int screen_w = 1920;
	if(pos_x > 1025 - screen_w / 2)
	{
		node_copy->GetTransform()->SetLocalPosition(Vector3(pos_x - 2050, pos_y, 0));
		node_copy->SetActive(true);
	}
	else if(pos_x < -(1025 - screen_w / 2))
	{
		node_copy->GetTransform()->SetLocalPosition(Vector3(pos_x + 2050, pos_y, 0));
		node_copy->SetActive(true);
	}
	else
	{
		node_copy->SetActive(false);
	}

	node->GetTransform()->SetLocalPosition(Vector3(pos_x, pos_y, 0));
}

static int map_pos_to_plant(int type_0, float pos)
{
	int x = 0;

	if(type_0 == 0)
	{
		x = Mathf::RoundToInt((pos + g_map_pos * 0.9f) / 380);
	}
	else if(type_0 == 1)
	{
		x = Mathf::RoundToInt((pos + 150 + g_map_pos * 0.8f) / 300);
	}
	else if(type_0 == 2)
	{
		x = Mathf::RoundToInt((pos + 230 + g_map_pos * 0.7f) / 460);
	}

	return x;
}

static float plant_pos_to_map(int type_0, int pos)
{
	float x = 0;

	if(type_0 == 0)
	{
		x = pos * 380 - g_map_pos * 0.9f;
	}
	else if(type_0 == 1)
	{
		x = pos * 300 - g_map_pos * 0.8f - 150;
	}
	else if(type_0 == 2)
	{
		x = pos * 460 - g_map_pos * 0.7f - 230;
	}

	return x;
}

static void set_plants_pos()
{
	for(auto &plants : g_plants)
	{
		for(size_t i=0; i<plants.size(); i++)
		{
			if(plants[i])
			{
				plants[i]->GetTransform()->SetLocalPosition(Vector3(plant_pos_to_map(plants[i]->type_0, plants[i]->pos), -326, 0));
			}
		}
	}
}

static void set_map_pos(float x)
{
	float limit_pos_left = -g_map_pos_limit - x;
	float limit_pos_right = g_map_pos_limit - x;
	if(limit_pos_left > -800)
	{
		x = 800 - g_map_pos_limit;
	}
	else if(limit_pos_right < 800)
	{
		x = g_map_pos_limit - 800;
	}
	limit_pos_left = -g_map_pos_limit - x;
	limit_pos_right = g_map_pos_limit - x;

	float y = g_map_limit_left->GetTransform()->GetLocalPosition().y;
	g_map_limit_left->GetTransform()->SetLocalPosition(Vector3(limit_pos_left, y, 0));
	g_map_limit_right->GetTransform()->SetLocalPosition(Vector3(limit_pos_right, y, 0));

	g_map_pos = x;

	g_grass_pos_0 = g_grass_pos_0_init + g_map_pos * 0.4f;
	set_grass_pos((int) g_grass_pos_0, g_grass_node_0, g_grass_node_0_copy);
	g_grass_pos_1 = g_grass_pos_1_init + g_map_pos * 0.6f;
	set_grass_pos((int) g_grass_pos_1, g_grass_node_1, g_grass_node_1_copy);
	g_ground_pos = g_ground_pos_init + g_map_pos * 1;
	set_ground_pos((int) g_ground_pos, g_ground_node, g_ground_node_copy);
	g_wave_pos_0 = g_wave_pos_0_init - g_map_pos * 1.1f;
	set_wave_pos((int) g_wave_pos_0, g_wave_node_0);
	g_wave_pos_1 = g_wave_pos_1_init - g_map_pos * 1.1f;
	set_wave_pos((int) g_wave_pos_1, g_wave_node_1);

	set_plants_pos();
}

struct GroundEventListener : public UIEventListener
{
	const float momentum_amount = 30;
	Vector3 m_momentum;
	bool m_drag_end;
	bool m_draged;

	GroundEventListener():
		m_momentum(),
		m_drag_end(true),
		m_draged(false)
	{}

	virtual void OnDragStart()
	{
		m_drag_end = false;
		m_draged = false;
	}

	virtual void OnDrag(const Vector3 &delta)
	{
		float map_pos = g_map_pos - delta.x;

		set_map_pos(map_pos);

		m_draged = true;
		m_momentum = Vector3::Lerp(m_momentum, m_momentum + delta * (0.01f * momentum_amount), 0.67f);
	}

	virtual void OnDragEnd()
	{
		m_drag_end = true;
		m_draged = false;
	}

	static Vector3 SpringDampen(Vector3 &velocity, float strength, float delta_time)
	{
		if (delta_time > 1.0f) delta_time = 1.0f;
		float dampening_factor = 1.0f - strength * 0.001f;
		int ms = Mathf::RoundToInt(delta_time * 1000.0f);
		float total_dampening = pow(dampening_factor, ms);
		Vector3 total = velocity * ((total_dampening - 1.0f) / log(dampening_factor));
		velocity = velocity * total_dampening;
		return total * 0.06f;
	}

	virtual void LateUpdate()
	{
		if(m_drag_end && m_momentum.SqrMagnitude() > 1)
		{
			float dampen_strength = 9;
			float delta = GTTime::GetDeltaTime();
			Vector3 offset = SpringDampen(m_momentum, dampen_strength, delta);

			set_map_pos(g_map_pos - offset.x);
		}
		else
		{
			if(m_draged)
			{
				m_draged = false;
			}
			else
			{
				m_momentum *= 0.5f;
			}
		}
	}
};

struct ButtonSettingsEventListener : public UIEventListener
{
	virtual void OnClick()
	{
		g_win_settings->SetActive(true);

		auto cover = g_win_settings->GetTransform()->GetParent().lock()->Find("window settings cover")->GetGameObject()->GetComponent<SpriteNode>();
		cover->GetGameObject()->SetActive(true);
		cover->SetColor(Color(0, 0, 0, 0));

		auto tc = cover->GetGameObject()->AddComponent<TweenColor>();
		tc->duration = 0.2f;
		tc->from = Color(0, 0, 0, 0);
		tc->to = Color(0, 0, 0, 0.7f);
		tc->curve = AnimationCurve();
		tc->curve.keys.push_back(Keyframe(0, 0, 0, 0));
		tc->curve.keys.push_back(Keyframe(1, 1, 0, 0));
		tc->target = GetComponentPtr();
		tc->on_set_value =
			[cover](Component *tween, std::weak_ptr<Component> &target, void *value)
			{
				Color c = *(Color *) value;

				cover->SetColor(c);
			};
	}
};

struct ButtonResumeEventListener : public UIEventListener
{
	virtual void OnClick()
	{
		auto cover = g_win_settings->GetTransform()->GetParent().lock()->Find("window settings cover")->GetGameObject()->GetComponent<SpriteNode>();
		cover->GetGameObject()->SetActive(false);
		g_win_settings->SetActive(false);
	}
};

struct ButtonAboutEventListener : public UIEventListener
{
	virtual void OnClick()
	{
		auto settings_main = g_win_settings->GetTransform()->Find("settings_main");
		settings_main->GetGameObject()->SetActive(false);
		auto settings_about = g_win_settings->GetTransform()->Find("settings_about");
		settings_about->GetGameObject()->SetActive(true);
	}
};

struct ButtonAboutBackEventListener : public UIEventListener
{
	virtual void OnClick()
	{
		auto settings_main = g_win_settings->GetTransform()->Find("settings_main");
		settings_main->GetGameObject()->SetActive(true);
		auto settings_about = g_win_settings->GetTransform()->Find("settings_about");
		settings_about->GetGameObject()->SetActive(false);
	}
};

struct ButtonUpEventListener : public UIEventListener
{
	static void OnTweenFinished(Component *tween, std::weak_ptr<Component> &target)
	{
		g_ui_bag_up = !g_ui_bag_up;

		auto s = g_bag->GetTransform()->Find("up")->GetGameObject()->GetComponent<SpriteNode>()->GetSprite();
		if(g_ui_bag_up)
		{
			s->GetAtlas().lock()->SetSpriteData(s, "up");
			g_bag->GetTransform()->Find("bag")->GetGameObject()->SetActive(false);
		}
		else
		{
			s->GetAtlas().lock()->SetSpriteData(s, "down");
		}
	}

	virtual void OnClick()
	{
		auto tp = g_bag->GetComponent<TweenPosition>();
		if(tp)
		{
			return;
		}

		g_bag->GetTransform()->Find("bag")->GetGameObject()->SetActive(true);

		tp = g_bag->AddComponent<TweenPosition>();
		tp->duration = 0.2f;
		tp->curve = AnimationCurve();
		tp->curve.keys.push_back(Keyframe(0, 0, 0, 0));
		tp->curve.keys.push_back(Keyframe(1, 1, 0, 0));
		tp->target = GetComponentPtr();
		tp->on_finished = ButtonUpEventListener::OnTweenFinished;

		auto tpg = g_batch_game->GetGameObject()->AddComponent<TweenPosition>();
		tpg->duration = 0.2f;
		tpg->curve = AnimationCurve();
		tpg->curve.keys.push_back(Keyframe(0, 0, 0, 0));
		tpg->curve.keys.push_back(Keyframe(1, 1, 0, 0));

		auto tpt = g_bag->GetTransform()->Find("tabs")->GetGameObject()->AddComponent<TweenPosition>();
		tpt->duration = 0.2f;
		tpt->curve = AnimationCurve();
		tpt->curve.keys.push_back(Keyframe(0, 0, 0, 0));
		tpt->curve.keys.push_back(Keyframe(1, 1, 0, 0));

		if(g_ui_bag_up)
		{
			tp->from = Vector3(0, 88, 0);
			tp->to = Vector3(0, 425, 0);

			tpg->from = tpg->GetTransform()->GetLocalPosition();
			tpg->to = tpg->from + Vector3(0, 290, 0);

			tpt->from = tpt->GetTransform()->GetLocalPosition();
			tpt->to = tpt->from + Vector3(0, 75, 0);
		}
		else
		{
			tp->from = Vector3(0, 425, 0);
			tp->to = Vector3(0, 88, 0);

			tpg->from = tpg->GetTransform()->GetLocalPosition();
			tpg->to = tpg->from - Vector3(0, 290, 0);

			tpt->from = tpt->GetTransform()->GetLocalPosition();
			tpt->to = tpt->from - Vector3(0, 75, 0);
		}

		auto sound = GetGameObject()->GetComponent<AudioSource>();
		sound->Play();
	}
};

struct TabEventListener : public UIEventListener
{
	int index;
	int selected;
	std::vector<SpriteNode *> tab_group;

	virtual void OnClick()
	{
		if(selected != index)
		{
			auto tab_name = std::string("tab_") + GTString::ToString(selected).str;
			auto sprite = tab_group[selected]->GetSprite();
			sprite->GetAtlas().lock()->SetSpriteData(sprite, tab_name);

			tab_name = std::string("tab_") + GTString::ToString(index).str;
			sprite = tab_group[index]->GetSprite();
			sprite->GetAtlas().lock()->SetSpriteData(sprite, tab_name + "_selected");

			for(int i=0; i<5; i++)
			{
				auto card = tab_group[selected]->GetTransform()->GetParent().lock()->Find("card_" + GTString::ToString(selected * 5 + i).str);
				if(card)
				{
					card->GetGameObject()->SetActive(false);
				}

				card = tab_group[index]->GetTransform()->GetParent().lock()->Find("card_" + GTString::ToString(index * 5 + i).str);
				if(card)
				{
					card->GetGameObject()->SetActive(true);
				}
			}

			// set selected tab
			for(int i=0; i<5; i++)
			{
				auto handler = tab_group[i]->GetGameObject()->GetComponent<TabEventListener>();
				handler->selected = index;
			}
		}
	}
};

struct CardEventListener : public UIEventListener
{
	int type_0;
	int type_1;

	virtual void OnDrag(const Vector3 &delta)
	{
		Vector3 pos = UICanvas::GetLastRayHit().point;
		auto pos_local = GetTransform()->InverseTransformPoint(pos);

		auto tree = GetTransform()->Find("tree")->GetGameObject();
		if(!tree->IsActiveSelf())
		{
			tree->SetActive(true);
		}

		if(pos_local.y > 253.0f)
		{
			pos_local.y = 253.0f;
			
			// 去掉tabs card的偏移
			float x = pos_local.x + (200 + 270.0f * type_1) - 740;

			int pos_x = map_pos_to_plant(type_0, x);
			pos_local.x = plant_pos_to_map(type_0, pos_x);

			// 加上tabs card的偏移
			pos_local.x = pos_local.x - (200 + 270.0f * type_1) + 740;
		}
		
		pos_local.y = Mathf::Max(pos_local.y, -162.0f);

		tree->GetTransform()->SetLocalPosition(pos_local);
		tree->GetComponent<SpriteNode>()->SetColor(Color(1, 1, 1, 0.7f));
	}

	virtual void OnDragEnd()
	{
		Vector3 pos = UICanvas::GetLastRayHit().point;
		auto pos_local = GetTransform()->InverseTransformPoint(pos);

		auto tree = GetTransform()->Find("tree")->GetGameObject();
		tree->SetActive(false);
		tree->GetComponent<SpriteNode>()->SetColor(Color(1, 1, 1, 1));

		if( pos_local.y > 253.0f
			&& type_0 < 3)
		{
			// 去掉tabs card的偏移
			float x = pos_local.x + (200 + 270.0f * type_1) - 740;
			if( x > g_map_limit_left->GetTransform()->GetLocalPosition().x &&
				x < g_map_limit_right->GetTransform()->GetLocalPosition().x)
			{
				auto &plants = g_plants[type_0];
				int pos_x = map_pos_to_plant(type_0, x);

				size_t target_size = Mathf::Abs(pos_x) * 2 + 1;
				if(plants.size() >= target_size)
				{
					auto &card = g_cards[type_0 * 5 + type_1];

					int index = pos_x + (plants.size() - 1) / 2;
					if( !plants[index] 
						//&& card.price_base <= g_gold
						)
					{
						auto obj = GameObject::Instantiate(tree);
						std::shared_ptr<Plant> p;
					
						if(type_0 == 0)
						{
							if(type_1 == 0 || type_1 == 1 || type_1 == 3)
							{
								p = obj->AddComponent<PlantSmall4>();
							}
							else
							{
								p = obj->AddComponent<PlantSmall2>();
							}
						}
						else if(type_0 == 1)
						{
							p = obj->AddComponent<PlantMiddle>();
						}
						else if(type_0 == 2)
						{
							p = obj->AddComponent<PlantBig>();
						}

						p->type_0 = type_0;
						p->type_1 = type_1;
						p->pos = pos_x;
						p->tree = obj->GetComponent<SpriteNode>();
						p->fruit = GetTransform()->Find("fruit")->GetGameObject()->GetComponent<SpriteNode>();
						
						plants[index] = p;

						obj->SetActive(true);
						p->GetTransform()->SetParent(g_batch_game->GetTransform());
						auto s = obj->GetComponent<SpriteNode>();
						if(type_0 == 0)
						{
							s->SetSortingOrder(8);
						}
						else
						{
							s->SetSortingOrder(7 - type_0 * 2);
						}
						g_batch_game->AddSprite(s);

						set_plants_pos();

						// add planted
						card.planted++;
						card.label_planted.lock()->GetLabel()->SetText("<outline>" + GTString::ToString(card.planted).str + "</outline>");
					
						// spend gold
						set_gold(g_gold - card.price_base);
					}
				}
			}
		}
	}
};
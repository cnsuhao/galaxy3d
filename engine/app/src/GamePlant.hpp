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
#include "BoxCollider.h"
#include "UIEventListener.h"

using namespace Galaxy3D;

struct Item
{
	int type_0;
	int type_1;
	std::string name;
	int price_base;
	int level;
	int planted;

	Item():
		type_0(-1),
		type_1(-1),
		name(""),
		price_base(-1),
		level(-1),
		planted(-1)
	{}
	Item(int type_0, int type_1, const std::string &name, int price_base, int level, int planted):
		type_0(type_0),
		type_1(type_1),
		name(name),
		price_base(price_base),
		level(level),
		planted(planted)
	{}
};

static GameObject *g_grass_node_0;
static GameObject *g_grass_node_0_copy;
static GameObject *g_grass_node_1;
static GameObject *g_grass_node_1_copy;
static GameObject *g_ground_node;
static GameObject *g_ground_node_copy;
static GameObject *g_wave_node_0;
static GameObject *g_wave_node_1;
static GameObject *g_bag;
static SpriteBatchRenderer *g_batch_game;
static float g_grass_pos_0_init;
static float g_grass_pos_1_init;
static float g_ground_pos_init;
static float g_wave_pos_0_init;
static float g_wave_pos_1_init;
static float g_map_pos = 0;
static float g_grass_pos_0;
static float g_grass_pos_1;
static float g_ground_pos = 0;
static float g_wave_pos_0 = 0;
static float g_wave_pos_1 = 0;
static bool g_ui_bag_up = true;
static Item g_cards[25] = {
	Item(0, 0, "carrot", 1, 0, 0)
};
static int g_tab_current = 0;

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

static void set_map_pos(float x)
{
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
}

struct GroundEventListener : public UIEventListener
{
	float m_down_x;
	float m_down_map_pos;

	virtual void OnDragStart()
	{
		m_down_x = UICanvas::GetLastPosition().x;
		m_down_map_pos = g_map_pos;
	}

	virtual void OnDrag(const Vector3 &delta)
	{
		float offset = m_down_x - UICanvas::GetLastPosition().x;

		set_map_pos(m_down_map_pos + offset);
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
	}
};

struct TabEventListener : public UIEventListener
{
	virtual void OnClick()
	{
		
	}
};

struct CardEventListener : public UIEventListener
{
	virtual void OnClick()
	{
		
	}
};
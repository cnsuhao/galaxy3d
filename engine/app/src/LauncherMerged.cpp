#include "LauncherMerged.h"
#include "GameObject.h"
#include "Camera.h"
#include "Application.h"
#include "Sprite.h"
#include "SpriteBatchRenderer.h"
#include "Layer.h"
#include "LayerMask.h"
#include "UICanvas.h"
#include "Screen.h"
#include "Label.h"
#include "TextRenderer.h"
#include "GTString.h"
#include "GTUIManager.h"
#include "BoxCollider.h"
#include "UIEventListener.h"
#include "TweenRotation.h"

struct Tile : public Component
{
	int point;
	SpriteNode *node;
};

static TextRenderer *g_fps;
static SpriteBatchRenderer *g_root_batch;
static GameObject *g_rotate;
static GameObject *g_rotate_tiles;
static SpriteBatchRenderer *g_rotate_batch;
static GameObject *g_rotate_collider;
static float g_rotate_degree;
static std::weak_ptr<Sprite> g_tile;
static const float g_scale = 0.75f;
static int g_score = 0;
static int g_score_best = 0;
static int g_coin = 0;
static int g_coin_destroy_price = 0;
static Tile *g_grids[25];

struct RotateTileEventListener : UIEventListener
{
	std::weak_ptr<TweenRotation> tween;
	Vector3 drag_start_pos_input;
	Vector3 drag_start_pos_tiles;
	Vector3 drag_offset;
	bool drag_out;

	virtual void OnClick()
	{
		if(g_rotate_tiles->GetTransform()->GetChildCount() == 2 && !drag_out)
		{
			if(tween.expired())
			{
				auto t = g_rotate_tiles->AddComponent<TweenRotation>();
				
				t->duration = 0.3f;
				t->loop = false;
				t->curve = AnimationCurve();
				t->curve.keys.push_back(Keyframe(0, 0, 0, 0));
				t->curve.keys.push_back(Keyframe(1, 1, 0, 0));
				t->from = Vector3(0, 0, g_rotate_degree);
				t->to = Vector3(0, 0, g_rotate_degree - 90);
				tween = t;

				for(int i=0; i<2; i++)
				{
					auto tile = g_rotate_tiles->GetTransform()->Find(GTString::ToString(i).str);
					t = tile->GetGameObject()->GetComponent<TweenRotation>();
					if(!t)
					{
						Component::Destroy(std::dynamic_pointer_cast<Component>(t));
					}
					t = tile->GetGameObject()->AddComponent<TweenRotation>();

					t->duration = 0.3f;
					t->loop = false;
					t->curve = AnimationCurve();
					t->curve.keys.push_back(Keyframe(0, 0, 0, 0));
					t->curve.keys.push_back(Keyframe(1, 1, 0, 0));
					t->from = Vector3(0, 0, -g_rotate_degree);
					t->to = Vector3(0, 0, -g_rotate_degree + 90);
				}

				g_rotate_degree -= 90;
			}
		}

		drag_out = false;
	}

	virtual void OnDragStart()
	{
		drag_start_pos_input = UICanvas::GetLastPosition();
		drag_start_pos_tiles = g_rotate_tiles->GetTransform()->GetLocalPosition();
		drag_out = false;
		drag_offset = Vector3(0, 0, 0);
	}

	virtual void OnDrag(const Vector3 &delta)
	{
		Vector3 offset = UICanvas::GetLastPosition() - drag_start_pos_input;
		if(offset.y > 70)
		{
			if(Mathf::FloatEqual(drag_offset.y, 0))
			{
				drag_offset.y = 200;
			}

			if(g_rotate->IsActiveSelf() && g_rotate_tiles->GetTransform()->GetChildCount() == 2)
			{
				g_rotate->SetActive(false);
			}
		}
		
		auto pos = drag_start_pos_tiles + offset * (1920.0f / Screen::GetHeight());
		g_rotate_tiles->GetTransform()->SetLocalPosition(pos + drag_offset);
	}

	virtual void OnDragOut(std::weak_ptr<GameObject> &dragged)
	{
		if(UICanvas::IsPressStarted())
		{
			drag_out = true;
		}
	}

	virtual void OnDragEnd()
	{
		auto pos_world = g_rotate_tiles->GetTransform()->GetPosition();
		auto pos_in_grids = g_root_batch->GetTransform()->InverseTransformPoint(pos_world);
		bool join = false;

		float left_bottom = -2 * 175;
		float x = (pos_in_grids.x - left_bottom) / 175;
		float y = (pos_in_grids.y - left_bottom) / 175;
		float x_mod = x - floor(x);
		float y_mod = y - floor(y);

		if(g_rotate_tiles->GetTransform()->GetChildCount() == 1)
		{
			if( (x_mod < 0.3f || x_mod > 0.7f) &&
				(y_mod < 0.3f || y_mod > 0.7f))
			{
				int x_index = Mathf::RoundToInt(x);
				int y_index = Mathf::RoundToInt(y);

				if(	x_index >= 0 && x_index < 5 &&
					y_index >= 0 && y_index < 5)
				{
					join = true;
				}
			}
		}
		else
		{
			int degree = ((int) g_rotate_degree) % 360;
			if(degree < 0)
			{
				degree += 360;
			}
			bool vertical = false;
			if(degree == 90 || degree == 270)
			{
				vertical = true;
			}

			if(vertical)
			{
				if( (x_mod < 0.3f || x_mod > 0.7f) &&
					(y_mod - 0.5f < 0.3f && y_mod - 0.5f > -0.3f))
				{
					int x_index = Mathf::RoundToInt(x);
					int y_index_0 = Mathf::RoundToInt(y - 0.5f);
					int y_index_1 = y_index_0 + 1;
					if(	x_index >= 0 && x_index < 5 &&
						y_index_0 >= 0 && y_index_0 < 4)
					{
						join = true;
					}
				}
			}
			else
			{
				if( (y_mod < 0.3f || y_mod > 0.7f) &&
					(x_mod - 0.5f < 0.3f && x_mod - 0.5f > -0.3f))
				{
					int x_index_0 = Mathf::RoundToInt(x - 0.5f);
					int x_index_1 = x_index_0 + 1;
					int y_index = Mathf::RoundToInt(y);
					if( x_index_0 >= 0 && x_index_0 < 4 &&
						y_index >= 0 && y_index < 5)
					{
						join = true;
					}
				}
			}
		}

		if(!join)
		{
			g_rotate_tiles->GetTransform()->SetLocalPosition(drag_start_pos_tiles);

			auto hit = UICanvas::GetRayHitObject();
			if(hit.expired() || hit.lock().get() != g_rotate_collider)
			{
				drag_out = false;
			}

			if(g_rotate_tiles->GetTransform()->GetChildCount() == 2)
			{
				g_rotate->SetActive(true);
			}
		}
	}
};

std::shared_ptr<SpriteNode> create_tile(int point)
{
	auto node = GameObject::Create("")->AddComponent<SpriteNode>();
	node->SetSprite(g_tile.lock());
	g_rotate_batch->AddSprite(node);
	auto tile_node = node;

	auto sprite_point = g_tile.lock()->GetAtlas().lock()->CreateSprite(
		"point",
		Vector2(0.5f, 0.5f),
		100,
		Sprite::Type::Simple,
		Vector2(0, 0));
	auto sprite_m = g_tile.lock()->GetAtlas().lock()->CreateSprite(
		"m",
		Vector2(0.5f, 0.5f),
		100,
		Sprite::Type::Simple,
		Vector2(0, 0));

	int point_dis = 40;
	switch(point)
	{
	case 7:
		{
			node->SetColor(Color(255, 255, 255, 255) / 255.0f);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_m);
			g_rotate_batch->AddSprite(node);
		}
		break;
	case 1:
		{
			node->SetColor(Color(126, 77, 164, 255) / 255.0f);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);
		}
		break;
	case 2:
		{
			node->SetColor(Color(243, 112, 34, 255) / 255.0f);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(-point_dis / g_scale), Mathf::Round(point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(point_dis / g_scale), Mathf::Round(-point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);
		}
		break;
	case 3:
		{
			node->SetColor(Color(255, 49, 88, 255) / 255.0f);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(-point_dis / g_scale), Mathf::Round(point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(point_dis / g_scale), Mathf::Round(-point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);
		}
		break;
	case 4:
		{
			node->SetColor(Color(60, 138, 218, 255) / 255.0f);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(-point_dis / g_scale), Mathf::Round(point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(-point_dis / g_scale), Mathf::Round(-point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(point_dis / g_scale), Mathf::Round(-point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(point_dis / g_scale), Mathf::Round(point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);
		}
		break;
	case 5:
		{
			node->SetColor(Color(130, 188, 51, 255) / 255.0f);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(-point_dis / g_scale), Mathf::Round(point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(-point_dis / g_scale), Mathf::Round(-point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(point_dis / g_scale), Mathf::Round(-point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(point_dis / g_scale), Mathf::Round(point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);
		}
		break;
	case 6:
		{
			node->SetColor(Color(255, 201, 37, 255) / 255.0f);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(-point_dis / g_scale), Mathf::Round(point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(-point_dis / g_scale), 0, 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(-point_dis / g_scale), Mathf::Round(-point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(point_dis / g_scale), Mathf::Round(-point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(point_dis / g_scale), 0, 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(point_dis / g_scale), Mathf::Round(point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);
		}
		break;
	}

	auto tile = tile_node->GetGameObject()->AddComponent<Tile>();
	tile->point = point;
	tile->node = tile_node.get();

	return tile_node;
}

void new_tile(int max_point)
{
	int count = Mathf::RandomRange(0, 100) >= 50 ? 2 : 1;
	if(count == 1)
	{
		g_rotate->SetActive(false);
	}
	else
	{
		g_rotate->SetActive(true);
	}

	g_rotate_degree = 0;

	auto rotate_tiles = GameObject::Create("rotate_tiles");
	rotate_tiles->GetTransform()->SetParent(g_rotate_batch->GetTransform());
	rotate_tiles->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
	rotate_tiles->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
	g_rotate_tiles = rotate_tiles.get();
	
	int point_prev;
	for(int i=0; i<count; i++)
	{
		int point = 0;

		do
		{
			if(max_point < 7)
			{
				point = Mathf::RandomRange(1, max_point + 1);
			}
			else
			{
				point = (int) Mathf::RandomRange(1.0f, 7.3f);//0.3 for m, 1.0 for 1 ~ 6
			}

			if(i == 0)
			{
				point_prev = point;
				break;
			}
			else if(i == 1)
			{
				if(point != point_prev)
				{
					break;
				}
			}
		}while(true);

		auto node = create_tile(point);
		node->SetName(GTString::ToString(i).str);
		node->GetTransform()->SetParent(g_rotate_tiles->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(175.0f * i - (175.0f * (count - 1) * 0.5f), 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1) * g_scale);
	}
}

std::shared_ptr<Label> create_label(GameObject *parent, Vector3 &pos, int font_size, LabelPivot::Enum pivot)
{
	auto label = Label::Create("", "heiti", font_size, pivot, LabelAlign::Auto, false);
	auto tr = GameObject::Create("")->AddComponent<TextRenderer>();
	tr->SetLabel(label);
	tr->SetSortingOrder(1, 0);
	tr->GetTransform()->SetParent(parent->GetTransform());
	tr->GetTransform()->SetLocalScale(Vector3(1, 1, 1) * (1.0f / g_scale));
	tr->GetTransform()->SetLocalPosition(pos);

	return label;
}

void LauncherMerged::Start()
{
	GTUIManager::LoadFont("heiti", Application::GetDataPath() + "/Assets/font/heiti.ttc");

	float pixel_per_unit = 100;

    auto cam = GameObject::Create("")->AddComponent<Camera>();
    cam->SetOrthographic(true);
	cam->SetOrthographicSize(1 / pixel_per_unit * Screen::GetHeight() / 2);
	cam->SetClipPlane(-1, 1);
	cam->SetCullingMask(LayerMask::GetMask(Layer::UI));
	cam->SetDepth(1);
	cam->SetClearColor(Color(42, 42, 42, 255) / 255.0f);

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

	auto destroy_price = create_label(node->GetGameObject().get(), Vector3(Mathf::Round(135 / g_scale), Mathf::Round(34 / g_scale), 0), 40, LabelPivot::Center);
	destroy_price->SetText(GTString::ToString(g_coin_destroy_price).str);

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

	auto score_best = create_label(node->GetGameObject().get(), Vector3(Mathf::Round(90 / g_scale), Mathf::Round(-30 / g_scale), 0), 40, LabelPivot::Left);
	score_best->SetText(GTString::ToString(g_score_best).str);

	auto coin = create_label(node->GetGameObject().get(), Vector3(Mathf::Round(90 / g_scale), Mathf::Round(-98 / g_scale), 0), 40, LabelPivot::Left);
	coin->SetText(GTString::ToString(g_coin).str);

	sprite = atlas->CreateSprite(
		"pause",
		Vector2(1, 0),
		100,
		Sprite::Type::Simple,
		Vector2(0, 0));
	node = GameObject::Create("")->AddComponent<SpriteNode>();
	node->GetTransform()->SetParent(batch->GetTransform());
	node->GetTransform()->SetLocalScale(Vector3(1, 1, 1) * g_scale);
	node->SetSprite(sprite);
	node->SetAnchor(Vector4(1, 0, -30, -100));
	batch->AddSprite(node);
	
	auto score = create_label(node->GetGameObject().get(), Vector3(Mathf::Round(-180 / g_scale), Mathf::Round(-75 / g_scale), 0), 80, LabelPivot::Right);
	score->SetText(GTString::ToString(g_score).str);

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
	auto collider = node->GetGameObject()->AddComponent<BoxCollider>();
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

	cam->GetGameObject()->SetLayerRecursively(Layer::UI);
	cam->GetTransform()->SetParent(GetTransform());

	memset(g_grids, 0, sizeof(g_grids));
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
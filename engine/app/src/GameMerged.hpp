#ifndef __GameMerged_h__
#define __GameMerged_h__

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
#include "TweenScale.h"
#include "TweenPosition.h"
#include "TweenColor.h"
#include "Localization.h"
#include "Input.h"
#include "Debug.h"

struct Tile : public Component
{
	int point;
	std::vector<std::weak_ptr<SpriteNode>> nodes;
};

static std::shared_ptr<Tile> create_tile(int point);
static void new_tile(int max_point);
static void join_tile(int index, Tile *t);
static void merge();
static void on_lose();

static TextRenderer *g_fps;
static SpriteBatchRenderer *g_root_batch;
static GameObject *g_rotate;
static GameObject *g_rotate_tiles;
static SpriteBatchRenderer *g_rotate_batch;
static GameObject *g_rotate_collider;
static GameObject *g_window_pause;
static float g_rotate_degree;
static std::weak_ptr<Sprite> g_tile;
static const float g_scale = 0.75f;
static int g_score = 0;
static Label *g_score_label;
static int g_score_best = 0;
static Label *g_score_best_label;
static int g_coin = 0;
static int g_coin_destroy_price = 0;
static Tile *g_tiles[25];
static std::list<int> g_merge_check;

static void restart()
{
	for(int i=0; i<25; i++)
	{
		if(g_tiles[i] != NULL)
		{
			for(auto j : g_tiles[i]->nodes)
			{
				g_root_batch->RemoveSprite(j.lock());
			}

			GameObject::Destroy(g_tiles[i]->nodes[0].lock()->GetGameObject());
			g_tiles[i] = NULL;
		}
	}

	int count = g_rotate_tiles->GetTransform()->GetChildCount();
	for(int i=0; i<count; i++)
	{
		auto t = g_rotate_tiles->GetTransform()->GetChild(i)->GetGameObject()->GetComponent<Tile>();
		for(auto j : t->nodes)
		{
			g_rotate_batch->RemoveSprite(j.lock());
		}

		GameObject::Destroy(g_rotate_tiles->GetTransform()->GetGameObject());
	}
	g_rotate_tiles = NULL;
	g_score = 0;
	g_score_label->SetText(GTString::ToString(g_score).str);
	g_merge_check.clear();

	new_tile(2);
}

struct MergeOutTweenScale : public TweenScale
{
	static void OnFinished(Component *tween, std::weak_ptr<Component> &target)
	{
		auto thiz = (MergeOutTweenScale *) tween;
		auto tile = tween->GetGameObject()->GetComponent<Tile>();

		for(auto &i : tile->nodes)
		{
			g_root_batch->RemoveSprite(i.lock());
		}
		GameObject::Destroy(tween->GetGameObject());
	}
};

struct MergeInTweenScale : public TweenScale
{
	static void OnFinished(Component *tween, std::weak_ptr<Component> &target)
	{
		if(!g_merge_check.empty())
		{
			merge();
		}
	}
};

static bool exist_empty_2()
{
	for(int i=0; i<25; i++)
	{
		if(g_tiles[i] == NULL)
		{
			if( (i % 5 > 0 && i - 1 >= 0 && i - 1 < 25 && g_tiles[i - 1] == NULL) ||
				(i + 5 >= 0 && i + 5 < 25 && g_tiles[i + 5] == NULL) ||
				(i % 5 < 4 && i + 1 >= 0 && i + 1 < 25 && g_tiles[i + 1] == NULL) ||
				(i - 5 >= 0 && i - 5 < 25 && g_tiles[i - 5] == NULL))
			{
				return true;
			}
		}
	}

	return false;
}

static int find_max_point()
{
	int max = 0;

	for(auto i : g_tiles)
	{
		if(i != NULL)
		{
			max = Mathf::Max(max, i->point);
		}
	}

	max = Mathf::Max(max, 2);

	return max;
}

static void check_tile(int current, int next, std::vector<int> &open, std::vector<int> &close)
{
	auto t = g_tiles[next];

	if(t != NULL && t->point == g_tiles[current]->point)
	{
		auto find_close = std::find(close.begin(), close.end(), next);
		auto find_open = std::find(open.begin(), open.end(), next);
		if(find_close == close.end() && find_open == open.end())
		{
			open.push_back(next);
		}
	}
}

static void merge_search(std::vector<int> &open, std::vector<int> &close)
{
	int current = open.back();
	open.pop_back();
	close.push_back(current);

	auto find = std::find(g_merge_check.begin(), g_merge_check.end(), current);
	if(find != g_merge_check.end())
	{
		g_merge_check.erase(find);
	}

	int x = current % 5;
	int y = current / 5;
	
	if(x - 1 >= 0)
	{
		check_tile(current, x - 1 + y * 5, open, close);
	}

	if(x + 1 < 5)
	{
		check_tile(current, x + 1 + y * 5, open, close);
	}

	if(y - 1 >= 0)
	{
		check_tile(current, x + (y - 1) * 5, open, close);
	}

	if(y + 1 < 5)
	{
		check_tile(current, x + (y + 1) * 5, open, close);
	}

	if(!open.empty())
	{
		merge_search(open, close);
	}
}

static bool check_sort(int a, int b)
{
	return g_tiles[a]->point < g_tiles[b]->point;
}

static void merge()
{
	if(g_merge_check.size() > 1)
	{
		g_merge_check.sort(check_sort);
	}

	bool have_merge = false;
	while(!g_merge_check.empty())
	{
		std::vector<int> open;
		std::vector<int> close;
		open.push_back(g_merge_check.front());
		g_merge_check.pop_front();
		merge_search(open, close);

		if(close.size() >= 3)
		{
			have_merge = true;

			if(g_tiles[close[0]]->point == 7)
			{
				int index = close[0];

				int rounds[8];
				int i = 0;
				
				if(index % 5 > 0)
				{
					rounds[i++] = index - 1;
					rounds[i++] = index + 4;
					rounds[i++] = index - 6;
				}

				if(index % 5 < 4)
				{
					rounds[i++] = index + 1;
					rounds[i++] = index + 6;
					rounds[i++] = index - 4;
				}
				rounds[i++] = index + 5;
				rounds[i++] = index - 5;

				for(auto j : rounds)
				{
					if(j >= 0 && j < 25 && g_tiles[j] != NULL)
					{
						auto find = std::find(close.begin(), close.end(), j);
						if(find == close.end())
						{
							close.push_back(j);
						}
					}
				}

				have_merge = false;
			}

			for(size_t i=0; i<close.size(); i++)
			{
				int index = close[i];
				int point = g_tiles[index]->point;
				auto t_out = g_tiles[index]->GetGameObject()->AddComponent<MergeOutTweenScale>();
				t_out->duration = 0.2f;
				t_out->curve = AnimationCurve();
				if(i == 0 && point == 7)
				{
					t_out->duration = 0.4f;
					t_out->curve.keys.push_back(Keyframe(0, 0, -4.0f, -4.0f));
					t_out->curve.keys.push_back(Keyframe(0.5f, -2.0f, 6.0f, 6.0f));
					t_out->curve.keys.push_back(Keyframe(1, 1, 6.0f, 6.0f));
				}
				else
				{
					t_out->curve.keys.push_back(Keyframe(0, 0, 0, 0));
					t_out->curve.keys.push_back(Keyframe(1, 1, 0, 0));
				}
				t_out->from = t_out->GetTransform()->GetLocalScale();
				t_out->to = t_out->from * 0.1f;
				t_out->target = t_out;
				t_out->on_finished = MergeOutTweenScale::OnFinished;
				g_score += point;
				g_score_label->SetText(GTString::ToString(g_score).str);
				if(g_score > g_score_best)
				{
					g_score_best = g_score;
					g_score_best_label->SetText(GTString::ToString(g_score_best).str);
				}

				g_tiles[index] = NULL;

				if(i == 0 && point < 7)
				{
					auto tile_next = create_tile(point + 1);

					join_tile(index, tile_next.get());
					tile_next->GetTransform()->SetLocalScale(Vector3(1, 1, 1) * g_scale * 0.1f);
					
					auto t_in = g_tiles[index]->GetGameObject()->AddComponent<MergeInTweenScale>();
					t_in->duration = 0.2f;
					t_in->curve = AnimationCurve();
					t_in->curve.keys.push_back(Keyframe(0, 0, 0, 0));
					t_in->curve.keys.push_back(Keyframe(1, 1, 0, 0));
					t_in->from = t_in->GetTransform()->GetLocalScale();
					t_in->to = Vector3(1, 1, 1) * g_scale;
					t_in->target = t_in;
					t_in->on_finished = MergeInTweenScale::OnFinished;

					g_merge_check.push_back(index);
				}
			}

			break;
		}
	}

	if(!have_merge)
	{
		bool lose = true;

		for(int i=0; i<25; i++)
		{
			if(g_tiles[i] == NULL)
			{
				lose = false;
				break;
			}
		}

		if(!lose)
		{
			int max = find_max_point();
			new_tile(max);
		}
		else
		{
			on_lose();
		}
	}
}

static void join_tile(int index, Tile *t)
{
	g_tiles[index] = t;

	for(size_t i=0; i<t->nodes.size(); i++)
	{
		auto node = t->nodes[i].lock();
		auto batch = node->GetBatch().lock();
		if(batch)
		{
			batch->RemoveSprite(node);
		}

		node->SetSortingOrder(1);
		g_root_batch->AddSprite(node);

		if(i == 0)
		{
			int x = index % 5;
			int y = index / 5;
			node->GetTransform()->SetParent(g_root_batch->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(175.0f * (x - 2), 175.0f * (y - 2), 0));
			node->GetTransform()->SetLocalRotation(Quaternion::Identity());
		}
	}
}

static bool join_tile_1(int index, Tile *t)
{
	if(g_tiles[index] == NULL)
	{
		join_tile(index, t);

		g_merge_check.clear();
		g_merge_check.push_back(index);
		merge();

		return true;
	}

	return false;
}

static bool join_tile_2(int index_0, Tile *t0, int index_1, Tile *t1)
{
	if(g_tiles[index_0] == NULL && g_tiles[index_1] == NULL)
	{
		join_tile(index_0, t0);
		join_tile(index_1, t1);

		g_merge_check.clear();
		g_merge_check.push_back(index_0);
		g_merge_check.push_back(index_1);
		merge();

		return true;
	}

	return false;
}

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
				
				t->duration = 0.2f;
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

					t->duration = 0.2f;
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
			if( (x_mod < 0.45f || x_mod > 0.05f) &&
				(y_mod < 0.45f || y_mod > 0.05f))
			{
				int x_index = Mathf::RoundToInt(x);
				int y_index = Mathf::RoundToInt(y);

				if(	x_index >= 0 && x_index < 5 &&
					y_index >= 0 && y_index < 5)
				{
					Tile *t = g_rotate_tiles->GetTransform()->Find("0")->GetGameObject()->GetComponent<Tile>().get();
					join = join_tile_1(y_index * 5 + x_index, t);
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
				if( (x_mod < 0.45f || x_mod > 0.05f) &&
					(y_mod - 0.5f < 0.45f && y_mod - 0.5f > -0.45f))
				{
					int x_index = Mathf::RoundToInt(x);
					int y_index_0 = Mathf::RoundToInt(y - 0.5f);
					int y_index_1 = y_index_0 + 1;
					if(	x_index >= 0 && x_index < 5 &&
						y_index_0 >= 0 && y_index_0 < 4)
					{
						Tile *t0 = g_rotate_tiles->GetTransform()->Find("0")->GetGameObject()->GetComponent<Tile>().get();
						Tile *t1 = g_rotate_tiles->GetTransform()->Find("1")->GetGameObject()->GetComponent<Tile>().get();

						if(degree == 90)
						{
							join = join_tile_2(y_index_0 * 5 + x_index, t0, y_index_1 * 5 + x_index, t1);
						}
						else
						{
							join = join_tile_2(y_index_0 * 5 + x_index, t1, y_index_1 * 5 + x_index, t0);
						}
					}
				}
			}
			else
			{
				if( (y_mod < 0.45f || y_mod > 0.05f) &&
					(x_mod - 0.5f < 0.45f && x_mod - 0.5f > -0.45f))
				{
					int x_index_0 = Mathf::RoundToInt(x - 0.5f);
					int x_index_1 = x_index_0 + 1;
					int y_index = Mathf::RoundToInt(y);
					if( x_index_0 >= 0 && x_index_0 < 4 &&
						y_index >= 0 && y_index < 5)
					{
						Tile *t0 = g_rotate_tiles->GetTransform()->Find("0")->GetGameObject()->GetComponent<Tile>().get();
						Tile *t1 = g_rotate_tiles->GetTransform()->Find("1")->GetGameObject()->GetComponent<Tile>().get();

						if(degree == 0)
						{
							join = join_tile_2(y_index * 5 + x_index_0, t0, y_index * 5 + x_index_1, t1);
						}
						else
						{
							join = join_tile_2(y_index * 5 + x_index_0, t1, y_index * 5 + x_index_1, t0);
						}
					}
				}
			}
		}

		if(!join)
		{
			auto t = g_rotate_tiles->AddComponent<TweenPosition>();
			t->duration = 0.2f;
			t->curve = AnimationCurve();
			t->curve.keys.push_back(Keyframe(0, 0, 0, 0));
			t->curve.keys.push_back(Keyframe(1, 1, 0, 0));
			t->from = g_rotate_tiles->GetTransform()->GetLocalPosition();
			t->to = drag_start_pos_tiles;

			if(g_rotate_tiles->GetTransform()->GetChildCount() == 2)
			{
				g_rotate->SetActive(true);
			}
		}

		auto hit = UICanvas::GetRayHitObject();
		if(hit.expired() || hit.lock().get() != g_rotate_collider)
		{
			drag_out = false;
		}
	}
};

static std::shared_ptr<Tile> create_tile(int point)
{
	auto node = GameObject::Create("")->AddComponent<SpriteNode>();
	node->SetSprite(g_tile.lock());
	g_rotate_batch->AddSprite(node);
	auto tile_node = node;
	auto tile = tile_node->GetGameObject()->AddComponent<Tile>();
	tile->point = point;
	tile->nodes.push_back(tile_node);

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
			tile->nodes.push_back(node);
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
			tile->nodes.push_back(node);
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
			tile->nodes.push_back(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(point_dis / g_scale), Mathf::Round(-point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);
			tile->nodes.push_back(node);
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
			tile->nodes.push_back(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);
			tile->nodes.push_back(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(point_dis / g_scale), Mathf::Round(-point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);
			tile->nodes.push_back(node);
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
			tile->nodes.push_back(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(-point_dis / g_scale), Mathf::Round(-point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);
			tile->nodes.push_back(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(point_dis / g_scale), Mathf::Round(-point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);
			tile->nodes.push_back(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(point_dis / g_scale), Mathf::Round(point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);
			tile->nodes.push_back(node);
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
			tile->nodes.push_back(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(-point_dis / g_scale), Mathf::Round(-point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);
			tile->nodes.push_back(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(point_dis / g_scale), Mathf::Round(-point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);
			tile->nodes.push_back(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(point_dis / g_scale), Mathf::Round(point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);
			tile->nodes.push_back(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);
			tile->nodes.push_back(node);
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
			tile->nodes.push_back(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(-point_dis / g_scale), 0, 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);
			tile->nodes.push_back(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(-point_dis / g_scale), Mathf::Round(-point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);
			tile->nodes.push_back(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(point_dis / g_scale), Mathf::Round(-point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);
			tile->nodes.push_back(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(point_dis / g_scale), 0, 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);
			tile->nodes.push_back(node);

			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tile_node->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(Mathf::Round(point_dis / g_scale), Mathf::Round(point_dis / g_scale), 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite_point);
			g_rotate_batch->AddSprite(node);
			tile->nodes.push_back(node);
		}
		break;
	}

	return tile;
}

static void new_tile(int max_point)
{
	int count = Mathf::RandomRange(0, 100) >= 50 ? 2 : 1;
	if(!exist_empty_2())
	{
		count = 1;
	}
	if(count == 1)
	{
		g_rotate->SetActive(false);
	}
	else
	{
		g_rotate->SetActive(true);
	}

	g_rotate_degree = 0;

	if(g_rotate_tiles == NULL)
	{
		auto rotate_tiles = GameObject::Create("rotate_tiles");
		rotate_tiles->GetTransform()->SetParent(g_rotate_batch->GetTransform());
		rotate_tiles->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		g_rotate_tiles = rotate_tiles.get();
	}
	g_rotate_tiles->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
	g_rotate_tiles->GetTransform()->SetLocalRotation(Quaternion::Identity());
	
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
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1) * 0.1f);

		auto t = node->GetGameObject()->AddComponent<TweenScale>();
		t->duration = 0.2f;
		t->curve = AnimationCurve();
		t->curve.keys.push_back(Keyframe(0, 0, 0, 0));
		t->curve.keys.push_back(Keyframe(1, 1, 0, 0));
		t->from = Vector3(1, 1, 1) * 0.1f;
		t->to = Vector3(1, 1, 1) * g_scale;
	}
}

#endif
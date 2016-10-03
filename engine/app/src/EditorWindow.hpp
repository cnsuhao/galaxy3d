#ifndef __EditorWindow_hpp__
#define __EditorWindow_hpp__

#include "SpriteBatchRenderer.h"
#include "TextRenderer.h"

class EditorWindow : public Component
{
public:
	UICanvas *canvas;
	UIAtlas *atlas;
	Vector2 position;
	Vector2 size;

	void HideOrShow()
	{
		auto tc = GetGameObject()->GetComponent<TweenColor>();
		if(!tc)
		{
			tc = GetGameObject()->AddComponent<TweenColor>();
			tc->duration = 0.2f;
			if(win_batch->GetColor().a > 0)
			{
				tc->from = Color(1, 1, 1, 0.7f);
				tc->to = Color(1, 1, 1, 0);
			}
			else
			{
				tc->from = Color(1, 1, 1, 0);
				tc->to = Color(1, 1, 1, 0.7f);
			}
			tc->curve = AnimationCurve::DefaultLinear();
			tc->target = win_batch->GetComponentPtr();
			tc->on_set_value = [this](Component *tween, std::weak_ptr<Component> &target, void *value)
				{
					auto c = *(Color *) value;
					auto old = this->win_batch->GetColor();
					old.a = c.a;
					this->win_batch->SetColor(old);
					auto labels = this->GetGameObject()->GetComponentsInChildren<TextRenderer>();
					for(auto i : labels)
					{
						old = i->GetColor();
						old.a = c.a / 0.7f;
						i->SetColor(old);
					}
				};
		}
	}

protected:
	SpriteBatchRenderer *win_batch;
	Label *label_title;
	Sprite *sprite_win_bg;

	virtual void Start()
	{
		float pixel_per_unit = 100.0f;

		auto batch = GetGameObject()->AddComponent<SpriteBatchRenderer>();
		batch->GetTransform()->SetParent(canvas->GetTransform());
		batch->GetTransform()->SetLocalPosition(position);
		batch->GetTransform()->SetLocalScale(Vector3::One());
		batch->SetSortingOrder(0, 0);
		batch->SetColor(Color(1, 1, 1, 0));
		win_batch = batch.get();

		auto title_bg = atlas->CreateSprite(
			"title_bg",
			Vector2(0, 1),
			pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(0, 0));
		auto node = GameObject::Create("title")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(1, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3::One());
		node->SetSprite(title_bg);
		node->SetSortingOrder(0);
		win_batch->AddSprite(node);

		auto label = Label::Create("", "simsun", 10, LabelPivot::BottomLeft, LabelAlign::Auto, false);
		auto tr = GameObject::Create("Label")->AddComponent<TextRenderer>();
		tr->SetLabel(label);
		tr->SetSortingOrder(0, 1);
		tr->GetTransform()->SetParent(node->GetTransform());
		tr->GetTransform()->SetLocalPosition(Vector3(15, 2, 0));
		tr->GetTransform()->SetLocalScale(Vector3::One());
		tr->SetColor(Color(180, 180, 180, 0) / 255.0f);
		label_title = label.get();

		auto win_bg = atlas->CreateSprite(
			"win_bg",
			Vector2(0, 0),
			pixel_per_unit,
			Sprite::Type::Sliced,
			size);
		win_bg->SetBorder(Vector4(3, 3, 3, 3));
		node = GameObject::Create("bg")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3::Zero());
		node->GetTransform()->SetLocalScale(Vector3::One());
		node->SetSprite(win_bg);
		node->SetSortingOrder(0);
		win_batch->AddSprite(node);
		sprite_win_bg = win_bg.get();
	}
};

#endif
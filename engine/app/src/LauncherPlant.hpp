#include "GamePlant.hpp"

class LauncherPlant : public Component, public IScreenResizeEventListener
{
protected:
	TextRenderer *m_fps;
	Camera *m_cam;
	UICanvas *m_canvas;
	SpriteBatchRenderer *m_batch_ui;
	std::shared_ptr<Sprite> m_sprite_coin_anim[8];

	virtual void OnScreenResize(int width, int height)
	{
		m_cam->SetOrthographicSize(1 / g_pixel_per_unit * Screen::GetHeight() / 2);
		m_cam->GetTransform()->SetLocalScale(Vector3::One() * (Screen::GetHeight() / 1080.f) * (1.0f / g_pixel_per_unit));
	
		float input_scale = 1080.f / Screen::GetHeight();
		float input_offset_x = (1920 - Screen::GetWidth() * input_scale) * 0.5f;
		float input_offset_y = (1080 - Screen::GetHeight() * input_scale) * 0.5f;
		Input::SetMousePositionScaleOffset(Vector3::One() * input_scale, Vector3(input_offset_x, input_offset_y, 0));
	}

	std::shared_ptr<TextRenderer> CreateLabel(GameObject *parent, const Vector3 &pos, int font_size, LabelPivot::Enum pivot, int order)
	{
		auto label = Label::Create("", "heiti", font_size, pivot, LabelAlign::Auto, true);
		auto tr = GameObject::Create("")->AddComponent<TextRenderer>();
		tr->SetLabel(label);
		tr->SetSortingOrder(order, 0);
		tr->GetTransform()->SetParent(parent->GetTransform());
		tr->GetTransform()->SetLocalPosition(pos);
		tr->GetTransform()->SetLocalScale(Vector3::One());

		return tr;
	}

	void CreateWindowSettings(UICanvas *canvas, UIAtlas *atlas)
	{
		auto batch_win = GameObject::Create("")->AddComponent<SpriteBatchRenderer>();
		batch_win->GetTransform()->SetParent(canvas->GetTransform());
		batch_win->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		batch_win->SetSortingOrder(5, 0);
		
		auto sprite = atlas->CreateSprite(
			"white",
			Vector2(0.5f, 0.5f),
			g_pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(2048, 2048),
			Vector4(1, 1, -1, -1));
		auto node = GameObject::Create("window settings cover")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(batch_win->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(sprite);
		node->SetSortingOrder(0);
		node->SetColor(Color(0, 0, 0, 0.7f));
		batch_win->AddSprite(node);
		auto collider = node->GetGameObject()->AddComponent<BoxCollider>();
		collider->SetSize(sprite->GetSize());
		node->GetGameObject()->SetActive(false);

		sprite = atlas->CreateSprite(
			"window",
			Vector2(0.5f, 0.5f),
			g_pixel_per_unit,
			Sprite::Type::Sliced,
			Vector2(910, 584));
		sprite->SetBorder(Vector4(30, 30, 30, 30));
		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(batch_win->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(sprite);
		node->SetSortingOrder(1);
		batch_win->AddSprite(node);
		g_win_settings = node->GetGameObject().get();
		g_win_settings->SetActive(false);

		std::shared_ptr<Sprite> lans[3];
		sprite = atlas->CreateSprite(
			"lan en",
			Vector2(0.5f, 0.5f),
			g_pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(0, 0));
		lans[0] = sprite;
		sprite = atlas->CreateSprite(
			"lan zh",
			Vector2(0.5f, 0.5f),
			g_pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(0, 0));
		lans[1] = sprite;
		lans[2] = sprite;
		
		for(int i=0; i<3; i++)
		{
			g_sprite_lan[i] = lans[i];
		}

		sprite = atlas->CreateSprite(
				"button big green",
				Vector2(0.5f, 0.5f),
				g_pixel_per_unit,
				Sprite::Type::Simple,
				Vector2(0, 0));
		std::shared_ptr<Sprite> button = sprite;

		sprite = atlas->CreateSprite(
				"back",
				Vector2(0.5f, 0.5f),
				g_pixel_per_unit,
				Sprite::Type::Simple,
				Vector2(0, 0));
		std::shared_ptr<Sprite> back = sprite;

		{
			auto settings_main = GameObject::Create("settings_main");
			settings_main->GetTransform()->SetParent(g_win_settings->GetTransform());
			settings_main->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
			settings_main->GetTransform()->SetLocalScale(Vector3(1, 1, 1));

			node = GameObject::Create("button resume")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(settings_main->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(0, 100, 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(button);
			node->SetSortingOrder(2);
			batch_win->AddSprite(node);
			collider = node->GetGameObject()->AddComponent<BoxCollider>();
			collider->SetSize(button->GetSize());
			node->GetGameObject()->AddComponent<ButtonResumeEventListener>();
			
			std::string text = Localization::GetString("resume");
			auto label = CreateLabel(node->GetGameObject().get(), Vector3(0, 0, 0), 50, LabelPivot::Center, 6);
			label->GetLabel()->SetText(text);
			g_localized_text["resume"] = label.get();

			node = GameObject::Create("button about")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(settings_main->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(0, -100, 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(button);
			node->SetSortingOrder(2);
			batch_win->AddSprite(node);
			collider = node->GetGameObject()->AddComponent<BoxCollider>();
			collider->SetSize(button->GetSize());
			node->GetGameObject()->AddComponent<ButtonAboutEventListener>();

			text = Localization::GetString("about");
			label = CreateLabel(node->GetGameObject().get(), Vector3(0, 0, 0), 50, LabelPivot::Center, 6);
			label->GetLabel()->SetText(text);
			g_localized_text["about"] = label.get();

			node = GameObject::Create("lan")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(settings_main->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(370, -210, 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(lans[g_language]);
			node->SetSortingOrder(2);
			batch_win->AddSprite(node);
			collider = node->GetGameObject()->AddComponent<BoxCollider>();
			collider->SetSize(lans[g_language]->GetSize());
			node->GetGameObject()->AddComponent<ButtonLanEventListener>();
			g_sprite_node_lan = node.get();
		}

		{
			auto settings_about = GameObject::Create("settings_about");
			settings_about->GetTransform()->SetParent(g_win_settings->GetTransform());
			settings_about->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
			settings_about->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			settings_about->SetActive(false);

			std::string text = Localization::GetString("about_info");
			auto label = CreateLabel(settings_about.get(), Vector3(0, 0, 0), 40, LabelPivot::Center, 6);
			label->GetLabel()->SetText(text);
			g_localized_text["about_info"] = label.get();

			text = "Produced by <color=#00ff00ff>ViryTech</color>";
			label = CreateLabel(settings_about.get(), Vector3(420, -250, 0), 32, LabelPivot::Right, 6);
			label->GetLabel()->SetText("<outline>" + text + "</outline>");

			node = GameObject::Create("button back")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(settings_about->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(-370, -210, 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(back);
			node->SetSortingOrder(2);
			batch_win->AddSprite(node);
			collider = node->GetGameObject()->AddComponent<BoxCollider>();
			collider->SetSize(back->GetSize());
			node->GetGameObject()->AddComponent<ButtonAboutBackEventListener>();
		}

		{
			auto settings_lan = GameObject::Create("settings_lan");
			settings_lan->GetTransform()->SetParent(g_win_settings->GetTransform());
			settings_lan->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
			settings_lan->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			settings_lan->SetActive(false);

			const char *lan_keys[3] = {"lan en", "lan zh-hans", "lan zh-hant"};
			for(int i=0; i<3; i++)
			{
				node = GameObject::Create("button lan " + GTString::ToString(i))->AddComponent<SpriteNode>();
				node->GetTransform()->SetParent(settings_lan->GetTransform());
				node->GetTransform()->SetLocalPosition(Vector3(-220.f + 430 * (i % 2), 200.f - 160 * (i / 2), 0));
				node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
				node->SetSprite(button);
				node->SetSortingOrder(2);
				batch_win->AddSprite(node);
				collider = node->GetGameObject()->AddComponent<BoxCollider>();
				collider->SetSize(button->GetSize());
				auto handler = node->GetGameObject()->AddComponent<ButtonLanSelectEventListener>();
				handler->index = i;
				auto lan = node;

				node = GameObject::Create("en")->AddComponent<SpriteNode>();
				node->GetTransform()->SetParent(lan->GetTransform());
				node->GetTransform()->SetLocalPosition(Vector3(-145, 0, 0));
				node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
				node->SetSprite(lans[i]);
				node->SetSortingOrder(3);
				batch_win->AddSprite(node);

				std::string text = Localization::GetString(lan_keys[i]);
				auto label = CreateLabel(lan->GetGameObject().get(), Vector3(50, 0, 0), 40, LabelPivot::Center, 6);
				label->GetLabel()->SetText(text);
			}
			
			node = GameObject::Create("button back")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(settings_lan->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(-370, -210, 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(back);
			node->SetSortingOrder(2);
			batch_win->AddSprite(node);
			collider = node->GetGameObject()->AddComponent<BoxCollider>();
			collider->SetSize(back->GetSize());
			node->GetGameObject()->AddComponent<ButtonLanBackEventListener>();
		}
	}
	
	void CreateUI(UICanvas *canvas, UIAtlas *atlas)
	{
		auto batch_ui = GameObject::Create("")->AddComponent<SpriteBatchRenderer>();
		batch_ui->GetTransform()->SetParent(canvas->GetTransform());
		batch_ui->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		batch_ui->SetSortingOrder(3, 0);
		m_batch_ui = batch_ui.get();

		auto batch_bag = GameObject::Create("")->AddComponent<SpriteBatchRenderer>();
		batch_bag->GetTransform()->SetParent(batch_ui->GetTransform());
		batch_bag->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		batch_bag->SetSortingOrder(1, 0);

		auto sprite = atlas->CreateSprite(
			"bar_bottom",
			Vector2(0.5f, 1),
			g_pixel_per_unit,
			Sprite::Type::Tiled,
			Vector2(1920, 89));
		auto node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(batch_ui->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(sprite);
		node->SetSortingOrder(0);
		batch_ui->AddSprite(node);

		sprite = atlas->CreateSprite(
			"exp bar",
			Vector2(0, 0.5f),
			g_pixel_per_unit,
			Sprite::Type::Sliced,
			Vector2(300, 63));
		sprite->SetBorder(Vector4(15, 0, 15, 0));
		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(batch_ui->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(-940, 47, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(sprite);
		node->SetSortingOrder(1);
		batch_ui->AddSprite(node);

		sprite = atlas->CreateSprite(
			"exp",
			Vector2(0, 0.5f),
			g_pixel_per_unit,
			Sprite::Type::Sliced,
			Vector2(295, 56));
		sprite->SetBorder(Vector4(8, 0, 8, 0));
		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(batch_ui->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(-937, 47, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(sprite);
		node->SetSortingOrder(2);
		batch_ui->AddSprite(node);
		g_sprite_exp = node.get();

		auto label_level = CreateLabel(batch_ui->GetGameObject().get(), Vector3(-790, 47, 0), 40, LabelPivot::Center, 4);
		label_level->GetLabel()->SetText("<outline>Lv." + GTString::ToString(g_level) + "</outline>");
		label_level->SetColor(Color(31, 255, 5, 255) / 255.0f);
		g_label_level = label_level.get();

		sprite = atlas->CreateSprite(
			"gold",
			Vector2(0.5f, 0.5f),
			g_pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(0, 0));
		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(batch_ui->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(-550, 47, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(sprite);
		node->SetSortingOrder(1);
		batch_ui->AddSprite(node);

		auto label_gold = CreateLabel(batch_ui->GetGameObject().get(), Vector3(-490, 47, 0), 40, LabelPivot::Left, 4);
		label_gold->GetLabel()->SetText("<outline>" + GTString::ToString(g_gold) + "</outline>");
		label_gold->SetColor(Color(228, 255, 0, 255) / 255.0f);
		g_label_gold = label_gold.get();

		sprite = atlas->CreateSprite(
			"settings",
			Vector2(0.5f, 0.5f),
			g_pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(0, 0));
		node = GameObject::Create("settings")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(batch_ui->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(912, 45, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(sprite);
		node->SetSortingOrder(1);
		batch_ui->AddSprite(node);
		auto collider = node->GetGameObject()->AddComponent<BoxCollider>();
		collider->SetSize(sprite->GetSize());
		node->GetGameObject()->AddComponent<ButtonSettingsEventListener>();

		auto bag = GameObject::Create("bag");
		bag->GetTransform()->SetParent(batch_bag->GetTransform());
		bag->GetTransform()->SetLocalPosition(Vector3(0, 88, 0));
		bag->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		g_bag = bag.get();

		sprite = atlas->CreateSprite(
			"bag",
			Vector2(0.5f, 0),
			g_pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(0, 0));
		node = GameObject::Create("bag")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(bag->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(sprite);
		node->SetSortingOrder(1);
		batch_bag->AddSprite(node);
		collider = node->GetGameObject()->AddComponent<BoxCollider>();
		collider->SetSize(Vector3(1700, 346, 0));
		collider->SetCenter(Vector3(0, -173, 0));

		sprite = atlas->CreateSprite(
			"up",
			Vector2(0.5f, 0.5f),
			g_pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(0, 0));
		node = GameObject::Create("up")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(bag->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(740, 30, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(sprite);
		node->SetSortingOrder(0);
		batch_bag->AddSprite(node);
		collider = node->GetGameObject()->AddComponent<BoxCollider>();
		collider->SetSize(Vector3(148, 73, 0));
		node->GetGameObject()->AddComponent<ButtonUpEventListener>();

		auto audio_src = node->GetGameObject()->AddComponent<AudioSource>();
		auto sound_click = AudioClip::LoadFromFile(Application::GetDataPath() + "/Assets/audio/click.wav");
		audio_src->SetClip(sound_click);

		auto tabs = GameObject::Create("tabs");
		tabs->GetTransform()->SetParent(bag->GetTransform());
		tabs->GetTransform()->SetLocalPosition(Vector3(-740, -7, 0));
		tabs->GetTransform()->SetLocalScale(Vector3(1, 1, 1));

		g_tab_group.resize(5);
		for(int i=0; i<5; i++)
		{
			auto tab_name = std::string("tab_") + GTString::ToString(i);
			sprite = atlas->CreateSprite(
				tab_name,
				Vector2(0.5f, 1),
				g_pixel_per_unit,
				Sprite::Type::Simple,
				Vector2(0, 0));
			node = GameObject::Create(tab_name)->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(tabs->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(i * 175.f, -75, 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
			node->SetSprite(sprite);
			node->SetSortingOrder(0);
			batch_bag->AddSprite(node);
			collider = node->GetGameObject()->AddComponent<BoxCollider>();
			collider->SetSize(Vector3(147, 73, 0));
			collider->SetCenter(Vector3(0, 37, 0));
			node->GetGameObject()->AddComponent<TabEventListener>();

			if(i == 0)
			{
				sprite->GetAtlas().lock()->SetSpriteData(sprite, tab_name + "_selected");
			}

			g_tab_group[i] = node.get();
		}

		for(int i=0; i<5; i++)
		{
			auto handler = g_tab_group[i]->GetGameObject()->GetComponent<TabEventListener>();
			handler->index = i;
		}

		g_tab_group[3]->GetGameObject()->SetActive(false);
		g_tab_group[4]->GetGameObject()->SetActive(false);
		
		auto cards_0 = GameObject::Create("cards_0");
		cards_0->GetTransform()->SetParent(bag->GetTransform());
		cards_0->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		cards_0->GetTransform()->SetLocalScale(Vector3(1, 1, 1));

		auto sprite_card = atlas->CreateSprite(
			"card",
			Vector2(0.5f, 0.5f),
			g_pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(0, 0));
		auto sprite_card_locked = atlas->CreateSprite(
			"card locked",
			Vector2(0.5f, 0.5f),
			g_pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(0, 0));
		for(int i=0; i<25; i++)
		{
			auto &item = g_cards[i];
			
			if(!item.name.empty())
			{
				int type = i / 5;
				int index = i % 5;
				auto card = GameObject::Create("card_" + GTString::ToString(i))->AddComponent<SpriteNode>();
				card->GetTransform()->SetParent(tabs->GetTransform());
				card->GetTransform()->SetLocalPosition(Vector3(200 + 270.0f * index, -242, 0));
				card->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
				card->SetSprite(sprite_card);
				card->SetSortingOrder(2);
				batch_bag->AddSprite(card);
				collider = card->GetGameObject()->AddComponent<BoxCollider>();
				collider->SetSize(Vector3(234, 321, 0));
				auto listener = card->GetGameObject()->AddComponent<CardEventListener>();
				listener->type_0 = type;
				listener->type_1 = index;
				card->GetGameObject()->SetActive(false);

				auto card_locked = GameObject::Create("card_locked_" + GTString::ToString(i))->AddComponent<SpriteNode>();
				card_locked->GetTransform()->SetParent(tabs->GetTransform());
				card_locked->GetTransform()->SetLocalPosition(Vector3(200 + 270.0f * index, -242, 0));
				card_locked->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
				card_locked->SetSprite(sprite_card_locked);
				card_locked->SetSortingOrder(2);
				batch_bag->AddSprite(card_locked);
				card_locked->GetGameObject()->SetActive(false);

				if(type == 0)
				{
					if(g_level >= item.unlock_level)
					{
						card->GetGameObject()->SetActive(true);
					}
					else
					{
						card_locked->GetGameObject()->SetActive(true);
					}
				}

				sprite = atlas->CreateSprite(
					item.name + " icon",
					Vector2(0.5f, 1),
					g_pixel_per_unit,
					Sprite::Type::Simple,
					Vector2(0, 0));
				node = GameObject::Create("icon")->AddComponent<SpriteNode>();
				node->GetTransform()->SetParent(card->GetTransform());
				node->GetTransform()->SetLocalPosition(Vector3(0, -67, 0));
				node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
				node->SetSprite(sprite);
				node->SetSortingOrder(3);
				batch_bag->AddSprite(node);

				auto label_price = CreateLabel(card->GetGameObject().get(), Vector3(-30, -115, 0), 40, LabelPivot::Left, 2);
				int price = (int) (item.price_multiply * g_price_plant[item.planted]);
				label_price->GetLabel()->SetText("<outline>" + GTString::ToString(price) + "</outline>");
				label_price->SetColor(Color(228, 255, 0, 255) / 255.0f);
				item.label_price = label_price;

				auto label_planted = CreateLabel(card->GetGameObject().get(), Vector3(40, 126, 0), 40, LabelPivot::Left, 2);
				label_planted->GetLabel()->SetText("<outline>" + GTString::ToString(item.planted) + "</outline>");
				label_planted->SetColor(Color(202, 237, 255, 255) / 255.0f);
				item.label_planted = label_planted;

				sprite = atlas->CreateSprite(
					item.name + " tree",
					Vector2(0.5f, 1),
					g_pixel_per_unit,
					Sprite::Type::Simple,
					Vector2(0, 0));
				node = GameObject::Create("tree")->AddComponent<SpriteNode>();
				node->GetTransform()->SetParent(card->GetTransform());
				node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
				node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
				node->SetSprite(sprite);
				node->SetSortingOrder(1);
				batch_ui->AddSprite(node);
				node->GetGameObject()->SetActive(false);

				Vector2 fruit_pivot;
				if(type == 0)
				{
					fruit_pivot = Vector2(0.5f, 1);
				}
				else
				{
					fruit_pivot = Vector2(0.5f, 0);
				}

				sprite = atlas->CreateSprite(
					item.name + " fruit",
					fruit_pivot,
					g_pixel_per_unit,
					Sprite::Type::Simple,
					Vector2(0, 0));
				node = GameObject::Create("fruit")->AddComponent<SpriteNode>();
				node->GetTransform()->SetParent(card->GetTransform());
				node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
				node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
				node->SetSprite(sprite);
				node->GetGameObject()->SetActive(false);

				if(type != g_tab_current)
				{
					node->GetGameObject()->SetActive(false);
				}
			}
		}

		for(int i=0; i<8; i++)
		{
			m_sprite_coin_anim[i] = g_atlas->CreateSprite(
				"coin " + GTString::ToString(i),
				Vector2(0.5f, 0.5f),
				g_pixel_per_unit,
				Sprite::Type::Simple,
				Vector2(0, 0));
			g_sprite_coin_anim[i] = m_sprite_coin_anim[i];
		}

		m_batch_ui->GetTransform()->SetLocalPosition(Vector3(0, -540, 0));

		set_exp(g_exp);
		set_level(g_level);
		set_gold(g_gold);
	}

    virtual void Start()
	{
		auto bgm = GameObject::Create("")->AddComponent<AudioSource>();
		bgm->SetLoop(true);
		bgm->PlayMp3File(Application::GetDataPath() + "/Assets/audio/bgm.mp3");

		Screen::AddResizeListener(std::dynamic_pointer_cast<IScreenResizeEventListener>(GetComponentPtr()));

		GTUIManager::LoadFont("heiti", Application::GetDataPath() + "/Assets/font/heiti.ttc");
		set_language(0);
		load_config();

		auto screen_buffer = RenderTexture::Create(1920, 1080, RenderTextureFormat::RGBA32, DepthBuffer::Depth_0, FilterMode::Bilinear, TextureWrapMode::Clamp);
		auto cam_screen = GameObject::Create("")->AddComponent<Camera>();
		cam_screen->SetOrthographic(true);
		cam_screen->SetOrthographicSize(1 / g_pixel_per_unit * Screen::GetHeight() / 2);
		cam_screen->SetClipPlane(-1, 1);
		cam_screen->SetCullingMask(LayerMask::GetMask(Layer::Default));
		cam_screen->SetDepth(1);
		cam_screen->SetClearColor(Color(0, 0, 0, 1));
		cam_screen->GetTransform()->SetLocalScale(Vector3::One() * (Screen::GetHeight() / 1080.f) * (1.0f / g_pixel_per_unit));
		m_cam = cam_screen.get();

		auto screen_sprite = Sprite::Create(std::dynamic_pointer_cast<Texture>(screen_buffer));

		auto screen_renderer = GameObject::Create("")->AddComponent<SpriteRenderer>();
		screen_renderer->GetTransform()->SetParent(cam_screen->GetTransform());
#ifdef ANDROID
		screen_renderer->GetTransform()->SetLocalScale(Vector3(1, -1, 1));
#else
		screen_renderer->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
#endif
		screen_renderer->SetSprite(screen_sprite);

		float input_scale = 1080.f / Screen::GetHeight();
		float input_offset_x = (1920 - Screen::GetWidth() * input_scale) * 0.5f;
		float input_offset_y = (1080 - Screen::GetHeight() * input_scale) * 0.5f;
		Input::SetMousePositionScaleOffset(Vector3::One() * input_scale, Vector3(input_offset_x, input_offset_y, 0));

		auto cam = GameObject::Create("")->AddComponent<Camera>();
		cam->SetOrthographic(true);
		cam->SetOrthographicSize(1 / g_pixel_per_unit * 1080 / 2);
		cam->SetClipPlane(-1, 1);
		cam->SetCullingMask(LayerMask::GetMask(Layer::UI));
		cam->SetDepth(0);
		cam->SetClearColor(Color(142, 239, 255, 255) / 255.0f);
		cam->SetRenderTexture(screen_buffer);

		auto canvas = GameObject::Create("")->AddComponent<UICanvas>();
		canvas->GetTransform()->SetParent(cam->GetTransform());
		canvas->GetTransform()->SetLocalScale(Vector3::One() * (1.0f / g_pixel_per_unit));
		canvas->SetSize(1920, 1080);
		canvas->SetCamera(cam);
		m_canvas = canvas.get();

		auto label = Label::Create("", "heiti", 20, LabelPivot::Top, LabelAlign::Auto, true);
		auto tr = GameObject::Create("fps")->AddComponent<TextRenderer>();
		tr->SetLabel(label);
		tr->SetSortingOrder(1000, 0);
		tr->GetTransform()->SetParent(canvas->GetTransform());
		tr->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		tr->SetColor(Color(0, 0, 0, 1));
		tr->SetAnchor(Vector4(0.5f, 0, 0, 0));
		m_fps = tr.get();
		m_fps->GetGameObject()->SetActive(false);
		
		auto batch = GameObject::Create("")->AddComponent<SpriteBatchRenderer>();
		batch->GetTransform()->SetParent(canvas->GetTransform());
		batch->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		batch->SetSortingOrder(-1, 0);
		g_batch_game = batch.get();

		auto atlas = GTUIManager::LoadAtlas("plant", Application::GetDataPath() + "/Assets/image/plant.json");
		g_atlas = atlas.get();

		g_grass_node_0 = GameObject::Create("grass_node_0").get();
		g_grass_node_0->GetTransform()->SetParent(batch->GetTransform());
		g_grass_node_0->GetTransform()->SetLocalPosition(Vector3(0, -325, 0));
		g_grass_node_0->GetTransform()->SetLocalScale(Vector3(1, 1, 1));

		g_grass_node_0_copy = GameObject::Create("grass_node_0").get();
		g_grass_node_0_copy->GetTransform()->SetParent(batch->GetTransform());
		g_grass_node_0_copy->GetTransform()->SetLocalPosition(Vector3(0, -325, 0));
		g_grass_node_0_copy->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		g_grass_node_0_copy->SetActive(false);

		g_grass_node_1 = GameObject::Create("grass_node_1").get();
		g_grass_node_1->GetTransform()->SetParent(batch->GetTransform());
		g_grass_node_1->GetTransform()->SetLocalPosition(Vector3(0, -385, 0));
		g_grass_node_1->GetTransform()->SetLocalScale(Vector3(1, 1, 1));

		g_grass_node_1_copy = GameObject::Create("grass_node_1").get();
		g_grass_node_1_copy->GetTransform()->SetParent(batch->GetTransform());
		g_grass_node_1_copy->GetTransform()->SetLocalPosition(Vector3(0, -385, 0));
		g_grass_node_1_copy->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		g_grass_node_1_copy->SetActive(false);

		auto grass_0 = atlas->CreateSprite(
			"grass_0",
			Vector2(1, 1),
			g_pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(1025, 120),
			Vector4(1, 0, -1, 0));
		auto node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(g_grass_node_0->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(grass_0);
		node->SetSortingOrder(1);
		batch->AddSprite(node);

		auto grass_1 = atlas->CreateSprite(
			"grass_1",
			Vector2(0, 1),
			g_pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(1025, 120),
			Vector4(1, 0, -1, 0));
		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(g_grass_node_0->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(grass_1);
		node->SetSortingOrder(1);
		batch->AddSprite(node);

		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(g_grass_node_1->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(grass_0);
		node->SetSortingOrder(2);
		node->SetColor(Color(0.5f, 0.8f, 0.5f, 1));
		batch->AddSprite(node);

		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(g_grass_node_1->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(grass_1);
		node->SetSortingOrder(2);
		node->SetColor(Color(0.5f, 0.8f, 0.5f, 1));
		batch->AddSprite(node);

		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(g_grass_node_0_copy->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(grass_0);
		node->SetSortingOrder(1);
		batch->AddSprite(node);

		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(g_grass_node_0_copy->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(grass_1);
		node->SetSortingOrder(1);
		batch->AddSprite(node);

		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(g_grass_node_1_copy->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(grass_0);
		node->SetSortingOrder(2);
		node->SetColor(Color(0.5f, 0.8f, 0.5f, 1));
		batch->AddSprite(node);

		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(g_grass_node_1_copy->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(grass_1);
		node->SetSortingOrder(2);
		node->SetColor(Color(0.5f, 0.8f, 0.5f, 1));
		batch->AddSprite(node);

		auto sprite_white = atlas->CreateSprite(
			"white",
			Vector2(0.5f, 0),
			g_pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(1920, 1000),
			Vector4(1, 1, -1, -1));

		// transparent sprite for ground boxcollider
		auto ground = GameObject::Create("ground")->AddComponent<SpriteNode>();
		ground->GetTransform()->SetParent(batch->GetTransform());
		ground->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		ground->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		ground->SetSprite(sprite_white);
		ground->SetSortingOrder(0);
		ground->SetColor(Color(1, 1, 1, 0));
		batch->AddSprite(ground);
		auto collider_ground = ground->GetGameObject()->AddComponent<BoxCollider>();
		collider_ground->SetSize(Vector3(2048, 2048, 0));
		ground->GetGameObject()->AddComponent<GroundEventListener>();

		auto sprite = atlas->CreateSprite(
			"ground",
			Vector2(0.5f, 0),
			g_pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(0, 0));
		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(ground->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, -325, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(sprite);
		node->SetSortingOrder(1000);
		batch->AddSprite(node);
		g_ground_node = node->GetGameObject().get();

		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(ground->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, -325, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(sprite);
		node->SetSortingOrder(1000);
		batch->AddSprite(node);
		g_ground_node_copy = node->GetGameObject().get();
		g_ground_node_copy->SetActive(false);

		sprite = atlas->CreateSprite(
			"limit",
			Vector2(0.5f, 1.f),
			g_pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(0, 0));
		node = GameObject::Create("limit left")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(ground->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, -325, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(sprite);
		node->SetSortingOrder(999);
		batch->AddSprite(node);
		auto collider = node->GetGameObject()->AddComponent<BoxCollider>();
		collider->SetSize(sprite->GetSize());
		collider->SetCenter(Vector3(0, sprite->GetSize().y * 0.5f, 0));
		node->GetGameObject()->AddComponent<MapLimitIncreaseEventListener>();
		g_map_limit_left = node->GetGameObject().get();

		node = GameObject::Create("limit right")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(ground->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, -325, 0));
		node->GetTransform()->SetLocalRotation(Quaternion::Euler(0, 180, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(sprite);
		node->SetSortingOrder(999);
		batch->AddSprite(node);
		collider = node->GetGameObject()->AddComponent<BoxCollider>();
		collider->SetSize(sprite->GetSize());
		collider->SetCenter(Vector3(0, sprite->GetSize().y * 0.5f, 0));
		node->GetGameObject()->AddComponent<MapLimitIncreaseEventListener>();
		g_map_limit_right = node->GetGameObject().get();

		sprite = atlas->CreateSprite(
			"gold",
			Vector2(0.5f, 0.5f),
			g_pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(0, 0));

		auto limit_info = GameObject::Create("limit_info");
		limit_info->GetTransform()->SetParent(g_map_limit_left->GetTransform());
		limit_info->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		limit_info->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		limit_info->SetActive(false);

		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(limit_info->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(-50, 150, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(sprite);
		node->SetSortingOrder(999);
		batch->AddSprite(node);

		auto label_limit = CreateLabel(limit_info.get(), Vector3(0, 150, 0), 40, LabelPivot::Left, 0);
		label_limit->GetLabel()->SetText("<bold>" + 
			GTString::ToString(g_gold) + "/" + GTString::ToString(g_gold_limit_increase) +
			"</bold>");
		label_limit->SetColor(Color(1, 0, 0, 1));
		g_label_gold_limit_increase_left = label_limit.get();

		limit_info = GameObject::Create("limit_info");
		limit_info->GetTransform()->SetParent(g_map_limit_right->GetTransform());
		limit_info->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		limit_info->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		limit_info->SetActive(false);

		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(limit_info->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(50, 150, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(sprite);
		node->SetSortingOrder(999);
		batch->AddSprite(node);

		label_limit = CreateLabel(limit_info.get(), Vector3(0, 150, 0), 40, LabelPivot::Right, 0);
		label_limit->GetLabel()->SetText("<bold>" +
			GTString::ToString(g_gold) + "/" + GTString::ToString(g_gold_limit_increase) +
			"/<bold>");
		label_limit->SetColor(Color(1, 0, 0, 1));
		g_label_gold_limit_increase_right = label_limit.get();

		sprite = atlas->CreateSprite(
			"wave",
			Vector2(0, 1),
			g_pixel_per_unit,
			Sprite::Type::Simple,
			Vector2(199, 50),
			Vector4(1, 0, -1, -2));

		auto wave = GameObject::Create("");
		wave->GetTransform()->SetParent(batch->GetTransform());
		wave->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));
		wave->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		auto tp = wave->AddComponent<TweenPosition>();
		tp->duration = 3.0f;
		tp->loop = true;
		tp->curve = AnimationCurve();
		tp->curve.keys.push_back(Keyframe(0, 0, 1, 1));
		tp->curve.keys.push_back(Keyframe(0.5f, 1, 1, -1));
		tp->curve.keys.push_back(Keyframe(1, 0, -1, -1));
		tp->from = Vector3(0, 0, 0);
		tp->to = Vector3(0, 20, 0);

		auto wave_node_0 = GameObject::Create("");
		wave_node_0->GetTransform()->SetParent(wave->GetTransform());
		wave_node_0->GetTransform()->SetLocalPosition(Vector3(0, -410, 0));
		wave_node_0->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		g_wave_node_0 = wave_node_0.get();
		
		for(int i=0; i<11; i++)
		{
			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(g_wave_node_0->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(-1920 / 2.0f + (i - 1) * 199.0f, 0, 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 0.7f, 1));
			node->SetSprite(sprite);
			node->SetSortingOrder(1002);
			batch->AddSprite(node);

			auto ts = node->GetGameObject()->AddComponent<TweenScale>();
			ts->duration = 3.0f;
			ts->loop = true;
			ts->curve = AnimationCurve();
			ts->curve.keys.push_back(Keyframe(0, 0, 1, 1));
			ts->curve.keys.push_back(Keyframe(0.5f, 1, 1, -1));
			ts->curve.keys.push_back(Keyframe(1, 0, -1, -1));
			ts->from = Vector3(1, 0.35f, 1);
			ts->to = Vector3(1, 0.75f, 1);
		}

		auto wave_node_1 = GameObject::Create("");
		wave_node_1->GetTransform()->SetParent(wave->GetTransform());
		wave_node_1->GetTransform()->SetLocalPosition(Vector3(0, -440, 0));
		wave_node_1->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		g_wave_node_1 = wave_node_1.get();
		
		for(int i=0; i<11; i++)
		{
			node = GameObject::Create("")->AddComponent<SpriteNode>();
			node->GetTransform()->SetParent(g_wave_node_1->GetTransform());
			node->GetTransform()->SetLocalPosition(Vector3(-1920 / 2.0f + (i - 1) * 199.0f, 0, 0));
			node->GetTransform()->SetLocalScale(Vector3(1, 0.7f, 1));
			node->SetSprite(sprite);
			node->SetSortingOrder(1002);
			batch->AddSprite(node);

			auto ts = node->GetGameObject()->AddComponent<TweenScale>();
			ts->duration = 3.0f;
			ts->loop = true;
			ts->curve = AnimationCurve();
			ts->curve.keys.push_back(Keyframe(0, 0, 1, 1));
			ts->curve.keys.push_back(Keyframe(0.5f, 1, 1, -1));
			ts->curve.keys.push_back(Keyframe(1, 0, -1, -1));
			ts->from = Vector3(1, 0.35f, 1);
			ts->to = Vector3(1, 0.75f, 1);
		}

		// for water
		float water_offset = 2;
		node = GameObject::Create("")->AddComponent<SpriteNode>();
		node->GetTransform()->SetParent(wave->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, -410 + water_offset, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(sprite_white);
		node->SetSortingOrder(1001);
		node->SetColor(Color(57, 131, 254, 255) / 255.0f);
		batch->AddSprite(node);
		g_batch_game->GetTransform()->SetLocalPosition(Vector3(0, 0, 0));

		CreateUI(canvas.get(), atlas.get());
		CreateWindowSettings(canvas.get(), atlas.get());

		cam->GetGameObject()->SetLayerRecursively(Layer::UI);
		cam->GetTransform()->SetParent(GetTransform());

		g_map_pos = 0;
		g_map_pos_limit = 1000;
		g_grass_pos_0 = 0;
		g_grass_pos_1 = 1025;
		g_ground_pos = 0;
		g_wave_pos_0 = 0;
		g_wave_pos_1 = 100;

		g_grass_pos_0_init = g_grass_pos_0;
		g_grass_pos_1_init = g_grass_pos_1;
		g_ground_pos_init = g_ground_pos;
		g_wave_pos_0_init = g_wave_pos_0;
		g_wave_pos_1_init = g_wave_pos_1;

		set_map_pos(g_map_pos);

		g_plants[0].resize(101);
		g_plants[1].resize(100);
		g_plants[2].resize(100);
	}

	virtual void Update()
	{
		std::string hit_name;
		auto hit = UICanvas::GetRayHitObject().lock();
		if(hit)
		{
			hit_name = hit->GetName();
		}

		m_fps->GetLabel()->SetText("fps:" + GTString::ToString(GTTime::GetFPS()) + "\n" +
			"draw call:" + GTString::ToString(GTTime::GetDrawCall()) + "\n" +
			"ray hit ui:" + hit_name + "\n"
			"map_x:" + GTString::ToString((int) g_map_pos));

		float wave_speed = GTTime::GetDeltaTime() * 120;
		
		g_wave_pos_0 -= wave_speed;
		g_wave_pos_0_init -= wave_speed;
		set_wave_pos((int) g_wave_pos_0, g_wave_node_0);

		g_wave_pos_1 -= wave_speed * 2.0f;
		g_wave_pos_1_init -= wave_speed * 2.0f;
		set_wave_pos((int) g_wave_pos_1, g_wave_node_1);
	}
};
#include "Component.h"
#include "Camera.h"
#include "Screen.h"
#include "UICanvas.h"
#include "SpriteRenderer.h"
#include "Application.h"
#include "GameObject.h"
#include "TweenPosition.h"
#include "Image.h"
#include "GTFile.h"
#include "Debug.h"

using namespace Galaxy3D;

static const float PIXEL_PER_UNIT = 100.f;
static const float GRAVITY = 400.f;
static const int WHEEL_D = 38;
static const int WHEEL_R = WHEEL_D / 2;
static std::vector<int> g_heights;
static SpriteRenderer *g_car;
static Vector2 g_velocity(0, 0);
static bool g_grounded[2] = {false, false};
static float g_car_deg = 0;

class LauncherLimbo : public Component
{
	virtual void Start()
	{
		//1440 x 720
		auto cam = GameObject::Create("")->AddComponent<Camera>();
		cam->SetOrthographic(true);
		cam->SetOrthographicSize(1 / PIXEL_PER_UNIT * Screen::GetHeight() / 2 * 720 / Screen::GetHeight());
		cam->SetClipPlane(-1, 1);
		cam->SetDepth(0);
		cam->SetClearColor(Color(71, 71, 71, 255) / 255.f);

		auto canvas = GameObject::Create("")->AddComponent<UICanvas>();
		canvas->GetTransform()->SetParent(cam->GetTransform());
		canvas->GetTransform()->SetLocalScale(Vector3::One() * (1.0f / PIXEL_PER_UNIT));

		auto ground3 = Sprite::LoadFromFile(Application::GetDataPath() + "/Assets/image/ground3.png");
		auto node = GameObject::Create("")->AddComponent<SpriteRenderer>();
		node->GetTransform()->SetParent(canvas->GetTransform());
		node->GetTransform()->SetLocalScale(Vector3(1440 / 1280.f, 1, 1));
		node->SetSprite(ground3);
		node->SetSortingOrder(0, 0);

		auto fog = Sprite::LoadFromFile(Application::GetDataPath() + "/Assets/image/fog.jpg");
		node = GameObject::Create("")->AddComponent<SpriteRenderer>();
		node->GetTransform()->SetParent(canvas->GetTransform());
		node->GetTransform()->SetLocalScale(Vector3(1440 / 1280.f, 720 / 800.f, 1));
		node->SetSprite(fog);
		node->SetSortingOrder(0, 1);
		node->SetColor(Color(1, 1, 1, 68 / 255.f));

		auto witch = Sprite::LoadFromFile(Application::GetDataPath() + "/Assets/image/witch.png");
		node = GameObject::Create("")->AddComponent<SpriteRenderer>();
		node->GetTransform()->SetParent(canvas->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(-552, 174, 0));
		node->GetTransform()->SetLocalScale(Vector3::One() * 0.2f);
		node->SetSprite(witch);
		node->SetSortingOrder(0, 2);
		auto tp = node->GetGameObject()->AddComponent<TweenPosition>();
		tp->duration = 3.f;
		tp->loop = true;
		tp->from = node->GetTransform()->GetLocalPosition();
		tp->to = tp->from + Vector3(0, 10, 0);
		tp->curve = AnimationCurve();
		tp->curve.keys.push_back(Keyframe(0, 0, 1, 1));
		tp->curve.keys.push_back(Keyframe(0.5f, 1, 1, -1));
		tp->curve.keys.push_back(Keyframe(1, 0, -1, -1));

		auto tree2 = Sprite::LoadFromFile(Application::GetDataPath() + "/Assets/image/tree2.png");
		node = GameObject::Create("")->AddComponent<SpriteRenderer>();
		node->GetTransform()->SetParent(canvas->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(-90, 19, 0));
		node->GetTransform()->SetLocalScale(Vector3::One());
		node->SetSprite(tree2);
		node->SetSortingOrder(0, 2);

		auto ground2 = Sprite::LoadFromFile(Application::GetDataPath() + "/Assets/image/ground2.png");
		node = GameObject::Create("")->AddComponent<SpriteRenderer>();
		node->GetTransform()->SetParent(canvas->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, -108, 0));
		node->GetTransform()->SetLocalScale(Vector3(1440 / 1280.f, 1, 1));
		node->SetSprite(ground2);
		node->SetSortingOrder(0, 3);

		node = GameObject::Create("")->AddComponent<SpriteRenderer>();
		node->GetTransform()->SetParent(canvas->GetTransform());
		node->GetTransform()->SetLocalScale(Vector3(1440 / 1280.f, 720 / 800.f, 1));
		node->SetSprite(fog);
		node->SetSortingOrder(0, 4);
		node->SetColor(Color(1, 1, 1, 116 / 255.f));

		auto bird = Sprite::LoadFromFile(Application::GetDataPath() + "/Assets/image/bird.png");
		node = GameObject::Create("")->AddComponent<SpriteRenderer>();
		node->GetTransform()->SetParent(canvas->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(167, -93, 0));
		node->GetTransform()->SetLocalScale(Vector3::One());
		node->SetSprite(bird);
		node->SetSortingOrder(0, 5);

		auto mushroom = Sprite::LoadFromFile(Application::GetDataPath() + "/Assets/image/mushroom.png");
		node = GameObject::Create("")->AddComponent<SpriteRenderer>();
		node->GetTransform()->SetParent(canvas->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(513, -270, 0));
		node->GetTransform()->SetLocalScale(Vector3::One() * 0.4f);
		node->SetSprite(mushroom);
		node->SetSortingOrder(0, 5);

		node = GameObject::Create("")->AddComponent<SpriteRenderer>();
		node->GetTransform()->SetParent(canvas->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(-221, -199, 0));
		node->GetTransform()->SetLocalScale(Vector3::One() * 0.4f);
		node->SetSprite(mushroom);
		node->SetSortingOrder(0, 5);

		auto tree1 = Sprite::LoadFromFile(Application::GetDataPath() + "/Assets/image/tree1.png");
		node = GameObject::Create("")->AddComponent<SpriteRenderer>();
		node->GetTransform()->SetParent(canvas->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(316, -79, 0));
		node->GetTransform()->SetLocalScale(Vector3::One());
		node->SetSprite(tree1);
		node->SetSortingOrder(0, 6);

		auto car = Sprite::LoadFromFile(Application::GetDataPath() + "/Assets/image/car.png");
		node = GameObject::Create("")->AddComponent<SpriteRenderer>();
		node->GetTransform()->SetParent(canvas->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(-250, 0, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(car);
		node->SetSortingOrder(0, 7);
		g_car = node.get();

		auto wheel = Sprite::LoadFromFile(Application::GetDataPath() + "/Assets/image/wheel.png");
		node = GameObject::Create("left")->AddComponent<SpriteRenderer>();
		node->GetTransform()->SetParent(g_car->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(-53, -28, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(wheel);
		node->SetSortingOrder(0, 7);

		node = GameObject::Create("right")->AddComponent<SpriteRenderer>();
		node->GetTransform()->SetParent(g_car->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(47, -28, 0));
		node->GetTransform()->SetLocalScale(Vector3(1, 1, 1));
		node->SetSprite(wheel);
		node->SetSortingOrder(0, 7);

		auto ground1 = Sprite::LoadFromFile(Application::GetDataPath() + "/Assets/image/ground1.png");
		node = GameObject::Create("")->AddComponent<SpriteRenderer>();
		node->GetTransform()->SetParent(canvas->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, -108, 0));
		node->GetTransform()->SetLocalScale(Vector3(1440 / 1280.f, 1, 1));
		node->SetSprite(ground1);
		node->SetSortingOrder(0, 8);

		auto ground1_fog = Sprite::LoadFromFile(Application::GetDataPath() + "/Assets/image/ground1_fog.png");
		node = GameObject::Create("")->AddComponent<SpriteRenderer>();
		node->GetTransform()->SetParent(canvas->GetTransform());
		node->GetTransform()->SetLocalPosition(Vector3(0, 652, 0));
		node->GetTransform()->SetLocalScale(Vector3(1440 / 188.f, 720 / 178.f, 1));
		node->SetSprite(ground1_fog);
		node->SetSortingOrder(0, 8);
		node->SetColor(Color(1, 1, 1, 0.33f));

		int file_size;
        char *bytes = (char *) GTFile::ReadAllBytes(Application::GetDataPath() + "/Assets/image/ground1_collider.png", &file_size);
        if(bytes != NULL)
		{
			int w, h, bpp;
			char *pixels = Image::LoadPng(bytes, file_size, &w, &h, &bpp);

			g_heights.resize(w);
			for(int i=0; i<w; i++)
			{
				for(int j=0; j<h; j++)
				{
					if(pixels[(i + j * w) * 4 + 3] != 0)
					{
						g_heights[i] = j;
						break;
					}
				}
			}

			free(pixels);
			free(bytes);
		}
	}

	void UpdateDrop()
	{
		float t = 0.0167f;
		g_velocity.y = g_velocity.y - GRAVITY * t;
		float s = g_velocity.y * t;

		auto car_pos = g_car->GetTransform()->GetLocalPosition();
		float y_next = car_pos.y + s;
		int x_hit;
		int y_hit;

		// left wheel
		bool grounded_left = CheckWheelCollide(1440 / 2 + (int) car_pos.x - 53, (int) y_next - 28, &g_heights[0], &x_hit, &y_hit);
		bool grounded_right = CheckWheelCollide(1440 / 2 + (int) car_pos.x + 47, (int) y_next - 28, &g_heights[0], &x_hit, &y_hit);

		if(grounded_left || grounded_right)
		{
			int x = Mathf::Abs(WHEEL_R - x_hit);
			int h = (int) sqrt(WHEEL_R * WHEEL_R - x * x);
			int y = y_hit + h + 28;

			car_pos.y = (float) y;
			g_car->GetTransform()->SetLocalPosition(car_pos);

			g_grounded[0] = grounded_left;
			g_grounded[1] = grounded_right;
		}
		else
		{
			car_pos.y = y_next;
			g_car->GetTransform()->SetLocalPosition(car_pos);
		}
	}

	void UpdateRotate()
	{
		float t = 0.0167f;
		float s = g_velocity.y * t;
		auto old_rot = g_car->GetTransform()->GetLocalRotation();
		auto old_pos = g_car->GetTransform()->GetPosition();

		if(g_grounded[0])
		{
			auto left = g_car->GetTransform()->Find("left");
			auto left_pos = left->GetPosition();

			float a = 53 + 47;
			float cos_a = 1 - s * s / (2 * a * a);
			float deg = acosf(cos_a) * Mathf::Rad2Deg;
			auto rot = Quaternion::Euler(0, 0, -deg);
			g_car_deg -= deg;
			rot = g_car->GetTransform()->GetLocalRotation() * rot;
			g_car->GetTransform()->SetLocalRotation(rot);

			auto offset = left->GetPosition() - left_pos;
			auto pos = g_car->GetTransform()->GetPosition();
			pos = pos - offset;
			g_car->GetTransform()->SetPosition(pos);

			auto right = g_car->GetTransform()->Find("right");
			auto right_pos = rot * right->GetLocalPosition();

			int x_hit, y_hit;
			auto car_pos = g_car->GetTransform()->GetLocalPosition();
			auto wheel_pos = car_pos + right_pos;
			bool grounded_right = CheckWheelCollide(1440 / 2 + (int) wheel_pos.x, (int) wheel_pos.y, &g_heights[0], &x_hit, &y_hit);
			if(grounded_right)
			{
				g_car->GetTransform()->SetLocalRotation(old_rot);
				g_car->GetTransform()->SetPosition(old_pos);
				g_car_deg += deg;

				g_velocity.y *= 0.5f;

				if(s > -1.f)
				{
					g_velocity.y = 0;
					g_grounded[1] = true;
				}
			}
			else
			{
				g_velocity.y = g_velocity.y - GRAVITY * t;
			}
		}
		else if(g_grounded[1])
		{
			auto right = g_car->GetTransform()->Find("right");
			auto right_pos = right->GetPosition();

			float a = 53 + 47;
			float cos_a = 1 - s * s / (2 * a * a);
			float deg = acosf(cos_a) * Mathf::Rad2Deg;
			auto rot = Quaternion::Euler(0, 0, deg);
			g_car_deg += deg;
			rot = g_car->GetTransform()->GetLocalRotation() * rot;
			g_car->GetTransform()->SetLocalRotation(rot);

			auto offset = right->GetPosition() - right_pos;
			auto pos = g_car->GetTransform()->GetPosition();
			pos = pos - offset;
			g_car->GetTransform()->SetPosition(pos);

			auto left = g_car->GetTransform()->Find("left");
			auto left_pos = rot * left->GetLocalPosition();

			int x_hit, y_hit;
			auto car_pos = g_car->GetTransform()->GetLocalPosition();
			auto wheel_pos = car_pos + left_pos;
			bool grounded_left = CheckWheelCollide(1440 / 2 + (int) wheel_pos.x, (int) wheel_pos.y, &g_heights[0], &x_hit, &y_hit);
			if(grounded_left)
			{
				g_car->GetTransform()->SetLocalRotation(old_rot);
				g_car->GetTransform()->SetPosition(old_pos);
				g_car_deg -= deg;

				g_velocity.y *= 0.5f;

				if(s > -1.f)
				{
					g_velocity.y = 0;
					g_grounded[0] = true;
				}
			}
			else
			{
				g_velocity.y = g_velocity.y - GRAVITY * t;
			}
		}
	}

	void SetCarPos(const Vector3 &new_pos)
	{
		//g_car->GetTransform()->SetLocalPosition(new_pos);
	}

	void UpdateDrive()
	{
		auto rot = Quaternion::Euler(0, 0, g_car_deg);
		auto a_dir = rot * Vector3(-Mathf::Sign(g_car_deg), 0, 0);
		float a = abs(sinf(Mathf::Deg2Rad * g_car_deg)) * GRAVITY;
		Vector3 a_vec = a_dir * a;

		float t = 0.0167f;
		g_velocity = g_velocity + a_vec * t;
		auto s = g_velocity * t;

		auto car_pos = g_car->GetTransform()->GetLocalPosition();
		car_pos += s;
		SetCarPos(car_pos);
	}

	virtual void Update()
	{
		if(!g_grounded[0] && !g_grounded[1])
		{
			UpdateDrop();
		}
		else if(!(g_grounded[0] && g_grounded[1]))
		{
			UpdateRotate();
		}
		else
		{
			UpdateDrive();
		}
	}

	bool CheckWheelCollide(int wheel_x, int wheel_y, int *collider_heights, int *x_hit, int *y_hit)
	{
		for(int i=0; i<WHEEL_D; i++)
		{
			int y_road = -(collider_heights[(wheel_x - WHEEL_R + i) * 1280 / 1440] - 400 + 108);

			int x = Mathf::Abs(WHEEL_R - i);
			int h = (int) sqrt(WHEEL_R * WHEEL_R - x * x);
			int y_wheel_point = wheel_y - h;

			if(y_wheel_point <= y_road)
			{
				*x_hit = i;
				*y_hit = y_road;
				return true;
			}
		}

		return false;
	}
};
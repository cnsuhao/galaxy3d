#include "MirHero.h"
#include "MirMap.h"
#include "GTString.h"
#include "Application.h"
#include "GTFile.h"
#include "GTTime.h"
#include "zlib.h"
#include "Debug.h"

struct ActionFrame
{
    int index;
    int length;
    int full;
    float update_time;
};

static ActionFrame g_action_frames[] =
{
	{8 * 0,		4, 8, 0.5f},
	{8 * 8,		6, 8, 0.12f},
	{8 * 16,	6, 8, 0.12f},
	{8 * 24,	1, 1, 0.5f},
	{8 * 25,	6, 8, 0.12f},
	{8 * 33,	6, 8, 0.12f},
	{8 * 41,	8, 8, 0.12f},
	{8 * 49,	6, 8, 0.15f},
	{8 * 57,	2, 2, 0.32f},
	{8 * 59,	3, 8, 0.2f},
	{8 * 67,	4, 8, 0.2f},
};

struct Direction
{
	int x;
	int y;
};

static Direction g_dirs[] =
{
	{0, -1},
	{1, -1},
	{1, 0},
	{1, 1},
	{0, 1},
	{-1, 1},
	{-1, 0},
	{-1, -1},
};

std::unordered_map<std::string, MirHero::Frames> g_frames;

MirHero::MirHero(int x, int y, int body, int hair, int weapon, int sex):
	m_pox_x(x),
	m_pox_y(y),
	m_pox_y_offset(0),
	m_sex(sex),
	m_body(body),
	m_hair(hair),
	m_weapon(weapon),
	m_frames_body(nullptr),
	m_frames_hair(nullptr),
	m_frames_weapon(nullptr),
	m_action(Action::Idle),
	m_direction(3),
	m_frame(0),
	m_cmd_action(Action::None),
	m_cmd_dir(-1),
	m_is_main(false)
{
	CreateSprites();

	m_frame_time = GTTime::GetRealTimeSinceStartup();
}

void MirHero::LoadTexture(const std::string &name, Frames **pframes)
{
	auto find = g_frames.find(name);
	if(find != g_frames.end())
	{
		*pframes = &find->second;
		return;
	}

	g_frames[name] = Frames();
	*pframes = &g_frames[name];
	Frames *frames = *pframes;

	int width = -1;
	int height = -1;

	//load info
	std::string name_bytes = Application::GetDataPath() + "/Assets/mir/hero/" + name + ".bytes";
	FILE *file_bytes = fopen(name_bytes.c_str(), "rb");
	if(file_bytes != nullptr)
	{
		fread(&width, 4, 1, file_bytes);
		fread(&height, 4, 1, file_bytes);

		for(int i=0; i<FRAME_COUNT; i++)
        {
			Rect r;
			fread(&r, 16, 1, file_bytes);

            short offset_x;
            short offset_y;
            fread(&offset_x, 2, 1, file_bytes);
			fread(&offset_y, 2, 1, file_bytes);

			frames->frames[i].info.rect = r;
			frames->frames[i].info.offset_x = offset_x;
			frames->frames[i].info.offset_y = offset_y;
        }

		fclose(file_bytes);
	}

	//load texture
	auto bytes_alpha = GTFile::ReadAllBytes(Application::GetDataPath() + "/Assets/mir/hero/" + name + "_alpha.bytes");

	uLongf dest_size = width * height;
	char *dest = (char *) malloc(dest_size);
	int err = uncompress((Bytef *) dest, &dest_size, (Bytef *) &bytes_alpha[0], bytes_alpha.size());
	if(err != Z_OK)
	{
		Debug::Log("unzip mir hero image data error");
	}

	auto tex = Texture2D::Create(width, height, TextureFormat::Alpha8, FilterMode::Point, TextureWrapMode::Clamp);
	tex->SetPixels(dest);
	tex->Apply();

	free(dest);

	//create sprites
	for(int i=0; i<FRAME_COUNT; i++)
	{
		Rect r = frames->frames[i].info.rect;
        r.left *= width;
        r.width *= width;
        r.top *= height;
        r.height *= height;

		r.top = height - r.top - r.height;

		frames->frames[i].sprite = Sprite::Create(tex, r, Vector2(0, 0), 100, Vector4(0, 0, 0, 0));
	}

	frames->tex_w = width;
	frames->tex_h = height;
}

void MirHero::CreateSprites()
{
	m_obj = GameObject::Create("hero");

	m_renderer_body = CreateSprite();
	m_renderer_hair = CreateSprite();
	m_renderer_weapon = CreateSprite();

	int x = m_pox_x * MirMap::TILE_WIDTH;
	int y = m_pox_y * MirMap::TILE_HEIGHT;
	m_obj->GetTransform()->SetPosition(Vector3((float) x, (float) -y, 0) * MIR_PIXEL_TO_UNIT);

	EquipCloth(m_body);
	EquipHair(m_hair);
	EquipWeapon(m_weapon);
}

std::shared_ptr<SpriteRenderer> MirHero::CreateSprite()
{
	std::shared_ptr<SpriteRenderer> s = GameObject::Create("")->AddComponent<SpriteRenderer>();
	s->GetTransform()->SetParent(m_obj->GetTransform());

	auto mat = Material::Create("Mir Sprite");
	mat->SetTexture("ColorTable", MirImage::GetColorTable());
	s->SetSharedMaterial(mat);

	return s;
}

void MirHero::EquipCloth(int id)
{
	m_body = id;

    std::string name = "Hum_" + GTString::ToString(m_body * 2 + m_sex).str;

	LoadTexture(name, &m_frames_body);
	UpdateBodyTexture();
}

void MirHero::EquipHair(int id)
{
	m_hair = id;

	if(id == 0)
	{
		m_renderer_hair->SetSprite(std::shared_ptr<Sprite>());
		return;
	}
    
	std::string name = "Hair_" + GTString::ToString(m_hair * 2 + m_sex).str;

    LoadTexture(name, &m_frames_hair);
	UpdateHairTexture();
}

void MirHero::EquipWeapon(int id)
{
	m_weapon = id;

	if(id == 0)
	{
		m_renderer_weapon->SetSprite(std::shared_ptr<Sprite>());
		return;
	}

	std::string name = "Weapon_" + GTString::ToString(m_weapon * 2 + m_sex).str;

    LoadTexture(name, &m_frames_weapon);
	UpdateWeaponTexture();
}

void MirHero::UpdateBodyTexture()
{
	int index = g_action_frames[(int) m_action].index + g_action_frames[(int) m_action].full * m_direction + m_frame;
	Frame &frame = m_frames_body->frames[index];

	m_renderer_body->SetSprite(frame.sprite);
	m_renderer_body->SetSortingOrder(2, (int) ((m_pox_y + m_pox_y_offset) * 10));
	m_renderer_body->UpdateSprite();
	
	float x = frame.info.offset_x;
	float y = frame.info.offset_y;

	m_renderer_body->GetTransform()->SetLocalPosition(Vector3(x, -y, 0) * MIR_PIXEL_TO_UNIT);
}

void MirHero::UpdateHairTexture()
{
	int index = g_action_frames[(int) m_action].index + g_action_frames[(int) m_action].full * m_direction + m_frame;
	Frame &frame = m_frames_hair->frames[index];

	m_renderer_hair->SetSprite(frame.sprite);
	m_renderer_hair->SetSortingOrder(2, (int) ((m_pox_y + m_pox_y_offset) * 10 + 1));
	m_renderer_hair->UpdateSprite();
	
	float x = frame.info.offset_x;
	float y = frame.info.offset_y;

	m_renderer_hair->GetTransform()->SetLocalPosition(Vector3(x, -y, 0) * MIR_PIXEL_TO_UNIT);
}

void MirHero::UpdateWeaponTexture()
{
	int order;
	if(m_direction > 1 && m_direction < 5)
	{
        order = (int) ((m_pox_y + m_pox_y_offset) * 10 + 2);
    }
    else
    {
        order = (int) ((m_pox_y + m_pox_y_offset) * 10 - 1);
    }

	if(	m_action == Action::Attack ||
		m_action == Action::Attack2 ||
		m_action == Action::Attack3)
	{
		if(m_direction <= 1 || m_direction >= 7)
		{
			if(m_frame < g_action_frames[(int) m_action].length/2)
			{
				order = (int) ((m_pox_y + m_pox_y_offset) * 10 + 2);
			}
		}
	}

	int index = g_action_frames[(int) m_action].index + g_action_frames[(int) m_action].full * m_direction + m_frame;
	Frame &frame = m_frames_weapon->frames[index];

	m_renderer_weapon->SetSprite(frame.sprite);
	m_renderer_weapon->SetSortingOrder(2, order);
	m_renderer_weapon->UpdateSprite();
	
	float x = frame.info.offset_x;
	float y = frame.info.offset_y;

	m_renderer_weapon->GetTransform()->SetLocalPosition(Vector3(x, -y, 0) * MIR_PIXEL_TO_UNIT);
}

void MirHero::UpdateMove(float distance)
{
	float pox_x_offset = g_dirs[m_direction].x * (m_frame + 1) * distance / g_action_frames[(int) m_action].length;
	m_pox_y_offset = g_dirs[m_direction].y * (m_frame + 1) * distance / g_action_frames[(int) m_action].length;

	float x = (m_pox_x + pox_x_offset) * MirMap::TILE_WIDTH;
	float y = (m_pox_y + m_pox_y_offset) * MirMap::TILE_HEIGHT;
	int ix = Mathf::RoundToInt(x);
	int iy = Mathf::RoundToInt(y);

	if(ix % 2 == 1)
	{
		ix++;
	}

	if(iy % 2 == 1)
	{
		iy++;
	}

	m_obj->GetTransform()->SetPosition(Vector3((float) ix, (float) -iy, 0) * MIR_PIXEL_TO_UNIT);
}

void MirHero::Update()
{
	float now = GTTime::GetRealTimeSinceStartup();
	auto &action_frame = g_action_frames[(int) m_action];
	if(now - m_frame_time > action_frame.update_time)
	{
		m_frame_time = now;

		m_frame++;
		if(m_frame >= action_frame.length)
		{
			m_frame = 0;

			OnActionEnd();
		}

		switch(m_action)
		{
		case Action::Run:
			UpdateMove(2.0f);
			break;
		case Action::Walk:
			UpdateMove(1.0f);
			break;
		}

		UpdateBodyTexture();
		UpdateHairTexture();
		UpdateWeaponTexture();
	}

	m_cmd_dir = -1;
	m_cmd_action = Action::None;
}

void MirHero::ChangeAction(Action::Enum action)
{
	m_action = action;
	m_frame = 0;
	m_frame_time = GTTime::GetRealTimeSinceStartup();

	switch(m_action)
	{
	case Action::Run:
		UpdateMove(2.0f);
		break;
	case Action::Walk:
		UpdateMove(1.0f);
		break;
	}

	UpdateBodyTexture();
	UpdateHairTexture();
	UpdateWeaponTexture();
}

void MirHero::ActionRun(int dir)
{
	switch(m_action)
	{
	case Action::Idle:
		m_direction = dir;
		ChangeAction(Action::Run);
		break;
	case Action::Run:
	case Action::Walk:
		m_cmd_dir = dir;
		m_cmd_action = Action::Run;
		break;
	}
}

void MirHero::ActionWalk(int dir)
{
	switch(m_action)
	{
	case Action::Idle:
		m_direction = dir;
		ChangeAction(Action::Walk);
		break;
	case Action::Run:
	case Action::Walk:
		m_cmd_dir = dir;
		m_cmd_action = Action::Walk;
		break;
	}
}

void MirHero::OnActionEnd()
{
	switch(m_action)
	{
	case Action::Run:
		m_pox_x += g_dirs[m_direction].x * 2;
		m_pox_y += g_dirs[m_direction].y * 2;
		m_pox_y_offset = 0;
		if(m_is_main)
		{
			MirMap::Scroll(g_dirs[m_direction].x, g_dirs[m_direction].y, 2);
		}

		if(m_cmd_action == Action::Run)
		{
			m_direction = m_cmd_dir;
		}
		else if(m_cmd_action == Action::Walk)
		{
			m_direction = m_cmd_dir;
			m_action = Action::Walk;
		}
		else
		{
			m_action = Action::Idle;
		}
		break;
	case Action::Walk:
		m_pox_x += g_dirs[m_direction].x;
		m_pox_y += g_dirs[m_direction].y;
		m_pox_y_offset = 0;
		if(m_is_main)
		{
			MirMap::Scroll(g_dirs[m_direction].x, g_dirs[m_direction].y, 1);
		}

		if(m_cmd_action == Action::Walk)
		{
			m_direction = m_cmd_dir;
		}
		else if(m_cmd_action == Action::Run)
		{
			m_direction = m_cmd_dir;
			m_action = Action::Run;
		}
		else
		{
			m_action = Action::Idle;
		}
		break;
	}
}

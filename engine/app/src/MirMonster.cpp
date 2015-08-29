#include "MirMonster.h"
#include "MirMap.h"
#include "GTString.h"
#include "Application.h"
#include "GTFile.h"
#include "GTTime.h"
#include "Debug.h"
#include "zlib.h"
#include <unordered_map>

struct ActionFrame
{
    int index;
    int length;
    int full;
    float update_time;
};

static ActionFrame g_action_frames_360[MirMonster::Action::Count] =
{
	{8 * 0,		4, 10, 0.5f},
	{8 * 10,	6, 10, 0.12f},
	{8 * 20,	6, 10, 0.12f},
	{8 * 30,	2, 2, 0.4f},
	{260,		10, 10, 0.12f},
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

static std::unordered_map<int, int> g_frame_counts;
static std::unordered_map<std::string, MirMonster::Frames> g_frames;

MirMonster::MirMonster(int file, int index, int x, int y, int dir):
	m_file(file),
	m_index(index),
	m_frame_count(0),
	m_pox_x(x),
	m_pox_y(y),
	m_pox_y_offset(0),
	m_frames_body(nullptr),
	m_action(Action::Idle),
	m_direction(dir),
	m_frame(0),
	m_cmd_action(Action::None),
	m_cmd_dir(-1)
{
	if(g_frame_counts.empty())
	{
		g_frame_counts[12] = 360;
	}

	m_frame_count = g_frame_counts[file];

	CreateSprites();

	m_name = "Mon" + GTString::ToString(m_file).str + "_" + GTString::ToString(m_index).str;
	LoadTexture(m_name, &m_frames_body, m_frame_count);
	UpdateBodyTexture();

	m_frame_time = GTTime::GetRealTimeSinceStartup();
}

void MirMonster::Update()
{
	float now = GTTime::GetRealTimeSinceStartup();
	auto &action_frame = g_action_frames_360[(int) m_action];
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
		case Action::Move:
			UpdateMove();
			break;
		}

		UpdateBodyTexture();
	}

	m_cmd_dir = -1;
	m_cmd_action = Action::None;
}

void MirMonster::UpdateMove()
{
	float pox_x_offset = g_dirs[m_direction].x * (m_frame + 1) * 1.0f / g_action_frames_360[(int) m_action].length;
	m_pox_y_offset = g_dirs[m_direction].y * (m_frame + 1) * 1.0f / g_action_frames_360[(int) m_action].length;
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
	auto pos_pixel = Vector2((float) ix, (float) iy);

	m_obj->GetTransform()->SetPosition(Vector3(pos_pixel.x, -pos_pixel.y, 0) * MIR_PIXEL_TO_UNIT);
}

bool MirMonster::OnTouchDown(const Vector2 &pos)
{
	float x = (float) m_pox_x * MirMap::TILE_WIDTH;
	float y = (float) m_pox_y * MirMap::TILE_HEIGHT;
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
	auto pos_pixel = Vector2((float) ix, (float) iy);

	int index = g_action_frames_360[(int) m_action].index + g_action_frames_360[(int) m_action].full * m_direction + m_frame;
	Frame &frame = m_frames_body->frames[index];

	pos_pixel.x += frame.info.offset_x;
	pos_pixel.y += frame.info.offset_y;

	if(	pos.x > pos_pixel.x &&
		pos.y > pos_pixel.y &&
		pos.x < pos_pixel.x + frame.info.rect.width * m_frames_body->tex_w &&
		pos.y < pos_pixel.y + frame.info.rect.height * m_frames_body->tex_h)
	{
		return true;
	}

	return false;
}

void MirMonster::OnActionEnd()
{
	switch(m_action)
	{
	case Action::Move:
		m_pox_x += g_dirs[m_direction].x;
		m_pox_y += g_dirs[m_direction].y;
		m_pox_y_offset = 0;

		m_action = Action::Idle;
		break;
	}
}

void MirMonster::UpdateBodyTexture()
{
	int index = g_action_frames_360[(int) m_action].index + g_action_frames_360[(int) m_action].full * m_direction + m_frame;
	Frame &frame = m_frames_body->frames[index];

	m_renderer_body->SetSprite(frame.sprite);
	m_renderer_body->SetSortingOrder(2, (int) ((m_pox_y + m_pox_y_offset) * 10 - 2));
	
	float x = frame.info.offset_x;
	float y = frame.info.offset_y;

	m_renderer_body->GetTransform()->SetLocalPosition(Vector3(x, -y, 0) * MIR_PIXEL_TO_UNIT);
}

void MirMonster::LoadTexture(const std::string &name, Frames **pframes, int frame_count)
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
	frames->frames.resize(frame_count);

	int width = -1;
	int height = -1;

	//load info
	std::string name_bytes = Application::GetDataPath() + "/Assets/mir/monster/" + name + ".bytes";
	FILE *file_bytes = fopen(name_bytes.c_str(), "rb");
	if(file_bytes != nullptr)
	{
		fread(&width, 4, 1, file_bytes);
		fread(&height, 4, 1, file_bytes);

		for(int i=0; i<frame_count; i++)
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
	auto bytes_alpha = GTFile::ReadAllBytes(Application::GetDataPath() + "/Assets/mir/monster/" + name + "_alpha.bytes");

	uLongf dest_size = width * height;
	char *dest = (char *) malloc(dest_size);
	int err = uncompress((Bytef *) dest, &dest_size, (Bytef *) &bytes_alpha[0], bytes_alpha.size());
	if(err != Z_OK)
	{
		Debug::Log("unzip mir monster image data error");
	}

	auto tex = Texture2D::Create(width, height, TextureFormat::Alpha8, FilterMode::Point, TextureWrapMode::Clamp);
	tex->SetPixels(dest);
	tex->Apply();

	free(dest);

	//create sprites
	for(int i=0; i<frame_count; i++)
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

void MirMonster::CreateSprites()
{
	m_obj = GameObject::Create(m_name);

	m_renderer_body = CreateSprite();

	int x = m_pox_x * MirMap::TILE_WIDTH;
	int y = m_pox_y * MirMap::TILE_HEIGHT;
	m_obj->GetTransform()->SetPosition(Vector3((float) x, (float) -y, 0) * MIR_PIXEL_TO_UNIT);
}

std::shared_ptr<SpriteRenderer> MirMonster::CreateSprite()
{
	std::shared_ptr<SpriteRenderer> s = GameObject::Create("")->AddComponent<SpriteRenderer>();
	s->GetTransform()->SetParent(m_obj->GetTransform());

	auto mat = Material::Create("UI/Mir Sprite");
	mat->SetTexture("ColorTable", MirImage::GetColorTable());
	s->SetSharedMaterial(mat);

	return s;
}
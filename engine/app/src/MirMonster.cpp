#include "MirMonster.h"
#include "MirMap.h"
#include "GTString.h"
#include <unordered_map>

static std::unordered_map<int, int> g_frame_counts;

MirMonster::MirMonster(int file, int index, int x, int y, int dir):
	m_file(file),
	m_index(index),
	m_frame_count(0),
	m_pox_x(x),
	m_pox_y(y),
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

	CreateSprites();

	m_name = "Mon" + GTString::ToString(m_file).str + "_" + GTString::ToString(m_index).str;
	//LoadTexture(name, &m_frames_body);
	//UpdateBodyTexture();

	m_frame_time = GTTime::GetRealTimeSinceStartup();
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

	auto mat = Material::Create("Mir Sprite");
	mat->SetTexture("ColorTable", MirImage::GetColorTable());
	s->SetSharedMaterial(mat);

	return s;
}
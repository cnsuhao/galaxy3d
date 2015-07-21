#include "MirHero.h"
#include "MirMap.h"
#include "GTString.h"

struct ImageInfo
{
    Rect rect;
    short offset_x;
    short offset_y;
};

static std::shared_ptr<Texture2D> LoadTexture(const std::string &name, ImageInfo *infos)
{
	static std::shared_ptr<Texture2D> tex;

	return tex;
}

MirHero::MirHero(int x, int y, int body, int hair, int weapon, int sex):
	m_pox_x(x),
	m_pox_y(y),
	m_sex(sex),
	m_body(body),
	m_hair(hair),
	m_weapon(weapon)
{
	CreateSprites();
}

void MirHero::CreateSprites()
{
	m_obj = GameObject::Create("hero");

	m_body_renderer = CreateSprite();
	m_hair_renderer = CreateSprite();
	m_weapon_renderer = CreateSprite();

	float x = m_pox_x * MirMap::TILE_WIDTH * MIR_PIXEL_TO_UNIT;
	float y = (m_pox_y - 1) * MirMap::TILE_HEIGHT * MIR_PIXEL_TO_UNIT;
	m_obj->GetTransform()->SetPosition(Vector3(x, -y, 0));

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
	ImageInfo infos[FRAME_COUNT];

    auto tex = LoadTexture(name, infos);
}

void MirHero::EquipHair(int id)
{
	m_hair = id;

	if(id == 0)
	{
		m_hair_renderer->SetSprite(std::shared_ptr<Sprite>());
		return;
	}
    
	std::string name = "Hair_" + GTString::ToString(m_hair * 2 + m_sex).str;
	ImageInfo infos[FRAME_COUNT];

    LoadTexture(name, infos);
}

void MirHero::EquipWeapon(int id)
{
	m_weapon = id;

	if(id == 0)
	{
		m_weapon_renderer->SetSprite(std::shared_ptr<Sprite>());
		return;
	}

	std::string name = "Weapon_" + GTString::ToString(m_hair * 2 + m_sex).str;
	ImageInfo infos[FRAME_COUNT];

    LoadTexture(name, infos);
}

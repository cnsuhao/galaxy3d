#ifndef __MirHero_h__
#define __MirHero_h__

#include "GameObject.h"
#include "SpriteRenderer.h"

using namespace Galaxy3D;

class MirHero
{
public:
	static const int FRAME_COUNT = 600;
    static const int CLOTH_COUNT = 12;
	static const int HAIR_COUNT = 3;
    static const int WEAPON_COUNT = 38;

	MirHero(int x, int y, int body, int hair, int weapon, int sex = 0);
	void EquipCloth(int id);
	void EquipHair(int id);
	void EquipWeapon(int id);

private:
	int m_pox_x;
	int m_pox_y;
	int m_sex;//0:boy, 1:girl
	int m_body;
	int m_hair;
	int m_weapon;
	std::shared_ptr<GameObject> m_obj;
	std::shared_ptr<SpriteRenderer> m_body_renderer;
	std::shared_ptr<SpriteRenderer> m_hair_renderer;
	std::shared_ptr<SpriteRenderer> m_weapon_renderer;

	void CreateSprites();
	std::shared_ptr<SpriteRenderer> CreateSprite();
};

#endif
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

	struct ImageInfo
	{
		Rect rect;
		short offset_x;
		short offset_y;
	};

	struct Frame
	{
		ImageInfo info;
		std::shared_ptr<Sprite> sprite;
	};

	struct Frames
	{
		Frame frames[FRAME_COUNT];
		int tex_w;
		int tex_h;
	};

	struct Action
	{
		enum Enum
		{
			Idle,
			Walk,
			Run,
			ReadyBattle,
			Attack,
			Attack2,
			Attack3,
			Cast,
			Gather,
			Hit,
			Die,

			Count,

			None,
		};	
	};

	MirHero(int x, int y, int body, int hair, int weapon, int sex = 0);
	void EquipCloth(int id);
	void EquipHair(int id);//boy:2 girl:1,2
	void EquipWeapon(int id);
	std::shared_ptr<GameObject> GetGameObject() const {return m_obj;}
	void Update();
	void ActionRun(int dir);
	void ActionWalk(int dir);

private:
	int m_pox_x;
	int m_pox_y;
	float m_pox_y_offset;
	int m_sex;//0:boy, 1:girl
	int m_body;
	int m_hair;
	int m_weapon;
	std::shared_ptr<GameObject> m_obj;
	std::shared_ptr<SpriteRenderer> m_renderer_body;
	std::shared_ptr<SpriteRenderer> m_renderer_hair;
	std::shared_ptr<SpriteRenderer> m_renderer_weapon;
	Frames *m_frames_body;
	Frames *m_frames_hair;
	Frames *m_frames_weapon;
	Action::Enum m_action;
	int m_direction;
	int m_frame;
	float m_frame_time;
	Action::Enum m_cmd_action;
	int m_cmd_dir;

	static void LoadTexture(const std::string &name, Frames **frames);
	void CreateSprites();
	std::shared_ptr<SpriteRenderer> CreateSprite();
	void UpdateBodyTexture();
	void UpdateHairTexture();
	void UpdateWeaponTexture();
	void OnActionEnd();
	void UpdateMove(float distance);
	void ChangeAction(Action::Enum action);
};

#endif
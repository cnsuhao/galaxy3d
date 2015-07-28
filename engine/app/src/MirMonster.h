#ifndef __MirMonster_h__
#define __MirMonster_h__

#include "GameObject.h"
#include "SpriteRenderer.h"

using namespace Galaxy3D;

class MirMonster
{
public:
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
		std::vector<Frame> frames;
		int tex_w;
		int tex_h;
	};

	struct Action
	{
		enum Enum
		{
			Idle,
			Move,
			Attack,
			Hit,
			Die,

			Count,

			None,
		};	
	};

	MirMonster(int file, int index, int x, int y, int dir);
	void Update();
	bool OnTouchDown(const Vector2 &pos);
	Vector2 GetPos() const {return Vector2((float) m_pox_x, (float) m_pox_y);}

private:
	int m_file;
	int m_index;
	std::string m_name;
	int m_frame_count;
	int m_pox_x;
	int m_pox_y;
	float m_pox_y_offset;
	std::shared_ptr<GameObject> m_obj;
	std::shared_ptr<SpriteRenderer> m_renderer_body;
	Frames *m_frames_body;
	Action::Enum m_action;
	int m_direction;
	int m_frame;
	float m_frame_time;
	Action::Enum m_cmd_action;
	int m_cmd_dir;

	static void LoadTexture(const std::string &name, Frames **frames, int file);
	void CreateSprites();
	std::shared_ptr<SpriteRenderer> CreateSprite();
	void UpdateBodyTexture();
	void OnActionEnd();
	void UpdateMove();
};

#endif
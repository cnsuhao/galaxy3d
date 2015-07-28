#ifndef __Launcher_h__
#define __Launcher_h__

#include "Component.h"
#include "Application.h"
#include "Texture2D.h"
#include "Screen.h"

#include "SpriteRenderer.h"
#include "SpriteNode.h"
#include "SpriteBatchRenderer.h"
#include "Label.h"
#include "TextRenderer.h"
#include "GTString.h"
#include "GTTime.h"
#include "Input.h"
#include "Debug.h"

#include "MirMap.h"
#include "MirHero.h"
#include "MirMonster.h"

namespace Galaxy3D
{
	class Launcher :public Component
	{
	public:
		virtual ~Launcher();

	protected:
		std::shared_ptr<TextRenderer> fps;
		std::shared_ptr<Camera> camera;
		std::shared_ptr<MirHero> hero;
		std::shared_ptr<MirHero> hero2;
		std::shared_ptr<MirHero> hero3;
		std::shared_ptr<MirMonster> mon;
		bool touch_down;
		Vector2 touch_pos;

		virtual void Start();
		virtual void Update();
		bool OnTouchDown(const Vector2 &pos);
	};
}

#endif
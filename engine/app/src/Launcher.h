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

namespace Galaxy3D
{
	class Launcher :public Component
	{
	protected:
		virtual void Start();
		virtual void Update();
	};
}

#endif
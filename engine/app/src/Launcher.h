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
#include "GTFile.h"
#include "Input.h"
#include "Debug.h"
#include "Mesh.h"
#include "Layer.h"
#include "LayerMask.h"
#include "Animation.h"
#include "Terrain.h"
#include "TerrainRenderer.h"
#include "LightmapSettings.h"
#include "RenderSettings.h"
#include "Physics.h"

namespace Galaxy3D
{
	class Launcher :public Component
	{
	public:
		virtual ~Launcher();

	protected:
		std::shared_ptr<TextRenderer> fps;
		std::shared_ptr<Camera> cam2d;
        std::shared_ptr<Camera> cam3d;
        std::shared_ptr<Animation> anim;
        float time_move_begin;
        float time_move_end;
        Vector3 pos_old;
        Vector3 pos_new;

		virtual void Start();
		virtual void Update();
	};
}

#endif
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
#include "TweenPosition.h"
#include "TweenRotation.h"
#include "ParticleSystem.h"
#include "ParticleSystemRenderer.h"

namespace Galaxy3D
{
	class Launcher :public Component
	{
	public:
        static void OnTweenSetValue(Component *tween, std::weak_ptr<Component> &target, void *value);
        static void OnTweenFinished(Component *tween, std::weak_ptr<Component> &target);
		virtual ~Launcher();

	protected:
		std::shared_ptr<TextRenderer> fps;
		std::shared_ptr<Camera> cam2d;
        std::shared_ptr<Camera> cam3d;
        Vector3 cam_offset;
        std::shared_ptr<Animation> anim;

		virtual void Start();
		virtual void Update();
        virtual void LateUpdate();
	};
}

#endif
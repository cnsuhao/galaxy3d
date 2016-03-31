LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE				:= libViry3D
LOCAL_CFLAGS				:= -Wall -DANDROID -DIOAPI_NO_64 -DFPM_ARM
LOCAL_CPPFLAGS				:= -std=c++11 -fexceptions -frtti

LOCAL_C_INCLUDES			:= \
	../../src \
	../../src/gles2 \
	../../depend/openal-soft-winphone-master/include \
	../../depend/bullet3-2.83.6/src \
	../../depend/json/src/json \
	../../depend/freetype-2.6/include \
	../../depend/zlib-1.2.8 \
	../../depend/mad

LOCAL_SRC_FILES			:= \
	../../depend/zlib-1.2.8/contrib/minizip/ioapi.c \
	../../depend/zlib-1.2.8/contrib/minizip/zip.c \
	../../depend/zlib-1.2.8/contrib/minizip/unzip.c \
	../../src/gles2/GraphicsDevice.cpp \
	../../src/gles2/Material.cpp \
	../../src/gles2/RenderTexture.cpp \
	../../src/gles2/RenderStates.cpp \
	../../src/gles2/Shader.cpp \
	../../src/gles2/Texture2D.cpp \
    ../../src/Animation.cpp \
    ../../src/AnimationCurve.cpp \
    ../../src/Application.cpp \
    ../../src/AudioClip.cpp \
    ../../src/AudioListener.cpp \
    ../../src/AudioManager.cpp \
    ../../src/AudioSource.cpp \
    ../../src/Bounds.cpp \
    ../../src/BoxCollider.cpp \
    ../../src/Camera.cpp \
    ../../src/CharacterController.cpp \
    ../../src/Collider.cpp \
    ../../src/Color.cpp \
    ../../src/Component.cpp \
    ../../src/Cursor.cpp \
    ../../src/Debug.cpp \
    ../../src/FrustumBounds.cpp \
    ../../src/GameObject.cpp \
    ../../src/GTFile.cpp \
    ../../src/GTString.cpp \
    ../../src/GTStringUTF32.cpp \
    ../../src/GTTime.cpp \
    ../../src/GTUIManager.cpp \
    ../../src/Guid.cpp \
    ../../src/HighlightingObject.cpp \
    ../../src/Image.cpp \
    ../../src/ImageEffect.cpp \
    ../../src/ImageEffectBloom.cpp \
    ../../src/ImageEffectGlobalFog.cpp \
    ../../src/ImageEffectHighlighting.cpp \
    ../../src/ImageEffectToneMapping.cpp \
    ../../src/Input.cpp \
    ../../src/Label.cpp \
	../../src/LabelBatchRenderer.cpp \
	../../src/LabelNode.cpp \
    ../../src/Light.cpp \
    ../../src/LightmapSettings.cpp \
    ../../src/Localization.cpp \
    ../../src/Mathf.cpp \
    ../../src/Matrix4x4.cpp \
    ../../src/Mesh.cpp \
    ../../src/MeshCollider.cpp \
    ../../src/MeshRenderer.cpp \
    ../../src/NavMesh.cpp \
    ../../src/NavMeshAgent.cpp \
    ../../src/Object.cpp \
    ../../src/Octree.cpp \
    ../../src/ParticleSystem.cpp \
    ../../src/ParticleSystemRenderer.cpp \
    ../../src/Physics.cpp \
    ../../src/Quaternion.cpp \
    ../../src/Ray.cpp \
    ../../src/Rect.cpp \
    ../../src/Renderer.cpp \
    ../../src/RenderSettings.cpp \
    ../../src/Screen.cpp \
    ../../src/SkinnedMeshRenderer.cpp \
    ../../src/SkyBox.cpp \
    ../../src/Sprite.cpp \
    ../../src/SpriteBatchRenderer.cpp \
    ../../src/SpriteNode.cpp \
    ../../src/SpriteRenderer.cpp \
    ../../src/Terrain.cpp \
    ../../src/TerrainCollider.cpp \
    ../../src/TerrainRenderer.cpp \
    ../../src/TextRenderer.cpp \
    ../../src/Transform.cpp \
    ../../src/TweenColor.cpp \
    ../../src/Tweenner.cpp \
    ../../src/TweenPosition.cpp \
    ../../src/TweenRotation.cpp \
    ../../src/TweenScale.cpp \
    ../../src/UIAtlas.cpp \
    ../../src/UICanvas.cpp \
    ../../src/UIScrollBar.cpp \
    ../../src/UIScrollView.cpp \
    ../../src/UISelectView.cpp \
    ../../src/UISlider.cpp \
    ../../src/UIToggle.cpp \
    ../../src/Vector2.cpp \
    ../../src/Vector3.cpp \
    ../../src/World.cpp

include $(BUILD_STATIC_LIBRARY)

#include <jni.h>
#include <GLES2/gl2.h>

#include "Screen.h"
#include "GraphicsDevice.h"
#include "World.h"
#include "Camera.h"
#include "Debug.h"
#include "Input.h"
#include "GTFile.h"
#include "Application.h"
#include "../../src/LauncherMerged.h"
#include <vector>
#include <list>

using namespace Galaxy3D;

extern std::vector<Touch> g_input_touches;
extern std::list<Touch> g_input_touch_buffer;
extern bool g_key_down[KeyCode::COUNT];
extern bool g_key[KeyCode::COUNT];
extern bool g_key_up[KeyCode::COUNT];
extern bool g_key_held[KeyCode::COUNT];
extern bool g_mouse_button_down[3];
extern bool g_mouse_button_up[3];
extern Vector3 g_mouse_position;
extern bool g_mouse_button_held[3];

static std::string call_java_string_method(JNIEnv *env, jobject thiz, const std::string &method_name)
{
	auto clazz = env->GetObjectClass(thiz);
	auto method = env->GetMethodID(clazz, method_name.c_str(), "()Ljava/lang/String;");
	jstring java_string = (jstring) env->CallObjectMethod(thiz, method);
	const char *cstring = env->GetStringUTFChars(java_string, NULL);
	std::string result = cstring;
	env->ReleaseStringUTFChars(java_string, cstring);

	env->DeleteLocalRef(java_string);
	env->DeleteLocalRef(clazz);

	return result;
}

static int native_on_start(JNIEnv *env, jobject thiz, int w, int h)
{
	auto package_path = call_java_string_method(env, thiz, "GetPackagePath");
	auto files_dir_path = call_java_string_method(env, thiz, "GetFilesDirPath");

	Debug::Log("get package_path:%s", package_path.c_str());
	Debug::Log("get files_dir_path:%s", files_dir_path.c_str());
	
	Application::SetDataPath(files_dir_path);

	GTFile::Unzip(package_path, "assets/Assets", Application::GetDataPath() + "/Assets", true);

    Screen::SetSize(w, h);
	GraphicsDevice::GetInstance()->Init(0);
	World::Init();

	GameObject::Create("")->AddComponent<LauncherMerged>();

	return 0;
}

static int native_on_update(JNIEnv *env, jobject thiz)
{
	World::Update();
	Camera::RenderAll();

	return 0;
}

static int native_on_destroy(JNIEnv *env, jobject thiz)
{
	World::Done();
    GraphicsDevice::Done();

	return 0;
}

static int native_on_pause(JNIEnv *env, jobject thiz)
{
	Debug::Log("native_on_pause");

	World::OnPause();

	return 0;
}

static int native_on_resume(JNIEnv *env, jobject thiz)
{
	Debug::Log("native_on_resume");

	World::OnResume();

	return 0;
}
/*
static int GetKey(int key_code)
{
	int key = -1;

	if(key_code == 82)//KEYCODE_MENU
	{
		key = KeyCode_Menu;
	}
	else if(key_code == 4)//KEYCODE_BACK
	{
		key = KeyCode_Backspace;
	}
	else if(key_code == 24)//KEYCODE_VOLUME_UP
	{
		key = KeyCode_PageUp;
	}
	else if(key_code == 25)//KEYCODE_VOLUME_DOWN
	{
		key = KeyCode_PageDown;
	}

	return key;
}

static int on_key_event(int key_code, bool down)
{
	int key = GetKey(key_code);
	if(key >= 0)
	{
		if(down)
		{
			if(!Input::m_key_held[key])
			{
				Input::m_key_down[key] = true;
				Input::m_key_held[key] = true;
			}
		}
		else
		{
			Input::m_key_up[key] = true;
			Input::m_key_held[key] = false;
			Input::m_key[key] = false;
		}
	}

	return -1;
}
*/
static int native_on_keyup(JNIEnv *env, jobject thiz, int key_code)
{
	//on_key_event(key_code, false);
	return 0;
}

static int native_on_keydown(JNIEnv *env, jobject thiz, int key_code)
{
	//on_key_event(key_code, true);
	return 0;
}

#pragma pack(push)
#pragma pack(4)
struct TouchEvent
{
	int act;
	int index;
	int count;
	long long time;
	float xys[20];
};
#pragma pack(pop)

enum MotionEvent
{
	ACTION_DOWN = 0,
	ACTION_UP = 1,
	ACTION_MOVE = 2,
	ACTION_CANCEL = 3,
	ACTION_POINTER_DOWN = 5,
	ACTION_POINTER_UP = 6,
};

static TouchEvent g_touch;

static void swap_bytes(void *data, int size)
{
	char *p = (char *) data;
	int half = size >> 1;
	for(int i=0; i<half; i++)
	{
		char t = p[i];
		p[i] = p[size - 1 - i];
		p[size - 1 - i] = t;
	}
}

static void touch_begin(void *event)
{
	TouchEvent *t = (TouchEvent *) event;

	float x = t->xys[t->index * 2];
	float y = t->xys[t->index * 2 + 1];

	Touch touch;
    touch.deltaPosition = Vector2(0, 0);
    touch.deltaTime = 0;
    touch.time = t->time / 1000.0f;
    touch.fingerId = t->index;
    touch.phase = TouchPhase::Began;
    touch.tapCount = t->count;
    touch.position = Vector2(x, y);

	if(!g_input_touches.empty())
	{
		g_input_touch_buffer.push_back(touch);
	}
	else
	{
		g_input_touches.push_back(touch);
	}

	if(touch.fingerId == 0)
    {
		g_mouse_button_down[0] = true;
        g_mouse_position.x = (float) x;
        g_mouse_position.y = (float) y;
        g_mouse_button_held[0] = true;

		Debug::Log("down");
    }
}

static void touch_update(void *event)
{
	TouchEvent *t = (TouchEvent *) event;

	float x = t->xys[t->index * 2];
	float y = t->xys[t->index * 2 + 1];

	Touch touch;
    touch.deltaPosition = Vector2(0, 0);
    touch.deltaTime = 0;
    touch.time = t->time / 1000.0f;
    touch.fingerId = t->index;
    touch.tapCount = t->count;
    touch.position = Vector2(x, y);

	if(t->act == ACTION_MOVE)
	{
		touch.phase = TouchPhase::Moved;
	}
	else
	{
		touch.phase = TouchPhase::Ended;
	}

	if(!g_input_touches.empty())
	{
		g_input_touch_buffer.push_back(touch);
	}
	else
	{
		g_input_touches.push_back(touch);
	}

    if(touch.fingerId == 0)
    {
        if(touch.phase == TouchPhase::Ended || touch.phase == TouchPhase::Canceled)
        {
            g_mouse_button_up[0] = true;
			g_mouse_position.x = (float) x;
			g_mouse_position.y = (float) y;
			g_mouse_button_held[0] = false;
                        
            Debug::Log("up");
        }
        else if(touch.phase == TouchPhase::Moved)
        {
            g_mouse_position.x = (float) x;
			g_mouse_position.y = (float) y;
        }
    }
}

static int native_on_touch(JNIEnv *env, jobject thiz, jbyteArray data)
{
	int size = env->GetArrayLength(data);

	env->GetByteArrayRegion(data, 0, size, (jbyte *) &g_touch);

	swap_bytes(&g_touch.act, sizeof(int));
	swap_bytes(&g_touch.index, sizeof(int));
	swap_bytes(&g_touch.count, sizeof(int));
	swap_bytes(&g_touch.time, sizeof(long long));

	for(int i=0; i<g_touch.count; i++)
	{
		swap_bytes(&g_touch.xys[i*2], sizeof(float));
		swap_bytes(&g_touch.xys[i*2 + 1], sizeof(float));

		g_touch.xys[i*2 + 1] = Screen::GetHeight() - 1 - g_touch.xys[i*2 + 1];
	}

	MotionEvent event = (MotionEvent) g_touch.act;
	switch(event)
	{
	case ACTION_DOWN:
	case ACTION_POINTER_DOWN:
		touch_begin(&g_touch);
		break;

	case ACTION_MOVE:
	case ACTION_UP:
	case ACTION_POINTER_UP:
	case ACTION_CANCEL:
		touch_update(&g_touch);
		break;
	}
	
	return 0;
}

extern "C" {

JNIEXPORT
jint
JNICALL
Java_com_virytech_viry3d_GLView_nativeOnStart(JNIEnv *env, jobject thiz,
	jint width, jint height)
{
	return native_on_start(env, thiz, width, height);
}

JNIEXPORT
jint
JNICALL
Java_com_virytech_viry3d_GLView_nativeOnUpdate(JNIEnv *env, jobject thiz)
{
	return native_on_update(env, thiz);
}

JNIEXPORT
jint
JNICALL
Java_com_virytech_viry3d_GLView_nativeOnDestroy(JNIEnv *env, jobject thiz)
{
	return native_on_destroy(env, thiz);
}

JNIEXPORT
jint
JNICALL
Java_com_virytech_viry3d_GLView_nativeOnPause(JNIEnv *env, jobject thiz)
{
	return native_on_pause(env, thiz);
}

JNIEXPORT
jint
JNICALL
Java_com_virytech_viry3d_GLView_nativeOnResume(JNIEnv *env, jobject thiz)
{
	return native_on_resume(env, thiz);
}

JNIEXPORT
jint
JNICALL
Java_com_virytech_viry3d_GLView_nativeOnKeyUp(JNIEnv *env, jobject thiz, jint key_code)
{
	return native_on_keyup(env, thiz, key_code);
}

JNIEXPORT
jint
JNICALL
Java_com_virytech_viry3d_GLView_nativeOnKeyDown(JNIEnv *env, jobject thiz, jint key_code)
{
	return native_on_keydown(env, thiz, key_code);
}

JNIEXPORT
jint
JNICALL
Java_com_virytech_viry3d_GLView_nativeOnTouch(JNIEnv *env, jobject thiz, jbyteArray data)
{
	return native_on_touch(env, thiz, data);
}

}
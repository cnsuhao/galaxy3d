#ifndef __GTTime_h__
#define __GTTime_h__

namespace Galaxy3D
{
	class GTTime
	{
		friend class World;
		friend class Camera;
		friend class Renderer;

	public:
		static int GetFrameCount() {return m_frame_count;}
        static float GetTime();//calls have same time value in same frame
		static float GetRealTimeSinceStartup();
		static float GetDeltaTime() {return m_time_delta;}
		static long long GetTimeMS();
        static int GetFPS() {return m_fps;}
        static int GetDrawCall() {return m_draw_call;}

	private:
		static long long m_time_startup;
		static float m_time_delta;
		static float m_time_record;
        static float m_time;
		static int m_frame_count;
		static int m_frame_record;
		static int m_fps;
		static float m_render_time;
		static float m_update_time;
		static int m_draw_call;
	};
}

#endif
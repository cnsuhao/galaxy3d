#ifndef __GTTime_h__
#define __GTTime_h__

namespace Galaxy3D
{
	class GTTime
	{
		friend class World;
		friend class Camera;
		friend class Renderer;
		friend class Launcher;

	public:
		static int GetFrameCount(){return m_frame_count;}
		static float GetRealTimeSinceStartup();
		static float GetDeltaTime(){return m_time_delta;}
		static long long GetTimeMS();

	private:
		static long long m_time_startup;
		static long long m_time_update;
		static float m_time_delta;
		static float m_time_record;
		static int m_frame_count;
		static int m_frame_record;
		static int m_fps;
		static float m_render_time;
		static float m_update_time;
		static int m_draw_call; 
	};
}

#endif
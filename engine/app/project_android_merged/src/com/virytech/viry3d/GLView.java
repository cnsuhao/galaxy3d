package com.virytech.viry3d;

import java.io.File;
import java.nio.ByteBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;

public class GLView extends GLSurfaceView
{
	static
    {
        System.loadLibrary("Game");
    }
	
	GLRenderer m_renderer;
	final String m_package_path;
	final String m_files_dir_path;
	
	public String GetPackagePath()
	{
		return m_package_path;
	}
	
	public String GetFilesDirPath()
	{
		return m_files_dir_path;
	}
	
	public GLView(Context context)
	{
		super(context);
		
		m_package_path = context.getPackageResourcePath();
        File files_dir = context.getExternalFilesDir(null);
        if(files_dir == null)
        {
        	files_dir = context.getFilesDir();
        }
        m_files_dir_path = files_dir.getAbsolutePath();
		
		setEGLContextClientVersion(2);
		m_renderer = new GLRenderer();
        setRenderer(m_renderer);
	}
	
	public void onDestroy() {
		queueEvent(
				new Runnable()
				{
					public void run()
					{
						nativeOnDestroy();
					}
				}
			);
	}
	
	public void onPause() {
		queueEvent(
				new Runnable()
				{
					public void run()
					{
						nativeOnPause();
					}
				}
			);
	}
	
	public void onResume() {
		queueEvent(
				new Runnable()
				{
					public void run()
					{
						nativeOnResume();
					}
				}
			);
	}
	
	public boolean onKeyUp(final int keyCode, KeyEvent event)
	{
		queueEvent(
				new Runnable()
				{
					public void run()
					{
						nativeOnKeyUp(keyCode);
					}
				}
			);

		return false;
	}
	
	public boolean onKeyDown(final int keyCode, KeyEvent event)
	{
		queueEvent(
				new Runnable()
				{
					public void run()
					{
						nativeOnKeyDown(keyCode);
					}
				}
			);
		
		return false;
	}
	
	class TouchEvent
	{
		public int act;
		public int index;
		public int count;
		public long time;
		public float[] xys;
	}
	
	TouchEvent g_touch;
	
	public boolean onTouch(MotionEvent event)
	{
		int action = event.getAction();
		int count = event.getPointerCount();
		int index = (action & 0xff00) >> 8;
		long time = event.getEventTime();
		int act = action & 0xff;
		
		if(count <= 10)
		{
			if(g_touch == null)
			{
				g_touch = new TouchEvent();
				g_touch.xys = new float[20];
			}
			g_touch.act = act;
			g_touch.index = index;
			g_touch.count = count;
			g_touch.time = time;
			for(int i=0; i<count; i++)
			{
				g_touch.xys[i*2] = event.getX(i);
				g_touch.xys[i*2+1] = event.getY(i);
			}
			
			final ByteBuffer buffer = ByteBuffer.allocate(4 + 4 + 4 + 8 + 8 * 10);
			buffer.putInt(g_touch.act);
			buffer.putInt(g_touch.index);
			buffer.putInt(g_touch.count);
			buffer.putLong(g_touch.time);
			for(int i=0; i<count; i++)
			{
				buffer.putFloat(g_touch.xys[i*2]);
				buffer.putFloat(g_touch.xys[i*2+1]);
			}
			
			queueEvent(
					new Runnable()
					{
						public void run()
						{
							nativeOnTouch(buffer.array());
						}
					}
				);
		}
		
		switch(act)
		{
		case MotionEvent.ACTION_DOWN:
			//Log.e("Galaxy3D onTouchEvent", "ACTION_DOWN index:" + index + " count:" + count + " time:" + time);
			break;
		case MotionEvent.ACTION_POINTER_DOWN:
			//Log.e("Galaxy3D onTouchEvent", "ACTION_POINTER_DOWN index:" + index + " count:" + count + " time:" + time);
			break;
		case MotionEvent.ACTION_MOVE:
			//Log.e("Galaxy3D onTouchEvent", "ACTION_MOVE index:" + index + " count:" + count + " time:" + time);
			break;
		case MotionEvent.ACTION_UP:
			//Log.e("Galaxy3D onTouchEvent", "ACTION_UP index:" + index + " count:" + count + " time:" + time);
			break;
		case MotionEvent.ACTION_POINTER_UP:
			//Log.e("Galaxy3D onTouchEvent", "ACTION_POINTER_UP index:" + index + " count:" + count + " time:" + time);
			break;
		case MotionEvent.ACTION_CANCEL:
			//Log.e("Galaxy3D onTouchEvent", "ACTION_CANCEL index:" + index + " count:" + count + " time:" + time);
			break;
		default:
		    Log.e("Galaxy3D onTouchEvent", "ACTION_UNKNOWN:" + act + " index:" + index + " count:" + count + " time:" + time);
		    break;
		}
		
		return true;
	}
	
	protected void onSizeChanged(int w, int h, int oldw, int oldh) {
    	m_renderer.width = w;
    	m_renderer.height = h;
    }
	
	class GLRenderer implements GLSurfaceView.Renderer
    {
		public int width;
		public int height;
		boolean m_init = false;
		
		public GLRenderer()
		{
		}
		
        public void onDrawFrame(GL10 gl) {
        	nativeOnUpdate();
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        	if(!m_init)
        	{
        		m_init = true;
        		nativeOnStart(width, height);
        	}
        }
    }
	
	native int nativeOnStart(int w, int h);
	native int nativeOnUpdate();
	native int nativeOnDestroy();
	native int nativeOnPause();
	native int nativeOnResume();
	native int nativeOnKeyUp(int key_code);
	native int nativeOnKeyDown(int key_code);
	native int nativeOnTouch(byte[] data);
}
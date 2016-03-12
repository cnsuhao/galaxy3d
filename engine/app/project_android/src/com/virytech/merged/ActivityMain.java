package com.virytech.merged;

import com.virytech.viry3d.GLView;

import android.app.Activity;
import android.graphics.PixelFormat;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Window;
import android.view.WindowManager;

public class ActivityMain extends Activity
{
	GLView m_player;
	
	@Override protected void onCreate(Bundle savedInstanceState)
	{
		requestWindowFeature(Window.FEATURE_NO_TITLE);
        super.onCreate(savedInstanceState);
        
        getWindow().takeSurface(null);
		setTheme(android.R.style.Theme_NoTitleBar_Fullscreen);
		getWindow().setFormat(PixelFormat.RGBX_8888);
		getWindow().setFlags (WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

        m_player = new GLView(this);
        
        setContentView(m_player);
    }
    
    @Override protected void onDestroy()
	{
    	m_player.onDestroy();
		super.onDestroy();
	}

	@Override protected void onPause()
	{
		super.onPause();
		m_player.onPause();
	}

	@Override protected void onResume()
	{
		super.onResume();
		m_player.onResume();
	}

	@Override public boolean onKeyUp(int keyCode, KeyEvent event)     { return m_player.onKeyUp(keyCode, event); }
	@Override public boolean onKeyDown(int keyCode, KeyEvent event)   { return m_player.onKeyDown(keyCode, event); }
	@Override public boolean onTouchEvent(MotionEvent event)          { return m_player.onTouch(event); }
	@Override public boolean onGenericMotionEvent(MotionEvent event)  { return false; }
}

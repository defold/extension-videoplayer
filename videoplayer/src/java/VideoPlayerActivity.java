package com.defold.android.videoplayer;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;


import android.view.TextureView;

public class VideoPlayerActivity extends Activity {

	public TextureView textureView;

	public static VideoPlayerActivity instance = null;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		VideoPlayerActivity.instance = this;

		//Log.i("VideoPlayerActivity", "onCreate(): MAWE HELLO!");
		// setContentView(R.layout.main);
		// LinearLayout root=(LinearLayout) findViewById(R.id.root);
		// (...)

		Log.i("VideoPlayerActivity", "new TextureView");
        textureView = new TextureView(this);
	}

	@Override
    protected void onDestroy()
    {
		Log.i("VideoPlayerActivity", "onDestroy(): MAWE HELLO!");
    }
}

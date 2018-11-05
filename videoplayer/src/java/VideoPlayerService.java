package com.defold.android.videoplayer;
import android.app.IntentService;
import android.content.Intent;
import android.util.Log;


import android.view.TextureView;

public class VideoPlayerService extends IntentService {
	public TextureView textureView;

    public VideoPlayerService() {
    	super("VideoPlayerService");
		Log.i("VideoPlayerService", "VideoPlayerService()");
    }

    @Override
    protected void onHandleIntent(Intent workIntent) {
        // Gets data from the incoming Intent
        String dataString = workIntent.getDataString();

		Log.i("VideoPlayerService", "dataString: " + dataString);

		Log.i("VideoPlayerService", "new TextureView");
        textureView = new TextureView(this);
    }
}

package com.defold.android.videoplayer;

import android.content.Context;
import android.util.Log;


class VideoplayerExtension {
    public static final String TAG = "defold-videoplayer";

    private static void LOG(String message) {
        Log.v("defold-videoplayer", message);
    }

    // public static void Init(final Context context) {
    //     LOG("VideoplayerExtension: Init()");
    //     MovieSurfaceTexture.init(context);
    // }

    public static MovieSurfaceTexture Open(final Context context, String uri, int id) {
        LOG("VideoplayerExtension: Opening video");
        return new MovieSurfaceTexture(context, uri, id);
    }
    public static void Close(MovieSurfaceTexture movie) {
        LOG("VideoplayerExtension: Closing video");
        movie.close();
    }
    public static void Update(MovieSurfaceTexture movie, float dt) {
        //LOG("VideoplayerExtension: Updating video " + Float.toString(dt));
        movie.update();
    }
    public static void Start(MovieSurfaceTexture movie, int texture) {
        LOG("VideoplayerExtension: Starting video");
        movie.start(texture);
    }
    public static void Stop(MovieSurfaceTexture movie) {
        LOG("VideoplayerExtension: Stopping video");
        movie.stop();
    }
    public static void Pause(MovieSurfaceTexture movie) {
        LOG("VideoplayerExtension: Pausing video");
        movie.pause();
    }
    public static void SetupSurface(MovieSurfaceTexture movie, int texture) {
        LOG("VideoplayerExtension: Starting video");
        movie.setupSurface(texture);
    }
}

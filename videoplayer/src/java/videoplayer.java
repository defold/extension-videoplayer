package com.defold.android.videoplayer;

import android.content.Context;
import android.util.Log;


class VideoplayerExtension {
    public static final String TAG = "defold-videoplayer";

    private static void LOG(String message) {
        Log.v("defold-videoplayer", message);
    }

    public static void Init(final Context context) {
        LOG("VideoplayerExtension: Init()");
        Movie.init(context);
    }
    public static Movie Open(final Context context, String uri, int id) {
        LOG("VideoplayerExtension: Opening video");
        Movie movie = new Movie(context, uri, id);
        return movie;
    }
    public static void Close(Movie movie) {
        LOG("VideoplayerExtension: Closing video");
        movie.close();
    }
    public static void Update(float dt, Movie movie) {
        //LOG("VideoplayerExtension: Updating video " + Float.toString(dt));
        movie.update();
    }
    public static void Start(Movie movie) {
        LOG("VideoplayerExtension: Starting video");
        movie.start();
    }
    public static void Stop(Movie movie) {
        LOG("VideoplayerExtension: Stopping video");
        movie.stop();
    }
    public static void Pause(Movie movie) {
        LOG("VideoplayerExtension: Pausing video");
        movie.pause();
    }
}

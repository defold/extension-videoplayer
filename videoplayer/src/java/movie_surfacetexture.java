package com.defold.android.videoplayer;

import android.app.Activity;

import android.content.res.AssetFileDescriptor;
import android.content.Context;
import android.media.MediaPlayer;
import android.net.Uri;

import android.os.Bundle;
import android.view.View;
import android.view.Surface;
import android.graphics.SurfaceTexture;
import android.graphics.Canvas;
import android.graphics.Bitmap;

// debug
import android.graphics.Color;
import android.graphics.Paint;

import android.util.Log;
import java.util.Map;
import java.io.File;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.lang.Exception;
import java.lang.Runnable;
import java.nio.ByteBuffer;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

class MovieSurfaceTexture implements MediaPlayer.OnPreparedListener {

    private static void LOG(String message) {
        Log.v("defold-videoplayer", message);
    }

    private static final String LINK = "https://www.sample-videos.com/video/mp4/720/big_buck_bunny_720p_1mb.mp4";
    private static final String FILE = "big_buck_bunny_720p_1mb.mp4";

    private int id;
    private String uri;

    private boolean startVideo = false;
    private boolean isReady = false;
    private MediaPlayer mediaPlayer;

    private SurfaceTexture surfaceTexture;
    private Surface surface;

    private native void handleVideoFrame(int id, int width, int height);
    private native int videoIsReady(int id, int width, int height);

    private static void setup(final MovieSurfaceTexture instance, Context context) {
        LOG("MOVIE: setup()");
        final Activity activity = (Activity)context;

        LOG("MOVIE: new MediaPlayer()");
        instance.mediaPlayer = new MediaPlayer();

        try {
            // FILE ACCESS:
            //instance.mediaPlayer.setDataSource(context, Uri.fromFile(new File(FILE)), null);
            //instance.mediaPlayer.setDataSource(context, instance.videoSource, null);
            //AssetFileDescriptor afd = context.getResources().getAssets().openFd("cutscene_1.webm");

            // InputStream is = context.getResources().getAssets().open("cutscene_1.webm");
            // String s = getStringFromInputStream(is);
            // LOG("MOVIE: HELOOO");
            // LOG(s);
            String path = FILE;
            LOG("MOVIE: uri: " + instance.uri);
            LOG("MOVIE: openFd(): " + path);
            AssetFileDescriptor afd = context.getResources().getAssets().openFd(path);
            instance.mediaPlayer.setDataSource(afd.getFileDescriptor(),afd.getStartOffset(),afd.getLength());
            //instance.mediaPlayer.setDataSource(LINK);
        } catch (FileNotFoundException e) {
            LOG(e.toString());
            return;
        } catch (IOException e) {
            LOG(e.toString());
            return;
        }
        instance.mediaPlayer.setOnPreparedListener(instance);

        // mediaPlayer.setOnErrorListener(new MediaPlayer.OnErrorListener() {
        //         @Override
        //         public boolean onError(MediaPlayer mp, int what, int extra) {
        //             if (extra == MediaPlayer.MEDIA_ERROR_SERVER_DIED
        //                     || extra == MediaPlayer.MEDIA_ERROR_MALFORMED) {
        //                 LOG("MOVIE: error on playing");
        //             } else if (extra == MediaPlayer.MEDIA_ERROR_IO) {
        //                 LOG("MOVIE: error on playing");
        //                 return false;
        //             }
        //             return false;
        //         }
        //     });

        // mPlayer.setOnBufferingUpdateListener(new MediaPlayer.OnBufferingUpdateListener() {

        //     public void onBufferingUpdate(MediaPlayer mp, int percent) {
        //         Log.e("onBufferingUpdate", "" + percent);

        //     }
        // });

        instance.mediaPlayer.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
            @Override
            public void onCompletion(MediaPlayer mp) {
                LOG("MOVIE: onCompletion: Yes");
                //sendPlayerStatus("completed");
            }
        });

        // instance.mediaPlayer.setOnInfoListener(new MediaPlayer.OnInfoListener() {
        //     @Override
        //     public boolean onInfo(MediaPlayer mp, int what, int extra) {
        //      LOG("MOVIE: onInfo");
        //         return false;
        //     }
        // });

        instance.mediaPlayer.prepareAsync();
        LOG("MOVIE: setup() end");
    }

    public MovieSurfaceTexture(final Context context, String uri, int id){
        LOG("MOVIE: MovieSurfaceTexture()");
        this.uri = uri;
        this.id = id;

        isReady = false;

        MovieSurfaceTexture.setup(this, context);
    }

    @Override
    public void onPrepared(MediaPlayer mediaPlayer){
        LOG("MOVIE: MovieSurfaceTexture onPrepared()");

        int texture = videoIsReady(id, mediaPlayer.getVideoWidth(), mediaPlayer.getVideoHeight()); // Call into the native code
  //       if (texture <= 0) {
  //        return;
  //       }

  //       surfaceTexture = new SurfaceTexture(texture);
        // surfaceTexture.setOnFrameAvailableListener(new SurfaceTexture.OnFrameAvailableListener(){
        //  @Override
        //  public void onFrameAvailable(SurfaceTexture surfaceTexture){
        //      LOG("NEW FRAME AVAILABLE!");
        //  }
        // });

  //       LOG(String.format("MOVIE: MovieSurfaceTexture texture: %d", texture));

        // surface = new Surface(surfaceTexture);

        // mediaPlayer.setSurface(surface);
        isReady = true;
    }

    public void close(){
        LOG("MOVIE: MovieSurfaceTexture close()");
        if (mediaPlayer != null) {
            mediaPlayer.stop();
            mediaPlayer.release();
        }
    }

    public void setupSurface(int texture) {
        surfaceTexture = new SurfaceTexture(texture);
        surfaceTexture.setOnFrameAvailableListener(new SurfaceTexture.OnFrameAvailableListener(){
            @Override
            public void onFrameAvailable(SurfaceTexture surfaceTexture){
                LOG("NEW FRAME AVAILABLE!");
            }
        });

        LOG(String.format("MOVIE: MovieSurfaceTexture setupSurface: %d", texture));

        surface = new Surface(surfaceTexture);
        mediaPlayer.setSurface(surface);

        mediaPlayer.setLooping(true); // only for debugging purposes
    }

    public void update() {
        if (surfaceTexture != null && startVideo)
        {
            mediaPlayer.start();
            startVideo = false;
            LOG(String.format("MOVIE: MovieSurfaceTexture mediaPlayer.start()!"));
        }

        // if (mediaPlayer.isPlaying()) {
     //     LOG(String.format("MOVIE: MovieSurfaceTexture isPlaying() !"));
        // } else {
     //     LOG(String.format("MOVIE: MovieSurfaceTexture isPlaying() -- noooooo"));
        // }

        if(mediaPlayer != null && !mediaPlayer.isPlaying()) {
            return;
        }

        LOG("MOVIE: MovieSurfaceTexture Update");
        handleVideoFrame(id, mediaPlayer.getVideoWidth(), mediaPlayer.getVideoHeight());
    }

    public void start(int texture){
        LOG(String.format("MOVIE: MovieSurfaceTexture start()"));
        startVideo = true;
        return;

  //       LOG(String.format("MOVIE: MovieSurfaceTexture start(%d)", texture) );

  //       if (texture <= 0) {
  //        return;
  //       }

  //       surfaceTexture = new SurfaceTexture(texture);
        // surfaceTexture.setOnFrameAvailableListener(new SurfaceTexture.OnFrameAvailableListener(){
        //  @Override
        //  public void onFrameAvailable(SurfaceTexture surfaceTexture){
        //      LOG("NEW FRAME AVAILABLE!");
        //  }
        // });

  //       LOG(String.format("MOVIE: MovieSurfaceTexture texture: %d", texture));

        // surface = new Surface(surfaceTexture);

        // mediaPlayer.setSurface(surface);

  //       mediaPlayer.setLooping(true); // only for debugging purposes

  //       mediaPlayer.start();
    }

    public void stop(){
        LOG("MOVIE: MovieSurfaceTexture stop()");
        mediaPlayer.stop();
    }

    public void pause(){
        LOG("MOVIE: MovieSurfaceTexture pause()");
        mediaPlayer.pause();
    }
}

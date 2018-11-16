package com.defold.android.videoplayer;

import android.app.Activity;

import android.content.res.AssetFileDescriptor;
import android.content.Context;
import android.media.MediaPlayer;
import android.net.Uri;

import android.os.Bundle;
import android.view.View;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
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

class MovieSurfaceView implements MediaPlayer.OnPreparedListener {

    private static void LOG(String message) {
        Log.v("defold-videoplayer", message);
    }

    private static final String LINK = "https://www.sample-videos.com/video/mp4/720/big_buck_bunny_720p_1mb.mp4";
    private static final String FILE = "big_buck_bunny_720p_1mb.mp4";

    private int id;
    private String uri;

    private boolean isOK;
    private boolean isReady;
    private MediaPlayer mediaPlayer;

    private SurfaceView surfaceView;
    private Surface surface;

    private Canvas canvas;
    private Bitmap bitmap;

    private native void handleVideoFrame(int id, int width, int height, Bitmap bitmap);
    private native void videoIsReady(int id, int width, int height);

    private static void setup(final MovieSurfaceView instance, Context context) {
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

        LOG("MOVIE: new SurfaceView");
        instance.surfaceView = new SurfaceView(context);
        ViewGroup viewGroup = (ViewGroup)activity.findViewById(android.R.id.content);
        //viewGroup.addView(instance.surfaceView, 400, 400);
        viewGroup.addView(instance.surfaceView, new ViewGroup.LayoutParams(400, 400));
        //instance.surfaceView.setVisibility(View.GONE);
        //instance.surfaceView.setVisibility(View.INVISIBLE);
        // int index = viewGroup.indexOfChild(instance.surfaceView);
        // for(int i = 0; i<index; i++)
        // {
        //     viewGroup.bringChildToFront(viewGroup.getChildAt(i));
        // }

        LOG("MOVIE: instance.surfaceView.getHolder");
        SurfaceHolder holder = instance.surfaceView.getHolder();
        holder.addCallback(new Callback(){
            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                LOG("MOVIE: surfaceChanged");
            }

            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                LOG("MOVIE: surfaceCreated");
                instance.surface = holder.getSurface();
                instance.mediaPlayer.prepareAsync();
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                LOG("MOVIE: surfaceDestroyed");
            }
        });

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

        LOG("MOVIE: setup() end");
    }

    public MovieSurfaceView(final Context context, String _uri, int _id){
        LOG("MOVIE: MovieSurfaceView()");
        uri = _uri;
        id = _id;

        isOK = false;
        isReady = false;

        final MovieSurfaceView instance = this;
        Activity activity = (Activity)context;
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                MovieSurfaceView.setup(instance, context);
                instance.isOK = true;
            }
        });
    }

    @Override
    public void onPrepared(MediaPlayer mediaPlayer){
        LOG("MOVIE: MovieSurfaceView onPrepared()");
        bitmap = Bitmap.createBitmap(
            mediaPlayer.getVideoWidth(),
            mediaPlayer.getVideoHeight(),
            Bitmap.Config.ARGB_8888
        );
        canvas = new Canvas(bitmap);

        mediaPlayer.setSurface(surface);
        isReady = true;

        mediaPlayer.setLooping(true); // only for debugging purposes

        videoIsReady(id, mediaPlayer.getVideoWidth(), mediaPlayer.getVideoHeight()); // Call into the native code

        //mediaPlayer.setScreenOnWhilePlaying(true); // seemed to crash aafter a while?
        //mediaPlayer.start();
    }

    public void close(){
        LOG("MOVIE: MovieSurfaceView close()");
        if (mediaPlayer != null) {
            mediaPlayer.stop();
            mediaPlayer.release();
        }
    }

    // public static Bitmap screenShot(View view) {
    //     Bitmap bitmap = null;
    //     if (view.getWidth() > 0 && view.getHeight() > 0) {
    //         bitmap = Bitmap.createBitmap(view.getWidth(),
    //                 view.getHeight(), Bitmap.Config.ARGB_8888);
    //         Canvas canvas = new Canvas(bitmap);
    //         view.draw(canvas);
    //     }
    //     return bitmap;
    // }

    public Bitmap takeScreenShot(View u, int width, int height){
        u.setDrawingCacheEnabled(true);
        u.layout(0, 0, width, height);
        u.buildDrawingCache();
        Bitmap b = Bitmap.createBitmap(u.getDrawingCache());
        u.setDrawingCacheEnabled(false);
        u.destroyDrawingCache();
        return b;
    }

    public void update() {
        if(canvas == null || (mediaPlayer != null && !mediaPlayer.isPlaying())) {
            return;
        }

        LOG("MOVIE: MovieSurfaceView Update");
        surfaceView.draw(canvas);

    Paint paint = new Paint();
    paint.setStyle(Paint.Style.STROKE);
    paint.setColor(Color.rgb(255, 255, 255));
    paint.setStrokeWidth(10);
    canvas.drawRect(100, 100, 200, 200, paint);

        // Bitmap b = takeScreenShot(surfaceView,
        //     mediaPlayer.getVideoWidth(),
        //     mediaPlayer.getVideoHeight());

        // handleVideoFrame(id, b.getWidth(), b.getHeight(), b);

        handleVideoFrame(id, bitmap.getWidth(), bitmap.getHeight(), bitmap);

    }

    public void start(){
        LOG("MOVIE: MovieSurfaceView start()");
        mediaPlayer.start();
    }

    public void stop(){
        LOG("MOVIE: MovieSurfaceView stop()");
        mediaPlayer.stop();
    }

    public void pause(){
        LOG("MOVIE: MovieSurfaceView pause()");
        mediaPlayer.pause();
    }
}

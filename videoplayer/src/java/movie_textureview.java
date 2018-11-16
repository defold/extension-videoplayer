package com.defold.android.videoplayer;

import android.app.Activity;

import android.content.res.AssetFileDescriptor;
import android.content.Context;
import android.content.Intent;
import android.media.MediaPlayer;
import android.net.Uri;

import android.os.Bundle;
import android.view.View;
import android.view.Surface;
import android.view.TextureView;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.WindowManager;
import android.graphics.Canvas;
import android.graphics.Bitmap;
import android.graphics.SurfaceTexture;

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


class MovieTextureView implements MediaPlayer.OnPreparedListener, TextureView.SurfaceTextureListener {

    private static void LOG(String message) {
        Log.v("defold-videoplayer", message);
    }

    private static final String LINK = "https://www.sample-videos.com/video/mp4/720/big_buck_bunny_720p_1mb.mp4"; //"http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4"
    private static final String FILE = "big_buck_bunny_720p_1mb.mp4";

    private int id;
    private String uri;

    private Context context;
    private MediaPlayer mediaPlayer;

    private TextureView textureView;
    private Surface surface;
    private boolean isOK;

    // private Canvas canvas;
    // private Bitmap bitmap;
    private static boolean activityStarted = false;

    private native void handleVideoFrame(int id, int width, int height, Bitmap bitmap);
    private native void videoIsReady(int id, int width, int height);

    private static void setup(final MovieTextureView instance, Context context) {
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

        // LOG("MOVIE: new TextureView");
        // instance.textureView = new TextureView(context);
        // ViewGroup viewGroup = (ViewGroup)activity.findViewById(android.R.id.content);
        // viewGroup.addView(instance.textureView);

        //instance.textureView.setVisibility(View.GONE);
        //instance.textureView.setVisibility(View.INVISIBLE);
        // int index = viewGroup.indexOfChild(instance.textureView);
        // for(int i = 0; i<index; i++)
        // {
        //     viewGroup.bringChildToFront(viewGroup.getChildAt(i));
        // }

        // LOG("MOVIE: instance.textureView.getHolder");
        // SurfaceHolder holder = instance.textureView.getHolder();
        // holder.addCallback(new Callback(){
        //     @Override
        //     public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        //         LOG("MOVIE: surfaceChanged");
        //     }

        //     @Override
        //     public void surfaceCreated(SurfaceHolder holder) {
        //         LOG("MOVIE: surfaceCreated");
        //         instance.surface = holder.getSurface();
        //         instance.mediaPlayer.prepareAsync();
        //     }

        //     @Override
        //     public void surfaceDestroyed(SurfaceHolder holder) {
        //         LOG("MOVIE: surfaceDestroyed");
        //     }
        // });

        instance.mediaPlayer.prepareAsync();

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

    private static void startActivity(Context context) {
        LOG("MOVIE: startActivity()");
        Intent intent = new Intent(context.getApplicationContext(), VideoPlayerActivity.class);
        intent.putExtra("TEST_MESSAGE", "HELLO FROM Activity");
        // intent.putExtra(PARAM_AUTOFINISH_TRANSACTIONS, IapGooglePlay.this.autoFinishTransactions);
        // intent.putExtra(PARAM_PRODUCT, product);
        // intent.putExtra(PARAM_PRODUCT_TYPE, type);
        // intent.setAction(Action.BUY.toString());
        context.startActivity(intent);
        LOG("MOVIE: startActivity() end");
    }

    public static void init(final Context context){
        LOG("MOVIE: init()");
        startActivity(context);
        activityStarted = true;
    }

    public MovieTextureView(final Context context, String _uri, int _id){
        LOG("MOVIE: MovieTextureView()");
        uri = _uri;
        id = _id;
        isOK = false;

        if (VideoPlayerActivity.instance != null) {
            LOG("MOVIE: has activity() YAY!");
        } else {
            LOG("MOVIE: has NO activity()");
        }
        this.context = VideoPlayerActivity.instance;

        this.context = context;
        final Activity activity = (Activity)context;
        final MovieTextureView movie = this;

        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {

                LOG("MOVIE: getWindow() setFlags");

                activity.getWindow().setFlags(  WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED,
                                                WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED);

                LOG("MOVIE: new TextureView");
                textureView = new TextureView(activity);
                textureView.setSurfaceTextureListener(movie);

                ViewGroup viewGroup = (ViewGroup)activity.findViewById(android.R.id.content);
                viewGroup.addView(textureView);
            }
        });
    }

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surfaceTexture, int width, int height) {
        LOG("MOVIE: MovieTextureView onSurfaceTextureAvailable()");
        surface = new Surface(surfaceTexture);
        final MovieTextureView instance = this;
        final Context context = this.context;
        ((Activity)context).runOnUiThread(new Runnable() {
            @Override
            public void run() {
                MovieTextureView.setup(instance, context);
            }
        });
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
       // Log.i(TAG,"onSurfaceTextureSizeChanged");
        LOG("MOVIE: MovieTextureView onSurfaceTextureSizeChanged()");
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
       // Log.i(TAG,"onSurfaceTextureDestroyed");
        LOG("MOVIE: MovieTextureView onSurfaceTextureDestroyed()");
        return true;
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
       // Log.i(TAG,"onSurfaceTextureUpdated");
        LOG("MOVIE: MovieTextureView onSurfaceTextureUpdated()");
    }

    @Override
    public void onPrepared(MediaPlayer mediaPlayer){
        LOG("MOVIE: MovieTextureView onPrepared()");
        // bitmap = Bitmap.createBitmap(
        //     mediaPlayer.getVideoWidth(),
        //     mediaPlayer.getVideoHeight(),
        //     Bitmap.Config.ARGB_8888
        // );
        // canvas = new Canvas(bitmap);

        // mediaPlayer.setSurface(surface);

        mediaPlayer.setSurface(surface);

        mediaPlayer.setLooping(true); // only for debugging purposes

        videoIsReady(id, mediaPlayer.getVideoWidth(), mediaPlayer.getVideoHeight()); // Call into the native code

        isOK = true;

        //mediaPlayer.setScreenOnWhilePlaying(true); // seemed to crash aafter a while?
        //mediaPlayer.start();
    }

    public void close(){
        LOG("MOVIE: MovieTextureView close()");
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
        if(!isOK || (mediaPlayer != null && !mediaPlayer.isPlaying())) {
            return;
        }

        LOG("MOVIE: MovieTextureView Update");
    //     textureView.draw(canvas);

    // Paint paint = new Paint();
    // paint.setStyle(Paint.Style.STROKE);
    // paint.setColor(Color.rgb(255, 255, 255));
    // paint.setStrokeWidth(10);
    // canvas.drawRect(100, 100, 200, 200, paint);

        // Bitmap b = takeScreenShot(textureView,
        //     mediaPlayer.getVideoWidth(),
        //     mediaPlayer.getVideoHeight());

        // handleVideoFrame(id, b.getWidth(), b.getHeight(), b);


        // https://www.javatips.net/api/android.view.textureview
        Bitmap b = textureView.getBitmap();
        handleVideoFrame(id, b.getWidth(), b.getHeight(), b);

        //handleVideoFrame(id, bitmap.getWidth(), bitmap.getHeight(), bitmap);
    }

    public void start(){
        LOG("MOVIE: MovieTextureView start()");
        mediaPlayer.start();
    }

    public void stop(){
        LOG("MOVIE: MovieTextureView stop()");
        mediaPlayer.stop();
    }

    public void pause(){
        LOG("MOVIE: MovieTextureView pause()");
        mediaPlayer.pause();
    }
}

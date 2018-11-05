#if defined(DM_PLATFORM_ANDROID)

#include "videoplayer_private.h"

#include <android/log.h>
#include <android/bitmap.h>

static const int MAX_NUM_VIDEOS = 1;

struct SAndroidVideoInfo
{
    VideoPlayer_Callback    m_Callback;
    jobject                 m_Video;
    void*                   m_CallbackContext;
    int                     m_Width;
    int                     m_Height;
    bool                    m_IsReady;
};

struct SVideoPlayerAndroidContext
{
    jobject   m_Activity;
    jclass    m_Class;
    jmethodID m_OpenFn;
    jmethodID m_CloseFn;
    jmethodID m_UpdateFn;
    jmethodID m_StartFn;
    jmethodID m_StopFn;
    jmethodID m_PauseFn;

    int                 m_NumVideos;
    SAndroidVideoInfo   m_Videos[MAX_NUM_VIDEOS];
};

SVideoPlayerAndroidContext g_AndroidVideoContext;

static JNIEnv* Attach()
{
    JNIEnv* env;
    dmGraphics::GetNativeAndroidJavaVM()->AttachCurrentThread(&env, NULL);
    return env;
}

static jclass GetClass(JNIEnv* env, const char* classname)
{
    jclass activity_class = env->FindClass("android/app/NativeActivity");
    jmethodID get_class_loader = env->GetMethodID(activity_class, "getClassLoader", "()Ljava/lang/ClassLoader;");
    jobject cls = env->CallObjectMethod(dmGraphics::GetNativeAndroidActivity(), get_class_loader);
    jclass class_loader = env->FindClass("java/lang/ClassLoader");
    jmethodID find_class = env->GetMethodID(class_loader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

    jstring str_class_name = env->NewStringUTF(classname);
    jclass outcls = (jclass)env->CallObjectMethod(cls, find_class, str_class_name);
    env->DeleteLocalRef(str_class_name);
    return outcls;
}

struct AttachScope
{
    AttachScope() : env(Attach()) {}
    ~AttachScope() { dmGraphics::GetNativeAndroidJavaVM()->DetachCurrentThread(); }
    JNIEnv* env;
};


#ifdef __cplusplus
    extern "C" {
#endif
    JNIEXPORT void JNICALL Java_com_defold_android_videoplayer_Movie_videoIsReady(JNIEnv* env, jobject video, jint id, jint width, jint height)
    {
        assert(id >= 0 && id < g_AndroidVideoContext.m_NumVideos);
        VideoEventInfo eventinfo;
        eventinfo.m_Event = VIDEO_EVENT_READY;
        SAndroidVideoInfo& info = g_AndroidVideoContext.m_Videos[(int)id];
        info.m_Width = width;
        info.m_Height = height;
        int result = info.m_Callback(&eventinfo, info.m_CallbackContext);
        info.m_IsReady = result != 0;
    }

    JNIEXPORT void JNICALL Java_com_defold_android_videoplayer_Movie_handleVideoFrame(JNIEnv* env, jobject video, jint id, jint width, jint height, jobject bitmap)
    {
        dmLogWarning("%s: %s:%d:", __FILE__, __FUNCTION__, __LINE__);
        __android_log_print(ANDROID_LOG_VERBOSE, "defold-videoplayer", "handleVideoFrame: Hello from native function");

        AndroidBitmapInfo bitmapInfo;
        int ret;
        if ((ret = AndroidBitmap_getInfo(env, bitmap, &bitmapInfo)) < 0)
        {
            dmLogError("AndroidBitmap_getInfo() failed ! error=%d", ret);
            return;
        }
        if (bitmapInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
        {
            dmLogError("Bitmap format is not RGBA_8888!");
            return;
        }
        void* bitmapPixels;
        if ((ret = AndroidBitmap_lockPixels(env, bitmap, &bitmapPixels)) < 0)
        {
            dmLogError("AndroidBitmap_lockPixels() failed ! error=%d", ret);
            return;
        }

        VideoEventInfo eventinfo;
        eventinfo.m_Event = VIDEO_EVENT_FRAME_READY;
        eventinfo.m_Width = bitmapInfo.width;
        eventinfo.m_Height = bitmapInfo.height;
        eventinfo.m_Stride = bitmapInfo.stride;
        eventinfo.m_Depth = 4;
        eventinfo.m_Data = bitmapPixels;
        SAndroidVideoInfo& info = g_AndroidVideoContext.m_Videos[(int)id];
        info.m_Callback(&eventinfo, info.m_CallbackContext);

        AndroidBitmap_unlockPixels(env, bitmap);
    }
#ifdef __cplusplus
    }
#endif


int VideoPlayer_Init()
{
    dmLogWarning("%s: %s:%d:", __FILE__, __FUNCTION__, __LINE__);
    AttachScope scope;
    JNIEnv* env = scope.env;
    jclass cls                          = GetClass(env, "com.defold.android.videoplayer.VideoplayerExtension");
    g_AndroidVideoContext.m_Activity    = dmGraphics::GetNativeAndroidActivity();
    g_AndroidVideoContext.m_Class       = cls;

    // TODO: Skip the VideoPlayer class, and invoke the Movie class directly
    g_AndroidVideoContext.m_OpenFn      = env->GetStaticMethodID(cls, "Open", "(Landroid/content/Context;Ljava/lang/String;I)Lcom/defold/android/videoplayer/Movie;");
    g_AndroidVideoContext.m_CloseFn     = env->GetStaticMethodID(cls, "Close", "(Lcom/defold/android/videoplayer/Movie;)V");
    g_AndroidVideoContext.m_UpdateFn    = env->GetStaticMethodID(cls, "Update", "(FLcom/defold/android/videoplayer/Movie;)V");
    g_AndroidVideoContext.m_StartFn     = env->GetStaticMethodID(cls, "Start", "(Lcom/defold/android/videoplayer/Movie;)V");
    g_AndroidVideoContext.m_StopFn      = env->GetStaticMethodID(cls, "Stop", "(Lcom/defold/android/videoplayer/Movie;)V");
    g_AndroidVideoContext.m_PauseFn     = env->GetStaticMethodID(cls, "Pause", "(Lcom/defold/android/videoplayer/Movie;)V");

    g_AndroidVideoContext.m_NumVideos   = 0;

    jmethodID initFn = env->GetStaticMethodID(cls, "Init", "(Landroid/content/Context;)V");
    env->CallStaticVoidMethod(cls, initFn, g_AndroidVideoContext.m_Activity);

    // if all is ok, return 1
    return 1;
}

void VideoPlayer_Exit()
{
    return;
}

HNativeVideo VideoPlayer_Open(const char* uri, VideoPlayer_Callback cbk, void* ctx)
{
    dmLogWarning("%s: %s:%d:", __FILE__, __FUNCTION__, __LINE__);
    if (g_AndroidVideoContext.m_NumVideos >= MAX_NUM_VIDEOS)
    {
        dmLogError("Max number of videos opened: %d", MAX_NUM_VIDEOS);
        return 0;
    }

    int id = g_AndroidVideoContext.m_NumVideos;

    AttachScope scope;
    jstring juri = scope.env->NewStringUTF(uri);
    jobject jvideo = scope.env->CallStaticObjectMethod(g_AndroidVideoContext.m_Class, g_AndroidVideoContext.m_OpenFn, g_AndroidVideoContext.m_Activity, juri, id);
    jvideo = (jobject)scope.env->NewGlobalRef(jvideo);
    scope.env->DeleteLocalRef(juri);
    if (jvideo)
    {
        ++g_AndroidVideoContext.m_NumVideos;
        SAndroidVideoInfo& info = g_AndroidVideoContext.m_Videos[id];
        info.m_Video = jvideo;
        info.m_Callback = cbk;
        info.m_CallbackContext = ctx;
        info.m_Width = 0;
        info.m_Height = 0;
        info.m_IsReady = false;
        return (HNativeVideo)(id+1);
    }
    return (HNativeVideo)0;
}

bool VideoPlayer_IsReady(HNativeVideo video)
{
    dmLogWarning("%s: %s:%d:", __FILE__, __FUNCTION__, __LINE__);
    AttachScope scope;
    SAndroidVideoInfo& info = g_AndroidVideoContext.m_Videos[((int)video) - 1];
    return info.m_IsReady;
}

void VideoPlayer_GetInfo(HNativeVideo video, VideoInfo& outinfo)
{
    dmLogWarning("%s: %s:%d:", __FILE__, __FUNCTION__, __LINE__);
    SAndroidVideoInfo& info = g_AndroidVideoContext.m_Videos[((int)video) - 1];
    outinfo.m_Width = info.m_Width;
    outinfo.m_Height = info.m_Height;
}

void VideoPlayer_Close(HNativeVideo video)
{
    dmLogWarning("%s: %s:%d:", __FILE__, __FUNCTION__, __LINE__);
    AttachScope scope;
    SAndroidVideoInfo& info = g_AndroidVideoContext.m_Videos[((int)video) - 1];
    scope.env->CallStaticVoidMethod(g_AndroidVideoContext.m_Class, g_AndroidVideoContext.m_CloseFn, info.m_Video);
}

void VideoPlayer_Start(HNativeVideo video)
{
    dmLogWarning("%s: %s:%d:", __FILE__, __FUNCTION__, __LINE__);
    AttachScope scope;
    SAndroidVideoInfo& info = g_AndroidVideoContext.m_Videos[((int)video) - 1];
    scope.env->CallStaticVoidMethod(g_AndroidVideoContext.m_Class, g_AndroidVideoContext.m_StartFn, info.m_Video);
}

void VideoPlayer_Stop(HNativeVideo video)
{
    dmLogWarning("%s: %s:%d:", __FILE__, __FUNCTION__, __LINE__);
    AttachScope scope;
    SAndroidVideoInfo& info = g_AndroidVideoContext.m_Videos[((int)video) - 1];
    scope.env->CallStaticVoidMethod(g_AndroidVideoContext.m_Class, g_AndroidVideoContext.m_StopFn, info.m_Video);
}

void VideoPlayer_Pause(HNativeVideo video)
{
    dmLogWarning("%s: %s:%d:", __FILE__, __FUNCTION__, __LINE__);
    AttachScope scope;
    SAndroidVideoInfo& info = g_AndroidVideoContext.m_Videos[((int)video) - 1];
    scope.env->CallStaticVoidMethod(g_AndroidVideoContext.m_Class, g_AndroidVideoContext.m_PauseFn, info.m_Video);
}

void VideoPlayer_Update(float dt, HNativeVideo video)
{
    //dmLogWarning("%s: %s:%d:", __FILE__, __FUNCTION__, __LINE__);
    AttachScope scope;
    SAndroidVideoInfo& info = g_AndroidVideoContext.m_Videos[((int)video) - 1];
    scope.env->CallStaticVoidMethod(g_AndroidVideoContext.m_Class, g_AndroidVideoContext.m_UpdateFn, dt, info.m_Video);
}

#endif

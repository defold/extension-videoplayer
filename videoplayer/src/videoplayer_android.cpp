#if defined(DM_PLATFORM_ANDROID)

#include "videoplayer_private.h"

#include <android/log.h>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#define GL_BGRA GL_BGRA_EXT

#include <EGL/egl.h>

static const int MAX_NUM_VIDEOS = 1;


// Testing / RnD setup

//#define TEST_TEXTURE_VIEW

#define MOVIE_CLASS_NAME MovieSurfaceTexture
//#define MOVIE_JAVA_CLASS_NAME "Lcom/defold/android/videoplayer/MovieSurfaceView;"
//#define MOVIE_JAVA_CLASS_NAME "Lcom/defold/android/videoplayer/MovieTextureView;"
#define MOVIE_JAVA_CLASS_NAME "Lcom/defold/android/videoplayer/MovieSurfaceTexture;"


struct SAndroidVideoInfo
{
    VideoPlayer_Callback    m_Callback;
    jobject                 m_Video;
    void*                   m_CallbackContext;
    int                     m_Width;
    int                     m_Height;
    int                     m_Texture;
    GLuint                  m_Fbo;
    GLuint                  m_RenderBuffer;
    GLuint                  m_Vbo;
    GLuint                  m_VertexShader;
    GLuint                  m_FragmentShader;
    GLuint                  m_ShaderProgram;
    GLuint                  m_AttribPosition;
    GLuint                  m_AttribTexcoord;
    bool                    m_IsReady;
    bool                    m_FrameReady;
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
    jmethodID m_SetupSurfaceFn;

    int                 m_NumVideos;
    SAndroidVideoInfo   m_Videos[MAX_NUM_VIDEOS];
};

SVideoPlayerAndroidContext g_AndroidVideoContext;

static int CreateTexture(int width, int height);

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

#define CHECK_GL_ERROR_VOID(...) \
    { \
        int glresult = glGetError(); \
        if (glresult != GL_NO_ERROR) \
        { \
            dmLogError("%s: %d: glGetError: %d", __FUNCTION__, __LINE__, glresult); \
            return; \
        } \
    }

#define CHECK_GL_ERROR(_RETVAL) \
    { \
        int glresult = glGetError(); \
        if (glresult != GL_NO_ERROR) \
        { \
            dmLogError("%s: %d: glGetError: %d", __FUNCTION__, __LINE__, glresult); \
            return (_RETVAL); \
        } \
    }

static void CreateFBO(int width, int height, GLuint* fbo, GLuint* render_buf)
{
    glGenFramebuffers(1, fbo);
    CHECK_GL_ERROR_VOID();
    // glGenRenderbuffers(1, render_buf);
    // CHECK_GL_ERROR_VOID();
    // glBindRenderbuffer(GL_RENDERBUFFER, *render_buf);
    // CHECK_GL_ERROR_VOID();
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);
    // CHECK_GL_ERROR_VOID();
    glBindFramebuffer(GL_FRAMEBUFFER, *fbo);
    CHECK_GL_ERROR_VOID();
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, *render_buf);
    // CHECK_GL_ERROR_VOID();
    // glBindRenderbuffer(GL_RENDERBUFFER, 0);
    // CHECK_GL_ERROR_VOID();

    *render_buf = CreateTexture(width, height);
    CHECK_GL_ERROR_VOID();
    glBindTexture(GL_TEXTURE_2D, *render_buf);
    CHECK_GL_ERROR_VOID();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *render_buf, 0);
    CHECK_GL_ERROR_VOID();

        // if(buffer_type_flags & dmGraphics::BUFFER_TYPE_COLOR_BIT)
        // {
        //     uint32_t color_buffer_index = GetBufferTypeIndex(BUFFER_TYPE_COLOR_BIT);
        //     rt->m_ColorBufferTexture = NewTexture(context, creation_params[color_buffer_index]);
        //     SetTexture(rt->m_ColorBufferTexture, params[color_buffer_index]);
        //     // attach the texture to FBO color attachment point
        //     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rt->m_ColorBufferTexture->m_Texture, 0);
        //     CHECK_GL_ERROR
        // }
    glBindTexture(GL_TEXTURE_2D, 0);
    CHECK_GL_ERROR_VOID();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERROR_VOID();
}

static void DestroyFBO(GLuint fbo, GLuint render_buf)
{
    if (glIsFramebuffer(fbo)) {
        glDeleteFramebuffers(1, &fbo);
        glDeleteRenderbuffers(1, &render_buf);
    }
}

static void CreateVBO(GLuint* vbo, GLuint attribPosition, GLuint attribTexcoord)
{
    // CW order?
    float vertices[] = {
        -0.95f, -0.05f, 0, 1,
        -0.05f, -0.95f, 1, 0,
        -0.95f, -0.95f, 0, 0,

        -0.95f, -0.05f, 0, 1,
        -0.05f, -0.05f, 1, 1,
        -0.05f, -0.95f, 1, 0
    };

    // float vertices[] = {
    //      0.0f,  0.5f,
    //      0.5f, -0.5f,
    //     -0.5f, -0.5f
    // };
    glGenBuffers(1, vbo);
    CHECK_GL_ERROR_VOID();
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    CHECK_GL_ERROR_VOID();
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    CHECK_GL_ERROR_VOID();
    glEnableVertexAttribArray(attribPosition);
    CHECK_GL_ERROR_VOID();
    // glVertexAttribPointer(attribPosition, 2, GL_FLOAT, GL_FALSE, 0, 0);
    // CHECK_GL_ERROR_VOID();
    glEnableVertexAttribArray(attribTexcoord);
    CHECK_GL_ERROR_VOID();
    // glVertexAttribPointer(attribTexcoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
    // CHECK_GL_ERROR_VOID();
}

static void RenderVbo(GLuint fbo, GLuint vbo, GLuint program, GLuint attribPosition, GLuint attribTexcoord, GLuint texture)
{
    uint32_t stride = sizeof(float)*4;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glUseProgram(program);
    CHECK_GL_ERROR_VOID();
    glEnableVertexAttribArray(attribPosition);
    CHECK_GL_ERROR_VOID();
    glVertexAttribPointer(attribPosition, 2, GL_FLOAT, GL_FALSE, stride, 0);
    CHECK_GL_ERROR_VOID();
    glEnableVertexAttribArray(attribTexcoord);
    CHECK_GL_ERROR_VOID();
    glVertexAttribPointer(attribTexcoord, 2, GL_FLOAT, GL_FALSE, stride, (void*)(uintptr_t)(sizeof(float)*2) );
    CHECK_GL_ERROR_VOID();

    // glEnable(GL_TEXTURE_2D);
    // CHECK_GL_ERROR_VOID();

    glActiveTexture(GL_TEXTURE0);
    CHECK_GL_ERROR_VOID();

    glBindTexture(GL_TEXTURE_2D, texture);
    CHECK_GL_ERROR_VOID();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    CHECK_GL_ERROR_VOID();
}

static const char* g_VideoPlayerVShader = "\
attribute mediump vec2 position;\
attribute mediump vec2 texcoord;\
varying mediump vec2 var_texcoord;\
void main()\
{\
    var_texcoord = texcoord;\
    gl_Position = vec4(position, 0.0, 1.0);\
}";

static const char* g_VideoPlayerFShader = "\
varying mediump vec2 var_texcoord;\
uniform lowp sampler2D tex;\
void main()\
{\
    vec4 t = texture2D(tex, var_texcoord.xy);\
    gl_FragColor = vec4(var_texcoord.x, var_texcoord.y, t.z * 0.5 + 0.5, 1);\
}";

// gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\

static void CreateShader(GLuint* program, GLuint* vshader, GLuint* fshader, GLuint* attribPosition, GLuint* attribTexcoord)
{
    *vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(*vshader, 1, &g_VideoPlayerVShader, NULL);
    glCompileShader(*vshader);
    *fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(*fshader, 1, &g_VideoPlayerFShader, NULL);
    glCompileShader(*fshader);
    *program = glCreateProgram();
    glAttachShader(*program, *vshader);
    glAttachShader(*program, *fshader);
    glLinkProgram(*program);

    *attribPosition = glGetAttribLocation(*program, "position");
    CHECK_GL_ERROR_VOID();
    *attribTexcoord = glGetAttribLocation(*program, "texcoord");
    CHECK_GL_ERROR_VOID();
}

static void GetTextureData(GLuint fbo, int texture, int width, int height, void* buffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // GLint current_texture = 0;
    // glGetIntegerv(GL_TEXTURE_BINDING_2D, &current_texture);
    // glBindTexture(GL_TEXTURE_2D, texture);
    // glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);
    // glBindTexture(GL_TEXTURE_2D, current_texture);

    //glReadBuffer(GL_COLOR_ATTACHMENT0);

    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void CreateOpenGLRelated(SAndroidVideoInfo* info)
{
    int width = info->m_Width;
    int height = info->m_Height;
    CreateFBO(width, height, &info->m_Fbo, &info->m_RenderBuffer);
    CreateShader(&info->m_ShaderProgram, &info->m_VertexShader, &info->m_FragmentShader, &info->m_AttribPosition, &info->m_AttribTexcoord);
    CreateVBO(&info->m_Vbo, info->m_AttribPosition, info->m_AttribTexcoord);

    // Create the texture
    int texture = CreateTexture(width, height);
    if (texture <= 0)
    {
        dmLogError("Could not create OpenGL texture for video rendering");
        VideoEventInfo eventinfo;
        eventinfo.m_Event = VIDEO_EVENT_FAILED;
        info->m_Callback(&eventinfo, info->m_CallbackContext);
        DestroyFBO(info->m_Fbo, info->m_RenderBuffer);
        return;
    }
    info->m_Texture = texture;
}

static void FrameCallback(SAndroidVideoInfo* info)
{
    int width = info->m_Width;
    int height = info->m_Height;
    if (info->m_Fbo == 0)
    {
        CreateOpenGLRelated(info);
    }

    void* mem = malloc(width*height*4);

    RenderVbo(info->m_Fbo, info->m_Vbo, info->m_ShaderProgram, info->m_AttribPosition, info->m_AttribTexcoord, info->m_Texture);

    GetTextureData(info->m_Fbo, info->m_Texture, width, height, mem);

    VideoEventInfo eventinfo;
    eventinfo.m_Event = VIDEO_EVENT_FRAME_READY;
    eventinfo.m_Width = width;
    eventinfo.m_Height = height;
    eventinfo.m_Depth = 4;
    eventinfo.m_Stride = width * eventinfo.m_Depth;
    eventinfo.m_Data = mem;

    info->m_Callback(&eventinfo, info->m_CallbackContext);

    free(mem);
}

#ifdef __cplusplus
    extern "C" {
#endif

    #define MOVIE_MAKE_NATIVE_FN(clsname, fnname) Java_com_defold_android_videoplayer_ ## clsname ## fnname

    JNIEXPORT int JNICALL Java_com_defold_android_videoplayer_MovieSurfaceTexture_videoIsReady(JNIEnv* env, jobject video, jint id, jint width, jint height)
    {
        dmLogWarning("%s:%d:", __FUNCTION__, __LINE__);
        assert(id >= 0 && id < g_AndroidVideoContext.m_NumVideos);

        SAndroidVideoInfo& info = g_AndroidVideoContext.m_Videos[id];
        info.m_Width = width;
        info.m_Height = height;
        info.m_Fbo = 0;
        info.m_Texture = 0;

        // CreateFBO(info.m_Width, info.m_Height, &info.m_Fbo, &info.m_RenderBuffer);

        // // Create the texture
        // int texture = CreateTexture(width, height);
        // if (texture <= 0)
        // {
        //     dmLogError("Could not create OpenGL texture for video rendering");
        //     VideoEventInfo eventinfo;
        //     eventinfo.m_Event = VIDEO_EVENT_FAILED;
        //     info.m_Callback(&eventinfo, info.m_CallbackContext);
        //     return 0;
        // }
        // info.m_Texture = texture;

        VideoEventInfo eventinfo;
        eventinfo.m_Event = VIDEO_EVENT_READY;
        eventinfo.m_Width = width;
        eventinfo.m_Height = height;
        eventinfo.m_Depth = 4;
        int result = info.m_Callback(&eventinfo, info.m_CallbackContext); // Creates the dmBuffer
        info.m_IsReady = result != 0;
        return result;
    }

    JNIEXPORT void JNICALL Java_com_defold_android_videoplayer_MovieSurfaceTexture_handleVideoFrame(JNIEnv* env, jobject video, jint id, jint width, jint height)
    {
        //dmLogWarning("%s:%d:", __FUNCTION__, __LINE__);
        assert(id >= 0 && id < g_AndroidVideoContext.m_NumVideos);

        SAndroidVideoInfo& info = g_AndroidVideoContext.m_Videos[(int)id];
        info.m_FrameReady = 1;

        // NOTE: Not thread safe? we might be in the middle of rendering the engine frame
        // Should delay commands to the next update

        // void* mem = malloc(width*height*4);

        // GetTextureData(info.m_Fbo, info.m_Texture, width, height, mem);

        // VideoEventInfo eventinfo;
        // eventinfo.m_Event = VIDEO_EVENT_FRAME_READY;
        // eventinfo.m_Width = width;
        // eventinfo.m_Height = height;
        // eventinfo.m_Stride = width * 4;
        // eventinfo.m_Depth = 4;
        // eventinfo.m_Data = mem;

        // info.m_Callback(&eventinfo, info.m_CallbackContext);

        // free(mem);
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
    //g_AndroidVideoContext.m_Class       = cls;

    g_AndroidVideoContext.m_Class       = (jclass)env->NewGlobalRef(cls);

    // TODO: Skip the VideoPlayer class, and invoke the Movie class directly
    g_AndroidVideoContext.m_OpenFn      = env->GetStaticMethodID(cls, "Open", "(Landroid/content/Context;Ljava/lang/String;I)" MOVIE_JAVA_CLASS_NAME);
    g_AndroidVideoContext.m_CloseFn     = env->GetStaticMethodID(cls, "Close", "(" MOVIE_JAVA_CLASS_NAME ")V");
    g_AndroidVideoContext.m_UpdateFn    = env->GetStaticMethodID(cls, "Update", "(" MOVIE_JAVA_CLASS_NAME "F)V");
    g_AndroidVideoContext.m_StartFn     = env->GetStaticMethodID(cls, "Start", "(" MOVIE_JAVA_CLASS_NAME "I)V");
    g_AndroidVideoContext.m_StopFn      = env->GetStaticMethodID(cls, "Stop", "(" MOVIE_JAVA_CLASS_NAME ")V");
    g_AndroidVideoContext.m_PauseFn     = env->GetStaticMethodID(cls, "Pause", "(" MOVIE_JAVA_CLASS_NAME ")V");
    g_AndroidVideoContext.m_SetupSurfaceFn     = env->GetStaticMethodID(cls, "SetupSurface", "(" MOVIE_JAVA_CLASS_NAME "I)V");

    g_AndroidVideoContext.m_NumVideos   = 0;

#if defined(TEST_TEXTURE_VIEW)
    // Testing to create an activity in an init step
    jmethodID initFn = env->GetStaticMethodID(cls, "Init", "(Landroid/content/Context;)V");
    env->CallStaticVoidMethod(cls, initFn, g_AndroidVideoContext.m_Activity);
#endif

    // if all is ok, return 1
    return 1;
}

void VideoPlayer_Exit()
{
    AttachScope scope;
    scope.env->DeleteGlobalRef(g_AndroidVideoContext.m_Class);
    return;
}

static int CreateTexture(int width, int height)
{
    EGLDisplay egl_display = EGL_NO_DISPLAY;
    EGLSurface egl_surface = EGL_NO_SURFACE;
    EGLContext defold_context = dmGraphics::GetNativeAndroidEGLContext();
    EGLContext egl_context = eglGetCurrentContext();
    // if (defold_context != egl_context)
    // {
    //     //egl_display = eglGetCurrentDisplay();
    //     egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    //     if (egl_display == EGL_NO_DISPLAY) {
    //         dmLogError("EGL: Failed to get current display: %d.", eglGetError());
    //         return -1;
    //     }

    //     egl_context = eglGetCurrentContext();
    //     // if (egl_context == EGL_NO_CONTEXT) {
    //     //     dmLogError("EGL: Failed to get current egl context: %d.", eglGetError());
    //     //     return -1;
    //     // }

    //     egl_surface = eglGetCurrentSurface(EGL_DRAW);
    //     // if (egl_context == EGL_NO_SURFACE) {
    //     //     dmLogError("EGL: Failed to get current egl surface: %d.", eglGetError());
    //     //     return -1;
    //     // }

    //     EGLSurface defold_egl_surface = dmGraphics::GetNativeAndroidEGLSurface();
    //     if (!eglMakeCurrent(egl_display, defold_egl_surface, defold_egl_surface, defold_context)) {
    //         dmLogError("EGL: Failed to switch to defold surface: %d." , eglGetError());
    //         return -1;
    //     }
    // }

    int textureID;
    glGenTextures( 1, (GLuint*)&textureID );
    CHECK_GL_ERROR(0);


    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Fill it with black pixels
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    // if (defold_context != egl_context)
    // {
    //     if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context)) {
    //         dmLogError("EGL: Failed to switch to defold surface: %d." , eglGetError());
    //         return -1;
    //     }
    // }

    return textureID;
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
    jobject jvideo = scope.env->CallStaticObjectMethod(g_AndroidVideoContext.m_Class, g_AndroidVideoContext.m_OpenFn, dmGraphics::GetNativeAndroidActivity(), juri, id);
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
        info.m_Texture = 0;
        info.m_IsReady = false;
        return (HNativeVideo)id;
    }
    return (HNativeVideo)-1;
}

bool VideoPlayer_IsReady(HNativeVideo video)
{
    dmLogWarning("%s: %s:%d:", __FILE__, __FUNCTION__, __LINE__);
    AttachScope scope;
    SAndroidVideoInfo& info = g_AndroidVideoContext.m_Videos[video];
    DestroyFBO(info.m_Fbo, info.m_RenderBuffer);
    return info.m_IsReady;
}

void VideoPlayer_GetInfo(HNativeVideo video, VideoInfo& outinfo)
{
    dmLogWarning("%s: %s:%d:", __FILE__, __FUNCTION__, __LINE__);
    SAndroidVideoInfo& info = g_AndroidVideoContext.m_Videos[video];
    outinfo.m_Width = info.m_Width;
    outinfo.m_Height = info.m_Height;
    outinfo.m_Depth = 4;
}

void VideoPlayer_Close(HNativeVideo video)
{
    dmLogWarning("%s: %s:%d:", __FILE__, __FUNCTION__, __LINE__);
    AttachScope scope;
    SAndroidVideoInfo& info = g_AndroidVideoContext.m_Videos[video];
    scope.env->CallStaticVoidMethod(g_AndroidVideoContext.m_Class, g_AndroidVideoContext.m_CloseFn, info.m_Video);
    scope.env->DeleteGlobalRef(info.m_Video);
}

void VideoPlayer_Start(HNativeVideo video)
{
    dmLogWarning("%s: %s:%d:", __FILE__, __FUNCTION__, __LINE__);
    AttachScope scope;
    SAndroidVideoInfo& info = g_AndroidVideoContext.m_Videos[video];
    scope.env->CallStaticVoidMethod(g_AndroidVideoContext.m_Class, g_AndroidVideoContext.m_StartFn, info.m_Video, info.m_Texture);
}

void VideoPlayer_Stop(HNativeVideo video)
{
    dmLogWarning("%s: %s:%d:", __FILE__, __FUNCTION__, __LINE__);
    AttachScope scope;
    SAndroidVideoInfo& info = g_AndroidVideoContext.m_Videos[video];
    scope.env->CallStaticVoidMethod(g_AndroidVideoContext.m_Class, g_AndroidVideoContext.m_StopFn, info.m_Video);
}

void VideoPlayer_Pause(HNativeVideo video)
{
    dmLogWarning("%s: %s:%d:", __FILE__, __FUNCTION__, __LINE__);
    AttachScope scope;
    SAndroidVideoInfo& info = g_AndroidVideoContext.m_Videos[video];
    scope.env->CallStaticVoidMethod(g_AndroidVideoContext.m_Class, g_AndroidVideoContext.m_PauseFn, info.m_Video);
}

void VideoPlayer_Update(float dt, HNativeVideo video)
{
    //dmLogWarning("%s: %s:%d:", __FILE__, __FUNCTION__, __LINE__);
    AttachScope scope;
    SAndroidVideoInfo& info = g_AndroidVideoContext.m_Videos[video];

    if (info.m_Texture == 0)
    {
        CreateOpenGLRelated(&info);
        assert(info.m_Texture > 0);
        scope.env->CallStaticVoidMethod(g_AndroidVideoContext.m_Class, g_AndroidVideoContext.m_SetupSurfaceFn, info.m_Video, info.m_Texture);
    }

    scope.env->CallStaticVoidMethod(g_AndroidVideoContext.m_Class, g_AndroidVideoContext.m_UpdateFn, info.m_Video, dt);

    if (info.m_FrameReady)
    {
        FrameCallback(&info);
    }
}

#endif

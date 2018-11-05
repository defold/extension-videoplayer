#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#if !defined(WIN32)
#include <unistd.h> // ?? what includes
#endif

#define LIB_NAME "VideoPlayer"
#define MODULE_NAME "videoplayer"

// Defold SDK
#include <dmsdk/sdk.h>

#include "videoplayer_private.h"

// temp

#include <android/log.h>

const static int s_MaxNumVideos = 1;
static const int s_BytesPerPixel = 3;

struct SVideoPlayerContext
{
    Video*  m_Videos[s_MaxNumVideos];
    bool    m_IsReady[s_MaxNumVideos];
    int m_NumVideos;
};

SVideoPlayerContext g_VideoPlayerContext;


static int VideoPlayer_EventCallback(const VideoEventInfo* info, void* ctx)
{
    __android_log_print(ANDROID_LOG_VERBOSE, "defold-videoplayer", "VideoPlayer_EventCallback");
    Video* video = (Video*)ctx;
    if (info->m_Event == VIDEO_EVENT_READY)
    {
    __android_log_print(ANDROID_LOG_VERBOSE, "defold-videoplayer", "VIDEO_EVENT_READY");
        if (video->m_VideoBufferLuaRef == LUA_NOREF)
        {
            VideoInfo info;
            VideoPlayer_GetInfo(video->m_Video, info);

    __android_log_print(ANDROID_LOG_VERBOSE, "defold-videoplayer", "VIDEO_EVENT_READY INFO: %d, %d", info.m_Width, info.m_Height);

            dmBuffer::Result r = VideoPlayer_CreateBuffer(info.m_Width, info.m_Height, &video->m_VideoBuffer);
            // TODO: Call user if it failed

            // Increase ref count
            dmScript::LuaHBuffer buffer = {video->m_VideoBuffer, false};
            dmScript::PushBuffer(video->m_L, buffer);
            video->m_VideoBufferLuaRef = dmScript::Ref(video->m_L, LUA_REGISTRYINDEX);

            g_VideoPlayerContext.m_IsReady[video->m_Id] = true;
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else if (info->m_Event == VIDEO_EVENT_FRAME_READY)
    {
    __android_log_print(ANDROID_LOG_VERBOSE, "defold-videoplayer", "VIDEO_EVENT_FRAME_READY");
        if (video->m_VideoBuffer)
        {
            uint8_t* data = 0;
            uint32_t datasize = 0;
            dmBuffer::GetBytes(video->m_VideoBuffer, (void**)&data, &datasize);
            if ((info->m_Width * info->m_Depth) != info->m_Stride)
            {
                dmLogWarning("FRAME: Width: %u   stride: %u", (info->m_Width * info->m_Depth), info->m_Stride);
                // uint32_t height = info->m_Height;
                // for ( uint32_t y = 0; y < height; ++y)
                // {

                // }
            }
            else // They're the same, we can do a full copy
            {
    __android_log_print(ANDROID_LOG_VERBOSE, "defold-videoplayer", "CASE2: width: %d  height: %d  depth: %d  stride: %d", info->m_Width, info->m_Height, info->m_Depth, info->m_Stride);

                dmLogWarning("FRAME: Width: %u   stride: %u", (info->m_Width * info->m_Depth), info->m_Stride);
                memcpy((void*)data, info->m_Data, datasize);
            }

            return 1;
        }
    }
    return 0;
}

static Video* CheckVideo(lua_State* L, int index)
{
    Video* video = (Video*)(uintptr_t)luaL_checknumber(L, index);
    if (video == 0 || video->m_Video == 0)
    {
        return (Video*)(uintptr_t)luaL_error(L, "Invalid video passed in");
    }
    return video;
}

static int Open(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);

    if (g_VideoPlayerContext.m_NumVideos >= s_MaxNumVideos)
    {
        lua_pushnil(L);
        dmLogError("Max number of videos already open: %d", g_VideoPlayerContext.m_NumVideos);
        return 1;
    }

    if (lua_isstring(L, 1) ) {
        const char* uri = luaL_checkstring(L, 1);

        Video* video = (Video*)malloc(sizeof(Video));
        memset(video, 0, sizeof(Video));

        HNativeVideo nativevideo = VideoPlayer_Open(uri, VideoPlayer_EventCallback, (void*)video);
        if (nativevideo) {
            video->m_Video = nativevideo;
            video->m_L = L;
            video->m_VideoBufferLuaRef = LUA_NOREF;
            video->m_VideoBuffer = 0;
            video->m_Id = g_VideoPlayerContext.m_NumVideos++;

            g_VideoPlayerContext.m_Videos[video->m_Id] = video;
            g_VideoPlayerContext.m_IsReady[video->m_Id] = false;
            lua_pushnumber(L, (uintptr_t)video);
        } else {
            free(video);
            lua_pushnil(L);
            dmLogError("Failed to open video: %s", uri);
        }
    }
    return 1;
}

static int Close(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    Video* video = CheckVideo(L, 1);
    VideoPlayer_Close(video->m_Video);
    if (video->m_Video != LUA_NOREF)
    {
        dmScript::Unref(L, LUA_REGISTRYINDEX, video->m_Video);
    }

    g_VideoPlayerContext.m_Videos[video->m_Id] = 0;
    g_VideoPlayerContext.m_IsReady[video->m_Id] = false;

    free(video);
    return 0;
}

static int Start(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    Video* video = CheckVideo(L, 1);
    VideoPlayer_Start(video->m_Video);
    return 0;
}

static int Stop(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    Video* video = CheckVideo(L, 1);
    VideoPlayer_Stop(video->m_Video);
    return 0;
}

static int Pause(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    Video* video = CheckVideo(L, 1);
    VideoPlayer_Pause(video->m_Video);
    return 0;
}

static int IsReady(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    Video* video = CheckVideo(L, 1);
    //bool ready = VideoPlayer_IsReady(video->m_Video) != 0;
    bool ready = g_VideoPlayerContext.m_IsReady[video->m_Id];
    lua_pushboolean(L, ready);
    return 1;
}

static int GetInfo(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);

    Video* video = CheckVideo(L, 1);
    VideoInfo info;
    VideoPlayer_GetInfo(video->m_Video, info);

    lua_newtable(L);
    lua_pushnumber(L, info.m_Width);
    lua_setfield(L, -2, "width");
    lua_pushnumber(L, info.m_Height);
    lua_setfield(L, -2, "height");
    lua_pushnumber(L, s_BytesPerPixel);     // TODO: Check if we can use ARGB instead, for even faster copies
    lua_setfield(L, -2, "bytes_per_pixel");
    return 1;
}

static int GetBuffer(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    Video* video = CheckVideo(L, 1);
    lua_rawgeti(L, LUA_REGISTRYINDEX, video->m_VideoBufferLuaRef);
    return 1;
}

static const luaL_reg Module_methods[] =
{
    {"open", Open},
    {"close", Close},
    //{"update", Update}, // In the vpx case, let the extension update them all
    {"start", Start},
    {"stop", Stop},
    {"pause", Pause},
    {"is_ready", IsReady},
    {"get_info", GetInfo},
    {"get_buffer", GetBuffer},
    {0, 0}
};

static void LuaInit(lua_State* L)
{
    int top = lua_gettop(L);
    luaL_register(L, MODULE_NAME, Module_methods);
    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

static dmExtension::Result AppInitializeVideoPlayer(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result InitializeVideoPlayer(dmExtension::Params* params)
{
    if (VideoPlayer_Init()) {
        g_VideoPlayerContext.m_NumVideos = 0;
        LuaInit(params->m_L);
        dmLogInfo("Registered %s extension", MODULE_NAME);
    } else {
        dmLogError("Failed to init %s extension", MODULE_NAME);
    }
    return dmExtension::RESULT_OK;
}

static dmExtension::Result UpdateVideoPlayer(dmExtension::Params* params)
{
    if (g_VideoPlayerContext.m_NumVideos) {
        // TODO: calculate a dt
        float dt = 0; // For those platforms that update in software, others we need polling to copy the pixels
        for (int i = 0; i < g_VideoPlayerContext.m_NumVideos; ++i)
        {
            VideoPlayer_Update(dt, g_VideoPlayerContext.m_Videos[i]->m_Video);
        }
    }
    return dmExtension::RESULT_OK;
}

static dmExtension::Result AppFinalizeVideoPlayer(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeVideoPlayer(dmExtension::Params* params)
{
    for (int i = 0; i < g_VideoPlayerContext.m_NumVideos; ++i)
    {
        Video* video = g_VideoPlayerContext.m_Videos[i];
        VideoPlayer_Close(video->m_Video);
        if (video->m_VideoBufferLuaRef != LUA_NOREF)
        {
            dmScript::Unref(params->m_L, LUA_REGISTRYINDEX, video->m_VideoBufferLuaRef);
        }
        free(video);
    }
    VideoPlayer_Exit();
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(VideoPlayer, LIB_NAME, AppInitializeVideoPlayer, AppFinalizeVideoPlayer, InitializeVideoPlayer, UpdateVideoPlayer, 0, FinalizeVideoPlayer)

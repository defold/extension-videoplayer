#pragma once

#include <dmsdk/sdk.h>

typedef int HNativeVideo;

struct VideoInfo
{
    int m_Width;
    int m_Height;
    int m_Depth;
};

struct Video
{
    HNativeVideo        m_Video;
    dmBuffer::HBuffer   m_VideoBuffer; // Output to engine
    lua_State*          m_L;
    int                 m_VideoBufferLuaRef;
    int                 m_Id;
};

enum VideoEvent
{
    VIDEO_EVENT_READY,          // Called when the video is ready to start playing
    VIDEO_EVENT_FRAME_READY,    // Called when a new video frame is ready
    VIDEO_EVENT_FAILED,         // The operation failed
};

struct VideoEventInfo
{
    void*       m_Data;     // Depending on event type. VIDEO_EVENT_FRAME_READY -> frame pixel data
    VideoEvent  m_Event;
    uint32_t    m_Width;
    uint32_t    m_Height;
    uint32_t    m_Stride;
    uint32_t    m_Depth;    // 1,2,3,4 channels
};

// Callback function
typedef int (*VideoPlayer_Callback)(const VideoEventInfo* info, void* ctx);

// Implemented separately for each platform
int             VideoPlayer_Init(); // return 1 for ok
void            VideoPlayer_Exit();

HNativeVideo    VideoPlayer_Open(const char* uri, VideoPlayer_Callback cbk, void* ctx);
bool            VideoPlayer_IsReady(HNativeVideo video);
void            VideoPlayer_GetInfo(HNativeVideo video, VideoInfo& info);
void            VideoPlayer_Close(HNativeVideo video);
void            VideoPlayer_Start(HNativeVideo video);
void            VideoPlayer_Stop(HNativeVideo video);
void            VideoPlayer_Pause(HNativeVideo video);
void            VideoPlayer_Update(float dt, HNativeVideo video); // vpx videos need individual updates


// Helper functions
dmBuffer::Result VideoPlayer_CreateBuffer(int width, int height, int depth, dmBuffer::HBuffer* buffer);


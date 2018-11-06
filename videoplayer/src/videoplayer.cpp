#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#if !defined(WIN32)
#include <unistd.h>
#endif

#define LIB_NAME "VideoPlayer"
#define MODULE_NAME "videoplayer"

// Defold SDK
#define DLIB_LOG_DOMAIN LIB_NAME
#include <dmsdk/sdk.h>

// libvpx + libwebm
#include <vpx/vpx_decoder.h>
#include <vpx/tools/tools_common.h> // VpxInputContext
#include <webm/webmdec.h>           // WebmInputContext

#include <libyuv/convert_from.h>

// Example code
// https://github.com/webmproject/libvpx/blob/master/examples/simple_decoder.c

static const int s_BytesPerPixel = 3;


enum MovieType
{
    MOVIE_TYPE_WEBM
};


struct Movie
{
    VpxInputContext     m_VpxCtx;
    WebmInputContext    m_WebmCtx;
    const VpxInterface* m_FourCCInterface;
    vpx_codec_ctx_t     m_Decoder;
    vpx_codec_dec_cfg_t m_DecoderCfg;
    dmBuffer::HBuffer   m_VideoBuffer; // Output
    MovieType           m_Type;
    float               m_Time;
    int                 m_Frame;
    int                 m_VideoBufferLuaRef;
    int                 m_Corrupted;
};

static uint64_t g_VideoBufferStreamName = dmHashString64("rgb");


static int Open(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);

    dmScript::LuaHBuffer* sourcebuffer = dmScript::CheckBuffer(L, 1);

    Movie* movie = new Movie;
    memset(movie, 0, sizeof(*movie));

    {
        uint8_t* data = 0;
        uint32_t datasize = 0;
        dmBuffer::GetBytes(sourcebuffer->m_Buffer, (void**)&data, &datasize);

        movie->m_VpxCtx.length = datasize;
        movie->m_VpxCtx.buffer = (uint8_t*)malloc(datasize);
        memcpy(movie->m_VpxCtx.buffer, data, datasize);
    }

    if( !file_is_webm(&movie->m_WebmCtx, &movie->m_VpxCtx) )
    {
        luaL_error(L, "File is not a WebM movie");
        return 0;
    }
    movie->m_Type = MOVIE_TYPE_WEBM;

    if( movie->m_Type == MOVIE_TYPE_WEBM)
    {
        if (webm_guess_framerate(&movie->m_WebmCtx, &movie->m_VpxCtx))
        {
            luaL_error(L, "Failed to guess frame rate");
            return 0;
        }
    }

    movie->m_FourCCInterface = get_vpx_decoder_by_fourcc(movie->m_VpxCtx.fourcc);
    if( !movie->m_FourCCInterface )
    {
        luaL_error(L, "Failed to find fourcc decoder");
        return 0;
    }

    if (vpx_codec_dec_init(&movie->m_Decoder, movie->m_FourCCInterface->codec_interface(), &movie->m_DecoderCfg, 0)) {
        fprintf(stderr, "Failed to initialize decoder: %s\n", vpx_codec_error(&movie->m_Decoder));
        luaL_error(L, "Failed to initialize decoder");
    }


    const uint32_t size = movie->m_VpxCtx.width*movie->m_VpxCtx.height;
    dmBuffer::StreamDeclaration streams_decl[] = {
        {g_VideoBufferStreamName, dmBuffer::VALUE_TYPE_UINT8, 3}
    };

    dmBuffer::Create(size, streams_decl, 1, &movie->m_VideoBuffer);

    // Reset texture to 0
    uint8_t* data = 0;
    uint32_t datasize = 0;
    dmBuffer::GetBytes(movie->m_VideoBuffer, (void**)&data, &datasize);
    memset(data, 0, datasize);

    // Increase ref count
    dmScript::LuaHBuffer buffer = {movie->m_VideoBuffer, false};
    dmScript::PushBuffer(L, buffer);
    movie->m_VideoBufferLuaRef = dmScript::Ref(L, LUA_REGISTRYINDEX);

    lua_pushnumber(L, (uintptr_t)movie);

    return 1;
}

static int Close(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    Movie* movie = (Movie*)(uintptr_t) luaL_checknumber(L, 1);
    assert(movie != 0);

    free(movie->m_VpxCtx.buffer);

    webm_free(&movie->m_WebmCtx);
    vpx_codec_destroy(&movie->m_Decoder);

    dmScript::Unref(L, LUA_REGISTRYINDEX, movie->m_VideoBufferLuaRef); // We want it destroyed by the GC

    delete movie;
    return 0;
}

static int Update(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    Movie* movie = (Movie*)(uintptr_t) luaL_checknumber(L, 1);
    assert(movie != 0);

    float dt = (float) luaL_checknumber(L, 2);

    if( movie->m_Corrupted )
    {
        return 0;
    }

    movie->m_Time += dt;

    uint32_t num_frames_tick = 0;
    {
        float seconds_per_frame = (float)movie->m_VpxCtx.framerate.denominator / movie->m_VpxCtx.framerate.numerator;
        int frames = (int)(movie->m_Time / seconds_per_frame);
        num_frames_tick = frames - movie->m_Frame + 1;
    }

    if( num_frames_tick == 0 )
    {
        if( movie->m_Frame > 0 )
        {
            return 0;
        }
    }

    uint8_t* out_stream = 0x0;
    uint32_t out_size = 0x0;
    uint32_t out_num_components = 0x0;
    uint32_t out_stride = 0x0;
    dmBuffer::Result r = dmBuffer::GetStream(movie->m_VideoBuffer, g_VideoBufferStreamName, (void**)&out_stream, &out_size, &out_num_components, &out_stride);
    if( r != dmBuffer::RESULT_OK )
    {
        printf("Video stream was not retrieved properly: %d", (int)r);
        lua_pushnil(L);
        return 1;
    }

    int result = 0;
    uint8_t* framebuffer = 0;
    size_t framebuffersize = 0;

    vpx_image_t* last_img = 0;
    for( uint32_t i = 0; i < num_frames_tick; ++i )
    {
        if( movie->m_Type == MOVIE_TYPE_WEBM )
        {
            result = webm_read_frame(&movie->m_WebmCtx, &framebuffer, &framebuffersize);
        }

        if( !result )
        {
            if (vpx_codec_decode(&movie->m_Decoder, framebuffer, (unsigned int)framebuffersize, NULL, 0)) {
                const char* detail = vpx_codec_error_detail(&movie->m_Decoder);
                movie->m_Corrupted = 1;
                printf("Video frame decode error: '%s'", detail);
                return 0;
            }
        }

        vpx_image_t* img = 0;
        vpx_codec_iter_t iter = NULL;
        while ((img = vpx_codec_get_frame(&movie->m_Decoder, &iter)) != NULL) {
            last_img = img;
            ++movie->m_Frame;
        }
    }

    vpx_image_t* img = last_img;
    if( !img ) // looping?
    {
        VpxRational framerate = movie->m_VpxCtx.framerate;
        webm_free(&movie->m_WebmCtx);
        file_is_webm(&movie->m_WebmCtx, &movie->m_VpxCtx);
        movie->m_VpxCtx.framerate = framerate;
        movie->m_Time = 0;
        movie->m_Frame = 0;
        // TODO: Callback ?
        return 0;
    }

    // I420ToRGB flips the R&B channels, which we don't want
    libyuv::I420ToRAW(img->planes[VPX_PLANE_Y], img->stride[VPX_PLANE_Y],
                      img->planes[VPX_PLANE_U], img->stride[VPX_PLANE_U],
                      img->planes[VPX_PLANE_V], img->stride[VPX_PLANE_V],
                      (uint8_t*)out_stream, img->d_w * 3,
                      img->d_w, -img->d_h); // negative height flips the image

    dmBuffer::ValidateBuffer(movie->m_VideoBuffer);

    return 0;
}

static int GetInfo(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);

    Movie* movie = (Movie*)(uintptr_t) luaL_checknumber(L, 1);
    assert(movie != 0);

    lua_newtable(L);
    lua_pushstring(L, "width");
    lua_pushnumber(L, movie->m_VpxCtx.width);
    lua_rawset(L, -3);
    lua_pushstring(L, "height");
    lua_pushnumber(L, movie->m_VpxCtx.height);
    lua_rawset(L, -3);
    lua_pushstring(L, "bytes_per_pixel");
    lua_pushnumber(L, s_BytesPerPixel);
    lua_rawset(L, -3);

    return 1;
}


static int GetFrame(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);

    Movie* movie = (Movie*)(uintptr_t) luaL_checknumber(L, 1);
    assert(movie != 0);

    lua_rawgeti(L,LUA_REGISTRYINDEX, movie->m_VideoBufferLuaRef);

    return 1;
}

static const luaL_reg Module_methods[] =
{
    {"open", Open},
    {"close", Close},
    {"update", Update},
    {"get_info", GetInfo},
    {"get_frame", GetFrame},
    {0, 0}
};

static void LuaInit(lua_State* L)
{
    int top = lua_gettop(L);
    luaL_register(L, MODULE_NAME, Module_methods);

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

dmExtension::Result AppInitializeVideoPlayer(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeVideoPlayer(dmExtension::Params* params)
{
    LuaInit(params->m_L);
    printf("Registered %s Extension\n", MODULE_NAME);
    return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalizeVideoPlayer(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizeVideoPlayer(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(VideoPlayer, LIB_NAME, AppInitializeVideoPlayer, AppFinalizeVideoPlayer, InitializeVideoPlayer, 0, 0, FinalizeVideoPlayer)

#if !defined(DM_PLATFORM_ANDROID)

#include "videoplayer_private.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#if !defined(WIN32)
#include <unistd.h>
#endif

// libvpx + libwebm
#include <vpx/vpx_decoder.h>
#include <vpx/tools/tools_common.h> // VpxInputContext
#include <webm/webmdec.h>           // WebmInputContext

namespace dmVideoPlayer
{
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
		int                 m_FrameCount;
	};

	static uint64_t g_VideoBufferStreamName = dmHashString64("rgb");

	int GetFrameCount(Movie* movie)
	{
		uint8_t* framebuffer = 0;
		size_t framebuffersize = 0;
		int numframes = 0;
		while(!webm_read_frame(&movie->m_WebmCtx, &framebuffer, &framebuffersize))
		{
			++numframes;
		}

		printf("Num frames: %d\n", numframes);
		return numframes;
	}


	int Platform_Open(lua_State* L)
	{
		const char* filename = luaL_checkstring(L, 1);
		assert(filename != 0);

		Movie* movie = new Movie;
		memset(movie, 0, sizeof(*movie));

		{
			FILE* fp = fopen(filename, "rb");
			if(fp == 0)
			{
				fprintf(stderr, "Failed to open file: %s\n", filename);
				luaL_error(L, "Failed to open file!");
				return 0;
			}

			fseek(fp, 0, SEEK_END);
			uint32_t datasize = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			printf("Movie size: %d\n", datasize);

			movie->m_VpxCtx.length = datasize;
			movie->m_VpxCtx.buffer = (uint8_t*)malloc(datasize);

			fread(movie->m_VpxCtx.buffer, 1, datasize, fp);

			fclose(fp);
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

		//movie->m_FrameCount = GetFrameCount(movie);

		lua_pushnumber(L, (uintptr_t)movie);

		return 1;
	}

	int Platform_Close(lua_State* L)
	{
		Movie* movie = (Movie*)(uintptr_t) luaL_checknumber(L, 1);
		assert(movie != 0);

		free(movie->m_VpxCtx.buffer);
		webm_free(&movie->m_WebmCtx);
		vpx_codec_destroy(&movie->m_Decoder);
		dmScript::Unref(L, LUA_REGISTRYINDEX, movie->m_VideoBufferLuaRef); // We want it destroyed by the GC
		delete movie;

		return 0;
	}

	// http://stackoverflow.com/questions/17088118/decode-from-vp8-video-frame-to-rgb
	void ConvertYV12toRGB(const vpx_image_t* img, uint32_t buffersize, uint8_t* buffer)
	{
		uint8_t* yPlane = img->planes[VPX_PLANE_Y];
		uint8_t* uPlane = img->planes[VPX_PLANE_U];
		uint8_t* vPlane = img->planes[VPX_PLANE_V];

		int stride = img->d_w * s_BytesPerPixel;
		buffer = buffer + buffersize - stride;

		for (unsigned int imgY = 0; imgY < img->d_h; imgY++) {
			for (unsigned int imgX = 0; imgX < img->d_w; imgX++) {
				int y = yPlane[imgY * img->stride[VPX_PLANE_Y] + imgX];
				int u = uPlane[(imgY / 2) * img->stride[VPX_PLANE_U] + (imgX / 2)];
				int v = vPlane[(imgY / 2) * img->stride[VPX_PLANE_V] + (imgX / 2)];

				int c = y - 16;
				int d = (u - 128);
				int e = (v - 128);

				#define CLAMP(_X_) ((_X_) < 0 ? 0 : ((_X_)>255 ? 255 : (_X_)))

				int r = CLAMP((298 * c           + 409 * e + 128) >> 8);
				int g = CLAMP((298 * c - 100 * d - 208 * e + 128) >> 8);
				int b = CLAMP((298 * c + 516 * d           + 128) >> 8);

				buffer[imgX*3 + 0] = (uint8_t)r;
				buffer[imgX*3 + 1] = (uint8_t)g;
				buffer[imgX*3 + 2] = (uint8_t)b;
			}
			buffer -= stride;
		}
	}


	int Platform_Update(lua_State* L)
	{
		Movie* movie = (Movie*)(uintptr_t) luaL_checknumber(L, 1);
		assert(movie != 0);

		float dt = (float) luaL_checknumber(L, 2);

		if( movie->m_Corrupted )
		{
			lua_pushboolean(L, true);
			return 1;
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
				lua_pushboolean(L, false);
				return 1;
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
			//lua_pushnil(L);
			lua_pushboolean(L, true);
			return 1;
		}

		int result = 0;
		uint8_t* framebuffer = 0;
		size_t framebuffersize = 0;

		for( uint32_t i = 0; i < num_frames_tick; ++i )
		{
			++movie->m_Frame;

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
					lua_pushboolean(L, true);
					return 1;
				}
			}
		}

		vpx_codec_iter_t iter = NULL;
		vpx_image_t* img = vpx_codec_get_frame(&movie->m_Decoder, &iter);
		if( !img )
		{
			lua_pushboolean(L, true);
			return 1;
		}

		// component size is 1
		ConvertYV12toRGB(img, out_size * out_stride, (uint8_t*)out_stream);

		dmBuffer::ValidateBuffer(movie->m_VideoBuffer);

		lua_pushboolean(L, false);
		return 1;
	}

	int Platform_GetInfo(lua_State* L)
	{
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


	int Platform_GetFrame(lua_State* L)
	{
		Movie* movie = (Movie*)(uintptr_t) luaL_checknumber(L, 1);
		assert(movie != 0);

		lua_rawgeti(L,LUA_REGISTRYINDEX, movie->m_VideoBufferLuaRef);

		return 1;
	}

	dmExtension::Result Platform_Initialize(dmExtension::Params* params)
	{
		printf("Registered %s (Default) Extension\n", MODULE_NAME);
		return dmExtension::RESULT_OK;
	}
	dmExtension::Result Platform_Finalize(dmExtension::Params* params) { return dmExtension::RESULT_OK; }
	dmExtension::Result Platform_AppInitialize(dmExtension::AppParams* params) { return dmExtension::RESULT_OK; }
	dmExtension::Result Platform_AppFinalize(dmExtension::AppParams* params) { return dmExtension::RESULT_OK; }

}

#endif

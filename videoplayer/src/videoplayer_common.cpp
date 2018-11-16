#include "videoplayer_private.h"

static uint64_t g_VideoBufferStreamName3 = dmHashString64("rgb");
static uint64_t g_VideoBufferStreamName4 = dmHashString64("rgba");

dmBuffer::Result VideoPlayer_CreateBuffer(int width, int height, int depth, dmBuffer::HBuffer* buffer)
{
    dmBuffer::StreamDeclaration streams_decl[] = {
        {depth == 3 ? g_VideoBufferStreamName3 : g_VideoBufferStreamName4, dmBuffer::VALUE_TYPE_UINT8, (uint8_t)depth}
    };

    const uint32_t size = (uint32_t)(width * height);
    return dmBuffer::Create(size, streams_decl, 1, buffer);
}

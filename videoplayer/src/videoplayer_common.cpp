#include "videoplayer_private.h"

static uint64_t g_VideoBufferStreamName = dmHashString64("rgb");

dmBuffer::Result VideoPlayer_CreateBuffer(int width, int height, dmBuffer::HBuffer* buffer)
{
    dmBuffer::StreamDeclaration streams_decl[] = {
        {g_VideoBufferStreamName, dmBuffer::VALUE_TYPE_UINT8, 3}
    };

    const uint32_t size = (uint32_t)(width * height);
    return dmBuffer::Create(size, streams_decl, 1, buffer);
}

//-----------------------------------------------------------------------------
// COPYRIGHT (C) 2020   CHIPS&MEDIA INC. ALL RIGHTS RESERVED
// 
// This file is distributed under BSD 3 clause and LGPL2.1 (dual license)
// SPDX License Identifier: BSD-3-Clause
// SPDX License Identifier: LGPL-2.1-only
// 
// The entire notice above must be reproduced on all authorized copies.
// 
// Description  : 
//-----------------------------------------------------------------------------
#include <time.h>
#include "main_helper.h"
#include "libavformat/avformat.h"

static BOOL initFFmpeg;

typedef struct FeederFrameContext {
    AVFormatContext*    avContext;
    BOOL                isFirstPacket;
    Uint32              videoIndex;
    Uint8*              tempBuffer;
    Uint32              tempRdPtr;
    Uint32              tempWrPtr;
} FeederFrameContext;

void* BSFeederFrameSize_Create(const char* path)
{
    /*lint -esym(438, avContext) */
    FeederFrameContext* ffmpegReader = NULL;
    AVFormatContext*    avContext    = NULL;
    AVCodecContext*     codec        = NULL;
    AVInputFormat*      fmt          = NULL;
    Int32               error;
    Int32               videoIndex;

    jdi_lock();
    if (initFFmpeg == FALSE) {
        av_register_all();
        initFFmpeg = TRUE;
    }
    jdi_unlock();

    if ((avContext=avformat_alloc_context()) == NULL) {
        return NULL;
    }

    avContext->flags |= CODEC_FLAG_TRUNCATED;
    if ((error=avformat_open_input(&avContext, path, fmt, NULL))) {
        JLOG(ERR, "%s:%d failed to av_open_input_file error(%d), %s\n",
             __FILE__, __LINE__, error, path);
        goto __failed_to_end;
    }

    if ((error=avformat_find_stream_info(avContext, NULL)) < 0) {
        JLOG(ERR, "%s:%d failed to avformat_find_stream_info. error(%d)\n",
            __FUNCTION__, __LINE__, error);
        goto __failed_to_end;
    }

    videoIndex = av_find_best_stream(avContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (videoIndex < 0) {
        JLOG(ERR, "%s:%d failed to av_find_best_stream.\n", __FUNCTION__, __LINE__);
        goto __failed_to_end;
    }

    codec = avContext->streams[videoIndex]->codec;
    if (codec->codec_id != AV_CODEC_ID_MJPEG) {
        JLOG(ERR, "%s:%d Not supported codec_id: %d\n", __FUNCTION__, __LINE__, codec->codec_id);
        goto __failed_to_end;
    }

    if ((ffmpegReader=(FeederFrameContext*)malloc(sizeof(FeederFrameContext))) == NULL)
        goto __failed_to_end;

    ffmpegReader->avContext     = avContext;
    ffmpegReader->videoIndex    = videoIndex;
    ffmpegReader->isFirstPacket = TRUE;
    ffmpegReader->tempBuffer    = NULL;
    ffmpegReader->tempRdPtr     = 0;
    ffmpegReader->tempWrPtr     = 0;

    return (void*)ffmpegReader;

__failed_to_end:
    if (avContext) {
        avformat_free_context(avContext);
        avContext = NULL;
    }

    if (ffmpegReader) {
        free(ffmpegReader);
    }

    return NULL;
    /*lint +esym(438, avContext) */
}

BOOL BSFeederFrameSize_Destroy(
    void*   feeder
    )
{
    FeederFrameContext*  ctx = (FeederFrameContext*)feeder;

    if (ctx == NULL) {
        JLOG(ERR, "%s:%d Invalid handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (ctx->avContext)
        avformat_close_input(&ctx->avContext);

    free(ctx);

    return TRUE;
}

Int32 BSFeederFrameSize_Act(void* feeder, BSChunk* packet)
{
    FeederFrameContext* ffmpegReader = (FeederFrameContext*)feeder;
    AVFormatContext*    avFormatContext = ffmpegReader->avContext;
    AVPacket            avpacket;
    Int32               error;
    Uint8*              ptr;
    Uint32              size;
    Int32               packetSize = -1;

    if (ffmpegReader->tempBuffer) {
        goto __consume_tempBuffer;
    }

    av_init_packet(&avpacket);
    while (TRUE) {
        error = av_read_frame(avFormatContext, &avpacket);
        if (error < 0) {
            if (error == AVERROR_EOF || avFormatContext->pb->eof_reached == TRUE) {
                packet->eos = TRUE;
                return 0;
            }
            else {
                JLOG(ERR, "%s:%d failed to av_read_frame error(0x%08x)\n", 
                    __FUNCTION__, __LINE__, error);
                goto __end_read;
            }
        } 

        if (avpacket.stream_index != ffmpegReader->videoIndex) 
            continue;

        break;
    }

    if (avpacket.size == 0) 
        return 0;

    if (avpacket.size >= (signed)packet->size )
    {
        JLOG(ERR, "one packet size(%d) is bigger than STREAM_BUF_SIZE(%d)\n", avpacket.size, packet->size);
        return -1;
    }
   
    memset(packet->data, 0x00, packet->size);

    ptr  = avpacket.data;
    size = avpacket.size;

    memcpy((char*)packet->data, ptr, size);
    packetSize = size;

    if (avFormatContext->pb->eof_reached && avFormatContext->packet_buffer == NULL) {
        packet->eos = TRUE;
    }

__end_read:
    av_free_packet(&avpacket);

    return packetSize;

__consume_tempBuffer:
    if (ffmpegReader->tempBuffer != NULL) {
        memcpy(packet->data, ffmpegReader->tempBuffer, ffmpegReader->tempWrPtr);
        packetSize = ffmpegReader->tempWrPtr;
        free(ffmpegReader->tempBuffer);
        ffmpegReader->tempBuffer = NULL;
        ffmpegReader->tempWrPtr  = 0;
        ffmpegReader->tempRdPtr  = 0;
    }

    return packetSize;
}

BOOL BSFeederFrameSize_Rewind(
    void* feeder
    )
{
    FeederFrameContext*      ffmpegReader = (FeederFrameContext*)feeder;
    AVFormatContext*    avFormatContext = ffmpegReader->avContext;
    Int32               ret;

    if ((ret=av_seek_frame(avFormatContext, ffmpegReader->videoIndex, 0, 0)) < 0) {
        JLOG(ERR, "%s:%d Failed to av_seek_frame:(ret:%d)\n", __FUNCTION__, __LINE__, ret);
        return FALSE;
    }

    return TRUE;
}

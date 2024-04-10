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
#include "jpuapi.h"
#include "main_helper.h"

typedef struct {
    FeedingMethod   method;
    Uint8*          remainData;
    Uint32          remainDataSize;
    Uint32          remainOffset;
    void*           actualFeeder;
    Uint32          room;
    BOOL            eos;
    EndianMode      endian;
} BitstreamFeeder;

extern void* BSFeederFixedSize_Create(const char* path);
extern BOOL BSFeederFixedSize_Destroy(void* feeder);
extern Int32 BSFeederFixedSize_Act(void* feeder, BSChunk* chunk);
extern void BSFeederFixedSize_SetFeedingSize(void* feeder, Uint32  feedingSize);

extern void* BSFeederFrameSize_Create(const char* path);
extern BOOL BSFeederFrameSize_Destroy(void* feeder);
extern Int32 BSFeederFrameSize_Act(void* feeder, BSChunk* packet);

/**
* Abstract Bitstream Feeader Functions 
*/
BSFeeder BitstreamFeeder_Create(const char* path, FeedingMethod method, EndianMode endian)
{
    BitstreamFeeder*    handle = NULL;
    void*               feeder = NULL;

    switch (method) {
    case FEEDING_METHOD_FIXED_SIZE:
        feeder = BSFeederFixedSize_Create(path);
        break;
#if 0 //ndef PLATFORM_NON_OS
    case FEEDING_METHOD_FRAME_SIZE:
        feeder = BSFeederFrameSize_Create(path);
        break;
#endif
    default:
        feeder = NULL;
        break;
    }

    if (feeder != NULL) {
        if ((handle=(BitstreamFeeder*)malloc(sizeof(BitstreamFeeder))) == NULL) {
            JLOG(ERR, "%s:%d Failed to allocate memory\n", __FUNCTION__, __LINE__);
            return NULL;
        }
        handle->actualFeeder = feeder;
        handle->method       = method;
        handle->remainData   = NULL;
        handle->remainDataSize = 0;
        handle->remainOffset = 0;
        handle->eos          = FALSE;
        handle->endian       = endian;
    }

    return (BSFeeder)handle;
}

Uint32 BitstreamFeeder_Act(BSFeeder feeder, JpgDecHandle handle, jpu_buffer_t* bsBuffer)
{
    BitstreamFeeder* bsf = (BitstreamFeeder*)feeder;
    Int32            feedingSize = 0;
    int              room;
    BSChunk          chunk = {0};
    EndianMode       endian;
    PhysicalAddress  wrPtr;

    if (bsf == NULL) {
        JLOG(ERR, "%s:%d Null handle\n", __FUNCTION__, __LINE__);
        return 0;
    }

    JPU_DecGetBitstreamBuffer(handle, NULL/* rdPtr */, &wrPtr, &room);

    endian = bsf->endian;

    if (bsf->remainData == NULL) {
        chunk.size = bsBuffer->size; 
        chunk.data = malloc(chunk.size);
        chunk.eos  = FALSE;
        if (chunk.data == NULL) {
            JLOG(ERR, "%s:%d failed to allocate memory\n", __FUNCTION__, __LINE__);
            return 0;
        }
        switch (bsf->method) {
        case FEEDING_METHOD_FIXED_SIZE:
            feedingSize = BSFeederFixedSize_Act(bsf->actualFeeder, &chunk);
            break;
#if 0//ndef PLATFORM_NON_OS
        case FEEDING_METHOD_FRAME_SIZE:
            feedingSize = BSFeederFrameSize_Act(bsf->actualFeeder, &chunk);
            break;
#endif
        default:
            JLOG(ERR, "%s:%d Invalid method(%d)\n", __FUNCTION__, __LINE__, bsf->method);
            free(chunk.data);
            return 0;
        }
    }
    else {
        chunk.data  = bsf->remainData;
        feedingSize = bsf->remainDataSize;
    }

    if (feedingSize < 0) {
        JLOG(ERR, "feeding size is negative value: %d\n", feedingSize);
        free(chunk.data);
        return 0;
    }

    if (feedingSize > 0) {
        Uint32          rightSize=0, leftSize=0;
        PhysicalAddress base  = bsBuffer->phys_addr;
        Uint32          size  = bsBuffer->size;
        Uint32          wSize = feedingSize;
        Uint8*          ptr   = chunk.data + bsf->remainOffset;

        if ((Int32)room < feedingSize) {
            wSize               = room;
            bsf->remainData     = chunk.data;
            bsf->remainDataSize = feedingSize - wSize;
            bsf->remainOffset   += wSize;
        }
        else {
            bsf->remainData     = NULL;
            bsf->remainDataSize = 0;
            bsf->remainOffset   = 0;
        }

        leftSize = wSize;
        if ((wrPtr+wSize) >= (base+size)) {
            PhysicalAddress endAddr = base+size;
            rightSize = endAddr-wrPtr;
            leftSize  = (wrPtr+wSize) - endAddr;
            if (rightSize > 0) {
                JpuWriteMem(wrPtr, ptr, rightSize, (int)endian);
            }
            wrPtr = base;
        }

        JpuWriteMem(wrPtr, ptr+rightSize, leftSize, (int)endian);
#ifdef DEBUG_INPUT_JPEG_COMPARE
        {
            Uint8*          puc2;

            puc2 = malloc(leftSize);
            JpuReadMem(wrPtr, puc2, leftSize, endian);
            if ( memcmp(ptr+rightSize, puc2, leftSize) )
                JLOG(ERR, "input jpg data differ %x, %d(%x)\n", ptr+rightSize, leftSize, leftSize);
            else
                JLOG(ERR, "input jpg data match\n", ptr+rightSize, leftSize, leftSize);
        }
#endif
        JPU_DecUpdateBitstreamBuffer(handle, wSize);
    }

    if ((TRUE == chunk.eos) && (bsf->remainDataSize == 0 /* there is no remain data to be fed more*/)) {
        JPU_DecUpdateBitstreamBuffer(handle, 0);
    }

    if ((TRUE == chunk.eos) && (bsf->remainDataSize == 0 /* there is no remain data to be fed more*/)) {
        bsf->eos = TRUE; 
    }
    else {
        bsf->eos = FALSE;
    }
    if (NULL == bsf->remainData) 
        free(chunk.data);

    return feedingSize;
}

BOOL BitstreamFeeder_IsEos(BSFeeder feeder)
{
    BitstreamFeeder* bsf = (BitstreamFeeder*)feeder;

    if (bsf == NULL) {
        JLOG(ERR, "%s:%d Null handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    return bsf->eos;
}

BOOL BitstreamFeeder_Destroy(BSFeeder feeder)
{
    BitstreamFeeder* bsf = (BitstreamFeeder*)feeder;

    if (bsf == NULL) {
        return FALSE;
    }

    switch (bsf->method) {
    case FEEDING_METHOD_FIXED_SIZE:
        BSFeederFixedSize_Destroy(bsf->actualFeeder);
        break;
#if 0//ndef PLATFORM_NON_OS
    case FEEDING_METHOD_FRAME_SIZE:
        BSFeederFrameSize_Destroy(bsf->actualFeeder);
        break;
#endif
    default:
        JLOG(ERR, "%s:%d Invalid method(%d)\n", __FUNCTION__, __LINE__, bsf->method);
        break;
    }

    if (bsf->remainData) {
        free(bsf->remainData);
    }

    free(bsf);

    return TRUE;
}

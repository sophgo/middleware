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
#include <stdio.h>
#include <time.h>
#include "main_helper.h"

#define DEFAULT_FEEDING_SIZE    0x20000         /* 128KBytes */

typedef struct FeederFixedContext {
    FILE*   fp;
    Uint32          feedingSize;
    BOOL    eos;
} FeederFixedContext;

void* BSFeederFixedSize_Create(const char* path)
{
    FILE*                fp = NULL;
    FeederFixedContext*  context=NULL;

    if ((fp=fopen(path, "rb")) == NULL) {
        JLOG(ERR, "%s:%d failed to open %s\n", __FUNCTION__, __LINE__, path);
        return NULL;
    }

    context = (FeederFixedContext*)malloc(sizeof(FeederFixedContext));
    if (context == NULL) {
        JLOG(ERR, "%s:%d failed to allocate memory\n", __FUNCTION__, __LINE__);
        fclose(fp);
        return NULL;
    }

    context->fp          = fp;
    context->feedingSize = DEFAULT_FEEDING_SIZE;
    context->eos         = FALSE;

    return (void*)context;
}

//lint -e482
BOOL BSFeederFixedSize_Destroy(void* feeder)
{
    FeederFixedContext* context = (FeederFixedContext*)feeder;

    if (context == NULL) {
        JLOG(ERR, "%s:%d Null handle\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (context->fp) 
        fclose(context->fp);

    free(context);

    return TRUE;
}
//int +e482

Int32 BSFeederFixedSize_Act(void* feeder, BSChunk* chunk)
{
    FeederFixedContext* context = (FeederFixedContext*)feeder;
    Uint32              nRead;
    Uint32              size;
    Uint32          feedingSize;

    if (context == NULL) {
        JLOG(ERR, "%s:%d Null handle\n", __FUNCTION__, __LINE__);
        return 0;
    }

    if (context->eos == TRUE) {
        chunk->eos = TRUE;
        return 0;
    }

    feedingSize = context->feedingSize;
    size = (chunk->size < feedingSize) ? chunk->size : feedingSize;
    do {
        nRead = fread(chunk->data, 1, size, context->fp);
        if ((Int32)nRead < 0) {
            JLOG(ERR, "%s:%d failed to read bitstream\n", __FUNCTION__, __LINE__);
            return 0;
        } 
        else if (nRead < size) {
            context->eos = TRUE;
            chunk->eos   = TRUE;
        }
    } while (FALSE);

    return nRead;
}



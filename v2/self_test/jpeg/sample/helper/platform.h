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

#ifndef __JPU_PLATFORM_H__
#define __JPU_PLATFORM_H__

#include "jputypes.h"
/************************************************************************/
/* JpuMutex                                                                */
/************************************************************************/
typedef void*   JpuMutex;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
extern void MSleep(Uint32);

extern JpuMutex JpuMutex_Create(
    void
    );

extern void JpuMutex_Destroy(
    JpuMutex   handle
    );

extern BOOL JpuMutex_Lock(
    JpuMutex   handle
    );

extern BOOL JpuMutex_Unlock(
    JpuMutex   handle
    );

Uint32 GetRandom(
    Uint32   start,
    Uint32   end
    );
#ifdef __cplusplus
}
#endif /* __cplusplus */

/************************************************************************/
/* JpuThread                                                               */
/************************************************************************/
typedef void*   JpuThread;
typedef void(*JpuThreadRunner)(void*);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern JpuThread JpuThread_Create(
    JpuThreadRunner    func,
    void*           arg
    );

extern BOOL JpuThread_Join(
    JpuThread thread
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __JPU_PLATFORM_H__ */


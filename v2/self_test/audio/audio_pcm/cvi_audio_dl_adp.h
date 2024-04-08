#ifndef __CVI_AUDIO_DL_ADP_H__
#define __CVI_AUDIO_DL_ADP_H__
#if defined(__CV181X__) || defined(__CV180X__) || defined(__CV186X__)
#include <linux/cvi_type.h>
#else
#include "cvi_type.h"
#endif
CVI_S32 CVI_Audio_Dlpath(CVI_CHAR *pChLibPath);

CVI_S32 CVI_Audio_Dlopen(CVI_VOID **pLibhandle, CVI_CHAR *pChLibName);

CVI_S32 CVI_Audio_Dlsym(CVI_VOID **pFunchandle, CVI_VOID *Libhandle,
			CVI_CHAR *pChFuncName);

CVI_S32 CVI_Audio_Dlclose(CVI_VOID *Libhandle);

#endif

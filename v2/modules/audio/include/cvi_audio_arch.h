#ifndef __CVI_AUDIO_ARCH_H__
#define __CVI_AUDIO_ARCH_H__
#ifdef __cplusplus
extern "C" {
#endif
//already known chip
#if defined(ARCH_CV183X) || defined(ARCH_CV182X)
#include "cvi_defines.h"
#include "cvi_type.h"
#include "cvi_common.h"
#else

//new chip
#if defined(__CV181X__) || defined(__CV180X__) || defined(__CV186X__)
#include <linux/cvi_defines.h>
#include <linux/cvi_type.h>
#include <linux/cvi_common.h>
#else
//unknown chip
#include "cvi_defines.h"
#include "cvi_type.h"
#include "cvi_common.h"
#endif
#endif
#ifdef __cplusplus
}
#endif
#endif

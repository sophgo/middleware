#ifndef __RGN_UT_H__
#define __RGN_UT_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/prctl.h>
#include <sys/time.h>

#include <cvi_common.h>
#include <cvi_comm_vpss.h>
#include <cvi_comm_vb.h>
#include "cvi_region.h"
#include "cvi_sys.h"
#include "cvi_vpss.h"
#include "cvi_buffer.h"
#include "cvi_vo.h"
#include "cvi_vb.h"
#include "fontmod.h"
#include "rgn_ut_fun.h"

#define S_CTRL_PTR(_fd, _cfg, _ioctl)\
	do {\
		struct rgn_ext_control ec1;\
		memset(&ec1, 0, sizeof(ec1));\
		ec1.id = _ioctl;\
		ec1.ptr1 = (void *)_cfg;\
		if (ioctl(_fd, RGN_IOC_S_CTRL, &ec1) < 0) {\
			fprintf(stderr, "RGN_IOC_S_CTRL - %s NG\n", __func__);\
			return -1;\
		} \
		return 0;\
	} while (0)

#define G_CTRL_PTR(_fd, _cfg, _ioctl)\
	do {\
		struct rgn_ext_control ec1;\
		memset(&ec1, 0, sizeof(ec1));\
		ec1.id = _ioctl;\
		ec1.ptr1 = (void *)_cfg;\
		if (ioctl(_fd, RGN_IOC_G_CTRL, &ec1) < 0) {\
			fprintf(stderr, "RGN_IOC_G_CTRL - %s NG\n", __func__);\
			return -1;\
		} \
	} while (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __RGN_UT_H__ */

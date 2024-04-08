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

#include <cvi_base.h>
#include <linux/cvi_common.h>
#include "sample_comm.h"
#include "cvi_sys.h"
#include "cvi_osdc.h"
#include <linux/cvi_type.h>
#include <linux/cvi_vip.h>
#include <linux/rgn_uapi.h>

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

int _sc_set_rgn(int fd, struct cvi_rgn_cfg *cfg);
int _disp_set_rgn(int fd, struct cvi_rgn_cfg *cfg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __RGN_UT_H__ */

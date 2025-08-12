#ifndef __VI_UT_H__
#define __VI_UT_H__

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
#include <inttypes.h>

#include <cvi_common.h>
#include "sample_comm.h"
#include "cvi_sns_ctrl.h"
#include "cvi_sys.h"
#include "cvi_base.h"
#include <cvi_type.h>

#include <vi_uapi.h>
#include <vi_isp.h>
#include <vi_tun_cfg.h>

#if defined(__CV181X__)
#define CHIP_TYPE "cv181x_vi"
#elif defined(__CV180X__)
#define CHIP_TYPE "cv180x_vi"
#elif defined(__CV186X__)
#define CHIP_TYPE "soph_vi"
#endif

#define VI_UT_PRT(fmt...)                                 \
	do {                                              \
		printf("[%s]-%d: ", __func__, __LINE__);  \
		printf(fmt);                              \
	} while (0)

#define S_CTRL_VALUE(_fd, _cfg, _ioctl)\
	do {\
		struct vi_ext_control ec1;\
		memset(&ec1, 0, sizeof(ec1));\
		ec1.id = _ioctl;\
		ec1.value = _cfg;\
		if (ioctl(_fd, VI_IOC_S_CTRL, &ec1) < 0) {\
			fprintf(stderr, "VI_IOC_S_CTRL - %s NG\n", __func__);\
			return -1;\
		} \
		return 0;\
	} while (0)

#define S_CTRL_VALUE64(_fd, _cfg, _ioctl)\
	do {\
		struct vi_ext_control ec1;\
		memset(&ec1, 0, sizeof(ec1));\
		ec1.id = _ioctl;\
		ec1.value64 = _cfg;\
		if (ioctl(_fd, VI_IOC_S_CTRL, &ec1) < 0) {\
			fprintf(stderr, "VI_IOC_S_CTRL - %s NG\n", __func__);\
			return -1;\
		} \
		return 0;\
	} while (0)

#define S_CTRL_PTR(_fd, _cfg, _ioctl)\
	do {\
		struct vi_ext_control ec1;\
		memset(&ec1, 0, sizeof(ec1));\
		ec1.id = _ioctl;\
		ec1.ptr = (void *)_cfg;\
		if (ioctl(_fd, VI_IOC_S_CTRL, &ec1) < 0) {\
			fprintf(stderr, "VI_IOC_S_CTRL - %s NG\n", __func__);\
			return -1;\
		} \
		return 0;\
	} while (0)

#define G_CTRL_VALUE(_fd, _out, _ioctl)\
	do {\
		struct vi_ext_control ec1;\
		memset(&ec1, 0, sizeof(ec1));\
		ec1.id = _ioctl;\
		ec1.value = 0;\
		if (ioctl(_fd, VI_IOC_G_CTRL, &ec1) < 0) {\
			fprintf(stderr, "VI_IOC_G_CTRL - %s NG\n", __func__);\
			return -1;\
		} \
		*_out = ec1.value;\
	} while (0)

#define G_CTRL_PTR(_fd, _cfg, _ioctl)\
	do {\
		struct vi_ext_control ec1;\
		memset(&ec1, 0, sizeof(ec1));\
		ec1.id = _ioctl;\
		ec1.ptr = (void *)_cfg;\
		if (ioctl(_fd, VI_IOC_G_CTRL, &ec1) < 0) {\
			fprintf(stderr, "VI_IOC_G_CTRL - %s NG\n", __func__);\
			return -1;\
		} \
	} while (0)

#define QBUF_NUM	2

typedef struct _VI_UT_CTX {
	CVI_S32 fd;
	SIZE_S stSize[VI_MAX_DEV_NUM];
	PIXEL_FORMAT_E enPixelFormat;
	COMPRESS_MODE_E enCompressMode;
	CVI_U32 u32Align;
	CVI_U32 u32Case;
	VB_BLK blk[QBUF_NUM];
	char filepath_le[128];
	char filepath_se[128];
	char binpath[128];
	pid_t pid;
	CVI_BOOL isInotityExit;
	pthread_t inotify_thread;

	CVI_U32 is_enable_sensor	: 1;
	CVI_U32 is_be_online		: 1;
	CVI_U32 is_post_online		: 1;
	CVI_U32 is_patgen_enable	: 1;
	CVI_U32 is_hdr_enable		: 1;
	CVI_U32 is_vpss_online		: 1;
	CVI_U32 is_get_dpcm_mode	: 1;
	//for auto test use
	CVI_U32 is_dpcm_on		: 1;
	CVI_U32 is_hdr_on		: 1;
	CVI_U32 is_dump_yuv		: 1;
	CVI_U32 is_dump_raw		: 1;
	CVI_U32 is_test_mode		: 1;
	CVI_U32 is_custom_flow		: 1;
	CVI_U32 is_crop_yuv		: 1;
	CVI_U32 is_crop_raw		: 1;
	CVI_U32 is_3dnr_off		: 1;
	CVI_U32 is_fpga_ip_test		: 1;
} VI_UT_CTX;

extern VI_UT_CTX vi_ut_ctx;
extern SAMPLE_VI_CONFIG_S stViConfig;
extern bool g_is_patgen;

CVI_S32 vi_ut_plat_sys_init(void);
CVI_S32 vi_ut_sys_config_online_mode(void);
CVI_S32 vi_ut_vpss_config_online_mode(void);
CVI_S32 vi_ut_save_frame2file(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);
CVI_S32 vi_ut_plat_vi_startchn(SAMPLE_VI_CONFIG_S *pstViConfig);
CVI_S32 vi_ut_plat_vi_init(void);
CVI_S32 vi_ut_plat_vi_deinit(void);

CVI_S32 vi_ioctl_test(int fd);
CVI_S32 vi_raw_replay_test(void);
CVI_S32 vi_raw_replay_manual_test(void);

CVI_S32 vi_ut_get_pipe_frame(CVI_U8 dev);
CVI_S32 vi_ut_get_chn_frame(CVI_U8 chn);
CVI_S32 vi_ut_get_vpss_chn_frame(CVI_U8 chn);
long vi_ut_diff_in_us(struct timespec t1, struct timespec t2);
CVI_S32 vi_ut_set_rawdump_crop(CVI_U32 dev, VIDEO_FRAME_INFO_S *pstVideoFrame);

CVI_S32 vi_fpga_dosomething(void);
CVI_S32 vi_fpga_dump_yuv(CVI_U8 chn);
CVI_S32 vi_fpga_dump_raw(CVI_U8 pipe);
CVI_S32 vi_fpga_parse_arg(int argc, char *argv[]);
CVI_S32 vi_ut_slt(char *filename);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __VI_UT_H__ */

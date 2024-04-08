#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/param.h>
#include <inttypes.h>

#include "sample_comm.h"
#include "../../modules/sys/include/cvi_base.h"

#if 0
CVI_S32 CVI_SYS_FIFO_TEST(void)
{
	struct vbq test1, test2;
	VB_S *vb;

	FIFO_INIT(&test1, 1);
	FIFO_INIT(&test2, 5);

	if (FIFO_CAPACITY(&test1) != 1) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "FIFO_CAPACITY 1 NG.\n");
		return CVI_FAILURE;
	}

	if (FIFO_CAPACITY(&test2) != 5) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "FIFO_CAPACITY 2 NG.\n");
		return CVI_FAILURE;
	}

	if (FIFO_SIZE(&test1) != 0) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "FIFO_SIZE 1 NG.\n");
		return CVI_FAILURE;
	}

	if (FIFO_SIZE(&test2) != 0) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "FIFO_SIZE 2 NG.\n");
		return CVI_FAILURE;
	}

	if (FIFO_EMPTY(&test1) != true) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "FIFO_EMPTY 1 NG.\n");
		return CVI_FAILURE;
	}

	if (FIFO_EMPTY(&test2) != true) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "FIFO_EMPTY 2 NG.\n");
		return CVI_FAILURE;
	}

	vb = malloc(sizeof(*vb));
	vb->magic = 1;
	FIFO_PUSH(&test1, vb);

	if (FIFO_FULL(&test1) != true) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "FIFO_FULL 1 NG.\n");
		return CVI_FAILURE;
	}

	vb = malloc(sizeof(*vb));
	vb->magic = 2;
	FIFO_PUSH(&test2, vb);

	if (FIFO_FULL(&test2) == true) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "FIFO_FULL 2 NG.\n");
		return CVI_FAILURE;
	}

	if (FIFO_SIZE(&test1) != 1) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "FIFO_SIZE 3 NG.\n");
		return CVI_FAILURE;
	}

	if (FIFO_SIZE(&test2) != 1) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "FIFO_SIZE 4 NG.\n");
		return CVI_FAILURE;
	}

	FIFO_POP(&test1, &vb);
	if (vb->magic != 1) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "FIFO_POP content NG.\n");
		return CVI_FAILURE;
	}
	free(vb);

	if (FIFO_SIZE(&test1) != 0) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "FIFO_SIZE 5 NG.\n");
		return CVI_FAILURE;
	}

	vb = malloc(sizeof(*vb));
	vb->magic = 3;
	FIFO_PUSH(&test2, vb);

	FIFO_GET_FRONT(&test2, &vb);
	if (vb->magic != 2) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "FIFO_GET_FRONT NG.\n");
		return CVI_FAILURE;
	}

	FIFO_GET_TAIL(&test2, &vb);
	if (vb->magic != 3) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "FIFO_GET_TAIL NG.\n");
		return CVI_FAILURE;
	}

	FIFO_POP(&test2, &vb);
	if (vb->magic != 2) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "FIFO_POP content NG.\n");
		return CVI_FAILURE;
	}
	free(vb);

	if (FIFO_SIZE(&test2) != 1) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "FIFO_SIZE 6 NG.\n");
		return CVI_FAILURE;
	}

	FIFO_POP(&test2, &vb);
	if (vb->magic != 3) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "FIFO_POP content NG.\n");
		return CVI_FAILURE;
	}
	free(vb);

	if (FIFO_SIZE(&test2) != 0) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "FIFO_SIZE 7 NG.\n");
		return CVI_FAILURE;
	}

	FIFO_EXIT(&test1);
	FIFO_EXIT(&test2);
	return CVI_SUCCESS;
}
#endif

int _compare_vbconfig(VB_CAL_CONFIG_S *pstVbCfgNew, VB_CAL_CONFIG_S *pstVbCfgGolden)
{
	return  (pstVbCfgGolden->u32VBSize != pstVbCfgNew->u32VBSize) ||
		(pstVbCfgGolden->u32MainStride != pstVbCfgNew->u32MainStride) ||
		(pstVbCfgGolden->u32CStride != pstVbCfgNew->u32CStride) ||
		(pstVbCfgGolden->u32MainSize != pstVbCfgNew->u32MainSize) ||
		(pstVbCfgGolden->u32MainYSize != pstVbCfgNew->u32MainYSize) ||
		(pstVbCfgGolden->u32MainCSize != pstVbCfgNew->u32MainCSize) ||
		(pstVbCfgGolden->u32VBSize != pstVbCfgNew->u32VBSize) ||
		(pstVbCfgGolden->plane_num != pstVbCfgNew->plane_num);
}

void _print_vbconfig(VB_CAL_CONFIG_S *pstVbCalConfig)
{
	CVI_TRACE_LOG(CVI_DBG_ERR, ".u32VBSize = %d,\n", pstVbCalConfig->u32VBSize);
	CVI_TRACE_LOG(CVI_DBG_ERR, ".u32MainStride = %d,\n", pstVbCalConfig->u32MainStride);
	CVI_TRACE_LOG(CVI_DBG_ERR, ".u32CStride = %d,\n", pstVbCalConfig->u32CStride);
	CVI_TRACE_LOG(CVI_DBG_ERR, ".u32MainSize = %d,\n", pstVbCalConfig->u32MainSize);
	CVI_TRACE_LOG(CVI_DBG_ERR, ".u32MainYSize = %d,\n", pstVbCalConfig->u32MainYSize);
	CVI_TRACE_LOG(CVI_DBG_ERR, ".u32MainCSize = %d,\n", pstVbCalConfig->u32MainCSize);
	CVI_TRACE_LOG(CVI_DBG_ERR, ".u16AddrAlign = %d,\n", pstVbCalConfig->u16AddrAlign);
	CVI_TRACE_LOG(CVI_DBG_ERR, ".plane_num = %d,\n", pstVbCalConfig->plane_num);
}

CVI_S32 CVI_SYS_FMT_TEST(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CAL_CONFIG_S stVbCalConfig;
#ifdef ARCH_CV183X
	VB_CAL_CONFIG_S stVbCalConfig_YUV420_1920x1080 = {
		.u32VBSize = 3122688,
		.u32MainStride = 1920,
		.u32CStride = 960,
		.u32MainSize = 3110400,
		.u32MainYSize = 2073600,
		.u32MainCSize = 518400,
		.u16AddrAlign = 4096,
		.plane_num = 3,
	};
	VB_CAL_CONFIG_S stVbCalConfig_YUV422_1920x1080 = {
		.u32VBSize = 4159488,
		.u32MainStride = 1920,
		.u32CStride = 960,
		.u32MainSize = 4147200,
		.u32MainYSize = 2073600,
		.u32MainCSize = 1036800,
		.u16AddrAlign = 4096,
		.plane_num = 3,
	};
	VB_CAL_CONFIG_S stVbCalConfig_NV21_1920x1080 = {
		.u32VBSize = 3110400,
		.u32MainStride = 1920,
		.u32CStride = 1920,
		.u32MainSize = 3110400,
		.u32MainYSize = 2073600,
		.u32MainCSize = 1036800,
		.u16AddrAlign = 32,
		.plane_num = 2,
	};
	VB_CAL_CONFIG_S stVbCalConfig_NV16_1920x1080 = {
		.u32VBSize = 4147200,
		.u32MainStride = 1920,
		.u32CStride = 1920,
		.u32MainSize = 4147200,
		.u32MainYSize = 2073600,
		.u32MainCSize = 2073600,
		.u16AddrAlign = 32,
		.plane_num = 2,
	};
	VB_CAL_CONFIG_S stVbCalConfig_YUYV_1920x1080 = {
		.u32VBSize = 4147200,
		.u32MainStride = 3840,
		.u32CStride = 0,
		.u32MainSize = 4147200,
		.u32MainYSize = 4147200,
		.u32MainCSize = 0,
		.u16AddrAlign = 32,
		.plane_num = 1,
	};
	VB_CAL_CONFIG_S stVbCalConfig_RGB_1920x1080 = {
		.u32VBSize = 6220800,
		.u32MainStride = 5760,
		.u32CStride = 0,
		.u32MainSize = 6220800,
		.u32MainYSize = 6220800,
		.u32MainCSize = 0,
		.u16AddrAlign = 32,
		.plane_num = 1,
	};
	VB_CAL_CONFIG_S stVbCalConfig_BGRP_1920x1080 = {
		.u32VBSize = 6233088,
		.u32MainStride = 1920,
		.u32CStride = 1920,
		.u32MainSize = 6220800,
		.u32MainYSize = 2073600,
		.u32MainCSize = 2073600,
		.u16AddrAlign = 4096,
		.plane_num = 3,
	};
	VB_CAL_CONFIG_S stVbCalConfig_YUV420_800x600 = {
		.u32VBSize = 761088,
		.u32MainStride = 832,
		.u32CStride = 416,
		.u32MainSize = 748800,
		.u32MainYSize = 499200,
		.u32MainCSize = 124800,
		.u16AddrAlign = 4096,
		.plane_num = 3,
	};
	VB_CAL_CONFIG_S stVbCalConfig_HSVP_1100x400 = {
		.u32VBSize = 1356288,
		.u32MainStride = 1120,
		.u32CStride = 1120,
		.u32MainSize = 1344000,
		.u32MainYSize = 448000,
		.u32MainCSize = 448000,
		.u16AddrAlign = 4096,
		.plane_num = 3,
	};
	VB_CAL_CONFIG_S stVbCalConfig_ARGB1555_400x300 = {
		.u32VBSize = 240000,
		.u32MainStride = 800,
		.u32CStride = 0,
		.u32MainSize = 240000,
		.u32MainYSize = 240000,
		.u32MainCSize = 0,
		.u16AddrAlign = 32,
		.plane_num = 1,
	};
	VB_CAL_CONFIG_S stVbCalConfig_ARGB4444_200x200 = {
		.u32VBSize = 83200,
		.u32MainStride = 416,
		.u32CStride = 0,
		.u32MainSize = 83200,
		.u32MainYSize = 83200,
		.u32MainCSize = 0,
		.u16AddrAlign = 32,
		.plane_num = 1,
	};
#elif defined(ARCH_CV182X) || defined(__CV181X__) || defined(__CV180X__) || defined(__CV186X__)
	VB_CAL_CONFIG_S stVbCalConfig_YUV420_1920x1080 = {
		.u32VBSize = 3110400,
		.u32MainStride = 1920,
		.u32CStride = 960,
		.u32MainSize = 3110400,
		.u32MainYSize = 2073600,
		.u32MainCSize = 518400,
		.u16AddrAlign = 64,
		.plane_num = 3,
	};
	VB_CAL_CONFIG_S stVbCalConfig_YUV422_1920x1080 = {
		.u32VBSize = 4147200,
		.u32MainStride = 1920,
		.u32CStride = 960,
		.u32MainSize = 4147200,
		.u32MainYSize = 2073600,
		.u32MainCSize = 1036800,
		.u16AddrAlign = 64,
		.plane_num = 3,
	};
	VB_CAL_CONFIG_S stVbCalConfig_NV21_1920x1080 = {
		.u32VBSize = 3110400,
		.u32MainStride = 1920,
		.u32CStride = 1920,
		.u32MainSize = 3110400,
		.u32MainYSize = 2073600,
		.u32MainCSize = 1036800,
		.u16AddrAlign = 64,
		.plane_num = 2,
	};
	VB_CAL_CONFIG_S stVbCalConfig_NV16_1920x1080 = {
		.u32VBSize = 4147200,
		.u32MainStride = 1920,
		.u32CStride = 1920,
		.u32MainSize = 4147200,
		.u32MainYSize = 2073600,
		.u32MainCSize = 2073600,
		.u16AddrAlign = 64,
		.plane_num = 2,
	};
	VB_CAL_CONFIG_S stVbCalConfig_YUYV_1920x1080 = {
		.u32VBSize = 4147200,
		.u32MainStride = 3840,
		.u32CStride = 0,
		.u32MainSize = 4147200,
		.u32MainYSize = 4147200,
		.u32MainCSize = 0,
		.u16AddrAlign = 64,
		.plane_num = 1,
	};
	VB_CAL_CONFIG_S stVbCalConfig_RGB_1920x1080 = {
		.u32VBSize = 6220800,
		.u32MainStride = 5760,
		.u32CStride = 0,
		.u32MainSize = 6220800,
		.u32MainYSize = 6220800,
		.u32MainCSize = 0,
		.u16AddrAlign = 64,
		.plane_num = 1,
	};
	VB_CAL_CONFIG_S stVbCalConfig_BGRP_1920x1080 = {
		.u32VBSize = 6220800,
		.u32MainStride = 1920,
		.u32CStride = 1920,
		.u32MainSize = 6220800,
		.u32MainYSize = 2073600,
		.u32MainCSize = 2073600,
		.u16AddrAlign = 64,
		.plane_num = 3,
	};
	VB_CAL_CONFIG_S stVbCalConfig_YUV420_800x600 = {
		.u32VBSize = 806400,
		.u32MainStride = 896,
		.u32CStride = 448,
		.u32MainSize = 806400,
		.u32MainYSize = 537600,
		.u32MainCSize = 134400,
		.u16AddrAlign = 64,
		.plane_num = 3,
	};
	VB_CAL_CONFIG_S stVbCalConfig_HSVP_1100x400 = {
		.u32VBSize = 1382400,
		.u32MainStride = 1152,
		.u32CStride = 1152,
		.u32MainSize = 1382400,
		.u32MainYSize = 460800,
		.u32MainCSize = 460800,
		.u16AddrAlign = 64,
		.plane_num = 3,
	};
	VB_CAL_CONFIG_S stVbCalConfig_ARGB1555_400x300 = {
		.u32VBSize = 249600,
		.u32MainStride = 832,
		.u32CStride = 0,
		.u32MainSize = 249600,
		.u32MainYSize = 249600,
		.u32MainCSize = 0,
		.u16AddrAlign = 64,
		.plane_num = 1,
	};
	VB_CAL_CONFIG_S stVbCalConfig_ARGB4444_200x200 = {
		.u32VBSize = 89600,
		.u32MainStride = 448,
		.u32CStride = 0,
		.u32MainSize = 89600,
		.u32MainYSize = 89600,
		.u32MainCSize = 0,
		.u16AddrAlign = 64,
		.plane_num = 1,
	};
#endif

	COMMON_GetPicBufferConfig(1920, 1080, PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (_compare_vbconfig(&stVbCalConfig, &stVbCalConfig_YUV420_1920x1080)) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "YUV420 1920x1080 vb config NG:\n");
		_print_vbconfig(&stVbCalConfig);
		CVI_TRACE_LOG(CVI_DBG_ERR, "Expected config values:\n");
		_print_vbconfig(&stVbCalConfig_YUV420_1920x1080);
		s32Ret = CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(1920, 1080, PIXEL_FORMAT_YUV_PLANAR_422, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (_compare_vbconfig(&stVbCalConfig, &stVbCalConfig_YUV422_1920x1080)) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "YUV422 1920x1080 vb config NG:\n");
		_print_vbconfig(&stVbCalConfig);
		CVI_TRACE_LOG(CVI_DBG_ERR, "Expected config values:\n");
		_print_vbconfig(&stVbCalConfig_YUV422_1920x1080);
		s32Ret = CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(1920, 1080, PIXEL_FORMAT_NV21, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (_compare_vbconfig(&stVbCalConfig, &stVbCalConfig_NV21_1920x1080)) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "NV21 1920x1080 vb config NG:\n");
		_print_vbconfig(&stVbCalConfig);
		CVI_TRACE_LOG(CVI_DBG_ERR, "Expected config values:\n");
		_print_vbconfig(&stVbCalConfig_NV21_1920x1080);
		s32Ret = CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(1920, 1080, PIXEL_FORMAT_NV16, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (_compare_vbconfig(&stVbCalConfig, &stVbCalConfig_NV16_1920x1080)) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "NV16 1920x1080 vb config NG:\n");
		_print_vbconfig(&stVbCalConfig);
		CVI_TRACE_LOG(CVI_DBG_ERR, "Expected config values:\n");
		_print_vbconfig(&stVbCalConfig_NV16_1920x1080);
		s32Ret = CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(1920, 1080, PIXEL_FORMAT_YUYV, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (_compare_vbconfig(&stVbCalConfig, &stVbCalConfig_YUYV_1920x1080)) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "YUYV 1920x1080 vb config NG:\n");
		_print_vbconfig(&stVbCalConfig);
		CVI_TRACE_LOG(CVI_DBG_ERR, "Expected config values:\n");
		_print_vbconfig(&stVbCalConfig_YUYV_1920x1080);
		s32Ret = CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(1920, 1080, PIXEL_FORMAT_RGB_888, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (_compare_vbconfig(&stVbCalConfig, &stVbCalConfig_RGB_1920x1080)) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "BGR 1920x1080 vb config NG:\n");
		_print_vbconfig(&stVbCalConfig);
		CVI_TRACE_LOG(CVI_DBG_ERR, "Expected config values:\n");
		_print_vbconfig(&stVbCalConfig_RGB_1920x1080);
		s32Ret = CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(1920, 1080, PIXEL_FORMAT_BGR_888_PLANAR, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (_compare_vbconfig(&stVbCalConfig, &stVbCalConfig_BGRP_1920x1080)) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "BGR_Planar 1920x1080 vb config NG:\n");
		_print_vbconfig(&stVbCalConfig);
		CVI_TRACE_LOG(CVI_DBG_ERR, "Expected config values:\n");
		_print_vbconfig(&stVbCalConfig_BGRP_1920x1080);
		s32Ret = CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(800, 600, PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (_compare_vbconfig(&stVbCalConfig, &stVbCalConfig_YUV420_800x600)) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "YUV420 800x600 vb config NG:\n");
		_print_vbconfig(&stVbCalConfig);
		CVI_TRACE_LOG(CVI_DBG_ERR, "Expected config values:\n");
		_print_vbconfig(&stVbCalConfig_YUV420_800x600);
		s32Ret = CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(1100, 400, PIXEL_FORMAT_HSV_888_PLANAR, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (_compare_vbconfig(&stVbCalConfig, &stVbCalConfig_HSVP_1100x400)) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "HSV Planar 1100x400 vb config NG:\n");
		_print_vbconfig(&stVbCalConfig);
		CVI_TRACE_LOG(CVI_DBG_ERR, "Expected config values:\n");
		_print_vbconfig(&stVbCalConfig_HSVP_1100x400);
		s32Ret = CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(400, 300, PIXEL_FORMAT_ARGB_1555, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (_compare_vbconfig(&stVbCalConfig, &stVbCalConfig_ARGB1555_400x300)) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "ARGB1555 400x300 vb config NG:\n");
		_print_vbconfig(&stVbCalConfig);
		CVI_TRACE_LOG(CVI_DBG_ERR, "Expected config values:\n");
		_print_vbconfig(&stVbCalConfig_ARGB1555_400x300);
		s32Ret = CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(200, 200, PIXEL_FORMAT_ARGB_4444, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (_compare_vbconfig(&stVbCalConfig, &stVbCalConfig_ARGB4444_200x200)) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "ARGB4444_00x200 vb config NG:\n");
		_print_vbconfig(&stVbCalConfig);
		CVI_TRACE_LOG(CVI_DBG_ERR, "Expected config values:\n");
		_print_vbconfig(&stVbCalConfig_ARGB4444_200x200);
		s32Ret = CVI_FAILURE;
	}

	return s32Ret;
}

CVI_S32 _VPSS_INIT(void)
{
	VPSS_GRP           VpssGrp = 0;
	VPSS_GRP_ATTR_S    stVpssGrpAttr;
	VPSS_CHN           VpssChn        = VPSS_CHN0;
	CVI_BOOL           abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
	CVI_S32 s32Ret = CVI_SUCCESS;

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat                  = PIXEL_FORMAT_RGB_888_PLANAR;
	stVpssGrpAttr.u32MaxW                        = 256;
	stVpssGrpAttr.u32MaxH                        = 256;

	astVpssChnAttr[VpssChn].u32Width                    = 256;
	astVpssChnAttr[VpssChn].u32Height                   = 256;
	astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat               = PIXEL_FORMAT_RGB_888_PLANAR;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth                    = 1;
	astVpssChnAttr[VpssChn].bMirror                     = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip                       = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		goto error;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		goto error;
	}

	return s32Ret;
error:
	return s32Ret;
}

void _memcpy_test(void *src, void *dst, size_t size, CVI_U32 *duration)
{
	struct timespec time[2];

	clock_gettime(CLOCK_MONOTONIC, &time[0]);
	memcpy(src, dst, size);
	clock_gettime(CLOCK_MONOTONIC, &time[1]);
	*duration = get_diff_in_us(time[0], time[1]);
}

CVI_S32 _flush_test(SIZE_S stSize, CVI_U32 bufLen, bool isFlush)
{
	VIDEO_FRAME_INFO_S stVideoFrame, stVideoFrameOut;
	CVI_VOID *pVirAddr = NULL, *pVirAddrOut = NULL;

	for (int i = 0; i < 1000; ++i) {
		// prepare input buffer
		SAMPLE_COMM_PrepareFrame(stSize, PIXEL_FORMAT_RGB_888_PLANAR, &stVideoFrame);
		pVirAddr = CVI_SYS_MmapCache(stVideoFrame.stVFrame.u64PhyAddr[0], bufLen);
		if (pVirAddr == CVI_NULL) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_Mmap NG.\n");
			return CVI_FAILURE;
		}
		memset(pVirAddr, i, 0x4000);

		// flush to make sure data into dram
		if (isFlush)
			CVI_SYS_IonFlushCache(stVideoFrame.stVFrame.u64PhyAddr[0], pVirAddr, 0x4000);

		// use hw dma to move
		CVI_VPSS_SendFrame(0, &stVideoFrame, -1);
		CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrame.stVFrame.u64PhyAddr[0]));
		CVI_VPSS_GetChnFrame(0, 0, &stVideoFrameOut, 1000);

		// check if flush work
		pVirAddrOut = CVI_SYS_MmapCache(stVideoFrameOut.stVFrame.u64PhyAddr[0], bufLen);
		if (pVirAddrOut == CVI_NULL) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_Mmap NG.\n");
			CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrameOut);
			return CVI_FAILURE;
		}
		//printf("%#x %#x\n", *(CVI_U32 *)pVirAddr, *(((CVI_U32 *)pVirAddr) + 1));
		for (int j = 0; j < 1000; ++j) {
			if (*((CVI_U32 *)pVirAddrOut + j) != ((i & 0xff) * 0x01010101U)) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_IonFlushCache not work\n");
				CVI_TRACE_LOG(CVI_DBG_ERR, "---Flush test round(%d) offset(%d)---\n",
					      i, j * 4);
				CVI_TRACE_LOG(CVI_DBG_ERR, "phy-addr:   IN(%#"PRIx64") OUT(%#"PRIx64")\n",
					      stVideoFrame.stVFrame.u64PhyAddr[0],
					      stVideoFrameOut.stVFrame.u64PhyAddr[0]);
				CVI_TRACE_LOG(CVI_DBG_ERR, "SRC current(%#x) next64(%#x)\n",
					      *((CVI_U32 *)pVirAddr + j), *((CVI_U32 *)pVirAddr + j + 16));
				CVI_TRACE_LOG(CVI_DBG_ERR, "DST current(%#x) next64(%#x)\n",
					      *((CVI_U32 *)pVirAddrOut + j), *((CVI_U32 *)pVirAddrOut + j + 16));
				CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrameOut);
				return CVI_FAILURE;
			}
		}

		CVI_SYS_Munmap(pVirAddr, bufLen);
		CVI_SYS_Munmap(pVirAddrOut, bufLen);
		CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrameOut);
	}
	return CVI_SUCCESS;
}

CVI_S32 _invalidate_test(SIZE_S stSize, CVI_U32 bufLen, bool isInval)
{
	VIDEO_FRAME_INFO_S stVideoFrame, stVideoFrameOut;
	CVI_VOID *pVirAddr = NULL, *pVirAddrOut = NULL;

	for (int i = 0; i < 1000; ++i) {
		// prepare input buffer
		SAMPLE_COMM_PrepareFrame(stSize, PIXEL_FORMAT_RGB_888_PLANAR, &stVideoFrame);
		pVirAddr = CVI_SYS_MmapCache(stVideoFrame.stVFrame.u64PhyAddr[0], bufLen);
		if (pVirAddr == CVI_NULL) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_Mmap NG.\n");
			return CVI_FAILURE;
		}

		memset(pVirAddr, i, 0x4000);
		CVI_SYS_IonFlushCache(stVideoFrame.stVFrame.u64PhyAddr[0], pVirAddr, 0x4000);

		// prepare output buffer
		SAMPLE_COMM_PrepareFrame(stSize, PIXEL_FORMAT_RGB_888_PLANAR, &stVideoFrameOut);
		pVirAddrOut = CVI_SYS_MmapCache(stVideoFrameOut.stVFrame.u64PhyAddr[0], bufLen);
		if (pVirAddrOut == CVI_NULL) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_Mmap NG.\n");
			return CVI_FAILURE;
		}
		memset(pVirAddrOut, 0, 0x4000);
		// flush to make sure data into dram
		CVI_SYS_IonFlushCache(stVideoFrameOut.stVFrame.u64PhyAddr[0], pVirAddrOut, 0x4000);
		// update cache to new values
		for (int j = 0; j < 1000; ++j) {
			if (*((CVI_U32 *)pVirAddrOut + j) != 0) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "memset not zero.\n");
			}
		}

		// use hw dma to move
		CVI_VPSS_SendChnFrame(0, 0, &stVideoFrameOut, -1);
		CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameOut.stVFrame.u64PhyAddr[0]));
		CVI_VPSS_SendFrame(0, &stVideoFrame, -1);
		CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrame.stVFrame.u64PhyAddr[0]));
		CVI_VPSS_GetChnFrame(0, 0, &stVideoFrameOut, 1000);

		if (isInval)
			CVI_SYS_IonInvalidateCache(stVideoFrameOut.stVFrame.u64PhyAddr[0], pVirAddrOut, bufLen);

		// check if invalidate work
		//printf("%#x %#x\n", *(CVI_U32 *)pVirAddrOut, *(((CVI_U32 *)pVirAddrOut) + 1));
		for (int j = 0; j < 1000; ++j) {
			if (*((CVI_U32 *)pVirAddrOut + j) != ((i & 0xff) * 0x01010101U)) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_IonInvalCache not work\n");
				CVI_TRACE_LOG(CVI_DBG_ERR, "---Inval test round(%d) offset(%d)---\n",
					      i, j * 4);
				CVI_TRACE_LOG(CVI_DBG_ERR, "phy-addr:   IN(%#"PRIx64") OUT(%#"PRIx64")\n",
					      stVideoFrame.stVFrame.u64PhyAddr[0],
					      stVideoFrameOut.stVFrame.u64PhyAddr[0]);
				CVI_TRACE_LOG(CVI_DBG_ERR, "SRC current(%#x) next64(%#x)\n",
					      *((CVI_U32 *)pVirAddr + j), *((CVI_U32 *)pVirAddr + j + 16));
				CVI_TRACE_LOG(CVI_DBG_ERR, "DST current(%#x) next64(%#x)\n",
					      *((CVI_U32 *)pVirAddrOut + j), *((CVI_U32 *)pVirAddrOut + j + 16));
				CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrameOut);
				return CVI_FAILURE;
			}
		}

		CVI_SYS_Munmap(pVirAddr, bufLen);
		CVI_SYS_Munmap(pVirAddrOut, bufLen);
		CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrameOut);
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_ION_TEST(void)
{
	CVI_U64 u64PhyAddr = 0;
	CVI_VOID *pVirAddr = NULL;
	CVI_U8 *pu8TestBuf;
	CVI_U32 duration1, duration2, duration_cached, duration_non_cached;
	const CVI_U32 bufLen = 0x2F0000;
	VB_BLK blk;

	if (CVI_SYS_IonAlloc((CVI_U64 *)NULL, &pVirAddr, "sys_test", bufLen) ==
	    CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_IonAlloc NG. Null pointer\n");
		return CVI_FAILURE;
	}

	// test ion non-cached
	// --------------------------------------
	if (CVI_SYS_IonAlloc(&u64PhyAddr, &pVirAddr, "sys_test", bufLen) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_IonAlloc NG.\n");
		return CVI_FAILURE;
	}

	if (u64PhyAddr == 0 || pVirAddr == 0) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_IonAlloc NG. zero phy/vir address\n");
		return CVI_FAILURE;
	}

	pu8TestBuf = malloc(bufLen);
	_memcpy_test(pu8TestBuf, pVirAddr, bufLen, &duration1);
	_memcpy_test(pu8TestBuf, pVirAddr, bufLen, &duration_non_cached);
	free(pu8TestBuf);
	CVI_TRACE_LOG(CVI_DBG_WARN, "*non-cached memcpy(%d bytes) %d - %d\n", bufLen, duration1, duration_non_cached);
	if (CVI_SYS_IonFree(u64PhyAddr, pVirAddr) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_IonFree NG.\n");
		return CVI_FAILURE;
	}

	// test ion cached
	// --------------------------------------
	if (CVI_SYS_IonAlloc_Cached(&u64PhyAddr, &pVirAddr, "sys_test", bufLen) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_IonAlloc_Cached NG.\n");
		return CVI_FAILURE;
	}

	if (u64PhyAddr == 0 || pVirAddr == 0) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_IonAlloc NG. zero phy/vir address\n");
		return CVI_FAILURE;
	}

	pu8TestBuf = malloc(bufLen);
	_memcpy_test(pu8TestBuf, pVirAddr, bufLen, &duration1);
	_memcpy_test(pu8TestBuf, pVirAddr, bufLen, &duration_cached);
	free(pu8TestBuf);
	CVI_TRACE_LOG(CVI_DBG_WARN, "*cached memcpy(%d bytes)     %d - %d\n", bufLen, duration1, duration_cached);

	if (CVI_SYS_IonFree(u64PhyAddr, pVirAddr) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_IonFree NG.\n");
		return CVI_FAILURE;
	}

	if ((duration_cached * 2) > duration_non_cached) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "cached not work. cached(%dus) non-cached(%dus)\n",
			      duration_cached, duration_non_cached);
		return CVI_FAILURE;
	}

	// test mmap pool cached
	// --------------------------------------
	if (CVI_VB_MmapPool(0) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VB_MmapPool NG\n");
		return CVI_FAILURE;
	}

	blk = CVI_VB_GetBlock(0, bufLen);
	if (blk == VB_INVALID_HANDLE) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VB_GetBlock NG\n");
		return CVI_FAILURE;
	}

	if (CVI_VB_GetBlockVirAddr(0, blk, &pVirAddr) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VB_GetBlockVirAddr NG\n");
		return CVI_FAILURE;
	}

	pu8TestBuf = malloc(bufLen);
	_memcpy_test(pu8TestBuf, pVirAddr, bufLen, &duration1);
	_memcpy_test(pu8TestBuf, pVirAddr, bufLen, &duration2);
	free(pu8TestBuf);
	CVI_TRACE_LOG(CVI_DBG_WARN, "*map pool memcpy(%d bytes)     %d - %d\n", bufLen, duration1, duration2);

	CVI_VB_ReleaseBlock(blk);

	if (CVI_VB_MunmapPool(0) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VB_MmapPool NG\n");
		return CVI_FAILURE;
	}

	if ((duration_cached * 2) > duration_non_cached) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "cached not work. cached(%dus) non-cached(%dus)\n",
			      duration_cached, duration_non_cached);
		return CVI_FAILURE;
	}

	// test vb mmap
	// --------------------------------------
	for (int i = 0; i < 3; ++i) {
		blk = CVI_VB_GetBlock(VB_INVALID_POOLID, bufLen);
		if (blk == VB_INVALID_HANDLE) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VB_GetBlock NG\n");
			return CVI_FAILURE;
		}

		u64PhyAddr = CVI_VB_Handle2PhysAddr(blk);
		pVirAddr = CVI_SYS_MmapCache(u64PhyAddr, bufLen);
		if (pVirAddr == CVI_NULL) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_Mmap NG.\n");
			CVI_VB_ReleaseBlock(blk);
			return CVI_FAILURE;
		}

		pu8TestBuf = malloc(bufLen);
		_memcpy_test(pu8TestBuf, pVirAddr, bufLen, &duration1);
		_memcpy_test(pu8TestBuf, pVirAddr, bufLen, &duration2);
		free(pu8TestBuf);
		CVI_TRACE_LOG(CVI_DBG_WARN, "*vb%d memcpy(%d bytes)        %d - %d\n",
			      i, bufLen, duration1, duration2);

		if ((duration2 * 2) > duration_non_cached) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "cached not work. cached(%dus) non-cached(%dus)\n",
				      duration1, duration_non_cached);
			CVI_VB_ReleaseBlock(blk);
			return CVI_FAILURE;
		}

		if (CVI_SYS_Munmap(pVirAddr, bufLen) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_Munmap NG.\n");
			CVI_VB_ReleaseBlock(blk);
			return CVI_FAILURE;
		}
		CVI_VB_ReleaseBlock(blk);
	}

	// test flush/invalidate
	// --------------------------------------
	SIZE_S stSize = { .u32Width = 256, .u32Height = 256 };
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};

	abChnEnable[0] = CVI_TRUE;
	_VPSS_INIT();

#if 0
	if (_flush_test(stSize, bufLen, false) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "no flush should be NG.\n");
		SAMPLE_COMM_VPSS_Stop(0, abChnEnable);
		return CVI_FAILURE;
	}
#endif

	if (_flush_test(stSize, bufLen, true) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "flush NG.\n");
		SAMPLE_COMM_VPSS_Stop(0, abChnEnable);
		return CVI_FAILURE;
	}

#if 0
	if (_invalidate_test(stSize, bufLen, false) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "no invalidate should be NG.\n");
		SAMPLE_COMM_VPSS_Stop(0, abChnEnable);
		return CVI_FAILURE;
	}
#endif
	if (_invalidate_test(stSize, bufLen, true) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "invalidate NG.\n");
		SAMPLE_COMM_VPSS_Stop(0, abChnEnable);
		return CVI_FAILURE;
	}


	SAMPLE_COMM_VPSS_Stop(0, abChnEnable);

	return CVI_SUCCESS;
}

CVI_S32 CVI_VB_TEST(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_BLK blk;
	VIDEO_FRAME_INFO_S stVideoFrame, stVideoFrameOut;
	CVI_VOID *pVirAddr = NULL, *pVirAddrOut = NULL;
	SIZE_S stSize = { .u32Width = 256, .u32Height = 256 };
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	CVI_U32 cnt;

	// test mmap pool cached
	if (CVI_VB_MmapPool(0) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VB_MmapPool NG\n");
		return CVI_FAILURE;
	}

	if (CVI_VB_MmapPool(0) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VB_MmapPool NG\n");
		return CVI_FAILURE;
	}

	SAMPLE_COMM_PrepareFrame(stSize, PIXEL_FORMAT_RGB_888_PLANAR, &stVideoFrame);
	blk = CVI_VB_PhysAddr2Handle(stVideoFrame.stVFrame.u64PhyAddr[0]);
	if (blk == VB_INVALID_HANDLE) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VB_PhysAddr2Handle NG\n");
		s32Ret = CVI_FAILURE;
		goto ERR_PHY2HAND;
	}

	if (CVI_VB_Handle2PhysAddr(blk) != stVideoFrame.stVFrame.u64PhyAddr[0]) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VB_Handle2PhysAddr NG\n");
		s32Ret = CVI_FAILURE;
		goto ERR_HAND2PHY;
	}

	if (CVI_VB_GetBlockVirAddr(0, blk, &pVirAddr) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VB_GetBlockVirAddr NG\n");
		s32Ret = CVI_FAILURE;
		goto ERR_GETBLKVIR;
	}

	_VPSS_INIT();

	memset(pVirAddr, 0xab, 0x4000);
	CVI_SYS_IonFlushCache(stVideoFrame.stVFrame.u64PhyAddr[0], pVirAddr, 0x4000);

	if (CVI_VB_InquireUserCnt(blk, &cnt) != CVI_SUCCESS || cnt != 1) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VB_InquireUserCnt NG\n");
		s32Ret = CVI_FAILURE;
	}

	// use hw dma to move
	CVI_VPSS_SendFrame(0, &stVideoFrame, -1);
	CVI_VPSS_GetChnFrame(0, 0, &stVideoFrameOut, 1000);

	pVirAddrOut = CVI_SYS_MmapCache(stVideoFrameOut.stVFrame.u64PhyAddr[0], 0x4000);
	if (pVirAddrOut == CVI_NULL) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_MmapCache NG.\n");
		s32Ret = CVI_FAILURE;
		goto ERR_MMAPCACHE;
	}
	for (int j = 0; j < 1000; ++j) {
		if (*((CVI_U32 *)pVirAddrOut + j) != 0xabababab) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VB_MmapPool's vir-addr NG\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "offset(%d) value(%#x)\n", j, *((CVI_U32 *)pVirAddrOut + j));
			s32Ret = CVI_FAILURE;
			break;
		}
	}

ERR_MMAPCACHE:
	CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrameOut);
ERR_GETBLKVIR:
ERR_HAND2PHY:
	CVI_VB_ReleaseBlock(blk);
	if (CVI_VB_InquireUserCnt(blk, &cnt) != CVI_SUCCESS || cnt != 0) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VB_InquireUserCnt NG\n");
		s32Ret = CVI_FAILURE;
	}
ERR_PHY2HAND:
	if (CVI_VB_MunmapPool(0) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VB_MmapPool NG\n");
		return CVI_FAILURE;
	}

	abChnEnable[0] = CVI_TRUE;
	SAMPLE_COMM_VPSS_Stop(0, abChnEnable);

	return s32Ret;
}

CVI_S32 CVI_SYS_BIND_TEST(void)
{
	MMF_CHN_S stSrcChn, stDestChn;
	MMF_BIND_DEST_S stDests;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};

	_VPSS_INIT();

	if (SAMPLE_COMM_VI_Bind_VPSS(0, 0, 0) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi bind vpss failed.\n");
		return CVI_FAILURE;
	}

	stDestChn.enModId = CVI_ID_VPSS;
	stDestChn.s32DevId = 0;
	stDestChn.s32ChnId = 0;
	if (CVI_SYS_GetBindbyDest(&stDestChn, &stSrcChn) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_GetBindbyDest failed.\n");
		return CVI_FAILURE;
	}
	if (stSrcChn.enModId != CVI_ID_VI || stSrcChn.s32DevId != 0 || stSrcChn.s32ChnId != 0) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "src chn info incorrect. !\n");
		return CVI_FAILURE;
	}

	stSrcChn.enModId = CVI_ID_VI;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = 0;
	if (CVI_SYS_GetBindbySrc(&stSrcChn, &stDests) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_GetBindbySrc failed.\n");
		return CVI_FAILURE;
	}
	if (stDests.u32Num != 1 || stDests.astMmfChn[0].enModId != CVI_ID_VPSS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "dst chn info incorrect.\n");
		return CVI_FAILURE;
	}

	if (SAMPLE_COMM_VI_UnBind_VPSS(0, 0, 0) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi unbind vpss failed.\n");
		return CVI_FAILURE;
	}

	abChnEnable[0] = CVI_TRUE;
	SAMPLE_COMM_VPSS_Stop(0, abChnEnable);
	return CVI_SUCCESS;
}

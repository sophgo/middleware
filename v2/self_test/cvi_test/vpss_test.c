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

#include "vpss_test.h"
#include "../../modules/sys/include/cvi_base.h"

static PIXEL_FORMAT_E filefmt[] = {
	PIXEL_FORMAT_RGB_888,
	PIXEL_FORMAT_BGR_888,
	PIXEL_FORMAT_RGB_888_PLANAR,
	PIXEL_FORMAT_BGR_888_PLANAR,

	PIXEL_FORMAT_YUV_PLANAR_420,
	PIXEL_FORMAT_YUV_PLANAR_422,
	PIXEL_FORMAT_YUV_400,
};

static CVI_CHAR *filenames[] = {
	"res/golden.rgb",
	"res/golden.bgr",
	"res/golden.rgbm",
	"res/golden.bgrm",

	"res/golden.yuv420",
	"res/golden.yuv422",
	"res/golden.y",
};

static CVI_S32 _1835_VPSS_VI_TEST(SIZE_S stSize, bool fail_pause)
{
	SIZE_S stSizeOut = {.u32Width = 1920, .u32Height = 1080};
	CVI_S32 s32Ret = CVI_SUCCESS;
	VIDEO_FRAME_INFO_S stVideoFrame;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};

	s32Ret = SAMPLE_PLAT_VPSS_INIT(0, stSize, stSizeOut);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vpss init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VI_Bind_VPSS(0, 0, 0);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	for (int i = 0; i < 3; ++i) {
		s32Ret = CVI_VPSS_GetChnFrame(0, 0, &stVideoFrame, 2000);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
			s32Ret = CVI_FAILURE;
			break;
		}
		CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrame);
	}

	if (s32Ret == CVI_FAILURE && fail_pause)
		PAUSE();

	SAMPLE_COMM_VI_UnBind_VPSS(0, 0, 0);

	abChnEnable[0] = CVI_TRUE;
	SAMPLE_COMM_VPSS_Stop(0, abChnEnable);

	return s32Ret;
}

static CVI_S32 _1835_VPSS_FMT_INPUT_TEST(bool fail_pause)
{
	SIZE_S stSize = {.u32Width = 1920, .u32Height = 1080};
	VPSS_GRP_ATTR_S    stVpssGrpAttr;
	VPSS_GRP	   VpssGrp = 0;
	VPSS_CHN	   VpssChn = VPSS_CHN0;
	CVI_BOOL	   abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
	VIDEO_FRAME_INFO_S stVideoFrame;
	CVI_CHAR filename[64];
	CVI_S32 s32Ret = CVI_SUCCESS;
	bool is_result_match = CVI_TRUE;
	CVI_CHAR *filenames_out[] = {
		"res/vpss_fmt_in.bin",
		"res/vpss_fmt_in.bin",
		"res/vpss_fmt_in.bin",
		"res/vpss_fmt_in.bin",
		"res/vpss_fmt_in.bin",
		"res/vpss_fmt_in_yuv422.bin",
		"res/vpss_fmt_in_y.bin",
	};

	// VpssGrp 0
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = PIXEL_FORMAT_YUV_PLANAR_420;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;

	astVpssChnAttr[VpssChn].u32Width		    = 1280;
	astVpssChnAttr[VpssChn].u32Height		    = 720;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_RGB_888_PLANAR;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	for (CVI_U32 i = 0; i < ARRAY_SIZE(filefmt); ++i) {
		stVpssGrpAttr.enPixelFormat = filefmt[i];
		CVI_VPSS_SetGrpAttr(VpssGrp, &stVpssGrpAttr);

		CVI_TRACE_LOG(CVI_DBG_INFO, "filefmt(%d) filename(%s)\n", filefmt[i], filenames[i]);
		SAMPLE_COMM_VPSS_SendFrame(VpssGrp, &stSize, filefmt[i], filenames[i]);
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, 1000);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnFrame for grp%d chn%d. s32Ret: 0x%x !\n"
				, VpssGrp, VpssChn, s32Ret);
			is_result_match = CVI_FALSE;
			continue;
		}

		if (!SAMPLE_COMM_FRAME_CompareWithFile(filenames_out[i], &stVideoFrame)) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "input test(%d) format(%d) mismatch!\n", i, filefmt[i]);
			is_result_match = CVI_FALSE;

			snprintf(filename, 32, "./res/vpss_fmt_in_ng_%d.bin", i);
			SAMPLE_COMM_FRAME_SaveToFile(filename, &stVideoFrame);

			if (fail_pause)
				PAUSE();
		}

		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);
	}

	SAMPLE_COMM_VPSS_Stop(VpssGrp, abChnEnable);
	return (is_result_match) ? CVI_SUCCESS : CVI_FAILURE;
}

static CVI_S32 _1835_VPSS_FMT_OUTPUT_TEST(bool fail_pause)
{
	SIZE_S stSize = {.u32Width = 1920, .u32Height = 1080};
	VPSS_GRP_ATTR_S    stVpssGrpAttr;
	VPSS_GRP	   VpssGrp = 0;
	VPSS_CHN	   VpssChn = VPSS_CHN0;
	CVI_BOOL	   abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
	VIDEO_FRAME_INFO_S stVideoFrame;
	CVI_CHAR filename[64];
	CVI_S32 s32Ret = CVI_SUCCESS;
	bool is_result_match = CVI_TRUE;
	PIXEL_FORMAT_E filefmt_out[] = {
		PIXEL_FORMAT_RGB_888,
		PIXEL_FORMAT_BGR_888,
		PIXEL_FORMAT_RGB_888_PLANAR,
		PIXEL_FORMAT_BGR_888_PLANAR,

		PIXEL_FORMAT_YUV_PLANAR_420,
		PIXEL_FORMAT_YUV_PLANAR_422,
		PIXEL_FORMAT_YUV_400,

		//PIXEL_FORMAT_HSV_888,
		//PIXEL_FORMAT_HSV_888_PLANAR,
	};

	// VpssGrp 0
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = PIXEL_FORMAT_YUV_PLANAR_420;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;

	astVpssChnAttr[VpssChn].u32Width		    = 1280;
	astVpssChnAttr[VpssChn].u32Height		    = 720;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_RGB_888_PLANAR;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	for (CVI_U32 i = 0; i < ARRAY_SIZE(filefmt_out); ++i) {
		astVpssChnAttr[VpssChn].enPixelFormat = filefmt_out[i];
		CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &astVpssChnAttr[VpssChn]);

		CVI_TRACE_LOG(CVI_DBG_INFO, "filefmt(%d) filename(%s)\n", filefmt[i], filenames[i]);
		SAMPLE_COMM_VPSS_SendFrame(VpssGrp, &stSize, PIXEL_FORMAT_YUV_PLANAR_420, filenames[4]);
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, 1000);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnFrame for grp%d chn%d. s32Ret: 0x%x !\n"
				, VpssGrp, VpssChn, s32Ret);
			is_result_match = CVI_FALSE;
			continue;
		}

		snprintf(filename, 32, "./res/vpss_fmt_out_%d.bin", i);
		if (!SAMPLE_COMM_FRAME_CompareWithFile(filename, &stVideoFrame)) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "output test(%d) format(%d) mismatch!\n", i, filefmt_out[i]);
			is_result_match = CVI_FALSE;

			snprintf(filename, 32, "./res/vpss_fmt_out_ng_%d.bin", i);
			SAMPLE_COMM_FRAME_SaveToFile(filename, &stVideoFrame);

			if (fail_pause)
				PAUSE();
		}

		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);
	}

	SAMPLE_COMM_VPSS_Stop(VpssGrp, abChnEnable);
	return (is_result_match) ? CVI_SUCCESS : CVI_FAILURE;
}

static CVI_S32 _1835_VPSS_YUV_GOLDEN2K_TEST(bool fail_pause)
{
	SIZE_S stSize = {.u32Width = 1920, .u32Height = 1080};
	VPSS_GRP_ATTR_S    stVpssGrpAttr;
	VPSS_GRP	   VpssGrp = 0;
	VPSS_CHN	   VpssChn = VPSS_CHN0;
	CVI_BOOL	   abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
	VIDEO_FRAME_INFO_S stVideoFrame;
	CVI_CHAR filename[64];
	CVI_S32 s32Ret = CVI_SUCCESS;
	bool is_result_match = CVI_TRUE;
	CVI_U8 testcase_vpss_chns[3] = { 1, 2, 4 };
	VPSS_CROP_INFO_S stCropInfo;

	// VpssGrp 0
	//  1920x1080 yuv420 input + crop (100, 200, 1200, 600) + opencv-bilinear
	//  ch0: 736x1280 rgb planar + flip
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = PIXEL_FORMAT_YUV_PLANAR_420;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;

	astVpssChnAttr[VpssChn].u32Width		    = 736;
	astVpssChnAttr[VpssChn].u32Height		    = 1280;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_RGB_888_PLANAR;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	CVI_VPSS_SetChnScaleCoefLevel(VpssGrp, VpssChn, VPSS_SCALE_COEF_OPENCV_BILINEAR);

	// VpssGrp 1
	//  1920x1080 yuv420 input
	//  ch0: 608x608 rgb planar + aspect_ratio_auto + normalize + bgcolor
	//  ch1: 320x240 bgr packed + aspect_ratio_auto + bgcolor
	VpssGrp = 1;
	VpssChn = 0;
	astVpssChnAttr[VpssChn].u32Width		    = 608;
	astVpssChnAttr[VpssChn].u32Height		    = 608;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_RGB_888_PLANAR;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_TRUE;
	astVpssChnAttr[VpssChn].stNormalize.factor[0] = 0.8415;
	astVpssChnAttr[VpssChn].stNormalize.factor[1] = 0.8415;
	astVpssChnAttr[VpssChn].stNormalize.factor[2] = 0.8415;
	astVpssChnAttr[VpssChn].stNormalize.mean[0] = 102.9801;
	astVpssChnAttr[VpssChn].stNormalize.mean[1] = 115.9465;
	astVpssChnAttr[VpssChn].stNormalize.mean[2] = 122.7717;
	astVpssChnAttr[VpssChn].stNormalize.rounding = VPSS_ROUNDING_TO_EVEN;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;

	// Output RGB format, size is the same with VI's.
	VpssChn = 1;
	astVpssChnAttr[VpssChn].u32Width		    = 320;
	astVpssChnAttr[VpssChn].u32Height		    = 240;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_BGR_888;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = abChnEnable[1] = CVI_TRUE;
	abChnEnable[2] = abChnEnable[3] = CVI_FALSE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}


	// VpssGrp 2
	//  1920x1080 yuv420 input
	//  ch0: 608x608 rgb planar + aspect_ratio_auto + bgcolor
	//  ch1: 480x640 bgr packed + aspect_ratio_auto + bgcolor + crop (100, 200, 1200, 600)
	//  ch2: 720x1280 yuv420 + rotation90
	//  ch3: 320x320 rgb planar + aspect_ratio_auto + bgcolor
	VpssGrp = 2;
	VpssChn = 0;
	astVpssChnAttr[VpssChn].u32Width		    = 608;
	astVpssChnAttr[VpssChn].u32Height		    = 608;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_RGB_888_PLANAR;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	VpssChn = 1;
	astVpssChnAttr[VpssChn].u32Width		    = 480;
	astVpssChnAttr[VpssChn].u32Height		    = 640;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_BGR_888;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	VpssChn = 2;
	astVpssChnAttr[VpssChn].u32Width		    = 720;
	astVpssChnAttr[VpssChn].u32Height		    = 1280;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_YUV_PLANAR_420;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_NONE;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	VpssChn = 3;
	astVpssChnAttr[VpssChn].u32Width		    = 320;
	astVpssChnAttr[VpssChn].u32Height		    = 320;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_RGB_888_PLANAR;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = abChnEnable[1] = abChnEnable[2] = abChnEnable[3] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	CVI_VPSS_SetChnRotation(VpssGrp, 2, ROTATION_90);

	stCropInfo.bEnable = CVI_TRUE;
	stCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
	stCropInfo.stCropRect.s32X = 100;
	stCropInfo.stCropRect.s32Y = 200;
	stCropInfo.stCropRect.u32Width = 1200;
	stCropInfo.stCropRect.u32Height = 600;
	CVI_VPSS_SetGrpCrop(0, &stCropInfo);
	CVI_VPSS_SetChnCrop(2, 1, &stCropInfo);

	// 2k test and compare result.
	for (VpssGrp = 0; VpssGrp < 3; ++VpssGrp) {
		SAMPLE_COMM_VPSS_SendFrame(VpssGrp, &stSize, PIXEL_FORMAT_YUV_PLANAR_420, filenames[4]);
		for (VpssChn = 0; VpssChn < testcase_vpss_chns[VpssGrp]; ++VpssChn) {
			s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, 1000);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnFrame for grp%d chn%d. s32Ret: 0x%x !\n"
					, VpssGrp, VpssChn, s32Ret);
				is_result_match = CVI_FALSE;
				continue;
			}

			snprintf(filename, 32, "./res/vpss_%d_%d.bin", VpssGrp, VpssChn);
			if (!SAMPLE_COMM_FRAME_CompareWithFile(filename, &stVideoFrame)) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "grp%d chn%d. mismatch!\n", VpssGrp, VpssChn);
				is_result_match = CVI_FALSE;

				snprintf(filename, 32, "./res/vpss_ng_%d_%d.bin", VpssGrp, VpssChn);
				SAMPLE_COMM_FRAME_SaveToFile(filename, &stVideoFrame);

				if (fail_pause)
					PAUSE();
			}

			CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);
		}
	}

	return (is_result_match) ? CVI_SUCCESS : CVI_FAILURE;
}

static CVI_S32 _1835_VPSS_YUV_GOLDEN4K_TEST(bool fail_pause)
{
	SIZE_S stSize = {.u32Width = 3840, .u32Height = 2160};
	VPSS_GRP_ATTR_S    stVpssGrpAttr;
	VPSS_GRP	   VpssGrp = 3;
	VPSS_CHN	   VpssChn = VPSS_CHN0;
	CVI_BOOL	   abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
	VIDEO_FRAME_INFO_S stVideoFrame;
	CVI_CHAR filename[64];
	CVI_S32 s32Ret = CVI_SUCCESS;
	bool is_result_match = CVI_TRUE;
	VPSS_CROP_INFO_S stCropInfo;

	// VpssGrp 3
	//  3840x2160 yuv420 input
	//  ch0: 3320x1280 rgb planar + aspect_ratio_auto + bgcolor
	//  ch1: 3000x1680 yuv420 + aspect_ratio_auto + bgcolor + flip + mirror
	//  ch2: 1480x720 rgb packed + aspect_ratio_auto + bgcolor
	//  ch3: 1920x1080 yuv422 + aspect_ratio_auto + bgcolor + mirror
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = PIXEL_FORMAT_YUV_PLANAR_420;
	stVpssGrpAttr.u32MaxW			     = 3840;
	stVpssGrpAttr.u32MaxH			     = 2160;

	VpssChn = 0;
	astVpssChnAttr[VpssChn].u32Width		    = 3320;
	astVpssChnAttr[VpssChn].u32Height		    = 1280;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_RGB_888_PLANAR;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	VpssChn = 1;
	astVpssChnAttr[VpssChn].u32Width		    = 3000;
	astVpssChnAttr[VpssChn].u32Height		    = 1680;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_YUV_PLANAR_420;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_TRUE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	VpssChn = 2;
	astVpssChnAttr[VpssChn].u32Width		    = 1480;
	astVpssChnAttr[VpssChn].u32Height		    = 720;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_RGB_888;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	VpssChn = 3;
	astVpssChnAttr[VpssChn].u32Width		    = 1920;
	astVpssChnAttr[VpssChn].u32Height		    = 1080;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_YUV_PLANAR_422;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_TRUE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = abChnEnable[1] = abChnEnable[2] = abChnEnable[3] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	// 4k test and compare result.
	SAMPLE_COMM_VPSS_SendFrame(VpssGrp, &stSize, PIXEL_FORMAT_YUV_PLANAR_420, "./res/golden_4k.yuv");
	for (VpssChn = 0; VpssChn < 4; ++VpssChn) {
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, 1000);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnFrame for grp%d chn%d. s32Ret: 0x%x !\n"
				, VpssGrp, VpssChn, s32Ret);
			is_result_match = CVI_FALSE;
			continue;
		}

		snprintf(filename, 32, "./res/vpss_%d_%d.bin", VpssGrp, VpssChn);
		if (!SAMPLE_COMM_FRAME_CompareWithFile(filename, &stVideoFrame)) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "grp%d chn%d. mismatch!\n", VpssGrp, VpssChn);
			is_result_match = CVI_FALSE;

			snprintf(filename, 32, "./res/vpss_ng_%d_%d.bin", VpssGrp, VpssChn);
			SAMPLE_COMM_FRAME_SaveToFile(filename, &stVideoFrame);

			if (fail_pause)
				PAUSE();
		}

		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);
	}

	// VpssGrp 8
	//  3840x2160 yuv420 input
	//  ch0: 800x600 yuv420 + aspect_ratio_auto + bgcolor, crop in left tile
	//  ch1: 800x600 yuv420 + aspect_ratio_auto + bgcolor, crop in right tile
	//  ch2: 800x600 yuv420 + aspect_ratio_auto + bgcolor, crop in middle of tile
	VpssGrp = 8;

	VpssChn = 0;
	astVpssChnAttr[VpssChn].u32Width		    = 800;
	astVpssChnAttr[VpssChn].u32Height		    = 600;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_YUV_PLANAR_420;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	VpssChn = 1;
	astVpssChnAttr[VpssChn].u32Width		    = 800;
	astVpssChnAttr[VpssChn].u32Height		    = 600;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_YUV_PLANAR_420;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	VpssChn = 2;
	astVpssChnAttr[VpssChn].u32Width		    = 800;
	astVpssChnAttr[VpssChn].u32Height		    = 600;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_YUV_PLANAR_420;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = abChnEnable[1] = abChnEnable[2] = CVI_TRUE;
	abChnEnable[3] = CVI_FALSE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	stCropInfo.bEnable = CVI_TRUE;
	stCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
	stCropInfo.stCropRect.s32X = 0;
	stCropInfo.stCropRect.s32Y = 1300;
	stCropInfo.stCropRect.u32Width = 800;
	stCropInfo.stCropRect.u32Height = 600;
	CVI_VPSS_SetChnCrop(VpssGrp, 0, &stCropInfo);

	stCropInfo.bEnable = CVI_TRUE;
	stCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
	stCropInfo.stCropRect.s32X = 2200;
	stCropInfo.stCropRect.s32Y = 1300;
	stCropInfo.stCropRect.u32Width = 800;
	stCropInfo.stCropRect.u32Height = 600;
	CVI_VPSS_SetChnCrop(VpssGrp, 1, &stCropInfo);

	stCropInfo.bEnable = CVI_TRUE;
	stCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
	stCropInfo.stCropRect.s32X = 1800;
	stCropInfo.stCropRect.s32Y = 1300;
	stCropInfo.stCropRect.u32Width = 800;
	stCropInfo.stCropRect.u32Height = 600;
	CVI_VPSS_SetChnCrop(VpssGrp, 2, &stCropInfo);

	// 4k test and compare result.
	SAMPLE_COMM_VPSS_SendFrame(VpssGrp, &stSize, PIXEL_FORMAT_YUV_PLANAR_420, "./res/golden_4k.yuv");
	for (VpssChn = 0; VpssChn < 3; ++VpssChn) {
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, 1000);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnFrame for grp%d chn%d. s32Ret: 0x%x !\n"
				, VpssGrp, VpssChn, s32Ret);
			is_result_match = CVI_FALSE;
			continue;
		}

		snprintf(filename, 32, "./res/vpss_%d_%d.bin", VpssGrp, VpssChn);
		if (!SAMPLE_COMM_FRAME_CompareWithFile(filename, &stVideoFrame)) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "grp%d chn%d. mismatch!\n", VpssGrp, VpssChn);
			is_result_match = CVI_FALSE;

			snprintf(filename, 32, "./res/vpss_ng_%d_%d.bin", VpssGrp, VpssChn);
			SAMPLE_COMM_FRAME_SaveToFile(filename, &stVideoFrame);

			if (fail_pause)
				PAUSE();
		}

		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);
	}

	SAMPLE_COMM_VPSS_Stop(VpssGrp, abChnEnable);
	return (is_result_match) ? CVI_SUCCESS : CVI_FAILURE;
}

static CVI_S32 _1835_VPSS_RGB_GOLDEN2K_TEST(bool fail_pause)
{
	SIZE_S stSize = {.u32Width = 1920, .u32Height = 1080};
	VPSS_GRP_ATTR_S    stVpssGrpAttr;
	VPSS_GRP	   VpssGrp = 4;
	VPSS_CHN	   VpssChn = VPSS_CHN0;
	CVI_BOOL	   abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
	VIDEO_FRAME_INFO_S stVideoFrame;
	CVI_CHAR filename[64];
	CVI_S32 s32Ret = CVI_SUCCESS;
	bool is_result_match = CVI_TRUE;

	// VpssGrp 4
	//  1920x1080 rgb planar input
	//  ch0: 608x608 rgb planar + aspect_ratio_auto + normalize + bgcolor
	//  ch1: 320x240 bgr packed + aspect_ratio_auto + bgcolor
	//  ch2: 1920x108 bgr planar + aspect_ratio_none
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = PIXEL_FORMAT_RGB_888_PLANAR;
	stVpssGrpAttr.u32MaxW			     = 1920;
	stVpssGrpAttr.u32MaxH			     = 1080;

	VpssChn = 0;
	astVpssChnAttr[VpssChn].u32Width                    = 608;
	astVpssChnAttr[VpssChn].u32Height                   = 608;
	astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat               = PIXEL_FORMAT_RGB_888_PLANAR;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth                    = 1;
	astVpssChnAttr[VpssChn].bMirror                     = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip                       = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode        = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_TRUE;
	astVpssChnAttr[VpssChn].stNormalize.factor[0] = 0.8415;
	astVpssChnAttr[VpssChn].stNormalize.factor[1] = 0.8415;
	astVpssChnAttr[VpssChn].stNormalize.factor[2] = 0.8415;
	astVpssChnAttr[VpssChn].stNormalize.mean[0] = 102.9801;
	astVpssChnAttr[VpssChn].stNormalize.mean[1] = 115.9465;
	astVpssChnAttr[VpssChn].stNormalize.mean[2] = 122.7717;
	astVpssChnAttr[VpssChn].stNormalize.rounding = VPSS_ROUNDING_TO_EVEN;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;

	VpssChn = 1;
	astVpssChnAttr[VpssChn].u32Width                    = 320;
	astVpssChnAttr[VpssChn].u32Height                   = 240;
	astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat               = PIXEL_FORMAT_BGR_888;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth                    = 1;
	astVpssChnAttr[VpssChn].bMirror                     = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip                       = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode        = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;

	VpssChn = 2;
	astVpssChnAttr[VpssChn].u32Width                    = 1920;
	astVpssChnAttr[VpssChn].u32Height                   = 1080;
	astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat               = PIXEL_FORMAT_BGR_888_PLANAR;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth                    = 1;
	astVpssChnAttr[VpssChn].bMirror                     = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip                       = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
	astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = abChnEnable[1] = abChnEnable[2] = CVI_TRUE;
	abChnEnable[3] = CVI_FALSE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	// test and compare result.
	SAMPLE_COMM_VPSS_SendFrame(VpssGrp, &stSize, PIXEL_FORMAT_RGB_888_PLANAR, filenames[2]);
	for (VpssChn = 0; VpssChn < 3; ++VpssChn) {
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, 1000);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnFrame for grp%d chn%d. s32Ret: 0x%x !\n"
				, VpssGrp, VpssChn, s32Ret);
			is_result_match = CVI_FALSE;
			continue;
		}

		snprintf(filename, 32, "./res/vpss_%d_%d.bin", VpssGrp, VpssChn);
		if (!SAMPLE_COMM_FRAME_CompareWithFile(filename, &stVideoFrame)) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "grp%d chn%d. mismatch!\n", VpssGrp, VpssChn);
			is_result_match = CVI_FALSE;

			snprintf(filename, 32, "./res/vpss_ng_%d_%d.bin", VpssGrp, VpssChn);
			SAMPLE_COMM_FRAME_SaveToFile(filename, &stVideoFrame);

			if (fail_pause)
				PAUSE();
		}

		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);
	}

	// VpssGrp 5
	//  1920x1080 bgr planar input(grp4 chn2 output)
	//  ch0: 608x608 rgb planar + aspect_ratio_auto + normalize + bgcolor
	//  ch1: 320x240 bgr packed + aspect_ratio_auto + bgcolor
	VpssGrp = 5;
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = PIXEL_FORMAT_BGR_888_PLANAR;
	stVpssGrpAttr.u32MaxW			     = 1920;
	stVpssGrpAttr.u32MaxH			     = 1080;

	VpssChn = 0;
	astVpssChnAttr[VpssChn].u32Width                    = 608;
	astVpssChnAttr[VpssChn].u32Height                   = 608;
	astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat               = PIXEL_FORMAT_RGB_888_PLANAR;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth                    = 1;
	astVpssChnAttr[VpssChn].bMirror                     = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip                       = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode        = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_TRUE;
	astVpssChnAttr[VpssChn].stNormalize.factor[0] = 0.8415;
	astVpssChnAttr[VpssChn].stNormalize.factor[1] = 0.8415;
	astVpssChnAttr[VpssChn].stNormalize.factor[2] = 0.8415;
	astVpssChnAttr[VpssChn].stNormalize.mean[0] = 102.9801;
	astVpssChnAttr[VpssChn].stNormalize.mean[1] = 115.9465;
	astVpssChnAttr[VpssChn].stNormalize.mean[2] = 122.7717;
	astVpssChnAttr[VpssChn].stNormalize.rounding = VPSS_ROUNDING_TO_EVEN;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;

	VpssChn = 1;
	astVpssChnAttr[VpssChn].u32Width                    = 320;
	astVpssChnAttr[VpssChn].u32Height                   = 240;
	astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat               = PIXEL_FORMAT_BGR_888;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth                    = 1;
	astVpssChnAttr[VpssChn].bMirror                     = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip                       = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode        = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = abChnEnable[1] = CVI_TRUE;
	abChnEnable[2] = abChnEnable[3] = CVI_FALSE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	SAMPLE_COMM_VPSS_SendFrame(VpssGrp, &stSize, PIXEL_FORMAT_BGR_888_PLANAR, "./res/vpss_4_2.bin");
	for (VpssChn = 0; VpssChn < 2; ++VpssChn) {
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, 1000);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnFrame for grp%d chn%d. s32Ret: 0x%x !\n"
				, VpssGrp, VpssChn, s32Ret);
			is_result_match = CVI_FALSE;
			continue;
		}

		snprintf(filename, 32, "./res/vpss_%d_%d.bin", 4, VpssChn);
		if (!SAMPLE_COMM_FRAME_CompareWithFile(filename, &stVideoFrame)) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "grp%d chn%d. mismatch!\n", VpssGrp, VpssChn);
			is_result_match = CVI_FALSE;

			snprintf(filename, 32, "./res/vpss_ng_%d_%d.bin", VpssGrp, VpssChn);
			SAMPLE_COMM_FRAME_SaveToFile(filename, &stVideoFrame);

			if (fail_pause)
				PAUSE();
		}

		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);
	}

	return (is_result_match) ? CVI_SUCCESS : CVI_FAILURE;
}

static CVI_S32 _1835_VPSS_YUV_GOLDEN2K_DUAL_TEST(bool fail_pause)
{
	SIZE_S stSize = {.u32Width = 1920, .u32Height = 1080};
	VPSS_GRP_ATTR_S    stVpssGrpAttr;
	VPSS_GRP	   VpssGrp = 0;
	VPSS_CHN	   VpssChn = VPSS_CHN0;
	CVI_BOOL	   abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
	VIDEO_FRAME_INFO_S stVideoFrame;
	CVI_CHAR filename[64];
	CVI_S32 s32Ret = CVI_SUCCESS;
	bool is_result_match = CVI_TRUE;
	CVI_U8 testcase_vpss_chns[2] = { 1, 3,};
	VPSS_CROP_INFO_S stCropInfo;

	// VpssGrp 0
	//  1920x1080 yuv420 input + crop (100, 200, 1200, 600) + opencv-bilinear
	//  ch0: 736x1280 rgb planar + flip
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = PIXEL_FORMAT_YUV_PLANAR_420;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;

	astVpssChnAttr[VpssChn].u32Width		    = 736;
	astVpssChnAttr[VpssChn].u32Height		    = 1280;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_RGB_888_PLANAR;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	CVI_VPSS_SetChnScaleCoefLevel(VpssGrp, VpssChn, VPSS_SCALE_COEF_OPENCV_BILINEAR);

	// VpssGrp 1
	//  1920x1080 yuv420 input
	//  ch0: 608x608 rgb planar + aspect_ratio_auto + normalize + bgcolor
	//  ch1: 320x240 bgr packed + aspect_ratio_auto + bgcolor
	//  ch2: 1920x1080 yuv422
	VpssGrp = 1;
	VpssChn = 0;
	astVpssChnAttr[VpssChn].u32Width		    = 608;
	astVpssChnAttr[VpssChn].u32Height		    = 608;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_RGB_888_PLANAR;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_TRUE;
	astVpssChnAttr[VpssChn].stNormalize.factor[0] = 0.8415;
	astVpssChnAttr[VpssChn].stNormalize.factor[1] = 0.8415;
	astVpssChnAttr[VpssChn].stNormalize.factor[2] = 0.8415;
	astVpssChnAttr[VpssChn].stNormalize.mean[0] = 102.9801;
	astVpssChnAttr[VpssChn].stNormalize.mean[1] = 115.9465;
	astVpssChnAttr[VpssChn].stNormalize.mean[2] = 122.7717;
	astVpssChnAttr[VpssChn].stNormalize.rounding = VPSS_ROUNDING_TO_EVEN;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;

	VpssChn = 1;
	astVpssChnAttr[VpssChn].u32Width		    = 320;
	astVpssChnAttr[VpssChn].u32Height		    = 240;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_BGR_888;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	VpssChn = 2;
	astVpssChnAttr[VpssChn].u32Width		    = 1920;
	astVpssChnAttr[VpssChn].u32Height		    = 1080;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_YUV_PLANAR_422;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_NONE;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = abChnEnable[1] = abChnEnable[2] = CVI_TRUE;
	abChnEnable[3] = CVI_FALSE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	stCropInfo.bEnable = CVI_TRUE;
	stCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
	stCropInfo.stCropRect.s32X = 100;
	stCropInfo.stCropRect.s32Y = 200;
	stCropInfo.stCropRect.u32Width = 1200;
	stCropInfo.stCropRect.u32Height = 600;
	CVI_VPSS_SetGrpCrop(0, &stCropInfo);

	// 2k test and compare result.
	for (VpssGrp = 0; VpssGrp < 2; ++VpssGrp) {
		SAMPLE_COMM_VPSS_SendFrame(VpssGrp, &stSize, PIXEL_FORMAT_YUV_PLANAR_420, filenames[4]);
		for (VpssChn = 0; VpssChn < testcase_vpss_chns[VpssGrp]; ++VpssChn) {
			s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, 1000);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnFrame for grp%d chn%d. s32Ret: 0x%x !\n"
					, VpssGrp, VpssChn, s32Ret);
				is_result_match = CVI_FALSE;
				continue;
			}

			snprintf(filename, 32, "./res/vpss_%d_%d.bin", VpssGrp, VpssChn);
			if (!SAMPLE_COMM_FRAME_CompareWithFile(filename, &stVideoFrame)) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "grp%d chn%d. mismatch!\n", VpssGrp, VpssChn);
				is_result_match = CVI_FALSE;

				snprintf(filename, 32, "./res/vpss_ng_%d_%d.bin", VpssGrp, VpssChn);
				SAMPLE_COMM_FRAME_SaveToFile(filename, &stVideoFrame);

				if (fail_pause)
					PAUSE();
			}

			CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);
		}
	}

	return (is_result_match) ? CVI_SUCCESS : CVI_FAILURE;
}

static CVI_S32 _1835_VPSS_GRP_PARAM_TEST(bool fail_pause)
{
	VPSS_GRP_ATTR_S stGrpAttr;
	VPSS_CROP_INFO_S stCropInfo;
	PROC_AMP_CTRL_S stAmpCtrl;
	CVI_S32 tmp;
	SIZE_S stSize;
	VIDEO_FRAME_INFO_S stVideoFrame;

	(void) fail_pause;

	if (CVI_VPSS_GetAvailableGrp() != 2) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetAvailableGrp should be 7\n");
		return CVI_FAILURE;
	}

	if (CVI_VPSS_GetGrpAttr(7, &stGrpAttr) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetGrpAttr should not OK - unexist grp7!\n");
		return CVI_FAILURE;
	}

	if (CVI_VPSS_GetGrpAttr(0, &stGrpAttr) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetGrpAttr NG on grp0!\n");
		return CVI_FAILURE;
	}

	if (CVI_VPSS_GetGrpCrop(8, &stCropInfo) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetGrpCrop should not OK - unexist grp8!\n");
		return CVI_FAILURE;
	}

	if (CVI_VPSS_GetGrpCrop(0, &stCropInfo) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetGrpCrop NG on grp0!\n");
		return CVI_FAILURE;
	}

	stSize.u32Width = stGrpAttr.u32MaxW;
	stSize.u32Height = stGrpAttr.u32MaxH;
	SAMPLE_COMM_PrepareFrame(stSize, stGrpAttr.enPixelFormat, &stVideoFrame);
	stVideoFrame.stVFrame.u32Height += 10;
	if (CVI_VPSS_SendFrame(0, &stVideoFrame, -1) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SendFrame should not OK - grp size mismatch\n");
		return CVI_FAILURE;
	}
	stVideoFrame.stVFrame.u32Height -= 10;

	stVideoFrame.stVFrame.u32Width -= 10;
	if (CVI_VPSS_SendFrame(0, &stVideoFrame, -1) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SendFrame should not OK - grp size mismatch\n");
		return CVI_FAILURE;
	}
	stVideoFrame.stVFrame.u32Width += 10;

	stVideoFrame.stVFrame.u32Stride[0] += 16;
	if (CVI_VPSS_SendFrame(0, &stVideoFrame, -1) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SendFrame should not OK - grp u32Stride not align\n");
		return CVI_FAILURE;
	}
	stVideoFrame.stVFrame.u32Stride[0] -= 16;

	stVideoFrame.stVFrame.u32Length[0] -= 16;
	if (CVI_VPSS_SendFrame(0, &stVideoFrame, -1) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SendFrame should not OK - grp u32Length not enough\n");
		return CVI_FAILURE;
	}
	stVideoFrame.stVFrame.u32Length[0] += 16;

	stVideoFrame.stVFrame.u64PhyAddr[0] += 16;
	if (CVI_VPSS_SendFrame(0, &stVideoFrame, -1) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SendFrame should not OK - grp u64PhyAddr not align\n");
		return CVI_FAILURE;
	}
	stVideoFrame.stVFrame.u64PhyAddr[0] -= 16;

	stVideoFrame.stVFrame.u64PhyAddr[1] += 0x100;
	if (CVI_VPSS_SendFrame(0, &stVideoFrame, -1) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SendFrame should not OK - grp u64PhyAddr not align\n");
		return CVI_FAILURE;
	}
	stVideoFrame.stVFrame.u64PhyAddr[1] -= 0x100;

	stVideoFrame.stVFrame.s16OffsetBottom = -1;
	stVideoFrame.stVFrame.s16OffsetTop = 0;
	stVideoFrame.stVFrame.s16OffsetLeft = 0;
	stVideoFrame.stVFrame.s16OffsetRight = 0;
	if (CVI_VPSS_SendFrame(0, &stVideoFrame, -1) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SendFrame should not OK - grp offset negative\n");
		return CVI_FAILURE;
	}

	stVideoFrame.stVFrame.s16OffsetBottom = 0;
	stVideoFrame.stVFrame.s16OffsetTop = 0;
	stVideoFrame.stVFrame.s16OffsetLeft = 0;
	stVideoFrame.stVFrame.s16OffsetRight = 7;
	if (CVI_VPSS_SendFrame(0, &stVideoFrame, -1) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SendFrame should not OK - grp offset width can't be odd\n");
		return CVI_FAILURE;
	}

	stVideoFrame.stVFrame.s16OffsetBottom = 5;
	stVideoFrame.stVFrame.s16OffsetTop = 0;
	stVideoFrame.stVFrame.s16OffsetLeft = 0;
	stVideoFrame.stVFrame.s16OffsetRight = 0;
	if (CVI_VPSS_SendFrame(0, &stVideoFrame, -1) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SendFrame should not OK - grp offset height can't be odd\n");
		return CVI_FAILURE;
	}

	stVideoFrame.stVFrame.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	if (CVI_VPSS_SendFrame(0, &stVideoFrame, -1) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SendFrame should not OK - grp fmt mismatch\n");
		return CVI_FAILURE;
	}
	stVideoFrame.stVFrame.enPixelFormat = PIXEL_FORMAT_BGR_888;
	if (CVI_VPSS_SendFrame(0, &stVideoFrame, -1) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SendFrame should not OK - grp fmt mismatch\n");
		return CVI_FAILURE;
	}

	stCropInfo.bEnable = CVI_FALSE;
	stCropInfo.stCropRect.s32X = -4;
	stCropInfo.stCropRect.s32Y = -4;
	stCropInfo.stCropRect.u32Width = stGrpAttr.u32MaxW / 2;
	stCropInfo.stCropRect.u32Height = stGrpAttr.u32MaxH / 2;
	if (CVI_VPSS_SetGrpCrop(0, &stCropInfo) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SetGrpCrop NG - crop disable on invalid crop-info!\n");
		return CVI_FAILURE;
	}

	stCropInfo.bEnable = CVI_TRUE;
	if (CVI_VPSS_SetGrpCrop(0, &stCropInfo) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SetGrpCrop should not OK - invalid crop-info!\n");
		return CVI_FAILURE;
	}

	stCropInfo.stCropRect.s32X = 80;
	stCropInfo.stCropRect.s32Y = 60;
	stCropInfo.stCropRect.u32Width = stGrpAttr.u32MaxW + 8;
	stCropInfo.stCropRect.u32Height = stGrpAttr.u32MaxH;
	if (CVI_VPSS_SetGrpCrop(0, &stCropInfo) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SetGrpCrop should not OK - invalid crop-info!\n");
		return CVI_FAILURE;
	}

	stCropInfo.bEnable = CVI_TRUE;
	stCropInfo.stCropRect.s32X = 80;
	stCropInfo.stCropRect.s32Y = 60;
	stCropInfo.stCropRect.u32Width = stGrpAttr.u32MaxW / 2;
	stCropInfo.stCropRect.u32Height = stGrpAttr.u32MaxH / 2;
	if (CVI_VPSS_SetGrpCrop(0, &stCropInfo) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SetGrpCrop NG - crop-info!\n");
		return CVI_FAILURE;
	}

	if (CVI_VPSS_GetGrpProcAmp(0, PROC_AMP_MAX, &tmp) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetGrpProcAmp should not OK - invalid proc_amp_e!\n");
		return CVI_FAILURE;
	}

	if (CVI_VPSS_GetGrpProcAmp(0, PROC_AMP_BRIGHTNESS, &tmp) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetGrpProcAmp NG on grp0!\n");
		return CVI_FAILURE;
	}

	if (CVI_VPSS_GetGrpProcAmpCtrl(0, PROC_AMP_BRIGHTNESS, &stAmpCtrl) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetGrpProcAmpCtrl NG on grp0!\n");
		return CVI_FAILURE;
	}

	if (CVI_VPSS_SetGrpProcAmp(0, PROC_AMP_BRIGHTNESS, stAmpCtrl.maximum) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SetGrpProcAmp NG on grp0!\n");
		return CVI_FAILURE;
	}

	if (CVI_VPSS_SetGrpProcAmp(0, PROC_AMP_BRIGHTNESS, stAmpCtrl.maximum + 8) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SetGrpProcAmp should not OK - over max!\n");
		return CVI_FAILURE;
	}

	if (CVI_VPSS_SetGrpProcAmp(0, PROC_AMP_BRIGHTNESS, stAmpCtrl.minimum) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SetGrpProcAmp NG on grp0!\n");
		return CVI_FAILURE;
	}

	if (CVI_VPSS_SetGrpProcAmp(0, PROC_AMP_BRIGHTNESS, stAmpCtrl.minimum - 4) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SetGrpProcAmp should not OK - over min!\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 _1835_VPSS_CHN_PARAM_TEST(bool fail_pause)
{
	VPSS_GRP_ATTR_S stGrpAttr;
	VPSS_CHN_ATTR_S stChnAttr;
	VPSS_CROP_INFO_S stCropInfo;

	(void) fail_pause;

	// CVI_VPSS_GetChnAttr
	if (CVI_VPSS_GetChnAttr(7, 0, &stChnAttr) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnAttr should not OK - unexist grp7 chn0!\n");
		return CVI_FAILURE;
	}

	if (CVI_VPSS_GetChnAttr(0, 0, &stChnAttr) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnAttr NG on grp0 chn0!\n");
		return CVI_FAILURE;
	}

	if (CVI_VPSS_GetGrpAttr(0, &stGrpAttr) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetGrpAttr NG on grp0!\n");
		return CVI_FAILURE;
	}

	// CVI_VPSS_SetChnAttr
	stChnAttr.enPixelFormat = PIXEL_FORMAT_NV12;
	if (CVI_VPSS_SetChnAttr(0, 0, &stChnAttr) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SetChnAttr should not OK - invalid format!\n");
		return CVI_FAILURE;
	}

	stChnAttr.enPixelFormat = PIXEL_FORMAT_YUV_400;
	if (CVI_VPSS_SetChnAttr(0, 0, &stChnAttr) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SetChnAttr should NG!\n");
		return CVI_FAILURE;
	}

	stChnAttr.enPixelFormat = PIXEL_FORMAT_YUV_400;
	stChnAttr.stAspectRatio.enMode = ASPECT_RATIO_MANUAL;
	stChnAttr.stAspectRatio.stVideoRect.s32X = 0;
	stChnAttr.stAspectRatio.stVideoRect.s32Y = 0;
	stChnAttr.stAspectRatio.stVideoRect.u32Width = 0;
	stChnAttr.stAspectRatio.stVideoRect.u32Height = 0;
	if (CVI_VPSS_SetChnAttr(0, 0, &stChnAttr) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SetChnAttr should not OK - invalid ASPECT_RATIO_MANUAL!\n");
		return CVI_FAILURE;
	}

	stChnAttr.enPixelFormat = PIXEL_FORMAT_YUV_400;
	stChnAttr.stAspectRatio.enMode = ASPECT_RATIO_MANUAL;
	stChnAttr.stAspectRatio.stVideoRect.s32X = 40;
	stChnAttr.stAspectRatio.stVideoRect.s32Y = 0;
	stChnAttr.stAspectRatio.stVideoRect.u32Width = stChnAttr.u32Width;
	stChnAttr.stAspectRatio.stVideoRect.u32Height = stChnAttr.u32Height;
	if (CVI_VPSS_SetChnAttr(0, 0, &stChnAttr) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SetChnAttr should not OK - invalid ASPECT_RATIO_MANUAL!\n");
		return CVI_FAILURE;
	}

	stChnAttr.stAspectRatio.enMode = ASPECT_RATIO_MANUAL;
	stChnAttr.stAspectRatio.stVideoRect.s32X = 40;
	stChnAttr.stAspectRatio.stVideoRect.s32Y = 0;
	stChnAttr.stAspectRatio.stVideoRect.u32Width = stChnAttr.u32Width / 2;
	stChnAttr.stAspectRatio.stVideoRect.u32Height = stChnAttr.u32Height;
	if (CVI_VPSS_SetChnAttr(0, 0, &stChnAttr) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SetChnAttr NG - ASPECT_RATIO_NONE!\n");
		return CVI_FAILURE;
	}

	stChnAttr.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stChnAttr.stAspectRatio.stVideoRect.s32X = 40;
	stChnAttr.stAspectRatio.stVideoRect.s32Y = 0;
	stChnAttr.stAspectRatio.stVideoRect.u32Width = stChnAttr.u32Width;
	stChnAttr.stAspectRatio.stVideoRect.u32Height = stChnAttr.u32Height;
	if (CVI_VPSS_SetChnAttr(0, 0, &stChnAttr) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SetChnAttr NG - ASPECT_RATIO_NONE!\n");
		return CVI_FAILURE;
	}

	// CVI_VPSS_GetChnCrop
	if (CVI_VPSS_GetChnCrop(8, 0, &stCropInfo) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnCrop should not OK - unexist grp8!\n");
		return CVI_FAILURE;
	}

	if (CVI_VPSS_GetChnCrop(0, 0, &stCropInfo) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnCrop NG on grp0!\n");
		return CVI_FAILURE;
	}

	// CVI_VPSS_SetChnCrop
	stCropInfo.bEnable = CVI_FALSE;
	stCropInfo.stCropRect.s32X = -4;
	stCropInfo.stCropRect.s32Y = -4;
	stCropInfo.stCropRect.u32Width = 6;
	stCropInfo.stCropRect.u32Height = 6;
	if (CVI_VPSS_SetChnCrop(0, 0, &stCropInfo) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SetChnCrop NG - crop disable on invalid crop-info!\n");
		return CVI_FAILURE;
	}

	stCropInfo.bEnable = CVI_TRUE;
	if (CVI_VPSS_SetChnCrop(0, 0, &stCropInfo) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SetChnCrop should not OK - invalid crop-info!\n");
		return CVI_FAILURE;
	}

	stCropInfo.stCropRect.s32X = stGrpAttr.u32MaxW - 2;
	stCropInfo.stCropRect.s32Y = stGrpAttr.u32MaxH - 2;
	stCropInfo.stCropRect.u32Width = 10;
	stCropInfo.stCropRect.u32Height = 10;
	if (CVI_VPSS_SetChnCrop(0, 0, &stCropInfo) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SetChnCrop should not OK - invalid crop-info!\n");
		return CVI_FAILURE;
	}

	stCropInfo.bEnable = CVI_TRUE;
	stCropInfo.stCropRect.s32X = 80;
	stCropInfo.stCropRect.s32Y = 60;
	stCropInfo.stCropRect.u32Width = stGrpAttr.u32MaxW / 2;
	stCropInfo.stCropRect.u32Height = stGrpAttr.u32MaxH / 2;
	if (CVI_VPSS_SetChnCrop(0, 0, &stCropInfo) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SetChnCrop NG - crop-info!\n");
		return CVI_FAILURE;
	}

	// enable grp crop to test.
	stCropInfo.bEnable = CVI_TRUE;
	stCropInfo.stCropRect.s32X = 80;
	stCropInfo.stCropRect.s32Y = 60;
	stCropInfo.stCropRect.u32Width = stGrpAttr.u32MaxW / 2;
	stCropInfo.stCropRect.u32Height = stGrpAttr.u32MaxH / 2;
	if (CVI_VPSS_SetGrpCrop(0, &stCropInfo) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SetGrpCrop NG - crop-info!\n");
		return CVI_FAILURE;
	}

	stCropInfo.bEnable = CVI_TRUE;
	stCropInfo.stCropRect.s32X = stGrpAttr.u32MaxW / 2 - 8;
	stCropInfo.stCropRect.s32Y = stGrpAttr.u32MaxH / 2;
	stCropInfo.stCropRect.u32Width = 10;
	stCropInfo.stCropRect.u32Height = 10;
	if (CVI_VPSS_SetChnCrop(0, 0, &stCropInfo) == CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SetChnCrop should not OK - invalid crop-info!\n");
		return CVI_FAILURE;
	}

	stCropInfo.bEnable = CVI_TRUE;
	stCropInfo.stCropRect.s32X = 80;
	stCropInfo.stCropRect.s32Y = 60;
	stCropInfo.stCropRect.u32Width = stGrpAttr.u32MaxW / 2;
	stCropInfo.stCropRect.u32Height = stGrpAttr.u32MaxH / 2;
	if (CVI_VPSS_SetChnCrop(0, 0, &stCropInfo) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SetChnCrop NG - crop-info!\n");
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}

static CVI_S32 _1835_VPSS_YUV422_RGB_GOLDEN2K_DUAL_TEST(bool fail_pause)
{
	SIZE_S stSize = {.u32Width = 1920, .u32Height = 1080};
	VPSS_GRP_ATTR_S    stVpssGrpAttr;
	VPSS_GRP	   VpssGrp = 5;
	VPSS_CHN	   VpssChn = VPSS_CHN0;
	CVI_BOOL	   abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
	VIDEO_FRAME_INFO_S stVideoFrame;
	CVI_CHAR filename[64];
	CVI_S32 s32Ret = CVI_SUCCESS;
	bool is_result_match = CVI_TRUE;

	// VpssGrp 5
	//  1920x1080 yuv422 input
	//  ch0: 736x1280 rgb planar + flip
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = PIXEL_FORMAT_YUV_PLANAR_422;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;

	astVpssChnAttr[VpssChn].u32Width		    = 736;
	astVpssChnAttr[VpssChn].u32Height		    = 1280;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_RGB_888_PLANAR;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	CVI_VPSS_SetChnScaleCoefLevel(VpssGrp, VpssChn, VPSS_SCALE_COEF_OPENCV_BILINEAR);

	SAMPLE_COMM_VPSS_SendFrame(VpssGrp, &stSize, PIXEL_FORMAT_YUV_PLANAR_422, filenames[5]);
	for (VpssChn = 0; VpssChn < 1; ++VpssChn) {
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, 1000);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnFrame for grp%d chn%d. s32Ret: 0x%x !\n"
				, VpssGrp, VpssChn, s32Ret);
			is_result_match = CVI_FALSE;
			continue;
		}

		snprintf(filename, 32, "./res/vpss_%d_%d.bin", VpssGrp, VpssChn);
		if (!SAMPLE_COMM_FRAME_CompareWithFile(filename, &stVideoFrame)) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "grp%d chn%d. mismatch!\n", VpssGrp, VpssChn);
			is_result_match = CVI_FALSE;

			snprintf(filename, 32, "./res/vpss_ng_%d_%d.bin", VpssGrp, VpssChn);
			SAMPLE_COMM_FRAME_SaveToFile(filename, &stVideoFrame);

			if (fail_pause)
				PAUSE();
		}

		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);
	}

	// VpssGrp 6
	//  1920x1080 bgr packed input
	//  ch0: 608x608 rgb planar + aspect_ratio_auto + normalize + bgcolor
	//  ch1: 320x240 bgr packed + aspect_ratio_auto + bgcolor
	//  ch2: 1920x1080 yuv422 + mirror
	VpssGrp = 6;
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = PIXEL_FORMAT_BGR_888;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;

	VpssChn = 0;
	astVpssChnAttr[VpssChn].u32Width		    = 608;
	astVpssChnAttr[VpssChn].u32Height		    = 608;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_RGB_888_PLANAR;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_TRUE;
	astVpssChnAttr[VpssChn].stNormalize.factor[0] = 0.8415;
	astVpssChnAttr[VpssChn].stNormalize.factor[1] = 0.8415;
	astVpssChnAttr[VpssChn].stNormalize.factor[2] = 0.8415;
	astVpssChnAttr[VpssChn].stNormalize.mean[0] = 102.9801;
	astVpssChnAttr[VpssChn].stNormalize.mean[1] = 115.9465;
	astVpssChnAttr[VpssChn].stNormalize.mean[2] = 122.7717;
	astVpssChnAttr[VpssChn].stNormalize.rounding = VPSS_ROUNDING_TO_EVEN;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;

	VpssChn = 1;
	astVpssChnAttr[VpssChn].u32Width		    = 320;
	astVpssChnAttr[VpssChn].u32Height		    = 240;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_BGR_888;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	VpssChn = 2;
	astVpssChnAttr[VpssChn].u32Width		    = 1920;
	astVpssChnAttr[VpssChn].u32Height		    = 1080;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_YUV_PLANAR_422;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_TRUE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_NONE;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = abChnEnable[1] = abChnEnable[2] = CVI_TRUE;
	abChnEnable[3] = CVI_FALSE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	SAMPLE_COMM_VPSS_SendFrame(VpssGrp, &stSize, PIXEL_FORMAT_BGR_888, filenames[1]);
	for (VpssChn = 0; VpssChn < 3; ++VpssChn) {
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, 1000);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnFrame for grp%d chn%d. s32Ret: 0x%x !\n"
				, VpssGrp, VpssChn, s32Ret);
			is_result_match = CVI_FALSE;
			continue;
		}

		snprintf(filename, 32, "./res/vpss_%d_%d.bin", VpssGrp, VpssChn);
		if (!SAMPLE_COMM_FRAME_CompareWithFile(filename, &stVideoFrame)) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "grp%d chn%d. mismatch!\n", VpssGrp, VpssChn);
			is_result_match = CVI_FALSE;

			snprintf(filename, 32, "./res/vpss_ng_%d_%d.bin", VpssGrp, VpssChn);
			SAMPLE_COMM_FRAME_SaveToFile(filename, &stVideoFrame);

			if (fail_pause)
				PAUSE();
		}

		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);
	}

	// VpssGrp 7
	//  1920x1080 yuv420 input
	//  ch0: 608x608 HSV
	//  ch1: 320x240 Y-only + aspect_ratio_auto + bgcolor
	//  ch2: 1920x1080 bgr planar + mirror
	VpssGrp = 7;
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = PIXEL_FORMAT_YUV_PLANAR_420;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;

	VpssChn = 0;
	astVpssChnAttr[VpssChn].u32Width		    = 608;
	astVpssChnAttr[VpssChn].u32Height		    = 608;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_HSV_888_PLANAR;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_NONE;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	VpssChn = 1;
	astVpssChnAttr[VpssChn].u32Width		    = 320;
	astVpssChnAttr[VpssChn].u32Height		    = 240;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_YUV_400;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	VpssChn = 2;
	astVpssChnAttr[VpssChn].u32Width		    = 1920;
	astVpssChnAttr[VpssChn].u32Height		    = 1080;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_BGR_888_PLANAR;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 1;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_TRUE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_NONE;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = abChnEnable[1] = abChnEnable[2] = CVI_TRUE;
	abChnEnable[3] = CVI_FALSE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	SAMPLE_COMM_VPSS_SendFrame(VpssGrp, &stSize, PIXEL_FORMAT_YUV_PLANAR_420, filenames[4]);
	for (VpssChn = 0; VpssChn < 3; ++VpssChn) {
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, 1000);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnFrame for grp%d chn%d. s32Ret: 0x%x !\n"
				, VpssGrp, VpssChn, s32Ret);
			is_result_match = CVI_FALSE;
			continue;
		}

		snprintf(filename, 32, "./res/vpss_%d_%d.bin", VpssGrp, VpssChn);
		if (!SAMPLE_COMM_FRAME_CompareWithFile(filename, &stVideoFrame)) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "grp%d chn%d. mismatch!\n", VpssGrp, VpssChn);
			is_result_match = CVI_FALSE;

			snprintf(filename, 32, "./res/vpss_ng_%d_%d.bin", VpssGrp, VpssChn);
			SAMPLE_COMM_FRAME_SaveToFile(filename, &stVideoFrame);

			if (fail_pause)
				PAUSE();
		}

		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);
	}
	return (is_result_match) ? CVI_SUCCESS : CVI_FAILURE;
}

static CVI_S32 _1835_VPSS_GRP_ATTR_STRESS_TEST(void)
{
	VPSS_GRP VpssGrp = 0;
	VPSS_GRP_ATTR_S stVpssGrpAttr;
	VPSS_CHN VpssChn = VPSS_CHN0;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
	CVI_U32 w, h, i, w_gap, h_gap;
	PIXEL_FORMAT_E pixFmt;
	VIDEO_FRAME_INFO_S stVideoFrame, stVideoFrameOut;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32TotalSteps = 258; // w/h take total 258 points to test from min to max

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate	= -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate	= -1;
	stVpssGrpAttr.enPixelFormat			= PIXEL_FORMAT_RGB_888_PLANAR;
	stVpssGrpAttr.u32MaxW				= 256;
	stVpssGrpAttr.u32MaxH				= 256;

	//chn attr setting 1
	astVpssChnAttr[0].u32Width			= 256;
	astVpssChnAttr[0].u32Height			= 256;
	astVpssChnAttr[0].enVideoFormat			= VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[0].enPixelFormat			= PIXEL_FORMAT_RGB_888_PLANAR;
	astVpssChnAttr[0].stFrameRate.s32SrcFrameRate   = 30;
	astVpssChnAttr[0].stFrameRate.s32DstFrameRate   = 30;
	astVpssChnAttr[0].u32Depth			= 1;
	astVpssChnAttr[0].bMirror			= CVI_FALSE;
	astVpssChnAttr[0].bFlip				= CVI_FALSE;
	astVpssChnAttr[0].stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	astVpssChnAttr[0].stAspectRatio.bEnableBgColor  = CVI_TRUE;
	astVpssChnAttr[0].stAspectRatio.u32BgColor	= COLOR_RGB_BLACK;
	astVpssChnAttr[0].stNormalize.bEnable		= CVI_FALSE;

	//chn attr setting 2
	astVpssChnAttr[1].u32Width			= 1920;
	astVpssChnAttr[1].u32Height			= 1080;
	astVpssChnAttr[1].enVideoFormat			= VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[1].enPixelFormat			= PIXEL_FORMAT_YUV_PLANAR_422;
	astVpssChnAttr[1].stFrameRate.s32SrcFrameRate   = 30;
	astVpssChnAttr[1].stFrameRate.s32DstFrameRate   = 30;
	astVpssChnAttr[1].u32Depth			= 1;
	astVpssChnAttr[1].bMirror			= CVI_TRUE;
	astVpssChnAttr[1].bFlip				= CVI_FALSE;
	astVpssChnAttr[1].stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	astVpssChnAttr[1].stAspectRatio.bEnableBgColor  = CVI_TRUE;
	astVpssChnAttr[1].stAspectRatio.u32BgColor	= COLOR_RGB_BLACK;
	astVpssChnAttr[1].stNormalize.bEnable		= CVI_FALSE;

	/*start vpss grp0 chn0*/
	abChnEnable[0] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	for (i = 0; i < 2; ++i) { // Use different chn attr settings
		CVI_U32 u32MaxGrpW = MIN(astVpssChnAttr[i].u32Width*32, VPSS_MAX_IMAGE_WIDTH);
		CVI_U32 u32MaxGrpH = MIN(astVpssChnAttr[i].u32Height*32, VPSS_MAX_IMAGE_HEIGHT);

		w_gap = (u32MaxGrpW - VPSS_MIN_IMAGE_WIDTH) / (u32TotalSteps - 1);
		h_gap = (u32MaxGrpH - 1) / (u32TotalSteps - 1);
		for (pixFmt = 0; pixFmt < PIXEL_FORMAT_MAX; ++pixFmt) { // for each grp pixel format
			CVI_BOOL isYuv420 = IS_FMT_YUV420(pixFmt);
			CVI_BOOL isYuv422 = IS_FMT_YUV422(pixFmt);

			if (!VPSS_GRP_SUPPORT_FMT(pixFmt))
				continue;

			for (w = 0; w < u32TotalSteps; ++w) { // for each input width
				CVI_U32 w_in = (w == u32TotalSteps - 1)
					? (u32MaxGrpW) : (VPSS_MIN_IMAGE_WIDTH + w*w_gap);

				if (isYuv420 || isYuv422)
					w_in = (w_in % 2 == 0) ? w_in : w_in + 1;

				for (h = 0; h < u32TotalSteps; ++h) { // for each input height
					CVI_U32 h_in = (h == u32TotalSteps - 1) ? (u32MaxGrpH) : (1 + h*h_gap);

					if (isYuv420)
						h_in = (h_in % 2 == 0) ? h_in : h_in + 1;

					// update grp/chn attr pixfmt/w/h
					stVpssGrpAttr.enPixelFormat = pixFmt;
					stVpssGrpAttr.u32MaxW = w_in;
					stVpssGrpAttr.u32MaxH = h_in;
					CVI_VPSS_SetGrpAttr(VpssGrp, &stVpssGrpAttr);
					CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &astVpssChnAttr[i]);

					// prepare frame corresponding to current grp pixFmt/w/h
					SIZE_S stSize = {.u32Width = w_in, .u32Height = h_in};
					VB_BLK blk;

					SAMPLE_COMM_PrepareFrame(stSize, pixFmt, &stVideoFrame);

					CVI_VPSS_SendFrame(VpssGrp, &stVideoFrame, -1);
					blk = CVI_VB_PhysAddr2Handle(stVideoFrame.stVFrame.u64PhyAddr[0]);
					CVI_VB_ReleaseBlock(blk);
					s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, 2000);
					CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
					if (s32Ret != CVI_SUCCESS) {
						CVI_TRACE_LOG(CVI_DBG_ERR,
							"CVI_VPSS_GetChnFrame fail, grp(%d, %d, %d), chn(%d, %d, %d), Ret: 0x%x!\n"
							, w_in, h_in, pixFmt, astVpssChnAttr[i].u32Width
							, astVpssChnAttr[i].u32Height
							, astVpssChnAttr[i].enPixelFormat, s32Ret);
						return s32Ret;
					}
				}
			}
		}
	}

	return s32Ret;
}

static CVI_S32 _1835_VPSS_CHN_ATTR_STRESS_TEST(void)
{
	VPSS_GRP VpssGrp = 0;
	VPSS_GRP_ATTR_S stVpssGrpAttr[2];
	VPSS_CHN VpssChn = VPSS_CHN0;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
	CVI_U32 w, h, i, w_gap, h_gap;
	PIXEL_FORMAT_E pixFmt;
	VIDEO_FRAME_INFO_S stVideoFrame, stVideoFrameOut;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32TotalSteps = 258; // w/h take total 258 points to test from min to max

	//grp attr setting 1
	stVpssGrpAttr[0].stFrameRate.s32SrcFrameRate	= -1;
	stVpssGrpAttr[0].stFrameRate.s32DstFrameRate	= -1;
	stVpssGrpAttr[0].enPixelFormat			= PIXEL_FORMAT_RGB_888_PLANAR;
	stVpssGrpAttr[0].u32MaxW			= 256;
	stVpssGrpAttr[0].u32MaxH			= 256;

	//grp attr setting 2
	stVpssGrpAttr[1].stFrameRate.s32SrcFrameRate	= -1;
	stVpssGrpAttr[1].stFrameRate.s32DstFrameRate	= -1;
	stVpssGrpAttr[1].enPixelFormat			= PIXEL_FORMAT_YUV_PLANAR_420;
	stVpssGrpAttr[1].u32MaxW			= 1920;
	stVpssGrpAttr[1].u32MaxH			= 1080;

	astVpssChnAttr[VpssChn].u32Width			= 256;
	astVpssChnAttr[VpssChn].u32Height			= 256;
	astVpssChnAttr[VpssChn].enVideoFormat			= VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat			= PIXEL_FORMAT_RGB_888_PLANAR;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate     = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate     = 30;
	astVpssChnAttr[VpssChn].u32Depth			= 1;
	astVpssChnAttr[VpssChn].bMirror				= CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip				= CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor    = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor	= COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable		= CVI_FALSE;

	/*start vpss grp0 chn0*/
	abChnEnable[0] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr[0], astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr[0], astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	for (i = 0; i < 2; ++i) { // Use different grp attr settings
		CVI_U32 u32MinChnW = MAX((stVpssGrpAttr[i].u32MaxW/32 + 1), VPSS_MIN_IMAGE_WIDTH);
		CVI_U32 u32MinChnH = MAX((stVpssGrpAttr[i].u32MaxH/32 + 1), 1);

		w_gap = (VPSS_MAX_IMAGE_WIDTH - u32MinChnW) / (u32TotalSteps - 1);
		h_gap = (VPSS_MAX_IMAGE_HEIGHT - u32MinChnH) / (u32TotalSteps - 1);
		for (pixFmt = 0; pixFmt < PIXEL_FORMAT_MAX; ++pixFmt) { // for each chn pixel format
			CVI_BOOL isYuv420 = IS_FMT_YUV420(pixFmt);
			CVI_BOOL isYuv422 = IS_FMT_YUV422(pixFmt);

			if (!VPSS_CHN_SUPPORT_FMT(pixFmt))
				continue;

			for (w = 0; w < u32TotalSteps; ++w) { // for each output width
				CVI_U32 w_out = (w == u32TotalSteps - 1)
					? (VPSS_MAX_IMAGE_WIDTH) : (u32MinChnW + w*w_gap);

				if (isYuv420 || isYuv422)
					w_out = (w_out % 2 == 0) ? w_out : w_out + 1;

				for (h = 0; h < u32TotalSteps; ++h) { // for each output height
					CVI_U32 h_out = (h == u32TotalSteps - 1)
						? (VPSS_MAX_IMAGE_HEIGHT) : (u32MinChnH + h*h_gap);

					if (isYuv420)
						h_out = (h_out % 2 == 0) ? h_out : h_out + 1;

					// update grp/chn attr pixfmt/w/h
					CVI_VPSS_SetGrpAttr(VpssGrp, &stVpssGrpAttr[i]);
					astVpssChnAttr[VpssChn].enPixelFormat = pixFmt;
					astVpssChnAttr[VpssChn].u32Width = w_out;
					astVpssChnAttr[VpssChn].u32Height = h_out;
					CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &astVpssChnAttr[VpssChn]);

					// prepare frame corresponding to current grp pixFmt/w/h
					SIZE_S stSize;
					VB_BLK blk;

					stSize.u32Width = stVpssGrpAttr[i].u32MaxW;
					stSize.u32Height = stVpssGrpAttr[i].u32MaxH;
					SAMPLE_COMM_PrepareFrame(stSize, stVpssGrpAttr[i].enPixelFormat, &stVideoFrame);

					CVI_VPSS_SendFrame(VpssGrp, &stVideoFrame, -1);
					blk = CVI_VB_PhysAddr2Handle(stVideoFrame.stVFrame.u64PhyAddr[0]);
					CVI_VB_ReleaseBlock(blk);
					s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, 2000);
					CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
					if (s32Ret != CVI_SUCCESS) {
						CVI_TRACE_LOG(CVI_DBG_ERR,
							"CVI_VPSS_GetChnFrame fail, grp(%d, %d, %d), chn(%d, %d, %d), Ret: 0x%x!\n"
							, stVpssGrpAttr[i].u32MaxW, stVpssGrpAttr[i].u32MaxH
							, stVpssGrpAttr[i].enPixelFormat, w_out, h_out, pixFmt, s32Ret);
						return s32Ret;
					}
				}
			}
		}
	}

	return s32Ret;
}

struct vpss_test_ops vpss_test_ops_1835 = {
	.vi_test = _1835_VPSS_VI_TEST,
	.in_fmt_test = _1835_VPSS_FMT_INPUT_TEST,
	.out_fmt_test = _1835_VPSS_FMT_OUTPUT_TEST,
	.yuv_2k_test = _1835_VPSS_YUV_GOLDEN2K_TEST,
	.yuv_4k_test = _1835_VPSS_YUV_GOLDEN4K_TEST,
	.rgb_test = _1835_VPSS_RGB_GOLDEN2K_TEST,
	.dual_yuv_test = _1835_VPSS_YUV_GOLDEN2K_DUAL_TEST,
	.dual_combo_test = _1835_VPSS_YUV422_RGB_GOLDEN2K_DUAL_TEST,
	.param_grp_test = _1835_VPSS_GRP_PARAM_TEST,
	.param_chn_test = _1835_VPSS_CHN_PARAM_TEST,
	.grp_attr_stress_test = _1835_VPSS_GRP_ATTR_STRESS_TEST,
	.chn_attr_stress_test = _1835_VPSS_CHN_ATTR_STRESS_TEST,
};

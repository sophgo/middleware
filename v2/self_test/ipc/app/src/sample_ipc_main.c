#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <errno.h>
#include <syslog.h>
#include <inttypes.h>

#include "sample_comm.h"
#include "cvi_sys.h"
#include <cvi_type.h>
#include "fontmod.h"
#ifdef __ENABLE_RTSP__
#include "rtsp.h"
#endif

#include "cvi_sys.h"
#include "cvi_sns_ctrl.h"

#define USING_INI_CFG
// #define _SINGLETHREAD_FORALLSTREAM_
// #define _USE_GETFD_
#define TEST_INTERVAL 1

#define SCALE_WIDTH 1920
#define SCALE_HEIGHT 1080


#define MAX_FILE_LEN 64
#define MAX_STR_LEN 32
#define MAX_RTPS_SERVER 6


#define IsASCII(a)				(((a) >= 0x00 && (a) <= 0x7F) ? 1 : 0)
#define BYTE_BITS				8
#define NOASCII_CHARACTER_BYTES	2
#define OSD_LIB_FONT_W			24
#define OSD_LIB_FONT_H			24

#if (VPSS_MAX_PHY_CHN_NUM < 4)
#define ARCH_CV1822
#endif
typedef struct _samplertspctx_ {
#ifdef __ENABLE_RTSP__
	CVI_RTSP_CTX *pstServerCtx;
	CVI_RTSP_SESSION *pVideoSession;
#endif
	VENC_CHN VencChn;
	CVI_BOOL bStart;
} SAMPLE_RTSP_CTX;

typedef struct _samplevencctx_ {
	CVI_S32 s32VencChnCnt;
	vencChnCtx astChnCtx[VENC_MAX_CHN_NUM];
} SAMPLE_VENC_CTX;

typedef enum _osdtype_ {
	TYPE_PICTURE,
	TYPE_STRING,
	TYPE_TIME,
	TYPE_END
} OSD_TYPE_E;

typedef struct _sampleosdattr_ {
	RGN_HANDLE Handle;
	OSD_TYPE_E enType;
	MMF_CHN_S stChn;
	RECT_S stRect;
	union {
		char filename[MAX_FILE_LEN];
		char str[MAX_STR_LEN];
	};
} SAMPLE_OSD_ATTR_S;

static pthread_t pthTime;
static CVI_BOOL bRunTime;
static SAMPLE_RTSP_CTX s_astRtspCtx[MAX_RTPS_SERVER];
static SAMPLE_VI_CONFIG_S stViConfig;
static SAMPLE_VENC_CTX stVencCtx;
static pthread_t gs_IpcVencTask[VENC_MAX_CHN_NUM];
#ifdef _SINGLETHREAD_FORALLSTREAM_
static pthread_t gs_IpcVencAllTask;
#endif
static CVI_BOOL g_UseSelect = CVI_FALSE;

CVI_U32 INPUT_WIDTH = 3840;
CVI_U32 INPUT_HEIGHT = 2160;
int low_mem_profile;

#define RES2_WIDTH 1920
#define RES2_HEIGHT 1080
#ifdef ARCH_CV1822
#define  CODEC_TYPE1 "264"
#define  CODEC_TYPE2 "mjp"
#define  CODEC_TYPE2_2 "264"
#else
#define  CODEC_TYPE1 "265"
#define  CODEC_TYPE2 "264"
#define  CODEC_TYPE2_2 "265"
#endif

chnInputCfg IpcVencChnCfg[] = {
	{
		.codec = CODEC_TYPE1,
		.width = 3840,
		.height = 2160,
		.bind_mode = VENC_BIND_VPSS,
		.vpssGrp = 1,
		.vpssChn = 0,
		.output_path = "/mnt/data/venc0_4k",
		.outputFileName = "",
		.num_frames = -1,
		.bsMode = 0,
		.rcMode = 0,
		.iqp = 30,
		.pqp = 30,
		.gop = 60,
		.bitrate = 8000,
		.firstFrmstartQp = 30,
		.minIqp = -1,
		.maxIqp = -1,
		.minQp = -1,
		.maxQp = -1,
		.srcFramerate = 25,
		.framerate = 25,
		.bVariFpsEn = 0,
		.maxbitrate = -1,
		.statTime = -1,
		.chgNum = -1,
		.quality = -1,
		.pixel_format = 0,
		.bitstreamBufSize = 0,
		.single_LumaBuf = 0,
		.single_core = 0,
		.forceIdr = -1,
		.tempLayer = 0,
		.testRoi = 0,
		.gopMode = 0,
	},

	{	.codec = CODEC_TYPE2,
		.width = RES2_WIDTH,
		.height = RES2_HEIGHT,
		.bind_mode = VENC_BIND_VPSS,
		.vpssGrp = 1,
		.vpssChn = 1,
		.output_path = "/mnt/data/venc1_1080p",
		.outputFileName = "",
		.num_frames = -1,
		.bsMode = 0,
		.rcMode = 0,
		.iqp = 30,
		.pqp = 30,
		.gop = 60,
		.bitrate = 2000,
		.firstFrmstartQp = 30,
		.minIqp = -1,
		.maxIqp = -1,
		.minQp = -1,
		.maxQp = -1,
		.srcFramerate = 30,
		.framerate = 30,
		.bVariFpsEn = 0,
		.maxbitrate = -1,
		.statTime = -1,
		.chgNum = -1,
		.quality = -1,
		.pixel_format = 0,
		.bitstreamBufSize = 0,
		.single_LumaBuf = 0,
		.single_core = 0,
		.forceIdr = -1,
		.tempLayer = 0,
		.testRoi = 0,
		.gopMode = 0,
	},
#if (VPSS_MAX_PHY_CHN_NUM > 3)
	{	.codec = "mjp",
		.width = 1920,
		.height = 1080,
		.bind_mode = VENC_BIND_VPSS,
		.vpssGrp = 1,
		.vpssChn = 2,
		.output_path = "/mnt/data/venc2_1080p",
		.outputFileName = "",
		.num_frames = -1,
		.bsMode = 0,
		.rcMode = 0,
		.iqp = -1,
		.pqp = -1,
		.gop = -1,
		.bitrate = 1000,
		.firstFrmstartQp = 30,
		.minIqp = -1,
		.maxIqp = -1,
		.minQp = -1,
		.maxQp = -1,
		.srcFramerate = 25,
		.framerate = 25,
		.bVariFpsEn = 0,
		.maxbitrate = -1,
		.statTime = -1,
		.chgNum = -1,
		.quality = 60,
		.pixel_format = 0,
		.bitstreamBufSize = 0,
		.single_LumaBuf = 0,
		.single_core = 0,
		.forceIdr = -1,
		.tempLayer = 0,
		.testRoi = 0,
		.gopMode = 0,
	},
#endif
	{	.codec = "264",
		.width = 720,
		.height = 576,
		.bind_mode = VENC_BIND_VPSS,
		.vpssGrp = 2,
		.vpssChn = 0,
		.output_path = "/mnt/data/venc3_720x576",
		.outputFileName = "",
		.num_frames = -1,
		.bsMode = 0,
		.rcMode = 0,
		.iqp = 30,
		.pqp = 30,
		.gop = 60,
		.bitrate = 1000,
		.firstFrmstartQp = 30,
		.minIqp = -1,
		.maxIqp = -1,
		.minQp = -1,
		.maxQp = -1,
		.srcFramerate = 25,
		.framerate = 25,
		.bVariFpsEn = 0,
		.maxbitrate = -1,
		.statTime = -1,
		.chgNum = -1,
		.quality = -1,
		.pixel_format = 0,
		.bitstreamBufSize = 0,
		.single_LumaBuf = 0,
		.single_core = 0,
		.forceIdr = -1,
		.tempLayer = 0,
		.testRoi = 0,
		.gopMode = 0,
	},

	{	.codec = "264",
		.width = 720,
		.height = 576,
		.bind_mode = VENC_BIND_VPSS,
		.vpssGrp = 2,
		.vpssChn = 1,
		.output_path = "/mnt/data/venc4_720x576",
		.outputFileName = "",
		.num_frames = -1,
		.bsMode = 0,
		.rcMode = 0,
		.iqp = 30,
		.pqp = 30,
		.gop = 60,
		.bitrate = 1000,
		.firstFrmstartQp = 30,
		.minIqp = -1,
		.maxIqp = -1,
		.minQp = -1,
		.maxQp = -1,
		.srcFramerate = 25,
		.framerate = 25,
		.bVariFpsEn = 0,
		.maxbitrate = -1,
		.statTime = -1,
		.chgNum = -1,
		.quality = -1,
		.pixel_format = 0,
		.bitstreamBufSize = 0,
		.single_LumaBuf = 0,
		.single_core = 0,
		.forceIdr = -1,
		.tempLayer = 0,
		.testRoi = 0,
		.gopMode = 0,
	},
#if (VPSS_MAX_PHY_CHN_NUM > 3)
	{	.codec = "264",
		.width = 720,
		.height = 576,
		.bind_mode = VENC_BIND_VPSS,
		.vpssGrp = 2,
		.vpssChn = 2,
		.output_path = "/mnt/data/venc5_720x576",
		.outputFileName = "",
		.num_frames = -1,
		.bsMode = 0,
		.rcMode = 0,
		.iqp = 30,
		.pqp = 30,
		.gop = 60,
		.bitrate = 1000,
		.firstFrmstartQp = 30,
		.minIqp = -1,
		.maxIqp = -1,
		.minQp = -1,
		.maxQp = -1,
		.srcFramerate = 25,
		.framerate = 25,
		.bVariFpsEn = 0,
		.maxbitrate = -1,
		.statTime = -1,
		.chgNum = -1,
		.quality = -1,
		.pixel_format = 0,
		.bitstreamBufSize = 0,
		.single_LumaBuf = 0,
		.single_core = 0,
		.forceIdr = -1,
		.tempLayer = 0,
		.testRoi = 0,
		.gopMode = 0,
	},
#endif
#if 1
	{
		.codec = "jpg",
		.width = 320,
		.height = 320,
		.bind_mode = VENC_BIND_DISABLE,
		.vpssGrp = -1,
		.vpssChn = -1,
		.output_path = "/mnt/data/venc6_320x320",
		.outputFileName = "",
		.num_frames = -1,
		.bsMode = 0,
		.rcMode = 4,
		.iqp = 46,
		.pqp = 46,
		.gop = -1,
		.bitrate = 8000,
		.firstFrmstartQp = -1,
		.minIqp = 30,
		.maxIqp = 60,
		.minQp = 30,
		.maxQp = 60,
		.srcFramerate = 30,
		.framerate = 30,
		.bVariFpsEn = 0,
		.maxbitrate = -1,
		.statTime = -1,
		.chgNum = -1,
		.quality = 20,
		.pixel_format = (SAMPLE_PIXEL_FORMAT == PIXEL_FORMAT_NV21) ? 3 : 0,
		.bitstreamBufSize = 0,
		.single_LumaBuf = 0,
		.single_core = 0,
		.forceIdr = -1,
		.tempLayer = 0,
		.testRoi = 0,
		.gopMode = 0,
	},
#endif
};


SAMPLE_OSD_ATTR_S IpcOSDcfg[] = {
	//grp1 chn0 3840x2160
	{
		.Handle = 0,
		.enType = TYPE_TIME,
		.stChn = {CVI_ID_VPSS, 1, 0},
		.stRect = {20, 20, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
	},
	{
		.Handle = 1,
		.enType = TYPE_STRING,
		.stChn = {CVI_ID_VPSS, 1, 0},
		.stRect = {20, 300, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
		.str = "cvitek",
	},
	{
		.Handle = 2,
		.enType = TYPE_STRING,
		.stChn = {CVI_ID_VPSS, 1, 0},
		.stRect = {1024, 20, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
		.str = "E:113.95 N:22.53",
	},
	{
		.Handle = 3,
		.enType = TYPE_STRING,
		.stChn = {CVI_ID_VPSS, 1, 0},
		.stRect = {1024, 300, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
		.str = "IPCamera",
	},
	{
		.Handle = 4,
		.enType = TYPE_PICTURE,
		.stChn = {CVI_ID_VPSS, 1, 0},
		.stRect = {1024, 60, 300, 200},
		.filename = "tiger.bmp",
	},

	//grp1 chn1 1920x1080
	{
		.Handle = 5,
		.enType = TYPE_TIME,
		.stChn = {CVI_ID_VPSS, 1, 1},
		.stRect = {20, 20, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
	},
	{
		.Handle = 6,
		.enType = TYPE_STRING,
		.stChn = {CVI_ID_VPSS, 1, 1},
		.stRect = {20, 60, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
		.str = "cvitek",
	},
	{
		.Handle = 7,
		.enType = TYPE_STRING,
		.stChn = {CVI_ID_VPSS, 1, 1},
		.stRect = {20, 100, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
		.str = "E:113.95 N:22.53",
	},
	{
		.Handle = 8,
		.enType = TYPE_STRING,
		.stChn = {CVI_ID_VPSS, 1, 1},
		.stRect = {20, 200, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
		.str = "IPCamera",
	},
	{
		.Handle = 9,
		.enType = TYPE_PICTURE,
		.stChn = {CVI_ID_VPSS, 1, 1},
		.stRect = {20, 300, 300, 200},
		.filename = "tiger.bmp",
	},
#if (VPSS_MAX_PHY_CHN_NUM > 3)
	//grp1 chn2 1920x1080
	{
		.Handle = 10,
		.enType = TYPE_TIME,
		.stChn = {CVI_ID_VPSS, 1, 2},
		.stRect = {20, 20, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
	},
	{
		.Handle = 11,
		.enType = TYPE_STRING,
		.stChn = {CVI_ID_VPSS, 1, 2},
		.stRect = {20, 300, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
		.str = "cvitek",
	},
	{
		.Handle = 12,
		.enType = TYPE_STRING,
		.stChn = {CVI_ID_VPSS, 1, 2},
		.stRect = {1470, 20, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
		.str = "E:113.95 N:22.53",
	},
	{
		.Handle = 13,
		.enType = TYPE_STRING,
		.stChn = {CVI_ID_VPSS, 1, 2},
		.stRect = {1470, 300, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
		.str = "IPCamera",
	},
	{
		.Handle = 14,
		.enType = TYPE_PICTURE,
		.stChn = {CVI_ID_VPSS, 1, 2},
		.stRect = {900, 60, 300, 200},
		.filename = "tiger.bmp",
	},
#endif
	//grp2 chn0 720x576
	{
		.Handle = 15,
		.enType = TYPE_TIME,
		.stChn = {CVI_ID_VPSS, 2, 0},
		.stRect = {32, 32, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
	},
	{
		.Handle = 16,
		.enType = TYPE_STRING,
		.stChn = {CVI_ID_VPSS, 2, 0},
		.stRect = {20, 64, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
		.str = "cvitek",
	},
	{
		.Handle = 17,
		.enType = TYPE_STRING,
		.stChn = {CVI_ID_VPSS, 2, 0},
		.stRect = {32, 448, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
		.str = "E:113.95 N:22.53",
	},
	{
		.Handle = 18,
		.enType = TYPE_STRING,
		.stChn = {CVI_ID_VPSS, 2, 0},
		.stRect = {32, 512, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
		.str = "IPCamera",
	},
	{
		.Handle = 19,
		.enType = TYPE_PICTURE,
		.stChn = {CVI_ID_VPSS, 2, 0},
		.stRect = {256, 64, 300, 200},
		.filename = "tiger.bmp",
	},
	//grp2 chn1 720x576
	{
		.Handle = 20,
		.enType = TYPE_TIME,
		.stChn = {CVI_ID_VPSS, 2, 1},
		.stRect = {32, 32, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
	},
	{
		.Handle = 21,
		.enType = TYPE_STRING,
		.stChn = {CVI_ID_VPSS, 2, 1},
		.stRect = {32, 352, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
		.str = "cvitek",
	},
	{
		.Handle = 22,
		.enType = TYPE_STRING,
		.stChn = {CVI_ID_VPSS, 2, 1},
		.stRect = {32, 448, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
		.str = "E:113.95 N:22.53",
	},
	{
		.Handle = 23,
		.enType = TYPE_STRING,
		.stChn = {CVI_ID_VPSS, 2, 1},
		.stRect = {32, 512, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
		.str = "IPCamera",
	},
	{
		.Handle = 24,
		.enType = TYPE_PICTURE,
		.stChn = {CVI_ID_VPSS, 2, 1},
		.stRect = {256, 64, 300, 200},
		.filename = "tiger.bmp",
	},
#if (VPSS_MAX_PHY_CHN_NUM > 3)
	//grp2 chn2 720x576
	{
		.Handle = 25,
		.enType = TYPE_TIME,
		.stChn = {CVI_ID_VPSS, 2, 2},
		.stRect = {32, 32, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
	},
	{
		.Handle = 26,
		.enType = TYPE_STRING,
		.stChn = {CVI_ID_VPSS, 2, 2},
		.stRect = {32, 352, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
		.str = "cvitek",
	},
	{
		.Handle = 27,
		.enType = TYPE_STRING,
		.stChn = {CVI_ID_VPSS, 2, 2},
		.stRect = {32, 448, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
		.str = "E:113.95 N:22.53",
	},
	{
		.Handle = 28,
		.enType = TYPE_STRING,
		.stChn = {CVI_ID_VPSS, 2, 2},
		.stRect = {32, 512, OSD_LIB_FONT_W, OSD_LIB_FONT_H},
		.str = "IPCamera",
	},
	{
		.Handle = 29,
		.enType = TYPE_PICTURE,
		.stChn = {CVI_ID_VPSS, 2, 2},
		.stRect = {256, 64, 300, 200},
		.filename = "tiger.bmp",
	}
#endif
};


SIZE_S g_stSize;
CVI_S32 Init_Vpss(VPSS_GRP VpssGrp, ROTATION_E enRotation)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_GRP_ATTR_S    stVpssGrpAttr;
	CVI_BOOL		   abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssGrpAttr.enPixelFormat	= VI_PIXEL_FORMAT;

	if (VpssGrp == 0 || VpssGrp == 1) {
		if (enRotation == ROTATION_90 || enRotation == ROTATION_270) {
			stVpssGrpAttr.u32MaxW						 = g_stSize.u32Height;
			stVpssGrpAttr.u32MaxH						 = g_stSize.u32Width;
		} else {
			stVpssGrpAttr.u32MaxW						 = g_stSize.u32Width;
			stVpssGrpAttr.u32MaxH						 = g_stSize.u32Height;
		}
	} else {
		if (enRotation == ROTATION_90 || enRotation == ROTATION_270) {
			stVpssGrpAttr.u32MaxW						 = SCALE_HEIGHT;
			stVpssGrpAttr.u32MaxH						 = SCALE_WIDTH;
		} else {
			stVpssGrpAttr.u32MaxW						 = SCALE_WIDTH;
			stVpssGrpAttr.u32MaxH						 = SCALE_HEIGHT;
		}
	}

	if (VpssGrp == 0) {
		//scale
		abChnEnable[0] = CVI_TRUE;
		if (enRotation == ROTATION_90 || enRotation == ROTATION_270) {
			astVpssChnAttr[0].u32Width = SCALE_HEIGHT;
			astVpssChnAttr[0].u32Height = SCALE_WIDTH;
		} else {
			astVpssChnAttr[0].u32Width = SCALE_WIDTH;
			astVpssChnAttr[0].u32Height = SCALE_HEIGHT;
		}

		astVpssChnAttr[0].enVideoFormat	= VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[0].enPixelFormat = VI_PIXEL_FORMAT;
		astVpssChnAttr[0].stFrameRate.s32SrcFrameRate = -1;
		astVpssChnAttr[0].stFrameRate.s32DstFrameRate = -1;
		astVpssChnAttr[0].u32Depth = 0;
		astVpssChnAttr[0].bMirror = CVI_FALSE;
		astVpssChnAttr[0].bFlip = CVI_FALSE;
		astVpssChnAttr[0].stAspectRatio.enMode = ASPECT_RATIO_NONE;
		astVpssChnAttr[0].stNormalize.bEnable = CVI_FALSE;

	} else if (VpssGrp == 1) {
		//H265 4k
		abChnEnable[0] = CVI_TRUE;
		if (enRotation == ROTATION_90 || enRotation == ROTATION_270) {
			astVpssChnAttr[0].u32Width = ALIGN(INPUT_HEIGHT, 64);
			astVpssChnAttr[0].u32Height = INPUT_WIDTH;
			astVpssChnAttr[0].stAspectRatio.enMode = ASPECT_RATIO_MANUAL;
			astVpssChnAttr[0].stAspectRatio.stVideoRect.s32X = 0;
			astVpssChnAttr[0].stAspectRatio.stVideoRect.s32Y = 0;
			astVpssChnAttr[0].stAspectRatio.stVideoRect.u32Width = INPUT_HEIGHT;
			astVpssChnAttr[0].stAspectRatio.stVideoRect.u32Height = INPUT_WIDTH;
			astVpssChnAttr[0].stAspectRatio.bEnableBgColor = CVI_TRUE;
			astVpssChnAttr[0].stAspectRatio.u32BgColor = 0;
		} else {
			astVpssChnAttr[0].u32Width = INPUT_WIDTH;
			astVpssChnAttr[0].u32Height = INPUT_HEIGHT;
			astVpssChnAttr[0].stAspectRatio.enMode = ASPECT_RATIO_NONE;
		}

		astVpssChnAttr[0].enVideoFormat = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[0].enPixelFormat = SAMPLE_PIXEL_FORMAT;
		astVpssChnAttr[0].stFrameRate.s32SrcFrameRate = -1;
		astVpssChnAttr[0].stFrameRate.s32DstFrameRate = -1;
		astVpssChnAttr[0].u32Depth = 0;
		astVpssChnAttr[0].bMirror = CVI_FALSE;
		astVpssChnAttr[0].bFlip = CVI_FALSE;
		astVpssChnAttr[0].stNormalize.bEnable = CVI_FALSE;

		//H264 1080P
		abChnEnable[1] = CVI_TRUE;
		memcpy(&astVpssChnAttr[1], &astVpssChnAttr[0], sizeof(VPSS_CHN_ATTR_S));
		if (enRotation == ROTATION_90 || enRotation == ROTATION_270) {
			astVpssChnAttr[1].u32Width = ALIGN(RES2_HEIGHT, 64);
			astVpssChnAttr[1].u32Height = RES2_WIDTH;
			astVpssChnAttr[1].stAspectRatio.enMode = ASPECT_RATIO_MANUAL;
			astVpssChnAttr[1].stAspectRatio.stVideoRect.s32X = 0;
			astVpssChnAttr[1].stAspectRatio.stVideoRect.s32Y = 0;
			astVpssChnAttr[1].stAspectRatio.stVideoRect.u32Width = RES2_HEIGHT;
			astVpssChnAttr[1].stAspectRatio.stVideoRect.u32Height = RES2_WIDTH;
			astVpssChnAttr[1].stAspectRatio.bEnableBgColor = CVI_TRUE;
			astVpssChnAttr[1].stAspectRatio.u32BgColor = 0;
		} else {
			astVpssChnAttr[1].u32Width = RES2_WIDTH;
			astVpssChnAttr[1].u32Height = RES2_HEIGHT;
			astVpssChnAttr[1].stAspectRatio.enMode = ASPECT_RATIO_NONE;
		}
#if (VPSS_MAX_PHY_CHN_NUM > 3)
		//MJPEG 1080P
		abChnEnable[2] = CVI_TRUE;
		memcpy(&astVpssChnAttr[2], &astVpssChnAttr[1], sizeof(VPSS_CHN_ATTR_S));
		astVpssChnAttr[2].stFrameRate.s32SrcFrameRate = -1;
		astVpssChnAttr[2].stFrameRate.s32DstFrameRate = -1;
#endif
	} else if (VpssGrp == 2) {
		//H264 720*576
		abChnEnable[0] = CVI_TRUE;

		if (enRotation == ROTATION_90 || enRotation == ROTATION_270) {
			astVpssChnAttr[0].u32Width = ALIGN(576, 64);
			astVpssChnAttr[0].u32Height = 720;
			astVpssChnAttr[0].stAspectRatio.stVideoRect.u32Width = 576;
			astVpssChnAttr[0].stAspectRatio.stVideoRect.u32Height = 720;

		} else {
			astVpssChnAttr[0].u32Width = ALIGN(720, 64);
			astVpssChnAttr[0].u32Height = 576;
			astVpssChnAttr[0].stAspectRatio.stVideoRect.u32Width = 720;
			astVpssChnAttr[0].stAspectRatio.stVideoRect.u32Height = 576;
		}

		astVpssChnAttr[0].enVideoFormat = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[0].enPixelFormat = SAMPLE_PIXEL_FORMAT;
		astVpssChnAttr[0].stFrameRate.s32SrcFrameRate = -1;
		astVpssChnAttr[0].stFrameRate.s32DstFrameRate = -1;
		astVpssChnAttr[0].u32Depth = 0;
		astVpssChnAttr[0].bMirror = CVI_FALSE;
		astVpssChnAttr[0].bFlip = CVI_FALSE;
		astVpssChnAttr[0].stAspectRatio.enMode = ASPECT_RATIO_MANUAL;
		astVpssChnAttr[0].stAspectRatio.stVideoRect.s32X = 0;
		astVpssChnAttr[0].stAspectRatio.stVideoRect.s32Y = 0;
		astVpssChnAttr[0].stAspectRatio.bEnableBgColor = CVI_TRUE;
		astVpssChnAttr[0].stAspectRatio.u32BgColor = 0;
		astVpssChnAttr[0].stNormalize.bEnable = CVI_FALSE;

		//H264 720*576
		abChnEnable[1] = CVI_TRUE;
		memcpy(&astVpssChnAttr[1], &astVpssChnAttr[0], sizeof(VPSS_CHN_ATTR_S));
#if (VPSS_MAX_PHY_CHN_NUM > 3)
		//H264 720*576
		abChnEnable[2] = CVI_TRUE;
		memcpy(&astVpssChnAttr[2], &astVpssChnAttr[0], sizeof(VPSS_CHN_ATTR_S));
#endif
	} else if (VpssGrp == 3) {
		//AI 960x540
		abChnEnable[0] = CVI_TRUE;
		if (enRotation == ROTATION_90 || enRotation == ROTATION_270) {
			astVpssChnAttr[0].u32Width  = 540;
			astVpssChnAttr[0].u32Height = 960;
		} else {
			astVpssChnAttr[0].u32Width  = 960;
			astVpssChnAttr[0].u32Height = 540;
		}

		astVpssChnAttr[0].enVideoFormat				  = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[0].enPixelFormat				  = PIXEL_FORMAT_YUV_PLANAR_420;
		astVpssChnAttr[0].stFrameRate.s32SrcFrameRate = -1;
		astVpssChnAttr[0].stFrameRate.s32DstFrameRate = -1;
		astVpssChnAttr[0].u32Depth					  = 1;
		astVpssChnAttr[0].bMirror					  = CVI_FALSE;
		astVpssChnAttr[0].bFlip						  = CVI_FALSE;
		astVpssChnAttr[0].stAspectRatio.enMode		  = ASPECT_RATIO_NONE;
		astVpssChnAttr[0].stNormalize.bEnable = CVI_FALSE;

		//AI 960x540
		abChnEnable[1] = CVI_TRUE;
		memcpy(&astVpssChnAttr[1], &astVpssChnAttr[0], sizeof(VPSS_CHN_ATTR_S));
	} else if (VpssGrp == 4) {
		//Jpeg venc
		abChnEnable[0] = CVI_TRUE;
		astVpssChnAttr[0].u32Width					  = 320;
		astVpssChnAttr[0].u32Height					  = 320;
		astVpssChnAttr[0].enVideoFormat				  = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[0].enPixelFormat				  = SAMPLE_PIXEL_FORMAT;
		astVpssChnAttr[0].stFrameRate.s32SrcFrameRate = -1;
		astVpssChnAttr[0].stFrameRate.s32DstFrameRate = -1;
		astVpssChnAttr[0].u32Depth					  = 0;
		astVpssChnAttr[0].bMirror					  = CVI_FALSE;
		astVpssChnAttr[0].bFlip						  = CVI_FALSE;
		astVpssChnAttr[0].stAspectRatio.enMode		  = ASPECT_RATIO_NONE;
		astVpssChnAttr[0].stNormalize.bEnable = CVI_FALSE;

	}

	/*start vpss*/
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_VOID Deinit_Vpss(VPSS_GRP VpssGrp)
{
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};

	switch (VpssGrp) {
	case 1:
	case 2:
#if (VPSS_MAX_PHY_CHN_NUM > 3)
		abChnEnable[2] = CVI_TRUE;
#endif
	case 3:
		abChnEnable[1] = CVI_TRUE;
	case 0:
	case 4:
		abChnEnable[0] = CVI_TRUE;
		break;
	default:
		break;
	}

	SAMPLE_COMM_VPSS_Stop(VpssGrp, abChnEnable);
}

static CVI_S32 CheckInputCfg(chnInputCfg *pIc)
{
	if (!strcmp(pIc->codec, "264") || !strcmp(pIc->codec, "265")) {

		CVI_VENC_CFG("framerate = %d\n", pIc->framerate);

		if (pIc->gop < 1) {
			if (!strcmp(pIc->codec, "264"))
				pIc->gop = DEF_264_GOP;
			else
				pIc->gop = DEF_GOP;
		}
		CVI_VENC_CFG("gop = %d\n", pIc->gop);

		if (!strcmp(pIc->codec, "265")) {
			if (pIc->single_LumaBuf > 0) {
				SAMPLE_PRT("single_LumaBuf only supports H.264\n");
				pIc->single_LumaBuf = 0;
			}
		} else if (low_mem_profile == 1) {
			pIc->single_LumaBuf = 1;
		}
		pIc->iqp = (pIc->iqp >= 0) ? pIc->iqp : DEF_IQP;
		pIc->pqp = (pIc->pqp >= 0) ? pIc->pqp : DEF_PQP;
#ifdef CVI_H26X_MAX_I_PROP_DEFAULT
		pIc->maxIprop = CVI_H26X_MAX_I_PROP_DEFAULT;
		pIc->minIprop = CVI_H26X_MIN_I_PROP_DEFAULT;
#endif

		if (pIc->rcMode == -1) {
			pIc->rcMode = SAMPLE_RC_FIXQP;
		}

		if (pIc->rcMode == SAMPLE_RC_CBR || pIc->rcMode == SAMPLE_RC_VBR) {

			if (pIc->rcMode == SAMPLE_RC_CBR) {
				if (pIc->bitrate <= 0) {
					SAMPLE_PRT("CBR bitrate must be not less than 0");
					return -1;
				}
				CVI_VENC_CFG("RC_CBR, bitrate = %d\n", pIc->bitrate);
			} else if (pIc->rcMode == SAMPLE_RC_VBR) {
				if (pIc->maxbitrate <= 0) {
					SAMPLE_PRT("VBR must be not less than 0");
					return -1;
				}
				CVI_VENC_CFG("RC_VBR, maxbitrate = %d\n", pIc->maxbitrate);
			}

			pIc->firstFrmstartQp =
				(pIc->firstFrmstartQp < 0 ||
				 pIc->firstFrmstartQp > 51) ? 63 : pIc->firstFrmstartQp;
			CVI_VENC_CFG("firstFrmstartQp = %d\n", pIc->firstFrmstartQp);

			pIc->maxIqp = (pIc->maxIqp >= 0) ? pIc->maxIqp : DEF_264_MAXIQP;
			pIc->minIqp = (pIc->minIqp >= 0) ? pIc->minIqp : DEF_264_MINIQP;
			pIc->maxQp = (pIc->maxQp >= 0) ? pIc->maxQp : DEF_264_MAXQP;
			pIc->minQp = (pIc->minQp >= 0) ? pIc->minQp : DEF_264_MINQP;
			CVI_VENC_CFG("maxQp = %d, minQp = %d, maxIqp = %d, minIqp = %d\n",
					pIc->maxQp,
					pIc->minQp,
					pIc->maxIqp,
					pIc->minIqp);

			if (pIc->statTime == 0) {
				pIc->statTime = DEF_STAT_TIME;
			}
			CVI_VENC_CFG("statTime = %d\n", pIc->statTime);
		} else if (pIc->rcMode == SAMPLE_RC_FIXQP) {
			if (pIc->firstFrmstartQp != -1) {
				CVI_VENC_WARN("firstFrmstartQp is invalid in FixQP mode\n");
				pIc->firstFrmstartQp = -1;
			}

			pIc->bitrate = 0;
			CVI_VENC_CFG("RC_FIXQP, iqp = %d, pqp = %d\n",
				pIc->iqp,
				pIc->pqp);
		} else {
			SAMPLE_PRT("codec = %s, rcMode = %d, not supported RC mode\n", pIc->codec, pIc->rcMode);
			return -1;
		}
	} else if (!strcmp(pIc->codec, "mjp") || !strcmp(pIc->codec, "jpg")) {
		if (pIc->rcMode == -1) {
			pIc->rcMode = SAMPLE_RC_FIXQP;
			pIc->quality = (pIc->quality != -1) ? pIc->quality : 0;
		} else if (pIc->rcMode == SAMPLE_RC_FIXQP) {
			if (pIc->quality < 0)
				pIc->quality = 0;
			else if (pIc->quality >= 100)
				pIc->quality = 99;
			else if (pIc->quality == 0)
				pIc->quality = 1;
		}
	} else {
		SAMPLE_PRT("codec = %s\n", pIc->codec);
		return -1;
	}

	return 0;
}

static PIC_SIZE_E GetVencSize(CVI_U32 u32Width, CVI_U32 u32Height)
{
	if (u32Width == 352 && u32Height == 288)
		return PIC_CIF;
	else if (u32Width == 720 && u32Height == 576)
		return PIC_D1_PAL;
	else if (u32Width == 720 && u32Height == 480)
		return PIC_D1_NTSC;
	else if (u32Width == 1280 && u32Height == 720)
		return PIC_720P;
	else if (u32Width == 1920 && u32Height == 1080)
		return PIC_1080P;
	else if (u32Width == 2592 && u32Height == 1520)
		return PIC_2592x1520;
	else if (u32Width == 2592 && u32Height == 1536)
		return PIC_2592x1536;
	else if (u32Width == 2592 && u32Height == 1944)
		return PIC_2592x1944;
	else if (u32Width == 2716 && u32Height == 1524)
		return PIC_2716x1524;
	else if (u32Width == 3840 && u32Height == 2160)
		return PIC_3840x2160;
	else if (u32Width == 4096 && u32Height == 2160)
		return PIC_4096x2160;
	else if (u32Width == 3000 && u32Height == 3000)
		return PIC_3000x3000;
	else if (u32Width == 4000 && u32Height == 3000)
		return PIC_4000x3000;
	else if (u32Width == 3840 && u32Height == 8640)
		return PIC_3840x8640;
	else if (u32Width == 640 && u32Height == 480)
		return PIC_640x480;
	else
		return PIC_CUSTOMIZE;
}


static CVI_U32 Venc_Init_Chn(VENC_CHN VencChn, vencChnCtx *pstChnCtx)
{
	chnInputCfg *pIc = &pstChnCtx->chnIc;
	CVI_S32 s32Ret = CVI_SUCCESS;

	pstChnCtx->enPixelFormat = pIc->pixel_format == 0 ?
		PIXEL_FORMAT_YUV_PLANAR_420 : (pIc->pixel_format == 3) ?
		PIXEL_FORMAT_NV21 : PIXEL_FORMAT_YUV_PLANAR_422;
	pstChnCtx->VencChn = VencChn;
	pstChnCtx->enGopMode = pIc->gopMode;
	pstChnCtx->u32Profile = 0;
	pstChnCtx->s32FbCnt = 1;

	if (!strcmp(pIc->codec, "265"))
		pstChnCtx->enPayLoad = PT_H265;
	else if (!strcmp(pIc->codec, "264"))
		pstChnCtx->enPayLoad = PT_H264;
	else if (!strcmp(pIc->codec, "mjp"))
		pstChnCtx->enPayLoad = PT_MJPEG;
	else if (!strcmp(pIc->codec, "jpg"))
		pstChnCtx->enPayLoad = PT_JPEG;

	pstChnCtx->stSize.u32Width = pIc->width;
	pstChnCtx->stSize.u32Height = pIc->height;

	if (pstChnCtx->enPixelFormat != PIXEL_FORMAT_YUV_PLANAR_422)
		pstChnCtx->u32FrameSize =
			(pstChnCtx->stSize.u32Width * pstChnCtx->stSize.u32Height * 3) >> 1;
	else
		pstChnCtx->u32FrameSize = pstChnCtx->stSize.u32Width * pstChnCtx->stSize.u32Height * 2;

	pstChnCtx->enRcMode = pIc->rcMode;
	SAMPLE_PRT("enPayLoad = %d, enRcMode = %d, bsMode = %d\n",
			   pstChnCtx->enPayLoad, pstChnCtx->enRcMode, pIc->bsMode);

	pstChnCtx->enSize = GetVencSize(pIc->width, pIc->height);
	if (pstChnCtx->enSize == PIC_BUTT) {
		SAMPLE_PRT("unsupport size %d x %d\n", pIc->width, pIc->height);
		return CVI_FAILURE;
	}

	if (pstChnCtx->enPayLoad != PT_JPEG) {
		s32Ret = SAMPLE_COMM_VENC_GetGopAttr(pstChnCtx->enGopMode, &pstChnCtx->stGopAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("Venc Get GopAttr for %#x!\n", s32Ret);
			return CVI_FAILURE;
		}
	}

	s32Ret = SAMPLE_COMM_VENC_Start(
			pIc,
			pstChnCtx->VencChn,
			pstChnCtx->enPayLoad,
			pstChnCtx->enSize,
			pstChnCtx->enRcMode,
			pstChnCtx->u32Profile,
			CVI_FALSE,
			&pstChnCtx->stGopAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Venc Start failed for %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	pstChnCtx->num_frames = pIc->num_frames;

	if (pIc->bsMode == BS_MODE_QUERY_STAT) {
		char file_ext[16];

		SAMPLE_COMM_VENC_GetFilePostfix(pstChnCtx->enPayLoad, file_ext);
		snprintf(pIc->outputFileName, MAX_STRING_LEN, "%s%s",
				pIc->output_path, file_ext);
	}

	SAMPLE_PRT("open output file %s\n", pIc->outputFileName);

	pstChnCtx->pFile = fopen(pIc->outputFileName, "wb");
	if (pstChnCtx->pFile == NULL) {
		SAMPLE_PRT("open file err, %s\n", pIc->outputFileName);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}
#ifdef __ENABLE_RTSP__
void SendToRtsp(CVI_RTSP_CTX *ctx, CVI_RTSP_SESSION *session, VENC_STREAM_S *pstStream)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VENC_PACK_S *ppack;
	CVI_RTSP_DATA data;

	memset(&data, 0, sizeof(CVI_RTSP_DATA));

	data.blockCnt = pstStream->u32PackCount;
	for (CVI_U32 i = 0; i < pstStream->u32PackCount; i++) {
		ppack = &pstStream->pstPack[i];
		data.dataPtr[i] = ppack->pu8Addr + ppack->u32Offset;
		data.dataLen[i] = ppack->u32Len - ppack->u32Offset;
	}

	s32Ret = CVI_RTSP_WriteFrame(ctx, session->video, &data);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_RTSP_WriteFrame failed\n");
	}
}
#endif
#ifdef _SINGLETHREAD_FORALLSTREAM_

void fillFDArray(CVI_S32 fd_array[], CVI_S32 numChn, int *max_fd)
{
	*max_fd = -1;
	for (CVI_S32 ch = 0; ch < numChn; ch++) {
		if (fd_array[ch] < -1) {
			continue;
		}
		fd_array[ch] = CVI_VENC_GetFd(ch);
		if (*max_fd < fd_array[ch]) {
			*max_fd = fd_array[ch];
		}
		// SAMPLE_PRT("CVI_VENC_GetFd(%d) = %d, max = %d\n", ch, fd_array[ch], *max_fd);
	}
}

static CVI_VOID *GetVencAllStreamProc(CVI_VOID *pArgs)
{
	SAMPLE_VENC_CTX *pstVencCtx = (SAMPLE_VENC_CTX *) pArgs;
	CVI_S32 numChn = pstVencCtx->s32VencChnCnt;
	CVI_CHAR TaskName[64];
	CVI_S32 s32Ret, i = 0;
	CVI_BOOL bIsRunning;
#ifdef _USE_GETFD_
	CVI_S32 fd_array[VENC_MAX_CHN_NUM];
	int max_fd = -1;

	memset(fd_array, -1, sizeof(fd_array));
	fillFDArray(fd_array, 6, &max_fd);
#endif
	sprintf(TaskName, "VencGetAllStream");
	prctl(PR_SET_NAME, TaskName, 0, 0, 0);

	for (CVI_S32 ch = 0; ch < numChn; ch++) {
		vencChnCtx *pstChnCtx = &pstVencCtx->astChnCtx[ch];

		pstChnCtx->chnStat = CHN_STAT_START;
		pstChnCtx->nextChnStat = CHN_STAT_START;
	}
	usleep(1000);

do {
#ifndef _USE_GETFD_
	for (CVI_S32 ch = 0; ch < numChn; ch++) {
		CVI_S32 s32timeout = 10;
#else
	fd_set readfds;
	struct timeval tv;
	int r, k;
	CVI_S32 s32timeout = 0;
	CVI_S32 ext_chn = -1;

	FD_ZERO(&readfds);
	for (k = 0; k < numChn; k++) {
		if (fd_array[k] >= 0)
			FD_SET(fd_array[k], &readfds);
	}
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	if (max_fd < 0)
		break;

	r = select(max_fd + 1, &readfds, NULL, NULL, &tv);
	if (r < 0) {
		SAMPLE_PRT("select ret %d errno %d\n", r, errno);
		if (errno == EINTR)
			continue;
		usleep(10000);
		fillFDArray(fd_array, 6, &max_fd);
		goto check_exit;
		// continue;
	} else if (r == 0) {
		SAMPLE_PRT("select timeout\n");
		goto check_exit;
	} else {
		for (k = 0; k < numChn; k++) {
			if (fd_array[k] >= 0 && FD_ISSET(fd_array[k], &readfds)) {
				ext_chn = k;
				break;
			}
		}
	}

	do {
		CVI_S32 ch = ext_chn;
#endif
		vencChnCtx *pstChnCtx = &pstVencCtx->astChnCtx[ch];
		VENC_CHN VencChn = pstChnCtx->VencChn;
		chnInputCfg *pIc = &pstChnCtx->chnIc;
		SAMPLE_RTSP_CTX *pstRtspCtx = NULL;

		if (strcmp(pstVencCtx->astChnCtx[ch].chnIc.codec, "jpg") == 0) {
			continue;
		}

		// SAMPLE_PRT("venc chn%d turn ", pstChnCtx->VencChn);

		for (CVI_S32 j = 0; j < MAX_RTPS_SERVER; j++) {
			if (s_astRtspCtx[j].bStart && (s_astRtspCtx[j].VencChn == VencChn)) {
				pstRtspCtx = &s_astRtspCtx[j];
				break;
			}
		}

		if (pstChnCtx->chnStat == CHN_STAT_START) {
			VENC_CHN_STATUS_S stStat;

#if 0
			if (pIc->forceIdr > 0 && pIc->forceIdr == i) {
				CVI_BOOL bInstant = CVI_TRUE;

				CVI_VENC_RequestIDR(VencChn, bInstant);
				SAMPLE_PRT("CVI_VENC_RequestIDR\n");
			}

			if (pIc->testRoi > 0 && pIc->testRoi == i) {
				VENC_CHN_ATTR_S stVencChnAttr;

				s32Ret = CVI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
				if (s32Ret != CVI_SUCCESS) {
					SAMPLE_PRT("CVI_VENC_GetChnAttr, VencChn = %d, s32Ret = %d\n",
							VencChn, s32Ret);
					break;
				}

				s32Ret = SAMPLE_COMM_VENC_SetRoiAttr(VencChn, stVencChnAttr.stVencAttr.enType);
				if (s32Ret != CVI_SUCCESS) {
					SAMPLE_PRT("SAMPLE_COMM_VENC_SetRoiAttr, %d\n", s32Ret);
					break;
				}

			}
#endif
			if (pIc->bsMode == BS_MODE_QUERY_STAT) {
				VENC_CHN_ATTR_S stVencChnAttr;
				VENC_STREAM_S stStream;

				s32Ret = CVI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
				if (s32Ret != CVI_SUCCESS) {
					SAMPLE_PRT("CVI_VENC_GetChnAttr, VencChn = %d, s32Ret = %d\n",
							VencChn, s32Ret);
					break;
				}

				s32Ret = CVI_VENC_QueryStatus(VencChn, &stStat);
				if (s32Ret != CVI_SUCCESS) {
					SAMPLE_PRT("CVI_VENC_QueryStatus, Vench = %d, s32Ret = %d\n",
							VencChn, s32Ret);
					break;
				}

				if (!stStat.u32CurPacks) {
					usleep(10000);
					continue;
				}

				stStream.pstPack =
					(VENC_PACK_S *)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
				if (stStream.pstPack == NULL) {
					SAMPLE_PRT("malloc memory failed!\n");
					break;
				}

				CVI_SYS_TraceBegin("VENC_GetStream");
				s32Ret = CVI_VENC_GetStream(VencChn, &stStream, s32timeout);
				if (s32Ret == CVI_ERR_VENC_BUSY) {
					CVI_SYS_TraceEnd();
					free(stStream.pstPack);
					stStream.pstPack = NULL;
#ifdef _USE_GETFD_
					printf("venc chn%d - timeout, wait next round\n", VencChn);
#endif
					continue;
				} else if (s32Ret != CVI_SUCCESS) {
					SAMPLE_PRT("CVI_VENC_GetStream, VencChn = %d, s32Ret = 0x%X\n",
							VencChn, s32Ret);
					free(stStream.pstPack);
					stStream.pstPack = NULL;
					CVI_SYS_TraceEnd();
					pstChnCtx->nextChnStat = CHN_STAT_STOP;
					break;
				}
				CVI_SYS_TraceEnd();

				if (pstRtspCtx) {
					SendToRtsp(pstRtspCtx->pstServerCtx, pstRtspCtx->pVideoSession, &stStream);
				} else {
					s32Ret = SAMPLE_COMM_VENC_SaveStream(
							stVencChnAttr.stVencAttr.enType,
							pstChnCtx->pFile,
							&stStream);
					if (s32Ret != CVI_SUCCESS) {
						SAMPLE_PRT("SAMPLE_COMM_VENC_SaveStream, s32Ret = %d\n", s32Ret);
						free(stStream.pstPack);
						stStream.pstPack = NULL;
						break;
					}
				}
				s32Ret = CVI_VENC_ReleaseStream(VencChn, &stStream);
				if (s32Ret != CVI_SUCCESS) {
					SAMPLE_PRT("CVI_VENC_ReleaseStream, s32Ret = %d\n", s32Ret);
					free(stStream.pstPack);
					stStream.pstPack = NULL;
					break;
				}
				free(stStream.pstPack);
				stStream.pstPack = NULL;
			}
			i++;
			if (pIc->bind_mode) {
				if (pstChnCtx->chnStat != pstChnCtx->nextChnStat) {
					//printf("========[%s][%d]======\n", __func__, __LINE__);
					s32Ret = CVI_VENC_QueryStatus(VencChn, &stStat);
					if (s32Ret != CVI_SUCCESS) {
						SAMPLE_PRT("CVI_VENC_QueryStatus, Vench = %d, s32Ret = %d\n",
								VencChn, s32Ret);
						break;
					}

					SAMPLE_PRT("[%d] u32LeftStreamFrames = %d\n",
							VencChn, stStat.u32LeftStreamFrames);
					if (stStat.u32LeftStreamFrames <= 0) {
						pstChnCtx->chnStat = CHN_STAT_STOP;
						SAMPLE_PRT("chnStat = CHN_STAT_STOP\n");
					}
				}
			}
		}
		// printf(" - done\n");
#ifndef _USE_GETFD_
	}
#else
	} while (0);
check_exit:
#endif
	bIsRunning = CVI_FALSE;
	for (CVI_S32 ch = 0; ch < numChn; ch++) {
		vencChnCtx *pstChnCtx = &pstVencCtx->astChnCtx[ch];

		if (pstChnCtx->chnStat == CHN_STAT_START) {
			bIsRunning = CVI_TRUE;
		}
	}
} while (bIsRunning);
	SAMPLE_PRT("Get-out of GetVencAllStreamProc\n");
	return (CVI_VOID *) CVI_SUCCESS;
}
#endif

static CVI_VOID *GetVencStreamProc(CVI_VOID *pArgs)
{
	vencChnCtx *pstChnCtx = (vencChnCtx *)pArgs;
	VENC_CHN VencChn = pstChnCtx->VencChn;
	chnInputCfg *pIc = &pstChnCtx->chnIc;
	CVI_CHAR TaskName[64];
	CVI_S32 s32Ret, i = 0;
	SAMPLE_RTSP_CTX *pstRtspCtx = NULL;
	CVI_S32 s32VencFd = -1;

	sprintf(TaskName, "chn%dVencGetStream", VencChn);
	prctl(PR_SET_NAME, TaskName, 0, 0, 0);
	SAMPLE_PRT("venc task%d start\n", VencChn);

	for (CVI_S32 j = 0; j < MAX_RTPS_SERVER; j++) {
		if (s_astRtspCtx[j].bStart && (s_astRtspCtx[j].VencChn == VencChn)) {
			pstRtspCtx = &s_astRtspCtx[j];
			break;
		}
	}

	usleep(1000);
	pstChnCtx->chnStat = CHN_STAT_START;
	pstChnCtx->nextChnStat = CHN_STAT_START;

	while (pstChnCtx->chnStat == CHN_STAT_START) {
		VENC_CHN_STATUS_S stStat;

#ifdef _USE_GETFD_
		if (g_UseSelect && s32VencFd < 0) {
			s32VencFd = CVI_VENC_GetFd(VencChn);
			if (s32VencFd < 0) {
				SAMPLE_PRT("CVI_VENC_GetFd failed with%#x!\n", s32VencFd);
				break;
			}
		}
#endif
		if (pIc->forceIdr > 0 && pIc->forceIdr == i) {
			CVI_BOOL bInstant = CVI_TRUE;

			CVI_VENC_RequestIDR(VencChn, bInstant);
			SAMPLE_PRT("CVI_VENC_RequestIDR\n");
		}

		if (pIc->testRoi > 0 && pIc->testRoi == i) {
			VENC_CHN_ATTR_S stVencChnAttr;

			s32Ret = CVI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("CVI_VENC_GetChnAttr, VencChn = %d, s32Ret = %d\n",
						VencChn, s32Ret);
				break;
			}

			s32Ret = SAMPLE_COMM_VENC_SetRoiAttr(VencChn, stVencChnAttr.stVencAttr.enType);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("SAMPLE_COMM_VENC_SetRoiAttr, %d\n", s32Ret);
				break;
			}

		}

		if (g_UseSelect && s32VencFd >= 0) {
			struct timeval TimeoutVal;
			fd_set read_fds;

			FD_ZERO(&read_fds);
			FD_SET(s32VencFd, &read_fds);
			TimeoutVal.tv_sec  = 5;
			TimeoutVal.tv_usec = 0;
			s32Ret = select(s32VencFd + 1, &read_fds, NULL, NULL, &TimeoutVal);
			if (s32Ret < 0) {
				if (errno == EINTR)
					continue;
				SAMPLE_PRT("select failed!\n");
				break;
			} else if (s32Ret == 0) {
				SAMPLE_PRT("select time out??\n");
				break;
			}
		}

		if (pIc->bsMode == BS_MODE_QUERY_STAT) {
			VENC_CHN_ATTR_S stVencChnAttr;
			VENC_STREAM_S stStream;

			s32Ret = CVI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("CVI_VENC_GetChnAttr, VencChn = %d, s32Ret = %d\n",
						VencChn, s32Ret);
				break;
			}

			s32Ret = CVI_VENC_QueryStatus(VencChn, &stStat);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("CVI_VENC_QueryStatus, Vench = %d, s32Ret = %d\n",
						VencChn, s32Ret);
				break;
			}

			if (!stStat.u32CurPacks) {
				usleep(10000);
				continue;
			}

			stStream.pstPack =
				(VENC_PACK_S *)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
			if (stStream.pstPack == NULL) {
				SAMPLE_PRT("malloc memory failed!\n");
				break;
			}

			CVI_SYS_TraceBegin("VENC_GetStream");
			s32Ret = CVI_VENC_GetStream(VencChn, &stStream, -1);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("CVI_VENC_GetStream, VencChn = %d, s32Ret = 0x%X\n",
						VencChn, s32Ret);
				free(stStream.pstPack);
				stStream.pstPack = NULL;
				break;
			}
			CVI_SYS_TraceEnd();

			if (pstRtspCtx) {
#ifdef __ENABLE_RTSP__
				SendToRtsp(pstRtspCtx->pstServerCtx, pstRtspCtx->pVideoSession, &stStream);
#endif
			} else {
				s32Ret = SAMPLE_COMM_VENC_SaveStream(
						stVencChnAttr.stVencAttr.enType,
						pstChnCtx->pFile,
						&stStream);
				if (s32Ret != CVI_SUCCESS) {
					SAMPLE_PRT("SAMPLE_COMM_VENC_SaveStream, s32Ret = %d\n", s32Ret);
					free(stStream.pstPack);
					stStream.pstPack = NULL;
					break;
				}
			}
			s32Ret = CVI_VENC_ReleaseStream(VencChn, &stStream);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("CVI_VENC_ReleaseStream, s32Ret = %d\n", s32Ret);
				free(stStream.pstPack);
				stStream.pstPack = NULL;
				break;
			}
			free(stStream.pstPack);
			stStream.pstPack = NULL;
		}

		i++;
		if (pIc->bind_mode) {
			if (pstChnCtx->chnStat != pstChnCtx->nextChnStat) {
				//printf("========[%s][%d]======\n", __func__, __LINE__);
				s32Ret = CVI_VENC_QueryStatus(VencChn, &stStat);
				if (s32Ret != CVI_SUCCESS) {
					SAMPLE_PRT("CVI_VENC_QueryStatus, Vench = %d, s32Ret = %d\n",
							VencChn, s32Ret);
					break;
				}

				SAMPLE_PRT("u32LeftStreamFrames = %d\n",
						stStat.u32LeftStreamFrames);
				if (stStat.u32LeftStreamFrames <= 0) {
					pstChnCtx->chnStat = CHN_STAT_STOP;
					SAMPLE_PRT("chnStat = CHN_STAT_STOP\n");
				}
			}
		}
	}
#ifdef _USE_GETFD_
	if (s32VencFd >= 0)
		CVI_VENC_CloseFd(pstChnCtx->VencChn);
#endif
	SAMPLE_PRT("venc task%d end\n", pstChnCtx->VencChn);

	return (CVI_VOID *) CVI_SUCCESS;
}


static CVI_VOID *ThreadFrameToJpeg(CVI_VOID *pArgs)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VIDEO_FRAME_INFO_S stVencFrame;
	vencChnCtx *pstChnCtx = (vencChnCtx *)pArgs;
	VENC_CHN VencChn = pstChnCtx->VencChn;
	VPSS_GRP VpssGrp = 4;
	//CVI_U32 pic_cnt = 0;
	CVI_CHAR TaskName[64];

	sprintf(TaskName, "ThreadFrameToJpeg");
	prctl(PR_SET_NAME, TaskName, 0, 0, 0);

	pstChnCtx->chnStat = CHN_STAT_START;

	while (pstChnCtx->chnStat == CHN_STAT_START) {
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, 0, &stVencFrame, 3000);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_GetChnFrame failed with %#x\n", s32Ret);
			break;
		}

		CVI_S32 s32SetFrameMilliSec = 20000;
		VENC_STREAM_S stStream;
		VENC_CHN_STATUS_S stStat;
		VENC_CHN_ATTR_S stVencChnAttr;

		s32Ret = CVI_VENC_SendFrame(VencChn, &stVencFrame, s32SetFrameMilliSec);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_VENC_SendFrame failed! %d\n", s32Ret);
			break;
		}
		s32Ret = CVI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("CVI_VENC_GetChnAttr, VencChn = %d, s32Ret = %d\n",
					VencChn, s32Ret);
			break;
		}
		s32Ret = CVI_VENC_QueryStatus(VencChn, &stStat);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_VENC_QueryStatus failed with %#x!\n", s32Ret);
			break;
		}
		if (!stStat.u32CurPacks) {
			printf("NOTE: Current frame is NULL!\n");
			break;
		}
		stStream.pstPack =
			(VENC_PACK_S *)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
		if (stStream.pstPack == NULL) {
			CVI_VENC_ERR("malloc memory failed!\n");
			break;
		}
		s32Ret = CVI_VENC_GetStream(VencChn, &stStream, -1);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("CVI_VENC_GetStream, VencChn = %d, s32Ret = 0x%X\n",
					VencChn, s32Ret);
			free(stStream.pstPack);
			stStream.pstPack = NULL;
			break;
		}
		#if 0
		CVI_CHAR picname[64];
		FILE *fp;

		sprintf(picname, "venc_pic_%d", pic_cnt);
		pic_cnt++;
		fp = fopen(picname, "wb");
		s32Ret = SAMPLE_COMM_VENC_SaveStream(
				stVencChnAttr.stVencAttr.enType,
				//pstChnCtx->pFile,
				fp,
				&stStream);
		#elif 1
		#else
		s32Ret = SAMPLE_COMM_VENC_SaveStream(
				stVencChnAttr.stVencAttr.enType,
				pstChnCtx->pFile,
				&stStream);
		#endif
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("SAMPLE_COMM_VENC_SaveStream, s32Ret = %d\n", s32Ret);
			free(stStream.pstPack);
			stStream.pstPack = NULL;
			break;
		}

		s32Ret = CVI_VENC_ReleaseStream(VencChn, &stStream);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("CVI_VENC_ReleaseStream, s32Ret = %d\n", s32Ret);
			free(stStream.pstPack);
			stStream.pstPack = NULL;
			break;
		}
		free(stStream.pstPack);
		stStream.pstPack = NULL;

		if (CVI_VPSS_ReleaseChnFrame(VpssGrp, 0, &stVencFrame) != CVI_SUCCESS)
			SAMPLE_PRT("CVI_VPSS_ReleaseChnFrame NG\n");

		usleep(30*1000);
	}
	return NULL;
}

static CVI_S32 InitVencCtx(SAMPLE_VENC_CTX *pstVencCtx, ROTATION_E enRotation)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 numChn = ARRAY_SIZE(IpcVencChnCfg);

	memset(pstVencCtx, 0, sizeof(SAMPLE_VENC_CTX));
	pstVencCtx->s32VencChnCnt = numChn;
	cviGetMask();

	for (CVI_S32 i = 0; i < numChn; i++) {
		memcpy(&pstVencCtx->astChnCtx[i].chnIc, &IpcVencChnCfg[i], sizeof(chnInputCfg));
		if (enRotation == ROTATION_90 || enRotation == ROTATION_270) {
			CVI_U32 tmp = pstVencCtx->astChnCtx[i].chnIc.width;

			pstVencCtx->astChnCtx[i].chnIc.width = pstVencCtx->astChnCtx[i].chnIc.height;
			pstVencCtx->astChnCtx[i].chnIc.height = tmp;
		}

		s32Ret = CheckInputCfg(&pstVencCtx->astChnCtx[i].chnIc);
		if (s32Ret) {
			SAMPLE_PRT("CheckInputCfg failed\n");
			return CVI_FAILURE;
		}
	}
	return s32Ret;
}

CVI_S32 SAMPLE_IPC_VENC_Stop(VENC_CHN VencChn)
{
	CVI_S32 s32Ret;

	SAMPLE_PRT("CVI_VENC_StopRecvFrame [%d] here.\n", VencChn);
	s32Ret = CVI_VENC_StopRecvFrame(VencChn);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("CVI_VENC_StopRecvPic vechn[%d] failed with %#x!\n",
				VencChn, s32Ret);
		return CVI_FAILURE;
	}

	SAMPLE_PRT("CVI_VENC_ResetChn [%d] here.\n", VencChn);
	s32Ret = CVI_VENC_ResetChn(VencChn);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("CVI_VENC_ResetChn vechn[%d] failed with %#x!\n",
				VencChn, s32Ret);
		return CVI_FAILURE;
	}

	SAMPLE_PRT("CVI_VENC_DestroyChn [%d] here.\n", VencChn);
	s32Ret = CVI_VENC_DestroyChn(VencChn);
	if (s32Ret != CVI_SUCCESS) {
		CVI_VENC_ERR("CVI_VENC_DestroyChn vechn[%d] failed with %#x!\n",
				VencChn, s32Ret);
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}

static CVI_S32 Start_Venc(SAMPLE_VENC_CTX *pstVencCtx)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 numChn = pstVencCtx->s32VencChnCnt;

	for (CVI_S32 s32ChnIdx = 0; s32ChnIdx < numChn; s32ChnIdx++) {
		s32Ret = Venc_Init_Chn(s32ChnIdx, &pstVencCtx->astChnCtx[s32ChnIdx]);
		if (s32Ret) {
			SAMPLE_PRT("[Chn %d]venc init chn failed\n", s32ChnIdx);
			return CVI_FAILURE;
		}

		if (!strcmp(pstVencCtx->astChnCtx[s32ChnIdx].chnIc.codec, "jpg")) {
			s32Ret = pthread_create(&gs_IpcVencTask[s32ChnIdx], NULL,
				ThreadFrameToJpeg, (CVI_VOID *)&pstVencCtx->astChnCtx[s32ChnIdx]);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("pthread_create failed with %#x!\n", s32Ret);
				return s32Ret;
			}
		} else {
			gs_IpcVencTask[s32ChnIdx] = 0;
#ifndef _SINGLETHREAD_FORALLSTREAM_
			s32Ret = pthread_create(
					&gs_IpcVencTask[s32ChnIdx],
					NULL,
					GetVencStreamProc,
					(CVI_VOID *)&pstVencCtx->astChnCtx[s32ChnIdx]);
			if (s32Ret) {
				SAMPLE_PRT("[Chn %d]pthread_create failed:%s\n", s32ChnIdx, strerror(errno));
				return CVI_FAILURE;
			}

			SAMPLE_PRT("[Chn %d] Start recv Stream\n", s32ChnIdx);
		}
	}
#else
		}
	}
	s32Ret = pthread_create(
			&gs_IpcVencAllTask,
			NULL,
			GetVencAllStreamProc,
			(CVI_VOID *)pstVencCtx);
	if (s32Ret) {
		SAMPLE_PRT("pthread_create GetVencAllStreamProc failed\n");
		return CVI_FAILURE;
	}

	SAMPLE_PRT("Start_Venc for all Success\n");
#endif
	return s32Ret;
}


static CVI_VOID Stop_Venc(SAMPLE_VENC_CTX *pstVencCtx)
{
	chnInputCfg *pIc;
	vencChnCtx *pstChnCtx;
	CVI_S32 s32ChnIdx;

	for (s32ChnIdx = 0; s32ChnIdx < pstVencCtx->s32VencChnCnt; s32ChnIdx++) {
		pstChnCtx = &pstVencCtx->astChnCtx[s32ChnIdx];
		pIc = &pstChnCtx->chnIc;

		//pstChnCtx->nextChnStat = CHN_STAT_STOP;
		pstChnCtx->chnStat = CHN_STAT_STOP;
		usleep(1000);
		if (gs_IpcVencTask[s32ChnIdx] != 0) {
			pthread_join(gs_IpcVencTask[s32ChnIdx], NULL);
			gs_IpcVencTask[s32ChnIdx] = 0;
		}

		if (pIc->bind_mode == VENC_BIND_VPSS) {
			SAMPLE_COMM_VPSS_UnBind_VENC(pIc->vpssGrp, pIc->vpssChn, s32ChnIdx);
		}

		SAMPLE_IPC_VENC_Stop(s32ChnIdx);
	}

	for (s32ChnIdx = 0; s32ChnIdx < pstVencCtx->s32VencChnCnt; s32ChnIdx++) {
		pstChnCtx = &pstVencCtx->astChnCtx[s32ChnIdx];
		pIc = &pstChnCtx->chnIc;

		if (pstChnCtx->pFile) {
			fclose(pstChnCtx->pFile);
			pstChnCtx->pFile = NULL;
		}

		if (pIc->bind_mode == VENC_BIND_DISABLE) {
			if (pstChnCtx->fpSrc) {
				fclose(pstChnCtx->fpSrc);
				pstChnCtx->fpSrc = NULL;
			}
		}
	}
	SAMPLE_PRT("venc stop\n");
}

static CVI_S32 StartOneChnVenc(int s32ChnIdx, SAMPLE_VENC_CTX *pstVencCtx, chnInputCfg *pIpcVencChnCfg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	memset(&pstVencCtx->astChnCtx[s32ChnIdx], 0, sizeof(vencChnCtx));
	memcpy(&pstVencCtx->astChnCtx[s32ChnIdx].chnIc, pIpcVencChnCfg, sizeof(chnInputCfg));

	s32Ret = CheckInputCfg(&pstVencCtx->astChnCtx[s32ChnIdx].chnIc);
	if (s32Ret) {
		SAMPLE_PRT("CheckInputCfg failed\n");
		return CVI_FAILURE;
	}

	s32Ret = Venc_Init_Chn(s32ChnIdx, &pstVencCtx->astChnCtx[s32ChnIdx]);
	if (s32Ret) {
		SAMPLE_PRT("[Chn %d]venc init chn failed\n", s32ChnIdx);
		return CVI_FAILURE;
	}

	s32Ret = pthread_create(
			&gs_IpcVencTask[s32ChnIdx],
			NULL,
			GetVencStreamProc,
			(CVI_VOID *)&pstVencCtx->astChnCtx[s32ChnIdx]);
	if (s32Ret) {
		SAMPLE_PRT("[Chn %d]pthread_create failed\n", s32ChnIdx);
		return CVI_FAILURE;
	}

	SAMPLE_PRT("[Chn %d] Start recv Stream\n", s32ChnIdx);

	return s32Ret;
}

static CVI_VOID StopOneChnVenc(int s32ChnIdx, SAMPLE_VENC_CTX *pstVencCtx)
{
	chnInputCfg *pIc;
	vencChnCtx *pstChnCtx;

	pstChnCtx = &pstVencCtx->astChnCtx[s32ChnIdx];
	pIc = &pstChnCtx->chnIc;

	pstChnCtx->chnStat = CHN_STAT_STOP;
	//printf("========pthread_join begin======\n");
	usleep(1000);
	if (gs_IpcVencTask[s32ChnIdx] != 0) {
		pthread_join(gs_IpcVencTask[s32ChnIdx], NULL);
		gs_IpcVencTask[s32ChnIdx] = 0;
	}
	//printf("========pthread_join end======\n");

	if (pIc->bind_mode == VENC_BIND_VPSS) {
		SAMPLE_COMM_VPSS_UnBind_VENC(pIc->vpssGrp, pIc->vpssChn, s32ChnIdx);
	}
	//printf("========[%s][%d]======\n", __func__, __LINE__);
	SAMPLE_IPC_VENC_Stop(s32ChnIdx);

	if (pstChnCtx->pFile) {
		fclose(pstChnCtx->pFile);
		pstChnCtx->pFile = NULL;
	}

	if (pIc->bind_mode == VENC_BIND_DISABLE) {
		if (pstChnCtx->fpSrc) {
			fclose(pstChnCtx->fpSrc);
			pstChnCtx->fpSrc = NULL;
		}
	}
	SAMPLE_PRT("venc %d stop\n", s32ChnIdx);
}


void *AI_Thread(void *arg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VIDEO_FRAME_INFO_S stVideoFrame1, stVideoFrame2;

	prctl(PR_SET_NAME, "AI_Thread");

	while (true) {
		s32Ret = CVI_VPSS_GetChnFrame(3, 0, &stVideoFrame1, 3000);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_GetChnFrame failed with %#x\n", s32Ret);
			break;
		}
		s32Ret = CVI_VPSS_GetChnFrame(3, 1, &stVideoFrame2, 3000);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VPSS_GetChnFrame failed with %#x\n", s32Ret);
			break;
		}

		if (CVI_VPSS_ReleaseChnFrame(3, 0, &stVideoFrame1) != CVI_SUCCESS)
			SAMPLE_PRT("CVI_VPSS_ReleaseChnFrame NG\n");

		if (CVI_VPSS_ReleaseChnFrame(3, 1, &stVideoFrame2) != CVI_SUCCESS)
			SAMPLE_PRT("CVI_VPSS_ReleaseChnFrame NG\n");

		usleep(30*1000);
	}

	return NULL;
}

static CVI_S32 Start_AI(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	pthread_t ai_thread;

	s32Ret = pthread_create(&ai_thread, NULL, AI_Thread, NULL);

	return s32Ret;
}

static void GetTimeStr(const struct tm *pstTime, char *pazStr, int s32MaxLen)
{
	time_t nowTime;
	struct tm stTime = {
		0,
	};

	if (!pstTime) {
		time(&nowTime);
		localtime_r(&nowTime, &stTime);
		pstTime = &stTime;
	}

	snprintf(pazStr, s32MaxLen, "%04d-%02d-%02d %02d:%02d:%02d",
		pstTime->tm_year + 1900, pstTime->tm_mon + 1, pstTime->tm_mday,
		pstTime->tm_hour, pstTime->tm_min, pstTime->tm_sec);
}

static int GetNonASCNum(char *string, int len)
{
	int i;
	int n = 0;

	for (i = 0; i < len; i++) {
		if (string[i] == '\0')
			break;
		if (!IsASCII(string[i])) {
			i++;
			n++;
		}
	}

	return n;
}

static int GetFontMod(char *Character, uint8_t **FontMod, int *FontModLen)
{
	uint32_t offset = 0;
	uint32_t areacode = 0;
	uint32_t bitcode = 0;

	if (IsASCII(Character[0])) {
		areacode = 3;
		bitcode = (uint32_t)((uint8_t)Character[0] - 0x20);
	} else {
		areacode = (uint32_t)((uint8_t)Character[0] - 0xA0);
		bitcode = (uint32_t)((uint8_t)Character[1] - 0xA0);
	}
	offset = (94 * (areacode - 1) + (bitcode - 1)) * (OSD_LIB_FONT_W * OSD_LIB_FONT_H / 8);
	*FontMod = (uint8_t *)g_fontLib + offset;
	*FontModLen = OSD_LIB_FONT_W*OSD_LIB_FONT_H / 8;
	return CVI_SUCCESS;
}


static int OSDUpdateBitmap(RGN_HANDLE RgnHdl, char *szStr, BITMAP_S *pstBitmap)
{
	int s32Ret;
	uint32_t u32CanvasWidth, u32CanvasHeight, u32BgColor, u32Color;
	SIZE_S stFontSize;
	int s32StrLen = strnlen(szStr, MAX_STR_LEN);
	int NonASCNum = GetNonASCNum(szStr, s32StrLen);

	u32CanvasWidth = OSD_LIB_FONT_W * (s32StrLen - NonASCNum * (NOASCII_CHARACTER_BYTES - 1));
	u32CanvasHeight = OSD_LIB_FONT_H;
	stFontSize.u32Width = OSD_LIB_FONT_W;
	stFontSize.u32Height = OSD_LIB_FONT_H;
	u32BgColor = 0x7fff;
	u32Color = 0xffff;

	if (szStr == NULL) {
		SAMPLE_PRT("szStr NULL pointer!\n");
		return CVI_FAILURE;
	}

	pstBitmap->pData = malloc(2 * (pstBitmap->u32Width) * (pstBitmap->u32Height));
	if (pstBitmap->pData == NULL) {
		SAMPLE_PRT("malloc osd memroy err!\n");
		return CVI_FAILURE;
	}

	uint16_t *puBmData = (uint16_t *)pstBitmap->pData;
	uint32_t u32BmRow, u32BmCol;

	for (u32BmRow = 0; u32BmRow < u32CanvasHeight; ++u32BmRow) {
		int NonASCShow = 0;

		for (u32BmCol = 0; u32BmCol < u32CanvasWidth; ++u32BmCol) {
			int s32BmDataIdx = u32BmRow * pstBitmap->u32Width + u32BmCol;
			int s32CharIdx = u32BmCol / stFontSize.u32Width;
			int s32StringIdx = s32CharIdx + NonASCShow * (NOASCII_CHARACTER_BYTES - 1);

			if (NonASCNum > 0 && s32CharIdx > 0) {
				NonASCShow = GetNonASCNum(szStr, s32StringIdx);
				s32StringIdx = s32CharIdx + NonASCShow * (NOASCII_CHARACTER_BYTES - 1);
			}
			int s32CharCol = (u32BmCol - (stFontSize.u32Width * s32CharIdx)) * OSD_LIB_FONT_W /
							stFontSize.u32Width;
			int s32CharRow = u32BmRow * OSD_LIB_FONT_H / stFontSize.u32Height;
			int s32HexOffset = s32CharRow * OSD_LIB_FONT_W / BYTE_BITS + s32CharCol / BYTE_BITS;
			int s32BitOffset = s32CharCol % BYTE_BITS;
			uint8_t *FontMod = NULL;
			int FontModLen = 0;

			if (GetFontMod(&szStr[s32StringIdx], &FontMod, &FontModLen) == CVI_SUCCESS) {
				if (FontMod != NULL && s32HexOffset < FontModLen) {
					uint8_t temp = FontMod[s32HexOffset];

					if ((temp >> ((BYTE_BITS - 1) - s32BitOffset)) & 0x1)
						puBmData[s32BmDataIdx] = (uint16_t)u32Color;
					else
						puBmData[s32BmDataIdx] = (uint16_t)u32BgColor;
					continue;
				}
			}
			SAMPLE_PRT("GetFontMod Fail\n");
			return CVI_FAILURE;
		}
	}

	s32Ret = CVI_RGN_SetBitMap(RgnHdl, pstBitmap);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_RGN_SetBitMap failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

	free(pstBitmap->pData);

	return s32Ret;
}

static CVI_S32 OSD_Create(SAMPLE_OSD_ATTR_S *pstOsdAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	RGN_ATTR_S stRegion;
	RGN_CHN_ATTR_S stChnAttr;
	CVI_S32 s32StrLen;
	char szStr[MAX_STR_LEN] = {0};
	char *pszStr = NULL;
	BITMAP_S stBitmap;
	RGN_HANDLE Handle = pstOsdAttr->Handle;
	OSD_TYPE_E enType = pstOsdAttr->enType;

	if (enType == TYPE_PICTURE) {
		stRegion.unAttr.stOverlay.stSize.u32Height = pstOsdAttr->stRect.u32Height;
		stRegion.unAttr.stOverlay.stSize.u32Width = pstOsdAttr->stRect.u32Width;
	} else if (enType == TYPE_STRING) {
		s32StrLen = strlen(pstOsdAttr->str);
		pszStr = pstOsdAttr->str;
		stRegion.unAttr.stOverlay.stSize.u32Height = OSD_LIB_FONT_H;
		stRegion.unAttr.stOverlay.stSize.u32Width = OSD_LIB_FONT_W * s32StrLen;
	} else if (enType == TYPE_TIME) {
		GetTimeStr(NULL, szStr, MAX_STR_LEN);
		s32StrLen = strnlen(szStr, MAX_STR_LEN);
		pszStr = szStr;
		stRegion.unAttr.stOverlay.stSize.u32Height = OSD_LIB_FONT_H;
		stRegion.unAttr.stOverlay.stSize.u32Width = OSD_LIB_FONT_W * s32StrLen;
	}

	stRegion.enType = OVERLAY_RGN;
	stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	stRegion.unAttr.stOverlay.u32BgColor = 0x00000000; // ARGB1555 transparent
	stRegion.unAttr.stOverlay.u32CanvasNum = 2;
	s32Ret = CVI_RGN_Create(Handle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}


	stChnAttr.bShow = CVI_TRUE;
	stChnAttr.enType = OVERLAY_RGN;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = pstOsdAttr->stRect.s32X;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = pstOsdAttr->stRect.s32Y;
	stChnAttr.unChnAttr.stOverlayChn.u32Layer = 0;
	s32Ret = CVI_RGN_AttachToChn(Handle, &pstOsdAttr->stChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		goto EXIT0;
	}

	if (enType == TYPE_PICTURE) {
		s32Ret = SAMPLE_COMM_REGION_GetUpCanvas(Handle, pstOsdAttr->filename);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_REGION_GetUpCanvas failed!\n");
			goto EXIT1;
		}
	} else if (enType == TYPE_STRING || enType == TYPE_TIME) {
		stBitmap.u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
		stBitmap.u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;
		stBitmap.enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
		s32Ret = OSDUpdateBitmap(Handle, pszStr, &stBitmap);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("OSDUpdateBitmap failed!\n");
			goto EXIT1;
		}
	}

	return CVI_SUCCESS;

EXIT1:
	s32Ret = CVI_RGN_DetachFromChn(Handle, &pstOsdAttr->stChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_RGN_DetachFromChn failed with %#x!\n", s32Ret);
	}

EXIT0:
	s32Ret = CVI_RGN_Destroy(Handle);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_RGN_Destroy failed with %#x!\n", s32Ret);
	}

	return s32Ret;
}

static CVI_VOID OSD_Destroy(SAMPLE_OSD_ATTR_S *pstOsdAttr)
{
	CVI_S32 s32Ret;
	RGN_HANDLE Handle = pstOsdAttr->Handle;

	CVI_RGN_DetachFromChn(Handle, &pstOsdAttr->stChn);

	s32Ret = CVI_RGN_Destroy(Handle);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_RGN_Destroy failed with %#x!\n", s32Ret);
	}
}

static void *UpdateTimestamp(void *arg)
{
	CVI_S32 s32Ret;
	CVI_S32 num = ARRAY_SIZE(IpcOSDcfg);
	BITMAP_S stBitmap;
	RGN_ATTR_S stRegion;
	CVI_BOOL bflag = CVI_FALSE;
	char szStr[MAX_STR_LEN] = {0};


	while (bRunTime) {
		GetTimeStr(NULL, szStr, MAX_STR_LEN);
		for (CVI_S32 i = 0; i < num; i++) {
			if (IpcOSDcfg[i].enType == TYPE_TIME) {
				if (!bflag) {
					s32Ret = CVI_RGN_GetAttr(IpcOSDcfg[i].Handle, &stRegion);
					if (s32Ret != CVI_SUCCESS) {
						SAMPLE_PRT("CVI_RGN_GetAttr failed!\n");
						return NULL;
					}
					stBitmap.u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
					stBitmap.u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;
					stBitmap.enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
					bflag = CVI_TRUE;
				}
				s32Ret = OSDUpdateBitmap(IpcOSDcfg[i].Handle, szStr, &stBitmap);
				if (s32Ret != CVI_SUCCESS) {
					SAMPLE_PRT("OSDUpdateBitmap failed!\n");
					return NULL;
				}
			}
		}

		sleep(1);
	}
	return NULL;
}

static CVI_S32 COVER_Create(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	RGN_ATTR_S stRegion;
	RGN_CHN_ATTR_S stChnAttr;
	// CVI_S32 s32StrLen;
	RGN_HANDLE Handle = 50;
	MMF_CHN_S stChn = {CVI_ID_VPSS, 1, 1};

	stRegion.enType = COVER_RGN;
	s32Ret = CVI_RGN_Create(Handle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
		return CVI_FAILURE;
	}

#if 1
	stChnAttr.bShow = CVI_TRUE;
	stChnAttr.enType = COVER_RGN;
	stChnAttr.unChnAttr.stCoverChn.enCoverType = AREA_RECT;
	stChnAttr.unChnAttr.stCoverChn.stRect.u32Height = 100;
	stChnAttr.unChnAttr.stCoverChn.stRect.u32Width = 100;
	stChnAttr.unChnAttr.stCoverChn.u32Color = 0x0000ffff;
	stChnAttr.unChnAttr.stCoverChn.enCoordinate = RGN_ABS_COOR;
	stChnAttr.unChnAttr.stCoverChn.stRect.s32X = 20;
	stChnAttr.unChnAttr.stCoverChn.stRect.s32Y = 520;
	stChnAttr.unChnAttr.stCoverChn.u32Layer = 0;
#else
	stChnAttr.bShow = CVI_TRUE;
	stChnAttr.enType = COVEREX_RGN;
	stChnAttr.unChnAttr.stCoverExChn.enCoverType = AREA_RECT;
	stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = 100;
	stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width = 100;
	stChnAttr.unChnAttr.stCoverExChn.u32Color = 0x0000ffff;
	stChnAttr.unChnAttr.stCoverExChn.stRect.s32X = 20;
	stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y = 520;
	stChnAttr.unChnAttr.stCoverExChn.u32Layer = 0;

#endif
	s32Ret = CVI_RGN_AttachToChn(Handle, &stChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		goto EXIT0;
	}

	return CVI_SUCCESS;

EXIT0:
	s32Ret = CVI_RGN_Destroy(Handle);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_RGN_Destroy failed with %#x!\n", s32Ret);
	}

	return s32Ret;
}

static CVI_VOID COVER_Destroy(void)
{
	CVI_S32 s32Ret;
	RGN_HANDLE Handle = 50;
	MMF_CHN_S stChn = {CVI_ID_VPSS, 1, 1};

	CVI_RGN_DetachFromChn(Handle, &stChn);

	s32Ret = CVI_RGN_Destroy(Handle);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_RGN_Destroy failed with %#x!\n", s32Ret);
	}
}

static CVI_S32 OSD_Init(CVI_VOID)
{
	CVI_S32 s32Ret;
	CVI_S32 num = ARRAY_SIZE(IpcOSDcfg);
	CVI_BOOL Timestamp = CVI_FALSE;

	for (CVI_S32 i = 0; i < num; i++) {
		s32Ret = OSD_Create(&IpcOSDcfg[i]);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("OSD_Create failed!\n");
			return CVI_FAILURE;
		}
		if (IpcOSDcfg[i].enType == TYPE_TIME)
			Timestamp = CVI_TRUE;
	}

	if (Timestamp) {
		bRunTime = CVI_TRUE;
		if (pthread_create(&pthTime, NULL, UpdateTimestamp, NULL) != 0) {
			SAMPLE_PRT("pthread_create failed!\n");
			goto exit;
		}
	}
	return CVI_SUCCESS;

exit:
	for (CVI_S32 i = 0; i < num; i++)
		OSD_Destroy(&IpcOSDcfg[i]);

	return CVI_FAILURE;
}

static CVI_VOID OSD_Deinit(CVI_VOID)
{
	CVI_S32 num = ARRAY_SIZE(IpcOSDcfg);
	CVI_BOOL Timestamp = CVI_FALSE;

	for (CVI_S32 i = 0; i < num; i++) {
		if (IpcOSDcfg[i].enType == TYPE_TIME)
			Timestamp = CVI_TRUE;
	}
	if (Timestamp) {
		bRunTime = CVI_FALSE;
		if (pthTime > 0)
			pthread_join(pthTime, NULL);
	}

	for (CVI_S32 i = 0; i < num; i++)
		OSD_Destroy(&IpcOSDcfg[i]);

	SAMPLE_PRT("osd deinit\n");
}
#ifdef __ENABLE_RTSP__
static CVI_VOID connect(const char *ip, CVI_VOID *arg)
{

	SAMPLE_PRT("rtsp connect: %s\n", ip);
}

static CVI_VOID disconnect(const char *ip, CVI_VOID *arg)
{

	SAMPLE_PRT("rtsp disconnect: %s\n", ip);
}

static CVI_S32 create_rtsp_server(CVI_VOID)
{
	CVI_RTSP_CONFIG config = {0};
	CVI_RTSP_CTX *pstServerCtx = NULL;

	config.port = 8554;
	if (CVI_RTSP_Create(&pstServerCtx, &config) < 0) {
		SAMPLE_PRT("fail to create rtsp\n");
		return CVI_FAILURE;
	}

	// set listener
	CVI_RTSP_STATE_LISTENER listener = {0};

	listener.onConnect = connect;
	listener.argConn = pstServerCtx;
	listener.onDisconnect = disconnect;
	CVI_RTSP_SetListener(pstServerCtx, &listener);

	if (CVI_RTSP_Start(pstServerCtx) < 0) {
		SAMPLE_PRT("fail to rtsp start\n");
		return CVI_FAILURE;
	}

	for (CVI_S32 i = 0; i < 6; i++) {
		CVI_RTSP_SESSION_ATTR attr = {0};

		s_astRtspCtx[i].pstServerCtx = pstServerCtx;
		s_astRtspCtx[i].VencChn = i;
		if (strcmp(IpcVencChnCfg[i].codec, "265") == 0) {
			attr.video.codec = RTSP_VIDEO_H265;
		} else if (strcmp(IpcVencChnCfg[i].codec, "264") == 0) {
			attr.video.codec = RTSP_VIDEO_H264;
		} else if (strcmp(IpcVencChnCfg[i].codec, "mjp") == 0) {
			attr.video.codec = RTSP_VIDEO_JPEG;
		}
#if 0
		switch (i) {
		case 0:
			attr.video.codec = RTSP_VIDEO_H265;
			s_astRtspCtx[i].VencChn = 0;
			break;
		case 1:
			attr.video.codec = RTSP_VIDEO_H264;
			s_astRtspCtx[i].VencChn = 1;
			break;
		case 2:
			attr.video.codec = RTSP_VIDEO_JPEG;
			s_astRtspCtx[i].VencChn = 2;
			break;
		case 3:
			attr.video.codec = RTSP_VIDEO_H264;
			s_astRtspCtx[i].VencChn = 3;
			break;
		case 4:
			attr.video.codec = RTSP_VIDEO_H264;
			s_astRtspCtx[i].VencChn = 4;
			break;
		case 5:
			attr.video.codec = RTSP_VIDEO_H264;
			s_astRtspCtx[i].VencChn = 5;
			break;
		default:
			break;
		}
#endif
		snprintf(attr.name, sizeof(attr.name), "live%d", i);
		CVI_RTSP_CreateSession(s_astRtspCtx[i].pstServerCtx, &attr,
			&s_astRtspCtx[i].pVideoSession);

		s_astRtspCtx[i].bStart = CVI_TRUE;
		SAMPLE_PRT("======rtsp start [VencChn%d  %s]  ======\n",
			s_astRtspCtx[i].VencChn, attr.name);
	}

	return 0;
}


static CVI_VOID stop_rtsp_server(CVI_VOID)
{
	CVI_RTSP_Stop(s_astRtspCtx[0].pstServerCtx);
	for (CVI_S32 i = 0; i < 6; i++) {

		if (s_astRtspCtx[i].bStart) {
			CVI_RTSP_DestroySession(s_astRtspCtx[i].pstServerCtx,
				s_astRtspCtx[i].pVideoSession);
			s_astRtspCtx[i].bStart = CVI_FALSE;
		}
	}

	CVI_RTSP_Destroy(&s_astRtspCtx[0].pstServerCtx);
	SAMPLE_PRT("rtsp stop\n");
}
#endif

static CVI_VOID Set_SingleESBuffer(void)
{
	CVI_S32 s32Ret;

	for (VENC_MODTYPE_E modtype = MODTYPE_H264E; modtype <= MODTYPE_JPEGE; modtype++) {
		VENC_PARAM_MOD_S stModParam;

		stModParam.enVencModType = modtype;
		s32Ret = CVI_VENC_GetModParam(&stModParam);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("CVI_VENC_GetModParam type %d failure\n", modtype);
			return;
		}

		switch (modtype) {
		case MODTYPE_H264E:
			stModParam.stH264eModParam.bSingleEsBuf = true;
			break;
		case MODTYPE_H265E:
			stModParam.stH265eModParam.bSingleEsBuf = true;
			break;
		case MODTYPE_JPEGE:
			stModParam.stJpegeModParam.bSingleEsBuf = true;
			break;
		default:
			CVI_VENC_ERR("SAMPLE_COMM_VENC_SetModParam invalid type %d failure\n", modtype);
			return;
		}

		s32Ret = CVI_VENC_SetModParam(&stModParam);
		if (s32Ret != CVI_SUCCESS) {
			CVI_VENC_ERR("CVI_VENC_SetModParam type %d failure\n", modtype);
			return;
		}
	}
}

static CVI_S32 SAMPLE_IPC(void)
{
#ifndef USING_INI_CFG
	SAMPLE_SNS_TYPE_E  enSnsType		= OV_OS08A20_MIPI_8M_30FPS_10BIT_WDR2TO1;
	WDR_MODE_E	   enWDRMode			= WDR_MODE_2To1_LINE;
	//SAMPLE_SNS_TYPE_E  enSnsType		= OV_OS08A20_MIPI_8M_30FPS_10BIT;
	//WDR_MODE_E	   enWDRMode		= WDR_MODE_NONE;
	DYNAMIC_RANGE_E    enDynamicRange	= DYNAMIC_RANGE_SDR8;
	PIXEL_FORMAT_E	   enPixFormat		= VI_PIXEL_FORMAT;
	VIDEO_FORMAT_E	   enVideoFormat	= VIDEO_FORMAT_LINEAR;
	COMPRESS_MODE_E    enCompressMode	= COMPRESS_MODE_NONE;
	// VI_VPSS_MODE_E	   enMastPipeMode	= VI_OFFLINE_VPSS_OFFLINE;
#else
	SAMPLE_INI_CFG_S stIniCfg = {};
	DYNAMIC_RANGE_E    enDynamicRange   = DYNAMIC_RANGE_SDR8;
	PIXEL_FORMAT_E	   enPixFormat	    = VI_PIXEL_FORMAT;
	VIDEO_FORMAT_E	   enVideoFormat    = VIDEO_FORMAT_LINEAR;
	COMPRESS_MODE_E    enCompressMode   = (low_mem_profile == 1) ? COMPRESS_MODE_TILE : COMPRESS_MODE_NONE;
	// VI_VPSS_MODE_E	   enMastPipeMode   = VI_OFFLINE_VPSS_OFFLINE;

	memset(&stIniCfg, 0, sizeof(SAMPLE_INI_CFG_S));
	stIniCfg.enSource  = VI_PIPE_FRAME_SOURCE_DEV;
	stIniCfg.devNum    = 1;
	stIniCfg.enSnsType[0] = SONY_IMX327_MIPI_2M_30FPS_12BIT;
	stIniCfg.enWDRMode[0] = WDR_MODE_NONE;
	stIniCfg.s32BusId[0]  = 3;
	stIniCfg.MipiDev[0]   = 0xFF;
	stIniCfg.u8UseMultiSns = 0;
#endif
	VB_CONFIG_S stVbConf;
	PIC_SIZE_E enPicSize;
	CVI_U32 u32BlkSize;
	CVI_S32 s32Ret = CVI_SUCCESS;

	VI_DEV ViDev = 0;
	VI_PIPE ViPipe = 0;
	VI_CHN ViChn = 0;
	CVI_S32 s32WorkSnsId = 0;
	VI_PIPE_ATTR_S	   stPipeAttr;

#ifdef USING_INI_CFG
	// Get config from ini if found.
	if (SAMPLE_COMM_VI_ParseIni(&stIniCfg)) {
		SAMPLE_PRT("Parse complete\n");
	}
#endif
	/************************************************
	 * step1:  Config VI
	 ************************************************/
	SAMPLE_COMM_VI_GetSensorInfo(&stViConfig);

#ifndef USING_INI_CFG
	stViConfig.astViInfo[s32WorkSnsId].stSnsInfo.enSnsType		 = enSnsType;
	stViConfig.s32WorkingViNum					 = 1;
	stViConfig.as32WorkingViId[0]					 = 0;
	stViConfig.astViInfo[s32WorkSnsId].stSnsInfo.MipiDev		 = 0xFF;
	stViConfig.astViInfo[s32WorkSnsId].stSnsInfo.s32BusId		 = 3;
	stViConfig.astViInfo[s32WorkSnsId].stDevInfo.ViDev		 = ViDev;
	stViConfig.astViInfo[s32WorkSnsId].stDevInfo.enWDRMode		 = enWDRMode;
	stViConfig.astViInfo[s32WorkSnsId].stPipeInfo.enMastPipeMode = enMastPipeMode;
	stViConfig.astViInfo[s32WorkSnsId].stPipeInfo.aPipe[0]		 = ViPipe;
	stViConfig.astViInfo[s32WorkSnsId].stPipeInfo.aPipe[1]		 = -1;
	stViConfig.astViInfo[s32WorkSnsId].stPipeInfo.aPipe[2]		 = -1;
	stViConfig.astViInfo[s32WorkSnsId].stPipeInfo.aPipe[3]		 = -1;
	stViConfig.astViInfo[s32WorkSnsId].stChnInfo.ViChn		 = ViChn;
	stViConfig.astViInfo[s32WorkSnsId].stChnInfo.enPixFormat	 = enPixFormat;
	stViConfig.astViInfo[s32WorkSnsId].stChnInfo.enDynamicRange  = enDynamicRange;
	stViConfig.astViInfo[s32WorkSnsId].stChnInfo.enVideoFormat	 = enVideoFormat;
	stViConfig.astViInfo[s32WorkSnsId].stChnInfo.enCompressMode  = enCompressMode;
#else
	SAMPLE_COMM_VI_IniToViCfg(&stIniCfg, &stViConfig);
	stViConfig.astViInfo[s32WorkSnsId].stChnInfo.ViChn	     = ViChn;
	stViConfig.astViInfo[s32WorkSnsId].stChnInfo.enPixFormat     = enPixFormat;
	stViConfig.astViInfo[s32WorkSnsId].stChnInfo.enDynamicRange  = enDynamicRange;
	stViConfig.astViInfo[s32WorkSnsId].stChnInfo.enVideoFormat   = enVideoFormat;
	stViConfig.astViInfo[s32WorkSnsId].stChnInfo.enCompressMode  = enCompressMode;
#endif
	/************************************************
	 * step2:  Get input size
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(stViConfig.astViInfo[s32WorkSnsId].stSnsInfo.enSnsType, &enPicSize);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VI_GetSizeBySensor failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &g_stSize);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed with %#x\n", s32Ret);
		return s32Ret;
	}

	INPUT_WIDTH = g_stSize.u32Width;
	INPUT_HEIGHT = g_stSize.u32Height;
	IpcVencChnCfg[0].width = INPUT_WIDTH;
	IpcVencChnCfg[0].height = INPUT_HEIGHT;

	/************************************************
	 * step3:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	CVI_U32 idx = 0;

	u32BlkSize = COMMON_GetPicBufferSize(INPUT_HEIGHT + 32, INPUT_WIDTH, SAMPLE_PIXEL_FORMAT, DATA_BITWIDTH_8
						, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	stVbConf.astCommPool[idx].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[idx].u32BlkCnt	= (low_mem_profile == 1) ? 4 : 12;
	if (INPUT_WIDTH == 1920) {
		stVbConf.astCommPool[idx].u32BlkCnt	= (low_mem_profile == 1) ? 9 : 20;
	}
	SAMPLE_PRT("common pool[%d] BlkSize %d\n", idx, u32BlkSize);
	idx++;

	if (INPUT_WIDTH != 1920) {
		u32BlkSize = COMMON_GetPicBufferSize(1080, 1920, SAMPLE_PIXEL_FORMAT, DATA_BITWIDTH_8
							, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		stVbConf.astCommPool[idx].u32BlkSize	= u32BlkSize;
		stVbConf.astCommPool[idx].u32BlkCnt	= (low_mem_profile == 1) ? 5 : 10;
		SAMPLE_PRT("common pool[%d] BlkSize %d\n", idx, u32BlkSize);
		idx++;
	}

	u32BlkSize = COMMON_GetPicBufferSize(720, 576, SAMPLE_PIXEL_FORMAT, DATA_BITWIDTH_8
						, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	stVbConf.astCommPool[idx].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[idx].u32BlkCnt	= (low_mem_profile == 1) ? 4 : 9;
	SAMPLE_PRT("common pool[%d] BlkSize %d\n", idx, u32BlkSize);
	idx++;

	u32BlkSize = COMMON_GetPicBufferSize(960, 540, SAMPLE_PIXEL_FORMAT, DATA_BITWIDTH_8
						, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	stVbConf.astCommPool[idx].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[idx].u32BlkCnt	= (low_mem_profile == 1) ? 3 : 8;
	SAMPLE_PRT("common pool[%d] BlkSize %d\n", idx, u32BlkSize);
	idx++;

	u32BlkSize = COMMON_GetPicBufferSize(320, 320, SAMPLE_PIXEL_FORMAT, DATA_BITWIDTH_8
						, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	stVbConf.astCommPool[idx].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[idx].u32BlkCnt	= 3;
	SAMPLE_PRT("common pool[%d] BlkSize %d\n", idx, u32BlkSize);
	idx++;
	stVbConf.u32MaxPoolCnt		= idx;

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("system init failed with %#x\n", s32Ret);
		return -1;
	}

	/************************************************
	 * step4:  Init VI ISP
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_StartSensor(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("system start sensor failed with %#x\n", s32Ret);
		goto EXIT0;
	}
	s32Ret = SAMPLE_COMM_VI_StartDev(&stViConfig.astViInfo[ViDev]);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("VI_StartDev failed with %#x!\n", s32Ret);
		goto EXIT0;
	}
	s32Ret = SAMPLE_COMM_VI_StartMIPI(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("system start MIPI failed with %#x\n", s32Ret);
		goto EXIT0;
	}

	stPipeAttr.bYuvSkip = CVI_FALSE;
	stPipeAttr.u32MaxW = g_stSize.u32Width;
	stPipeAttr.u32MaxH = g_stSize.u32Height;
	stPipeAttr.enPixFmt = PIXEL_FORMAT_RGB_BAYER_12BPP;
	stPipeAttr.enBitWidth = DATA_BITWIDTH_12;
	stPipeAttr.stFrameRate.s32SrcFrameRate = -1;
	stPipeAttr.stFrameRate.s32DstFrameRate = -1;
	stPipeAttr.bNrEn = CVI_TRUE;
	stPipeAttr.bYuvBypassPath = CVI_FALSE;
	stPipeAttr.enCompressMode = enCompressMode;
	s32Ret = CVI_VI_CreatePipe(ViPipe, &stPipeAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_CreatePipe failed with %#x!\n", s32Ret);
		goto EXIT0;
	}

	s32Ret = CVI_VI_StartPipe(ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_StartPipe failed with %#x!\n", s32Ret);
		goto EXIT0;
	}

	s32Ret = CVI_VI_GetPipeAttr(ViPipe, &stPipeAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_StartPipe failed with %#x!\n", s32Ret);
		goto EXIT0;
	}

	s32Ret = SAMPLE_COMM_VI_CreateIsp(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("VI_CreateIsp failed with %#x!\n", s32Ret);
		goto EXIT0;
	}

	s32Ret = SAMPLE_COMM_VI_StartViChn(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("StartViChn failed with %#x!\n", s32Ret);
		goto EXIT1;
	}
#if 0
	//set linear mode
	ISP_FSWDR_ATTR_S stFSWDRAttr;

	s32Ret = CVI_ISP_GetFSWDRAttr(ViPipe, &stFSWDRAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_ISP_GetFSWDRAttr failed with %#x!\n", s32Ret);
		goto EXIT1;
	}
	stFSWDRAttr.Enable = CVI_FALSE;

	s32Ret = CVI_ISP_SetFSWDRAttr(ViPipe, &stFSWDRAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_ISP_SetFSWDRAttr failed with %#x!\n", s32Ret);
		goto EXIT1;
	}
#endif

	/************************************************
	 * step5:  Init VPSS
	 ************************************************/
	VPSS_GRP aVpssGrp[5] = {0, 1, 2, 3, 4};
	ROTATION_E enRotation = ROTATION_0;

	if (CVI_VI_SetChnRotation(ViPipe, ViChn, enRotation)) {
		SAMPLE_PRT("CVI_VI_SetChnRotation failed\n");
		goto EXIT1;
	}

	for (CVI_S32 i = 0; i < 5; i++) {
		s32Ret = Init_Vpss(aVpssGrp[i], enRotation);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("Init vpss grp %d failed with %#x\n", aVpssGrp[i], s32Ret);
			goto EXIT2;
		}
	}

	for (CVI_S32 i = 0; i < 2; i++) {
		s32Ret = SAMPLE_COMM_VI_Bind_VPSS(ViPipe, ViChn, aVpssGrp[i]);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
			goto EXIT3;
		}
	}

	for (CVI_S32 i = 2; i < 5; i++) {
		s32Ret = SAMPLE_COMM_VPSS_Bind_VPSS(aVpssGrp[0], 0, aVpssGrp[i]);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
			goto EXIT3;
		}
	}

	/************************************************
	 * step6:  Init VENC
	 ************************************************/

	s32Ret = OSD_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("OSD_Init failed with %#x!\n", s32Ret);
		goto EXIT4;
	}

	s32Ret = COVER_Create();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("COVER_Create failed with %#x!\n", s32Ret);
		goto EXIT5;
	}
#ifdef __ENABLE_RTSP__
	/************************************************
	 * step7:  create rtsp server
	 ************************************************/
	s32Ret = create_rtsp_server();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("create_rtsp_server failed with %#x!\n", s32Ret);
		goto EXIT5;
	}
#endif
	/************************************************
	 * step8:  Init VENC
	 ************************************************/
	s32Ret = InitVencCtx(&stVencCtx, enRotation);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("InitVencCtx failed with %#x!\n", s32Ret);
		goto EXIT6;
	}

	Set_SingleESBuffer();

	s32Ret = Start_Venc(&stVencCtx);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Start_Venc failed with %#x!\n", s32Ret);
		goto EXIT6;
	}

	s32Ret = Start_AI();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Start_AI failed with %#x!\n", s32Ret);
		goto EXIT6;
	}


	//PAUSE();
	return CVI_SUCCESS;

	Stop_Venc(&stVencCtx);
EXIT6:
#ifdef __ENABLE_RTSP__
	stop_rtsp_server();
#endif
EXIT5:
	COVER_Destroy();
	OSD_Deinit();
EXIT4:

	for (CVI_S32 i = 0; i < 2; i++)
		SAMPLE_COMM_VI_UnBind_VPSS(ViPipe, ViChn, aVpssGrp[i]);

	for (CVI_S32 i = 2; i < 5; i++)
		SAMPLE_COMM_VPSS_UnBind_VPSS(aVpssGrp[0], 0, aVpssGrp[i]);

EXIT3:

	for (CVI_S32 i = 0; i < 5; i++)
		Deinit_Vpss(aVpssGrp[i]);
EXIT2:
	SAMPLE_COMM_VI_DestroyVi(&stViConfig);
EXIT1:
	SAMPLE_COMM_VI_DestroyIsp(&stViConfig);
EXIT0:
	SAMPLE_COMM_SYS_Exit();
	return s32Ret;
}

void SAMPLE_EXIT(void)
{
	VI_PIPE ViPipe = 0;
	VI_CHN ViChn = 0;
	VPSS_GRP aVpssGrp[5] = {0, 1, 2, 3, 4};

	Stop_Venc(&stVencCtx);
	// terminate called after throwing an instance of 'std::system_error'?
	// stop_rtsp_server();
	COVER_Destroy();
	OSD_Deinit();
	SAMPLE_PRT("Stop_Venc\n");
	for (CVI_S32 i = 0; i < 2; i++)
		SAMPLE_COMM_VI_UnBind_VPSS(ViPipe, ViChn, aVpssGrp[i]);

	for (CVI_S32 i = 2; i < 5; i++)
		SAMPLE_COMM_VPSS_UnBind_VPSS(aVpssGrp[0], 0, aVpssGrp[i]);


	for (CVI_S32 i = 0; i < 5; i++)
		Deinit_Vpss(aVpssGrp[i]);
	SAMPLE_PRT("Deinit_Vpss\n");
	SAMPLE_COMM_VI_DestroyVi(&stViConfig);
	SAMPLE_COMM_VI_DestroyIsp(&stViConfig);
	SAMPLE_COMM_SYS_Exit();
}

void SAMPLE_VIO_HandleSig(CVI_S32 signo)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	if (SIGINT == signo || SIGTERM == signo) {
		//todo for release
		SAMPLE_PRT("Program termination abnormally\n");
	}
	exit(-1);
}

#if 1
static CVI_S32 test_switch_isp_linear_wdr(CVI_S32 cnt)
{
	VI_PIPE ViPipe = 0;
	ISP_FSWDR_ATTR_S stFSWDRAttr;

	for (CVI_S32 i = 0; i < cnt; i++) {
		printf("========count:%d========\n", i);
		if (CVI_ISP_GetFSWDRAttr(ViPipe, &stFSWDRAttr)) {
			printf("CVI_ISP_GetFSWDRAttr failed\n");
			return -1;
		}
		if (i % 2)
			stFSWDRAttr.Enable = CVI_FALSE;
		else
			stFSWDRAttr.Enable = CVI_TRUE;
		if (CVI_ISP_SetFSWDRAttr(ViPipe, &stFSWDRAttr)) {
			printf("CVI_ISP_SetFSWDRAttr failed\n");
			return -1;
		}
		sleep(TEST_INTERVAL);
	}

	return CVI_SUCCESS;
}
#else
static CVI_S32 test_switch_isp_linear_wdr(CVI_S32 cnt)
{
	CVI_S32 s32Ret;
	CVI_S32 s32WorkSnsId = 0;
	VI_DEV ViDev = 0;
	VI_PIPE ViPipe = 0;
	VI_CHN ViChn = 0;
	SAMPLE_SNS_TYPE_E  enSnsType		= OV_OS08A20_MIPI_8M_30FPS_10BIT_WDR2TO1;
	WDR_MODE_E	   enWDRMode			= WDR_MODE_2To1_LINE;
	DYNAMIC_RANGE_E    enDynamicRange	= DYNAMIC_RANGE_SDR8;
	PIXEL_FORMAT_E	   enPixFormat		= PIXEL_FORMAT_YUV_PLANAR_420;
	VIDEO_FORMAT_E	   enVideoFormat	= VIDEO_FORMAT_LINEAR;
	COMPRESS_MODE_E    enCompressMode	= COMPRESS_MODE_NONE;
	VI_VPSS_MODE_E	   enMastPipeMode	= VI_OFFLINE_VPSS_OFFLINE;

	for (CVI_S32 i = 0; i < cnt; i++) {
		printf("========count:%d========\n", i);

		// Stop main sensor ISP & sensor.
		SAMPLE_COMM_VI_DestroyIsp(&stViConfig);
		// Stop VI.
		SAMPLE_COMM_VI_DestroyVi(&stViConfig);
		// Close ISP device.
		s32Ret = SAMPLE_COMM_VI_CLOSE();
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vi close failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
		if (i % 2) {
			enSnsType = OV_OS08A20_MIPI_8M_30FPS_10BIT;
			enWDRMode = WDR_MODE_NONE;

		} else {
			enSnsType = OV_OS08A20_MIPI_8M_30FPS_10BIT_WDR2TO1;
			enWDRMode = WDR_MODE_2To1_LINE;
		}

		SAMPLE_COMM_VI_GetSensorInfo(&stViConfig);

		stViConfig.astViInfo[s32WorkSnsId].stSnsInfo.enSnsType		 = enSnsType;
		stViConfig.s32WorkingViNum					 = 1;
		stViConfig.as32WorkingViId[0]					 = 0;
		stViConfig.astViInfo[s32WorkSnsId].stSnsInfo.MipiDev		 = 0xFF;
		stViConfig.astViInfo[s32WorkSnsId].stSnsInfo.s32BusId		 = 3;
		stViConfig.astViInfo[s32WorkSnsId].stDevInfo.ViDev		 = ViDev;
		stViConfig.astViInfo[s32WorkSnsId].stDevInfo.enWDRMode		 = enWDRMode;
		stViConfig.astViInfo[s32WorkSnsId].stPipeInfo.enMastPipeMode = enMastPipeMode;
		stViConfig.astViInfo[s32WorkSnsId].stPipeInfo.aPipe[0]		 = ViPipe;
		stViConfig.astViInfo[s32WorkSnsId].stPipeInfo.aPipe[1]		 = -1;
		stViConfig.astViInfo[s32WorkSnsId].stPipeInfo.aPipe[2]		 = -1;
		stViConfig.astViInfo[s32WorkSnsId].stPipeInfo.aPipe[3]		 = -1;
		stViConfig.astViInfo[s32WorkSnsId].stChnInfo.ViChn		 = ViChn;
		stViConfig.astViInfo[s32WorkSnsId].stChnInfo.enPixFormat	 = enPixFormat;
		stViConfig.astViInfo[s32WorkSnsId].stChnInfo.enDynamicRange  = enDynamicRange;
		stViConfig.astViInfo[s32WorkSnsId].stChnInfo.enVideoFormat	 = enVideoFormat;
		stViConfig.astViInfo[s32WorkSnsId].stChnInfo.enCompressMode  = enCompressMode;

		// open Isp device.
		s32Ret = SAMPLE_COMM_VI_OPEN();
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vi open failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
		// Initial VI & ISP.
		s32Ret = SAMPLE_PLAT_VI_INIT(&stViConfig);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vi init failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}

		sleep(5);
	}

	return CVI_SUCCESS;
}

#endif

static CVI_S32 test_switch_vi_rotate(CVI_S32 cnt)
{
	CVI_S32 n, s32Ret;
	ROTATION_E enRotation = ROTATION_0;
	VI_PIPE ViPipe = 0;
	VI_CHN ViChn = 0;
	VPSS_GRP aVpssGrp[5] = {0, 1, 2, 3, 4};

	for (n = 0; n < cnt; n++) {
		printf("========count:%d========\n", n);
		if (++enRotation > ROTATION_270)
			enRotation = ROTATION_0;

		Stop_Venc(&stVencCtx);
		OSD_Deinit();
		for (CVI_S32 i = 0; i < 2; i++)
			SAMPLE_COMM_VI_UnBind_VPSS(ViPipe, ViChn, aVpssGrp[i]);

		for (CVI_S32 i = 2; i < 5; i++)
			SAMPLE_COMM_VPSS_UnBind_VPSS(aVpssGrp[0], 0, aVpssGrp[i]);

		for (CVI_S32 i = 0; i < 5; i++)
			Deinit_Vpss(aVpssGrp[i]);
		if (CVI_VI_SetChnRotation(ViPipe, ViChn, enRotation)) {
			printf("CVI_VI_SetChnRotation failed\n");
			return -1;
		}
		for (CVI_S32 i = 0; i < 5; i++) {
			s32Ret = Init_Vpss(aVpssGrp[i], enRotation);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("Init vpss grp %d failed with %#x\n", aVpssGrp[i], s32Ret);
			}
		}

		for (CVI_S32 i = 0; i < 2; i++) {
			s32Ret = SAMPLE_COMM_VI_Bind_VPSS(ViPipe, ViChn, aVpssGrp[i]);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
			}
		}

		for (CVI_S32 i = 2; i < 5; i++) {
			s32Ret = SAMPLE_COMM_VPSS_Bind_VPSS(aVpssGrp[0], 0, aVpssGrp[i]);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
			}
		}
		s32Ret = OSD_Init();
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("OSD_Init failed with %#x!\n", s32Ret);
		}

		s32Ret = InitVencCtx(&stVencCtx, enRotation);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("InitVencCtx failed with %#x!\n", s32Ret);
		}
		s32Ret = Start_Venc(&stVencCtx);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("Start_Venc failed with %#x!\n", s32Ret);
		}

		sleep(TEST_INTERVAL);
	}
	return CVI_SUCCESS;
}

static CVI_S32 test_switch_flip_mirror(CVI_S32 cnt)
{
	CVI_S32 i;
	CVI_S32 s32Ret;
	VI_CHN ViChn = 0;
	CVI_BOOL bMirror = CVI_FALSE;
	CVI_BOOL bFlip = CVI_FALSE;
	VI_PIPE pipeID = 0;

	for (i = 0; i < cnt; i++) {
		printf("========count:%d========\n", i);
		if (i % 2) {
			bFlip = !bFlip;
		} else {
			bMirror = !bMirror;
		}
		s32Ret = CVI_VI_SetChnFlipMirror(pipeID, ViChn, bFlip, bMirror);
		if (s32Ret == CVI_ERR_VI_NOT_SUPPORT) {
			printf("CVI_VI_SetChnFlipMirror not support, bypassed\n");
			return CVI_SUCCESS;
		} else if (s32Ret) {
			printf("CVI_VI_SetChnFlipMirror failed\n");
			return -1;
		}

		sleep(TEST_INTERVAL);
	}
	return CVI_SUCCESS;

}

static CVI_S32 test_switch_venc_codec(CVI_S32 cnt)
{
	CVI_S32 i;
	VENC_CHN VeChn = 1;
	chnInputCfg newVencChnCfg;

	for (i = 0; i < cnt; i++) {
		printf("========count:%d========\n", i);
		StopOneChnVenc(VeChn, &stVencCtx);
		memcpy(&newVencChnCfg, &IpcVencChnCfg[VeChn], sizeof(chnInputCfg));

		if (i % 2)
			strcpy(newVencChnCfg.codec, CODEC_TYPE2);
		else
			strcpy(newVencChnCfg.codec, CODEC_TYPE2_2);

		if (StartOneChnVenc(VeChn, &stVencCtx, &newVencChnCfg)) {
			printf("StartOneChnVenc failed\n");
			return -1;
		}
		sleep(TEST_INTERVAL);
	}

	return CVI_SUCCESS;

}

static CVI_S32 test_switch_venc_resolution(CVI_S32 cnt)
{
	CVI_S32 i;
	VPSS_GRP VpssGrp = 1;
	VPSS_CHN VpssChn = 1;
	VENC_CHN VeChn = 1;
	chnInputCfg newVencChnCfg;
	VPSS_CHN_ATTR_S stChnAttr;
	CVI_U32 offset;

	for (i = 0; i < cnt; i++) {
		printf("========count:%d========\n", i);
		StopOneChnVenc(VeChn, &stVencCtx);
		memcpy(&newVencChnCfg, &IpcVencChnCfg[VeChn], sizeof(chnInputCfg));

		if (CVI_VPSS_GetChnAttr(VpssGrp, VpssChn, &stChnAttr)) {
			printf("CVI_VPSS_GetChnAttr failed\n");
			return -1;
		}
		for (int j = 5; j < 10; j++) {
			OSD_Destroy(&IpcOSDcfg[j]);
		}
		COVER_Destroy();
		CVI_VPSS_DisableChn(VpssGrp, VpssChn);

		if (i % 2) {
			newVencChnCfg.width = RES2_WIDTH;
			newVencChnCfg.height = RES2_HEIGHT;
			offset = 20;
		} else {
			newVencChnCfg.width = 1280;
			newVencChnCfg.height = 720;
			offset = 100;
		}

		stChnAttr.u32Width = newVencChnCfg.width;
		stChnAttr.u32Height = newVencChnCfg.height;
		if (CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stChnAttr)) {
			printf("CVI_VPSS_SetChnAttr failed\n");
			return -1;
		}
		CVI_VPSS_EnableChn(VpssGrp, VpssChn);
		for (int j = 5; j < 10; j++) {
			IpcOSDcfg[j].stRect.s32X = offset;
			OSD_Create(&IpcOSDcfg[j]);
		}
		COVER_Create();
		if (StartOneChnVenc(VeChn, &stVencCtx, &newVencChnCfg)) {
			printf("StartOneChnVenc failed\n");
			return -1;
		}
		sleep(TEST_INTERVAL);
	}

	return CVI_SUCCESS;
}


static CVI_S32 test_change_venc_bitrate(CVI_S32 cnt)
{
	CVI_S32 i;
	CVI_U32 MinBitRate = 512;
	CVI_U32 MaxBitRate = 4096;
	VENC_CHN VeChn = 1;
	VENC_CHN_ATTR_S stChnAttr;

	for (i = 0; i < cnt; i++) {
		printf("========count:%d========\n", i);
		if (CVI_VENC_GetChnAttr(VeChn, &stChnAttr)) {
			printf("CVI_VENC_GetChnAttr failed\n");
			return -1;
		}
		if (i % 2)
			stChnAttr.stRcAttr.stH264Cbr.u32BitRate = MaxBitRate;
		else
			stChnAttr.stRcAttr.stH264Cbr.u32BitRate = MinBitRate;


		if (CVI_VENC_SetChnAttr(VeChn, &stChnAttr)) {
			printf("CVI_VENC_SetChnAttr failed\n");
			return -1;
		}
		sleep(TEST_INTERVAL);
	}
	return CVI_SUCCESS;
}

static CVI_S32 test_change_venc_framerate(CVI_S32 cnt)
{
	CVI_S32 i;
	VENC_CHN VeChn = 1;
	VENC_CHN_ATTR_S stChnAttr;

	for (i = 0; i < cnt; i++) {
		printf("========count:%d========\n", i);
		if (CVI_VENC_GetChnAttr(VeChn, &stChnAttr)) {
			printf("CVI_VENC_GetChnAttr failed\n");
			return -1;
		}
		if (i % 2)
			stChnAttr.stRcAttr.stH264Cbr.fr32DstFrameRate = 30;
		else
			stChnAttr.stRcAttr.stH264Cbr.fr32DstFrameRate = 15;

		if (CVI_VENC_SetChnAttr(VeChn, &stChnAttr)) {
			printf("CVI_VENC_SetChnAttr failed\n");
			return -1;
		}
		sleep(TEST_INTERVAL);
	}
	return CVI_SUCCESS;
}

static CVI_S32 test_change_select_mode(CVI_S32 cnt)
{
	CVI_S32 i;

	for (i = 0; i < cnt; i++) {
		printf("========count:%d========\n", i);
		g_UseSelect = !g_UseSelect;
		printf("========Use Select :%d========\n", g_UseSelect);
		sleep(TEST_INTERVAL);
	}
	return CVI_SUCCESS;
}

static CVI_S32 handle_op(CVI_S32 op, CVI_S32 repeat)
{
	CVI_S32 s32Ret = CVI_FAILURE;
	CVI_S32 cnt;

	if (repeat == -1) {
		SAMPLE_PRT("repeat counter:");
		scanf("%d", &cnt);
	} else {
		cnt = repeat;
	}

	switch (op) {
	case 1:
		s32Ret = test_switch_vi_rotate(cnt);
		break;
	case 2:
		s32Ret = test_switch_flip_mirror(cnt);
		break;
	case 3:
		s32Ret = test_switch_isp_linear_wdr(cnt);
		break;
	case 4:
		s32Ret = test_switch_venc_codec(cnt);
		break;
	case 5:
		s32Ret = test_switch_venc_resolution(cnt);
		break;
	case 6:
		s32Ret = test_change_venc_bitrate(cnt);
		break;
	case 7:
		s32Ret = test_change_venc_framerate(cnt);
		break;
	case 8:
		s32Ret = test_change_select_mode(cnt);
		break;
	case 100:
		printf("========run test_switch_isp_linear_wdr ========\n");
		s32Ret = test_switch_isp_linear_wdr(cnt);
		if (s32Ret != CVI_SUCCESS) {
			syslog(LOG_ERR, "run test_switch_isp_linear_wdr fail");
			return s32Ret;
		}
		printf("========run test_switch_flip_mirror ========\n");
		s32Ret = test_switch_flip_mirror(cnt);
		if (s32Ret != CVI_SUCCESS) {
			syslog(LOG_ERR, "run test_switch_flip_mirror fail");
			return s32Ret;
		}
		printf("========run test_change_venc_bitrate ========\n");
		s32Ret = test_change_venc_bitrate(cnt);
		if (s32Ret != CVI_SUCCESS) {
			syslog(LOG_ERR, "run test_change_venc_bitrate fail");
			return s32Ret;
		}
		printf("========run test_change_venc_framerate ========\n");
		s32Ret = test_change_venc_framerate(cnt);
		if (s32Ret != CVI_SUCCESS) {
			syslog(LOG_ERR, "run test_change_venc_framerate fail");
			return s32Ret;
		}
	case 101:
		printf("========run test_switch_vi_rotate ========\n");
		s32Ret = test_switch_vi_rotate(cnt);
		if (s32Ret != CVI_SUCCESS) {
			syslog(LOG_ERR, "run test_switch_vi_rotate fail");
			return s32Ret;
		}
		printf("========run test_switch_venc_codec ========\n");
		s32Ret = test_switch_venc_codec(cnt);
		if (s32Ret != CVI_SUCCESS) {
			syslog(LOG_ERR, "run test_switch_venc_codec fail");
			return s32Ret;
		}
		printf("========run test_switch_venc_resolution ========\n");
		s32Ret = test_switch_venc_resolution(cnt);
		if (s32Ret != CVI_SUCCESS) {
			syslog(LOG_ERR, "run test_switch_venc_resolution fail");
			return s32Ret;
		}
	default:
		break;
	}

	return s32Ret;
}

#define ION_TOTALMEM "/sys/firmware/devicetree/base/reserved-memory/ion/size"
void load_ion_totalmem(void)
{
	FILE *fp = NULL;
	char mem_buf[16] = "";
	int64_t ion_total_mem = 0;

	fp = fopen(ION_TOTALMEM, "r");
	if (fp == NULL) {
		SAMPLE_PRT("fopen %s fail\n", ION_TOTALMEM);
		return;
	}
	if (fread(mem_buf, 1, sizeof(mem_buf), fp) > 0) {
		char *_low_mem_env = getenv("LOW_MEM_PROFILE");

		memcpy(&ion_total_mem, mem_buf, sizeof(int64_t));
		ion_total_mem = be64toh(ion_total_mem);
		SAMPLE_PRT("total memory %"PRId64" bytes\n", ion_total_mem);
		if (_low_mem_env) {
			low_mem_profile = atoi(_low_mem_env);
		} else if (ion_total_mem > 0) {
			if (ion_total_mem < 80 * 1024 * 1024) {
				low_mem_profile = 1;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	CVI_S32 s32Ret = CVI_FAILURE;
	int op;
	MMF_VERSION_S stVersion;
	struct sched_param param;

	CVI_SYS_GetVersion(&stVersion);
	SAMPLE_PRT("MMF Version:%s\n", stVersion.version);
	load_ion_totalmem();

	(CVI_VOID) argc;
	(CVI_VOID) argv;

	signal(SIGINT, SAMPLE_VIO_HandleSig);
	signal(SIGTERM, SAMPLE_VIO_HandleSig);

	s32Ret = SAMPLE_IPC();
	if (s32Ret != CVI_SUCCESS)
		SAMPLE_PRT("SAMPLE_IPC failed!\n");

	system("stty erase ^H");

	printf("Thread %d sched_setscheduler(SCHED_RR)\n", getpid());
	param.sched_priority = sched_get_priority_max(SCHED_RR);
	sched_setscheduler(0, SCHED_RR, &param);

	if (argc > 2) {
		int cnt = atoi(argv[2]);

		op = atoi(argv[1]);
		printf("argc = %d op = %d cnt = %d\n", argc, op, cnt);
		s32Ret = handle_op(op, cnt);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("op(%d) failed with %#x!\n", op, s32Ret);
		}
	} else {
	while (1) {
		op = 0;
		SAMPLE_PRT("----------------------test------------------------\n");
		SAMPLE_PRT("1: switch VI rotation (0/90/180/270)\n");
		SAMPLE_PRT("2: switch flip/mirror\n");
		SAMPLE_PRT("3: switch ISP WDR/linear\n");
		SAMPLE_PRT("4: switch VENC H264/H265\n");
		SAMPLE_PRT("5: switch VENC resolution\n");
		SAMPLE_PRT("6: change VENC bitrate\n");
		SAMPLE_PRT("7: change VENC framerate\n");
		SAMPLE_PRT("255: exit\n");
		scanf("%d", &op);
		if (op == 255)
			break;

		s32Ret = handle_op(op, -1);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("op(%d) failed with %#x!\n", op, s32Ret);
			break;
		}
		if (op > 99) {
			SAMPLE_PRT("op(%d) return with %#x!\n", op, s32Ret);
			break;
		}
	}
	}

	SAMPLE_EXIT();

	if (argc > 2) {
		if (s32Ret == CVI_SUCCESS) {
			printf("TEST_PASS\n");
		} else {
			printf("TEST_FAIL\n");
		}
	}

	return s32Ret;
}


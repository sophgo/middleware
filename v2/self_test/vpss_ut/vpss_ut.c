#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <pthread.h>

#include "cvi_buffer.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vpss.h"
#include "cvi_gdc.h"
#include "vpss_ut_comm.h"

#define DEFAULT_W 1920
#define DEFAULT_H 1080
#define DEFAULT_FBCTABLE_LENGTH 69632

#define THREAD_CNT 16

#ifndef FPGA_PORTING
#define UT_TIMEOUT_MS 1000
#define TEST_CNT0 10000
#define TEST_CNT1 1000
#else
#define UT_TIMEOUT_MS 60000
#define TEST_CNT0 100
#define TEST_CNT1 100
#endif

#define RANDOM(min, max) ((rand() % ((max) - (min) + 1)) + (min))


//file: http://disk-sophgo-vip.quickconnect.cn/sharing/iglxSGiJB
#define VPSS_DEFAULT_FILE_IN      "res/1080p.yuv420"
#define VPSS_LIMIT_FILE_IN        "res/4608_2592.rgb"
#define VPSS_TILE_MODE_FILE_IN    "res/5000_2000.yuv420"
#define VPSS_MAX_FILE_IN          "res/8192_8192.rgb"
#define VPSS_RGB_FILE_IN          "res/1080p.rgb"
#define VPSS_ROT_FILE_IN          "res/ldc/input/1920x1080.yuv"
#define VPSS_LDC_FILE_IN          "res/ldc/input/1920x1080_barrel_0.3.yuv"
#define VPSS_DWA_FILE_IN          "res/input/1920x1080_barrel_0.3.yuv"
#define DWA_FILE_IN_FISHEYE       "res/input/fisheye_floor_1024x1024.yuv"
#define VPSS_DEFAULT_FBC_FILE_IN0 "res/fbc/1920x1080_table_y.bin"
#define VPSS_DEFAULT_FBC_FILE_IN1 "res/fbc/1920x1080_table_c.bin"
#define VPSS_DEFAULT_FBC_FILE_IN2 "res/fbc/1920x1080_data_y.bin"
#define VPSS_DEFAULT_FBC_FILE_IN3 "res/fbc/1920x1080_data_c.bin"
#define VPSS_STITCH_FILE_IN0      "res/1080p.yuv420"
#define VPSS_STITCH_FILE_IN1      "res/1920_1080_422p.yuv"

#define MD5_BASIC             "ba2c34cc33a6ee83732b6ced5ea7c8bf"
#define MD5_FBD_BASIC         "4fab5140588725ad4af8ef50c2900bf5"
#define MD5_1_TO_2_CHN0       "d7ce686e6d32700784c6e83abb3916b1"
#define MD5_1_TO_2_CHN1       "ad2ec6fe09dea2b4c7163b62c0fad5ce"
#define MD5_1_TO_3_CHN0       "08dbb8f414ecae651aca06761e073799"
#define MD5_1_TO_3_CHN1       "ac43e14957e0a7d97f0bb10e3b9f3603"
#define MD5_1_TO_3_CHN2       "c6a4ab39a2a61dddcb45b187846adafe"
#define MD5_1_TO_4_CHN0       "08dbb8f414ecae651aca06761e073799"
#define MD5_1_TO_4_CHN1       "0f000356a4060b5709a06e29ad35515d"
#define MD5_1_TO_4_CHN2       "d4489b059164c2590bd470bf50a69716"
#define MD5_1_TO_4_CHN3       "c6a4ab39a2a61dddcb45b187846adafe"
#define MD5_LIMIT_WIDTH       "6fa49fc0e5a8dfd6c10a2e2d5386e61b"
#define MD5_MAX_RES           "6ab482e5c66eb46fb41369524dc879c1"
#define MD5_MIRROR            "aac8aa00f04df8e5a13e985da519885f"
#define MD5_FLIP              "890e5209ffa53bfbd594dc7c5cc17edf"
#define MD5_MIRROR_FLIP       "79edba95635f1e8f8de793542b571c7e"
#define MD5_ASPECT_RATIO1     "b16632e310a37d2a98c50c05afa7719f"
#define MD5_ASPECT_RATIO2     "4e78f8bdb723fb8726a3531f8654c78c"
#define MD5_ASPECT_RATIO3     "e7715253f0cba6180d7c1bed7ae9f186"
#define MD5_BYTE_ALIGN        "85584eb3fee0a24b0f06ab4ef8efbc3e"
#define MD5_DRAW_RECT         "ba8a7a4c3de54cf115e67a9b4f3cf8e9"
#define MD5_AMP_BRIGHTNESS    "a2f26d7fd035144e34c8f16f510994e7"
#define MD5_AMP_CONTRAST      "5bb1c13775c4179850bb9f0682303dc3"
#define MD5_AMP_SATURATION    "8a55dfc8e6e4f4da2a93e1a4eb764141"
#define MD5_AMP_HUE           "188b1bae2c7fb4fe7789d12b8137be67"
#define MD5_NORMALIZE         "47c16ab4426300b684bc16fe0e8f6091"
#define MD5_CONVERT           "3cf37a3a754c484bb6a0068fddb99daa"
#define MD5_SCALE_COEF1       "9bf41589235d410777533254016cd579"
#define MD5_SCALE_COEF2       "d4d29d7a0f812d4c6f4474d3de770ac5"
#define MD5_SCALE_COEF3       "58956ab1812e82e79bd6be83da145c04"
#define MD5_SCALE_COEF4       "0225821009938c2f8f33297a8ed35827"
#define MD5_Y_RATIO           "74d67e4f04510d5cd9e41cd750663b00"
#define MD5_HIDE              "f07d0b060e007107c339f63652210453"
#define MD5_STITCH            "d43c764507f86485dab81a36beb47c53"
#define MD5_STITCH_PIP        "86c194575a06ce6d2a0c103cbe3f77e4"
#define MD5_STITCH_GRID       "f02427fc82a05ceee1efb7a3fca6ee8e"
#define MD5_TILE_CHN0         "1a0a02966deb1066b2e7884aa88384b4"
#define MD5_TILE_CHN1         "f8ee91af9ccb40bfb6c4c5ce7dbe4099"

#define SLT_REF               "res/1920_1080.nv21"

#define OUT_FILE_PREFIX           "./out"

#define GDC_FILE_IN_LDC_BARREL_0P3_MESH_0       "res/ldc/input/1920x1080_barrel_0.3_r0_ofst_0_0_d-200.mesh"

typedef struct _VPSS_BASIC_TEST_PARAM {
	VPSS_GRP VpssGrp;
	SIZE_S stSizeIn;
	SIZE_S stSizeOut;
	CVI_BOOL bMirror;
	CVI_BOOL bFlip;
	CVI_BOOL bHide;
	PIXEL_FORMAT_E enFormatIn;
	PIXEL_FORMAT_E enFormatOut;
	ASPECT_RATIO_S stAspectRatio;
	VPSS_NORMALIZE_S stNormalize;
	VPSS_CROP_INFO_S stGrpCropInfo;
	VPSS_CROP_INFO_S stChnCropInfo;
	VPSS_DRAW_RECT_S stDrawRect;
	VPSS_CONVERT_S stConvert;
	VPSS_LDC_ATTR_S stLDCAttr;
	FISHEYE_ATTR_S stFishEyeAttr;
	CVI_BOOL bUseLoadMesh;
	ROTATION_E enRotation;
	VPSS_SCALE_COEF_E enCoef;
	CVI_U32 u32ChnAlign;
	CVI_FLOAT YRatio;
	CVI_U32 u32CheckSum;
	CVI_U32 u32FbcTableLength;
	CVI_CHAR aszMD5Sum[33];
	CVI_CHAR aszFileNameIn[64];
	CVI_CHAR aszFileNameFbcIn[4][64];
	CVI_CHAR aszFileNameOut[64];
	CVI_CHAR aszFileNameRef[64];
} VPSS_BASIC_TEST_PARAM;


struct VPSS_CHN_PARAM {
	CVI_BOOL bEnable;
	VPSS_CHN VpssChn;
	SIZE_S stSizeOut;
	CVI_BOOL bMirror;
	CVI_BOOL bFlip;
	PIXEL_FORMAT_E enFormatOut;
	ASPECT_RATIO_S stAspectRatio;
	VPSS_NORMALIZE_S stNormalize;
	CVI_U32 u32CheckSum;
	CVI_CHAR aszMD5Sum[33];
	CVI_CHAR aszFileNameOut[64];
	CVI_CHAR aszFileNameRef[64];
} ;

typedef struct _VPSS_MULTI_TEST_PARAM {
	VPSS_GRP VpssGrp;
	SIZE_S stSizeIn;
	PIXEL_FORMAT_E enFormatIn;
	CVI_CHAR aszFileNameIn[64];
	struct VPSS_CHN_PARAM astChnParam[VPSS_MAX_CHN_NUM];
} VPSS_MULTI_TEST_PARAM;

typedef enum _VPSS_TEST_OP {
	VPSS_TEST_BASIC = 0,
	VPSS_TEST_1_to_2,
	VPSS_TEST_1_to_3,
	VPSS_TEST_1_to_4,
	VPSS_TEST_LIMIT_WIDTH,
	VPSS_TEST_MAX_RES,
	VPSS_TEST_MIRROR,
	VPSS_TEST_FLIP,
	VPSS_TEST_MIRROR_FLIP,
	VPSS_TEST_ASPECT_RATIO,
	VPSS_TEST_MULTI_GRP,
	VPSS_TEST_MULTI_THREAD,
	VPSS_TEST_BYTE_ALIGN,
	VPSS_TEST_RESIZE,
	VPSS_TEST_MAX_SCALING,
	VPSS_TEST_TILE_MODE,
	VPSS_TEST_DRAW_RECT,
	VPSS_TEST_FORMAT,
	VPSS_TEST_GRP_CROP,
	VPSS_TEST_CHN_CROP,
	VPSS_TEST_AMP_CTRL,
	VPSS_TEST_NORMALIZE,
	VPSS_TEST_CONVERT_TO,
	VPSS_TEST_SCALE_COEF,
	VPSS_TEST_Y_RATIO,
	VPSS_TEST_HIDE,
	VPSS_TEST_ROT,
	VPSS_TEST_LDC,
	VPSS_TEST_FISHEYE,
	VPSS_TEST_FBD,
	VPSS_TEST_PRESSURE,
	VPSS_TEST_PERF,
	VPSS_TEST_MP_GET_CHN_FRM,
	VPSS_TEST_STITCH,
	VPSS_TEST_STITCH_PIP,
	VPSS_TEST_STITCH_FOUR_GRID,
	VPSS_TEST_TILE_1_to_2,
	VPSS_TEST_SLT,
	VPSS_TEST_C_MODEL,
	VPSS_TEST_GET_REGION_LUMA,
	VPSS_TEST_USER_CONFIG = 100,
	VPSS_TEST_AUTO = 200,
} VPSS_TEST_OP;


static pthread_mutex_t s_SyncMutex = PTHREAD_MUTEX_INITIALIZER;
static CVI_U32 s_u32Flag;

void vpss_ut_HandleSig(CVI_S32 signo)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	if (SIGINT == signo || SIGTERM == signo) {
		CVI_SYS_Exit();
		CVI_VB_Exit();
		VPSS_UT_PRT("Program termination abnormally\n");
	}
	exit(-1);
}

static CVI_S32 basic(const VPSS_BASIC_TEST_PARAM *pTestParam)
{
	CVI_S32 i, s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = pTestParam->VpssGrp;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	VIDEO_FRAME_INFO_S stVideoFrame;
	CVI_BOOL bFlag = CVI_FALSE;
	CVI_BOOL bSaveFile = CVI_FALSE;
	MMF_CHN_S Chn;

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSizeIn = COMMON_GetPicBufferSize(pTestParam->stSizeIn.u32Width, pTestParam->stSizeIn.u32Height,
		pTestParam->enFormatIn, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSizeOut = COMMON_GetPicBufferSize(pTestParam->stSizeOut.u32Width, pTestParam->stSizeOut.u32Height,
		pTestParam->enFormatOut, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 2;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 1 + (((pTestParam->stLDCAttr.bEnable) || pTestParam->stFishEyeAttr.bEnable) ? 1 : 0);
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
	stVbConf.astCommPool[1].u32BlkCnt	= 1 + (pTestParam->stLDCAttr.bEnable || pTestParam->stFishEyeAttr.bEnable || pTestParam->enRotation ? 1 : 0);
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	VPSS_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn);
	VPSS_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = pTestParam->enFormatIn;
	stVpssGrpAttr.u32MaxW			     = pTestParam->stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH			     = pTestParam->stSizeIn.u32Height;

	if (pTestParam->stLDCAttr.bEnable && pTestParam->stLDCAttr.stAttr.enRotation != 0) {
		stVpssChnAttr.u32Width		    = ALIGN(pTestParam->stSizeIn.u32Width, DEFAULT_ALIGN);
		stVpssChnAttr.u32Height		    = ALIGN(pTestParam->stSizeIn.u32Height, DEFAULT_ALIGN);
	} else {
		stVpssChnAttr.u32Width		    = pTestParam->stSizeOut.u32Width;
		stVpssChnAttr.u32Height		    = pTestParam->stSizeOut.u32Height;
	}

	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = pTestParam->enFormatOut;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= pTestParam->bMirror;
	stVpssChnAttr.bFlip				= pTestParam->bFlip;
	stVpssChnAttr.stAspectRatio		= pTestParam->stAspectRatio;
	stVpssChnAttr.stNormalize		= pTestParam->stNormalize;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_AttachVbPool(VpssGrp, VpssChn, 1);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_AttachVbPool failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//grp crop
	if (pTestParam->stGrpCropInfo.bEnable) {
		s32Ret = CVI_VPSS_SetGrpCrop(VpssGrp, &pTestParam->stGrpCropInfo);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetGrpCrop failed with %#x\n", s32Ret);
			goto exit4;
		}
	}

	//chn crop
	if (pTestParam->stChnCropInfo.bEnable) {
		s32Ret = CVI_VPSS_SetChnCrop(VpssGrp, VpssChn, &pTestParam->stChnCropInfo);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetChnCrop failed with %#x\n", s32Ret);
			goto exit4;
		}
	}

	//chn Draw rect
	bFlag = CVI_FALSE;
	for (i = 0; i < VPSS_RECT_NUM; i++)
		if (pTestParam->stDrawRect.astRect[i].bEnable)
			bFlag = CVI_TRUE;
	if (bFlag) {
		s32Ret = CVI_VPSS_SetChnDrawRect(VpssGrp, VpssChn, &pTestParam->stDrawRect);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetChnDrawRect failed with %#x\n", s32Ret);
			goto exit4;
		}
	}

	//chn Convert to
	if (pTestParam->stConvert.bEnable) {
		s32Ret = CVI_VPSS_SetChnConvert(VpssGrp, VpssChn, &pTestParam->stConvert);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetChnConvert failed with %#x\n", s32Ret);
			goto exit4;
		}
	}

	//chn FishEye
	if (pTestParam->stFishEyeAttr.bEnable) {
		s32Ret = CVI_VPSS_SetChnFisheye(VpssGrp, VpssChn, &pTestParam->stFishEyeAttr);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetChnFisheye failed with %#x\n", s32Ret);
			goto exit4;
		}
	}

	//chn LDC
	if (pTestParam->stLDCAttr.bEnable) {
		// if (pTestParam->bUseLoadMesh) {
		// 	MESH_DUMP_ATTR_S MeshDumpAttr;

		// 	strcpy(MeshDumpAttr.binFileName , GDC_FILE_IN_LDC_BARREL_0P3_MESH_0);
		// 	MeshDumpAttr.enModId = CVI_ID_VPSS;
		// 	MeshDumpAttr.vpssMeshAttr.grp = 0;
		// 	MeshDumpAttr.vpssMeshAttr.chn = 0;

		// 	s32Ret = CVI_GDC_LoadMesh(&MeshDumpAttr, &pTestParam->stLDCAttr.stAttr);
		// 	if (s32Ret != CVI_SUCCESS) {
		// 		VPSS_UT_PRT("CVI_GDC_LoadMesh failed with %#x\n", s32Ret);
		// 		goto exit4;
		// 	}
		// } else {
		s32Ret = CVI_VPSS_SetChnLDCAttr(VpssGrp, VpssChn, &pTestParam->stLDCAttr);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetChnLDCAttr failed with %#x\n", s32Ret);
			goto exit4;
		}
		// }
	}

	//chn rotation
	if (pTestParam->enRotation != ROTATION_0) {
		s32Ret = CVI_VPSS_SetChnRotation(VpssGrp, VpssChn, pTestParam->enRotation);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetChnRotation failed with %#x\n", s32Ret);
			goto exit4;
		}
	}

	if (pTestParam->enRotation != ROTATION_0 || pTestParam->stFishEyeAttr.bEnable || pTestParam->stLDCAttr.bEnable) {
		Chn.enModId = CVI_ID_VPSS;
		Chn.s32DevId = VpssGrp;
		Chn.s32ChnId = VpssChn;
		s32Ret = CVI_GDC_AttachVbPool(&Chn, 1);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_GDC_AttachVbPool failed with %#x\n", s32Ret);
			goto exit4;
		}
	}

	//chn coef
	if (pTestParam->enCoef != VPSS_SCALE_COEF_BICUBIC) {
		s32Ret = CVI_VPSS_SetChnScaleCoefLevel(VpssGrp, VpssChn, pTestParam->enCoef);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetChnScaleCoefLevel failed with %#x\n", s32Ret);
			goto exit4;
		}
	}

	//chn align
	if (pTestParam->u32ChnAlign > 0) {
		s32Ret = CVI_VPSS_SetChnAlign(VpssGrp, VpssChn, pTestParam->u32ChnAlign);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetChnAlign failed with %#x\n", s32Ret);
			goto exit4;
		}
	}

	//chn YRatio
	if (pTestParam->YRatio > 0) {
		s32Ret = CVI_VPSS_SetChnYRatio(VpssGrp, VpssChn, pTestParam->YRatio);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetChnYRatio failed with %#x\n", s32Ret);
			goto exit4;
		}
	}

	//chn hide
	if (pTestParam->bHide) {
		s32Ret = CVI_VPSS_HideChn(VpssGrp, VpssChn);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_HideChn failed with %#x\n", s32Ret);
			goto exit4;
		}
	}

	//send frame
	s32Ret = FileSendToVpss(VpssGrp, &pTestParam->stSizeIn,
		pTestParam->enFormatIn, pTestParam->aszFileNameIn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("FileSendToVpss fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, UT_TIMEOUT_MS);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}
	VPSS_UT_PRT("***CVI_VPSS_GetChnFrame Success***\n");

	if (pTestParam->aszMD5Sum[0]) {
		if (CompareWithMD5(pTestParam->aszMD5Sum, &stVideoFrame)) {
			if (pTestParam->aszFileNameRef[0] &&
				CompareWithFile(pTestParam->aszFileNameRef, &stVideoFrame) == CVI_SUCCESS)
				s32Ret = CVI_SUCCESS;
			else {
				bSaveFile = CVI_TRUE;
				s32Ret = CVI_FAILURE;
				VPSS_UT_PRT("Compare MD5 fail, MD5:%s\n", pTestParam->aszMD5Sum);
			}
		} else {
			bSaveFile = CVI_FALSE;
		}
	}

	if (bSaveFile && pTestParam->aszFileNameOut[0]) {
		if (FrameSaveToFile(pTestParam->aszFileNameOut, &stVideoFrame)) {
			VPSS_UT_PRT("FrameSaveToFile. s32Ret: 0x%x !\n", s32Ret);
			CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);
			goto exit4;
		}
		VPSS_UT_PRT("output file:%s\n", pTestParam->aszFileNameOut);
	}

	CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);

	if (pTestParam->enRotation != ROTATION_0 || pTestParam->stFishEyeAttr.bEnable || pTestParam->stLDCAttr.bEnable) {
		Chn.enModId = CVI_ID_VPSS;
		Chn.s32DevId = VpssGrp;
		Chn.s32ChnId = VpssChn;
		s32Ret = CVI_GDC_DetachVbPool(&Chn);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_GDC_DetachVbPool failed with %#x\n", s32Ret);
			goto exit4;
		}
	}

exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();
	return s32Ret;
}

static CVI_S32 fbd_basic(const VPSS_BASIC_TEST_PARAM *pTestParam)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = pTestParam->VpssGrp;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	VIDEO_FRAME_INFO_S stVideoFrame;
	CVI_BOOL bSaveFile = CVI_TRUE;

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSizeIn = COMMON_GetPicBufferSize(pTestParam->stSizeIn.u32Width, pTestParam->stSizeIn.u32Height,
		pTestParam->enFormatIn, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSizeOut = COMMON_GetPicBufferSize(pTestParam->stSizeOut.u32Width, pTestParam->stSizeOut.u32Height,
		pTestParam->enFormatOut, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 2;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn * 2;
	stVbConf.astCommPool[0].u32BlkCnt	= 1;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
	stVbConf.astCommPool[1].u32BlkCnt	= 1 + (pTestParam->stLDCAttr.bEnable || pTestParam->enRotation ? 1 : 0);
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	VPSS_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn);
	VPSS_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = pTestParam->enFormatIn;
	stVpssGrpAttr.u32MaxW			     = pTestParam->stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH			     = pTestParam->stSizeIn.u32Height;

	stVpssChnAttr.u32Width		    = pTestParam->stSizeOut.u32Width;
	stVpssChnAttr.u32Height		    = pTestParam->stSizeOut.u32Height;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = pTestParam->enFormatOut;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= pTestParam->bMirror;
	stVpssChnAttr.bFlip				= pTestParam->bFlip;
	stVpssChnAttr.stAspectRatio		= pTestParam->stAspectRatio;
	stVpssChnAttr.stNormalize		= pTestParam->stNormalize;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_AttachVbPool(VpssGrp, VpssChn, 1);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_AttachVbPool failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//chn align
	if (pTestParam->u32ChnAlign > 0) {
		s32Ret = CVI_VPSS_SetChnAlign(VpssGrp, VpssChn, pTestParam->u32ChnAlign);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetChnAlign failed with %#x\n", s32Ret);
			goto exit4;
		}
	}

	//send frame
	s32Ret = FbcFileSendToVpss(VpssGrp, &pTestParam->stSizeIn,
		pTestParam->enFormatIn, pTestParam->aszFileNameFbcIn, pTestParam->u32FbcTableLength);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("FileSendToVpss fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, UT_TIMEOUT_MS);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}
	VPSS_UT_PRT("***CVI_VPSS_GetChnFrame Success***\n");

	if (pTestParam->aszMD5Sum[0]) {
		if (CompareWithMD5(pTestParam->aszMD5Sum, &stVideoFrame)) {
			bSaveFile = CVI_TRUE;
			s32Ret = CVI_FAILURE;
			VPSS_UT_PRT("Compare MD5 fail, MD5:%s\n", pTestParam->aszMD5Sum);
		} else {
			bSaveFile = CVI_FALSE;
		}
	}

	if (bSaveFile && pTestParam->aszFileNameOut[0]) {
		if (FrameSaveToFile(pTestParam->aszFileNameOut, &stVideoFrame)) {
			VPSS_UT_PRT("FrameSaveToFile. s32Ret: 0x%x !\n", s32Ret);
			CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);
			goto exit4;
		}
		VPSS_UT_PRT("output file:%s\n", pTestParam->aszFileNameOut);
	}

	if (pTestParam->aszFileNameRef[0]) {
		if (CompareWithFile(pTestParam->aszFileNameRef, &stVideoFrame)) {
			VPSS_UT_PRT("CompareWithFile fail.\n");
			CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);
			s32Ret = -1;
			goto exit4;
		}
	}

	CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);

exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();
	return s32Ret;
}

static CVI_S32 basic_mutli_chn(VPSS_MULTI_TEST_PARAM *pTestParam)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i, n;
	VPSS_GRP VpssGrp = pTestParam->VpssGrp;
	VPSS_CHN VpssChn;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	VIDEO_FRAME_INFO_S stVideoFrame;
	struct VPSS_CHN_PARAM *pstChnParam;
	CVI_BOOL bSaveFile = CVI_TRUE;

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSizeIn = COMMON_GetPicBufferSize(pTestParam->stSizeIn.u32Width, pTestParam->stSizeIn.u32Height,
		pTestParam->enFormatIn, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 1;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	VPSS_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn);
	n = 1;
	for (i = 0; i < VPSS_MAX_CHN_NUM; i++) {
		pstChnParam = &pTestParam->astChnParam[i];
		if (!pstChnParam->bEnable)
			continue;
		u32BlkSizeOut = COMMON_GetPicBufferSize(pstChnParam->stSizeOut.u32Width,
					pstChnParam->stSizeOut.u32Height, pstChnParam->enFormatOut,
					DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		stVbConf.astCommPool[n].u32BlkSize	= u32BlkSizeOut;
		stVbConf.astCommPool[n].u32BlkCnt	= 1;
		stVbConf.astCommPool[n].enRemapMode	= VB_REMAP_MODE_CACHED;
		VPSS_UT_PRT("common pool[%d] BlkSize %d\n", n, u32BlkSizeOut);
		n++;
	}
	stVbConf.u32MaxPoolCnt = n;

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = pTestParam->enFormatIn;
	stVpssGrpAttr.u32MaxW			     = pTestParam->stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH			     = pTestParam->stSizeIn.u32Height;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	n = 1;
	for (i = 0; i < VPSS_MAX_CHN_NUM; i++) {
		pstChnParam = &pTestParam->astChnParam[i];
		if (!pstChnParam->bEnable)
			continue;
		VpssChn = i;
		stVpssChnAttr.u32Width		    = pstChnParam->stSizeOut.u32Width;
		stVpssChnAttr.u32Height		    = pstChnParam->stSizeOut.u32Height;
		stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
		stVpssChnAttr.enPixelFormat		    = pstChnParam->enFormatOut;
		stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
		stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
		stVpssChnAttr.u32Depth			= 1;
		stVpssChnAttr.bMirror			= pstChnParam->bMirror;
		stVpssChnAttr.bFlip				= pstChnParam->bFlip;
		stVpssChnAttr.stAspectRatio		= pstChnParam->stAspectRatio;
		stVpssChnAttr.stNormalize		= pstChnParam->stNormalize;
		s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x, chn(%d)\n", s32Ret, i);
			goto exit2;
		}

		s32Ret = CVI_VPSS_AttachVbPool(VpssGrp, VpssChn, n);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_AttachVbPool failed with %#x\n", s32Ret);
			goto exit2;
		}

		s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_EnableChn failed with %#x, chn(%d)\n", s32Ret, i);
			goto exit2;
		}
		n++;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit2;
	}

	//send frame
	s32Ret = FileSendToVpss(VpssGrp, &pTestParam->stSizeIn,
		pTestParam->enFormatIn, pTestParam->aszFileNameIn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("FileSendToVpss fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit3;
	}

	for (i = 0; i < VPSS_MAX_CHN_NUM; i++) {
		pstChnParam = &pTestParam->astChnParam[i];
		if (!pstChnParam->bEnable)
			continue;
		VpssChn = i;
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, UT_TIMEOUT_MS);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("Grp(%d) Chn(%d), CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n",
				VpssGrp, VpssChn, s32Ret);
			goto exit3;
		}
		VPSS_UT_PRT("***Grp(%d) Chn(%d) CVI_VPSS_GetChnFrame Success***\n", VpssGrp, VpssChn);

		if (pstChnParam->aszMD5Sum[0]) {
			if (CompareWithMD5(pstChnParam->aszMD5Sum, &stVideoFrame)) {
				bSaveFile = CVI_TRUE;
				s32Ret = CVI_FAILURE;
				VPSS_UT_PRT("chn%d Compare MD5 fail, MD5:%s\n",
					i, pstChnParam->aszMD5Sum);
			} else {
				bSaveFile = CVI_FALSE;
			}
		}

		if (bSaveFile && pstChnParam->aszFileNameOut[0]) {
			if (FrameSaveToFile(pstChnParam->aszFileNameOut, &stVideoFrame)) {
				VPSS_UT_PRT("Grp(%d) Chn(%d),FrameSaveToFile. s32Ret: 0x%x !\n",
					VpssGrp, VpssChn, s32Ret);
			}
			VPSS_UT_PRT("output file:%s\n", pstChnParam->aszFileNameOut);
		}

		if (pstChnParam->aszFileNameRef[0]) {
			if (CompareWithFile(pstChnParam->aszFileNameRef, &stVideoFrame)) {
				VPSS_UT_PRT("Grp(%d) Chn(%d),CompareWithFile fail.\n", VpssGrp, VpssChn);
				CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);
				s32Ret = CVI_FAILURE;
				goto exit3;
			}
		}

		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);

		if (s32Ret)
			break;
	}

exit3:
	CVI_VPSS_StopGrp(VpssGrp);
exit2:
	for (i = 0; i < VPSS_MAX_CHN_NUM; i++) {
		if (!pTestParam->astChnParam[i].bEnable)
			continue;
		CVI_VPSS_DisableChn(VpssGrp, i);
	}
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();
	return s32Ret;
}

static CVI_S32 vpss_test_basic(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = DEFAULT_W;
	stTestParam.stSizeOut.u32Height = DEFAULT_H;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_NV21;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.u32CheckSum = 0x13030706;
	strncpy(stTestParam.aszMD5Sum, MD5_BASIC, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_fbd_basic(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = DEFAULT_W;
	stTestParam.stSizeOut.u32Height = DEFAULT_H;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.u32CheckSum = 0xc3f4e743;
	strncpy(stTestParam.aszMD5Sum, MD5_FBD_BASIC, sizeof(stTestParam.aszMD5Sum));
	stTestParam.u32FbcTableLength = DEFAULT_FBCTABLE_LENGTH;
	strncpy(stTestParam.aszFileNameFbcIn[0], VPSS_DEFAULT_FBC_FILE_IN0, sizeof(stTestParam.aszFileNameFbcIn[0]));
	strncpy(stTestParam.aszFileNameFbcIn[1], VPSS_DEFAULT_FBC_FILE_IN1, sizeof(stTestParam.aszFileNameFbcIn[1]));
	strncpy(stTestParam.aszFileNameFbcIn[2], VPSS_DEFAULT_FBC_FILE_IN2, sizeof(stTestParam.aszFileNameFbcIn[2]));
	strncpy(stTestParam.aszFileNameFbcIn[3], VPSS_DEFAULT_FBC_FILE_IN3, sizeof(stTestParam.aszFileNameFbcIn[3]));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = fbd_basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_1_to_2(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i, s32ChnNum = 2;
	VPSS_MULTI_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));

	for (i = 0; i < s32ChnNum; i++) {
		stTestParam.astChnParam[i].bEnable = CVI_TRUE;
		stTestParam.astChnParam[i].VpssChn = i;
		stTestParam.astChnParam[i].stSizeOut.u32Width = DEFAULT_W;
		stTestParam.astChnParam[i].stSizeOut.u32Height = DEFAULT_H;
		stTestParam.astChnParam[i].bMirror = CVI_FALSE;
		stTestParam.astChnParam[i].bFlip = CVI_FALSE;
		stTestParam.astChnParam[i].enFormatOut = (i == 0) ? PIXEL_FORMAT_NV21 : PIXEL_FORMAT_RGB_888;
		stTestParam.astChnParam[i].stAspectRatio.enMode = ASPECT_RATIO_NONE;
		stTestParam.astChnParam[i].stNormalize.bEnable = CVI_FALSE;
		snprintf(stTestParam.astChnParam[i].aszFileNameOut, 64, "%s/%s_chn%d_%d_%d_%s.bin",
			OUT_FILE_PREFIX, __func__, i,
			stTestParam.astChnParam[i].stSizeOut.u32Width,
			stTestParam.astChnParam[i].stSizeOut.u32Height,
			GetFmtName(stTestParam.astChnParam[i].enFormatOut));
	}

	strncpy(stTestParam.astChnParam[0].aszMD5Sum, MD5_1_TO_2_CHN0,
		sizeof(stTestParam.astChnParam[0].aszMD5Sum));
	strncpy(stTestParam.astChnParam[1].aszMD5Sum, MD5_1_TO_2_CHN1,
		sizeof(stTestParam.astChnParam[1].aszMD5Sum));
	stTestParam.astChnParam[0].u32CheckSum = 0x202f50f7;
	stTestParam.astChnParam[1].u32CheckSum = 0x4d474279;
	s32Ret = basic_mutli_chn(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_1_to_3(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i, s32ChnNum = 3;
	VPSS_MULTI_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));

	stTestParam.astChnParam[0].enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.astChnParam[1].enFormatOut = PIXEL_FORMAT_NV21;
	stTestParam.astChnParam[2].enFormatOut = PIXEL_FORMAT_RGB_888;
	stTestParam.astChnParam[0].stSizeOut.u32Width = DEFAULT_W;
	stTestParam.astChnParam[0].stSizeOut.u32Height = DEFAULT_H;
	stTestParam.astChnParam[1].stSizeOut.u32Width = 1280;
	stTestParam.astChnParam[1].stSizeOut.u32Height = 720;
	stTestParam.astChnParam[2].stSizeOut.u32Width = 640;
	stTestParam.astChnParam[2].stSizeOut.u32Height = 480;

	for (i = 0; i < s32ChnNum; i++) {
		stTestParam.astChnParam[i].bEnable = CVI_TRUE;
		stTestParam.astChnParam[i].VpssChn = i;
		stTestParam.astChnParam[i].bMirror = CVI_FALSE;
		stTestParam.astChnParam[i].bFlip = CVI_FALSE;
		stTestParam.astChnParam[i].stAspectRatio.enMode = ASPECT_RATIO_NONE;
		stTestParam.astChnParam[i].stNormalize.bEnable = CVI_FALSE;
		snprintf(stTestParam.astChnParam[i].aszFileNameOut, 64, "%s/%s_chn%d_%d_%d_%s.bin",
			OUT_FILE_PREFIX, __func__, i,
			stTestParam.astChnParam[i].stSizeOut.u32Width,
			stTestParam.astChnParam[i].stSizeOut.u32Height,
			GetFmtName(stTestParam.astChnParam[i].enFormatOut));
	}

	strncpy(stTestParam.astChnParam[0].aszMD5Sum, MD5_1_TO_3_CHN0,
		sizeof(stTestParam.astChnParam[0].aszMD5Sum));
	strncpy(stTestParam.astChnParam[1].aszMD5Sum, MD5_1_TO_3_CHN1,
		sizeof(stTestParam.astChnParam[1].aszMD5Sum));
	strncpy(stTestParam.astChnParam[2].aszMD5Sum, MD5_1_TO_3_CHN2,
		sizeof(stTestParam.astChnParam[2].aszMD5Sum));

	stTestParam.astChnParam[0].u32CheckSum = 0x66aa69de;
	stTestParam.astChnParam[1].u32CheckSum = 0xabb84d27;
	stTestParam.astChnParam[2].u32CheckSum = 0x9ea313ad;
	s32Ret = basic_mutli_chn(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_1_to_4(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i, s32ChnNum = 4;
	VPSS_MULTI_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));

	stTestParam.astChnParam[0].enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.astChnParam[1].enFormatOut = PIXEL_FORMAT_HSV_888;
	stTestParam.astChnParam[2].enFormatOut = PIXEL_FORMAT_YUV_400;
	stTestParam.astChnParam[3].enFormatOut = PIXEL_FORMAT_RGB_888;
	stTestParam.astChnParam[0].stSizeOut.u32Width = DEFAULT_W;
	stTestParam.astChnParam[0].stSizeOut.u32Height = DEFAULT_H;
	stTestParam.astChnParam[1].stSizeOut.u32Width = DEFAULT_W;
	stTestParam.astChnParam[1].stSizeOut.u32Height = DEFAULT_H;
	stTestParam.astChnParam[2].stSizeOut.u32Width = 1280;
	stTestParam.astChnParam[2].stSizeOut.u32Height = 720;
	stTestParam.astChnParam[3].stSizeOut.u32Width = 640;
	stTestParam.astChnParam[3].stSizeOut.u32Height = 480;

	for (i = 0; i < s32ChnNum; i++) {
		stTestParam.astChnParam[i].bEnable = CVI_TRUE;
		stTestParam.astChnParam[i].VpssChn = i;
		stTestParam.astChnParam[i].bMirror = CVI_FALSE;
		stTestParam.astChnParam[i].bFlip = CVI_FALSE;
		stTestParam.astChnParam[i].stAspectRatio.enMode = ASPECT_RATIO_NONE;
		stTestParam.astChnParam[i].stNormalize.bEnable = CVI_FALSE;
		snprintf(stTestParam.astChnParam[i].aszFileNameOut, 64, "%s/%s_chn%d_%d_%d_%s.bin",
			OUT_FILE_PREFIX, __func__, i,
			stTestParam.astChnParam[i].stSizeOut.u32Width,
			stTestParam.astChnParam[i].stSizeOut.u32Height,
			GetFmtName(stTestParam.astChnParam[i].enFormatOut));
	}

	strncpy(stTestParam.astChnParam[0].aszMD5Sum, MD5_1_TO_4_CHN0,
		sizeof(stTestParam.astChnParam[0].aszMD5Sum));
	strncpy(stTestParam.astChnParam[1].aszMD5Sum, MD5_1_TO_4_CHN1,
		sizeof(stTestParam.astChnParam[1].aszMD5Sum));
	strncpy(stTestParam.astChnParam[2].aszMD5Sum, MD5_1_TO_4_CHN2,
		sizeof(stTestParam.astChnParam[2].aszMD5Sum));
	strncpy(stTestParam.astChnParam[3].aszMD5Sum, MD5_1_TO_4_CHN3,
		sizeof(stTestParam.astChnParam[3].aszMD5Sum));
	stTestParam.astChnParam[0].u32CheckSum = 0x66aa69de;
	stTestParam.astChnParam[1].u32CheckSum = 0x93fb6e1a;
	stTestParam.astChnParam[2].u32CheckSum = 0xafde8336;
	stTestParam.astChnParam[3].u32CheckSum = 0x9ea313ad;
	s32Ret = basic_mutli_chn(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_limit_width(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;
	CVI_U32 u32Height = 2592;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = VPSS_HW_LIMIT_WIDTH;
	stTestParam.stSizeIn.u32Height = u32Height;
	stTestParam.stSizeOut.u32Width = VPSS_HW_LIMIT_WIDTH;
	stTestParam.stSizeOut.u32Height = u32Height;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_RGB_888;
	stTestParam.enFormatOut = PIXEL_FORMAT_RGB_888;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.u32CheckSum = 0x3507909a;
	strncpy(stTestParam.aszMD5Sum, MD5_LIMIT_WIDTH, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_LIMIT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_max_resolution(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = VPSS_MAX_IMAGE_WIDTH;
	stTestParam.stSizeIn.u32Height = VPSS_MAX_IMAGE_HEIGHT;
	stTestParam.stSizeOut.u32Width = VPSS_MAX_IMAGE_WIDTH;
	stTestParam.stSizeOut.u32Height = VPSS_MAX_IMAGE_HEIGHT;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_RGB_888;
	stTestParam.enFormatOut = PIXEL_FORMAT_RGB_888;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.u32CheckSum = 0x63b7be2c;
	strncpy(stTestParam.aszMD5Sum, MD5_MAX_RES, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_MAX_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_mirror(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = DEFAULT_W;
	stTestParam.stSizeOut.u32Height = DEFAULT_H;
	stTestParam.bMirror = CVI_TRUE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.u32CheckSum = 0xca233e75;
	strncpy(stTestParam.aszMD5Sum, MD5_MIRROR, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_flip(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = DEFAULT_W;
	stTestParam.stSizeOut.u32Height = DEFAULT_H;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_TRUE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.u32CheckSum = 0x6832c8c6;
	strncpy(stTestParam.aszMD5Sum, MD5_FLIP, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_mirror_flip(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = DEFAULT_W;
	stTestParam.stSizeOut.u32Height = DEFAULT_H;
	stTestParam.bMirror = CVI_TRUE;
	stTestParam.bFlip = CVI_TRUE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.u32CheckSum = 0x8a0831cd;
	strncpy(stTestParam.aszMD5Sum, MD5_MIRROR_FLIP, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_aspect_ratio(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = DEFAULT_W;
	stTestParam.stSizeOut.u32Height = DEFAULT_H;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_MANUAL;
	stTestParam.stAspectRatio.bEnableBgColor = CVI_TRUE;
	stTestParam.stAspectRatio.u32BgColor = 0;
	stTestParam.stAspectRatio.stVideoRect.s32X = 64;
	stTestParam.stAspectRatio.stVideoRect.s32Y = 64;
	stTestParam.stAspectRatio.stVideoRect.u32Width = 1280;
	stTestParam.stAspectRatio.stVideoRect.u32Height = 720;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.u32CheckSum = 0x1e28594b;
	strncpy(stTestParam.aszMD5Sum, MD5_ASPECT_RATIO1, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_1_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret |= basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	//x,y odd
	stTestParam.stAspectRatio.stVideoRect.s32X = 65;
	stTestParam.stAspectRatio.stVideoRect.s32Y = 31;
	stTestParam.stAspectRatio.stVideoRect.u32Width = 1280;
	stTestParam.stAspectRatio.stVideoRect.u32Height = 720;
	stTestParam.u32CheckSum = 0x30186f67;
	strncpy(stTestParam.aszMD5Sum, MD5_ASPECT_RATIO2, sizeof(stTestParam.aszMD5Sum));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_2_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret |= basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	//w,h odd
	stTestParam.stAspectRatio.stVideoRect.s32X = 64;
	stTestParam.stAspectRatio.stVideoRect.s32Y = 30;
	stTestParam.stAspectRatio.stVideoRect.u32Width = 1281;
	stTestParam.stAspectRatio.stVideoRect.u32Height = 721;
	stTestParam.u32CheckSum = 0xd6b83235;
	strncpy(stTestParam.aszMD5Sum, MD5_ASPECT_RATIO3, sizeof(stTestParam.aszMD5Sum));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_3_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret |= basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_multi_grp(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	SIZE_S stSizeIn = {DEFAULT_W, DEFAULT_H};
	SIZE_S stSizeOut = {640, 480};
	PIXEL_FORMAT_E enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	PIXEL_FORMAT_E enFormatOut = PIXEL_FORMAT_NV21;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	VIDEO_FRAME_INFO_S stVideoFrameIn;
	VIDEO_FRAME_INFO_S stVideoFrameOut;
	CVI_S32 s32GrpNum = 16;
	CVI_CHAR *pFileNameIn = VPSS_DEFAULT_FILE_IN;

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSizeIn = COMMON_GetPicBufferSize(stSizeIn.u32Width, stSizeIn.u32Height,
		enFormatIn, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSizeOut = COMMON_GetPicBufferSize(stSizeOut.u32Width, stSizeOut.u32Height,
		enFormatOut, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 2;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 1;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
	stVbConf.astCommPool[1].u32BlkCnt	= s32GrpNum;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	VPSS_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn);
	VPSS_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enFormatIn;
	stVpssGrpAttr.u32MaxW			     = stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSizeIn.u32Height;

	stVpssChnAttr.u32Width		    = stSizeOut.u32Width;
	stVpssChnAttr.u32Height		    = stSizeOut.u32Height;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enFormatOut;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip				= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode	= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable	= CVI_FALSE;

	for (VpssGrp = 0; VpssGrp < s32GrpNum; VpssGrp++) {
		s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
			goto exit1;
		}

		s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
			goto exit1;
		}

		s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
			goto exit1;
		}

		/*start vpss*/
		s32Ret = CVI_VPSS_StartGrp(VpssGrp);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
			goto exit1;
		}
	}

	//send frame
	s32Ret = FileToFrame(&stSizeIn, enFormatIn, pFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit1;
	}
	for (VpssGrp = 0; VpssGrp < s32GrpNum; VpssGrp++) {
		s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SendFrame fail.\n");
			CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
			goto exit1;
		}
	}
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));

	for (VpssGrp = 0; VpssGrp < s32GrpNum; VpssGrp++) {
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, UT_TIMEOUT_MS);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
			goto exit1;
		}
		VPSS_UT_PRT("***Grp(%d) Chn(%d), CVI_VPSS_GetChnFrame Success***\n", VpssGrp, VpssChn);

		s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_ReleaseChnFrame for Grp(%d) Chn(%d). s32Ret: 0x%x !\n",
				VpssGrp, VpssChn, s32Ret);
			goto exit1;
		}
	}

exit1:
	for (VpssGrp = 0; VpssGrp < s32GrpNum; VpssGrp++) {
		CVI_VPSS_StopGrp(VpssGrp);
		CVI_VPSS_DisableChn(VpssGrp, VpssChn);
		CVI_VPSS_DestroyGrp(VpssGrp);
	}
	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();

	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_VOID *multi_thread_run(CVI_VOID *arg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i, s32Repeat = TEST_CNT0, s32AvgFrameRate;
	VPSS_GRP VpssGrp;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VIDEO_FRAME_INFO_S stVideoFrameOut, stVideoFrameIn;
	PIXEL_FORMAT_E enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	PIXEL_FORMAT_E enFormatOut = PIXEL_FORMAT_NV21;
	SIZE_S stSizeIn = {DEFAULT_W, DEFAULT_H};
	CVI_CHAR *pFileNameIn = VPSS_DEFAULT_FILE_IN;
	CVI_U64 u64PTS, u64StartPTS, u64EndPTS, u64CurPTS1, u64CurPTS2, u64CostTime;
	CVI_U32 u32FrameCnt;
	CVI_U64 u64MinCostTime = 10000, u64MaxCostTime = 0;

	arg = arg;

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate	 = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate	 = -1;
	stVpssGrpAttr.enPixelFormat 		 = enFormatIn;
	stVpssGrpAttr.u32MaxW				 = stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH				 = stSizeIn.u32Height;

	stVpssChnAttr.u32Width			= DEFAULT_W;
	stVpssChnAttr.u32Height 		= DEFAULT_H;
	stVpssChnAttr.enVideoFormat 		= VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat 		= enFormatOut;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip 			= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable		= CVI_FALSE;


	VpssGrp = CVI_VPSS_GetAvailableGrp();
	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		return NULL;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit0;
	}

	s32Ret = CVI_VPSS_AttachVbPool(VpssGrp, VpssChn, 1);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_AttachVbPool failed with %#x\n", s32Ret);
		goto exit0;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit0;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit1;
	}

	//send frame
	s32Ret = FileToFrame(&stSizeIn, enFormatIn, pFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit2;
	}

	CVI_SYS_GetCurPTS(&u64PTS);
	u32FrameCnt = 0;
	u64StartPTS = u64PTS;

	for (i = 0; i < s32Repeat; i++) {
		CVI_SYS_GetCurPTS(&u64CurPTS1);
		s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SendFrame fail.\n");
			goto exit3;
		}

		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, UT_TIMEOUT_MS);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
			goto exit3;
		}
		CVI_SYS_GetCurPTS(&u64CurPTS2);

		s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_ReleaseChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
			goto exit3;
		}

		u32FrameCnt++;
		u64CostTime = u64CurPTS2 - u64CurPTS1;
		u64MinCostTime = u64CostTime < u64MinCostTime ? u64CostTime : u64MinCostTime;
		u64MaxCostTime = u64CostTime > u64MaxCostTime ? u64CostTime : u64MaxCostTime;

		if ((u64CurPTS2 - u64PTS) >= 1000000) {
			VPSS_UT_PRT("[VpssGrp%d] FrameRate:%d fps\n", VpssGrp, u32FrameCnt);
			u32FrameCnt = 0;
			u64PTS = u64CurPTS2;
		}

	}
	CVI_SYS_GetCurPTS(&u64EndPTS);
	s32AvgFrameRate = s32Repeat / ((u64EndPTS - u64StartPTS) / 1000000);
#if 0
	if ((u64MaxCostTime - u64MinCostTime) > 4000)
		s32Ret = -1;
	if (s32AvgFrameRate < (2400/THREAD_CNT - 5))
		s32Ret = -1;
#endif
	VPSS_UT_PRT("\n[VpssGrp%d] 1080P cost time: Min-Max: (%ld, %ld)us, offset=%ld\n", VpssGrp,
		u64MinCostTime, u64MaxCostTime, u64MaxCostTime - u64MinCostTime);
	VPSS_UT_PRT("\n[VpssGrp%d] average FrameRate:%d fps\n", VpssGrp, s32AvgFrameRate);

exit3:
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
exit2:
	CVI_VPSS_StopGrp(VpssGrp);
exit1:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit0:
	CVI_VPSS_DestroyGrp(VpssGrp);

	if (s32Ret == CVI_SUCCESS) {
		pthread_mutex_lock(&s_SyncMutex);
		s_u32Flag |= BIT(VpssGrp);
		pthread_mutex_unlock(&s_SyncMutex);
	}

	return NULL;
}

static CVI_S32 vpss_test_multi_thread(CVI_VOID)
{
	CVI_S32 i, s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	CVI_S32 s32ThreadNum = THREAD_CNT;
	pthread_t thread[THREAD_CNT] = {[0 ... THREAD_CNT - 1] = -1};

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSizeIn = COMMON_GetPicBufferSize(DEFAULT_W, DEFAULT_H, PIXEL_FORMAT_YUV_PLANAR_420,
		DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSizeOut = COMMON_GetPicBufferSize(DEFAULT_W, DEFAULT_H, PIXEL_FORMAT_NV21,
		DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt				= 2;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= s32ThreadNum;
	stVbConf.astCommPool[0].enRemapMode = VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
	stVbConf.astCommPool[1].u32BlkCnt	= s32ThreadNum;
	stVbConf.astCommPool[1].enRemapMode = VB_REMAP_MODE_CACHED;
	VPSS_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn);
	VPSS_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	s_u32Flag = 0;
	//system("echo 0x3ff > /sys/module/cv186x_vpss/parameters/work_mask");

	for (i = 0; i < s32ThreadNum; i++) {
		s32Ret = pthread_create(&thread[i], NULL, multi_thread_run, NULL);
		if (s32Ret < 0) {
			VPSS_UT_PRT("pthread_create fail. s32Ret: 0x%x !\n", s32Ret);
			break;
		}
	}

	for (i = 0; i < s32ThreadNum; i++)
		if (thread[i] > 0)
			pthread_join(thread[i], NULL);

	//system("echo 0xff > /sys/module/cv186x_vpss/parameters/work_mask");
	sleep(1);

	for (i = 0; i < s32ThreadNum; i++)
		if (!(s_u32Flag & BIT(i)))
			s32Ret = CVI_FAILURE;

	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();

	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_byte_align(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = 642;
	stTestParam.stSizeOut.u32Height = 480;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_NV21;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.u32ChnAlign = 1;
	stTestParam.u32CheckSum = 0x6f16a1b1;
	strncpy(stTestParam.aszMD5Sum, MD5_BYTE_ALIGN, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_resize(CVI_VOID)
{
	CVI_S32 i, s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	VIDEO_FRAME_INFO_S stVideoFrameIn, stVideoFrameOut;
	SIZE_S stSizeIn = {1920, 1080};
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	CVI_CHAR *pstFileNameIn = VPSS_DEFAULT_FILE_IN;
	CVI_S32 s32Min = 16;
	CVI_S32 s32Max = 8192;
#ifndef FPGA_PORTING
	CVI_S32 s32Step = 32;
#else
	CVI_S32 s32Step = 256;
#endif
	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSizeIn = COMMON_GetPicBufferSize(stSizeIn.u32Width, stSizeIn.u32Height,
		enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSizeOut = COMMON_GetPicBufferSize(8192, 8192,
		enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 2;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 1;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
	stVbConf.astCommPool[1].u32BlkCnt	= 1;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	VPSS_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn);
	VPSS_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enPixelFormat;
	stVpssGrpAttr.u32MaxW			     = stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSizeIn.u32Height;

	stVpssChnAttr.u32Width		    = stSizeIn.u32Width;
	stVpssChnAttr.u32Height		    = stSizeIn.u32Height;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enPixelFormat;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip				= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable		= CVI_FALSE;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_AttachVbPool(VpssGrp, VpssChn, 1);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_AttachVbPool failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//send frame
	s32Ret = FileToFrame(&stSizeIn, enPixelFormat, pstFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	for (i = s32Min; i <= s32Max; i += s32Step) {
		stVpssChnAttr.u32Width = i;
		stVpssChnAttr.u32Height = i;
		s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
			goto exit5;
		}
		s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SendFrame fail.\n");
			goto exit5;
		}
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, UT_TIMEOUT_MS);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
			VPSS_UT_PRT("output: w=%d h=%d fail\n", i, i);
			goto exit5;
		}
		s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_ReleaseChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
			goto exit5;
		}
	}

exit5:
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();

	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_max_scaling(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	VIDEO_FRAME_INFO_S stVideoFrame1, stVideoFrame2, stVideoFrame3;
	SIZE_S stSizeIn = {1920, 1080};
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	CVI_CHAR *pstFileNameIn = VPSS_DEFAULT_FILE_IN;
	CVI_S32 s32Max = 4608;
	CVI_S32 s32ScalingRatio = 128;
	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSizeIn = COMMON_GetPicBufferSize(stSizeIn.u32Width, stSizeIn.u32Height,
		enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSizeOut = COMMON_GetPicBufferSize(s32Max, s32Max,
		enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 2;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 1;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
	stVbConf.astCommPool[1].u32BlkCnt	= 2;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	VPSS_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn);
	VPSS_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enPixelFormat;
	stVpssGrpAttr.u32MaxW			     = stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSizeIn.u32Height;

	stVpssChnAttr.u32Width		    = s32Max;
	stVpssChnAttr.u32Height		    = s32Max;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enPixelFormat;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip				= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable		= CVI_FALSE;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_AttachVbPool(VpssGrp, VpssChn, 1);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_AttachVbPool failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//send frame
	s32Ret = FileSendToVpss(VpssGrp, &stSizeIn, enPixelFormat, pstFileNameIn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("FileSendToVpss fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame1, UT_TIMEOUT_MS);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	//1/128 scaling down
	stVpssGrpAttr.u32MaxW = s32Max;
	stVpssGrpAttr.u32MaxH = s32Max;
	s32Ret = CVI_VPSS_SetGrpAttr(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SetGrpAttr(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame1);
		goto exit4;
	}
	stVpssChnAttr.u32Width = s32Max / s32ScalingRatio;
	stVpssChnAttr.u32Height = s32Max / s32ScalingRatio;
	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame1);
		goto exit4;
	}
	s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrame1, 1000);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SendFrame fail.\n");
		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame1);
		goto exit4;
	}
	CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame1);

	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame2, UT_TIMEOUT_MS);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}
	VPSS_UT_PRT("128 scaling down successful.\n");

	//128 scaling up
	stVpssGrpAttr.u32MaxW = s32Max / s32ScalingRatio;
	stVpssGrpAttr.u32MaxH = s32Max / s32ScalingRatio;
	s32Ret = CVI_VPSS_SetGrpAttr(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SetGrpAttr(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame2);
		goto exit4;
	}
	stVpssChnAttr.u32Width = s32Max;
	stVpssChnAttr.u32Height = s32Max;
	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame2);
		goto exit4;
	}
	s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrame2, 1000);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SendFrame fail.\n");
		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame2);
		goto exit4;
	}
	CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame2);

	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame3, UT_TIMEOUT_MS);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}
	s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame3);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_ReleaseChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}
	VPSS_UT_PRT("128 scaling up successful.\n");

exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();

	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_tile_mode(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 i;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn;
	VIDEO_FRAME_INFO_S stVideoFrameIn, stVideoFrameOut;
	SIZE_S stSizeIn = {5000, 2000};
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	CVI_CHAR *pstFileNameIn = VPSS_TILE_MODE_FILE_IN;
	CVI_CHAR aszFileNameOut[64];

	snprintf(aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stSizeIn.u32Width,
		stSizeIn.u32Height,
		GetFmtName(enPixelFormat));

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSizeIn = COMMON_GetPicBufferSize(stSizeIn.u32Width, stSizeIn.u32Height,
		enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 1;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 2;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	VPSS_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enPixelFormat;
	stVpssGrpAttr.u32MaxW			     = stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSizeIn.u32Height;

	stVpssChnAttr.u32Width		    = stSizeIn.u32Width;
	stVpssChnAttr.u32Height		    = stSizeIn.u32Height;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enPixelFormat;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip				= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable		= CVI_FALSE;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//send frame
	s32Ret = FileToFrame(&stSizeIn, enPixelFormat, pstFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SendFrame fail.\n");
		goto exit5;
	}

	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, UT_TIMEOUT_MS);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
	}
	VPSS_UT_PRT("***CVI_VPSS_GetChnFrame Success, checksum(%x)***\n", stVideoFrameOut.stVFrame.u32FrameFlag);

	s32Ret = FrameSaveToFile(aszFileNameOut, &stVideoFrameOut);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("FrameSaveToFile. s32Ret: 0x%x !\n", s32Ret);
		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
		goto exit5;
	}
	printf("output file:%s\n", aszFileNameOut);

	s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_ReleaseChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
	}

	VPSS_CROP_INFO_S stCropInfo;

	stCropInfo.bEnable = CVI_TRUE;
	stCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
	stCropInfo.stCropRect.s32Y = 10;
	stCropInfo.stCropRect.u32Width = 640;
	stCropInfo.stCropRect.u32Height = 480;

	stVpssChnAttr.u32Width = 1280;
	stVpssChnAttr.u32Height = 720;
	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit5;
	}

	for (i = stSizeIn.u32Width / 2 - stCropInfo.stCropRect.u32Width - 64;
		i < (stSizeIn.u32Width / 2 + 64);
		i += 8) {
		stCropInfo.stCropRect.s32X = i;
		s32Ret = CVI_VPSS_SetChnCrop(VpssGrp, VpssChn, &stCropInfo);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetChnCrop failed with %#x\n", s32Ret);
			goto exit5;
		}
		s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SendFrame fail.\n");
			goto exit5;
		}
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, UT_TIMEOUT_MS);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
			VPSS_UT_PRT("crop(%d %d %d %d) fail\n", stCropInfo.stCropRect.s32X,
				stCropInfo.stCropRect.s32Y,
				stCropInfo.stCropRect.u32Width,
				stCropInfo.stCropRect.u32Height);
			goto exit5;
		}
		s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_ReleaseChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
			goto exit5;
		}
	}

	srand((unsigned)time(NULL));

	for (i = 0; i < TEST_CNT1; i++) {
		stCropInfo.stCropRect.s32X = rand() % stSizeIn.u32Width;
		stCropInfo.stCropRect.u32Width = (rand() % stSizeIn.u32Width + 4) & (~0x1);
		if (((stCropInfo.stCropRect.s32X + stCropInfo.stCropRect.u32Width) >
			stSizeIn.u32Width) || (stCropInfo.stCropRect.u32Width < VPSS_MIN_IMAGE_WIDTH)) {
			i--;
			continue;
		}

		stVpssChnAttr.u32Width = stCropInfo.stCropRect.u32Width;
		s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
			goto exit5;
		}
		s32Ret = CVI_VPSS_SetChnCrop(VpssGrp, VpssChn, &stCropInfo);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetChnCrop failed with %#x\n", s32Ret);
			goto exit5;
		}
		s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SendFrame fail.\n");
			goto exit5;
		}
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, UT_TIMEOUT_MS);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
			VPSS_UT_PRT("crop(%d %d %d %d)\n", stCropInfo.stCropRect.s32X,
				stCropInfo.stCropRect.s32Y,
				stCropInfo.stCropRect.u32Width,
				stCropInfo.stCropRect.u32Height);
			goto exit5;
		}
		s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_ReleaseChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
			goto exit5;
		}
	}

exit5:
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();

	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_draw_rect(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = DEFAULT_W;
	stTestParam.stSizeOut.u32Height = DEFAULT_H;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_NV21;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.stDrawRect.astRect[0].bEnable = CVI_TRUE;
	stTestParam.stDrawRect.astRect[0].u32BgColor = 0xffff;
	stTestParam.stDrawRect.astRect[0].u16Thick = 6;
	stTestParam.stDrawRect.astRect[0].stRect.s32X = 96;
	stTestParam.stDrawRect.astRect[0].stRect.s32Y = 96;
	stTestParam.stDrawRect.astRect[0].stRect.u32Width = 308;
	stTestParam.stDrawRect.astRect[0].stRect.u32Height = 208;
	for (int i = 1; i < VPSS_RECT_NUM; i++) {
		memcpy(&stTestParam.stDrawRect.astRect[i], &stTestParam.stDrawRect.astRect[0],
			sizeof(stTestParam.stDrawRect.astRect[0]));
		stTestParam.stDrawRect.astRect[i].stRect.s32X += i * 200;
		stTestParam.stDrawRect.astRect[i].stRect.s32Y += i * 64;
	}
	stTestParam.u32CheckSum = 0x143f04f7;
	strncpy(stTestParam.aszMD5Sum, MD5_DRAW_RECT, sizeof(stTestParam.aszMD5Sum));

	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_format(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 i, j;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	VIDEO_FRAME_INFO_S stVideoFrameIn, stVideoFrameOut0, stVideoFrameOut1;
	SIZE_S stSize = {1920, 1080};
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	CVI_CHAR *pstFileNameIn = VPSS_DEFAULT_FILE_IN;
	CVI_U32 randomIndex = 0;
	PIXEL_FORMAT_E fmt_in[] = {
		PIXEL_FORMAT_RGB_888,
		PIXEL_FORMAT_BGR_888,
		PIXEL_FORMAT_NV12,
		PIXEL_FORMAT_RGB_888_PLANAR,
		PIXEL_FORMAT_NV21,
		PIXEL_FORMAT_BGR_888_PLANAR,
		PIXEL_FORMAT_NV16,
		PIXEL_FORMAT_NV61,
		PIXEL_FORMAT_YUYV,
		PIXEL_FORMAT_YVYU,
		PIXEL_FORMAT_UYVY,
		PIXEL_FORMAT_VYUY,
		PIXEL_FORMAT_YUV_444,
		PIXEL_FORMAT_YUV_PLANAR_420,
		PIXEL_FORMAT_YUV_PLANAR_422,
		PIXEL_FORMAT_YUV_PLANAR_444,
		PIXEL_FORMAT_YUV_400,
	};
	PIXEL_FORMAT_E fmt_out[] = {
		PIXEL_FORMAT_RGB_888,
		PIXEL_FORMAT_BGR_888,
		PIXEL_FORMAT_NV12,
		PIXEL_FORMAT_RGB_888_PLANAR,
		PIXEL_FORMAT_NV21,
		PIXEL_FORMAT_BGR_888_PLANAR,
		PIXEL_FORMAT_NV16,
		PIXEL_FORMAT_NV61,
		PIXEL_FORMAT_YUYV,
		PIXEL_FORMAT_YVYU,
		PIXEL_FORMAT_UYVY,
		PIXEL_FORMAT_VYUY,
		PIXEL_FORMAT_YUV_444,
		PIXEL_FORMAT_YUV_PLANAR_420,
		PIXEL_FORMAT_YUV_PLANAR_422,
		PIXEL_FORMAT_YUV_PLANAR_444,
		PIXEL_FORMAT_YUV_400,
		PIXEL_FORMAT_HSV_888,
		PIXEL_FORMAT_HSV_888_PLANAR,
		PIXEL_FORMAT_FP32_C3_PLANAR,
		PIXEL_FORMAT_FP16_C3_PLANAR,
		PIXEL_FORMAT_BF16_C3_PLANAR,
		PIXEL_FORMAT_INT8_C3_PLANAR,
		PIXEL_FORMAT_UINT8_C3_PLANAR
	};

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSizeIn = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height,
		PIXEL_FORMAT_RGB_888_PLANAR, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSizeOut = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height,
		PIXEL_FORMAT_FP32_C3_PLANAR, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 2;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 1;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
	stVbConf.astCommPool[1].u32BlkCnt	= 2;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	VPSS_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn);
	VPSS_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enPixelFormat;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;

	stVpssChnAttr.u32Width		    = stSize.u32Width;
	stVpssChnAttr.u32Height		    = stSize.u32Height;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enPixelFormat;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip				= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable		= CVI_FALSE;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_AttachVbPool(VpssGrp, VpssChn, 1);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_AttachVbPool failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//send frame
	s32Ret = FileToFrame(&stSize, enPixelFormat, pstFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	for (i = 0; i < ARRAY_SIZE(fmt_in); i++) {
		stVpssGrpAttr.enPixelFormat = enPixelFormat;
		s32Ret = CVI_VPSS_SetGrpAttr(VpssGrp, &stVpssGrpAttr);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetGrpAttr failed with %#x\n", s32Ret);
			goto exit5;
		}
		stVpssChnAttr.enPixelFormat = fmt_in[i];
		s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
			goto exit5;
		}
		s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SendFrame fail.\n");
			goto exit5;
		}
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut0, UT_TIMEOUT_MS);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
			VPSS_UT_PRT("output fmt: %d\n", i);
			goto exit5;
		}

		stVpssGrpAttr.enPixelFormat = fmt_in[i];
		s32Ret = CVI_VPSS_SetGrpAttr(VpssGrp, &stVpssGrpAttr);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetGrpAttr failed with %#x\n", s32Ret);
			CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut0);
			goto exit5;
		}
		for (j = 0; j < ARRAY_SIZE(fmt_out); j++) {
			stVpssChnAttr.enPixelFormat = fmt_out[j];
			s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
			if (s32Ret != CVI_SUCCESS) {
				VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
				CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut0);
				goto exit5;
			}
			s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameOut0, 1000);
			if (s32Ret != CVI_SUCCESS) {
				VPSS_UT_PRT("CVI_VPSS_SendFrame fail.\n");
				CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut0);
				goto exit5;
			}
			s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut1, UT_TIMEOUT_MS);
			if (s32Ret != CVI_SUCCESS) {
				VPSS_UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
				VPSS_UT_PRT("output fmt: %d\n", i);
				CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut0);
				goto exit5;
			}
			s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut1);
			if (s32Ret != CVI_SUCCESS) {
				VPSS_UT_PRT("CVI_VPSS_ReleaseChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
				CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut0);
				goto exit5;
			}
		}

		s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut0);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_ReleaseChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
			goto exit5;
		}
	}

	srand(time(NULL));
	for (i = 0; i < 100; ++i) {
		stVpssGrpAttr.enPixelFormat = enPixelFormat;
		s32Ret = CVI_VPSS_SetGrpAttr(VpssGrp, &stVpssGrpAttr);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetGrpAttr failed with %#x\n", s32Ret);
			goto exit5;
		}
		randomIndex = rand() % ARRAY_SIZE(fmt_in);
		stVpssChnAttr.enPixelFormat = fmt_in[randomIndex];
		s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
			goto exit5;
		}
		s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SendFrame fail.\n");
			goto exit5;
		}
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut0, UT_TIMEOUT_MS);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
			VPSS_UT_PRT("output fmt: %d\n", i);
			goto exit5;
		}

		stVpssGrpAttr.enPixelFormat = fmt_in[randomIndex];
		s32Ret = CVI_VPSS_SetGrpAttr(VpssGrp, &stVpssGrpAttr);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetGrpAttr failed with %#x\n", s32Ret);
			CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut0);
			goto exit5;
		}

		for (j = 0; j < 100; j++) {
			randomIndex = rand() % ARRAY_SIZE(fmt_out);
			stVpssChnAttr.enPixelFormat = fmt_out[randomIndex];
			s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
			if (s32Ret != CVI_SUCCESS) {
				VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
				CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut0);
				goto exit5;
			}
			s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameOut0, 1000);
			if (s32Ret != CVI_SUCCESS) {
				VPSS_UT_PRT("CVI_VPSS_SendFrame fail.\n");
				CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut0);
				goto exit5;
			}
			s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut1, UT_TIMEOUT_MS);
			if (s32Ret != CVI_SUCCESS) {
				VPSS_UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
				VPSS_UT_PRT("output fmt: %d\n", i);
				CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut0);
				goto exit5;
			}
			s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut1);
			if (s32Ret != CVI_SUCCESS) {
				VPSS_UT_PRT("CVI_VPSS_ReleaseChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
				CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut0);
				goto exit5;
			}
		}

		s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut0);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_ReleaseChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
			goto exit5;
		}
	}

exit5:
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();

	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 test_crop(CVI_BOOL isChn)
{
	CVI_S32 i, s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	VIDEO_FRAME_INFO_S stVideoFrameIn, stVideoFrameOut;
	SIZE_S stSizeIn = {1920, 1080};
	SIZE_S stSizeOut = {640, 480};
	PIXEL_FORMAT_E enPixelFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	PIXEL_FORMAT_E enPixelFormatOut = PIXEL_FORMAT_RGB_888;
	CVI_CHAR *pstFileNameIn = VPSS_DEFAULT_FILE_IN;
	VPSS_CROP_INFO_S stCropInfo;

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSizeIn = COMMON_GetPicBufferSize(stSizeIn.u32Width, stSizeIn.u32Height,
		enPixelFormatIn, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSizeOut = COMMON_GetPicBufferSize(stSizeOut.u32Width, stSizeOut.u32Height,
		enPixelFormatOut, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 2;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 1;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
	stVbConf.astCommPool[1].u32BlkCnt	= 1;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	VPSS_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn);
	VPSS_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enPixelFormatIn;
	stVpssGrpAttr.u32MaxW			     = stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSizeIn.u32Height;

	stVpssChnAttr.u32Width		    = stSizeOut.u32Width;
	stVpssChnAttr.u32Height		    = stSizeOut.u32Height;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enPixelFormatOut;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip				= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable		= CVI_FALSE;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_AttachVbPool(VpssGrp, VpssChn, 1);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_AttachVbPool failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//send frame
	s32Ret = FileToFrame(&stSizeIn, enPixelFormatIn, pstFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	stCropInfo.bEnable = CVI_TRUE;
	stCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
	srand((unsigned)time(NULL));

	for (i = 0; i <= 50; i++) {
		stCropInfo.stCropRect.s32X = RANDOM(0, stSizeIn.u32Width - 16);
		stCropInfo.stCropRect.s32Y = RANDOM(0, stSizeIn.u32Height - 16);
		stCropInfo.stCropRect.u32Width = RANDOM(16, stSizeIn.u32Width) & ~(0x1);
		stCropInfo.stCropRect.u32Height = RANDOM(16, stSizeIn.u32Height) & ~(0x1);
		if (((stCropInfo.stCropRect.s32X + stCropInfo.stCropRect.u32Width)
			> stSizeIn.u32Width)
			|| ((stCropInfo.stCropRect.s32Y + stCropInfo.stCropRect.u32Height)
			> stSizeIn.u32Height)) {
			i--;
			continue;
		}

		if (isChn)
			s32Ret = CVI_VPSS_SetChnCrop(VpssGrp, VpssChn, &stCropInfo);
		else
			s32Ret = CVI_VPSS_SetGrpCrop(VpssGrp, &stCropInfo);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
			goto exit5;
		}
		s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SendFrame fail.\n");
			goto exit5;
		}
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, UT_TIMEOUT_MS);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
			VPSS_UT_PRT("crop fail,x=%d y=%d w=%d h=%d\n",
				stCropInfo.stCropRect.s32X, stCropInfo.stCropRect.s32Y,
				stCropInfo.stCropRect.u32Width, stCropInfo.stCropRect.u32Height);
			goto exit5;
		}
		s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_ReleaseChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
			goto exit5;
		}
	}

exit5:
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();

	return s32Ret;
}

static CVI_S32 vpss_test_grp_crop(CVI_VOID)
{
	CVI_S32 s32Ret = test_crop(CVI_FALSE);

	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_chn_crop(CVI_VOID)
{
	CVI_S32 s32Ret = test_crop(CVI_TRUE);

	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_amp_ctrl(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	VIDEO_FRAME_INFO_S stVideoFrameIn, stVideoFrameOut;
	SIZE_S stSize = {1920, 1080};
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	CVI_CHAR *pstFileNameIn = VPSS_DEFAULT_FILE_IN;

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height,
		enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	stVbConf.u32MaxPoolCnt              = 1;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 2;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	VPSS_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSize);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enPixelFormat;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;

	stVpssChnAttr.u32Width		    = stSize.u32Width;
	stVpssChnAttr.u32Height		    = stSize.u32Height;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enPixelFormat;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip				= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable		= CVI_FALSE;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//send frame
	s32Ret = FileToFrame(&stSize, enPixelFormat, pstFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	PROC_AMP_E type;
	CVI_S32 value = 80;
	PROC_AMP_CTRL_S ctrl;
	CVI_CHAR FileName[64];
	CVI_CHAR *pSuffix[PROC_AMP_MAX] = {"brightness", "contrast", "saturation", "hue"};
	CVI_CHAR *pszMd5[PROC_AMP_MAX] = {MD5_AMP_BRIGHTNESS, MD5_AMP_CONTRAST,
		MD5_AMP_SATURATION, MD5_AMP_HUE};
	//CVI_U32 au32CheckSum[PROC_AMP_MAX] = {0x596d1930, 0xc7b3f358, 0x9b199b91, 0xf85907af};


	for (type = PROC_AMP_BRIGHTNESS; type < PROC_AMP_MAX; type++) {
		CVI_VPSS_GetGrpProcAmpCtrl(VpssGrp, type, &ctrl);
		if ((ctrl.minimum != 0) || (ctrl.maximum != 100)
			|| (ctrl.step != 1) || (ctrl.default_value != 50)) {
			VPSS_UT_PRT("CVI_VPSS_GetGrpProcAmpCtrl fail!\n");
			s32Ret = CVI_FAILURE;
			goto exit5;
		}
		s32Ret = CVI_VPSS_SetGrpProcAmp(VpssGrp, type, value);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetGrpProcAmp failed with %#x\n", s32Ret);
			goto exit5;
		}
		s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SendFrame fail.\n");
			goto exit5;
		}
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, UT_TIMEOUT_MS);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
			goto exit5;
		}

		VPSS_UT_PRT("***CVI_VPSS_GetChnFrame Success***\n");


		if (CompareWithMD5(pszMd5[type], &stVideoFrameOut)) {
			snprintf(FileName, 64, "%s/%s_%s_%d_%d_%s.bin",
				OUT_FILE_PREFIX, __func__, pSuffix[type],
				stSize.u32Width,
				stSize.u32Height,
				GetFmtName(enPixelFormat));
			FrameSaveToFile(FileName, &stVideoFrameOut);
			s32Ret = CVI_FAILURE;
			VPSS_UT_PRT("Compare MD5 fail, MD5:%s\n", pszMd5[type]);
			VPSS_UT_PRT("output file:%s\n", FileName);
		}

		CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
		CVI_VPSS_SetGrpProcAmp(VpssGrp, type, ctrl.default_value);

		//if (s32Ret)
		//	break;
	}

exit5:
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();

	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

//normalize: x*factor - mean
static CVI_S32 vpss_test_normalize(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = DEFAULT_W;
	stTestParam.stSizeOut.u32Height = DEFAULT_H;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_RGB_888;
	stTestParam.enFormatOut = PIXEL_FORMAT_RGB_888;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_TRUE;
	stTestParam.stNormalize.rounding = VPSS_ROUNDING_TO_EVEN;
	stTestParam.stNormalize.factor[0] = 0.5;
	stTestParam.stNormalize.factor[1] = 0.5;
	stTestParam.stNormalize.factor[2] = 0.5;
	stTestParam.stNormalize.mean[0] = 10;
	stTestParam.stNormalize.mean[1] = 10;
	stTestParam.stNormalize.mean[2] = 10;
	stTestParam.u32CheckSum = 0x891e2d0b;
	strncpy(stTestParam.aszMD5Sum, MD5_NORMALIZE, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_RGB_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

//convert: ax + b
static CVI_S32 vpss_test_convert(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = DEFAULT_W;
	stTestParam.stSizeOut.u32Height = DEFAULT_H;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_RGB_888;
	stTestParam.enFormatOut = PIXEL_FORMAT_UINT8_C3_PLANAR;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.stConvert.bEnable = CVI_TRUE;
	stTestParam.stConvert.u32aFactor[0] = 2 * 8192;
	stTestParam.stConvert.u32aFactor[1] = 2 * 8192;
	stTestParam.stConvert.u32aFactor[2] = 2 * 8192;
	stTestParam.stConvert.u32bFactor[0] = 5 * 8192;
	stTestParam.stConvert.u32bFactor[1] = 5 * 8192;
	stTestParam.stConvert.u32bFactor[2] = 5 * 8192;
	stTestParam.u32CheckSum = 0xda899972;
	strncpy(stTestParam.aszMD5Sum, MD5_CONVERT, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_RGB_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_scale_coef(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = 1280;
	stTestParam.stSizeOut.u32Height = 720;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.enCoef = VPSS_SCALE_COEF_BICUBIC;
	stTestParam.u32CheckSum = 0xc3cf194b;
	strncpy(stTestParam.aszMD5Sum, MD5_SCALE_COEF1, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s_bicubic.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret |= basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	//bilinear
	stTestParam.enCoef = VPSS_SCALE_COEF_BILINEAR;
	stTestParam.u32CheckSum = 0x5583d343;
	strncpy(stTestParam.aszMD5Sum, MD5_SCALE_COEF2, sizeof(stTestParam.aszMD5Sum));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s_bilinear.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret |= basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	//nearest
	stTestParam.enCoef = VPSS_SCALE_COEF_NEAREST;
	stTestParam.u32CheckSum = 0x39768945;
	strncpy(stTestParam.aszMD5Sum, MD5_SCALE_COEF3, sizeof(stTestParam.aszMD5Sum));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s_nearest.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret |= basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	//opencv bicubic
	stTestParam.enCoef = VPSS_SCALE_COEF_BICUBIC_OPENCV;
	stTestParam.u32CheckSum = 0xbdc97502;
	strncpy(stTestParam.aszMD5Sum, MD5_SCALE_COEF4, sizeof(stTestParam.aszMD5Sum));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s_bicubic_cv.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret |= basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_y_ratio(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = DEFAULT_W;
	stTestParam.stSizeOut.u32Height = DEFAULT_H;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.YRatio = 0.5;
	stTestParam.u32CheckSum = 0x4e5ca43c;
	strncpy(stTestParam.aszMD5Sum, MD5_Y_RATIO, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret |= basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_hide(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = DEFAULT_W;
	stTestParam.stSizeOut.u32Height = DEFAULT_H;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.bHide = CVI_TRUE;
	stTestParam.u32CheckSum = 0x3c3a4000;
	strncpy(stTestParam.aszMD5Sum, MD5_HIDE, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_rotation(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = ALIGN(DEFAULT_W, DEFAULT_ALIGN);
	stTestParam.stSizeOut.u32Height = ALIGN(DEFAULT_H, DEFAULT_ALIGN);
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_NV21;
	stTestParam.enFormatOut = PIXEL_FORMAT_NV21;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.enRotation = ROTATION_90;
	strncpy(stTestParam.aszFileNameIn, VPSS_ROT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_ldc(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;
	VPSS_LDC_ATTR_S stLDCAttr= {CVI_TRUE, {CVI_TRUE, 0, 0, 0, 0, 0, -200, {0}, 0}};

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = DEFAULT_W;
	stTestParam.stSizeOut.u32Height = DEFAULT_H;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;

	memcpy(&stTestParam.stLDCAttr, &stLDCAttr, sizeof(stLDCAttr));
	strncpy(stTestParam.aszFileNameIn, VPSS_DWA_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_fisheye(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;
	FISHEYE_ATTR_S stFisheyeAttr;
	stFisheyeAttr.bEnable = CVI_TRUE;
	stFisheyeAttr.bBgColor = CVI_TRUE;
	stFisheyeAttr.u32BgColor = YUV_8BIT(0, 128, 128);
	stFisheyeAttr.s32HorOffset = 1280 / 2;
	stFisheyeAttr.s32VerOffset = 720 / 2;
	stFisheyeAttr.enMountMode = FISHEYE_DESKTOP_MOUNT;
	stFisheyeAttr.enUseMode = MODE_PANORAMA_360;
	stFisheyeAttr.u32RegionNum = 1;

	stFisheyeAttr.u32TrapezoidCoef	  = 2;
	stFisheyeAttr.s32FanStrength	  = 0;

	stFisheyeAttr.astFishEyeRegionAttr[0].enViewMode	    = FISHEYE_VIEW_NORMAL;
	stFisheyeAttr.astFishEyeRegionAttr[0].u32InRadius	    = 0;
	stFisheyeAttr.astFishEyeRegionAttr[0].u32OutRadius
		= MIN2(stFisheyeAttr.s32HorOffset, stFisheyeAttr.s32VerOffset);
	stFisheyeAttr.astFishEyeRegionAttr[0].u32Pan		    = 180;
	stFisheyeAttr.astFishEyeRegionAttr[0].u32Tilt		    = 20;
	stFisheyeAttr.astFishEyeRegionAttr[0].u32HorZoom	    = 2048;
	stFisheyeAttr.astFishEyeRegionAttr[0].u32VerZoom	    = 2048;
	stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.s32X	    = 0;
	stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.s32Y	    = 0;
	stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.u32Width    = 1280;
	stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.u32Height   = 720;


	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = 1024;
	stTestParam.stSizeIn.u32Height = 1024;
	stTestParam.stSizeOut.u32Width = 1280;
	stTestParam.stSizeOut.u32Height = 720;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;

	memcpy(&stTestParam.stFishEyeAttr, &stFisheyeAttr, sizeof(stFisheyeAttr));
	strncpy(stTestParam.aszFileNameIn, DWA_FILE_IN_FISHEYE, sizeof(stTestParam.aszFileNameIn));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_VOID *pressure_thread_run(CVI_VOID *arg)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 i, j, s32Repeat = TEST_CNT0;
	VPSS_GRP VpssGrp;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VIDEO_FRAME_INFO_S stVideoFrameOut, stVideoFrameIn;
	PIXEL_FORMAT_E enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	PIXEL_FORMAT_E enFormatOut = PIXEL_FORMAT_NV21;
	SIZE_S stSizeIn = {DEFAULT_W, DEFAULT_H};
	SIZE_S astSizeOut[VPSS_MAX_CHN_NUM] = {{DEFAULT_W, DEFAULT_H}, {1280, 720}, {640, 360}, {320, 180}};
	CVI_BOOL abChnEnable[VPSS_MAX_CHN_NUM];
	CVI_CHAR *pFileNameIn = VPSS_DEFAULT_FILE_IN;
	CVI_U32 u32ChnMask;

	arg = arg;

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate	 = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate	 = -1;
	stVpssGrpAttr.enPixelFormat 		 = enFormatIn;
	stVpssGrpAttr.u32MaxW				 = stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH				 = stSizeIn.u32Height;

	VpssGrp = CVI_VPSS_GetAvailableGrp();
	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		return NULL;
	}

	for (i = 0; i < VPSS_MAX_CHN_NUM; i++) {
		VpssChn = i;
		stVpssChnAttr.u32Width			= astSizeOut[i].u32Width;
		stVpssChnAttr.u32Height 		= astSizeOut[i].u32Height;
		stVpssChnAttr.enVideoFormat 		= VIDEO_FORMAT_LINEAR;
		stVpssChnAttr.enPixelFormat 		= enFormatOut;
		stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
		stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
		stVpssChnAttr.u32Depth			= 1;
		stVpssChnAttr.bMirror			= CVI_FALSE;
		stVpssChnAttr.bFlip 			= CVI_FALSE;
		stVpssChnAttr.stAspectRatio.enMode		= ASPECT_RATIO_NONE;
		stVpssChnAttr.stNormalize.bEnable		= CVI_FALSE;

		s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
			goto exit0;
		}

		s32Ret = CVI_VPSS_AttachVbPool(VpssGrp, VpssChn, 1 + i);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_AttachVbPool failed with %#x\n", s32Ret);
			goto exit0;
		}

		s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
			goto exit0;
		}
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit1;
	}

	//send frame
	s32Ret = FileToFrame(&stSizeIn, enFormatIn, pFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit2;
	}

	for (i = 0; i < s32Repeat; i++) {
		u32ChnMask = 0;

		for (j = 0; j < VPSS_MAX_CHN_NUM; j++) {
			abChnEnable[j] = rand() % 2 ? CVI_TRUE : CVI_FALSE;
			if (abChnEnable[j]) {
				CVI_VPSS_EnableChn(VpssGrp, j);
				u32ChnMask |= BIT(j);
			} else
				CVI_VPSS_DisableChn(VpssGrp, j);
		}

		if (!u32ChnMask)
			continue;

		s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SendFrame fail.\n");
			goto exit3;
		}

		for (j = 0; j < VPSS_MAX_CHN_NUM; j++) {
			if (!abChnEnable[j])
				continue;
			VpssChn = j;
			s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, UT_TIMEOUT_MS);
			if (s32Ret != CVI_SUCCESS) {
				VPSS_UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
				goto exit3;
			}

			s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
			if (s32Ret != CVI_SUCCESS) {
				VPSS_UT_PRT("CVI_VPSS_ReleaseChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
				goto exit3;
			}
		}
	}

exit3:
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
exit2:
	CVI_VPSS_StopGrp(VpssGrp);
exit1:
	for (j = 0; j < VPSS_MAX_CHN_NUM; j++)
		CVI_VPSS_DisableChn(VpssGrp, j);
exit0:
	CVI_VPSS_DestroyGrp(VpssGrp);

	if (s32Ret == CVI_SUCCESS) {
		pthread_mutex_lock(&s_SyncMutex);
		s_u32Flag |= BIT(VpssGrp);
		pthread_mutex_unlock(&s_SyncMutex);
	}

	return NULL;
}

static CVI_S32 vpss_test_pressure(CVI_VOID)
{
	CVI_S32 i, s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	CVI_S32 s32ThreadNum = 10;
	pthread_t thread[10] = {[0 ... 9] = -1};
	SIZE_S astSizeOut[VPSS_MAX_CHN_NUM] = {{DEFAULT_W, DEFAULT_H}, {1280, 720}, {640, 360}, {320, 180}};

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt				= 1 + VPSS_MAX_CHN_NUM;

	u32BlkSizeIn = COMMON_GetPicBufferSize(DEFAULT_W, DEFAULT_H, PIXEL_FORMAT_YUV_PLANAR_420,
		DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= s32ThreadNum;
	stVbConf.astCommPool[0].enRemapMode = VB_REMAP_MODE_CACHED;
	VPSS_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn);

	for (i = 0; i < VPSS_MAX_CHN_NUM; i++) {
		u32BlkSizeOut = COMMON_GetPicBufferSize(astSizeOut[i].u32Width, astSizeOut[i].u32Height,
			PIXEL_FORMAT_NV21, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

		stVbConf.astCommPool[1 + i].u32BlkSize	= u32BlkSizeOut;
		stVbConf.astCommPool[1 + i].u32BlkCnt	= s32ThreadNum;
		stVbConf.astCommPool[1 + i].enRemapMode = VB_REMAP_MODE_CACHED;
		VPSS_UT_PRT("common pool[%d] BlkSize %d\n", 1 + i, u32BlkSizeOut);
	}

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	srand((unsigned)time(NULL));
	s_u32Flag = 0;

	for (i = 0; i < s32ThreadNum; i++) {
		s32Ret = pthread_create(&thread[i], NULL, pressure_thread_run, NULL);
		if (s32Ret < 0) {
			VPSS_UT_PRT("pthread_create fail. s32Ret: 0x%x !\n", s32Ret);
			break;
		}
	}

	for (i = 0; i < s32ThreadNum; i++)
		if (thread[i] > 0)
			pthread_join(thread[i], NULL);

	for (i = 0; i < s32ThreadNum; i++)
		if (!(s_u32Flag & BIT(i)))
			s32Ret = CVI_FAILURE;

	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();

	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_perf(CVI_VOID)
{
	CVI_S32 i, s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	VIDEO_FRAME_INFO_S stVideoFrameIn, stVideoFrameOut;
	SIZE_S stSize = {1920, 1080};
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	CVI_CHAR *pstFileNameIn = VPSS_DEFAULT_FILE_IN;
	CVI_U64 u64CurPTS1, u64CurPTS2, u64CostTime;
	CVI_U64 u64MinCostTime = 10000, u64MaxCostTime = 0;

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height,
		enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 1;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 2;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	VPSS_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSize);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enPixelFormat;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;

	stVpssChnAttr.u32Width		    = stSize.u32Width;
	stVpssChnAttr.u32Height		    = stSize.u32Height;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enPixelFormat;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip				= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable		= CVI_FALSE;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//send frame
	s32Ret = FileToFrame(&stSize, enPixelFormat, pstFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	for (i = 0; i < TEST_CNT0; i++) {
		CVI_SYS_GetCurPTS(&u64CurPTS1);
		s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_SendFrame fail.\n");
			goto exit5;
		}
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, UT_TIMEOUT_MS);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
			goto exit5;
		}
		CVI_SYS_GetCurPTS(&u64CurPTS2);
		u64CostTime = u64CurPTS2 - u64CurPTS1;
		u64MinCostTime = u64CostTime < u64MinCostTime ? u64CostTime : u64MinCostTime;
		u64MaxCostTime = u64CostTime > u64MaxCostTime ? u64CostTime : u64MaxCostTime;

		s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("CVI_VPSS_ReleaseChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
			goto exit5;
		}
		//system("cat /proc/soph/vpss");
	}
	if ((u64MaxCostTime - u64MinCostTime) > 500) {
		s32Ret = -1;
		VPSS_UT_PRT("Time fluctuation anomaly !!!\n");
	}
	VPSS_UT_PRT("1080P cost time: Min-Max: (%ld, %ld)us, offset=%ld\n", u64MinCostTime,
		u64MaxCostTime, u64MaxCostTime - u64MinCostTime);

exit5:
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();

	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_mp_get_chn_frm_test(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	SIZE_S stSize = {DEFAULT_W, DEFAULT_H};
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	CVI_CHAR *pstFileNameIn = VPSS_DEFAULT_FILE_IN;

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height,
		enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 1;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 2;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	VPSS_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSize);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enPixelFormat;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;

	stVpssChnAttr.u32Width		    = stSize.u32Width;
	stVpssChnAttr.u32Height		    = stSize.u32Height;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enPixelFormat;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip				= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable		= CVI_FALSE;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//send frame
	s32Ret = FileSendToVpss(VpssGrp, &stSize, enPixelFormat, pstFileNameIn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	s32Ret = system("./vpss_ut_client 0");
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("client fail\n");
	}

exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();

	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_stitch(CVI_VOID)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ChnNum = 2;
	VPSS_STITCH_CHN_ATTR_S *pstInput;
	VPSS_STITCH_OUTPUT_ATTR_S stOutput;
	VIDEO_FRAME_INFO_S stVideoFrame;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	SIZE_S stSizeIn = {DEFAULT_W, DEFAULT_H};
	SIZE_S stSizeOut = {DEFAULT_W*2, DEFAULT_H*2};
	PIXEL_FORMAT_E aenFormatIn[2] = {PIXEL_FORMAT_YUV_PLANAR_420, PIXEL_FORMAT_YUV_PLANAR_422};
	PIXEL_FORMAT_E enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	CVI_CHAR *aszFileNameIn[2] = {VPSS_STITCH_FILE_IN0, VPSS_STITCH_FILE_IN1};
	RECT_S astDstRect[2] = {{0, 0, 960, 540}, {960, 540, 1920, 1080}};
	CVI_U32 i;
	CVI_CHAR aszFileNameOut[64];

	snprintf(aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stSizeOut.u32Width,
		stSizeOut.u32Height,
		GetFmtName(enFormatOut));

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt              = 2;

	u32BlkSize = COMMON_GetPicBufferSize(stSizeIn.u32Width, stSizeIn.u32Height, aenFormatIn[1],
					DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 2;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_NONE;
	VPSS_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSize);

	u32BlkSize = COMMON_GetPicBufferSize(stSizeOut.u32Width, stSizeOut.u32Height, enFormatOut,
					DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[1].u32BlkCnt	= 1;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_NONE;
	VPSS_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSize);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Set VPSS Stitch Attr
	 ************************************************/
	// stInput
	pstInput = (VPSS_STITCH_CHN_ATTR_S *)calloc(sizeof(VPSS_STITCH_CHN_ATTR_S), u32ChnNum);

	for (i = 0; i < u32ChnNum; i++) {
		s32Ret = FileToFrame(&stSizeIn, aenFormatIn[i], aszFileNameIn[i], &pstInput[i].stVideoFrame);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("%s FileToFrame fail, s32Ret: 0x%x !\n", aszFileNameIn[i], s32Ret);
			goto exit1;
		}
		pstInput[i].stDstRect = astDstRect[i];
	}

	// stOutput
	stOutput.u32Color = 0xffff00;
	stOutput.enPixelformat = enFormatOut;
	stOutput.u32Width = stSizeOut.u32Width;
	stOutput.u32Height = stSizeOut.u32Height;

	s32Ret = CVI_VPSS_Stitch(u32ChnNum, pstInput, &stOutput, &stVideoFrame);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_Stitch fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit2;
	}

	if (CompareWithMD5(MD5_STITCH, &stVideoFrame)) {
		FrameSaveToFile(aszFileNameOut, &stVideoFrame);
		s32Ret = CVI_FAILURE;
		VPSS_UT_PRT("Compare MD5 fail, MD5:%s\n", MD5_STITCH);
	}

	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrame.stVFrame.u64PhyAddr[0]));
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(pstInput[0].stVideoFrame.stVFrame.u64PhyAddr[0]));
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(pstInput[1].stVideoFrame.stVFrame.u64PhyAddr[0]));

exit2:
	if (pstInput) {
		free(pstInput);
		pstInput = NULL;
	}

exit1:
	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();

	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_stitch_pip(CVI_VOID)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ChnNum = 2;
	VPSS_STITCH_CHN_ATTR_S *pstInput;
	VPSS_STITCH_OUTPUT_ATTR_S stOutput;
	VIDEO_FRAME_INFO_S stVideoFrame;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	SIZE_S stSizeIn = {DEFAULT_W, DEFAULT_H};
	SIZE_S stSizeOut = {DEFAULT_W, DEFAULT_H};
	PIXEL_FORMAT_E aenFormatIn[2] = {PIXEL_FORMAT_YUV_PLANAR_420, PIXEL_FORMAT_YUV_PLANAR_422};
	PIXEL_FORMAT_E enFormatOut = PIXEL_FORMAT_NV21;
	CVI_CHAR *aszFileNameIn[2] = {VPSS_STITCH_FILE_IN0, VPSS_STITCH_FILE_IN1};
	RECT_S astDstRect[2] = {{0, 0, 1920, 1080}, {30, 600, 640, 360}};
	CVI_U32 i;
	CVI_CHAR aszFileNameOut[64];

	snprintf(aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stSizeOut.u32Width,
		stSizeOut.u32Height,
		GetFmtName(enFormatOut));

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt              = 1;

	u32BlkSize = COMMON_GetPicBufferSize(stSizeIn.u32Width, stSizeIn.u32Height, aenFormatIn[1],
					DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 3;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_NONE;
	VPSS_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSize);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Set VPSS Stitch Attr
	 ************************************************/
	// stInput
	pstInput = (VPSS_STITCH_CHN_ATTR_S *)calloc(sizeof(VPSS_STITCH_CHN_ATTR_S), u32ChnNum);

	for (i = 0; i < u32ChnNum; i++) {
		s32Ret = FileToFrame(&stSizeIn, aenFormatIn[i], aszFileNameIn[i], &pstInput[i].stVideoFrame);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("%s FileToFrame fail, s32Ret: 0x%x !\n", aszFileNameIn[i], s32Ret);
			goto exit1;
		}
		pstInput[i].stDstRect = astDstRect[i];
	}

	// stOutput
	stOutput.u32Color = 0x0;
	stOutput.enPixelformat = enFormatOut;
	stOutput.u32Width = stSizeOut.u32Width;
	stOutput.u32Height = stSizeOut.u32Height;

	s32Ret = CVI_VPSS_Stitch(u32ChnNum, pstInput, &stOutput, &stVideoFrame);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_Stitch fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit2;
	}

	if (CompareWithMD5(MD5_STITCH_PIP, &stVideoFrame)) {
		FrameSaveToFile(aszFileNameOut, &stVideoFrame);
		s32Ret = CVI_FAILURE;
		VPSS_UT_PRT("Compare MD5 fail, MD5:%s\n", MD5_STITCH_PIP);
	}

	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrame.stVFrame.u64PhyAddr[0]));
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(pstInput[0].stVideoFrame.stVFrame.u64PhyAddr[0]));
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(pstInput[1].stVideoFrame.stVFrame.u64PhyAddr[0]));

exit2:
	if (pstInput) {
		free(pstInput);
		pstInput = NULL;
	}

exit1:
	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();

	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_stitch_four_grid(CVI_VOID)
{
	CVI_S32 s32Ret;
	CVI_U32 u32ChnNum = 4;
	VPSS_STITCH_CHN_ATTR_S *pstInput;
	VPSS_STITCH_OUTPUT_ATTR_S stOutput;
	VIDEO_FRAME_INFO_S stVideoFrame;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	SIZE_S stSizeIn = {DEFAULT_W, DEFAULT_H};
	SIZE_S stSizeOut = {DEFAULT_W*2, DEFAULT_H*2};
	PIXEL_FORMAT_E aenFormatIn[4] = {PIXEL_FORMAT_YUV_PLANAR_420, PIXEL_FORMAT_YUV_PLANAR_422,
									PIXEL_FORMAT_YUV_PLANAR_422, PIXEL_FORMAT_YUV_PLANAR_420};
	PIXEL_FORMAT_E enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	CVI_CHAR *aszFileNameIn[4] = {VPSS_STITCH_FILE_IN0, VPSS_STITCH_FILE_IN1,
								VPSS_STITCH_FILE_IN1, VPSS_STITCH_FILE_IN0};
	RECT_S astDstRect[4] = {{0, 0, 1920, 1080}, {1920, 0, 1920, 1080},
							{0, 1080, 1920, 1080}, {1920, 1080, 1920, 1080}};
	CVI_U32 i;
	CVI_CHAR aszFileNameOut[64];

	snprintf(aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stSizeOut.u32Width,
		stSizeOut.u32Height,
		GetFmtName(enFormatOut));

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt              = 2;

	u32BlkSize = COMMON_GetPicBufferSize(stSizeIn.u32Width, stSizeIn.u32Height, aenFormatIn[1],
					DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 4;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_NONE;
	VPSS_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSize);

	u32BlkSize = COMMON_GetPicBufferSize(stSizeOut.u32Width, stSizeOut.u32Height, enFormatOut,
					DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[1].u32BlkCnt	= 1;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_NONE;
	VPSS_UT_PRT("common pool[1] BlkSize %d\n", u32BlkSize);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Set VPSS Stitch Attr
	 ************************************************/
	// stInput
	pstInput = (VPSS_STITCH_CHN_ATTR_S *)calloc(sizeof(VPSS_STITCH_CHN_ATTR_S), u32ChnNum);

	for (i = 0; i < u32ChnNum; i++) {
		s32Ret = FileToFrame(&stSizeIn, aenFormatIn[i], aszFileNameIn[i], &pstInput[i].stVideoFrame);
		if (s32Ret != CVI_SUCCESS) {
			VPSS_UT_PRT("%s FileToFrame fail, s32Ret: 0x%x !\n", aszFileNameIn[i], s32Ret);
			goto exit1;
		}
		pstInput[i].stDstRect = astDstRect[i];
	}

	// stOutput
	stOutput.u32Color = 0x0;
	stOutput.enPixelformat = enFormatOut;
	stOutput.u32Width = stSizeOut.u32Width;
	stOutput.u32Height = stSizeOut.u32Height;

	s32Ret = CVI_VPSS_Stitch(u32ChnNum, pstInput, &stOutput, &stVideoFrame);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_Stitch fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit2;
	}

	if (CompareWithMD5(MD5_STITCH_GRID, &stVideoFrame)) {
		FrameSaveToFile(aszFileNameOut, &stVideoFrame);
		s32Ret = CVI_FAILURE;
		VPSS_UT_PRT("Compare MD5 fail, MD5:%s\n", MD5_STITCH_GRID);
	}

	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrame.stVFrame.u64PhyAddr[0]));
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(pstInput[0].stVideoFrame.stVFrame.u64PhyAddr[0]));
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(pstInput[1].stVideoFrame.stVFrame.u64PhyAddr[0]));
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(pstInput[2].stVideoFrame.stVFrame.u64PhyAddr[0]));
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(pstInput[3].stVideoFrame.stVFrame.u64PhyAddr[0]));

exit2:
	if (pstInput) {
		free(pstInput);
		pstInput = NULL;
	}

exit1:
	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();

	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_tile_1_to_2(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	SIZE_S stSizeIn = {5000, 2000};
	VPSS_MULTI_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = stSizeIn.u32Width;
	stTestParam.stSizeIn.u32Height = stSizeIn.u32Height;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	strncpy(stTestParam.aszFileNameIn, VPSS_TILE_MODE_FILE_IN, sizeof(stTestParam.aszFileNameIn));

	stTestParam.astChnParam[0].bEnable = CVI_TRUE;
	stTestParam.astChnParam[0].VpssChn = 0;
	stTestParam.astChnParam[0].stSizeOut.u32Width = stSizeIn.u32Width;
	stTestParam.astChnParam[0].stSizeOut.u32Height = stSizeIn.u32Height;
	stTestParam.astChnParam[0].bMirror = CVI_FALSE;
	stTestParam.astChnParam[0].bFlip = CVI_FALSE;
	stTestParam.astChnParam[0].enFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.astChnParam[0].stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.astChnParam[0].stNormalize.bEnable = CVI_FALSE;
	snprintf(stTestParam.astChnParam[0].aszFileNameOut, 64, "%s/%s_chn%d_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__, 0,
		stTestParam.astChnParam[0].stSizeOut.u32Width,
		stTestParam.astChnParam[0].stSizeOut.u32Height,
		GetFmtName(stTestParam.astChnParam[0].enFormatOut));

	stTestParam.astChnParam[1].bEnable = CVI_TRUE;
	stTestParam.astChnParam[1].VpssChn = 1;
	stTestParam.astChnParam[1].stSizeOut.u32Width = 960;
	stTestParam.astChnParam[1].stSizeOut.u32Height = 540;
	stTestParam.astChnParam[1].bMirror = CVI_FALSE;
	stTestParam.astChnParam[1].bFlip = CVI_FALSE;
	stTestParam.astChnParam[1].enFormatOut = PIXEL_FORMAT_RGB_888_PLANAR;
	stTestParam.astChnParam[1].stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.astChnParam[1].stNormalize.bEnable = CVI_FALSE;
	snprintf(stTestParam.astChnParam[1].aszFileNameOut, 64, "%s/%s_chn%d_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__, 1,
		stTestParam.astChnParam[1].stSizeOut.u32Width,
		stTestParam.astChnParam[1].stSizeOut.u32Height,
		GetFmtName(stTestParam.astChnParam[1].enFormatOut));

	strncpy(stTestParam.astChnParam[0].aszMD5Sum, MD5_TILE_CHN0,
		sizeof(stTestParam.astChnParam[0].aszMD5Sum));
	strncpy(stTestParam.astChnParam[1].aszMD5Sum, MD5_TILE_CHN1,
		sizeof(stTestParam.astChnParam[1].aszMD5Sum));
	s32Ret = basic_mutli_chn(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_slt(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = DEFAULT_W;
	stTestParam.stSizeIn.u32Height = DEFAULT_H;
	stTestParam.stSizeOut.u32Width = DEFAULT_W;
	stTestParam.stSizeOut.u32Height = DEFAULT_H;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	stTestParam.enFormatOut = PIXEL_FORMAT_NV21;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.u32CheckSum = 0x13030706;
	strncpy(stTestParam.aszMD5Sum, MD5_BASIC, sizeof(stTestParam.aszMD5Sum));
	strncpy(stTestParam.aszFileNameIn, VPSS_DEFAULT_FILE_IN, sizeof(stTestParam.aszFileNameIn));
	strncpy(stTestParam.aszFileNameRef, SLT_REF, sizeof(stTestParam.aszFileNameRef));
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_csc_rgb2yuv(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	SIZE_S stSize = {1920, 1080};
	PIXEL_FORMAT_E enPixelFormatIn = PIXEL_FORMAT_RGB_888;
	PIXEL_FORMAT_E enPixelFormatOut = PIXEL_FORMAT_YUV_PLANAR_420;
	CVI_CHAR *pstFileNameIn = VPSS_RGB_FILE_IN;
	VIDEO_FRAME_INFO_S stVideoFrameIn, stVideoFrameOut;

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height,
				PIXEL_FORMAT_RGB_888_PLANAR, DATA_BITWIDTH_8,
				COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 1;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 2;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enPixelFormatIn;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;

	stVpssChnAttr.u32Width		    = stSize.u32Width;
	stVpssChnAttr.u32Height		    = stSize.u32Height;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enPixelFormatOut;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip				= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable		= CVI_FALSE;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//RGB2YUV
	s32Ret = FileToFrame(&stSize, enPixelFormatIn, pstFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SendFrame fail.\n");
		goto exit5;
	}

	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, 1000);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_GetChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
	}

	s32Ret = CompareCmodel_rgb2yuv(&stVideoFrameIn, &stVideoFrameOut);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CompareCmodel_rgb2yuv fail!\n");
	} else {
		VPSS_UT_PRT("RGB2YUV OK!\n");
	}

	CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);


exit5:
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();

	return s32Ret;
}

static CVI_S32 vpss_test_csc_yuv2rgb(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	SIZE_S stSize = {1920, 1080};
	PIXEL_FORMAT_E enPixelFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	PIXEL_FORMAT_E enPixelFormatOut = PIXEL_FORMAT_RGB_888;
	CVI_CHAR *pstFileNameIn = VPSS_DEFAULT_FILE_IN;
	VIDEO_FRAME_INFO_S stVideoFrameIn, stVideoFrameOut;

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height,
				PIXEL_FORMAT_RGB_888_PLANAR, DATA_BITWIDTH_8,
				COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 1;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 2;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enPixelFormatIn;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;

	stVpssChnAttr.u32Width		    = stSize.u32Width;
	stVpssChnAttr.u32Height		    = stSize.u32Height;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enPixelFormatOut;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;
	stVpssChnAttr.bMirror			= CVI_FALSE;
	stVpssChnAttr.bFlip				= CVI_FALSE;
	stVpssChnAttr.stAspectRatio.enMode		= ASPECT_RATIO_NONE;
	stVpssChnAttr.stNormalize.bEnable		= CVI_FALSE;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//YUV2RGB
	s32Ret = FileToFrame(&stSize, enPixelFormatIn, pstFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("FileToFrame fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SendFrame fail.\n");
		goto exit5;
	}

	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, 1000);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_GetChnFrame for grp0 chn0. s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
	}

	s32Ret = CompareCmodel_yuv2rgb(&stVideoFrameIn, &stVideoFrameOut);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CompareCmodel_yuv2rgb fail!\n");
	} else {
		VPSS_UT_PRT("YUV2RGB OK!\n");
	}

	CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);


exit5:
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();

	return s32Ret;
}

static CVI_S32 vpss_test_c_model(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = vpss_test_csc_rgb2yuv();
	s32Ret |= vpss_test_csc_yuv2rgb();

	return s32Ret;
}

static CVI_S32 vpss_test_user_config(CVI_VOID)
{
	CVI_S32 i, s32Ret = CVI_SUCCESS;
	CVI_U32 u32WidthIn, u32HeightIn, u32WidthOut, u32HeightOut;
	CVI_U32 u32FormatIn, u32FormatOut;
	RECT_S stChnCrop;
	VPSS_BASIC_TEST_PARAM stTestParam;

	memset(&stTestParam, 0, sizeof(stTestParam));
	printf("\n---vpss config---\n");
	printf("input width:");
	scanf("%d", &u32WidthIn);
	printf("input height:");
	scanf("%d", &u32HeightIn);

	printf("output width:");
	scanf("%d", &u32WidthOut);
	printf("output height:");
	scanf("%d", &u32HeightOut);

	printf("chn crop x:");
	scanf("%d", &stChnCrop.s32X);
	printf("chn crop y:");
	scanf("%d", &stChnCrop.s32Y);
	printf("chn crop width:");
	scanf("%d", &stChnCrop.u32Width);
	printf("chn crop height:");
	scanf("%d", &stChnCrop.u32Height);

	printf("format list:\n");
	for (i = PIXEL_FORMAT_RGB_888; i < PIXEL_FORMAT_MAX; i++) {
		if (strncmp(GetFmtName(i), "unknown", sizeof("unknown")))
			printf("%2d : %s\n", i, GetFmtName(i));
	}
	printf("input format:");
	scanf("%d", &u32FormatIn);
	printf("output format:");
	scanf("%d", &u32FormatOut);

	printf("input file:");
	scanf("%s", stTestParam.aszFileNameIn);

	stTestParam.VpssGrp = 0;
	stTestParam.stSizeIn.u32Width = u32WidthIn;
	stTestParam.stSizeIn.u32Height = u32HeightIn;
	stTestParam.stSizeOut.u32Width = u32WidthOut;
	stTestParam.stSizeOut.u32Height = u32HeightOut;
	stTestParam.bMirror = CVI_FALSE;
	stTestParam.bFlip = CVI_FALSE;
	stTestParam.enFormatIn = u32FormatIn;
	stTestParam.enFormatOut = u32FormatOut;
	stTestParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
	stTestParam.stNormalize.bEnable = CVI_FALSE;
	stTestParam.stChnCropInfo.bEnable = CVI_TRUE;
	stTestParam.stChnCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
	stTestParam.stChnCropInfo.stCropRect = stChnCrop;
	snprintf(stTestParam.aszFileNameOut, 64, "%s/%s_%d_%d_%s.bin",
		OUT_FILE_PREFIX, __func__,
		stTestParam.stSizeOut.u32Width,
		stTestParam.stSizeOut.u32Height,
		GetFmtName(stTestParam.enFormatOut));

	s32Ret = basic(&stTestParam);
	TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 vpss_test_get_region_luma(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = VPSS_CHN0;
	VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S stVpssChnAttr = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	SIZE_S stSize = {DEFAULT_W, DEFAULT_H};
	PIXEL_FORMAT_E enFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	RECT_S rgn_rect[2] = {{20, 20, 50, 50}, {21, 30, 60, 80}};
	VIDEO_REGION_INFO_S stRegionInfo = {.u32RegionNum = 2, .pstRegion = rgn_rect};
	CVI_U64 *p64LumaData;
	CVI_U32 i;

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height,
		enFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	stVbConf.u32MaxPoolCnt              = 1;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 2;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	VPSS_UT_PRT("common pool[0] BlkSize %d\n", u32BlkSize);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = enFormat;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;

	stVpssChnAttr.u32Width		    = stSize.u32Width;
	stVpssChnAttr.u32Height		    = stSize.u32Height;
	stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	stVpssChnAttr.enPixelFormat		    = enFormat;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Depth			= 1;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	s32Ret = FileSendToVpss(VpssGrp, &stSize, enFormat, VPSS_DEFAULT_FILE_IN);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("FileSendToVpss fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	p64LumaData = malloc(sizeof(CVI_U64) * stRegionInfo.u32RegionNum);
	if (p64LumaData == NULL) {
		VPSS_UT_PRT("Memory allocation failed!\n");
		s32Ret = CVI_FAILURE;
		goto exit5;
	}
	if (CVI_VPSS_GetRegionLuma(0, 0, &stRegionInfo, p64LumaData, 1000) != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_GetRegionLuma failed!\n");
		s32Ret = CVI_FAILURE;
		goto exit5;
	}

	for (i = 0; i < stRegionInfo.u32RegionNum; i++) {
		VPSS_UT_PRT("Region[%d]: luma %ld\n", i, p64LumaData[i]);
	}

	rgn_rect[1].s32X = 1920;
	rgn_rect[1].u32Width = 20;
	stRegionInfo.pstRegion = rgn_rect;

	s32Ret = FileSendToVpss(VpssGrp, &stSize, enFormat, VPSS_DEFAULT_FILE_IN);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("FileSendToVpss fail, s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
	}

	if (CVI_VPSS_GetRegionLuma(0, 0, &stRegionInfo, p64LumaData, 1000) == CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_GetRegionLuma Should not OK - invalid param!\n");
		s32Ret = CVI_FAILURE;
	}

exit5:
	if (p64LumaData != NULL) {
		free(p64LumaData);
		p64LumaData = NULL;
	}
exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	CVI_VPSS_DisableChn(VpssGrp, VpssChn);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();
	return s32Ret;
}

static CVI_S32 vpss_test_auto(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret |= vpss_test_basic();
	s32Ret |= vpss_test_1_to_2();
	s32Ret |= vpss_test_1_to_3();
	s32Ret |= vpss_test_1_to_4();
	s32Ret |= vpss_test_limit_width();
	s32Ret |= vpss_test_max_resolution();
	s32Ret |= vpss_test_mirror();
	s32Ret |= vpss_test_flip();
	s32Ret |= vpss_test_mirror_flip();
	s32Ret |= vpss_test_aspect_ratio();
	s32Ret |= vpss_test_multi_grp();
	s32Ret |= vpss_test_multi_thread();
	s32Ret |= vpss_test_byte_align();
	s32Ret |= vpss_test_resize();
	s32Ret |= vpss_test_tile_mode();
	s32Ret |= vpss_test_tile_1_to_2();
	s32Ret |= vpss_test_draw_rect();
	s32Ret |= vpss_test_format();
	s32Ret |= vpss_test_grp_crop();
	s32Ret |= vpss_test_chn_crop();
	s32Ret |= vpss_test_amp_ctrl();
	s32Ret |= vpss_test_normalize();
	s32Ret |= vpss_test_convert();
	s32Ret |= vpss_test_scale_coef();
	s32Ret |= vpss_test_y_ratio();
	s32Ret |= vpss_test_hide();
	s32Ret |= vpss_test_rotation();
	s32Ret |= vpss_test_ldc();
	s32Ret |= vpss_test_fisheye();
	s32Ret |= vpss_test_fbd_basic();
	s32Ret |= vpss_test_pressure();
	s32Ret |= vpss_mp_get_chn_frm_test();
	s32Ret |= vpss_test_stitch();
	s32Ret |= vpss_test_stitch_pip();
	s32Ret |= vpss_test_stitch_four_grid();
	s32Ret |= vpss_test_get_region_luma();

	return s32Ret;
}

static CVI_S32 _vpss_handle_op(CVI_S32 op)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	switch (op) {
	case VPSS_TEST_BASIC:
		s32Ret = vpss_test_basic();
		break;
	case VPSS_TEST_1_to_2:
		s32Ret = vpss_test_1_to_2();
		break;
	case VPSS_TEST_1_to_3:
		s32Ret = vpss_test_1_to_3();
		break;
	case VPSS_TEST_1_to_4:
		s32Ret = vpss_test_1_to_4();
		break;
	case VPSS_TEST_LIMIT_WIDTH:
		s32Ret = vpss_test_limit_width();
		break;
	case VPSS_TEST_MAX_RES:
		s32Ret = vpss_test_max_resolution();
		break;
	case VPSS_TEST_MIRROR:
		s32Ret = vpss_test_mirror();
		break;
	case VPSS_TEST_FLIP:
		s32Ret = vpss_test_flip();
		break;
	case VPSS_TEST_MIRROR_FLIP:
		s32Ret = vpss_test_mirror_flip();
		break;
	case VPSS_TEST_ASPECT_RATIO:
		s32Ret = vpss_test_aspect_ratio();
		break;
	case VPSS_TEST_MULTI_GRP:
		s32Ret = vpss_test_multi_grp();
		break;
	case VPSS_TEST_MULTI_THREAD:
		s32Ret = vpss_test_multi_thread();
		break;
	case VPSS_TEST_BYTE_ALIGN:
		s32Ret = vpss_test_byte_align();
		break;
	case VPSS_TEST_RESIZE:
		s32Ret = vpss_test_resize();
		break;
	case VPSS_TEST_MAX_SCALING:
		s32Ret = vpss_test_max_scaling();
		break;
	case VPSS_TEST_TILE_MODE:
		s32Ret = vpss_test_tile_mode();
		break;
	case VPSS_TEST_DRAW_RECT:
		s32Ret = vpss_test_draw_rect();
		break;
	case VPSS_TEST_FORMAT:
		s32Ret = vpss_test_format();
		break;
	case VPSS_TEST_GRP_CROP:
		s32Ret = vpss_test_grp_crop();
		break;
	case VPSS_TEST_CHN_CROP:
		s32Ret = vpss_test_chn_crop();
		break;
	case VPSS_TEST_AMP_CTRL:
		s32Ret = vpss_test_amp_ctrl();
		break;
	case VPSS_TEST_NORMALIZE:
		s32Ret = vpss_test_normalize();
		break;
	case VPSS_TEST_CONVERT_TO:
		s32Ret = vpss_test_convert();
		break;
	case VPSS_TEST_SCALE_COEF:
		s32Ret = vpss_test_scale_coef();
		break;
	case VPSS_TEST_Y_RATIO:
		s32Ret = vpss_test_y_ratio();
		break;
	case VPSS_TEST_HIDE:
		s32Ret = vpss_test_hide();
		break;
	case VPSS_TEST_ROT:
		s32Ret = vpss_test_rotation();
		break;
	case VPSS_TEST_LDC:
		s32Ret = vpss_test_ldc();
		break;
	case VPSS_TEST_FISHEYE:
		s32Ret = vpss_test_fisheye();
		break;
	case VPSS_TEST_FBD:
		s32Ret = vpss_test_fbd_basic();
		break;
	case VPSS_TEST_PRESSURE:
		s32Ret = vpss_test_pressure();
		break;
	case VPSS_TEST_PERF:
		s32Ret = vpss_test_perf();
		break;
	case VPSS_TEST_MP_GET_CHN_FRM:
		s32Ret = vpss_mp_get_chn_frm_test();
		break;
	case VPSS_TEST_USER_CONFIG:
		s32Ret = vpss_test_user_config();
		break;
	case VPSS_TEST_STITCH:
		s32Ret = vpss_test_stitch();
		break;
	case VPSS_TEST_STITCH_PIP:
		s32Ret = vpss_test_stitch_pip();
		break;
	case VPSS_TEST_STITCH_FOUR_GRID:
		s32Ret = vpss_test_stitch_four_grid();
		break;
	case VPSS_TEST_TILE_1_to_2:
		s32Ret = vpss_test_tile_1_to_2();
		break;
	case VPSS_TEST_SLT:
		s32Ret = vpss_test_slt();
		break;
	case VPSS_TEST_C_MODEL:
		s32Ret = vpss_test_c_model();
		break;
	case VPSS_TEST_GET_REGION_LUMA:
		s32Ret = vpss_test_get_region_luma();
		break;
	case VPSS_TEST_AUTO:
		s32Ret = vpss_test_auto();
		break;
	default:
		s32Ret = CVI_FAILURE;
		break;
	}

	return s32Ret;
}

static void vpss_show_help(void)
{
	VPSS_UT_PRT("%4d: vpss basic test\n", VPSS_TEST_BASIC);
	VPSS_UT_PRT("%4d: vpss 2 chn\n", VPSS_TEST_1_to_2);
	VPSS_UT_PRT("%4d: vpss 3 chn\n", VPSS_TEST_1_to_3);
	VPSS_UT_PRT("%4d: vpss 4 chn\n", VPSS_TEST_1_to_4);
	VPSS_UT_PRT("%4d: limit width(%d)\n", VPSS_TEST_LIMIT_WIDTH, VPSS_HW_LIMIT_WIDTH);
	VPSS_UT_PRT("%4d: Maximum resolution\n", VPSS_TEST_MAX_RES);
	VPSS_UT_PRT("%4d: mirror\n", VPSS_TEST_MIRROR);
	VPSS_UT_PRT("%4d: flip\n", VPSS_TEST_FLIP);
	VPSS_UT_PRT("%4d: rotation 180(mirror + flip)\n", VPSS_TEST_MIRROR_FLIP);
	VPSS_UT_PRT("%4d: aspect ratio\n", VPSS_TEST_ASPECT_RATIO);
	VPSS_UT_PRT("%4d: multi grp\n", VPSS_TEST_MULTI_GRP);
	VPSS_UT_PRT("%4d: multi thread\n", VPSS_TEST_MULTI_THREAD);
	VPSS_UT_PRT("%4d: byte align\n", VPSS_TEST_BYTE_ALIGN);
	VPSS_UT_PRT("%4d: resize\n", VPSS_TEST_RESIZE);
	VPSS_UT_PRT("%4d: 128 scaling\n", VPSS_TEST_MAX_SCALING);
	VPSS_UT_PRT("%4d: tile mode\n", VPSS_TEST_TILE_MODE);
	VPSS_UT_PRT("%4d: draw rectangle\n", VPSS_TEST_DRAW_RECT);
	VPSS_UT_PRT("%4d: pixel format\n", VPSS_TEST_FORMAT);
	VPSS_UT_PRT("%4d: group crop\n", VPSS_TEST_GRP_CROP);
	VPSS_UT_PRT("%4d: channel crop\n", VPSS_TEST_CHN_CROP);
	VPSS_UT_PRT("%4d: amp ctrl\n", VPSS_TEST_AMP_CTRL);
	VPSS_UT_PRT("%4d: normalize\n", VPSS_TEST_NORMALIZE);
	VPSS_UT_PRT("%4d: convert\n", VPSS_TEST_CONVERT_TO);
	VPSS_UT_PRT("%4d: scale coef\n", VPSS_TEST_SCALE_COEF);
	VPSS_UT_PRT("%4d: y ratio\n", VPSS_TEST_Y_RATIO);
	VPSS_UT_PRT("%4d: hide\n", VPSS_TEST_HIDE);
	VPSS_UT_PRT("%4d: rotation\n", VPSS_TEST_ROT);
	VPSS_UT_PRT("%4d: ldc\n", VPSS_TEST_LDC);
	VPSS_UT_PRT("%4d: fisheye\n", VPSS_TEST_FISHEYE);
	VPSS_UT_PRT("%4d: fbd\n", VPSS_TEST_FBD);
	VPSS_UT_PRT("%4d: pressure test\n", VPSS_TEST_PRESSURE);
	VPSS_UT_PRT("%4d: perf test(1920x1080)\n", VPSS_TEST_PERF);
	VPSS_UT_PRT("%4d: multi-process get chn frame test\n", VPSS_TEST_MP_GET_CHN_FRM);
	VPSS_UT_PRT("%4d: stitch\n", VPSS_TEST_STITCH);
	VPSS_UT_PRT("%4d: stitch picture in picture\n", VPSS_TEST_STITCH_PIP);
	VPSS_UT_PRT("%4d: stitch four-square grid\n", VPSS_TEST_STITCH_FOUR_GRID);
	VPSS_UT_PRT("%4d: tile mode 2 chn\n", VPSS_TEST_TILE_1_to_2);
	VPSS_UT_PRT("%4d: slt case\n", VPSS_TEST_SLT);
	VPSS_UT_PRT("%4d: c-model test\n", VPSS_TEST_C_MODEL);
	VPSS_UT_PRT("%4d: get region luma test\n", VPSS_TEST_GET_REGION_LUMA);
	VPSS_UT_PRT("%4d: user config\n", VPSS_TEST_USER_CONFIG);
	VPSS_UT_PRT("%4d: auto test\n", VPSS_TEST_AUTO);

	VPSS_UT_PRT(" 255: exit\n");
}

int main(int argc, char **argv)
{
	CVI_S32 s32Ret;
	CVI_S32 op = 255;
	char mkdir_cmd[64] = {0};

	VPSS_UT_PRT("Create Output Directory %s !\n", OUT_FILE_PREFIX);
	snprintf(mkdir_cmd, 63, "mkdir -p %s", OUT_FILE_PREFIX);
	system(mkdir_cmd);

	system("stty erase ^H");

	signal(SIGINT, vpss_ut_HandleSig);
	signal(SIGTERM, vpss_ut_HandleSig);

	if (argc >= 2) {
		op = (CVI_S32)atoi(argv[1]);
		s32Ret = _vpss_handle_op(op);
		VPSS_UT_PRT("vpss ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	} else {
		do {
			vpss_show_help();
			scanf("%d", &op);

			s32Ret = _vpss_handle_op(op);
			if (op != 255)
				VPSS_UT_PRT("vpss ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
		} while (op != 255);
	}

	return s32Ret;
}


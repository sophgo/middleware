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
#include "cvi_gdc.h"

#include "sample_fisheye_comm.h"

#ifndef BIT
#define BIT(nr)      (UINT64_C(1) << (nr))
#endif

//file: http://disk-sophgo-vip.quickconnect.cn/sharing/iglxSGiJB

#define GDC_FILE_IN_ROT                         "res/ldc/input/1920x1080.yuv"
#define GDC_FILE_OUT_ROT0                       "res/ldc/output/1920x1080_rot0.yuv"
#define GDC_FILE_OUT_ROT90                      "res/ldc/output/1920x1080_rot90.yuv"
#define GDC_FILE_OUT_ROT270                     "res/ldc/output/1920x1080_rot270.yuv"

#define GDC_FILE_IN_ROT_2                         "res/ldc/input/4608x4608.yuv"
#define GDC_FILE_OUT_ROT0_2                       "res/ldc/output/4608x4608_rot0.yuv"

#define GDC_FILE_IN_FISHEYE                     "res/ldc/input/fisheye_floor_1024x1024.yuv"
#define GDC_FILE_OUT_FISHEYE_PANORAMA           "res/ldc/output/fisheye_floor_panorama360_1280x720.yuv"
#define GDC_FILE_OUT_FISHEYE_PANORAMA_180       "res/ldc/output/fisheye_floor_panorama180_1280x720.yuv"
#define GDC_FILE_OUT_FISHEYE_02_1O4R            "res/ldc/output/fisheye_floor_02_1O4R_1280x720.yuv"
#define GDC_FILE_OUT_FISHEYE_03_4R              "res/ldc/output/fisheye_floor_03_4R_1280x720.yuv"
#define GDC_FILE_OUT_FISHEYE_04_1P2R            "res/ldc/output/fisheye_floor_04_1P2R_1280x720.yuv"
#define GDC_FILE_OUT_FISHEYE_05_1P2R            "res/ldc/output/fisheye_floor_05_1P2R_1280x720.yuv"
#define GDC_FILE_OUT_FISHEYE_06_1P              "res/ldc/output/fisheye_floor_06_1P_1280x720.yuv"
#define GDC_FILE_OUT_FISHEYE_07_2P              "res/ldc/output/fisheye_floor_07_2P_1280x720.yuv"

#define GDC_FILE_IN_FMT_0                      "res/ldc/input/1920x1080.yuv"
#define GDC_FILE_OUT_FMT_0                     "res/ldc/output/1920x1088.yuv"
#define GDC_FILE_IN_FMT_1                      "res/ldc/input/1920x1080_yonly.yuv"
#define GDC_FILE_OUT_FMT_1                     "res/ldc/output/1920x1088_yonly.yuv"
#define DWA_FILE_IN_FMT_0                      "res/ldc/input/1920x1080_yuv420.yuv"
#define DWA_FILE_OUT_FMT_0                     "res/ldc/output/1920x1080_yuv420.yuv"
#define DWA_FILE_IN_FMT_1                      "res/ldc/input/1920x1080_yonly.yuv"
#define DWA_FILE_OUT_FMT_1                     "res/ldc/output/1920x1080_yonly_dwa.yuv"
#define DWA_FILE_IN_FMT_2                      "res/ldc/input/1920x1080_yuv444planner.yuv"
#define DWA_FILE_OUT_FMT_2                     "res/ldc/output/1920x1080_yuv444planner.yuv"
#define DWA_FILE_IN_FMT_3                      "res/ldc/input/1920x1080_rgb888planner.bin"
#define DWA_FILE_OUT_FMT_3                     "res/ldc/output/1920x1080_rgb888planner.bin"

#define GDC_FILE_IN_NOT_ALIGN			"res/ldc/input/666x666_yuv400.yuv"
#define GDC_FILE_OUT_NOT_ALIGN_0		"res/ldc/output/666x666_ldc_yuv400.yuv"
#define GDC_FILE_OUT_NOT_ALIGN_1		"res/ldc/output/666x666_dwa_yuv400.yuv"

#define GDC_FILE_IN_CMDQ                       "res/ldc/input/1920x1080.yuv"
#define GDC_FILE_OUT_CMDQ                      "res/ldc/output/1920x1080_cmdq.yuv"
#define GDC_FILE_IN_CMDQ_1TO2                  "res/ldc/input/1920x1080.yuv"
#define GDC_FILE_OUT_CMDQ_1TO2_0               "res/ldc/output/1920x1080_ldc_cmdq_1to2.yuv"

#define GDC_FILE_IN_AFFINE                      "res/ldc/input/girls_1920x1080.yuv"
#define GDC_FILE_OUT_AFFINE                     "res/ldc/output/girls_affine_128x1280.yuv"

#define DWA_FILE_IN_LDC_BARREL_0P3              "res/ldc/input/1920x1080_barrel_0.3_yuv420.yuv"
#define DWA_FILE_OUT_LDC_BARREL_0P3_0           "res/ldc/output/1920x1080_barrel_0.3_r0_ofst_0_0_d-200.yuv"
#define DWA_FILE_OUT_LDC_BARREL_0P3_1           "res/ldc/output/1920x1080_barrel_0.3_r0_ofst_0_0_d-200_2048x1280.yuv"
#define DWA_FILE_OUT_LDC_BARREL_0P3_2           "res/ldc/output/1920x1080_barrel_0.3_r100_ofst_0_0_d-200.yuv"
#define DWA_FILE_OUT_LDC_BARREL_0P3_3           "res/ldc/output/1920x1080_barrel_0.3_r100_ofst_0_0_d-200_2048x1280.yuv"
#define DWA_FILE_IN_LDC_PINCUSHION_0P3          "res/ldc/input/1920x1080_pincushion_0.3.yuv"
#define DWA_FILE_OUT_LDC_PINCUSHION_0P3_0       "res/ldc/output/1920x1080_pincushion_0.3_r0_ofst_0_0_d400.yuv"
#define DWA_FILE_OUT_LDC_PINCUSHION_0P3_1       "res/ldc/output/1920x1080_pincushion_0.3_r0_ofst_0_0_d400_2048x1280.yuv"
#define DWA_FILE_OUT_LDC_PINCUSHION_0P3_2       "res/ldc/output/1920x1080_pincushion_0.3_r100_ofst_0_0_d400.yuv"
#define DWA_FILE_OUT_LDC_PINCUSHION_0P3_3       "res/ldc/output/1920x1080_pincushion_0.3_r100_ofst_0_0_d400_2048x1280.yuv"

#define DWA_FILE_IN_LDC_GRID_INFO_L             "res/ldc/input/imgL_1280X720.yonly.yuv"
#define DWA_FILE_OUT_LDC_GRID_INFO_L            "res/ldc/output/imgL_1280x720.yonly.yuv"
#define DWA_FILE_IN_LDC_GRID_L                  "res/ldc/input/grid_info_79_43_3397_80_45_1280x720.dat"
#define DWA_FILE_IN_LDC_GRID_INFO_R             "res/ldc/input/imgR_1280X720.yonly.yuv"
#define DWA_FILE_OUT_LDC_GRID_INFO_R            "res/ldc/output/imgR_1280x720.yonly.yuv"
#define DWA_FILE_IN_LDC_GRID_R                  "res/ldc/input/grid_info_79_44_3476_80_45_1280x720.dat"

#define DWA_FILE_IN_DIS_GRID                      "res/ldc/input/dis/grid_info_94_52_4888_96_54_dst_1920x1080_src_3840x2160.dat"
#define DWA_FILE_IN_DIS0                          "res/ldc/input/dis/srcL_cx4_000000.yuv"
#define DWA_FILE_IN_DIS1                          "res/ldc/input/dis/srcL_cx4_000001.yuv"
#define DWA_FILE_IN_DIS2                          "res/ldc/input/dis/srcL_cx4_000002.yuv"
#define DWA_FILE_OUT_DIS0                         "res/ldc/output/dst_eis_1920_1080_0.yuv"
#define DWA_FILE_OUT_DIS1                         "res/ldc/output/dst_eis_1920_1080_1.yuv"
#define DWA_FILE_OUT_DIS2                         "res/ldc/output/dst_eis_1920_1080_2.yuv"
#define DWA_FILE_SRC_MESH_0                       "res/ldc/input/dis/src_mesh_4888_f000.dat"
#define DWA_FILE_SRC_MESH_1                       "res/ldc/input/dis/src_mesh_4888_f001.dat"
#define DWA_FILE_SRC_MESH_2                       "res/ldc/input/dis/src_mesh_4888_f002.dat"

#define LDC_MAX_W    4608
#define LDC_MAX_H    4608
#define LDC_MIN_W    64
#define LDC_MIN_H    64
#define DWA_MAX_W    4096
#define DWA_MAX_H    4096
#define DWA_MIN_W    32
#define DWA_MIN_H    32

#ifndef FPGA_PORTING
#define GDC_REPEAT_TIMES 6
#else
#define GDC_REPEAT_TIMES 2
#endif

typedef CVI_S32 (*p_func)(void);

#define MAX_FUNC_CNT 100

static CVI_BOOL bEnProc;
static CVI_BOOL g_gdc_save_file;
static CVI_BOOL needSuspend;

typedef enum _GDC_TEST_OP {
	GDC_TEST_ROT = 0,
	GDC_TEST_LDC,
	GDC_TEST_FISHEYE,
	GDC_TEST_AFFINE,
	GDC_TEST_FMT,
	GDC_TEST_SIZE_NO_ALIGN,
	GDC_TEST_CMDQ,
	GDC_TEST_CMDQ_1TO2,
	GDC_TEST_ASYNC,
	GDC_TEST_MULTI_THREAD,
	GDC_TEST_LOAD_GRID_INFO_LDC,
	GDC_TEST_RST,
	GDC_TEST_DIS,
} GDC_TEST_OP;

typedef struct _GDC_BASIC_TEST_PARAM {
	SIZE_S size_in;
	SIZE_S size_out;
	char filename_in[128];
	char filename_out[128];
	ROTATION_E enRotation;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	VIDEO_FRAME_INFO_S stVideoFrameIn;
	VIDEO_FRAME_INFO_S stVideoFrameOut;
	VB_BLK inBlk, outBlk;
	PIXEL_FORMAT_E enPixelFormat;
	GDC_HANDLE hHandle;
	GDC_TASK_ATTR_S stTask;
	GDC_IDENTITY_ATTR_S identity;
	GDC_TEST_OP op;
	CVI_BOOL bnalign;
} GDC_BASIC_TEST_PARAM;

void gdc_ut_HandleSig(CVI_S32 signo)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	if (SIGINT == signo || SIGTERM == signo) {
		CVI_SYS_Exit();
		CVI_VB_Exit();
		GDC_UT_PRT("Program termination abnormally\n");
	}
	exit(-1);
}

static CVI_S32 gdc_basic_add_tsk(GDC_BASIC_TEST_PARAM *param, void *ptr)
{
	LDC_ATTR_S *LDCAttr;
	ROTATION_E enRotation;
	FISHEYE_ATTR_S *FisheyeAttr;
	AFFINE_ATTR_S *affineAttr;

	CVI_S32 s32Ret = CVI_FAILURE;

	if (!param) {
		GDC_UT_PRT("gdc_basic fail, null ptr for test param\n");
		return CVI_FAILURE;
	}

	switch (param->op) {
	case GDC_TEST_ROT:
		enRotation =(ROTATION_E)(uintptr_t)ptr;

		s32Ret = CVI_GDC_AddRotationTask(param->hHandle, &param->stTask, enRotation);
		if (s32Ret) {
			GDC_UT_PRT("CVI_GDC_AddRotationTask failed!\n");
		}
		break;
	case GDC_TEST_LDC:
		LDCAttr =(LDC_ATTR_S *)ptr;

		s32Ret = CVI_GDC_AddLDCTask(param->hHandle, &param->stTask, LDCAttr, param->enRotation);
		if (s32Ret) {
			GDC_UT_PRT("CVI_GDC_AddLDCTask failed!\n");
		}
		break;
	case GDC_TEST_FISHEYE:
		FisheyeAttr = (FISHEYE_ATTR_S *)ptr;

		if (FisheyeAttr->bBgColor)
			param->stTask.au64privateData[3] = (CVI_U64)FisheyeAttr->u32BgColor;

		s32Ret = CVI_GDC_AddCorrectionTask(param->hHandle, &param->stTask, FisheyeAttr);
		if (s32Ret) {
			GDC_UT_PRT("CVI_GDC_AddCorrectionTask failed!\n");
		}
		break;
	case GDC_TEST_AFFINE:
		affineAttr =(AFFINE_ATTR_S *)ptr;

		s32Ret = CVI_GDC_AddAffineTask(param->hHandle, &param->stTask, affineAttr);
		if (s32Ret) {
			GDC_UT_PRT("CVI_GDC_AddAffineTask failed!\n");
		}
		break;
	default:
		GDC_UT_PRT("not allow this op(%d) fail\n", param->op);
		break;
	}

	return s32Ret;
}

static CVI_S32 gdc_basic(GDC_BASIC_TEST_PARAM *param, void *ptr)
{
	int times = GDC_REPEAT_TIMES;
	VB_CONFIG_S stVbConf = {0};
	CVI_S32 s32Ret;
	CVI_U32 BlkSize;
	CVI_BOOL bEnHwLDC = 0;

	if (!param) {
		GDC_UT_PRT("gdc_basic fail, null ptr for test param\n");
		return CVI_FAILURE;
	}

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	param->u32BlkSizeIn = COMMON_GetPicBufferSize(ALIGN(param->size_in.u32Width, GDC_STRIDE_ALIGN)
		, ALIGN(param->size_in.u32Height, GDC_STRIDE_ALIGN), param->enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);
	param->u32BlkSizeOut = COMMON_GetPicBufferSize(ALIGN(param->size_out.u32Width, GDC_STRIDE_ALIGN)
		, ALIGN(param->size_out.u32Height, GDC_STRIDE_ALIGN), param->enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);

	BlkSize = (param->u32BlkSizeIn > param->u32BlkSizeOut) ? param->u32BlkSizeIn : param->u32BlkSizeOut;

	stVbConf.u32MaxPoolCnt				= 1;
	stVbConf.astCommPool[0].u32BlkSize	= BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 5;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_NOCACHE;

	GDC_UT_PRT("common pool[0] BlkSize %d\n", BlkSize);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		GDC_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		GDC_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		GDC_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init GDC
	 ************************************************/
	s32Ret = CVI_GDC_Init();
	if (s32Ret != CVI_SUCCESS) {
		GDC_UT_PRT("CVI_GDC_Init failed!\n");
		goto exit1;
	}

	if (param->op == GDC_TEST_ROT)
		bEnHwLDC = 1;

	do {
		param->hHandle = 0;
		memset(&param->stVideoFrameIn, 0, sizeof(param->stVideoFrameIn));
		memset(&param->stVideoFrameOut, 0, sizeof(param->stVideoFrameOut));
		if (param->bnalign) {
			s32Ret = GDCFileToFrame2(&param->size_in, param->enPixelFormat, param->filename_in, &param->stVideoFrameIn);
			if (s32Ret) {
				GDC_UT_PRT("GDCFileToFrame2 failed!\n");
				goto exit2;
			}

			s32Ret = GDC_COMM_PrepareFrame2(&param->size_out, param->enPixelFormat, &param->stVideoFrameOut);
			if (s32Ret) {
				GDC_UT_PRT("GDC_COMM_PrepareFrame2 failed!\n");
				goto exit2;
			}
		} else {
			s32Ret = GDCFileToFrame(&param->size_in, param->enPixelFormat, param->filename_in, &param->stVideoFrameIn, bEnHwLDC);
			if (s32Ret) {
				GDC_UT_PRT("GDCFileToFrame failed!\n");
				goto exit2;
			}


			s32Ret = GDC_COMM_PrepareFrame(&param->size_out, param->enPixelFormat, &param->stVideoFrameOut, bEnHwLDC);
			if (s32Ret) {
				GDC_UT_PRT("GDC_COMM_PrepareFrame failed!\n");
				goto exit2;
			}
		}

		memset(param->stTask.au64privateData, 0, sizeof(param->stTask.au64privateData));
		memcpy(&param->stTask.stImgIn, &param->stVideoFrameIn, sizeof(param->stVideoFrameIn));
		memcpy(&param->stTask.stImgOut, &param->stVideoFrameOut, sizeof(param->stVideoFrameOut));

		s32Ret = CVI_GDC_BeginJob(&param->hHandle);
		if (s32Ret) {
			GDC_UT_PRT("CVI_GDC_BeginJob failed!\n");
			goto exit2;
		}

		s32Ret = CVI_GDC_SetJobIdentity(param->hHandle, &param->identity);
		if (s32Ret) {
			GDC_UT_PRT("CVI_GDC_SetJobIdentity failed!\n");
			goto exit2;
		}

		s32Ret = gdc_basic_add_tsk(param, ptr);
		if (s32Ret != CVI_SUCCESS) {
			GDC_UT_PRT("gdc_basic_add_tsk. s32Ret: 0x%x !\n", s32Ret);
			goto exit2;
		}

		s32Ret = CVI_GDC_EndJob(param->hHandle);
		if (s32Ret) {
			GDC_UT_PRT("CVI_GDC_EndJob failed!\n");
			goto exit2;
		}

		GDC_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param->stVideoFrameIn.stVFrame.u64PhyAddr[0]
			, param->stVideoFrameIn.stVFrame.u64PhyAddr[1], param->stVideoFrameIn.stVFrame.u64PhyAddr[2]);
		GDC_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param->stVideoFrameOut.stVFrame.u64PhyAddr[0]
			, param->stVideoFrameOut.stVFrame.u64PhyAddr[1], param->stVideoFrameOut.stVFrame.u64PhyAddr[2]);

		if (g_gdc_save_file) {
			s32Ret = GDCFrameSaveToFile(param->filename_out, &param->stVideoFrameOut);
			if (s32Ret != CVI_SUCCESS) {
				GDC_UT_PRT("GDCFrameSaveToFile. s32Ret: 0x%x !\n", s32Ret);
				goto exit2;
			}
			GDC_UT_PRT("-------------------times:(%d)----------------------\n", times);
			GDC_UT_PRT("output file:%s\n", param->filename_out);
		}
		param->inBlk = CVI_VB_PhysAddr2Handle(param->stVideoFrameIn.stVFrame.u64PhyAddr[0]);
		if (param->inBlk != VB_INVALID_HANDLE) {
			s32Ret |= CVI_VB_ReleaseBlock(param->inBlk);
			param->inBlk = VB_INVALID_HANDLE;
		}

		param->outBlk = CVI_VB_PhysAddr2Handle(param->stVideoFrameOut.stVFrame.u64PhyAddr[0]);
		if (param->outBlk != VB_INVALID_HANDLE) {
			s32Ret |= CVI_VB_ReleaseBlock(param->outBlk);
			param->outBlk = VB_INVALID_HANDLE;
		}

		if (s32Ret) {
			GDC_UT_PRT("release VB fail.\n");
			goto exit2;
		}
	} while (times--);

	if (bEnProc)
		system("cat /proc/soph/ldc");

exit2:
	if (s32Ret)
		if (param->hHandle)
			s32Ret |= CVI_GDC_CancelJob(param->hHandle);
	s32Ret |= CVI_GDC_DeInit();
	if (s32Ret) {
		GDC_UT_PRT("CVI_GDC_DeInit fail.\n");
	}
	param->inBlk = CVI_VB_PhysAddr2Handle(param->stVideoFrameIn.stVFrame.u64PhyAddr[0]);
	if (param->inBlk != VB_INVALID_HANDLE) {
		s32Ret |= CVI_VB_ReleaseBlock(param->inBlk);
		param->inBlk = VB_INVALID_HANDLE;
	}
	param->outBlk = CVI_VB_PhysAddr2Handle(param->stVideoFrameOut.stVFrame.u64PhyAddr[0]);
	if (param->outBlk != VB_INVALID_HANDLE) {
		s32Ret |= CVI_VB_ReleaseBlock(param->outBlk);
		param->outBlk = VB_INVALID_HANDLE;
	}
exit1:
	s32Ret |= CVI_SYS_Exit();
exit0:
	s32Ret |= CVI_VB_Exit();

	return s32Ret;
}

static CVI_S32 gdc_test_rot(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ROTATION_E rot[3] = {ROTATION_0, ROTATION_90, ROTATION_270};
	GDC_BASIC_TEST_PARAM param = {0};
	int cnt = 3;
	char *filename_in[3] = {GDC_FILE_IN_ROT, GDC_FILE_IN_ROT, GDC_FILE_IN_ROT};
	char *filename_out[3] = {GDC_FILE_OUT_ROT0, GDC_FILE_OUT_ROT90, GDC_FILE_OUT_ROT270};

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		param.size_in.u32Width = 1920;
		param.size_in.u32Height = 1080;
		if (i) {
			param.size_out.u32Width = 1088;
			param.size_out.u32Height = 1920;
		} else {
			param.size_out.u32Width = 1920;
			param.size_out.u32Height = 1088;
		}
		param.enPixelFormat = PIXEL_FORMAT_NV21;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_rot_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_rot_%d", i);
		param.identity.syncIo = CVI_TRUE;

		param.op = GDC_TEST_ROT;

		s32Ret = gdc_basic(&param, (void *)rot[i]);
		if (s32Ret != CVI_SUCCESS) {
			GDC_UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	GDC_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_ldc(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	LDC_ATTR_S stLdcAttr[8] = {
		{CVI_TRUE, 0, 0, 0, 0, 0, -200, {0}, 0},
		{CVI_TRUE, 0, 0, 0, 0, 0, -200, {0}, 0},
		{CVI_TRUE, 0, 0, 100, 0, 0, -200, {0}, 0},
		{CVI_TRUE, 0, 0, 100, 0, 0, -200, {0}, 0},
		{CVI_TRUE, 0, 0, 0, 0, 0, 400, {0}, 0},
		{CVI_TRUE, 0, 0, 0, 0, 0, 400, {0}, 0},
		{CVI_TRUE, 0, 0, 100, 0, 0, 400, {0}, 0},
		{CVI_TRUE, 0, 0, 100, 0, 0, 400, {0}, 0},
	};
	GDC_BASIC_TEST_PARAM param = {0};
	int cnt = 8;
	char *filename_in[8] = {
		DWA_FILE_IN_LDC_BARREL_0P3,
		DWA_FILE_IN_LDC_BARREL_0P3,
		DWA_FILE_IN_LDC_BARREL_0P3,
		DWA_FILE_IN_LDC_BARREL_0P3,
		DWA_FILE_IN_LDC_PINCUSHION_0P3,
		DWA_FILE_IN_LDC_PINCUSHION_0P3,
		DWA_FILE_IN_LDC_PINCUSHION_0P3,
		DWA_FILE_IN_LDC_PINCUSHION_0P3,
	};
	char *filename_out[8] = {
		DWA_FILE_OUT_LDC_BARREL_0P3_0,
		DWA_FILE_OUT_LDC_BARREL_0P3_1,
		DWA_FILE_OUT_LDC_BARREL_0P3_2,
		DWA_FILE_OUT_LDC_BARREL_0P3_3,
		DWA_FILE_OUT_LDC_PINCUSHION_0P3_0,
		DWA_FILE_OUT_LDC_PINCUSHION_0P3_1,
		DWA_FILE_OUT_LDC_PINCUSHION_0P3_2,
		DWA_FILE_OUT_LDC_PINCUSHION_0P3_3,
	};

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);

		param.size_in.u32Width = 1920;
		param.size_in.u32Height = 1080;
		if ((i % 2) == 0) {
			param.size_out.u32Width = 1920;
			param.size_out.u32Height = 1080;
		} else {
			param.size_out.u32Width = 2048;
			param.size_out.u32Height = 1280;
		}
		param.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_ldc_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_ldc_%d", i);
		param.identity.syncIo = CVI_TRUE;

		param.op = GDC_TEST_LDC;

		s32Ret = gdc_basic(&param, (void *)&stLdcAttr[i]);
		if (s32Ret != CVI_SUCCESS) {
			GDC_UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	GDC_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_fisheye(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	FISHEYE_ATTR_S stFisheyeAttr = {0};
	GDC_BASIC_TEST_PARAM param = {0};
	int cnt = 8;
	char *filename_in[8] = {GDC_FILE_IN_FISHEYE, GDC_FILE_IN_FISHEYE, GDC_FILE_IN_FISHEYE, GDC_FILE_IN_FISHEYE, \
		GDC_FILE_IN_FISHEYE, GDC_FILE_IN_FISHEYE, GDC_FILE_IN_FISHEYE, GDC_FILE_IN_FISHEYE};
	char *filename_out[8] = {GDC_FILE_OUT_FISHEYE_PANORAMA, GDC_FILE_OUT_FISHEYE_PANORAMA_180, GDC_FILE_OUT_FISHEYE_02_1O4R, \
		GDC_FILE_OUT_FISHEYE_03_4R, GDC_FILE_OUT_FISHEYE_04_1P2R, GDC_FILE_OUT_FISHEYE_05_1P2R, GDC_FILE_OUT_FISHEYE_06_1P, GDC_FILE_OUT_FISHEYE_07_2P};

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		param.size_in.u32Width = 1024;
		param.size_in.u32Height = 1024;
		param.size_out.u32Width = 1280;
		param.size_out.u32Height = 720;
		param.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_fisheye_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_fisheye_%d", i);
		param.identity.syncIo = CVI_TRUE;

		param.op = GDC_TEST_FISHEYE;

		stFisheyeAttr.bEnable = CVI_TRUE;
		stFisheyeAttr.bBgColor = CVI_TRUE;
		stFisheyeAttr.u32BgColor = YUV_8BIT(0, 128, 128);
		stFisheyeAttr.s32HorOffset = param.size_in.u32Width / 2;
		stFisheyeAttr.s32VerOffset = param.size_in.u32Height / 2;
		if (i == 0) {
			stFisheyeAttr.enMountMode = FISHEYE_DESKTOP_MOUNT;
			stFisheyeAttr.enUseMode = MODE_PANORAMA_360;
			stFisheyeAttr.astFishEyeRegionAttr[0].enViewMode = FISHEYE_VIEW_360_PANORAMA;
		} else if (i == 1) {
			stFisheyeAttr.enMountMode = FISHEYE_WALL_MOUNT;
			stFisheyeAttr.enUseMode = MODE_PANORAMA_180;
			stFisheyeAttr.astFishEyeRegionAttr[0].enViewMode = FISHEYE_VIEW_180_PANORAMA;
		} else if (i == 2) {
			stFisheyeAttr.enMountMode = FISHEYE_DESKTOP_MOUNT;
			stFisheyeAttr.enUseMode = MODE_02_1O4R;
			stFisheyeAttr.astFishEyeRegionAttr[0].enViewMode = FISHEYE_VIEW_360_PANORAMA;
		} else if (i == 3) {
			stFisheyeAttr.enMountMode = FISHEYE_DESKTOP_MOUNT;
			stFisheyeAttr.enUseMode = MODE_03_4R;
			stFisheyeAttr.astFishEyeRegionAttr[0].enViewMode = FISHEYE_VIEW_360_PANORAMA;
		} else if (i == 4) {
			stFisheyeAttr.enMountMode = FISHEYE_WALL_MOUNT;
			stFisheyeAttr.enUseMode = MODE_04_1P2R;
			stFisheyeAttr.astFishEyeRegionAttr[0].enViewMode = FISHEYE_VIEW_180_PANORAMA;
		} else if (i == 5) {
			stFisheyeAttr.enMountMode = FISHEYE_WALL_MOUNT;
			stFisheyeAttr.enUseMode = MODE_05_1P2R;
			stFisheyeAttr.astFishEyeRegionAttr[0].enViewMode = FISHEYE_VIEW_180_PANORAMA;
		} else if (i == 6) {
			stFisheyeAttr.enMountMode = FISHEYE_WALL_MOUNT;
			stFisheyeAttr.enUseMode = MODE_06_1P;
			stFisheyeAttr.astFishEyeRegionAttr[0].enViewMode = FISHEYE_VIEW_180_PANORAMA;
		} else if (i == 7) {
			stFisheyeAttr.enMountMode = FISHEYE_DESKTOP_MOUNT;
			stFisheyeAttr.enUseMode = MODE_07_2P;
			stFisheyeAttr.astFishEyeRegionAttr[0].enViewMode = FISHEYE_VIEW_360_PANORAMA;
		}

		stFisheyeAttr.u32RegionNum = 1;

		s32Ret = gdc_basic(&param, (void *)&stFisheyeAttr);
		if (s32Ret != CVI_SUCCESS) {
			GDC_UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	GDC_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_affine(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	// for FD/FR profiling - girls generation
	POINT2F_S faces[9][4] = {
		{ {.x = 722.755, .y = 65.7575}, {.x = 828.402, .y = 80.6858}, {.x = 707.827, .y = 171.405}, {.x = 813.474, .y = 186.333} },
		{ {.x = 494.919, .y = 117.918}, {.x = 605.38,  .y = 109.453}, {.x = 503.384, .y = 228.378}, {.x = 613.845, .y = 219.913} },
		{ {.x = 1509.06, .y = 147.139}, {.x = 1592.4,  .y = 193.044}, {.x = 1463.15, .y = 230.48 }, {.x = 1546.5,  .y = 276.383} },
		{ {.x = 1580.21, .y = 66.7939}, {.x = 1694.1,  .y = 70.356 }, {.x = 1576.65, .y = 180.682}, {.x = 1690.54, .y = 184.243} },
		{ {.x = 178.76,  .y = 90.4814}, {.x = 286.234, .y = 80.799 }, {.x = 188.442, .y = 197.955}, {.x = 295.916, .y = 188.273} },
		{ {.x = 1195.57, .y = 139.226}, {.x = 1292.69, .y = 104.122}, {.x = 1230.68, .y = 236.34}, {.x = 1327.79, .y = 201.236}, },
		{ {.x = 398.669, .y = 109.872}, {.x = 501.93, .y = 133.357}, {.x = 375.184, .y = 213.133}, {.x = 478.445, .y = 236.618}, },
		{ {.x = 845.989, .y = 94.591}, {.x = 949.411, .y = 63.6143}, {.x = 876.966, .y = 198.013}, {.x = 980.388, .y = 167.036}, },
		{ {.x = 1060.19, .y = 58.7882}, {.x = 1170.61, .y = 61.9105}, {.x = 1057.07, .y = 169.203}, {.x = 1167.48, .y = 172.325}, },
	};
	AFFINE_ATTR_S stAffineAttr;
	GDC_BASIC_TEST_PARAM param = {0};
	int cnt = 1;
	char *filename_in[1] = {GDC_FILE_IN_AFFINE};
	char *filename_out[1] = {GDC_FILE_OUT_AFFINE};

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		param.size_in.u32Width = 1920;
		param.size_in.u32Height = 1080;
		param.size_out.u32Width = 128;
		param.size_out.u32Height = 1280;
		param.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_affine_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_affine_%d", i);
		param.identity.syncIo = CVI_TRUE;

		param.op = GDC_TEST_AFFINE;

		stAffineAttr.u32RegionNum = 9;
		memcpy(stAffineAttr.astRegionAttr, faces, sizeof(faces));
		stAffineAttr.stDestSize.u32Width = 112;
		stAffineAttr.stDestSize.u32Height = 112;

		s32Ret = gdc_basic(&param, (void *)&stAffineAttr);
		if (s32Ret != CVI_SUCCESS) {
			GDC_UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	GDC_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_fmt(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	GDC_BASIC_TEST_PARAM param = {0};
	int cnt = 6;
	char *filename_in[6] = {GDC_FILE_IN_FMT_0, GDC_FILE_IN_FMT_1, DWA_FILE_IN_FMT_0, DWA_FILE_IN_FMT_1, DWA_FILE_IN_FMT_2, DWA_FILE_IN_FMT_3};
	char *filename_out[6] = {GDC_FILE_OUT_FMT_0, GDC_FILE_OUT_FMT_1, DWA_FILE_OUT_FMT_0, DWA_FILE_OUT_FMT_1, DWA_FILE_OUT_FMT_2, DWA_FILE_OUT_FMT_3};
	PIXEL_FORMAT_E enPixelFormat[6] = {
		PIXEL_FORMAT_NV21,
		PIXEL_FORMAT_YUV_400,
		PIXEL_FORMAT_YUV_PLANAR_420,
		PIXEL_FORMAT_YUV_400,
		PIXEL_FORMAT_YUV_PLANAR_444,
		PIXEL_FORMAT_RGB_888_PLANAR,
	};

	LDC_ATTR_S stLdcAttr[4] = {
		{CVI_TRUE, 0, 0, 0, 0, 0, 400, {0}, 0},
		{CVI_TRUE, 0, 0, 50, 0, 0, 400, {0}, 0},
		{CVI_TRUE, 0, 0, 100, 0, 0, 400, {0}, 0},
		{CVI_TRUE, 0, 0, 100, 0, 0, 400, {0}, 0},
	};
	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		param.size_in.u32Width = 1920;
		param.size_in.u32Height = 1080;
		param.size_out.u32Width = 1920;

		param.enPixelFormat = enPixelFormat[i];
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_fmt_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_fmt_%d", i);
		param.identity.syncIo = CVI_TRUE;

		if (i < 2) {
			param.op = GDC_TEST_ROT;
			param.size_out.u32Height = 1088;
			s32Ret = gdc_basic(&param, (void *)ROTATION_0);
		} else {
			param.op = GDC_TEST_LDC;
			param.size_out.u32Height = 1080;
			s32Ret = gdc_basic(&param, (void *)&stLdcAttr[i - 2]);
		}

		if (s32Ret != CVI_SUCCESS) {
			GDC_UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	GDC_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_not_align(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	LDC_ATTR_S stLdcAttr[1] = {
		{CVI_TRUE, 0, 0, 100, 0, 0, 400, {0}, 0},
	};
	GDC_BASIC_TEST_PARAM param = {0};
	int cnt = 2;
	char *filename_in[2] = {
		GDC_FILE_IN_NOT_ALIGN,
		GDC_FILE_IN_NOT_ALIGN,
	};
	char *filename_out[2] = {
		GDC_FILE_OUT_NOT_ALIGN_0,
		GDC_FILE_OUT_NOT_ALIGN_1,
	};

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);

		param.size_in.u32Width = 666;
		param.size_in.u32Height = 666;
		param.size_out.u32Width = 666;
		param.size_out.u32Height = 666;
		param.bnalign = CVI_TRUE;

		param.enPixelFormat = PIXEL_FORMAT_YUV_400;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_gdc_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_gdc_%d", i);
		param.identity.syncIo = CVI_TRUE;

		if (i == 0) {
			param.op = GDC_TEST_ROT;
			s32Ret = gdc_basic(&param, (void *)ROTATION_0);
		} else {
			param.op = GDC_TEST_LDC;
			s32Ret = gdc_basic(&param, (void *)&stLdcAttr[i - 1]);
		}

		if (s32Ret != CVI_SUCCESS) {
			GDC_UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	GDC_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_reset(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	GDC_BASIC_TEST_PARAM param = {0};
	VB_CONFIG_S stVbConf = {0};
	CVI_U32 BlkSize;
	int times = 4, err_times = 4;

	do {
		for (CVI_U8 cnt = 0; cnt < times; cnt ++) {
			param.op = GDC_TEST_ROT;
			param.size_in.u32Width =   1920;
			param.size_in.u32Height =  1080;
			param.size_out.u32Width =  1920;
			param.size_out.u32Height = 1080;
			param.enPixelFormat = PIXEL_FORMAT_NV21;
			snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_reset_%d", cnt);
			param.identity.enModId = CVI_ID_USER;
			param.identity.u32ID = 0;
			snprintf(param.identity.Name, sizeof(param.identity.Name),	"job_reset_%d", cnt);
			param.identity.syncIo = CVI_TRUE;
			param.op = GDC_TEST_ROT;
			param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width, param.size_in.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
				, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);
			param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width, param.size_out.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
				, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);

			BlkSize = (param.u32BlkSizeIn > param.u32BlkSizeOut) ? param.u32BlkSizeIn : param.u32BlkSizeOut;

			stVbConf.u32MaxPoolCnt				= 1;
			stVbConf.astCommPool[0].u32BlkSize	= BlkSize;
			stVbConf.astCommPool[0].u32BlkCnt	= 3;

			GDC_UT_PRT("common pool[0] BlkSize %d\n", BlkSize);

			s32Ret = CVI_VB_SetConfig(&stVbConf);
			if (s32Ret != CVI_SUCCESS) {
				GDC_UT_PRT("CVI_VB_SetConf failed!\n");
				return s32Ret;
			}

			s32Ret = CVI_VB_Init();
			if (s32Ret != CVI_SUCCESS) {
				GDC_UT_PRT("CVI_VB_Init failed!\n");
				return s32Ret;
			}

			s32Ret = CVI_SYS_Init();
			if (s32Ret != CVI_SUCCESS) {
				GDC_UT_PRT("CVI_SYS_Init failed!\n");
				goto exit0;
			}

			s32Ret = CVI_GDC_Init();
			if (s32Ret != CVI_SUCCESS) {
				GDC_UT_PRT("CVI_GDC_Init failed!\n");
				goto exit1;
			}
			param.hHandle = 0;
			memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
			s32Ret = GDC_COMM_PrepareFrame(&param.size_in, param.enPixelFormat, &param.stVideoFrameIn, 1);
			if (s32Ret) {
				GDC_UT_PRT("GDC_COMM_PrepareFrame in failed!\n");
				goto exit2;
			}

			memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
			s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut, 1);
			if (s32Ret) {
				GDC_UT_PRT("GDC_COMM_PrepareFrame out failed!\n");
				goto exit2;
			}

			memset(param.stTask.au64privateData, 0, sizeof(param.stTask.au64privateData));
			memcpy(&param.stTask.stImgIn, &param.stVideoFrameIn, sizeof(param.stVideoFrameIn));
			memcpy(&param.stTask.stImgOut, &param.stVideoFrameOut, sizeof(param.stVideoFrameOut));

			s32Ret = CVI_GDC_BeginJob(&param.hHandle);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_BeginJob failed!\n");
				goto exit2;
			}

			s32Ret = CVI_GDC_SetJobIdentity(param.hHandle, &param.identity);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_SetJobIdentity failed!\n");
				goto exit2;
			}

			if (cnt == 0) {
				param.stTask.stImgIn.stVFrame.u32Width = 0x3fff;
				param.stTask.stImgIn.stVFrame.u32Height = 0x3fff;
				param.stTask.stImgOut.stVFrame.u32Width = 0x3fff;
				param.stTask.stImgOut.stVFrame.u32Height = 0x3fff;
			}

			s32Ret = CVI_GDC_AddRotationTask(param.hHandle, &param.stTask, ROTATION_0);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_AddRotationTask failed!\n");
			}

			s32Ret = CVI_GDC_EndJob(param.hHandle);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_EndJob failed!\n");
				goto exit2;
			}

			GDC_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param.stVideoFrameIn.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameIn.stVFrame.u64PhyAddr[1], param.stVideoFrameIn.stVFrame.u64PhyAddr[2]);
			GDC_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param.stVideoFrameOut.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameOut.stVFrame.u64PhyAddr[1], param.stVideoFrameOut.stVFrame.u64PhyAddr[2]);

			param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
			if (param.inBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);
				param.inBlk = VB_INVALID_HANDLE;
			}
			param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
			if (param.outBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);
				param.outBlk = VB_INVALID_HANDLE;
			}

			if (s32Ret) {
				GDC_UT_PRT("release VB fail.\n");
				goto exit2;
			}
		exit2:
			if (s32Ret)
				if (param.hHandle)
					s32Ret |= CVI_GDC_CancelJob(param.hHandle);
			s32Ret |= CVI_GDC_DeInit();
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_DeInit fail.\n");
			}

			param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
			if (param.inBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);
				param.inBlk = VB_INVALID_HANDLE;
			}
			param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
			if (param.outBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);
					param.outBlk = VB_INVALID_HANDLE;
			}
		exit1:
			s32Ret |= CVI_SYS_Exit();
		exit0:
			s32Ret |= CVI_VB_Exit();

			if (cnt == 0 && s32Ret == CVI_SUCCESS) {
				GDC_UT_PRT("unknown error, expect NG, but OK occur\n");
				return CVI_FAILURE;
			}
		}
	} while (err_times--);
	GDC_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_cmdq(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	GDC_BASIC_TEST_PARAM param = {0};
	VB_CONFIG_S stVbConf = {0};
	char *filename_in[1] = {GDC_FILE_IN_CMDQ};
	char *filename_out[1] = {GDC_FILE_OUT_CMDQ};
	GDC_TASK_ATTR_S stTask_1st;
	GDC_TASK_ATTR_S stTask_2nd;
	SIZE_S size_1st_out, size_2nd_out;
	int times = GDC_REPEAT_TIMES;
	VB_BLK Blk;
	CVI_U32 BlkSize;

	for (CVI_U8 cnt = 0; cnt < 1; cnt ++) {
		strcpy(param.filename_in, filename_in[0]);
		strcpy(param.filename_out, filename_out[0]);

		param.op = GDC_TEST_ROT;
		param.size_in.u32Width =   1920;
		param.size_in.u32Height =  1080;
		param.size_out.u32Width =  1920;
		param.size_out.u32Height = 1088;
		param.enPixelFormat = PIXEL_FORMAT_NV21;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_cmdq_%d", cnt);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = 0;
		snprintf(param.identity.Name, sizeof(param.identity.Name),	"job_cmdq_%d", cnt);
		param.identity.syncIo = CVI_TRUE;
		param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width, param.size_in.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);
		param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width, param.size_out.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);

		BlkSize = (param.u32BlkSizeIn > param.u32BlkSizeOut) ? param.u32BlkSizeIn : param.u32BlkSizeOut;

		stVbConf.u32MaxPoolCnt				= 1;
		stVbConf.astCommPool[0].u32BlkSize	= BlkSize;
		stVbConf.astCommPool[0].u32BlkCnt	= 3;

		GDC_UT_PRT("common pool[0] BlkSize %d\n", BlkSize);

		s32Ret = CVI_VB_SetConfig(&stVbConf);
		if (s32Ret != CVI_SUCCESS) {
			GDC_UT_PRT("CVI_VB_SetConf failed!\n");
			return s32Ret;
		}

		s32Ret = CVI_VB_Init();
		if (s32Ret != CVI_SUCCESS) {
			GDC_UT_PRT("CVI_VB_Init failed!\n");
			return s32Ret;
		}

		s32Ret = CVI_SYS_Init();
		if (s32Ret != CVI_SUCCESS) {
			GDC_UT_PRT("CVI_SYS_Init failed!\n");
			goto exit0;
		}

		s32Ret = CVI_GDC_Init();
		if (s32Ret != CVI_SUCCESS) {
			GDC_UT_PRT("CVI_GDC_Init failed!\n");
			goto exit1;
		}

		times = GDC_REPEAT_TIMES;

		do {
			param.hHandle = 0;
			memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
			s32Ret = GDCFileToFrame(&param.size_in, param.enPixelFormat, param.filename_in, &param.stVideoFrameIn, CVI_TRUE);
			if (s32Ret) {
				GDC_UT_PRT("GDCFileToFrame failed!\n");
				goto exit2;
			}

			memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
			s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut, CVI_TRUE);
			if (s32Ret) {
				GDC_UT_PRT("GDC_COMM_PrepareFrame out failed!\n");
				goto exit2;
			}

			memset(param.stTask.au64privateData, 0, sizeof(param.stTask.au64privateData));
			memcpy(&param.stTask.stImgIn, &param.stVideoFrameIn, sizeof(param.stVideoFrameIn));
			memcpy(&param.stTask.stImgOut, &param.stVideoFrameOut, sizeof(param.stVideoFrameOut));

			s32Ret = CVI_GDC_BeginJob(&param.hHandle);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_BeginJob failed!\n");
				goto exit2;
			}

			s32Ret = CVI_GDC_SetJobIdentity(param.hHandle, &param.identity);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_SetJobIdentity failed!\n");
				goto exit2;
			}

			//1st rot90
			memset(&stTask_1st.stImgIn, 0, sizeof(stTask_1st.stImgIn));
			memcpy(&stTask_1st.stImgIn, &param.stVideoFrameIn, sizeof(stTask_1st.stImgIn));

			memset(&stTask_1st.stImgOut, 0, sizeof(stTask_1st.stImgOut));
			size_1st_out.u32Width = ALIGN(stTask_1st.stImgIn.stVFrame.u32Height, GDC_STRIDE_ALIGN);
			size_1st_out.u32Height = ALIGN(stTask_1st.stImgIn.stVFrame.u32Width, GDC_STRIDE_ALIGN);
			snprintf(stTask_1st.name, sizeof(param.stTask.name), "%s_1st_%d", param.stTask.name, times);

			s32Ret = GDC_COMM_PrepareFrame(&size_1st_out, param.enPixelFormat, &stTask_1st.stImgOut, CVI_TRUE);
			if (s32Ret) {
				GDC_UT_PRT("GDC_COMM_PrepareFrame 1st out failed!\n");
				goto exit2;
			}

			s32Ret = CVI_GDC_AddRotationTask(param.hHandle, &stTask_1st, ROTATION_90);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_AddRotationTask 1st failed!\n");
				goto exit2;
			}

			//2nd rot90 again
			memset(&stTask_2nd.stImgIn, 0, sizeof(stTask_2nd.stImgIn));
			memcpy(&stTask_2nd.stImgIn, &stTask_1st.stImgOut, sizeof(stTask_2nd.stImgIn));

			memset(&stTask_2nd.stImgOut, 0, sizeof(stTask_2nd.stImgOut));
			memcpy(&stTask_2nd.stImgOut, &param.stVideoFrameOut, sizeof(stTask_2nd.stImgIn));
			size_2nd_out.u32Width = ALIGN(stTask_2nd.stImgIn.stVFrame.u32Height, GDC_STRIDE_ALIGN);
			size_2nd_out.u32Height = ALIGN(stTask_2nd.stImgIn.stVFrame.u32Width, GDC_STRIDE_ALIGN);
			stTask_2nd.stImgOut.stVFrame.u32Width = size_2nd_out.u32Width;
			stTask_2nd.stImgOut.stVFrame.u32Height = size_2nd_out.u32Height;
			snprintf(stTask_2nd.name, sizeof(param.stTask.name), "%s_2nd_%d", param.stTask.name, times);

			s32Ret = CVI_GDC_AddRotationTask(param.hHandle, &stTask_2nd, ROTATION_90);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_AddRotationTask 2nd failed!\n");
				goto exit2;
			}

			s32Ret = CVI_GDC_EndJob(param.hHandle);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_EndJob failed!\n");
				goto exit2;
			}

			GDC_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param.stVideoFrameIn.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameIn.stVFrame.u64PhyAddr[1], param.stVideoFrameIn.stVFrame.u64PhyAddr[2]);
			GDC_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param.stVideoFrameOut.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameOut.stVFrame.u64PhyAddr[1], param.stVideoFrameOut.stVFrame.u64PhyAddr[2]);

			if (g_gdc_save_file) {
				s32Ret = GDCFrameSaveToFile(param.filename_out, &stTask_2nd.stImgOut);
				if (s32Ret != CVI_SUCCESS) {
					GDC_UT_PRT("GDCFrameSaveToFile s32Ret: 0x%x !\n", s32Ret);
					goto exit2;
				}

				GDC_UT_PRT("output file:%s\n", param.filename_out);
			}
			param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
			if (param.inBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);
				param.inBlk = VB_INVALID_HANDLE;
			}
			param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
			if (param.outBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);
				param.outBlk = VB_INVALID_HANDLE;
			}
			Blk = CVI_VB_PhysAddr2Handle(stTask_1st.stImgOut.stVFrame.u64PhyAddr[0]);
			if (Blk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(Blk);
				Blk = VB_INVALID_HANDLE;
			}

			if (s32Ret) {
				GDC_UT_PRT("release VB fail.\n");
				goto exit2;
			}
		} while (times--);

	exit2:
		if (s32Ret)
			if (param.hHandle)
				s32Ret |= CVI_GDC_CancelJob(param.hHandle);
		s32Ret |= CVI_GDC_DeInit();
		if (s32Ret) {
			GDC_UT_PRT("CVI_GDC_DeInit fail.\n");
		}

		param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
		if (param.inBlk != VB_INVALID_HANDLE) {
			s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);
			param.inBlk = VB_INVALID_HANDLE;
		}
		param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
		if (param.outBlk != VB_INVALID_HANDLE) {
			s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);
			param.outBlk = VB_INVALID_HANDLE;
		}
		Blk = CVI_VB_PhysAddr2Handle(stTask_1st.stImgOut.stVFrame.u64PhyAddr[0]);
		if (Blk != VB_INVALID_HANDLE) {
			s32Ret |= CVI_VB_ReleaseBlock(Blk);
			Blk = VB_INVALID_HANDLE;
		}
	exit1:
		s32Ret |= CVI_SYS_Exit();
	exit0:
		s32Ret |= CVI_VB_Exit();

		if (s32Ret)
			goto err;
	}

err:
	GDC_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_cmdq_1to2(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	GDC_BASIC_TEST_PARAM param = {0};
	VB_CONFIG_S stVbConf = {0};
	char *filename_in[1] = {GDC_FILE_IN_CMDQ_1TO2};
	char *filename_out[1] = {GDC_FILE_OUT_CMDQ_1TO2_0};
	GDC_TASK_ATTR_S stTask_tmp;
	VIDEO_FRAME_INFO_S stVideoFrameOut_tmp;
	int times = GDC_REPEAT_TIMES;
	VB_BLK Blk;
	CVI_U32 BlkSize;
	CVI_CHAR name[32];

	for (CVI_U8 cnt = 0; cnt < 2; cnt ++) {
		strcpy(param.filename_in, filename_in[0]);
		strcpy(param.filename_out, filename_out[0]);

		param.op = GDC_TEST_ROT;
		param.size_in.u32Width =   1920;
		param.size_in.u32Height =  1080;
		param.size_out.u32Width =  1920;
		param.size_out.u32Height = 1088;
		param.enPixelFormat = PIXEL_FORMAT_NV21;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_cmdq_%d", cnt);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = 0;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_cmdq_%d", cnt);
		param.identity.syncIo = CVI_TRUE;
		param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width, param.size_in.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);
		param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width, param.size_out.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);

		BlkSize = (param.u32BlkSizeIn > param.u32BlkSizeOut) ? param.u32BlkSizeIn : param.u32BlkSizeOut;

		stVbConf.u32MaxPoolCnt				= 1;
		stVbConf.astCommPool[0].u32BlkSize	= BlkSize;
		stVbConf.astCommPool[0].u32BlkCnt	= 3;

		GDC_UT_PRT("common pool[0] BlkSize %d\n", BlkSize);

		s32Ret = CVI_VB_SetConfig(&stVbConf);
		if (s32Ret != CVI_SUCCESS) {
			GDC_UT_PRT("CVI_VB_SetConf failed!\n");
			return s32Ret;
		}

		s32Ret = CVI_VB_Init();
		if (s32Ret != CVI_SUCCESS) {
			GDC_UT_PRT("CVI_VB_Init failed!\n");
			return s32Ret;
		}

		s32Ret = CVI_SYS_Init();
		if (s32Ret != CVI_SUCCESS) {
			GDC_UT_PRT("CVI_SYS_Init failed!\n");
			goto exit0;
		}

		s32Ret = CVI_GDC_Init();
		if (s32Ret != CVI_SUCCESS) {
			GDC_UT_PRT("CVI_GDC_Init failed!\n");
			goto exit1;
		}

		times = GDC_REPEAT_TIMES;

		do {
			param.hHandle = 0;
			memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
			s32Ret = GDCFileToFrame(&param.size_in, param.enPixelFormat, param.filename_in, &param.stVideoFrameIn, CVI_TRUE);
			if (s32Ret) {
				GDC_UT_PRT("GDCFileToFrame failed!\n");
				goto exit2;
			}

			memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
			s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut, CVI_TRUE);
			if (s32Ret) {
				GDC_UT_PRT("GDC_COMM_PrepareFrame out 1st failed!\n");
				goto exit2;
			}

			memset(param.stTask.au64privateData, 0, sizeof(param.stTask.au64privateData));
			memcpy(&param.stTask.stImgIn, &param.stVideoFrameIn, sizeof(param.stVideoFrameIn));
			memcpy(&param.stTask.stImgOut, &param.stVideoFrameOut, sizeof(param.stVideoFrameOut));

			s32Ret = CVI_GDC_BeginJob(&param.hHandle);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_BeginJob failed!\n");
				goto exit2;
			}

			s32Ret = CVI_GDC_SetJobIdentity(param.hHandle, &param.identity);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_SetJobIdentity failed!\n");
				goto exit2;
			}

			strcpy(name, param.stTask.name);
			snprintf(param.stTask.name, sizeof(param.stTask.name), "%s_1st_%d", name, times);
			s32Ret = CVI_GDC_AddRotationTask(param.hHandle, &param.stTask, ROTATION_0);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_AddRotationTask 1st failed!\n");
				goto exit2;
			}

			//1to2
			memset(&stVideoFrameOut_tmp, 0, sizeof(stVideoFrameOut_tmp));
			s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &stVideoFrameOut_tmp, CVI_TRUE);
			if (s32Ret) {
				GDC_UT_PRT("GDC_COMM_PrepareFrame out 2nd failed!\n");
				goto exit2;
			}

			memset(&stTask_tmp, 0, sizeof(stTask_tmp));
			memcpy(&stTask_tmp.stImgIn, &param.stVideoFrameIn, sizeof(VIDEO_FRAME_INFO_S));
			memcpy(&stTask_tmp.stImgOut, &stVideoFrameOut_tmp, sizeof(VIDEO_FRAME_INFO_S));
			snprintf(stTask_tmp.name, sizeof(param.stTask.name), "%s_2nd_%d", name, times);
			s32Ret = CVI_GDC_AddRotationTask(param.hHandle, &stTask_tmp, ROTATION_0);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_AddRotationTask 2nd failed!\n");
				goto exit2;
			}

			s32Ret = CVI_GDC_EndJob(param.hHandle);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_EndJob failed!\n");
				goto exit2;
			}

			GDC_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param.stVideoFrameIn.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameIn.stVFrame.u64PhyAddr[1], param.stVideoFrameIn.stVFrame.u64PhyAddr[2]);
			GDC_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param.stVideoFrameOut.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameOut.stVFrame.u64PhyAddr[1], param.stVideoFrameOut.stVFrame.u64PhyAddr[2]);

			if (g_gdc_save_file) {
				s32Ret = GDCFrameSaveToFile(param.filename_out, &param.stVideoFrameOut);
				if (s32Ret != CVI_SUCCESS) {
					GDC_UT_PRT("GDCFrameSaveToFile s32Ret: 0x%x !\n", s32Ret);
					goto exit2;
				}
				GDC_UT_PRT("output file:%s\n", param.filename_out);
			}
			param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
			if (param.inBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);
				param.inBlk = VB_INVALID_HANDLE;
			}
			param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
			if (param.outBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);
				param.outBlk = VB_INVALID_HANDLE;
			}
			Blk = CVI_VB_PhysAddr2Handle(stTask_tmp.stImgOut.stVFrame.u64PhyAddr[0]);
			if (Blk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(Blk);
				Blk = VB_INVALID_HANDLE;
			}

			if (s32Ret) {
				GDC_UT_PRT("release VB fail.\n");
				goto exit2;
			}
		} while (times--);

	exit2:
		if (s32Ret)
			if (param.hHandle)
				s32Ret |= CVI_GDC_CancelJob(param.hHandle);
		s32Ret |= CVI_GDC_DeInit();
		if (s32Ret) {
			GDC_UT_PRT("CVI_GDC_DeInit fail.\n");
		}

		param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
		if (param.inBlk != VB_INVALID_HANDLE) {
			s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);
			param.inBlk = VB_INVALID_HANDLE;
		}
		param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
		if (param.outBlk != VB_INVALID_HANDLE) {
			s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);
			param.outBlk = VB_INVALID_HANDLE;
		}
		Blk = CVI_VB_PhysAddr2Handle(stTask_tmp.stImgOut.stVFrame.u64PhyAddr[0]);
		if (Blk != VB_INVALID_HANDLE) {
			s32Ret |= CVI_VB_ReleaseBlock(Blk);
			Blk = VB_INVALID_HANDLE;
		}
	exit1:
		s32Ret |= CVI_SYS_Exit();
	exit0:
		s32Ret |= CVI_VB_Exit();

		if (s32Ret)
			goto err;
	}

err:
	GDC_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_async(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	VB_CONFIG_S stVbConf = {0};
	GDC_BASIC_TEST_PARAM param = {0};
	char *filename_in[2] = {GDC_FILE_IN_ROT, GDC_FILE_IN_FISHEYE};
	char *filename_out[2] = {GDC_FILE_OUT_ROT0, GDC_FILE_OUT_FISHEYE_PANORAMA};
	GDC_TEST_OP op[2] = {GDC_TEST_ROT, GDC_TEST_FISHEYE};
	CVI_U32 WidthIn[2] = {1920, 1024};
	CVI_U32 HeightIn[2] = {1080, 1024};
	CVI_U32 WidthOut[2] = {1920, 1280};
	CVI_U32 HeightOut[2] = {1088, 720};
	CVI_U8 times = GDC_REPEAT_TIMES;
	void *ptr;
	CVI_U32 BlkSize;

	FISHEYE_ATTR_S stFisheyeAttr = {0};

	param.size_in.u32Width = 1920;
	param.size_in.u32Height = 1080;
	param.size_out.u32Width = 1920;
	param.size_out.u32Height = 1088;
	param.enPixelFormat = PIXEL_FORMAT_NV21;

	param.identity.enModId = CVI_ID_USER;
	param.identity.u32ID = 0;
	param.identity.syncIo = CVI_FALSE;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width, param.size_in.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);
	param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width, param.size_out.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);

	BlkSize = (param.u32BlkSizeIn > param.u32BlkSizeOut) ? param.u32BlkSizeIn : param.u32BlkSizeOut;

	stVbConf.u32MaxPoolCnt				= 1;
	stVbConf.astCommPool[0].u32BlkSize	= BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 10;

	GDC_UT_PRT("common pool[0] BlkSize %d\n", BlkSize);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		GDC_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		GDC_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		GDC_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	s32Ret = CVI_GDC_Init();
	if (s32Ret != CVI_SUCCESS) {
		GDC_UT_PRT("CVI_GDC_Init failed!\n");
		goto exit1;
	}

	do {
		for (CVI_U8 i = 0; i < 2; i++) {
			strcpy(param.filename_in, filename_in[i]);
			strcpy(param.filename_out, filename_out[i]);
			param.size_in.u32Width = WidthIn[i];
			param.size_in.u32Height = HeightIn[i];
			param.size_out.u32Width = WidthOut[i];
			param.size_out.u32Height = HeightOut[i];
			param.op = op[i];

			snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_async_%d_%d", i, times);
			snprintf(param.identity.Name, sizeof(param.identity.Name), "job_async_%d_%d", i, times);

			if (i == 0)
				param.enPixelFormat = PIXEL_FORMAT_NV21;
			else
				param.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;

			param.hHandle = 0;
			memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
			s32Ret = GDCFileToFrame(&param.size_in, param.enPixelFormat, param.filename_in, &param.stVideoFrameIn, 1);
			if (s32Ret) {
				GDC_UT_PRT("GDCFileToFrame failed!\n");
				goto exit2;
			}

			memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
			s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut, 1);
			if (s32Ret) {
				GDC_UT_PRT("GDC_COMM_PrepareFrame failed!\n");
				goto exit2;
			}

			memset(param.stTask.au64privateData, 0, sizeof(param.stTask.au64privateData));
			memcpy(&param.stTask.stImgIn, &param.stVideoFrameIn, sizeof(param.stVideoFrameIn));
			memcpy(&param.stTask.stImgOut, &param.stVideoFrameOut, sizeof(param.stVideoFrameOut));

			s32Ret = CVI_GDC_BeginJob(&param.hHandle);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_BeginJob failed!\n");
				goto exit2;
			}

			s32Ret = CVI_GDC_SetJobIdentity(param.hHandle, &param.identity);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_SetJobIdentity failed!\n");
				goto exit2;
			}

			if (i == 0) {
				ptr = (void *)ROTATION_0;
			} else {
				stFisheyeAttr.bEnable = CVI_TRUE;
				stFisheyeAttr.bBgColor = CVI_TRUE;
				stFisheyeAttr.u32BgColor = YUV_8BIT(0, 128, 128);
				stFisheyeAttr.s32HorOffset = param.size_in.u32Width / 2;
				stFisheyeAttr.s32VerOffset = param.size_in.u32Height / 2;
				stFisheyeAttr.enMountMode = FISHEYE_DESKTOP_MOUNT;
				stFisheyeAttr.enUseMode = MODE_PANORAMA_360;
				stFisheyeAttr.u32RegionNum = 1;
				ptr = (void *)&stFisheyeAttr;
			}

			s32Ret = gdc_basic_add_tsk(&param, ptr);
			if (s32Ret != CVI_SUCCESS) {
				GDC_UT_PRT("gdc_basic_add_tsk. s32Ret: 0x%x !\n", s32Ret);
				goto exit2;
			}

			s32Ret = CVI_GDC_EndJob(param.hHandle);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_EndJob failed!\n");
				goto exit2;
			}

			if (needSuspend) {
				s32Ret = CVI_GDC_Suspend();
				if (s32Ret != CVI_SUCCESS) {
					GDC_UT_PRT("CVI_GDC_Suspend fail. s32Ret: 0x%x !\n", s32Ret);
					goto exit2;
				}
				s32Ret = CVI_GDC_Resume();
				if (s32Ret != CVI_SUCCESS) {
					GDC_UT_PRT("CVI_GDC_Resume fail. s32Ret: 0x%x !\n", s32Ret);
					goto exit2;
				}
			}

			usleep(1000*500);

			if ((s32Ret = CVI_GDC_GetChnFrame(&param.identity, &param.stVideoFrameOut, 5000)) != CVI_SUCCESS)
				break;

			param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
			if (param.inBlk != VB_INVALID_HANDLE)
				s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);

			param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
			if (param.outBlk != VB_INVALID_HANDLE)
				s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);

			if (s32Ret) {
				GDC_UT_PRT("release VB fail.\n");
				goto exit2;
			}
		}
	} while (times--);

exit2:
	if (s32Ret)
		if (param.hHandle)
			s32Ret |= CVI_GDC_CancelJob(param.hHandle);
	s32Ret |= CVI_GDC_DeInit();
	if (s32Ret) {
		GDC_UT_PRT("CVI_GDC_DeInit fail.\n");
	}

	if (param.stVideoFrameIn.stVFrame.u64PhyAddr[0]) {
		param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
		if (param.inBlk != VB_INVALID_HANDLE)
			s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);
	}
	if (param.stVideoFrameOut.stVFrame.u64PhyAddr[0]) {
		param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
		if (param.outBlk != VB_INVALID_HANDLE)
			s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);
	}
exit1:
	s32Ret |= CVI_SYS_Exit();
exit0:
	s32Ret |= CVI_VB_Exit();

	GDC_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_BOOL LdcAsyncDoneFlag;
void *test_gdc_async_thread(void *data)
{
	CVI_S32 s32MilliSec = 5000;
	GDC_BASIC_TEST_PARAM *param = (GDC_BASIC_TEST_PARAM *)(data);
	CVI_S32 s32Ret;

	if (!param)
		goto EXIT;

	while (!LdcAsyncDoneFlag) {
		s32Ret = CVI_GDC_GetChnFrame(&param->identity, &param->stVideoFrameOut, s32MilliSec);
		if (s32Ret) {
			usleep(1000);
			continue;
		}

		GDC_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param->stVideoFrameIn.stVFrame.u64PhyAddr[0]
			, param->stVideoFrameIn.stVFrame.u64PhyAddr[1], param->stVideoFrameIn.stVFrame.u64PhyAddr[2]);
		GDC_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param->stVideoFrameOut.stVFrame.u64PhyAddr[0]
			, param->stVideoFrameOut.stVFrame.u64PhyAddr[1], param->stVideoFrameOut.stVFrame.u64PhyAddr[2]);

		//if (g_gdc_save_file) {
		if (0) {
			s32Ret = GDCFrameSaveToFile(param->filename_out, &param->stVideoFrameOut);
			if (s32Ret) {
				GDC_UT_PRT("GDCFrameSaveToFile fail\n");
			}
		}

		param->inBlk = CVI_VB_PhysAddr2Handle(param->stVideoFrameIn.stVFrame.u64PhyAddr[0]);
		if (param->inBlk != VB_INVALID_HANDLE)
			CVI_VB_ReleaseBlock(param->inBlk);

		param->outBlk = CVI_VB_PhysAddr2Handle(param->stVideoFrameOut.stVFrame.u64PhyAddr[0]);
		if (param->outBlk != VB_INVALID_HANDLE)
			CVI_VB_ReleaseBlock(param->outBlk);
	}
EXIT:
	pthread_exit(0);
}

static CVI_S32 gdc_basic_do_job(GDC_BASIC_TEST_PARAM *param, int times, void *op_ptr)
{
	CVI_S32 s32Ret;

	do {
		param->hHandle = 0;
		memset(&param->stVideoFrameIn, 0, sizeof(param->stVideoFrameIn));
		s32Ret = GDCFileToFrame(&param->size_in, param->enPixelFormat, param->filename_in, &param->stVideoFrameIn, param->op);
		if (s32Ret) {
			GDC_UT_PRT("GDCFileToFrame failed!\n");
			return CVI_FAILURE;
		}

		memset(&param->stVideoFrameOut, 0, sizeof(param->stVideoFrameOut));
		s32Ret = GDC_COMM_PrepareFrame(&param->size_out, param->enPixelFormat, &param->stVideoFrameOut, param->op);
		if (s32Ret) {
			GDC_UT_PRT("GDC_COMM_PrepareFrame failed!\n");
			return CVI_FAILURE;
		}

		memset(param->stTask.au64privateData, 0, sizeof(param->stTask.au64privateData));
		memcpy(&param->stTask.stImgIn, &param->stVideoFrameIn, sizeof(param->stVideoFrameIn));
		memcpy(&param->stTask.stImgOut, &param->stVideoFrameOut, sizeof(param->stVideoFrameOut));

		s32Ret = CVI_GDC_BeginJob(&param->hHandle);
		if (s32Ret) {
			GDC_UT_PRT("CVI_GDC_BeginJob failed!\n");
			return CVI_FAILURE;
		}

		s32Ret = CVI_GDC_SetJobIdentity(param->hHandle, &param->identity);
		if (s32Ret) {
			GDC_UT_PRT("CVI_GDC_SetJobIdentity failed!\n");
			return CVI_FAILURE;
		}

		s32Ret = gdc_basic_add_tsk(param, op_ptr);
		if (s32Ret != CVI_SUCCESS) {
			GDC_UT_PRT("gdc_basic_add_tsk. s32Ret: 0x%x !\n", s32Ret);
			return CVI_FAILURE;
		}

		s32Ret = CVI_GDC_EndJob(param->hHandle);
		if (s32Ret) {
			GDC_UT_PRT("CVI_GDC_EndJob failed! times is %d\n", times);
			return CVI_FAILURE;
		}

		if (param->stVideoFrameIn.stVFrame.u64PhyAddr[0]) {
			param->inBlk = CVI_VB_PhysAddr2Handle(param->stVideoFrameIn.stVFrame.u64PhyAddr[0]);
			if (param->inBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param->inBlk);
				param->inBlk = VB_INVALID_HANDLE;
			}
		}
		if (param->stVideoFrameOut.stVFrame.u64PhyAddr[0]) {
			param->outBlk = CVI_VB_PhysAddr2Handle(param->stVideoFrameOut.stVFrame.u64PhyAddr[0]);
			if (param->outBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param->outBlk);
				param->outBlk = VB_INVALID_HANDLE;
			}
		}

		if (s32Ret) {
			GDC_UT_PRT("release VB fail.\n");
			break;
		}
	} while (times--);

	return s32Ret;
}

void *gdc_basic_thread_func0(void *data)
{
	CVI_S32 s32Ret;
	GDC_BASIC_TEST_PARAM *param = (GDC_BASIC_TEST_PARAM *)(data);
	int times = GDC_REPEAT_TIMES * 10;
	void *ptr = (void *)ROTATION_90;

	s32Ret = gdc_basic_do_job(param, times, ptr);
	if (s32Ret) {
		GDC_UT_PRT("test fail.\n");
	}

	if (s32Ret)
		if (param->hHandle)
			s32Ret = CVI_GDC_CancelJob(param->hHandle);

	GDC_CHECK_RET(s32Ret);

	pthread_exit(0);
}

void *gdc_basic_thread_func1(void *data)
{
	CVI_S32 s32Ret;
	GDC_BASIC_TEST_PARAM *param = (GDC_BASIC_TEST_PARAM *)(data);
	int times = GDC_REPEAT_TIMES * 10;
	void *ptr;
	LDC_ATTR_S stLdcAttr = {CVI_TRUE, 0, 0, 0, 0, 0, -200, {0}, 0};

	ptr = (void *)&stLdcAttr;

	s32Ret = gdc_basic_do_job(param, times, ptr);
	if (s32Ret) {
		GDC_UT_PRT("test fail.\n");
	}

	if (s32Ret)
		if (param->hHandle)
			s32Ret = CVI_GDC_CancelJob(param->hHandle);


	GDC_CHECK_RET(s32Ret);

	pthread_exit(0);
}
void *gdc_basic_thread_func2(void *data)
{
	CVI_S32 s32Ret;
	GDC_BASIC_TEST_PARAM *param = (GDC_BASIC_TEST_PARAM *)(data);
	int times = GDC_REPEAT_TIMES * 10;
	POINT2F_S faces[9][4] = {
		{ {.x = 722.755, .y = 65.7575}, {.x = 828.402, .y = 80.6858}, {.x = 707.827, .y = 171.405}, {.x = 813.474, .y = 186.333} },
		{ {.x = 494.919, .y = 117.918}, {.x = 605.38,  .y = 109.453}, {.x = 503.384, .y = 228.378}, {.x = 613.845, .y = 219.913} },
		{ {.x = 1509.06, .y = 147.139}, {.x = 1592.4,  .y = 193.044}, {.x = 1463.15, .y = 230.48 }, {.x = 1546.5,  .y = 276.383} },
		{ {.x = 1580.21, .y = 66.7939}, {.x = 1694.1,  .y = 70.356 }, {.x = 1576.65, .y = 180.682}, {.x = 1690.54, .y = 184.243} },
		{ {.x = 178.76,  .y = 90.4814}, {.x = 286.234, .y = 80.799 }, {.x = 188.442, .y = 197.955}, {.x = 295.916, .y = 188.273} },
		{ {.x = 1195.57, .y = 139.226}, {.x = 1292.69, .y = 104.122}, {.x = 1230.68, .y = 236.34}, {.x = 1327.79, .y = 201.236}, },
		{ {.x = 398.669, .y = 109.872}, {.x = 501.93, .y = 133.357}, {.x = 375.184, .y = 213.133}, {.x = 478.445, .y = 236.618}, },
		{ {.x = 845.989, .y = 94.591}, {.x = 949.411, .y = 63.6143}, {.x = 876.966, .y = 198.013}, {.x = 980.388, .y = 167.036}, },
		{ {.x = 1060.19, .y = 58.7882}, {.x = 1170.61, .y = 61.9105}, {.x = 1057.07, .y = 169.203}, {.x = 1167.48, .y = 172.325}, },
	};
	AFFINE_ATTR_S stAffineAttr;
	void *ptr;

	stAffineAttr.u32RegionNum = 9;
	memcpy(stAffineAttr.astRegionAttr, faces, sizeof(faces));
	stAffineAttr.stDestSize.u32Width = 112;
	stAffineAttr.stDestSize.u32Height = 112;
	ptr = (void *)&stAffineAttr;

	s32Ret = gdc_basic_do_job(param, times, ptr);
	if (s32Ret) {
		GDC_UT_PRT("test fail.\n");
	}

	if (s32Ret)
		if (param->hHandle)
			s32Ret = CVI_GDC_CancelJob(param->hHandle);

	GDC_CHECK_RET(s32Ret);

	pthread_exit(0);
}
void *gdc_basic_thread_func3(void *data)
{
	CVI_S32 s32Ret;
	GDC_BASIC_TEST_PARAM *param = (GDC_BASIC_TEST_PARAM *)(data);
	int times = GDC_REPEAT_TIMES * 10;
	void *ptr;
	FISHEYE_ATTR_S stFisheyeAttr = {0};

	stFisheyeAttr.bEnable = CVI_TRUE;
	stFisheyeAttr.bBgColor = CVI_TRUE;
	stFisheyeAttr.u32BgColor = YUV_8BIT(0, 128, 128);
	stFisheyeAttr.s32HorOffset = param->size_in.u32Width / 2;
	stFisheyeAttr.s32VerOffset = param->size_in.u32Height / 2;
	stFisheyeAttr.enMountMode = FISHEYE_DESKTOP_MOUNT;
	stFisheyeAttr.enUseMode = MODE_PANORAMA_360;
	stFisheyeAttr.u32RegionNum = 1;
	ptr = (void *)&stFisheyeAttr;

	s32Ret = gdc_basic_do_job(param, times, ptr);
	if (s32Ret) {
		GDC_UT_PRT("test fail.\n");
	}

	if (s32Ret)
		if (param->hHandle)
			s32Ret = CVI_GDC_CancelJob(param->hHandle);

	GDC_CHECK_RET(s32Ret);

	pthread_exit(0);
}

static CVI_S32 gdc_test_multi_thread(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf = {0};
	GDC_BASIC_TEST_PARAM param[4] = {0};
	char *filename_in[4] = {GDC_FILE_IN_ROT_2, DWA_FILE_IN_LDC_BARREL_0P3, GDC_FILE_IN_AFFINE, GDC_FILE_IN_FISHEYE};
	char *filename_out[4] = {GDC_FILE_OUT_ROT0_2, DWA_FILE_OUT_LDC_BARREL_0P3_0, GDC_FILE_OUT_AFFINE, GDC_FILE_OUT_FISHEYE_PANORAMA};
	GDC_TEST_OP op[4] = {GDC_TEST_ROT, GDC_TEST_LDC, GDC_TEST_AFFINE, GDC_TEST_FISHEYE};
	CVI_U32 WidthIn[4] = {4608, 1920, 1920, 1024};
	CVI_U32 HeightIn[4] = {4608, 1080, 1080, 1024};
	CVI_U32 WidthOut[4] = {4608, 1920, 128, 1280};
	CVI_U32 HeightOut[4] = {4608, 1080, 1280, 720};
	int i, rc;
	pthread_t thread[4] = {[0 ... 3] = -1};

	param[0].size_in.u32Width = 4608;
	param[0].size_in.u32Height = 4608;
	param[0].size_out.u32Width = 4608;
	param[0].size_out.u32Height = 4608;
	param[0].enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	param[0].u32BlkSizeIn = COMMON_GetPicBufferSize(ALIGN(param[0].size_in.u32Width, GDC_STRIDE_ALIGN)
		, ALIGN(param[0].size_in.u32Height, GDC_STRIDE_ALIGN), PIXEL_FORMAT_YUV_PLANAR_444
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);
	param[0].u32BlkSizeOut = COMMON_GetPicBufferSize(ALIGN(param[0].size_in.u32Width, GDC_STRIDE_ALIGN)
		, ALIGN(param[0].size_in.u32Height, GDC_STRIDE_ALIGN), PIXEL_FORMAT_YUV_PLANAR_444
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);

	stVbConf.u32MaxPoolCnt              = 2;
	stVbConf.astCommPool[0].u32BlkSize	= param[0].u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 10;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize	= param[0].u32BlkSizeOut;
	stVbConf.astCommPool[1].u32BlkCnt	= 10;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	GDC_UT_PRT("common pool[0] BlkSize %d\n", param[0].u32BlkSizeIn);
	GDC_UT_PRT("common pool[1] BlkSize %d\n", param[0].u32BlkSizeOut);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		GDC_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		GDC_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		GDC_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	s32Ret = CVI_GDC_Init();
	if (s32Ret != CVI_SUCCESS) {
		GDC_UT_PRT("CVI_GDC_Init failed!\n");
		goto exit1;
	}

	for (i = 0; i < 4; i++) {
		param[i].enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
		if (i == 0)
			param[i].enPixelFormat = PIXEL_FORMAT_NV21;
		param[i].identity.syncIo = CVI_TRUE;
		strcpy(param[i].filename_in, filename_in[i]);
		strcpy(param[i].filename_out, filename_out[i]);
		param[i].size_in.u32Width = WidthIn[i];
		param[i].size_in.u32Height = HeightIn[i];
		param[i].size_out.u32Width = WidthOut[i];
		param[i].size_out.u32Height = HeightOut[i];
		param[i].op = op[i];
		snprintf(param[i].stTask.name, sizeof(param[i].stTask.name), "tsk_multi_th_%d", i);
		param[i].identity.enModId = CVI_ID_USER;
		param[i].identity.u32ID = i;
		snprintf(param[i].identity.Name, sizeof(param[i].identity.Name), "job_multi_th_%d", i);
		if (i == 0)
			rc = pthread_create(&thread[i], NULL, gdc_basic_thread_func0, (void *)&param[i]);
		else if (i == 1)
			rc = pthread_create(&thread[i], NULL, gdc_basic_thread_func1, (void *)&param[i]);
		else if (i == 2)
			rc = pthread_create(&thread[i], NULL, gdc_basic_thread_func2, (void *)&param[i]);
		else
			rc = pthread_create(&thread[i], NULL, gdc_basic_thread_func3, (void *)&param[i]);

		if (rc) {
			GDC_UT_PRT("pthread_create fail. s32Ret: 0x%x !\n", s32Ret);
			break;
		}
	}

	for (i = 0; i < 4; i++)
		pthread_join(thread[i], NULL);

	s32Ret |= CVI_GDC_DeInit();
	if (s32Ret) {
		GDC_UT_PRT("CVI_GDC_DeInit failed!\n");
	}
exit1:
	s32Ret |= CVI_SYS_Exit();
exit0:
	s32Ret |= CVI_VB_Exit();

	GDC_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_ldc_grid_info(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	LDC_ATTR_S stLdcAttr = {0};
	GDC_BASIC_TEST_PARAM param = {0};
	char *filename_in[2] = {DWA_FILE_IN_LDC_GRID_INFO_L, DWA_FILE_IN_LDC_GRID_INFO_R};
	char *filename_out[2] = {DWA_FILE_OUT_LDC_GRID_INFO_L, DWA_FILE_OUT_LDC_GRID_INFO_R};
	char *filename_grid[2] = {DWA_FILE_IN_LDC_GRID_L, DWA_FILE_IN_LDC_GRID_R};

	for (int i = 0; i < 2; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);

		param.size_in.u32Width = 1280;
		param.size_in.u32Height = 720;
		param.size_out.u32Width = 1280;
		param.size_out.u32Height = 720;

		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_gdc_grid_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = 0;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_gdc_grid__%d", i);
		param.identity.syncIo = CVI_TRUE;

		param.enPixelFormat = PIXEL_FORMAT_YUV_400;
		param.op = GDC_TEST_LDC;
		stLdcAttr.stGridInfoAttr.Enable = CVI_TRUE;
		strcpy(stLdcAttr.stGridInfoAttr.gridFileName, filename_grid[i]);
		strcpy(stLdcAttr.stGridInfoAttr.gridBindName, param.stTask.name);

		s32Ret |= gdc_basic(&param, (void *)&stLdcAttr);
		if (s32Ret != CVI_SUCCESS) {
			GDC_UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	GDC_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_dis(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf = {0};
	LDC_ATTR_S stLdcAttr[3] = {0};
	GDC_BASIC_TEST_PARAM param[3] = {0};
	char *filename_in[3] = {DWA_FILE_IN_DIS0, DWA_FILE_IN_DIS1, DWA_FILE_IN_DIS2};
	char *filename_out[3] = {DWA_FILE_OUT_DIS0, DWA_FILE_OUT_DIS1, DWA_FILE_OUT_DIS2};
	char *filename_grid = DWA_FILE_IN_DIS_GRID;
	char *src_mesh_name[3] = {DWA_FILE_SRC_MESH_0, DWA_FILE_SRC_MESH_1, DWA_FILE_SRC_MESH_2};
	int times = 0, i;
	int num_mesh = 4888;
	int src_x_mesh[num_mesh * 4][4], src_y_mesh[num_mesh * 4][4];

	for (i = 0; i < 3; i++) {
		strcpy(param[i].filename_in, filename_in[i]);
		strcpy(param[i].filename_out, filename_out[i]);
		param[i].size_in.u32Width = 3840;
		param[i].size_in.u32Height = 2160;
		param[i].size_out.u32Width = 1920;
		param[i].size_out.u32Height = 1080;
		param[i].enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
		snprintf(param[i].stTask.name, sizeof(param[i].stTask.name), "gdc_tsk_dis_grid_%d", i);
		param[i].identity.enModId = CVI_ID_USER;
		param[i].identity.u32ID = 0;
		snprintf(param[i].identity.Name, sizeof(param[i].identity.Name), "gdc_job_dis_grid%d", i);
		param[i].identity.syncIo = CVI_TRUE;

		param[i].op = GDC_TEST_LDC;
		stLdcAttr[i].stGridInfoAttr.Enable = CVI_TRUE;
		strcpy(stLdcAttr[i].stGridInfoAttr.gridFileName, filename_grid);
		strcpy(stLdcAttr[i].stGridInfoAttr.gridBindName, param[i].stTask.name);
		stLdcAttr[i].stGridInfoAttr.grid_in.u32Width = param[i].size_in.u32Width;
		stLdcAttr[i].stGridInfoAttr.grid_in.u32Height = param[i].size_in.u32Height;
		stLdcAttr[i].stGridInfoAttr.grid_out.u32Width = param[i].size_out.u32Width;
		stLdcAttr[i].stGridInfoAttr.grid_out.u32Height = param[i].size_out.u32Height;
		stLdcAttr[i].stGridInfoAttr.bEISEnable = CVI_TRUE;
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	param[0].u32BlkSizeIn = COMMON_GetPicBufferSize(param[0].size_in.u32Width, param[0].size_in.u32Height, param[0].enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);
	param[0].u32BlkSizeOut = COMMON_GetPicBufferSize(param[0].size_out.u32Width, param[0].size_out.u32Height, param[0].enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);

	stVbConf.u32MaxPoolCnt				= 2;
	stVbConf.astCommPool[0].u32BlkSize	= param[0].u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 1;
	stVbConf.astCommPool[0].enRemapMode = VB_REMAP_MODE_CACHED;
	GDC_UT_PRT("common pool[0] BlkSize %d\n", param[0].u32BlkSizeIn);

	stVbConf.astCommPool[1].u32BlkSize	= param[0].u32BlkSizeOut;
	stVbConf.astCommPool[1].u32BlkCnt	= 1;
	stVbConf.astCommPool[1].enRemapMode = VB_REMAP_MODE_CACHED;
	GDC_UT_PRT("common pool[1] BlkSize %d\n", param[0].u32BlkSizeOut);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		GDC_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		GDC_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		GDC_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	s32Ret = CVI_GDC_Init();
	if (s32Ret != CVI_SUCCESS) {
		GDC_UT_PRT("CVI_GDC_Init failed!\n");
		goto exit1;
	}

	for (i = 0; i < 3; i++) {
		times = 0;
		do {
			param[i].hHandle = 0;
			memset(&param[i].stVideoFrameIn, 0, sizeof(param[i].stVideoFrameIn));
			s32Ret = GDCFileToFrame(&param[i].size_in, param[i].enPixelFormat, param[i].filename_in, &param[i].stVideoFrameIn, 0);
			if (s32Ret) {
				GDC_UT_PRT("GDCFileToFrame failed!\n");
				goto exit2;
			}
			memset(&param[i].stVideoFrameOut, 0, sizeof(param[i].stVideoFrameOut));
			s32Ret = GDC_COMM_PrepareFrame(&param[i].size_out, param[i].enPixelFormat, &param[i].stVideoFrameOut, 0);
			if (s32Ret) {
				GDC_UT_PRT("GDC_COMM_PrepareFrame failed!\n");
				goto exit2;
			}

			memset(param[i].stTask.au64privateData, 0, sizeof(param[i].stTask.au64privateData));
			memcpy(&param[i].stTask.stImgIn, &param[i].stVideoFrameIn, sizeof(param[i].stVideoFrameIn));
			memcpy(&param[i].stTask.stImgOut, &param[i].stVideoFrameOut, sizeof(param[i].stVideoFrameOut));

			s32Ret = CVI_GDC_BeginJob(&param[i].hHandle);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_BeginJob failed!\n");
				goto exit2;
			}

			s32Ret = CVI_GDC_SetJobIdentity(param[i].hHandle, &param[i].identity);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_SetJobIdentity failed!\n");
				goto exit2;
			}

			s32Ret = CVI_GDC_AddLDCTask(param[i].hHandle, &param[i].stTask, &stLdcAttr[i], ROTATION_0);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_AddLDCTask failed!\n");
				goto exit2;
			}

			s32Ret = gdc_load_src_mesh_coordinate(src_mesh_name[i], src_x_mesh, src_y_mesh);
			if (s32Ret) {
				GDC_UT_PRT("gdc_load_src_mesh_coordinate failed!\n");
				goto exit2;
			}

			s32Ret = CVI_GDC_UpdateMeshCoordinate(stLdcAttr[i].stGridInfoAttr.gridBindName, src_x_mesh, src_y_mesh, NULL, NULL, num_mesh);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_AddLDCTask failed!\n");
				goto exit2;
			}

			CVI_SYS_IonFlushCache(param[i].stTask.au64privateData[0], (void *)param[i].stTask.au64privateData[1], 0x150000);

#define DUMP_MESH_COORDINATE 0
#if DUMP_MESH_COORDINATE
			char filename[128];
			FILE *fpDbg;

			snprintf(filename, 128, "reorder_mesh_tbl_%d.bin", i);
			fpDbg = fopen(filename, "wb");
			fwrite((void *)(param[i].stTask.au64privateData[1] + 0x50000), sizeof(int), 325632/sizeof(int), fpDbg);
			fclose(fpDbg);
#endif
			s32Ret = CVI_GDC_EndJob(param[i].hHandle);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_EndJob failed!\n");
				goto exit2;
			}

			if (g_gdc_save_file) {
				s32Ret = GDCFrameSaveToFile(param[i].filename_out, &param[i].stVideoFrameOut);
				if (s32Ret != CVI_SUCCESS) {
					GDC_UT_PRT("GDCFrameSaveToFile. s32Ret: 0x%x !\n", s32Ret);
					goto exit2;
				}
				GDC_UT_PRT("output file:%s\n", param[i].filename_out);
			}

			if (param[i].stVideoFrameIn.stVFrame.u64PhyAddr[0]) {
				param[i].inBlk = CVI_VB_PhysAddr2Handle(param[i].stVideoFrameIn.stVFrame.u64PhyAddr[0]);
				if (param[i].inBlk != VB_INVALID_HANDLE) {
					s32Ret |= CVI_VB_ReleaseBlock(param[i].inBlk);
					param[i].inBlk = VB_INVALID_HANDLE;
				}
			}
			if (param[i].stVideoFrameOut.stVFrame.u64PhyAddr[0]) {
				param[i].outBlk = CVI_VB_PhysAddr2Handle(param[i].stVideoFrameOut.stVFrame.u64PhyAddr[0]);
				if (param[i].outBlk != VB_INVALID_HANDLE) {
					s32Ret |= CVI_VB_ReleaseBlock(param[i].outBlk);
					param[i].outBlk = VB_INVALID_HANDLE;
				}
			}

			if (s32Ret) {
				GDC_UT_PRT("release VB fail.\n");
				break;
			}
		} while (times--);
	}
exit2:
	s32Ret |= CVI_GDC_DeInit();
exit1:
	s32Ret |= CVI_SYS_Exit();
exit0:
	s32Ret |= CVI_VB_Exit();

	GDC_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 _gdc_handle_op(CVI_S32 op)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	switch (op) {
	case GDC_TEST_ROT:
		s32Ret = gdc_test_rot();
		break;
	case GDC_TEST_LDC:
		s32Ret = gdc_test_ldc();
		break;
	case GDC_TEST_FISHEYE:
		s32Ret = gdc_test_fisheye();
		break;
	case GDC_TEST_AFFINE:
		s32Ret = gdc_test_affine();
		break;
	case GDC_TEST_FMT:
		s32Ret = gdc_test_fmt();
		break;
	case GDC_TEST_SIZE_NO_ALIGN:
		s32Ret = gdc_test_not_align();
		break;
	case GDC_TEST_CMDQ:
		s32Ret = gdc_test_cmdq();
		break;
	case GDC_TEST_CMDQ_1TO2:
		s32Ret = gdc_test_cmdq_1to2();
		break;
	case GDC_TEST_ASYNC:
		s32Ret = gdc_test_async();
		break;
	case GDC_TEST_MULTI_THREAD:
		s32Ret = gdc_test_multi_thread();
		break;
	case GDC_TEST_LOAD_GRID_INFO_LDC:
		s32Ret = gdc_test_ldc_grid_info();
		break;
	case GDC_TEST_RST:
		s32Ret = gdc_test_reset();
		break;
	case GDC_TEST_DIS:
		s32Ret = gdc_test_dis();
		break;
	default:
		s32Ret = CVI_FAILURE;
		break;
	}

	return s32Ret;
}

static void gdc_show_help(void)
{
	GDC_UT_PRT("%4d: gdc basic test rot\n", GDC_TEST_ROT);
	GDC_UT_PRT("%4d: gdc basic test ldc\n", GDC_TEST_LDC);
	GDC_UT_PRT("%4d: gdc basic test fisheye\n", GDC_TEST_FISHEYE);
	GDC_UT_PRT("%4d: gdc basic test affine\n", GDC_TEST_AFFINE);
	GDC_UT_PRT("%4d: gdc basic test fmt\n", GDC_TEST_FMT);
	GDC_UT_PRT("%4d: gdc basic test size not align\n", GDC_TEST_SIZE_NO_ALIGN);
	GDC_UT_PRT("%4d: gdc basic test cmdq\n", GDC_TEST_CMDQ);
	GDC_UT_PRT("%4d: gdc basic test cmdq_1to2\n", GDC_TEST_CMDQ_1TO2);
	GDC_UT_PRT("%4d: gdc basic test async\n", GDC_TEST_ASYNC);
	GDC_UT_PRT("%4d: gdc basic test multi thread\n", GDC_TEST_MULTI_THREAD);
	GDC_UT_PRT("%4d: gdc basic test grid_info_ldc\n", GDC_TEST_LOAD_GRID_INFO_LDC);
	GDC_UT_PRT("%4d: gdc basic test reset\n", GDC_TEST_RST);
	GDC_UT_PRT("%4d: gdc basic test dis\n", GDC_TEST_DIS);
	GDC_UT_PRT("255: exit\n");
}

int main(int argc, char **argv)
{
	CVI_S32 s32Ret;
	CVI_S32 op = 255;

	system("stty erase ^H");

	signal(SIGINT, gdc_ut_HandleSig);
	signal(SIGTERM, gdc_ut_HandleSig);

	if (argc >= 2) {
		op = (CVI_S32)atoi(argv[1]);
		g_gdc_save_file = (CVI_BOOL)atoi(argv[2]);

		s32Ret = _gdc_handle_op(op);
		GDC_UT_PRT("gdc ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	} else {
		g_gdc_save_file = CVI_TRUE;
		do {
			gdc_show_help();
			scanf("%d", &op);

			s32Ret = _gdc_handle_op(op);
			if (op != 255)
				GDC_UT_PRT("gdc ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
		} while (op != 255);
	}

	return s32Ret;
}

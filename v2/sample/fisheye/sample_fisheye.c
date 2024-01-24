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
#include "cvi_dwa.h"
#include "sample_fisheye_comm.h"

#ifndef BIT
#define BIT(nr)      (UINT64_C(1) << (nr))
#endif

//file: http://disk-sophgo-vip.quickconnect.cn/sharing/iglxSGiJB
#define DWA_FILE_IN_FISHEYE                     "res/input/fisheye_floor_1024x1024.yuv"
#define DWA_FILE_OUT_FISHEYE_PANORAMA           "res/output/fisheye_floor_panorama360_1280x720.yuv"
#define DWA_FILE_OUT_FISHEYE_4R                 "res/output/fisheye_floor_4R_1280x720.yuv"
#define DWA_FILE_PEF_FISHEYE_PANORAMA           "res/pef/fisheye_floor_panorama360_1280x720.yuv"
#define DWA_FILE_PEF_FISHEYE_4R                 "res/pef/fisheye_floor_4R_1280x720.yuv"

#define DWA_FILE_IN_ROT                         "res/input/1920x1080.yuv"
#define DWA_FILE_OUT_ROT0                       "res/output/1920x1080_rot0.yuv"
#define DWA_FILE_OUT_ROT90                      "res/output/1920x1080_rot90.yuv"
#define DWA_FILE_OUT_ROT180                     "res/output/1920x1080_rot180.yuv"
#define DWA_FILE_OUT_ROT270                     "res/output/1920x1080_rot270.yuv"
#define DWA_FILE_PEF_ROT0                       "res/pef/1920x1080_rot0.yuv"
#define DWA_FILE_PEF_ROT90                      "res/pef/1920x1080_rot90.yuv"
#define DWA_FILE_PEF_ROT180                     "res/pef/1920x1080_rot180.yuv"
#define DWA_FILE_PEF_ROT270                     "res/pef/1920x1080_rot270.yuv"
#define DWA_FILE_IN_ROT_4M                      "res/input/2560x1440.yuv"
#define DWA_FILE_OUT_ROT0_4M                    "res/output/2560x1440_rot0.yuv"
#define DWA_FILE_OUT_ROT90_4M                   "res/output/2560x1440_rot90.yuv"
#define DWA_FILE_OUT_ROT180_4M                  "res/output/2560x1440_rot180.yuv"
#define DWA_FILE_OUT_ROT270_4M                  "res/output/2560x1440_rot270.yuv"
#define DWA_FILE_PEF_ROT0_4M                    "res/pef/2560x1440_rot0.yuv"
#define DWA_FILE_PEF_ROT90_4M                   "res/pef/2560x1440_rot90.yuv"
#define DWA_FILE_PEF_ROT180_4M                  "res/pef/2560x1440_rot180.yuv"
#define DWA_FILE_PEF_ROT270_4M                  "res/pef/2560x1440_rot270.yuv"

#define DWA_FILE_IN_ROT_1                         "res/input/128x128.yuv"
#define DWA_FILE_OUT_ROT0_1                       "res/output/128x128_rot0.yuv"
#define DWA_FILE_PEF_ROT0_1                       "res/pef/128x128_rot0.yuv"
#define DWA_FILE_OUT_ROT90_1                       "res/output/128x128_rot90.yuv"
#define DWA_FILE_PEF_ROT90_1                       "res/pef/128x128_rot90.yuv"
#define DWA_FILE_OUT_ROT180_1                       "res/output/128x128_rot180.yuv"
#define DWA_FILE_PEF_ROT180_1                       "res/pef/128x128_rot180.yuv"
#define DWA_FILE_OUT_ROT270_1                       "res/output/128x128_rot270.yuv"
#define DWA_FILE_PEF_ROT270_1                       "res/pef/128x128_rot270.yuv"

#define DWA_FILE_IN_ROT_2                         "res/input/4096x4096.yuv"
#define DWA_FILE_OUT_ROT0_2                       "res/output/4096x4096_rot0.yuv"
#define DWA_FILE_PEF_ROT0_2                       "res/pef/4096x4096_rot0.yuv"
#define DWA_FILE_OUT_ROT90_2                       "res/output/4096x4096_rot90.yuv"
#define DWA_FILE_PEF_ROT90_2                       "res/pef/4096x4096_rot90.yuv"
#define DWA_FILE_OUT_ROT180_2                       "res/output/4096x4096_rot180.yuv"
#define DWA_FILE_PEF_ROT180_2                       "res/pef/4096x4096_rot180.yuv"
#define DWA_FILE_OUT_ROT270_2                       "res/output/4096x4096_rot270.yuv"
#define DWA_FILE_PEF_ROT270_2                       "res/pef/4096x4096_rot270.yuv"

#define DWA_FILE_IN_LDC_BARREL_0P3              "res/input/1920x1080_barrel_0.3.yuv"
#define DWA_FILE_OUT_LDC_BARREL_0P3_0           "res/output/1920x1080_barrel_0.3_r0_ofst_0_0_d-200.yuv"
#define DWA_FILE_OUT_LDC_BARREL_0P3_1           "res/output/1920x1080_barrel_0.3_r0_ofst_0_0_d-200_2048x1280.yuv"
#define DWA_FILE_OUT_LDC_BARREL_0P3_2           "res/output/1920x1080_barrel_0.3_r100_ofst_0_0_d-200.yuv"
#define DWA_FILE_OUT_LDC_BARREL_0P3_3           "res/output/1920x1080_barrel_0.3_r100_ofst_0_0_d-200_2048x1280.yuv"
#define DWA_FILE_PEF_LDC_BARREL_0P3_0           "res/pef/1920x1080_barrel_0.3_r0_ofst_0_0_d-200.yuv"
#define DWA_FILE_PEF_LDC_BARREL_0P3_1           "res/pef/1920x1080_barrel_0.3_r0_ofst_0_0_d-200_2048x1280.yuv"
#define DWA_FILE_PEF_LDC_BARREL_0P3_2           "res/pef/1920x1080_barrel_0.3_r100_ofst_0_0_d-200.yuv"
#define DWA_FILE_PEF_LDC_BARREL_0P3_3           "res/pef/1920x1080_barrel_0.3_r100_ofst_0_0_d-200_2048x1280.yuv"
#define DWA_FILE_IN_LDC_PINCUSHION_0P3          "res/input/1920x1080_pincushion_0.3.yuv"
#define DWA_FILE_OUT_LDC_PINCUSHION_0P3_0       "res/output/1920x1080_pincushion_0.3_r0_ofst_0_0_d400.yuv"
#define DWA_FILE_OUT_LDC_PINCUSHION_0P3_1       "res/output/1920x1080_pincushion_0.3_r0_ofst_0_0_d400_2048x1280.yuv"
#define DWA_FILE_OUT_LDC_PINCUSHION_0P3_2       "res/output/1920x1080_pincushion_0.3_r100_ofst_0_0_d400.yuv"
#define DWA_FILE_OUT_LDC_PINCUSHION_0P3_3       "res/output/1920x1080_pincushion_0.3_r100_ofst_0_0_d400_2048x1280.yuv"
#define DWA_FILE_PEF_LDC_PINCUSHION_0P3_0       "res/pef/1920x1080_pincushion_0.3_r0_ofst_0_0_d400.yuv"
#define DWA_FILE_PEF_LDC_PINCUSHION_0P3_1       "res/pef/1920x1080_pincushion_0.3_r0_ofst_0_0_d400_2048x1280.yuv"
#define DWA_FILE_PEF_LDC_PINCUSHION_0P3_2       "res/pef/1920x1080_pincushion_0.3_r100_ofst_0_0_d400.yuv"
#define DWA_FILE_PEF_LDC_PINCUSHION_0P3_3       "res/pef/1920x1080_pincushion_0.3_r100_ofst_0_0_d400_2048x1280.yuv"

#define DWA_FILE_IN_AFFINE                      "res/input/girls_1920x1080.yuv"
#define DWA_FILE_OUT_AFFINE                     "res/output/girls_affine_128x1280.yuv"
#define DWA_FILE_PEF_AFFINE                     "res/pef/girls_affine_128x1280.yuv"

#define DWA_FILE_IN_FMT_0                      "res/input/1920x1080.yuv"
#define DWA_FILE_OUT_FMT_0                     "res/output/1920x1080.yuv"
#define DWA_FILE_PEF_FMT_0                     "res/pef/1920x1080.yuv"
#define DWA_FILE_IN_FMT_1                      "res/input/1920x1080_yonly.yuv"
#define DWA_FILE_OUT_FMT_1                     "res/output/1920x1080_yonly.yuv"
#define DWA_FILE_PEF_FMT_1                     "res/pef/1920x1080_yonly.yuv"
#define DWA_FILE_IN_FMT_2                      "res/input/1920x1080_yuv444planner.yuv"
#define DWA_FILE_OUT_FMT_2                     "res/output/1920x1080_yuv444planner.yuv"
#define DWA_FILE_PEF_FMT_2                     "res/pef/1920x1080_yuv444planner.yuv"
#define DWA_FILE_IN_FMT_3                      "res/input/1920x1080_rgb888planner.bin"
#define DWA_FILE_OUT_FMT_3                     "res/output/1920x1080_rgb888planner.bin"
#define DWA_FILE_PEF_FMT_3                     "res/pef/1920x1080_rgb888planner.bin"

#define DWA_FILE_IN_NOT_ALIGN                  "res/input/666x666_yuv400.yuv"
#define DWA_FILE_OUT_NOT_ALIGN                 "res/output/666x666_yuv400.yuv"

#define DWA_FILE_IN_CMDQ                       "res/input/1920x1080.yuv"
#define DWA_FILE_OUT_CMDQ                      "res/output/1920x1080_cmdq.yuv"
#define DWA_FILE_PEF_CMDQ                      "res/pef/1920x1080_cmdq.yuv"
#define DWA_FILE_IN_CMDQ_1TO2                  "res/input/1920x1080.yuv"
#define DWA_FILE_OUT_CMDQ_1TO2_0               "res/output/1920x1080_cmdq_1to2_0.yuv"
#define DWA_FILE_PEF_CMDQ_1TO2_0               "res/pef/1920x1080_cmdq_1to2_0.yuv"
#define DWA_FILE_OUT_CMDQ_1TO2_1               "res/output/1920x1080_cmdq_1to2_1.yuv"
#define DWA_FILE_PEF_CMDQ_1TO2_1               "res/pef/1920x1080_cmdq_1to2_1.yuv"
#define DWA_FILE_IN_CMDQ_1TO2_MAX              "res/input/4096x4096.yuv"
#define DWA_FILE_OUT_CMDQ_1TO2_0_MAX           "res/output/4096x4096_cmdq_1to2_0.yuv"
#define DWA_FILE_PEF_CMDQ_1TO2_0_MAX           "res/pef/4096x4096_cmdq_1to2_0.yuv"
#define DWA_FILE_OUT_CMDQ_1TO2_1_MAX           "res/output/4096x4096_cmdq_1to2_1.yuv"
#define DWA_FILE_PEF_CMDQ_1TO2_1_MAX           "res/pef/4096x4096_cmdq_1to2_1.yuv"

#define DWA_FILE_IN_LDC_GRID_INFO_L             "res/input/imgL_1280X720.yonly.yuv"
#define DWA_FILE_OUT_LDC_GRID_INFO_L            "res/output/imgL_1280x720.yonly.yuv"
#define DWA_FILE_PEF_LDC_GRID_INFO_L            "res/pef/imgL_1280x720.yonly.yuv"
#define DWA_FILE_IN_LDC_GRID_L                  "res/input/grid_info_79_43_3397_80_45_1280x720.dat"
#define DWA_FILE_IN_LDC_GRID_INFO_R             "res/input/imgR_1280X720.yonly.yuv"
#define DWA_FILE_OUT_LDC_GRID_INFO_R            "res/output/imgR_1280x720.yonly.yuv"
#define DWA_FILE_PEF_LDC_GRID_INFO_R            "res/pef/imgR_1280x720.yonly.yuv"
#define DWA_FILE_IN_LDC_GRID_R                  "res/input/grid_info_79_44_3476_80_45_1280x720.dat"

#define DWA_MAX_W    4096
#define DWA_MAX_H    4096
#define DWA_MIN_W    32
#define DWA_MIN_H    32

#define MAX_FUNC_CNT 100

#ifndef FPGA_PORTING
#define DWA_REPEAT_TIMES 6
#else
#define DWA_REPEAT_TIMES 2
#endif

typedef CVI_S32 (*p_func)(void);

static CVI_BOOL bEnProc;
static CVI_BOOL g_dwa_save_file;

typedef enum _DWA_TEST_OP {
	DWA_TEST_FISHEYE = 0,
	DWA_TEST_ROT,
	DWA_TEST_ROT1,
	DWA_TEST_ROT2,
	DWA_TEST_LDC,
	DWA_TEST_AFFINE,
	DWA_TEST_MAX_SIZE,
	DWA_TEST_FMT,
	DWA_TEST_SIZE_NO_ALIGN,
	DWA_TEST_CMDQ,
	DWA_TEST_CMDQ_1TO2,
	DWA_TEST_CMDQ_1TO2_MAX,
	DWA_TEST_ONLINE,
	DWA_TEST_MIX,
	DWA_TEST_ASYNC,
	DWA_TEST_MULTI_THREAD,
	DWA_TEST_PEF,
	DWA_TEST_LDC_GRID_INFO,
	DWA_TEST_RST,
	DWA_TEST_PRESURE_SIZE_FOR_EACH = 98,
	DWA_TEST_AUTO_REGRESSION = 99,
	DWA_TEST_USER_CONFIG = 100,
	DWA_TEST_DUP_FD,
	DWA_TEST_RST_FD,
} DWA_TEST_OP;

typedef struct _DWA_BASIC_TEST_PARAM {
	SIZE_S size_in;
	SIZE_S size_out;
	char filename_in[128];
	char filename_out[128];
	char filename_pef[128];
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	VIDEO_FRAME_INFO_S stVideoFrameIn;
	VIDEO_FRAME_INFO_S stVideoFrameOut;
	VB_BLK inBlk, outBlk;
	PIXEL_FORMAT_E enPixelFormat;
	GDC_HANDLE hHandle;
	GDC_TASK_ATTR_S stTask;
	GDC_IDENTITY_ATTR_S identity;
	DWA_TEST_OP op;
	CVI_BOOL needPef;
} DWA_BASIC_TEST_PARAM;

void dwa_ut_HandleSig(CVI_S32 signo)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	if (SIGINT == signo || SIGTERM == signo) {
		CVI_SYS_Exit();
		CVI_VB_Exit();
		DWA_UT_PRT("Program termination abnormally\n");
	}
	exit(-1);
}

static CVI_S32 dwa_basic_add_tsk(DWA_BASIC_TEST_PARAM *param, void *ptr)
{
	FISHEYE_ATTR_S *FisheyeAttr;
	AFFINE_ATTR_S *affineAttr;
	LDC_ATTR_S *LDCAttr;
	ROTATION_E enRotation;

	CVI_S32 s32Ret = CVI_FAILURE;

	if (!param) {
		DWA_UT_PRT("dwa_basic fail, null ptr for test param\n");
		return CVI_FAILURE;
	}

	switch (param->op) {
	case DWA_TEST_FISHEYE:
		FisheyeAttr = (FISHEYE_ATTR_S *)ptr;

		s32Ret = CVI_DWA_AddCorrectionTask(param->hHandle, &param->stTask, FisheyeAttr);
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_AddCorrectionTask failed!\n");
		}
		break;
	case DWA_TEST_ROT:
		enRotation =(ROTATION_E)(uintptr_t)ptr;

		s32Ret = CVI_DWA_AddRotationTask(param->hHandle, &param->stTask, enRotation);
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_AddRotationTask failed!\n");
		}
		break;
	case DWA_TEST_LDC:
		LDCAttr =(LDC_ATTR_S *)ptr;
		enRotation = (ROTATION_E)param->stTask.reserved;

		s32Ret = CVI_DWA_AddLDCTask(param->hHandle, &param->stTask, LDCAttr, enRotation);
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_AddLDCTask failed!\n");
		}
		break;
	case DWA_TEST_AFFINE:
		affineAttr =(AFFINE_ATTR_S *)ptr;

		s32Ret = CVI_DWA_AddAffineTask(param->hHandle, &param->stTask, affineAttr);
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_AddAffineTask failed!\n");
		}
		break;
	default:
		DWA_UT_PRT("not allow this op(%d) fail\n", param->op);
		break;
	}

	return s32Ret;
}

static CVI_S32 dwa_basic(DWA_BASIC_TEST_PARAM *param, void *ptr)
{
	int times = DWA_REPEAT_TIMES;
	VB_CONFIG_S stVbConf;
	CVI_S32 s32Ret;

	if (!param) {
		DWA_UT_PRT("dwa_basic fail, null ptr for test param\n");
		return CVI_FAILURE;
	}

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	param->u32BlkSizeIn = COMMON_GetPicBufferSize(param->size_in.u32Width, param->size_in.u32Height, param->enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);
	param->u32BlkSizeOut = COMMON_GetPicBufferSize(param->size_out.u32Width, param->size_out.u32Height, param->enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);

	stVbConf.u32MaxPoolCnt              = 2;
	stVbConf.astCommPool[0].u32BlkSize	= param->u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 5;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize	= param->u32BlkSizeOut;
	stVbConf.astCommPool[1].u32BlkCnt	= 5;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	DWA_UT_PRT("common pool[0] BlkSize %d\n", param->u32BlkSizeIn);
	DWA_UT_PRT("common pool[1] BlkSize %d\n", param->u32BlkSizeOut);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init DWA
	 ************************************************/
	s32Ret = CVI_DWA_Init();
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_DWA_Init failed!\n");
		goto exit1;
	}

	do {
		param->hHandle = 0;
		memset(&param->stVideoFrameIn, 0, sizeof(param->stVideoFrameIn));
		s32Ret = DWAFileToFrame(&param->size_in, param->enPixelFormat, param->filename_in, &param->stVideoFrameIn);
		if (s32Ret) {
			DWA_UT_PRT("DWAFileToFrame failed!\n");
			goto exit2;
		}

		memset(&param->stVideoFrameOut, 0, sizeof(param->stVideoFrameOut));
		s32Ret = DWA_COMM_PrepareFrame(&param->size_out, param->enPixelFormat, &param->stVideoFrameOut);
		if (s32Ret) {
			DWA_UT_PRT("DWA_COMM_PrepareFrame failed!\n");
			goto exit2;
		}

		memset(param->stTask.au64privateData, 0, sizeof(param->stTask.au64privateData));
		memcpy(&param->stTask.stImgIn, &param->stVideoFrameIn, sizeof(param->stVideoFrameIn));
		memcpy(&param->stTask.stImgOut, &param->stVideoFrameOut, sizeof(param->stVideoFrameOut));

		s32Ret = CVI_DWA_BeginJob(&param->hHandle);
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_BeginJob failed!\n");
			goto exit2;
		}

		s32Ret = CVI_DWA_SetJobIdentity(param->hHandle, &param->identity);
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_SetJobIdentity failed!\n");
			goto exit2;
		}

		s32Ret = dwa_basic_add_tsk(param, ptr);
		if (s32Ret != CVI_SUCCESS) {
			DWA_UT_PRT("dwa_basic_add_tsk. s32Ret: 0x%x !\n", s32Ret);
			goto exit2;
		}

		s32Ret = CVI_DWA_EndJob(param->hHandle);
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_EndJob failed!\n");
			goto exit2;
		}

		DWA_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param->stVideoFrameIn.stVFrame.u64PhyAddr[0]
			, param->stVideoFrameIn.stVFrame.u64PhyAddr[1], param->stVideoFrameIn.stVFrame.u64PhyAddr[2]);
		DWA_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param->stVideoFrameOut.stVFrame.u64PhyAddr[0]
			, param->stVideoFrameOut.stVFrame.u64PhyAddr[1], param->stVideoFrameOut.stVFrame.u64PhyAddr[2]);

		//CVI_DWA_GetChnFrame();
		if (g_dwa_save_file) {
			s32Ret = DWAFrameSaveToFile(param->filename_out, &param->stVideoFrameOut);
			if (s32Ret != CVI_SUCCESS) {
				DWA_UT_PRT("DWAFrameSaveToFile. s32Ret: 0x%x !\n", s32Ret);
				goto exit2;
			}
			DWA_UT_PRT("-------------------times:(%d)----------------------\n", times);
			DWA_UT_PRT("output file:%s\n", param->filename_out);
			DWA_UT_PRT("pef file:%s\n", param->filename_pef);


			if (param->needPef) {
				s32Ret = DWACompareWithFile(param->filename_pef, &param->stVideoFrameOut);
				if (s32Ret != CVI_SUCCESS) {
					DWA_UT_PRT("DWACompareWithFile fail.\n");
					goto exit2;
				}
			}
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
			DWA_UT_PRT("release VB fail.\n");
			goto exit2;
		}
	} while (times--);

	if (bEnProc)
		system("cat /proc/soph/dwa");
exit2:
	if (s32Ret)
		if (param->hHandle)
			s32Ret |= CVI_DWA_CancelJob(param->hHandle);
	s32Ret |= CVI_DWA_DeInit();
	if (s32Ret) {
		DWA_UT_PRT("CVI_DWA_DeInit fail.\n");
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

static CVI_S32 dwa_test_fisheye(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	FISHEYE_ATTR_S stFisheyeAttr;
	DWA_BASIC_TEST_PARAM param = {0};
	int cnt = 2;
	char *filename_in[2] = {DWA_FILE_IN_FISHEYE, DWA_FILE_IN_FISHEYE};
	char *filename_out[2] = {DWA_FILE_OUT_FISHEYE_PANORAMA, DWA_FILE_OUT_FISHEYE_4R};
	char *filename_pef[2] = {DWA_FILE_PEF_FISHEYE_PANORAMA, DWA_FILE_PEF_FISHEYE_4R};
	param.needPef = CVI_TRUE;

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		strcpy(param.filename_pef, filename_pef[i]);
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

		param.op = DWA_TEST_FISHEYE;

		stFisheyeAttr.bEnable = CVI_TRUE;
		stFisheyeAttr.bBgColor = CVI_TRUE;
		stFisheyeAttr.u32BgColor = YUV_8BIT(0, 128, 128);
		stFisheyeAttr.s32HorOffset = param.size_in.u32Width / 2;
		stFisheyeAttr.s32VerOffset = param.size_in.u32Height / 2;
		stFisheyeAttr.enMountMode = FISHEYE_DESKTOP_MOUNT;
		if (i == 0)
			stFisheyeAttr.enUseMode = MODE_PANORAMA_360;
		else
			stFisheyeAttr.enUseMode = MODE_03_4R;

		stFisheyeAttr.u32RegionNum = 1;

		s32Ret = dwa_basic(&param, (void *)&stFisheyeAttr);
		if (s32Ret != CVI_SUCCESS) {
			DWA_UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	DWA_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 dwa_test_rot(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ROTATION_E rot[4];
	DWA_BASIC_TEST_PARAM param = {0};
	int cnt = 4;
	char *filename_in[4] = {DWA_FILE_IN_ROT, DWA_FILE_IN_ROT, DWA_FILE_IN_ROT, DWA_FILE_IN_ROT};
	char *filename_out[4] = {DWA_FILE_OUT_ROT0, DWA_FILE_OUT_ROT90, DWA_FILE_OUT_ROT180, DWA_FILE_OUT_ROT270};
	char *filename_pef[4] = {DWA_FILE_PEF_ROT0, DWA_FILE_PEF_ROT90, DWA_FILE_PEF_ROT180, DWA_FILE_PEF_ROT270};
	param.needPef = CVI_TRUE;

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		strcpy(param.filename_pef, filename_pef[i]);
		param.size_in.u32Width = 1920;
		param.size_in.u32Height = 1080;
		if (i == 1 || i == 3) {
			param.size_out.u32Width = 1088;
			param.size_out.u32Height = 1920;
		} else {
			param.size_out.u32Width = 1920;
			param.size_out.u32Height = 1080;
		}
		param.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_rot_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_rot_%d", i);
		param.identity.syncIo = CVI_TRUE;

		param.op = DWA_TEST_ROT;

		rot[i] = (ROTATION_E)i;

		s32Ret = dwa_basic(&param, (void *)rot[i]);
		if (s32Ret != CVI_SUCCESS) {
			DWA_UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	DWA_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 dwa_test_rot_4m(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ROTATION_E rot[4];
	DWA_BASIC_TEST_PARAM param = {0};
	int cnt = 4;
	char *filename_in[4] = {DWA_FILE_IN_ROT_4M, DWA_FILE_IN_ROT_4M, DWA_FILE_IN_ROT_4M, DWA_FILE_IN_ROT_4M};
	char *filename_out[4] = {DWA_FILE_OUT_ROT0_4M, DWA_FILE_OUT_ROT90_4M, DWA_FILE_OUT_ROT180_4M, DWA_FILE_OUT_ROT270_4M};
	char *filename_pef[4] = {DWA_FILE_PEF_ROT0_4M, DWA_FILE_PEF_ROT90_4M, DWA_FILE_PEF_ROT180_4M, DWA_FILE_PEF_ROT270_4M};
	param.needPef = CVI_TRUE;

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		strcpy(param.filename_pef, filename_pef[i]);
		param.size_in.u32Width = 2560;
		param.size_in.u32Height = 1440;
		if (i == 1 || i == 3) {
			param.size_out.u32Width = 1472;
			param.size_out.u32Height = 2560;
		} else {
			param.size_out.u32Width = 2560;
			param.size_out.u32Height = 1440;
		}
		param.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_rot_4m_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_rot_4m_%d", i);
		param.identity.syncIo = CVI_TRUE;

		param.op = DWA_TEST_ROT;

		rot[i] = (ROTATION_E)i;

		s32Ret = dwa_basic(&param, (void *)rot[i]);
		if (s32Ret != CVI_SUCCESS) {
			DWA_UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	DWA_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 dwa_test_rot_small(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ROTATION_E rot;
	DWA_BASIC_TEST_PARAM param = {0};
	int cnt = 4;
	char *filename_in[4] = {DWA_FILE_IN_ROT_1, DWA_FILE_IN_ROT_1, DWA_FILE_IN_ROT_1, DWA_FILE_IN_ROT_1};
	char *filename_out[4] = {DWA_FILE_OUT_ROT0_1, DWA_FILE_OUT_ROT90_1, DWA_FILE_OUT_ROT180_1, DWA_FILE_OUT_ROT270_1};
	char *filename_pef[4] = {DWA_FILE_OUT_ROT0_1, DWA_FILE_PEF_ROT90_1, DWA_FILE_PEF_ROT180_1, DWA_FILE_PEF_ROT270_1};
	param.needPef = CVI_TRUE;

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		strcpy(param.filename_pef, filename_pef[i]);
		param.size_in.u32Width = 128;
		param.size_in.u32Height = 128;
		param.size_out.u32Width = 128;
		param.size_out.u32Height = 128;

		param.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_rot_1_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_rot_1_%d", i);
		param.identity.syncIo = CVI_TRUE;

		param.op = DWA_TEST_ROT;

		rot = (ROTATION_E)i;

		s32Ret = dwa_basic(&param, (void *)rot);
		if (s32Ret != CVI_SUCCESS) {
			DWA_UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	DWA_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 dwa_test_ldc(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	LDC_ATTR_S stLdcAttr[8] = {
		{CVI_TRUE, 0, 0, 0, 0, 0, -200, {0}, 0, 0},
		{CVI_TRUE, 0, 0, 0, 0, 0, -200, {0}, 0, 0},
		{CVI_TRUE, 0, 0, 100, 0, 0, -200, {0}, 0, 0},
		{CVI_TRUE, 0, 0, 100, 0, 0, -200, {0}, 0, 0},
		{CVI_TRUE, 0, 0, 0, 0, 0, 400, {0}, 0, 0},
		{CVI_TRUE, 0, 0, 0, 0, 0, 400, {0}, 0, 0},
		{CVI_TRUE, 0, 0, 100, 0, 0, 400, {0}, 0, 0},
		{CVI_TRUE, 0, 0, 100, 0, 0, 400, {0}, 0, 0},
	};
	DWA_BASIC_TEST_PARAM param = {0};
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
	char *filename_pef[8] = {
		DWA_FILE_PEF_LDC_BARREL_0P3_0,
		DWA_FILE_PEF_LDC_BARREL_0P3_1,
		DWA_FILE_PEF_LDC_BARREL_0P3_2,
		DWA_FILE_PEF_LDC_BARREL_0P3_3,
		DWA_FILE_PEF_LDC_PINCUSHION_0P3_0,
		DWA_FILE_PEF_LDC_PINCUSHION_0P3_1,
		DWA_FILE_PEF_LDC_PINCUSHION_0P3_2,
		DWA_FILE_PEF_LDC_PINCUSHION_0P3_3,
	};
	param.needPef = CVI_TRUE;

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		strcpy(param.filename_pef, filename_pef[i]);

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

		param.op = DWA_TEST_LDC;

		s32Ret = dwa_basic(&param, (void *)&stLdcAttr[i]);
		if (s32Ret != CVI_SUCCESS) {
			DWA_UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	DWA_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 dwa_test_affine(CVI_VOID)
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
	DWA_BASIC_TEST_PARAM param = {0};
	int cnt = 1;
	char *filename_in[1] = {DWA_FILE_IN_AFFINE};
	char *filename_out[1] = {DWA_FILE_OUT_AFFINE};
	char *filename_pef[1] = {DWA_FILE_PEF_AFFINE};
	param.needPef = CVI_TRUE;

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		strcpy(param.filename_pef, filename_pef[i]);
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

		param.op = DWA_TEST_AFFINE;

		stAffineAttr.u32RegionNum = 9;
		memcpy(stAffineAttr.astRegionAttr, faces, sizeof(faces));
		stAffineAttr.stDestSize.u32Width = 112;
		stAffineAttr.stDestSize.u32Height = 112;

		s32Ret = dwa_basic(&param, (void *)&stAffineAttr);
		if (s32Ret != CVI_SUCCESS) {
			DWA_UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	DWA_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 dwa_test_rot_maxsize(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ROTATION_E rot[4];
	DWA_BASIC_TEST_PARAM param = {0};
	int cnt = 4;
	char *filename_in[4] = {DWA_FILE_IN_ROT_2, DWA_FILE_IN_ROT_2, DWA_FILE_IN_ROT_2, DWA_FILE_IN_ROT_2};
	char *filename_out[4] = {DWA_FILE_OUT_ROT0_2, DWA_FILE_OUT_ROT90_2, DWA_FILE_OUT_ROT180_2, DWA_FILE_OUT_ROT270_2};
	char *filename_pef[4] = {DWA_FILE_PEF_ROT0_2, DWA_FILE_PEF_ROT90_2, DWA_FILE_PEF_ROT180_2, DWA_FILE_PEF_ROT270_2};
	param.needPef = CVI_TRUE;

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		strcpy(param.filename_pef, filename_pef[i]);
		param.size_in.u32Width = 4096;
		param.size_in.u32Height = 4096;
		param.size_out.u32Width = 4096;
		param.size_out.u32Height = 4096;

		param.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_rot_2_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_rot_2_%d", i);
		param.identity.syncIo = CVI_TRUE;

		param.op = DWA_TEST_ROT;

		rot[i] = (ROTATION_E)i;

		s32Ret = dwa_basic(&param, (void *)rot[i]);
		if (s32Ret != CVI_SUCCESS) {
			DWA_UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	DWA_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 dwa_test_fmt(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	DWA_BASIC_TEST_PARAM param = {0};
	int cnt = 4;
	char *filename_in[4] = {DWA_FILE_IN_FMT_0, DWA_FILE_IN_FMT_1, DWA_FILE_IN_FMT_2, DWA_FILE_IN_FMT_3};
	char *filename_out[4] = {DWA_FILE_OUT_FMT_0, DWA_FILE_OUT_FMT_1, DWA_FILE_OUT_FMT_2, DWA_FILE_OUT_FMT_3};
	char *filename_pef[4] = {DWA_FILE_PEF_FMT_0, DWA_FILE_PEF_FMT_1, DWA_FILE_PEF_FMT_2, DWA_FILE_PEF_FMT_3};
	PIXEL_FORMAT_E enPixelFormat[4] = {
		PIXEL_FORMAT_YUV_PLANAR_420,
		PIXEL_FORMAT_YUV_400,
		PIXEL_FORMAT_YUV_PLANAR_444,
		PIXEL_FORMAT_RGB_888_PLANAR
	};
	param.needPef = CVI_TRUE;

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		strcpy(param.filename_pef, filename_pef[i]);
		param.size_in.u32Width = 1920;
		param.size_in.u32Height = 1080;
		param.size_out.u32Width = 1920;
		param.size_out.u32Height = 1080;

		param.enPixelFormat = enPixelFormat[i];
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_fmt_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_fmt_%d", i);
		param.identity.syncIo = CVI_TRUE;

		param.op = DWA_TEST_ROT;

		s32Ret = dwa_basic(&param, (void *)ROTATION_0);
		if (s32Ret != CVI_SUCCESS) {
			DWA_UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	DWA_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 dwa_test_not_align(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	DWA_BASIC_TEST_PARAM param = {0};
	int cnt = 1;
	char *filename_in[1] = {DWA_FILE_IN_NOT_ALIGN};
	char *filename_out[1] = {DWA_FILE_OUT_NOT_ALIGN};

	param.needPef = CVI_FALSE;

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		param.size_in.u32Width = 666;
		param.size_in.u32Height = 666;
		param.size_out.u32Width = 666;
		param.size_out.u32Height = 666;

		param.enPixelFormat = PIXEL_FORMAT_YUV_400;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_not_align_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_not_align_%d", i);
		param.identity.syncIo = CVI_TRUE;

		param.op = DWA_TEST_ROT;

		s32Ret = dwa_basic(&param, (void *)ROTATION_0);
		if (s32Ret != CVI_SUCCESS) {
			DWA_UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	DWA_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 dwa_test_reset(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	DWA_BASIC_TEST_PARAM param = {0};
	VB_CONFIG_S stVbConf;
	int times = 4, err_times = 4;

	do {
		for (CVI_U8 cnt = 0; cnt < times; cnt ++) {
			param.op = DWA_TEST_ROT;
			param.size_in.u32Width =   1920;
			param.size_in.u32Height =  1080;
			param.size_out.u32Width =  1920;
			param.size_out.u32Height = 1080;
			param.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
			snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_reset_%d", cnt);
			param.identity.enModId = CVI_ID_USER;
			param.identity.u32ID = 0;
			snprintf(param.identity.Name, sizeof(param.identity.Name),	"job_reset_%d", cnt);
			param.identity.syncIo = CVI_TRUE;
			param.op = DWA_TEST_ROT;
			param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width, param.size_in.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
				, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);
			param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width, param.size_out.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
				, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);

			stVbConf.u32MaxPoolCnt				= 2;
			stVbConf.astCommPool[0].u32BlkSize	= param.u32BlkSizeIn;
			stVbConf.astCommPool[0].u32BlkCnt	= 2;
			stVbConf.astCommPool[0].enRemapMode = VB_REMAP_MODE_CACHED;
			stVbConf.astCommPool[1].u32BlkSize	= param.u32BlkSizeOut;
			stVbConf.astCommPool[1].u32BlkCnt	= 2;
			stVbConf.astCommPool[1].enRemapMode = VB_REMAP_MODE_CACHED;
			DWA_UT_PRT("common pool[0] BlkSize %d\n", param.u32BlkSizeIn);
			DWA_UT_PRT("common pool[1] BlkSize %d\n", param.u32BlkSizeOut);

			s32Ret = CVI_VB_SetConfig(&stVbConf);
			if (s32Ret != CVI_SUCCESS) {
				DWA_UT_PRT("CVI_VB_SetConf failed!\n");
				return s32Ret;
			}

			s32Ret = CVI_VB_Init();
			if (s32Ret != CVI_SUCCESS) {
				DWA_UT_PRT("CVI_VB_Init failed!\n");
				return s32Ret;
			}

			s32Ret = CVI_SYS_Init();
			if (s32Ret != CVI_SUCCESS) {
				DWA_UT_PRT("CVI_SYS_Init failed!\n");
				goto exit0;
			}

			s32Ret = CVI_DWA_Init();
			if (s32Ret != CVI_SUCCESS) {
				DWA_UT_PRT("CVI_DWA_Init failed!\n");
				goto exit1;
			}
			param.hHandle = 0;
			memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
			s32Ret = DWA_COMM_PrepareFrame(&param.size_in, param.enPixelFormat, &param.stVideoFrameIn);
			if (s32Ret) {
				DWA_UT_PRT("DWA_COMM_PrepareFrame in failed!\n");
				goto exit2;
			}

			memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
			s32Ret = DWA_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut);
			if (s32Ret) {
				DWA_UT_PRT("DWA_COMM_PrepareFrame out failed!\n");
				goto exit2;
			}

			memset(param.stTask.au64privateData, 0, sizeof(param.stTask.au64privateData));
			memcpy(&param.stTask.stImgIn, &param.stVideoFrameIn, sizeof(param.stVideoFrameIn));
			memcpy(&param.stTask.stImgOut, &param.stVideoFrameOut, sizeof(param.stVideoFrameOut));

			s32Ret = CVI_DWA_BeginJob(&param.hHandle);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_BeginJob failed!\n");
				goto exit2;
			}

			s32Ret = CVI_DWA_SetJobIdentity(param.hHandle, &param.identity);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_SetJobIdentity failed!\n");
				goto exit2;
			}

			s32Ret = CVI_DWA_AddRotationTask(param.hHandle, &param.stTask, ROTATION_0);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_AddRotationTask failed!\n");
			}

			if (cnt == 0) {//make invalid param
				void *virAddr = (void *)(uintptr_t)(param.stTask.au64privateData[1]);
				memset(virAddr, 0, CVI_DWA_MESH_SIZE_ROT);
			}

			s32Ret = CVI_DWA_EndJob(param.hHandle);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_EndJob failed!\n");
				goto exit2;
			}

			DWA_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param.stVideoFrameIn.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameIn.stVFrame.u64PhyAddr[1], param.stVideoFrameIn.stVFrame.u64PhyAddr[2]);
			DWA_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param.stVideoFrameOut.stVFrame.u64PhyAddr[0]
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
				DWA_UT_PRT("release VB fail.\n");
				goto exit2;
			}

		exit2:
			if (s32Ret)
				if (param.hHandle)
					s32Ret |= CVI_DWA_CancelJob(param.hHandle);
			s32Ret |= CVI_DWA_DeInit();
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_DeInit fail.\n");
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
				DWA_UT_PRT("unknown error, expect NG, but OK occur\n");
				return CVI_FAILURE;
			}
		}
	} while (err_times--);
	DWA_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 dwa_test_cmdq(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	DWA_BASIC_TEST_PARAM param = {0};
	VB_CONFIG_S stVbConf;
	char *filename_in[1] = {DWA_FILE_IN_CMDQ};
	char *filename_out[1] = {DWA_FILE_OUT_CMDQ};
	char *filename_pef[1] = {DWA_FILE_PEF_CMDQ};
	GDC_TASK_ATTR_S stTask_1st;
	GDC_TASK_ATTR_S stTask_2nd;
	SIZE_S size_1st_out, size_2nd_out;
	int times = DWA_REPEAT_TIMES;
	VB_BLK Blk;

	for (CVI_U8 cnt = 0; cnt < 2; cnt ++) {
		strcpy(param.filename_in, filename_in[0]);
		strcpy(param.filename_out, filename_out[0]);
		strcpy(param.filename_pef, filename_pef[0]);

		param.op = DWA_TEST_ROT;
		param.size_in.u32Width =   1920;
		param.size_in.u32Height =  1080;
		param.size_out.u32Width =  1920;
		param.size_out.u32Height = 1088;
		param.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_cmdq_%d", cnt);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = 0;
		snprintf(param.identity.Name, sizeof(param.identity.Name),	"job_cmdq_%d", cnt);
		param.identity.syncIo = CVI_TRUE;
		param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width, param.size_in.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);
		param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width, param.size_out.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);
		param.needPef = CVI_TRUE;

		stVbConf.u32MaxPoolCnt				= 2;
		stVbConf.astCommPool[0].u32BlkSize	= param.u32BlkSizeIn;
		stVbConf.astCommPool[0].u32BlkCnt	= 4;
		stVbConf.astCommPool[0].enRemapMode = VB_REMAP_MODE_CACHED;
		stVbConf.astCommPool[1].u32BlkSize	= param.u32BlkSizeOut;
		stVbConf.astCommPool[1].u32BlkCnt	= 4;
		stVbConf.astCommPool[1].enRemapMode = VB_REMAP_MODE_CACHED;
		DWA_UT_PRT("common pool[0] BlkSize %d\n", param.u32BlkSizeIn);
		DWA_UT_PRT("common pool[1] BlkSize %d\n", param.u32BlkSizeOut);

		s32Ret = CVI_VB_SetConfig(&stVbConf);
		if (s32Ret != CVI_SUCCESS) {
			DWA_UT_PRT("CVI_VB_SetConf failed!\n");
			return s32Ret;
		}

		s32Ret = CVI_VB_Init();
		if (s32Ret != CVI_SUCCESS) {
			DWA_UT_PRT("CVI_VB_Init failed!\n");
			return s32Ret;
		}

		s32Ret = CVI_SYS_Init();
		if (s32Ret != CVI_SUCCESS) {
			DWA_UT_PRT("CVI_SYS_Init failed!\n");
			goto exit0;
		}

		s32Ret = CVI_DWA_Init();
		if (s32Ret != CVI_SUCCESS) {
			DWA_UT_PRT("CVI_DWA_Init failed!\n");
			goto exit1;
		}

		times = DWA_REPEAT_TIMES;

		do {
			param.hHandle = 0;
			memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
			s32Ret = DWAFileToFrame(&param.size_in, param.enPixelFormat, param.filename_in, &param.stVideoFrameIn);
			if (s32Ret) {
				DWA_UT_PRT("DWAFileToFrame failed!\n");
				goto exit2;
			}

			memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
			s32Ret = DWA_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut);
			if (s32Ret) {
				DWA_UT_PRT("DWA_COMM_PrepareFrame out failed!\n");
				goto exit2;
			}

			memset(param.stTask.au64privateData, 0, sizeof(param.stTask.au64privateData));
			memcpy(&param.stTask.stImgIn, &param.stVideoFrameIn, sizeof(param.stVideoFrameIn));
			memcpy(&param.stTask.stImgOut, &param.stVideoFrameOut, sizeof(param.stVideoFrameOut));

			s32Ret = CVI_DWA_BeginJob(&param.hHandle);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_BeginJob failed!\n");
				goto exit2;
			}

			s32Ret = CVI_DWA_SetJobIdentity(param.hHandle, &param.identity);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_SetJobIdentity failed!\n");
				goto exit2;
			}

			//1st rot90
			memset(&stTask_1st.stImgIn, 0, sizeof(stTask_1st.stImgIn));
			memcpy(&stTask_1st.stImgIn, &param.stVideoFrameIn, sizeof(stTask_1st.stImgIn));

			memset(&stTask_1st.stImgOut, 0, sizeof(stTask_1st.stImgOut));
			size_1st_out.u32Width = ALIGN(stTask_1st.stImgIn.stVFrame.u32Height, DWA_STRIDE_ALIGN);
			size_1st_out.u32Height = ALIGN(stTask_1st.stImgIn.stVFrame.u32Width, DWA_STRIDE_ALIGN);
			snprintf(stTask_1st.name, sizeof(param.stTask.name), "%s_1st_%d", param.stTask.name, times);

			s32Ret = DWA_COMM_PrepareFrame(&size_1st_out, param.enPixelFormat, &stTask_1st.stImgOut);
			if (s32Ret) {
				DWA_UT_PRT("DWA_COMM_PrepareFrame 1st out failed!\n");
				goto exit2;
			}

			s32Ret = CVI_DWA_AddRotationTask(param.hHandle, &stTask_1st, ROTATION_90);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_AddRotationTask 1st failed!\n");
				goto exit2;
			}

			//2nd rot90 again
			memset(&stTask_2nd.stImgIn, 0, sizeof(stTask_2nd.stImgIn));
			memcpy(&stTask_2nd.stImgIn, &stTask_1st.stImgOut, sizeof(stTask_2nd.stImgIn));

			memset(&stTask_2nd.stImgOut, 0, sizeof(stTask_2nd.stImgOut));
			memcpy(&stTask_2nd.stImgOut, &param.stVideoFrameOut, sizeof(stTask_2nd.stImgIn));
			size_2nd_out.u32Width = ALIGN(stTask_2nd.stImgIn.stVFrame.u32Height, DWA_STRIDE_ALIGN);
			size_2nd_out.u32Height = ALIGN(stTask_2nd.stImgIn.stVFrame.u32Width, DWA_STRIDE_ALIGN);
			stTask_2nd.stImgOut.stVFrame.u32Width = size_2nd_out.u32Width;
			stTask_2nd.stImgOut.stVFrame.u32Height = size_2nd_out.u32Height;
			snprintf(stTask_2nd.name, sizeof(param.stTask.name), "%s_2nd_%d", param.stTask.name, times);

			s32Ret = CVI_DWA_AddRotationTask(param.hHandle, &stTask_2nd, ROTATION_90);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_AddRotationTask 2nd failed!\n");
				goto exit2;
			}

			s32Ret = CVI_DWA_EndJob(param.hHandle);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_EndJob failed!\n");
				goto exit2;
			}

			DWA_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param.stVideoFrameIn.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameIn.stVFrame.u64PhyAddr[1], param.stVideoFrameIn.stVFrame.u64PhyAddr[2]);
			DWA_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param.stVideoFrameOut.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameOut.stVFrame.u64PhyAddr[1], param.stVideoFrameOut.stVFrame.u64PhyAddr[2]);

			if (g_dwa_save_file) {
				s32Ret = DWAFrameSaveToFile(param.filename_out, &param.stVideoFrameOut);
				if (s32Ret != CVI_SUCCESS) {
					DWA_UT_PRT("DWAFrameSaveToFile s32Ret: 0x%x !\n", s32Ret);
					goto exit2;
				}

				DWA_UT_PRT("output file:%s\n", param.filename_out);
				DWA_UT_PRT("pef file:%s\n", param.filename_pef);

				if (param.needPef) {
					s32Ret = DWACompareWithFile(param.filename_pef, &param.stVideoFrameOut);
					if (s32Ret != CVI_SUCCESS) {
						DWA_UT_PRT("DWACompareWithFile fail.\n");
						goto exit2;
					}
				}
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
				DWA_UT_PRT("release VB fail.\n");
				goto exit2;
			}
		} while (times--);

	exit2:
		if (s32Ret)
			if (param.hHandle)
				s32Ret |= CVI_DWA_CancelJob(param.hHandle);
		s32Ret |= CVI_DWA_DeInit();
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_DeInit fail.\n");
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
	DWA_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 dwa_test_cmdq_1to2(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	DWA_BASIC_TEST_PARAM param = {0};
	VB_CONFIG_S stVbConf;
	char *filename_in[1] = {DWA_FILE_IN_CMDQ_1TO2};
	char *filename_out[2] = {DWA_FILE_OUT_CMDQ_1TO2_0, DWA_FILE_OUT_CMDQ_1TO2_1};
	char *filename_pef[2] = {DWA_FILE_PEF_CMDQ_1TO2_0, DWA_FILE_PEF_CMDQ_1TO2_1};
	GDC_TASK_ATTR_S stTask_tmp;
	VIDEO_FRAME_INFO_S stVideoFrameOut_tmp;
	int times = DWA_REPEAT_TIMES;
	VB_BLK Blk;
	CVI_CHAR name[32];

	for (CVI_U8 cnt = 0; cnt < 2; cnt ++) {
		strcpy(param.filename_in, filename_in[0]);
		strcpy(param.filename_out, filename_out[0]);
		strcpy(param.filename_pef, filename_pef[0]);

		param.op = DWA_TEST_ROT;
		param.size_in.u32Width =   1920;
		param.size_in.u32Height =  1080;
		param.size_out.u32Width =  1920;
		param.size_out.u32Height = 1080;
		param.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_cmdq_%d", cnt);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = 0;
		snprintf(param.identity.Name, sizeof(param.identity.Name),	"job_cmdq_%d", cnt);
		param.identity.syncIo = CVI_TRUE;
		param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width, param.size_in.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);
		param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width, param.size_out.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);
		param.needPef = CVI_TRUE;

		stVbConf.u32MaxPoolCnt				= 2;
		stVbConf.astCommPool[0].u32BlkSize	= param.u32BlkSizeIn;
		stVbConf.astCommPool[0].u32BlkCnt	= 4;
		stVbConf.astCommPool[0].enRemapMode = VB_REMAP_MODE_CACHED;
		stVbConf.astCommPool[1].u32BlkSize	= param.u32BlkSizeOut;
		stVbConf.astCommPool[1].u32BlkCnt	= 4;
		stVbConf.astCommPool[1].enRemapMode = VB_REMAP_MODE_CACHED;
		DWA_UT_PRT("common pool[0] BlkSize %d\n", param.u32BlkSizeIn);
		DWA_UT_PRT("common pool[1] BlkSize %d\n", param.u32BlkSizeOut);

		s32Ret = CVI_VB_SetConfig(&stVbConf);
		if (s32Ret != CVI_SUCCESS) {
			DWA_UT_PRT("CVI_VB_SetConf failed!\n");
			return s32Ret;
		}

		s32Ret = CVI_VB_Init();
		if (s32Ret != CVI_SUCCESS) {
			DWA_UT_PRT("CVI_VB_Init failed!\n");
			return s32Ret;
		}

		s32Ret = CVI_SYS_Init();
		if (s32Ret != CVI_SUCCESS) {
			DWA_UT_PRT("CVI_SYS_Init failed!\n");
			goto exit0;
		}

		s32Ret = CVI_DWA_Init();
		if (s32Ret != CVI_SUCCESS) {
			DWA_UT_PRT("CVI_DWA_Init failed!\n");
			goto exit1;
		}

		times = DWA_REPEAT_TIMES;

		do {
			param.hHandle = 0;
			memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
			s32Ret = DWAFileToFrame(&param.size_in, param.enPixelFormat, param.filename_in, &param.stVideoFrameIn);
			if (s32Ret) {
				DWA_UT_PRT("DWAFileToFrame failed!\n");
				goto exit2;
			}

			memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
			s32Ret = DWA_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut);
			if (s32Ret) {
				DWA_UT_PRT("DWA_COMM_PrepareFrame out 1st failed!\n");
				goto exit2;
			}

			memset(param.stTask.au64privateData, 0, sizeof(param.stTask.au64privateData));
			memcpy(&param.stTask.stImgIn, &param.stVideoFrameIn, sizeof(param.stVideoFrameIn));
			memcpy(&param.stTask.stImgOut, &param.stVideoFrameOut, sizeof(param.stVideoFrameOut));

			s32Ret = CVI_DWA_BeginJob(&param.hHandle);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_BeginJob failed!\n");
				goto exit2;
			}

			s32Ret = CVI_DWA_SetJobIdentity(param.hHandle, &param.identity);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_SetJobIdentity failed!\n");
				goto exit2;
			}

			strcpy(name, param.stTask.name);
			snprintf(param.stTask.name, sizeof(param.stTask.name), "%s_1st_%d", name, times);
			s32Ret = CVI_DWA_AddRotationTask(param.hHandle, &param.stTask, ROTATION_0);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_AddRotationTask 1st failed!\n");
				goto exit2;
			}

			//1to2
			memset(&stVideoFrameOut_tmp, 0, sizeof(stVideoFrameOut_tmp));
			s32Ret = DWA_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &stVideoFrameOut_tmp);
			if (s32Ret) {
				DWA_UT_PRT("DWA_COMM_PrepareFrame out 2nd failed!\n");
				goto exit2;
			}

			memset(&stTask_tmp, 0, sizeof(stTask_tmp));
			memcpy(&stTask_tmp.stImgIn, &param.stVideoFrameIn, sizeof(VIDEO_FRAME_INFO_S));
			memcpy(&stTask_tmp.stImgOut, &stVideoFrameOut_tmp, sizeof(VIDEO_FRAME_INFO_S));
			snprintf(stTask_tmp.name, sizeof(param.stTask.name), "%s_2nd_%d", name, times);
			s32Ret = CVI_DWA_AddRotationTask(param.hHandle, &stTask_tmp, ROTATION_0);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_AddRotationTask 2nd failed!\n");
				goto exit2;
			}

			s32Ret = CVI_DWA_EndJob(param.hHandle);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_EndJob failed!\n");
				goto exit2;
			}

			DWA_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param.stVideoFrameIn.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameIn.stVFrame.u64PhyAddr[1], param.stVideoFrameIn.stVFrame.u64PhyAddr[2]);
			DWA_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param.stVideoFrameOut.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameOut.stVFrame.u64PhyAddr[1], param.stVideoFrameOut.stVFrame.u64PhyAddr[2]);

			if (g_dwa_save_file) {
				s32Ret = DWAFrameSaveToFile(param.filename_out, &param.stVideoFrameOut);
				if (s32Ret != CVI_SUCCESS) {
					DWA_UT_PRT("DWAFrameSaveToFile s32Ret: 0x%x !\n", s32Ret);
					goto exit2;
				}
				DWA_UT_PRT("output file:%s\n", param.filename_out);
				DWA_UT_PRT("pef file:%s\n", param.filename_pef);

				if (param.needPef) {
					s32Ret = DWACompareWithFile(param.filename_out, &stTask_tmp.stImgOut);
					if (s32Ret != CVI_SUCCESS) {
						DWA_UT_PRT("DWACompareWithFile fail.\n");
						goto exit2;
					}
					s32Ret = DWACompareWithFile(param.filename_pef, &param.stVideoFrameOut);
					if (s32Ret != CVI_SUCCESS) {
						DWA_UT_PRT("GDCCompareWithFile fail.\n");
						goto exit2;
					}
				}
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
				DWA_UT_PRT("release VB fail.\n");
				goto exit2;
			}
		} while (times--);

	exit2:
		if (s32Ret)
			if (param.hHandle)
				s32Ret |= CVI_DWA_CancelJob(param.hHandle);
		s32Ret |= CVI_DWA_DeInit();
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_DeInit fail.\n");
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
	DWA_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 dwa_test_cmdq_1to2_maxsize(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	DWA_BASIC_TEST_PARAM param = {0};
	VB_CONFIG_S stVbConf;
	char *filename_in[1] = {DWA_FILE_IN_CMDQ_1TO2_MAX};
	char *filename_out[2] = {DWA_FILE_OUT_CMDQ_1TO2_0_MAX, DWA_FILE_OUT_CMDQ_1TO2_1_MAX};
	char *filename_pef[2] = {DWA_FILE_PEF_CMDQ_1TO2_0_MAX, DWA_FILE_PEF_CMDQ_1TO2_1_MAX};
	GDC_TASK_ATTR_S stTask_tmp;
	VIDEO_FRAME_INFO_S stVideoFrameOut_tmp;
	int times = DWA_REPEAT_TIMES;
	VB_BLK Blk;
	CVI_CHAR name[32];

	for (CVI_U8 cnt = 0; cnt < 2; cnt ++) {
		strcpy(param.filename_in, filename_in[0]);
		strcpy(param.filename_out, filename_out[0]);
		strcpy(param.filename_pef, filename_pef[0]);

		param.op = DWA_TEST_ROT;
		param.size_in.u32Width =   4096;
		param.size_in.u32Height =  4096;
		param.size_out.u32Width =  4096;
		param.size_out.u32Height = 4096;
		param.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_cmdq_%d", cnt);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = 0;
		snprintf(param.identity.Name, sizeof(param.identity.Name),	"job_cmdq_%d", cnt);
		param.identity.syncIo = CVI_TRUE;
		param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width, param.size_in.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);
		param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width, param.size_out.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);
		param.needPef = CVI_TRUE;

		stVbConf.u32MaxPoolCnt				= 2;
		stVbConf.astCommPool[0].u32BlkSize	= param.u32BlkSizeIn;
		stVbConf.astCommPool[0].u32BlkCnt	= 4;
		stVbConf.astCommPool[0].enRemapMode = VB_REMAP_MODE_CACHED;
		stVbConf.astCommPool[1].u32BlkSize	= param.u32BlkSizeOut;
		stVbConf.astCommPool[1].u32BlkCnt	= 4;
		stVbConf.astCommPool[1].enRemapMode = VB_REMAP_MODE_CACHED;
		DWA_UT_PRT("common pool[0] BlkSize %d\n", param.u32BlkSizeIn);
		DWA_UT_PRT("common pool[1] BlkSize %d\n", param.u32BlkSizeOut);

		s32Ret = CVI_VB_SetConfig(&stVbConf);
		if (s32Ret != CVI_SUCCESS) {
			DWA_UT_PRT("CVI_VB_SetConf failed!\n");
			return s32Ret;
		}

		s32Ret = CVI_VB_Init();
		if (s32Ret != CVI_SUCCESS) {
			DWA_UT_PRT("CVI_VB_Init failed!\n");
			return s32Ret;
		}

		s32Ret = CVI_SYS_Init();
		if (s32Ret != CVI_SUCCESS) {
			DWA_UT_PRT("CVI_SYS_Init failed!\n");
			goto exit0;
		}

		s32Ret = CVI_DWA_Init();
		if (s32Ret != CVI_SUCCESS) {
			DWA_UT_PRT("CVI_DWA_Init failed!\n");
			goto exit1;
		}

		times = DWA_REPEAT_TIMES;

		do {
			param.hHandle = 0;
			memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
			s32Ret = DWAFileToFrame(&param.size_in, param.enPixelFormat, param.filename_in, &param.stVideoFrameIn);
			if (s32Ret) {
				DWA_UT_PRT("DWAFileToFrame failed!\n");
				goto exit2;
			}

			memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
			s32Ret = DWA_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut);
			if (s32Ret) {
				DWA_UT_PRT("DWA_COMM_PrepareFrame out 1st failed!\n");
				goto exit2;
			}

			memset(param.stTask.au64privateData, 0, sizeof(param.stTask.au64privateData));
			memcpy(&param.stTask.stImgIn, &param.stVideoFrameIn, sizeof(param.stVideoFrameIn));
			memcpy(&param.stTask.stImgOut, &param.stVideoFrameOut, sizeof(param.stVideoFrameOut));

			s32Ret = CVI_DWA_BeginJob(&param.hHandle);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_BeginJob failed!\n");
				goto exit2;
			}

			s32Ret = CVI_DWA_SetJobIdentity(param.hHandle, &param.identity);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_SetJobIdentity failed!\n");
				goto exit2;
			}

			strcpy(name, param.stTask.name);
			snprintf(param.stTask.name, sizeof(param.stTask.name), "%s_1st_%d", name, times);
			s32Ret = CVI_DWA_AddRotationTask(param.hHandle, &param.stTask, ROTATION_0);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_AddRotationTask 1st failed!\n");
				goto exit2;
			}

			//1to2
			memset(&stVideoFrameOut_tmp, 0, sizeof(stVideoFrameOut_tmp));
			s32Ret = DWA_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &stVideoFrameOut_tmp);
			if (s32Ret) {
				DWA_UT_PRT("DWA_COMM_PrepareFrame out 2nd failed!\n");
				goto exit2;
			}

			memset(&stTask_tmp, 0, sizeof(stTask_tmp));
			memcpy(&stTask_tmp.stImgIn, &param.stVideoFrameIn, sizeof(VIDEO_FRAME_INFO_S));
			memcpy(&stTask_tmp.stImgOut, &stVideoFrameOut_tmp, sizeof(VIDEO_FRAME_INFO_S));
			snprintf(stTask_tmp.name, sizeof(param.stTask.name), "%s_2nd_%d", name, times);
			s32Ret = CVI_DWA_AddRotationTask(param.hHandle, &stTask_tmp, ROTATION_0);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_AddRotationTask 2nd failed!\n");
				goto exit2;
			}

			s32Ret = CVI_DWA_EndJob(param.hHandle);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_EndJob failed!\n");
				goto exit2;
			}

			DWA_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param.stVideoFrameIn.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameIn.stVFrame.u64PhyAddr[1], param.stVideoFrameIn.stVFrame.u64PhyAddr[2]);
			DWA_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param.stVideoFrameOut.stVFrame.u64PhyAddr[0]
				, param.stVideoFrameOut.stVFrame.u64PhyAddr[1], param.stVideoFrameOut.stVFrame.u64PhyAddr[2]);
			if (g_dwa_save_file) {
				s32Ret = DWAFrameSaveToFile(param.filename_out, &param.stVideoFrameOut);
				if (s32Ret != CVI_SUCCESS) {
					DWA_UT_PRT("GDCFrameSaveToFile s32Ret: 0x%x !\n", s32Ret);
					goto exit2;
				}
				DWA_UT_PRT("output file:%s\n", param.filename_out);
				DWA_UT_PRT("pef file:%s\n", param.filename_pef);

				if (param.needPef) {
					s32Ret = DWACompareWithFile(param.filename_out, &stTask_tmp.stImgOut);
					if (s32Ret != CVI_SUCCESS) {
						DWA_UT_PRT("GDCCompareWithFile fail.\n");
						goto exit2;
					}

					s32Ret = DWACompareWithFile(param.filename_pef, &param.stVideoFrameOut);
					if (s32Ret != CVI_SUCCESS) {
						DWA_UT_PRT("GDCCompareWithFile fail.\n");
						goto exit2;
					}
				}
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
				DWA_UT_PRT("release VB fail.\n");
				goto exit2;
			}
		} while (times--);

	exit2:
		if (s32Ret)
			if (param.hHandle)
				s32Ret |= CVI_DWA_CancelJob(param.hHandle);
		s32Ret |= CVI_DWA_DeInit();
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_DeInit fail.\n");
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
	DWA_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 dwa_test_online(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	DWA_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 dwa_test_async(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	DWA_BASIC_TEST_PARAM param = {0};
	char *filename_in[1] = {DWA_FILE_IN_ROT};
	char *filename_out[1] = {DWA_FILE_OUT_ROT0};
	char *filename_pef[1] = {DWA_FILE_PEF_ROT0};
	CVI_U8 times = DWA_REPEAT_TIMES * 10;

	strcpy(param.filename_in, filename_in[0]);
	strcpy(param.filename_out, filename_out[0]);
	strcpy(param.filename_pef, filename_pef[0]);
	param.size_in.u32Width = 1920;
	param.size_in.u32Height = 1080;
	param.size_out.u32Width = 1920;
	param.size_out.u32Height = 1080;
	param.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_rot_async");
	param.identity.enModId = CVI_ID_USER;
	param.identity.u32ID = 0;
	snprintf(param.identity.Name, sizeof(param.identity.Name), "job_rot_async");
	param.identity.syncIo = CVI_FALSE;
	param.op = DWA_TEST_ROT;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width, param.size_in.u32Height, param.enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);
	param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width, param.size_out.u32Height, param.enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);

	stVbConf.u32MaxPoolCnt              = 2;
	stVbConf.astCommPool[0].u32BlkSize	= param.u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 10;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize	= param.u32BlkSizeOut;
	stVbConf.astCommPool[1].u32BlkCnt	= 10;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	DWA_UT_PRT("common pool[0] BlkSize %d\n", param.u32BlkSizeIn);
	DWA_UT_PRT("common pool[1] BlkSize %d\n", param.u32BlkSizeOut);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	s32Ret = CVI_DWA_Init();
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_DWA_Init failed!\n");
		goto exit1;
	}

	do {
		param.hHandle = 0;
		memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
		s32Ret = DWAFileToFrame(&param.size_in, param.enPixelFormat, param.filename_in, &param.stVideoFrameIn);
		if (s32Ret) {
			DWA_UT_PRT("DWAFileToFrame failed!\n");
			goto exit2;
		}

		memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
		s32Ret = DWA_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut);
		if (s32Ret) {
			DWA_UT_PRT("DWA_COMM_PrepareFrame failed!\n");
			goto exit2;
		}

		memset(param.stTask.au64privateData, 0, sizeof(param.stTask.au64privateData));
		memcpy(&param.stTask.stImgIn, &param.stVideoFrameIn, sizeof(param.stVideoFrameIn));
		memcpy(&param.stTask.stImgOut, &param.stVideoFrameOut, sizeof(param.stVideoFrameOut));

		s32Ret = CVI_DWA_BeginJob(&param.hHandle);
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_BeginJob failed!\n");
			goto exit2;
		}

		s32Ret = CVI_DWA_SetJobIdentity(param.hHandle, &param.identity);
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_SetJobIdentity failed!\n");
			goto exit2;
		}

		s32Ret = CVI_DWA_AddRotationTask(param.hHandle, &param.stTask, ROTATION_0);
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_AddRotationTask 1st failed!\n");
			goto exit2;
		}

		s32Ret = CVI_DWA_EndJob(param.hHandle);
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_EndJob failed!\n");
			goto exit2;
		}

		usleep(1000*500);

		if ((s32Ret = CVI_DWA_GetChnFrame(&param.identity, &param.stVideoFrameOut, 500)) != CVI_SUCCESS)
			break;

		param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
		if (param.inBlk != VB_INVALID_HANDLE)
			s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);

		param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
		if (param.outBlk != VB_INVALID_HANDLE)
			s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);

		if (s32Ret) {
			DWA_UT_PRT("release VB fail.\n");
			goto exit2;
		}
	} while (times--);

exit2:
	if (s32Ret)
		if (param.hHandle)
			s32Ret |= CVI_DWA_CancelJob(param.hHandle);
	s32Ret |= CVI_DWA_DeInit();
	if (s32Ret) {
		DWA_UT_PRT("CVI_DWA_DeInit fail.\n");
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

	DWA_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static DWA_BASIC_TEST_PARAM param_mix[4] = {0};

#if 0
static pthread_mutex_t g_tMutex_mix  = PTHREAD_MUTEX_INITIALIZER;
static int g_cur_case = -1;
static CVI_BOOL DwaAsyncDoneFlag;

void *test_async_thread(void *data)
{
	CVI_S32 s32MilliSec = 50;
	CVI_S32 s32Ret;
	(void)data;
	int cur_case_mix = -1;

	while (!DwaAsyncDoneFlag) {
		pthread_mutex_lock(&g_tMutex_mix);
		cur_case_mix = g_cur_case;
		pthread_mutex_unlock(&g_tMutex_mix);

		if (cur_case_mix < 0 || cur_case_mix >= 4) {
			usleep(1000);
			continue;
		}

		s32Ret = CVI_DWA_GetChnFrame(&param_mix[cur_case_mix].identity, &param_mix[cur_case_mix].stVideoFrameOut, s32MilliSec);
		if (!s32Ret)
			continue;

		DWA_UT_PRT("[%s]\n", param_mix[cur_case_mix].filename_in);
		DWA_UT_PRT("[%s]\n", param_mix[cur_case_mix].filename_out);
		DWA_UT_PRT("op[%d]\n", param_mix[cur_case_mix].op);
		DWA_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param_mix[cur_case_mix].stVideoFrameIn.stVFrame.u64PhyAddr[0]
			, param_mix[cur_case_mix].stVideoFrameIn.stVFrame.u64PhyAddr[1], param_mix[cur_case_mix].stVideoFrameIn.stVFrame.u64PhyAddr[2]);
		DWA_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param_mix[cur_case_mix].stVideoFrameOut.stVFrame.u64PhyAddr[0]
			, param_mix[cur_case_mix].stVideoFrameOut.stVFrame.u64PhyAddr[1], param_mix[cur_case_mix].stVideoFrameOut.stVFrame.u64PhyAddr[2]);

#if 0
		s32Ret = DWAFrameSaveToFile(param->filename_out, &param->stVideoFrameOut);
		if (s32Ret) {
			DWA_UT_PRT("DWAFrameSaveToFile fail\n");
		}
#endif
		if (param_mix[cur_case_mix].stVideoFrameIn.stVFrame.u64PhyAddr[0]) {
			param_mix[cur_case_mix].inBlk = CVI_VB_PhysAddr2Handle(param_mix[cur_case_mix].stVideoFrameIn.stVFrame.u64PhyAddr[0]);
			if (param_mix[cur_case_mix].inBlk != VB_INVALID_HANDLE)
				s32Ret |= CVI_VB_ReleaseBlock(param_mix[cur_case_mix].inBlk);
		}

		if (param_mix[cur_case_mix].stVideoFrameOut.stVFrame.u64PhyAddr[0]) {
			param_mix[cur_case_mix].outBlk = CVI_VB_PhysAddr2Handle(param_mix[cur_case_mix].stVideoFrameOut.stVFrame.u64PhyAddr[0]);
			if (param_mix[cur_case_mix].outBlk != VB_INVALID_HANDLE)
				s32Ret |= CVI_VB_ReleaseBlock(param_mix[cur_case_mix].outBlk);
		}

		if (s32Ret) {
			DWA_UT_PRT("release VB fail.\n");
			goto EXIT;
		}
	}
EXIT:
	pthread_exit(0);
}

static CVI_S32 dwa_test_mix(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	DWA_BASIC_TEST_PARAM param = {0};
	char *filename_in[4] = {DWA_FILE_IN_ROT, DWA_FILE_IN_LDC_BARREL_0P3, DWA_FILE_IN_AFFINE, DWA_FILE_IN_FISHEYE};
	char *filename_out[4] = {DWA_FILE_OUT_ROT0, DWA_FILE_OUT_LDC_BARREL_0P3_0, DWA_FILE_OUT_AFFINE, DWA_FILE_OUT_FISHEYE_PANORAMA};
	char *filename_pef[4] = {DWA_FILE_PEF_ROT0, DWA_FILE_PEF_LDC_BARREL_0P3_0, DWA_FILE_PEF_AFFINE, DWA_FILE_PEF_FISHEYE_PANORAMA};
	DWA_TEST_OP op[4] = {DWA_TEST_ROT, DWA_TEST_LDC, DWA_TEST_AFFINE, DWA_TEST_FISHEYE};
	CVI_U32 WidthIn[4] = {1920, 1920, 1920, 1024};
	CVI_U32 HeightIn[4] = {1080, 1080, 1080, 1024};
	CVI_U32 WidthOut[4] = {1920, 1920, 128, 1280};
	CVI_U32 HeightOut[4] = {1080, 1080, 1280, 720};
	int rc;
	pthread_t tid;
	CVI_U8 times = DWA_REPEAT_TIMES * 10;
	void *ptr;
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
	LDC_ATTR_S stLdcAttr = {CVI_TRUE, 0, 0, 0, 0, 0, -200};
	FISHEYE_ATTR_S stFisheyeAttr;

	param.size_in.u32Width = 1920;
	param.size_in.u32Height = 1080;
	param.size_out.u32Width = 1920;
	param.size_out.u32Height = 1080;
	param.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;

	//snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_mix");
	param.identity.enModId = CVI_ID_USER;
	param.identity.u32ID = 0;
	snprintf(param.identity.Name, sizeof(param.identity.Name), "job_mix");
	param.identity.syncIo = CVI_FALSE;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width, param.size_in.u32Height, param.enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);
	param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width, param.size_out.u32Height, param.enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);

	stVbConf.u32MaxPoolCnt              = 2;
	stVbConf.astCommPool[0].u32BlkSize	= param.u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 10;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize	= param.u32BlkSizeOut;
	stVbConf.astCommPool[1].u32BlkCnt	= 10;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	DWA_UT_PRT("common pool[0] BlkSize %d\n", param.u32BlkSizeIn);
	DWA_UT_PRT("common pool[1] BlkSize %d\n", param.u32BlkSizeOut);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	s32Ret = CVI_DWA_Init();
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_DWA_Init failed!\n");
		goto exit1;
	}

	DwaAsyncDoneFlag = CVI_FALSE;
	struct sched_param t_param;
	pthread_attr_t attr;

	t_param.sched_priority = 99;

	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &t_param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	rc = pthread_create(&tid, NULL, test_async_thread, NULL);
	if (rc < 0) {
		DWA_UT_PRT("rc is %d, threads create fail\n", rc);
		perror("Fail:");
		return CVI_FAILURE;
	}

	do {
		for (CVI_U8 i = 0; i < 4; i++) {
			strcpy(param_mix[i].filename_in, filename_in[i]);
			strcpy(param_mix[i].filename_out, filename_out[i]);
			strcpy(param_mix[i].filename_pef, filename_pef[i]);
			param_mix[i].size_in.u32Width = WidthIn[i];
			param_mix[i].size_in.u32Height = HeightIn[i];
			param_mix[i].size_out.u32Width = WidthOut[i];
			param_mix[i].size_out.u32Height = HeightOut[i];
			param_mix[i].enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
			param_mix[i].op = op[i];
			param_mix[i].identity.enModId = CVI_ID_USER;
			param_mix[i].identity.u32ID = i;
			snprintf(param_mix[i].identity.Name, sizeof(param_mix[i].identity.Name), "job_mix_%d", i);
			param_mix[i].identity.syncIo = CVI_FALSE;

			param_mix[i].hHandle = 0;
			snprintf(param_mix[i].stTask.name, sizeof(param_mix[i].stTask.name), "tsk_mix_%d", i);

			memset(&param_mix[i].stVideoFrameIn, 0, sizeof(param_mix[i].stVideoFrameIn));
			s32Ret = DWAFileToFrame(&param_mix[i].size_in, param_mix[i].enPixelFormat, param_mix[i].filename_in, &param_mix[i].stVideoFrameIn);
			if (s32Ret) {
				DWA_UT_PRT("DWAFileToFrame failed!\n");
				goto exit2;
			}

			memset(&param_mix[i].stVideoFrameOut, 0, sizeof(param_mix[i].stVideoFrameOut));
			s32Ret = DWA_COMM_PrepareFrame(&param_mix[i].size_out, param_mix[i].enPixelFormat, &param_mix[i].stVideoFrameOut);
			if (s32Ret) {
				DWA_UT_PRT("DWA_COMM_PrepareFrame failed!\n");
				goto exit2;
			}

			memset(param_mix[i].stTask.au64privateData, 0, sizeof(param_mix[i].stTask.au64privateData));
			memcpy(&param_mix[i].stTask.stImgIn, &param_mix[i].stVideoFrameIn, sizeof(param_mix[i].stVideoFrameIn));
			memcpy(&param_mix[i].stTask.stImgOut, &param_mix[i].stVideoFrameOut, sizeof(param_mix[i].stVideoFrameOut));

			s32Ret = CVI_DWA_BeginJob(&param_mix[i].hHandle);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_BeginJob failed!\n");
				goto exit2;
			}

			s32Ret = CVI_DWA_SetJobIdentity(param_mix[i].hHandle, &param_mix[i].identity);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_SetJobIdentity failed!\n");
				goto exit2;
			}

			if (i == 0) {
				ptr = (void *)ROTATION_0;
			} else if (i == 1) {
				ptr = (void *)&stLdcAttr;
			} else if (i == 2) {
				stAffineAttr.u32RegionNum = 9;
				memcpy(stAffineAttr.astRegionAttr, faces, sizeof(faces));
				stAffineAttr.stDestSize.u32Width = 112;
				stAffineAttr.stDestSize.u32Height = 112;
				ptr = (void *)&stAffineAttr;
			} else {
				stFisheyeAttr.bEnable = CVI_TRUE;
				stFisheyeAttr.bBgColor = CVI_TRUE;
				stFisheyeAttr.u32BgColor = YUV_8BIT(0, 128, 128);
				stFisheyeAttr.s32HorOffset = param_mix[i].size_in.u32Width / 2;
				stFisheyeAttr.s32VerOffset = param_mix[i].size_in.u32Height / 2;
				stFisheyeAttr.enMountMode = FISHEYE_DESKTOP_MOUNT;
				stFisheyeAttr.enUseMode = MODE_PANORAMA_360;
				stFisheyeAttr.u32RegionNum = 1;
				ptr = (void *)&stFisheyeAttr;
			}

			s32Ret = dwa_basic_add_tsk(&param_mix[i], ptr);
			if (s32Ret != CVI_SUCCESS) {
				DWA_UT_PRT("dwa_basic_add_tsk. s32Ret: 0x%x !\n", s32Ret);
				goto exit2;
			}

			s32Ret = CVI_DWA_EndJob(param_mix[i].hHandle);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_EndJob failed!\n");
				goto exit2;
			}

			pthread_mutex_lock(&g_tMutex_mix);
			g_cur_case = i;
			DWA_UT_PRT("g_cur_case[%d]\n", g_cur_case);
			pthread_mutex_unlock(&g_tMutex_mix);
			usleep(1000*10);
		}
	}while (times--);
exit2:
	DwaAsyncDoneFlag = CVI_TRUE;
	pthread_join(tid, NULL);

	for (CVI_U8 i = 0; i < 4; i++) {
		if (param_mix[i].hHandle) {
			s32Ret |= CVI_DWA_CancelJob(param_mix[i].hHandle);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_CancelJob fail.\n");
			}
		}
	}
	s32Ret |= CVI_DWA_DeInit();
	if (s32Ret) {
		DWA_UT_PRT("CVI_DWA_DeInit fail.\n");
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

	if (s32Ret) {
		DWA_UT_PRT("release VB fail.\n");
	}

exit1:
	s32Ret |= CVI_SYS_Exit();
exit0:
	s32Ret |= CVI_VB_Exit();

	DWA_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}
#endif

static void dwa_sig_handle(int sig)
{
	(void)sig;
	CVI_S32 s32Ret;
	CVI_S32 s32MilliSec = 10;
	int i;

	for (i = 0; i < 4; i++) {
		s32Ret = CVI_DWA_GetChnFrame(&param_mix[i].identity, &param_mix[i].stVideoFrameOut, s32MilliSec);
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_GetChnFrame fail, cur_case[%d]\n", i);
		} else
			break;
	}

	if (i >= 4) {
		DWA_UT_PRT("[%s] fail\n", __FUNCTION__);
		return;
	}
	DWA_UT_PRT("[%s]\n", param_mix[i].filename_in);
	DWA_UT_PRT("[%s]\n", param_mix[i].filename_out);
	DWA_UT_PRT("op[%d]\n", param_mix[i].op);

	DWA_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param_mix[i].stVideoFrameIn.stVFrame.u64PhyAddr[0]
		, param_mix[i].stVideoFrameIn.stVFrame.u64PhyAddr[1], param_mix[i].stVideoFrameIn.stVFrame.u64PhyAddr[2]);
	DWA_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param_mix[i].stVideoFrameOut.stVFrame.u64PhyAddr[0]
		, param_mix[i].stVideoFrameOut.stVFrame.u64PhyAddr[1], param_mix[i].stVideoFrameOut.stVFrame.u64PhyAddr[2]);

	if (param_mix[i].stVideoFrameIn.stVFrame.u64PhyAddr[0]) {
		param_mix[i].inBlk = CVI_VB_PhysAddr2Handle(param_mix[i].stVideoFrameIn.stVFrame.u64PhyAddr[0]);
		if (param_mix[i].inBlk != VB_INVALID_HANDLE)
			s32Ret = CVI_VB_ReleaseBlock(param_mix[i].inBlk);
	}
	if (s32Ret)
		DWA_UT_PRT("release in VB fail\n");

	if (param_mix[i].stVideoFrameOut.stVFrame.u64PhyAddr[0]) {
		param_mix[i].outBlk = CVI_VB_PhysAddr2Handle(param_mix[i].stVideoFrameOut.stVFrame.u64PhyAddr[0]);
		if (param_mix[i].outBlk != VB_INVALID_HANDLE)
			s32Ret = CVI_VB_ReleaseBlock(param_mix[i].outBlk);
	}
	if (s32Ret)
		DWA_UT_PRT("release out VB fail\n");
}

static int dwa_init_fasync(void)
{
	int dwa_fd, flags;

	dwa_fd = CVI_DWA_GetDevFd();
	if (dwa_fd < 0) {
		DWA_UT_PRT("CVI_DWA_GetDevFd fail!\n");
		return -1;
	}

	signal(SIGIO, dwa_sig_handle);
	fcntl(dwa_fd, F_SETOWN, getpid());

	flags = fcntl(dwa_fd, F_GETFL);
	fcntl(dwa_fd, F_SETFL, flags | FASYNC);

	return 0;
}

static int dwa_deinit_fasync(void)
{
	int dwa_fd, flags;

	dwa_fd = CVI_DWA_GetDevFd();
	if (dwa_fd < 0) {
		DWA_UT_PRT("CVI_DWA_GetDevFd fail!\n");
		return -1;
	}

	signal(SIGIO, NULL);

	flags = fcntl(dwa_fd, F_GETFL);
	fcntl(dwa_fd, F_SETFL, flags & (~FASYNC));

	return 0;
}

static CVI_S32 dwa_test_mix(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	DWA_BASIC_TEST_PARAM param = {0};
	char *filename_in[4] = {DWA_FILE_IN_ROT, DWA_FILE_IN_LDC_BARREL_0P3, DWA_FILE_IN_AFFINE, DWA_FILE_IN_FISHEYE};
	char *filename_out[4] = {DWA_FILE_OUT_ROT0, DWA_FILE_OUT_LDC_BARREL_0P3_0, DWA_FILE_OUT_AFFINE, DWA_FILE_OUT_FISHEYE_PANORAMA};
	char *filename_pef[4] = {DWA_FILE_PEF_ROT0, DWA_FILE_PEF_LDC_BARREL_0P3_0, DWA_FILE_PEF_AFFINE, DWA_FILE_PEF_FISHEYE_PANORAMA};
	DWA_TEST_OP op[4] = {DWA_TEST_ROT, DWA_TEST_LDC, DWA_TEST_AFFINE, DWA_TEST_FISHEYE};
	CVI_U32 WidthIn[4] = {1920, 1920, 1920, 1024};
	CVI_U32 HeightIn[4] = {1080, 1080, 1080, 1024};
	CVI_U32 WidthOut[4] = {1920, 1920, 128, 1280};
	CVI_U32 HeightOut[4] = {1080, 1080, 1280, 720};
	CVI_U8 times = DWA_REPEAT_TIMES * 10;
	void *ptr;
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
	LDC_ATTR_S stLdcAttr = {CVI_TRUE, 0, 0, 0, 0, 0, -200, {0}, 0, 0};
	FISHEYE_ATTR_S stFisheyeAttr;

	param.size_in.u32Width = 1920;
	param.size_in.u32Height = 1080;
	param.size_out.u32Width = 1920;
	param.size_out.u32Height = 1080;
	param.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;

	param.identity.enModId = CVI_ID_USER;
	param.identity.u32ID = 0;
	snprintf(param.identity.Name, sizeof(param.identity.Name), "job_mix");
	param.identity.syncIo = CVI_FALSE;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width, param.size_in.u32Height, param.enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);
	param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width, param.size_out.u32Height, param.enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);

	stVbConf.u32MaxPoolCnt              = 2;
	stVbConf.astCommPool[0].u32BlkSize	= param.u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 10;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize	= param.u32BlkSizeOut;
	stVbConf.astCommPool[1].u32BlkCnt	= 10;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	DWA_UT_PRT("common pool[0] BlkSize %d\n", param.u32BlkSizeIn);
	DWA_UT_PRT("common pool[1] BlkSize %d\n", param.u32BlkSizeOut);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	s32Ret = CVI_DWA_Init();
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_DWA_Init failed!\n");
		goto exit1;
	}

	if (dwa_init_fasync()) {
		DWA_UT_PRT("dwa_init_fasync failed!\n");
		goto exit2;
	}

	do {
		for (CVI_U8 i = 0; i < 4; i++) {
			strcpy(param_mix[i].filename_in, filename_in[i]);
			strcpy(param_mix[i].filename_out, filename_out[i]);
			strcpy(param_mix[i].filename_pef, filename_pef[i]);
			param_mix[i].size_in.u32Width = WidthIn[i];
			param_mix[i].size_in.u32Height = HeightIn[i];
			param_mix[i].size_out.u32Width = WidthOut[i];
			param_mix[i].size_out.u32Height = HeightOut[i];
			param_mix[i].enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
			param_mix[i].op = op[i];
			param_mix[i].identity.enModId = CVI_ID_USER;
			param_mix[i].identity.u32ID = i;
			snprintf(param_mix[i].identity.Name, sizeof(param_mix[i].identity.Name), "job_mix_%d", i);
			param_mix[i].identity.syncIo = CVI_FALSE;

			param_mix[i].hHandle = 0;
			snprintf(param_mix[i].stTask.name, sizeof(param_mix[i].stTask.name), "tsk_mix_%d", i);

			memset(&param_mix[i].stVideoFrameIn, 0, sizeof(param_mix[i].stVideoFrameIn));
			s32Ret = DWAFileToFrame(&param_mix[i].size_in, param_mix[i].enPixelFormat, param_mix[i].filename_in, &param_mix[i].stVideoFrameIn);
			if (s32Ret) {
				DWA_UT_PRT("DWAFileToFrame failed!\n");
				goto exit3;
			}

			memset(&param_mix[i].stVideoFrameOut, 0, sizeof(param_mix[i].stVideoFrameOut));
			s32Ret = DWA_COMM_PrepareFrame(&param_mix[i].size_out, param_mix[i].enPixelFormat, &param_mix[i].stVideoFrameOut);
			if (s32Ret) {
				DWA_UT_PRT("DWA_COMM_PrepareFrame failed!\n");
				goto exit3;
			}

			memset(param_mix[i].stTask.au64privateData, 0, sizeof(param_mix[i].stTask.au64privateData));
			memcpy(&param_mix[i].stTask.stImgIn, &param_mix[i].stVideoFrameIn, sizeof(param_mix[i].stVideoFrameIn));
			memcpy(&param_mix[i].stTask.stImgOut, &param_mix[i].stVideoFrameOut, sizeof(param_mix[i].stVideoFrameOut));

			s32Ret = CVI_DWA_BeginJob(&param_mix[i].hHandle);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_BeginJob failed!\n");
				goto exit3;
			}

			s32Ret = CVI_DWA_SetJobIdentity(param_mix[i].hHandle, &param_mix[i].identity);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_SetJobIdentity failed!\n");
				goto exit3;
			}

			if (i == 0) {
				ptr = (void *)ROTATION_0;
			} else if (i == 1) {
				ptr = (void *)&stLdcAttr;
			} else if (i == 2) {
				stAffineAttr.u32RegionNum = 9;
				memcpy(stAffineAttr.astRegionAttr, faces, sizeof(faces));
				stAffineAttr.stDestSize.u32Width = 112;
				stAffineAttr.stDestSize.u32Height = 112;
				ptr = (void *)&stAffineAttr;
			} else {
				stFisheyeAttr.bEnable = CVI_TRUE;
				stFisheyeAttr.bBgColor = CVI_TRUE;
				stFisheyeAttr.u32BgColor = YUV_8BIT(0, 128, 128);
				stFisheyeAttr.s32HorOffset = param_mix[i].size_in.u32Width / 2;
				stFisheyeAttr.s32VerOffset = param_mix[i].size_in.u32Height / 2;
				stFisheyeAttr.enMountMode = FISHEYE_DESKTOP_MOUNT;
				stFisheyeAttr.enUseMode = MODE_PANORAMA_360;
				stFisheyeAttr.u32RegionNum = 1;
				ptr = (void *)&stFisheyeAttr;
			}

			s32Ret = dwa_basic_add_tsk(&param_mix[i], ptr);
			if (s32Ret != CVI_SUCCESS) {
				DWA_UT_PRT("dwa_basic_add_tsk. s32Ret: 0x%x !\n", s32Ret);
				goto exit3;
			}

			s32Ret = CVI_DWA_EndJob(param_mix[i].hHandle);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_EndJob failed!\n");
				goto exit3;
			}

			usleep(1000*10);
		}
	}while (times--);
exit3:
	usleep(1000*500);
	for (CVI_U8 i = 0; i < 4; i++) {
		if (s32Ret && param_mix[i].hHandle) {
			s32Ret |= CVI_DWA_CancelJob(param_mix[i].hHandle);
			if (s32Ret) {
				DWA_UT_PRT("CVI_DWA_CancelJob fail.\n");
			}
		}

		if (param_mix[i].stVideoFrameIn.stVFrame.u64PhyAddr[0]) {
			param_mix[i].inBlk = CVI_VB_PhysAddr2Handle(param_mix[i].stVideoFrameIn.stVFrame.u64PhyAddr[0]);
			if (param_mix[i].inBlk != VB_INVALID_HANDLE)
				s32Ret |= CVI_VB_ReleaseBlock(param_mix[i].inBlk);
		}
		if (param_mix[i].stVideoFrameOut.stVFrame.u64PhyAddr[0]) {
			param_mix[i].outBlk = CVI_VB_PhysAddr2Handle(param_mix[i].stVideoFrameOut.stVFrame.u64PhyAddr[0]);
			if (param_mix[i].outBlk != VB_INVALID_HANDLE)
				s32Ret |= CVI_VB_ReleaseBlock(param_mix[i].outBlk);
		}
		if (s32Ret) {
			DWA_UT_PRT("release VB fail.\n");
		}
	}

exit2:
	dwa_deinit_fasync();

	s32Ret |= CVI_DWA_DeInit();
	if (s32Ret) {
		DWA_UT_PRT("CVI_DWA_DeInit fail.\n");
	}

exit1:
	s32Ret |= CVI_SYS_Exit();
exit0:
	s32Ret |= CVI_VB_Exit();

	DWA_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 dwa_basic_do_job(DWA_BASIC_TEST_PARAM *param, int times, void *op_ptr)
{
	CVI_S32 s32Ret;

	do {
		param->hHandle = 0;
		memset(&param->stVideoFrameIn, 0, sizeof(param->stVideoFrameIn));
		s32Ret = DWAFileToFrame(&param->size_in, param->enPixelFormat, param->filename_in, &param->stVideoFrameIn);
		if (s32Ret) {
			DWA_UT_PRT("DWAFileToFrame failed!\n");
			return CVI_FAILURE;
		}

		memset(&param->stVideoFrameOut, 0, sizeof(param->stVideoFrameOut));
		s32Ret = DWA_COMM_PrepareFrame(&param->size_out, param->enPixelFormat, &param->stVideoFrameOut);
		if (s32Ret) {
			DWA_UT_PRT("DWA_COMM_PrepareFrame failed!\n");
			return CVI_FAILURE;
		}

		memset(param->stTask.au64privateData, 0, sizeof(param->stTask.au64privateData));
		memcpy(&param->stTask.stImgIn, &param->stVideoFrameIn, sizeof(param->stVideoFrameIn));
		memcpy(&param->stTask.stImgOut, &param->stVideoFrameOut, sizeof(param->stVideoFrameOut));

		s32Ret = CVI_DWA_BeginJob(&param->hHandle);
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_BeginJob failed!\n");
			return CVI_FAILURE;
		}

		s32Ret = CVI_DWA_SetJobIdentity(param->hHandle, &param->identity);
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_SetJobIdentity failed!\n");
			return CVI_FAILURE;
		}

		s32Ret = dwa_basic_add_tsk(param, op_ptr);
		if (s32Ret != CVI_SUCCESS) {
			DWA_UT_PRT("dwa_basic_add_tsk. s32Ret: 0x%x !\n", s32Ret);
			return CVI_FAILURE;
		}

		s32Ret = CVI_DWA_EndJob(param->hHandle);
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_EndJob failed!\n");
			return CVI_FAILURE;
		}

#if 0
		if (g_dwa_save_file) {
			s32Ret = DWAFrameSaveToFile(param->filename_out, &param->stVideoFrameOut);
			if (s32Ret != CVI_SUCCESS) {
				DWA_UT_PRT("DWAFrameSaveToFile. s32Ret: 0x%x !\n", s32Ret);
				return CVI_FAILURE;
			}
			DWA_UT_PRT("-------------------times:(%d)----------------------\n", times);
			DWA_UT_PRT("output file:%s\n", param->filename_out);
			DWA_UT_PRT("pef file:%s\n", param->filename_pef);

			if (param->needPef) {
				s32Ret = DWACompareWithFile(param->filename_pef, &param->stVideoFrameOut);
				if (s32Ret != CVI_SUCCESS) {
					DWA_UT_PRT("DWACompareWithFile fail.\n");
					return CVI_FAILURE;
				}
			}
		}
#endif
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
			DWA_UT_PRT("release VB fail.\n");
			break;
		}
	} while (times--);

	return s32Ret;
}

void *dwa_basic_thread_func0(void *data)
{
	CVI_S32 s32Ret;
	DWA_BASIC_TEST_PARAM *param = (DWA_BASIC_TEST_PARAM *)(data);
	int times = DWA_REPEAT_TIMES * 10;
	void *ptr = (void *)ROTATION_0;

	s32Ret = dwa_basic_do_job(param, times, ptr);
	if (s32Ret) {
		DWA_UT_PRT("test fail.\n");
	}

	if (s32Ret)
		if (param->hHandle)
			s32Ret = CVI_DWA_CancelJob(param->hHandle);
	DWA_CHECK_RET(s32Ret);

	pthread_exit(0);
}
void *dwa_basic_thread_func1(void *data)
{
	CVI_S32 s32Ret;
	DWA_BASIC_TEST_PARAM *param = (DWA_BASIC_TEST_PARAM *)(data);
	int times = DWA_REPEAT_TIMES * 10;
	void *ptr;
	LDC_ATTR_S stLdcAttr = {CVI_TRUE, 0, 0, 0, 0, 0, -200, {0}, 0, 0};

	ptr = (void *)&stLdcAttr;

	s32Ret = dwa_basic_do_job(param, times, ptr);
	if (s32Ret) {
		DWA_UT_PRT("test fail.\n");
	}

	if (s32Ret)
		if (param->hHandle)
			s32Ret = CVI_DWA_CancelJob(param->hHandle);

	DWA_CHECK_RET(s32Ret);

	pthread_exit(0);
}
void *dwa_basic_thread_func2(void *data)
{
	CVI_S32 s32Ret;
	DWA_BASIC_TEST_PARAM *param = (DWA_BASIC_TEST_PARAM *)(data);
	int times = DWA_REPEAT_TIMES * 10;
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

	s32Ret = dwa_basic_do_job(param, times, ptr);
	if (s32Ret) {
		DWA_UT_PRT("test fail.\n");
	}

	if (s32Ret)
		if (param->hHandle)
			s32Ret = CVI_DWA_CancelJob(param->hHandle);

	DWA_CHECK_RET(s32Ret);

	pthread_exit(0);
}
void *dwa_basic_thread_func3(void *data)
{
	CVI_S32 s32Ret;
	DWA_BASIC_TEST_PARAM *param = (DWA_BASIC_TEST_PARAM *)(data);
	int times = DWA_REPEAT_TIMES * 10;
	void *ptr;
	FISHEYE_ATTR_S stFisheyeAttr;

	stFisheyeAttr.bEnable = CVI_TRUE;
	stFisheyeAttr.bBgColor = CVI_TRUE;
	stFisheyeAttr.u32BgColor = YUV_8BIT(0, 128, 128);
	stFisheyeAttr.s32HorOffset = param->size_in.u32Width / 2;
	stFisheyeAttr.s32VerOffset = param->size_in.u32Height / 2;
	stFisheyeAttr.enMountMode = FISHEYE_DESKTOP_MOUNT;
	stFisheyeAttr.enUseMode = MODE_PANORAMA_360;
	stFisheyeAttr.u32RegionNum = 1;
	ptr = (void *)&stFisheyeAttr;

	s32Ret = dwa_basic_do_job(param, times, ptr);
	if (s32Ret) {
		DWA_UT_PRT("test fail.\n");
	}

	if (s32Ret)
		if (param->hHandle)
			s32Ret = CVI_DWA_CancelJob(param->hHandle);

	DWA_CHECK_RET(s32Ret);

	pthread_exit(0);
}

static CVI_S32 dwa_test_multi_thread(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	DWA_BASIC_TEST_PARAM param[4] = {0};
	char *filename_in[4] = {DWA_FILE_IN_ROT, DWA_FILE_IN_LDC_BARREL_0P3, DWA_FILE_IN_AFFINE, DWA_FILE_IN_FISHEYE};
	char *filename_out[4] = {DWA_FILE_OUT_ROT0, DWA_FILE_OUT_LDC_BARREL_0P3_0, DWA_FILE_OUT_AFFINE, DWA_FILE_OUT_FISHEYE_PANORAMA};
	char *filename_pef[4] = {DWA_FILE_PEF_ROT0, DWA_FILE_PEF_LDC_BARREL_0P3_0, DWA_FILE_PEF_AFFINE, DWA_FILE_PEF_FISHEYE_PANORAMA};
	DWA_TEST_OP op[4] = {DWA_TEST_ROT, DWA_TEST_LDC, DWA_TEST_AFFINE, DWA_TEST_FISHEYE};
	CVI_U32 WidthIn[4] = {1920, 1920, 1920, 1024};
	CVI_U32 HeightIn[4] = {1080, 1080, 1080, 1024};
	CVI_U32 WidthOut[4] = {1920, 1920, 128, 1280};
	CVI_U32 HeightOut[4] = {1080, 1080, 1280, 720};
	int i, rc;
	pthread_t thread[4] = {[0 ... 3] = -1};

	param[0].size_in.u32Width = 1920;
	param[0].size_in.u32Height = 1080;
	param[0].size_out.u32Width = 1920;
	param[0].size_out.u32Height = 1080;
	param[0].enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	param[0].u32BlkSizeIn = COMMON_GetPicBufferSize(param[0].size_in.u32Width, param[0].size_in.u32Height, param[0].enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);
	param[0].u32BlkSizeOut = COMMON_GetPicBufferSize(param[0].size_out.u32Width, param[0].size_out.u32Height, param[0].enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);

	stVbConf.u32MaxPoolCnt              = 2;
	stVbConf.astCommPool[0].u32BlkSize	= param[0].u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 10;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize	= param[0].u32BlkSizeOut;
	stVbConf.astCommPool[1].u32BlkCnt	= 10;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	DWA_UT_PRT("common pool[0] BlkSize %d\n", param[0].u32BlkSizeIn);
	DWA_UT_PRT("common pool[1] BlkSize %d\n", param[0].u32BlkSizeOut);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	s32Ret = CVI_DWA_Init();
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_DWA_Init failed!\n");
		goto exit1;
	}

	for (i = 0; i < 4; i++) {
		param[i].enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
		param[i].needPef = CVI_TRUE;
		param[i].identity.syncIo = CVI_TRUE;
		strcpy(param[i].filename_in, filename_in[i]);
		strcpy(param[i].filename_out, filename_out[i]);
		strcpy(param[i].filename_pef, filename_pef[i]);
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
			rc = pthread_create(&thread[i], NULL, dwa_basic_thread_func0, (void *)&param[i]);
		else if (i == 1)
			rc = pthread_create(&thread[i], NULL, dwa_basic_thread_func1, (void *)&param[i]);
		else if (i == 2)
			rc = pthread_create(&thread[i], NULL, dwa_basic_thread_func2, (void *)&param[i]);
		else
			rc = pthread_create(&thread[i], NULL, dwa_basic_thread_func3, (void *)&param[i]);

		if (rc) {
			DWA_UT_PRT("pthread_create fail. s32Ret: 0x%x !\n", s32Ret);
			break;
		}
	}

	for (i = 0; i < 4; i++)
		pthread_join(thread[i], NULL);

	s32Ret |= CVI_DWA_DeInit();
	if (s32Ret) {
		DWA_UT_PRT("CVI_DWA_DeInit failed!\n");
	}
exit1:
	s32Ret |= CVI_SYS_Exit();
exit0:
	s32Ret |= CVI_VB_Exit();

	DWA_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 dwa_test_pef(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	p_func test_func[4] = {dwa_test_rot, dwa_test_rot_4m, dwa_test_ldc, dwa_test_fmt};
	bEnProc = CVI_TRUE;

	for (int i =0 ;i < 4; i++)
		s32Ret |= test_func[i]();

	bEnProc = CVI_FALSE;

	return s32Ret;
}

static CVI_S32 dwa_test_ldc_grid_info(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	LDC_ATTR_S stLdcAttr[2] = {0};
	DWA_BASIC_TEST_PARAM param[2] = {0};
	char *filename_in[2] = {DWA_FILE_IN_LDC_GRID_INFO_L, DWA_FILE_IN_LDC_GRID_INFO_R};
	char *filename_out[2] = {DWA_FILE_OUT_LDC_GRID_INFO_L, DWA_FILE_OUT_LDC_GRID_INFO_R};
	char *filename_pef[2] = {DWA_FILE_PEF_LDC_GRID_INFO_L, DWA_FILE_PEF_LDC_GRID_INFO_R};;
	char *filename_grid[2] = {DWA_FILE_IN_LDC_GRID_L, DWA_FILE_IN_LDC_GRID_R};
	int times = 2;

	for (int i = 0; i < 2; i++) {
		param[i].needPef = CVI_FALSE;
		strcpy(param[i].filename_in, filename_in[i]);
		strcpy(param[i].filename_out, filename_out[i]);
		strcpy(param[i].filename_pef, filename_pef[i]);
		param[i].size_in.u32Width = 1280;
		param[i].size_in.u32Height = 720;
		param[i].size_out.u32Width = 1280;
		param[i].size_out.u32Height = 720;
		param[i].enPixelFormat = PIXEL_FORMAT_YUV_400;
		snprintf(param[i].stTask.name, sizeof(param[i].stTask.name), "dwa_tsk_ldc_grid_%d", i);
		param[i].identity.enModId = CVI_ID_USER;
		param[i].identity.u32ID = 0;
		snprintf(param[i].identity.Name, sizeof(param[i].identity.Name), "dwa_job_ldc_grid_%d", i);
		param[i].identity.syncIo = (i==0 ? CVI_FALSE : CVI_TRUE);

		param[i].op = DWA_TEST_LDC;
		stLdcAttr[i].stGridInfoAttr.Enable = CVI_TRUE;
		strcpy(stLdcAttr[i].stGridInfoAttr.gridFileName, filename_grid[i]);
		strcpy(stLdcAttr[i].stGridInfoAttr.gridBindName, param[i].stTask.name);
	}

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	param[0].u32BlkSizeIn = COMMON_GetPicBufferSize(param[0].size_in.u32Width, param[0].size_in.u32Height, param[0].enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);
	param[0].u32BlkSizeOut = COMMON_GetPicBufferSize(param[0].size_out.u32Width, param[0].size_out.u32Height, param[0].enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);

	stVbConf.u32MaxPoolCnt              = 1;
	stVbConf.astCommPool[0].u32BlkSize	= param[0].u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 8;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	DWA_UT_PRT("common pool[0] BlkSize %d\n", param[0].u32BlkSizeIn);

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	s32Ret = CVI_DWA_Init();
	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("CVI_DWA_Init failed!\n");
		goto exit1;
	}

	do {
		param[0].hHandle = 0;
		param[1].hHandle = 0;
		memset(&param[0].stVideoFrameIn, 0, sizeof(param[0].stVideoFrameIn));
		memset(&param[1].stVideoFrameIn, 0, sizeof(param[1].stVideoFrameIn));
		s32Ret = DWAFileToFrame(&param[0].size_in, param[0].enPixelFormat, param[0].filename_in, &param[0].stVideoFrameIn);
		if (s32Ret) {
			DWA_UT_PRT("DWAFileToFrame failed!\n");
			goto exit2;
		}
		s32Ret = DWAFileToFrame(&param[1].size_in, param[1].enPixelFormat, param[1].filename_in, &param[1].stVideoFrameIn);
		if (s32Ret) {
			DWA_UT_PRT("DWAFileToFrame failed!\n");
			goto exit2;
		}

		memset(&param[0].stVideoFrameOut, 0, sizeof(param[0].stVideoFrameOut));
		memset(&param[1].stVideoFrameOut, 0, sizeof(param[1].stVideoFrameOut));
		s32Ret = DWA_COMM_PrepareFrame(&param[0].size_out, param[0].enPixelFormat, &param[0].stVideoFrameOut);
		if (s32Ret) {
			DWA_UT_PRT("DWA_COMM_PrepareFrame failed!\n");
			goto exit2;
		}
		s32Ret = DWA_COMM_PrepareFrame(&param[1].size_out, param[1].enPixelFormat, &param[1].stVideoFrameOut);
		if (s32Ret) {
			DWA_UT_PRT("DWA_COMM_PrepareFrame failed!\n");
			goto exit2;
		}

		memset(param[0].stTask.au64privateData, 0, sizeof(param[0].stTask.au64privateData));
		memset(param[1].stTask.au64privateData, 0, sizeof(param[1].stTask.au64privateData));
		memcpy(&param[0].stTask.stImgIn, &param[0].stVideoFrameIn, sizeof(param[0].stVideoFrameIn));
		memcpy(&param[0].stTask.stImgOut, &param[0].stVideoFrameOut, sizeof(param[0].stVideoFrameOut));
		memcpy(&param[1].stTask.stImgIn, &param[1].stVideoFrameIn, sizeof(param[1].stVideoFrameIn));
		memcpy(&param[1].stTask.stImgOut, &param[1].stVideoFrameOut, sizeof(param[1].stVideoFrameOut));
		s32Ret = CVI_DWA_BeginJob(&param[0].hHandle);
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_BeginJob failed!\n");
			goto exit2;
		}
		s32Ret = CVI_DWA_BeginJob(&param[1].hHandle);
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_BeginJob failed!\n");
			goto exit2;
		}

		s32Ret = CVI_DWA_SetJobIdentity(param[0].hHandle, &param[0].identity);
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_SetJobIdentity failed!\n");
			goto exit2;
		}
		s32Ret = CVI_DWA_SetJobIdentity(param[1].hHandle, &param[1].identity);
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_SetJobIdentity failed!\n");
			goto exit2;
		}

		s32Ret = CVI_DWA_AddLDCTask(param[0].hHandle, &param[0].stTask, &stLdcAttr[0], ROTATION_0);
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_AddLDCTask failed!\n");
		}
		s32Ret = CVI_DWA_AddLDCTask(param[1].hHandle, &param[1].stTask, &stLdcAttr[1], ROTATION_0);
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_AddLDCTask failed!\n");
		}


		s32Ret = CVI_DWA_EndJob(param[0].hHandle);
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_EndJob failed!\n");
			goto exit2;
		}
		s32Ret = CVI_DWA_EndJob(param[1].hHandle);
		if (s32Ret) {
			DWA_UT_PRT("CVI_DWA_EndJob failed!\n");
			goto exit2;
		}

		if (g_dwa_save_file) {
			s32Ret = DWAFrameSaveToFile(param[0].filename_out, &param[0].stVideoFrameOut);
			if (s32Ret != CVI_SUCCESS) {
				DWA_UT_PRT("DWAFrameSaveToFile. s32Ret: 0x%x !\n", s32Ret);
				goto exit2;
			}
			DWA_UT_PRT("output file:%s\n", param[0].filename_out);
			DWA_UT_PRT("pef file:%s\n", param[0].filename_pef);

			s32Ret = DWAFrameSaveToFile(param[1].filename_out, &param[1].stVideoFrameOut);
			if (s32Ret != CVI_SUCCESS) {
				DWA_UT_PRT("DWAFrameSaveToFile. s32Ret: 0x%x !\n", s32Ret);
				goto exit2;
			}
			DWA_UT_PRT("output file:%s\n", param[1].filename_out);
			DWA_UT_PRT("pef file:%s\n", param[1].filename_pef);

			if (param[0].needPef) {
				s32Ret = DWACompareWithFile(param[0].filename_pef, &param[0].stVideoFrameOut);
				if (s32Ret != CVI_SUCCESS) {
					DWA_UT_PRT("DWACompareWithFile fail.\n");
					goto exit2;
				}
			}
			if (param[1].needPef) {
				s32Ret = DWACompareWithFile(param[1].filename_pef, &param[1].stVideoFrameOut);
				if (s32Ret != CVI_SUCCESS) {
					DWA_UT_PRT("DWACompareWithFile fail.\n");
					goto exit2;
				}
			}
		}

		if (param[0].stVideoFrameIn.stVFrame.u64PhyAddr[0]) {
			param[0].inBlk = CVI_VB_PhysAddr2Handle(param[0].stVideoFrameIn.stVFrame.u64PhyAddr[0]);
			if (param[0].inBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param[0].inBlk);
				param[0].inBlk = VB_INVALID_HANDLE;
			}
		}
		if (param[0].stVideoFrameOut.stVFrame.u64PhyAddr[0]) {
			param[0].outBlk = CVI_VB_PhysAddr2Handle(param[0].stVideoFrameOut.stVFrame.u64PhyAddr[0]);
			if (param[0].outBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param[0].outBlk);
				param[0].outBlk = VB_INVALID_HANDLE;
			}
		}
		if (param[1].stVideoFrameIn.stVFrame.u64PhyAddr[0]) {
			param[1].inBlk = CVI_VB_PhysAddr2Handle(param[1].stVideoFrameIn.stVFrame.u64PhyAddr[0]);
			if (param[1].inBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param[1].inBlk);
				param[1].inBlk = VB_INVALID_HANDLE;
			}
		}
		if (param[1].stVideoFrameOut.stVFrame.u64PhyAddr[0]) {
			param[1].outBlk = CVI_VB_PhysAddr2Handle(param[1].stVideoFrameOut.stVFrame.u64PhyAddr[0]);
			if (param[1].outBlk != VB_INVALID_HANDLE) {
				s32Ret |= CVI_VB_ReleaseBlock(param[1].outBlk);
				param[1].outBlk = VB_INVALID_HANDLE;
			}
		}

		if (s32Ret) {
			DWA_UT_PRT("release VB fail.\n");
			break;
		}
	} while (times--);

exit2:
	s32Ret |= CVI_DWA_DeInit();
exit1:
	s32Ret |= CVI_SYS_Exit();
exit0:
	s32Ret |= CVI_VB_Exit();

	DWA_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 dwa_test_presure_size_for_each(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int times = DWA_REPEAT_TIMES;
	VB_CONFIG_S stVbConf;
	DWA_BASIC_TEST_PARAM param = {0};
	PIXEL_FORMAT_E enPixelFormat[4] = {
		PIXEL_FORMAT_YUV_PLANAR_420,
		PIXEL_FORMAT_YUV_400,
		PIXEL_FORMAT_YUV_PLANAR_444,
		PIXEL_FORMAT_RGB_888_PLANAR
	};

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	for (int w = DWA_MIN_W; w < DWA_MAX_W; w += DWA_MIN_W) {
		for (int h = DWA_MIN_H; h < DWA_MAX_H; h += DWA_MIN_H) {
			for (CVI_U8 fmt_idx = 0; fmt_idx < 4; fmt_idx ++) {
				param.size_in.u32Width = w;
				param.size_in.u32Height = h;
				param.size_out.u32Width = w;
				param.size_out.u32Height = h;
				param.enPixelFormat = enPixelFormat[fmt_idx];
				snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_fmt%d_w%d_h%d", fmt_idx, w, h);
				param.identity.enModId = CVI_ID_USER;
				param.identity.u32ID = (w + h);
				snprintf(param.identity.Name, sizeof(param.identity.Name),  "job_fmt%d_w%d_h%d", fmt_idx, w, h);
				param.identity.syncIo = CVI_TRUE;
				param.op = DWA_TEST_ROT;
				param.u32BlkSizeIn = COMMON_GetPicBufferSize(DWA_MAX_W, DWA_MAX_H, PIXEL_FORMAT_YUV_PLANAR_444
					, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);
				param.u32BlkSizeOut = COMMON_GetPicBufferSize(DWA_MAX_W, DWA_MAX_H, PIXEL_FORMAT_YUV_PLANAR_444
					, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN);

				stVbConf.u32MaxPoolCnt				= 2;
				stVbConf.astCommPool[0].u32BlkSize	= param.u32BlkSizeIn;
				stVbConf.astCommPool[0].u32BlkCnt	= 2;
				stVbConf.astCommPool[0].enRemapMode = VB_REMAP_MODE_CACHED;
				stVbConf.astCommPool[1].u32BlkSize	= param.u32BlkSizeOut;
				stVbConf.astCommPool[1].u32BlkCnt	= 2;
				stVbConf.astCommPool[1].enRemapMode = VB_REMAP_MODE_CACHED;
				DWA_UT_PRT("common pool[0] BlkSize %d\n", param.u32BlkSizeIn);
				DWA_UT_PRT("common pool[1] BlkSize %d\n", param.u32BlkSizeOut);

				s32Ret = CVI_VB_SetConfig(&stVbConf);
				if (s32Ret != CVI_SUCCESS) {
					DWA_UT_PRT("CVI_VB_SetConf failed!\n");
					return s32Ret;
				}

				s32Ret = CVI_VB_Init();
				if (s32Ret != CVI_SUCCESS) {
					DWA_UT_PRT("CVI_VB_Init failed!\n");
					return s32Ret;
				}

				s32Ret = CVI_SYS_Init();
				if (s32Ret != CVI_SUCCESS) {
					DWA_UT_PRT("CVI_SYS_Init failed!\n");
					goto exit0;
				}

				s32Ret = CVI_DWA_Init();
				if (s32Ret != CVI_SUCCESS) {
					DWA_UT_PRT("CVI_DWA_Init failed!\n");
					goto exit1;
				}

				times = DWA_REPEAT_TIMES;
				do {
					memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
					s32Ret = DWA_COMM_PrepareFrame(&param.size_in, param.enPixelFormat, &param.stVideoFrameIn);
					if (s32Ret) {
						DWA_UT_PRT("DWA_COMM_PrepareFrame in failed!\n");
						goto exit2;
					}

					memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
					s32Ret = DWA_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut);
					if (s32Ret) {
						DWA_UT_PRT("DWA_COMM_PrepareFrame out failed!\n");
						goto exit2;
					}

					memset(param.stTask.au64privateData, 0, sizeof(param.stTask.au64privateData));
					memcpy(&param.stTask.stImgIn, &param.stVideoFrameIn, sizeof(param.stVideoFrameIn));
					memcpy(&param.stTask.stImgOut, &param.stVideoFrameOut, sizeof(param.stVideoFrameOut));

					s32Ret = CVI_DWA_BeginJob(&param.hHandle);
					if (s32Ret) {
						DWA_UT_PRT("CVI_DWA_BeginJob failed!\n");
						goto exit2;
					}

					s32Ret = CVI_DWA_SetJobIdentity(param.hHandle, &param.identity);
					if (s32Ret) {
						DWA_UT_PRT("CVI_DWA_SetJobIdentity failed!\n");
						goto exit2;
					}

					s32Ret = CVI_DWA_AddRotationTask(param.hHandle, &param.stTask, ROTATION_0);
					if (s32Ret) {
						DWA_UT_PRT("CVI_DWA_AddRotationTask failed!\n");
					}

					s32Ret = CVI_DWA_EndJob(param.hHandle);
					if (s32Ret) {
						DWA_UT_PRT("CVI_DWA_EndJob failed!\n");
						goto exit2;
					}

					DWA_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param.stVideoFrameIn.stVFrame.u64PhyAddr[0]
						, param.stVideoFrameIn.stVFrame.u64PhyAddr[1], param.stVideoFrameIn.stVFrame.u64PhyAddr[2]);
					DWA_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param.stVideoFrameOut.stVFrame.u64PhyAddr[0]
						, param.stVideoFrameOut.stVFrame.u64PhyAddr[1], param.stVideoFrameOut.stVFrame.u64PhyAddr[2]);
					DWA_UT_PRT("-------------------times:(%d)----------------------\n", times);

					param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
					if (param.inBlk != VB_INVALID_HANDLE)
						s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);

					param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
					if (param.outBlk != VB_INVALID_HANDLE)
						s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);

					if (s32Ret) {
						DWA_UT_PRT("release VB fail.\n");
						break;
					}
				} while (times--);
			exit2:
				if (s32Ret && param.hHandle)
					s32Ret |= CVI_DWA_CancelJob(param.hHandle);
				s32Ret |= CVI_DWA_DeInit();
				if (s32Ret) {
					DWA_UT_PRT("CVI_DWA_DeInit fail.\n");
				}

				param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
				if (param.inBlk != VB_INVALID_HANDLE)
					s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);

				param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
				if (param.outBlk != VB_INVALID_HANDLE)
					s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);
			exit1:
				s32Ret |= CVI_SYS_Exit();
			exit0:
				s32Ret |= CVI_VB_Exit();

				if (s32Ret)
					goto err;
			}
		}
	}

err:
	DWA_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 dwa_test_auto_regression(CVI_VOID)
{
	CVI_S32 s32Ret[100] = {[0 ... 99] = CVI_SUCCESS};
	CVI_S32 Ret = CVI_SUCCESS;
	p_func test_func[MAX_FUNC_CNT] = {
		dwa_test_fisheye,
		dwa_test_rot,
		dwa_test_rot_small,
		dwa_test_rot_4m,
		dwa_test_ldc,
		dwa_test_affine,
		dwa_test_rot_maxsize,
		dwa_test_fmt,
		dwa_test_not_align,
		dwa_test_cmdq,
		dwa_test_cmdq_1to2,
		dwa_test_cmdq_1to2_maxsize,
		dwa_test_online,
		dwa_test_mix,
		dwa_test_async,
		dwa_test_multi_thread,
		dwa_test_pef,
		dwa_test_ldc_grid_info,
		dwa_test_reset,
		//dwa_test_presure_size_for_each();//it takes too long time
	};

	for (int i = 0; i < MAX_FUNC_CNT; i++) {
		if (test_func[i]) {
			s32Ret[i] = test_func[i]();
			Ret |= s32Ret[i];
		}
	}

	for (int i = 0; i < MAX_FUNC_CNT; i++) {
		if (s32Ret[i])
			DWA_UT_PRT("op[%d] fail, ret[%d]\n", i, s32Ret[i]);
	}

	DWA_TEST_CHECK_RET(Ret);
	return Ret;
}

static CVI_S32 dwa_test_user_config(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32WidthIn, u32HeightIn;
	CVI_U32 u32WidthOut, u32HeightOut;
	FISHEYE_ATTR_S stFisheyeAttr;
	int fmt;
	int op, tmp;
	CVI_U8 i;
	DWA_BASIC_TEST_PARAM stTestParam = {0};

	memset(&stTestParam, 0, sizeof(stTestParam));
	printf("\n---dwa config---\n");
	printf("input width:");
	scanf("%d", &u32WidthIn);
	printf("input height:");
	scanf("%d", &u32HeightIn);

	printf("output width:");
	scanf("%d", &u32WidthOut);
	printf("output height:");
	scanf("%d", &u32HeightOut);

	printf("format list:\n");
	for (i = PIXEL_FORMAT_RGB_888; i < PIXEL_FORMAT_MAX; i++) {
		if (strncmp(DWAGetFmtName(i), "unknown", sizeof("unknown")))
			printf("%2d : %s\n", i, DWAGetFmtName(i));
	}
	printf("input format:");
	scanf("%d", &fmt);

	printf("input file:");
	scanf("%s", stTestParam.filename_in);

	stTestParam.size_in.u32Width = u32WidthIn;
	stTestParam.size_in.u32Height = u32HeightIn;
	stTestParam.size_out.u32Width = u32WidthOut;
	stTestParam.size_out.u32Height = u32HeightOut;
	stTestParam.enPixelFormat = (PIXEL_FORMAT_E)fmt;
	stTestParam.identity.syncIo = CVI_TRUE;
	snprintf(stTestParam.filename_out, 128, "res/output/%s_%d_%d_%s.bin", __func__,
		stTestParam.size_out.u32Width,
		stTestParam.size_out.u32Height,
		DWAGetFmtName(stTestParam.enPixelFormat));

	printf("input rot or fisheye:(rot:0,fisheye:1) ");
	scanf("%d", &op);
	printf("\n");
	if (op == 0) {
		ROTATION_E rot;
		printf("input rot:(rot0:0, rot90:1, rot180:2, rot270:3) ");
		scanf("%d", &tmp);
		printf("\n");

		rot = (ROTATION_E)tmp;
		stTestParam.op = DWA_TEST_ROT;
		s32Ret = dwa_basic(&stTestParam, (void *)rot);
	} else {
		printf("input UseMode:\n");
		printf("MODE_PANORAMA_360 = 1\n");
		printf("MODE_PANORAMA_180 = 2\n");
		printf("MODE_01_1O = 3\n");
		printf("MODE_02_1O4R = 4\n");
		printf("MODE_03_4R = 5\n");
		printf("MODE_04_1P2R = 6\n");
		printf("MODE_05_1P2R = 7\n");
		printf("MODE_06_1P = 8\n");
		printf("MODE_07_2P = 9\n");

		scanf("%d", &tmp);
		stFisheyeAttr.enUseMode = (USAGE_MODE)tmp;
		stTestParam.op = DWA_TEST_FISHEYE;
		stFisheyeAttr.bEnable = CVI_TRUE;
		stFisheyeAttr.bBgColor = CVI_TRUE;
		stFisheyeAttr.u32BgColor = YUV_8BIT(0, 128, 128);
		stFisheyeAttr.s32HorOffset = stTestParam.size_in.u32Width / 2;
		stFisheyeAttr.s32VerOffset = stTestParam.size_in.u32Height / 2;
		stFisheyeAttr.enMountMode = FISHEYE_DESKTOP_MOUNT;
		stFisheyeAttr.u32RegionNum = 1;

		s32Ret = dwa_basic(&stTestParam, (void *)&stFisheyeAttr);
	}

	if (s32Ret != CVI_SUCCESS) {
		DWA_UT_PRT("Test failed.\n");
		return s32Ret;
	}

	DWA_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static int dup_fd;
static int dup_fd_bak = 1000;

static CVI_S32 dwa_dup_fd(CVI_VOID)
{
	dup_fd = open( "./dwa_printf_dup_log.txt ", O_CREAT | O_RDWR | O_TRUNC);
	dup2(STDOUT_FILENO, dup_fd_bak);/*backup stdout*/
	dup2(dup_fd, STDOUT_FILENO);
	return CVI_SUCCESS;
}

static CVI_S32 dwa_rst_fd(CVI_VOID)
{
	dup2(dup_fd_bak, fileno(stdout));/*recover stdout*/
	close(dup_fd);
	return CVI_SUCCESS;
}

static CVI_S32 _dwa_handle_op(CVI_S32 op)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	switch (op) {
	case DWA_TEST_FISHEYE:
		s32Ret = dwa_test_fisheye();
		break;
	case DWA_TEST_ROT:
		s32Ret = dwa_test_rot();
		break;
	case DWA_TEST_ROT1:
		s32Ret = dwa_test_rot_small();
		break;
	case DWA_TEST_ROT2:
		s32Ret = dwa_test_rot_4m();
		break;
	case DWA_TEST_LDC:
		s32Ret = dwa_test_ldc();
		break;
	case DWA_TEST_AFFINE:
		s32Ret = dwa_test_affine();
		break;
	case DWA_TEST_MAX_SIZE:
		s32Ret = dwa_test_rot_maxsize();
		break;
	case DWA_TEST_FMT:
		s32Ret = dwa_test_fmt();
		break;
	case DWA_TEST_SIZE_NO_ALIGN:
		s32Ret = dwa_test_not_align();
		break;
	case DWA_TEST_CMDQ:
		s32Ret = dwa_test_cmdq();
		break;
	case DWA_TEST_CMDQ_1TO2:
		s32Ret = dwa_test_cmdq_1to2();
		break;
	case DWA_TEST_CMDQ_1TO2_MAX:
		s32Ret = dwa_test_cmdq_1to2_maxsize();
		break;
	case DWA_TEST_ONLINE:
		s32Ret = dwa_test_online();
		break;
	case DWA_TEST_MIX:
		s32Ret = dwa_test_mix();
		break;
	case DWA_TEST_ASYNC:
		s32Ret = dwa_test_async();
		break;
	case DWA_TEST_MULTI_THREAD:
		s32Ret = dwa_test_multi_thread();
		break;
	case DWA_TEST_PEF:
		s32Ret = dwa_test_pef();
		break;
	case DWA_TEST_LDC_GRID_INFO:
		s32Ret = dwa_test_ldc_grid_info();
		break;
	case DWA_TEST_RST:
		s32Ret = dwa_test_reset();
		break;
	case DWA_TEST_PRESURE_SIZE_FOR_EACH:
		s32Ret = dwa_test_presure_size_for_each();
		break;
	case DWA_TEST_AUTO_REGRESSION:
		s32Ret = dwa_test_auto_regression();
		break;
	case DWA_TEST_USER_CONFIG:
		s32Ret = dwa_test_user_config();
		break;
	case DWA_TEST_DUP_FD:
		s32Ret = dwa_dup_fd();
		break;
	case DWA_TEST_RST_FD:
		s32Ret = dwa_rst_fd();
		break;
	default:
		s32Ret = CVI_FAILURE;
		break;
	}

	return s32Ret;
}

static void dwa_show_help(void)
{
	DWA_UT_PRT("%4d: dwa basic test fisheye\n", DWA_TEST_FISHEYE);
	DWA_UT_PRT("%4d: dwa basic test rot\n", DWA_TEST_ROT);
	DWA_UT_PRT("%4d: dwa basic test rot small\n", DWA_TEST_ROT1);
	DWA_UT_PRT("%4d: dwa basic test rot 4m\n", DWA_TEST_ROT2);
	DWA_UT_PRT("%4d: dwa basic test ldc\n", DWA_TEST_LDC);
	DWA_UT_PRT("%4d: dwa basic test affine\n", DWA_TEST_AFFINE);
	DWA_UT_PRT("%4d: dwa basic test maxsize\n", DWA_TEST_MAX_SIZE);
	DWA_UT_PRT("%4d: dwa basic test fmt\n", DWA_TEST_FMT);
	DWA_UT_PRT("%4d: dwa basic test size not align\n", DWA_TEST_SIZE_NO_ALIGN);
	DWA_UT_PRT("%4d: dwa basic test cmdq\n", DWA_TEST_CMDQ);
	DWA_UT_PRT("%4d: dwa basic test cmdq_1to2\n", DWA_TEST_CMDQ_1TO2);
	DWA_UT_PRT("%4d: dwa basic test cmdq_1to2 maxsize\n", DWA_TEST_CMDQ_1TO2_MAX);
	DWA_UT_PRT("%4d: dwa basic test online\n", DWA_TEST_ONLINE);
	DWA_UT_PRT("%4d: dwa basic test mix\n", DWA_TEST_MIX);
	DWA_UT_PRT("%4d: dwa basic test async\n", DWA_TEST_ASYNC);
	DWA_UT_PRT("%4d: dwa basic test multi thread\n", DWA_TEST_MULTI_THREAD);
	DWA_UT_PRT("%4d: dwa basic test pef\n", DWA_TEST_PEF);
	DWA_UT_PRT("%4d: dwa basic test ldc grid_info\n", DWA_TEST_LDC_GRID_INFO);
	DWA_UT_PRT("%4d: dwa basic test reset\n", DWA_TEST_RST);
	DWA_UT_PRT("%4d: dwa test presure size for each\n", DWA_TEST_PRESURE_SIZE_FOR_EACH);
	DWA_UT_PRT("%4d: dwa test auto regression\n", DWA_TEST_AUTO_REGRESSION);
	DWA_UT_PRT("%4d: dwa user cofig test\n", DWA_TEST_USER_CONFIG);
	DWA_UT_PRT("%4d: dwa dup fd\n", DWA_TEST_DUP_FD);
	DWA_UT_PRT("%4d: dwa dup fd\n", DWA_TEST_RST_FD);

	DWA_UT_PRT("255: exit\n");
}

int main(int argc, char **argv)
{
	CVI_S32 s32Ret;
	CVI_S32 op = 255;

	system("stty erase ^H");

	signal(SIGINT, dwa_ut_HandleSig);
	signal(SIGTERM, dwa_ut_HandleSig);

	if (argc >= 2) {
		op = (CVI_S32)atoi(argv[1]);
		g_dwa_save_file = (CVI_BOOL)atoi(argv[2]);
		s32Ret = _dwa_handle_op(op);
		DWA_UT_PRT("dwa ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	} else {
		g_dwa_save_file = CVI_TRUE;
		do {
			dwa_show_help();
			scanf("%d", &op);

			s32Ret = _dwa_handle_op(op);
			if (op != 255)
				DWA_UT_PRT("dwa ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
		} while (op != 255);
	}

	return s32Ret;
}

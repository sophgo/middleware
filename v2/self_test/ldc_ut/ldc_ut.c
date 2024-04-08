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
#include "gdc_mesh.h"

#include "ldc_ut_comm.h"

#ifndef BIT
#define BIT(nr)      (UINT64_C(1) << (nr))
#endif

//file: http://disk-sophgo-vip.quickconnect.cn/sharing/iglxSGiJB

#define GDC_FILE_IN_ROT                         "res/ldc/input/1920x1080.yuv"
#define GDC_FILE_OUT_ROT0                       "res/ldc/output/1920x1080_rot0.yuv"
#define GDC_FILE_OUT_ROT90                      "res/ldc/output/1920x1080_rot90.yuv"
#define GDC_FILE_OUT_ROT270                     "res/ldc/output/1920x1080_rot270.yuv"
#define GDC_FILE_PEF_ROT0                       "res/ldc/pef/1920x1080_rot0.yuv"
#define GDC_FILE_PEF_ROT90                      "res/ldc/pef/1920x1080_rot90.yuv"
#define GDC_FILE_PEF_ROT270                     "res/ldc/pef/1920x1080_rot270.yuv"
#define GDC_FILE_IN_ROT_4M                      "res/ldc/input/2560x1440.yuv"
#define GDC_FILE_OUT_ROT0_4M                    "res/ldc/output/2560x1440_rot0.yuv"
#define GDC_FILE_OUT_ROT90_4M                   "res/ldc/output/2560x1440_rot90.yuv"
#define GDC_FILE_OUT_ROT270_4M                  "res/ldc/output/2560x1440_rot270.yuv"
#define GDC_FILE_PEF_ROT0_4M                    "res/ldc/pef/2560x1440_rot0.yuv"
#define GDC_FILE_PEF_ROT90_4M                   "res/ldc/pef/2560x1440_rot90.yuv"
#define GDC_FILE_PEF_ROT270_4M                  "res/ldc/pef/2560x1440_rot270.yuv"

#define GDC_FILE_IN_ROT_1                         "res/ldc/input/128x128.yuv"
#define GDC_FILE_OUT_ROT0_1                       "res/ldc/output/128x128_rot0.yuv"
#define GDC_FILE_PEF_ROT0_1                       "res/ldc/pef/128x128_rot0.yuv"
#define GDC_FILE_OUT_ROT90_1                       "res/ldc/output/128x128_rot90.yuv"
#define GDC_FILE_PEF_ROT90_1                       "res/ldc/pef/128x128_rot90.yuv"
#define GDC_FILE_OUT_ROT270_1                       "res/ldc/output/128x128_rot270.yuv"
#define GDC_FILE_PEF_ROT270_1                       "res/ldc/pef/128x128_rot270.yuv"

#define GDC_FILE_IN_ROT_2                         "res/ldc/input/4608x4608.yuv"
#define GDC_FILE_OUT_ROT0_2                       "res/ldc/output/4608x4608_rot0.yuv"
#define GDC_FILE_PEF_ROT0_2                       "res/ldc/pef/4608x4608_rot0.yuv"
#define GDC_FILE_OUT_ROT90_2                       "res/ldc/output/4608x4608_rot90.yuv"
#define GDC_FILE_PEF_ROT90_2                       "res/ldc/pef/4608x4608_rot90.yuv"
#define GDC_FILE_OUT_ROT270_2                       "res/ldc/output/4608x4608_rot270.yuv"
#define GDC_FILE_PEF_ROT270_2                       "res/ldc/pef/4608x4608_rot270.yuv"
#define GDC_FILE_OUT_ROT_XYFLIP_2                   "res/ldc/output/4608x4608_rot_xy_flip.yuv"
#define GDC_FILE_PEF_ROT_XYFLIP_2                   "res/ldc/pef/4608x4608_rot_xy_flip.yuv"

#define GDC_FILE_IN_LDC_BARREL_0P3              "res/ldc/input/1920x1080_barrel_0.3.yuv"
#define GDC_FILE_OUT_LDC_BARREL_0P3_0           "res/ldc/output/1920x1080_barrel_0.3_r0_ofst_0_0_d-200.yuv"
#define GDC_FILE_OUT_LDC_BARREL_0P3_1           "res/ldc/output/1920x1080_barrel_0.3_r50_ofst_0_0_d-200.yuv"
#define GDC_FILE_OUT_LDC_ROT_BARREL_0P3_2       "res/ldc/output/1920x1080_barrel_rot_0.3_r100_ofst_0_0_d-200.yuv"
#define GDC_FILE_PEF_LDC_BARREL_0P3_0           "res/ldc/pef/1920x1080_barrel_0.3_r0_ofst_0_0_d-200.yuv"
#define GDC_FILE_PEF_LDC_BARREL_0P3_1           "res/ldc/pef/1920x1080_barrel_0.3_r50_ofst_0_0_d-200.yuv"
#define GDC_FILE_PEF_LDC_ROT_BARREL_0P3_2       "res/ldc/pef/1920x1080_barrel_rot_0.3_r100_ofst_0_0_d-200.yuv"
#define GDC_FILE_IN_LDC_PINCUSHION_0P3          "res/ldc/input/1920x1080_pincushion_0.3.yuv"
#define GDC_FILE_OUT_LDC_PINCUSHION_0P3_0       "res/ldc/output/1920x1080_pincushion_0.3_r0_ofst_0_0_d400.yuv"
#define GDC_FILE_OUT_LDC_PINCUSHION_0P3_1       "res/ldc/output/1920x1080_pincushion_0.3_r50_ofst_0_0_d400.yuv"
#define GDC_FILE_PEF_LDC_PINCUSHION_0P3_0       "res/ldc/pef/1920x1080_pincushion_0.3_r0_ofst_0_0_d400.yuv"
#define GDC_FILE_PEF_LDC_PINCUSHION_0P3_1       "res/ldc/pef/1920x1080_pincushion_0.3_r50_ofst_0_0_d400.yuv"

#define GDC_FILE_IN_LDC_BARREL_0P3_MESH_0       "res/ldc/input/1920x1080_barrel_0.3_r0_ofst_0_0_d-200.mesh"
#define GDC_FILE_IN_LDC_BARREL_0P3_MESH_1       "res/ldc/input/1920x1080_barrel_0.3_r50_ofst_0_0_d-200.mesh"
#define GDC_FILE_IN_LDC_PINCUSHION_0P3_MESH_0       "res/ldc/input/1920x1080_pincushion_0.3_r0_ofst_0_0_d400.mesh"
#define GDC_FILE_IN_LDC_PINCUSHION_0P3_MESH_1       "res/ldc/input/1920x1080_pincushion_0.3_r50_ofst_0_0_d400.mesh"

#define GDC_MULTI_THREADS_LOAD_MESH_BOOL	0

#define GDC_FILE_IN_FMT_0                      "res/ldc/input/1920x1080.yuv"
#define GDC_FILE_OUT_FMT_0                     "res/ldc/output/1920x1080.yuv"
#define GDC_FILE_PEF_FMT_0                     "res/ldc/pef/1920x1080.yuv"
#define GDC_FILE_IN_FMT_1                      "res/ldc/input/1920x1080_yonly.yuv"
#define GDC_FILE_OUT_FMT_1                     "res/ldc/output/1920x1080_yonly.yuv"
#define GDC_FILE_PEF_FMT_1                     "res/ldc/pef/1920x1080_yonly.yuv"

#define GDC_FILE_IN_NOT_ALIGN                  "res/ldc/input/666x666_yuv400.yuv"
#define GDC_FILE_OUT_NOT_ALIGN                 "res/ldc/output/666x666_yuv400.yuv"

#define GDC_FILE_IN_CMDQ                       "res/ldc/input/1920x1080.yuv"
#define GDC_FILE_OUT_CMDQ                      "res/ldc/output/1920x1080_cmdq.yuv"
#define GDC_FILE_PEF_CMDQ                      "res/ldc/pef/1920x1080_cmdq.yuv"
#define GDC_FILE_IN_CMDQ_1TO2                  "res/ldc/input/1920x1080.yuv"
#define GDC_FILE_OUT_CMDQ_1TO2_0               "res/ldc/output/1920x1080_cmdq_1to2_0.yuv"
#define GDC_FILE_PEF_CMDQ_1TO2_0               "res/ldc/pef/1920x1080_cmdq_1to2_0.yuv"
#define GDC_FILE_IN_CMDQ_1TO2_MAX              "res/ldc/input/4608x4608.yuv"
#define GDC_FILE_OUT_CMDQ_1TO2_0_MAX           "res/ldc/output/4608x4608_cmdq_1to2_0.yuv"
#define GDC_FILE_PEF_CMDQ_1TO2_0_MAX           "res/ldc/pef/4608x4608_cmdq_1to2_0.yuv"

#define GDC_FILE_IN_LDC_GRID_INFO              "res/ldc/input/1280x768.yuv"
#define GDC_FILE_OUT_LDC_GRID_INFO             "res/ldc/output/1280x768_grid_info.yuv"
#define GDC_FILE_PEF_LDC_GRID_INFO             "res/ldc/pef/1280x768_grid_info.yuv"
#define GDC_FILE_IN_LDC_GRID                   "res/ldc/input/grid_info_79_44_3476_80_45_1280x720.dat"

#define GDC_FILE_IN_LDC_GRID_INFO0              "res/ldc/input/grid/nv21/left_00.yuv"
#define GDC_FILE_IN_LDC_GRID_INFO1              "res/ldc/input/grid/nv21/left_01.yuv"
#define GDC_FILE_IN_LDC_GRID_INFO2              "res/ldc/input/grid/nv21/left_02.yuv"
#define GDC_FILE_IN_LDC_GRID_INFO3              "res/ldc/input/grid/nv21/left_03.yuv"
#define GDC_FILE_IN_LDC_GRID_INFO4              "res/ldc/input/grid/nv21/left_04.yuv"
#define GDC_FILE_IN_LDC_GRID_INFO5              "res/ldc/input/grid/nv21/left_05.yuv"
#define GDC_FILE_IN_LDC_GRID_INFO6              "res/ldc/input/grid/nv21/left_06.yuv"
#define GDC_FILE_IN_LDC_GRID_INFO7              "res/ldc/input/grid/nv21/left_07.yuv"
#define GDC_FILE_IN_LDC_GRID_INFO8              "res/ldc/input/grid/nv21/left_08.yuv"
#define GDC_FILE_IN_LDC_GRID_INFO9              "res/ldc/input/grid/nv21/left_09.yuv"
#define GDC_FILE_OUT_LDC_GRID_INFO0              "res/ldc/output/grid/nv21/left_00.yuv"
#define GDC_FILE_OUT_LDC_GRID_INFO1              "res/ldc/output/grid/nv21/left_01.yuv"
#define GDC_FILE_OUT_LDC_GRID_INFO2              "res/ldc/output/grid/nv21/left_02.yuv"
#define GDC_FILE_OUT_LDC_GRID_INFO3              "res/ldc/output/grid/nv21/left_03.yuv"
#define GDC_FILE_OUT_LDC_GRID_INFO4              "res/ldc/output/grid/nv21/left_04.yuv"
#define GDC_FILE_OUT_LDC_GRID_INFO5              "res/ldc/output/grid/nv21/left_05.yuv"
#define GDC_FILE_OUT_LDC_GRID_INFO6              "res/ldc/output/grid/nv21/left_06.yuv"
#define GDC_FILE_OUT_LDC_GRID_INFO7              "res/ldc/output/grid/nv21/left_07.yuv"
#define GDC_FILE_OUT_LDC_GRID_INFO8              "res/ldc/output/grid/nv21/left_08.yuv"
#define GDC_FILE_OUT_LDC_GRID_INFO9              "res/ldc/output/grid/nv21/left_09.yuv"
#define GDC_FILE_IN_LDC_GRID_0_9                 "res/ldc/input/grid/grid_info_79_44_3476_80_45_1280x720.dat"

#define GDC_MAX_W    4608
#define GDC_MAX_H    4608
#define GDC_MIN_W    64
#define GDC_MIN_H    64

#ifndef FPGA_PORTING
#define GDC_REPEAT_TIMES 6
#else
#define GDC_REPEAT_TIMES 2
#endif

typedef CVI_S32 (*p_func)(void);

#define GDC_DEFAULT_PIXEL_FMT PIXEL_FORMAT_NV21
#define MAX_FUNC_CNT 100

static CVI_BOOL bEnProc;
static CVI_BOOL g_gdc_save_file;

typedef enum _GDC_TEST_OP {
	GDC_TEST_ROT = 0,
	GDC_TEST_ROT1,
	GDC_TEST_ROT2,
	GDC_TEST_LDC,
	GDC_TEST_LDC_LOAD_MESH,
	GDC_TEST_MAX_SIZE,
	GDC_TEST_FMT,
	GDC_TEST_SIZE_NO_ALIGN,
	GDC_TEST_CMDQ,
	GDC_TEST_CMDQ_1TO2,
	GDC_TEST_CMDQ_1TO2_MAX,
	GDC_TEST_ONLINE,
	GDC_TEST_ASYNC,
	GDC_TEST_MIX,
	GDC_TEST_MIX_FASYNC,
	GDC_TEST_MULTI_THREAD,
	GDC_TEST_PEF,
	GDC_TEST_LOAD_GRID_INFO,
	GDC_TEST_LOAD_GRID_INFO2,
	GDC_TEST_RST,
	GDC_TEST_PRESURE_SIZE_FOR_EACH = 98,
	GDC_TEST_AUTO_REGRESSION = 99,
	GDC_TEST_USER_CONFIG = 100,
	GDC_TEST_DUP_FD = 101,
	GDC_TEST_RST_FD = 102,
} GDC_TEST_OP;

typedef struct _GDC_BASIC_TEST_PARAM {
	SIZE_S size_in;
	SIZE_S size_out;
	char filename_in[128];
	char filename_out[128];
	char filename_pef[128];
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
	CVI_BOOL needPef;
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
	char *meshName;
	CVI_U64 u64PhyAddr;
	CVI_VOID *pVirAddr;

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

		s32Ret = CVI_GDC_GenLDCMesh(param->size_in.u32Width, param->size_in.u32Height, LDCAttr
			, param->stTask.name, &u64PhyAddr, &pVirAddr);
		if (s32Ret) {
			GDC_UT_PRT("gen LDC mesh(%s) fail\n", param->stTask.name);
			break;
		}

		param->stTask.au64privateData[0] = u64PhyAddr;
		s32Ret = CVI_GDC_AddLDCTask(param->hHandle, &param->stTask, LDCAttr, param->enRotation);
		if (s32Ret) {
			GDC_UT_PRT("CVI_GDC_AddLDCTask failed!\n");
		}
		break;
	case GDC_TEST_LDC_LOAD_MESH:
		meshName =(char *)ptr;
		LDC_ATTR_S stLDCAttr = {0};

		s32Ret = CVI_GDC_LoadLDCMesh(param->size_out.u32Width, param->size_out.u32Height
			, meshName, param->stTask.name, &u64PhyAddr, &pVirAddr);
		if (s32Ret) {
			GDC_UT_PRT("gen LDC mesh(%s) fail for tsk(%s)\n", meshName, param->stTask.name);
			break;
		}

		param->stTask.au64privateData[0] = u64PhyAddr;
		s32Ret = CVI_GDC_AddLDCTask(param->hHandle, &param->stTask, &stLDCAttr, ROTATION_0);
		if (s32Ret) {
			GDC_UT_PRT("CVI_GDC_AddLDCTask failed!\n");
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
	VB_CONFIG_S stVbConf;
	CVI_S32 s32Ret;
	CVI_U32 BlkSize;

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
	stVbConf.astCommPool[0].u32BlkCnt	= 3;
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

	do {
		param->hHandle = 0;
		memset(&param->stVideoFrameIn, 0, sizeof(param->stVideoFrameIn));
		s32Ret = GDCFileToFrame(&param->size_in, param->enPixelFormat, param->filename_in, &param->stVideoFrameIn);
		if (s32Ret) {
			GDC_UT_PRT("GDCFileToFrame failed!\n");
			goto exit2;
		}

		memset(&param->stVideoFrameOut, 0, sizeof(param->stVideoFrameOut));
		s32Ret = GDC_COMM_PrepareFrame(&param->size_out, param->enPixelFormat, &param->stVideoFrameOut);
		if (s32Ret) {
			GDC_UT_PRT("GDC_COMM_PrepareFrame failed!\n");
			goto exit2;
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
			GDC_UT_PRT("pef file:%s\n", param->filename_pef);

			if (param->needPef) {
				s32Ret = GDCCompareWithFile(param->filename_pef, &param->stVideoFrameOut);
				if (s32Ret != CVI_SUCCESS) {
					GDC_UT_PRT("GDCCompareWithFile fail.\n");
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
	CVI_GDC_FreeCurTaskMesh(param->stTask.name);
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
	char *filename_pef[3] = {GDC_FILE_PEF_ROT0, GDC_FILE_PEF_ROT90, GDC_FILE_PEF_ROT270};
	param.needPef = CVI_TRUE;

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		strcpy(param.filename_pef, filename_pef[i]);
		param.size_in.u32Width = 1920;
		param.size_in.u32Height = 1080;
		if (i) {
			param.size_out.u32Width = 1088;
			param.size_out.u32Height = 1920;
		} else {
			param.size_out.u32Width = 1920;
			param.size_out.u32Height = 1088;
		}
		param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
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

static CVI_S32 gdc_test_rot_4m(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ROTATION_E rot[3] = {ROTATION_0, ROTATION_90, ROTATION_270};
	GDC_BASIC_TEST_PARAM param = {0};
	int cnt = 3;
	char *filename_in[3] = {GDC_FILE_IN_ROT_4M, GDC_FILE_IN_ROT_4M, GDC_FILE_IN_ROT_4M};
	char *filename_out[3] = {GDC_FILE_OUT_ROT0_4M, GDC_FILE_OUT_ROT90_4M, GDC_FILE_OUT_ROT270_4M};
	char *filename_pef[3] = {GDC_FILE_PEF_ROT0_4M, GDC_FILE_PEF_ROT90_4M, GDC_FILE_PEF_ROT270_4M};
	param.needPef = CVI_TRUE;

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		strcpy(param.filename_pef, filename_pef[i]);
		param.size_in.u32Width = 2560;
		param.size_in.u32Height = 1440;
		if (i) {
			param.size_out.u32Width = 1472;
			param.size_out.u32Height = 2560;
		} else {
			param.size_out.u32Width = 2560;
			param.size_out.u32Height = 1472;
		}
		param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_rot_4m_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_rot_4m_%d", i);
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

static CVI_S32 gdc_test_rot_small(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ROTATION_E rot[3] = {ROTATION_0, ROTATION_90, ROTATION_270};
	GDC_BASIC_TEST_PARAM param = {0};
	int cnt = 3;
	char *filename_in[3] = {GDC_FILE_IN_ROT_1, GDC_FILE_IN_ROT_1, GDC_FILE_IN_ROT_1};
	char *filename_out[3] = {GDC_FILE_OUT_ROT0_1, GDC_FILE_OUT_ROT90_1, GDC_FILE_OUT_ROT270_1};
	char *filename_pef[3] = {GDC_FILE_PEF_ROT0_1, GDC_FILE_PEF_ROT90_1, GDC_FILE_PEF_ROT270_1};
	param.needPef = CVI_TRUE;

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		strcpy(param.filename_pef, filename_pef[i]);
		param.size_in.u32Width = 128;
		param.size_in.u32Height = 128;
		param.size_out.u32Width = 128;
		param.size_out.u32Height = 128;

		param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_rot_1_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_rot_1_%d", i);
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
	LDC_ATTR_S stLdcAttr[5] = {
		{CVI_TRUE, 0, 0, 0, 0, 0, -200, {0}, 0, 0},
		{CVI_TRUE, 0, 0, 50, 0, 0, -200, {0}, 0, 0},
		{CVI_TRUE, 0, 0, 100, 0, 0, -200, {0}, 0, 1},
		{CVI_TRUE, 0, 0, 0, 0, 0, 400, {0}, 0, 0},
		{CVI_TRUE, 0, 0, 50, 0, 0, 400, {0}, 0, 0},
	};
	GDC_BASIC_TEST_PARAM param = {0};
	int cnt = 5;
	char *filename_in[5] = {
		GDC_FILE_IN_LDC_BARREL_0P3,
		GDC_FILE_IN_LDC_BARREL_0P3,
		GDC_FILE_IN_LDC_BARREL_0P3,
		GDC_FILE_IN_LDC_PINCUSHION_0P3,
		GDC_FILE_IN_LDC_PINCUSHION_0P3,
	};
	char *filename_out[5] = {
		GDC_FILE_OUT_LDC_BARREL_0P3_0,
		GDC_FILE_OUT_LDC_BARREL_0P3_1,
		GDC_FILE_OUT_LDC_ROT_BARREL_0P3_2,
		GDC_FILE_OUT_LDC_PINCUSHION_0P3_0,
		GDC_FILE_OUT_LDC_PINCUSHION_0P3_1,
	};
	char *filename_pef[5] = {
		GDC_FILE_PEF_LDC_BARREL_0P3_0,
		GDC_FILE_PEF_LDC_BARREL_0P3_1,
		GDC_FILE_PEF_LDC_ROT_BARREL_0P3_2,
		GDC_FILE_PEF_LDC_PINCUSHION_0P3_0,
		GDC_FILE_PEF_LDC_PINCUSHION_0P3_1,
	};
	param.needPef = CVI_TRUE;

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		strcpy(param.filename_pef, filename_pef[i]);

		param.size_in.u32Width = 1920;
		param.size_in.u32Height = 1080;
		if (i == 2) {
			param.size_out.u32Width = ALIGN(1080, DEFAULT_ALIGN);
			param.size_out.u32Height = 1920;
		} else {
			param.size_out.u32Width = 1920;
			param.size_out.u32Height = ALIGN(1080, DEFAULT_ALIGN);
		}

		param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_gdc_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_gdc_%d", i);
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

static CVI_S32 gdc_test_ldc_load_mesh(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	char *mesh_file_name[4] = {
		GDC_FILE_IN_LDC_BARREL_0P3_MESH_0,
		GDC_FILE_IN_LDC_BARREL_0P3_MESH_1,
		GDC_FILE_IN_LDC_PINCUSHION_0P3_MESH_0,
		GDC_FILE_IN_LDC_PINCUSHION_0P3_MESH_1,
	};
	GDC_BASIC_TEST_PARAM param = {0};
	int cnt = 4;
	char *filename_in[4] = {
		GDC_FILE_IN_LDC_BARREL_0P3,
		GDC_FILE_IN_LDC_BARREL_0P3,
		GDC_FILE_IN_LDC_PINCUSHION_0P3,
		GDC_FILE_IN_LDC_PINCUSHION_0P3,
	};
	char *filename_out[4] = {
		GDC_FILE_OUT_LDC_BARREL_0P3_0,
		GDC_FILE_OUT_LDC_BARREL_0P3_1,
		GDC_FILE_OUT_LDC_PINCUSHION_0P3_0,
		GDC_FILE_OUT_LDC_PINCUSHION_0P3_1,
	};
	char *filename_pef[4] = {
		GDC_FILE_PEF_LDC_BARREL_0P3_0,
		GDC_FILE_PEF_LDC_BARREL_0P3_1,
		GDC_FILE_PEF_LDC_PINCUSHION_0P3_0,
		GDC_FILE_PEF_LDC_PINCUSHION_0P3_1,
	};
	param.needPef = CVI_TRUE;

	for (CVI_U8 i = 1; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		strcpy(param.filename_pef, filename_pef[i]);

		param.size_in.u32Width = 1920;
		param.size_in.u32Height = 1080;
		param.size_out.u32Width = 1920;
		param.size_out.u32Height = ALIGN(1080, DEFAULT_ALIGN);
		param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_gdc_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_gdc_%d", i);
		param.identity.syncIo = CVI_TRUE;

		param.op = GDC_TEST_LDC_LOAD_MESH;

		s32Ret = gdc_basic(&param, (void *)mesh_file_name[i]);
		if (s32Ret != CVI_SUCCESS) {
			GDC_UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	GDC_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_rot_maxsize(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	ROTATION_E rot[4] = {ROTATION_0, ROTATION_90, ROTATION_270, ROTATION_XY_FLIP};
	GDC_BASIC_TEST_PARAM param = {0};
	int cnt = 4;
	char *filename_in[4] = {GDC_FILE_IN_ROT_2, GDC_FILE_IN_ROT_2, GDC_FILE_IN_ROT_2, GDC_FILE_IN_ROT_2};
	char *filename_out[4] = {GDC_FILE_OUT_ROT0_2, GDC_FILE_OUT_ROT90_2, GDC_FILE_OUT_ROT270_2, GDC_FILE_OUT_ROT_XYFLIP_2};
	char *filename_pef[4] = {GDC_FILE_PEF_ROT0_2, GDC_FILE_PEF_ROT90_2, GDC_FILE_PEF_ROT270_2, GDC_FILE_PEF_ROT_XYFLIP_2};
	param.needPef = CVI_TRUE;

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		strcpy(param.filename_pef, filename_pef[i]);
		param.size_in.u32Width = GDC_MAX_W;
		param.size_in.u32Height = GDC_MAX_H;
		param.size_out.u32Width = GDC_MAX_W;
		param.size_out.u32Height = GDC_MAX_H;

		param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_rot_2_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_rot_2_%d", i);
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

static CVI_S32 gdc_test_fmt(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	GDC_BASIC_TEST_PARAM param = {0};
	int cnt = 2;
	char *filename_in[2] = {GDC_FILE_IN_FMT_0, GDC_FILE_IN_FMT_1};
	char *filename_out[2] = {GDC_FILE_OUT_FMT_0, GDC_FILE_OUT_FMT_1};
	char *filename_pef[2] = {GDC_FILE_PEF_FMT_0, GDC_FILE_PEF_FMT_1};
	PIXEL_FORMAT_E enPixelFormat[2] = {
		GDC_DEFAULT_PIXEL_FMT,
		PIXEL_FORMAT_YUV_400,
	};
	param.needPef = CVI_TRUE;

	for (CVI_U8 i = 0; i < cnt; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);
		strcpy(param.filename_pef, filename_pef[i]);
		param.size_in.u32Width = 1920;
		param.size_in.u32Height = 1080;
		param.size_out.u32Width = 1920;
		param.size_out.u32Height = 1088;

		param.enPixelFormat = enPixelFormat[i];
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_fmt_%d", i);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = i;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_fmt_%d", i);
		param.identity.syncIo = CVI_TRUE;

		param.op = GDC_TEST_ROT;

		s32Ret = gdc_basic(&param, (void *)ROTATION_0);
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
	GDC_BASIC_TEST_PARAM param = {0};
	char *filename_in = GDC_FILE_IN_NOT_ALIGN;
	char *filename_out = GDC_FILE_OUT_NOT_ALIGN;
	VB_CONFIG_S stVbConf;
	CVI_U32 BlkSize;

	strcpy(param.filename_in, filename_in);
	strcpy(param.filename_out, filename_out);
	param.size_in.u32Width = 666;
	param.size_in.u32Height = 666;
	param.size_out.u32Width = 666;
	param.size_out.u32Height = 666;

	param.enPixelFormat = PIXEL_FORMAT_YUV_400;
	snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_not_align");
	param.identity.enModId = CVI_ID_USER;
	param.identity.u32ID = 0;
	snprintf(param.identity.Name, sizeof(param.identity.Name), "job_not_align");
	param.identity.syncIo = CVI_TRUE;
	param.op = GDC_TEST_ROT;
	param.needPef = CVI_FALSE;

	param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width, param.size_in.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);
	param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width, param.size_out.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);

	BlkSize = (param.u32BlkSizeIn > param.u32BlkSizeOut) ? param.u32BlkSizeIn : param.u32BlkSizeOut;

	stVbConf.u32MaxPoolCnt				= 1;
	stVbConf.astCommPool[0].u32BlkSize	= BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 3;
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

	s32Ret = CVI_GDC_Init();
	if (s32Ret != CVI_SUCCESS) {
		GDC_UT_PRT("CVI_GDC_Init failed!\n");
		goto exit1;
	}

	param.hHandle = 0;
	memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
	s32Ret = GDCFileToFrame2(&param.size_in, param.enPixelFormat, param.filename_in, &param.stVideoFrameIn);
	if (s32Ret) {
		GDC_UT_PRT("GDCFileToFrame failed!\n");
		goto exit2;
	}

	memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
	s32Ret = GDC_COMM_PrepareFrame2(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut);
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

exit2:
	if (s32Ret)
		if (param.hHandle)
			s32Ret |= CVI_GDC_CancelJob(param.hHandle);
	CVI_GDC_FreeCurTaskMesh(param.stTask.name);
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

	GDC_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_reset(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	GDC_BASIC_TEST_PARAM param = {0};
	VB_CONFIG_S stVbConf;
	CVI_U32 BlkSize;
	int times = 4, err_times = 4;

	do {
		for (CVI_U8 cnt = 0; cnt < times; cnt ++) {
			param.op = GDC_TEST_ROT;
			param.size_in.u32Width =   1920;
			param.size_in.u32Height =  1080;
			param.size_out.u32Width =  1920;
			param.size_out.u32Height = 1080;
			param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
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
			s32Ret = GDC_COMM_PrepareFrame(&param.size_in, param.enPixelFormat, &param.stVideoFrameIn);
			if (s32Ret) {
				GDC_UT_PRT("GDC_COMM_PrepareFrame in failed!\n");
				goto exit2;
			}

			memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
			s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut);
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
			CVI_GDC_FreeCurTaskMesh(param.stTask.name);
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
	VB_CONFIG_S stVbConf;
	char *filename_in[1] = {GDC_FILE_IN_CMDQ};
	char *filename_out[1] = {GDC_FILE_OUT_CMDQ};
	char *filename_pef[1] = {GDC_FILE_PEF_CMDQ};
	GDC_TASK_ATTR_S stTask_1st;
	GDC_TASK_ATTR_S stTask_2nd;
	SIZE_S size_1st_out, size_2nd_out;
	int times = GDC_REPEAT_TIMES;
	VB_BLK Blk;
	CVI_U32 BlkSize;

	for (CVI_U8 cnt = 0; cnt < 1; cnt ++) {
		strcpy(param.filename_in, filename_in[0]);
		strcpy(param.filename_out, filename_out[0]);
		strcpy(param.filename_pef, filename_pef[0]);

		param.op = GDC_TEST_ROT;
		param.size_in.u32Width =   1920;
		param.size_in.u32Height =  1080;
		param.size_out.u32Width =  1920;
		param.size_out.u32Height = 1088;
		param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_cmdq_%d", cnt);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = 0;
		snprintf(param.identity.Name, sizeof(param.identity.Name),	"job_cmdq_%d", cnt);
		param.identity.syncIo = CVI_TRUE;
		param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width, param.size_in.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);
		param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width, param.size_out.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);
		param.needPef = CVI_TRUE;

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
			s32Ret = GDCFileToFrame(&param.size_in, param.enPixelFormat, param.filename_in, &param.stVideoFrameIn);
			if (s32Ret) {
				GDC_UT_PRT("GDCFileToFrame failed!\n");
				goto exit2;
			}

			memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
			s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut);
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

			s32Ret = GDC_COMM_PrepareFrame(&size_1st_out, param.enPixelFormat, &stTask_1st.stImgOut);
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
				GDC_UT_PRT("pef file:%s\n", param.filename_pef);

				if (param.needPef) {
					s32Ret = GDCCompareWithFile(param.filename_pef, &stTask_2nd.stImgOut);
					if (s32Ret != CVI_SUCCESS) {
						GDC_UT_PRT("GDCCompareWithFile fail.\n");
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
				GDC_UT_PRT("release VB fail.\n");
				goto exit2;
			}
		} while (times--);

	exit2:
		if (s32Ret)
			if (param.hHandle)
				s32Ret |= CVI_GDC_CancelJob(param.hHandle);
		CVI_GDC_FreeCurTaskMesh(stTask_1st.name);
		CVI_GDC_FreeCurTaskMesh(stTask_2nd.name);
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
	VB_CONFIG_S stVbConf;
	char *filename_in[1] = {GDC_FILE_IN_CMDQ_1TO2};
	char *filename_out[1] = {GDC_FILE_OUT_CMDQ_1TO2_0};
	char *filename_pef[1] = {GDC_FILE_PEF_CMDQ_1TO2_0};
	GDC_TASK_ATTR_S stTask_tmp;
	VIDEO_FRAME_INFO_S stVideoFrameOut_tmp;
	int times = GDC_REPEAT_TIMES;
	VB_BLK Blk;
	CVI_U32 BlkSize;
	CVI_CHAR name[32];

	for (CVI_U8 cnt = 0; cnt < 2; cnt ++) {
		strcpy(param.filename_in, filename_in[0]);
		strcpy(param.filename_out, filename_out[0]);
		strcpy(param.filename_pef, filename_pef[0]);

		param.op = GDC_TEST_ROT;
		param.size_in.u32Width =   1920;
		param.size_in.u32Height =  1080;
		param.size_out.u32Width =  1920;
		param.size_out.u32Height = 1088;
		param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_cmdq_%d", cnt);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = 0;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_cmdq_%d", cnt);
		param.identity.syncIo = CVI_TRUE;
		param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width, param.size_in.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);
		param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width, param.size_out.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);
		param.needPef = CVI_TRUE;

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
			s32Ret = GDCFileToFrame(&param.size_in, param.enPixelFormat, param.filename_in, &param.stVideoFrameIn);
			if (s32Ret) {
				GDC_UT_PRT("GDCFileToFrame failed!\n");
				goto exit2;
			}

			memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
			s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut);
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
			s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &stVideoFrameOut_tmp);
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
				GDC_UT_PRT("pef file:%s\n", param.filename_pef);

				if (param.needPef) {
					s32Ret = GDCCompareWithFile(param.filename_out, &stTask_tmp.stImgOut);
					if (s32Ret != CVI_SUCCESS) {
						GDC_UT_PRT("GDCCompareWithFile fail.\n");
						goto exit2;
					}

					s32Ret = GDCCompareWithFile(param.filename_pef, &param.stVideoFrameOut);
					if (s32Ret != CVI_SUCCESS) {
						GDC_UT_PRT("GDCCompareWithFile fail.\n");
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
				GDC_UT_PRT("release VB fail.\n");
				goto exit2;
			}
		} while (times--);

	exit2:
		if (s32Ret)
			if (param.hHandle)
				s32Ret |= CVI_GDC_CancelJob(param.hHandle);
		CVI_GDC_FreeCurTaskMesh(param.stTask.name);
		CVI_GDC_FreeCurTaskMesh(stTask_tmp.name);
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

static CVI_S32 gdc_test_cmdq_1to2_maxsize(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	GDC_BASIC_TEST_PARAM param = {0};
	VB_CONFIG_S stVbConf;
	char *filename_in[1] = {GDC_FILE_IN_CMDQ_1TO2_MAX};
	char *filename_out[1] = {GDC_FILE_OUT_CMDQ_1TO2_0_MAX};
	char *filename_pef[1] = {GDC_FILE_PEF_CMDQ_1TO2_0_MAX};
	GDC_TASK_ATTR_S stTask_tmp;
	VIDEO_FRAME_INFO_S stVideoFrameOut_tmp;
	int times = GDC_REPEAT_TIMES;
	VB_BLK Blk;
	CVI_U32 BlkSize;
	CVI_CHAR name[32];

	for (CVI_U8 cnt = 0; cnt < 1; cnt ++) {
		strcpy(param.filename_in, filename_in[0]);
		strcpy(param.filename_out, filename_out[0]);
		strcpy(param.filename_pef, filename_pef[0]);

		param.op = GDC_TEST_ROT;
		param.size_in.u32Width =   GDC_MAX_W;
		param.size_in.u32Height =  GDC_MAX_H;
		param.size_out.u32Width =  GDC_MAX_W;
		param.size_out.u32Height = GDC_MAX_H;
		param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_cmdq_%d", cnt);
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = 0;
		snprintf(param.identity.Name, sizeof(param.identity.Name),	"job_cmdq_%d", cnt);
		param.identity.syncIo = CVI_TRUE;
		param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width, param.size_in.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);
		param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width, param.size_out.u32Height, PIXEL_FORMAT_YUV_PLANAR_444
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);
		param.needPef = CVI_TRUE;

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
			s32Ret = GDCFileToFrame(&param.size_in, param.enPixelFormat, param.filename_in, &param.stVideoFrameIn);
			if (s32Ret) {
				GDC_UT_PRT("GDCFileToFrame failed!\n");
				goto exit2;
			}

			memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
			s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut);
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
			s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &stVideoFrameOut_tmp);
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
				GDC_UT_PRT("pef file:%s\n", param.filename_pef);

				if (param.needPef) {
					s32Ret = GDCCompareWithFile(param.filename_out, &stTask_tmp.stImgOut);
					if (s32Ret != CVI_SUCCESS) {
						GDC_UT_PRT("GDCCompareWithFile fail.\n");
						goto exit2;
					}

					s32Ret = GDCCompareWithFile(param.filename_pef, &param.stVideoFrameOut);
					if (s32Ret != CVI_SUCCESS) {
						GDC_UT_PRT("GDCCompareWithFile fail.\n");
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
				GDC_UT_PRT("release VB fail.\n");
				goto exit2;
			}
		} while (times--);

	exit2:
		if (s32Ret)
			if (param.hHandle)
				s32Ret |= CVI_GDC_CancelJob(param.hHandle);
		CVI_GDC_FreeCurTaskMesh(param.stTask.name);
		CVI_GDC_FreeCurTaskMesh(stTask_tmp.name);
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
	VB_CONFIG_S stVbConf;
	GDC_BASIC_TEST_PARAM param = {0};
	char *filename_in[1] = {GDC_FILE_IN_ROT};
	char *filename_out[1] = {GDC_FILE_OUT_ROT0};
	char *filename_pef[1] = {GDC_FILE_PEF_ROT0};
	CVI_U8 times = GDC_REPEAT_TIMES * 10;
	CVI_U32 BlkSize;

	strcpy(param.filename_in, filename_in[0]);
	strcpy(param.filename_out, filename_out[0]);
	strcpy(param.filename_pef, filename_pef[0]);
	param.size_in.u32Width = 1920;
	param.size_in.u32Height = 1080;
	param.size_out.u32Width = 1920;
	param.size_out.u32Height = 1088;
	param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
	snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_rot_async");
	param.identity.enModId = CVI_ID_USER;
	param.identity.u32ID = 0;
	snprintf(param.identity.Name, sizeof(param.identity.Name), "job_rot_async");
	param.identity.syncIo = CVI_FALSE;
	param.op = GDC_TEST_ROT;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width, param.size_in.u32Height, param.enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);
	param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width, param.size_out.u32Height, param.enPixelFormat
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

	do {
		param.hHandle = 0;
		memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
		s32Ret = GDCFileToFrame(&param.size_in, param.enPixelFormat, param.filename_in, &param.stVideoFrameIn);
		if (s32Ret) {
			GDC_UT_PRT("GDCFileToFrame failed!\n");
			goto exit2;
		}

		memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
		s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut);
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

		s32Ret = CVI_GDC_AddRotationTask(param.hHandle, &param.stTask, ROTATION_0);
		if (s32Ret) {
			GDC_UT_PRT("CVI_GDC_AddRotationTask 1st failed!\n");
			goto exit2;
		}

		s32Ret = CVI_GDC_EndJob(param.hHandle);
		if (s32Ret) {
			GDC_UT_PRT("CVI_GDC_EndJob failed!\n");
			goto exit2;
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
	} while (times--);

exit2:
	if (s32Ret)
		if (param.hHandle)
			s32Ret |= CVI_GDC_CancelJob(param.hHandle);
	CVI_GDC_FreeCurTaskMesh(param.stTask.name);
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

static CVI_S32 gdc_test_online(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

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
			usleep(1000* 5);
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

static CVI_S32 gdc_test_mix(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	VB_CONFIG_S stVbConf;
	GDC_BASIC_TEST_PARAM param = {0};
	char *filename_in[2] = {GDC_FILE_IN_ROT, GDC_FILE_IN_LDC_BARREL_0P3};
	char *filename_out[2] = {GDC_FILE_OUT_ROT0, GDC_FILE_OUT_LDC_BARREL_0P3_0};
	char *filename_pef[2] = {GDC_FILE_PEF_ROT0, GDC_FILE_PEF_LDC_BARREL_0P3_0};
	GDC_TEST_OP op[2] = {GDC_TEST_ROT, GDC_TEST_LDC_LOAD_MESH};
	CVI_U32 WidthIn[2] = {1920, 1920};
	CVI_U32 HeightIn[2] = {1080, 1080};
	CVI_U32 WidthOut[2] = {1920, 1920};
	CVI_U32 HeightOut[2] = {1088, 1088};
	int rc;
	pthread_t tid;
	CVI_U8 times = GDC_REPEAT_TIMES * 10;
	void *ptr;
	char *mesh_file_name = GDC_FILE_IN_LDC_BARREL_0P3_MESH_0;
	CVI_U32 BlkSize;

	param.size_in.u32Width = 1920;
	param.size_in.u32Height = 1080;
	param.size_out.u32Width = 1920;
	param.size_out.u32Height = 1088;
	param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;

	//snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_mix");
	param.identity.enModId = CVI_ID_USER;
	param.identity.u32ID = 0;
	snprintf(param.identity.Name, sizeof(param.identity.Name), "job_mix");
	param.identity.syncIo = CVI_FALSE;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width, param.size_in.u32Height, param.enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);
	param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width, param.size_out.u32Height, param.enPixelFormat
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

	LdcAsyncDoneFlag = CVI_FALSE;
	struct sched_param t_param;
	pthread_attr_t attr;

	t_param.sched_priority = 99;

	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &t_param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	rc = pthread_create(&tid, NULL, test_gdc_async_thread, &param);
	if (rc < 0) {
		GDC_UT_PRT("rc is %d, threads create fail\n", rc);
		perror("Fail:");
		return CVI_FAILURE;
	}

	do {
		for (CVI_U8 i = 0; i < 2; i++) {
			strcpy(param.filename_in, filename_in[i]);
			strcpy(param.filename_out, filename_out[i]);
			strcpy(param.filename_pef, filename_pef[i]);
			param.size_in.u32Width = WidthIn[i];
			param.size_in.u32Height = HeightIn[i];
			param.size_out.u32Width = WidthOut[i];
			param.size_out.u32Height = HeightOut[i];
			param.op = op[i];

			param.hHandle = 0;
			snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_mix_%d", i);

			memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
			s32Ret = GDCFileToFrame(&param.size_in, param.enPixelFormat, param.filename_in, &param.stVideoFrameIn);
			if (s32Ret) {
				GDC_UT_PRT("GDCFileToFrame failed!\n");
				goto exit2;
			}

			memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
			s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut);
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
				ptr = (void *)mesh_file_name;
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
			usleep(500*1000);
		}
	}while (times--);
exit2:
	LdcAsyncDoneFlag = CVI_TRUE;
	pthread_join(tid, NULL);

	if (s32Ret && param.hHandle) {
		s32Ret |= CVI_GDC_CancelJob(param.hHandle);
		if (s32Ret) {
			GDC_UT_PRT("CVI_GDC_CancelJob fail.\n");
		}
	}
	CVI_GDC_FreeCurTaskMesh(param.stTask.name);

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
	if (s32Ret) {
		GDC_UT_PRT("release VB fail.\n");
	}
exit1:
	s32Ret |= CVI_SYS_Exit();
exit0:
	s32Ret |= CVI_VB_Exit();

	GDC_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static GDC_BASIC_TEST_PARAM param_mix_ldc[2] = {0};

static void gdc_sig_handle(int sig)
{
	(void)sig;
	CVI_S32 s32Ret;
	CVI_S32 s32MilliSec = 10;
	int i;

	for (i = 0; i < 4; i++) {
		s32Ret = CVI_GDC_GetChnFrame(&param_mix_ldc[i].identity, &param_mix_ldc[i].stVideoFrameOut, s32MilliSec);
		if (s32Ret) {
			GDC_UT_PRT("CVI_GDC_GetChnFrame fail, cur_case[%d]\n", i);
		} else
			break;
	}

	if (i >= 4) {
		GDC_UT_PRT("[%s] fail\n", __FUNCTION__);
		return;
	}
	GDC_UT_PRT("[%s]\n", param_mix_ldc[i].filename_in);
	GDC_UT_PRT("[%s]\n", param_mix_ldc[i].filename_out);
	GDC_UT_PRT("op[%d]\n", param_mix_ldc[i].op);

	GDC_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param_mix_ldc[i].stVideoFrameIn.stVFrame.u64PhyAddr[0]
		, param_mix_ldc[i].stVideoFrameIn.stVFrame.u64PhyAddr[1], param_mix_ldc[i].stVideoFrameIn.stVFrame.u64PhyAddr[2]);
	GDC_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param_mix_ldc[i].stVideoFrameOut.stVFrame.u64PhyAddr[0]
		, param_mix_ldc[i].stVideoFrameOut.stVFrame.u64PhyAddr[1], param_mix_ldc[i].stVideoFrameOut.stVFrame.u64PhyAddr[2]);

	if (param_mix_ldc[i].stVideoFrameIn.stVFrame.u64PhyAddr[0]) {
		param_mix_ldc[i].inBlk = CVI_VB_PhysAddr2Handle(param_mix_ldc[i].stVideoFrameIn.stVFrame.u64PhyAddr[0]);
		if (param_mix_ldc[i].inBlk != VB_INVALID_HANDLE)
			s32Ret = CVI_VB_ReleaseBlock(param_mix_ldc[i].inBlk);
	}
	if (s32Ret)
		GDC_UT_PRT("release in VB fail\n");

	if (param_mix_ldc[i].stVideoFrameOut.stVFrame.u64PhyAddr[0]) {
		param_mix_ldc[i].outBlk = CVI_VB_PhysAddr2Handle(param_mix_ldc[i].stVideoFrameOut.stVFrame.u64PhyAddr[0]);
		if (param_mix_ldc[i].outBlk != VB_INVALID_HANDLE)
			s32Ret = CVI_VB_ReleaseBlock(param_mix_ldc[i].outBlk);
	}
	if (s32Ret)
		GDC_UT_PRT("release out VB fail\n");
}

static int gdc_init_fasync(void)
{
	int gdc_fd, flags;

	gdc_fd = CVI_GDC_GetDevFd();
	if (gdc_fd < 0) {
		GDC_UT_PRT("CVI_GDC_GetDevFd fail!\n");
		return -1;
	}

	signal(SIGIO, gdc_sig_handle);
	fcntl(gdc_fd, F_SETOWN, getpid());

	flags = fcntl(gdc_fd, F_GETFL);
	fcntl(gdc_fd, F_SETFL, flags | FASYNC);

	return 0;
}

static int gdc_deinit_fasync(void)
{
	int gdc_fd, flags;

	gdc_fd = CVI_GDC_GetDevFd();
	if (gdc_fd < 0) {
		GDC_UT_PRT("CVI_GDC_GetDevFd fail!\n");
		return -1;
	}

	signal(SIGIO, NULL);

	flags = fcntl(gdc_fd, F_GETFL);
	fcntl(gdc_fd, F_SETFL, flags & (~FASYNC));

	return 0;
}

static CVI_S32 gdc_test_mix_fasync(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	VB_CONFIG_S stVbConf;
	GDC_BASIC_TEST_PARAM param = {0};
	char *filename_in[2] = {GDC_FILE_IN_ROT, GDC_FILE_IN_LDC_BARREL_0P3};
	char *filename_out[2] = {GDC_FILE_OUT_ROT0, GDC_FILE_OUT_LDC_BARREL_0P3_0};
	char *filename_pef[2] = {GDC_FILE_PEF_ROT0, GDC_FILE_PEF_LDC_BARREL_0P3_0};
	GDC_TEST_OP op[2] = {GDC_TEST_ROT, GDC_TEST_LDC_LOAD_MESH};
	CVI_U32 WidthIn[2] = {1920, 1920};
	CVI_U32 HeightIn[2] = {1080, 1080};
	CVI_U32 WidthOut[2] = {1920, 1920};
	CVI_U32 HeightOut[2] = {1088, 1088};
	CVI_U8 times = GDC_REPEAT_TIMES * 10;
	void *ptr;
	char *mesh_file_name = GDC_FILE_IN_LDC_BARREL_0P3_MESH_0;
	CVI_U32 BlkSize;

	param.size_in.u32Width = 1920;
	param.size_in.u32Height = 1080;
	param.size_out.u32Width = 1920;
	param.size_out.u32Height = 1088;
	param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;

	//snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_mix");
	param.identity.enModId = CVI_ID_USER;
	param.identity.u32ID = 0;
	snprintf(param.identity.Name, sizeof(param.identity.Name), "job_mix");
	param.identity.syncIo = CVI_FALSE;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	param.u32BlkSizeIn = COMMON_GetPicBufferSize(param.size_in.u32Width, param.size_in.u32Height, param.enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);
	param.u32BlkSizeOut = COMMON_GetPicBufferSize(param.size_out.u32Width, param.size_out.u32Height, param.enPixelFormat
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

	if (gdc_init_fasync()) {
		GDC_UT_PRT("gdc_init_fasync failed!\n");
		goto exit2;
	}

	do {
		for (CVI_U8 i = 0; i < 2; i++) {
			strcpy(param_mix_ldc[i].filename_in, filename_in[i]);
			strcpy(param_mix_ldc[i].filename_out, filename_out[i]);
			strcpy(param_mix_ldc[i].filename_pef, filename_pef[i]);
			param_mix_ldc[i].size_in.u32Width = WidthIn[i];
			param_mix_ldc[i].size_in.u32Height = HeightIn[i];
			param_mix_ldc[i].size_out.u32Width = WidthOut[i];
			param_mix_ldc[i].size_out.u32Height = HeightOut[i];
			param_mix_ldc[i].enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
			param_mix_ldc[i].op = op[i];
			param_mix_ldc[i].identity.enModId = CVI_ID_USER;
			param_mix_ldc[i].identity.u32ID = i;
			snprintf(param_mix_ldc[i].identity.Name, sizeof(param_mix_ldc[i].identity.Name), "job_mix_%d", i);
			param_mix_ldc[i].identity.syncIo = CVI_FALSE;

			param_mix_ldc[i].hHandle = 0;
			snprintf(param_mix_ldc[i].stTask.name, sizeof(param_mix_ldc[i].stTask.name), "tsk_mix_%d", i);

			memset(&param_mix_ldc[i].stVideoFrameIn, 0, sizeof(param_mix_ldc[i].stVideoFrameIn));
			s32Ret = GDCFileToFrame(&param_mix_ldc[i].size_in, param_mix_ldc[i].enPixelFormat, param_mix_ldc[i].filename_in, &param_mix_ldc[i].stVideoFrameIn);
			if (s32Ret) {
				GDC_UT_PRT("GDCFileToFrame failed!\n");
				goto exit3;
			}

			memset(&param_mix_ldc[i].stVideoFrameOut, 0, sizeof(param_mix_ldc[i].stVideoFrameOut));
			s32Ret = GDC_COMM_PrepareFrame(&param_mix_ldc[i].size_out, param_mix_ldc[i].enPixelFormat, &param_mix_ldc[i].stVideoFrameOut);
			if (s32Ret) {
				GDC_UT_PRT("GDC_COMM_PrepareFrame failed!\n");
				goto exit3;
			}

			memset(param_mix_ldc[i].stTask.au64privateData, 0, sizeof(param_mix_ldc[i].stTask.au64privateData));
			memcpy(&param_mix_ldc[i].stTask.stImgIn, &param_mix_ldc[i].stVideoFrameIn, sizeof(param_mix_ldc[i].stVideoFrameIn));
			memcpy(&param_mix_ldc[i].stTask.stImgOut, &param_mix_ldc[i].stVideoFrameOut, sizeof(param_mix_ldc[i].stVideoFrameOut));

			s32Ret = CVI_GDC_BeginJob(&param_mix_ldc[i].hHandle);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_BeginJob failed!\n");
				goto exit3;
			}

			s32Ret = CVI_GDC_SetJobIdentity(param_mix_ldc[i].hHandle, &param_mix_ldc[i].identity);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_SetJobIdentity failed!\n");
				goto exit3;
			}

			if (i == 0) {
				ptr = (void *)ROTATION_0;
			} else {
				ptr = (void *)mesh_file_name;
			}

			s32Ret = gdc_basic_add_tsk(&param_mix_ldc[i], ptr);
			if (s32Ret != CVI_SUCCESS) {
				GDC_UT_PRT("gdc_basic_add_tsk. s32Ret: 0x%x !\n", s32Ret);
				goto exit3;
			}

			s32Ret = CVI_GDC_EndJob(param_mix_ldc[i].hHandle);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_EndJob failed!\n");
				goto exit3;
			}
			usleep(1000*10);
		}
	}while (times--);
exit3:
	usleep(1000*500);
	for (CVI_U8 i = 0; i < 2; i++) {
		if (s32Ret && param_mix_ldc[i].hHandle) {
			s32Ret |= CVI_GDC_CancelJob(param_mix_ldc[i].hHandle);
			if (s32Ret) {
				GDC_UT_PRT("CVI_GDC_CancelJob fail.\n");
			}
		}
		CVI_GDC_FreeCurTaskMesh(param_mix_ldc[i].stTask.name);

		if (param_mix_ldc[i].stVideoFrameIn.stVFrame.u64PhyAddr[0]) {
			param_mix_ldc[i].inBlk = CVI_VB_PhysAddr2Handle(param_mix_ldc[i].stVideoFrameIn.stVFrame.u64PhyAddr[0]);
			if (param_mix_ldc[i].inBlk != VB_INVALID_HANDLE)
				s32Ret |= CVI_VB_ReleaseBlock(param_mix_ldc[i].inBlk);
		}
		if (param_mix_ldc[i].stVideoFrameOut.stVFrame.u64PhyAddr[0]) {
			param_mix_ldc[i].outBlk = CVI_VB_PhysAddr2Handle(param_mix_ldc[i].stVideoFrameOut.stVFrame.u64PhyAddr[0]);
			if (param_mix_ldc[i].outBlk != VB_INVALID_HANDLE)
				s32Ret |= CVI_VB_ReleaseBlock(param_mix_ldc[i].outBlk);
		}
		if (s32Ret) {
			GDC_UT_PRT("release VB fail.\n");
		}
	}

exit2:
	gdc_deinit_fasync();

	s32Ret |= CVI_GDC_DeInit();
	if (s32Ret) {
		GDC_UT_PRT("CVI_GDC_DeInit fail.\n");
	}
exit1:
	s32Ret |= CVI_SYS_Exit();
exit0:
	s32Ret |= CVI_VB_Exit();

	GDC_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_basic_do_job(GDC_BASIC_TEST_PARAM *param, int times, void *op_ptr)
{
	CVI_S32 s32Ret;

	do {
		param->hHandle = 0;
		memset(&param->stVideoFrameIn, 0, sizeof(param->stVideoFrameIn));
		s32Ret = GDCFileToFrame(&param->size_in, param->enPixelFormat, param->filename_in, &param->stVideoFrameIn);
		if (s32Ret) {
			GDC_UT_PRT("GDCFileToFrame failed!\n");
			return CVI_FAILURE;
		}

		memset(&param->stVideoFrameOut, 0, sizeof(param->stVideoFrameOut));
		s32Ret = GDC_COMM_PrepareFrame(&param->size_out, param->enPixelFormat, &param->stVideoFrameOut);
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
			GDC_UT_PRT("CVI_GDC_EndJob failed!\n");
			return CVI_FAILURE;
		}

#if 0
		if (g_gdc_save_file) {
			s32Ret = GDCFrameSaveToFile(param->filename_out, &param->stVideoFrameOut);
			if (s32Ret != CVI_SUCCESS) {
				GDC_UT_PRT("GDCFrameSaveToFile. s32Ret: 0x%x !\n", s32Ret);
				return CVI_FAILURE;
			}
			GDC_UT_PRT("-------------------times:(%d)----------------------\n", times);
			GDC_UT_PRT("output file:%s\n", param->filename_out);
			GDC_UT_PRT("pef file:%s\n", param->filename_pef);

			if (param->needPef) {
				s32Ret = GDCCompareWithFile(param->filename_pef, &param->stVideoFrameOut);
				if (s32Ret != CVI_SUCCESS) {
					GDC_UT_PRT("GDCCompareWithFile fail.\n");
					return CVI_FAILURE;
				}
			}
		}
#endif
		param->inBlk = CVI_VB_PhysAddr2Handle(param->stVideoFrameIn.stVFrame.u64PhyAddr[0]);
		if (param->inBlk != VB_INVALID_HANDLE) {
			CVI_VB_ReleaseBlock(param->inBlk);
			param->inBlk = VB_INVALID_HANDLE;
		}
		param->outBlk = CVI_VB_PhysAddr2Handle(param->stVideoFrameOut.stVFrame.u64PhyAddr[0]);
		if (param->outBlk != VB_INVALID_HANDLE) {
			CVI_VB_ReleaseBlock(param->outBlk);
			param->outBlk = VB_INVALID_HANDLE;
		}
	} while (times--);

	return s32Ret;
}

void *gdc_basic_thread_func0(void *data)
{
	CVI_S32 s32Ret;
	GDC_BASIC_TEST_PARAM *param = (GDC_BASIC_TEST_PARAM *)(data);
	int times = GDC_REPEAT_TIMES * 10;
	void *ptr = (void *)ROTATION_0;

	s32Ret = gdc_basic_do_job(param, times, ptr);
	if (s32Ret) {
		GDC_UT_PRT("test fail.\n");
	}

	if (s32Ret)
		if (param->hHandle)
			CVI_GDC_CancelJob(param->hHandle);
	CVI_GDC_FreeCurTaskMesh(param->stTask.name);
	GDC_CHECK_RET(s32Ret);

	pthread_exit(0);
}
void *gdc_basic_thread_func1(void *data)
{
	CVI_S32 s32Ret;
	GDC_BASIC_TEST_PARAM *param = (GDC_BASIC_TEST_PARAM *)(data);
	int times = 0;
	char *mesh_file_name = GDC_FILE_IN_LDC_BARREL_0P3_MESH_0;
	void *ptr = (void *)mesh_file_name;

	s32Ret = gdc_basic_do_job(param, times, ptr);
	if (s32Ret) {
		GDC_UT_PRT("test fail.\n");
	}

	if (s32Ret)
		if (param->hHandle)
			CVI_GDC_CancelJob(param->hHandle);
	CVI_GDC_FreeCurTaskMesh(param->stTask.name);
	GDC_CHECK_RET(s32Ret);

	pthread_exit(0);
}

static CVI_S32 gdc_test_multi_thread(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	GDC_BASIC_TEST_PARAM param[8] = {0};
	CVI_U32 BlkSize;
	int i, rc, threads;

#if 0
	char *filename_in[4] = {GDC_FILE_IN_ROT, GDC_FILE_IN_ROT, GDC_FILE_IN_LDC_BARREL_0P3, GDC_FILE_IN_LDC_BARREL_0P3};
	char *filename_out[4] = {GDC_FILE_OUT_ROT0, GDC_FILE_OUT_ROT0, GDC_FILE_OUT_LDC_BARREL_0P3_0, GDC_FILE_OUT_LDC_BARREL_0P3_0};
	char *filename_pef[4] = {GDC_FILE_PEF_ROT0, GDC_FILE_PEF_ROT0, GDC_FILE_PEF_LDC_BARREL_0P3_0, GDC_FILE_PEF_LDC_BARREL_0P3_0};
	GDC_TEST_OP op[4] = {GDC_TEST_ROT, GDC_TEST_ROT, GDC_TEST_LDC, GDC_TEST_LDC};
	CVI_U32 WidthIn[4] = {1920, 1920, 1920, 1920};
	CVI_U32 HeightIn[4] = {1080, 1080, 1080, 1080};
	CVI_U32 WidthOut[4] = {1920, 1920, 1920, 1920};
	CVI_U32 HeightOut[4] = {1088, 1088, 1088, 1088};
#else
#if GDC_MULTI_THREADS_LOAD_MESH_BOOL
	char *filename_in[8] = {GDC_FILE_IN_LDC_BARREL_0P3, GDC_FILE_IN_LDC_BARREL_0P3, GDC_FILE_IN_LDC_BARREL_0P3, GDC_FILE_IN_LDC_BARREL_0P3,GDC_FILE_IN_LDC_BARREL_0P3, GDC_FILE_IN_LDC_BARREL_0P3, GDC_FILE_IN_LDC_BARREL_0P3, GDC_FILE_IN_LDC_BARREL_0P3};
	char *filename_out[8] = {GDC_FILE_OUT_ROT0_2, GDC_FILE_OUT_ROT0_2, GDC_FILE_OUT_ROT0_2, GDC_FILE_OUT_ROT0_2, GDC_FILE_OUT_ROT0_2, GDC_FILE_OUT_ROT0_2, GDC_FILE_OUT_ROT0_2, GDC_FILE_OUT_ROT0_2};
	char *filename_pef[8] = {GDC_FILE_PEF_LDC_BARREL_0P3_0, GDC_FILE_PEF_LDC_BARREL_0P3_0, GDC_FILE_PEF_LDC_BARREL_0P3_0, GDC_FILE_PEF_LDC_BARREL_0P3_0, GDC_FILE_PEF_LDC_BARREL_0P3_0, GDC_FILE_PEF_LDC_BARREL_0P3_0, GDC_FILE_PEF_LDC_BARREL_0P3_0, GDC_FILE_PEF_LDC_BARREL_0P3_0};
	GDC_TEST_OP op[8] = {GDC_TEST_LDC_LOAD_MESH, GDC_TEST_LDC_LOAD_MESH, GDC_TEST_LDC_LOAD_MESH, GDC_TEST_LDC_LOAD_MESH, GDC_TEST_LDC_LOAD_MESH, GDC_TEST_LDC_LOAD_MESH, GDC_TEST_LDC_LOAD_MESH, GDC_TEST_LDC_LOAD_MESH};
	CVI_U32 WidthIn[8] = {1920, 1920, 1920, 1920, 1920, 1920, 1920, 1920};
	CVI_U32 HeightIn[8] = {1080, 1080, 1080, 1080, 1080, 1080, 1080, 1080};
	CVI_U32 WidthOut[8] = {1920, 1920, 1920, 1920, 1920, 1920, 1920, 1920};
	CVI_U32 HeightOut[8] = {1088, 1088, 1088, 1088, 1088, 1088, 1088, 1088};
	pthread_t thread[8] = {[0 ... 7] = -1};
#else
	char *filename_in[4] = {GDC_FILE_IN_ROT_2, GDC_FILE_IN_ROT_2, GDC_FILE_IN_ROT_2, GDC_FILE_IN_ROT_2};
	char *filename_out[4] = {GDC_FILE_OUT_ROT0_2, GDC_FILE_OUT_ROT0_2, GDC_FILE_OUT_ROT0_2, GDC_FILE_OUT_ROT0_2};
	char *filename_pef[4] = {GDC_FILE_PEF_ROT0_2, GDC_FILE_PEF_ROT0_2, GDC_FILE_PEF_ROT0_2, GDC_FILE_PEF_ROT0_2};
	GDC_TEST_OP op[4] = {GDC_TEST_ROT, GDC_TEST_ROT, GDC_TEST_ROT, GDC_TEST_ROT};
	CVI_U32 WidthIn[4] = {4608, 4608, 4608, 4608};
	CVI_U32 HeightIn[4] = {4608, 4608, 4608, 4608};
	CVI_U32 WidthOut[4] = {4608, 4608, 4608, 4608};
	CVI_U32 HeightOut[4] = {4608, 4608, 4608, 4608};
	pthread_t thread[4] = {[0 ... 3] = -1};
#endif
#endif

	param[0].size_in.u32Width = WidthIn[0];
	param[0].size_in.u32Height = HeightIn[0];
	param[0].size_out.u32Width = WidthOut[0];
#if GDC_MULTI_THREADS_LOAD_MESH_BOOL
	param[0].size_out.u32Height = ALIGN(HeightOut[0], DEFAULT_ALIGN);
#else
	param[0].size_out.u32Height = HeightOut[0];
#endif
	param[0].enPixelFormat = GDC_DEFAULT_PIXEL_FMT;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	param[0].u32BlkSizeIn = COMMON_GetPicBufferSize(param[0].size_in.u32Width, param[0].size_in.u32Height, param[0].enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);
	param[0].u32BlkSizeOut = COMMON_GetPicBufferSize(param[0].size_out.u32Width, param[0].size_out.u32Height, param[0].enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);

	BlkSize = (param->u32BlkSizeIn > param->u32BlkSizeOut) ? param->u32BlkSizeIn : param->u32BlkSizeOut;

	stVbConf.u32MaxPoolCnt				= 1;
	stVbConf.astCommPool[0].u32BlkSize	= BlkSize;
#if GDC_MULTI_THREADS_LOAD_MESH_BOOL
	stVbConf.astCommPool[0].u32BlkCnt	= 20;
#else
	stVbConf.astCommPool[0].u32BlkCnt	= 10;
#endif

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

#if GDC_MULTI_THREADS_LOAD_MESH_BOOL
	threads = 8;
#else
	threads = 4;
#endif

	for (i = 0; i < threads; i++) {
		param[i].enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
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

		#if GDC_MULTI_THREADS_LOAD_MESH_BOOL
		rc = pthread_create(&thread[i], NULL, gdc_basic_thread_func1, (void *)&param[i]);
		#else
		rc = pthread_create(&thread[i], NULL, gdc_basic_thread_func0, (void *)&param[i]);
		#endif

		if (rc) {
			GDC_UT_PRT("pthread_create fail. s32Ret: 0x%x !\n", s32Ret);
			break;
		}
	}

	for (i = 0; i < threads; i++)
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

static CVI_S32 gdc_test_presure_size_for_each(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int times = GDC_REPEAT_TIMES;
	VB_CONFIG_S stVbConf;
	GDC_BASIC_TEST_PARAM param = {0};
	PIXEL_FORMAT_E enPixelFormat[2] = {
		GDC_DEFAULT_PIXEL_FMT,
		PIXEL_FORMAT_YUV_400,
	};
	CVI_U32 BlkSize;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	for (int w = GDC_MIN_W; w < GDC_MAX_W; w += GDC_MIN_W) {
		for (int h = GDC_MIN_H; h < GDC_MAX_H; h += GDC_MIN_H) {
			for (CVI_U8 fmt_idx = 0; fmt_idx < 2; fmt_idx ++) {
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
				param.op = GDC_TEST_ROT;
				param.u32BlkSizeIn = COMMON_GetPicBufferSize(GDC_MAX_W, GDC_MAX_H, PIXEL_FORMAT_YUV_PLANAR_444
					, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);
				param.u32BlkSizeOut = COMMON_GetPicBufferSize(GDC_MAX_W, GDC_MAX_H, PIXEL_FORMAT_YUV_PLANAR_444
					, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, GDC_STRIDE_ALIGN);

				BlkSize = (param.u32BlkSizeIn > param.u32BlkSizeOut) ? param.u32BlkSizeIn : param.u32BlkSizeOut;

				stVbConf.u32MaxPoolCnt				= 1;
				stVbConf.astCommPool[0].u32BlkSize	= BlkSize;
				stVbConf.astCommPool[0].u32BlkCnt	= 2;

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
					memset(&param.stVideoFrameIn, 0, sizeof(param.stVideoFrameIn));
					s32Ret = GDC_COMM_PrepareFrame(&param.size_in, param.enPixelFormat, &param.stVideoFrameIn);
					if (s32Ret) {
						GDC_UT_PRT("GDC_COMM_PrepareFrame in failed!\n");
						goto exit2;
					}

					memset(&param.stVideoFrameOut, 0, sizeof(param.stVideoFrameOut));
					s32Ret = GDC_COMM_PrepareFrame(&param.size_out, param.enPixelFormat, &param.stVideoFrameOut);
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
					GDC_UT_PRT("-------------------times:(%d)----------------------\n", times);

					param.inBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameIn.stVFrame.u64PhyAddr[0]);
					if (param.inBlk != VB_INVALID_HANDLE)
						s32Ret |= CVI_VB_ReleaseBlock(param.inBlk);

					param.outBlk = CVI_VB_PhysAddr2Handle(param.stVideoFrameOut.stVFrame.u64PhyAddr[0]);
					if (param.outBlk != VB_INVALID_HANDLE)
						s32Ret |= CVI_VB_ReleaseBlock(param.outBlk);

					if (s32Ret) {
						GDC_UT_PRT("release VB fail.\n");
						break;
					}
				} while (times--);
			exit2:
				if (s32Ret && param.hHandle)
					s32Ret |= CVI_GDC_CancelJob(param.hHandle);
				CVI_GDC_FreeCurTaskMesh(param.stTask.name);
				s32Ret |= CVI_GDC_DeInit();
				if (s32Ret) {
					GDC_UT_PRT("CVI_GDC_DeInit fail.\n");
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
	GDC_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_pef(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	p_func test_func[4] = {gdc_test_rot, gdc_test_rot_4m, gdc_test_ldc_load_mesh, gdc_test_fmt};
	bEnProc = CVI_TRUE;

	for (int i =0 ;i < 4; i++)
		s32Ret |= test_func[i]();

	bEnProc = CVI_FALSE;

	return s32Ret;
}

static CVI_S32 gdc_test_ldc_grid_info(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	LDC_ATTR_S stLdcAttr = {0};
	GDC_BASIC_TEST_PARAM param = {0};
	char *filename_in = GDC_FILE_IN_LDC_GRID_INFO;
	char *filename_out = GDC_FILE_OUT_LDC_GRID_INFO;
	char *filename_pef = GDC_FILE_PEF_LDC_GRID_INFO;
	char *filename_grid = GDC_FILE_IN_LDC_GRID;

	param.needPef = CVI_FALSE;

	strcpy(param.filename_in, filename_in);
	strcpy(param.filename_out, filename_out);
	strcpy(param.filename_pef, filename_pef);

	param.size_in.u32Width = 1280;
	param.size_in.u32Height = ALIGN(720, DEFAULT_ALIGN);
	param.size_out.u32Width = 1280;
	param.size_out.u32Height = ALIGN(720, DEFAULT_ALIGN);
	param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
	snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_gdc_grid_0");
	param.identity.enModId = CVI_ID_USER;
	param.identity.u32ID = 0;
	snprintf(param.identity.Name, sizeof(param.identity.Name), "job_gdc_grid_0");
	param.identity.syncIo = CVI_TRUE;

	param.op = GDC_TEST_LDC;

	stLdcAttr.stGridInfoAttr.Enable = CVI_TRUE;
	strcpy(stLdcAttr.stGridInfoAttr.gridFileName, filename_grid);
	strcpy(stLdcAttr.stGridInfoAttr.gridBindName, param.stTask.name);

	s32Ret = gdc_basic(&param, (void *)&stLdcAttr);
	if (s32Ret != CVI_SUCCESS) {
		GDC_UT_PRT("Test failed.\n");
		return s32Ret;
	}

	GDC_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 gdc_test_ldc_grid_info2(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	LDC_ATTR_S stLdcAttr = {0};
	GDC_BASIC_TEST_PARAM param = {0};
	char *filename_in[10] = {
		GDC_FILE_IN_LDC_GRID_INFO0, GDC_FILE_IN_LDC_GRID_INFO1, GDC_FILE_IN_LDC_GRID_INFO2, GDC_FILE_IN_LDC_GRID_INFO3, GDC_FILE_IN_LDC_GRID_INFO4,
		GDC_FILE_IN_LDC_GRID_INFO5, GDC_FILE_IN_LDC_GRID_INFO6, GDC_FILE_IN_LDC_GRID_INFO7, GDC_FILE_IN_LDC_GRID_INFO8, GDC_FILE_IN_LDC_GRID_INFO9,
	};
	char *filename_out[10] = {
		GDC_FILE_OUT_LDC_GRID_INFO0, GDC_FILE_OUT_LDC_GRID_INFO1, GDC_FILE_OUT_LDC_GRID_INFO2, GDC_FILE_OUT_LDC_GRID_INFO3, GDC_FILE_OUT_LDC_GRID_INFO4,
		GDC_FILE_OUT_LDC_GRID_INFO5, GDC_FILE_OUT_LDC_GRID_INFO6, GDC_FILE_OUT_LDC_GRID_INFO7, GDC_FILE_OUT_LDC_GRID_INFO8, GDC_FILE_OUT_LDC_GRID_INFO9,
	};
	char *filename_grid = GDC_FILE_IN_LDC_GRID_0_9;

	param.needPef = CVI_FALSE;

	for (int i = 0; i < 10; i++) {
		strcpy(param.filename_in, filename_in[i]);
		strcpy(param.filename_out, filename_out[i]);

		param.size_in.u32Width = 1280;
		param.size_in.u32Height = 720;
		param.size_out.u32Width = 1280;
		param.size_out.u32Height = ALIGN(720, DEFAULT_ALIGN);
		param.enPixelFormat = GDC_DEFAULT_PIXEL_FMT;
		snprintf(param.stTask.name, sizeof(param.stTask.name), "tsk_gdc_grid_0_9");//fix grid_info
		param.identity.enModId = CVI_ID_USER;
		param.identity.u32ID = 0;
		snprintf(param.identity.Name, sizeof(param.identity.Name), "job_gdc_grid_%d", i);
		param.identity.syncIo = CVI_TRUE;

		param.op = GDC_TEST_LDC;

		stLdcAttr.stGridInfoAttr.Enable = CVI_TRUE;
		strcpy(stLdcAttr.stGridInfoAttr.gridFileName, filename_grid);
		strcpy(stLdcAttr.stGridInfoAttr.gridBindName, param.stTask.name);

		s32Ret = gdc_basic(&param, (void *)&stLdcAttr);
		if (s32Ret != CVI_SUCCESS) {
			GDC_UT_PRT("Test failed.\n");
			return s32Ret;
		}
	}

	GDC_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}


static CVI_S32 gdc_test_auto_regression(CVI_VOID)
{
	CVI_S32 s32Ret[100] = {[0 ... 99] = CVI_SUCCESS};
	CVI_S32 Ret = CVI_SUCCESS;
	p_func test_func[MAX_FUNC_CNT] = {
		gdc_test_rot,
		gdc_test_rot_small,
		gdc_test_rot_4m,
		gdc_test_ldc,
		gdc_test_ldc_load_mesh,
		gdc_test_rot_maxsize,
		gdc_test_fmt,
		gdc_test_not_align,
		gdc_test_cmdq,
		gdc_test_cmdq_1to2,
		gdc_test_cmdq_1to2_maxsize,
		gdc_test_async,
		gdc_test_online,
		gdc_test_mix,
		gdc_test_mix_fasync,
		gdc_test_multi_thread,
		gdc_test_pef,
		gdc_test_ldc_grid_info,
		gdc_test_ldc_grid_info2,
		gdc_test_reset,
		//gdc_test_presure_size_for_each();//it takes too long time
	};

	for (int i = 0; i < MAX_FUNC_CNT; i++) {
		if (test_func[i]) {
			s32Ret[i] = test_func[i]();
			Ret |= s32Ret[i];
		}
	}

	for (int i = 0; i < MAX_FUNC_CNT; i++) {
		if (s32Ret[i])
			GDC_UT_PRT("op[%d] fail, ret[%d]\n", i, s32Ret[i]);
	}

	GDC_TEST_CHECK_RET(Ret);
	return Ret;
}

static CVI_S32 gdc_test_user_config(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32WidthIn, u32HeightIn;
	CVI_U32 u32WidthOut, u32HeightOut;
	int fmt;
	int op, tmp;
	CVI_U8 i;
	GDC_BASIC_TEST_PARAM stTestParam = {0};

	memset(&stTestParam, 0, sizeof(stTestParam));
	printf("\n---gdc config---\n");
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
		if (strncmp(GDCGetFmtName(i), "unknown", sizeof("unknown")))
			printf("%2d : %s\n", i, GDCGetFmtName(i));
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
		GDCGetFmtName(stTestParam.enPixelFormat));

	printf("input rot or ldc:(rot:0,ldc:1) ");
	scanf("%d", &op);
	printf("\n");
	if (op == 0) {
		ROTATION_E rot;
		printf("input rot:(rot0:0, rot90:1, rot180:2, rot270:3) ");
		scanf("%d", &tmp);
		printf("\n");

		rot = (ROTATION_E)tmp;
		stTestParam.op = GDC_TEST_ROT;
		s32Ret = gdc_basic(&stTestParam, (void *)rot);
	} else {

		LDC_ATTR_S stLDCAttr = {0};

		printf("Keep AspectRatio 1(Y)/0(N): ");
		scanf("%d", &tmp);
		stLDCAttr.bAspect = tmp;
		if (stLDCAttr.bAspect) {
			printf("Ratio (0 ~ 100): ");
			scanf("%d", &tmp);
			stLDCAttr.s32XYRatio = tmp;
		} else {
			printf("XRatio (0 ~ 100): ");
			scanf("%d", &stLDCAttr.s32XRatio);
			printf("YRatio (0 ~ 100): ");
			scanf("%d", &stLDCAttr.s32YRatio);
		}
		printf("XOffset (-511 ~ 511): ");
		scanf("%d", &stLDCAttr.s32CenterXOffset);
		printf("YOffset (-511 ~ 511): ");
		scanf("%d", &stLDCAttr.s32CenterYOffset);
		printf("DistortionRatio (-300 ~ 500): ");
		scanf("%d", &stLDCAttr.s32DistortionRatio);

		stTestParam.op = GDC_TEST_LDC;
		s32Ret = gdc_basic(&stTestParam, (void *)&stLDCAttr);
	}

	if (s32Ret != CVI_SUCCESS) {
		GDC_UT_PRT("Test failed.\n");
		return s32Ret;
	}

	GDC_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static int dup_fd;
static int dup_fd_bak = 1000;

static CVI_S32 gdc_dup_fd(CVI_VOID)
{
	dup_fd = open( "./gdc_printf_dup_log.txt ", O_CREAT | O_RDWR | O_TRUNC);
	dup2(STDOUT_FILENO, dup_fd_bak);/*backup stdout*/
	dup2(dup_fd, STDOUT_FILENO);
	return CVI_SUCCESS;
}

static CVI_S32 gdc_rst_fd(CVI_VOID)
{
	dup2(dup_fd_bak, fileno(stdout));/*recover stdout*/
	close(dup_fd);
	return CVI_SUCCESS;
}

static CVI_S32 _gdc_handle_op(CVI_S32 op)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	switch (op) {
	case GDC_TEST_ROT:
		s32Ret = gdc_test_rot();
		break;
	case GDC_TEST_ROT1:
		s32Ret = gdc_test_rot_small();
		break;
	case GDC_TEST_ROT2:
		s32Ret = gdc_test_rot_4m();
		break;
	case GDC_TEST_LDC:
		s32Ret = gdc_test_ldc();
		break;
	case GDC_TEST_LDC_LOAD_MESH:
		s32Ret = gdc_test_ldc_load_mesh();
		break;
	case GDC_TEST_MAX_SIZE:
		s32Ret = gdc_test_rot_maxsize();
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
	case GDC_TEST_CMDQ_1TO2_MAX:
		s32Ret = gdc_test_cmdq_1to2_maxsize();
		break;
	case GDC_TEST_ONLINE:
		s32Ret = gdc_test_online();
		break;
	case GDC_TEST_ASYNC:
		s32Ret = gdc_test_async();
		break;
	case GDC_TEST_MIX:
		s32Ret = gdc_test_mix();
		break;
	case GDC_TEST_MIX_FASYNC:
		s32Ret = gdc_test_mix_fasync();
		break;
	case GDC_TEST_MULTI_THREAD:
		s32Ret = gdc_test_multi_thread();
		break;
	case GDC_TEST_PEF:
		s32Ret = gdc_test_pef();
		break;
	case GDC_TEST_LOAD_GRID_INFO:
		s32Ret = gdc_test_ldc_grid_info();
		break;
	case GDC_TEST_LOAD_GRID_INFO2:
		s32Ret = gdc_test_ldc_grid_info2();
		break;
	case GDC_TEST_RST:
		s32Ret = gdc_test_reset();
		break;
	case GDC_TEST_PRESURE_SIZE_FOR_EACH:
		s32Ret = gdc_test_presure_size_for_each();
		break;
	case GDC_TEST_AUTO_REGRESSION:
		s32Ret = gdc_test_auto_regression();
		break;
	case GDC_TEST_USER_CONFIG:
		s32Ret = gdc_test_user_config();
		break;
	case GDC_TEST_DUP_FD:
		s32Ret = gdc_dup_fd();
		break;
	case GDC_TEST_RST_FD:
		s32Ret = gdc_rst_fd();
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
	GDC_UT_PRT("%4d: gdc basic test rot small\n", GDC_TEST_ROT1);
	GDC_UT_PRT("%4d: gdc basic test rot 4m\n", GDC_TEST_ROT2);
	GDC_UT_PRT("%4d: gdc basic test ldc\n", GDC_TEST_LDC);
	GDC_UT_PRT("%4d: gdc basic test ldc load mesh\n", GDC_TEST_LDC_LOAD_MESH);
	GDC_UT_PRT("%4d: gdc basic test maxsize\n", GDC_TEST_MAX_SIZE);
	GDC_UT_PRT("%4d: gdc basic test fmt\n", GDC_TEST_FMT);
	GDC_UT_PRT("%4d: gdc basic test size not align\n", GDC_TEST_SIZE_NO_ALIGN);
	GDC_UT_PRT("%4d: gdc basic test cmdq\n", GDC_TEST_CMDQ);
	GDC_UT_PRT("%4d: gdc basic test cmdq_1to2\n", GDC_TEST_CMDQ_1TO2);
	GDC_UT_PRT("%4d: gdc basic test cmdq_1to2 maxsize\n", GDC_TEST_CMDQ_1TO2_MAX);
	GDC_UT_PRT("%4d: gdc basic test online\n", GDC_TEST_ONLINE);
	GDC_UT_PRT("%4d: gdc basic test async\n", GDC_TEST_ASYNC);
	GDC_UT_PRT("%4d: gdc basic test mix\n", GDC_TEST_MIX);
	GDC_UT_PRT("%4d: gdc basic test mix fasync\n", GDC_TEST_MIX_FASYNC);
	GDC_UT_PRT("%4d: gdc basic test multi thread\n", GDC_TEST_MULTI_THREAD);
	GDC_UT_PRT("%4d: gdc basic test pef\n", GDC_TEST_PEF);
	GDC_UT_PRT("%4d: gdc basic test grid_info\n", GDC_TEST_LOAD_GRID_INFO);
	GDC_UT_PRT("%4d: gdc basic test grid_info2\n", GDC_TEST_LOAD_GRID_INFO2);
	GDC_UT_PRT("%4d: gdc basic test reset\n", GDC_TEST_RST);
	GDC_UT_PRT("%4d: gdc test presure size for each\n", GDC_TEST_PRESURE_SIZE_FOR_EACH);
	GDC_UT_PRT("%4d: gdc test auto regression\n", GDC_TEST_AUTO_REGRESSION);
	GDC_UT_PRT("%4d: gdc user cofig test\n", GDC_TEST_USER_CONFIG);
	GDC_UT_PRT("%4d: gdc dup fd\n", GDC_TEST_DUP_FD);
	GDC_UT_PRT("%4d: gdc dup fd\n", GDC_TEST_RST_FD);

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

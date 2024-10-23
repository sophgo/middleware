#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "cvi_buffer.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"

#include "sample_comm.h"
#include "affine_trans.hpp"


#define FILE_NAME "check_813_000000.yuv_Y_ori.ppm.yuv"
#define FILE_WIDTH_CNV    1280
#define FILE_HEIGHT_CNV   720
#define FILE_MOUNT_CNV    FISHEYE_DESKTOP_MOUNT


static CVI_S32 _handle_op(CVI_S32 op, CVI_VOID *param1, CVI_VOID *param2);

static void _vpss_start_for_fdfr(VPSS_GRP VpssGrp, SIZE_S stViSize)
{
	VPSS_GRP_ATTR_S stVpssGrpAttr;
	VPSS_CHN VpssChn = VPSS_CHN0;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = { 0 };
	VPSS_CHN_ATTR_S astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
	CVI_S32 s32Ret = CVI_SUCCESS;

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssGrpAttr.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	stVpssGrpAttr.u32MaxW = stViSize.u32Width;
	stVpssGrpAttr.u32MaxH = stViSize.u32Height;

	// smaller, normalized output
	VpssChn = 0;
	astVpssChnAttr[VpssChn].u32Width = 608;
	astVpssChnAttr[VpssChn].u32Height = 608;
	astVpssChnAttr[VpssChn].enVideoFormat = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat = PIXEL_FORMAT_RGB_888_PLANAR;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth = 1;
	astVpssChnAttr[VpssChn].bMirror = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stNormalize.bEnable = CVI_TRUE;
	astVpssChnAttr[VpssChn].stNormalize.factor[0] = 0.8415;
	astVpssChnAttr[VpssChn].stNormalize.factor[1] = 0.8415;
	astVpssChnAttr[VpssChn].stNormalize.factor[2] = 0.8415;
	astVpssChnAttr[VpssChn].stNormalize.mean[0] = 102.9801;
	astVpssChnAttr[VpssChn].stNormalize.mean[1] = 115.9465;
	astVpssChnAttr[VpssChn].stNormalize.mean[2] = 122.7717;
	astVpssChnAttr[VpssChn].stNormalize.rounding = VPSS_ROUNDING_TO_EVEN;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor = COLOR_RGB_BLACK;

	// Output RGB format, size is the same with VI's.
	VpssChn = 1;
	astVpssChnAttr[VpssChn].u32Width = 320;
	astVpssChnAttr[VpssChn].u32Height = 240;
	astVpssChnAttr[VpssChn].enVideoFormat = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat = PIXEL_FORMAT_BGR_888;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth = 1;
	astVpssChnAttr[VpssChn].bMirror = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = abChnEnable[1] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr,
				       astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr,
					astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return;
	}
}

/* Setup VPSS grp which will scale down input and output 1/2 at chn0, 1/4 at chn1.
 *
 * VpssGrp: the vpss-grp to be setup.
 * stSize: input size.
 */
static void _vpss_start_pyramid(VPSS_GRP VpssGrp, SIZE_S stSize)
{
	VPSS_GRP_ATTR_S stVpssGrpAttr;
	VPSS_CHN VpssChn;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = { 0 };
	VPSS_CHN_ATTR_S astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
	CVI_S32 s32Ret = CVI_SUCCESS;

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssGrpAttr.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	stVpssGrpAttr.u32MaxW = stSize.u32Width;
	stVpssGrpAttr.u32MaxH = stSize.u32Height;

	// smaller, normalized output
	VpssChn = 0;
	astVpssChnAttr[VpssChn].u32Width = (stSize.u32Width >> 1) & ~0x1;
	astVpssChnAttr[VpssChn].u32Height = (stSize.u32Height >> 1) & ~0x1;
	astVpssChnAttr[VpssChn].enVideoFormat = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth = 1;
	astVpssChnAttr[VpssChn].bMirror = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable = CVI_FALSE;

	// Output RGB format, size is the same with VI's.
	VpssChn = 1;
	astVpssChnAttr[VpssChn].u32Width = (stSize.u32Width >> 2) & ~0x1;
	astVpssChnAttr[VpssChn].u32Height = (stSize.u32Height >> 2) & ~0x1;
	astVpssChnAttr[VpssChn].enVideoFormat = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth = 1;
	astVpssChnAttr[VpssChn].bMirror = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = abChnEnable[1] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr,
				       astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr,
					astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return;
	}
}

static void _vpss_stop(VPSS_GRP VpssGrp)
{
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = { 0 };
	CVI_S32 s32Ret = CVI_SUCCESS;

	abChnEnable[0] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Stop(VpssGrp, abChnEnable);
	if (s32Ret != CVI_SUCCESS)
		SAMPLE_PRT("stop vpss group failed. s32Ret: 0x%x !\n", s32Ret);
}

static CVI_S32 _vpss_get_chn_frame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
				   const char *filename)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	FILE *fp;
	CVI_S32 ret = CVI_SUCCESS;
	CVI_VOID *vir_addr;
	CVI_U32 plane_offset;

	ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, 50);
	if (ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VPSS_GetChnFrame failed with %#x\n", ret);
		return ret;
	}

	fp = fopen(filename, "w");
	if (fp == CVI_NULL) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "open data file error\n");
		return CVI_FAILURE;
	}

	size_t image_size = stVideoFrame.stVFrame.u32Length[0] +
			    stVideoFrame.stVFrame.u32Length[1] +
			    stVideoFrame.stVFrame.u32Length[2];

	SAMPLE_PRT("width: %d, height: %d, total_buf_length: %zu\n",
		   stVideoFrame.stVFrame.u32Width,
		   stVideoFrame.stVFrame.u32Height, image_size);
	vir_addr =
		CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0], image_size);
	plane_offset = 0;
	for (int i = 0; i < 3; i++) {
		if (stVideoFrame.stVFrame.u32Length[i] == 0)
			continue;

		stVideoFrame.stVFrame.pu8VirAddr[i] =
			(CVI_U8 *)vir_addr + plane_offset;
		plane_offset += stVideoFrame.stVFrame.u32Length[i];
		SAMPLE_PRT("plane(%d): paddr(%#" PRIx64
			   ") vaddr(%p) stride(%d)\n",
			   i, stVideoFrame.stVFrame.u64PhyAddr[i],
			   stVideoFrame.stVFrame.pu8VirAddr[i],
			   stVideoFrame.stVFrame.u32Stride[i]);
		fwrite(stVideoFrame.stVFrame.pu8VirAddr[i],
		       stVideoFrame.stVFrame.u32Length[i], 1, fp);
	}
	CVI_SYS_Munmap(vir_addr, image_size);

	if (CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame) != 0)
		SAMPLE_PRT("CVI_VPSS_ReleaseChnFrame NG\n");

	fclose(fp);
	return ret;
}

static int _vi_get_chn_frmae_and_send_vpss(CVI_VOID *param1, CVI_VOID *param2)
{
	SAMPLE_VI_CONFIG_S *pstViConfig = param1;
	SIZE_S *pstSize = param2;

	VI_DEV ViDev = 0;
	VI_PIPE ViPipe = 0;
	VI_PIPE_ATTR_S stPipeAttr;
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = SAMPLE_COMM_VI_StartSensor(pstViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR,
			      "system start sensor failed with %#x\n", s32Ret);
		return s32Ret;
	}
	s32Ret = SAMPLE_COMM_VI_StartDev(&pstViConfig->astViInfo[ViDev]);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "VI_StartDev failed with %#x!\n",
			      s32Ret);
		return s32Ret;
	}
	s32Ret = SAMPLE_COMM_VI_StartMIPI(pstViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR,
			      "system start MIPI failed with %#x\n", s32Ret);
		return s32Ret;
	}

	stPipeAttr.bYuvSkip = CVI_FALSE;
	stPipeAttr.u32MaxW = pstSize->u32Width;
	stPipeAttr.u32MaxH = pstSize->u32Height;
	stPipeAttr.enPixFmt = PIXEL_FORMAT_RGB_BAYER_12BPP;
	stPipeAttr.enBitWidth = DATA_BITWIDTH_12;
	stPipeAttr.stFrameRate.s32SrcFrameRate = -1;
	stPipeAttr.stFrameRate.s32DstFrameRate = -1;
	stPipeAttr.bNrEn = CVI_TRUE;
	s32Ret = CVI_VI_CreatePipe(ViPipe, &stPipeAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_CreatePipe failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_StartPipe(ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_StartPipe failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VI_GetPipeAttr(ViPipe, &stPipeAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VI_StartPipe failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VI_CreateIsp(pstViConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "VI_CreateIsp failed with %#x!\n",
			      s32Ret);
		return s32Ret;
	}

	SAMPLE_COMM_VI_StartViChn(pstViConfig);

	_vpss_start_for_fdfr(0, *pstSize);

	VIDEO_FRAME_INFO_S stVideoFrame;

	if (CVI_VI_GetChnFrame(0, 0, &stVideoFrame, 1000) == 0) {
		if (CVI_VPSS_SendFrame(0, &stVideoFrame, -1) == CVI_SUCCESS) {
			_vpss_get_chn_frame(0, 0, "dump0.bin");
			_vpss_get_chn_frame(0, 1, "dump1.bin");
		}
		if (CVI_VI_ReleaseChnFrame(0, 0, &stVideoFrame) != 0)
			SAMPLE_PRT("CVI_VI_ReleaseChnFrame NG\n");
	} else {
		SAMPLE_PRT("CVI_VI_GetChnFrame NG\n");
	}

	return s32Ret;
}

static long diff_in_us(struct timespec t1, struct timespec t2)
{
	struct timespec diff;

	if (t2.tv_nsec - t1.tv_nsec < 0) {
		diff.tv_sec = t2.tv_sec - t1.tv_sec - 1;
		diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
	} else {
		diff.tv_sec = t2.tv_sec - t1.tv_sec;
		diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
	}
	return (diff.tv_sec * 1000000.0 + diff.tv_nsec / 1000.0);
}

static void _vpss_stress_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 loop = 0;
	VPSS_GRP VpssGrp = 0;
	VIDEO_FRAME_INFO_S stVideoFrame, stOutVideoFrame0, stOutVideoFrame1;
	VB_BLK blk;
	FILE *fp;
	CVI_U32 u32len;
	SIZE_S stSize = { .u32Width = 1920, .u32Height = 1080 };

	if (SAMPLE_COMM_PrepareFrame(stSize, PIXEL_FORMAT_YUV_PLANAR_420,
				     &stVideoFrame) != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_PrepareFrame failed\n");
		return;
	}
	blk = CVI_VB_PhysAddr2Handle(stVideoFrame.stVFrame.u64PhyAddr[0]);

	SAMPLE_PRT("Make sure input file, sample.yuv of 1920x108 ready.\n");

	stVideoFrame.stVFrame.pu8VirAddr[0] =
		CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0],
			     stVideoFrame.stVFrame.u32Length[0]);
	stVideoFrame.stVFrame.pu8VirAddr[1] =
		CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[1],
			     stVideoFrame.stVFrame.u32Length[1]);
	stVideoFrame.stVFrame.pu8VirAddr[2] =
		CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[2],
			     stVideoFrame.stVFrame.u32Length[2]);
	SAMPLE_PRT("phy addr(%#" PRIx64 ", %#" PRIx64 ", %#" PRIx64 "\n",
		   stVideoFrame.stVFrame.u64PhyAddr[0],
		   stVideoFrame.stVFrame.u64PhyAddr[1],
		   stVideoFrame.stVFrame.u64PhyAddr[2]);
	SAMPLE_PRT("vir addr(%p, %p, %p\n", stVideoFrame.stVFrame.pu8VirAddr[0],
		   stVideoFrame.stVFrame.pu8VirAddr[1],
		   stVideoFrame.stVFrame.pu8VirAddr[2]);

	// open data file & fread into the mmap address
	fp = fopen("sample.yuv", "r");
	if (fp == CVI_NULL) {
		SAMPLE_PRT("open data file error\n");
		goto OPEN_FAIL;
	}

	for (int i = 0; i < 3; ++i) {
		u32len = fread((void *)stVideoFrame.stVFrame.pu8VirAddr[i],
			       stVideoFrame.stVFrame.u32Length[i], 1, fp);
		if (u32len <= 0) {
			SAMPLE_PRT("fread data(%d) error\n", i);
			goto READ_FAIL;
		}
	}

	_vpss_start_for_fdfr(0, stSize);

	SAMPLE_PRT("how many loops to do(11111 is infinite: ");
	scanf("%d", &loop);
	struct timespec start, end, end2;
	CVI_U32 ng_count = 0, ok_count = 0;

	while (loop > 0) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		CVI_VPSS_SendFrame(VpssGrp, &stVideoFrame, -1);

		clock_gettime(CLOCK_MONOTONIC, &end);
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, 0, &stOutVideoFrame0,
					      500);
		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, 1, &stOutVideoFrame1,
					      500);
		clock_gettime(CLOCK_MONOTONIC, &end2);
		if (s32Ret != CVI_SUCCESS) {
			++ng_count;
			SAMPLE_PRT("CVI_VPSS_GetChnFrame failed with %#x\n",
				   s32Ret);
		} else {
			++ok_count;
			if (CVI_VPSS_ReleaseChnFrame(VpssGrp, 0,
						     &stOutVideoFrame0) !=
			    CVI_SUCCESS)
				SAMPLE_PRT("CVI_VPSS_ReleaseChnFrame 0 NG\n");
			if (CVI_VPSS_ReleaseChnFrame(VpssGrp, 1,
						     &stOutVideoFrame1) !=
			    CVI_SUCCESS)
				SAMPLE_PRT("CVI_VPSS_ReleaseChnFrame 1 NG\n");
		}
		SAMPLE_PRT("sendtoget:%ldus, sendtogetdone: %ldus\n",
			   diff_in_us(start, end), diff_in_us(start, end2));

		if (loop != 11111)
			loop--;
	}
	SAMPLE_PRT("Summary OK: %d, NG: %d\n", ok_count, ng_count);

	_vpss_stop(0);
READ_FAIL:
	fclose(fp);
OPEN_FAIL:
	CVI_VB_ReleaseBlock(blk);
	CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[0],
		       stVideoFrame.stVFrame.u32Length[0]);
	CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[1],
		       stVideoFrame.stVFrame.u32Length[1]);
	CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[2],
		       stVideoFrame.stVFrame.u32Length[2]);
}

static void _affine_test(void)
{
	CVI_U8 affine_num = 2;
	VIDEO_FRAME_INFO_S stVideoFrame;
	VB_BLK blk_in, blk_out;
	FILE *fp;
	CVI_U32 u32len;
	SIZE_S stSize = { .u32Width = 1920, .u32Height = 1080 };
	GDC_HANDLE hHandle;
	GDC_TASK_ATTR_S stTask;
	AFFINE_ATTR_S stAffineAttr;

	if (SAMPLE_COMM_PrepareFrame(stSize, PIXEL_FORMAT_YUV_PLANAR_420,
				     &stVideoFrame) != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_PrepareFrame failed\n");
		return;
	}
	blk_in = CVI_VB_PhysAddr2Handle(stVideoFrame.stVFrame.u64PhyAddr[0]);

	/************************************************
	 * step1:  find bounding box by landmarks.
	 ************************************************/
	stAffineAttr.u32RegionNum = affine_num;
	POINT2F_S faces[][5] = {
		{ { .x = 1242, .y = 170 },
		  { .x = 1273, .y = 158 },
		  { .x = 1262, .y = 185 },
		  { .x = 1263, .y = 207 },
		  { .x = 1280, .y = 199 } },
		{ { .x = 1515, .y = 201 },
		  { .x = 1540, .y = 219 },
		  { .x = 1520, .y = 226 },
		  { .x = 1500, .y = 234 },
		  { .x = 1518, .y = 244 } },
	};

	// Need opencv library support
	// find_bounding_box source code in affine_trans.cpp
	find_bounding_box(faces[0], stAffineAttr.astRegionAttr[0]);
	find_bounding_box(faces[1], stAffineAttr.astRegionAttr[1]);

	stAffineAttr.stDestSize.u32Width = 112;
	stAffineAttr.stDestSize.u32Height = 112;

	/************************************************
	 * step2:  Read input file.
	 ************************************************/
	SAMPLE_PRT("Make sure input file, girls.yuv of 1920x108 ready.\n");

	stVideoFrame.stVFrame.pu8VirAddr[0] =
		CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0],
			     stVideoFrame.stVFrame.u32Length[0]);
	stVideoFrame.stVFrame.pu8VirAddr[1] =
		CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[1],
			     stVideoFrame.stVFrame.u32Length[1]);
	stVideoFrame.stVFrame.pu8VirAddr[2] =
		CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[2],
			     stVideoFrame.stVFrame.u32Length[2]);
	SAMPLE_PRT("phy addr(%#" PRIx64 ", %#" PRIx64 ", %#" PRIx64 "\n",
		   stVideoFrame.stVFrame.u64PhyAddr[0],
		   stVideoFrame.stVFrame.u64PhyAddr[1],
		   stVideoFrame.stVFrame.u64PhyAddr[2]);
	SAMPLE_PRT("vir addr(%p, %p, %p\n", stVideoFrame.stVFrame.pu8VirAddr[0],
		   stVideoFrame.stVFrame.pu8VirAddr[1],
		   stVideoFrame.stVFrame.pu8VirAddr[2]);

	// open data file & fread into the mmap address
	fp = fopen("girls.yuv", "r");
	if (fp == CVI_NULL) {
		SAMPLE_PRT("open data file error\n");
		goto ROPEN_FAIL;
	}

	for (int i = 0; i < 3; ++i) {
		u32len = fread((void *)stVideoFrame.stVFrame.pu8VirAddr[i],
			       stVideoFrame.stVFrame.u32Length[i], 1, fp);
		if (u32len <= 0) {
			SAMPLE_PRT("fread data(%d) error\n", i);
			goto READ_FAIL;
		}
	}

	stTask.stImgIn = stVideoFrame;

	/************************************************
	 * step3:  Prepare output buffer
	 ************************************************/
	stSize.u32Width = ALIGN(112, 32);
	stSize.u32Height = 112 * affine_num;

	if (SAMPLE_COMM_PrepareFrame(stSize, PIXEL_FORMAT_YUV_PLANAR_420,
				     &stVideoFrame) != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_PrepareFrame failed\n");
		goto GET_VB_OUT_FAIL;
	}
	blk_out = CVI_VB_PhysAddr2Handle(stVideoFrame.stVFrame.u64PhyAddr[0]);

	SAMPLE_PRT("phy addr(%#" PRIx64 ", %#" PRIx64 ", %#" PRIx64 "\n",
		   stVideoFrame.stVFrame.u64PhyAddr[0],
		   stVideoFrame.stVFrame.u64PhyAddr[1],
		   stVideoFrame.stVFrame.u64PhyAddr[2]);

	stTask.stImgOut = stVideoFrame;

	/************************************************
	 * step4:  Setup GDC.
	 ************************************************/
	CVI_GDC_BeginJob(&hHandle);

	CVI_GDC_AddAffineTask(hHandle, &stTask, &stAffineAttr);

	if (CVI_GDC_EndJob(hHandle) != CVI_SUCCESS) {
		SAMPLE_PRT("GDC Job failed.\n");
		goto GDC_FAIL;
	}

	/************************************************
	 * step5:  Write result to file.
	 ************************************************/
	SAMPLE_COMM_FRAME_SaveToFile("align.yuv", &stTask.stImgOut);

GDC_FAIL:
	CVI_VB_ReleaseBlock(blk_out);
GET_VB_OUT_FAIL:
READ_FAIL:
	fclose(fp);
ROPEN_FAIL:
	CVI_SYS_Munmap(stTask.stImgIn.stVFrame.pu8VirAddr[0],
		       stTask.stImgIn.stVFrame.u32Length[0]);
	CVI_SYS_Munmap(stTask.stImgIn.stVFrame.pu8VirAddr[1],
		       stTask.stImgIn.stVFrame.u32Length[1]);
	CVI_SYS_Munmap(stTask.stImgIn.stVFrame.pu8VirAddr[2],
		       stTask.stImgIn.stVFrame.u32Length[2]);
	CVI_VB_ReleaseBlock(blk_in);
}

static void _pyramid_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_CHAR filename[32];

	/* Step 1
	 *  Prepare vpss input from file.
	 */
	VIDEO_FRAME_INFO_S stVideoFrame, stVideoFrameOut;
	CVI_U32 u32len;
	SIZE_S stSize = { .u32Width = 1920, .u32Height = 1080 };
	VB_BLK blk;
	FILE *fp;

	if (SAMPLE_COMM_PrepareFrame(stSize, PIXEL_FORMAT_YUV_PLANAR_420,
				     &stVideoFrame) != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_PrepareFrame failed\n");
		return;
	}
	blk = CVI_VB_PhysAddr2Handle(stVideoFrame.stVFrame.u64PhyAddr[0]);

	stVideoFrame.stVFrame.pu8VirAddr[0] =
		CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0],
			     stVideoFrame.stVFrame.u32Length[0]);
	stVideoFrame.stVFrame.pu8VirAddr[1] =
		CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[1],
			     stVideoFrame.stVFrame.u32Length[1]);
	stVideoFrame.stVFrame.pu8VirAddr[2] =
		CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[2],
			     stVideoFrame.stVFrame.u32Length[2]);
	SAMPLE_PRT("phy addr(%#" PRIx64 ", %#" PRIx64 ", %#" PRIx64 "\n",
		   stVideoFrame.stVFrame.u64PhyAddr[0],
		   stVideoFrame.stVFrame.u64PhyAddr[1],
		   stVideoFrame.stVFrame.u64PhyAddr[2]);
	SAMPLE_PRT("vir addr(%p, %p, %p\n", stVideoFrame.stVFrame.pu8VirAddr[0],
		   stVideoFrame.stVFrame.pu8VirAddr[1],
		   stVideoFrame.stVFrame.pu8VirAddr[2]);

	// open data file & fread into the mmap address
	fp = fopen("sample.yuv", "r");
	if (fp == CVI_NULL) {
		SAMPLE_PRT("open data file error\n");
		goto OPEN_FAIL;
	}

	for (int i = 0; i < 3; ++i) {
		u32len = fread((void *)stVideoFrame.stVFrame.pu8VirAddr[i],
			       stVideoFrame.stVFrame.u32Length[i], 1, fp);
		if (u32len <= 0) {
			SAMPLE_PRT("fread data(%d) error\n", i);
			goto READ_FAIL;
		}
	}

	/* Step 2
	 *  Setup VPSS to do image pyramid.
	 *   Grp0: Take input from file and output 1/2 & 1/4 outputs
	 *   Grp1: Take input from grp0-chn1 and output 1/2 & 1/4 outputs, which are 1/8 and 1/16 of
	 * original input.
	 */
	_vpss_start_pyramid(0, stSize);

	stSize.u32Width = (stSize.u32Width >> 2) & ~0x1;
	stSize.u32Height = (stSize.u32Height >> 2) & ~0x1;
	_vpss_start_pyramid(1, stSize);

	s32Ret = SAMPLE_COMM_VPSS_Bind_VPSS(0, 1, 1);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR,
			      "vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
		return;
	}

	/* Step 3
	 *  Send frame to VPSS to process and get output.
	 */
	CVI_VPSS_SendFrame(0, &stVideoFrame, -1);
	for (VPSS_GRP grp = 0; grp < 2; grp++)
		for (VPSS_CHN chn = 0; chn < 2; chn++) {
			s32Ret = CVI_VPSS_GetChnFrame(grp, chn,
						      &stVideoFrameOut, 500);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(
					CVI_DBG_ERR,
					"CVI_VPSS_GetChnFrame for grp%d chn%d. s32Ret: 0x%x !\n",
					grp, chn, s32Ret);
				return;
			}

			snprintf(filename, 32, "pyramid%d_%d.yuv", grp, chn);
			SAMPLE_COMM_FRAME_SaveToFile(filename,
							 &stVideoFrameOut);

			CVI_VPSS_ReleaseChnFrame(grp, chn, &stVideoFrameOut);
		}

READ_FAIL:
	fclose(fp);
OPEN_FAIL:
	CVI_VB_ReleaseBlock(blk);
	CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[0],
		       stVideoFrame.stVFrame.u32Length[0]);
	CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[1],
		       stVideoFrame.stVFrame.u32Length[1]);
	CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[2],
		       stVideoFrame.stVFrame.u32Length[2]);
}

static void _normalize_test(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VIDEO_FRAME_INFO_S stVideoFrame, stVideoFrameYUV, stVideoFrameRGB;
	CVI_U32 u32len;
	FILE *fp;
	SIZE_S stSize = { .u32Width = 1920, .u32Height = 1080 };
	VB_BLK blk;

	/* Step 1
	 *  start vpss
	 */
	_vpss_start_for_fdfr(0, stSize);

	/* Step 2
	 *  Prepare vpss input from file.
	 *  test yuv frame
	 */
	if (SAMPLE_COMM_PrepareFrame(stSize, PIXEL_FORMAT_YUV_PLANAR_420,
				     &stVideoFrame) != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_PrepareFrame failed\n");
		return;
	}
	blk = CVI_VB_PhysAddr2Handle(stVideoFrame.stVFrame.u64PhyAddr[0]);

	SAMPLE_PRT("phy addr(%#" PRIx64 ", %#" PRIx64 ", %#" PRIx64 "\n",
		   stVideoFrame.stVFrame.u64PhyAddr[0],
		   stVideoFrame.stVFrame.u64PhyAddr[1],
		   stVideoFrame.stVFrame.u64PhyAddr[2]);

	// open data file & fread into the mmap address
	fp = fopen("norm_test.yuv", "r");
	if (fp == CVI_NULL) {
		SAMPLE_PRT("open data file error\n");
		return;
	}

	for (int i = 0; i < 3; ++i) {
		if (stVideoFrame.stVFrame.u32Length[i] == 0)
			break;

		stVideoFrame.stVFrame.pu8VirAddr[i] =
			CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[i],
				     stVideoFrame.stVFrame.u32Length[i]);

		u32len = fread((void *)stVideoFrame.stVFrame.pu8VirAddr[i],
			       stVideoFrame.stVFrame.u32Length[i], 1, fp);
		if (u32len <= 0) {
			SAMPLE_PRT("fread data(%d) error\n", i);
			goto READ_FAIL;
		}
	}
	fclose(fp);

	CVI_VPSS_SendFrame(0, &stVideoFrame, -1);
	s32Ret = CVI_VPSS_GetChnFrame(0, 0, &stVideoFrameYUV, 500);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR,
			      "CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n",
			      s32Ret);
		return;
	}

	CVI_VB_ReleaseBlock(blk);
	SAMPLE_COMM_FRAME_SaveToFile("norm_yuv.bin", &stVideoFrameYUV);
	CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrameYUV);

	PAUSE();
	PAUSE();

	/* Step 3
	 *  Prepare vpss input from file.
	 *  test rgb frame
	 */
	VPSS_GRP_ATTR_S stGrpAttr;

	s32Ret = CVI_VPSS_GetGrpAttr(0, &stGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetGrpAttr is fail\n");
		return;
	}

	stGrpAttr.enPixelFormat = PIXEL_FORMAT_BGR_888;
	CVI_VPSS_SetGrpAttr(0, &stGrpAttr);

	if (SAMPLE_COMM_PrepareFrame(stSize, PIXEL_FORMAT_BGR_888,
				     &stVideoFrame) != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_PrepareFrame failed\n");
		return;
	}
	blk = CVI_VB_PhysAddr2Handle(stVideoFrame.stVFrame.u64PhyAddr[0]);

	SAMPLE_PRT("phy addr(%#" PRIx64 ", %#" PRIx64 ", %#" PRIx64 "\n",
		   stVideoFrame.stVFrame.u64PhyAddr[0],
		   stVideoFrame.stVFrame.u64PhyAddr[1],
		   stVideoFrame.stVFrame.u64PhyAddr[2]);

	// open data file & fread into the mmap address
	fp = fopen("norm_test.bgr", "r");
	if (fp == CVI_NULL) {
		SAMPLE_PRT("open data file error\n");
		return;
	}

	for (int i = 0; i < 3; ++i) {
		if (stVideoFrame.stVFrame.u32Length[i] == 0)
			break;

		stVideoFrame.stVFrame.pu8VirAddr[i] =
			CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[i],
				     stVideoFrame.stVFrame.u32Length[i]);

		u32len = fread((void *)stVideoFrame.stVFrame.pu8VirAddr[i],
			       stVideoFrame.stVFrame.u32Length[i], 1, fp);
		if (u32len <= 0) {
			SAMPLE_PRT("fread data(%d) error\n", i);
			goto READ_FAIL;
		}
	}

	CVI_VPSS_SendFrame(0, &stVideoFrame, -1);
	s32Ret = CVI_VPSS_GetChnFrame(0, 0, &stVideoFrameRGB, 500);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR,
			      "CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n",
			      s32Ret);
		return;
	}

	CVI_VB_ReleaseBlock(blk);
	SAMPLE_COMM_FRAME_SaveToFile("norm_rgb.bin", &stVideoFrameRGB);
	CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrameRGB);

READ_FAIL:
	fclose(fp);
}

bool _renew = true;

static void _cnv_warp_test(bool method)
{
	float *_pfmesh_data;

	// read

	int f_size;

	FILE *fpMesh = fopen("init_mesh_file.dat", "rb");

	if (fpMesh == NULL) {
		printf("init mesh file not found.\n");

		return;
	}
	fseek(fpMesh, 0, SEEK_END);
	f_size = ftell(fpMesh) / sizeof(float);
	if (f_size == -1) {
		printf("mesh file can't tell.\n");
		return;
	}

	_pfmesh_data = (float *)malloc(f_size * sizeof(float));
	fseek(fpMesh, 0, SEEK_SET);
	fread(_pfmesh_data, sizeof(float), f_size, fpMesh);
	fclose(fpMesh);
	// end
	uint32_t tbl_param[5] = {0x1440, 0x1200, 0x1200, 3, 4};
	uint8_t *p_idl, *p_tbl;

	//int f_size;
	FILE *fp = fopen("cnv_mesh_id.bin", "r");

	if (fp == NULL) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "mesh id file not found.\n");
		return;
	}
	fseek(fp, 0, SEEK_END);
	f_size = ftell(fp);
	tbl_param[3] = f_size;
	p_idl = (uint8_t *)malloc(sizeof(uint8_t) * f_size);
	if (f_size == -1) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "mesh id file can't tell.\n");
		return;
	}
	fseek(fp, 0, SEEK_SET);
	fread(p_idl, 1, f_size, fp);
	fclose(fp);
	printf("mesh id size: %d\n", f_size);

	fp = fopen("cnv_mesh_tbl.bin", "r");
	if (fp == NULL) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "mesh tbl file not found.\n");
		return;
	}
	fseek(fp, 0, SEEK_END);
	f_size = ftell(fp);
	tbl_param[4] = f_size;
	p_tbl = (uint8_t *)malloc(sizeof(uint8_t) * f_size);

	if (f_size == -1) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "mesh tbl file can't tell.\n");
		return;
	}
	fseek(fp, 0, SEEK_SET);
	fread(p_tbl, 1, f_size, fp);
	fclose(fp);

	FISHEYE_ATTR_S stAffineAttr;

	stAffineAttr.bEnable = CVI_TRUE;
	stAffineAttr.bBgColor = CVI_TRUE;
	stAffineAttr.u32BgColor = YUV_8BIT(0, 128, 128);
	stAffineAttr.s32HorOffset = FILE_WIDTH_CNV / 2;
	stAffineAttr.s32VerOffset = FILE_HEIGHT_CNV / 2;
	stAffineAttr.enMountMode = FILE_MOUNT_CNV;
	stAffineAttr.enUseMode = 0;
	stAffineAttr.u32RegionNum = 1;

	// end
	CVI_U8 affine_num = 1;
	VIDEO_FRAME_INFO_S stVideoFrame;
	VB_BLK blk_in, blk_out;
	//FILE *fp;
	CVI_U32 u32len;
	SIZE_S stSize = { .u32Width = 1280, .u32Height = 720 };
	GDC_HANDLE hHandle;
	GDC_TASK_ATTR_S stTask;
	//FISHEYE_ATTR_S stAffineAttr;
	VB_CAL_CONFIG_S stVbCalConfig;



	COMMON_GetPicBufferConfig(stSize.u32Width, stSize.u32Height,
				  PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8,
				  COMPRESS_MODE_NONE, DEFAULT_ALIGN,
				  &stVbCalConfig);

	/************************************************
	 * step1:  find bounding box by landmarks.
	 ************************************************/
	//stAffineAttr.u32RegionNum = affine_num;
	// POINT2F_S faces[][5] = {
	//	{ {.x = 1242, .y = 170}, {.x = 1273, .y = 158}, {.x = 1262, .y = 185}
	//	, {.x = 1263, .y = 207}, {.x = 1280, .y = 199} },
	//	{ {.x = 1515, .y = 201}, {.x = 1540, .y = 219}, {.x = 1520, .y = 226}
	//	, {.x = 1500, .y = 234}, {.x = 1518, .y = 244} },
	//};

	// find_bounding_box(faces[0], stAffineAttr.astRegionAttr[0]);
	// find_bounding_box(faces[1], stAffineAttr.astRegionAttr[1]);

	/************************************************
	 * step2:  Read input file.
	 ************************************************/

	SAMPLE_PRT("Make sure input file, cnv.yuv of 1280x720 ready.\n");

	stVideoFrame.stVFrame.enCompressMode = COMPRESS_MODE_NONE;
	stVideoFrame.stVFrame.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	stVideoFrame.stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
	stVideoFrame.stVFrame.enColorGamut = COLOR_GAMUT_BT709;
	stVideoFrame.stVFrame.u32Width = stSize.u32Width;
	stVideoFrame.stVFrame.u32Height = stSize.u32Height;
	stVideoFrame.stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
	stVideoFrame.stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
	stVideoFrame.stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
	stVideoFrame.stVFrame.u32TimeRef = 0;
	stVideoFrame.stVFrame.u64PTS = 0;
	stVideoFrame.stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;

	blk_in = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCalConfig.u32VBSize);
	if (blk_in == VB_INVALID_HANDLE) {
		SAMPLE_PRT("Can't acquire vb block\n");
		return;
	}

	stVideoFrame.u32PoolId = CVI_VB_Handle2PoolId(blk_in);
	stVideoFrame.stVFrame.u32Length[0] = stVbCalConfig.u32MainYSize;
	stVideoFrame.stVFrame.u32Length[1] =
		stVideoFrame.stVFrame.u32Length[2] = stVbCalConfig.u32MainCSize;
	stVideoFrame.stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk_in);
	stVideoFrame.stVFrame.u64PhyAddr[1] =
		stVideoFrame.stVFrame.u64PhyAddr[0] +
		ALIGN(stVbCalConfig.u32MainYSize, stVbCalConfig.u16AddrAlign);
	stVideoFrame.stVFrame.u64PhyAddr[2] =
		stVideoFrame.stVFrame.u64PhyAddr[1] +
		ALIGN(stVbCalConfig.u32MainCSize, stVbCalConfig.u16AddrAlign);

	stVideoFrame.stVFrame.pu8VirAddr[0] =
		CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0],
			     stVideoFrame.stVFrame.u32Length[0]);
	stVideoFrame.stVFrame.pu8VirAddr[1] =
		CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[1],
			     stVideoFrame.stVFrame.u32Length[1]);
	stVideoFrame.stVFrame.pu8VirAddr[2] =
		CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[2],
			     stVideoFrame.stVFrame.u32Length[2]);
	SAMPLE_PRT("phy addr(%#" PRIx64 ", %#" PRIx64 ", %#" PRIx64 "\n",
		   stVideoFrame.stVFrame.u64PhyAddr[0],
		   stVideoFrame.stVFrame.u64PhyAddr[1],
		   stVideoFrame.stVFrame.u64PhyAddr[2]);
	SAMPLE_PRT("vir addr(%p, %p, %p\n", stVideoFrame.stVFrame.pu8VirAddr[0],
		   stVideoFrame.stVFrame.pu8VirAddr[1],
		   stVideoFrame.stVFrame.pu8VirAddr[2]);

	// open data file & fread into the mmap address
	fp = fopen(FILE_NAME, "r");
	if (fp == CVI_NULL) {
		SAMPLE_PRT("open data file error\n");
		goto ROPEN_FAIL;
	}

	for (int i = 0; i < 3; ++i) {
		u32len = fread((void *)stVideoFrame.stVFrame.pu8VirAddr[i],
			       stVideoFrame.stVFrame.u32Length[i], 1, fp);
		if (u32len <= 0) {
			SAMPLE_PRT("fread data(%d) error\n", i);
			goto READ_FAIL;
		}
	}

	stTask.stImgIn = stVideoFrame;
	printf("step3\n");
	/************************************************
	 * step3:  Prepare output buffer
	 ************************************************/
	// stSize.u32Width = ALIGN(112, 32);
	// stSize.u32Height = 112 * affine_num;
	stSize.u32Width = ALIGN(1280, 32);
	stSize.u32Height = 720 * affine_num;

	COMMON_GetPicBufferConfig(stSize.u32Width, stSize.u32Height,
				  PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8,
				  COMPRESS_MODE_NONE, DEFAULT_ALIGN,
				  &stVbCalConfig);

	stVideoFrame.stVFrame.enCompressMode = COMPRESS_MODE_NONE;
	stVideoFrame.stVFrame.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	stVideoFrame.stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
	stVideoFrame.stVFrame.u32Width = stSize.u32Width;
	stVideoFrame.stVFrame.u32Height = stSize.u32Height;
	stVideoFrame.stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
	stVideoFrame.stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
	stVideoFrame.stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
	stVideoFrame.stVFrame.u32TimeRef = 0;
	stVideoFrame.stVFrame.u64PTS = 0;
	stVideoFrame.stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;

	blk_out = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCalConfig.u32VBSize);
	if (blk_out == VB_INVALID_HANDLE) {
		CVI_VB_ReleaseBlock(blk_in);
		goto GET_VB_OUT_FAIL;
	}

	stVideoFrame.u32PoolId = CVI_VB_Handle2PoolId(blk_out);
	stVideoFrame.stVFrame.u32Length[0] = stVbCalConfig.u32MainYSize;
	stVideoFrame.stVFrame.u32Length[1] =
		stVideoFrame.stVFrame.u32Length[2] = stVbCalConfig.u32MainCSize;
	stVideoFrame.stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk_out);
	stVideoFrame.stVFrame.u64PhyAddr[1] =
		stVideoFrame.stVFrame.u64PhyAddr[0] +
		ALIGN(stVbCalConfig.u32MainYSize, stVbCalConfig.u16AddrAlign);
	stVideoFrame.stVFrame.u64PhyAddr[2] =
		stVideoFrame.stVFrame.u64PhyAddr[1] +
		ALIGN(stVbCalConfig.u32MainCSize, stVbCalConfig.u16AddrAlign);

	SAMPLE_PRT("phy addr(%#" PRIx64 ", %#" PRIx64 ", %#" PRIx64 "\n",
		   stVideoFrame.stVFrame.u64PhyAddr[0],
		   stVideoFrame.stVFrame.u64PhyAddr[1],
		   stVideoFrame.stVFrame.u64PhyAddr[2]);

	printf("%d %d %d\n", stVideoFrame.stVFrame.u32Length[0],
	       stVideoFrame.stVFrame.u32Length[1],
	       stVideoFrame.stVFrame.u32Length[2]);

	stTask.stImgOut = stVideoFrame;

	struct timeval t0, t1;

	gettimeofday(&t0, NULL);

	/************************************************
	 * step4:  Setup GDC.
	 ************************************************/
	CVI_GDC_BeginJob(&hHandle);
	// if (method) {
	// 	CVI_GDC_AddCnvWarpTask(_pfmesh_data, hHandle, &stTask, &stAffineAttr, &_renew);
	// } else {
	// 	CVI_GDC_AddCorrectionTaskCNV(hHandle, &stTask, &stAffineAttr, p_tbl, p_idl, &tbl_param[0]);
	// }
	if (CVI_GDC_EndJob(hHandle) != CVI_SUCCESS) {
		SAMPLE_PRT("GDC Job failed.\n");
		goto GDC_FAIL;
	}

	gettimeofday(&t1, NULL);
	unsigned long elapsed_dwa =
		(t1.tv_sec - t0.tv_sec) * 1000000 + t1.tv_usec - t0.tv_usec;
	printf("cnv warp speed: %lu\n", elapsed_dwa);

	/************************************************
	 * step5:  Write result to file.
	 ************************************************/
	SAMPLE_COMM_FRAME_SaveToFile("cnv_warp.yuv", &stTask.stImgOut);

GDC_FAIL:
	CVI_VB_ReleaseBlock(blk_out);
GET_VB_OUT_FAIL:
READ_FAIL:
	fclose(fp);
ROPEN_FAIL:
	CVI_SYS_Munmap(stTask.stImgIn.stVFrame.pu8VirAddr[0],
		       stTask.stImgIn.stVFrame.u32Length[0]);
	CVI_SYS_Munmap(stTask.stImgIn.stVFrame.pu8VirAddr[1],
		       stTask.stImgIn.stVFrame.u32Length[1]);
	CVI_SYS_Munmap(stTask.stImgIn.stVFrame.pu8VirAddr[2],
		       stTask.stImgIn.stVFrame.u32Length[2]);
	CVI_VB_ReleaseBlock(blk_in);

	free(_pfmesh_data);
	free(p_idl);
	free(p_tbl);
}

static CVI_S32 _handle_op(CVI_S32 op, CVI_VOID *param1, CVI_VOID *param2)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	switch (op) {
	case 1: {
		_vi_get_chn_frmae_and_send_vpss(param1, param2);
		break;
	}

	case 2: {
		_vpss_stress_test();
		break;
	}

	case 3: {
		_affine_test();
		break;
	}

	case 4: {
		_pyramid_test();
		break;
	}

	case 5: {
		_normalize_test();
		break;
	}

	case 6: {
		printf("run cnv warp\n");
		_cnv_warp_test(true);
		printf("finished\n");
		break;
	}

	case 7: {
		printf("run cnv warp mesh tbl\n");
		_cnv_warp_test(false);
		printf("finished\n");
		break;
	}

	case 200: {
		LOG_LEVEL_CONF_S log_conf;
		CVI_CHAR buffer[20];
		CVI_S32 tmp;

		for (CVI_U8 i = 0; i < CVI_ID_BUTT; ++i) {
			if ((i % 3) == 0)
				printf("\n");
			log_conf.enModId = i;
			CVI_LOG_GetLevelConf(&log_conf);
			snprintf(buffer, sizeof(buffer), "%s(%d): %d",
				 CVI_SYS_GetModName(i), i, log_conf.s32Level);
			printf("%-19s", buffer);
		}
		printf("\n");
		printf("ModID(111 for all):\n");
		scanf("%d", &tmp);
		printf("Log level(0~7):\n");
		scanf("%d", &log_conf.s32Level);
		log_conf.enModId = tmp;
		if (log_conf.enModId != 111)
			CVI_LOG_SetLevelConf(&log_conf);
		else {
			for (CVI_U8 i = 0; i < CVI_ID_BUTT; ++i) {
				log_conf.enModId = i;
				CVI_LOG_SetLevelConf(&log_conf);
			}
		}
		break;
	}
	case 201: {
		LOG_LEVEL_CONF_S log_conf;
		CVI_CHAR buffer[20];

		for (CVI_U8 i = 0; i < CVI_ID_BUTT; ++i) {
			if ((i % 3) == 0)
				printf("\n");
			log_conf.enModId = i;
			CVI_LOG_GetLevelConf(&log_conf);
			snprintf(buffer, sizeof(buffer), "%s(%d): %d",
				 CVI_SYS_GetModName(i), i, log_conf.s32Level);
			printf("%-19s", buffer);
		}
		printf("\n");
		break;
	}
	default:
		break;
	}

	return s32Ret;
}

int main(void)
{
	SAMPLE_SNS_TYPE_E enSnsType = SENSOR0_TYPE;
	WDR_MODE_E enWDRMode = WDR_MODE_NONE;
	DYNAMIC_RANGE_E enDynamicRange = DYNAMIC_RANGE_SDR8;
	PIXEL_FORMAT_E enPixFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	VIDEO_FORMAT_E enVideoFormat = VIDEO_FORMAT_LINEAR;
	COMPRESS_MODE_E enCompressMode = COMPRESS_MODE_NONE;
	VI_VPSS_MODE_E enMastPipeMode = VI_OFFLINE_VPSS_OFFLINE;

	VB_CONFIG_S stVbConf;
	PIC_SIZE_E enPicSize;
	CVI_U32 u32BlkSize;
	SIZE_S stSize;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int op;

	VI_DEV ViDev = 0;
	VI_PIPE ViPipe = 0;
	VI_CHN ViChn = 0;
	CVI_S32 s32WorkSnsId = 0;
	SAMPLE_VI_CONFIG_S stViConfig;

	/************************************************
	 * step1:  Config VI
	 ************************************************/
	SAMPLE_COMM_VI_GetSensorInfo(&stViConfig);

	stViConfig.astViInfo[s32WorkSnsId].stSnsInfo.enSnsType = enSnsType;
	stViConfig.s32WorkingViNum = 1;
	stViConfig.as32WorkingViId[0] = 0;
	stViConfig.astViInfo[s32WorkSnsId].stSnsInfo.MipiDev = 0xFF;
	stViConfig.astViInfo[s32WorkSnsId].stSnsInfo.s32BusId = 3;
	stViConfig.astViInfo[s32WorkSnsId].stDevInfo.ViDev = ViDev;
	stViConfig.astViInfo[s32WorkSnsId].stDevInfo.enWDRMode = enWDRMode;
	stViConfig.astViInfo[s32WorkSnsId].stPipeInfo.enMastPipeMode =
		enMastPipeMode;
	stViConfig.astViInfo[s32WorkSnsId].stPipeInfo.aPipe[0] = ViPipe;
	stViConfig.astViInfo[s32WorkSnsId].stPipeInfo.aPipe[1] = -1;
	stViConfig.astViInfo[s32WorkSnsId].stPipeInfo.aPipe[2] = -1;
	stViConfig.astViInfo[s32WorkSnsId].stPipeInfo.aPipe[3] = -1;
	stViConfig.astViInfo[s32WorkSnsId].stChnInfo.ViChn = ViChn;
	stViConfig.astViInfo[s32WorkSnsId].stChnInfo.enPixFormat = enPixFormat;
	stViConfig.astViInfo[s32WorkSnsId].stChnInfo.enDynamicRange =
		enDynamicRange;
	stViConfig.astViInfo[s32WorkSnsId].stChnInfo.enVideoFormat =
		enVideoFormat;
	stViConfig.astViInfo[s32WorkSnsId].stChnInfo.enCompressMode =
		enCompressMode;

	/************************************************
	 * step2:  Get  input size
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(enSnsType, &enPicSize);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VI_GetSizeBySensor failed with %#x\n",
			   s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed with %#x\n",
			   s32Ret);
		return s32Ret;
	}

	/************************************************
	 * step3:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt = 2;

	u32BlkSize =
		COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height,
					SAMPLE_PIXEL_FORMAT, DATA_BITWIDTH_8,
					enCompressMode, DEFAULT_ALIGN);
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt = 6;
	SAMPLE_PRT("common pool[0] BlkSize %d\n", u32BlkSize);

	u32BlkSize =
		COMMON_GetPicBufferSize(stSize.u32Height, stSize.u32Width,
					PIXEL_FORMAT_RGB_888, DATA_BITWIDTH_8,
					enCompressMode, DEFAULT_ALIGN);
	stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[1].u32BlkCnt = 4;
	SAMPLE_PRT("common pool[1] BlkSize %d\n", u32BlkSize);

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("system init failed with %#x\n", s32Ret);
		return -1;
	}

	system("stty erase ^H");

	do {
		SAMPLE_PRT("1: VPSS send & get frame for fd&fr.\n");
		SAMPLE_PRT("2: VPSS fd stress test.\n");
		SAMPLE_PRT("3: Affine test.\n");
		SAMPLE_PRT("4: image pyramid test.\n");
		SAMPLE_PRT("5: normalize test.\n");
		SAMPLE_PRT("6: cvi cnv dwa test.\n");
		SAMPLE_PRT("7: cvi cnv dwa test with mesh tbl.\n");
		SAMPLE_PRT("200:set log level 201: get log level\n");
		SAMPLE_PRT("202:log to file\n");
		SAMPLE_PRT("255: exit\n");
		scanf("%d", &op);

		if (op == 1)
			_handle_op(op, &stViConfig, &stSize);
		else
			_handle_op(op, NULL, NULL);
	} while (op != 255);

	_vpss_stop(0);

	SAMPLE_COMM_VI_DestroyIsp(&stViConfig);

	SAMPLE_COMM_VI_DestroyVi(&stViConfig);

	SAMPLE_COMM_SYS_Exit();
}

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


#define SAMPLE_VPSS_DEFAULT_FILE_IN      "res/1080p.yuv420"

#define SAMPLE_VPSS_PRT(fmt...) \
	do { \
		printf("[%s]-%d: ", __func__, __LINE__); \
		printf(fmt); \
	} while (0)

CVI_CHAR * GetFileSuffix(PIXEL_FORMAT_E enPixFmt)
{
	switch (enPixFmt)
	{
	case PIXEL_FORMAT_RGB_888:
		return "rgb";
	case PIXEL_FORMAT_BGR_888:
		return "bgr";
	case PIXEL_FORMAT_RGB_888_PLANAR:
		return "rgbm";
	case PIXEL_FORMAT_BGR_888_PLANAR:
		return "bgrm";
	case PIXEL_FORMAT_YUV_PLANAR_422:
		return "422";
	case PIXEL_FORMAT_YUV_PLANAR_420:
		return "420";
	case PIXEL_FORMAT_YUV_PLANAR_444:
		return "444";
	case PIXEL_FORMAT_YUV_400:
		return "y";
	case PIXEL_FORMAT_HSV_888:
		return "hsv";
	case PIXEL_FORMAT_HSV_888_PLANAR:
		return "hsvm";
	case PIXEL_FORMAT_NV12:
		return "nv12";
	case PIXEL_FORMAT_NV21:
		return "nv21";
	case PIXEL_FORMAT_NV16:
		return "nv16";
	case PIXEL_FORMAT_NV61:
		return "61";
	case PIXEL_FORMAT_YUYV:
		return "yuyv";
	case PIXEL_FORMAT_UYVY:
		return "uyvy";
	case PIXEL_FORMAT_YVYU:
		return "yvyu";
	case PIXEL_FORMAT_VYUY:
		return "vyuy";
	case PIXEL_FORMAT_FP32_C3_PLANAR:
		return "fp32";
	case PIXEL_FORMAT_FP16_C3_PLANAR:
		return "fp16";
	case PIXEL_FORMAT_BF16_C3_PLANAR:
		return "bf16";
	case PIXEL_FORMAT_INT8_C3_PLANAR:
		return "int8";
	case PIXEL_FORMAT_UINT8_C3_PLANAR:
		return "uint8";

	default:
		return "unknown";
	}
}

CVI_S32 SAMPLE_VPSS_FileToFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat,
		CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	VB_BLK blk;
	CVI_U32 u32len;
	VB_CAL_CONFIG_S stVbCalConfig;
	FILE *fp;

	COMMON_GetPicBufferConfig(stSize->u32Width, stSize->u32Height, enPixelFormat, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	memset(&stVideoFrame, 0, sizeof(stVideoFrame));
	stVideoFrame.stVFrame.enCompressMode = COMPRESS_MODE_NONE;
	stVideoFrame.stVFrame.enPixelFormat = enPixelFormat;
	stVideoFrame.stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
	stVideoFrame.stVFrame.enColorGamut = COLOR_GAMUT_BT709;
	stVideoFrame.stVFrame.u32Width = stSize->u32Width;
	stVideoFrame.stVFrame.u32Height = stSize->u32Height;
	stVideoFrame.stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
	stVideoFrame.stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
	stVideoFrame.stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
	stVideoFrame.stVFrame.u32TimeRef = 0;
	stVideoFrame.stVFrame.u64PTS = 0;
	stVideoFrame.stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;

	blk = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCalConfig.u32VBSize);
	if (blk == VB_INVALID_HANDLE) {
		SAMPLE_VPSS_PRT("CVI_VB_GetBlock fail\n");
		return CVI_FAILURE;
	}

	//open data file & fread into the mmap address
	fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		SAMPLE_VPSS_PRT("open data file error\n");
		CVI_VB_ReleaseBlock(blk);
		return CVI_FAILURE;
	}

	stVideoFrame.u32PoolId = CVI_VB_Handle2PoolId(blk);
	stVideoFrame.stVFrame.u32Length[0] = stVbCalConfig.u32MainYSize;
	stVideoFrame.stVFrame.u32Length[1] = stVbCalConfig.u32MainCSize;
	stVideoFrame.stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
	stVideoFrame.stVFrame.u64PhyAddr[1] = stVideoFrame.stVFrame.u64PhyAddr[0]
		+ ALIGN(stVbCalConfig.u32MainYSize, stVbCalConfig.u16AddrAlign);
	if (stVbCalConfig.plane_num == 3) {
		stVideoFrame.stVFrame.u32Length[2] = stVbCalConfig.u32MainCSize;
		stVideoFrame.stVFrame.u64PhyAddr[2] = stVideoFrame.stVFrame.u64PhyAddr[1]
			+ ALIGN(stVbCalConfig.u32MainCSize, stVbCalConfig.u16AddrAlign);
	}

	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		if (stVideoFrame.stVFrame.u32Length[i] == 0)
			continue;
		stVideoFrame.stVFrame.pu8VirAddr[i]
			= CVI_SYS_MmapCache(stVideoFrame.stVFrame.u64PhyAddr[i], stVideoFrame.stVFrame.u32Length[i]);

		u32len = fread(stVideoFrame.stVFrame.pu8VirAddr[i], stVideoFrame.stVFrame.u32Length[i], 1, fp);
		if (u32len <= 0) {
			SAMPLE_VPSS_PRT("vpss send frame: fread plane%d error\n", i);
			fclose(fp);
			CVI_VB_ReleaseBlock(blk);
			return CVI_FAILURE;
		}
		CVI_SYS_IonInvalidateCache(stVideoFrame.stVFrame.u64PhyAddr[i],
					   stVideoFrame.stVFrame.pu8VirAddr[i],
					   stVideoFrame.stVFrame.u32Length[i]);
	}

	SAMPLE_VPSS_PRT("length of buffer(%d, %d, %d)\n", stVideoFrame.stVFrame.u32Length[0]
		, stVideoFrame.stVFrame.u32Length[1], stVideoFrame.stVFrame.u32Length[2]);
	SAMPLE_VPSS_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", stVideoFrame.stVFrame.u64PhyAddr[0]
		, stVideoFrame.stVFrame.u64PhyAddr[1], stVideoFrame.stVFrame.u64PhyAddr[2]);
	SAMPLE_VPSS_PRT("vir addr(%p, %p, %p)\n", stVideoFrame.stVFrame.pu8VirAddr[0]
		, stVideoFrame.stVFrame.pu8VirAddr[1], stVideoFrame.stVFrame.pu8VirAddr[2]);

	fclose(fp);

	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		if (stVideoFrame.stVFrame.u32Length[i] == 0)
			continue;
		CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[i], stVideoFrame.stVFrame.u32Length[i]);
	}
	memcpy(pstVideoFrame, &stVideoFrame, sizeof(stVideoFrame));

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_VPSS_FrameSaveToFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	FILE *fp;
	CVI_U32 u32len, u32DataLen;

	fp = fopen(filename, "w");
	if (fp == CVI_NULL) {
		SAMPLE_VPSS_PRT("open data file(%s) error\n", filename);
		return CVI_FAILURE;
	}

	for (int i = 0; i < 3; ++i) {
		u32DataLen = pstVideoFrame->stVFrame.u32Stride[i] * pstVideoFrame->stVFrame.u32Height;
		if (u32DataLen == 0)
			continue;
		if (i > 0 && ((pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
			(pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) ||
			(pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)))
			u32DataLen >>= 1;

		pstVideoFrame->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);

		SAMPLE_VPSS_PRT("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d)\n",
			   i, pstVideoFrame->stVFrame.u64PhyAddr[i],
			   pstVideoFrame->stVFrame.pu8VirAddr[i],
			   pstVideoFrame->stVFrame.u32Stride[i]);
		SAMPLE_VPSS_PRT(" data_len(%d) plane_len(%d)\n",
			      u32DataLen, pstVideoFrame->stVFrame.u32Length[i]);
		u32len = fwrite(pstVideoFrame->stVFrame.pu8VirAddr[i], u32DataLen, 1, fp);
		if (u32len <= 0) {
			SAMPLE_VPSS_PRT("fwrite data(%d) error\n", i);
			s32Ret = CVI_FAILURE;
			break;
		}
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}

	fclose(fp);
	return s32Ret;
}

CVI_S32 SAMPLE_VPSS_Simple(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	VIDEO_FRAME_INFO_S stVideoFrameIn, stVideoFrameOut;
	SIZE_S stSize = {1920, 1080};
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	CVI_CHAR *pFileNameIn = SAMPLE_VPSS_DEFAULT_FILE_IN;
	CVI_CHAR aszFileNameOut[64];

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

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	VPSS_GRP           VpssGrp        = 0;
	VPSS_CHN           VpssChn        = VPSS_CHN0;
	VPSS_GRP_ATTR_S    stVpssGrpAttr  = {0};
	VPSS_CHN_ATTR_S    stVpssChnAttr  = {0};

	stVpssGrpAttr.u32MaxW                        = stSize.u32Width;
	stVpssGrpAttr.u32MaxH                        = stSize.u32Height;
	stVpssGrpAttr.enPixelFormat                  = enPixelFormat;
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;

    stVpssChnAttr.u32Width                      = stSize.u32Width;
    stVpssChnAttr.u32Height                     = stSize.u32Height;
    stVpssChnAttr.enVideoFormat                 = VIDEO_FORMAT_LINEAR;
    stVpssChnAttr.enPixelFormat                 = enPixelFormat;
    stVpssChnAttr.stFrameRate.s32SrcFrameRate   = -1;
    stVpssChnAttr.stFrameRate.s32DstFrameRate   = -1;
    stVpssChnAttr.u32Depth                      = 1;
    stVpssChnAttr.bMirror                       = CVI_FALSE;
    stVpssChnAttr.bFlip                         = CVI_FALSE;
    stVpssChnAttr.stAspectRatio.enMode          = ASPECT_RATIO_NONE;
    stVpssChnAttr.stNormalize.bEnable           = CVI_FALSE;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	/*send frame*/
	s32Ret = SAMPLE_VPSS_FileToFrame(&stSize, enPixelFormat, pFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("SAMPLE_VPSS_FileToFrame failed. s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_SendFrame fail. s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
	}

	/*get frame*/
	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, 1000);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
	}

	snprintf(aszFileNameOut, 64, "%s_%d_%d_%s.bin",
		__func__,
		stVideoFrameOut.stVFrame.u32Width,
		stVideoFrameOut.stVFrame.u32Height,
		GetFileSuffix(stVideoFrameOut.stVFrame.enPixelFormat));

	s32Ret = SAMPLE_VPSS_FrameSaveToFile(aszFileNameOut, &stVideoFrameOut);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("SAMPLE_VPSS_FrameSaveToFile fail. s32Ret: 0x%x !\n", s32Ret);
	}

	s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_ReleaseChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
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

CVI_S32 SAMPLE_VPSS_MultiChn(CVI_VOID)
{
	CVI_S32 i, s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	VIDEO_FRAME_INFO_S stVideoFrameIn, stVideoFrameOut;
	SIZE_S stSizeIn = {1920, 1080};
	PIXEL_FORMAT_E enPixelFormatIn = PIXEL_FORMAT_YUV_PLANAR_420;
	SIZE_S astSizeOut[VPSS_MAX_CHN_NUM] =
		{{1920, 1080}, {1280, 720}, {640, 360}, {640, 360}};
	PIXEL_FORMAT_E enPixelFormatOut[VPSS_MAX_CHN_NUM] =
		{PIXEL_FORMAT_YUV_PLANAR_420, PIXEL_FORMAT_NV21,
		PIXEL_FORMAT_YUV_400, PIXEL_FORMAT_RGB_888_PLANAR};
	CVI_CHAR *pFileNameIn = SAMPLE_VPSS_DEFAULT_FILE_IN;
	CVI_CHAR aszFileNameOut[64];

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSize = COMMON_GetPicBufferSize(stSizeIn.u32Width, stSizeIn.u32Height,
		enPixelFormatIn, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	//vb in
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 1;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.u32MaxPoolCnt = 1;
	//vb out
	for (i = 0; i < VPSS_MAX_CHN_NUM; i++) {
		u32BlkSize = COMMON_GetPicBufferSize(astSizeOut[i].u32Width, astSizeOut[i].u32Height,
			enPixelFormatOut[i], DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		stVbConf.astCommPool[1 + i].u32BlkSize	= u32BlkSize;
		stVbConf.astCommPool[1 + i].u32BlkCnt	= 1;
		stVbConf.astCommPool[1 + i].enRemapMode	= VB_REMAP_MODE_CACHED;
		stVbConf.u32MaxPoolCnt++;
	}

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	VPSS_GRP           VpssGrp        = 0;
	VPSS_CHN           VpssChn        = VPSS_CHN0;
	VPSS_GRP_ATTR_S    stVpssGrpAttr  = {0};
	VPSS_CHN_ATTR_S    stVpssChnAttr  = {0};

	stVpssGrpAttr.u32MaxW                        = stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH                        = stSizeIn.u32Height;
	stVpssGrpAttr.enPixelFormat                  = enPixelFormatIn;
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	for (i = 0; i < VPSS_MAX_CHN_NUM; i++) {
		VpssChn = i;
	    stVpssChnAttr.u32Width                      = astSizeOut[i].u32Width;
	    stVpssChnAttr.u32Height                     = astSizeOut[i].u32Height;
	    stVpssChnAttr.enVideoFormat                 = VIDEO_FORMAT_LINEAR;
	    stVpssChnAttr.enPixelFormat                 = enPixelFormatOut[i];
	    stVpssChnAttr.stFrameRate.s32SrcFrameRate   = -1;
	    stVpssChnAttr.stFrameRate.s32DstFrameRate   = -1;
	    stVpssChnAttr.u32Depth                      = 1;
	    stVpssChnAttr.bMirror                       = CVI_FALSE;
	    stVpssChnAttr.bFlip                         = CVI_FALSE;
	    stVpssChnAttr.stAspectRatio.enMode          = ASPECT_RATIO_NONE;
	    stVpssChnAttr.stNormalize.bEnable           = CVI_FALSE;

		s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_VPSS_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
			goto exit2;
		}

		s32Ret = CVI_VPSS_AttachVbPool(VpssGrp, VpssChn, 1 + i);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_VPSS_PRT("CVI_VPSS_AttachVbPool failed with %#x\n", s32Ret);
			goto exit2;
		}

		s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_VPSS_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
			goto exit2;
		}
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	/*send frame*/
	s32Ret = SAMPLE_VPSS_FileToFrame(&stSizeIn, enPixelFormatIn, pFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("SAMPLE_VPSS_FileToFrame failed. s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_SendFrame fail. s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
	}

	/*get frame*/
	for (i = 0; i < VPSS_MAX_CHN_NUM; i++) {
		VpssChn = i;

		s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, 1000);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_VPSS_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
			goto exit5;
		}

		snprintf(aszFileNameOut, 64, "%s_%d_%d_%s.bin",
			__func__,
			stVideoFrameOut.stVFrame.u32Width,
			stVideoFrameOut.stVFrame.u32Height,
			GetFileSuffix(stVideoFrameOut.stVFrame.enPixelFormat));

		s32Ret = SAMPLE_VPSS_FrameSaveToFile(aszFileNameOut, &stVideoFrameOut);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_VPSS_PRT("SAMPLE_VPSS_FrameSaveToFile fail. s32Ret: 0x%x !\n", s32Ret);
		}

		s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_VPSS_PRT("CVI_VPSS_ReleaseChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
		}
	}

exit5:
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrameIn.stVFrame.u64PhyAddr[0]));
exit4:
	CVI_VPSS_StopGrp(VpssGrp);
exit3:
	for (i = 0; i < VPSS_MAX_CHN_NUM; i++)
		CVI_VPSS_DisableChn(VpssGrp, i);
exit2:
	CVI_VPSS_DestroyGrp(VpssGrp);
exit1:
	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();

	return s32Ret;
}

CVI_S32 SAMPLE_VPSS_ChnCrop(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn, u32BlkSizeOut;
	VIDEO_FRAME_INFO_S stVideoFrameIn, stVideoFrameOut;
	SIZE_S stSizeIn = {1920, 1080};
	SIZE_S stSizeOut = {640, 360};
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	CVI_CHAR *pFileNameIn = SAMPLE_VPSS_DEFAULT_FILE_IN;
	CVI_CHAR aszFileNameOut[64];

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSizeIn = COMMON_GetPicBufferSize(stSizeIn.u32Width, stSizeIn.u32Height,
		enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSizeOut = COMMON_GetPicBufferSize(stSizeOut.u32Width, stSizeOut.u32Height,
		enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.u32MaxPoolCnt              = 2;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn;
	stVbConf.astCommPool[0].u32BlkCnt	= 1;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
	stVbConf.astCommPool[1].u32BlkCnt	= 1;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	VPSS_GRP           VpssGrp        = 0;
	VPSS_CHN           VpssChn        = VPSS_CHN0;
	VPSS_GRP_ATTR_S    stVpssGrpAttr  = {0};
	VPSS_CHN_ATTR_S    stVpssChnAttr  = {0};

	stVpssGrpAttr.u32MaxW                        = stSizeIn.u32Width;
	stVpssGrpAttr.u32MaxH                        = stSizeIn.u32Height;
	stVpssGrpAttr.enPixelFormat                  = enPixelFormat;
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;

    stVpssChnAttr.u32Width                      = stSizeOut.u32Width;
    stVpssChnAttr.u32Height                     = stSizeOut.u32Height;
    stVpssChnAttr.enVideoFormat                 = VIDEO_FORMAT_LINEAR;
    stVpssChnAttr.enPixelFormat                 = enPixelFormat;
    stVpssChnAttr.stFrameRate.s32SrcFrameRate   = -1;
    stVpssChnAttr.stFrameRate.s32DstFrameRate   = -1;
    stVpssChnAttr.u32Depth                      = 1;
    stVpssChnAttr.bMirror                       = CVI_FALSE;
    stVpssChnAttr.bFlip                         = CVI_FALSE;
    stVpssChnAttr.stAspectRatio.enMode          = ASPECT_RATIO_NONE;
    stVpssChnAttr.stNormalize.bEnable           = CVI_FALSE;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	//chn crop
	VPSS_CROP_INFO_S stCropInfo;

	stCropInfo.bEnable = CVI_TRUE;
	stCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
	stCropInfo.stCropRect.s32X = 20;
	stCropInfo.stCropRect.s32Y = 30;
	stCropInfo.stCropRect.u32Width = 640;
	stCropInfo.stCropRect.u32Height = 360;
	s32Ret = CVI_VPSS_SetChnCrop(VpssGrp, VpssChn, &stCropInfo);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_SetChnCrop failed with %#x\n", s32Ret);
		goto exit4;
	}

	/*send frame*/
	s32Ret = SAMPLE_VPSS_FileToFrame(&stSizeIn, enPixelFormat, pFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("SAMPLE_VPSS_FileToFrame failed. s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_SendFrame fail. s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
	}

	/*get frame*/
	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, 1000);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
	}

	snprintf(aszFileNameOut, 64, "%s_%d_%d_%s.bin",
		__func__,
		stVideoFrameOut.stVFrame.u32Width,
		stVideoFrameOut.stVFrame.u32Height,
		GetFileSuffix(stVideoFrameOut.stVFrame.enPixelFormat));

	s32Ret = SAMPLE_VPSS_FrameSaveToFile(aszFileNameOut, &stVideoFrameOut);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("SAMPLE_VPSS_FrameSaveToFile fail. s32Ret: 0x%x !\n", s32Ret);
	}

	s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_ReleaseChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
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

CVI_S32 SAMPLE_VPSS_AspectRatio(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	VIDEO_FRAME_INFO_S stVideoFrameIn, stVideoFrameOut;
	SIZE_S stSize = {1920, 1080};
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	CVI_CHAR *pFileNameIn = SAMPLE_VPSS_DEFAULT_FILE_IN;
	CVI_CHAR aszFileNameOut[64];

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

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	VPSS_GRP           VpssGrp        = 0;
	VPSS_CHN           VpssChn        = VPSS_CHN0;
	VPSS_GRP_ATTR_S    stVpssGrpAttr  = {0};
	VPSS_CHN_ATTR_S    stVpssChnAttr  = {0};

	stVpssGrpAttr.u32MaxW                        = stSize.u32Width;
	stVpssGrpAttr.u32MaxH                        = stSize.u32Height;
	stVpssGrpAttr.enPixelFormat                  = enPixelFormat;
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;

    stVpssChnAttr.u32Width                      = stSize.u32Width;
    stVpssChnAttr.u32Height                     = stSize.u32Height;
    stVpssChnAttr.enVideoFormat                 = VIDEO_FORMAT_LINEAR;
    stVpssChnAttr.enPixelFormat                 = enPixelFormat;
    stVpssChnAttr.stFrameRate.s32SrcFrameRate   = -1;
    stVpssChnAttr.stFrameRate.s32DstFrameRate   = -1;
    stVpssChnAttr.u32Depth                      = 1;
    stVpssChnAttr.bMirror                       = CVI_FALSE;
    stVpssChnAttr.bFlip                         = CVI_FALSE;
    stVpssChnAttr.stNormalize.bEnable           = CVI_FALSE;
    stVpssChnAttr.stAspectRatio.enMode          = ASPECT_RATIO_MANUAL;
	stVpssChnAttr.stAspectRatio.bEnableBgColor  = CVI_TRUE;
	stVpssChnAttr.stAspectRatio.u32BgColor      = 0x0;
	stVpssChnAttr.stAspectRatio.stVideoRect.s32X = 100;
	stVpssChnAttr.stAspectRatio.stVideoRect.s32Y = 60;
	stVpssChnAttr.stAspectRatio.stVideoRect.u32Width = 1280;
	stVpssChnAttr.stAspectRatio.stVideoRect.u32Height = 720;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	/*send frame*/
	s32Ret = SAMPLE_VPSS_FileToFrame(&stSize, enPixelFormat, pFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("SAMPLE_VPSS_FileToFrame failed. s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_SendFrame fail. s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
	}

	/*get frame*/
	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, 1000);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
	}

	snprintf(aszFileNameOut, 64, "%s_%d_%d_%s.bin",
		__func__,
		stVideoFrameOut.stVFrame.u32Width,
		stVideoFrameOut.stVFrame.u32Height,
		GetFileSuffix(stVideoFrameOut.stVFrame.enPixelFormat));

	s32Ret = SAMPLE_VPSS_FrameSaveToFile(aszFileNameOut, &stVideoFrameOut);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("SAMPLE_VPSS_FrameSaveToFile fail. s32Ret: 0x%x !\n", s32Ret);
	}

	s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_ReleaseChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
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

CVI_S32 SAMPLE_VPSS_DrawRect(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize;
	VIDEO_FRAME_INFO_S stVideoFrameIn, stVideoFrameOut;
	SIZE_S stSize = {1920, 1080};
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	CVI_CHAR *pFileNameIn = SAMPLE_VPSS_DEFAULT_FILE_IN;
	CVI_CHAR aszFileNameOut[64];

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

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	VPSS_GRP           VpssGrp        = 0;
	VPSS_CHN           VpssChn        = VPSS_CHN0;
	VPSS_GRP_ATTR_S    stVpssGrpAttr  = {0};
	VPSS_CHN_ATTR_S    stVpssChnAttr  = {0};

	stVpssGrpAttr.u32MaxW                        = stSize.u32Width;
	stVpssGrpAttr.u32MaxH                        = stSize.u32Height;
	stVpssGrpAttr.enPixelFormat                  = enPixelFormat;
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;

    stVpssChnAttr.u32Width                      = stSize.u32Width;
    stVpssChnAttr.u32Height                     = stSize.u32Height;
    stVpssChnAttr.enVideoFormat                 = VIDEO_FORMAT_LINEAR;
    stVpssChnAttr.enPixelFormat                 = enPixelFormat;
    stVpssChnAttr.stFrameRate.s32SrcFrameRate   = -1;
    stVpssChnAttr.stFrameRate.s32DstFrameRate   = -1;
    stVpssChnAttr.u32Depth                      = 1;
    stVpssChnAttr.bMirror                       = CVI_FALSE;
    stVpssChnAttr.bFlip                         = CVI_FALSE;
    stVpssChnAttr.stAspectRatio.enMode          = ASPECT_RATIO_NONE;
    stVpssChnAttr.stNormalize.bEnable           = CVI_FALSE;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		goto exit1;
	}

	s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
		goto exit2;
	}

	/*start vpss*/
	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		goto exit3;
	}

	/*draw rect*/
	VPSS_DRAW_RECT_S stDrawRect;

	stDrawRect.astRect[0].bEnable = CVI_TRUE;
	stDrawRect.astRect[0].u32BgColor = 0xffff;
	stDrawRect.astRect[0].u16Thick = 6;
	stDrawRect.astRect[0].stRect.s32X = 96;
	stDrawRect.astRect[0].stRect.s32Y = 96;
	stDrawRect.astRect[0].stRect.u32Width = 308;
	stDrawRect.astRect[0].stRect.u32Height = 208;
	for (int i = 1; i < VPSS_RECT_NUM; i++) {
		memcpy(&stDrawRect.astRect[i], &stDrawRect.astRect[0],
			sizeof(stDrawRect.astRect[0]));
		stDrawRect.astRect[i].stRect.s32X += i * 200;
		stDrawRect.astRect[i].stRect.s32Y += i * 64;
	}
	s32Ret = CVI_VPSS_SetChnDrawRect(VpssGrp, VpssChn, &stDrawRect);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_SetChnDrawRect failed with %#x\n", s32Ret);
		goto exit4;
	}

	/*send frame*/
	s32Ret = SAMPLE_VPSS_FileToFrame(&stSize, enPixelFormat, pFileNameIn, &stVideoFrameIn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("SAMPLE_VPSS_FileToFrame failed. s32Ret: 0x%x !\n", s32Ret);
		goto exit4;
	}

	s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrameIn, 1000);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_SendFrame fail. s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
	}

	/*get frame*/
	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrameOut, 1000);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
	}

	snprintf(aszFileNameOut, 64, "%s_%d_%d_%s.bin",
		__func__,
		stVideoFrameOut.stVFrame.u32Width,
		stVideoFrameOut.stVFrame.u32Height,
		GetFileSuffix(stVideoFrameOut.stVFrame.enPixelFormat));

	s32Ret = SAMPLE_VPSS_FrameSaveToFile(aszFileNameOut, &stVideoFrameOut);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("SAMPLE_VPSS_FrameSaveToFile fail. s32Ret: 0x%x !\n", s32Ret);
	}

	s32Ret = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrameOut);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_VPSS_PRT("CVI_VPSS_ReleaseChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
		goto exit5;
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

CVI_VOID SAMPLE_VPSS_HandleSig(CVI_S32 signo)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	if (SIGINT == signo || SIGTERM == signo) {
		//todo for release
		SAMPLE_VPSS_PRT("Program termination abnormally\n");
	}
	exit(-1);
}

CVI_VOID SAMPLE_VPSS_Usage(CVI_CHAR *sPrgNm)
{
	printf("Usage : %s <index>\n", sPrgNm);
	printf("index:\n");
	printf("\t 0)simple case.\n");
	printf("\t 1)multi chn.\n");
	printf("\t 2)chn crop.\n");
	printf("\t 3)aspect ratio.\n");
	printf("\t 4)draw rectangle.\n");
}

CVI_S32 main(CVI_S32 argc, CVI_CHAR *argv[])
{
	CVI_S32 s32Ret = CVI_FAILURE;
	CVI_S32 s32Index;

	if (argc < 2) {
		SAMPLE_VPSS_Usage(argv[0]);
		return CVI_FAILURE;
	}

	if (!strncmp(argv[1], "-h", 2)) {
		SAMPLE_VPSS_Usage(argv[0]);
		return CVI_SUCCESS;
	}

	signal(SIGINT, SAMPLE_VPSS_HandleSig);
	signal(SIGTERM, SAMPLE_VPSS_HandleSig);

	s32Index = atoi(argv[1]);
	switch (s32Index) {
	case 0:
		s32Ret = SAMPLE_VPSS_Simple();
		break;
	case 1:
		s32Ret = SAMPLE_VPSS_MultiChn();
		break;
	case 2:
		s32Ret = SAMPLE_VPSS_ChnCrop();
		break;
	case 3:
		s32Ret = SAMPLE_VPSS_AspectRatio();
		break;
	case 4:
		s32Ret = SAMPLE_VPSS_DrawRect();
		break;

	default:
		SAMPLE_VPSS_PRT("the index %d is invaild!\n", s32Index);
		SAMPLE_VPSS_Usage(argv[0]);
		return CVI_FAILURE;
	}

	if (s32Ret == CVI_SUCCESS)
		SAMPLE_VPSS_PRT("SAMPLE_VPSS exit success!\n");
	else
		SAMPLE_VPSS_PRT("SAMPLE_VPSS exit abnormally!\n");

	return s32Ret;
}


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>

#include "cvi_buffer.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vpss.h"
#include "vpss_ut_comm.h"
#include "md5sum.h"
#include "vpss_cmodel.h"


CVI_CHAR * GetFmtName(PIXEL_FORMAT_E enPixFmt)
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
		return "nv61";
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

static CVI_VOID get_chroma_size_shift_factor(PIXEL_FORMAT_E enPixelFormat,
		CVI_S32 *w_shift, CVI_S32 *h_shift, CVI_S32 *s32PixelSize, CVI_U32 *u32Planar)
{
	switch (enPixelFormat) {
	case PIXEL_FORMAT_YUV_PLANAR_420:
		*w_shift = 1;
		*h_shift = 1;
		*s32PixelSize = 1;
		*u32Planar = 3;
		break;
	case PIXEL_FORMAT_YUV_PLANAR_422:
		*w_shift = 1;
		*h_shift = 0;
		*s32PixelSize = 1;
		*u32Planar = 3;
		break;
	case PIXEL_FORMAT_YUV_PLANAR_444:
	case PIXEL_FORMAT_RGB_888_PLANAR:
	case PIXEL_FORMAT_BGR_888_PLANAR:
	case PIXEL_FORMAT_HSV_888_PLANAR:
	case PIXEL_FORMAT_INT8_C3_PLANAR:
	case PIXEL_FORMAT_UINT8_C3_PLANAR:
		*w_shift = 0;
		*h_shift = 0;
		*s32PixelSize = 1;
		*u32Planar = 3;
		break;
	case PIXEL_FORMAT_RGB_888:
	case PIXEL_FORMAT_BGR_888:
	case PIXEL_FORMAT_HSV_888:
		*w_shift = 31;
		*h_shift = 31;
		*s32PixelSize = 3;
		*u32Planar = 1;
		break;
	case PIXEL_FORMAT_NV12:
	case PIXEL_FORMAT_NV21:
		*w_shift = 0;
		*h_shift = 1;
		*s32PixelSize = 1;
		*u32Planar = 2;
		break;
	case PIXEL_FORMAT_NV16:
	case PIXEL_FORMAT_NV61:
		*w_shift = 0;
		*h_shift = 0;
		*s32PixelSize = 1;
		*u32Planar = 2;
		break;
	case PIXEL_FORMAT_YUYV:
	case PIXEL_FORMAT_UYVY:
	case PIXEL_FORMAT_YVYU:
	case PIXEL_FORMAT_VYUY:
		*w_shift = 31;
		*h_shift = 31;
		*s32PixelSize = 2;
		*u32Planar = 1;
		break;
	case PIXEL_FORMAT_YUV_400: // no chroma
		*w_shift = 31;
		*h_shift = 31;
		*s32PixelSize = 1;
		*u32Planar = 1;
		break;
	case PIXEL_FORMAT_FP32_C3_PLANAR:
		*w_shift = 0;
		*h_shift = 0;
		*s32PixelSize = 4;
		*u32Planar = 3;
		break;
	case PIXEL_FORMAT_FP16_C3_PLANAR:
	case PIXEL_FORMAT_BF16_C3_PLANAR:
		*w_shift = 0;
		*h_shift = 0;
		*s32PixelSize = 2;
		*u32Planar = 3;
		break;
	default:
		*w_shift = 31;
		*h_shift = 31;
		*s32PixelSize = 1;
		*u32Planar = 1;
		break;
	}
}


CVI_S32 FileSendToVpss(VPSS_GRP VpssGrp, const SIZE_S *stSize,
		PIXEL_FORMAT_E enPixelFormat, const CVI_CHAR *filename)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
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
		VPSS_UT_PRT("CVI_VB_GetBlock fail\n");
		return CVI_FAILURE;
	}

	//open data file & fread into the mmap address
	fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		VPSS_UT_PRT("open data file error\n");
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
			VPSS_UT_PRT("vpss send frame: fread plane%d error\n", i);
			fclose(fp);
			CVI_VB_ReleaseBlock(blk);
			return CVI_FAILURE;
		}
		CVI_SYS_IonFlushCache(stVideoFrame.stVFrame.u64PhyAddr[i],
					   stVideoFrame.stVFrame.pu8VirAddr[i],
					   stVideoFrame.stVFrame.u32Length[i]);
	}

	VPSS_UT_PRT("length of buffer(%d, %d, %d)\n", stVideoFrame.stVFrame.u32Length[0]
		, stVideoFrame.stVFrame.u32Length[1], stVideoFrame.stVFrame.u32Length[2]);
	VPSS_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", stVideoFrame.stVFrame.u64PhyAddr[0]
		, stVideoFrame.stVFrame.u64PhyAddr[1], stVideoFrame.stVFrame.u64PhyAddr[2]);
	VPSS_UT_PRT("vir addr(%p, %p, %p)\n", stVideoFrame.stVFrame.pu8VirAddr[0]
		, stVideoFrame.stVFrame.pu8VirAddr[1], stVideoFrame.stVFrame.pu8VirAddr[2]);

	fclose(fp);

	VPSS_UT_PRT("read file done and send vpss frame.\n");
	s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrame, 1000);
	if (s32Ret != CVI_SUCCESS)
		VPSS_UT_PRT("CVI_VPSS_SendFrame fail.\n");

	CVI_VB_ReleaseBlock(blk);

	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		if (stVideoFrame.stVFrame.u32Length[i] == 0)
			continue;
		CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[i], stVideoFrame.stVFrame.u32Length[i]);
	}
	return s32Ret;
}

CVI_S32 FbcFileSendToVpss(VPSS_GRP VpssGrp, const SIZE_S *stSize,
		PIXEL_FORMAT_E enPixelFormat, const CVI_CHAR filename[4][64], CVI_U32 fbctablelength)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VIDEO_FRAME_INFO_S stVideoFrame;
	VB_BLK blk;
	CVI_U32 u32len;
	VB_CAL_CONFIG_S stVbCalConfig;
	FILE *fp[4];

	COMMON_GetPicBufferConfig(stSize->u32Width, stSize->u32Height, enPixelFormat, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	stVbCalConfig.plane_num = 4;

	memset(&stVideoFrame, 0, sizeof(stVideoFrame));
	stVideoFrame.stVFrame.enCompressMode = COMPRESS_MODE_FRAME;
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

	blk = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCalConfig.u32VBSize * 2);
	if (blk == VB_INVALID_HANDLE) {
		VPSS_UT_PRT("CVI_VB_GetBlock fail\n");
		return CVI_FAILURE;
	}

	//open data file & fread into the mmap address
	for(int i = 0; i < 4; i++){
		fp[i] = fopen(filename[i], "r");
		if (fp[i] == CVI_NULL) {
			VPSS_UT_PRT("open data file error\n");
			CVI_VB_ReleaseBlock(blk);
			return CVI_FAILURE;
		}
	}

	stVideoFrame.u32PoolId = CVI_VB_Handle2PoolId(blk);
	stVideoFrame.stVFrame.u32Length[0] = fbctablelength;
	stVideoFrame.stVFrame.u32Length[1] = fbctablelength / 2;
	stVideoFrame.stVFrame.u32Length[2] = stVbCalConfig.u32MainYSize;
	stVideoFrame.stVFrame.u32ExtLength = stVbCalConfig.u32MainYSize / 2;
	stVideoFrame.stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
	stVideoFrame.stVFrame.u64PhyAddr[1] = stVideoFrame.stVFrame.u64PhyAddr[0]
		+ ALIGN(stVideoFrame.stVFrame.u32Length[0], stVbCalConfig.u16AddrAlign);
	stVideoFrame.stVFrame.u64PhyAddr[2] = stVideoFrame.stVFrame.u64PhyAddr[1]
		+ ALIGN(stVideoFrame.stVFrame.u32Length[1], stVbCalConfig.u16AddrAlign);
	stVideoFrame.stVFrame.u64ExtPhyAddr = stVideoFrame.stVFrame.u64PhyAddr[2]
		+ ALIGN(stVideoFrame.stVFrame.u32Length[2], stVbCalConfig.u16AddrAlign);

	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		if (i < 3) {
			if (stVideoFrame.stVFrame.u32Length[i] == 0)
				continue;
			stVideoFrame.stVFrame.pu8VirAddr[i]
				= CVI_SYS_MmapCache(stVideoFrame.stVFrame.u64PhyAddr[i], stVideoFrame.stVFrame.u32Length[i]);

			u32len = fread(stVideoFrame.stVFrame.pu8VirAddr[i], stVideoFrame.stVFrame.u32Length[i], 1, fp[i]);
			if (u32len <= 0) {
				VPSS_UT_PRT("vpss send frame: fread plane%d error\n", i);
				for(int j = 0; j < i; j++)
					fclose(fp[j]);
				CVI_VB_ReleaseBlock(blk);
				return CVI_FAILURE;
			}
			CVI_SYS_IonFlushCache(stVideoFrame.stVFrame.u64PhyAddr[i],
						stVideoFrame.stVFrame.pu8VirAddr[i],
						stVideoFrame.stVFrame.u32Length[i]);
		} else {
			if (stVideoFrame.stVFrame.u32ExtLength == 0)
				continue;
			stVideoFrame.stVFrame.pu8ExtVirtAddr
				= (CVI_U8 *)CVI_SYS_MmapCache(stVideoFrame.stVFrame.u64ExtPhyAddr, stVideoFrame.stVFrame.u32ExtLength);

			u32len = fread((void *)stVideoFrame.stVFrame.pu8ExtVirtAddr, stVideoFrame.stVFrame.u32ExtLength, 1, fp[i]);
			if (u32len <= 0) {
				VPSS_UT_PRT("vpss send frame: fread plane%d error\n", i);
				for(int j = 0; j < i; j++)
					fclose(fp[j]);
				CVI_VB_ReleaseBlock(blk);
				return CVI_FAILURE;
			}
			CVI_SYS_IonFlushCache(stVideoFrame.stVFrame.u64ExtPhyAddr,
						(CVI_VOID *)stVideoFrame.stVFrame.pu8ExtVirtAddr,
						stVideoFrame.stVFrame.u32ExtLength);
		}
	}

	VPSS_UT_PRT("length of buffer(%d, %d, %d, %d)\n", stVideoFrame.stVFrame.u32Length[0]
		, stVideoFrame.stVFrame.u32Length[1], stVideoFrame.stVFrame.u32Length[2], (CVI_U32)stVideoFrame.stVFrame.u32ExtLength);
	VPSS_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64", %#"PRIx64")\n", stVideoFrame.stVFrame.u64PhyAddr[0]
		, stVideoFrame.stVFrame.u64PhyAddr[1], stVideoFrame.stVFrame.u64PhyAddr[2], stVideoFrame.stVFrame.u64ExtPhyAddr);
	VPSS_UT_PRT("vir addr(%p, %p, %p, %p)\n", stVideoFrame.stVFrame.pu8VirAddr[0]
		, stVideoFrame.stVFrame.pu8VirAddr[1], stVideoFrame.stVFrame.pu8VirAddr[2], (CVI_U8 *)stVideoFrame.stVFrame.pu8ExtVirtAddr);
	for (int i = 0; i < 4; i++)
		fclose(fp[i]);

	VPSS_UT_PRT("read file done and send vpss frame.\n");
	s32Ret = CVI_VPSS_SendFrame(VpssGrp, &stVideoFrame, 1000);
	if (s32Ret != CVI_SUCCESS)
		VPSS_UT_PRT("CVI_VPSS_SendFrame fail.\n");

	CVI_VB_ReleaseBlock(blk);

	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		if (i < 3) {
			if (stVideoFrame.stVFrame.u32Length[i] == 0)
				continue;
			CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[i], stVideoFrame.stVFrame.u32Length[i]);
		} else {
			if (stVideoFrame.stVFrame.u32ExtLength == 0)
				continue;
			CVI_SYS_Munmap((void *)stVideoFrame.stVFrame.pu8ExtVirtAddr, stVideoFrame.stVFrame.u32ExtLength);
		}
	}
	return s32Ret;
}

CVI_S32 FileToFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat,
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
		VPSS_UT_PRT("CVI_VB_GetBlock fail\n");
		return CVI_FAILURE;
	}

	//open data file & fread into the mmap address
	fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		VPSS_UT_PRT("open data file error\n");
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
			VPSS_UT_PRT("vpss send frame: fread plane%d error\n", i);
			fclose(fp);
			CVI_VB_ReleaseBlock(blk);
			return CVI_FAILURE;
		}
		CVI_SYS_IonFlushCache(stVideoFrame.stVFrame.u64PhyAddr[i],
					   stVideoFrame.stVFrame.pu8VirAddr[i],
					   stVideoFrame.stVFrame.u32Length[i]);
	}

	VPSS_UT_PRT("length of buffer(%d, %d, %d)\n", stVideoFrame.stVFrame.u32Length[0]
		, stVideoFrame.stVFrame.u32Length[1], stVideoFrame.stVFrame.u32Length[2]);
	VPSS_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", stVideoFrame.stVFrame.u64PhyAddr[0]
		, stVideoFrame.stVFrame.u64PhyAddr[1], stVideoFrame.stVFrame.u64PhyAddr[2]);
	VPSS_UT_PRT("vir addr(%p, %p, %p)\n", stVideoFrame.stVFrame.pu8VirAddr[0]
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

CVI_S32 FrameFullSaveToFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	FILE *fp;
	CVI_U32 u32len, u32DataLen;

	fp = fopen(filename, "w");
	if (fp == CVI_NULL) {
		VPSS_UT_PRT("open data file(%s) error\n", filename);
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

		CVI_SYS_IonInvalidateCache(pstVideoFrame->stVFrame.u64PhyAddr[i],
			pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
		VPSS_UT_PRT("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d)\n",
			   i, pstVideoFrame->stVFrame.u64PhyAddr[i],
			   pstVideoFrame->stVFrame.pu8VirAddr[i],
			   pstVideoFrame->stVFrame.u32Stride[i]);
		VPSS_UT_PRT(" data_len(%d) plane_len(%d)\n",
			      u32DataLen, pstVideoFrame->stVFrame.u32Length[i]);
		u32len = fwrite(pstVideoFrame->stVFrame.pu8VirAddr[i], u32DataLen, 1, fp);
		if (u32len <= 0) {
			VPSS_UT_PRT("fwrite data(%d) error\n", i);
			s32Ret = CVI_FAILURE;
			break;
		}
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}

	fclose(fp);
	return s32Ret;
}

CVI_S32 FrameSaveToFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	FILE *fp;
	CVI_U32 i;
	CVI_S32 c_w_shift, c_h_shift; // chroma width/height shift
	CVI_S32 s32PixelSize;
	CVI_U32 u32Planar;
	CVI_U8 *w_ptr;
	CVI_U32 image_size = 0;
	CVI_S32 plane_offset = 0;
	CVI_VOID *vir_addr = NULL;
	VIDEO_FRAME_S *pstVFrame = &pstVideoFrame->stVFrame;

	fp = fopen(filename, "w");
	if (fp == CVI_NULL) {
		VPSS_UT_PRT("open data file(%s) error\n", filename);
		return CVI_FAILURE;
	}

	image_size = pstVFrame->u32Length[0] + pstVFrame->u32Length[1] + pstVFrame->u32Length[2];
	vir_addr = CVI_SYS_Mmap(pstVFrame->u64PhyAddr[0], image_size);
	CVI_SYS_IonInvalidateCache(pstVFrame->u64PhyAddr[0], vir_addr, image_size);

	for (i = 0; i < 3; i++) {
		if (pstVFrame->u32Length[i] == 0)
			continue;
		pstVFrame->pu8VirAddr[i] = vir_addr + plane_offset;
		plane_offset += pstVFrame->u32Length[i];
	}

	VPSS_UT_PRT("u32Width = %d, u32Height = %d\n", pstVFrame->u32Width, pstVFrame->u32Height);
	VPSS_UT_PRT("u32Stride[0] = %d, u32Stride[1] = %d, u32Stride[2] = %d\n",
		 pstVFrame->u32Stride[0], pstVFrame->u32Stride[1], pstVFrame->u32Stride[2]);
	VPSS_UT_PRT("u32Length[0] = %d, u32Length[1] = %d, u32Length[2] = %d\n",
		 pstVFrame->u32Length[0], pstVFrame->u32Length[1], pstVFrame->u32Length[2]);

	get_chroma_size_shift_factor(pstVFrame->enPixelFormat, &c_w_shift,
					  &c_h_shift, &s32PixelSize, &u32Planar);

	// save Y
	w_ptr = pstVFrame->pu8VirAddr[0];
	for (i = 0; i < pstVFrame->u32Height; i++) {
		fwrite(w_ptr + i * pstVFrame->u32Stride[0], s32PixelSize,
		       pstVFrame->u32Width, fp);
	}
	// save U
	if (u32Planar >= 2) {
		w_ptr = pstVFrame->pu8VirAddr[1];
		for (i = 0; i < (pstVFrame->u32Height >> c_h_shift); i++) {
			fwrite(w_ptr + i * pstVFrame->u32Stride[1], s32PixelSize,
			       pstVFrame->u32Width >> c_w_shift, fp);
		}
	}
	// save V
	if (u32Planar >= 3) {
		w_ptr = pstVFrame->pu8VirAddr[2];
		for (i = 0; i < (pstVFrame->u32Height >> c_h_shift); i++) {
			fwrite(w_ptr + i * pstVFrame->u32Stride[2], s32PixelSize,
			       pstVFrame->u32Width >> c_w_shift, fp);
		}
	}

	CVI_SYS_Munmap(vir_addr, image_size);
	fclose(fp);

	return 0;
}

CVI_S32 CompareWithFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	FILE *fp;
	CVI_U32 u32len, plane_len, data_len;
	CVI_U32 u32LumaData, u32ChromaData = 0, data_height;
	CVI_S32 result = CVI_SUCCESS;
	VB_CAL_CONFIG_S stVbCalConfig;

	u32LumaData = pstVideoFrame->stVFrame.u32Width;
	data_height = pstVideoFrame->stVFrame.u32Height;

	COMMON_GetPicBufferConfig(pstVideoFrame->stVFrame.u32Width, pstVideoFrame->stVFrame.u32Height,
		pstVideoFrame->stVFrame.enPixelFormat, DATA_BITWIDTH_8,
		COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_RGB_888_PLANAR ||
	    pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_BGR_888_PLANAR ||
	    pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_444) {
		u32ChromaData = u32LumaData;
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_422) {
		u32ChromaData =  (pstVideoFrame->stVFrame.u32Width / 2);
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) {
		u32ChromaData =  (pstVideoFrame->stVFrame.u32Width / 2);
		data_height = pstVideoFrame->stVFrame.u32Height / 2;
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV12 ||
		   pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV21) {
		u32ChromaData = u32LumaData;
		data_height = pstVideoFrame->stVFrame.u32Height / 2;
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV16 ||
		   pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV61) {
		u32ChromaData = u32LumaData;
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUYV ||
		   pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_UYVY ||
		   pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YVYU ||
		   pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_VYUY) {
		u32LumaData *= 2;
		u32ChromaData = 0;
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_400) {
		u32ChromaData = 0;
	}

	VPSS_UT_PRT("u32LumaSize(%d): u32ChromaSize(%d)\n",
		stVbCalConfig.u32MainYSize, stVbCalConfig.u32MainCSize);
	VPSS_UT_PRT("u32LumaData(%d): u32ChromaData(%d)\n", u32LumaData, u32ChromaData);
	fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		VPSS_UT_PRT("open data file, %s, error\n", filename);
		return CVI_FAILURE;
	}

	CVI_U8 buffer[stVbCalConfig.u32MainYSize];
	CVI_U32 offset = 0;

	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		plane_len = (i == 0) ? stVbCalConfig.u32MainYSize : stVbCalConfig.u32MainCSize;
		if (plane_len == 0)
			continue;
		data_len = (i == 0) ? u32LumaData : u32ChromaData;
		offset = 0;

		pstVideoFrame->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
		CVI_SYS_IonInvalidateCache(pstVideoFrame->stVFrame.u64PhyAddr[i],
			pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);

		u32len = fread(buffer, plane_len, 1, fp);
		if (u32len <= 0) {
			VPSS_UT_PRT("fread data(%d) error\n", i);
			result = CVI_FAILURE;
			break;
		}
		// line by line check to avoid padding data mismatch problem.
		for (CVI_U32 line = 0; line < data_height; ++line) {
			if (memcmp(buffer + offset, pstVideoFrame->stVFrame.pu8VirAddr[i] + offset, data_len) != 0) {
				VPSS_UT_PRT("plane(%d) line(%d) offset(%d) data mismatch:\n",
					      i, line, offset);
				VPSS_UT_PRT(" paddr(%#"PRIx64") vaddr(%p) stride(%d)\n",
					      pstVideoFrame->stVFrame.u64PhyAddr[i],
					      pstVideoFrame->stVFrame.pu8VirAddr[i],
					      pstVideoFrame->stVFrame.u32Stride[i]);

				result = CVI_FAILURE;
				break;
			}
			offset += pstVideoFrame->stVFrame.u32Stride[i];
		}
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}

	fclose(fp);
	return result;
}

CVI_S32 CompareWithMD5(const CVI_CHAR *md5sum, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	CVI_S32 result = CVI_SUCCESS;
	CVI_S32 c_w_shift, c_h_shift; // chroma width/height shift
	CVI_S32 s32PixelSize;
	CVI_U32 i, u32Planar;
	CVI_U8 *w_ptr;
	CVI_U32 image_size = 0;
	CVI_S32 plane_offset = 0;
	CVI_VOID *vir_addr = NULL;
	VIDEO_FRAME_S *pstVFrame = &pstVideoFrame->stVFrame;
	MD5_CTX md5_ctx;
	CVI_CHAR md[MD5_DIGEST_LENGTH];
	CVI_CHAR md_str[32];
	CVI_CHAR *p = md_str;
	CVI_S32 s32Index = 0;

	memset(md, 0, MD5_DIGEST_LENGTH);

	image_size = pstVFrame->u32Length[0] + pstVFrame->u32Length[1] + pstVFrame->u32Length[2];
	vir_addr = CVI_SYS_Mmap(pstVFrame->u64PhyAddr[0], image_size);
	CVI_SYS_IonInvalidateCache(pstVFrame->u64PhyAddr[0], vir_addr, image_size);

	for (i = 0; i < 3; i++) {
		if (pstVFrame->u32Length[i] == 0)
			continue;
		pstVFrame->pu8VirAddr[i] = vir_addr + plane_offset;
		plane_offset += pstVFrame->u32Length[i];
	}

	get_chroma_size_shift_factor(pstVFrame->enPixelFormat, &c_w_shift,
					  &c_h_shift, &s32PixelSize, &u32Planar);
	MD5_Init(&md5_ctx);

	// Compare Y
	w_ptr = pstVFrame->pu8VirAddr[0];
	for (i = 0; i < pstVFrame->u32Height; i++) {
		MD5_Update(&md5_ctx, w_ptr + i * pstVFrame->u32Stride[0],
			s32PixelSize * pstVFrame->u32Width);
	}
	// Compare U
	if (u32Planar >= 2) {
		w_ptr = pstVFrame->pu8VirAddr[1];
		for (i = 0; i < (pstVFrame->u32Height >> c_h_shift); i++) {
			MD5_Update(&md5_ctx, w_ptr + i * pstVFrame->u32Stride[1],
				s32PixelSize * (pstVFrame->u32Width >> c_w_shift));
		}
	}
	// Compare V
	if (u32Planar >= 3) {
		w_ptr = pstVFrame->pu8VirAddr[2];
		for (i = 0; i < (pstVFrame->u32Height >> c_h_shift); i++) {
			MD5_Update(&md5_ctx, w_ptr + i * pstVFrame->u32Stride[2],
				s32PixelSize * (pstVFrame->u32Width >> c_w_shift));
		}
	}

	MD5_Final((unsigned char *)md, &md5_ctx);
	CVI_SYS_Munmap(vir_addr, image_size);

	for (i = 0; i < MD5_DIGEST_LENGTH; i++)
		s32Index += snprintf(p + s32Index, 32, "%02x", md[i]);

	if (strncmp(md5sum, md_str, 32)) {
		VPSS_UT_PRT("md5sum error, frame md5sum:%s\n", md_str);
		result = CVI_FAILURE;
	}

	return result;
}

CVI_S32 CompareCmodel_rgb2yuv(VIDEO_FRAME_INFO_S *pstVideoFrameIn, VIDEO_FRAME_INFO_S *pstVideoFrameOut)
{
	CVI_U32 i, w, h;
	CVI_S32 result = CVI_SUCCESS;
	CVI_U8 yuvData[3];
	CVI_U8 *p, *y, *u, *v;
	CVI_BOOL uv_bypass = CVI_TRUE;

	//rgb packed
	pstVideoFrameIn->stVFrame.pu8VirAddr[0]
		= CVI_SYS_Mmap(pstVideoFrameIn->stVFrame.u64PhyAddr[0], pstVideoFrameIn->stVFrame.u32Length[0]);

	for (i = 0; i < 3; ++i) {
		if (pstVideoFrameOut->stVFrame.u32Length[i] == 0)
			continue;

		pstVideoFrameOut->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(pstVideoFrameOut->stVFrame.u64PhyAddr[i], pstVideoFrameOut->stVFrame.u32Length[i]);

		CVI_SYS_IonInvalidateCache(pstVideoFrameOut->stVFrame.u64PhyAddr[i],
			pstVideoFrameOut->stVFrame.pu8VirAddr[i], pstVideoFrameOut->stVFrame.u32Length[i]);
		VPSS_UT_PRT("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d) plane_len(%d)\n",
				i, pstVideoFrameOut->stVFrame.u64PhyAddr[i],
				pstVideoFrameOut->stVFrame.pu8VirAddr[i],
				pstVideoFrameOut->stVFrame.u32Stride[i],
				pstVideoFrameOut->stVFrame.u32Length[i]);
	}

	for (h = 0; h < pstVideoFrameOut->stVFrame.u32Height; h++) {
		p = pstVideoFrameIn->stVFrame.pu8VirAddr[0] + h * pstVideoFrameIn->stVFrame.u32Stride[0];

		for (w = 0; w < pstVideoFrameOut->stVFrame.u32Width; w++) {
			vpss_csc_rgb2yuv(p, yuvData);
			y = pstVideoFrameOut->stVFrame.pu8VirAddr[0] +
				pstVideoFrameOut->stVFrame.u32Stride[0] * h + w;
			if (yuvData[0] != *y) {
				VPSS_UT_PRT("y data error,(%d -> %d), w:%d h:%d\n", yuvData[0], *y, w, h);
				result = CVI_FAILURE;
				break;
			}
			/*u00 u01
			  u10 u11
			right bottom, yuv420 use u11, yuv422 use u01/u11 */
			if (pstVideoFrameOut->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) {
				u = pstVideoFrameOut->stVFrame.pu8VirAddr[1] +
					pstVideoFrameOut->stVFrame.u32Stride[1] * (h / 2) + (w / 2);
				v = pstVideoFrameOut->stVFrame.pu8VirAddr[2] +
					pstVideoFrameOut->stVFrame.u32Stride[2] * (h / 2) + (w / 2);
				uv_bypass = ((h % 2) && (w % 2)) ? CVI_FALSE : CVI_TRUE;
			} else if (pstVideoFrameOut->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_422) {
				u = pstVideoFrameOut->stVFrame.pu8VirAddr[1] +
					pstVideoFrameOut->stVFrame.u32Stride[1] * h + (w / 2);
				v = pstVideoFrameOut->stVFrame.pu8VirAddr[2] +
					pstVideoFrameOut->stVFrame.u32Stride[2] * h + (w / 2);
				uv_bypass = (w % 2) ? CVI_FALSE : CVI_TRUE;
			} else if (pstVideoFrameOut->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_444) {
				u = pstVideoFrameOut->stVFrame.pu8VirAddr[1] +
					pstVideoFrameOut->stVFrame.u32Stride[1] * h + w;
				v = pstVideoFrameOut->stVFrame.pu8VirAddr[2] +
					pstVideoFrameOut->stVFrame.u32Stride[2] * h + w;
				uv_bypass = CVI_FALSE;
			}

			if (!uv_bypass && (yuvData[1] != *u)) {
				VPSS_UT_PRT("u data error,(%d -> %d), w:%d h:%d\n", yuvData[1], *u, w, h);
				result = CVI_FAILURE;
				break;
			}
			if (!uv_bypass && (yuvData[2] != *v)) {
				VPSS_UT_PRT("v data error,(%d -> %d), w:%d h:%d\n", yuvData[2], *v, w, h);
				result = CVI_FAILURE;
				break;
			}
			p = p + 3;
		}
	}

	for (i = 0; i < 3; ++i) {
		if (pstVideoFrameOut->stVFrame.u32Length[i] == 0)
			continue;
		CVI_SYS_Munmap(pstVideoFrameOut->stVFrame.pu8VirAddr[i], pstVideoFrameOut->stVFrame.u32Length[i]);
	}
	CVI_SYS_Munmap(pstVideoFrameIn->stVFrame.pu8VirAddr[0], pstVideoFrameIn->stVFrame.u32Length[0]);

	return result;
}

#if 0
/*no fancy
  u00 u01
  u10 u11
right bottom, yuv420 use u11, yuv422 use u01/u11 */
CVI_VOID yuv420to444(CVI_U8 *pu8InData[3], CVI_U32 u32Width, CVI_U32 u32Height,
			CVI_U32 u32Stride[3], CVI_U8 *pu8OutData)
{
	CVI_U32 i, h, w;
	CVI_U8 *inptr, *outptr;

	//copy y
	memcpy(pu8OutData, pu8InData[0], u32Width * u32Height);

	//transfer uv
	for (i = 1; i < 3; i++) {
		outptr = pu8OutData + u32Width * u32Height * i;
		for (h = 0; h < u32Height; h++) {
			inptr = pu8InData[i] + u32Stride[i] * (h / 2);
			for (w = 0; w < u32Width; w++) {
				*outptr++ = *inptr;
				if (w % 2)
					inptr++;
			}
		}
	}
	VPSS_UT_PRT("---\n");
}

CVI_VOID yuv422to444(CVI_U8 *pu8InData[3], CVI_U32 u32Width, CVI_U32 u32Height,
			CVI_U32 u32Stride[3], CVI_U8 *pu8OutData)
{
	CVI_U32 i, h, w;
	CVI_U8 *inptr, *outptr;

	//copy y
	memcpy(pu8OutData, pu8InData[0], u32Width * u32Height);

	//transfer uv
	for (i = 1; i < 3; i++) {
		outptr = pu8OutData + u32Width * u32Height * i;
		for (h = 0; h < u32Height; h++) {
			inptr = pu8InData[i] + u32Stride[i] * h;
			for (w = 0; w < u32Width; w++) {
				*outptr++ = *inptr;
				if (w % 2)
					inptr++;
			}
		}
	}
	VPSS_UT_PRT("---\n");
}

#else
/*fancy upsample*/
CVI_VOID yuv420to444(CVI_U8 *pu8InData[3], CVI_U32 u32Width, CVI_U32 u32Height,
		CVI_U32 u32Stride[3], CVI_U8 *pu8OutData)
{
	CVI_U8 v, i;
	CVI_U32 u32Width_uv = u32Width / 2;
	CVI_U32 u32Height_uv = u32Height / 2;
	CVI_U32 in_h_num = 0, out_h_unm = 0;
	CVI_U8 *inptr0, *inptr1, *outptr;
	CVI_U8 *input_data, *output_data;
	CVI_U32 thiscolsum, nextcolsum, lastcolsum;
	CVI_S32 colctr;

	//copy y
	memcpy(pu8OutData, pu8InData[0], u32Width * u32Height);

	//transfer uv
	for (i = 1; i < 3; i++) {
		input_data = pu8InData[i];
		output_data = pu8OutData + u32Width * u32Height * i;
		out_h_unm = in_h_num = 0;

		while (out_h_unm < u32Height) {
			for (v = 0; v < 2; v++) {	/* inptr0 points to nearest input row, inptr1 points to next nearest */
				inptr0 = input_data + u32Stride[i] * in_h_num;
				if (v == 0)		/* next nearest is row above */
					inptr1 = inptr0 - (in_h_num ? u32Stride[i] : 0);
				else			/* next nearest is row below */
					inptr1 = inptr0 + ((in_h_num ==  u32Height_uv - 1) ? 0 : u32Stride[i]);

				outptr = output_data +  u32Width * out_h_unm++;
				thiscolsum = (*inptr0++) * 3 + (*inptr1++); /* Special case for first column */
				nextcolsum = (*inptr0++) * 3 + (*inptr1++);
				*outptr++ = ((thiscolsum * 4 + 8) >> 4);
				*outptr++ = ((thiscolsum * 3 + nextcolsum + 7) >> 4);
				lastcolsum = thiscolsum;
				thiscolsum = nextcolsum;

				for (colctr = u32Width_uv - 2; colctr > 0; colctr--) {
					/* General case: 3/4 * nearer pixel + 1/4 * further pixel in each */
					nextcolsum = (*inptr0++) * 3 + (*inptr1++);
					/* dimension, thus 9/16, 3/16, 3/16, 1/16 overall */
					*outptr++ = ((thiscolsum * 3 + lastcolsum + 8) >> 4);
					*outptr++ = ((thiscolsum * 3 + nextcolsum + 7) >> 4);
					lastcolsum = thiscolsum;
					thiscolsum = nextcolsum;
				}
				*outptr++ = ((thiscolsum * 3 + lastcolsum + 8) >> 4); /* Special case for last column */
				*outptr++ = ((thiscolsum * 4 + 7) >> 4);
			}
			in_h_num++;
		}
	}

	VPSS_UT_PRT("---\n");
}

CVI_VOID yuv422to444(CVI_U8 *pu8InData[3], CVI_U32 u32Width, CVI_U32 u32Height,
		CVI_U32 u32Stride[3], CVI_U8 *pu8OutData)
{
	CVI_U32 i, h, invalue;
	CVI_U32 u32Width_uv = u32Width / 2;
	CVI_U8 *inptr, *outptr;
	CVI_U8 *input_data, *output_data;
	CVI_S32 colctr;

	//copy y
	memcpy(pu8OutData, pu8InData[0], u32Width * u32Height);

	//transfer uv
	for (i = 1; i < 3; i++) {
		input_data = pu8InData[i];
		output_data = pu8OutData + u32Width * u32Height * i;
		for (h = 0; h < u32Height; h++) {
			inptr = input_data + u32Stride[i] * h;
			outptr = output_data + u32Width * h;

			/* Special case for first column */
			invalue = (*inptr++);
			*outptr++ = invalue;
			*outptr++ = ((invalue * 3 + (*inptr) + 2) >> 2);

			for (colctr = u32Width_uv - 2; colctr > 0; colctr--) {
				/* General case: 3/4 * nearer pixel + 1/4 * further pixel */
				invalue = (*inptr++) * 3;
				*outptr++ = ((invalue + (inptr[-2]) + 1) >> 2);
				*outptr++ = ((invalue + (*inptr) + 2) >> 2);
			}

			/* Special case for last column */
			invalue = (*inptr);
			*outptr++ = ((invalue * 3 + (inptr[-1]) + 1) >> 2);
			*outptr++ = invalue;
		}
	}
	VPSS_UT_PRT("---\n");
}
#endif

CVI_S32 CompareCmodel_yuv2rgb(VIDEO_FRAME_INFO_S *pstVideoFrameIn, VIDEO_FRAME_INFO_S *pstVideoFrameOut)
{
	CVI_U32 i, w, h;
	CVI_S32 result = CVI_SUCCESS;
	CVI_U8 yuvData[3], rgbData[3];
	CVI_U8 *p, *y, *u = NULL, *v = NULL;
	CVI_U8 *yuv444_data = NULL;
	CVI_S32 yuv444_data_len = pstVideoFrameIn->stVFrame.u32Height * pstVideoFrameIn->stVFrame.u32Width * 3;

	//rgb packed
	pstVideoFrameOut->stVFrame.pu8VirAddr[0]
		= CVI_SYS_Mmap(pstVideoFrameOut->stVFrame.u64PhyAddr[0], pstVideoFrameOut->stVFrame.u32Length[0]);

	for (i = 0; i < 3; ++i) {
		if (pstVideoFrameIn->stVFrame.u32Length[i] == 0)
			continue;

		pstVideoFrameIn->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(pstVideoFrameIn->stVFrame.u64PhyAddr[i], pstVideoFrameIn->stVFrame.u32Length[i]);

		VPSS_UT_PRT("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d) plane_len(%d)\n",
				i, pstVideoFrameIn->stVFrame.u64PhyAddr[i],
				pstVideoFrameIn->stVFrame.pu8VirAddr[i],
				pstVideoFrameIn->stVFrame.u32Stride[i],
				pstVideoFrameIn->stVFrame.u32Length[i]);
	}

	if (pstVideoFrameIn->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) {
		yuv444_data = malloc(yuv444_data_len);
		if (!yuv444_data) {
			VPSS_UT_PRT("malloc fail\n");
			result = CVI_FAILURE;
			goto exit;
		}
		yuv420to444(pstVideoFrameIn->stVFrame.pu8VirAddr,
					pstVideoFrameIn->stVFrame.u32Width,
					pstVideoFrameIn->stVFrame.u32Height,
					pstVideoFrameIn->stVFrame.u32Stride,
					yuv444_data);
	} else if(pstVideoFrameIn->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_422) {
		yuv444_data = malloc(yuv444_data_len);
		if (!yuv444_data) {
			VPSS_UT_PRT("malloc fail\n");
			result = CVI_FAILURE;
			goto exit;
		}
		yuv422to444(pstVideoFrameIn->stVFrame.pu8VirAddr,
					pstVideoFrameIn->stVFrame.u32Width,
					pstVideoFrameIn->stVFrame.u32Height,
					pstVideoFrameIn->stVFrame.u32Stride,
					yuv444_data);
	}

	for (h = 0; h < pstVideoFrameOut->stVFrame.u32Height; h++) {
		p = pstVideoFrameOut->stVFrame.pu8VirAddr[0] + h * pstVideoFrameOut->stVFrame.u32Stride[0];

		if ((pstVideoFrameIn->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420)
			|| (pstVideoFrameIn->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_422)) {
			y = yuv444_data + pstVideoFrameIn->stVFrame.u32Width * h;
			u = yuv444_data + pstVideoFrameIn->stVFrame.u32Height * pstVideoFrameIn->stVFrame.u32Width
				+ pstVideoFrameIn->stVFrame.u32Width * h;
			v = yuv444_data + pstVideoFrameIn->stVFrame.u32Height * pstVideoFrameIn->stVFrame.u32Width * 2
				+ pstVideoFrameIn->stVFrame.u32Width * h;

		} else {
			y = pstVideoFrameIn->stVFrame.pu8VirAddr[0] + pstVideoFrameIn->stVFrame.u32Stride[0] * h;
			u = pstVideoFrameIn->stVFrame.pu8VirAddr[1] + pstVideoFrameIn->stVFrame.u32Stride[1] * h;
			v = pstVideoFrameIn->stVFrame.pu8VirAddr[2] + pstVideoFrameIn->stVFrame.u32Stride[2] * h;
		}

		for (w = 0; w < pstVideoFrameOut->stVFrame.u32Width; w++) {
			yuvData[0] = *y++;
			yuvData[1] = *u++;
			yuvData[2] = *v++;

			vpss_csc_yuv2rgb(yuvData, rgbData);

			if (rgbData[0] != *p) {
				VPSS_UT_PRT("R data error, yuv(%d %d %d), rgb(sw:%d hw:%d), w:%d h:%d\n",
					yuvData[0], yuvData[1], yuvData[2], rgbData[0], *p, w, h);
				result = CVI_FAILURE;
				break;
			}
			p++;
			if (rgbData[1] != *p) {
				VPSS_UT_PRT("G data error, yuv(%d %d %d), rgb(sw:%d hw:%d), w:%d h:%d\n",
					yuvData[0], yuvData[1], yuvData[2], rgbData[1], *p, w, h);
				result = CVI_FAILURE;
				break;
			}
			p++;
			if (rgbData[2] != *p) {
				VPSS_UT_PRT("B data error, yuv(%d %d %d), rgb(sw:%d hw:%d), w:%d h:%d\n",
					yuvData[0], yuvData[1], yuvData[2], rgbData[2], *p, w, h);
				result = CVI_FAILURE;
				break;
			}
			p++;
		}
	}

exit:
	if (yuv444_data)
		free(yuv444_data);
	for (i = 0; i < 3; ++i) {
		if (pstVideoFrameIn->stVFrame.u32Length[i] == 0)
			continue;
		CVI_SYS_Munmap(pstVideoFrameIn->stVFrame.pu8VirAddr[i], pstVideoFrameIn->stVFrame.u32Length[i]);
	}

	CVI_SYS_Munmap(pstVideoFrameOut->stVFrame.pu8VirAddr[0], pstVideoFrameOut->stVFrame.u32Length[0]);

	return result;
}


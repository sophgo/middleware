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
#include "cvi_dwa.h"
#include "sample_fisheye_comm.h"

#define DWA_EXT_OP_CLR_VB (1)

CVI_CHAR * DWAGetFmtName(PIXEL_FORMAT_E enPixFmt)
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

CVI_S32 DWA_COMM_PrepareFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VB_BLK blk;
	VB_CAL_CONFIG_S stVbCalConfig;

	if (pstVideoFrame == CVI_NULL) {
		DWA_UT_PRT("Null pointer!\n");
		return CVI_FAILURE;
	}

	//COMMON_GetPicBufferConfig(stSize->u32Width, stSize->u32Height, enPixelFormat, DATA_BITWIDTH_8
		//, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN, &stVbCalConfig);
	COMMON_GetPicBufferConfig(stSize->u32Width, stSize->u32Height, enPixelFormat, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, 1, &stVbCalConfig);

	memset(pstVideoFrame, 0, sizeof(*pstVideoFrame));
	pstVideoFrame->stVFrame.enCompressMode = COMPRESS_MODE_NONE;
	pstVideoFrame->stVFrame.enPixelFormat = enPixelFormat;
	pstVideoFrame->stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
	pstVideoFrame->stVFrame.enColorGamut = COLOR_GAMUT_BT601;
	pstVideoFrame->stVFrame.u32Width = stSize->u32Width;
	pstVideoFrame->stVFrame.u32Height = stSize->u32Height;
	pstVideoFrame->stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
	pstVideoFrame->stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
	pstVideoFrame->stVFrame.u32TimeRef = 0;
	pstVideoFrame->stVFrame.u64PTS = 0;
	pstVideoFrame->stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;

	blk = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCalConfig.u32VBSize);
	if (blk == VB_INVALID_HANDLE) {
		DWA_UT_PRT("Can't acquire vb block\n");
		return CVI_FAILURE;
	}

	pstVideoFrame->u32PoolId = CVI_VB_Handle2PoolId(blk);
	pstVideoFrame->stVFrame.u32Length[0] = stVbCalConfig.u32MainYSize;
	pstVideoFrame->stVFrame.u32Length[1] = stVbCalConfig.u32MainCSize;
	pstVideoFrame->stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
	pstVideoFrame->stVFrame.u64PhyAddr[1] = pstVideoFrame->stVFrame.u64PhyAddr[0]
		+ ALIGN(stVbCalConfig.u32MainYSize, stVbCalConfig.u16AddrAlign);
	if (stVbCalConfig.plane_num == 3) {
		pstVideoFrame->stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
		pstVideoFrame->stVFrame.u32Length[2] = stVbCalConfig.u32MainCSize;
		pstVideoFrame->stVFrame.u64PhyAddr[2] = pstVideoFrame->stVFrame.u64PhyAddr[1]
			+ ALIGN(stVbCalConfig.u32MainCSize, stVbCalConfig.u16AddrAlign);
	}

#if DWA_EXT_OP_CLR_VB
	for (int i = 0; i < 3; ++i) {
		if (pstVideoFrame->stVFrame.u32Length[i] == 0)
			continue;
		pstVideoFrame->stVFrame.pu8VirAddr[i] = CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
		DWA_UT_PRT("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d) plane_len(%d)\n", i
			, pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.pu8VirAddr[i]
			, pstVideoFrame->stVFrame.u32Stride[i], pstVideoFrame->stVFrame.u32Length[i]);

		memset(pstVideoFrame->stVFrame.pu8VirAddr[i], 0, pstVideoFrame->stVFrame.u32Length[i]);
		CVI_SYS_IonFlushCache(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}
#endif

	return CVI_SUCCESS;
}


CVI_S32 DWAFileToFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat,
		CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VB_BLK blk;
	CVI_U32 u32len;
	CVI_S32 Ret;
	int i;
	FILE *fp;

	if (!pstVideoFrame) {
		DWA_UT_PRT("pstVideoFrame is null\n");
		return CVI_FAILURE_ILLEGAL_PARAM;
	}

	if (!filename) {
		DWA_UT_PRT("filename is null\n");
		return CVI_FAILURE_ILLEGAL_PARAM;
	}

	Ret = DWA_COMM_PrepareFrame(stSize, enPixelFormat, pstVideoFrame);
	if (Ret != CVI_SUCCESS) {
		DWA_UT_PRT("DWA_COMM_PrepareFrame FAIL,get VB fail\n");
		return CVI_FAILURE;
	}

	blk = CVI_VB_PhysAddr2Handle(pstVideoFrame->stVFrame.u64PhyAddr[0]);

	//open data file & fread into the mmap address
	fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		DWA_UT_PRT("open data file[%s] error\n", filename);
		CVI_VB_ReleaseBlock(blk);
		return CVI_FAILURE;
	}

	for (i = 0; i < 3; ++i) {
		if (pstVideoFrame->stVFrame.u32Length[i] == 0)
			continue;
		pstVideoFrame->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
#if 0
		DWA_UT_PRT("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d) plane_len(%d)\n",
			   i, pstVideoFrame->stVFrame.u64PhyAddr[i],
			   pstVideoFrame->stVFrame.pu8VirAddr[i],
			   pstVideoFrame->stVFrame.u32Stride[i],
			   pstVideoFrame->stVFrame.u32Length[i]);
#endif
		u32len = fread(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i], 1, fp);
		if (u32len <= 0) {
			DWA_UT_PRT("file to frame: fread plane%d error\n", i);
			CVI_VB_ReleaseBlock(blk);
			Ret = CVI_FAILURE;
			break;
		}
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}

	if (Ret)
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);

	fclose(fp);

	return Ret;
}

CVI_S32 DWAFrameSaveToFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	FILE *fp;
	CVI_U32 u32len, u32DataLen;
	int i;

	if (!pstVideoFrame) {
		DWA_UT_PRT("pstVideoFrame is null\n");
		return CVI_FAILURE_ILLEGAL_PARAM;
	}

	if (!filename) {
		DWA_UT_PRT("filename is null\n");
		return CVI_FAILURE_ILLEGAL_PARAM;
	}

	fp = fopen(filename, "w");
	if (fp == CVI_NULL) {
		DWA_UT_PRT("open data file(%s) error\n", filename);
		return CVI_FAILURE;
	}

	for (i = 0; i < 3; ++i) {
		u32DataLen = pstVideoFrame->stVFrame.u32Stride[i] * pstVideoFrame->stVFrame.u32Height;
		if (u32DataLen == 0)
			continue;
		if (i > 0 && ((pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
			(pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) ||
			(pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)))
			u32DataLen >>= 1;

		pstVideoFrame->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);

#if 0
		DWA_UT_PRT("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d)\n",
			   i, pstVideoFrame->stVFrame.u64PhyAddr[i],
			   pstVideoFrame->stVFrame.pu8VirAddr[i],
			   pstVideoFrame->stVFrame.u32Stride[i]);
		DWA_UT_PRT(" data_len(%d) plane_len(%d)\n",
			      u32DataLen, pstVideoFrame->stVFrame.u32Length[i]);
#endif
		u32len = fwrite(pstVideoFrame->stVFrame.pu8VirAddr[i], u32DataLen, 1, fp);
		if (u32len <= 0) {
			DWA_UT_PRT("fwrite data(%d) error\n", i);
			s32Ret = CVI_FAILURE;
			break;
		}
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}

	if (s32Ret)
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);

	fclose(fp);
	return s32Ret;
}

CVI_S32 DWACompareWithFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	FILE *fp;
	CVI_U32 u32len, plane_len, data_len;
	CVI_U32 u32LumaData, u32ChromaData = 0, data_height;
	CVI_S32 result = CVI_SUCCESS, unmapflag = 0;
	VB_CAL_CONFIG_S stVbCalConfig;
	int i;

	u32LumaData = pstVideoFrame->stVFrame.u32Width;
	data_height = pstVideoFrame->stVFrame.u32Height;

	COMMON_GetPicBufferConfig(pstVideoFrame->stVFrame.u32Width, pstVideoFrame->stVFrame.u32Height,
		pstVideoFrame->stVFrame.enPixelFormat, DATA_BITWIDTH_8,
		COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN, &stVbCalConfig);

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

	DWA_UT_PRT("u32LumaSize(%d): u32ChromaSize(%d)\n",
		stVbCalConfig.u32MainYSize, stVbCalConfig.u32MainCSize);
	DWA_UT_PRT("u32LumaData(%d): u32ChromaData(%d)\n", u32LumaData, u32ChromaData);
	fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		DWA_UT_PRT("open data file, %s, error\n", filename);
		return CVI_FAILURE;
	}

	CVI_U8 *buffer = calloc(1, stVbCalConfig.u32MainYSize);
	CVI_U32 offset = 0;

	for (i = 0; i < stVbCalConfig.plane_num; ++i) {
		plane_len = (i == 0) ? stVbCalConfig.u32MainYSize : stVbCalConfig.u32MainCSize;
		if (plane_len == 0)
			continue;
		data_len = (i == 0) ? u32LumaData : u32ChromaData;
		offset = 0;

		pstVideoFrame->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);

		u32len = fread(buffer, plane_len, 1, fp);
		if (u32len <= 0) {
			DWA_UT_PRT("fread data(%d) error\n", i);
			result = CVI_FAILURE;
			unmapflag = 1;
			break;
		}
		// line by line check to avoid padding data mismatch problem.
		for (CVI_U32 line = 0; line < data_height; ++line) {
			if (memcmp(buffer + offset, pstVideoFrame->stVFrame.pu8VirAddr[i] + offset, data_len) != 0) {
				DWA_UT_PRT("plane(%d) line(%d) offset(%d) data mismatch:\n",
					      i, line, offset);
				DWA_UT_PRT(" paddr(%#"PRIx64") vaddr(%p) stride(%d)\n",
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

	if (result && unmapflag)
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);

	fclose(fp);
	free(buffer);

	return result;
}


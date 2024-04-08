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
#include "cvi_dpu.h"
#include "dpu_ut_comm.h"

#define ALIGN_16   16

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

CVI_S32 FileToFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat,
		CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	VB_BLK blk;
	CVI_U32 u32len;
	VB_CAL_CONFIG_S stVbCalConfig;
	FILE *fp;

	COMMON_GetPicBufferConfig(stSize->u32Width, stSize->u32Height, enPixelFormat, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, ALIGN_16, &stVbCalConfig);

	memset(&stVideoFrame, 0, sizeof(stVideoFrame));
	stVideoFrame.stVFrame.enCompressMode = COMPRESS_MODE_NONE;
	stVideoFrame.stVFrame.enPixelFormat = enPixelFormat;
	stVideoFrame.stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
	stVideoFrame.stVFrame.enColorGamut = COLOR_GAMUT_BT709;
	stVideoFrame.stVFrame.u32Width = stSize->u32Width;
	stVideoFrame.stVFrame.u32Height = stSize->u32Height;
	stVideoFrame.stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;

	// stVideoFrame.stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
	// stVideoFrame.stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
	stVideoFrame.stVFrame.u32TimeRef = 0;
	stVideoFrame.stVFrame.u64PTS = 0;
	stVideoFrame.stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;
	DPU_UT_PRT("Format(%d) Width(%d) Height(%d) Stride(%d)\n",stVideoFrame.stVFrame.enPixelFormat,\
				stVideoFrame.stVFrame.u32Width,stVideoFrame.stVFrame.u32Height,stVideoFrame.stVFrame.u32Stride[0]);
	blk = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCalConfig.u32VBSize);
	if (blk == VB_INVALID_HANDLE) {
		DPU_UT_PRT("CVI_VB_GetBlock fail\n");
		return CVI_FAILURE;
	}

	//open data file & fread into the mmap address
	fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		DPU_UT_PRT("open data file error\n");
		CVI_VB_ReleaseBlock(blk);
		return CVI_FAILURE;
	}

	stVideoFrame.u32PoolId = CVI_VB_Handle2PoolId(blk);
	stVideoFrame.stVFrame.u32Length[0] = stVbCalConfig.u32MainYSize;
	// stVideoFrame.stVFrame.u32Length[1] = stVbCalConfig.u32MainCSize;
	stVideoFrame.stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
	// stVideoFrame.stVFrame.u64PhyAddr[1] = stVideoFrame.stVFrame.u64PhyAddr[0]
	// 	+ ALIGN(stVbCalConfig.u32MainYSize, stVbCalConfig.u16AddrAlign);
	// if (stVbCalConfig.plane_num == 3) {
	// 	stVideoFrame.stVFrame.u32Length[2] = stVbCalConfig.u32MainCSize;
	// 	stVideoFrame.stVFrame.u64PhyAddr[2] = stVideoFrame.stVFrame.u64PhyAddr[1]
	// 		+ ALIGN(stVbCalConfig.u32MainCSize, stVbCalConfig.u16AddrAlign);
	// }

	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		if (stVideoFrame.stVFrame.u32Length[i] == 0)
			continue;
		stVideoFrame.stVFrame.pu8VirAddr[i]
			= CVI_SYS_MmapCache(stVideoFrame.stVFrame.u64PhyAddr[i], stVideoFrame.stVFrame.u32Length[i]);

		u32len = fread(stVideoFrame.stVFrame.pu8VirAddr[i], stVideoFrame.stVFrame.u32Length[i], 1, fp);
		if (u32len <= 0) {
			DPU_UT_PRT("dpu send frame: fread plane%d error\n", i);
			fclose(fp);
			CVI_VB_ReleaseBlock(blk);
			return CVI_FAILURE;
		}
		CVI_SYS_IonFlushCache(stVideoFrame.stVFrame.u64PhyAddr[i],
					   stVideoFrame.stVFrame.pu8VirAddr[i],
					   stVideoFrame.stVFrame.u32Length[i]);
	}

	DPU_UT_PRT("length of buffer(%d)\n", stVideoFrame.stVFrame.u32Length[0]);
	DPU_UT_PRT("phy addr(%#"PRIx64")\n", stVideoFrame.stVFrame.u64PhyAddr[0]);
	DPU_UT_PRT("vir addr(%p)\n", stVideoFrame.stVFrame.pu8VirAddr[0]);

	fclose(fp);

	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		if (stVideoFrame.stVFrame.u32Length[i] == 0)
			continue;
		CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[i], stVideoFrame.stVFrame.u32Length[i]);
	}
	memcpy(pstVideoFrame, &stVideoFrame, sizeof(stVideoFrame));


	return CVI_SUCCESS;
}

CVI_S32 FrameSaveToFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	FILE *fp;
	CVI_U32 u32len, u32DataLen;

	fp = fopen(filename, "wb");
	if (fp == CVI_NULL) {
		DPU_UT_PRT("open data file error\n");
		return CVI_FAILURE;
	}

	for (int i = 0; i < 1; ++i) {
		u32DataLen = pstVideoFrame->stVFrame.u32Length[i];
		if (u32DataLen == 0)
			continue;

		pstVideoFrame->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
		CVI_SYS_IonInvalidateCache(pstVideoFrame->stVFrame.u64PhyAddr[i],
					   pstVideoFrame->stVFrame.pu8VirAddr[i],
					   pstVideoFrame->stVFrame.u32Length[i]);
		DPU_UT_PRT("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d)\n",
			   i, pstVideoFrame->stVFrame.u64PhyAddr[i],
			   pstVideoFrame->stVFrame.pu8VirAddr[i],
			   pstVideoFrame->stVFrame.u32Stride[i]);
		DPU_UT_PRT(" data_len(%d) plane_len(%d)\n",
			      u32DataLen, pstVideoFrame->stVFrame.u32Length[i]);
		u32len = fwrite(pstVideoFrame->stVFrame.pu8VirAddr[i], u32DataLen, 1, fp);

		if (u32len <= 0) {
			DPU_UT_PRT("fwrite data(%d) error\n", i);
			s32Ret = CVI_FAILURE;
			break;
		}
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}

	fclose(fp);
	return s32Ret;
}

CVI_S32 CompareWithFile(const CVI_CHAR *filenameSrc,const CVI_CHAR *filenameDst,CVI_S32 size)
{
	FILE *fp1;
	FILE *fp2;
	CVI_U8 *src;
	CVI_U8 *dst;
	CVI_S32 srcLen;
	CVI_S32 dstLen;
	src = (CVI_U8*)malloc(sizeof(CVI_U8)*size);
	dst = (CVI_U8*)malloc(sizeof(CVI_U8)*size);
	fp1 = fopen(filenameSrc, "rb+");
	if (fp1 == CVI_NULL) {
		DPU_UT_PRT("open src data file error\n");
		return CVI_FAILURE;
	}
	srcLen= fread(src,1,size,fp1);
	if(srcLen != size){
		DPU_UT_PRT("Size(%d) srcLen(%d )dst file size not match\n",size,srcLen);
		return CVI_FAILURE;
	}

	fp2 = fopen(filenameDst, "rb+");
	if (fp2 == CVI_NULL) {
		DPU_UT_PRT("open dst data file error\n");
		return CVI_FAILURE;
	}
	dstLen= fread(dst,1,size,fp2);
	if(dstLen != size){
		DPU_UT_PRT("Size(%d) dstLen(%d )dst file size not match\n",size,dstLen);
		return CVI_FAILURE;
	}
	for(int i=0;i<size;++i){
		if(src[i] != dst[i]){
			DPU_UT_PRT("i(%d) src(%d) not equal the dst(%d)\n",i,src[i],dst[i]);
			return CVI_FAILURE;
		}
	}
	DPU_UT_PRT(" compare done!\n");
	free(src);
	free(dst);
	fclose(fp1);
	fclose(fp2);
	return CVI_SUCCESS;
}



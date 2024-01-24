/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: sample/common/sample_common_dpu.c
 * Description:
 *   Common sample code for video process.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "sample_comm.h"
#include "sample_common_dpu.h"

#define DPU_EXT_OP_CLR_VB 1
#define ALIGN_16 16
#define TIMEOUT_GET_FRAME 1000
#define DWA_STRIDE_ALIGN    32

#define CHECK_DPU_FMT(grp, chn, fmt)									\													\
		if (!DPU_SUPPORT_FMT(fmt)) {		\
			CVI_TRACE_DPU(CVI_DBG_ERR, "Grp(%d) Chn(%d) invalid PixFormat(%d) for DPU.\n"		\
				      , grp, chn, (fmt));							\
			return CVI_ERR_DPU_ILLEGAL_PARAM;							\
		}

static inline CVI_S32 CHECK_DPU_GRP_VALID(DPU_GRP grp)
{
	if ((grp >= DPU_MAX_GRP_NUM) || (grp < 0)) {
		CVI_TRACE_DPU(CVI_DBG_ERR, "DpuGrp(%d) exceeds Max(%d)\n", grp, DPU_MAX_GRP_NUM);
		return CVI_ERR_DPU_ILLEGAL_PARAM;
	}
	return CVI_SUCCESS;
}

static inline CVI_S32 CHECK_DPU_CHN_VALID(DPU_CHN DpuChn)
{
	if ((DpuChn >= DPU_MAX_CHN_NUM) || (DpuChn < 0)) {
		CVI_TRACE_DPU(CVI_DBG_ERR, "Chn(%d) invalid.\n", DpuChn);
		return CVI_ERR_DPU_ILLEGAL_PARAM;
	}
	return CVI_SUCCESS;
}

/*****************************************************************************
 * function : Create DPU group & enable channel.
 *****************************************************************************/
CVI_S32 SAMPLE_COMM_DPU_Init(DPU_GRP DpuGrp, DPU_GRP_ATTR_S *pstDpuGrpAttr,
			      DPU_CHN_ATTR_S *pstDPUChnAttr)
{
	DPU_CHN DPUChn;
	CVI_S32 s32Ret;
	SAMPLE_PRT("+ \n");

	if(pstDpuGrpAttr == NULL){
		SAMPLE_PRT(" pstDpuGrpAttr nullPtr!\n");
		return CVI_FAILURE;
	}

	if(pstDPUChnAttr == NULL){
		SAMPLE_PRT(" pstDpuGrpAttr nullPtr!\n");
		return CVI_FAILURE;
	}

	s32Ret = CHECK_DPU_GRP_VALID(DpuGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_DPU_CreateGrp(DpuGrp, pstDpuGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_DPU_CreateGrp(grp:%d) failed with %#x!\n", DpuGrp, s32Ret);
		return CVI_FAILURE;
	}

	if(pstDpuGrpAttr->bIsBtcostOut){
		DPUChn=1;
		s32Ret = CVI_DPU_SetChnAttr(DpuGrp, DPUChn, pstDPUChnAttr);

		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_DPU_SetChnAttr failed with %#x\n", s32Ret);
			CVI_DPU_DestroyGrp(DpuGrp);
			return CVI_FAILURE;
		}

		s32Ret = CVI_DPU_EnableChn(DpuGrp, DPUChn);

		if (s32Ret != CVI_SUCCESS) {
			CVI_DPU_DisableChn(DpuGrp, DPUChn);
			CVI_DPU_DestroyGrp(DpuGrp);
			SAMPLE_PRT("CVI_DPU_EnableChn failed with %#x\n", s32Ret);
			return CVI_FAILURE;
		}
	}else{
		DPUChn=0;
		s32Ret = CVI_DPU_SetChnAttr(DpuGrp, DPUChn, pstDPUChnAttr);

		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_DPU_SetChnAttr failed with %#x\n", s32Ret);
			CVI_DPU_DestroyGrp(DpuGrp);
			return CVI_FAILURE;
		}

		s32Ret = CVI_DPU_EnableChn(DpuGrp, DPUChn);

		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_DPU_EnableChn failed with %#x\n", s32Ret);
			CVI_DPU_DisableChn(DpuGrp, DPUChn);
			CVI_DPU_DestroyGrp(DpuGrp);
			return CVI_FAILURE;
		}
	}
	SAMPLE_PRT("- \n");
	return CVI_SUCCESS;
}

/*****************************************************************************
 * function : start dpu grp.
 *****************************************************************************/
CVI_S32 SAMPLE_COMM_DPU_Start(DPU_GRP DpuGrp)
{
	CVI_S32 s32Ret;
	SAMPLE_PRT("+ \n");

	s32Ret = CHECK_DPU_GRP_VALID(DpuGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_DPU_StartGrp(DpuGrp);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_DPU_StartGrp failed with %#x\n", s32Ret);
		return CVI_FAILURE;
	}
	SAMPLE_PRT("- \n");
	return CVI_SUCCESS;
}

/* SAMPLE_COMM_DPU_Stop: stop dpu grp
 *
 * DpuGrp: the DPU Grp to control
 * pabChnEnable: array of DPU CHN, stop if true.
 */
CVI_S32 SAMPLE_COMM_DPU_Stop(DPU_GRP DpuGrp)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	DPU_CHN DPUChn;
	DPU_GRP_ATTR_S stGrpAttr;
	DPUChn = 0;
	SAMPLE_PRT("+ \n");
	s32Ret = CHECK_DPU_GRP_VALID(DpuGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&stGrpAttr,0,sizeof(stGrpAttr));
	s32Ret = CVI_DPU_DisableChn(DpuGrp, DPUChn);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("DPU stop Grp %d channel %d failed! Please check param\n",
		DpuGrp, DPUChn);
		return CVI_FAILURE;
	}

	s32Ret = CVI_DPU_GetGrpAttr(DpuGrp,&stGrpAttr);
	if(stGrpAttr.bIsBtcostOut){
		DPUChn = 0;
		s32Ret = CVI_DPU_DisableChn(DpuGrp, DPUChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("DPU stop Grp %d channel %d failed! Please check param\n",
			DpuGrp, DPUChn);
			return CVI_FAILURE;
		}
	}

	s32Ret = CVI_DPU_StopGrp(DpuGrp);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("DPU Stop Grp %d failed! Please check param\n", DpuGrp);
		return CVI_FAILURE;
	}

	s32Ret = CVI_DPU_DestroyGrp(DpuGrp);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("DPU Destroy Grp %d failed! Please check\n", DpuGrp);
		return CVI_FAILURE;
	}
	SAMPLE_PRT("- \n");
	return CVI_SUCCESS;
}

/* SAMPLE_COMM_DPU_SendFrame:
 *   send frame, whose data loaded from given filename.
 *
 * DpuGrp: the DPU Grp to control.
 * stSize: size of image.
 * enPixelFormat: format of image
 * filename: file to read.
 */
static CVI_S32 SAMPLE_COMM_DPU_FileToFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	VB_BLK blk;
	CVI_U32 u32len;
	VB_CAL_CONFIG_S stVbCalConfig;
	FILE *fp;
	VB_POOL_CONFIG_S pstVbPoolCfg ;
	VB_POOL pool_id;;
	SAMPLE_PRT("+ \n");

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
	stVideoFrame.stVFrame.u32TimeRef = 0;
	stVideoFrame.stVFrame.u64PTS = 0;
	stVideoFrame.stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;
	SAMPLE_PRT("Format(%d) Width(%d) Height(%d) Stride(%d)\n",stVideoFrame.stVFrame.enPixelFormat,\
				stVideoFrame.stVFrame.u32Width,stVideoFrame.stVFrame.u32Height,stVideoFrame.stVFrame.u32Stride[0]);

	memset(&pstVbPoolCfg, 0, sizeof(pstVbPoolCfg));
	pstVbPoolCfg.u32BlkCnt = 1;
	pstVbPoolCfg.u32BlkSize = stVbCalConfig.u32VBSize;
	pstVbPoolCfg.enRemapMode = VB_REMAP_MODE_NOCACHE;
	pool_id = CVI_VB_CreatePool(&pstVbPoolCfg);
	if(pool_id == VB_INVALID_POOLID){
		SAMPLE_PRT("BLK_cnt(%d) BLK_size(%d) BLK_id(%d) creat pool failed!\n",pstVbPoolCfg.u32BlkCnt,pstVbPoolCfg.u32BlkSize,pool_id);
		return CVI_FAILURE;
	}

	blk = CVI_VB_GetBlock(pool_id, stVbCalConfig.u32VBSize);
	if (blk == VB_INVALID_HANDLE) {
		SAMPLE_PRT("CVI_VB_GetBlock fail\n");
		return CVI_FAILURE;
	}

	//open data file & fread into the mmap address
	fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		SAMPLE_PRT("open data file error\n");
		CVI_VB_ReleaseBlock(blk);
		return CVI_FAILURE;
	}

	stVideoFrame.u32PoolId = CVI_VB_Handle2PoolId(blk);
	stVideoFrame.stVFrame.u32Length[0] = stVbCalConfig.u32MainYSize;
	stVideoFrame.stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);

	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		if (stVideoFrame.stVFrame.u32Length[i] == 0)
			continue;
		stVideoFrame.stVFrame.pu8VirAddr[i]
			= CVI_SYS_MmapCache(stVideoFrame.stVFrame.u64PhyAddr[i], stVideoFrame.stVFrame.u32Length[i]);

		u32len = fread(stVideoFrame.stVFrame.pu8VirAddr[i], stVideoFrame.stVFrame.u32Length[i], 1, fp);
		if (u32len <= 0) {
			SAMPLE_PRT("dpu send frame: fread plane%d error\n", i);
			fclose(fp);
			CVI_VB_ReleaseBlock(blk);
			return CVI_FAILURE;
		}
		CVI_SYS_IonFlushCache(stVideoFrame.stVFrame.u64PhyAddr[i],
					   stVideoFrame.stVFrame.pu8VirAddr[i],
					   stVideoFrame.stVFrame.u32Length[i]);
	}

	SAMPLE_PRT("length of buffer(%d)\n", stVideoFrame.stVFrame.u32Length[0]);
	SAMPLE_PRT("phy addr(%#"PRIx64")\n", stVideoFrame.stVFrame.u64PhyAddr[0]);
	SAMPLE_PRT("vir addr(%p)\n", stVideoFrame.stVFrame.pu8VirAddr[0]);

	fclose(fp);

	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		if (stVideoFrame.stVFrame.u32Length[i] == 0)
			continue;
		CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[i], stVideoFrame.stVFrame.u32Length[i]);
	}
	memcpy(pstVideoFrame, &stVideoFrame, sizeof(stVideoFrame));
	SAMPLE_PRT("- \n");
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_DPU_SendFrame(DPU_GRP DpuGrp ,VIDEO_FRAME_INFO_S *pstVideoFrameL,VIDEO_FRAME_INFO_S *pstVideoFrameR)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	// VB_BLK vb_left;
	// VB_BLK vb_right;
	SAMPLE_PRT("+ \n");
	s32Ret = CVI_DPU_SendFrame(DpuGrp,pstVideoFrameL,pstVideoFrameR,1000);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("DPU Grp(%d) Send Frame  failed! \n", DpuGrp);
		return CVI_FAILURE;
	}

	// vb_left = CVI_VB_PhysAddr2Handle(pstVideoFrameL->stVFrame.u64PhyAddr[0]);
	// if(CVI_VB_ReleaseBlock(vb_left)!=CVI_SUCCESS)
	// 	SAMPLE_PRT("ReleaseBlock blk_left fail !\n");
	// vb_right = CVI_VB_PhysAddr2Handle(pstVideoFrameR->stVFrame.u64PhyAddr[0]);
	// if(CVI_VB_ReleaseBlock(vb_right)!=CVI_SUCCESS)
	// 	SAMPLE_PRT("ReleaseBlock blk_right fail !\n");

	SAMPLE_PRT("- \n");
	return s32Ret;
}

CVI_S32 SAMPLE_COMM_DPU_SendFrame_FromFile(DPU_GRP DpuGrp ,SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, CVI_CHAR *filenameL,CVI_CHAR *filenameR)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VIDEO_FRAME_INFO_S stVideoFrameL;
	VIDEO_FRAME_INFO_S stVideoFrameR;
	s32Ret = CHECK_DPU_GRP_VALID(DpuGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	if(filenameL == NULL){
		SAMPLE_PRT("filenameL nullprt !\n");
		return CVI_FAILURE;
	}

	if(filenameR == NULL){
		SAMPLE_PRT("filenameR nullprt !\n");
		return CVI_FAILURE;
	}

	// memset(pstVideoFrameL,0,sizeof(*pstVideoFrameL));
	// memset(pstVideoFrameR,0,sizeof(*pstVideoFrameR));

	s32Ret = SAMPLE_COMM_DPU_FileToFrame(stSize,enPixelFormat,filenameL,&stVideoFrameL);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Left FileToFrame  failed! \n");
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_DPU_FileToFrame(stSize,enPixelFormat,filenameR,&stVideoFrameR);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Right FileToFrame  failed! \n");
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_DPU_SendFrame(DpuGrp,&stVideoFrameL,&stVideoFrameR);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SendFrame  failed! \n");
		return s32Ret;
	}
	SAMPLE_PRT("- \n");
	return s32Ret;
}

CVI_S32 SAMPLE_COMM_DPU_GetFrame(DPU_GRP DpuGrp ,DPU_CHN DpuChn,VIDEO_FRAME_INFO_S *pstVideoFrameOut)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	SAMPLE_PRT("+ \n");
	s32Ret = CVI_DPU_GetFrame(DpuGrp,DpuChn,pstVideoFrameOut,TIMEOUT_GET_FRAME);
	if (s32Ret != CVI_SUCCESS)
		SAMPLE_PRT("CVI_DPU_GetFrame fail. s32Ret: 0x%x !\n", s32Ret);

	SAMPLE_PRT("- \n");
	return s32Ret;
}

CVI_S32 SAMPLE_COMM_DPU_GetFrameToFile(DPU_GRP DpuGrp ,DPU_CHN DpuChn,CVI_CHAR *filename)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	FILE *fp;
	CVI_U32 u32len, u32DataLen;
	VIDEO_FRAME_INFO_S stVideoFrame;
	SAMPLE_PRT("+ \n");
	memset(&stVideoFrame,0,sizeof(stVideoFrame));
	SAMPLE_PRT("1 \n");
	s32Ret = CVI_DPU_GetFrame(DpuGrp,DpuChn,&stVideoFrame,TIMEOUT_GET_FRAME);
	if (s32Ret != CVI_SUCCESS){
		SAMPLE_PRT("CVI_DPU_GetFrame fail. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}
	SAMPLE_PRT("2 \n");
	fp = fopen(filename, "w");
	if (fp == CVI_NULL) {
		SAMPLE_PRT("open data file error\n");
		s32Ret = CVI_DPU_ReleaseFrame(DpuGrp,0,&stVideoFrame);
		if(s32Ret != CVI_SUCCESS)
			SAMPLE_PRT("Grp(%d) release frame fail\n", DpuGrp);
		return CVI_FAILURE;
	}
	SAMPLE_PRT("3 \n");
	for (int i = 0; i < 1; ++i) {
		u32DataLen = stVideoFrame.stVFrame.u32Length[i];
		if (u32DataLen == 0)
			continue;
		if (i > 0 && ((stVideoFrame.stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
			(stVideoFrame.stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) ||
			(stVideoFrame.stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)))
			u32DataLen >>= 1;

		stVideoFrame.stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[i], stVideoFrame.stVFrame.u32Length[i]);
		CVI_SYS_IonInvalidateCache(stVideoFrame.stVFrame.u64PhyAddr[i],
					   stVideoFrame.stVFrame.pu8VirAddr[i],
					   stVideoFrame.stVFrame.u32Length[i]);
		SAMPLE_PRT("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d)\n",
			   i, stVideoFrame.stVFrame.u64PhyAddr[i],
			   stVideoFrame.stVFrame.pu8VirAddr[i],
			   stVideoFrame.stVFrame.u32Stride[i]);
		SAMPLE_PRT(" data_len(%d) plane_len(%d)\n",
			      u32DataLen, stVideoFrame.stVFrame.u32Length[i]);
		u32len = fwrite(stVideoFrame.stVFrame.pu8VirAddr[i], u32DataLen, 1, fp);

		if (u32len <= 0) {
			SAMPLE_PRT("fwrite data(%d) error\n", i);
			s32Ret = CVI_DPU_ReleaseFrame(DpuGrp,0,&stVideoFrame);
			if(s32Ret != CVI_SUCCESS)
				SAMPLE_PRT("Grp(%d) release frame fail\n", DpuGrp);
			s32Ret = CVI_FAILURE;
			break;
		}
		CVI_SYS_Munmap(stVideoFrame.stVFrame.pu8VirAddr[i], stVideoFrame.stVFrame.u32Length[i]);
	}

	fclose(fp);
	s32Ret = CVI_DPU_ReleaseFrame(DpuGrp,0,&stVideoFrame);
	if(s32Ret != CVI_SUCCESS){
		SAMPLE_PRT("Grp(%d) release frame fail\n", DpuGrp);
		return s32Ret;
	}
	SAMPLE_PRT("- \n");
	return s32Ret;
}

CVI_S32 DPU_PrepareFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VB_BLK blk;
	VB_CAL_CONFIG_S stVbCalConfig;

	if (pstVideoFrame == CVI_NULL) {
		SAMPLE_PRT("Null pointer!\n");
		return CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(stSize->u32Width, stSize->u32Height, enPixelFormat, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DWA_STRIDE_ALIGN, &stVbCalConfig);
	//COMMON_GetPicBufferConfig(stSize->u32Width, stSize->u32Height, enPixelFormat, DATA_BITWIDTH_8
		//, COMPRESS_MODE_NONE, 1, &stVbCalConfig);

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
		SAMPLE_PRT("Can't acquire vb block\n");
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

#if DPU_EXT_OP_CLR_VB
	for (int i = 0; i < 3; ++i) {
		if (pstVideoFrame->stVFrame.u32Length[i] == 0)
			continue;
		pstVideoFrame->stVFrame.pu8VirAddr[i] = CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
		SAMPLE_PRT("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d) plane_len(%d)\n", i
			, pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.pu8VirAddr[i]
			, pstVideoFrame->stVFrame.u32Stride[i], pstVideoFrame->stVFrame.u32Length[i]);

		memset(pstVideoFrame->stVFrame.pu8VirAddr[i], 0, pstVideoFrame->stVFrame.u32Length[i]);
		CVI_SYS_IonFlushCache(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}
#endif

	return CVI_SUCCESS;
}


CVI_S32 DPU_FileToFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat,
		CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VB_BLK blk;
	CVI_U32 u32len;
	CVI_S32 Ret;
	int i;
	FILE *fp;

	if (!pstVideoFrame) {
		SAMPLE_PRT("pstVideoFrame is null\n");
		return CVI_FAILURE_ILLEGAL_PARAM;
	}

	if (!filename) {
		SAMPLE_PRT("filename is null\n");
		return CVI_FAILURE_ILLEGAL_PARAM;
	}

	Ret = DPU_PrepareFrame(stSize, enPixelFormat, pstVideoFrame);
	if (Ret != CVI_SUCCESS) {
		SAMPLE_PRT("DWA_COMM_PrepareFrame FAIL,get VB fail\n");
		return CVI_FAILURE;
	}

	blk = CVI_VB_PhysAddr2Handle(pstVideoFrame->stVFrame.u64PhyAddr[0]);

	//open data file & fread into the mmap address
	fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		SAMPLE_PRT("open data file[%s] error\n", filename);
		CVI_VB_ReleaseBlock(blk);
		return CVI_FAILURE;
	}

	for (i = 0; i < 3; ++i) {
		if (pstVideoFrame->stVFrame.u32Length[i] == 0)
			continue;
		pstVideoFrame->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
#if 0
		SAMPLE_PRT("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d) plane_len(%d)\n",
			   i, pstVideoFrame->stVFrame.u64PhyAddr[i],
			   pstVideoFrame->stVFrame.pu8VirAddr[i],
			   pstVideoFrame->stVFrame.u32Stride[i],
			   pstVideoFrame->stVFrame.u32Length[i]);
#endif
		u32len = fread(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i], 1, fp);
		if (u32len <= 0) {
			SAMPLE_PRT("file to frame: fread plane%d error\n", i);
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


CVI_S32 SAMPLE_COMM_DPU_DwaInit(VB_CONFIG_S *stVbConf)
{
	CVI_S32 s32Ret;
	s32Ret = CVI_VB_SetConfig(stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Init failed!\n");
		s32Ret |= CVI_VB_Exit();
		return s32Ret;
	}

	/************************************************
	 * step2:  Init DWA
	 ************************************************/
	s32Ret = CVI_DWA_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_DWA_Init failed!\n");
		s32Ret |= CVI_SYS_Exit();
		s32Ret |= CVI_VB_Exit();
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 SAMPLE_COMM_DPU_DwaStart(DWA_BASIC_PARAM *param)
{
	CVI_S32 s32Ret;
	LDC_ATTR_S LDCAttr;
	ROTATION_E enRotation;
	s32Ret = CVI_DWA_BeginJob(&param->hHandle);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_DWA_BeginJob failed!\n");
		goto exit2;
	}

	s32Ret = CVI_DWA_SetJobIdentity(param->hHandle, &param->identity);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_DWA_SetJobIdentity failed!\n");
		goto exit2;
	}

	memcpy(&LDCAttr,&param->LDCAttr,sizeof(LDCAttr));
	enRotation = 0;
	s32Ret = CVI_DWA_AddLDCTask(param->hHandle, &param->stTask, &LDCAttr, enRotation);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("dwa_basic_add_tsk. s32Ret: 0x%x !\n", s32Ret);
		goto exit2;
	}

	s32Ret = CVI_DWA_EndJob(param->hHandle);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_DWA_EndJob failed!\n");
		goto exit2;
	}

	SAMPLE_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param->stTask.stImgIn.stVFrame.u64PhyAddr[0]
		, param->stTask.stImgIn.stVFrame.u64PhyAddr[1], param->stTask.stImgIn.stVFrame.u64PhyAddr[2]);
	SAMPLE_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", param->stTask.stImgOut.stVFrame.u64PhyAddr[0]
		, param->stTask.stImgOut.stVFrame.u64PhyAddr[1], param->stTask.stImgOut.stVFrame.u64PhyAddr[2]);

	return s32Ret;

exit2:
	if (s32Ret != CVI_SUCCESS)
		if (param->hHandle)
			s32Ret |= CVI_DWA_CancelJob(param->hHandle);
	s32Ret |= CVI_DWA_DeInit();

	SAMPLE_PRT("CVI_DWA_DeInit fail.\n");


	// param->inBlk = CVI_VB_PhysAddr2Handle(param->stTask.stImgIn.stVFrame.u64PhyAddr[0]);
	// if (param->inBlk != VB_INVALID_HANDLE) {
	// 	s32Ret |= CVI_VB_ReleaseBlock(param->inBlk);
	// 	param->inBlk = VB_INVALID_HANDLE;
	// }
	// param->outBlk = CVI_VB_PhysAddr2Handle(param->stTask.stImgOut.stVFrame.u64PhyAddr[0]);
	// if (param->outBlk != VB_INVALID_HANDLE) {
	// 	s32Ret |= CVI_VB_ReleaseBlock(param->outBlk);
	// 	param->outBlk = VB_INVALID_HANDLE;
	// }

	return -1;
}
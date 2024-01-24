#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/prctl.h>
#include <inttypes.h>

#include "cvi_buffer.h"
#include "cvi_dpu.h"

#include "../common/sample_comm.h"
#include "sample_dpu.h"
#define ALIGN_16 16
#define ALIGN_32 32
#define DWA_LEFT_IN "dwa_left_in.bin"
#define DWA_RIGHT_IN "dwa_right_in.bin"
#define DWA_LEFT_OUT "dwa_left_out.bin"
#define DWA_RIGHT_OUT "dwa_right_out.bin"
#define DPU_LEFT_IN "dpu_left_in.bin"
#define DPU_RIGHT_IN "dpu_right_in.bin"

static DPU_GRP_ATTR_S grp_attr;
static DPU_CHN_ATTR_S chn_attr;
static DPU_GRP dpuGrp;
static pthread_t t1;
static pthread_t t2;
static DPU_DWA_BASIC_TEST_PARAM gParam;

static CVI_S32 threadDwa(void *arg)
{
	DWA_BASIC_PARAM *param = (DWA_BASIC_PARAM *)arg;
	if(0 != SAMPLE_COMM_DPU_DwaStart(param)){
		CVI_TRACE_LOG(CVI_DBG_ERR, "DPU_DwaStart failed!\n");
		return -1;
	}

	return 0;
}

// static CVI_S32 threadDwa(DWA_BASIC_PARAM *param)
// {
// 	//DWA_BASIC_PARAM *param = (DWA_BASIC_PARAM *)arg;
// 	if(0 != SAMPLE_COMM_DPU_DwaStart(param)){
// 		CVI_TRACE_LOG(CVI_DBG_ERR, "DPU_DwaStart failed!\n");
// 		return -1;
// 	}

// 	return 0;
// }


static CVI_S32 DPU_SaveFileFromFrame(VIDEO_FRAME_INFO_S *stVideoFrame,CVI_CHAR *filename)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	FILE *fp;
	CVI_U32 u32len, u32DataLen;
	fp = fopen(filename, "w");
	if (fp == CVI_NULL) {
		SAMPLE_PRT("open data file error\n");
		return CVI_FAILURE;
	}
	for (int i = 0; i < 3; ++i) {
		if(stVideoFrame->stVFrame.u32Length[i] ==0)
			continue;
		u32DataLen = stVideoFrame->stVFrame.u32Length[i];
		if (u32DataLen == 0)
			continue;
		if (i > 0 && ((stVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
			(stVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) ||
			(stVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)))
			u32DataLen >>= 1;

		stVideoFrame->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(stVideoFrame->stVFrame.u64PhyAddr[i], stVideoFrame->stVFrame.u32Length[i]);
		CVI_SYS_IonInvalidateCache(stVideoFrame->stVFrame.u64PhyAddr[i],
					   stVideoFrame->stVFrame.pu8VirAddr[i],
					   stVideoFrame->stVFrame.u32Length[i]);
		SAMPLE_PRT("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d)\n",
			   i, stVideoFrame->stVFrame.u64PhyAddr[i],
			   stVideoFrame->stVFrame.pu8VirAddr[i],
			   stVideoFrame->stVFrame.u32Stride[i]);
		SAMPLE_PRT(" data_len(%d) plane_len(%d)\n",
			      u32DataLen, stVideoFrame->stVFrame.u32Length[i]);
		u32len = fwrite(stVideoFrame->stVFrame.pu8VirAddr[i], u32DataLen, 1, fp);

		if (u32len <= 0) {
			SAMPLE_PRT("fwrite data(%d) error\n", i);
			s32Ret = CVI_FAILURE;
			break;
		}
		CVI_SYS_Munmap(stVideoFrame->stVFrame.pu8VirAddr[i], stVideoFrame->stVFrame.u32Length[i]);
	}

	fclose(fp);
	return s32Ret;
}

static CVI_S32 SAMPLE_DPU_SYS_Init(VB_CONFIG_S *pstVbConfig)
{
	CVI_S32 s32Ret = CVI_FAILURE;

	CVI_SYS_Exit();
	CVI_VB_Exit();

	if (pstVbConfig == NULL) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "input parameter is null, it is invaild!\n");
		return CVI_FAILURE;
	}

	s32Ret = CVI_VB_SetConfig(pstVbConfig);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_Init failed!\n");
		CVI_VB_Exit();
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_VOID SAMPLE_DPU_SYS_Exit(void)
{
	CVI_SYS_Exit();
	CVI_VB_Exit();
}

CVI_VOID SAMPLE_DPU_SetAttr(SIZE_S *stSize)
{
	grp_attr.enDpuMode = DPU_MODE_SGBM_MUX2;
	grp_attr.enMaskMode= DPU_MASK_MODE_7x7;
	grp_attr.enDispRange = DPU_DISP_RANGE_64;
	grp_attr.u16DispStartPos = 0;
	grp_attr.u32Rshift1 = 0;
	grp_attr.u32Rshift2 = 2;
	grp_attr.u32CaP1 = 1800;
	grp_attr.u32CaP2 = 14400;
	grp_attr.u32UniqRatio = 25;
	grp_attr.u32DispShift = 4;
	grp_attr.u32CensusShift = 1;
	grp_attr.u32FxBaseline = 864000;
	grp_attr.enDccDir = DPU_DCC_DIR_A14;
	grp_attr.u32FgsMaxCount = 19;
	grp_attr.u32FgsMaxT = 3;
	grp_attr.enDpuDepthUnit = DPU_DEPTH_UNIT_MM;
	grp_attr.bIsBtcostOut = false;

	memcpy(&grp_attr.stLeftImageSize,stSize,sizeof(grp_attr.stLeftImageSize));
	memcpy(&grp_attr.stRightImageSize,stSize,sizeof(grp_attr.stRightImageSize));
	memcpy(&chn_attr.stImgSize,stSize,sizeof(chn_attr.stImgSize));

}

CVI_S32 SAMPLE_DPU_BASE( SIZE_S stSize , CVI_CHAR *filenameL,CVI_CHAR *filenameR,CVI_CHAR *filenameOut)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    DPU_GRP dpuGrp = 0;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSizeIn_left,u32BlkSizeOut;
	CVI_U32 u32BlkSizeOut16,u32BlkSizeOut_btcost;
	grp_attr.stLeftImageSize.u32Width = stSize.u32Width;
	grp_attr.stLeftImageSize.u32Height = stSize.u32Height;
	grp_attr.stRightImageSize.u32Width = stSize.u32Width;
	grp_attr.stRightImageSize.u32Height = stSize.u32Height;
	chn_attr.stImgSize.u32Width = stSize.u32Width;
	chn_attr.stImgSize.u32Height = stSize.u32Height;
    memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
    u32BlkSizeIn_left = COMMON_GetPicBufferSize(grp_attr.stLeftImageSize.u32Width, grp_attr.stLeftImageSize.u32Height,
		PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);

    if(	grp_attr.enDpuMode == DPU_MODE_DEFAULT ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_MUX0 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_MUX2 ){

		stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn_left;
		stVbConf.astCommPool[0].u32BlkCnt	= 2;
		stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;

		u32BlkSizeOut = COMMON_GetPicBufferSize(grp_attr.stLeftImageSize.u32Width, grp_attr.stLeftImageSize.u32Height,
		PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);
		stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
		stVbConf.astCommPool[1].u32BlkCnt	= 1;
		stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
		SAMPLE_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn_left);
		if(grp_attr.bIsBtcostOut){
			u32BlkSizeOut_btcost = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width*128, chn_attr.stImgSize.u32Height,
			PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);
			stVbConf.astCommPool[2].u32BlkSize	= u32BlkSizeOut_btcost;
			stVbConf.astCommPool[2].u32BlkCnt	= 1;
			stVbConf.astCommPool[2].enRemapMode	= VB_REMAP_MODE_CACHED;
			SAMPLE_PRT("common pool[2] BlkSize %d\n", u32BlkSizeOut_btcost);
			stVbConf.u32MaxPoolCnt              = 3;
		}else{
			stVbConf.u32MaxPoolCnt              = 2;
		}

	}else if(grp_attr.enDpuMode == DPU_MODE_FGS_MUX0 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX0  ){

		stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn_left;
		stVbConf.astCommPool[0].u32BlkCnt	= 2;
		stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;

		u32BlkSizeOut = COMMON_GetPicBufferSize(grp_attr.stLeftImageSize.u32Width, grp_attr.stLeftImageSize.u32Height,
		PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);
		stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut;
		stVbConf.astCommPool[1].u32BlkCnt	= 1;
		stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
		SAMPLE_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn_left);

		if(grp_attr.bIsBtcostOut){
			u32BlkSizeOut_btcost = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width*128, chn_attr.stImgSize.u32Height,
			PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);
			stVbConf.astCommPool[2].u32BlkSize	= u32BlkSizeOut_btcost;
			stVbConf.astCommPool[2].u32BlkCnt	= 1;
			stVbConf.astCommPool[2].enRemapMode	= VB_REMAP_MODE_CACHED;
			SAMPLE_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut_btcost);
			stVbConf.u32MaxPoolCnt              = 3;
		}else{
			stVbConf.u32MaxPoolCnt              = 2;
		}
	}else if(grp_attr.enDpuMode == DPU_MODE_SGBM_MUX1 ){

		stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn_left;
		stVbConf.astCommPool[0].u32BlkCnt	= 2;
		stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
		SAMPLE_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn_left);

		u32BlkSizeOut16 = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width, chn_attr.stImgSize.u32Height,
		PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);
		stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut16*2;
		stVbConf.astCommPool[1].u32BlkCnt	= 1;
		stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
		SAMPLE_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut16*2);
		if(grp_attr.bIsBtcostOut){
			u32BlkSizeOut_btcost = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width*128, chn_attr.stImgSize.u32Height,
			PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);
			stVbConf.astCommPool[2].u32BlkSize	= u32BlkSizeOut_btcost;
			stVbConf.astCommPool[2].u32BlkCnt	= 1;
			stVbConf.astCommPool[2].enRemapMode	= VB_REMAP_MODE_CACHED;
			SAMPLE_PRT("common pool[2] BlkSize %d\n", u32BlkSizeOut_btcost);
			stVbConf.u32MaxPoolCnt              = 3;
		}else{
			stVbConf.u32MaxPoolCnt              = 2;
		}

	}else if(grp_attr.enDpuMode == DPU_MODE_FGS_MUX1 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX1 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX2 ){
		stVbConf.astCommPool[0].u32BlkSize	= u32BlkSizeIn_left;
		stVbConf.astCommPool[0].u32BlkCnt	= 2;
		stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
		SAMPLE_PRT("common pool[0] BlkSize %d\n", u32BlkSizeIn_left);

		u32BlkSizeOut16 = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width, chn_attr.stImgSize.u32Height,
		PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_32);
		stVbConf.astCommPool[1].u32BlkSize	= u32BlkSizeOut16*2;
		stVbConf.astCommPool[1].u32BlkCnt	= 1;
		stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
		SAMPLE_PRT("common pool[1] BlkSize %d\n", u32BlkSizeOut16*2);
		if(grp_attr.bIsBtcostOut){
			u32BlkSizeOut_btcost = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width*128, chn_attr.stImgSize.u32Height,
			PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);
			stVbConf.astCommPool[2].u32BlkSize	= u32BlkSizeOut_btcost;
			stVbConf.astCommPool[2].u32BlkCnt	= 1;
			stVbConf.astCommPool[2].enRemapMode	= VB_REMAP_MODE_CACHED;
			SAMPLE_PRT("common pool[2] BlkSize %d\n", u32BlkSizeOut_btcost);
			stVbConf.u32MaxPoolCnt              = 3;
		}else{
			stVbConf.u32MaxPoolCnt              = 2;
		}
	}

	s32Ret = SAMPLE_DPU_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SYS INIT failed!\n");
			SAMPLE_DPU_SYS_Exit();
			return CVI_FAILURE;
	}

    // s32Ret = SAMPLE_DPU_SYS_Init();
    // if (s32Ret != CVI_SUCCESS) {
	// 	SAMPLE_PRT("SYS INIT failed!\n");
    //     SAMPLE_DPU_SYS_Exit();
	// 	return CVI_FAILURE;
	// }

    s32Ret = SAMPLE_COMM_DPU_Init(dpuGrp, &grp_attr,&chn_attr);
    if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("DPU INIT failed!\n");
        SAMPLE_COMM_DPU_Stop(dpuGrp);
        SAMPLE_DPU_SYS_Exit();
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_DPU_Start(dpuGrp);
    if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("DPU Start failed!\n");
        SAMPLE_COMM_DPU_Stop(dpuGrp);
        SAMPLE_DPU_SYS_Exit();
		return CVI_FAILURE;
	}

    s32Ret = SAMPLE_COMM_DPU_SendFrame_FromFile(dpuGrp ,&stSize, PIXEL_FORMAT_YUV_400, filenameL,filenameR);
    if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Send Frame failed!\n");
        SAMPLE_COMM_DPU_Stop(dpuGrp);
        SAMPLE_DPU_SYS_Exit();
		return CVI_FAILURE;
	}

    if(!grp_attr.bIsBtcostOut)
        s32Ret = SAMPLE_COMM_DPU_GetFrameToFile(dpuGrp,0,filenameOut);
    else
        s32Ret = SAMPLE_COMM_DPU_GetFrameToFile(dpuGrp,1,filenameOut);

    if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Grp(%d) Chn(%d) Get Frame failed!\n",dpuGrp,grp_attr.bIsBtcostOut);
        SAMPLE_COMM_DPU_Stop(dpuGrp);
        SAMPLE_DPU_SYS_Exit();
		return CVI_FAILURE;
	}

    SAMPLE_COMM_DPU_Stop(dpuGrp);
    SAMPLE_DPU_SYS_Exit();
    return s32Ret;

}

CVI_S32 SAMPLE_DPU_SGBM_MUX0(SIZE_S stSize , CVI_CHAR *filenameL,CVI_CHAR *filenameR,CVI_CHAR *filenameOut)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
	SAMPLE_DPU_SetAttr(&stSize);
    grp_attr.enDpuMode =DPU_MODE_SGBM_MUX0;
    s32Ret = SAMPLE_DPU_BASE( stSize , filenameL,filenameR,filenameOut);
    if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("sample dpu failed!\n");
		return CVI_FAILURE;
	}
    return s32Ret;

}
CVI_S32 SAMPLE_DPU_SGBM_MUX1(SIZE_S stSize , CVI_CHAR *filenameL,CVI_CHAR *filenameR,CVI_CHAR *filenameOut)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
	SAMPLE_DPU_SetAttr(&stSize);
    grp_attr.enDpuMode =DPU_MODE_SGBM_MUX1;
    s32Ret = SAMPLE_DPU_BASE( stSize , filenameL,filenameR,filenameOut);
    if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("sample dpu failed!\n");
		return CVI_FAILURE;
	}
    return s32Ret;
}
CVI_S32 SAMPLE_DPU_SGBM_MUX2(SIZE_S stSize , CVI_CHAR *filenameL,CVI_CHAR *filenameR,CVI_CHAR *filenameOut)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
	SAMPLE_DPU_SetAttr(&stSize);
    grp_attr.enDpuMode =DPU_MODE_SGBM_MUX2;
    s32Ret = SAMPLE_DPU_BASE( stSize , filenameL,filenameR,filenameOut);
    if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("sample dpu failed!\n");
		return CVI_FAILURE;
	}
    return s32Ret;
}

CVI_S32 SAMPLE_DPU_SGBM_MUX3(SIZE_S stSize , CVI_CHAR *filenameL,CVI_CHAR *filenameR,CVI_CHAR *filenameOut)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
	SAMPLE_DPU_SetAttr(&stSize);
    grp_attr.enDpuMode =DPU_MODE_SGBM_MUX2;
	grp_attr.bIsBtcostOut = 1;
    s32Ret = SAMPLE_DPU_BASE( stSize , filenameL,filenameR,filenameOut);
    if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("sample dpu failed!\n");
		return CVI_FAILURE;
	}
    return s32Ret;
}
CVI_S32 SAMPLE_DPU_ONLINE_MUX0(SIZE_S stSize , CVI_CHAR *filenameL,CVI_CHAR *filenameR,CVI_CHAR *filenameOut)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
	SAMPLE_DPU_SetAttr(&stSize);
    grp_attr.enDpuMode =DPU_MODE_SGBM_FGS_ONLINE_MUX0;
    s32Ret = SAMPLE_DPU_BASE( stSize , filenameL,filenameR,filenameOut);
    if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("sample dpu failed!\n");
		return CVI_FAILURE;
	}
    return s32Ret;
}
CVI_S32 SAMPLE_DPU_ONLINE_MUX1(SIZE_S stSize , CVI_CHAR *filenameL,CVI_CHAR *filenameR,CVI_CHAR *filenameOut)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
	SAMPLE_DPU_SetAttr(&stSize);
    grp_attr.enDpuMode =DPU_MODE_SGBM_FGS_ONLINE_MUX1;
    s32Ret = SAMPLE_DPU_BASE( stSize , filenameL,filenameR,filenameOut);
    if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("sample dpu failed!\n");
		return CVI_FAILURE;
	}
    return s32Ret;
}
CVI_S32 SAMPLE_DPU_ONLINE_MUX2(SIZE_S stSize , CVI_CHAR *filenameL,CVI_CHAR *filenameR,CVI_CHAR *filenameOut)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
	SAMPLE_DPU_SetAttr(&stSize);
    grp_attr.enDpuMode =DPU_MODE_SGBM_FGS_ONLINE_MUX2;
    s32Ret = SAMPLE_DPU_BASE( stSize , filenameL,filenameR,filenameOut);
    if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("sample dpu failed!\n");
		return CVI_FAILURE;
	}
    return s32Ret;
}
CVI_S32 SAMPLE_DPU_FGS_MUX0(SIZE_S stSize , CVI_CHAR *filenameL,CVI_CHAR *filenameR,CVI_CHAR *filenameOut)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
	SAMPLE_DPU_SetAttr(&stSize);
    grp_attr.enDpuMode =DPU_MODE_FGS_MUX0;
    s32Ret = SAMPLE_DPU_BASE( stSize , filenameL,filenameR,filenameOut);
    if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("sample dpu failed!\n");
		return CVI_FAILURE;
	}
    return s32Ret;
}
CVI_S32 SAMPLE_DPU_FGS_MUX1(SIZE_S stSize , CVI_CHAR *filenameL,CVI_CHAR *filenameR,CVI_CHAR *filenameOut)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
	SAMPLE_DPU_SetAttr(&stSize);
    grp_attr.enDpuMode =DPU_MODE_FGS_MUX1;
    s32Ret = SAMPLE_DPU_BASE( stSize , filenameL,filenameR,filenameOut);
    if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("sample dpu failed!\n");
		return CVI_FAILURE;
	}
    return s32Ret;
}

static CVI_S32 SAMPLE_DPU_DWA_OfflineMode(DPU_DWA_BASIC_TEST_PARAM *param)
{
	int s32Ret;
	void *res1=NULL;
	void *res2=NULL;
	s32Ret = pthread_create(&t1,NULL,(void  *)threadDwa,&param->dwaParam[0]);
	//s32Ret = threadDwa(&param->dwaParam[0]);
	if(s32Ret != CVI_SUCCESS){
		SAMPLE_PRT("pthread_create t1 failed!\n");
		return s32Ret;
	}

	s32Ret = pthread_create(&t2,NULL,(void  *)threadDwa,&param->dwaParam[1]);
	//s32Ret = threadDwa(&param->dwaParam[1]);
	if(s32Ret != CVI_SUCCESS){
		SAMPLE_PRT("pthread_create t2 failed!\n");
		return s32Ret;
	}

	s32Ret = pthread_join(t1,&res1);
	if(s32Ret != CVI_SUCCESS){
		SAMPLE_PRT("pthread_join t1 failed!\n");
		return s32Ret;
	}

	s32Ret = pthread_join(t2,&res2);
	if(s32Ret != CVI_SUCCESS){
		SAMPLE_PRT("pthread_join t1 failed!\n");
		return s32Ret;
	}

	memcpy(&param->stVideoFrameLeftOut,&param->dwaParam[0].stTask.stImgOut, sizeof(param->stVideoFrameLeftOut));
	memcpy(&param->stVideoFrameRightOut,&param->dwaParam[1].stTask.stImgOut, sizeof(param->stVideoFrameRightOut));

	s32Ret = DPU_SaveFileFromFrame(&param->dwaParam[0].stTask.stImgOut,DWA_LEFT_OUT);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("DPU_SaveFileFromFrame (DWA left out) failed!\n");
        goto EXIT1;
	}

	s32Ret = DPU_SaveFileFromFrame(&param->dwaParam[1].stTask.stImgOut,DWA_RIGHT_OUT);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("DPU_SaveFileFromFrame (DWA right out) failed!\n");
        goto EXIT1;
	}

	s32Ret = DPU_SaveFileFromFrame(&param->stVideoFrameLeftOut,DPU_LEFT_IN);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("DPU_SaveFileFromFrame (DPU left in) failed!\n");
        goto EXIT1;
	}

	s32Ret = DPU_SaveFileFromFrame(&param->stVideoFrameRightOut,DPU_RIGHT_IN);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("DPU_SaveFileFromFrame (DPU right in) failed!\n");
        goto EXIT1;
	}

	s32Ret = SAMPLE_COMM_DPU_Init(param->DpuGrp, &param->stDpuGrpAttr,
			      &param->stDPUChnAttr);
    if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("DPU INIT failed!\n");
        goto EXIT1;
	}

	s32Ret = SAMPLE_COMM_DPU_Start(param->DpuGrp);
    if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("DPU Start failed!\n");
        goto EXIT1;
	}

	s32Ret = SAMPLE_COMM_DPU_SendFrame(param->DpuGrp ,&param->stVideoFrameLeftOut,&param->stVideoFrameRightOut);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("DPU_SendFrame failed!\n");
        goto EXIT1;
	}

	if(!param->stDpuGrpAttr.bIsBtcostOut)
        s32Ret = SAMPLE_COMM_DPU_GetFrameToFile(dpuGrp,0,param->filename_out);
    else{
		s32Ret = SAMPLE_COMM_DPU_GetFrameToFile(dpuGrp,1,param->filename_out);
	}

	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Grp(%d) Chn(%d) Get Frame failed!\n",dpuGrp,grp_attr.bIsBtcostOut);
        goto EXIT1;
	}

	return s32Ret;

EXIT1:
	// SAMPLE_COMM_DPU_Stop(dpuGrp);
    // SAMPLE_DPU_SYS_Exit();
	return CVI_FAILURE;
}

CVI_S32 SAMPLE_DPU_DWA_TEST(SIZE_S stSizeIn , SIZE_S stSizeOut, CVI_CHAR *filenameL,CVI_CHAR *filenameR,CVI_CHAR *filenameOut,
							CVI_CHAR * fileGridInfoL,CVI_CHAR * fileGridInfoR)
{
	SAMPLE_PRT("SAMPLE_DPU_DWA_TEST +++++++++++++ \n");
	//int times = DPU_REPEAT_TIMES;
	int s32Ret = 0;
	VB_BLK blk_left_in;
	VB_BLK blk_left_out;
	VB_BLK blk_right_in;
	VB_BLK blk_right_out;
	// MESH_DATA_ALL_S meshDataL;
	// MESH_DATA_ALL_S meshDataR;
	// LDC_ATTR_S ldc_attr_l;
	// LDC_ATTR_S ldc_attr_r;
	if(!filenameL){
		SAMPLE_PRT("filenameL ptr null \n");
		return -1;
	}

	if(!filenameR){
		SAMPLE_PRT("filenameR ptr null \n");
		return -1;
	}

	if(!fileGridInfoL){
		SAMPLE_PRT("fileGridInfoL ptr null \n");
		return -1;
	}

	if(!fileGridInfoR){
		SAMPLE_PRT("fileGridInfoL ptr null \n");
		return -1;
	}

	memset(&gParam,0,sizeof(DPU_DWA_BASIC_TEST_PARAM));
	gParam.size_in.u32Width = stSizeIn.u32Width;
	gParam.size_in.u32Height = stSizeIn.u32Height;
	gParam.size_out.u32Width = stSizeOut.u32Width;
	gParam.size_out.u32Height = stSizeOut.u32Height;
	gParam.enPixelFormat = PIXEL_FORMAT_YUV_400;
	gParam.DpuGrp = 0;

	strcpy(gParam.dwaParam[0].identity.Name,"dpu_L_dwa_job");
	gParam.dwaParam[0].identity.syncIo = CVI_FALSE;
	gParam.dwaParam[0].identity.u32ID = 0;
	// gParam.dwaParam[0].identity.enModId = CVI_ID_USER;

	strcpy(gParam.dwaParam[1].identity.Name,"dpu_R_dwa_job");
	gParam.dwaParam[1].identity.syncIo = CVI_TRUE;
	gParam.dwaParam[1].identity.u32ID = 1;
	// gParam.dwaParam[0].identity.enModId = CVI_ID_USER;
	SAMPLE_PRT("01 \n");
	// memset(&gParam.dwaParam[0].LDCAttr,0,sizeof(gParam.dwaParam[0].LDCAttr));
	// memset(&gParam.dwaParam[1].LDCAttr,0,sizeof(gParam.dwaParam[1].LDCAttr));

	strcpy(gParam.filename_left_in,filenameL);
	strcpy(gParam.filename_right_in,filenameR);
	strcpy(gParam.filename_out,filenameOut);
	// SAMPLE_PRT("001 \n");
	// memset(&meshDataL,0,sizeof(meshDataL));
	// memset(&meshDataR,0,sizeof(meshDataR));
	// init_meshdata(&meshDataL);
	// init_meshdata(&meshDataR);
	SAMPLE_PRT("02 \n");
	// load_meshdata(fileGridInfoL,&meshDataL,"dpu_imgL");
	// load_meshdata(fileGridInfoR,&meshDataR,"dpu_imgR");

	// save_meshdata("imgL", &meshDataL);
	// save_meshdata("imgR", &meshDataR);

	SAMPLE_PRT("03 \n");
	SAMPLE_DPU_SetAttr(&stSizeOut);
	memcpy(&gParam.stDpuGrpAttr,&grp_attr,sizeof(gParam.stDpuGrpAttr));
	memcpy(&gParam.stDPUChnAttr,&chn_attr,sizeof(gParam.stDPUChnAttr));

	SAMPLE_PRT("1 \n");
	memset(&gParam.dwaParam[0].LDCAttr,0,sizeof(LDC_ATTR_S));
	memset(&gParam.dwaParam[1].LDCAttr,0,sizeof(LDC_ATTR_S));

	gParam.dwaParam[0].LDCAttr.bAspect = CVI_TRUE;
	gParam.dwaParam[0].LDCAttr.s32XRatio = 0;
	gParam.dwaParam[0].LDCAttr.s32YRatio = 0;
	gParam.dwaParam[0].LDCAttr.s32XYRatio = 50;
	gParam.dwaParam[0].LDCAttr.s32CenterXOffset = 0;
	gParam.dwaParam[0].LDCAttr.s32CenterYOffset = 0;
	gParam.dwaParam[0].LDCAttr.s32DistortionRatio = 0;
	// gParam.dwaParam[0].LDCAttr.bGridInfo = CVI_TRUE;
	// gParam.dwaParam[0].LDCAttr.index = 0;

	gParam.dwaParam[1].LDCAttr.bAspect = CVI_TRUE;
	gParam.dwaParam[1].LDCAttr.s32XRatio = 0;
	gParam.dwaParam[1].LDCAttr.s32YRatio = 0;
	gParam.dwaParam[1].LDCAttr.s32XYRatio = 50;
	gParam.dwaParam[1].LDCAttr.s32CenterXOffset = 0;
	gParam.dwaParam[1].LDCAttr.s32CenterYOffset = 0;
	gParam.dwaParam[1].LDCAttr.s32DistortionRatio = 0;
	// gParam.dwaParam[1].LDCAttr.bGridInfo = CVI_TRUE;
	// gParam.dwaParam[1].LDCAttr.index = 1;
	SAMPLE_PRT("111 \n");
	gParam.dwaParam[0].stTask.reserved = 0;
	gParam.dwaParam[1].stTask.reserved = 0;
	memcpy(gParam.dwaParam[0].stTask.name,"dpu_L_dwa_tsk",128);
	memcpy(gParam.dwaParam[1].stTask.name,"dpu_R_dwa_tsk",128);

	gParam.dwaParam[0].LDCAttr.stGridInfoAttr.Enable = CVI_TRUE;
	strcpy(gParam.dwaParam[0].LDCAttr.stGridInfoAttr.gridFileName, fileGridInfoL);
	strcpy(gParam.dwaParam[0].LDCAttr.stGridInfoAttr.gridBindName, gParam.dwaParam[0].stTask.name);

	gParam.dwaParam[1].LDCAttr.stGridInfoAttr.Enable = CVI_TRUE;
	strcpy(gParam.dwaParam[1].LDCAttr.stGridInfoAttr.gridFileName, fileGridInfoR);
	strcpy(gParam.dwaParam[1].LDCAttr.stGridInfoAttr.gridBindName, gParam.dwaParam[1].stTask.name);

	// SAMPLE_PRT("dwaParam[0].LDCAttr.index(%d) \n",gParam.dwaParam[0].LDCAttr.index);
	// SAMPLE_PRT("dwaParam[1].LDCAttr.index(%d) \n",gParam.dwaParam[1].LDCAttr.index);
	SAMPLE_PRT("112 \n");
	// setGobalMeshData(&meshDataL,gParam.dwaParam[0].LDCAttr.index);
	// setGobalMeshData(&meshDataR,gParam.dwaParam[1].LDCAttr.index);
	// ldc_attr_l = {CVI_TRUE, 0, 0, 50, 0, 0, 0,CVI_TRUE,0};
	// ldc_attr_r = {CVI_TRUE, 0, 0, 50, 0, 0, 0,CVI_TRUE,1};
	// memcpy(&gParam.dwaParam[0].LDCAttr,&ldc_attr_l,sizeof(ldc_attr_l));
	// memcpy(&gParam.dwaParam[1].LDCAttr,&ldc_attr_r,sizeof(ldc_attr_r));
	/************************************************
	 * set vbConf
	 ************************************************/
	memset(&gParam.stVbConf, 0, sizeof(VB_CONFIG_S));
	SAMPLE_PRT("11 \n");
	gParam.u32BlkSizeIn = COMMON_GetPicBufferSize(gParam.size_in.u32Width, gParam.size_in.u32Height, gParam.enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, 32);

	gParam.u32BlkSizeOut_dwa = COMMON_GetPicBufferSize(gParam.size_out.u32Width, gParam.size_out.u32Height, gParam.enPixelFormat
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, 32);
	if(	grp_attr.enDpuMode == DPU_MODE_DEFAULT ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_MUX0 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_MUX2 ){
		if(stSizeIn.u32Width == stSizeOut.u32Width && stSizeIn.u32Height == stSizeOut.u32Height){
			gParam.stVbConf.astCommPool[0].u32BlkSize	= gParam.u32BlkSizeIn;
			gParam.stVbConf.astCommPool[0].u32BlkCnt	= 4;
			gParam.stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;

			gParam.u32BlkSizeOut = COMMON_GetPicBufferSize(grp_attr.stLeftImageSize.u32Width, grp_attr.stLeftImageSize.u32Height,
			PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);
			gParam.stVbConf.astCommPool[1].u32BlkSize	= gParam.u32BlkSizeOut;
			gParam.stVbConf.astCommPool[1].u32BlkCnt	= 1;
			gParam.stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
			SAMPLE_PRT("common pool[0] BlkSize %d\n", gParam.u32BlkSizeIn);
			SAMPLE_PRT("common pool[1] BlkSize %d\n", gParam.u32BlkSizeOut);
			if(grp_attr.bIsBtcostOut){
				gParam.u32BlkSizeOut_btcost = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width*128, chn_attr.stImgSize.u32Height,
				PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);
				gParam.stVbConf.astCommPool[2].u32BlkSize	= gParam.u32BlkSizeOut_btcost;
				gParam.stVbConf.astCommPool[2].u32BlkCnt	= 1;
				gParam.stVbConf.astCommPool[2].enRemapMode	= VB_REMAP_MODE_CACHED;
				SAMPLE_PRT("common pool[1] BlkSize %d\n", gParam.u32BlkSizeOut_btcost);
				gParam.stVbConf.u32MaxPoolCnt              = 3;
			}else{
				gParam.stVbConf.u32MaxPoolCnt              = 2;
			}
		}else{
			gParam.stVbConf.astCommPool[0].u32BlkSize	= gParam.u32BlkSizeIn;
			gParam.stVbConf.astCommPool[0].u32BlkCnt	= 2;
			gParam.stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;

			gParam.stVbConf.astCommPool[1].u32BlkSize	= gParam.u32BlkSizeOut_dwa;
			gParam.stVbConf.astCommPool[1].u32BlkCnt	= 3;
			gParam.stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
			SAMPLE_PRT("common pool[0] BlkSize %d\n", gParam.u32BlkSizeIn);
			SAMPLE_PRT("common pool[1] BlkSize %d\n", gParam.u32BlkSizeOut_dwa);
			if(grp_attr.bIsBtcostOut){
				gParam.u32BlkSizeOut_btcost = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width*128, chn_attr.stImgSize.u32Height,
				PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);
				gParam.stVbConf.astCommPool[2].u32BlkSize	= gParam.u32BlkSizeOut_btcost;
				gParam.stVbConf.astCommPool[2].u32BlkCnt	= 1;
				gParam.stVbConf.astCommPool[2].enRemapMode	= VB_REMAP_MODE_CACHED;
				SAMPLE_PRT("common pool[2] BlkSize %d\n", gParam.u32BlkSizeOut_btcost);
				gParam.stVbConf.u32MaxPoolCnt              = 3;
			}else{
				gParam.stVbConf.u32MaxPoolCnt              = 2;
			}
		}

	}else if(grp_attr.enDpuMode == DPU_MODE_FGS_MUX0 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX0  ){

		gParam.stVbConf.astCommPool[0].u32BlkSize	= gParam.u32BlkSizeIn;
		gParam.stVbConf.astCommPool[0].u32BlkCnt	= 4;
		gParam.stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;

		gParam.u32BlkSizeOut = COMMON_GetPicBufferSize(grp_attr.stLeftImageSize.u32Width, grp_attr.stLeftImageSize.u32Height,
		PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);
		gParam.stVbConf.astCommPool[1].u32BlkSize	= gParam.u32BlkSizeOut;
		gParam.stVbConf.astCommPool[1].u32BlkCnt	= 1;
		gParam.stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
		SAMPLE_PRT("common pool[0] BlkSize %d\n", gParam.u32BlkSizeIn);
		SAMPLE_PRT("common pool[1] BlkSize %d\n", gParam.u32BlkSizeOut);

		if(grp_attr.bIsBtcostOut){
			gParam.u32BlkSizeOut_btcost = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width*128, chn_attr.stImgSize.u32Height,
			PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);
			gParam.stVbConf.astCommPool[2].u32BlkSize	=gParam. u32BlkSizeOut_btcost;
			gParam.stVbConf.astCommPool[2].u32BlkCnt	= 1;
			gParam.stVbConf.astCommPool[2].enRemapMode	= VB_REMAP_MODE_CACHED;
			SAMPLE_PRT("common pool[1] BlkSize %d\n", gParam.u32BlkSizeOut_btcost);
			gParam.stVbConf.u32MaxPoolCnt              = 3;
		}else{
			gParam.stVbConf.u32MaxPoolCnt              = 2;
		}
	}else if(grp_attr.enDpuMode == DPU_MODE_SGBM_MUX1 ){

		gParam.stVbConf.astCommPool[0].u32BlkSize	= gParam.u32BlkSizeIn;
		gParam.stVbConf.astCommPool[0].u32BlkCnt	= 4;
		gParam.stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
		SAMPLE_PRT("common pool[0] BlkSize %d\n", gParam.u32BlkSizeIn);

		gParam.u32BlkSizeOut = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width, chn_attr.stImgSize.u32Height,
		PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);
		gParam.stVbConf.astCommPool[1].u32BlkSize	= gParam.u32BlkSizeOut*2;
		gParam.stVbConf.astCommPool[1].u32BlkCnt	= 1;
		gParam.stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
		SAMPLE_PRT("common pool[1] BlkSize %d\n", gParam.u32BlkSizeOut*2);
		if(grp_attr.bIsBtcostOut){
			gParam.u32BlkSizeOut_btcost = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width*128, chn_attr.stImgSize.u32Height,
			PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);
			gParam.stVbConf.astCommPool[2].u32BlkSize	= gParam.u32BlkSizeOut_btcost;
			gParam.stVbConf.astCommPool[2].u32BlkCnt	= 1;
			gParam.stVbConf.astCommPool[2].enRemapMode	= VB_REMAP_MODE_CACHED;
			SAMPLE_PRT("common pool[2] BlkSize %d\n",gParam. u32BlkSizeOut_btcost);
			gParam.stVbConf.u32MaxPoolCnt              = 3;
		}else{
			gParam.stVbConf.u32MaxPoolCnt              = 2;
		}

	}else if(grp_attr.enDpuMode == DPU_MODE_FGS_MUX1 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX1 ||
		grp_attr.enDpuMode == DPU_MODE_SGBM_FGS_ONLINE_MUX2 ){
		gParam.stVbConf.astCommPool[0].u32BlkSize	= gParam.u32BlkSizeIn;
		gParam.stVbConf.astCommPool[0].u32BlkCnt	= 4;
		gParam.stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
		SAMPLE_PRT("common pool[0] BlkSize %d\n", gParam.u32BlkSizeIn);

		gParam.u32BlkSizeOut = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width, chn_attr.stImgSize.u32Height,
		PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_32);
		gParam.stVbConf.astCommPool[1].u32BlkSize	= gParam.u32BlkSizeOut*2;
		gParam.stVbConf.astCommPool[1].u32BlkCnt	= 1;
		gParam.stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
		SAMPLE_PRT("common pool[1] BlkSize %d\n", gParam.u32BlkSizeOut*2);
		if(grp_attr.bIsBtcostOut){
			gParam.u32BlkSizeOut_btcost = COMMON_GetPicBufferSize(chn_attr.stImgSize.u32Width*128, chn_attr.stImgSize.u32Height,
			PIXEL_FORMAT_YUV_400, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, ALIGN_16);
			gParam.stVbConf.astCommPool[2].u32BlkSize	= gParam.u32BlkSizeOut_btcost;
			gParam.stVbConf.astCommPool[2].u32BlkCnt	= 1;
			gParam.stVbConf.astCommPool[2].enRemapMode	= VB_REMAP_MODE_CACHED;
			SAMPLE_PRT("common pool[2] BlkSize %d\n", gParam.u32BlkSizeOut_btcost);
			gParam.stVbConf.u32MaxPoolCnt              = 3;
		}else{
			gParam.stVbConf.u32MaxPoolCnt              = 2;
		}
	}else{
		SAMPLE_PRT("no this dpu mode! \n");
		return -1;
	}
	SAMPLE_PRT("12 \n");
	s32Ret = SAMPLE_DPU_SYS_Init(&gParam.stVbConf);
	if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SYS INIT failed!\n");
			goto EXIT5;
	}
	SAMPLE_PRT("13 \n");
	s32Ret = CVI_DWA_Init();
	if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("DWA INIT failed!\n");
			goto EXIT4;
	}
	SAMPLE_PRT("2 \n");
	gParam.dwaParam[0].hHandle = 0;
	memset(&gParam.stVideoFrameLeftIn, 0, sizeof(gParam.stVideoFrameLeftIn));
	s32Ret = DPU_FileToFrame(&gParam.size_in, gParam.enPixelFormat, gParam.filename_left_in, &gParam.stVideoFrameLeftIn);
	if (s32Ret) {
		SAMPLE_PRT("DWAFileToFrame left_in failed!\n");
		goto EXIT3;
	}
	SAMPLE_PRT("3 \n");
	memset(&gParam.stVideoFrameLeftOut, 0, sizeof(gParam.stVideoFrameLeftOut));
	s32Ret = DPU_PrepareFrame(&gParam.size_out, gParam.enPixelFormat, &gParam.stVideoFrameLeftOut);
	if (s32Ret) {
		SAMPLE_PRT("DWA_COMM_PrepareFrame left_out failed!\n");
		goto EXIT2;
	}

	memset(gParam.dwaParam[0].stTask.au64privateData, 0, sizeof(gParam.dwaParam[0].stTask.au64privateData));
	memcpy(&gParam.dwaParam[0].stTask.stImgIn, &gParam.stVideoFrameLeftIn, sizeof(gParam.stVideoFrameLeftIn));
	memcpy(&gParam.dwaParam[0].stTask.stImgOut, &gParam.stVideoFrameLeftOut, sizeof(gParam.stVideoFrameLeftOut));

	gParam.dwaParam[1].hHandle = 1;
	memset(&gParam.stVideoFrameRightIn, 0, sizeof(gParam.stVideoFrameRightIn));
	s32Ret = DPU_FileToFrame(&gParam.size_in, gParam.enPixelFormat, gParam.filename_right_in, &gParam.stVideoFrameRightIn);
	if (s32Ret) {
		SAMPLE_PRT("DWAFileToFrame right_in failed!\n");
		goto EXIT1;
	}

	memset(&gParam.stVideoFrameRightOut, 0, sizeof(gParam.stVideoFrameRightOut));
	s32Ret = DPU_PrepareFrame(&gParam.size_out, gParam.enPixelFormat, &gParam.stVideoFrameRightOut);
	if (s32Ret) {
		SAMPLE_PRT("DWA_COMM_PrepareFrame right_out failed!\n");
		goto EXIT0;
	}

	memset(gParam.dwaParam[1].stTask.au64privateData, 0, sizeof(gParam.dwaParam[1].stTask.au64privateData));
	memcpy(&gParam.dwaParam[1].stTask.stImgIn, &gParam.stVideoFrameRightIn, sizeof(gParam.stVideoFrameRightIn));
	memcpy(&gParam.dwaParam[1].stTask.stImgOut, &gParam.stVideoFrameRightOut, sizeof(gParam.stVideoFrameRightOut));

	s32Ret = DPU_SaveFileFromFrame(&gParam.dwaParam[0].stTask.stImgIn,DWA_LEFT_IN);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("DPU_SaveFileFromFrame (DWA left in) failed!\n");
        goto EXIT0;
	}

	s32Ret = DPU_SaveFileFromFrame(&gParam.dwaParam[1].stTask.stImgIn,DWA_RIGHT_IN);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("DPU_SaveFileFromFrame (DWA right im) failed!\n");
        goto EXIT0;
	}

	SAMPLE_PRT("4 \n");
	s32Ret = SAMPLE_DPU_DWA_OfflineMode(&gParam);
	if (s32Ret) {
		SAMPLE_PRT("SAMPLE_DPU_DWA_OfflineMode failed!\n");
		goto EXIT6;
	}

	SAMPLE_PRT("5 \n");
	/************************************************
	 * set LDCattr
	 ************************************************/

	// s32Ret = SAMPLE_COMM_DPU_Init(dpuGrp, &grp_attr,&chn_attr);
    // if (s32Ret != CVI_SUCCESS) {
	// 	SAMPLE_PRT("DPU INIT failed!\n");
    //     SAMPLE_COMM_DPU_Stop(dpuGrp);
    //     SAMPLE_DPU_SYS_Exit();
	// 	return CVI_FAILURE;
	// }
	// SAMPLE_PRT("6 \n");
	// s32Ret = SAMPLE_COMM_DPU_Start(dpuGrp);
    // if (s32Ret != CVI_SUCCESS) {
	// 	SAMPLE_PRT("DPU Start failed!\n");
    //     SAMPLE_COMM_DPU_Stop(dpuGrp);
    //     SAMPLE_DPU_SYS_Exit();
	// 	return CVI_FAILURE;
	// }
	// SAMPLE_PRT("7 \n");
	// memcpy(&gParam.stVideoFrameLeftOut,&gParam.dwaParam[0].stTask.stImgOut,sizeof(gParam.stVideoFrameLeftOut));
	// memcpy(&gParam.stVideoFrameRightOut,&gParam.dwaParam[1].stTask.stImgOut,sizeof(gParam.stVideoFrameRightOut));
    // s32Ret = SAMPLE_COMM_DPU_SendFrame(dpuGrp,&gParam.stVideoFrameLeftOut,&gParam.stVideoFrameRightOut);
    // if (s32Ret != CVI_SUCCESS) {
	// 	SAMPLE_PRT("Send Frame failed!\n");
    //     SAMPLE_COMM_DPU_Stop(dpuGrp);
    //     SAMPLE_DPU_SYS_Exit();
	// 	return CVI_FAILURE;
	// }
	// SAMPLE_PRT("8 \n");
    // if(!grp_attr.bIsBtcostOut)
    //     s32Ret = SAMPLE_COMM_DPU_GetFrameToFile(dpuGrp,0,filenameOut);
    // else
    //     s32Ret = SAMPLE_COMM_DPU_GetFrameToFile(dpuGrp,1,filenameOut);

    // if (s32Ret != CVI_SUCCESS) {
	// 	SAMPLE_PRT("Grp(%d) Chn(%d) Get Frame failed!\n",dpuGrp,grp_attr.bIsBtcostOut);
    //     SAMPLE_COMM_DPU_Stop(dpuGrp);
    //     SAMPLE_DPU_SYS_Exit();
	// 	return CVI_FAILURE;
	// }
EXIT6:
	SAMPLE_COMM_DPU_Stop(gParam.DpuGrp);
EXIT0:
	blk_right_out = CVI_VB_PhysAddr2Handle(gParam.stVideoFrameRightOut.stVFrame.u64PhyAddr[0]);
	CVI_VB_ReleaseBlock(blk_right_out);

EXIT1:
	blk_right_in = CVI_VB_PhysAddr2Handle(gParam.stVideoFrameRightIn.stVFrame.u64PhyAddr[0]);
	CVI_VB_ReleaseBlock(blk_right_in);

EXIT2:
	blk_left_out = CVI_VB_PhysAddr2Handle(gParam.stVideoFrameLeftOut.stVFrame.u64PhyAddr[0]);
	CVI_VB_ReleaseBlock(blk_left_out);

EXIT3:

	blk_left_in = CVI_VB_PhysAddr2Handle(gParam.stVideoFrameLeftIn.stVFrame.u64PhyAddr[0]);
	CVI_VB_ReleaseBlock(blk_left_in);

EXIT4:
	CVI_DWA_DeInit();
EXIT5:
    SAMPLE_DPU_SYS_Exit();
	SAMPLE_PRT("SAMPLE_DPU_DWA_TEST -------------- \n");
    return s32Ret;
}
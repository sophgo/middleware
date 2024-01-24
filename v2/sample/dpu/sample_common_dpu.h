#ifndef __SAMPLE_COMM_DPU_H__
#define __SAMPLE_COMM_DPU_H__
#include "cvi_dpu.h"
#include "cvi_dwa.h"
#include <linux/cvi_comm_dpu.h>
#include <linux/cvi_comm_video.h>
#include <pthread.h>

typedef struct _DWA_BASIC_PARAM {
	GDC_HANDLE hHandle;
	GDC_TASK_ATTR_S stTask;
	GDC_IDENTITY_ATTR_S identity;
	LDC_ATTR_S LDCAttr;
} DWA_BASIC_PARAM;

typedef struct _DPU_DWA_BASIC_TEST_PARAM {
	SIZE_S size_in;
	SIZE_S size_out;
	char filename_left_in[128];
	char filename_right_in[128];
	char filename_out[128];
	char filename_pef[128];
	CVI_U32 u32BlkSizeOut;
	CVI_U32 u32BlkSizeIn;
	CVI_U32 u32BlkSizeOut_btcost;
	CVI_U32 u32BlkSizeOut_dwa;
	VIDEO_FRAME_INFO_S stVideoFrameLeftIn;
	VIDEO_FRAME_INFO_S stVideoFrameRightIn;
	VIDEO_FRAME_INFO_S stVideoFrameLeftOut;
	VIDEO_FRAME_INFO_S stVideoFrameRightOut;
	VIDEO_FRAME_INFO_S stVideoFrameOut;
	VB_BLK leftInBlk, leftOutBlk;
	VB_BLK rightInBlk, rightOutBlk;
	VB_BLK rightOut;
	PIXEL_FORMAT_E enPixelFormat;
	VB_CONFIG_S stVbConf;
	DWA_BASIC_PARAM dwaParam[2];
	DPU_GRP DpuGrp;
	DPU_GRP_ATTR_S stDpuGrpAttr;
	DPU_CHN_ATTR_S stDPUChnAttr;
} DPU_DWA_BASIC_TEST_PARAM;

CVI_S32 SAMPLE_COMM_DPU_Init(DPU_GRP DpuGrp, DPU_GRP_ATTR_S *pstDpuGrpAttr,DPU_CHN_ATTR_S *pstDPUChnAttr);
CVI_S32 SAMPLE_COMM_DPU_Start(DPU_GRP DpuGrp);
CVI_S32 SAMPLE_COMM_DPU_Stop(DPU_GRP DpuGrp);
CVI_S32 SAMPLE_COMM_DPU_SendFrame(DPU_GRP DpuGrp ,VIDEO_FRAME_INFO_S *pstVideoFrameL,VIDEO_FRAME_INFO_S *pstVideoFrameR);
CVI_S32 SAMPLE_COMM_DPU_SendFrame_FromFile(DPU_GRP DpuGrp ,SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, CVI_CHAR *filenameL,CVI_CHAR *filenameR);
CVI_S32 SAMPLE_COMM_DPU_GetFrame(DPU_GRP DpuGrp ,DPU_CHN DpuChn,VIDEO_FRAME_INFO_S *pstVideoFrameOut);
CVI_S32 SAMPLE_COMM_DPU_GetFrameToFile(DPU_GRP DpuGrp ,DPU_CHN DpuChn,CVI_CHAR *filename);

CVI_S32 SAMPLE_COMM_DPU_DwaInit(VB_CONFIG_S *stVbConf);
CVI_S32 SAMPLE_COMM_DPU_DwaStart(DWA_BASIC_PARAM *param);
CVI_S32 DPU_FileToFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat,CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);
CVI_S32 DPU_PrepareFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, VIDEO_FRAME_INFO_S *pstVideoFrame);
#endif

#ifndef __DPU_UT_COMM_H__
#define __DPU_UT_COMM_H__
#include "cvi_type.h"
#define DPU_UT_PRT(fmt...)                               \
	do {                                                  \
		printf("[%s]-%d: ", __func__, __LINE__);          \
		printf(fmt);                                      \
	} while (0)


char * GetFmtName(PIXEL_FORMAT_E enPixFmt);
//CVI_S32 FileSendToDpu(DPU_GRP DpuGrp, SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, CVI_CHAR *filename);
CVI_S32 FileToFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat,
	char *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);

CVI_S32 FrameSaveToFile(const char *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);

CVI_S32 CompareWithFile(const CVI_CHAR *filenameSrc,const CVI_CHAR *filenameDest,CVI_S32 size);

void CVI_DPU_CheckRegWrite(void);

void CVI_DPU_CheckRegRead(void);

void CVI_DPU_CheckSgbmStatus(void);

void CVI_DPU_CheckFgsStatus(void);

#endif

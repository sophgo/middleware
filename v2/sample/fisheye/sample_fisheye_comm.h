#ifndef __SAMPLE_FISHEYE_COMM_H__
#define _SAMPLE_FISHEYE_COMM_H__

#define NONE	"\033[m"
#define RED	"\033[0;32;31m"
#define GREEN	"\033[0;32;32m"

#define CVI_DWA_MAGIC 0xbabeface

#define CVI_DWA_MESH_SIZE_ROT 0x60000
#define CVI_DWA_MESH_SIZE_AFFINE 0x20000
#define CVI_DWA_MESH_SIZE_FISHEYE 0xB0000

#define DWA_UT_PRT(fmt...)                               \
	do {                                                  \
		printf("[%s]-%d: ", __func__, __LINE__);          \
		printf(fmt);                                      \
	} while (0)

#define DWA_CHECK_RET(s32Ret) \
		do { \
			if (s32Ret == CVI_SUCCESS) \
				printf(GREEN"\n=== %s pass ===\n"NONE"\n", __func__); \
			else \
				printf(RED"\n=== %s fail ===\n"NONE"\n", __func__); \
		} while (0)

#define DWA_TEST_CHECK_RET(s32Ret) \
	do { \
		sleep(1); \
		if (s32Ret == CVI_SUCCESS) \
			printf(GREEN"\n=== %s pass ===\n"NONE"\n", __func__); \
		else \
			printf(RED"\n=== %s fail ===\n"NONE"\n", __func__); \
		sleep(1); \
	} while (0)

#ifndef MIN
#define MIN(a, b) (((a) < (b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b))?(a):(b))
#endif

#define DWA_STRIDE_ALIGN    32

CVI_CHAR * DWAGetFmtName(PIXEL_FORMAT_E enPixFmt);
CVI_S32 DWA_COMM_PrepareFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, VIDEO_FRAME_INFO_S *pstVideoFrame);
CVI_S32 DWAFileToFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat,
	CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);
CVI_S32 DWAFrameSaveToFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);
CVI_S32 DWACompareWithFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);

#endif

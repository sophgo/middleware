#ifndef __GDC_UT_COMM_H__
#define __GDC_UT_COMM_H__

#define NONE	"\033[m"
#define RED	"\033[0;32;31m"
#define GREEN	"\033[0;32;32m"

#define CVI_GDC_MAGIC 0xbabeface

#define CVI_GDC_MESH_SIZE_ROT 0x60000

#define GDC_UT_PRT(fmt...)                               \
	do {                                                  \
		printf("[%s]-%d: ", __func__, __LINE__);          \
		printf(fmt);                                      \
	} while (0)

#define GDC_CHECK_RET(s32Ret) \
		do { \
			if (s32Ret == CVI_SUCCESS) \
				printf(GREEN"\n=== %s pass ===\n"NONE"\n", __func__); \
			else \
				printf(RED"\n=== %s fail ===\n"NONE"\n", __func__); \
		} while (0)

#define GDC_TEST_CHECK_RET(s32Ret) \
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

#define GDC_STRIDE_ALIGN    64
#define DWA_STRIDE_ALIGN    32

CVI_CHAR * GDCGetFmtName(PIXEL_FORMAT_E enPixFmt);
CVI_S32 GDC_COMM_PrepareFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_BOOL bEnHwLDC);
CVI_S32 GDC_COMM_PrepareFrame2(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, VIDEO_FRAME_INFO_S *pstVideoFrame);
CVI_S32 GDCFileToFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_BOOL bEnHwLDC);
CVI_S32 GDCFileToFrame2(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);
CVI_S32 GDCFrameSaveToFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);
CVI_S32 GDCCompareWithFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);
CVI_S32 gdc_load_src_mesh_coordinate(const CVI_CHAR *filename, int src_x_mesh_buf[][4], int src_y_mesh_buf[][4]);

#endif

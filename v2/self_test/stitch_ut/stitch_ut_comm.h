#ifndef __STITCH_UT_COMM_H__
#define __STITCH_UT_COMM_H__

#define NONE	"\033[m"
#define RED	"\033[0;32;31m"
#define GREEN	"\033[0;32;32m"

#define STITCH_UT_PRT(fmt...)                               \
	do {                                                  \
		printf("[%s]-%d: ", __func__, __LINE__);          \
		printf(fmt);                                      \
	} while (0)

#define STITCH_TEST_CHECK_RET(s32Ret) \
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

CVI_CHAR * GetFmtName(PIXEL_FORMAT_E enPixFmt);
CVI_S32 FileSendToStitch(STITCH_SRC_IDX src_id, SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, CVI_CHAR *filename);
CVI_S32 FileSendToStitch2(STITCH_SRC_IDX src_id, SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, CVI_CHAR *filename);
CVI_S32 FileSendToStitchNoVb(STITCH_SRC_IDX src_id, SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, CVI_CHAR *filename, CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr);
CVI_S32 FileToFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat,
	CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);

CVI_S32 FrameSaveToFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);
CVI_S32 CompareWithFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);

#endif

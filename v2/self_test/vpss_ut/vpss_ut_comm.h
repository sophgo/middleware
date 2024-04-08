#ifndef __VPSS_UT_COMM_H__
#define __VPSS_UT_COMM_H__

#define NONE	"\033[m"
#define RED	"\033[0;32;31m"
#define GREEN	"\033[0;32;32m"

#define VPSS_UT_PRT(fmt...)                               \
	do {                                                  \
		printf("[%s]-%d: ", __func__, __LINE__);          \
		printf(fmt);                                      \
	} while (0)

#define TEST_CHECK_RET(s32Ret) \
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

#ifndef BIT
#define BIT(nr)      (UINT64_C(1) << (nr))
#endif

CVI_CHAR * GetFmtName(PIXEL_FORMAT_E enPixFmt);
CVI_S32 FileSendToVpss(VPSS_GRP VpssGrp, const SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat,
	const CVI_CHAR *filename);
CVI_S32 FbcFileSendToVpss(VPSS_GRP VpssGrp, const SIZE_S *stSize,
		PIXEL_FORMAT_E enPixelFormat, const CVI_CHAR filename[4][64], CVI_U32 fbctablelength);
CVI_S32 FileToFrame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat,
	CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);

CVI_S32 FrameSaveToFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);
CVI_S32 FrameFullSaveToFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);

CVI_S32 CompareWithFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame);
CVI_S32 CompareWithMD5(const CVI_CHAR *md5sum, VIDEO_FRAME_INFO_S *pstVideoFrame);

#endif

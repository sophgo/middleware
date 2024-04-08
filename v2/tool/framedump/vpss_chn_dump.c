#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <pthread.h>
#include <errno.h>

#include "linux/cvi_math.h"
#include "cvi_sys.h"
#include "cvi_vpss.h"

static CVI_U32 u32SignalFlag = 0;
static VIDEO_FRAME_INFO_S stFrameInfo;

static VPSS_GRP VpssGrp = 0;
static VPSS_CHN VpssChn = 0;

void VPSS_Chn_Dump_HandleSig(CVI_S32 signo)
{
	if (u32SignalFlag) {
		exit(-1);
	}

	if (SIGINT == signo || SIGTERM == signo) {
		u32SignalFlag++;
		if (stFrameInfo.stVFrame.u64PhyAddr[0])
			CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stFrameInfo);
		u32SignalFlag--;
		printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
	}

	exit(-1);
}

void GetFmtName(PIXEL_FORMAT_E enPixFmt, CVI_CHAR *szName)
{
	switch (enPixFmt)
	{
		case PIXEL_FORMAT_RGB_888:
			snprintf(szName, 10, "rgb");
			break;
		case PIXEL_FORMAT_BGR_888:
			snprintf(szName, 10, "bgr");
			break;
		case PIXEL_FORMAT_RGB_888_PLANAR:
			snprintf(szName, 10, "rgbm");
			break;
		case PIXEL_FORMAT_BGR_888_PLANAR:
			snprintf(szName, 10, "bgrm");
			break;
		case PIXEL_FORMAT_YUV_PLANAR_422:
			snprintf(szName, 10, "p422");
			break;
		case PIXEL_FORMAT_YUV_PLANAR_420:
			snprintf(szName, 10, "p420");
			break;
		case PIXEL_FORMAT_YUV_PLANAR_444:
			snprintf(szName, 10, "p444");
			break;
		case PIXEL_FORMAT_YUV_400:
			snprintf(szName, 10, "y");
			break;
		case PIXEL_FORMAT_HSV_888:
			snprintf(szName, 10, "hsv");
			break;
		case PIXEL_FORMAT_HSV_888_PLANAR:
			snprintf(szName, 10, "hsvm");
			break;
		case PIXEL_FORMAT_NV12:
			snprintf(szName, 10, "nv12");
			break;
		case PIXEL_FORMAT_NV21:
			snprintf(szName, 10, "nv21");
			break;
		case PIXEL_FORMAT_NV16:
			snprintf(szName, 10, "nv16");
			break;
		case PIXEL_FORMAT_NV61:
			snprintf(szName, 10, "nv61");
			break;
		case PIXEL_FORMAT_YUYV:
			snprintf(szName, 10, "yuyv");
			break;
		case PIXEL_FORMAT_UYVY:
			snprintf(szName, 10, "uyvy");
			break;
		case PIXEL_FORMAT_YVYU:
			snprintf(szName, 10, "yvyu");
			break;
		case PIXEL_FORMAT_VYUY:
			snprintf(szName, 10, "vyuy");
			break;

		default:
			snprintf(szName, 10, "unknown");
			break;
	}

}

CVI_VOID SAMPLE_MISC_VpssDump(VPSS_GRP Grp, VPSS_CHN Chn, CVI_U32 u32FrameCnt)
{
	CVI_S32 s32Ret = 0;
	CVI_S32 s32MilliSec = -1;
	CVI_U32 u32Cnt = u32FrameCnt;
	CVI_CHAR szFrameName[128], szPixFrm[10];
	CVI_BOOL bFlag = CVI_TRUE;
	FILE *pfd = CVI_NULL;
	CVI_S32 i;
	CVI_U32 u32DataLen, u32len;


	/* get frame  */
	while (u32Cnt--) {
		if (CVI_VPSS_GetChnFrame(Grp, Chn, &stFrameInfo, s32MilliSec) != CVI_SUCCESS) {
			printf("Get frame fail \n");
			usleep(1000);
			continue;
		}
		if (bFlag) {
			/* make file name */
			GetFmtName(stFrameInfo.stVFrame.enPixelFormat, szPixFrm);
			snprintf(szFrameName, 128, "./vpss_grp%d_chn%d_%dx%d_%s_%d.yuv", Grp, Chn,
					 stFrameInfo.stVFrame.u32Width, stFrameInfo.stVFrame.u32Height,
					 szPixFrm, u32FrameCnt);
			printf("Dump frame of vpss chn %d to file: \"%s\"\n", Chn, szFrameName);
			pfd = fopen(szFrameName, "wb");
			if (CVI_NULL == pfd) {
				printf("open file failed:%s!\n", strerror(errno));
				CVI_VPSS_ReleaseChnFrame(Grp, Chn, &stFrameInfo);
				return;
			}
			bFlag = CVI_FALSE;
		}

		for (i = 0; i < 3; ++i) {
			u32DataLen = stFrameInfo.stVFrame.u32Stride[i] * stFrameInfo.stVFrame.u32Height;
			if (u32DataLen == 0)
				continue;
			if (i > 0 && ((stFrameInfo.stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
				(stFrameInfo.stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) ||
				(stFrameInfo.stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)))
				u32DataLen >>= 1;

			stFrameInfo.stVFrame.pu8VirAddr[i]
				= CVI_SYS_Mmap(stFrameInfo.stVFrame.u64PhyAddr[i], stFrameInfo.stVFrame.u32Length[i]);

			printf("plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d)\n",
				   i, stFrameInfo.stVFrame.u64PhyAddr[i],
				   stFrameInfo.stVFrame.pu8VirAddr[i],
				   stFrameInfo.stVFrame.u32Stride[i]);
			printf(" data_len(%d) plane_len(%d)\n",
					  u32DataLen, stFrameInfo.stVFrame.u32Length[i]);
			u32len = fwrite(stFrameInfo.stVFrame.pu8VirAddr[i], u32DataLen, 1, pfd);
			if (u32len <= 0) {
				printf("fwrite data(%d) error\n", i);
				break;
			}
			CVI_SYS_Munmap(stFrameInfo.stVFrame.pu8VirAddr[i], stFrameInfo.stVFrame.u32Length[i]);
		}

		if (CVI_VPSS_ReleaseChnFrame(Grp, Chn, &stFrameInfo) != CVI_SUCCESS)
			printf("CVI_VPSS_ReleaseChnFrame fail\n");

		stFrameInfo.stVFrame.u64PhyAddr[0] = 0;
	}

	if (pfd)
		fclose(pfd);
}

static void usage(void)
{
	printf(
		"\n"
		"*************************************************\n"
		"Usage: ./vpss_chn_dump [VpssGrp] [VpssChn] [FrmCnt]\n"
		"1)VpssGrp: \n"
		"	Vpss group id\n"
		"2)VpssChn: \n"
		"	vpss chn id\n"
		"3)FrmCnt: \n"
		"	the count of frame to be dump\n"
		"*)Example:\n"
		"	e.g : ./vpss_chn_dump 0 0 1\n"
		"	e.g : ./vpss_chn_dump 1 3 2\n"
		"*************************************************\n"
		"\n");
}

int main(int argc, char **argv)
{
	CVI_U32 u32FrmCnt = 1;

	printf("\nNOTICE: This tool only can be used for TESTING !!!\n");
	printf("\tTo see more usage, please enter: ./vpss_chn_dump -h\n\n");

	if (argc > 1) {
		if (!strncmp(argv[1], "-h", 2)) {
			usage();
			exit(CVI_SUCCESS);
		}
	}

	if (argc < 4) {
		usage();
		exit(CVI_SUCCESS);
	}

	VpssGrp = atoi(argv[1]);

	if (!VALUE_BETWEEN(VpssGrp, 0, VPSS_MAX_GRP_NUM - 1)) {
		printf("grp id must be [0,%d]!!!!\n\n", VPSS_MAX_GRP_NUM - 1);
		return -1;
	}

	VpssChn = atoi(argv[2]);/* chn id*/

	if (!VALUE_BETWEEN(VpssChn, 0, VPSS_MAX_CHN_NUM - 1)) {
		printf("chn id must be [0,%d]!!!!\n\n", VPSS_MAX_CHN_NUM - 1);
		return -1;
	}

	u32SignalFlag = 0;
	signal(SIGINT, VPSS_Chn_Dump_HandleSig);
	signal(SIGTERM, VPSS_Chn_Dump_HandleSig);

	u32FrmCnt = atoi(argv[3]);/* frame count*/
	SAMPLE_MISC_VpssDump(VpssGrp, VpssChn, u32FrmCnt);

	return CVI_SUCCESS;
}


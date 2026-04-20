#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <pthread.h>
#include <errno.h>

#include "cvi_math.h"
#include "cvi_sys.h"
#include "cvi_vo.h"

static CVI_U32 u32SignalFlag = 0;
static VIDEO_FRAME_INFO_S stFrameInfo;

static VO_LAYER VoLayer = 0;
static VO_CHN VoChn = 0;

void VO_Chn_Dump_HandleSig(CVI_S32 signo)
{
	if (u32SignalFlag) {
		exit(-1);
	}

	if (SIGINT == signo || SIGTERM == signo) {
		u32SignalFlag++;
		if (stFrameInfo.stVFrame.u64PhyAddr[0])
			CVI_VO_ReleaseChnFrame(VoLayer, VoChn, &stFrameInfo);
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

CVI_VOID SAMPLE_MISC_VoChnDump(VO_LAYER Layer, VO_CHN Chn, CVI_U32 u32FrameCnt)
{
	CVI_S32 s32Ret = 0;
	CVI_S32 s32MilliSec = 1000;
	CVI_U32 u32Cnt = u32FrameCnt;
	CVI_CHAR szFrameName[128], szPixFrm[10];
	CVI_BOOL bFlag = CVI_TRUE;
	FILE *pfd = CVI_NULL;
	CVI_S32 i;
	CVI_U32 u32DataLen, u32len;


	/* get frame  */
	while (u32Cnt--) {
		if (CVI_VO_GetChnFrame(Layer, Chn, &stFrameInfo, s32MilliSec) != CVI_SUCCESS) {
			printf("Get frame fail \n");
			usleep(1000);
			continue;
		}
		if (bFlag) {
			/* make file name */
			GetFmtName(stFrameInfo.stVFrame.enPixelFormat, szPixFrm);
			snprintf(szFrameName, 128, "./vo_layer%d_chn%d_%dx%d_%s_%d.yuv", Layer, Chn,
					 stFrameInfo.stVFrame.u32Width, stFrameInfo.stVFrame.u32Height,
					 szPixFrm, u32FrameCnt);
			printf("Dump frame of vo chn %d to file: \"%s\"\n", Chn, szFrameName);
			pfd = fopen(szFrameName, "wb");
			if (CVI_NULL == pfd) {
				printf("open file failed:%s!\n", strerror(errno));
				CVI_VO_ReleaseChnFrame(Layer, Chn, &stFrameInfo);
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

		if (CVI_VO_ReleaseChnFrame(Layer, Chn, &stFrameInfo) != CVI_SUCCESS)
			printf("CVI_VO_ReleaseChnFrame fail\n");

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
		"Usage: ./vo_chn_dump [VoLayer] [VoChn] [FrmCnt]\n"
		"1)VoLayer: \n"
		"	vo layer id\n"
		"2)VoChn: \n"
		"	vo chn id\n"
		"3)FrmCnt: \n"
		"	the count of frame to be dump\n"
		"*)Example:\n"
		"	e.g : ./vo_chn_dump 0 0 1\n"
		"	e.g : ./vo_chn_dump 1 3 2\n"
		"*************************************************\n"
		"\n");
}

int main(int argc, char **argv)
{
	CVI_U32 u32FrmCnt = 1;

	printf("\nNOTICE: This tool only can be used for TESTING !!!\n");
	printf("\tTo see more usage, please enter: ./vo_chn_dump -h\n\n");

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

	VoLayer = atoi(argv[1]);

	if (!VALUE_BETWEEN(VoLayer, 0, VO_MAX_LAYER_NUM - 1)) {
		printf("layer id must be [0,%d]!!!!\n\n", VO_MAX_LAYER_NUM - 1);
		return -1;
	}

	VoChn = atoi(argv[2]);/* chn id*/

	if (!VALUE_BETWEEN(VoChn, 0, VO_MAX_CHN_NUM - 1)) {
		printf("chn id must be [0,%d]!!!!\n\n", VO_MAX_CHN_NUM - 1);
		return -1;
	}

	u32SignalFlag = 0;
	signal(SIGINT, VO_Chn_Dump_HandleSig);
	signal(SIGTERM, VO_Chn_Dump_HandleSig);

	u32FrmCnt = atoi(argv[3]);/* frame count*/
	SAMPLE_MISC_VoChnDump(VoLayer, VoChn, u32FrmCnt);

	return CVI_SUCCESS;
}


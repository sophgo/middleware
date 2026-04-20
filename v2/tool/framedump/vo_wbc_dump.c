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

static VO_DEV VoDev = 1;
static VO_WBC VoWbc = 0;

void VO_Wbc_Dump_HandleSig(CVI_S32 signo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (u32SignalFlag) {
		exit(-1);
	}

	if (SIGINT == signo || SIGTERM == signo) {
		u32SignalFlag++;
		if (stFrameInfo.stVFrame.u64PhyAddr[0]) {
			s32Ret = CVI_VO_ReleaseWbcFrame(VoWbc, &stFrameInfo);
			if (s32Ret != CVI_SUCCESS) {
				printf("CVI_VO_ReleaseWbcFrame failed\n");
			}

			s32Ret = CVI_VO_DisableWbc(VoWbc);
			if (s32Ret != CVI_SUCCESS) {
				printf("CVI_VO_DisableWbc failed\n");
			}

		}
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

CVI_VOID SAMPLE_MISC_VoWbcDump(VO_DEV VoDev, CVI_U32 u32FrameCnt)
{
	CVI_S32 s32Ret = 0;
	CVI_S32 s32MilliSec = 1000;
	CVI_U32 u32Cnt = u32FrameCnt;
	CVI_CHAR szFrameName[128], szPixFrm[10];
	CVI_BOOL bFlag = CVI_TRUE;
	FILE *pfd = CVI_NULL;
	CVI_S32 i;
	CVI_U32 u32DataLen, u32len;
    VO_LAYER VoLayer = VoDev;
    VO_WBC_SRC_S stWbcSrc;
    VO_WBC_ATTR_S stWbcAttr;
    CVI_U32 u32Depth;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;

    stWbcSrc.enSrcType = VO_WBC_SRC_DEV;
    stWbcSrc.u32SrcId = VoDev;
    s32Ret = CVI_VO_SetWbcSrc(VoWbc, &stWbcSrc);
    if (s32Ret != CVI_SUCCESS) {
        printf("CVI_VO_SetWbcSrc failed\n");
    }
    s32Ret = CVI_VO_GetWbcSrc(VoWbc, &stWbcSrc);
    if (s32Ret != CVI_SUCCESS) {
        printf("CVI_VO_GetWbcSrc failed\n");
    }

    s32Ret = CVI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
    if (s32Ret != CVI_SUCCESS) {
        printf("CVI_VO_GetVideoLayerAttr\n");
    }

    stWbcAttr.stTargetSize = stLayerAttr.stImageSize;
    stWbcAttr.enPixFormat = PIXEL_FORMAT_NV21;
    stWbcAttr.u32FrameRate = 30;
    stWbcAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
    stWbcAttr.enCompressMode = COMPRESS_MODE_NONE;
    s32Ret = CVI_VO_SetWbcAttr(VoWbc, &stWbcAttr);
    if (s32Ret != CVI_SUCCESS) {
        printf("CVI_VO_SetWbcAttr failed\n");
    }
    s32Ret = CVI_VO_GetWbcAttr(VoWbc, &stWbcAttr);
    if (s32Ret != CVI_SUCCESS) {
        printf("CVI_VO_GetWbcAttr failed\n");
    }

    s32Ret = CVI_VO_SetWbcDepth(VoWbc, 1);
    if (s32Ret != CVI_SUCCESS) {
        printf("CVI_VO_SetWbcAttr failed\n");
    }
    s32Ret = CVI_VO_GetWbcDepth(VoWbc, &u32Depth);
    if (s32Ret != CVI_SUCCESS) {
        printf("CVI_VO_GetWbcDepth failed\n");
    }

    printf("Wbc(srctype,srcid)=(%d,%d)\n",
        stWbcSrc.enSrcType, stWbcSrc.u32SrcId);
    printf("TargetSize %d %d\n", stWbcAttr.stTargetSize.u32Width,
        stWbcAttr.stTargetSize.u32Height);
    printf("PixFormat %d\n", stWbcAttr.enPixFormat);
    printf("FrameRate %d\n", stWbcAttr.u32FrameRate);
    printf("DynamicRange %d\n", stWbcAttr.enDynamicRange);
    printf("CompressMode %d\n", stWbcAttr.enCompressMode);
    printf("Depth %d\n", u32Depth);

    s32Ret = CVI_VO_EnableWbc(VoWbc);
    if (s32Ret != CVI_SUCCESS) {
        printf("CVI_VO_EnableWbc failed\n");
    }

	/* get frame  */
	while (u32Cnt--) {
		s32Ret = CVI_VO_GetWbcFrame(VoWbc, &stFrameInfo, s32MilliSec);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_VO_GetWbcFrame failed\n");
			usleep(1000);
			continue;
		}
		if (bFlag) {
			/* make file name */
			GetFmtName(stFrameInfo.stVFrame.enPixelFormat, szPixFrm);
			snprintf(szFrameName, 128, "./vo_dev%d_wbc%d_%dx%d_%s_%d.yuv", VoDev, VoWbc,
					 stFrameInfo.stVFrame.u32Width, stFrameInfo.stVFrame.u32Height,
					 szPixFrm, u32FrameCnt);
			printf("Dump frame of vo dev %d to file: \"%s\"\n", VoDev, szFrameName);
			pfd = fopen(szFrameName, "wb");
			if (CVI_NULL == pfd) {
				printf("open file failed:%s!\n", strerror(errno));
				s32Ret = CVI_VO_ReleaseWbcFrame(VoWbc, &stFrameInfo);
				if (s32Ret != CVI_SUCCESS) {
					printf("CVI_VO_ReleaseWbcFrame failed\n");
				}

				s32Ret = CVI_VO_DisableWbc(VoWbc);
				if (s32Ret != CVI_SUCCESS) {
					printf("CVI_VO_DisableWbc failed\n");
				}
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

		s32Ret = CVI_VO_ReleaseWbcFrame(VoWbc, &stFrameInfo);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_VO_ReleaseWbcFrame failed\n");
		}
		stFrameInfo.stVFrame.u64PhyAddr[0] = 0;
	}

	s32Ret = CVI_VO_DisableWbc(VoWbc);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_VO_DisableWbc failed\n");
	}

	if (pfd)
		fclose(pfd);
}

static void usage(void)
{
	printf(
		"\n"
		"*************************************************\n"
		"Usage: ./vo_wbc_dump [VoDev] [FrmCnt]\n"
		"1)VoDev: \n"
		"	vo dev id, only support dev1\n"
		"2)FrmCnt: \n"
		"	the count of frame to be dump\n"
		"*)Example:\n"
		"	e.g : ./vo_wbc_dump 1 1\n"
		"	e.g : ./vo_wbc_dump 1 3\n"
		"*************************************************\n"
		"\n");
}

int main(int argc, char **argv)
{
	CVI_U32 u32FrmCnt = 1;

	printf("\nNOTICE: This tool only can be used for TESTING !!!\n");
	printf("\tTo see more usage, please enter: ./vo_wbc_dump -h\n\n");

	if (argc > 1) {
		if (!strncmp(argv[1], "-h", 2)) {
			usage();
			exit(CVI_SUCCESS);
		}
	}

	if (argc < 3) {
		usage();
		exit(CVI_SUCCESS);
	}

	VoDev = atoi(argv[1]);

	if (!VALUE_BETWEEN(VoDev, 0, VO_MAX_DEV_NUM - 1)) {
		printf("vo dev id must be [0,%d]!!!!\n\n", VO_MAX_DEV_NUM - 1);
		return -1;
	}

	u32SignalFlag = 0;
	signal(SIGINT, VO_Wbc_Dump_HandleSig);
	signal(SIGTERM, VO_Wbc_Dump_HandleSig);

	u32FrmCnt = atoi(argv[2]);/* frame count*/
	SAMPLE_MISC_VoWbcDump(VoDev, u32FrmCnt);

	return CVI_SUCCESS;
}


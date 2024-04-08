#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <errno.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "cvi_vb.h"
#include "cvi_sys.h"
#include "cvi_vdec.h"
#include "cvi_venc.h"

#define ALIGN(x, a) (((x) + ((a)-1)) & ~((a)-1))

typedef struct _VDEC_THREAD_PARAM_S {
	CVI_S32 s32ChnId;
	PAYLOAD_TYPE_E enType;
	CVI_CHAR cFilePath[128];
	CVI_CHAR cFileName[128];
	CVI_S32 s32StreamMode;
	CVI_S32 s32MilliSec;
	CVI_S32 s32MinBufSize;
	CVI_S32 s32IntervalTime;
	CVI_U64 u64PtsInit;
	CVI_U64 u64PtsIncrease;
	CVI_BOOL bFileEnd;
	CVI_BOOL bDumpYUV;
	CVI_BOOL bStop;
} VDEC_THREAD_PARAM_S;

typedef struct _VENC_THREAD_PARAM_S {
	CVI_S32 s32ChnId;
	PAYLOAD_TYPE_E enType;
	CVI_CHAR cFileName[128];
	CVI_S32 s32MilliSec;
	CVI_S32 s32IntervalTime;
	CVI_BOOL bDumpYUV;
	CVI_BOOL bStop;
} VENC_THREAD_PARAM_S;

int video_decode(int argc, char *argv[]);
int video_encode(int argc, char *argv[]);

CVI_S32 vb_init(void);
CVI_S32 vb_deinit(void);

int main(int argc, char *argv[])
{
	if (argc < 5) {
		printf("encode:\n");
		printf("\t%s 0 0 test.yuv test.h264\n", argv[0]);
		printf("\t%s 0 1 test.yuv test.h265\n", argv[0]);
		printf("\t%s 0 2 test.yuv test.jpg\n" , argv[0]);
		printf("decode:\n");
		printf("\t%s 1 0 test.h264 test.yuv\n", argv[0]);
		printf("\t%s 1 1 test.h265 test.yuv\n", argv[0]);
		printf("\t%s 1 2 test.jpg  test.yuv\n", argv[0]);
		return 0;
	}

	if (CVI_SUCCESS != vb_init())
		return -1;

	if (atoi(argv[1]) == 0)
		video_encode(argc, argv);
	else if (atoi(argv[1]) == 1)
		video_decode(argc, argv);
	else
		printf("unsupport opt:%d\n", atoi(argv[1]));

	vb_deinit();
	return 0;
}

static int write_yuv(FILE *out_f, VIDEO_FRAME_S stVFrame)
{
	unsigned int i = 0;

	if (stVFrame.u32Length[0]) {
		for(i=0; i<stVFrame.u32Height; i++)
			fwrite(stVFrame.pu8VirAddr[0] + i*stVFrame.u32Stride[0], 1, stVFrame.u32Width, out_f);
	}

	if (stVFrame.u32Length[1]) {
		for(i=0; i<stVFrame.u32Height/2; i++)
			fwrite(stVFrame.pu8VirAddr[1] + i*stVFrame.u32Stride[1], 1, stVFrame.u32Width/2, out_f);
	}

	if (stVFrame.u32Length[2]) {
		for(i=0; i<stVFrame.u32Height/2; i++)
			fwrite(stVFrame.pu8VirAddr[2] + i*stVFrame.u32Stride[2], 1, stVFrame.u32Width/2, out_f);
	}

	return 0;
}

CVI_VOID *dec_proc_send(CVI_VOID *pArgs)
{
	VDEC_THREAD_PARAM_S *pstVdecThreadParam = (VDEC_THREAD_PARAM_S *)pArgs;
	CVI_BOOL bEndOfStream = CVI_FALSE;
	CVI_S32 s32UsedBytes = 0, s32ReadLen = 0;
	FILE *fpStrm = NULL;
	CVI_U8 *pu8Buf = NULL;
	VDEC_STREAM_S stStream;
	CVI_BOOL bFindStart, bFindEnd;
	CVI_U64 u64PTS = 0;
	CVI_U32 u32Len, u32Start;
	CVI_S32 s32Ret, i;

	printf("\n");

	prctl(PR_SET_NAME, "VideoSendStream", 0, 0, 0);

	fpStrm = fopen(pstVdecThreadParam->cFileName, "rb");
	if (fpStrm == NULL) {
		printf("can't open file %s in send stream thread!\n", pstVdecThreadParam->cFileName);
		pstVdecThreadParam->bFileEnd = CVI_TRUE;
		return (CVI_VOID *)(CVI_FAILURE);
	}

	pu8Buf = malloc(pstVdecThreadParam->s32MinBufSize);
	if (pu8Buf == NULL) {
		printf("can't alloc %d in send stream thread!\n", pstVdecThreadParam->s32MinBufSize);
		fclose(fpStrm);
		pstVdecThreadParam->bFileEnd = CVI_TRUE;
		return (CVI_VOID *)(CVI_FAILURE);
	}
	u64PTS = pstVdecThreadParam->u64PtsInit;
	while (1) {
		bEndOfStream = CVI_FALSE;
		bFindStart = CVI_FALSE;
		bFindEnd = CVI_FALSE;
		u32Start = 0;
		s32Ret = fseek(fpStrm, s32UsedBytes, SEEK_SET);
		s32ReadLen = fread(pu8Buf, 1, pstVdecThreadParam->s32MinBufSize, fpStrm);
		if (s32ReadLen == 0) {
			break;
		}

		if (pstVdecThreadParam->s32StreamMode == VIDEO_MODE_FRAME &&
				pstVdecThreadParam->enType == PT_H264) {
			for (i = 0; i < s32ReadLen - 8; i++) {
				int tmp = pu8Buf[i + 3] & 0x1F;

				if (pu8Buf[i] == 0 && pu8Buf[i + 1] == 0 && pu8Buf[i + 2] == 1 &&
					(((tmp == 0x5 || tmp == 0x1) && ((pu8Buf[i + 4] & 0x80) == 0x80)) ||
					 (tmp == 20 && (pu8Buf[i + 7] & 0x80) == 0x80))) {
					bFindStart = CVI_TRUE;
					i += 8;
					break;
				}
			}

			for (; i < s32ReadLen - 8; i++) {
				int tmp = pu8Buf[i + 3] & 0x1F;

				if (pu8Buf[i] == 0 && pu8Buf[i + 1] == 0 && pu8Buf[i + 2] == 1 &&
					(tmp == 15 || tmp == 7 || tmp == 8 || tmp == 6 ||
					 ((tmp == 5 || tmp == 1) && ((pu8Buf[i + 4] & 0x80) == 0x80)) ||
					 (tmp == 20 && (pu8Buf[i + 7] & 0x80) == 0x80))) {
					bFindEnd = CVI_TRUE;
					break;
				}
			}

			if (i > 0)
				s32ReadLen = i;
			if (bFindStart == CVI_FALSE) {
				printf("chn %d can not find H264 start code!s32ReadLen %d, s32UsedBytes %d.!\n",
					   pstVdecThreadParam->s32ChnId, s32ReadLen, s32UsedBytes);
			}
			if (bFindEnd == CVI_FALSE) {
				s32ReadLen = i + 8;
			}

		} else if (pstVdecThreadParam->s32StreamMode == VIDEO_MODE_FRAME &&
				pstVdecThreadParam->enType == PT_H265) {
			CVI_BOOL bNewPic = CVI_FALSE;

			for (i = 0; i < s32ReadLen - 6; i++) {
				CVI_U32 tmp = (pu8Buf[i + 3] & 0x7E) >> 1;

				bNewPic = (pu8Buf[i + 0] == 0 && pu8Buf[i + 1] == 0 && pu8Buf[i + 2] == 1 &&
					   (tmp <= 21) && ((pu8Buf[i + 5] & 0x80) == 0x80));

				if (bNewPic) {
					bFindStart = CVI_TRUE;
					i += 6;
					break;
				}
			}

			for (; i < s32ReadLen - 6; i++) {
				CVI_U32 tmp = (pu8Buf[i + 3] & 0x7E) >> 1;

				bNewPic = (pu8Buf[i + 0] == 0 && pu8Buf[i + 1] == 0 && pu8Buf[i + 2] == 1 &&
					   (tmp == 32 || tmp == 33 || tmp == 34 || tmp == 39 || tmp == 40 ||
						((tmp <= 21) && (pu8Buf[i + 5] & 0x80) == 0x80)));

				if (bNewPic) {
					bFindEnd = CVI_TRUE;
					break;
				}
			}
			if (i > 0)
				s32ReadLen = i;

			if (bFindEnd == CVI_FALSE) {
				s32ReadLen = i + 6;
			}

		} else if (pstVdecThreadParam->enType == PT_MJPEG || pstVdecThreadParam->enType == PT_JPEG) {
			for (i = 0; i < s32ReadLen - 1; i++) {
				if (pu8Buf[i] == 0xFF && pu8Buf[i + 1] == 0xD8) {
					u32Start = i;
					bFindStart = CVI_TRUE;
					i = i + 2;
					break;
				}
			}

			for (; i < s32ReadLen - 3; i++) {
				if ((pu8Buf[i] == 0xFF) && (pu8Buf[i + 1] & 0xF0) == 0xE0) {
					u32Len = (pu8Buf[i + 2] << 8) + pu8Buf[i + 3];
					i += 1 + u32Len;
				} else {
					break;
				}
			}

			for (; i < s32ReadLen - 1; i++) {
				if (pu8Buf[i] == 0xFF && pu8Buf[i + 1] == 0xD9) {
					bFindEnd = CVI_TRUE;
					break;
				}
			}
			s32ReadLen = i + 2;
		} else {
			if ((s32ReadLen != 0) && (s32ReadLen < pstVdecThreadParam->s32MinBufSize)) {
				bEndOfStream = CVI_TRUE;
			}
		}

		stStream.u64PTS = u64PTS;
		stStream.pu8Addr = pu8Buf + u32Start;
		stStream.u32Len = s32ReadLen;
		stStream.bEndOfFrame = (pstVdecThreadParam->s32StreamMode == VIDEO_MODE_FRAME) ? CVI_TRUE : CVI_FALSE;
		stStream.bEndOfStream = bEndOfStream;
		stStream.bDisplay = 1;

SendAgain:
		s32Ret = CVI_VDEC_SendStream(pstVdecThreadParam->s32ChnId,
				&stStream, pstVdecThreadParam->s32MilliSec);
		if (s32Ret != CVI_SUCCESS) {
			usleep(pstVdecThreadParam->s32IntervalTime);

			if (s32Ret == CVI_ERR_VDEC_BUSY)
				printf("timeout in vdec sendstream\n");

			goto SendAgain;
		} else {
			bEndOfStream = CVI_FALSE;
			s32UsedBytes = s32UsedBytes + s32ReadLen + u32Start;
			u64PTS += pstVdecThreadParam->u64PtsIncrease;
		}
		usleep(pstVdecThreadParam->s32IntervalTime);
	}

	fflush(stdout);
	if (pu8Buf != CVI_NULL) {
		free(pu8Buf);
	}

	fclose(fpStrm);

	memset(&stStream, 0 ,sizeof(VDEC_STREAM_S));
	stStream.bEndOfStream = CVI_TRUE;
SendAgain2:
	s32Ret = CVI_VDEC_SendStream(pstVdecThreadParam->s32ChnId, &stStream, -1);
	if (s32Ret != CVI_SUCCESS) {
		usleep(pstVdecThreadParam->s32IntervalTime);
		goto SendAgain2;
	}

	return (CVI_VOID *)CVI_SUCCESS;
}

CVI_VOID *dec_proc_get(CVI_VOID *pArgs)
{
	VDEC_THREAD_PARAM_S *pstVdecThreadParam = (VDEC_THREAD_PARAM_S *)pArgs;
	FILE *fp = CVI_NULL;
	CVI_S32 s32Ret, s32Cnt = 0;
	VDEC_CHN_ATTR_S stAttr;
	VIDEO_FRAME_INFO_S stVFrame;
	CVI_CHAR cSaveFile[256];

	printf("\n");

	prctl(PR_SET_NAME, "VdecGetPic", 0, 0, 0);

	s32Ret = CVI_VDEC_GetChnAttr(pstVdecThreadParam->s32ChnId, &stAttr);
	if (s32Ret != CVI_SUCCESS) {
		printf("get chn attr fail for %#x!\n", s32Ret);
		return (CVI_VOID *)(CVI_FAILURE);
	}

	if (stAttr.enType != PT_JPEG &&
	    stAttr.enType != PT_H264 &&
	    stAttr.enType != PT_H265 &&
	    stAttr.enType != PT_MJPEG) {
		printf("enType %d do not support save file!\n", stAttr.enType);
		return (CVI_VOID *)(CVI_FAILURE);
	}

	while (!pstVdecThreadParam->bStop) {
RETRY_GET_FRAME:
		s32Ret = CVI_VDEC_GetFrame(
				pstVdecThreadParam->s32ChnId,
				&stVFrame,
				pstVdecThreadParam->s32MilliSec);
		if (s32Ret == CVI_SUCCESS) {
			if (pstVdecThreadParam->bDumpYUV == 1) {
				if (s32Cnt == 0) {
					fp = fopen(pstVdecThreadParam->cFileName, "wb");
					if (fp == NULL) {
						printf("can't open file %s\n", cSaveFile);
						return (CVI_VOID *)(CVI_FAILURE);
					}
				}

				write_yuv(fp, stVFrame.stVFrame);
			}

			s32Cnt++;

			s32Ret = CVI_VDEC_ReleaseFrame(pstVdecThreadParam->s32ChnId, &stVFrame);
			if (s32Ret != CVI_SUCCESS) {
				printf("CVI_MPI_VDEC_ReleaseFrame fail for s32Ret=0x%x!\n", s32Ret);
			}
		} else {
			if (s32Ret == CVI_ERR_VDEC_BUSY) {
				printf("vdec getframe timeout...retry\n");
				goto RETRY_GET_FRAME;
			}
			usleep(pstVdecThreadParam->s32IntervalTime);
		}
	}

	if (pstVdecThreadParam->bDumpYUV == 1) {
		if (fp != CVI_NULL)
			fclose(fp);
	}

	return (CVI_VOID *)CVI_SUCCESS;
}

int video_decode(int argc, char *argv[])
{
	VDEC_CHN VdChn = 0;
	VDEC_CHN_ATTR_S stAttr = {0};
	pthread_t sendThread;
	pthread_t getThread;
	VDEC_THREAD_PARAM_S sendThreadParam = {0};
	VDEC_THREAD_PARAM_S getThreadParam = {0};
	int ret;

	if (atoi(argv[2]) == 0)
		stAttr.enType = PT_H264;
	else if (atoi(argv[2]) == 1)
		stAttr.enType = PT_H265;
	else if (atoi(argv[2]) == 2)
		stAttr.enType = PT_JPEG;
	else {
		printf("unsupport type: %d\n", atoi(argv[2]));
		return -1;
	}

	stAttr.u32StreamBufSize = 0x500000;
	stAttr.u32FrameBufCnt = 6;
	stAttr.enMode = VIDEO_MODE_FRAME;
	ret = CVI_VDEC_CreateChn(VdChn, &stAttr);
	if (ret != CVI_SUCCESS) {
		printf("CVI_VDEC_CreateChn failed %d\n", ret);
		return -1;
	}

	ret = CVI_VDEC_StartRecvStream(VdChn);
	if (ret != CVI_SUCCESS) {
		printf("CVI_VDEC_StartRecvStream failed %d\n", ret);
		return -1;
	}

	sendThreadParam.s32ChnId = 0;
	sendThreadParam.enType = stAttr.enType;
	memset(sendThreadParam.cFileName, 0, 128);
	memcpy(sendThreadParam.cFileName, argv[3], strlen(argv[3]));
	sendThreadParam.s32StreamMode = stAttr.enMode;
	sendThreadParam.s32MilliSec = -1;
	sendThreadParam.s32MinBufSize = 512*1024;
	sendThreadParam.s32IntervalTime = 1000;
	sendThreadParam.u64PtsInit = 0;
	sendThreadParam.u64PtsIncrease = 30;
	sendThreadParam.bDumpYUV = 1;
	ret = pthread_create(&sendThread, NULL, dec_proc_send, (CVI_VOID *)&sendThreadParam);

	getThreadParam.s32ChnId = 0;
	getThreadParam.enType = stAttr.enType;
	memset(getThreadParam.cFileName, 0, 128);
	memcpy(getThreadParam.cFileName, argv[4], strlen(argv[4]));
	getThreadParam.bDumpYUV = 1;
	ret = pthread_create(&getThread, NULL, dec_proc_get, (CVI_VOID *)&getThreadParam);

	pthread_join(sendThread, NULL);
	getThreadParam.bStop = 1;
	pthread_join(getThread, NULL);

	ret = CVI_VDEC_StopRecvStream(VdChn);
	if (ret != CVI_SUCCESS) {
		printf("CVI_VDEC_StopRecvStream failed %d\n", ret);
		return -1;
	}

	ret = CVI_VDEC_DestroyChn(VdChn);
	if (ret != CVI_SUCCESS) {
		printf("CVI_VDEC_DestroyChn failed %d\n", ret);
		return -1;
	}

	return ret;
}
CVI_VOID *enc_proc_send(CVI_VOID *pArgs)
{
	VENC_THREAD_PARAM_S *pSendThreadParam = pArgs;
	VENC_CHN_ATTR_S stAttr = {0};
	VIDEO_FRAME_INFO_S stFrame = {0};
	VB_POOL_CONFIG_S cfg = {0};
	VB_POOL pool = VB_INVALID_POOLID;
	VB_BLK blk = VB_INVALID_HANDLE;
	FILE *fpInput = NULL;
	CVI_U32 stride;
	CVI_S32 s32MilliSec = -1;
	CVI_U32 ySize = 0;
	CVI_U32 uvSize = 0;
	CVI_S32 s32ReadLen;
	CVI_S32 ret;

	fpInput = fopen(pSendThreadParam->cFileName, "rb");
	if (fpInput == NULL) {
		printf("can't open file %s\n", pSendThreadParam->cFileName);
		return NULL;
	}

	CVI_VENC_GetChnAttr(pSendThreadParam->s32ChnId, &stAttr);
	stride = ALIGN(stAttr.stVencAttr.u32PicWidth, 32);
	ySize = stride * stAttr.stVencAttr.u32PicHeight;
	uvSize = stride * stAttr.stVencAttr.u32PicHeight / 4;

	cfg.u32BlkSize = ySize + uvSize + uvSize;
	cfg.u32BlkCnt = 4;
	cfg.enRemapMode = VB_REMAP_MODE_NOCACHE;
	pool = CVI_VB_CreatePool(&cfg);
	if (pool == VB_INVALID_POOLID) {
		printf("CVI_VB_CreatePool NG.\n");
		return NULL;
	}

	ret = CVI_VB_MmapPool(pool);
	if (ret != CVI_SUCCESS) {
		printf("CVI_VB_MmapPool NG\n");
		return NULL;
	}

	while(1)
	{
		blk = CVI_VB_GetBlock(pool, cfg.u32BlkSize);
		if (blk == VB_INVALID_HANDLE) {
			printf("CVI_VB_GetBlock NG\n");
			sleep(1);
			continue;
		}

		stFrame.stVFrame.s32FrameIdx = -1;
		stFrame.stVFrame.u32Width = stAttr.stVencAttr.u32PicWidth;
		stFrame.stVFrame.u32Height = stAttr.stVencAttr.u32PicHeight;
		stFrame.stVFrame.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
		stFrame.stVFrame.u32Stride[0] = stAttr.stVencAttr.u32PicWidth;
		stFrame.stVFrame.u32Stride[1] = stAttr.stVencAttr.u32PicWidth / 2;
		stFrame.stVFrame.u32Stride[2] = stAttr.stVencAttr.u32PicWidth / 2;

		stFrame.stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
		stFrame.stVFrame.u64PhyAddr[1] = stFrame.stVFrame.u64PhyAddr[0] + ySize;
		stFrame.stVFrame.u64PhyAddr[2] = stFrame.stVFrame.u64PhyAddr[1] + uvSize;

		ret = CVI_VB_GetBlockVirAddr(pool, blk, (void **)&(stFrame.stVFrame.pu8VirAddr[0]));
		if (ret != CVI_SUCCESS) {
			printf("CVI_VB_GetBlockVirAddr NG, ret:%d\n", ret);
			return NULL;
		}

		stFrame.stVFrame.pu8VirAddr[1] = stFrame.stVFrame.pu8VirAddr[0] + ySize;
		stFrame.stVFrame.pu8VirAddr[2] = stFrame.stVFrame.pu8VirAddr[1] + uvSize;
		s32ReadLen = fread(stFrame.stVFrame.pu8VirAddr[0], 1, ySize+uvSize*2, fpInput);
		if (s32ReadLen == 0) {
			fclose(fpInput);
			break;
		}

		CVI_SYS_IonFlushCache(stFrame.stVFrame.u64PhyAddr[0], stFrame.stVFrame.pu8VirAddr[0], ySize+uvSize*2);

again:
		ret = CVI_VENC_SendFrame(pSendThreadParam->s32ChnId, &stFrame, s32MilliSec);
		if (ret != CVI_SUCCESS) {
			usleep(pSendThreadParam->s32IntervalTime);
			goto again;
		}

		CVI_VB_ReleaseBlock(blk);
		if (stAttr.stVencAttr.enType == PT_JPEG)
			break;

		usleep(pSendThreadParam->s32IntervalTime);
	}

again2:
	stFrame.stVFrame.bSrcEnd = CVI_TRUE;
	ret = CVI_VENC_SendFrame(pSendThreadParam->s32ChnId, &stFrame, s32MilliSec);
	if (ret != CVI_SUCCESS) {
		sleep(1);
		goto again2;
	}

	ret = CVI_VB_MunmapPool(pool);
	if (ret != CVI_SUCCESS) {
		printf("CVI_VB_DestroyPool NG\n");
		return NULL;
	}

	ret = CVI_VB_DestroyPool(pool);
	if (ret != CVI_SUCCESS) {
		printf("CVI_VB_DestroyPool NG\n");
		return NULL;
	}
	return NULL;
}

CVI_VOID *enc_proc_get(CVI_VOID *pArgs)
{
	VENC_THREAD_PARAM_S *pGetThreadParam = pArgs;
	VENC_CHN_STATUS_S stStat = {0};
	VENC_STREAM_S stStream;
	FILE *fpOutput = NULL;
	CVI_S32 s32MilliSec = 100;
	CVI_S32 ret;
	int i;

	fpOutput = fopen(pGetThreadParam->cFileName, "wb");
	if (fpOutput == NULL) {
		printf("can't open file %s\n", pGetThreadParam->cFileName);
		return NULL;
	}

	ret = CVI_VENC_QueryStatus(pGetThreadParam->s32ChnId, &stStat);
	if (ret != CVI_SUCCESS) {
		return NULL;
	}

	if (!stStat.u32CurPacks) {
		printf("u32CurPacks = NULL!\n");
		return NULL;
	}

	stStream.pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
	if (stStream.pstPack == NULL) {
		printf("malloc memory failed!\n");
		return NULL;
	}

	while(!pGetThreadParam->bStop) {
		ret = CVI_VENC_GetStream(pGetThreadParam->s32ChnId, &stStream, s32MilliSec);
		if (ret != CVI_SUCCESS) {
			continue;
		}

		for (i=0; i<stStream.u32PackCount; i++)
			fwrite(stStream.pstPack[i].pu8Addr , 1, stStream.pstPack[i].u32Len, fpOutput);

		ret = CVI_VENC_ReleaseStream(pGetThreadParam->s32ChnId, &stStream);
		if (ret != CVI_SUCCESS) {
			printf("CVI_VENC_ReleaseStream FAIL: 0x%x\n", ret);
			return NULL;
		}
	}

	fclose(fpOutput);
	free(stStream.pstPack);
	return NULL;
}

int video_encode(int argc, char *argv[])
{
	VENC_CHN VeChn = 0;
	VENC_CHN_ATTR_S stAttr = {0};
	VENC_THREAD_PARAM_S sendThreadParam = {0};
	VENC_THREAD_PARAM_S getThreadParam = {0};
	VENC_RC_PARAM_S stRcParam = {0};
	VENC_RECV_PIC_PARAM_S stRecvParam;
	pthread_t sendThread;
	pthread_t getThread;
	CVI_S32 ret;

	if (atoi(argv[2]) == 0)
		stAttr.stVencAttr.enType = PT_H264;
	else if (atoi(argv[2]) == 1)
		stAttr.stVencAttr.enType = PT_H265;
	else if (atoi(argv[2]) == 2)
		stAttr.stVencAttr.enType = PT_JPEG;
	else {
		printf("unsupport type: %d\n", atoi(argv[2]));
		return -1;
	}

	stAttr.stVencAttr.u32MaxPicWidth = 1920;
	stAttr.stVencAttr.u32MaxPicHeight = 1080;
	stAttr.stVencAttr.u32BufSize = 1024 * 1024;
	stAttr.stVencAttr.u32PicWidth = 1920;
	stAttr.stVencAttr.u32PicHeight = 1080;
	stAttr.stVencAttr.enEncMode = VENC_MODE_RECOMMEND;

	if (stAttr.stVencAttr.enType == PT_H264) {
		stAttr.stVencAttr.stAttrH264e.bSingleLumaBuf = CVI_FALSE;
		stAttr.stVencAttr.stAttrH264e.bRcnRefShareBuf = CVI_FALSE;
		stAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
		stAttr.stRcAttr.stH264Cbr.u32Gop = 25;
		stAttr.stRcAttr.stH264Cbr.u32StatTime = 2;
		stAttr.stRcAttr.stH264Cbr.u32SrcFrameRate = 25;
		stAttr.stRcAttr.stH264Cbr.fr32DstFrameRate = 25;
		stAttr.stRcAttr.stH264Cbr.u32BitRate = 1024;
		stAttr.stRcAttr.stH264Cbr.bVariFpsEn = CVI_FALSE;

	} else if (stAttr.stVencAttr.enType == PT_H265) {
		stAttr.stVencAttr.stAttrH265e.bRcnRefShareBuf = CVI_FALSE;
		stAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
		stAttr.stRcAttr.stH265Cbr.u32Gop = 25;
		stAttr.stRcAttr.stH265Cbr.u32StatTime = 2;
		stAttr.stRcAttr.stH265Cbr.u32SrcFrameRate = 25;
		stAttr.stRcAttr.stH265Cbr.fr32DstFrameRate = 25;
		stAttr.stRcAttr.stH265Cbr.u32BitRate = 1024;
		stAttr.stRcAttr.stH265Cbr.bVariFpsEn = CVI_FALSE;

	} else if (stAttr.stVencAttr.enType == PT_JPEG) {
		stAttr.stVencAttr.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
		stAttr.stVencAttr.stAttrJpege.bSupportDCF = CVI_FALSE;
		stAttr.stVencAttr.stAttrJpege.stMPFCfg.u8LargeThumbNailNum = 0;
		stAttr.stVencAttr.stAttrJpege.enReceiveMode = VENC_PIC_RECEIVE_SINGLE;
	}

	if (stAttr.stVencAttr.enType == PT_H264 || stAttr.stVencAttr.enType == PT_H265) {
		stAttr.stGopAttr.enGopMode = VENC_GOPMODE_NORMALP;
		stAttr.stGopAttr.stNormalP.s32IPQpDelta = 2;
		stAttr.stGopExAttr.u32GopPreset = GOP_PRESET_IDX_IPPPP;
	}

	ret = CVI_VENC_CreateChn(VeChn, &stAttr);
	if (ret != CVI_SUCCESS) {
		printf("CVI_VENC_CreateChn FAIL: 0x%x\n", ret);
		return -1;
	}

	ret =  CVI_VENC_GetRcParam(VeChn, &stRcParam);
	if (ret != CVI_SUCCESS) {
		printf("CVI_VENC_GetRcParam FAIL: 0x%x\n", ret);
		return -1;
	}
    stRcParam.s32FirstFrameStartQp = 30;
	stRcParam.s32InitialDelay = CVI_INITIAL_DELAY_DEFAULT;
	stRcParam.u32ThrdLv = 2;

	if (stAttr.stVencAttr.enType == PT_H264) {
		stRcParam.stParamH264Cbr.bQpMapEn = CVI_FALSE;
		// stRcParam.stParamH264Cbr.s32MaxReEncodeTimes = attr.s32MaxReEncodeTimes;
		stRcParam.stParamH264Cbr.u32MaxIprop = CVI_H26X_MAX_I_PROP_MAX;
		stRcParam.stParamH264Cbr.u32MinIprop = CVI_H26X_MAX_I_PROP_MIN;
		stRcParam.stParamH264Cbr.u32MaxIQp = CVI_H26X_MAXIQP_MAX;
		stRcParam.stParamH264Cbr.u32MinIQp = CVI_H26X_MINIQP_MIN;
		stRcParam.stParamH264Cbr.u32MaxQp = CVI_H26X_MAXQP_MAX;
		stRcParam.stParamH264Cbr.u32MinQp = CVI_H26X_MINQP_MIN;
	}

	if (stAttr.stVencAttr.enType == PT_H265) {
		stRcParam.stParamH265Cbr.bQpMapEn = CVI_FALSE;
		// stRcParam.stParamH265Cbr.s32MaxReEncodeTimes = attr.s32MaxReEncodeTimes;
		stRcParam.stParamH265Cbr.u32MaxIprop = CVI_H26X_MAX_I_PROP_MAX;
		stRcParam.stParamH265Cbr.u32MinIprop = CVI_H26X_MAX_I_PROP_MIN;
		stRcParam.stParamH265Cbr.u32MaxIQp = CVI_H26X_MAXIQP_MAX;
		stRcParam.stParamH265Cbr.u32MinIQp = CVI_H26X_MINIQP_MIN;
		stRcParam.stParamH265Cbr.u32MaxQp = CVI_H26X_MAXQP_MAX;
		stRcParam.stParamH265Cbr.u32MinQp = CVI_H26X_MINQP_MIN;
	}

    ret = CVI_VENC_SetRcParam(VeChn, &stRcParam);
	if (ret != CVI_SUCCESS) {
		printf("CVI_VENC_SetRcParam FAIL: 0x%x\n", ret);
		return -1;
	}

	ret = CVI_VENC_StartRecvFrame(VeChn, &stRecvParam);
	if (ret != CVI_SUCCESS) {
		printf("CVI_VENC_StartRecvFrame FAIL: 0x%x\n", ret);
		return -1;
	}

	sendThreadParam.s32ChnId = VeChn;
	sendThreadParam.enType = stAttr.stVencAttr.enType;
	memset(sendThreadParam.cFileName, 0, 128);
	memcpy(sendThreadParam.cFileName, argv[3], strlen(argv[3]));
	sendThreadParam.s32MilliSec = -1;
	sendThreadParam.s32IntervalTime = 1000;
	sendThreadParam.bDumpYUV = 1;
	ret = pthread_create(&sendThread, NULL, enc_proc_send, (CVI_VOID *)&sendThreadParam);

	getThreadParam.s32ChnId = 0;
	memset(getThreadParam.cFileName, 0, 128);
	memcpy(getThreadParam.cFileName, argv[4], strlen(argv[4]));
	getThreadParam.bDumpYUV = 1;
	ret = pthread_create(&getThread, NULL, enc_proc_get, (CVI_VOID *)&getThreadParam);

	pthread_join(sendThread, NULL);
	getThreadParam.bStop = 1;
	pthread_join(getThread, NULL);

	ret = CVI_VENC_StopRecvFrame(VeChn);
	if (ret != CVI_SUCCESS) {
		printf("CVI_VENC_StopRecvFrame FAIL: 0x%x\n", ret);
		return -1;
	}

	ret = CVI_VENC_DestroyChn(VeChn);
	if (ret != CVI_SUCCESS) {
		printf("CVI_VENC_DestroyChn FAIL: 0x%x\n", ret);
		return -1;
	}

	return 0;
}

#define COMMON_POOL0_BLK_SIZE (0x300000) // 3M
#define COMMON_POOL1_BLK_SIZE (0x100000) // 1M
#define COMMON_POOL0_BLK_CNT (3)
#define COMMON_POOL1_BLK_CNT (3)
#define VB_KERNEL_TEST_OP_OFFSET (100)

CVI_S32 vb_init(void)
{
	CVI_S32 s32Ret;
	VB_CONFIG_S stVbConf = {0};

	CVI_VB_Exit();
	// CVI_SYS_Exit();

	stVbConf.u32MaxPoolCnt = 2;
	stVbConf.astCommPool[0].u32BlkSize = COMMON_POOL0_BLK_SIZE;
	stVbConf.astCommPool[0].u32BlkCnt = COMMON_POOL0_BLK_CNT;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize = COMMON_POOL1_BLK_SIZE;
	stVbConf.astCommPool[1].u32BlkCnt = COMMON_POOL1_BLK_CNT;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 vb_deinit(void)
{
	CVI_VB_Exit();
	// CVI_SYS_Exit();
	return CVI_SUCCESS;
}


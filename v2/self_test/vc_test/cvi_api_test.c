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
#include "cvi_region.h"

#define ALIGN(x, a) (((x) + ((a)-1)) & ~((a)-1))

typedef struct _VENC_THREAD_PARAM_S {
	CVI_S32 s32ChnId;
	PAYLOAD_TYPE_E enType;
	CVI_CHAR cFileName[128];
	CVI_S32 s32MilliSec;
	CVI_S32 s32IntervalTime;
	CVI_BOOL bDumpYUV;
	CVI_BOOL bStop;
} VENC_THREAD_PARAM_S;

int video_encode(int argc, char *argv[]);

CVI_S32 vb_init(void);
CVI_S32 vb_deinit(void);

int main(int argc, char *argv[])
{
	if (argc < 3) {
		printf("encode:\n");
		printf("\t%s test.yuv test.jpg\n" , argv[0]);
		return 0;
	}

	if (CVI_SUCCESS != vb_init())
		return -1;

	video_encode(argc, argv);

	vb_deinit();
	return 0;
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

		stFrame.stVFrame.u32Length[0] = ySize;
		stFrame.stVFrame.u32Length[1] = uvSize;
		stFrame.stVFrame.u32Length[2] = uvSize;

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

	ret = CVI_VB_MunmapPool(pool);
	if (ret != CVI_SUCCESS) {
		printf("CVI_VB_DestroyPool NG\n");
		return NULL;
	}

	printf("pool destroy:pool=%d\n", pool);
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
	CVI_U32 i;

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

int rgn_init(RGN_HANDLE Handle, MMF_CHN_S stChn)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	RGN_TYPE_E enType;
	RGN_ATTR_S stRegion;
	RGN_CHN_ATTR_S stChnAttr;

	enType = OVERLAY_RGN;

	stRegion.enType = enType;
	stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
	stRegion.unAttr.stOverlay.stSize.u32Width = 1280;
	stRegion.unAttr.stOverlay.stSize.u32Height = 720;
	stRegion.unAttr.stOverlay.u32BgColor = 0x00;
	stRegion.unAttr.stOverlay.u32CanvasNum = 2;
	stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_SW;
	stRegion.unAttr.stOverlay.stCompressInfo.u32EstCompressedSize = RGN_CMPR_MIN_SIZE;

	s32Ret = CVI_RGN_Create(Handle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_RGN_Create failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	stChnAttr.bShow = true;
	stChnAttr.enType = enType;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 0;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;
	s32Ret = CVI_RGN_AttachToChn(Handle, &stChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_RGN_AttachToChn failed with %#x!\n", s32Ret);
		goto EXIT0;
	} else {
		return s32Ret;
	}

EXIT0:
	CVI_RGN_Destroy(Handle);
	return s32Ret;
}

int rgn_update(RGN_HANDLE Handle)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_U32 au32Colors[8] = {0xffff, 0x8000, 0x801f, 0x83e0, 0xfc00, 0x83ff, 0xffe0, 0xfc1f};

	RGN_CANVAS_CMPR_ATTR_S *pstCanvasCmprAttr;
	RGN_CMPR_OBJ_ATTR_S *pstObjAttr;
	RGN_CANVAS_INFO_S stCanvasInfo;
	RGN_ATTR_S stRegion;

	s32Ret = CVI_RGN_GetAttr(Handle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_RGN_GetAttr failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_RGN_GetCanvasInfo(Handle, &stCanvasInfo);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_RGN_GetCanvasInfo failed with %#x!\n", s32Ret);
		return s32Ret;
	}

	pstCanvasCmprAttr = stCanvasInfo.pstCanvasCmprAttr;
	pstObjAttr = stCanvasInfo.pstObjAttr;

	pstCanvasCmprAttr->u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
	pstCanvasCmprAttr->u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;
	pstCanvasCmprAttr->u32BgColor =  stRegion.unAttr.stOverlay.u32BgColor;
	pstCanvasCmprAttr->enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
	pstCanvasCmprAttr->u32BsSize = stRegion.unAttr.stOverlay.stCompressInfo.u32EstCompressedSize;
	pstCanvasCmprAttr->u32ObjNum = 12;

	pstObjAttr[0].stRgnRect.stRect.s32X = 0;
	pstObjAttr[0].stRgnRect.stRect.s32Y = 0;
	pstObjAttr[0].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[0].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[0].stRgnRect.u32Thick = 1;
	pstObjAttr[0].stRgnRect.u32Color = au32Colors[0];
	pstObjAttr[0].stRgnRect.u32IsFill = false;
	pstObjAttr[0].enObjType = RGN_CMPR_RECT;
	pstObjAttr[1].stRgnRect.stRect.s32X = 100;
	pstObjAttr[1].stRgnRect.stRect.s32Y = 100;
	pstObjAttr[1].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[1].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[1].stRgnRect.u32Thick = 2;
	pstObjAttr[1].stRgnRect.u32Color = au32Colors[1];
	pstObjAttr[1].stRgnRect.u32IsFill = false;
	pstObjAttr[1].enObjType = RGN_CMPR_RECT;
	pstObjAttr[2].stRgnRect.stRect.s32X = 200;
	pstObjAttr[2].stRgnRect.stRect.s32Y = 200;
	pstObjAttr[2].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[2].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[2].stRgnRect.u32Thick = 3;
	pstObjAttr[2].stRgnRect.u32Color = au32Colors[2];
	pstObjAttr[2].stRgnRect.u32IsFill = false;
	pstObjAttr[2].enObjType = RGN_CMPR_RECT;
	pstObjAttr[3].stRgnRect.stRect.s32X = 300;
	pstObjAttr[3].stRgnRect.stRect.s32Y = 300;
	pstObjAttr[3].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[3].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[3].stRgnRect.u32Thick = 4;
	pstObjAttr[3].stRgnRect.u32Color = au32Colors[3];
	pstObjAttr[3].stRgnRect.u32IsFill = false;
	pstObjAttr[3].enObjType = RGN_CMPR_RECT;
	pstObjAttr[4].stRgnRect.stRect.s32X = 400;
	pstObjAttr[4].stRgnRect.stRect.s32Y = 300;
	pstObjAttr[4].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[4].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[4].stRgnRect.u32Thick = 5;
	pstObjAttr[4].stRgnRect.u32Color = au32Colors[4];
	pstObjAttr[4].stRgnRect.u32IsFill = false;
	pstObjAttr[4].enObjType = RGN_CMPR_RECT;
	pstObjAttr[5].stRgnRect.stRect.s32X = 500;
	pstObjAttr[5].stRgnRect.stRect.s32Y = 200;
	pstObjAttr[5].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[5].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[5].stRgnRect.u32Thick = 6;
	pstObjAttr[5].stRgnRect.u32Color = au32Colors[5];
	pstObjAttr[5].stRgnRect.u32IsFill = false;
	pstObjAttr[5].enObjType = RGN_CMPR_RECT;
	pstObjAttr[6].stRgnRect.stRect.s32X = 600;
	pstObjAttr[6].stRgnRect.stRect.s32Y = 100;
	pstObjAttr[6].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[6].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[6].stRgnRect.u32Thick = 7;
	pstObjAttr[6].stRgnRect.u32Color = au32Colors[6];
	pstObjAttr[6].stRgnRect.u32IsFill = false;
	pstObjAttr[6].enObjType = RGN_CMPR_RECT;
	pstObjAttr[7].stRgnRect.stRect.s32X = 700;
	pstObjAttr[7].stRgnRect.stRect.s32Y = 000;
	pstObjAttr[7].stRgnRect.stRect.u32Width = 200;
	pstObjAttr[7].stRgnRect.stRect.u32Height = 200;
	pstObjAttr[7].stRgnRect.u32Thick = 8;
	pstObjAttr[7].stRgnRect.u32Color = au32Colors[7];
	pstObjAttr[7].stRgnRect.u32IsFill = false;
	pstObjAttr[7].enObjType = RGN_CMPR_RECT;

	pstObjAttr[8].stLine.stPointStart.s32X = 600;
	pstObjAttr[8].stLine.stPointStart.s32Y = 200;
	pstObjAttr[8].stLine.stPointEnd.s32X = 300;
	pstObjAttr[8].stLine.stPointEnd.s32Y = 400;
	pstObjAttr[8].stLine.u32Thick = 8;
	pstObjAttr[8].stLine.u32Color = au32Colors[7];
	pstObjAttr[8].enObjType = RGN_CMPR_LINE;
	pstObjAttr[9].stLine.stPointStart.s32X = 300;
	pstObjAttr[9].stLine.stPointStart.s32Y = 400;
	pstObjAttr[9].stLine.stPointEnd.s32X = 800;
	pstObjAttr[9].stLine.stPointEnd.s32Y = 700;
	pstObjAttr[9].stLine.u32Thick = 8;
	pstObjAttr[9].stLine.u32Color = au32Colors[7];
	pstObjAttr[9].enObjType = RGN_CMPR_LINE;
	pstObjAttr[10].stLine.stPointStart.s32X = 800;
	pstObjAttr[10].stLine.stPointStart.s32Y = 700;
	pstObjAttr[10].stLine.stPointEnd.s32X = 1100;
	pstObjAttr[10].stLine.stPointEnd.s32Y = 600;
	pstObjAttr[10].stLine.u32Thick = 8;
	pstObjAttr[10].stLine.u32Color = au32Colors[7];
	pstObjAttr[10].enObjType = RGN_CMPR_LINE;
	pstObjAttr[11].stLine.stPointStart.s32X = 1100;
	pstObjAttr[11].stLine.stPointStart.s32Y = 600;
	pstObjAttr[11].stLine.stPointEnd.s32X = 600;
	pstObjAttr[11].stLine.stPointEnd.s32Y = 200;
	pstObjAttr[11].stLine.u32Thick = 8;
	pstObjAttr[11].stLine.u32Color = au32Colors[7];
	pstObjAttr[11].enObjType = RGN_CMPR_LINE;

	s32Ret = CVI_RGN_UpdateCanvas(Handle);
	if (s32Ret != CVI_SUCCESS) {
		printf("CVI_RGN_UpdateCanvas failed with %#x!\n", s32Ret);
	}

	return s32Ret;
}

int rgn_deinit(RGN_HANDLE Handle, MMF_CHN_S stChn)
{
	CVI_RGN_DetachFromChn(Handle, &stChn);
	CVI_RGN_Destroy(Handle);
	return CVI_SUCCESS;
}

int video_encode(int argc __attribute__((unused)), char *argv[])
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
	RGN_HANDLE Handle = 0;
	MMF_CHN_S stChn = {0};

	stAttr.stVencAttr.enType = PT_JPEG;
	stAttr.stVencAttr.u32MaxPicWidth = 1920;
	stAttr.stVencAttr.u32MaxPicHeight = 1080;
	stAttr.stVencAttr.u32BufSize = 1024 * 1024;
	stAttr.stVencAttr.u32PicWidth = 1920;
	stAttr.stVencAttr.u32PicHeight = 1080;
	stAttr.stVencAttr.enEncMode = VENC_MODE_RECOMMEND;
	stAttr.stVencAttr.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
	stAttr.stVencAttr.stAttrJpege.bSupportDCF = CVI_FALSE;
	stAttr.stVencAttr.stAttrJpege.stMPFCfg.u8LargeThumbNailNum = 0;
	stAttr.stVencAttr.stAttrJpege.enReceiveMode = VENC_PIC_RECEIVE_SINGLE;

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

	stChn.enModId = CVI_ID_JPEGE;
	stChn.s32ChnId = VeChn;
	rgn_init(Handle, stChn);

	rgn_update(Handle);

	sendThreadParam.s32ChnId = VeChn;
	sendThreadParam.enType = stAttr.stVencAttr.enType;
	memset(sendThreadParam.cFileName, 0, 128);
	memcpy(sendThreadParam.cFileName, argv[1], strlen(argv[1]));
	sendThreadParam.s32MilliSec = -1;
	sendThreadParam.s32IntervalTime = 1000;
	sendThreadParam.bDumpYUV = 1;
	ret = pthread_create(&sendThread, NULL, enc_proc_send, (CVI_VOID *)&sendThreadParam);

	getThreadParam.s32ChnId = 0;
	memset(getThreadParam.cFileName, 0, 128);
	memcpy(getThreadParam.cFileName, argv[2], strlen(argv[2]));
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

	rgn_deinit(Handle, stChn);

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


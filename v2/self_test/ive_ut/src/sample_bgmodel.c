#include "cvi_ive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _IVE__BGMDL1_PIX_S {
	CVI_U16 u16ShortKeepTime; /*Candidate background short hold time*/
	CVI_U8 u8ChgCond; /*Time condition for candidate background into the changing state*/
	CVI_U8 u8PotenBgLife; /*Potential background cumulative access time */
	CVI_U8 u8WorkBgLife[3]; /*1# ~ 3# background vitality */
	CVI_U8 u8CandiBgLife; /*Candidate background vitality */
} IVE_BGMDL1_PIX_S;

CVI_S32 InitMatchBgModel(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstCurImg,
			 IVE_DATA_S *pstBgModel, IVE_IMAGE_S *pstFgFlag,
			 IVE_DST_IMAGE_S *pstDiffFg,
			 IVE_DST_MEM_INFO_S *pstStatData,
			 IVE_MATCH_BG_MODEL_CTRL_S *pstMatchBgModelCtrl,
			 CVI_U32 u32Width, CVI_U32 u32Height)
{
	//malloc pstCurImg
	CVI_IVE_CreateImage(pIveHandle, pstCurImg, IVE_IMAGE_TYPE_U8C1,
			    u32Width, u32Height);

	//malloc pstBgModel
	CVI_IVE_CreateDataInfo(pIveHandle, pstBgModel,
			       u32Width * sizeof(IVE_BG_MODEL_PIX_S),
			       u32Height);

	//malloc pstFgFlag
	CVI_IVE_CreateImage(pIveHandle, pstFgFlag, IVE_IMAGE_TYPE_U8C1,
			    u32Width, u32Height);

	//malloc pstMatchFg
	CVI_IVE_CreateImage(pIveHandle, pstDiffFg, IVE_IMAGE_TYPE_S16C1,
			    u32Width, u32Height);

	//malloc pstStatData
	pstStatData->u64PhyAddr =
	pstStatData->u64VirAddr = (CVI_U64)(IVE_BG_STAT_DATA_S *)malloc(sizeof(IVE_BG_STAT_DATA_S));
	pstStatData->u32Size = sizeof(IVE_BG_STAT_DATA_S);

	//init ctrl info
	pstMatchBgModelCtrl->u32CurFrmNum = 0;
	pstMatchBgModelCtrl->u32PreFrmNum = 0;
	pstMatchBgModelCtrl->u16TimeThr = 20;
	pstMatchBgModelCtrl->u8DiffThrCrlCoef = 0;
	pstMatchBgModelCtrl->u8DiffMaxThr = 10;
	pstMatchBgModelCtrl->u8DiffMinThr = 10;
	pstMatchBgModelCtrl->u8DiffThrInc = 0;
	pstMatchBgModelCtrl->u8FastLearnRate = 4;
	pstMatchBgModelCtrl->u8DetChgRegion = 1;

	return CVI_SUCCESS;
}

CVI_S32 InitUpdateBgModel(IVE_HANDLE pIveHandle, IVE_DST_IMAGE_S *pstBgImg,
			  IVE_DST_IMAGE_S *pstChgSta,
			  IVE_UPDATE_BG_MODEL_CTRL_S *pstUpdateBgModelCtrl,
			  CVI_U16 u32Width, CVI_U16 u32Height)
{
	//malloc pstBgImg
	CVI_IVE_CreateImage(pIveHandle, pstBgImg, IVE_IMAGE_TYPE_U8C1, u32Width,
			    u32Height);

	//malloc pstChgSta
	CVI_IVE_CreateImage(pIveHandle, pstChgSta, IVE_IMAGE_TYPE_U32C1,
			    u32Width, u32Height);

	//init ctrl info
	pstUpdateBgModelCtrl->u32CurFrmNum = 0;
	pstUpdateBgModelCtrl->u32PreChkTime = 0;
	pstUpdateBgModelCtrl->u32FrmChkPeriod = 30;
	pstUpdateBgModelCtrl->u32InitMinTime = 25;
	pstUpdateBgModelCtrl->u32StyBgMinBlendTime = 100;
	pstUpdateBgModelCtrl->u32StyBgMaxBlendTime = 1500;
	pstUpdateBgModelCtrl->u32DynBgMinBlendTime = 0;
	pstUpdateBgModelCtrl->u32StaticDetMinTime = 80;
	pstUpdateBgModelCtrl->u16FgMaxFadeTime = 15;
	pstUpdateBgModelCtrl->u16BgMaxFadeTime = 60;
	pstUpdateBgModelCtrl->u8StyBgAccTimeRateThr = 80;
	pstUpdateBgModelCtrl->u8ChgBgAccTimeRateThr = 60;
	pstUpdateBgModelCtrl->u8DynBgAccTimeThr = 0;
	pstUpdateBgModelCtrl->u8DynBgDepth = 3;
	pstUpdateBgModelCtrl->u8BgEffStaRateThr = 90;
	pstUpdateBgModelCtrl->u8AcceBgLearn = 0;
	pstUpdateBgModelCtrl->u8DetChgRegion = 1;

	return CVI_SUCCESS;
}

CVI_VOID UninitMatchBgModel(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstCurImg,
			    IVE_DATA_S *pstBgModel, IVE_IMAGE_S *pstFgFlag,
			    IVE_DST_IMAGE_S *pstMatchFg,
			    IVE_DST_MEM_INFO_S *pstStatData)
{
	CVI_SYS_FreeI(pIveHandle, pstCurImg);
	CVI_SYS_FreeD(pIveHandle, pstBgModel);
	CVI_SYS_FreeI(pIveHandle, pstFgFlag);
	CVI_SYS_FreeI(pIveHandle, pstMatchFg);
	free((IVE_BG_STAT_DATA_S *)pstStatData->u64VirAddr);
}

CVI_VOID UninitUpdateBgModel(IVE_HANDLE pIveHandle, IVE_DST_IMAGE_S *pstBgImg,
			     IVE_DST_IMAGE_S *pstChgSta)
{
	CVI_SYS_FreeI(pIveHandle, pstBgImg);
	CVI_SYS_FreeI(pIveHandle, pstChgSta);
}

int test_bgmodel(int bTileMode, int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	int input_w, input_h;
	char input_data[64];
	char output_data[64];
	char output_file1[64];
	char output_file2[64];
	char output_file3[64];
	char output_file4[64];

	if (bTileMode) {
		strcpy(input_data, "res/ive/campus.u8c1.1_100.raw");
		strcpy(output_data, "res/ive/result/sample_tile_BgModelSample2_BgMdl_100.bin");
		input_w = 352 * 2;
		input_h = 288;
		strcpy(output_file1,
		       "sample_tile_BgModelSample2_BgMdl_100.bin");
		strcpy(output_file2,
		       "sample_tile_BgModelSample2_DiffFg_100.bin");
		strcpy(output_file3,
		       "sample_tile_BgModelSample2_ChgSta_100.bin");
		strcpy(output_file4,
		       "sample_tile_BgModelSample2_FgFlag_100.bin");
	} else {
		strcpy(input_data, "res/ive/campus.u8c1.1_100.raw");
		strcpy(output_data, "res/ive/result/sample_BgModelSample2_BgMdl_100.bin");
		input_w = 352;
		input_h = 288;
		strcpy(output_file1, "sample_BgModelSample2_BgMdl_100.bin");
		strcpy(output_file2, "sample_BgModelSample2_DiffFg_100.bin");
		strcpy(output_file3, "sample_BgModelSample2_ChgSta_100.bin");
		strcpy(output_file4, "sample_BgModelSample2_FgFlag_100.bin");
	}

	CVI_U32 u32FrameNum;
	CVI_S32 u32FrameNumMax = 100;
	CVI_S32 s32FrmCnt = 0;
	CVI_U32 u32UpdCnt = 5;
	CVI_U32 u32PreUpdTime = 0;
	CVI_U32 u32PreChkTime = 0;
	CVI_U32 u32FrmUpdPeriod = 10;
	CVI_U32 u32FrmChkPeriod = 30;

	IVE_HANDLE handle = CVI_IVE_CreateHandle();
	//Read Array from file.
	IVE_MEM_INFO_S stInput;

	CVI_IVE_ReadMem(handle, &stInput, input_data,
			input_w * input_h * u32FrameNumMax);
	// Create src image.
	IVE_SRC_IMAGE_S src;
	IVE_DATA_S stBgModel;
	IVE_IMAGE_S stFgFlag;
	IVE_DST_IMAGE_S stDiffFg;
	IVE_DST_MEM_INFO_S stStatData;
	IVE_MATCH_BG_MODEL_CTRL_S stMatchBgModelCtrl;

	InitMatchBgModel(handle, &src, &stBgModel, &stFgFlag, &stDiffFg,
			 &stStatData, &stMatchBgModelCtrl, input_w, input_h);

	// Create ref image.
	IVE_DATA_S ref_BgMdl;

	CVI_IVE_ReadData(handle, &ref_BgMdl, output_data,
			      input_w * sizeof(IVE_BG_MODEL_PIX_S), input_h);

	// Create dst image.
	IVE_DST_IMAGE_S stBgImg;
	IVE_DST_IMAGE_S stChgStaLife;
	IVE_UPDATE_BG_MODEL_CTRL_S stUpdateBgModelCtrl;

	InitUpdateBgModel(handle, &stBgImg, &stChgStaLife, &stUpdateBgModelCtrl,
			  input_w, input_h);

	// Config Setting.
	stMatchBgModelCtrl.u32CurFrmNum = s32FrmCnt;

	// Run IVE
	printf("Run HW IVE BgModel.\n");
	for (s32FrmCnt = 0; s32FrmCnt < u32FrameNumMax; s32FrmCnt++) {
		u32FrameNum = s32FrmCnt + 1;
		if (bTileMode) {
			for (int i = 0; i < 288; i++) {
				memcpy(&((char *)(uintptr_t)src
						 .u64VirAddr[0])[i * input_w],
				       (void *)(uintptr_t)(stInput.u64VirAddr+(s32FrmCnt * 352 * 288 +
						   i * 352)),
				       352);
				memcpy(&((char *)(uintptr_t)src
						 .u64VirAddr[0])[i * input_w +
								 352],
				       (void *)(uintptr_t)(stInput.u64VirAddr+(s32FrmCnt * 352 * 288 +
						   i * 352)),
				       352);
			}
		} else {
			for (int i = 0; i < 288; i++) {
				memcpy(&((char *)(uintptr_t)src
						 .u64VirAddr[0])[i * src.u32Stride[0]],
				       (void *)(uintptr_t)(stInput.u64VirAddr+(s32FrmCnt * input_w * 288 +
						   i * input_w)),
				       input_w);
				int stride = src.u32Stride[0] - input_w;

				memset(&((char *)(uintptr_t)src
						 .u64VirAddr[0])[i * src.u32Stride[0] + input_w], 0x0, stride);
			}
		}

		IVE_BG_STAT_DATA_S *stat =
			(IVE_BG_STAT_DATA_S *)(uintptr_t)stStatData.u64VirAddr;
		stMatchBgModelCtrl.u32PreFrmNum =
			stMatchBgModelCtrl.u32CurFrmNum;
		stMatchBgModelCtrl.u32CurFrmNum = u32FrameNum;

		CVI_IVE_MatchBgModel(handle, &src, &stBgModel, &stFgFlag,
				     &stDiffFg, &stStatData,
				     &stMatchBgModelCtrl, bInstant);
		printf("CVI_IVE_MatchBgModel u32UpdCnt %d, u32FrameNum %d, ",
		       u32UpdCnt, u32FrameNum);
		printf("frm %d, stat u32PixNum = %d, u32SumLum = %d\n",
		       s32FrmCnt, stat->u32PixNum, stat->u32SumLum);
		if ((u32UpdCnt == 0 ||
		     u32FrameNum >= u32PreUpdTime + u32FrmUpdPeriod)) {
			u32UpdCnt++;
			u32PreUpdTime = u32FrameNum;
			stUpdateBgModelCtrl.u32CurFrmNum = u32FrameNum;
			stUpdateBgModelCtrl.u32PreChkTime = u32PreChkTime;
			stUpdateBgModelCtrl.u32FrmChkPeriod = 0;
			if (u32FrameNum >= u32PreChkTime + u32FrmChkPeriod) {
				stUpdateBgModelCtrl.u32FrmChkPeriod =
					u32FrmChkPeriod;
				u32PreChkTime = u32FrameNum;
			}

			CVI_IVE_UpdateBgModel(handle, &src, &stBgModel, &stFgFlag,
					      &stBgImg, &stChgStaLife,
					      &stStatData, &stUpdateBgModelCtrl,
					      bInstant);
			printf("CVI_IVE_UpdateBgModel frm %d, stat u32PixNum = %d, u32SumLum = %d\n",
			       s32FrmCnt, stat->u32PixNum, stat->u32SumLum);
		}
	}
	ret |= CVI_IVE_CompareIveData(&stBgModel, &ref_BgMdl);
	if (bWrite) {
		CVI_IVE_WriteData(handle, output_file1, &stBgModel);
		CVI_IVE_WriteImg(handle, output_file2, &stDiffFg);
		CVI_IVE_WriteImg(handle, output_file3, &stChgStaLife);
		CVI_IVE_WriteImg(handle, output_file4, &stFgFlag);
		CVI_IVE_WriteImg(handle, "sample_BgModelSample2_BgImg.yuv", &stBgImg);
	}

	IVE_DST_IMAGE_S stBgDiffFg, stFrmDiffFg, stChgStaImg, stChgStaFg, stChStaLift;

	CVI_IVE_CreateImage(handle, &stBgDiffFg, IVE_IMAGE_TYPE_S8C1,
						stDiffFg.u32Width, stDiffFg.u32Height);
	CVI_IVE_CreateImage(handle, &stFrmDiffFg, IVE_IMAGE_TYPE_S8C1,
						stDiffFg.u32Width, stDiffFg.u32Height);
	CVI_IVE_CreateImage(handle, &stChgStaImg, IVE_IMAGE_TYPE_U8C1,
						stChgStaLife.u32Width, stChgStaLife.u32Height);
	CVI_IVE_CreateImage(handle, &stChgStaFg, IVE_IMAGE_TYPE_U8C1,
						stChgStaLife.u32Width, stChgStaLife.u32Height);
	CVI_IVE_CreateImage(handle, &stChStaLift, IVE_IMAGE_TYPE_U16C1,
						stChgStaLife.u32Width, stChgStaLife.u32Height);
	CVI_IVE_DiffFg_Split(handle, &stDiffFg, &stBgDiffFg, &stFrmDiffFg);
	CVI_IVE_ChgSta_Split(handle, &stChgStaLife, &stChgStaImg, &stChgStaFg, &stChStaLift);

	CVI_IVE_WriteImg(handle, "sample_BgModelSample2_BgDiffFg.yuv", &stBgDiffFg);
	CVI_IVE_WriteImg(handle, "sample_BgModelSample2_FrmDiffFg.yuv", &stFrmDiffFg);
	CVI_IVE_WriteImg(handle, "sample_BgModelSample2_ChgStaImg.yuv", &stChgStaImg);
	CVI_IVE_WriteImg(handle, "sample_BgModelSample2_ChgStaFg.yuv", &stChgStaFg);
	CVI_IVE_WriteImg(handle, "sample_BgModelSample2_ChStaLift.yuv", &stChStaLift);

	CVI_SYS_FreeI(handle, &stBgDiffFg);
	CVI_SYS_FreeI(handle, &stFrmDiffFg);
	CVI_SYS_FreeI(handle, &stChgStaImg);
	CVI_SYS_FreeI(handle, &stChgStaFg);
	CVI_SYS_FreeI(handle, &stChStaLift);

	UninitMatchBgModel(handle, &src, &stBgModel, &stFgFlag, &stDiffFg,
			   &stStatData);
	UninitUpdateBgModel(handle, &stBgImg, &stChgStaLife);
	CVI_SYS_FreeD(handle, &ref_BgMdl);
	CVI_SYS_FreeM(handle, &stInput);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

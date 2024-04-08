#include "cvi_ive.h"
// #include "cvi_pattern.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_gmm2(int bTileMode, int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	CVI_BOOL pixelctrl = CVI_FALSE;
	CVI_U32 u32FrameNumMax = 32;
	CVI_U32 s32FrmCnt = 0;
	IVE_SRC_IMAGE_S stIveImg;
	IVE_DST_IMAGE_S stIveFg;
	IVE_DST_IMAGE_S stIveBg;
	IVE_MEM_INFO_S stModel;
	IVE_GMM2_CTRL_S stGmm2Ctrl;
	IVE_SRC_IMAGE_S stFactor;
	IVE_DST_IMAGE_S stMatchModelInfo;

	int input_w, input_h;
	// char *input_data = NULL;
	// char *output_data_bg = NULL;
	// char *output_data_fg = NULL;
	char input_data[64];
	char output_data_bg[64];
	char output_data_fg[64];
	char output_file_fg[64];
	char output_file_bg[64];
	CVI_U32 u32FrmNum;

	if (bTileMode) {
		input_w = 704;
		input_h = 288;
		// input_data = data_campus_raw;
		// output_data_fg = data_tile_GMM2_FG;
		// output_data_bg = data_tile_GMM2_BG;
		strcpy(input_data, "res/ive/campus.u8c1.1_100.raw");
		strcpy(output_data_fg, "res/ive/result/sample_tile_GMM2_U8C1_fg_31.yuv");
		strcpy(output_data_bg, "res/ive/result/sample_tile_GMM2_U8C1_bg_31.yuv");
		strcpy(output_file_bg, "sample_tile_GMM2_U8C1_bg_31.yuv");
		strcpy(output_file_fg, "sample_tile_GMM2_U8C1_fg_31.yuv");
	} else {
		input_w = 352;
		input_h = 288;
		// input_data = data_campus_raw;
		// output_data_fg = data_GMM2_FG;
		// output_data_bg = data_GMM2_BG;
		strcpy(input_data, "res/ive/campus.u8c1.1_100.raw");
		strcpy(output_data_fg, "res/ive/result/sample_GMM2_U8C1_fg_31.yuv");
		strcpy(output_data_bg, "res/ive/result/sample_GMM2_U8C1_bg_31.yuv");
		strcpy(output_file_bg, "sample_GMM2_U8C1_bg_31.yuv");
		strcpy(output_file_fg, "sample_GMM2_U8C1_fg_31.yuv");
	}
	//Read Array from file.
	//Read Array from file.
	FILE *fp;
	int buf_size = 32 * input_w * input_h;
	char data_campus_raw[buf_size];

	fp = fopen(input_data, "r");
	if (fp == NULL) {
		printf("open file %s failed\n", input_data);
	}
	int readCnt = fread(data_campus_raw, 1, buf_size, fp);

	if (readCnt == 0) {
		printf("read file %s failed\n", input_data);
	}
	fclose(fp);

	stGmm2Ctrl.u16VarRate = 1;
	stGmm2Ctrl.u8ModelNum = 3;
	stGmm2Ctrl.u9q7MaxVar = (16 * 16) << 7;
	stGmm2Ctrl.u9q7MinVar = (8 * 8) << 7;
	stGmm2Ctrl.u8GlbSnsFactor = 8;
	stGmm2Ctrl.enSnsFactorMode = (pixelctrl) ?
					     IVE_GMM2_SNS_FACTOR_MODE_PIX :
					     IVE_GMM2_SNS_FACTOR_MODE_GLB;
	stGmm2Ctrl.u16FreqThr = 12000;
	stGmm2Ctrl.u16FreqInitVal = 20000;
	stGmm2Ctrl.u16FreqAddFactor = 0xEF;
	stGmm2Ctrl.u16FreqReduFactor = 0xFF00;
	stGmm2Ctrl.u16LifeThr = 5000;
	stGmm2Ctrl.enLifeUpdateFactorMode =
		IVE_GMM2_LIFE_UPDATE_FACTOR_MODE_GLB;

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	CVI_IVE_CreateImage(handle, &stIveImg, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);
	if (pixelctrl) {
		CVI_IVE_ReadRawImage(handle, &stFactor, "res/ive/sample_GMM2_U8C1_PixelCtrl_Factor.raw",
				       IVE_IMAGE_TYPE_U16C1, input_w, input_h);
	} else {
		CVI_IVE_CreateImage(handle, &stFactor, IVE_IMAGE_TYPE_U16C1,
				    input_w, input_h);
	}

	// Create ref image.
	IVE_IMAGE_S ref_fg, ref_bg;

	CVI_IVE_ReadRawImage(handle, &ref_fg, output_data_fg,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_bg, output_data_bg,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	IVE_IMAGE_S ref_pc_fg, ref_pc_bg, ref_pc_match;

	CVI_IVE_ReadRawImage(handle, &ref_pc_fg, "res/ive/result/sample_GMM2_U8C1_PixelCtrl_fg_31.yuv",
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_pc_bg, "res/ive/result/sample_GMM2_U8C1_PixelCtrl_fg_31.yuv",
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_pc_match,
			       "res/ive/result/sample_GMM2_U8C1_PixelCtrl_match_31.yuv",
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	// Create dst image.
	CVI_IVE_CreateImage(handle, &stIveFg, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);
	CVI_IVE_CreateImage(handle, &stIveBg, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);
	CVI_IVE_CreateImage(handle, &stMatchModelInfo, IVE_IMAGE_TYPE_U8C1,
			    input_w, input_h);

	CVI_IVE_CreateMemInfo(handle, &stModel,
			      stGmm2Ctrl.u8ModelNum * 8 * input_w * input_h);

	// Run IVE
	printf("Run HW IVE GMM2.\n");
	for (s32FrmCnt = 0; s32FrmCnt < u32FrameNumMax; s32FrmCnt++) {
		if (bTileMode) {
			for (int i = 0; i < 288; i++) {
				memcpy(&((char *)(uintptr_t)stIveImg
						 .u64VirAddr[0])[i * input_w],
				       &data_campus_raw[s32FrmCnt * 352 * 288 +
						   i * 352],
				       352);
				memcpy(&((char *)(uintptr_t)stIveImg
						 .u64VirAddr[0])[i * input_w +
								 352],
				       &data_campus_raw[s32FrmCnt * 352 * 288 +
						   i * 352],
				       352);
			}
		} else {
			for (int i = 0; i < 288; i++) {
				memcpy(&((char *)(uintptr_t)stIveImg
						 .u64VirAddr[0])[i * stIveImg.u32Stride[0]],
				       &data_campus_raw[s32FrmCnt * input_w * 288 +
						   i * input_w],
				       input_w);
				int stride = stIveImg.u32Stride[0] - input_w;

				memset(&((char *)(uintptr_t)stIveImg
						 .u64VirAddr[0])[i * stIveImg.u32Stride[0] + input_w], 0x0, stride);
			}
		}
		u32FrmNum = s32FrmCnt + 1;
		if (stGmm2Ctrl.u8ModelNum == 1) {
			//If the parameter u8ModelNum is set to 1, the parameter u16FreqReduFactor
			//is usually set to a small value at the first N frames. Here, N = 500.
			stGmm2Ctrl.u16FreqReduFactor =
				(u32FrmNum >= 500) ? 0xFFA0 : 0xFC00;
		} else {
			//If the parameter u8ModelNum is more than 1, the global
			// life mode should be used at the first N frames,
			//and the parameter u16GlbLifeUpdateFactor is usually
			// set to a big value. Here, N = 500.
			stGmm2Ctrl.u16GlbLifeUpdateFactor =
				(u32FrmNum >= 500) ? 4 : 0xFFFF / u32FrmNum;
		}
		if (pixelctrl && u32FrmNum > 16)
			stGmm2Ctrl.enLifeUpdateFactorMode =
				IVE_GMM2_LIFE_UPDATE_FACTOR_MODE_PIX;

		CVI_IVE_GMM2(handle, &stIveImg, &stFactor, &stIveFg, &stIveBg,
			     &stMatchModelInfo, &stModel, &stGmm2Ctrl,
			     bInstant);
	}

	if (pixelctrl) {
		ret |= CVI_IVE_CompareIveImage(&stIveFg, &ref_pc_fg);
		ret |= CVI_IVE_CompareIveImage(&stIveBg, &ref_pc_bg);
		ret |= CVI_IVE_CompareIveImage(&stMatchModelInfo,
					       &ref_pc_match);
		if (bWrite) {
			CVI_IVE_WriteImg(handle, output_file_fg, &stIveFg);
			CVI_IVE_WriteImg(handle, output_file_bg, &stIveBg);
			CVI_IVE_WriteImg(
				handle,
				"sample_GMM2_U8C1_PixelCtrl_match_31.yuv",
				&stMatchModelInfo);
		}
	} else {
		ret |= CVI_IVE_CompareIveImage(&stIveFg, &ref_fg);
		ret |= CVI_IVE_CompareIveImage(&stIveBg, &ref_bg);
		if (bWrite) {
			CVI_IVE_WriteImg(handle, output_file_fg, &stIveFg);
			CVI_IVE_WriteImg(handle, output_file_bg, &stIveBg);
		}
	}

	CVI_SYS_FreeI(handle, &stIveImg);
	CVI_SYS_FreeI(handle, &stFactor);
	CVI_SYS_FreeI(handle, &stIveFg);
	CVI_SYS_FreeI(handle, &stIveBg);
	CVI_SYS_FreeI(handle, &stMatchModelInfo);
	CVI_SYS_FreeI(handle, &ref_fg);
	CVI_SYS_FreeI(handle, &ref_bg);
	CVI_SYS_FreeI(handle, &ref_pc_fg);
	CVI_SYS_FreeI(handle, &ref_pc_bg);
	CVI_SYS_FreeI(handle, &ref_pc_match);
	CVI_SYS_FreeM(handle, &stModel);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

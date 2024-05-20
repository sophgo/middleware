#include "cvi_ive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_gmm(int bTileMode, int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;

	CVI_U32 u32FrameNumMax = 32;
	CVI_U32 s32FrmCnt = 0;
	IVE_GMM_CTRL_S stGMMCtrl;

	int input_w, input_h;
	char input_data[64];
	char output_data_bg[64];
	char output_data_fg[64];
	char output_file_fg[64];
	char output_file_bg[64];

	stGMMCtrl.u0q16BgRatio = 45875;
	stGMMCtrl.u0q16InitWeight = 3277;
	stGMMCtrl.u22q10NoiseVar = 225 * 1024;
	stGMMCtrl.u22q10MaxVar = 2000 * 1024;
	stGMMCtrl.u22q10MinVar = 200 * 1024;
	stGMMCtrl.u8q8VarThr = (CVI_U16)(256 * 6.25);
	stGMMCtrl.u8ModelNum = 3;

	if (bTileMode) {
		strcpy(input_data, "res/ive/campus.u8c1.1_100.raw");
		strcpy(output_data_fg, "res/ive/result/sample_tile_GMM_U8C1_fg_31.yuv");
		strcpy(output_data_bg, "res/ive/result/sample_tile_GMM_U8C1_bg_31.yuv");
		input_w = 704;
		input_h = 288;
		strcpy(output_file_bg, "sample_tile_GMM_U8C1_bg_31.yuv");
		strcpy(output_file_fg, "sample_tile_GMM_U8C1_fg_31.yuv");
	} else {
		strcpy(input_data, "res/ive/campus.u8c1.1_100.raw");
		strcpy(output_data_fg, "res/ive/result/sample_GMM_U8C1_fg_31.yuv");
		strcpy(output_data_bg, "res/ive/result/sample_GMM_U8C1_bg_31.yuv");
		input_w = 352;
		input_h = 288;
		strcpy(output_file_bg, "sample_GMM_U8C1_bg_31.yuv");
		strcpy(output_file_fg, "sample_GMM_U8C1_fg_31.yuv");
	}

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

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src;

	CVI_IVE_CreateImage(handle, &src, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);

	IVE_DST_MEM_INFO_S stInput;

	CVI_IVE_ReadMem(handle, &stInput, input_data,
			     input_w * input_h * u32FrameNumMax);

	// Create ref image.
	IVE_IMAGE_S ref_fg, ref_bg;

	CVI_IVE_ReadRawImage(handle, &ref_fg, output_data_fg,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_bg, output_data_bg,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	// Create dst image.
	IVE_DST_IMAGE_S dst_bg, dst_fg;

	CVI_IVE_CreateImage(handle, &dst_bg, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);
	CVI_IVE_CreateImage(handle, &dst_fg, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);

	// Config Setting.
	IVE_MEM_INFO_S stModel;

	CVI_IVE_CreateMemInfo(handle, &stModel,
			      stGMMCtrl.u8ModelNum * 8 * input_w * input_h);
	memset((char *)(uintptr_t)stModel.u64VirAddr, 0, stModel.u32Size);
	// Run IVE
	printf("Run HW IVE GMM.\n");
	for (s32FrmCnt = 0; s32FrmCnt < u32FrameNumMax; s32FrmCnt++) {
		if (bTileMode) {
			for (int i = 0; i < 288; i++) {
				memcpy(&((char *)(uintptr_t)src
						 .u64VirAddr[0])[i * input_w],
				       &((char *)(uintptr_t)stInput.u64VirAddr)
					       [s32FrmCnt * 352 * 288 + i * 352],
				       352);
				memcpy(&((char *)(uintptr_t)src
						 .u64VirAddr[0])[i * input_w +
								 352],
				       &((char *)(uintptr_t)stInput.u64VirAddr)
					       [s32FrmCnt * 352 * 288 + i * 352],
				       352);
			}
		} else {
			for (int i = 0; i < 288; i++) {
				memcpy(&((char *)(uintptr_t)src
						 .u64VirAddr[0])[i * src.u32Stride[0]],
				       &data_campus_raw[s32FrmCnt * input_w * 288 +
						   i * input_w],
				       input_w);
				int stride = src.u32Stride[0] - input_w;

				memset(&((char *)(uintptr_t)src
						 .u64VirAddr[0])[i * src.u32Stride[0] + input_w], 0x0, stride);
			}
		}

		if (s32FrmCnt >= 500) {
			stGMMCtrl.u0q16LearnRate = 131; //0.02
		} else {
			stGMMCtrl.u0q16LearnRate = 65535 / (s32FrmCnt + 1);
		}
		CVI_IVE_GMM(handle, &src, &dst_fg, &dst_bg, &stModel,
			    &stGMMCtrl, bInstant);
	}
	ret |= CVI_IVE_CompareIveImage(&dst_fg, &ref_fg);
	ret |= CVI_IVE_CompareIveImage(&dst_bg, &ref_bg);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_fg, &dst_fg);
		CVI_IVE_WriteImg(handle, output_file_bg, &dst_bg);
	}

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeI(handle, &dst_bg);
	CVI_SYS_FreeI(handle, &dst_fg);
	CVI_SYS_FreeI(handle, &ref_fg);
	CVI_SYS_FreeI(handle, &ref_bg);
	CVI_SYS_FreeM(handle, &stInput);
	CVI_SYS_FreeM(handle, &stModel);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

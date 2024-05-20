#include "cvi_ive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_sobel(int bTileMode, int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	int input_w, input_h;
	char input_data[64];
	char output_data_Hor3x3[64];
	char output_data_Ver3x3[64];
	char output_data_Hor5x5[64];
	char output_data_Ver5x5[64];

	char output_file_Hor3x3[64];
	char output_file_Ver3x3[64];
	char output_file_Both_Hor3x3[64];
	char output_file_Both_Ver3x3[64];
	char output_file_Hor5x5[64];
	char output_file_Ver5x5[64];
	char output_file_Both_Hor5x5[64];
	char output_file_Both_Ver5x5[64];

	if (bTileMode) {
		input_w = 640;
		input_h = 480;
		strcpy(input_data, "res/ive/sky_640x480.yuv");
		strcpy(output_data_Hor3x3, "res/ive/result/sample_tile_Sobel_Hor3x3.yuv");
		strcpy(output_data_Ver3x3, "res/ive/result/sample_tile_Sobel_Ver3x3.yuv");
		strcpy(output_data_Hor5x5, "res/ive/result/sample_tile_Sobel_Hor5x5.yuv");
		strcpy(output_data_Ver5x5, "res/ive/result/sample_tile_Sobel_Ver5x5.yuv");
		strcpy(output_file_Hor3x3, "sample_tile_Sobel_Hor3x3.yuv");
		strcpy(output_file_Ver3x3, "sample_tile_Sobel_Ver3x3.yuv");
		strcpy(output_file_Both_Hor3x3,
		       "sample_tile_Sobel_Both3x3_H.yuv");
		strcpy(output_file_Both_Ver3x3,
		       "sample_tile_Sobel_Both3x3_V.yuv");
		strcpy(output_file_Hor5x5, "sample_tile_Sobel_Hor5x5.yuv");
		strcpy(output_file_Ver5x5, "sample_tile_Sobel_Ver5x5.yuv");
		strcpy(output_file_Both_Hor5x5,
		       "sample_tile_Sobel_Both5x5_H.yuv");
		strcpy(output_file_Both_Ver5x5,
		       "sample_tile_Sobel_Both5x5_V.yuv");
	} else {
		input_w = 352;
		input_h = 288;
		strcpy(input_data, "res/ive/00_352x288_y.yuv");
		strcpy(output_data_Hor3x3, "res/ive/result/sample_Sobel_Hor3x3.yuv");
		strcpy(output_data_Ver3x3, "res/ive/result/sample_Sobel_Ver3x3.yuv");
		strcpy(output_data_Hor5x5, "res/ive/result/sample_Sobel_Hor5x5.yuv");
		strcpy(output_data_Ver5x5, "res/ive/result/sample_Sobel_Ver5x5.yuv");
		strcpy(output_file_Hor3x3, "sample_Sobel_Hor3x3.yuv");
		strcpy(output_file_Ver3x3, "sample_Sobel_Ver3x3.yuv");
		strcpy(output_file_Both_Hor3x3, "sample_Sobel_Both3x3_H.yuv");
		strcpy(output_file_Both_Ver3x3, "sample_Sobel_Both3x3_V.yuv");
		strcpy(output_file_Hor5x5, "sample_Sobel_Hor5x5.yuv");
		strcpy(output_file_Ver5x5, "sample_Sobel_Ver5x5.yuv");
		strcpy(output_file_Both_Hor5x5, "sample_Sobel_Both5x5_H.yuv");
		strcpy(output_file_Both_Ver5x5, "sample_Sobel_Both5x5_V.yuv");
	}
	/* 5 by 5*/
	CVI_S8 arr5by5[25] = { -1, -2, 0,  2,  1, -4, -8, 0,  8,  4, -6, -12, 0,
			       12, 6,  -4, -8, 0, 8,  4,  -1, -2, 0, 2,	 1 };
	/* 3 by 3*/
	CVI_S8 arr3by3[25] = { 0, 0, 0, 0,  0, 0, -1, 0, 1, 0, 0, -2, 0,
			       2, 0, 0, -1, 0, 1, 0,  0, 0, 0, 0, 0 };
	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src;

	CVI_IVE_ReadRawImage(handle, &src, input_data, IVE_IMAGE_TYPE_U8C1,
			       input_w, input_h);

	// Create ref image.
	IVE_IMAGE_S ref_hor3x3, ref_hor5x5, ref_ver3x3, ref_ver5x5;

	CVI_IVE_ReadRawImage(handle, &ref_hor3x3, output_data_Hor3x3,
			       IVE_IMAGE_TYPE_S16C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_hor5x5, output_data_Hor5x5,
			       IVE_IMAGE_TYPE_S16C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_ver3x3, output_data_Ver3x3,
			       IVE_IMAGE_TYPE_S16C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_ver5x5, output_data_Ver5x5,
			       IVE_IMAGE_TYPE_S16C1, input_w, input_h);

	// Create dst image.
	IVE_DST_IMAGE_S dst_h, dst_v;

	CVI_IVE_CreateImage(handle, &dst_h, IVE_IMAGE_TYPE_S16C1, input_w,
			    input_h);
	CVI_IVE_CreateImage(handle, &dst_v, IVE_IMAGE_TYPE_S16C1, input_w,
			    input_h);

	// Config Setting.
	IVE_SOBEL_CTRL_S iveSblCtrl;

	memcpy(iveSblCtrl.as8Mask, arr5by5, 5 * 5 * sizeof(CVI_S8));

	// Run HW IVE.
	printf("Run HW IVE Sobel 5x5 Both.\n");
	iveSblCtrl.enOutCtrl = IVE_SOBEL_OUT_CTRL_BOTH;
	CVI_IVE_Sobel(handle, &src, &dst_h, &dst_v, &iveSblCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst_h, &ref_hor5x5);
	ret |= CVI_IVE_CompareIveImage(&dst_v, &ref_ver5x5);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_Both_Hor5x5, &dst_h);
		CVI_IVE_WriteImg(handle, output_file_Both_Ver5x5, &dst_v);
	}

	printf("Run HW IVE Sobel 5x5 Ver.\n");
	iveSblCtrl.enOutCtrl = IVE_SOBEL_OUT_CTRL_VER;
	CVI_IVE_Sobel(handle, &src, CVI_NULL, &dst_v, &iveSblCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst_v, &ref_ver5x5);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_Ver5x5, &dst_v);
	}

	printf("Run HW IVE Sobel 5x5 Hor.\n");
	iveSblCtrl.enOutCtrl = IVE_SOBEL_OUT_CTRL_HOR;
	CVI_IVE_Sobel(handle, &src, &dst_h, CVI_NULL, &iveSblCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst_h, &ref_hor5x5);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_Hor5x5, &dst_h);
	}

	memcpy(iveSblCtrl.as8Mask, arr3by3, 5 * 5 * sizeof(CVI_S8));

	printf("Run HW IVE Sobel 3x3 Both.\n");
	iveSblCtrl.enOutCtrl = IVE_SOBEL_OUT_CTRL_BOTH;
	CVI_IVE_Sobel(handle, &src, &dst_h, &dst_v, &iveSblCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst_h, &ref_hor3x3);
	ret |= CVI_IVE_CompareIveImage(&dst_v, &ref_ver3x3);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_Both_Hor3x3, &dst_h);
		CVI_IVE_WriteImg(handle, output_file_Both_Ver3x3, &dst_v);
	}

	printf("Run HW IVE Sobel 3x3 Ver.\n");
	iveSblCtrl.enOutCtrl = IVE_SOBEL_OUT_CTRL_VER;
	CVI_IVE_Sobel(handle, &src, CVI_NULL, &dst_v, &iveSblCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst_v, &ref_ver3x3);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_Ver3x3, &dst_v);
	}

	printf("Run HW IVE Sobel 3x3 Hor.\n");
	iveSblCtrl.enOutCtrl = IVE_SOBEL_OUT_CTRL_HOR;
	CVI_IVE_Sobel(handle, &src, &dst_h, CVI_NULL, &iveSblCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst_h, &ref_hor3x3);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_Hor3x3, &dst_h);
	}

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeI(handle, &ref_hor3x3);
	CVI_SYS_FreeI(handle, &ref_hor5x5);
	CVI_SYS_FreeI(handle, &ref_ver3x3);
	CVI_SYS_FreeI(handle, &ref_ver5x5);
	CVI_SYS_FreeI(handle, &dst_h);
	CVI_SYS_FreeI(handle, &dst_v);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

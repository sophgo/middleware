#include "cvi_ive.h"
// #include "cvi_pattern.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_normgrad(int bTileMode, int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	int input_w, input_h;
	// char *input_data = NULL;
	// char *output_data_Hor3x3 = NULL;
	// char *output_data_Ver3x3 = NULL;
	// char *output_data_Combine3x3 = NULL;
	// char *output_data_Hor5x5 = NULL;
	// char *output_data_Ver5x5 = NULL;
	char input_data[64];
	char output_data_Hor3x3[64];
	char output_data_Ver3x3[64];
	char output_data_Combine3x3[64];
	char output_data_Hor5x5[64];
	char output_data_Ver5x5[64];

	char output_file_Both3x3_H[64];
	char output_file_Both3x3_V[64];
	char output_file_Hor3x3[64];
	char output_file_Ver3x3[64];
	char output_file_Combine3x3[64];
	char output_file_Both5x5_H[64];
	char output_file_Both5x5_V[64];
	char output_file_Hor5x5[64];
	char output_file_Ver5x5[64];

	if (bTileMode) {
		// input_data = data_tile_640x480_y;
		// output_data_Hor3x3 = data_tile_NormGrad_Hor3x3;
		// output_data_Ver3x3 = data_tile_NormGrad_Ver3x3;
		// output_data_Combine3x3 = data_tile_NormGrad_Combine3x3;
		// output_data_Hor5x5 = data_tile_NormGrad_Hor5x5;
		// output_data_Ver5x5 = data_tile_NormGrad_Ver5x5;
		strcpy(input_data, "res/ive/sky_640x480.yuv");
		strcpy(output_data_Hor3x3, "res/ive/result/sample_tile_NormGrad_Hor3x3.yuv");
		strcpy(output_data_Ver3x3, "res/ive/result/sample_tile_NormGrad_Ver3x3.yuv");
		strcpy(output_data_Combine3x3, "res/ive/result/sample_tile_NormGrad_Combine3x3.yuv");
		strcpy(output_data_Hor5x5, "res/ive/result/sample_tile_NormGrad_Hor5x5.yuv");
		strcpy(output_data_Ver5x5, "res/ive/result/sample_tile_NormGrad_Ver5x5.yuv");
		input_w = 640;
		input_h = 480;
		strcpy(output_file_Both3x3_H,
		       "sample_tile_NormGrad_Both3x3_H.yuv");
		strcpy(output_file_Both3x3_V,
		       "sample_tile_NormGrad_Both3x3_V.yuv");
		strcpy(output_file_Hor3x3, "sample_tile_NormGrad_Hor3x3.yuv");
		strcpy(output_file_Ver3x3, "sample_tile_NormGrad_Ver3x3.yuv");
		strcpy(output_file_Combine3x3,
		       "sample_tile_NormGrad_Combine3x3.yuv");
		strcpy(output_file_Both5x5_H,
		       "sample_tile_NormGrad_Both5x5_H.yuv");
		strcpy(output_file_Both5x5_V,
		       "sample_tile_NormGrad_Both5x5_V.yuv");
		strcpy(output_file_Hor5x5, "sample_tile_NormGrad_Hor5x5.yuv");
		strcpy(output_file_Ver5x5, "sample_tile_NormGrad_Ver5x5.yuv");
	} else {
		// input_data = data_00_352x288_y;
		// output_data_Hor3x3 = data_NormGrad_Hor3x3;
		// output_data_Ver3x3 = data_NormGrad_Ver3x3;
		// output_data_Combine3x3 = data_NormGrad_Combine3x3;
		// output_data_Hor5x5 = data_NormGrad_Hor5x5;
		// output_data_Ver5x5 = data_NormGrad_Ver5x5;
		strcpy(input_data, "res/ive/00_352x288_y.yuv");
		strcpy(output_data_Hor3x3, "res/ive/result/sample_NormGrad_Hor3x3.yuv");
		strcpy(output_data_Ver3x3, "res/ive/result/sample_NormGrad_Ver3x3.yuv");
		strcpy(output_data_Combine3x3, "res/ive/result/sample_NormGrad_Combine3x3.yuv");
		strcpy(output_data_Hor5x5, "res/ive/result/sample_NormGrad_Hor5x5.yuv");
		strcpy(output_data_Ver5x5, "res/ive/result/sample_NormGrad_Ver5x5.yuv");
		input_w = 352;
		input_h = 288;
		strcpy(output_file_Both3x3_H, "sample_NormGrad_Both3x3_H.yuv");
		strcpy(output_file_Both3x3_V, "sample_NormGrad_Both3x3_V.yuv");
		strcpy(output_file_Hor3x3, "sample_NormGrad_Hor3x3.yuv");
		strcpy(output_file_Ver3x3, "sample_NormGrad_Ver3x3.yuv");
		strcpy(output_file_Combine3x3,
		       "sample_NormGrad_Combine3x3.yuv");
		strcpy(output_file_Both5x5_H, "sample_NormGrad_Both5x5_H.yuv");
		strcpy(output_file_Both5x5_V, "sample_NormGrad_Both5x5_V.yuv");
		strcpy(output_file_Hor5x5, "sample_NormGrad_Hor5x5.yuv");
		strcpy(output_file_Ver5x5, "sample_NormGrad_Ver5x5.yuv");
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
	IVE_IMAGE_S ref_hor3x3, ref_hor5x5, ref_ver3x3, ref_ver5x5, ref_comb3x3;

	CVI_IVE_ReadRawImage(handle, &ref_hor3x3, output_data_Hor3x3,
			       IVE_IMAGE_TYPE_S8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_hor5x5, output_data_Hor5x5,
			       IVE_IMAGE_TYPE_S8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_ver3x3, output_data_Ver3x3,
			       IVE_IMAGE_TYPE_S8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_ver5x5, output_data_Ver5x5,
			       IVE_IMAGE_TYPE_S8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_comb3x3, output_data_Combine3x3,
			       IVE_IMAGE_TYPE_U16C1, input_w, input_h);

	// Create dst image.
	IVE_DST_IMAGE_S dst_h, dst_v, dst_comb;

	CVI_IVE_CreateImage(handle, &dst_h, IVE_IMAGE_TYPE_S8C1, input_w,
			    input_h);
	CVI_IVE_CreateImage(handle, &dst_v, IVE_IMAGE_TYPE_S8C1, input_w,
			    input_h);
	CVI_IVE_CreateImage(handle, &dst_comb, IVE_IMAGE_TYPE_U16C1, input_w,
			    input_h);

	// Config Setting.
	IVE_NORM_GRAD_CTRL_S pstNormGradCtrl;

	pstNormGradCtrl.u8Norm = 8;
	memcpy(pstNormGradCtrl.as8Mask, arr3by3, 5 * 5 * sizeof(CVI_S8));
	// Run HW IVE.
	printf("Run HW IVE Norm Gradient 3x3 HOR And VER.\n");
	pstNormGradCtrl.enOutCtrl = IVE_NORM_GRAD_OUT_CTRL_HOR_AND_VER;
	CVI_IVE_NormGrad(handle, &src, &dst_h, &dst_v, CVI_NULL,
			 &pstNormGradCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst_h, &ref_hor3x3);
	ret |= CVI_IVE_CompareIveImage(&dst_v, &ref_ver3x3);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_Both3x3_H, &dst_h);
		CVI_IVE_WriteImg(handle, output_file_Both3x3_V, &dst_v);
	}

	printf("Run HW IVE Norm Gradient 3x3 Hor.\n");
	pstNormGradCtrl.enOutCtrl = IVE_NORM_GRAD_OUT_CTRL_HOR;
	CVI_IVE_NormGrad(handle, &src, &dst_h, CVI_NULL, CVI_NULL,
			 &pstNormGradCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst_h, &ref_hor3x3);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_Hor3x3, &dst_h);
	}

	printf("Run HW IVE Norm Gradient 3x3 Ver.\n");
	pstNormGradCtrl.enOutCtrl = IVE_NORM_GRAD_OUT_CTRL_VER;
	CVI_IVE_NormGrad(handle, &src, CVI_NULL, &dst_v, CVI_NULL,
			 &pstNormGradCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst_v, &ref_ver3x3);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_Ver3x3, &dst_v);
	}

	printf("Run HW IVE Norm Gradient 3x3 COMBINE.\n");
	pstNormGradCtrl.enOutCtrl = IVE_NORM_GRAD_OUT_CTRL_COMBINE;
	CVI_IVE_NormGrad(handle, &src, CVI_NULL, CVI_NULL, &dst_comb,
			 &pstNormGradCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst_comb, &ref_comb3x3);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_Combine3x3, &dst_comb);
	}

	memcpy(pstNormGradCtrl.as8Mask, arr5by5, 5 * 5 * sizeof(CVI_S8));

	printf("Run HW IVE Norm Gradient 5x5 HOR And VER.\n");
	pstNormGradCtrl.enOutCtrl = IVE_NORM_GRAD_OUT_CTRL_HOR_AND_VER;
	CVI_IVE_NormGrad(handle, &src, &dst_h, &dst_v, CVI_NULL,
			 &pstNormGradCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst_h, &ref_hor5x5);
	ret |= CVI_IVE_CompareIveImage(&dst_v, &ref_ver5x5);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_Both5x5_H, &dst_h);
		CVI_IVE_WriteImg(handle, output_file_Both5x5_V, &dst_v);
	}

	printf("Run HW IVE Norm Gradient 5x5 Hor.\n");
	pstNormGradCtrl.enOutCtrl = IVE_NORM_GRAD_OUT_CTRL_HOR;
	CVI_IVE_NormGrad(handle, &src, &dst_h, CVI_NULL, CVI_NULL,
			 &pstNormGradCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst_h, &ref_hor5x5);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_Hor5x5, &dst_h);
	}

	printf("Run HW IVE Norm Gradient 5x5 Ver.\n");
	pstNormGradCtrl.enOutCtrl = IVE_NORM_GRAD_OUT_CTRL_VER;
	CVI_IVE_NormGrad(handle, &src, CVI_NULL, &dst_v, CVI_NULL,
			 &pstNormGradCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst_v, &ref_ver5x5);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_Ver5x5, &dst_v);
	}

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeI(handle, &ref_hor3x3);
	CVI_SYS_FreeI(handle, &ref_hor5x5);
	CVI_SYS_FreeI(handle, &ref_comb3x3);
	CVI_SYS_FreeI(handle, &ref_ver3x3);
	CVI_SYS_FreeI(handle, &ref_ver5x5);
	CVI_SYS_FreeI(handle, &dst_h);
	CVI_SYS_FreeI(handle, &dst_v);
	CVI_SYS_FreeI(handle, &dst_comb);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}
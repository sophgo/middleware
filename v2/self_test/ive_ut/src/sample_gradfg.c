#include "cvi_ive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint16_t bswap_16(uint16_t x)
{
	return ((uint16_t)(x) & 0xff00) >> 8 |
			((uint16_t)(x) & 0x00ff) << 8;
}
CVI_S32 bswap(IVE_HANDLE pIveHandle, IVE_DST_IMAGE_S *pstSrc, IVE_DST_IMAGE_S *pstDst)
{
		UNUSED(pIveHandle);
		size_t u32Height = pstSrc->u32Height;
		size_t u32Width = pstSrc->u32Width;

		for (size_t i = 0; i < u32Height*u32Width; i++) {
			((uint16_t *)pstDst->u64VirAddr[0])[i] = bswap_16(((uint16_t *)pstSrc->u64VirAddr[0])[i]);
		}
	return 0;
}
int test_gradfg(int bTileMode, int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	int input_w, input_h;
	char input_data[64];
	char output_data_MIN[64];
	char output_data_CUR[64];
	char output_file_Min[64];
	char output_file_Cur[64];

	if (bTileMode) {
		input_w = 640;
		input_h = 480;
		strcpy(input_data, "res/ive/sky_640x480.yuv");
		strcpy(output_data_MIN, "res/ive/result/sample_tile_GradFg_FIND_MIN_GRAD.out");
		strcpy(output_data_CUR, "res/ive/result/sample_tile_GradFg_USE_CUR_GRAD.out");
		strcpy(output_file_Min, "sample_tile_GradFg_FIND_MIN_GRAD.out");
		strcpy(output_file_Cur, "sample_tile_GradFg_USE_CUR_GRAD.out");
	} else {
		input_w = 352;
		input_h = 288;
		strcpy(input_data, "res/ive/00_352x288_y.yuv");
		strcpy(output_data_MIN, "res/ive/result/sample_GradFg_FIND_MIN_GRAD.out");
		strcpy(output_data_CUR, "res/ive/result/sample_GradFg_USE_CUR_GRAD.out");
		strcpy(output_file_Min, "sample_GradFg_FIND_MIN_GRAD.out");
		strcpy(output_file_Cur, "sample_GradFg_USE_CUR_GRAD.out");
	}
	/* 3 by 3*/
	CVI_S8 arr3by3[25] = { 0, 0, 0, 0,  0, 0, -1, 0, 1, 0, 0, -2, 0,
			       2, 0, 0, -1, 0, 1, 0,  0, 0, 0, 0, 0 };

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S stBgDiffFg;

	CVI_IVE_ReadRawImage(handle, &stBgDiffFg, input_data,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	// Create ref image.
	IVE_IMAGE_S ref_min, ref_cur;

	CVI_IVE_ReadRawImage(handle, &ref_min, output_data_MIN,
					IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_cur, output_data_CUR,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	// Create dst image.
	IVE_DST_IMAGE_S stCurGrad, stBgGrad, stGradFg, stCurGradSwap, stBgGradSwap;

	CVI_IVE_CreateImage(handle, &stCurGrad, IVE_IMAGE_TYPE_U16C1, input_w,
			    input_h);
	CVI_IVE_CreateImage(handle, &stBgGrad, IVE_IMAGE_TYPE_U16C1, input_w,
			    input_h);
	CVI_IVE_CreateImage(handle, &stGradFg, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);

	CVI_IVE_CreateImage(handle, &stCurGradSwap, IVE_IMAGE_TYPE_U16C1, input_w,
			    input_h);
	CVI_IVE_CreateImage(handle, &stBgGradSwap, IVE_IMAGE_TYPE_U16C1, input_w,
			    input_h);

	// Config Setting.
	IVE_GRAD_FG_CTRL_S stCtrl;
	stCtrl.enMode = IVE_GRAD_FG_MODE_FIND_MIN_GRAD;
	stCtrl.u16EdwFactor = 1000;
	stCtrl.u8CrlCoefThr = 80;
	stCtrl.u8MagCrlThr = 4;
	stCtrl.u8MinMagDiff = 2;
	stCtrl.u8NoiseVal = 1;
	stCtrl.u8EdwDark = 1;

	IVE_NORM_GRAD_OUT_CTRL_E enOutCtrl = IVE_NORM_GRAD_OUT_CTRL_COMBINE;
	IVE_NORM_GRAD_CTRL_S stNormGradCtrl;

	stNormGradCtrl.enOutCtrl = enOutCtrl;
	memcpy(stNormGradCtrl.as8Mask, arr3by3, sizeof(CVI_S8) * 25);
	stNormGradCtrl.u8Norm = 8;

	// Create Fake Data. And Change Endianness
	CVI_IVE_NormGrad(handle, &stBgDiffFg, NULL, NULL, &stCurGrad,
			 &stNormGradCtrl, bInstant);

	stNormGradCtrl.u8Norm = 2;
	CVI_IVE_NormGrad(handle, &stBgDiffFg, NULL, NULL,  &stBgGrad,
			 &stNormGradCtrl, bInstant);

	bswap(handle, &stBgGrad, &stBgGradSwap);
	bswap(handle, &stCurGrad, &stCurGradSwap);

	// Run HW IVE.
	printf("Run HW IVE GradFg FIND_MIN_GRAD.\n");
	CVI_IVE_GradFg(handle, &stBgDiffFg, &stCurGradSwap, &stBgGradSwap, &stGradFg,
		       &stCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&stGradFg, &ref_min);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_Min, &stGradFg);
	}

	printf("Run HW IVE GradFg USE_CUR_GRAD.\n");
	stCtrl.enMode = IVE_GRAD_FG_MODE_USE_CUR_GRAD;
	CVI_IVE_GradFg(handle, &stBgDiffFg, &stCurGrad, &stBgGrad, &stGradFg,
		       &stCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&stGradFg, &ref_cur);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_Cur, &stGradFg);
	}

	CVI_SYS_FreeI(handle, &stBgDiffFg);
	CVI_SYS_FreeI(handle, &ref_min);
	CVI_SYS_FreeI(handle, &ref_cur);
	CVI_SYS_FreeI(handle, &stCurGrad);
	CVI_SYS_FreeI(handle, &stBgGrad);
	CVI_SYS_FreeI(handle, &stGradFg);
	CVI_SYS_FreeI(handle, &stCurGradSwap);
	CVI_SYS_FreeI(handle, &stBgGradSwap);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

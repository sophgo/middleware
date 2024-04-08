#include "cvi_ive.h"
// #include "cvi_pattern.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_dilate(int bTileMode, int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	int input_w, input_h;
	// char *input_data = NULL;
	// char *output_data1 = NULL;
	// char *output_data2 = NULL;
	char input_data[64];
	char output_data1[64];
	char output_data2[64];
	char output_file_3x3[64];
	char output_file_5x5[64];

	if (bTileMode) {
		// input_data = data_tile_640x480_y;
		// output_data1 = data_tile_Dilate_3x3;
		// output_data2 = data_tile_Dilate_5x5;
		strcpy(input_data, "res/ive/sky_640x480.yuv");
		strcpy(output_data1, "res/ive/result/sample_tile_Dilate_3x3.yuv");
		strcpy(output_data2, "res/ive/result/sample_tile_Dilate_5x5.yuv");
		input_w = 640;
		input_h = 480;
		strcpy(output_file_3x3, "sample_tile_Dilate_3x3.yuv");
		strcpy(output_file_5x5, "sample_tile_Dilate_5x5.yuv");
	} else {
		// input_data = data_bin_352x288_y;
		// output_data1 = data_Dilate3x3;
		// output_data2 = data_Dilate5x5;
		strcpy(input_data, "res/ive/bin_352x288_y.yuv");
		strcpy(output_data1, "res/ive/result/sample_Dilate_3x3_dilate_only.bin");
		strcpy(output_data2, "res/ive/result/sample_Dilate_5x5_dilate_only.bin");
		input_w = 352;
		input_h = 288;
		strcpy(output_file_3x3, "sample_Dilate_3x3_dilate_only.bin");
		strcpy(output_file_5x5, "sample_Dilate_5x5_dilate_only.bin");
	}
	CVI_U8 arr3by3[25] = { 0,   0, 0, 0, 0,	  0, 0, 255, 0, 0, 0, 255, 255,
			       255, 0, 0, 0, 255, 0, 0, 0,   0, 0, 0, 0 };

	CVI_U8 arr5by5[25] = { 0, 0,   255, 0,	 0,   0,   0, 255, 0,
			       0, 255, 255, 255, 255, 255, 0, 0,   255,
			       0, 0,   0,   0,	 255, 0,   0 };

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src;

	CVI_IVE_ReadRawImage(handle, &src, input_data, IVE_IMAGE_TYPE_U8C1,
			       input_w, input_h);

	// Create ref image.
	IVE_IMAGE_S ref_3x3, ref_5x5;

	CVI_IVE_ReadRawImage(handle, &ref_3x3, output_data1,
			       IVE_IMAGE_TYPE_U8C1, src.u32Width,
			       src.u32Height);
	CVI_IVE_ReadRawImage(handle, &ref_5x5, output_data2,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	// Create dst image.
	IVE_DST_IMAGE_S dst;

	CVI_IVE_CreateImage(handle, &dst, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);

	// Config Setting.
	IVE_DILATE_CTRL_S stCtrlDilate;

	memcpy(stCtrlDilate.au8Mask, arr3by3, sizeof(CVI_U8) * 25);

	// Run IVE
	printf("Run HW IVE Dilate 3x3.\n");
	CVI_IVE_Dilate(handle, &src, &dst, &stCtrlDilate, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst, &ref_3x3);
	if (bWrite)
		CVI_IVE_WriteImg(handle, output_file_3x3, &dst);

	memcpy(stCtrlDilate.au8Mask, arr5by5, sizeof(CVI_U8) * 25);

	printf("Run HW IVE Dilate 5x5.\n");
	CVI_IVE_Dilate(handle, &src, &dst, &stCtrlDilate, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst, &ref_5x5);
	if (bWrite)
		CVI_IVE_WriteImg(handle, output_file_5x5, &dst);

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeI(handle, &dst);
	CVI_SYS_FreeI(handle, &ref_3x3);
	CVI_SYS_FreeI(handle, &ref_5x5);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

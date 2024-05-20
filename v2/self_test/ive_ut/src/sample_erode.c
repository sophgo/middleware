#include "cvi_ive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_erode(int bTileMode, int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	int input_w, input_h;
	char input_data[64];
	char output_data_3x3[64];
	char output_data_5x5[64];
	char output_file_3x3[64];
	char output_file_5x5[64];

	if (bTileMode) {
		strcpy(input_data, "res/ive/sky_640x480.yuv");
		strcpy(output_data_3x3, "res/ive/result/sample_tile_Erode_3x3.yuv");
		strcpy(output_data_5x5, "res/ive/result/sample_tile_Erode_5x5.yuv");
		input_w = 640;
		input_h = 480;
		strcpy(output_file_3x3, "sample_tile_Erode_3x3.yuv");
		strcpy(output_file_5x5, "sample_tile_Erode_5x5.yuv");
	} else {
		strcpy(input_data, "res/ive/bin_352x288_y.yuv");
		strcpy(output_data_3x3, "res/ive/result/sample_Erode_3x3.bin.only_erode");
		strcpy(output_data_5x5, "res/ive/result/sample_Erode_5x5.bin.only_erode");
		input_w = 352;
		input_h = 288;
		strcpy(output_file_3x3, "sample_Erode_3x3.bin.only_erode");
		strcpy(output_file_5x5, "sample_Erode_5x5.bin.only_erode");
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

	CVI_IVE_ReadRawImage(handle, &ref_3x3, output_data_3x3,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_5x5, output_data_5x5,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	// Create dst image.
	IVE_DST_IMAGE_S dst;

	CVI_IVE_CreateImage(handle, &dst, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);

	// Config Setting.
	IVE_ERODE_CTRL_S stCtrlErode;

	memcpy(stCtrlErode.au8Mask, arr3by3, sizeof(CVI_U8) * 25);

	// Run IVE
	printf("Run HW IVE Erode 3x3.\n");
	CVI_IVE_Erode(handle, &src, &dst, &stCtrlErode, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst, &ref_3x3);
	if (bWrite)
		CVI_IVE_WriteImg(handle, output_file_3x3, &dst);

	memcpy(stCtrlErode.au8Mask, arr5by5, sizeof(CVI_U8) * 25);

	printf("Run HW IVE Erode 5x5.\n");
	CVI_IVE_Erode(handle, &src, &dst, &stCtrlErode, bInstant);
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

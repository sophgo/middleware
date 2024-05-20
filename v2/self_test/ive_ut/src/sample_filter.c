#include "cvi_ive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_filter(int bTileMode, int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	int input_w, input_h;
	char input_data1[64];
	char input_data2[64];
	char input_data3[64];
	char output_data1[64];
	char output_data2[64];
	char output_data3[64];
	char output_data4[64];
	char output_data5[64];
	char output_data6[64];
	char output_file1[64];
	char output_file2[64];
	char output_file3[64];
	char output_file4[64];
	char output_file5[64];
	char output_file6[64];

	//TODO:tilemode.
	if (bTileMode) {
		strcpy(input_data1, "res/ive/sky_640x480.yuv");
		strcpy(output_data1, "res/ive/result/sample_tile_Filter_Y3x3.yuv");
		strcpy(output_data2, "res/ive/result/sample_tile_Filter_Y5x5.yuv");
		input_w = 640;
		input_h = 480;
		strcpy(output_file1, "sample_tile_Filter_Y3x3.yuv");
		strcpy(output_file4, "sample_tile_Filter_Y5x5.yuv");
	} else {
		strcpy(input_data1, "res/ive/00_352x288_y.yuv");
		strcpy(input_data2, "res/ive/00_352x288_SP420.yuv");
		strcpy(input_data3, "res/ive/00_352x288_SP422.yuv");
		strcpy(output_data1, "res/ive/result/sample_Filter_Y3x3.yuv");
		strcpy(output_data2, "res/ive/result/sample_Filter_Y5x5.yuv");
		strcpy(output_data3, "res/ive/result/sample_Filter_420SP3x3.yuv");
		strcpy(output_data4, "res/ive/result/sample_Filter_420SP5x5.yuv");
		strcpy(output_data5, "res/ive/result/sample_Filter_422SP3x3.yuv");
		strcpy(output_data6, "res/ive/result/sample_Filter_422SP5x5.yuv");
		input_w = 352;
		input_h = 288;
		strcpy(output_file1, "sample_Filter_Y3x3.yuv");
		strcpy(output_file2, "sample_Filter_420SP3x3.yuv");
		strcpy(output_file3, "sample_Filter_422SP3x3.yuv");
		strcpy(output_file4, "sample_Filter_Y5x5.yuv");
		strcpy(output_file5, "sample_Filter_420SP5x5.yuv");
		strcpy(output_file6, "sample_Filter_422SP5x5.yuv");
	}

	CVI_S8 arr5by5[25] = { 1, 2, 3, 2, 1, 2, 5, 6, 5, 2, 3, 6, 8,
			       6, 3, 2, 5, 6, 5, 2, 1, 2, 3, 2, 1 };

	CVI_S8 arr3by3[25] = {
		0, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0, 2, 4,
		2, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0,
	};

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src, src_420, src_422;

	if (bTileMode) {
		CVI_IVE_ReadRawImage(handle, &src, input_data1,
			IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	} else {
		CVI_IVE_ReadRawImage(handle, &src, input_data1,
					IVE_IMAGE_TYPE_U8C1, input_w, input_h);
		CVI_IVE_ReadRawImage(handle, &src_420, input_data2,
					IVE_IMAGE_TYPE_YUV420SP, input_w, input_h);
		CVI_IVE_ReadRawImage(handle, &src_422, input_data3,
					IVE_IMAGE_TYPE_YUV422SP, input_w, input_h);
	}

	// Create ref image.
	IVE_IMAGE_S ref_y3x3, ref_y5x5, ref_420sp3x3, ref_420sp5x5,
		ref_422sp3x3, ref_422sp5x5;

	CVI_IVE_ReadRawImage(handle, &ref_y3x3, output_data1,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_y5x5, output_data2,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	if (!bTileMode) {
		CVI_IVE_ReadRawImage(handle, &ref_420sp3x3, output_data3,
					IVE_IMAGE_TYPE_YUV420SP, input_w, input_h);
		CVI_IVE_ReadRawImage(handle, &ref_420sp5x5, output_data4,
					IVE_IMAGE_TYPE_YUV420SP, input_w, input_h);
		CVI_IVE_ReadRawImage(handle, &ref_422sp3x3, output_data5,
					IVE_IMAGE_TYPE_YUV422SP, input_w, input_h);
		CVI_IVE_ReadRawImage(handle, &ref_422sp5x5, output_data6,
					IVE_IMAGE_TYPE_YUV422SP, input_w, input_h);
	}

	// Create dst image.
	IVE_DST_IMAGE_S dst_y, dst_420, dst_422;

	CVI_IVE_CreateImage(handle, &dst_y, IVE_IMAGE_TYPE_U8C1, src.u32Width,
			    src.u32Height);
	if (!bTileMode) {
		CVI_IVE_CreateImage(handle, &dst_420, IVE_IMAGE_TYPE_YUV420SP,
					src_420.u32Width, src_420.u32Height);
		CVI_IVE_CreateImage(handle, &dst_422, IVE_IMAGE_TYPE_YUV422SP,
					src_422.u32Width, src_422.u32Height);
	}

	// Config Setting.
	IVE_FILTER_CTRL_S iveFltCtrl;

	memcpy(iveFltCtrl.as8Mask, arr3by3, 5 * 5 * sizeof(CVI_S8));
	iveFltCtrl.u8Norm = 4;

	// Run HW IVE.
	printf("Run HW IVE Filter 3x3 U8C1.\n");
	CVI_IVE_Filter(handle, &src, &dst_y, &iveFltCtrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst_y, &ref_y3x3);
	if (bWrite)
		CVI_IVE_WriteImg(handle, output_file1, &dst_y);

	if (!bTileMode) {
		printf("Run HW IVE Filter 3x3 YUV420SP.\n");
		CVI_IVE_Filter(handle, &src_420, &dst_420, &iveFltCtrl, bInstant);
		ret |= CVI_IVE_CompareIveImage(&dst_420, &ref_420sp3x3);
		if (bWrite)
			CVI_IVE_WriteImg(handle, output_file2,
					&dst_420);

		printf("Run HW IVE Filter 3x3 YUV422SP.\n");
		CVI_IVE_Filter(handle, &src_422, &dst_422, &iveFltCtrl, bInstant);
		ret |= CVI_IVE_CompareIveImage(&dst_422, &ref_422sp3x3);
		if (bWrite)
			CVI_IVE_WriteImg(handle, output_file3, &dst_422);
	}
	memcpy(iveFltCtrl.as8Mask, arr5by5, 5 * 5 * sizeof(CVI_S8));
	iveFltCtrl.u8Norm = 7;

	printf("Run HW IVE Filter 5x5 U8C1.\n");
	CVI_IVE_Filter(handle, &src, &dst_y, &iveFltCtrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst_y, &ref_y5x5);
	if (bWrite)
		CVI_IVE_WriteImg(handle, output_file4, &dst_y);
	if (!bTileMode) {
		printf("Run HW IVE Filter 5x5 YUV420SP.\n");
		CVI_IVE_Filter(handle, &src_420, &dst_420, &iveFltCtrl, bInstant);
		ret |= CVI_IVE_CompareIveImage(&dst_420, &ref_420sp5x5);
		if (bWrite)
			CVI_IVE_WriteImg(handle, output_file5, &dst_420);

		printf("Run HW IVE Filter 5x5 YUV422SP.\n");
		CVI_IVE_Filter(handle, &src_422, &dst_422, &iveFltCtrl, bInstant);
		ret |= CVI_IVE_CompareIveImage(&dst_422, &ref_422sp5x5);
		if (bWrite)
			CVI_IVE_WriteImg(handle, output_file6, &dst_422);
	}
	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeI(handle, &dst_y);
	CVI_SYS_FreeI(handle, &ref_y3x3);
	CVI_SYS_FreeI(handle, &ref_y5x5);
	if (!bTileMode) {
		CVI_SYS_FreeI(handle, &src_420);
		CVI_SYS_FreeI(handle, &src_422);
		CVI_SYS_FreeI(handle, &dst_420);
		CVI_SYS_FreeI(handle, &dst_422);
		CVI_SYS_FreeI(handle, &ref_420sp3x3);
		CVI_SYS_FreeI(handle, &ref_420sp5x5);
		CVI_SYS_FreeI(handle, &ref_422sp3x3);
		CVI_SYS_FreeI(handle, &ref_422sp5x5);
	}
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

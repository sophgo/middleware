#include "cvi_ive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_filterandcsc(int bTileMode, int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	int input_w, input_h;
	char input_data[64];
	char output_data1[64];
	char output_data2[64];
	char output_file1[64];
	char output_file2[64];

	if (bTileMode) {
		input_w = 352 * 2;
		input_h = 288;
		strcpy(output_file1, "sample_tile_FilterAndCSC_420SPToVideoPlanar3x3.yuv");
		strcpy(output_file2, "sample_tile_FilterAndCSC_420SPToVideoPlanar5x5.yuv");
	} else {
		strcpy(input_data, "res/ive/00_352x288_SP420.yuv");
		strcpy(output_data1, "res/ive/result/sample_FilterAndCSC_420SPToVideoPlanar3x3.yuv");
		strcpy(output_data2, "res/ive/result/sample_FilterAndCSC_420SPToVideoPlanar5x5.yuv");
		input_w = 352;
		input_h = 288;
		strcpy(output_file1, "sample_FilterAndCSC_420SPToVideoPlanar3x3.yuv");
		strcpy(output_file2, "sample_FilterAndCSC_420SPToVideoPlanar5x5.yuv");
	}
	CVI_S8 arr5by5[25] = { 1, 2, 3, 2, 1, 2, 5, 6, 5, 2, 3, 6, 8,
			       6, 3, 2, 5, 6, 5, 2, 1, 2, 3, 2, 1 };

	CVI_S8 arr3by3[25] = {
		0, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0, 2, 4,
		2, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0,
	};

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src_420;

	if (bTileMode) {
		CVI_IVE_CreateImage(handle, &src_420, IVE_IMAGE_TYPE_YUV420SP,
			input_w, input_h);
		for (int i = 0; i < 432; i++) {
			memcpy(&((char *)(uintptr_t)src_420
						.u64VirAddr[0])[i * input_w],
					&input_data[i * 352], 352);
			memcpy(&((char *)(uintptr_t)src_420
						.u64VirAddr[0])[i * input_w +
								352],
					&input_data[i * 352], 352);
		}
	} else {
		CVI_IVE_ReadRawImage(handle, &src_420, input_data,
				IVE_IMAGE_TYPE_YUV420SP, input_w, input_h);
	}

	// Create ref image.
	IVE_IMAGE_S ref_y3x3, ref_y5x5;

	CVI_IVE_ReadRawImage(handle, &ref_y3x3, output_data1,
			       IVE_IMAGE_TYPE_U8C3_PLANAR, input_w,
			       input_h);
	CVI_IVE_ReadRawImage(handle, &ref_y5x5, output_data2,
			       IVE_IMAGE_TYPE_U8C3_PLANAR, input_w,
			       input_h);

	// Create dst image.
	IVE_DST_IMAGE_S dst;

	CVI_IVE_CreateImage(handle, &dst, IVE_IMAGE_TYPE_U8C3_PLANAR,
			    input_w, input_h);

	// Config Setting.
	IVE_FILTER_AND_CSC_CTRL_S stCtrl;

	// Run HW IVE.
	printf("Run HW IVE FilterAndCSC YUV2RGB.\n");
	stCtrl.enMode = IVE_CSC_MODE_VIDEO_BT601_YUV2RGB;
	stCtrl.u8Norm = 4;
	memcpy(stCtrl.as8Mask, arr3by3, sizeof(CVI_S8) * 25);
	CVI_IVE_FilterAndCSC(handle, &src_420, &dst, &stCtrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst, &ref_y3x3);
	if (bWrite) {
		CVI_IVE_WriteImg(
			handle, output_file1, &dst);
	}

	printf("Run HW IVE FilterAndCSC YUV2RGB.\n");
	stCtrl.enMode = IVE_CSC_MODE_VIDEO_BT601_YUV2RGB;
	stCtrl.u8Norm = 7;
	memcpy(stCtrl.as8Mask, arr5by5, sizeof(CVI_S8) * 25);
	CVI_IVE_FilterAndCSC(handle, &src_420, &dst, &stCtrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst, &ref_y5x5);
	if (bWrite) {
		CVI_IVE_WriteImg(
			handle, output_file2, &dst);
	}

	CVI_SYS_FreeI(handle, &src_420);
	CVI_SYS_FreeI(handle, &dst);
	CVI_SYS_FreeI(handle, &ref_y3x3);
	CVI_SYS_FreeI(handle, &ref_y5x5);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

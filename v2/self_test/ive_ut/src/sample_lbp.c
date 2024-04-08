#include "cvi_ive.h"
// #include "cvi_pattern.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_lbp(int bTileMode, int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	int input_w, input_h;
	// char *input_data = NULL;
	// char *output_data1 = NULL;
	// char *output_data2 = NULL;
	char input_data[64];
	char output_data1[64];
	char output_data2[64];
	char output_file1[64];
	char output_file2[64];
	//TODO:tilemode.
	if (bTileMode) {
		// input_data = data_00_352x288_y;
		// output_data1 = result_LBP_Normal;
		// output_data2 = result_LBP_Abs;
		input_w = 352 * 2;
		input_h = 288;
		strcpy(output_file1, "sample_tile_LBP_Normal.yuv");
		strcpy(output_file2, "sample_tile_LBP_Abs.yuv");
	} else {
		// input_data = data_00_352x288_y;
		// output_data1 = result_LBP_Normal;
		// output_data2 = result_LBP_Abs;
		strcpy(input_data, "res/ive/00_352x288_y.yuv");
		strcpy(output_data1, "res/ive/result/sample_LBP_Normal.yuv");
		strcpy(output_data2, "res/ive/result/sample_LBP_Abs.yuv");
		input_w = 352;
		input_h = 288;
		strcpy(output_file1, "sample_LBP_Normal.yuv");
		strcpy(output_file2, "sample_LBP_Abs.yuv");
	}

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src;

	// Create ref image.
	IVE_IMAGE_S ref_nor, ref_abs;

	if (bTileMode) {
		CVI_IVE_CreateImage(handle, &src, IVE_IMAGE_TYPE_U8C1,
			input_w, input_h);
		for (int i = 0; i < 288; i++) {
			memcpy(&((char *)(uintptr_t)src
					.u64VirAddr[0])[i * input_w],
					&input_data[i * 352],
					352);
			memcpy(&((char *)(uintptr_t)src
					.u64VirAddr[0])[i * input_w + 352],
					&input_data[i * 352],
					352);
		}
	} else {
		CVI_IVE_ReadRawImage(handle, &src, input_data,
				IVE_IMAGE_TYPE_U8C1, input_w, input_h);
		CVI_IVE_ReadRawImage(handle, &ref_nor, output_data1,
				IVE_IMAGE_TYPE_U8C1, input_w, input_h);
		CVI_IVE_ReadRawImage(handle, &ref_abs, output_data2,
				IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	}

	// Create dst image.
	IVE_DST_IMAGE_S dst;

	CVI_IVE_CreateImage(handle, &dst, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);

	// Config Setting.
	IVE_LBP_CTRL_S ctrl;

	ctrl.enMode = IVE_LBP_CMP_MODE_NORMAL;
	ctrl.un8BitThr.s8Val = (ctrl.enMode == IVE_LBP_CMP_MODE_ABS ? 35 : 41);

	// Run HW IVE
	printf("Run HW IVE LBP Normal.\n");
	CVI_IVE_LBP(handle, &src, &dst, &ctrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst, &ref_nor);
	if (bWrite)
		CVI_IVE_WriteImg(handle, output_file1, &dst);

	ctrl.enMode = IVE_LBP_CMP_MODE_ABS;
	ctrl.un8BitThr.s8Val = (ctrl.enMode == IVE_LBP_CMP_MODE_ABS ? 35 : 41);
	printf("Run HW IVE LBP ABS.\n");
	CVI_IVE_LBP(handle, &src, &dst, &ctrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst, &ref_abs);
	if (bWrite)
		CVI_IVE_WriteImg(handle, output_file2, &dst);

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeI(handle, &dst);
	CVI_SYS_FreeI(handle, &ref_nor);
	CVI_SYS_FreeI(handle, &ref_abs);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

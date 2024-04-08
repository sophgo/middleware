#include "cvi_ive.h"
// #include "cvi_pattern.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_thresh_u16(int bTileMode, int bWrite, int bInstant)
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

	//TODO:tilemode
	if (bTileMode) {
		// input_data = data_00_704x576_u16;
		// output_data1 = data_ThreshU16_MinMidMax;
		// output_data2 = data_ThreshU16_MinOriMax;
		strcpy(input_data, "res/ive/00_704x576.u16");
		strcpy(output_data1, "res/ive/result/sample_tile_Thresh_U16_To_U8_MinMidMax.yuv");
		strcpy(output_data2, "res/ive/result/sample_tile_Thresh_U16_To_U8_MinOriMax.yuv");
		input_w = 352 * 2;
		input_h = 288;
		strcpy(output_file1, "sample_tile_Thresh_U16_To_U8_MinMidMax.yuv");
		strcpy(output_file2, "sample_tile_Thresh_U16_To_U8_MinOriMax.yuv");
	} else {
		// input_data = data_00_704x576_u16;
		// output_data1 = data_ThreshU16_MinMidMax;
		// output_data2 = data_ThreshU16_MinOriMax;
		strcpy(input_data, "res/ive/00_704x576.u16");
		strcpy(output_data1, "res/ive/result/sample_Thresh_U16_To_U8_MinMidMax.yuv");
		strcpy(output_data2, "res/ive/result/sample_Thresh_U16_To_U8_MinOriMax.yuv");
		input_w = 352;
		input_h = 288;
		strcpy(output_file1, "sample_Thresh_U16_To_U8_MinMidMax.yuv");
		strcpy(output_file2, "sample_Thresh_U16_To_U8_MinOriMax.yuv");
	}
	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src;

	if (bTileMode) {
		CVI_IVE_CreateImage(handle, &src, IVE_IMAGE_TYPE_U16C1,
			input_w, input_h);
		for (int i = 0; i < 288; i++) {
			memcpy(&((char *)(uintptr_t)src
						.u64VirAddr[0])[i * input_w * 2],
					&input_data[i * 352 * 2],
					352 * 2);
			memcpy(&((char *)(uintptr_t)src
						.u64VirAddr[0])[i * input_w * 2 +
								352 * 2],
					&input_data[i * 352 * 2],
					352 * 2);
		}
	} else {
		CVI_IVE_ReadRawImage(handle, &src, input_data,
			       IVE_IMAGE_TYPE_U16C1, input_w, input_h);
	}

	// Create ref image.
	IVE_IMAGE_S ref_minmidmax, ref_minorimax;

	CVI_IVE_ReadRawImage(handle, &ref_minmidmax, output_data1,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_minorimax, output_data2,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	// Create dst image.
	IVE_DST_IMAGE_S dst;

	CVI_IVE_CreateImage(handle, &dst, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);

	// Config Setting.
	IVE_THRESH_U16_CTRL_S iveThreshCtrl;

	iveThreshCtrl.enMode = IVE_THRESH_U16_MODE_U16_TO_U8_MIN_MID_MAX;
	iveThreshCtrl.u16LowThr = 41;
	iveThreshCtrl.u16HighThr = 105;
	iveThreshCtrl.u8MinVal = 190;
	iveThreshCtrl.u8MidVal = 132;
	iveThreshCtrl.u8MaxVal = 225;

	// Run HW IVE.
	printf("Run HW IVE Threashold U16 U8_MinMidMax.\n");
	CVI_IVE_Thresh_U16(handle, &src, &dst, &iveThreshCtrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst, &ref_minmidmax);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file1, &dst);
	}

	// Config Setting.
	iveThreshCtrl.enMode = IVE_THRESH_U16_MODE_U16_TO_U8_MIN_ORI_MAX;
	iveThreshCtrl.u16LowThr = 108;
	iveThreshCtrl.u16HighThr = 121;
	iveThreshCtrl.u8MinVal = 174;
	iveThreshCtrl.u8MidVal = 82;
	iveThreshCtrl.u8MaxVal = 144;

	// Run HW IVE.
	printf("Run HW IVE Threashold U16 U8_MinOriMax.\n");
	CVI_IVE_Thresh_U16(handle, &src, &dst, &iveThreshCtrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst, &ref_minorimax);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file2, &dst);
	}

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeI(handle, &dst);
	CVI_SYS_FreeI(handle, &ref_minmidmax);
	CVI_SYS_FreeI(handle, &ref_minorimax);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

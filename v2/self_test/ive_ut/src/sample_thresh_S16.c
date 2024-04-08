#include "cvi_ive.h"
// #include "cvi_pattern.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_thresh_s16(int bTileMode, int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	int input_w, input_h;
	// char *input_data = NULL;
	// char *output_data1 = NULL;
	// char *output_data2 = NULL;
	// char *output_data3 = NULL;
	// char *output_data4 = NULL;
	char input_data[64];
	char output_data1[64];
	char output_data2[64];
	char output_data3[64];
	char output_data4[64];
	char output_file1[64];
	char output_file2[64];
	char output_file3[64];
	char output_file4[64];

	//TODO:tilemode
	IVE_HANDLE handle = CVI_IVE_CreateHandle();
	if (bTileMode) {
		// input_data = data_00_704x576_s16;
		// output_data1 = data_ThreshS16_S16ToS8_MinMidMax;
		// output_data2 = data_ThreshS16_S16ToS8_MinOriMax;
		// output_data3 = data_ThreshS16_S16ToU8_MinMidMax;
		// output_data4 = data_ThreshS16_S16ToU8_MinOriMax;
		strcpy(input_data, "res/ive/00_704x576.s16");
		strcpy(output_data1, "res/ive/result/sample_tile_Thresh_S16_To_S8_MinMidMax.yuv");
		strcpy(output_data2, "res/ive/result/sample_tile_Thresh_S16_To_S8_MinOriMax.yuv");
		strcpy(output_data3, "res/ive/result/sample_tile_Thresh_S16_To_U8_MinMidMax.yuv");
		strcpy(output_data4, "res/ive/result/sample_tile_Thresh_S16_To_U8_MinOriMax.yuv");
		input_w = 352 * 2;
		input_h = 288;
		strcpy(output_file1, "sample_tile_Thresh_S16_To_S8_MinMidMax.yuv");
		strcpy(output_file2, "sample_tile_Thresh_S16_To_S8_MinOriMax.yuv");
		strcpy(output_file3, "sample_tile_Thresh_S16_To_U8_MinMidMax.yuv");
		strcpy(output_file4, "sample_tile_Thresh_S16_To_U8_MinOriMax.yuv");
	} else {
		// input_data = data_00_704x576_s16;
		// output_data1 = data_ThreshS16_S16ToS8_MinMidMax;
		// output_data2 = data_ThreshS16_S16ToS8_MinOriMax;
		// output_data3 = data_ThreshS16_S16ToU8_MinMidMax;
		// output_data4 = data_ThreshS16_S16ToU8_MinOriMax;
		strcpy(input_data, "res/ive/00_704x576.s16");
		strcpy(output_data1, "res/ive/result/sample_Thresh_S16_To_S8_MinMidMax.yuv");
		strcpy(output_data2, "res/ive/result/sample_Thresh_S16_To_S8_MinOriMax.yuv");
		strcpy(output_data3, "res/ive/result/sample_Thresh_S16_To_U8_MinMidMax.yuv");
		strcpy(output_data4, "res/ive/result/sample_Thresh_S16_To_U8_MinOriMax.yuv");
		input_w = 352;
		input_h = 288;
		strcpy(output_file1, "sample_Thresh_S16_To_S8_MinMidMax.yuv");
		strcpy(output_file2, "sample_Thresh_S16_To_S8_MinOriMax.yuv");
		strcpy(output_file3, "sample_Thresh_S16_To_U8_MinMidMax.yuv");
		strcpy(output_file4, "sample_Thresh_S16_To_U8_MinOriMax.yuv");
	}
	// Create src image.
	IVE_IMAGE_S src;

	if (bTileMode) {
		CVI_IVE_CreateImage(handle, &src, IVE_IMAGE_TYPE_S16C1,
			input_w, input_h);
		for (int i = 0; i < 288; i++) {
			memcpy(&((char *)(uintptr_t)src
						.u64VirAddr[0])[i * input_w * 2],
					&input_data[i * 352 * 2], 352 * 2);
			memcpy(&((char *)(uintptr_t)src
						.u64VirAddr[0])[i * input_w * 2 +
								352 * 2],
					&input_data[i * 352 * 2], 352 * 2);
		}
	} else {
		CVI_IVE_ReadRawImage(handle, &src, input_data,
			       IVE_IMAGE_TYPE_S16C1, input_w, input_h);
	}

	// Create ref image.
	IVE_IMAGE_S ref_s16tos8_minmidmax, ref_s16tos8_minorimax,
		ref_s16tou8_minmidmax, ref_s16tou8_minorimax;

	CVI_IVE_ReadRawImage(handle, &ref_s16tos8_minmidmax,
			       output_data1,
			       IVE_IMAGE_TYPE_S8C1, src.u32Width,
			       src.u32Height);
	CVI_IVE_ReadRawImage(handle, &ref_s16tos8_minorimax,
			       output_data2,
			       IVE_IMAGE_TYPE_S8C1, src.u32Width,
			       src.u32Height);
	CVI_IVE_ReadRawImage(handle, &ref_s16tou8_minmidmax,
			       output_data3,
			       IVE_IMAGE_TYPE_U8C1, src.u32Width,
			       src.u32Height);
	CVI_IVE_ReadRawImage(handle, &ref_s16tou8_minorimax,
			       output_data4,
			       IVE_IMAGE_TYPE_U8C1, src.u32Width,
			       src.u32Height);

	// Create dst image.
	IVE_DST_IMAGE_S dst, dst_s8;

	CVI_IVE_CreateImage(handle, &dst, IVE_IMAGE_TYPE_U8C1, src.u32Width,
			    src.u32Height);
	CVI_IVE_CreateImage(handle, &dst_s8, IVE_IMAGE_TYPE_S8C1, src.u32Width,
			    src.u32Height);

	// Config Setting.
	IVE_THRESH_S16_CTRL_S iveThreshCtrl;

	iveThreshCtrl.enMode = IVE_THRESH_S16_MODE_S16_TO_S8_MIN_MID_MAX;
	iveThreshCtrl.s16LowThr = 41;
	iveThreshCtrl.s16HighThr = 105;
	iveThreshCtrl.un8MinVal.s8Val = -63;
	iveThreshCtrl.un8MidVal.s8Val = -5;
	iveThreshCtrl.un8MaxVal.s8Val = -98;

	// Run HW IVE.
	printf("Run HW IVE Threashold S16 S8_MinMidMax.\n");
	CVI_IVE_Thresh_S16(handle, &src, &dst_s8, &iveThreshCtrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst_s8, &ref_s16tos8_minmidmax);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file1, &dst_s8);
	}

	// Config Setting.
	iveThreshCtrl.enMode = IVE_THRESH_S16_MODE_S16_TO_S8_MIN_ORI_MAX;
	iveThreshCtrl.s16LowThr = 108;
	iveThreshCtrl.s16HighThr = 111;
	iveThreshCtrl.un8MinVal.s8Val = -47;
	iveThreshCtrl.un8MidVal.s8Val = 82;
	iveThreshCtrl.un8MaxVal.s8Val = -17;

	// Run HW IVE.
	printf("Run HW IVE Threashold S16 S8_MinOriMax.\n");
	CVI_IVE_Thresh_S16(handle, &src, &dst_s8, &iveThreshCtrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst_s8, &ref_s16tos8_minorimax);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file2, &dst_s8);
	}

	// Config Setting.
	iveThreshCtrl.enMode = IVE_THRESH_S16_MODE_S16_TO_U8_MIN_MID_MAX;
	iveThreshCtrl.s16LowThr = 73;
	iveThreshCtrl.s16HighThr = 191;
	iveThreshCtrl.un8MinVal.u8Val = 241;
	iveThreshCtrl.un8MidVal.u8Val = 187;
	iveThreshCtrl.un8MaxVal.u8Val = 233;

	// Run HW IVE.
	printf("Run HW IVE Threashold S16 U8_MinMidMax.\n");
	CVI_IVE_Thresh_S16(handle, &src, &dst, &iveThreshCtrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst, &ref_s16tou8_minmidmax);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file3, &dst);
	}

	// Config Setting.
	iveThreshCtrl.enMode = IVE_THRESH_S16_MODE_S16_TO_U8_MIN_ORI_MAX;
	iveThreshCtrl.s16LowThr = 235;
	iveThreshCtrl.s16HighThr = 251;
	iveThreshCtrl.un8MinVal.u8Val = 166;
	iveThreshCtrl.un8MidVal.u8Val = 219;
	iveThreshCtrl.un8MaxVal.u8Val = 60;

	// Run HW IVE.
	printf("Run HW IVE Threashold S16 U8_MinOriMax.\n");
	CVI_IVE_Thresh_S16(handle, &src, &dst, &iveThreshCtrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst, &ref_s16tou8_minorimax);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file4, &dst);
	}

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeI(handle, &dst);
	CVI_SYS_FreeI(handle, &dst_s8);
	CVI_SYS_FreeI(handle, &ref_s16tos8_minmidmax);
	CVI_SYS_FreeI(handle, &ref_s16tos8_minorimax);
	CVI_SYS_FreeI(handle, &ref_s16tou8_minmidmax);
	CVI_SYS_FreeI(handle, &ref_s16tou8_minorimax);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

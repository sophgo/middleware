#include "cvi_ive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_16botto8bit(int bTileMode, int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	int input_w, input_h;
	char input_data1[64];
	char input_data2[64];
	char output_data1[64];
	char output_data2[64];
	char output_data3[64];
	char output_data4[64];
	char output_file1[64];
	char output_file2[64];
	char output_file3[64];
	char output_file4[64];

	if (bTileMode) {
		strcpy(input_data1, "res/ive/00_704x576.u16");
		strcpy(input_data2, "res/ive/00_704x576.s16");
		strcpy(output_data1, "res/ive/result/sample_16BitTo8Bit_U16ToU8.yuv");
		strcpy(output_data2, "res/ive/result/sample_16BitTo8Bit_Abs.yuv");
		strcpy(output_data3, "res/ive/result/sample_16BitTo8Bit_S16ToS8.yuv");
		strcpy(output_data4, "res/ive/result/sample_16BitTo8Bit_Shift.yuv");
		input_w = 352 * 2;
		input_h = 288;
		strcpy(output_file1, "sample_tile_16BitTo8Bit_S16ToS8.yuv");
		strcpy(output_file2, "sample_tile_16BitTo8Bit_Abs.yuv");
		strcpy(output_file3, "sample_tile_16BitTo8Bit_Shift.yuv");
		strcpy(output_file4, "sample_tile_16BitTo8Bit_U16ToU8.yuv");
	} else {
		strcpy(input_data1, "res/ive/00_704x576.u16");
		strcpy(input_data2, "res/ive/00_704x576.s16");
		strcpy(output_data1, "res/ive/result/sample_16BitTo8Bit_U16ToU8.yuv");
		strcpy(output_data2, "res/ive/result/sample_16BitTo8Bit_Abs.yuv");
		strcpy(output_data3, "res/ive/result/sample_16BitTo8Bit_S16ToS8.yuv");
		strcpy(output_data4, "res/ive/result/sample_16BitTo8Bit_Shift.yuv");
		input_w = 352;
		input_h = 288;
		strcpy(output_file1, "sample_16BitTo8Bit_S16ToS8.yuv");
		strcpy(output_file2, "sample_16BitTo8Bit_Abs.yuv");
		strcpy(output_file3, "sample_16BitTo8Bit_Shift.yuv");
		strcpy(output_file4, "sample_16BitTo8Bit_U16ToU8.yuv");
	}
	IVE_HANDLE handle = CVI_IVE_CreateHandle();
	// Create src image.
	IVE_IMAGE_S src_u16, src_s16;

	if (bTileMode) {
		CVI_IVE_CreateImage(handle, &src_u16, IVE_IMAGE_TYPE_U16C1,
			input_w, input_h);
		CVI_IVE_CreateImage(handle, &src_s16, IVE_IMAGE_TYPE_S16C1,
			input_w, input_h);
		for (int i = 0; i < 288; i++) {
			memcpy(&((char *)(uintptr_t)src_u16
						.u64VirAddr[0])[i * input_w * 2],
					&input_data1[i * 352 * 2],
					352 * 2);
			memcpy(&((char *)(uintptr_t)src_u16
						.u64VirAddr[0])[i * input_w * 2 +
								352 * 2],
					&input_data1[i * 352 * 2],
					352 * 2);
			memcpy(&((char *)(uintptr_t)src_s16
						.u64VirAddr[0])[i * input_w * 2],
					&input_data2[i * 352 * 2],
					352 * 2);
			memcpy(&((char *)(uintptr_t)src_s16
						.u64VirAddr[0])[i * input_w * 2 +
								352 * 2],
					&input_data2[i * 352 * 2],
					352 * 2);
		}
	} else {
		CVI_IVE_ReadRawImage(handle, &src_u16, input_data1,
			       IVE_IMAGE_TYPE_U16C1, input_w, input_h);
		CVI_IVE_ReadRawImage(handle, &src_s16, input_data2,
			       IVE_IMAGE_TYPE_S16C1, input_w, input_h);
	}

	// Create ref image.
	IVE_IMAGE_S ref_u16tou8, ref_abs, ref_s16tos8, ref_shift;

	CVI_IVE_ReadRawImage(handle, &ref_u16tou8, output_data1,
			       IVE_IMAGE_TYPE_U8C1, input_w,
			       input_h);
	CVI_IVE_ReadRawImage(handle, &ref_abs, output_data2,
			       IVE_IMAGE_TYPE_U8C1, input_w,
			       input_h);
	CVI_IVE_ReadRawImage(handle, &ref_s16tos8, output_data3,
			       IVE_IMAGE_TYPE_S8C1, input_w,
			       input_h);
	CVI_IVE_ReadRawImage(handle, &ref_shift, output_data4,
			       IVE_IMAGE_TYPE_U8C1, input_w,
			       input_h);
	// Create dst image.
	IVE_DST_IMAGE_S dst_u8, dst_s8;

	CVI_IVE_CreateImage(handle, &dst_u8, IVE_IMAGE_TYPE_U8C1,
			    input_w, input_h);
	CVI_IVE_CreateImage(handle, &dst_s8, IVE_IMAGE_TYPE_S8C1,
			    input_w, input_h);

	// Config Setting.
	IVE_16BIT_TO_8BIT_CTRL_S ctrl;

	// Run HW IVE.
	printf("Run HW IVE S16 to S8.\n");
	ctrl.enMode = IVE_16BIT_TO_8BIT_MODE_S16_TO_S8;
	ctrl.u8Numerator = 41; //255
	ctrl.u16Denominator = 18508; //256
	ctrl.s8Bias = 0;
	CVI_IVE_16BitTo8Bit(handle, &src_s16, &dst_s8, &ctrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst_s8, &ref_s16tos8);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file1, &dst_s8);
	}

	printf("Run HW IVE S16 to U8 ABS.\n");
	ctrl.enMode = IVE_16BIT_TO_8BIT_MODE_S16_TO_U8_ABS;
	ctrl.u8Numerator = 190; //255
	ctrl.u16Denominator = 26690; //256
	ctrl.s8Bias = 0;
	CVI_IVE_16BitTo8Bit(handle, &src_s16, &dst_u8, &ctrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst_u8, &ref_abs);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file2, &dst_u8);
	}

	printf("Run HW IVE S16 to U8 BIAS.\n");
	ctrl.enMode = IVE_16BIT_TO_8BIT_MODE_S16_TO_U8_BIAS;
	ctrl.u8Numerator = 225; //255
	ctrl.u16Denominator = 15949; //256
	ctrl.s8Bias = -42;
	CVI_IVE_16BitTo8Bit(handle, &src_s16, &dst_u8, &ctrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst_u8, &ref_shift);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file3, &dst_u8);
	}

	printf("Run HW IVE U16 to U8.\n");
	ctrl.enMode = IVE_16BIT_TO_8BIT_MODE_U16_TO_U8;
	ctrl.u8Numerator = 174; //255
	ctrl.u16Denominator = 27136; //256
	ctrl.s8Bias = 0;
	CVI_IVE_16BitTo8Bit(handle, &src_u16, &dst_u8, &ctrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst_u8, &ref_u16tou8);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file4, &dst_u8);
	}

	CVI_SYS_FreeI(handle, &src_u16);
	CVI_SYS_FreeI(handle, &src_s16);
	CVI_SYS_FreeI(handle, &dst_u8);
	CVI_SYS_FreeI(handle, &dst_s8);
	CVI_SYS_FreeI(handle, &ref_u16tou8);
	CVI_SYS_FreeI(handle, &ref_abs);
	CVI_SYS_FreeI(handle, &ref_s16tos8);
	CVI_SYS_FreeI(handle, &ref_shift);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

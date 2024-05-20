#include "cvi_ive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_sub(int bTileMode, int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	int input_w, input_h;
	char input_data[64];
	char output_data[64];
	char output_file[64];

	if (bTileMode) {
		strcpy(input_data, "res/ive/sky_640x480.yuv");
		strcpy(output_data, "res/ive/result/sample_tile_Sub_Abs.yuv");
		input_w = 640;
		input_h = 480;
		strcpy(output_file, "sample_tile_Sub_Abs.yuv");
	} else {
		strcpy(input_data, "res/ive/00_352x288_y.yuv");
		strcpy(output_data, "res/ive/result/sample_Sub_Abs.yuv");
		input_w = 352;
		input_h = 288;
		strcpy(output_file, "sample_Sub_Abs.yuv");
	}

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src1, src2;

	CVI_IVE_ReadRawImage(handle, &src1, (char *)input_data,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_CreateImage(handle, &src2, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);
	memset((void *)(uintptr_t)src2.u64VirAddr[0], 255,
	       src1.u32Stride[0] * input_h);
	for (int j = input_h / 10; j < input_h * 9 / 10; j++) {
		for (int i = input_w / 10; i < input_w * 9 / 10; i++) {
			((char *)(uintptr_t)src2
				 .u64VirAddr[0])[i + j * src1.u32Stride[0]] = 0;
		}
	}

	// Create ref image.
	IVE_IMAGE_S ref_sub;

	CVI_IVE_ReadRawImage(handle, &ref_sub, (char *)output_data,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	// Create dst image.
	IVE_DST_IMAGE_S dst;

	CVI_IVE_CreateImage(handle, &dst, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);

	// Config Setting.
	IVE_SUB_CTRL_S iveSubCtrl;

	iveSubCtrl.enMode = IVE_SUB_MODE_ABS;

	// Run IVE
	printf("Run HW IVE SUB.\n");
	CVI_IVE_Sub(handle, &src1, &src2, &dst, &iveSubCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst, &ref_sub);
	if (bWrite)
		CVI_IVE_WriteImg(handle, output_file, &dst);

	CVI_SYS_FreeI(handle, &src1);
	CVI_SYS_FreeI(handle, &src2);
	CVI_SYS_FreeI(handle, &dst);
	CVI_SYS_FreeI(handle, &ref_sub);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

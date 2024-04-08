#include "cvi_ive.h"
// #include "cvi_pattern.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_xor(int bTileMode, int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	int input_w, input_h;
	// char *input_data = NULL;
	// char *output_data = NULL;
	char input_data[64];
	char output_data[64];
	char output_file[64];

	if (bTileMode) {
		// input_data = data_tile_640x480_y;
		// output_data = data_tile_Xor;
		strcpy(input_data, "res/ive/sky_640x480.yuv");
		strcpy(output_data, "res/ive/result/sample_tile_Xor.yuv");
		input_w = 640;
		input_h = 480;
		strcpy(output_file, "sample_tile_Xor.yuv");
	} else {
		// input_data = data_00_352x288_y;
		// output_data = data_Xor;
		strcpy(input_data, "res/ive/00_352x288_y.yuv");
		strcpy(output_data, "res/ive/result/sample_Xor.yuv");
		input_w = 352;
		input_h = 288;
		strcpy(output_file, "sample_Xor.yuv");
	}

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src1, src2;

	CVI_IVE_ReadRawImage(handle, &src1, input_data, IVE_IMAGE_TYPE_U8C1,
			       input_w, input_h);
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
	IVE_IMAGE_S ref_xor;

	CVI_IVE_ReadRawImage(handle, &ref_xor, (char *)output_data,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	// Create dst image.
	IVE_DST_IMAGE_S dst;

	CVI_IVE_CreateImage(handle, &dst, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);

	// Config Setting.

	// Run IVE
	printf("Run HW IVE Xor.\n");
	CVI_IVE_Xor(handle, &src1, &src2, &dst, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst, &ref_xor);
	if (bWrite)
		CVI_IVE_WriteImg(handle, output_file, &dst);

	CVI_SYS_FreeI(handle, &src1);
	CVI_SYS_FreeI(handle, &src2);
	CVI_SYS_FreeI(handle, &dst);
	CVI_SYS_FreeI(handle, &ref_xor);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

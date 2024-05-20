#include "cvi_ive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_imgtoodma(int bTileMode, int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	int input_w, input_h;
	char input_data[64];
	char output_data[64];
	char output_file[64];

	if (bTileMode) {
		input_w = 640;
		input_h = 480;
		strcpy(input_data, "res/ive/sky_640x480.yuv");
		strcpy(output_data, "res/ive/sky_640x480.yuv");
		strcpy(output_file, "sample_tile_ImgToOdma_sky_640x480_y.yuv");
	} else {
		input_w = 352;
		input_h = 288;
		strcpy(input_data, "res/ive/00_352x288_y.yuv");
		strcpy(output_data, "res/ive/00_352x288_y.yuv");
		strcpy(output_file, "sample_ImgToOdma_00_352x288_y.yuv");
	}
	/* mask */
	CVI_S8 mask[25] = { 0, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0, 2, 4,
			    2, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0 };

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src;

	CVI_IVE_ReadRawImage(handle, &src, input_data, IVE_IMAGE_TYPE_U8C1,
			       input_w, input_h);

	// Create ref image.
	IVE_IMAGE_S ref;

	CVI_IVE_ReadRawImage(handle, &ref, output_data, IVE_IMAGE_TYPE_U8C1,
			       input_w, input_h);

	// Create dst image.
	IVE_DST_IMAGE_S dst;

	CVI_IVE_CreateImage(handle, &dst, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);

	// Config Setting.
	IVE_FILTER_CTRL_S stCtrl;

	memcpy(stCtrl.as8Mask, mask, sizeof(CVI_S8) * 25);
	stCtrl.u8Norm = 4;

	// Run HW IVE.
	printf("Run HW IVE Img To ODMA.\n");
	CVI_IVE_imgInToOdma(handle, &src, &dst, &stCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst, &ref);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file, &dst);
	}

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeI(handle, &ref);
	CVI_SYS_FreeI(handle, &dst);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

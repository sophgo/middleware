#include "cvi_ive.h"
// #include "cvi_pattern.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_stcandicorner(int bTileMode, int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	int input_w, input_h;
	// char *input_data = NULL;
	// char *output_data = NULL;
	char input_data[64];
	char output_data[64];
	char output_file[64];

	if (bTileMode) {
		input_w = 640;
		input_h = 480;
		// input_data = data_tile_640x480_y;
		// output_data = data_tile_CandiCorner;
		strcpy(input_data, "res/ive/sky_640x480.yuv");
		strcpy(output_data, "res/ive/result/sample_tile_Shitomasi_sky_640x480.yuv");
		strcpy(output_file, "sample_tile_Shitomasi_sky_640x480.yuv");
	} else {
		input_w = 352;
		input_h = 288;
		// input_data = data_penguin_352x288_y;
		// output_data = data_CandiCorner;
		strcpy(input_data, "res/ive/penguin_352x288.gray.shitomasi.raw");
		strcpy(output_data, "res/ive/result/sample_Shitomasi_CandiCorner.yuv");
		strcpy(output_file, "sample_Shitomasi_CandiCorner.yuv");
	}

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src;

	CVI_IVE_ReadRawImage(handle, &src, (char *)input_data,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	// Create ref image.
	IVE_IMAGE_S ref;

	CVI_IVE_ReadRawImage(handle, &ref, (char *)output_data,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	// Create dst image.
	IVE_DST_IMAGE_S dst;

	CVI_IVE_CreateImage(handle, &dst, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);

	// Config Setting.
	IVE_ST_CANDI_CORNER_CTRL_S stStCandiCornerCtrl;

	stStCandiCornerCtrl.u0q8QualityLevel = 25;
	CVI_IVE_CreateMemInfo(handle, &stStCandiCornerCtrl.stMem,
			      4 * src.u32Height * src.u32Stride[0] +
				      sizeof(IVE_ST_MAX_EIG_S));

	// Run HW IVE.
	printf("Run HW IVE STCandiCorner.\n");
	CVI_IVE_STCandiCorner(handle, &src, &dst, &stStCandiCornerCtrl,
			      bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst, &ref);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file, &dst);
	}

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeI(handle, &ref);
	CVI_SYS_FreeI(handle, &dst);
	CVI_SYS_FreeM(handle, &stStCandiCornerCtrl.stMem);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

#include "cvi_ive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_bernsen(int bTileMode, int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	int input_w, input_h;
	char input_data[64];
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

	if (bTileMode) {
		strcpy(input_data, "res/ive/sky_640x480.yuv");
		strcpy(output_data1, "res/ive/result/sample_Bernsen_5x5.yuv");
		strcpy(output_data2, "res/ive/result/sample_Bernsen_5x5_Thresh.yuv");
		strcpy(output_data3, "res/ive/result/sample_Bernsen_5x5_Paper.yuv");
		strcpy(output_data4, "res/ive/result/sample_Bernsen_3x3.yuv");
		strcpy(output_data5, "res/ive/result/sample_Bernsen_3x3_Thresh.yuv");
		strcpy(output_data6, "res/ive/result/sample_Bernsen_3x3_Paper.yuv");
		input_w = 640;
		input_h = 480;
		strcpy(output_file1, "sample_tile_Bernsen_5x5.yuv");
		strcpy(output_file2, "sample_tile_Bernsen_5x5_Thresh.yuv");
		strcpy(output_file3, "sample_tile_Bernsen_5x5_Paper.yuv");
		strcpy(output_file4, "sample_tile_Bernsen_3x3.yuv");
		strcpy(output_file5, "sample_tile_Bernsen_3x3_Thresh.yuv");
		strcpy(output_file6, "sample_tile_Bernsen_3x3_Paper.yuv");
	} else {
		strcpy(input_data, "res/ive/00_352x288_y.yuv");
		strcpy(output_data1, "res/ive/result/sample_Bernsen_5x5.yuv");
		strcpy(output_data2, "res/ive/result/sample_Bernsen_5x5_Thresh.yuv");
		strcpy(output_data3, "res/ive/result/sample_Bernsen_5x5_Paper.yuv");
		strcpy(output_data4, "res/ive/result/sample_Bernsen_3x3.yuv");
		strcpy(output_data5, "res/ive/result/sample_Bernsen_3x3_Thresh.yuv");
		strcpy(output_data6, "res/ive/result/sample_Bernsen_3x3_Paper.yuv");
		input_w = 352;
		input_h = 288;
		strcpy(output_file1, "sample_Bernsen_5x5.yuv");
		strcpy(output_file2, "sample_Bernsen_5x5_Thresh.yuv");
		strcpy(output_file3, "sample_Bernsen_5x5_Paper.yuv");
		strcpy(output_file4, "sample_Bernsen_3x3.yuv");
		strcpy(output_file5, "sample_Bernsen_3x3_Thresh.yuv");
		strcpy(output_file6, "sample_Bernsen_3x3_Paper.yuv");
	}

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src;

	CVI_IVE_ReadRawImage(handle, &src, input_data,
					IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	// Create ref image.
	IVE_IMAGE_S ref_5x5_nor, ref_5x5_thr, ref_5x5_paper, ref_3x3_nor,
		ref_3x3_thr, ref_3x3_paper;

	CVI_IVE_ReadRawImage(handle, &ref_5x5_nor, output_data1,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_5x5_thr, output_data2,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_5x5_paper, output_data3,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_3x3_nor, output_data4,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_3x3_thr, output_data5,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_3x3_paper, output_data6,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	// Create dst image.
	IVE_DST_IMAGE_S dst_bernsen;

	CVI_IVE_CreateImage(handle, &dst_bernsen, IVE_IMAGE_TYPE_U8C1,
			    src.u32Width, src.u32Height);

	// Config Setting.
	IVE_BERNSEN_CTRL_S iveBernsenCtrl;

	iveBernsenCtrl.u8Thr = 128;
	iveBernsenCtrl.u8ContrastThreshold = 15;
	iveBernsenCtrl.u8WinSize = 5; /* 3x3 or 5x5 */

	// Run HW IVE
	printf("Run HW IVE Bernsen 5x5 NORMAL.\n");
	iveBernsenCtrl.enMode = IVE_BERNSEN_MODE_NORMAL;
	CVI_IVE_Bernsen(handle, &src, &dst_bernsen, &iveBernsenCtrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst_bernsen, &ref_5x5_nor);
	if (bWrite)
		CVI_IVE_WriteImg(handle, output_file1, &dst_bernsen);

	printf("Run HW IVE Bernsen 5x5 THRESH.\n");
	iveBernsenCtrl.enMode = IVE_BERNSEN_MODE_THRESH;
	CVI_IVE_Bernsen(handle, &src, &dst_bernsen, &iveBernsenCtrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst_bernsen, &ref_5x5_thr);
	if (bWrite)
		CVI_IVE_WriteImg(handle, output_file2,
				 &dst_bernsen);

	printf("Run HW IVE Bernsen 5x5 PAPER.\n");
	iveBernsenCtrl.enMode = IVE_BERNSEN_MODE_PAPER;
	CVI_IVE_Bernsen(handle, &src, &dst_bernsen, &iveBernsenCtrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst_bernsen, &ref_5x5_paper);
	if (bWrite)
		CVI_IVE_WriteImg(handle, output_file3, &dst_bernsen);

	iveBernsenCtrl.u8WinSize = 3; /* 3x3 or 5x5 */

	printf("Run HW IVE Bernsen 3x3 NORMAL.\n");
	iveBernsenCtrl.enMode = IVE_BERNSEN_MODE_NORMAL;
	CVI_IVE_Bernsen(handle, &src, &dst_bernsen, &iveBernsenCtrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst_bernsen, &ref_3x3_nor);
	if (bWrite)
		CVI_IVE_WriteImg(handle, output_file4, &dst_bernsen);

	printf("Run HW IVE Bernsen 3x3 THRESH.\n");
	iveBernsenCtrl.enMode = IVE_BERNSEN_MODE_THRESH;
	CVI_IVE_Bernsen(handle, &src, &dst_bernsen, &iveBernsenCtrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst_bernsen, &ref_3x3_thr);
	if (bWrite)
		CVI_IVE_WriteImg(handle, output_file5, &dst_bernsen);

	printf("Run HW IVE Bernsen 3x3 PAPER.\n");
	iveBernsenCtrl.enMode = IVE_BERNSEN_MODE_PAPER;
	CVI_IVE_Bernsen(handle, &src, &dst_bernsen, &iveBernsenCtrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst_bernsen, &ref_3x3_paper);
	if (bWrite)
		CVI_IVE_WriteImg(handle, output_file6, &dst_bernsen);

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeI(handle, &dst_bernsen);
	CVI_SYS_FreeI(handle, &ref_5x5_nor);
	CVI_SYS_FreeI(handle, &ref_5x5_thr);
	CVI_SYS_FreeI(handle, &ref_5x5_paper);
	CVI_SYS_FreeI(handle, &ref_3x3_nor);
	CVI_SYS_FreeI(handle, &ref_3x3_thr);
	CVI_SYS_FreeI(handle, &ref_3x3_paper);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

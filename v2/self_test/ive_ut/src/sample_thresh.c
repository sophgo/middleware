#include "cvi_ive.h"
// #include "cvi_pattern.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_threshold(int bTileMode, int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	int input_w, input_h;
	// char *input_data = NULL;
	// char *output_data1 = NULL;
	// char *output_data2 = NULL;
	// char *output_data3 = NULL;
	// char *output_data4 = NULL;
	// char *output_data5 = NULL;
	// char *output_data6 = NULL;
	// char *output_data7 = NULL;
	// char *output_data8 = NULL;
	char input_data[64];
	char output_data1[64];
	char output_data2[64];
	char output_data3[64];
	char output_data4[64];
	char output_data5[64];
	char output_data6[64];
	char output_data7[64];
	char output_data8[64];
	char output_file_Binary[64];
	char output_file_Trunc[64];
	char output_file_ToMinVal[64];
	char output_file_MinMidMax[64];
	char output_file_MinMidOri[64];
	char output_file_MinOriMax[64];
	char output_file_OriMidMax[64];
	char output_file_OriMidOri[64];

	if (bTileMode) {
		// input_data = data_tile_640x480_y;
		// output_data1 = data_tile_Thresh_Binary;
		// output_data2 = data_tile_Thresh_Trunc;
		// output_data3 = data_tile_Thresh_ToMinVal;
		// output_data4 = data_tile_Thresh_MinMidMax;
		// output_data5 = data_tile_Thresh_MinMidOri;
		// output_data6 = data_tile_Thresh_MinOriMax;
		// output_data7 = data_tile_Thresh_OriMidMax;
		// output_data8 = data_tile_Thresh_OriMidOri;
		strcpy(input_data, "res/ive/sky_640x480.yuv");
		strcpy(output_data1, "res/ive/result/sample_tile_Thresh_Binary.yuv");
		strcpy(output_data2, "res/ive/result/sample_tile_Thresh_Trunc.yuv");
		strcpy(output_data3, "res/ive/result/sample_tile_Thresh_ToMinVal.yuv");
		strcpy(output_data4, "res/ive/result/sample_tile_Thresh_MinMidMax.yuv");
		strcpy(output_data5, "res/ive/result/sample_tile_Thresh_MinMidOri.yuv");
		strcpy(output_data6, "res/ive/result/sample_tile_Thresh_MinOriMax.yuv");
		strcpy(output_data7, "res/ive/result/sample_tile_Thresh_OriMidMax.yuv");
		strcpy(output_data8, "res/ive/result/sample_tile_Thresh_OriMidOri.yuv");
		input_w = 640;
		input_h = 480;
		strcpy(output_file_Binary, "sample_tile_Thresh_Binary.yuv");
		strcpy(output_file_Trunc, "sample_tile_Thresh_Trunc.yuv");
		strcpy(output_file_ToMinVal, "sample_tile_Thresh_ToMinVal.yuv");
		strcpy(output_file_MinMidMax,
		       "sample_tile_Thresh_MinMidMax.yuv");
		strcpy(output_file_MinMidOri,
		       "sample_tile_Thresh_MinMidOri.yuv");
		strcpy(output_file_MinOriMax,
		       "sample_tile_Thresh_MinOriMax.yuv");
		strcpy(output_file_OriMidMax,
		       "sample_tile_Thresh_OriMidMax.yuv");
		strcpy(output_file_OriMidOri,
		       "sample_tile_Thresh_OriMidOri.yuv");
	} else {
		// input_data = data_00_352x288_y;
		// output_data1 = data_Thresh_Binary;
		// output_data2 = data_Thresh_Trunc;
		// output_data3 = data_Thresh_ToMinVal;
		// output_data4 = data_Thresh_MinMidMax;
		// output_data5 = data_Thresh_MinMidOri;
		// output_data6 = data_Thresh_MinOriMax;
		// output_data7 = data_Thresh_OriMidMax;
		// output_data8 = data_Thresh_OriMidOri;
		strcpy(input_data, "res/ive/00_352x288_y.yuv");
		strcpy(output_data1, "res/ive/result/sample_Thresh_Binary.yuv");
		strcpy(output_data2, "res/ive/result/sample_Thresh_Trunc.yuv");
		strcpy(output_data3, "res/ive/result/sample_Thresh_ToMinVal.yuv");
		strcpy(output_data4, "res/ive/result/sample_Thresh_MinMidMax.yuv");
		strcpy(output_data5, "res/ive/result/sample_Thresh_MinMidOri.yuv");
		strcpy(output_data6, "res/ive/result/sample_Thresh_MinOriMax.yuv");
		strcpy(output_data7, "res/ive/result/sample_Thresh_OriMidMax.yuv");
		strcpy(output_data8, "res/ive/result/sample_Thresh_OriMidOri.yuv");
		input_w = 352;
		input_h = 288;
		strcpy(output_file_Binary, "sample_Thresh_Binary.yuv");
		strcpy(output_file_Trunc, "sample_Thresh_Trunc.yuv");
		strcpy(output_file_ToMinVal, "sample_Thresh_ToMinVal.yuv");
		strcpy(output_file_MinMidMax, "sample_Thresh_MinMidMax.yuv");
		strcpy(output_file_MinMidOri, "sample_Thresh_MinMidOri.yuv");
		strcpy(output_file_MinOriMax, "sample_Thresh_MinOriMax.yuv");
		strcpy(output_file_OriMidMax, "sample_Thresh_OriMidMax.yuv");
		strcpy(output_file_OriMidOri, "sample_Thresh_OriMidOri.yuv");
	}
	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src;

	CVI_IVE_ReadRawImage(handle, &src, (char *)input_data,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	// Create ref image.
	IVE_IMAGE_S ref_binary, ref_tominval, ref_turnc, ref_minmidmax,
		ref_minmidori, ref_minorimax, ref_orimidmax, ref_orimidori;
	CVI_IVE_ReadRawImage(handle, &ref_binary, (char *)output_data1,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_turnc, (char *)output_data2,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_tominval, (char *)output_data3,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_minmidmax, (char *)output_data4,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_minmidori, (char *)output_data5,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_minorimax, (char *)output_data6,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_orimidmax, (char *)output_data7,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_orimidori, (char *)output_data8,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	// Create dst image.
	IVE_DST_IMAGE_S dst;

	CVI_IVE_CreateImage(handle, &dst, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);

	// Config Setting.
	IVE_THRESH_CTRL_S iveThreshCtrl;

	iveThreshCtrl.enMode = IVE_THRESH_MODE_BINARY;
	iveThreshCtrl.u8LowThr = 41;
	iveThreshCtrl.u8HighThr = 105;
	iveThreshCtrl.u8MinVal = 190;
	iveThreshCtrl.u8MidVal = 132;
	iveThreshCtrl.u8MaxVal = 225;

	// Run HW IVE.
	printf("Run HW IVE Threashold BINARY.\n");
	CVI_IVE_Thresh(handle, &src, &dst, &iveThreshCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst, &ref_binary);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_Binary, &dst);
	}

	// Config Setting.
	iveThreshCtrl.enMode = IVE_THRESH_MODE_TRUNC;
	iveThreshCtrl.u8LowThr = 169;
	iveThreshCtrl.u8HighThr = 210;
	iveThreshCtrl.u8MinVal = 174;
	iveThreshCtrl.u8MidVal = 82;
	iveThreshCtrl.u8MaxVal = 144;

	// Run HW IVE.
	printf("Run HW IVE Threashold TRUNC.\n");
	CVI_IVE_Thresh(handle, &src, &dst, &iveThreshCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst, &ref_turnc);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_Trunc, &dst);
	}

	// Config Setting.
	iveThreshCtrl.enMode = IVE_THRESH_MODE_TO_MINVAL;
	iveThreshCtrl.u8LowThr = 95;
	iveThreshCtrl.u8HighThr = 241;
	iveThreshCtrl.u8MinVal = 241;
	iveThreshCtrl.u8MidVal = 187;
	iveThreshCtrl.u8MaxVal = 233;

	// Run HW IVE.
	printf("Run HW IVE Threashold TO_MINVAL.\n");
	CVI_IVE_Thresh(handle, &src, &dst, &iveThreshCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst, &ref_tominval);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_ToMinVal, &dst);
	}

	// Config Setting.
	iveThreshCtrl.enMode = IVE_THRESH_MODE_MIN_MID_MAX;
	iveThreshCtrl.u8LowThr = 236;
	iveThreshCtrl.u8HighThr = 249;
	iveThreshCtrl.u8MinVal = 166;
	iveThreshCtrl.u8MidVal = 219;
	iveThreshCtrl.u8MaxVal = 60;

	// Run HW IVE.
	printf("Run HW IVE Threashold MIN_MID_MAX.\n");
	CVI_IVE_Thresh(handle, &src, &dst, &iveThreshCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst, &ref_minmidmax);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_MinMidMax, &dst);
	}

	// Config Setting.
	iveThreshCtrl.enMode = IVE_THRESH_MODE_ORI_MID_MAX;
	iveThreshCtrl.u8LowThr = 6;
	iveThreshCtrl.u8HighThr = 169;
	iveThreshCtrl.u8MinVal = 62;
	iveThreshCtrl.u8MidVal = 153;
	iveThreshCtrl.u8MaxVal = 36;

	// Run HW IVE.
	printf("Run HW IVE Threashold ORI_MID_MAX.\n");
	CVI_IVE_Thresh(handle, &src, &dst, &iveThreshCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst, &ref_orimidmax);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_OriMidMax, &dst);
	}

	// Config Setting.
	iveThreshCtrl.enMode = IVE_THRESH_MODE_MIN_MID_ORI;
	iveThreshCtrl.u8LowThr = 142;
	iveThreshCtrl.u8HighThr = 162;
	iveThreshCtrl.u8MinVal = 28;
	iveThreshCtrl.u8MidVal = 6;
	iveThreshCtrl.u8MaxVal = 183;

	// Run HW IVE.
	printf("Run HW IVE Threashold MIN_MID_ORI.\n");
	CVI_IVE_Thresh(handle, &src, &dst, &iveThreshCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst, &ref_minmidori);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_MinMidOri, &dst);
	}

	// Config Setting.
	iveThreshCtrl.enMode = IVE_THRESH_MODE_MIN_ORI_MAX;
	iveThreshCtrl.u8LowThr = 92;
	iveThreshCtrl.u8HighThr = 140;
	iveThreshCtrl.u8MinVal = 179;
	iveThreshCtrl.u8MidVal = 18;
	iveThreshCtrl.u8MaxVal = 77;

	// Run HW IVE.
	printf("Run HW IVE Threashold MIN_ORI_MAX.\n");
	CVI_IVE_Thresh(handle, &src, &dst, &iveThreshCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst, &ref_minorimax);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_MinOriMax, &dst);
	}

	// Config Setting.
	iveThreshCtrl.enMode = IVE_THRESH_MODE_ORI_MID_ORI;
	iveThreshCtrl.u8LowThr = 22;
	iveThreshCtrl.u8HighThr = 60;
	iveThreshCtrl.u8MinVal = 187;
	iveThreshCtrl.u8MidVal = 139;
	iveThreshCtrl.u8MaxVal = 166;

	// Run HW IVE.
	printf("Run HW IVE Threashold ORI_MID_ORI.\n");
	CVI_IVE_Thresh(handle, &src, &dst, &iveThreshCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst, &ref_orimidori);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_OriMidOri, &dst);
	}

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeI(handle, &dst);
	CVI_SYS_FreeI(handle, &ref_binary);
	CVI_SYS_FreeI(handle, &ref_turnc);
	CVI_SYS_FreeI(handle, &ref_tominval);
	CVI_SYS_FreeI(handle, &ref_minmidmax);
	CVI_SYS_FreeI(handle, &ref_minmidori);
	CVI_SYS_FreeI(handle, &ref_minorimax);
	CVI_SYS_FreeI(handle, &ref_orimidmax);
	CVI_SYS_FreeI(handle, &ref_orimidori);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

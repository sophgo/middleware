#include "cvi_ive.h"
#include "cvi_pattern.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_magandang(int bTileMode, int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	int input_w, input_h;
	// char *input_data = NULL;
	// char *output_data_3x3_Mag = NULL;
	// char *output_data_3x3_Ang = NULL;
	// char *output_data_3x3_MagThr = NULL;
	// char *output_data_5x5_Mag = NULL;
	// char *output_data_5x5_Ang = NULL;
	// char *output_data_5x5_MagThr = NULL;
	char input_data[64];
	char output_data_3x3_Mag[64];
	char output_data_3x3_Ang[64];
	char output_data_3x3_MagThr[64];
	char output_data_5x5_Mag[64];
	char output_data_5x5_Ang[64];
	char output_data_5x5_MagThr[64];

	char output_file_3x3_Mag[64];
	char output_file_3x3_Ang[64];
	char output_file_3x3_Only_Mag[64];
	char output_file_3x3_Thresh[64];
	char output_file_5x5_Mag[64];
	char output_file_5x5_Ang[64];
	char output_file_5x5_Only_Mag[64];
	char output_file_5x5_Thresh[64];

	if (bTileMode) {
		// input_data = data_tile_640x480_y;
		// output_data_3x3_Mag = data_tile_MagAndAng_3x3_Mag;
		// output_data_3x3_Ang = data_tile_MagAndAng_3x3_Ang;
		// output_data_3x3_MagThr = data_tile_MagAndAng_3x3_MagThr;
		// output_data_5x5_Mag = data_tile_MagAndAng_5x5_Mag;
		// output_data_5x5_Ang = data_tile_MagAndAng_5x5_Ang;
		// output_data_5x5_MagThr = data_tile_MagAndAng_5x5_MagThr;
		strcpy(input_data, "res/ive/sky_640x480.yuv");
		strcpy(output_data_3x3_Mag, "res/ive/result/sample_tile_MagAndAng_MagAndAng3x3_Mag.yuv");
		strcpy(output_data_3x3_Ang, "res/ive/result/sample_tile_MagAndAng_MagAndAng3x3_Ang.yuv");
		strcpy(output_data_3x3_MagThr, "res/ive/result/sample_tile_MagAndAng_Thresh3x3_Mag.yuv");
		strcpy(output_data_5x5_Mag, "res/ive/result/sample_tile_MagAndAng_MagAndAng5x5_Mag.yuv");
		strcpy(output_data_5x5_Ang, "res/ive/result/sample_tile_MagAndAng_MagAndAng5x5_Ang.yuv");
		strcpy(output_data_5x5_MagThr, "res/ive/result/sample_tile_MagAndAng_Thresh5x5_Mag.yuv");
		input_w = 640;
		input_h = 480;
		strcpy(output_file_3x3_Mag,
		       "sample_tile_MagAndAng_MagAndAng3x3_Mag.yuv");
		strcpy(output_file_3x3_Ang,
		       "sample_tile_MagAndAng_MagAndAng3x3_Ang.yuv");
		strcpy(output_file_3x3_Only_Mag,
		       "sample_tile_MagAndAng_Mag3x3_Mag.yuv");
		strcpy(output_file_3x3_Thresh,
		       "sample_tile_MagAndAng_Thresh3x3_Mag.yuv");
		strcpy(output_file_5x5_Mag,
		       "sample_tile_MagAndAng_MagAndAng5x5_Mag.yuv");
		strcpy(output_file_5x5_Ang,
		       "sample_tile_MagAndAng_MagAndAng5x5_Ang.yuv");
		strcpy(output_file_5x5_Only_Mag,
		       "sample_tile_MagAndAng_Mag5x5_Mag.yuv");
		strcpy(output_file_5x5_Thresh,
		       "sample_tile_MagAndAng_Thresh5x5_Mag.yuv");
	} else {
		// input_data = data_00_352x288_y;
		// output_data_3x3_Mag = data_MagAndAng_3x3_Mag;
		// output_data_3x3_Ang = data_MagAndAng_3x3_Ang;
		// output_data_3x3_MagThr = data_MagAndAng_3x3_MagThr;
		// output_data_5x5_Mag = data_MagAndAng_5x5_Mag;
		// output_data_5x5_Ang = data_MagAndAng_5x5_Ang;
		// output_data_5x5_MagThr = data_MagAndAng_5x5_MagThr;
		strcpy(input_data, "res/ive/00_352x288_y.yuv");
		strcpy(output_data_3x3_Mag, "res/ive/result/sample_MagAndAng_MagAndAng3x3_Mag.yuv");
		strcpy(output_data_3x3_Ang, "res/ive/result/sample_MagAndAng_MagAndAng3x3_Ang.yuv");
		strcpy(output_data_3x3_MagThr, "res/ive/result/sample_MagAndAng_Thresh3x3_Mag.yuv");
		strcpy(output_data_5x5_Mag, "res/ive/result/sample_MagAndAng_MagAndAng5x5_Mag.yuv");
		strcpy(output_data_5x5_Ang, "res/ive/result/sample_MagAndAng_MagAndAng5x5_Ang.yuv");
		strcpy(output_data_5x5_MagThr, "res/ive/result/sample_MagAndAng_Thresh5x5_Mag.yuv");
		input_w = 352;
		input_h = 288;
		strcpy(output_file_3x3_Mag,
		       "sample_MagAndAng_MagAndAng3x3_Mag.yuv");
		strcpy(output_file_3x3_Ang,
		       "sample_MagAndAng_MagAndAng3x3_Ang.yuv");
		strcpy(output_file_3x3_Only_Mag,
		       "sample_MagAndAng_Mag3x3_Mag.yuv");
		strcpy(output_file_3x3_Thresh,
		       "sample_MagAndAng_Thresh3x3_Mag.yuv");
		strcpy(output_file_5x5_Mag,
		       "sample_MagAndAng_MagAndAng5x5_Mag.yuv");
		strcpy(output_file_5x5_Ang,
		       "sample_MagAndAng_MagAndAng5x5_Ang.yuv");
		strcpy(output_file_5x5_Only_Mag,
		       "sample_MagAndAng_Mag5x5_Mag.yuv");
		strcpy(output_file_5x5_Thresh,
		       "sample_MagAndAng_Thresh5x5_Mag.yuv");
	}

	/* 5 by 5*/
	CVI_S8 arr5by5[25] = { -1, -2, 0,  2,  1, -4, -8, 0,  8,  4, -6, -12, 0,
			       12, 6,  -4, -8, 0, 8,  4,  -1, -2, 0, 2,	 1 };
	/* 3 by 3*/
	CVI_S8 arr3by3[25] = { 0, 0, 0, 0,  0, 0, -1, 0, 1, 0, 0, -2, 0,
			       2, 0, 0, -1, 0, 1, 0,  0, 0, 0, 0, 0 };
	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src;

	CVI_IVE_ReadRawImage(handle, &src, input_data, IVE_IMAGE_TYPE_U8C1,
			       input_w, input_h);

	// Create ref image.
	IVE_IMAGE_S ref_mag3x3, ref_mag5x5, ref_ang3x3, ref_ang5x5,
		ref_mag3x3thr, ref_mag5x5thr;

	CVI_IVE_ReadRawImage(handle, &ref_mag3x3, output_data_3x3_Mag,
			       IVE_IMAGE_TYPE_U16C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_ang3x3, output_data_3x3_Ang,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_mag5x5, output_data_5x5_Mag,
			       IVE_IMAGE_TYPE_U16C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_ang5x5, output_data_5x5_Ang,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_mag3x3thr, output_data_3x3_MagThr,
			       IVE_IMAGE_TYPE_U16C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_mag5x5thr, output_data_5x5_MagThr,
			       IVE_IMAGE_TYPE_U16C1, input_w, input_h);

	// Create dst image.
	IVE_DST_IMAGE_S dst_mag, dst_ang;

	CVI_IVE_CreateImage(handle, &dst_mag, IVE_IMAGE_TYPE_U16C1, input_w,
			    input_h);
	CVI_IVE_CreateImage(handle, &dst_ang, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);

	// Config Setting.
	IVE_MAG_AND_ANG_CTRL_S pstMaaCtrl;

	pstMaaCtrl.u16Thr = 0;
	memcpy(pstMaaCtrl.as8Mask, arr3by3, 5 * 5 * sizeof(CVI_S8));

	// Run HW IVE.
	printf("Run HW IVE MagAndAng 3x3 MAG_AND_ANG.\n");
	pstMaaCtrl.enOutCtrl = IVE_MAG_AND_ANG_OUT_CTRL_MAG_AND_ANG;
	CVI_IVE_MagAndAng(handle, &src, &dst_mag, &dst_ang, &pstMaaCtrl,
			  bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst_mag, &ref_mag3x3);
	ret |= CVI_IVE_CompareIveImage(&dst_ang, &ref_ang3x3);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_3x3_Mag, &dst_mag);
		CVI_IVE_WriteImg(handle, output_file_3x3_Ang, &dst_ang);
	}

	printf("Run HW IVE MagAndAng 3x3 MAG_AND_ANG.\n");
	pstMaaCtrl.enOutCtrl = IVE_MAG_AND_ANG_OUT_CTRL_MAG;
	CVI_IVE_MagAndAng(handle, &src, &dst_mag, CVI_NULL, &pstMaaCtrl,
			  bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst_mag, &ref_mag3x3);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_3x3_Only_Mag, &dst_mag);
	}

	memcpy(pstMaaCtrl.as8Mask, arr5by5, 5 * 5 * sizeof(CVI_S8));

	printf("Run HW IVE MagAndAng 5x5 MAG_AND_ANG.\n");
	pstMaaCtrl.enOutCtrl = IVE_MAG_AND_ANG_OUT_CTRL_MAG_AND_ANG;
	CVI_IVE_MagAndAng(handle, &src, &dst_mag, &dst_ang, &pstMaaCtrl,
			  bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst_mag, &ref_mag5x5);
	ret |= CVI_IVE_CompareIveImage(&dst_ang, &ref_ang5x5);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_5x5_Mag, &dst_mag);
		CVI_IVE_WriteImg(handle, output_file_5x5_Ang, &dst_ang);
	}

	printf("Run HW IVE MagAndAng 5x5 MAG_AND_ANG.\n");
	pstMaaCtrl.enOutCtrl = IVE_MAG_AND_ANG_OUT_CTRL_MAG;
	CVI_IVE_MagAndAng(handle, &src, &dst_mag, CVI_NULL, &pstMaaCtrl,
			  bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst_mag, &ref_mag5x5);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_5x5_Only_Mag, &dst_mag);
	}

	printf("Run HW IVE MagAndAng 3x3 MAG Thresh.\n");
	memcpy(pstMaaCtrl.as8Mask, arr3by3, 5 * 5 * sizeof(CVI_S8));
	pstMaaCtrl.u16Thr = 42;
	CVI_IVE_MagAndAng(handle, &src, &dst_mag, CVI_NULL, &pstMaaCtrl,
			  bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst_mag, &ref_mag3x3thr);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_3x3_Thresh, &dst_mag);
	}

	printf("Run HW IVE MagAndAng 5x5 MAG Thresh.\n");
	memcpy(pstMaaCtrl.as8Mask, arr5by5, 5 * 5 * sizeof(CVI_S8));
	pstMaaCtrl.u16Thr = 18468;
	CVI_IVE_MagAndAng(handle, &src, &dst_mag, CVI_NULL, &pstMaaCtrl,
			  bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst_mag, &ref_mag5x5thr);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, output_file_5x5_Thresh, &dst_mag);
	}

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeI(handle, &ref_mag3x3);
	CVI_SYS_FreeI(handle, &ref_mag5x5);
	CVI_SYS_FreeI(handle, &ref_ang3x3);
	CVI_SYS_FreeI(handle, &ref_ang5x5);
	CVI_SYS_FreeI(handle, &ref_mag3x3thr);
	CVI_SYS_FreeI(handle, &ref_mag5x5thr);
	CVI_SYS_FreeI(handle, &dst_mag);
	CVI_SYS_FreeI(handle, &dst_ang);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}
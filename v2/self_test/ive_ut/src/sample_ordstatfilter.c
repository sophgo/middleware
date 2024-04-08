#include "cvi_ive.h"
// #include "cvi_pattern.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_ordstatfilter(int bTileMode, int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	int input_w, input_h;
	// char *input_data = NULL;
	// char *output_data1 = NULL;
	// char *output_data2 = NULL;
	// char *output_data3 = NULL;
	char input_data[64];
	char output_data1[64];
	char output_data2[64];
	char output_data3[64];
	char output_file1[64];
	char output_file2[64];
	char output_file3[64];

	if (bTileMode) {
		// input_data = data_tile_640x480_y;
		// output_data1 = data_OrdStaFilter_Max;
		// output_data2 = data_OrdStaFilter_Min;
		// output_data3 = data_OrdStaFilter_Median;
		strcpy(input_data, "res/ive/sky_640x480.yuv");
		strcpy(output_data1, "res/ive/result/sample_tile_OrdStaFilter_Max.yuv");
		strcpy(output_data2, "res/ive/result/sample_tile_OrdStaFilter_Min.yuv");
		strcpy(output_data3, "res/ive/result/sample_tile_OrdStaFilter_Median.yuv");
		input_w = 640;
		input_h = 480;
		strcpy(output_file1, "sample_tile_OrdStaFilter_Max.yuv");
		strcpy(output_file2, "sample_tile_OrdStaFilter_Min.yuv");
		strcpy(output_file3, "sample_tile_OrdStaFilter_Median.yuv");
	} else {
		// input_data = data_00_352x288_y;
		// output_data1 = data_OrdStaFilter_Max;
		// output_data2 = data_OrdStaFilter_Min;
		// output_data3 = data_OrdStaFilter_Median;
		strcpy(input_data, "res/ive/00_352x288_y.yuv");
		strcpy(output_data1, "res/ive/result/sample_OrdStaFilter_Max.yuv");
		strcpy(output_data2, "res/ive/result/sample_OrdStaFilter_Min.yuv");
		strcpy(output_data3, "res/ive/result/sample_OrdStaFilter_Median.yuv");
		input_w = 352;
		input_h = 288;
		strcpy(output_file1, "sample_OrdStaFilter_Max.yuv");
		strcpy(output_file2, "sample_OrdStaFilter_Min.yuv");
		strcpy(output_file3, "sample_OrdStaFilter_Median.yuv");
	}

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src;

	CVI_IVE_ReadRawImage(handle, &src, input_data,
					IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	// Create ref image.
	IVE_IMAGE_S ref_max, ref_min, ref_med;

	CVI_IVE_ReadRawImage(handle, &ref_max, output_data1,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_min, output_data2,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadRawImage(handle, &ref_med, output_data3,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	// Create dst image.
	IVE_DST_IMAGE_S dst;

	CVI_IVE_CreateImage(handle, &dst, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);

	// Run HW IVE.
	printf("Run HW IVE OrdStaFilter Max.\n");
	IVE_ORD_STAT_FILTER_CTRL_S pstOrdStatFltCtrl;

	pstOrdStatFltCtrl.enMode = IVE_ORD_STAT_FILTER_MODE_MAX;
	CVI_IVE_OrdStatFilter(handle, &src, &dst, &pstOrdStatFltCtrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst, &ref_max);
	if (bWrite)
		CVI_IVE_WriteImg(handle, output_file1, &dst);

	printf("Run HW IVE OrdStaFilter Min.\n");
	pstOrdStatFltCtrl.enMode = IVE_ORD_STAT_FILTER_MODE_MIN;
	CVI_IVE_OrdStatFilter(handle, &src, &dst, &pstOrdStatFltCtrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst, &ref_min);
	if (bWrite)
		CVI_IVE_WriteImg(handle, output_file2, &dst);

	printf("Run HW IVE OrdStaFilter Median.\n");
	pstOrdStatFltCtrl.enMode = IVE_ORD_STAT_FILTER_MODE_MEDIAN;
	CVI_IVE_OrdStatFilter(handle, &src, &dst, &pstOrdStatFltCtrl, bInstant);
	if (!bTileMode)
		ret |= CVI_IVE_CompareIveImage(&dst, &ref_med);
	if (bWrite)
		CVI_IVE_WriteImg(handle, output_file3, &dst);

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeI(handle, &dst);
	CVI_SYS_FreeI(handle, &ref_max);
	CVI_SYS_FreeI(handle, &ref_min);
	CVI_SYS_FreeI(handle, &ref_med);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

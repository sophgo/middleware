#include "cvi_ive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

int test_framediffmotion(int bTileMode, int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	int input_w, input_h;
	char input_data1[64];
	char input_data2[64];
	char output_data[64];
	char input_array1[480 * 480 * 2];
	char input_array2[480 * 480 * 2];
	char output_file[64];
	char data_md1_480x480_y[480 * 480];
	char data_md2_480x480_y[480 * 480];




	//Read Array from file.
	int buf_size = 480 * 480;
	FILE *fp1, *fp2;

	strcpy(input_data1, "res/ive/md1_480x480.yuv");
	strcpy(input_data2, "res/ive/md2_480x480.yuv");

	fp1 = fopen(input_data1, "r");
	if (fp1 == NULL) {
		printf("open file %s failed\n", input_data1);
	}
	int readCnt = fread(data_md1_480x480_y, 1, buf_size, fp1);

	if (readCnt == 0) {
		printf("Image %s read failed.\n", input_data1);
	}

	fp2 = fopen(input_data2, "r");
	if (fp2 == NULL) {
		printf("open file %s failed\n", input_data2);
	}
	readCnt = fread(data_md2_480x480_y, 1, buf_size, fp2);

	if (readCnt == 0) {
		printf("Image %s read failed.\n", input_data2);
	}
	fclose(fp1);
	fclose(fp2);
	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Read image from file. CVI_IVE_ReadImage will do the flush for you.
	IVE_IMAGE_S src1, src2;

	if (bTileMode) {
		for (int i = 0; i < 480; i++) {
			memcpy(&input_array1[i * 480 * 2],
			       &data_md1_480x480_y[i * 480], 480);
			memcpy(&input_array1[i * 480 * 2 + 480],
			       &data_md1_480x480_y[i * 480], 480);
			memcpy(&input_array2[i * 480 * 2],
			       &data_md2_480x480_y[i * 480], 480);
			memcpy(&input_array2[i * 480 * 2 + 480],
			       &data_md2_480x480_y[i * 480], 480);
		}
		strcpy(output_data, "res/ive/result/sample_tile_FrameDiffMotion.yuv");
		input_w = 480 * 2;
		input_h = 480;
		strcpy(output_file, "sample_tile_FrameDiffMotion.yuv");

		CVI_IVE_ReadImageArray(handle, &src1, input_array1, IVE_IMAGE_TYPE_U8C1,
					input_w, input_h);
		CVI_IVE_ReadImageArray(handle, &src2, input_array2, IVE_IMAGE_TYPE_U8C1,
					input_w, input_h);

	} else {
		strcpy(output_data, "res/ive/result/sample_FrameDiffMotion.yuv");
		input_w = 480;
		input_h = 480;
		strcpy(output_file, "sample_FrameDiffMotion.yuv");

		CVI_IVE_ReadRawImage(handle, &src1, input_data1, IVE_IMAGE_TYPE_U8C1,
					input_w, input_h);
		CVI_IVE_ReadRawImage(handle, &src2, input_data2, IVE_IMAGE_TYPE_U8C1,
					input_w, input_h);

	}

	// Create ref image.
	IVE_IMAGE_S ref;

	CVI_IVE_ReadRawImage(handle, &ref, output_data, IVE_IMAGE_TYPE_U8C1,
			       input_w, input_h);

	// Create dst image.
	IVE_DST_IMAGE_S dst;

	CVI_IVE_CreateImage(handle, &dst, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);

	// Config Setting (Sub -> threshold -> erode -> dilate)
	IVE_FRAME_DIFF_MOTION_CTRL_S iveMDCtrl;

	iveMDCtrl.enSubMode = IVE_SUB_MODE_ABS;
	iveMDCtrl.enThrMode = IVE_THRESH_MODE_BINARY;
	iveMDCtrl.u8ThrMinVal = 0;
	iveMDCtrl.u8ThrMaxVal = 255;
	iveMDCtrl.u8ThrLow = 30;
	CVI_U8 arr[] = { 0, 0,	 255, 0,   0,	0,   0, 255, 0,
			 0, 255, 255, 255, 255, 255, 0, 0,   255,
			 0, 0,	 0,   0,   255, 0,   0 };
	memcpy(iveMDCtrl.au8ErodeMask, arr, 25 * sizeof(CVI_U8));
	memcpy(iveMDCtrl.au8DilateMask, arr, 25 * sizeof(CVI_U8));

	// Run IVE
	CVI_IVE_FrameDiffMotion(handle, &src1, &src2, &dst, &iveMDCtrl,
				bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst, &ref);

	if (bWrite)
		CVI_IVE_WriteImg(handle, output_file, &dst);

	CVI_SYS_FreeI(handle, &src1);
	CVI_SYS_FreeI(handle, &src2);
	CVI_SYS_FreeI(handle, &dst);
	CVI_SYS_FreeI(handle, &ref);
	CVI_IVE_DestroyHandle(handle);
	return ret;
}

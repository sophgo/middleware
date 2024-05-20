#include "cvi_ive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_csc(int bTileMode, int bWrite, int bInstant)
{
	UNUSED(bTileMode);
	int ret = CVI_SUCCESS;
	char input_data1[64];
	char input_data2[64];
	char input_data3[64];
	char output_data1[64];
	char output_data2[64];
	char output_data3[64];
	char output_data4[64];
	char output_data5[64];


	strcpy(input_data1, "res/ive/00_352x288_SP420.yuv");
	strcpy(input_data2, "res/ive/00_352x288_444.yuv");
	strcpy(input_data3, "res/ive/lena_480x480_planar.yuv");
	strcpy(output_data1, "res/ive/result/sample_CSC_BT601_YUV2HSV_480x480.vsh");
	strcpy(output_data2, "res/ive/result/sample_CSC_BT601_YUV2LAB_480x480.bal");
	strcpy(output_data3, "res/ive/result/sample_CSC_BT709_YUV2HSV_480x480.vsh");
	strcpy(output_data4, "res/ive/result/sample_CSC_BT709_YUV2LAB_480x480.bal");
	strcpy(output_data5, "res/ive/result/sample_CSC_YUV2RGB.rgb");

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src_420, src_444, src_pln;

	CVI_IVE_ReadRawImage(handle, &src_420, input_data1,
			       IVE_IMAGE_TYPE_YUV420SP, 352, 288);
	CVI_IVE_ReadRawImage(handle, &src_444, input_data2,
			       IVE_IMAGE_TYPE_U8C3_PLANAR, 352, 288);
	CVI_IVE_ReadRawImage(handle, &src_pln, input_data3,
			       IVE_IMAGE_TYPE_U8C3_PLANAR, 480, 480);

	// Create ref image.
	IVE_IMAGE_S ref_bt601_hsv, ref_bt601_lib, ref_bt709_hsv, ref_bt709_lib,
		ref_yuv2rgb;

	CVI_IVE_ReadRawImage(handle, &ref_bt601_hsv, output_data1,
			       IVE_IMAGE_TYPE_U8C3_PLANAR, src_pln.u32Width,
			       src_pln.u32Height);
	CVI_IVE_ReadRawImage(handle, &ref_bt601_lib, output_data2,
			       IVE_IMAGE_TYPE_U8C3_PLANAR, src_pln.u32Width,
			       src_pln.u32Height);
	CVI_IVE_ReadRawImage(handle, &ref_bt709_hsv, output_data3,
			       IVE_IMAGE_TYPE_U8C3_PLANAR, src_pln.u32Width,
			       src_pln.u32Height);
	CVI_IVE_ReadRawImage(handle, &ref_bt709_lib, output_data4,
			       IVE_IMAGE_TYPE_U8C3_PLANAR, src_pln.u32Width,
			       src_pln.u32Height);
	CVI_IVE_ReadRawImage(handle, &ref_yuv2rgb, output_data5,
			       IVE_IMAGE_TYPE_U8C3_PACKAGE, src_420.u32Width,
			       src_420.u32Height);

	// Create dst image.
	IVE_DST_IMAGE_S dst, dst1;

	CVI_IVE_CreateImage(handle, &dst, IVE_IMAGE_TYPE_U8C3_PACKAGE,
			    src_420.u32Width, src_420.u32Height);
	CVI_IVE_CreateImage(handle, &dst1, IVE_IMAGE_TYPE_U8C3_PLANAR,
			    src_pln.u32Width, src_pln.u32Height);

	// Config Setting.
	IVE_CSC_CTRL_S stCtrl;

	stCtrl.enMode = IVE_CSC_MODE_VIDEO_BT601_YUV2RGB;

	// Run HW IVE.
	printf("Run HW IVE CSC YUV2RGB.\n");
	CVI_IVE_CSC(handle, &src_444, &dst, &stCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst, &ref_yuv2rgb);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, "sample_CSC_YUV4442RGB.rgb", &dst);
	}

	printf("Run HW IVE CSC YUV2RGB.\n");
	CVI_IVE_CSC(handle, &src_420, &dst, &stCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst, &ref_yuv2rgb);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, "sample_CSC_YUV2RGB.rgb", &dst);
	}
	printf("Run HW IVE CSC BT601_YUV2HSV.\n");
	stCtrl.enMode = IVE_CSC_MODE_PIC_BT601_YUV2HSV;
	CVI_IVE_CSC(handle, &src_pln, &dst1, &stCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst1, &ref_bt601_hsv);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, "sample_CSC_BT601_YUV2HSV_480x480.vsh",
				 &dst1);
	}
	printf("Run HW IVE CSC BT709_YUV2HSV.\n");
	stCtrl.enMode = IVE_CSC_MODE_PIC_BT709_YUV2HSV;
	CVI_IVE_CSC(handle, &src_pln, &dst1, &stCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst1, &ref_bt709_hsv);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, "sample_CSC_BT709_YUV2HSV_480x480.vsh",
				 &dst1);
	}
	printf("Run HW IVE CSC BT601_YUV2LAB.\n");
	stCtrl.enMode = IVE_CSC_MODE_PIC_BT601_YUV2LAB;
	CVI_IVE_CSC(handle, &src_pln, &dst1, &stCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst1, &ref_bt601_lib);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, "sample_CSC_BT601_YUV2LAB_480x480.bal",
				 &dst1);
	}
	printf("Run HW IVE CSC BT709_YUV2LAB.\n");
	stCtrl.enMode = IVE_CSC_MODE_PIC_BT709_YUV2LAB;
	CVI_IVE_CSC(handle, &src_pln, &dst1, &stCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&dst1, &ref_bt709_lib);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, "sample_CSC_BT709_YUV2LAB_480x480.bal",
				 &dst1);
	}

	CVI_SYS_FreeI(handle, &src_420);
	CVI_SYS_FreeI(handle, &src_444);
	CVI_SYS_FreeI(handle, &src_pln);
	CVI_SYS_FreeI(handle, &dst);
	CVI_SYS_FreeI(handle, &dst1);
	CVI_SYS_FreeI(handle, &ref_bt601_hsv);
	CVI_SYS_FreeI(handle, &ref_bt601_lib);
	CVI_SYS_FreeI(handle, &ref_bt709_hsv);
	CVI_SYS_FreeI(handle, &ref_bt709_lib);
	CVI_SYS_FreeI(handle, &ref_yuv2rgb);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

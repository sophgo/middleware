#include "cvi_ive.h"
// #include "cvi_pattern.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct IVE_IMAGE_FILL_U8C3_PLANAR_S {
	CVI_U8 u8Type;
	CVI_U32 u32SrcPhyAddr[3];
	CVI_U32 u32DstPhyAddr[3];
	CVI_U16 u16SrcStride[3];
	CVI_U16 u16DstStride[3];
	CVI_U16 u16SrcWidth;
	CVI_U16 u16SrcHeight;
	CVI_U16 u16DstWidth;
	CVI_U16 u16DstHeight;
	CVI_U16 u16XScale;
	CVI_U16 u16YScale;
} IVE_IMAGE_FILL_U8C3_PLANAR_S;

int test_resize(int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	char input_data1[64];
	char input_data2[64];
	char input_data3[64];
	char output_data1[64];
	char output_data2[64];
	char output_data3[64];
	char output_data4[64];
	char output_data5[64];
	char output_data6[64];

	strcpy(input_data1, "res/ive/campus_352x288.rgb");
	strcpy(input_data2, "res/ive/campus_352x288.gray");
	strcpy(input_data3, "res/ive/campus_352x288.rgb");
	strcpy(output_data1, "res/ive/result/sample_Resize_Area_240p.rgb");
	strcpy(output_data2, "res/ive/result/sample_Resize_Area_gray.yuv");
	strcpy(output_data3, "res/ive/result/sample_Resize_Area_rgb.rgb");
	strcpy(output_data4, "res/ive/result/sample_Resize_Bilinear_240p.rgb");
	strcpy(output_data5, "res/ive/result/sample_Resize_Bilinear_gray.yuv");
	strcpy(output_data6, "res/ive/result/sample_Resize_Bilinear_rgb.rgb");
	IVE_DST_IMAGE_S astDst[3];
	IVE_SRC_IMAGE_S astSrc[3];
	CVI_U16 au16ResizeWidth[3];
	CVI_U16 au16ResizeHeight[3];

	au16ResizeWidth[0] = 176; //u32Width/2;
	au16ResizeHeight[0] = 144; //u32Height/3;

	au16ResizeWidth[1] = 400; //u32Width/3;
	au16ResizeHeight[1] = 300; //u32Height/2;

	au16ResizeWidth[2] = 320; //u32Width/2;
	au16ResizeHeight[2] = 240; //u32Height/3;

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	CVI_IVE_ReadRawImage(handle, &astSrc[0], input_data1,
			       IVE_IMAGE_TYPE_U8C3_PLANAR, 352, 288);
	CVI_IVE_ReadRawImage(handle, &astSrc[1], input_data2,
			       IVE_IMAGE_TYPE_U8C1, 352, 288);
	CVI_IVE_ReadRawImage(handle, &astSrc[2], input_data3,
			       IVE_IMAGE_TYPE_U8C3_PLANAR, 352, 288);
	// Create ref image.
	IVE_IMAGE_S ref_area_240p, ref_area_gray, ref_area_rgb,
		ref_bilinear_240p, ref_bilinear_gray, ref_bilinear_rgb;

	CVI_IVE_ReadRawImage(handle, &ref_area_240p, output_data1,
			       IVE_IMAGE_TYPE_U8C3_PLANAR, au16ResizeWidth[2],
			       au16ResizeHeight[2]);
	CVI_IVE_ReadRawImage(handle, &ref_area_gray, output_data2,
			       IVE_IMAGE_TYPE_U8C1, au16ResizeWidth[1],
			       au16ResizeHeight[1]);
	CVI_IVE_ReadRawImage(handle, &ref_area_rgb, output_data3,
			       IVE_IMAGE_TYPE_U8C3_PLANAR, au16ResizeWidth[0],
			       au16ResizeHeight[0]);
	CVI_IVE_ReadRawImage(handle, &ref_bilinear_240p,
			       output_data4,
			       IVE_IMAGE_TYPE_U8C3_PLANAR, au16ResizeWidth[2],
			       au16ResizeHeight[2]);
	CVI_IVE_ReadRawImage(handle, &ref_bilinear_gray,
			       output_data5, IVE_IMAGE_TYPE_U8C1,
			       au16ResizeWidth[1], au16ResizeHeight[1]);
	CVI_IVE_ReadRawImage(handle, &ref_bilinear_rgb,
			       output_data6,
			       IVE_IMAGE_TYPE_U8C3_PLANAR, au16ResizeWidth[0],
			       au16ResizeHeight[0]);

	// Create dst image.
	CVI_IVE_CreateImage(handle, &astDst[0], IVE_IMAGE_TYPE_U8C3_PLANAR,
			    au16ResizeWidth[0], au16ResizeHeight[0]);
	CVI_IVE_CreateImage(handle, &astDst[1], IVE_IMAGE_TYPE_U8C1,
			    au16ResizeWidth[1], au16ResizeHeight[1]);
	CVI_IVE_CreateImage(handle, &astDst[2], IVE_IMAGE_TYPE_U8C3_PLANAR,
			    au16ResizeWidth[2], au16ResizeHeight[2]);

	// Config Setting.
	IVE_RESIZE_CTRL_S stCtrl;

	stCtrl.u16Num = 3;
	CVI_IVE_CreateMemInfo(handle, &(stCtrl.stMem),
			      stCtrl.u16Num *
				      sizeof(IVE_IMAGE_FILL_U8C3_PLANAR_S) * 2);

	// Run HW IVE.
	printf("Run HW IVE Resize LINEAR.\n");
	stCtrl.enMode = IVE_RESIZE_MODE_LINEAR;
	CVI_IVE_Resize(handle, &astSrc[0], &astDst[0], &stCtrl, bInstant);
	CVI_IVE_Resize(handle, &astSrc[1], &astDst[1], &stCtrl, bInstant);
	CVI_IVE_Resize(handle, &astSrc[2], &astDst[2], &stCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&astDst[2], &ref_bilinear_240p);
	ret |= CVI_IVE_CompareIveImage(&astDst[1], &ref_bilinear_gray);
	ret |= CVI_IVE_CompareIveImage(&astDst[0], &ref_bilinear_rgb);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, "sample_Resize_Bilinear_rgb.rgb",
				 &astDst[0]);
		CVI_IVE_WriteImg(handle, "sample_Resize_Bilinear_gray.yuv",
				 &astDst[1]);
		CVI_IVE_WriteImg(handle, "sample_Resize_Bilinear_240p.rgb",
				 &astDst[2]);
	}

	printf("Run HW IVE Resize AREA.\n");
	stCtrl.enMode = IVE_RESIZE_MODE_AREA;
	CVI_IVE_Resize(handle, &astSrc[0], &astDst[0], &stCtrl, bInstant);
	CVI_IVE_Resize(handle, &astSrc[1], &astDst[1], &stCtrl, bInstant);
	CVI_IVE_Resize(handle, &astSrc[2], &astDst[2], &stCtrl, bInstant);
	ret |= CVI_IVE_CompareIveImage(&astDst[2], &ref_area_240p);
	ret |= CVI_IVE_CompareIveImage(&astDst[1], &ref_area_gray);
	ret |= CVI_IVE_CompareIveImage(&astDst[0], &ref_area_rgb);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, "sample_Resize_Area_rgb.rgb",
				 &astDst[0]);
		CVI_IVE_WriteImg(handle, "sample_Resize_Area_gray.yuv",
				 &astDst[1]);
		CVI_IVE_WriteImg(handle, "sample_Resize_Area_240p.rgb",
				 &astDst[2]);
	}

	CVI_SYS_FreeI(handle, &astSrc[0]);
	CVI_SYS_FreeI(handle, &astSrc[1]);
	CVI_SYS_FreeI(handle, &astSrc[2]);
	CVI_SYS_FreeI(handle, &astDst[0]);
	CVI_SYS_FreeI(handle, &astDst[1]);
	CVI_SYS_FreeI(handle, &astDst[2]);
	CVI_SYS_FreeI(handle, &ref_area_240p);
	CVI_SYS_FreeI(handle, &ref_area_gray);
	CVI_SYS_FreeI(handle, &ref_area_rgb);
	CVI_SYS_FreeI(handle, &ref_bilinear_240p);
	CVI_SYS_FreeI(handle, &ref_bilinear_gray);
	CVI_SYS_FreeI(handle, &ref_bilinear_rgb);
	CVI_SYS_FreeM(handle, &(stCtrl.stMem));
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

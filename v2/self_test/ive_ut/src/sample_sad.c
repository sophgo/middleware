#include "cvi_ive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_sad(int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	char input_data1[64];
	char input_data2[64];
	char output_data1[64];
	char output_data2[64];
	char output_data3[64];
	char output_data4[64];
	char output_data5[64];
	char output_data6[64];
	char output_data7[64];
	char output_data8[64];
	char output_data9[64];
	char output_data10[64];
	char output_data11[64];
	char output_data12[64];

	strcpy(input_data1, "res/ive/00_352x288_y.yuv");
	strcpy(input_data2, "res/ive/bin_352x288_y.yuv");
	strcpy(output_data1, "res/ive/result/sample_Sad_sad_mode0_out0.bin");
	strcpy(output_data2, "res/ive/result/sample_Sad_sad_mode0_out1.bin");
	strcpy(output_data3, "res/ive/result/sample_Sad_sad_mode1_out0.bin");
	strcpy(output_data4, "res/ive/result/sample_Sad_sad_mode1_out1.bin");
	strcpy(output_data5, "res/ive/result/sample_Sad_sad_mode2_out0.bin");
	strcpy(output_data6, "res/ive/result/sample_Sad_sad_mode2_out1.bin");
	strcpy(output_data7, "res/ive/result/sample_Sad_thr_mode0_out0.bin");
	strcpy(output_data8, "res/ive/result/sample_Sad_thr_mode0_out1.bin");
	strcpy(output_data9, "res/ive/result/sample_Sad_thr_mode1_out0.bin");
	strcpy(output_data10, "res/ive/result/sample_Sad_thr_mode1_out1.bin");
	strcpy(output_data11, "res/ive/result/sample_Sad_thr_mode2_out0.bin");
	strcpy(output_data12, "res/ive/result/sample_Sad_thr_mode2_out1.bin");

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src1, src2;

	CVI_IVE_ReadRawImage(handle, &src1, input_data1,
			       IVE_IMAGE_TYPE_U8C1, 352, 288);
	CVI_IVE_ReadRawImage(handle, &src2, input_data2,
			       IVE_IMAGE_TYPE_U8C1, 352, 288);

	// Create ref image.
	IVE_IMAGE_S ref_sad_mode0_out0, ref_sad_mode0_out1, ref_sad_mode1_out0,
		ref_sad_mode1_out1, ref_sad_mode2_out0, ref_sad_mode2_out1;
	IVE_IMAGE_S ref_thr_mode0_out0, ref_thr_mode0_out1, ref_thr_mode1_out0,
		ref_thr_mode1_out1, ref_thr_mode2_out0, ref_thr_mode2_out1;

	CVI_IVE_ReadRawImage(handle, &ref_sad_mode0_out0, output_data1,
			       IVE_IMAGE_TYPE_U16C1, src1.u32Width,
			       src1.u32Height);
	CVI_IVE_ReadRawImage(handle, &ref_sad_mode0_out1, output_data2,
			       IVE_IMAGE_TYPE_U8C1, src1.u32Width,
			       src1.u32Height);
	CVI_IVE_ReadRawImage(handle, &ref_sad_mode1_out0, output_data3,
			       IVE_IMAGE_TYPE_U16C1, src1.u32Width,
			       src1.u32Height);
	CVI_IVE_ReadRawImage(handle, &ref_sad_mode1_out1, output_data4,
			       IVE_IMAGE_TYPE_U8C1, src1.u32Width,
			       src1.u32Height);
	CVI_IVE_ReadRawImage(handle, &ref_sad_mode2_out0, output_data5,
			       IVE_IMAGE_TYPE_U16C1, src1.u32Width,
			       src1.u32Height);
	CVI_IVE_ReadRawImage(handle, &ref_sad_mode2_out1, output_data6,
			       IVE_IMAGE_TYPE_U8C1, src1.u32Width,
			       src1.u32Height);
	CVI_IVE_ReadRawImage(handle, &ref_thr_mode0_out0,
			       output_data7, IVE_IMAGE_TYPE_U8C1,
			       src1.u32Width, src1.u32Height);
	CVI_IVE_ReadRawImage(handle, &ref_thr_mode0_out1,
			       output_data8, IVE_IMAGE_TYPE_U8C1,
			       src1.u32Width, src1.u32Height);
	CVI_IVE_ReadRawImage(handle, &ref_thr_mode1_out0,
			       output_data9, IVE_IMAGE_TYPE_U8C1,
			       src1.u32Width, src1.u32Height);
	CVI_IVE_ReadRawImage(handle, &ref_thr_mode1_out1,
			       output_data10, IVE_IMAGE_TYPE_U8C1,
			       src1.u32Width, src1.u32Height);
	CVI_IVE_ReadRawImage(handle, &ref_thr_mode2_out0,
			       output_data11, IVE_IMAGE_TYPE_U8C1,
			       src1.u32Width, src1.u32Height);
	CVI_IVE_ReadRawImage(handle, &ref_thr_mode2_out1,
			       output_data12, IVE_IMAGE_TYPE_U8C1,
			       src1.u32Width, src1.u32Height);

	// Create dst image.
	IVE_DST_IMAGE_S stSad_8, stSad_16, stDst2, stThr;

	CVI_IVE_CreateImage(handle, &stSad_8, IVE_IMAGE_TYPE_U8C1,
			    src1.u32Width, src1.u32Height);
	CVI_IVE_CreateImage(handle, &stSad_16, IVE_IMAGE_TYPE_U16C1,
			    src1.u32Width, src1.u32Height);
	CVI_IVE_CreateImage(handle, &stDst2, IVE_IMAGE_TYPE_U8C1, src1.u32Width,
			    src1.u32Height);
	CVI_IVE_CreateImage(handle, &stThr, IVE_IMAGE_TYPE_U8C1, src1.u32Width,
			    src1.u32Height);

	// Config Setting.
	IVE_SAD_CTRL_S iveSadCtrl;

	iveSadCtrl.u16Thr =
		0x800; // Treshold value for thresholding an U16 image into an U8 image.
	iveSadCtrl.u8MinVal = 2;
	iveSadCtrl.u8MaxVal = 30;
	iveSadCtrl.enMode = IVE_SAD_MODE_MB_4X4;

	// Run HW IVE.
	printf("Run HW IVE SAD 16BIT MB_4X4.\n");
	iveSadCtrl.enOutCtrl = IVE_SAD_OUT_CTRL_16BIT_BOTH;
	CVI_IVE_SAD(handle, &src1, &src2, &stSad_16, &stThr, &iveSadCtrl,
		    bInstant);
	ret |= CVI_IVE_CompareSADImage(&stSad_16, &ref_sad_mode0_out0,
				       IVE_SAD_MODE_MB_4X4, CVI_FALSE);
	ret |= CVI_IVE_CompareSADImage(&stThr, &ref_thr_mode0_out0,
				       IVE_SAD_MODE_MB_4X4, CVI_FALSE);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, "sample_Sad_sad_mode0_out0.bin",
				 &stSad_16);
		CVI_IVE_WriteImg(handle, "sample_Sad_thr_mode0_out0.bin",
				 &stThr);
	}

	printf("Run HW IVE SAD 16BIT MB_8X8.\n");
	iveSadCtrl.enMode = IVE_SAD_MODE_MB_8X8;
	CVI_IVE_SAD(handle, &src1, &src2, &stSad_16, &stThr, &iveSadCtrl,
		    bInstant);
	ret |= CVI_IVE_CompareSADImage(&stSad_16, &ref_sad_mode1_out0,
				       IVE_SAD_MODE_MB_8X8, CVI_FALSE);
	ret |= CVI_IVE_CompareSADImage(&stThr, &ref_thr_mode1_out0,
				       IVE_SAD_MODE_MB_8X8, CVI_FALSE);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, "sample_Sad_sad_mode1_out0.bin",
				 &stSad_16);
		CVI_IVE_WriteImg(handle, "sample_Sad_thr_mode1_out0.bin",
				 &stThr);
	}
	printf("Run HW IVE SAD 16BIT MB_16X16.\n");
	iveSadCtrl.enMode = IVE_SAD_MODE_MB_16X16;
	CVI_IVE_SAD(handle, &src1, &src2, &stSad_16, &stThr, &iveSadCtrl,
		    bInstant);
	ret |= CVI_IVE_CompareSADImage(&stSad_16, &ref_sad_mode2_out0,
				       IVE_SAD_MODE_MB_16X16, CVI_FALSE);
	ret |= CVI_IVE_CompareSADImage(&stThr, &ref_thr_mode2_out0,
				       IVE_SAD_MODE_MB_16X16, CVI_FALSE);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, "sample_Sad_sad_mode2_out0.bin",
				 &stSad_16);
		CVI_IVE_WriteImg(handle, "sample_Sad_thr_mode2_out0.bin",
				 &stThr);
	}

	// Config Setting.
	iveSadCtrl.enOutCtrl = IVE_SAD_OUT_CTRL_8BIT_BOTH;
	IVE_DMA_CTRL_S stDMACtrl;

	stDMACtrl.enMode = IVE_DMA_MODE_INTERVAL_COPY;
	stDMACtrl.u8ElemSize = 1;
	stDMACtrl.u8HorSegSize = 2;
	stDMACtrl.u8VerSegRows = 1;
	stDMACtrl.u64Val = 0;

	IVE_DATA_S stdmaSrc1;

	stdmaSrc1.u64VirAddr = stDst2.u64VirAddr[0];
	stdmaSrc1.u64PhyAddr = stDst2.u64PhyAddr[0];
	stdmaSrc1.u32Stride = stDst2.u32Stride[0];
	stdmaSrc1.u32Width = stDst2.u32Width;
	stdmaSrc1.u32Height = stDst2.u32Height;

	IVE_DATA_S stdmaDst;

	stdmaDst.u64VirAddr = stSad_8.u64VirAddr[0];
	stdmaDst.u64PhyAddr = stSad_8.u64PhyAddr[0];
	stdmaDst.u32Stride = stSad_8.u32Stride[0] / 2;
	stdmaDst.u32Width = stSad_8.u32Width;
	stdmaDst.u32Height = stSad_8.u32Height;

	printf("Run HW IVE SAD 8BIT MB_4X4.\n");
	iveSadCtrl.enMode = IVE_SAD_MODE_MB_4X4;
	CVI_IVE_SAD(handle, &src1, &src2, &stDst2, &stThr, &iveSadCtrl,
		    bInstant);
	CVI_IVE_DMA(handle, &stdmaSrc1, &stdmaDst, &stDMACtrl, bInstant);
	ret |= CVI_IVE_CompareSADImage(&stSad_8, &ref_sad_mode0_out1,
				       IVE_SAD_MODE_MB_4X4, CVI_TRUE);
	ret |= CVI_IVE_CompareSADImage(&stThr, &ref_thr_mode0_out1,
				       IVE_SAD_MODE_MB_4X4, CVI_FALSE);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, "sample_Sad_sad_mode0_out1.bin",
				 &stSad_8);
		CVI_IVE_WriteImg(handle, "sample_Sad_thr_mode0_out1.bin",
				 &stThr);
	}

	printf("Run HW IVE SAD 8BIT MB_8X8.\n");
	iveSadCtrl.enMode = IVE_SAD_MODE_MB_8X8;
	CVI_IVE_SAD(handle, &src1, &src2, &stDst2, &stThr, &iveSadCtrl,
		    bInstant);
	CVI_IVE_DMA(handle, &stdmaSrc1, &stdmaDst, &stDMACtrl, bInstant);
	ret |= CVI_IVE_CompareSADImage(&stSad_8, &ref_sad_mode1_out1,
				       IVE_SAD_MODE_MB_8X8, CVI_TRUE);
	ret |= CVI_IVE_CompareSADImage(&stThr, &ref_thr_mode1_out1,
				       IVE_SAD_MODE_MB_8X8, CVI_FALSE);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, "sample_Sad_sad_mode1_out1.bin",
				 &stSad_8);
		CVI_IVE_WriteImg(handle, "sample_Sad_thr_mode1_out1.bin",
				 &stThr);
	}

	printf("Run HW IVE SAD 8BIT MB_16X16.\n");
	iveSadCtrl.enMode = IVE_SAD_MODE_MB_16X16;
	CVI_IVE_SAD(handle, &src1, &src2, &stDst2, &stThr, &iveSadCtrl,
		    bInstant);
	CVI_IVE_DMA(handle, &stdmaSrc1, &stdmaDst, &stDMACtrl, bInstant);
	ret |= CVI_IVE_CompareSADImage(&stSad_8, &ref_sad_mode2_out1,
				       IVE_SAD_MODE_MB_16X16, CVI_TRUE);
	ret |= CVI_IVE_CompareSADImage(&stThr, &ref_thr_mode2_out1,
				       IVE_SAD_MODE_MB_16X16, CVI_FALSE);
	if (bWrite) {
		CVI_IVE_WriteImg(handle, "sample_Sad_sad_mode2_out1.bin",
				 &stSad_8);
		CVI_IVE_WriteImg(handle, "sample_Sad_thr_mode2_out1.bin",
				 &stThr);
	}

	CVI_SYS_FreeI(handle, &src1);
	CVI_SYS_FreeI(handle, &src2);
	CVI_SYS_FreeI(handle, &stSad_8);
	CVI_SYS_FreeI(handle, &stSad_16);
	CVI_SYS_FreeI(handle, &stDst2);
	CVI_SYS_FreeI(handle, &stThr);
	CVI_SYS_FreeI(handle, &ref_sad_mode0_out0);
	CVI_SYS_FreeI(handle, &ref_sad_mode0_out1);
	CVI_SYS_FreeI(handle, &ref_sad_mode1_out0);
	CVI_SYS_FreeI(handle, &ref_sad_mode1_out1);
	CVI_SYS_FreeI(handle, &ref_sad_mode2_out0);
	CVI_SYS_FreeI(handle, &ref_sad_mode2_out1);
	CVI_SYS_FreeI(handle, &ref_thr_mode0_out0);
	CVI_SYS_FreeI(handle, &ref_thr_mode0_out1);
	CVI_SYS_FreeI(handle, &ref_thr_mode1_out0);
	CVI_SYS_FreeI(handle, &ref_thr_mode1_out1);
	CVI_SYS_FreeI(handle, &ref_thr_mode2_out0);
	CVI_SYS_FreeI(handle, &ref_thr_mode2_out1);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

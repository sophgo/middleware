#include "cvi_ive.h"
// #include "cvi_pattern.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_dma(int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	char input_data[64];
	char output_data1[64];
	char output_data2[64];
	char output_data3[64];
	char output_data4[64];


	strcpy(input_data, "res/ive/00_352x288_y.yuv");
	strcpy(output_data1, "res/ive/result/sample_DMA_Direct.bin");
	strcpy(output_data2, "res/ive/result/sample_DMA_Interval.bin");
	strcpy(output_data3, "res/ive/result/sample_DMA_Set3Byte.bin");
	strcpy(output_data4, "res/ive/result/sample_DMA_Set8Byte.bin");

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src;

	CVI_IVE_ReadRawImage(handle, &src, input_data,
			       IVE_IMAGE_TYPE_U8C1, 352, 288);

	IVE_DATA_S src_data;

	src_data.u64PhyAddr = src.u64PhyAddr[0];
	src_data.u64VirAddr = src.u64VirAddr[0];
	src_data.u32Stride = src.u32Stride[0];
	src_data.u32Width = src.u32Width;
	src_data.u32Height = src.u32Height;

	// Create ref image.
	IVE_DST_DATA_S ref_dir, ref_inter, ref_3bit, ref_8bit;

	CVI_IVE_ReadData(handle, &ref_dir, output_data1, src.u32Width,
			      src.u32Height);
	CVI_IVE_ReadData(handle, &ref_inter, output_data2,
			      src.u32Width / 4, src.u32Height);
	CVI_IVE_ReadData(handle, &ref_3bit, output_data3,
			      src.u32Width, src.u32Height);
	CVI_IVE_ReadData(handle, &ref_8bit, output_data4,
			      src.u32Width, src.u32Height);

	// Create dst image.
	IVE_DST_DATA_S dst_data;

	CVI_IVE_CreateDataInfo(handle, &dst_data, src_data.u32Width,
			       src_data.u32Height);

	// Config Setting.
	IVE_DMA_CTRL_S iveDmaCtrl;

	iveDmaCtrl.enMode = IVE_DMA_MODE_DIRECT_COPY;

	// Run HW IVE.
	printf("Run HW IVE DMA Direct Copy.\n");
	CVI_IVE_DMA(handle, &src_data, &dst_data, &iveDmaCtrl, bInstant);
	ret |= CVI_IVE_CompareIveData(&dst_data, &ref_dir);
	if (bWrite) {
		CVI_IVE_WriteData(handle, "sample_DMA_Direct.bin", &dst_data);
	}

	IVE_DST_DATA_S dst_data1;

	printf("Run HW IVE DMA Interval Copy.\n");
	iveDmaCtrl.enMode = IVE_DMA_MODE_INTERVAL_COPY;
	iveDmaCtrl.u8ElemSize = 1;
	iveDmaCtrl.u8HorSegSize = 4;
	iveDmaCtrl.u8VerSegRows = 1;
	int IntervalWidth = (src_data.u32Width / iveDmaCtrl.u8HorSegSize) *
			    iveDmaCtrl.u8ElemSize;
	int IntervalHeight = src_data.u32Height / iveDmaCtrl.u8VerSegRows;

	CVI_IVE_CreateDataInfo(handle, &dst_data1, IntervalWidth,
			       IntervalHeight);

	CVI_IVE_DMA(handle, &src_data, &dst_data1, &iveDmaCtrl, 1);
	ret |= CVI_IVE_CompareIveData(&dst_data1, &ref_inter);
	if (bWrite) {
		CVI_IVE_WriteData(handle, "sample_DMA_Interval.bin",
				  &dst_data1);
	}

	printf("Run HW IVE DMA 3BYTE Copy.\n");
	iveDmaCtrl.enMode = IVE_DMA_MODE_SET_3BYTE;
	iveDmaCtrl.u64Val = 0x123456789abcdef;
	iveDmaCtrl.u8HorSegSize = 1;
	CVI_IVE_DMA(handle, &src_data, &dst_data, &iveDmaCtrl, bInstant);
	ret |= CVI_IVE_CompareIveData(&dst_data, &ref_3bit);
	if (bWrite) {
		CVI_IVE_WriteData(handle, "sample_DMA_Set3Byte.bin", &dst_data);
	}

	printf("Run HW IVE DMA 8BYTE Copy.\n");
	iveDmaCtrl.enMode = IVE_DMA_MODE_SET_8BYTE;
	iveDmaCtrl.u64Val = 0x123456789abcdef;
	CVI_IVE_DMA(handle, &src_data, &dst_data, &iveDmaCtrl, bInstant);
	ret |= CVI_IVE_CompareIveData(&dst_data, &ref_8bit);
	if (bWrite) {
		CVI_IVE_WriteData(handle, "sample_DMA_Set8Byte.bin", &dst_data);
	}

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeD(handle, &dst_data);
	CVI_SYS_FreeD(handle, &dst_data1);
	CVI_SYS_FreeD(handle, &ref_dir);
	CVI_SYS_FreeD(handle, &ref_inter);
	CVI_SYS_FreeD(handle, &ref_3bit);
	CVI_SYS_FreeD(handle, &ref_8bit);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

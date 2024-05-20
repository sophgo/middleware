#include "cvi_ive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_cannyhysedge(int bTileMode, int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	int input_w, input_h;
	char input_data[64];
	char output_data_3x3[64];
	char output_data_Mem3x3[64];
	char output_data_5x5[64];
	char output_data_Mem5x5[64];
	char output_file_3x3[64];
	char output_file_Mem3x3[64];
	char output_file_5x5[64];
	char output_file_Mem5x5[64];

	if (bTileMode) {
		input_w = 640;
		input_h = 480;
		strcpy(input_data, "res/ive/sky_640x480.yuv");
		strcpy(output_data_3x3, "res/ive/result/sample_tile_CannyHysEdge_3x3.yuv");
		strcpy(output_data_Mem3x3, "res/ive/result/sample_tile_Canny_Mem_3x3_0.bin");
		strcpy(output_data_5x5, "res/ive/result/sample_tile_CannyHysEdge_5x5.yuv");
		strcpy(output_data_Mem5x5, "res/ive/result/sample_tile_Canny_Mem_5x5_0.bin");
		strcpy(output_file_3x3, "sample_tile_CannyHysEdge_3x3.yuv");
		strcpy(output_file_Mem3x3, "sample_tile_Canny_Mem_3x3_0.bin");
		strcpy(output_file_5x5, "sample_tile_CannyHysEdge_5x5.yuv");
		strcpy(output_file_Mem5x5, "sample_tile_Canny_Mem_5x5_0.bin");
	} else {
		input_w = 352;
		input_h = 288;
		strcpy(input_data, "res/ive/00_352x288_y.yuv");
		strcpy(output_data_3x3, "res/ive/result/sample_CannyHysEdge_3x3.yuv");
		strcpy(output_data_Mem3x3, "res/ive/result/sample_Canny_Mem_3x3_0.bin");
		strcpy(output_data_5x5, "res/ive/result/sample_CannyHysEdge_5x5.yuv");
		strcpy(output_data_Mem5x5, "res/ive/result/sample_Canny_Mem_5x5_0.bin");
		strcpy(output_file_3x3, "sample_CannyHysEdge_3x3.yuv");
		strcpy(output_file_Mem3x3, "sample_Canny_Mem_3x3_0.bin");
		strcpy(output_file_5x5, "sample_CannyHysEdge_5x5.yuv");
		strcpy(output_file_Mem5x5, "sample_Canny_Mem_5x5_0.bin");
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
	IVE_IMAGE_S ref_3x3, ref_5x5;
	IVE_MEM_INFO_S ref_mem3x3, ref_mem5x5;

	CVI_IVE_ReadMem(handle, &ref_mem3x3, output_data_Mem3x3,
			     input_w * input_h * (sizeof(IVE_POINT_U16_S)) +
				     sizeof(IVE_CANNY_STACK_SIZE_S));
	CVI_IVE_ReadRawImage(handle, &ref_3x3, output_data_3x3,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	CVI_IVE_ReadMem(handle, &ref_mem5x5, output_data_Mem5x5,
			     input_w * input_h * (sizeof(IVE_POINT_U16_S)) +
				     sizeof(IVE_CANNY_STACK_SIZE_S));
	CVI_IVE_ReadRawImage(handle, &ref_5x5, output_data_5x5,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	// Create dst image.
	IVE_DST_IMAGE_S stEdge;

	CVI_IVE_CreateImage(handle, &stEdge, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);

	IVE_MEM_INFO_S stStack;

	CVI_IVE_CreateMemInfo(handle, &stStack,
			      input_w * input_h * (sizeof(IVE_POINT_U16_S)) +
				      sizeof(IVE_CANNY_STACK_SIZE_S));

	// Config Setting.
	IVE_CANNY_HYS_EDGE_CTRL_S stCannyHysEdgeCtrl;

	CVI_IVE_CreateMemInfo(handle, &(stCannyHysEdgeCtrl.stMem),
			      input_w * input_h * 4 *
				      (sizeof(CVI_U16) + sizeof(CVI_U8)));
	stCannyHysEdgeCtrl.u16LowThr = 42;
	stCannyHysEdgeCtrl.u16HighThr = 3 * stCannyHysEdgeCtrl.u16LowThr;
	memset((void *)(uintptr_t)((IVE_MEM_INFO_S)stCannyHysEdgeCtrl.stMem)
		       .u64VirAddr,
	       0, ((IVE_MEM_INFO_S)stCannyHysEdgeCtrl.stMem).u32Size);
	memcpy(stCannyHysEdgeCtrl.as8Mask, arr3by3, 5 * 5 * sizeof(CVI_S8));

	// Run HW IVE.
	printf("Run HW IVE CannyEdge 3x3.\n");
	CVI_IVE_CannyHysEdge(handle, &src, &stEdge, &stStack,
			     &stCannyHysEdgeCtrl, bInstant);
	ret |= CVI_IVE_CompareIveMem(&stStack, &ref_mem3x3);
	ret |= CVI_IVE_CompareIveImage(&stEdge, &ref_3x3);
	if (bWrite) {
		CVI_IVE_WriteMem(handle, output_file_Mem3x3, &stStack);
		CVI_IVE_WriteImg(handle, output_file_3x3, &stEdge);
	}

	printf("Run HW IVE CannyEdge 5x5.\n");
	stCannyHysEdgeCtrl.u16LowThr = 108;
	stCannyHysEdgeCtrl.u16HighThr = 3 * stCannyHysEdgeCtrl.u16LowThr;
	memcpy(stCannyHysEdgeCtrl.as8Mask, arr5by5, 5 * 5 * sizeof(CVI_S8));
	CVI_IVE_CannyHysEdge(handle, &src, &stEdge, &stStack,
			     &stCannyHysEdgeCtrl, bInstant);
	ret |= CVI_IVE_CompareIveMem(&stStack, &ref_mem5x5);
	ret |= CVI_IVE_CompareIveImage(&stEdge, &ref_5x5);
	if (bWrite) {
		CVI_IVE_WriteMem(handle, output_file_Mem5x5, &stStack);
		CVI_IVE_WriteImg(handle, output_file_5x5, &stEdge);
	}

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeM(handle, &ref_mem3x3);
	CVI_SYS_FreeI(handle, &ref_3x3);
	CVI_SYS_FreeM(handle, &ref_mem5x5);
	CVI_SYS_FreeI(handle, &ref_5x5);
	CVI_SYS_FreeI(handle, &stEdge);
	CVI_SYS_FreeM(handle, &stStack);
	CVI_SYS_FreeM(handle, &stCannyHysEdgeCtrl.stMem);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

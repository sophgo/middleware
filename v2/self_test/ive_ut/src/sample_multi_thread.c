#include "cvi_ive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include "sample_ive.h"
static CVI_S32 ret = CVI_SUCCESS;

void *ive_thread_func0(void *arg)
{
	multi_thread_param *param = (multi_thread_param *)arg;
	int bTileMode, bWrite, bInstant, loop;
	int input_w, input_h, k;
	char input_data[64];
	char output_data[64];
	char output_file[64];

	bTileMode = param->bTileMode;
	bWrite = param->bWrite;
	bInstant = param->bInstant;
	loop = param->loop;

	printf("bInstant:%d\n", bInstant);
	if (bTileMode) {
		strcpy(input_data, "res/ive/sky_640x480.yuv");
		strcpy(output_data, "res/ive/result/sample_tile_Add.yuv");
		input_w = 640;
		input_h = 480;
		strcpy(output_file, "sample_tile_Add.yuv");
	} else {
		strcpy(input_data, "res/ive/00_352x288_y.yuv");
		strcpy(output_data, "res/ive/result/sample_Add.yuv");
		input_w = 352;
		input_h = 288;
		strcpy(output_file, "sample_Add.yuv");
	}

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src1, src2;

	CVI_IVE_ReadRawImage(handle, &src1, input_data, IVE_IMAGE_TYPE_U8C1,
				input_w, input_h);
	CVI_IVE_CreateImage(handle, &src2, IVE_IMAGE_TYPE_U8C1, input_w,
				input_h);
	memset((void *)(uintptr_t)src2.u64VirAddr[0], 255,
		src1.u32Stride[0] * input_h);
	for (int j = input_h / 10; j < input_h * 9 / 10; j++) {
		for (int i = input_w / 10; i < input_w * 9 / 10; i++) {
			((char *)(uintptr_t)src2
				.u64VirAddr[0])[i + j * src1.u32Stride[0]] = 0;
		}
	}

	// Create ref image.
	IVE_IMAGE_S ref_add;

	CVI_IVE_ReadRawImage(handle, &ref_add, output_data,
				IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	// Create dst image.
	IVE_DST_IMAGE_S dst;

	CVI_IVE_CreateImage(handle, &dst, IVE_IMAGE_TYPE_U8C1, input_w,
				input_h);

	// Config Setting.
	IVE_ADD_CTRL_S iveAddCtrl;

	iveAddCtrl.u0q16X = 65535;
	iveAddCtrl.u0q16Y = 65535;

	// Run IVE
	for (k = 0; k < loop; k++) {
		printf("Run HW IVE ADD.\n");
		CVI_IVE_Add(handle, &src1, &src2, &dst, &iveAddCtrl, bInstant);
		ret |= CVI_IVE_CompareIveImage(&dst, &ref_add);
		if (bWrite)
			CVI_IVE_WriteImg(handle, output_file, &dst);
	}
	CVI_SYS_FreeI(handle, &src1);
	CVI_SYS_FreeI(handle, &src2);
	CVI_SYS_FreeI(handle, &dst);
	CVI_SYS_FreeI(handle, &ref_add);
	CVI_IVE_DestroyHandle(handle);

	pthread_exit(0);
}
void *ive_thread_func1(void *arg)
{
	multi_thread_param *param = (multi_thread_param *)arg;
	int bTileMode, bWrite, bInstant, loop, k;
	int input_w, input_h;
	char input_data[64];
	char output_data[64];
	char output_file[64];

	bTileMode = param->bTileMode;
	bWrite = param->bWrite;
	bInstant = param->bInstant;
	loop = param->loop;

	printf("bInstant:%d\n", bInstant);
	if (bTileMode) {
		strcpy(input_data, "res/ive/sky_640x480.yuv");
		strcpy(output_data, "res/ive/result/sample_tile_Map.yuv");
		input_w = 640;
		input_h = 480;
		strcpy(output_file, "sample_tile_Map.yuv");
	} else {
		strcpy(input_data, "res/ive/00_352x288_y.yuv");
		strcpy(output_data, "res/ive/result/sample_Map.yuv");
		input_w = 352;
		input_h = 288;
		strcpy(output_file, "sample_Map.yuv");
	}

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src;

	CVI_IVE_ReadRawImage(handle, &src, input_data,
					IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	// Create ref image.
	IVE_IMAGE_S ref_map;

	CVI_IVE_ReadRawImage(handle, &ref_map, output_data,
			       IVE_IMAGE_TYPE_U8C1, input_w,
			       input_h);

	// Create dst image.
	IVE_DST_IMAGE_S dst_map;

	CVI_IVE_CreateImage(handle, &dst_map, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);

	// Config Setting.
	CVI_U32 dstTblByteSize = 256;
	IVE_MEM_INFO_S dstTbl;

	CVI_IVE_CreateMemInfo(handle, &dstTbl, dstTblByteSize);
	CVI_U8 FixMap[256] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x03,
		0x03, 0x04, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
		0x0C, 0x0D, 0x0F, 0x10, 0x11, 0x12, 0x14, 0x15, 0x17, 0x18,
		0x1A, 0x1B, 0x1D, 0x1E, 0x20, 0x21, 0x23, 0x24, 0x26, 0x27,
		0x29, 0x2A, 0x2C, 0x2D, 0x2F, 0x31, 0x32, 0x34, 0x35, 0x37,
		0x38, 0x3A, 0x3B, 0x3D, 0x3E, 0x40, 0x41, 0x43, 0x44, 0x45,
		0x47, 0x48, 0x4A, 0x4B, 0x4D, 0x4E, 0x50, 0x51, 0x52, 0x54,
		0x55, 0x56, 0x58, 0x59, 0x5A, 0x5B, 0x5D, 0x5E, 0x5F, 0x60,
		0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x69, 0x6A, 0x6B, 0x6D,
		0x6E, 0x70, 0x71, 0x73, 0x75, 0x76, 0x78, 0x7A, 0x7B, 0x7D,
		0x7E, 0x80, 0x81, 0x83, 0x84, 0x86, 0x87, 0x88, 0x89, 0x8B,
		0x8C, 0x8D, 0x8E, 0x90, 0x92, 0x94, 0x97, 0x9A, 0x9C, 0x9E,
		0xA1, 0xA3, 0xA5, 0xA6, 0xA7, 0xA9, 0xAA, 0xAB, 0xAC, 0xAC,
		0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB3, 0xB4, 0xB5, 0xB7, 0xB9,
		0xBB, 0xBD, 0xBF, 0xC1, 0xC4, 0xC7, 0xCC, 0xD1, 0xD5, 0xDA,
		0xDE, 0xE0, 0xE2, 0xE3, 0xE4, 0xE5, 0xE5, 0xE6, 0xE6, 0xE6,
		0xE6, 0xE6, 0xE7, 0xE7, 0xE7, 0xE8, 0xE8, 0xE9, 0xEA, 0xEC,
		0xED, 0xEE, 0xF0, 0xF2, 0xF4, 0xF5, 0xF7, 0xF8, 0xFA, 0xFB,
		0xFD, 0xFE, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	};

	for (CVI_U32 i = 0; i < dstTblByteSize; i++) {
		((char *)(uintptr_t)dstTbl.u64VirAddr)[i] = FixMap[i];
	}

	IVE_MAP_CTRL_S Ctrl;

	Ctrl.enMode = IVE_MAP_MODE_U8;

	// Run IVE
	for (k = 0; k < loop; k++) {
		printf("Run HW IVE Map MODE_U8.\n");
		CVI_IVE_Map(handle, &src, &dstTbl, &dst_map, &Ctrl, bInstant);
		if (!bTileMode) {
			ret |= CVI_IVE_CompareIveImage(&dst_map, &ref_map);
		}
		if (bWrite)
			CVI_IVE_WriteImg(handle, output_file, &dst_map);
	}
	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeI(handle, &dst_map);
	CVI_SYS_FreeI(handle, &ref_map);
	CVI_SYS_FreeM(handle, &dstTbl);
	CVI_IVE_DestroyHandle(handle);

	pthread_exit(0);

}
void *ive_thread_func2(void *arg)
{
	multi_thread_param *param = (multi_thread_param *)arg;
	int bTileMode, bWrite, bInstant, loop;
	char input_data[64];
	char output_data1[64];


	bTileMode = param->bTileMode;
	bWrite = param->bWrite;
	bInstant = param->bInstant;
	loop = param->loop;

	UNUSED(bTileMode);
	strcpy(input_data, "res/ive/00_352x288_y.yuv");
	strcpy(output_data1, "res/ive/result/sample_Integ_Combine.yuv");


	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src;

	CVI_IVE_ReadRawImage(handle, &src, input_data,
			       IVE_IMAGE_TYPE_U8C1, 352, 288);

	// Create ref image.
	IVE_MEM_INFO_S ref_com;

	CVI_IVE_ReadMem(handle, &ref_com, output_data1,
			     src.u32Width * src.u32Height *
				     sizeof(CVI_U64));


	// Create dst image.
	IVE_DST_MEM_INFO_S dst_integ64;

	CVI_IVE_CreateMemInfo(handle, &dst_integ64,
			      src.u32Width * src.u32Height *
				      sizeof(CVI_U64));

	// Config Setting.
	IVE_INTEG_CTRL_S pstIntegCtrl;

	pstIntegCtrl.enOutCtrl = IVE_INTEG_OUT_CTRL_COMBINE;

	// Run IVE
	for (int i = 0; i < loop; i++) {
		printf("Run HW IVE Integral COMBINE.\n");
		CVI_IVE_Integ(handle, &src, &dst_integ64, &pstIntegCtrl, bInstant);
		ret |= CVI_IVE_CompareIveMem(&dst_integ64, &ref_com);
		if (bWrite)
			CVI_IVE_WriteMem(handle, "sample_Integ_Combine.yuv",
					&dst_integ64);
	}

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeM(handle, &ref_com);
	CVI_SYS_FreeM(handle, &dst_integ64);
	CVI_IVE_DestroyHandle(handle);

	pthread_exit(0);
}
void *ive_thread_func3(void *arg)
{
	multi_thread_param *param = (multi_thread_param *)arg;
	int bTileMode, bWrite, bInstant;
	char input_data[64];
	char output_data1[64];
	int loop;


	bTileMode = param->bTileMode;
	bWrite = param->bWrite;
	bInstant = param->bInstant;
	loop = param->loop;

	UNUSED(bTileMode);
	strcpy(input_data, "res/ive/00_352x288_y.yuv");
	strcpy(output_data1, "res/ive/result/sample_DMA_Direct.bin");


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
	IVE_DST_DATA_S ref_dir;

	CVI_IVE_ReadData(handle, &ref_dir, output_data1, src.u32Width,
			      src.u32Height);

	// Create dst image.
	IVE_DST_DATA_S dst_data;

	CVI_IVE_CreateDataInfo(handle, &dst_data, src_data.u32Width,
			       src_data.u32Height);

	// Config Setting.
	IVE_DMA_CTRL_S iveDmaCtrl;

	iveDmaCtrl.enMode = IVE_DMA_MODE_DIRECT_COPY;

	// Run HW IVE.
	for (int i = 0; i < loop; i++) {
		printf("Run HW IVE DMA Direct Copy.\n");
		CVI_IVE_DMA(handle, &src_data, &dst_data, &iveDmaCtrl, bInstant);
		ret |= CVI_IVE_CompareIveData(&dst_data, &ref_dir);
		if (bWrite) {
			CVI_IVE_WriteData(handle, "sample_DMA_Direct.bin", &dst_data);
		}
	}


	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeD(handle, &dst_data);
	CVI_SYS_FreeD(handle, &ref_dir);
	CVI_IVE_DestroyHandle(handle);

	pthread_exit(0);
}
void *ive_thread_func4(void *arg)
{
	multi_thread_param *param = (multi_thread_param *)arg;
	int bTileMode, bWrite, bInstant;
	char input_data[64];
	char output_data[64];
	int loop;


	bTileMode = param->bTileMode;
	bWrite = param->bWrite;
	bInstant = param->bInstant;
	loop = param->loop;

	UNUSED(bTileMode);
	strcpy(input_data, "res/ive/00_352x288_y.yuv");
	strcpy(output_data, "res/ive/result/sample_Hist.bin");

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src;

	CVI_IVE_ReadRawImage(handle, &src, input_data,
			       IVE_IMAGE_TYPE_U8C1, 352, 288);

	// Create ref image.
	IVE_MEM_INFO_S ref_hist;

	CVI_IVE_ReadMem(handle, &ref_hist, output_data,
			     256 * sizeof(CVI_U32));

	// Create dst mem info.
	IVE_DST_MEM_INFO_S dst_hist;

	CVI_U32 dstHistSize = 256 * sizeof(CVI_U32);

	CVI_IVE_CreateMemInfo(handle, &dst_hist, dstHistSize);

	// Config Setting.

	// Run IVE
	for (int i = 0; i < loop; i++) {
		printf("Run HW IVE Hist.\n");
		CVI_IVE_Hist(handle, &src, &dst_hist, bInstant);
		ret |= CVI_IVE_CompareIveMem(&dst_hist, &ref_hist);
		if (bWrite)
			CVI_IVE_WriteMem(handle, "sample_Hist.bin", &dst_hist);
	}

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeM(handle, &ref_hist);
	CVI_SYS_FreeM(handle, &dst_hist);
	CVI_IVE_DestroyHandle(handle);

	pthread_exit(0);
}
void *ive_thread_func5(void *arg)
{
	multi_thread_param *param = (multi_thread_param *)arg;
	int bTileMode, bWrite, bInstant;
	char input_data1[64];
	char input_data2[64];
	char output_data[64];
	int loop;


	bTileMode = param->bTileMode;
	bWrite = param->bWrite;
	bInstant = param->bInstant;
	loop = param->loop;

	UNUSED(bTileMode);


	strcpy(input_data1, "res/ive/00_352x288_y.yuv");
	strcpy(input_data2, "res/ive/01_352x288_y.yuv");
	strcpy(output_data, "res/ive/result/sample_NCC_Mem.bin");

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src, src1;

	CVI_IVE_ReadRawImage(handle, &src, input_data1,
			       IVE_IMAGE_TYPE_U8C1, 352, 288);
	CVI_IVE_ReadRawImage(handle, &src1, input_data2,
			       IVE_IMAGE_TYPE_U8C1, 352, 288);

	// Create ref image.
	IVE_MEM_INFO_S ref;

	CVI_IVE_ReadMem(handle, &ref, output_data, sizeof(IVE_NCC_DST_MEM_S));

	// Create dst image.
	IVE_DST_MEM_INFO_S dstNCC;

	CVI_IVE_CreateMemInfo(handle, &dstNCC, sizeof(IVE_NCC_DST_MEM_S));

	// Run HW IVE.
	printf("Run HW IVE NCC.\n");
		for (int i = 0; i < loop; i++) {
			CVI_IVE_NCC(handle, &src, &src1, &dstNCC, bInstant);
			ret |= CVI_IVE_CompareIveMem(&dstNCC, &ref);
		if (bWrite) {
			CVI_IVE_WriteMem(handle, "sample_NCC_Mem.bin", &dstNCC);
		}

		CVI_U64 *numerator = (CVI_U64 *)(uintptr_t)dstNCC.u64VirAddr;
		CVI_U64 *quadSum1 =
			(CVI_U64 *)(uintptr_t)(dstNCC.u64VirAddr + sizeof(CVI_U64));
		CVI_U64 *quadSum2 = quadSum1 + 1;
		CVI_FLOAT fR = (CVI_FLOAT)(
			(CVI_DOUBLE)*numerator /
			(sqrt((CVI_DOUBLE)*quadSum1) * sqrt((CVI_DOUBLE)*quadSum2)));
		printf("NCC value is %f.\n", fR);
	}

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeI(handle, &src1);
	CVI_SYS_FreeM(handle, &dstNCC);
	CVI_SYS_FreeM(handle, &ref);
	CVI_IVE_DestroyHandle(handle);

	pthread_exit(0);
}
void *ive_thread_func6(void *arg)
{
	multi_thread_param *param = (multi_thread_param *)arg;
	int bTileMode, bWrite, bInstant;
	int input_w, input_h;
	char input_data[64];
	char output_data1[64];
	char output_file_3x3[64];
	int loop;


	bTileMode = param->bTileMode;
	bWrite = param->bWrite;
	bInstant = param->bInstant;
	loop = param->loop;

	UNUSED(bTileMode);


	if (bTileMode) {
		strcpy(input_data, "res/ive/sky_640x480.yuv");
		strcpy(output_data1, "res/ive/result/sample_tile_Dilate_3x3.yuv");
		input_w = 640;
		input_h = 480;
		strcpy(output_file_3x3, "sample_tile_Dilate_3x3.yuv");
	} else {
		strcpy(input_data, "res/ive/bin_352x288_y.yuv");
		strcpy(output_data1, "res/ive/result/sample_Dilate_3x3_dilate_only.bin");
		input_w = 352;
		input_h = 288;
		strcpy(output_file_3x3, "sample_Dilate_3x3_dilate_only.bin");
	}
	CVI_U8 arr3by3[25] = { 0,   0, 0, 0, 0,	  0, 0, 255, 0, 0, 0, 255, 255,
			       255, 0, 0, 0, 255, 0, 0, 0,   0, 0, 0, 0 };


	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src;

	CVI_IVE_ReadRawImage(handle, &src, input_data, IVE_IMAGE_TYPE_U8C1,
			       input_w, input_h);

	// Create ref image.
	IVE_IMAGE_S ref_3x3;

	CVI_IVE_ReadRawImage(handle, &ref_3x3, output_data1,
			       IVE_IMAGE_TYPE_U8C1, src.u32Width,
			       src.u32Height);

	// Create dst image.
	IVE_DST_IMAGE_S dst;

	CVI_IVE_CreateImage(handle, &dst, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);

	// Config Setting.
	IVE_DILATE_CTRL_S stCtrlDilate;

	memcpy(stCtrlDilate.au8Mask, arr3by3, sizeof(CVI_U8) * 25);

	// Run IVE
	for (int i = 0 ; i < loop; i++) {
		printf("Run HW IVE Dilate 3x3.\n");
		CVI_IVE_Dilate(handle, &src, &dst, &stCtrlDilate, bInstant);
		ret |= CVI_IVE_CompareIveImage(&dst, &ref_3x3);
		if (bWrite)
			CVI_IVE_WriteImg(handle, output_file_3x3, &dst);
	}

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeI(handle, &dst);
	CVI_SYS_FreeI(handle, &ref_3x3);
	CVI_IVE_DestroyHandle(handle);

	pthread_exit(0);
}
void *ive_thread_func7(void *arg)
{
	multi_thread_param *param = (multi_thread_param *)arg;
	int bTileMode, bWrite, bInstant;
	int input_w1, input_h1;
	char input_data1[64];
	char input_data2[64];
	char output_data_4c[64];
	char output_data_8c[64];

	char output_file_4c[64];
	char output_file_8c[64];
	int loop;


	bTileMode = param->bTileMode;
	bWrite = param->bWrite;
	bInstant = param->bInstant;
	loop = param->loop;

	UNUSED(bTileMode);


	strcpy(input_data1, "res/ive/ccl_raw_0.raw");
	strcpy(input_data2, "res/ive/ccl_raw_1.raw");
	strcpy(output_data_4c, "res/ive/result/sample_CCL_0.bin");
	strcpy(output_data_8c, "res/ive/result/sample_CCL_1.bin");

	strcpy(output_file_4c, "sample_CCL_0.bin");
	strcpy(output_file_8c, "sample_CCL_1.bin");


	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	input_w1 = 1280;
	input_h1 = 720;


	// Create src image.
	IVE_IMAGE_S src1;

	CVI_IVE_ReadRawImage(handle, &src1, input_data1, IVE_IMAGE_TYPE_U8C1, input_w1, input_h1);

	//Create ref data.
	IVE_MEM_INFO_S ref_4c;

	CVI_IVE_ReadMem(handle, &ref_4c, output_data_4c, sizeof(IVE_CCBLOB_S));

	//Create dst data.
	IVE_MEM_INFO_S dstCCL_4c;

	CVI_IVE_CreateMemInfo(handle, &dstCCL_4c, sizeof(IVE_CCBLOB_S));

	//Config Setting.
	IVE_CCL_CTRL_S ctrl;
	IVE_CCL_MODE_E enMode = IVE_CCL_MODE_4C;

	ctrl.enMode = enMode;
	ctrl.u16InitAreaThr = 4;
	ctrl.u16Step = 2;
	//Run HW IVE
	for (int i = 0; i < loop; i++) {
		printf("Run HW IVE CCL\n");
		CVI_IVE_CCL(handle, &src1, &dstCCL_4c, &ctrl, bInstant);
		ret |= CVI_IVE_CompareIveMem(&dstCCL_4c, &ref_4c);
		if (bWrite) {
			CVI_IVE_WriteMem(handle, output_file_4c, &dstCCL_4c);
		}
	}

	CVI_SYS_FreeI(handle, &src1);
	CVI_SYS_FreeM(handle, &dstCCL_4c);
	CVI_SYS_FreeM(handle, &ref_4c);
	CVI_IVE_DestroyHandle(handle);

	pthread_exit(0);
}
void *ive_thread_func8(void *arg)
{
	multi_thread_param *param = (multi_thread_param *)arg;
	int bTileMode, bWrite, bInstant;
	int input_w, input_h;
	char input_data[64];
	char output_data1[64];
	char output_file1[64];
	int loop;


	bTileMode = param->bTileMode;
	bWrite = param->bWrite;
	bInstant = param->bInstant;
	loop = param->loop;

	//TODO:tilemode.
	if (bTileMode) {

		input_w = 352 * 2;
		input_h = 288;
		strcpy(output_file1, "sample_tile_LBP_Normal.yuv");
	} else {
		strcpy(input_data, "res/ive/00_352x288_y.yuv");
		strcpy(output_data1, "res/ive/result/sample_LBP_Normal.yuv");
		input_w = 352;
		input_h = 288;
		strcpy(output_file1, "sample_LBP_Normal.yuv");
	}

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src;

	// Create ref image.
	IVE_IMAGE_S ref_nor;

	if (bTileMode) {
		CVI_IVE_CreateImage(handle, &src, IVE_IMAGE_TYPE_U8C1,
			input_w, input_h);
		for (int i = 0; i < 288; i++) {
			memcpy(&((char *)(uintptr_t)src
					.u64VirAddr[0])[i * input_w],
					&input_data[i * 352],
					352);
			memcpy(&((char *)(uintptr_t)src
					.u64VirAddr[0])[i * input_w + 352],
					&input_data[i * 352],
					352);
		}
	} else {
		CVI_IVE_ReadRawImage(handle, &src, input_data,
				IVE_IMAGE_TYPE_U8C1, input_w, input_h);
		CVI_IVE_ReadRawImage(handle, &ref_nor, output_data1,
				IVE_IMAGE_TYPE_U8C1, input_w, input_h);
	}

	// Create dst image.
	IVE_DST_IMAGE_S dst;

	CVI_IVE_CreateImage(handle, &dst, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);

	// Config Setting.
	IVE_LBP_CTRL_S ctrl;

	ctrl.enMode = IVE_LBP_CMP_MODE_NORMAL;
	ctrl.un8BitThr.s8Val = (ctrl.enMode == IVE_LBP_CMP_MODE_ABS ? 35 : 41);

	// Run HW IVE
	for (int i = 0; i < loop; i++) {
		printf("Run HW IVE LBP Normal.\n");
		CVI_IVE_LBP(handle, &src, &dst, &ctrl, bInstant);
		if (!bTileMode) {
			ret |= CVI_IVE_CompareIveImage(&dst, &ref_nor);
		}
		if (bWrite)
			CVI_IVE_WriteImg(handle, output_file1, &dst);
	}

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeI(handle, &dst);
	CVI_SYS_FreeI(handle, &ref_nor);
	CVI_IVE_DestroyHandle(handle);

	pthread_exit(0);
}
void *ive_thread_func9(void *arg)
{
	multi_thread_param *param = (multi_thread_param *)arg;
	int bTileMode, bWrite, bInstant;
	int input_w, input_h;
	char input_data[64];
	char output_data[64];
	char output_file[64];
	int loop;


	bTileMode = param->bTileMode;
	bWrite = param->bWrite;
	bInstant = param->bInstant;
	loop = param->loop;

	if (bTileMode) {

		strcpy(input_data, "res/ive/sky_640x480.yuv");
		strcpy(output_data, "res/ive/result/sample_tile_Or.yuv");
		input_w = 640;
		input_h = 480;
		strcpy(output_file, "sample_tile_Or.yuv");
	} else {
		strcpy(input_data, "res/ive/00_352x288_y.yuv");
		strcpy(output_data, "res/ive/result/sample_Or.yuv");
		input_w = 352;
		input_h = 288;
		strcpy(output_file, "sample_Or.yuv");
	}

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src1, src2;

	CVI_IVE_ReadRawImage(handle, &src1, input_data, IVE_IMAGE_TYPE_U8C1,
			       input_w, input_h);
	CVI_IVE_CreateImage(handle, &src2, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);
	memset((void *)(uintptr_t)src2.u64VirAddr[0], 255,
	       src1.u32Stride[0] * input_h);
	for (int j = input_h / 10; j < input_h * 9 / 10; j++) {
		for (int i = input_w / 10; i < input_w * 9 / 10; i++) {
			((char *)(uintptr_t)src2
				 .u64VirAddr[0])[i + j * src1.u32Stride[0]] = 0;
		}
	}

	// Create ref image.
	IVE_IMAGE_S ref_or;

	CVI_IVE_ReadRawImage(handle, &ref_or, output_data,
			       IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	// Create dst image.
	IVE_DST_IMAGE_S dst;

	CVI_IVE_CreateImage(handle, &dst, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);

	// Config Setting.

	// Run IVE
	for (int i = 0; i < loop; i++) {
		printf("Run HW IVE OR.\n");
		CVI_IVE_Or(handle, &src1, &src2, &dst, bInstant);
		ret |= CVI_IVE_CompareIveImage(&dst, &ref_or);
		if (bWrite)
			CVI_IVE_WriteImg(handle, output_file, &dst);
	}

	CVI_SYS_FreeI(handle, &src1);
	CVI_SYS_FreeI(handle, &src2);
	CVI_SYS_FreeI(handle, &dst);
	CVI_SYS_FreeI(handle, &ref_or);
	CVI_IVE_DestroyHandle(handle);

	pthread_exit(0);
}

int test_multi_thread(int bTileMode, int bWrite, int bInstant)
{
	pthread_t thread[10] = {[0 ... 9] = -1};
	int i, rc;
	multi_thread_param param;

	param.bTileMode = bTileMode;
	param.bWrite = bWrite;
	param.bInstant = bInstant;
	param.loop = 10;

	for (i = 0; i < 10; i++) {
		if (i == 0)
			rc = pthread_create(&thread[i], NULL, ive_thread_func0, (void *)&param);
		else if (i == 1)
			rc = pthread_create(&thread[i], NULL, ive_thread_func1, (void *)&param);
		else if (i == 2)
			rc = pthread_create(&thread[i], NULL, ive_thread_func2, (void *)&param);
		else if (i == 3)
			rc = pthread_create(&thread[i], NULL, ive_thread_func3, (void *)&param);
		else if (i == 4)
			rc = pthread_create(&thread[i], NULL, ive_thread_func4, (void *)&param);
		else if (i == 5)
			rc = pthread_create(&thread[i], NULL, ive_thread_func5, (void *)&param);
		else if (i == 6)
			rc = pthread_create(&thread[i], NULL, ive_thread_func6, (void *)&param);
		else if (i == 7)
			rc = pthread_create(&thread[i], NULL, ive_thread_func6, (void *)&param);
		else if (i == 8)
			rc = pthread_create(&thread[i], NULL, ive_thread_func8, (void *)&param);
		else
			rc = pthread_create(&thread[i], NULL, ive_thread_func9, (void *)&param);

		if (rc)
			printf("pthread_create failed, ret = 0x%x!\n", rc);
	}

	for (i = 0; i < 10; i++)
		pthread_join(thread[i], NULL);

	return ret;
}

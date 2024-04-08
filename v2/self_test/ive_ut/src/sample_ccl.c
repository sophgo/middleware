#include "cvi_ive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_ccl(int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	int input_w1, input_h1, input_w2, input_h2;
	char input_data1[64];
	char input_data2[64];
	char output_data_4c[64];
	char output_data_8c[64];

	char output_file_4c[64];
	char output_file_8c[64];

	strcpy(input_data1, "res/ive/ccl_raw_0.raw");
	strcpy(input_data2, "res/ive/ccl_raw_1.raw");
	strcpy(output_data_4c, "res/ive/result/sample_CCL_0.bin");
	strcpy(output_data_8c, "res/ive/result/sample_CCL_1.bin");

	strcpy(output_file_4c, "sample_CCL_0.bin");
	strcpy(output_file_8c, "sample_CCL_1.bin");


	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	input_w1 = 1280;
	input_h1 = 720;

	input_w2 = 720;
	input_h2 = 576;

	// Create src image.
	IVE_IMAGE_S src1, src2;

	CVI_IVE_ReadRawImage(handle, &src1, input_data1, IVE_IMAGE_TYPE_U8C1, input_w1, input_h1);
	CVI_IVE_ReadRawImage(handle, &src2, input_data2, IVE_IMAGE_TYPE_U8C1, input_w2, input_h2);

	//Create ref data.
	IVE_MEM_INFO_S ref_4c, ref_8c;

	CVI_IVE_ReadMem(handle, &ref_4c, output_data_4c, sizeof(IVE_CCBLOB_S));
	CVI_IVE_ReadMem(handle, &ref_8c, output_data_8c, sizeof(IVE_CCBLOB_S));

	//Create dst data.
	IVE_MEM_INFO_S dstCCL_4c, dstCCL_8c;

	CVI_IVE_CreateMemInfo(handle, &dstCCL_4c, sizeof(IVE_CCBLOB_S));
	CVI_IVE_CreateMemInfo(handle, &dstCCL_8c, sizeof(IVE_CCBLOB_S));

	//Config Setting.
	IVE_CCL_CTRL_S ctrl;
	IVE_CCL_MODE_E enMode = IVE_CCL_MODE_4C;

	ctrl.enMode = enMode;
	ctrl.u16InitAreaThr = 4;
	ctrl.u16Step = 2;
	//Run HW IVE
	printf("Run HW IVE CCL\n");
	CVI_IVE_CCL(handle, &src1, &dstCCL_4c, &ctrl, bInstant);
	ret |= CVI_IVE_CompareIveMem(&dstCCL_4c, &ref_4c);
	if (bWrite) {
		CVI_IVE_WriteMem(handle, output_file_4c, &dstCCL_4c);
	}

	//Config Setting.
	enMode = IVE_CCL_MODE_8C;
	ctrl.enMode = enMode;
	ctrl.u16InitAreaThr = 4;
	ctrl.u16Step = 2;

	//Run HW IVE CCL.
	printf("Run HW IVE CCL\n");
	CVI_IVE_CCL(handle, &src2, &dstCCL_8c, &ctrl, bInstant);
	ret |= CVI_IVE_CompareIveMem(&dstCCL_8c, &ref_8c);
	if (bWrite) {
		CVI_IVE_WriteMem(handle, output_file_8c, &dstCCL_8c);
	}

	CVI_SYS_FreeI(handle, &src1);
	CVI_SYS_FreeI(handle, &src2);
	CVI_SYS_FreeM(handle, &dstCCL_4c);
	CVI_SYS_FreeM(handle, &dstCCL_8c);
	CVI_SYS_FreeM(handle, &ref_4c);
	CVI_SYS_FreeM(handle, &ref_8c);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

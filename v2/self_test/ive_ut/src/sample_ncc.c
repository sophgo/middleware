#include "cvi_ive.h"
// #include "cvi_pattern.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int test_ncc(int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	char input_data1[64];
	char input_data2[64];
	char output_data[64];

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

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeI(handle, &src1);
	CVI_SYS_FreeM(handle, &dstNCC);
	CVI_SYS_FreeM(handle, &ref);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

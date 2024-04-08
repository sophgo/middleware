#include "cvi_ive.h"
// #include "cvi_pattern.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_hist(int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	char input_data[64];
	char output_data[64];

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
	printf("Run HW IVE Hist.\n");
	CVI_IVE_Hist(handle, &src, &dst_hist, bInstant);
	ret |= CVI_IVE_CompareIveMem(&dst_hist, &ref_hist);
	if (bWrite)
		CVI_IVE_WriteMem(handle, "sample_Hist.bin", &dst_hist);

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeM(handle, &ref_hist);
	CVI_SYS_FreeM(handle, &dst_hist);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}
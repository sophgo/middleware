#include "cvi_ive.h"
// #include "cvi_pattern.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_integ(int bWrite, int bInstant)
{
	int ret = CVI_SUCCESS;
	char input_data[64];
	char output_data1[64];
	char output_data2[64];
	char output_data3[64];

	strcpy(input_data, "res/ive/00_352x288_y.yuv");
	strcpy(output_data1, "res/ive/result/sample_Integ_Combine.yuv");
	strcpy(output_data2, "res/ive/result/sample_Integ_Sum.yuv");
	strcpy(output_data3, "res/ive/result/sample_Integ_Sqsum.yuv");

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	IVE_IMAGE_S src;

	CVI_IVE_ReadRawImage(handle, &src, input_data,
			       IVE_IMAGE_TYPE_U8C1, 352, 288);

	// Create ref image.
	IVE_MEM_INFO_S ref_com, ref_sum, ref_sqm;

	CVI_IVE_ReadMem(handle, &ref_com, output_data1,
			     src.u32Width * src.u32Height *
				     sizeof(CVI_U64));
	CVI_IVE_ReadMem(handle, &ref_sum, output_data2,
			     src.u32Width * src.u32Height *
				     sizeof(CVI_U32));
	CVI_IVE_ReadMem(handle, &ref_sqm, output_data3,
			     src.u32Width * src.u32Height *
				     sizeof(CVI_U64));

	// Create dst image.
	IVE_DST_MEM_INFO_S dst_integ32, dst_integ64;

	CVI_IVE_CreateMemInfo(handle, &dst_integ32,
			      src.u32Width * src.u32Height *
				      sizeof(CVI_U32));
	CVI_IVE_CreateMemInfo(handle, &dst_integ64,
			      src.u32Width * src.u32Height *
				      sizeof(CVI_U64));

	// Config Setting.
	IVE_INTEG_CTRL_S pstIntegCtrl;

	// Run IVE
	printf("Run HW IVE Integral COMBINE.\n");
	pstIntegCtrl.enOutCtrl = IVE_INTEG_OUT_CTRL_COMBINE;
	CVI_IVE_Integ(handle, &src, &dst_integ64, &pstIntegCtrl, bInstant);
	ret |= CVI_IVE_CompareIveMem(&dst_integ64, &ref_com);
	if (bWrite)
		CVI_IVE_WriteMem(handle, "sample_Integ_Combine.yuv",
				 &dst_integ64);

	printf("Run HW IVE Integral SUM.\n");
	pstIntegCtrl.enOutCtrl = IVE_INTEG_OUT_CTRL_SUM;
	CVI_IVE_Integ(handle, &src, &dst_integ32, &pstIntegCtrl, bInstant);
	ret |= CVI_IVE_CompareIveMem(&dst_integ32, &ref_sum);
	if (bWrite)
		CVI_IVE_WriteMem(handle, "sample_Integ_Sum.yuv", &dst_integ32);

	printf("Run HW IVE Integral SQSUM.\n");
	pstIntegCtrl.enOutCtrl = IVE_INTEG_OUT_CTRL_SQSUM;
	CVI_IVE_Integ(handle, &src, &dst_integ64, &pstIntegCtrl, bInstant);
	ret |= CVI_IVE_CompareIveMem(&dst_integ64, &ref_sqm);
	if (bWrite)
		CVI_IVE_WriteMem(handle, "sample_Integ_Sqsum.yuv",
				 &dst_integ64);

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeM(handle, &ref_com);
	CVI_SYS_FreeM(handle, &ref_sum);
	CVI_SYS_FreeM(handle, &ref_sqm);
	CVI_SYS_FreeM(handle, &dst_integ32);
	CVI_SYS_FreeM(handle, &dst_integ64);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

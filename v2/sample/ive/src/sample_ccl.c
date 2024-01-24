#include "cvi_ive.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

int main(int argc, char **argv)
{
	if (argc != 4) {
		printf("Incorrect loop value. Usage: %s <w> <h> <file_name>\n", argv[0]);
		printf("Example: %s 1280 720 data/ccl_raw_0.raw\n", argv[0]);
		return CVI_FAILURE;
	}
	const char *filename = argv[3];
	int ret = CVI_SUCCESS;
	int input_w, input_h;
	unsigned long elapsed_cpu;
	struct timeval t0, t1;
	CVI_BOOL bInstant = true;

	input_w = atoi(argv[1]);
	input_h = atoi(argv[2]);
	gettimeofday(&t0, NULL);

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

    // Create src image.
	IVE_IMAGE_S src;

	CVI_IVE_ReadRawImage(handle, &src, filename, IVE_IMAGE_TYPE_U8C1, input_w, input_h);

    //Create dst data.
	IVE_DST_MEM_INFO_S dstCCL;

	CVI_IVE_CreateMemInfo(handle, &dstCCL, sizeof(IVE_CCBLOB_S));

    //Config Setting.
	IVE_CCL_CTRL_S ctrl;
	IVE_CCL_MODE_E enMode = IVE_CCL_MODE_4C;

	ctrl.enMode = enMode;
	ctrl.u16InitAreaThr = 4;
	ctrl.u16Step = 2;
    //Run HW IVE
	printf("Run HW IVE CCL\n");
	CVI_IVE_CCL(handle, &src, &dstCCL, &ctrl, bInstant);
	gettimeofday(&t1, NULL);
	elapsed_cpu =
		((t1.tv_sec - t0.tv_sec) * 1000000 + t1.tv_usec - t0.tv_usec);
	printf("%s CPU time %lu\n", __func__, elapsed_cpu);

    //Save dst data.
	CVI_IVE_WriteMem(handle, "sample_CCL.bin", &dstCCL);


    //Release resource.
	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeM(handle, &dstCCL);
	CVI_IVE_DestroyHandle(handle);

	return ret;
}

#include "cvi_ive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_cmdq(void)
{
	int ret = CVI_SUCCESS;

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Run HW IVE
	printf("Run HW IVE CMDQ.\n");
	ret = CVI_IVE_CMDQ(handle);

	return ret;
}

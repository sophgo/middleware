#include "cvi_ive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int dump(void)
{
	int ret = CVI_SUCCESS;

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	ret = CVI_IVE_DUMP(handle);

	CVI_IVE_DestroyHandle(handle);
	return ret;
}
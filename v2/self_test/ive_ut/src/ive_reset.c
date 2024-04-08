#include "cvi_ive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int reset(void)
{
	int ret = CVI_SUCCESS;

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	ret = CVI_IVE_RESET(handle, 100);

	CVI_IVE_DestroyHandle(handle);
	return ret;
}
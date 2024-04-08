#include "vi_ut.h"
#include <inttypes.h>

static CVI_S32 _vi_ut_handle_op(CVI_S32 op)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	switch (op) {
	case 1:
		s32Ret = vi_ut_get_chn_frame(0);
		VI_UT_PRT("client get chn frame %s\n",
			(s32Ret == 0) ? "pass" : "fail");
		break;
	default:
		break;
	}

	return s32Ret;
}

int main(int argc, char **argv)
{
	CVI_S32 s32Ret = 0;
	int op;

	if (argc >= 2) {
		op = (CVI_S32)atoi(argv[1]);
		s32Ret = _vi_ut_handle_op(op);
	} else {
		VI_UT_PRT("1: dump vi yuv frame\n");
		s32Ret = CVI_FAILURE;
	}

	return s32Ret;
}


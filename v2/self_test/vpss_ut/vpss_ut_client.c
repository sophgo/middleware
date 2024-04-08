#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "cvi_buffer.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vpss.h"
#include "vpss_ut_comm.h"

typedef enum _VPSS_TEST_CLNT_OP {
	VPSS_TEST_CLNT_GET_CHN_FRAME,
} VPSS_TEST_CLNT_OP;


static CVI_S32 vpss_get_chn_frame_test(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VPSS_GRP VpssGrp = 0;
	VPSS_CHN VpssChn = 0;
	VIDEO_FRAME_INFO_S stVideoFrame;
	CVI_CHAR *pszMD5Sum = "f7f1b2de74f7b9586e89b23ec9ac23e4";

	s32Ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, 1000);
	if (s32Ret != CVI_SUCCESS) {
		VPSS_UT_PRT("CVI_VPSS_GetChnFrame fail!\n");
		goto exit;
	}
	VPSS_UT_PRT("***Grp(%d) Chn(%d) CVI_VPSS_GetChnFrame Success***\n",
			VpssGrp, VpssChn);

	if (CompareWithMD5(pszMD5Sum, &stVideoFrame)) {
		FrameSaveToFile("out/vpss_get_chn_frame_test.bin", &stVideoFrame);
		s32Ret = CVI_FAILURE;
		VPSS_UT_PRT("Compare MD5 fail, MD5:%s\n", pszMD5Sum);
	}
	CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame);

exit:
	VPSS_UT_PRT("client get chn frame %s\n", s32Ret == CVI_SUCCESS ? "pass" : "fail");

	return s32Ret;
}

static CVI_S32 _handle_op(CVI_S32 op)
{
	CVI_S32 s32Ret = CVI_FAILURE;

	switch (op) {
	case VPSS_TEST_CLNT_GET_CHN_FRAME:
		s32Ret = vpss_get_chn_frame_test();
		break;

	default:
		break;
	}

	return s32Ret;
}

static void show_help(void)
{
	VPSS_UT_PRT("%4d: Get Chn Frame\n", VPSS_TEST_CLNT_GET_CHN_FRAME);
}

CVI_S32 main(CVI_S32 argc, CVI_CHAR **argv)
{
	CVI_S32 s32Ret = 0;
	CVI_S32 op = 255;

	if (argc < 2) {
		show_help();
		return -1;
	}
	op = (CVI_S32)atoi(argv[1]);

	s32Ret = _handle_op(op);
	VPSS_UT_PRT("vpss client ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");

	return s32Ret;
}


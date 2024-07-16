#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <pthread.h>

#include "cvi_buffer.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_stitch.h"
#include "stitch_ut_comm.h"

#define BIT(nr)      (UINT64_C(1) << (nr))

//file: http://disk-sophgo-vip.quickconnect.cn/sharing/iglxSGiJB
#define STITCH_FILE_IN_LFT                 "res/stitch/input/c01_img1__4608x288.yuv"
#define STITCH_FILE_IN_RHT                 "res/stitch/input/c01_img2__4608x288.yuv"
#define STITCH_FILE_IN_WGT_ALPHA           "res/stitch/input/c01_alpha12_444p_m2__0_288x2304.bin"
#define STITCH_FILE_IN_WGT_BETA            "res/stitch/input/c01_beta12_444p_m2__0_288x2304.bin"
#define STITCH_FILE_OUT                    "res/stitch/output/c01_img12__6912x288.yuv"
#define STITCH_FILE_PEF                    "res/stitch/pef/c01_img12__6912x288.yuv"

#define STITCH_FILE_IN_WGT_ALPHA1           "res/stitch/input/c01_alpha12_m2__288x2304_short.bin"
#define STITCH_FILE_IN_WGT_BETA1            "res/stitch/input/c01_beta12_m2__288x2304_short.bin"
#define STITCH_FILE_OUT1                    "res/stitch/output/c01_img12__6912x288_1.yuv"
#define STITCH_FILE_PEF1                    "res/stitch/pef/c01_img12__6912x288_1.yuv"

#define STITCH_FULL_FILE_IN_LFT                 "res/stitch/input/c01_lft__4608x288_full_ovlp.yuv"
#define STITCH_FULL_FILE_IN_RHT                 "res/stitch/input/c01_rht__4608x288_full_ovlp.yuv"
#define STITCH_FULL_FILE_IN_WGT_ALPHA           "res/stitch/input/c01_alpha_444p_m2__0_288x4608_full_ovlp.bin"
#define STITCH_FULL_FILE_IN_WGT_BETA            "res/stitch/input/c01_beta_444p_m2__0_288x4608_full_ovlp.bin"
#define STITCH_FULL_FILE_OUT                    "res/stitch/output/c01_result_420p_c2_4608x288_full_ovlp.yuv"
#define STITCH_FULL_FILE_PEF                    "res/stitch/pef/c01_result_420p_c2_4608x288_full_ovlp.yuv"

#define STITCH_NONE_FILE_IN_LFT                 "res/stitch/input/1920x1080.yuv"
#define STITCH_NONE_FILE_IN_RHT                 "res/stitch/input/1920x1080.yuv"
#define STITCH_NONE_FILE_OUT                    "res/stitch/output/c01_result_420p_c2_3840x1080_none_ovlp.yuv"
#define STITCH_NONE_FILE_PEF                    "res/stitch/pef/c01_result_420p_c2_3840x1080_none_ovlp.yuv"

#define STITCH_NONE_FILE_IN_LFT1                 "res/stitch/input/c01_img1__1536x288.yuv"
#define STITCH_NONE_FILE_IN_RHT1                 "res/stitch/input/c01_img2__1536x288.yuv"
#define STITCH_NONE_FILE_OUT1                    "res/stitch/output/c01_result_420p_c2_3072x288_none_ovlp.yuv"
#define STITCH_NONE_FILE_PEF1                    "res/stitch/pef/c01_result_420p_c2_3072x288_none_ovlp.yuv"

#ifndef FPGA_PORTING
#define STITCH_LFT_OVLP_FILE_IN_LFT                 "res/stitch/input/lft_ovlp/c01_img1__2304x288.yuv"
#define STITCH_LFT_OVLP_FILE_IN_RHT                 "res/stitch/input/lft_ovlp/c01_img2__4608x288.yuv"
#define STITCH_LFT_OVLP_FILE_IN_WGT_ALPHA           "res/stitch/input/lft_ovlp/c01_alpha12_444p_m2__0_288x2304.bin"
#define STITCH_LFT_OVLP_FILE_IN_WGT_BETA            "res/stitch/input/lft_ovlp/c01_beta12_444p_m2__0_288x2304.bin"
#define STITCH_LFT_OVLP_FILE_OUT                    "res/stitch/output/lft_ovlp/c01_result_420p_c2_4608x288_lft_ovlp.yuv"
#define STITCH_LFT_OVLP_FILE_PEF                    "res/stitch/pef/lft_ovlp/c01_result_420p_c2_4608x288_lft_ovlp.yuv"

#define STITCH_RHT_OVLP_FILE_IN_LFT                 "res/stitch/input/rht_ovlp/c01_img1__4608x288.yuv"
#define STITCH_RHT_OVLP_FILE_IN_RHT                 "res/stitch/input/rht_ovlp/c01_img2__2304x288.yuv"
#define STITCH_RHT_OVLP_FILE_IN_WGT_ALPHA           "res/stitch/input/rht_ovlp/c01_alpha12_444p_m2__0_288x2304.bin"
#define STITCH_RHT_OVLP_FILE_IN_WGT_BETA            "res/stitch/input/rht_ovlp/c01_beta12_444p_m2__0_288x2304.bin"
#define STITCH_RHT_OVLP_FILE_OUT                    "res/stitch/output/rht_ovlp/c01_result_420p_c2_4608x288_rht_ovlp.yuv"
#define STITCH_RHT_OVLP_FILE_PEF                    "res/stitch/pef/rht_ovlp/c01_result_420p_c2_4608x288_rht_ovlp.yuv"
#else
#define STITCH_LFT_OVLP_FILE_IN_LFT                 "res/stitch/input/c01_img1__4608x288.yuv"
#define STITCH_LFT_OVLP_FILE_IN_RHT                 "res/stitch/input/c01_img2__6912x288.yuv"
#define STITCH_LFT_OVLP_FILE_IN_WGT_ALPHA           "res/stitch/input/c01_alpha_444p_m2__0_288x4608_full_ovlp.bin"
#define STITCH_LFT_OVLP_FILE_IN_WGT_BETA            "res/stitch/input/c01_beta_444p_m2__0_288x4608_full_ovlp.bin"
#define STITCH_LFT_OVLP_FILE_OUT                    "res/stitch/output/c01_result_420p_c2_6912x288_lft_ovlp.yuv"
#define STITCH_LFT_OVLP_FILE_PEF                    "res/stitch/pef/c01_result_420p_c2_6912x288_lft_ovlp.yuv"

#define STITCH_RHT_OVLP_FILE_IN_LFT                 "res/stitch/input/c01_img1__6912x288.yuv"
#define STITCH_RHT_OVLP_FILE_IN_RHT                 "res/stitch/input/c01_img2__4608x288.yuv"
#define STITCH_RHT_OVLP_FILE_IN_WGT_ALPHA           "res/stitch/input/c01_alpha_444p_m2__0_288x4608_full_ovlp.bin"
#define STITCH_RHT_OVLP_FILE_IN_WGT_BETA            "res/stitch/input/c01_beta_444p_m2__0_288x4608_full_ovlp.bin"
#define STITCH_RHT_OVLP_FILE_OUT                    "res/stitch/output/c01_result_420p_c2_6912x288_rht_ovlp.yuv"
#define STITCH_RHT_OVLP_FILE_PEF                    "res/stitch/pef/c01_result_420p_c2_6912x288_rht_ovlp.yuv"
#endif

#define STITCH_PURE_COLOR_FILE_IN_LFT                 "res/stitch/input/c01_lft__1536x384_pure_color.yuv"
#define STITCH_PURE_COLOR_FILE_IN_RHT                 "res/stitch/input/c01_rht__1088x384_pure_color.yuv"
#define STITCH_PURE_COLOR_FILE_IN_WGT_ALPHA           "res/stitch/input/c01_alpha_444p_m2__0_384x224.bin"
#define STITCH_PURE_COLOR_FILE_IN_WGT_BETA            "res/stitch/input/c01_beta_444p_m2__0_384x224.bin"
#define STITCH_PURE_COLOR_FILE_OUT                    "res/stitch/output/c01_result_c2_2400x384_pure_color.yuv"
#define STITCH_PURE_COLOR_FILE_PEF                    "res/stitch/pef/c01_result_c2_2400x384_pure_color.yuv"

#define STITCH_PURE_COLOR_FILE_IN_WGT_ALPHA1           "res/stitch/input/c01_alpha_m2__384x224_short.bin"
#define STITCH_PURE_COLOR_FILE_IN_WGT_BETA1            "res/stitch/input/c01_beta_m2__384x224_short.bin"
#define STITCH_PURE_COLOR_FILE_OUT1                    "res/stitch/output/c01_result_c2_2400x384_pure_color1.yuv"
#define STITCH_PURE_COLOR_FILE_PEF1                    "res/stitch/pef/c01_result_c2_2400x384_pure_color1.yuv"

#define STITCH_MIN_SIZE_FILE_IN_LFT1                 "res/stitch/input/c01_lft__64x64.yuv"
#define STITCH_MIN_SIZE_FILE_IN_RHT1                 "res/stitch/input/c01_rht__64x64.yuv"
#define STITCH_MIN_SIZE_FILE_IN_WGT_ALPHA1           "res/stitch/input/c01_alpha_444p_m2__0_64x32.bin"
#define STITCH_MIN_SIZE_FILE_IN_WGT_BETA1            "res/stitch/input/c01_beta_444p_m2__0_64x32.bin"
#define STITCH_MIN_SIZE_FILE_OUT1                    "res/stitch/output/c01_result_c2_96x64.yuv"
#define STITCH_MIN_SIZE_FILE_PEF1                    "res/stitch/pef/c01_result_c2_96x64.yuv"

#define STITCH_MIN_SIZE_FILE_IN_LFT2                 "res/stitch/input/c01_lft__128x128.yuv"
#define STITCH_MIN_SIZE_FILE_IN_RHT2                 "res/stitch/input/c01_rht__128x128.yuv"
#define STITCH_MIN_SIZE_FILE_IN_WGT_ALPHA2           "res/stitch/input/c01_alpha_444p_m2__0_128x64.bin"
#define STITCH_MIN_SIZE_FILE_IN_WGT_BETA2            "res/stitch/input/c01_beta_444p_m2__0_128x64.bin"
#define STITCH_MIN_SIZE_FILE_OUT2                    "res/stitch/output/c01_result_c2_192x128.yuv"
#define STITCH_MIN_SIZE_FILE_PEF2                    "res/stitch/pef/c01_result_c2_192x128.yuv"

#define STITCH_MIDDLE_SIZE_FILE_IN_LFT                 "res/stitch/input/c01_lft__1024x1024.yuv"
#define STITCH_MIDDLE_SIZE_FILE_IN_RHT                 "res/stitch/input/c01_rht__1024x1024.yuv"
#define STITCH_MIDDLE_SIZE_FILE_IN_WGT_ALPHA           "res/stitch/input/c01_alpha_444p_m2__0_1024x512.bin"
#define STITCH_MIDDLE_SIZE_FILE_IN_WGT_BETA            "res/stitch/input/c01_beta_444p_m2__0_1024x512.bin"
#define STITCH_MIDDLE_SIZE_FILE_OUT                    "res/stitch/output/c01_result_c2_1536x1024.yuv"
#define STITCH_MIDDLE_SIZE_FILE_PEF                    "res/stitch/pef/c01_result_c2_1536x1024.yuv"

#define STITCH_MIDDLE_SIZE_FILE_OUT2                   "res/stitch/output/c01_result_c2_2560x1024.yuv"
#define STITCH_MIDDLE_SIZE_FILE_PEF2                   "res/stitch/pef/c01_result_c2_2560x1024.yuv"

#define STITCH_FILE_OUT_400                         "res/stitch/output/c01_img12__6912x288_fmt400.yuv"
#define STITCH_FILE_PEF_400                         "res/stitch/pef/c01_img12__6912x288_fmt400.yuv"

#define STITCH_FILE_IN_LFT_422                      "res/stitch/input/c01_img1_422p__4608x288.yuv"
#define STITCH_FILE_IN_RHT_422                      "res/stitch/input/c01_img2_422p__4608x288.yuv"
#define STITCH_FILE_OUT_422                         "res/stitch/output/c01_img12__6912x288_fmt422p.yuv"
#define STITCH_FILE_PEF_422                         "res/stitch/pef/c01_img12__6912x288_fmt422p.yuv"

#define STITCH_FILE_IN_LFT_444                      "res/stitch/input/c01_img1_444p__4608x288.yuv"
#define STITCH_FILE_IN_RHT_444                      "res/stitch/input/c01_img2_444p__4608x288.yuv"
#define STITCH_FILE_OUT_444                         "res/stitch/output/c01_img12__6912x288_fmt444p.yuv"
#define STITCH_FILE_PEF_444                         "res/stitch/pef/c01_img12__6912x288_fmt444p.yuv"

#define STITCH_FILE_IN_LFT_RGB                      "res/stitch/input/c01_img1_RGB888P__1920x1080.bin"
#define STITCH_FILE_IN_RHT_RGB                      "res/stitch/input/c01_img2_RGB888P__1920x1080.bin"
#define STITCH_FILE_OUT_RGB                         "res/stitch/output/c01_img12__3840x1080_fmtRGB888P.bin"

#define STITCH_FILE_IN_WGT_ALPHA_12                 "res/stitch/input/c01_alpha12_444p_m2__0_288x2304.bin"
#define STITCH_FILE_IN_WGT_BETA_12                  "res/stitch/input/c01_beta12_444p_m2__0_288x2304.bin"
#define STITCH_FILE_IN_WGT_ALPHA_23                 "res/stitch/input/c01_alpha23_444p_m2__0_288x2304.bin"
#define STITCH_FILE_IN_WGT_BETA_23                  "res/stitch/input/c01_beta23_444p_m2__0_288x2304.bin"
#define STITCH_FILE_IN_WGT_ALPHA_34                 "res/stitch/input/c01_alpha34_444p_m2__0_288x2304.bin"
#define STITCH_FILE_IN_WGT_BETA_34                  "res/stitch/input/c01_beta34_444p_m2__0_288x2304.bin"
#define STITCH_FILE_IN_1                            "res/stitch/input/c01_img1__4608x288.yuv"
#define STITCH_FILE_IN_2                            "res/stitch/input/c01_img2__4608x288.yuv"
#define STITCH_FILE_IN_3                            "res/stitch/input/c01_img3__4608x288.yuv"
#define STITCH_FILE_IN_4                            "res/stitch/input/c01_img4__4608x288.yuv"
#define STITCH_FILE_OUT12                           "res/stitch/output/c01_img12__6912x288.yuv"
#define STITCH_FILE_OUT34                           "res/stitch/output/c01_img34__6912x288.yuv"
#define STITCH_FILE_OUT123                          "res/stitch/output/c01_img123__9216x288.yuv"
#define STITCH_FILE_OUT1234                         "res/stitch/output/c01_img1234__11520x288.yuv"
#define STITCH_FILE_OUT1234_REAL                    "res/stitch/output/c01_img1234__11520x288_real.yuv"

#define STITCH_FILE_PEF12                           "res/stitch/pef/c01_img12__6912x288.yuv"
#define STITCH_FILE_PEF34                           "res/stitch/pef/c01_img34__6912x288.yuv"
#define STITCH_FILE_PEF123                          "res/stitch/pef/c01_img123__9216x288.yuv"
#define STITCH_FILE_PEF1234                         "res/stitch/pef/c01_img1234__11520x288.yuv"
#define STITCH_FILE_PEF1234_REAL                    "res/stitch/pef/c01_img1234__11520x288_real.yuv"
#define STITCH_FILE_4K                              "res/stitch/input/3840x2160.yuv"
#define STITCH_FILE_OUT_4K                          "res/stitch/output/3840x2160.yuv"
#define STITCH_FILE_PEF_4K                          "res/stitch/pef/3840x2160.yuv"

#define STITCH_MIDDLE_SIZE_FILE_IN_WGT_ALPHA_1P4    "res/stitch/input/c01_alpha_444p_m2__0_256x1024.bin"
#define STITCH_MIDDLE_SIZE_FILE_IN_WGT_BETA_1P4     "res/stitch/input/c01_beta_444p_m2__0_256x1024.bin"
#define STITCH_FILE_OUT1234_REAL_MIDDLE             "res/stitch/output/c01_img1234__3072x1024_real.yuv"
#define STITCH_FILE_PEF1234_REAL_MIDDLE             "res/stitch/pef/c01_img1234__3072x1024_real.yuv"

#ifndef FPGA_PORTING
#define STITCH_REPECT_TIMES 10
#define STITCH_TIMEOUT 10000
#else
#define STITCH_REPECT_TIMES 2
#define STITCH_TIMEOUT 600000
#endif
#define MAX_FUNC_CNT 100

typedef CVI_S32 (*p_func)(void);

static CVI_BOOL g_stitch_save_file;
static CVI_BOOL g_stitch_need_rst;

typedef enum _STITCH_TEST_OP {
	STITCH_TEST_BASIC = 0,
	STITCH_TEST_FULL_OVLP,
	STITCH_TEST_NONE_OVLP,
	STITCH_TEST_NONE_OVLP1,
	STITCH_TEST_LFT_OVLP,
	STITCH_TEST_RHT_OVLP,
	STITCH_TEST_PURE_COLOR,
	STITCH_TEST_PURE_COLOR_MODE1,
	STITCH_TEST_FMT_400,
	STITCH_TEST_FMT_422P,
	STITCH_TEST_FMT_444P,
	STITCH_TEST_FMT_RGB,
	STITCH_TEST_WGT_MODE,
	STITCH_TEST_4WAY_FAKE,
	STITCH_TEST_4WAY_FAKE_MIDDLE,
	STITCH_TEST_4WAY,
	STITCH_TEST_4WAY_MIDDLE,
	STITCH_TEST_SIZE_MIN1,
	STITCH_TEST_SIZE_MIN2,
	STITCH_TEST_SIZE_MIDDLE,
	STITCH_TEST_SIZE_MAX,
	STITCH_TEST_ONLINE,
	STITCH_TEST_MULTI_THREAD,
	STITCH_TEST_PEF,
	STITCH_TEST_NO_VB,
	STITCH_TEST_EN_DIS_DEV_LOOP,
	STITCH_TEST_RST,
	STITCH_TEST_PRESURE_SIZE_FOR_EACH = 98,
	STITCH_TEST_AUTO_REGRESSION = 99,
	STITCH_TEST_USER_CONFIG = 100,
	STITCH_TEST_DUP_FD = 101,
	STITCH_TEST_RST_FD = 102,
	STITCH_TEST_CHECK_SUM = 103,
	STITCH_TEST_SUSPEND = 104,
	STITCH_TEST_RESUME = 105,
	STITCH_TEST_RUN_SUSPEND = 106,
	STITCH_TEST_AUTO_LOOP = 107,
} STITCH_TEST_OP;

typedef struct _STITCH_BASIC_TEST_PARAM {
	CVI_U8 srcNum;
	CVI_U8 CurSrcID[2];
	char filename_in[STITCH_MAX_SRC_NUM][128];
	char filename_out[128];
	char filename_pef[128];
	char wgt_name[STITCH_MAX_SRC_NUM + 2][128];

	STITCH_SRC_ATTR_S srcAttr;
	STITCH_CHN_ATTR_S chnAttr;
	STITCH_OP_ATTR_S opAttr;
	STITCH_WGT_ATTR_S wgtAttr;
	VIDEO_FRAME_INFO_S stVideoFrameOut;
	STITCH_TEST_OP op;
	CVI_BOOL needPef;
	CVI_BOOL needSuspend;
	CVI_BOOL needDumpReg;
} STITCH_BASIC_TEST_PARAM;

void stitch_ut_HandleSig(CVI_S32 signo)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	if (SIGINT == signo || SIGTERM == signo) {
		CVI_SYS_Exit();
		CVI_VB_Exit();
		STITCH_UT_PRT("Program termination abnormally\n");
	}
	exit(-1);
}

static CVI_S32 cfg_wgt_image(SIZE_S size, enum stitch_wgt_mode wgtmode, char *name, CVI_U64 *u64PhyAddr, CVI_VOID **pVirAddr)
{
	CVI_U32 wgt_len;
	FILE *fp;
	int len;

	if (!name) {
		STITCH_UT_PRT("wgt image file is null\n");
		return CVI_FAILURE;
	}

	fp = fopen(name, "rb");
	if (fp == CVI_NULL) {
		STITCH_UT_PRT("open data file [%s] error\n", name);
		return CVI_FAILURE;
	}

	wgt_len = size.u32Width * size.u32Height;
	if (wgtmode == STITCH_WGT_UV_SHARE)
		wgt_len = wgt_len << 1;
	if (CVI_SYS_IonAlloc(u64PhyAddr, pVirAddr, "stitch_wgt_image", wgt_len) != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_SYS_IonAlloc_Cached NG.\n");
		return CVI_FAILURE;
	}
	if (*u64PhyAddr == 0 || *pVirAddr == 0) {
		STITCH_UT_PRT("CVI_SYS_IonAlloc NG. zero phy/vir address\n");
		return CVI_FAILURE;
	}

	STITCH_UT_PRT("wgt_len[%d]\n", wgt_len);

	len = fread(*pVirAddr, wgt_len, 1, fp);
	if (len <= 0) {
		STITCH_UT_PRT("stitch read wgt image fread error, ret(%d)\n", len);
		fclose(fp);
		return CVI_FAILURE;
	}
	fflush(fp);
	fclose(fp);
	return CVI_SUCCESS;
}

static CVI_S32 cfg_wgt_image2(SIZE_S size, enum stitch_wgt_mode wgtmode, CVI_U64 *u64PhyAddr, CVI_VOID **pVirAddr)
{
	CVI_U32 wgt_len;

	wgt_len = size.u32Width * size.u32Height;
	if (wgtmode == STITCH_WGT_UV_SHARE)
		wgt_len = wgt_len << 1;
	if (CVI_SYS_IonAlloc(u64PhyAddr, pVirAddr, "stitch_wgt_image", wgt_len) != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_SYS_IonAlloc_Cached NG.\n");
		return CVI_FAILURE;
	}
	if (*u64PhyAddr == 0 || *pVirAddr == 0) {
		STITCH_UT_PRT("CVI_SYS_IonAlloc NG. zero phy/vir address\n");
		return CVI_FAILURE;
	}

	STITCH_UT_PRT("wgt_len[%d]\n", wgt_len);

	return CVI_SUCCESS;
}

static CVI_S32 basic(STITCH_BASIC_TEST_PARAM *pParam, CVI_S32 times)
{
	CVI_U32 i;
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize[STITCH_MAX_SRC_NUM + 1];
	PIXEL_FORMAT_E fmt_in = pParam->srcAttr.fmt_in;
	PIXEL_FORMAT_E fmt_out = pParam->chnAttr.fmt_out;
	CVI_S32 timout_ms = STITCH_TIMEOUT;

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	for (i = 0; i < pParam->srcNum; i++) {
		u32BlkSize[i] = COMMON_GetPicBufferSize(pParam->srcAttr.size[i].u32Width, pParam->srcAttr.size[i].u32Height, fmt_in
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, STITCH_ALIGN);
		STITCH_UT_PRT("src[%d], w[%d], h[%d]\n", i, pParam->srcAttr.size[i].u32Width, pParam->srcAttr.size[i].u32Height);
	}

	u32BlkSize[i] = COMMON_GetPicBufferSize(pParam->chnAttr.size.u32Width, pParam->chnAttr.size.u32Height, fmt_out
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, STITCH_ALIGN);

	stVbConf.u32MaxPoolCnt              = pParam->srcNum + 1;

	for (i = 0; i < stVbConf.u32MaxPoolCnt; i++) {
		stVbConf.astCommPool[i].u32BlkSize	= u32BlkSize[i];
		stVbConf.astCommPool[i].u32BlkCnt	= 1;
		stVbConf.astCommPool[i].enRemapMode	= VB_REMAP_MODE_CACHED;
		STITCH_UT_PRT("common pool[%d] BlkSize %d\n", i, stVbConf.astCommPool[i].u32BlkSize);
	}

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	/************************************************
	 * step2:  Init STITCH
	 ************************************************/
	s32Ret = CVI_STITCH_Init();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_Init failed!\n");
		goto exit1;
	}

	s32Ret = CVI_STITCH_Reset();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_Reset failed!\n");
		goto exit1;
	}

	s32Ret = CVI_STITCH_SetSrcAttr(&pParam->srcAttr);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetSrcAttr failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetChnAttr(&pParam->chnAttr);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetChnAttr failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetOpAttr(&pParam->opAttr);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetOpAttr failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetWgtAttr(&pParam->wgtAttr);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetWgtAttr failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetRegX(16);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetRegX failed!\n");
		goto exit2;
	}

	/*start stitch*/
	s32Ret = CVI_STITCH_EnableDev();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_EnableDev failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_AttachVbPool((VB_POOL)pParam->srcNum);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_AttachVbPool failed!\n");
		goto exit2;
	}

	do {
		//send frame
		for (i = 0; i < pParam->srcNum; i++) {
			STITCH_UT_PRT("start send src frame[%d]\n", i);

			s32Ret = FileSendToStitch((STITCH_SRC_IDX)i, &pParam->srcAttr.size[i], fmt_in, pParam->filename_in[i]);
			if (s32Ret != CVI_SUCCESS) {
				STITCH_UT_PRT("FileSendToStitch[%d] fail, s32Ret: 0x%x !\n", i, s32Ret);
				goto exit3;
			}
		}

		if (pParam->needSuspend) {
			s32Ret = CVI_STITCH_Suspend();
			if (s32Ret != CVI_SUCCESS) {
				STITCH_UT_PRT("CVI_STITCH_Suspend fail. s32Ret: 0x%x !\n", s32Ret);
				goto exit3;
			}
			s32Ret = CVI_STITCH_Resume();
			if (s32Ret != CVI_SUCCESS) {
				STITCH_UT_PRT("CVI_STITCH_Resume fail. s32Ret: 0x%x !\n", s32Ret);
				goto exit3;
			}
		}

		memset(&pParam->stVideoFrameOut, 0, sizeof(pParam->stVideoFrameOut));
		s32Ret = CVI_STITCH_GetChnFrame(&pParam->stVideoFrameOut, timout_ms);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("CVI_STITCH_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
			pParam->needDumpReg = CVI_TRUE;
			goto exit3;
		}

		STITCH_UT_PRT("***CVI_STITCH_GetChnFrame Success, start save and compare with golden***\n");
		STITCH_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", pParam->stVideoFrameOut.stVFrame.u64PhyAddr[0]
			, pParam->stVideoFrameOut.stVFrame.u64PhyAddr[1], pParam->stVideoFrameOut.stVFrame.u64PhyAddr[2]);

		if (g_stitch_save_file) {
			if (strlen(pParam->filename_out)) {
				s32Ret = FrameSaveToFile(pParam->filename_out, &pParam->stVideoFrameOut);
				if (s32Ret != CVI_SUCCESS) {
					STITCH_UT_PRT("FrameSaveToFile. s32Ret: 0x%x !\n", s32Ret);
					CVI_STITCH_ReleaseChnFrame(&pParam->stVideoFrameOut);
					goto exit3;
				}
				STITCH_UT_PRT("output file:%s\n", pParam->filename_out);
			}

			if (pParam->needPef) {
				s32Ret = CompareWithFile(pParam->filename_pef, &pParam->stVideoFrameOut);
				if (s32Ret != CVI_SUCCESS) {
					STITCH_UT_PRT("CompareWithFile fail.\n");
					CVI_STITCH_ReleaseChnFrame(&pParam->stVideoFrameOut);
					goto exit3;
				}
			}
		}

		s32Ret = CVI_STITCH_ReleaseChnFrame(&pParam->stVideoFrameOut);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("CVI_STITCH_ReleaseChnFrame fail s32Ret: 0x%x !\n", s32Ret);
			goto exit3;
		}
	} while (times--);
exit3:
	if (pParam->needDumpReg)
		CVI_STITCH_DumpRegInfo();

	CVI_STITCH_DisableDev();

	if (g_stitch_need_rst)
		CVI_STITCH_Reset();
exit2:
	CVI_STITCH_DeInit();
exit1:
	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();

	return s32Ret;
}

static CVI_S32 stitch_test_basic(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i, times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM] = {0};
	CVI_VOID *VirAddr[STITCH_MAX_SRC_NUM] = {0};
	char *wgt_name[STITCH_MAX_SRC_NUM] = {STITCH_FILE_IN_WGT_ALPHA, STITCH_FILE_IN_WGT_BETA};
	char *filename_in[STITCH_MAX_SRC_NUM] = {STITCH_FILE_IN_LFT, STITCH_FILE_IN_RHT};
	char *filename_out = STITCH_FILE_OUT;
	char *filename_pef = STITCH_FILE_PEF;

	param.needPef = CVI_TRUE;
	param.needDumpReg = CVI_FALSE;
	param.srcNum = 2;
	param.srcAttr.size[0].u32Width = 4608;
	param.srcAttr.size[0].u32Height = 288;
	param.srcAttr.size[1].u32Width = 4608;
	param.srcAttr.size[1].u32Height = 288;

	param.chnAttr.size.u32Width = 6912;
	param.chnAttr.size.u32Height = 288;

	param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_420;
	param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_420;

	param.srcAttr.way_num = STITCH_2_WAY;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 2304;//ovlap_attr from algo
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 4607;

	param.opAttr.data_src = STITCH_DATA_SRC_DDR;
	param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

	param.wgtAttr.size_wgt[0].u32Width =
		ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[0] - param.srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
	param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

	strcpy(param.filename_out, filename_out);
	strcpy(param.filename_pef, filename_pef);
	for (i = 0; i < param.srcNum; i++) {
		strcpy(param.wgt_name[i], wgt_name[i]);
		strcpy(param.filename_in[i], filename_in[i]);

		s32Ret = cfg_wgt_image(param.wgtAttr.size_wgt[0], param.opAttr.wgt_mode
			, param.wgt_name[i], &u64PhyAddr[i], &VirAddr[i]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
			return s32Ret;
		}
		param.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
	}

	s32Ret = basic(&param, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		goto free;
	}

free:
	for (i = 0; i < param.srcNum; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
	}

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 stitch_test_full_ovlp(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i, times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM] = {0};
	CVI_VOID *VirAddr[STITCH_MAX_SRC_NUM] = {0};

	char *wgt_name[STITCH_MAX_SRC_NUM] = {STITCH_FULL_FILE_IN_WGT_ALPHA, STITCH_FULL_FILE_IN_WGT_BETA};
	char *filename_in[STITCH_MAX_SRC_NUM] = {STITCH_FULL_FILE_IN_LFT, STITCH_FULL_FILE_IN_RHT};
	char *filename_out = STITCH_FULL_FILE_OUT;
	char *filename_pef = STITCH_FULL_FILE_PEF;

	param.needPef = CVI_TRUE;
	param.needDumpReg = CVI_FALSE;
	param.srcNum = 2;
	param.srcAttr.size[0].u32Width = 4608;
	param.srcAttr.size[0].u32Height = 288;
	param.srcAttr.size[1].u32Width = 4608;
	param.srcAttr.size[1].u32Height = 288;

	param.chnAttr.size.u32Width = 4608;
	param.chnAttr.size.u32Height = 288;

	param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_420;
	param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_420;

	param.srcAttr.way_num = STITCH_2_WAY;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 0;//ovlap_attr from algo
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 4607;

	param.opAttr.data_src = STITCH_DATA_SRC_DDR;
	param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

	param.wgtAttr.size_wgt[0].u32Width =
		ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[0] - param.srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
	param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

	strcpy(param.filename_out, filename_out);
	strcpy(param.filename_pef, filename_pef);
	for (i = 0; i < param.srcNum; i++) {
		strcpy(param.wgt_name[i], wgt_name[i]);
		strcpy(param.filename_in[i], filename_in[i]);

		s32Ret = cfg_wgt_image(param.wgtAttr.size_wgt[0], param.opAttr.wgt_mode
			, param.wgt_name[i], &u64PhyAddr[i], &VirAddr[i]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
			return s32Ret;
		}
		param.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
	}

	s32Ret = basic(&param, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		goto free;
	}

free:
	for (i = 0; i < param.srcNum; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
	}

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 stitch_test_none_ovlp(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i, times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	//CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM];
	//char *wgt_name[STITCH_MAX_SRC_NUM] = {STITCH_FULL_FILE_IN_WGT_ALPHA, STITCH_FULL_FILE_IN_WGT_BETA};
	char *filename_in[STITCH_MAX_SRC_NUM] = {STITCH_NONE_FILE_IN_LFT, STITCH_NONE_FILE_IN_RHT};
	char *filename_out = STITCH_NONE_FILE_OUT;
	char *filename_pef = STITCH_NONE_FILE_PEF;

	param.needPef = CVI_TRUE;
	param.needDumpReg = CVI_FALSE;
	param.srcNum = 2;
	param.srcAttr.size[0].u32Width = 1920;
	param.srcAttr.size[0].u32Height = 1080;
	param.srcAttr.size[1].u32Width = 1920;
	param.srcAttr.size[1].u32Height = 1080;

	param.chnAttr.size.u32Width = 3840;
	param.chnAttr.size.u32Height = 1080;

	param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_420;
	param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_420;

	param.srcAttr.way_num = STITCH_2_WAY;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 1920;//ovlap_attr from algo
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 1919;

	param.opAttr.data_src = STITCH_DATA_SRC_DDR;
	param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

	//param.wgtAttr.size_wgt[0].u32Width =
		//ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[0] - param.srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
	param.wgtAttr.size_wgt[0].u32Width = 0;
	param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

	strcpy(param.filename_out, filename_out);
	strcpy(param.filename_pef, filename_pef);
	for (i = 0; i < param.srcNum; i++) {
		//strcpy(param.wgt_name[i], wgt_name[i]);
		strcpy(param.filename_in[i], filename_in[i]);

		/*s32Ret = cfg_wgt_image(param.wgtAttr.size_wgt[0], param.opAttr.wgt_mode
			, param.wgt_name[i], &u64PhyAddr[i]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
			return s32Ret;
		}
		param.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
		*/
	}

	s32Ret = basic(&param, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		return s32Ret;
	}

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 stitch_test_none_ovlp1(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i, times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	//CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM];
	//char *wgt_name[STITCH_MAX_SRC_NUM] = {STITCH_FULL_FILE_IN_WGT_ALPHA, STITCH_FULL_FILE_IN_WGT_BETA};
	char *filename_in[STITCH_MAX_SRC_NUM] = {STITCH_NONE_FILE_IN_LFT1, STITCH_NONE_FILE_IN_RHT1};
	char *filename_out = STITCH_NONE_FILE_OUT1;
	char *filename_pef = STITCH_NONE_FILE_PEF1;

	param.needPef = CVI_TRUE;
	param.needDumpReg = CVI_FALSE;
	param.srcNum = 2;
	param.srcAttr.size[0].u32Width = 1536;
	param.srcAttr.size[0].u32Height = 288;
	param.srcAttr.size[1].u32Width = 1536;
	param.srcAttr.size[1].u32Height = 288;

	param.chnAttr.size.u32Width = 3072;
	param.chnAttr.size.u32Height = 288;

	param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_420;
	param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_420;

	param.srcAttr.way_num = STITCH_2_WAY;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 1536;//ovlap_attr from algo
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 1535;

	param.opAttr.data_src = STITCH_DATA_SRC_DDR;
	param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

	//param.wgtAttr.size_wgt[0].u32Width =
		//ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[0] - param.srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
	param.wgtAttr.size_wgt[0].u32Width = 0;
	param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

	strcpy(param.filename_out, filename_out);
	strcpy(param.filename_pef, filename_pef);
	for (i = 0; i < param.srcNum; i++) {
		//strcpy(param.wgt_name[i], wgt_name[i]);
		strcpy(param.filename_in[i], filename_in[i]);

		/*s32Ret = cfg_wgt_image(param.wgtAttr.size_wgt[0], param.opAttr.wgt_mode
			, param.wgt_name[i], &u64PhyAddr[i]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
			return s32Ret;
		}
		param.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
		*/
	}

	s32Ret = basic(&param, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		return s32Ret;
	}

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 stitch_test_lft_ovlp(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i, times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM] = {0};
	CVI_VOID *VirAddr[STITCH_MAX_SRC_NUM] = {0};
	char *wgt_name[STITCH_MAX_SRC_NUM] = {STITCH_LFT_OVLP_FILE_IN_WGT_ALPHA, STITCH_LFT_OVLP_FILE_IN_WGT_BETA};
	char *filename_in[STITCH_MAX_SRC_NUM] = {STITCH_LFT_OVLP_FILE_IN_LFT, STITCH_LFT_OVLP_FILE_IN_RHT};
	char *filename_out = STITCH_LFT_OVLP_FILE_OUT;
	char *filename_pef = STITCH_LFT_OVLP_FILE_PEF;

	param.needPef = CVI_TRUE;
	param.needDumpReg = CVI_FALSE;
	param.srcNum = 2;

#ifndef FPGA_PORTING
	param.srcAttr.size[0].u32Width = 2304;
	param.srcAttr.size[0].u32Height = 288;
	param.srcAttr.size[1].u32Width = 4608;
	param.srcAttr.size[1].u32Height = 288;

	param.chnAttr.size.u32Width = 4608;
	param.chnAttr.size.u32Height = 288;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 0;//ovlap_attr from algo
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 2303;
#else
	param.srcAttr.size[0].u32Width = 4608;
	param.srcAttr.size[0].u32Height = 288;
	param.srcAttr.size[1].u32Width = 6912;
	param.srcAttr.size[1].u32Height = 288;

	param.chnAttr.size.u32Width = 6912;
	param.chnAttr.size.u32Height = 288;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 0;//ovlap_attr from algo
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 4607;
#endif

	param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_420;
	param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_420;

	param.srcAttr.way_num = STITCH_2_WAY;

	param.opAttr.data_src = STITCH_DATA_SRC_DDR;
	param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

	param.wgtAttr.size_wgt[0].u32Width =
		ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[0] - param.srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
	param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

	strcpy(param.filename_out, filename_out);
	strcpy(param.filename_pef, filename_pef);
	for (i = 0; i < param.srcNum; i++) {
		strcpy(param.wgt_name[i], wgt_name[i]);
		strcpy(param.filename_in[i], filename_in[i]);

		s32Ret = cfg_wgt_image(param.wgtAttr.size_wgt[0], param.opAttr.wgt_mode
			, param.wgt_name[i], &u64PhyAddr[i], &VirAddr[i]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
			return s32Ret;
		}
		param.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
	}

	s32Ret = basic(&param, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		goto free;
	}

free:
	for (i = 0; i < param.srcNum; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
	}

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 stitch_test_rht_ovlp(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i, times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM] = {0};
	CVI_VOID *VirAddr[STITCH_MAX_SRC_NUM] = {0};
	char *wgt_name[STITCH_MAX_SRC_NUM] = {STITCH_RHT_OVLP_FILE_IN_WGT_ALPHA, STITCH_RHT_OVLP_FILE_IN_WGT_BETA};
	char *filename_in[STITCH_MAX_SRC_NUM] = {STITCH_RHT_OVLP_FILE_IN_LFT, STITCH_RHT_OVLP_FILE_IN_RHT};
	char *filename_out = STITCH_RHT_OVLP_FILE_OUT;
	char *filename_pef = STITCH_RHT_OVLP_FILE_PEF;

	param.needPef = CVI_TRUE;
	param.needDumpReg = CVI_FALSE;
	param.srcNum = 2;

#ifndef FPGA_PORTING
	param.srcAttr.size[0].u32Width = 4608;
	param.srcAttr.size[0].u32Height = 288;
	param.srcAttr.size[1].u32Width = 2304;
	param.srcAttr.size[1].u32Height = 288;

	param.chnAttr.size.u32Width = 4608;
	param.chnAttr.size.u32Height = 288;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 2304;//ovlap_attr from algo
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 4607;
#else
	param.srcAttr.size[0].u32Width = 6912;
	param.srcAttr.size[0].u32Height = 288;
	param.srcAttr.size[1].u32Width = 4608;
	param.srcAttr.size[1].u32Height = 288;

	param.chnAttr.size.u32Width = 6912;
	param.chnAttr.size.u32Height = 288;


	param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 2304;//ovlap_attr from algo
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 6911;
#endif
	param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_420;
	param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_420;

	param.srcAttr.way_num = STITCH_2_WAY;

	param.opAttr.data_src = STITCH_DATA_SRC_DDR;
	param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

	param.wgtAttr.size_wgt[0].u32Width =
		ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[0] - param.srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
	param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

	strcpy(param.filename_out, filename_out);
	strcpy(param.filename_pef, filename_pef);
	for (i = 0; i < param.srcNum; i++) {
		strcpy(param.wgt_name[i], wgt_name[i]);
		strcpy(param.filename_in[i], filename_in[i]);

		s32Ret = cfg_wgt_image(param.wgtAttr.size_wgt[0], param.opAttr.wgt_mode
			, param.wgt_name[i], &u64PhyAddr[i], &VirAddr[i]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
			return s32Ret;
		}
		param.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
	}

	s32Ret = basic(&param, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		goto free;
	}

free:
	for (i = 0; i < param.srcNum; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
	}

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 stitch_test_pure_color(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i, times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM] = {0};
	CVI_VOID *VirAddr[STITCH_MAX_SRC_NUM] = {0};
	char *wgt_name[STITCH_MAX_SRC_NUM] = {STITCH_PURE_COLOR_FILE_IN_WGT_ALPHA, STITCH_PURE_COLOR_FILE_IN_WGT_BETA};
	char *filename_in[STITCH_MAX_SRC_NUM] = {STITCH_PURE_COLOR_FILE_IN_LFT, STITCH_PURE_COLOR_FILE_IN_RHT};
	char *filename_out = STITCH_PURE_COLOR_FILE_OUT;
	char *filename_pef = STITCH_PURE_COLOR_FILE_PEF;

	param.needPef = CVI_TRUE;
	param.needDumpReg = CVI_FALSE;
	param.srcNum = 2;
	param.srcAttr.size[0].u32Width = 1536;
	param.srcAttr.size[0].u32Height = 384;
	param.srcAttr.size[1].u32Width = 1088;
	param.srcAttr.size[1].u32Height = 384;

	param.chnAttr.size.u32Width = 2400;
	param.chnAttr.size.u32Height = 384;

	param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_420;
	param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_420;

	param.srcAttr.way_num = STITCH_2_WAY;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 1312;//ovlap_attr from algo
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 1535;

	param.opAttr.data_src = STITCH_DATA_SRC_DDR;
	param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

	param.wgtAttr.size_wgt[0].u32Width =
		ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[0] - param.srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
	param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

	strcpy(param.filename_out, filename_out);
	strcpy(param.filename_pef, filename_pef);
	for (i = 0; i < param.srcNum; i++) {
		strcpy(param.wgt_name[i], wgt_name[i]);
		strcpy(param.filename_in[i], filename_in[i]);

		s32Ret = cfg_wgt_image(param.wgtAttr.size_wgt[0], param.opAttr.wgt_mode
			, param.wgt_name[i], &u64PhyAddr[i], &VirAddr[i]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
			return s32Ret;
		}
		param.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
	}

	s32Ret = basic(&param, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		goto free;
	}

free:
	for (i = 0; i < param.srcNum; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
	}

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 stitch_test_pure_color_mode1(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i, times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM] = {0};
	CVI_VOID *VirAddr[STITCH_MAX_SRC_NUM] = {0};
	char *wgt_name[STITCH_MAX_SRC_NUM] = {STITCH_PURE_COLOR_FILE_IN_WGT_ALPHA1, STITCH_PURE_COLOR_FILE_IN_WGT_BETA1};
	char *filename_in[STITCH_MAX_SRC_NUM] = {STITCH_PURE_COLOR_FILE_IN_LFT, STITCH_PURE_COLOR_FILE_IN_RHT};
	char *filename_out = STITCH_PURE_COLOR_FILE_OUT1;
	char *filename_pef = STITCH_PURE_COLOR_FILE_PEF1;

	param.needPef = CVI_TRUE;
	param.needDumpReg = CVI_FALSE;
	param.srcNum = 2;
	param.srcAttr.size[0].u32Width = 1536;
	param.srcAttr.size[0].u32Height = 384;
	param.srcAttr.size[1].u32Width = 1088;
	param.srcAttr.size[1].u32Height = 384;

	param.chnAttr.size.u32Width = 2400;
	param.chnAttr.size.u32Height = 384;

	param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_420;
	param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_420;

	param.srcAttr.way_num = STITCH_2_WAY;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 1312;//ovlap_attr from algo
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 1535;

	param.opAttr.data_src = STITCH_DATA_SRC_DDR;
	param.opAttr.wgt_mode = STITCH_WGT_UV_SHARE;

	param.wgtAttr.size_wgt[0].u32Width =
		ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[0] - param.srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
	param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

	strcpy(param.filename_out, filename_out);
	strcpy(param.filename_pef, filename_pef);
	for (i = 0; i < param.srcNum; i++) {
		strcpy(param.wgt_name[i], wgt_name[i]);
		strcpy(param.filename_in[i], filename_in[i]);

		s32Ret = cfg_wgt_image(param.wgtAttr.size_wgt[0], param.opAttr.wgt_mode
			, param.wgt_name[i], &u64PhyAddr[i], &VirAddr[i]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
			return s32Ret;
		}
		param.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
	}

	s32Ret = basic(&param, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		goto free;
	}

free:
	for (i = 0; i < param.srcNum; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
	}

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 stitch_test_fmt_400(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i, times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM] = {0};
	CVI_VOID *VirAddr[STITCH_MAX_SRC_NUM] = {0};
	char *wgt_name[STITCH_MAX_SRC_NUM] = {STITCH_FILE_IN_WGT_ALPHA, STITCH_FILE_IN_WGT_BETA};
	char *filename_in[STITCH_MAX_SRC_NUM] = {STITCH_FILE_IN_LFT, STITCH_FILE_IN_RHT};//same as basic, only read 1 planner
	char *filename_out = STITCH_FILE_OUT_400;
	char *filename_pef = STITCH_FILE_PEF_400;

	param.needPef = CVI_TRUE;
	param.needDumpReg = CVI_FALSE;
	param.srcNum = 2;
	param.srcAttr.size[0].u32Width = 4608;
	param.srcAttr.size[0].u32Height = 288;
	param.srcAttr.size[1].u32Width = 4608;
	param.srcAttr.size[1].u32Height = 288;

	param.chnAttr.size.u32Width = 6912;
	param.chnAttr.size.u32Height = 288;

	param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_400;
	param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_400;

	param.srcAttr.way_num = STITCH_2_WAY;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 2304;//ovlap_attr from algo
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 4607;

	param.opAttr.data_src = STITCH_DATA_SRC_DDR;
	param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

	param.wgtAttr.size_wgt[0].u32Width =
		ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[0] - param.srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
	param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

	strcpy(param.filename_out, filename_out);
	strcpy(param.filename_pef, filename_pef);
	for (i = 0; i < param.srcNum; i++) {
		strcpy(param.wgt_name[i], wgt_name[i]);
		strcpy(param.filename_in[i], filename_in[i]);

		s32Ret = cfg_wgt_image(param.wgtAttr.size_wgt[0], param.opAttr.wgt_mode
			, param.wgt_name[i], &u64PhyAddr[i], &VirAddr[i]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
			return s32Ret;
		}
		param.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
	}

	s32Ret = basic(&param, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		goto free;
	}

free:
	for (i = 0; i < param.srcNum; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
	}

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 stitch_test_fmt_422p(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i, times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM] = {0};
	CVI_VOID *VirAddr[STITCH_MAX_SRC_NUM] = {0};
	char *wgt_name[STITCH_MAX_SRC_NUM] = {STITCH_FILE_IN_WGT_ALPHA, STITCH_FILE_IN_WGT_BETA};
	char *filename_in[STITCH_MAX_SRC_NUM] = {STITCH_FILE_IN_LFT_422, STITCH_FILE_IN_RHT_422};
	char *filename_out = STITCH_FILE_OUT_422;
	char *filename_pef = STITCH_FILE_PEF_422;

	param.needPef = CVI_TRUE;
	param.needDumpReg = CVI_FALSE;
	param.srcNum = 2;
	param.srcAttr.size[0].u32Width = 4608;
	param.srcAttr.size[0].u32Height = 288;
	param.srcAttr.size[1].u32Width = 4608;
	param.srcAttr.size[1].u32Height = 288;

	param.chnAttr.size.u32Width = 6912;
	param.chnAttr.size.u32Height = 288;

	param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_422;
	param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_422;

	param.srcAttr.way_num = STITCH_2_WAY;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 2304;//ovlap_attr from algo
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 4607;

	param.opAttr.data_src = STITCH_DATA_SRC_DDR;
	param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

	param.wgtAttr.size_wgt[0].u32Width =
		ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[0] - param.srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
	param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

	strcpy(param.filename_out, filename_out);
	strcpy(param.filename_pef, filename_pef);
	for (i = 0; i < param.srcNum; i++) {
		strcpy(param.wgt_name[i], wgt_name[i]);
		strcpy(param.filename_in[i], filename_in[i]);

		s32Ret = cfg_wgt_image(param.wgtAttr.size_wgt[0], param.opAttr.wgt_mode
			, param.wgt_name[i], &u64PhyAddr[i], &VirAddr[i]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
			return s32Ret;
		}
		param.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
	}

	s32Ret = basic(&param, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		goto free;
	}

free:
	for (i = 0; i < param.srcNum; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
	}

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 stitch_test_fmt_444p(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i, times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM] = {0};
	CVI_VOID *VirAddr[STITCH_MAX_SRC_NUM] = {0};
	char *wgt_name[STITCH_MAX_SRC_NUM] = {STITCH_FILE_IN_WGT_ALPHA, STITCH_FILE_IN_WGT_BETA};
	char *filename_in[STITCH_MAX_SRC_NUM] = {STITCH_FILE_IN_LFT_444, STITCH_FILE_IN_RHT_444};
	char *filename_out = STITCH_FILE_OUT_444;
	char *filename_pef = STITCH_FILE_PEF_444;

	param.needPef = CVI_TRUE;
	param.needDumpReg = CVI_FALSE;
	param.srcNum = 2;
	param.srcAttr.size[0].u32Width = 4608;
	param.srcAttr.size[0].u32Height = 288;
	param.srcAttr.size[1].u32Width = 4608;
	param.srcAttr.size[1].u32Height = 288;

	param.chnAttr.size.u32Width = 6912;
	param.chnAttr.size.u32Height = 288;

	param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_444;
	param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_444;

	param.srcAttr.way_num = STITCH_2_WAY;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 2304;//ovlap_attr from algo
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 4607;

	param.opAttr.data_src = STITCH_DATA_SRC_DDR;
	param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

	param.wgtAttr.size_wgt[0].u32Width =
		ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[0] - param.srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
	param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

	strcpy(param.filename_out, filename_out);
	strcpy(param.filename_pef, filename_pef);
	for (i = 0; i < param.srcNum; i++) {
		strcpy(param.wgt_name[i], wgt_name[i]);
		strcpy(param.filename_in[i], filename_in[i]);

		s32Ret = cfg_wgt_image(param.wgtAttr.size_wgt[0], param.opAttr.wgt_mode
			, param.wgt_name[i], &u64PhyAddr[i], &VirAddr[i]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
			return s32Ret;
		}
		param.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
	}

	s32Ret = basic(&param, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		goto free;
	}

free:
	for (i = 0; i < param.srcNum; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
	}

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 stitch_test_fmt_rgb888(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i, times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	char *filename_in[STITCH_MAX_SRC_NUM] = {STITCH_FILE_IN_LFT_RGB, STITCH_FILE_IN_RHT_RGB};
	char *filename_out = STITCH_FILE_OUT_RGB;

	param.needPef = CVI_FALSE;
	param.needDumpReg = CVI_FALSE;
	param.srcNum = 2;
	param.srcAttr.size[0].u32Width = 1920;
	param.srcAttr.size[0].u32Height = 1080;
	param.srcAttr.size[1].u32Width = 1920;
	param.srcAttr.size[1].u32Height = 1080;

	param.chnAttr.size.u32Width = 3840;
	param.chnAttr.size.u32Height = 1080;

	param.srcAttr.fmt_in = PIXEL_FORMAT_RGB_888_PLANAR;
	param.chnAttr.fmt_out = PIXEL_FORMAT_RGB_888_PLANAR;

	param.srcAttr.way_num = STITCH_2_WAY;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 1920;//ovlap_attr from algo
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 1919;

	param.opAttr.data_src = STITCH_DATA_SRC_DDR;
	param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

	param.wgtAttr.size_wgt[0].u32Width = 0;
	param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

	strcpy(param.filename_out, filename_out);
	for (i = 0; i < param.srcNum; i++) {
		strcpy(param.filename_in[i], filename_in[i]);
	}

	s32Ret = basic(&param, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		return s32Ret;
	}

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 stitch_test_wgt_mode1(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i, times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM] = {0};
	CVI_VOID *VirAddr[STITCH_MAX_SRC_NUM] = {0};
	char *wgt_name[STITCH_MAX_SRC_NUM] = {STITCH_FILE_IN_WGT_ALPHA1, STITCH_FILE_IN_WGT_BETA1};
	char *filename_in[STITCH_MAX_SRC_NUM] = {STITCH_FILE_IN_LFT, STITCH_FILE_IN_RHT};
	char *filename_out = STITCH_FILE_OUT1;
	char *filename_pef = STITCH_FILE_PEF1;

	param.needPef = CVI_TRUE;
	param.needDumpReg = CVI_FALSE;
	param.srcNum = 2;
	param.srcAttr.size[0].u32Width = 4608;
	param.srcAttr.size[0].u32Height = 288;
	param.srcAttr.size[1].u32Width = 4608;
	param.srcAttr.size[1].u32Height = 288;

	param.chnAttr.size.u32Width = 6912;
	param.chnAttr.size.u32Height = 288;

	param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_420;
	param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_420;

	param.srcAttr.way_num = STITCH_2_WAY;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 2304;//ovlap_attr from algo
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 4607;

	param.opAttr.data_src = STITCH_DATA_SRC_DDR;
	param.opAttr.wgt_mode = STITCH_WGT_UV_SHARE;

	param.wgtAttr.size_wgt[0].u32Width =
		ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[0] - param.srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
	param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

	strcpy(param.filename_out, filename_out);
	strcpy(param.filename_pef, filename_pef);
	for (i = 0; i < param.srcNum; i++) {
		strcpy(param.wgt_name[i], wgt_name[i]);
		strcpy(param.filename_in[i], filename_in[i]);

		s32Ret = cfg_wgt_image(param.wgtAttr.size_wgt[0], param.opAttr.wgt_mode
			, param.wgt_name[i], &u64PhyAddr[i], &VirAddr[i]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
			return s32Ret;
		}
		param.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
	}

	s32Ret = basic(&param, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		goto free;
	}

free:
	for (i = 0; i < param.srcNum; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
	}

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

#ifndef FPGA_PORTING
static CVI_S32 basic2(STITCH_BASIC_TEST_PARAM *pParam, CVI_U8 times)
{
	CVI_U32 i;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 timout_ms = STITCH_TIMEOUT;
	UNUSED(times);

	/************************************************
	 * step2:  Init STITCH
	 ************************************************/
	s32Ret = CVI_STITCH_Init();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_STITCH_Reset();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_Reset failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetSrcAttr(&pParam->srcAttr);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetSrcAttr failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetChnAttr(&pParam->chnAttr);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetChnAttr failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetOpAttr(&pParam->opAttr);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetOpAttr failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetWgtAttr(&pParam->wgtAttr);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetWgtAttr failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetRegX(16);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetRegX failed!\n");
		goto exit2;
	}

	/*start stitch*/
	s32Ret = CVI_STITCH_EnableDev();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_EnableDev failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_AttachVbPool((VB_POOL)pParam->srcNum);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_AttachVbPool failed!\n");
		goto exit2;
	}

	//send frame
	for (i = 0; i < 2; i++) {
		STITCH_UT_PRT("start send src frame[%d], CurSrcID[%d]\n", i, pParam->CurSrcID[i]);
		s32Ret = FileSendToStitch((STITCH_SRC_IDX)pParam->CurSrcID[i], &pParam->srcAttr.size[i], pParam->srcAttr.fmt_in, pParam->filename_in[i]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("FileSendToStitch[%d] fail, s32Ret: 0x%x !\n", i, s32Ret);
			goto exit3;
		}
	}

	memset(&pParam->stVideoFrameOut, 0, sizeof(pParam->stVideoFrameOut));
	s32Ret = CVI_STITCH_GetChnFrame(&pParam->stVideoFrameOut, timout_ms);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
		pParam->needDumpReg = CVI_TRUE;
		goto exit3;
	}

	STITCH_UT_PRT("***CVI_STITCH_GetChnFrame Success, start save and compare with golden***\n");
	STITCH_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", pParam->stVideoFrameOut.stVFrame.u64PhyAddr[0]
		, pParam->stVideoFrameOut.stVFrame.u64PhyAddr[1], pParam->stVideoFrameOut.stVFrame.u64PhyAddr[2]);

	if (g_stitch_save_file) {
		if (strlen(pParam->filename_out)) {
			s32Ret = FrameSaveToFile(pParam->filename_out, &pParam->stVideoFrameOut);
			if (s32Ret != CVI_SUCCESS) {
				STITCH_UT_PRT("FrameSaveToFile. s32Ret: 0x%x !\n", s32Ret);
				CVI_STITCH_ReleaseChnFrame(&pParam->stVideoFrameOut);
				goto exit3;
			}
			STITCH_UT_PRT("output file:%s\n", pParam->filename_out);
		}

		if (pParam->needPef) {
			s32Ret = CompareWithFile(pParam->filename_pef, &pParam->stVideoFrameOut);
			if (s32Ret != CVI_SUCCESS) {
				STITCH_UT_PRT("CompareWithFile fail.\n");
				CVI_STITCH_ReleaseChnFrame(&pParam->stVideoFrameOut);
				goto exit3;
			}
		}
	}

	if (pParam->CurSrcID[1] == 3) {//last job in send can release chn frm.
		s32Ret = CVI_STITCH_ReleaseChnFrame(&pParam->stVideoFrameOut);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("CVI_STITCH_ReleaseChnFrame fail s32Ret: 0x%x !\n", s32Ret);
			goto exit3;
		}
	}
exit3:
	if (pParam->needDumpReg)
		CVI_STITCH_DumpRegInfo();

	CVI_STITCH_DisableDev();

	if (g_stitch_need_rst)
		CVI_STITCH_Reset();
exit2:
	CVI_STITCH_DeInit();

	return s32Ret;
}

static CVI_S32 stitch_test_4way_fake(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i= 0, j = 0, times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM] = {0};
	CVI_VOID *VirAddr[STITCH_MAX_SRC_NUM] = {0};
	char *wgt_name[3][2] = {
		{STITCH_FILE_IN_WGT_ALPHA_12, STITCH_FILE_IN_WGT_BETA_12},
		{STITCH_FILE_IN_WGT_ALPHA_23, STITCH_FILE_IN_WGT_BETA_23},
		{STITCH_FILE_IN_WGT_ALPHA_34, STITCH_FILE_IN_WGT_BETA_34},
	};
	char *filename_in[3][2] = {
		{STITCH_FILE_IN_1, STITCH_FILE_IN_2},
		{STITCH_FILE_IN_2, STITCH_FILE_IN_3},
		{STITCH_FILE_IN_3, STITCH_FILE_IN_4},
	};
	char *filename_out[3] = {STITCH_FILE_OUT12, STITCH_FILE_OUT123, STITCH_FILE_OUT1234};
	char *filename_pef[3] = {STITCH_FILE_PEF12, STITCH_FILE_PEF123, STITCH_FILE_PEF1234};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize[STITCH_MAX_SRC_NUM + 1];

	param.srcNum = 4;
	param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_420;
	param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_420;
	param.srcAttr.way_num = STITCH_2_WAY;

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	for (i = 0; i < param.srcNum; i++) {
		u32BlkSize[i] = COMMON_GetPicBufferSize(4608, 288, param.srcAttr.fmt_in
			, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, STITCH_ALIGN);
	}

	u32BlkSize[i] = COMMON_GetPicBufferSize(11520, 288, param.chnAttr.fmt_out
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, STITCH_ALIGN);

	stVbConf.u32MaxPoolCnt				= param.srcNum + 1;

	for (i = 0; i < (int)stVbConf.u32MaxPoolCnt; i++) {
		stVbConf.astCommPool[i].u32BlkSize	= u32BlkSize[i];
		stVbConf.astCommPool[i].u32BlkCnt	= 1;
		stVbConf.astCommPool[i].enRemapMode = VB_REMAP_MODE_CACHED;
		STITCH_UT_PRT("common pool[%d] BlkSize %d\n", i, stVbConf.astCommPool[i].u32BlkSize);
	}

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	for (j = 0; j < 1; j++) {//4 way stitch do 3 times
		param.needPef = CVI_FALSE;
		param.needDumpReg = CVI_FALSE;
		param.srcNum = 4;
		param.CurSrcID[0] = j;
		param.CurSrcID[1] = j + 1;
		param.srcAttr.size[j].u32Width = 4608;
		param.srcAttr.size[j].u32Height = 288;
		param.srcAttr.size[j+1].u32Width = 4608;
		param.srcAttr.size[j+1].u32Height = 288;

		param.chnAttr.size.u32Width = 6912;
		param.chnAttr.size.u32Height = 288;

		param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_420;
		param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_420;

		param.srcAttr.way_num = STITCH_2_WAY;

		param.srcAttr.bd_attr.bd_lx[j] = 0;//left img, bd_attr from algo
		param.srcAttr.bd_attr.bd_rx[j] = 0;
		param.srcAttr.bd_attr.bd_lx[j+1] = 0;//right img, bd_attr from algo
		param.srcAttr.bd_attr.bd_rx[j+1] = 0;

		param.srcAttr.ovlap_attr.ovlp_lx[j] = 2304;//ovlap_attr from algo
		param.srcAttr.ovlap_attr.ovlp_rx[j] = 4607;

		param.opAttr.data_src = STITCH_DATA_SRC_DDR;
		param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

		param.wgtAttr.size_wgt[0].u32Width =
			ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[0] - param.srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
		param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

		strcpy(param.filename_out, filename_out[j]);
		strcpy(param.filename_pef, filename_pef[j]);
		for (i = 0; i < 2; i++) {
			strcpy(param.wgt_name[i], wgt_name[j][i]);
			strcpy(param.filename_in[i], filename_in[j][i]);

			s32Ret = cfg_wgt_image(param.wgtAttr.size_wgt[0], param.opAttr.wgt_mode
				, param.wgt_name[i], &u64PhyAddr[i], &VirAddr[i]);
			if (s32Ret != CVI_SUCCESS) {
				STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
				goto exit1;
			}
			param.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
		}

		s32Ret = basic2(&param, times);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("Test failed.\n");
			goto exit1;
		}

		for (i = 0; i < 2; i++) {
			if (u64PhyAddr[i] && VirAddr[i])
				CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
		}
	}

exit1:
	for (i = 0; i < 2; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
		}
	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}
#else //use big sram when fpga
static void stitch_reconfig_param_stage2(STITCH_BASIC_TEST_PARAM *param)
{
	param->srcAttr.size[0].u32Width = 6912;
	param->srcAttr.size[0].u32Height = 288;
	param->srcAttr.size[1].u32Width = 6912;
	param->srcAttr.size[1].u32Height = 288;

	param->chnAttr.size.u32Width = 11520;
	param->chnAttr.size.u32Height = 288;

	param->srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param->srcAttr.bd_attr.bd_rx[0] = 0;
	param->srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param->srcAttr.bd_attr.bd_rx[1] = 0;

	param->srcAttr.ovlap_attr.ovlp_lx[0] = 4608;//ovlap_attr from algo
	param->srcAttr.ovlap_attr.ovlp_rx[0] = 6911;
}

static CVI_S32 stitch_test_4way_fake(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i= 0, j = 0, times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM] = {0};
	CVI_VOID *VirAddr[STITCH_MAX_SRC_NUM] = {0};
	char *wgt_name[3][2] = {
		{STITCH_FILE_IN_WGT_ALPHA_12, STITCH_FILE_IN_WGT_BETA_12},
		{STITCH_FILE_IN_WGT_ALPHA_34, STITCH_FILE_IN_WGT_BETA_34},
		{STITCH_FILE_IN_WGT_ALPHA_23, STITCH_FILE_IN_WGT_BETA_23},
	};
	char *filename_in[3][2] = {
		{STITCH_FILE_IN_1, STITCH_FILE_IN_2},
		{STITCH_FILE_IN_3, STITCH_FILE_IN_4},
		{STITCH_FILE_OUT12, STITCH_FILE_OUT34},
	};
	char *filename_out[3] = {STITCH_FILE_OUT12, STITCH_FILE_OUT34, STITCH_FILE_OUT1234};
	char *filename_pef[3] = {STITCH_FILE_PEF12, STITCH_FILE_PEF34, STITCH_FILE_PEF1234};

	for (j = 0; j < 3; j++) {
		param.needPef = CVI_TRUE;
		param.needDumpReg = CVI_FALSE;
		param.srcNum = 2;
		param.srcAttr.size[0].u32Width = 4608;
		param.srcAttr.size[0].u32Height = 288;
		param.srcAttr.size[1].u32Width = 4608;
		param.srcAttr.size[1].u32Height = 288;

		param.chnAttr.size.u32Width = 6912;
		param.chnAttr.size.u32Height = 288;

		param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_420;
		param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_420;

		param.srcAttr.way_num = STITCH_2_WAY;

		param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
		param.srcAttr.bd_attr.bd_rx[0] = 0;
		param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
		param.srcAttr.bd_attr.bd_rx[1] = 0;

		param.srcAttr.ovlap_attr.ovlp_lx[0] = 2304;//ovlap_attr from algo
		param.srcAttr.ovlap_attr.ovlp_rx[0] = 4607;

		if (j == 2)
			stitch_reconfig_param_stage2(&param);

		param.opAttr.data_src = STITCH_DATA_SRC_DDR;
		param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

		param.wgtAttr.size_wgt[0].u32Width =
			ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[0] - param.srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
		param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

		strcpy(param.filename_out, filename_out[j]);
		strcpy(param.filename_pef, filename_pef[j]);
		for (i = 0; i < param.srcNum; i++) {
			strcpy(param.wgt_name[i], wgt_name[j][i]);
			strcpy(param.filename_in[i], filename_in[j][i]);

			s32Ret = cfg_wgt_image(param.wgtAttr.size_wgt[0], param.opAttr.wgt_mode
				, param.wgt_name[i], &u64PhyAddr[i], &VirAddr[i]);
			if (s32Ret != CVI_SUCCESS) {
				STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
				return s32Ret;
			}
			param.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
		}

		s32Ret = basic(&param, times);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("Test failed.\n");
			goto free;
		}

		for (i = 0; i < param.srcNum; i++) {
			if (u64PhyAddr[i] && VirAddr[i])
				CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
		}
	}

free:
	for (i = 0; i < param.srcNum; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
	}

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}
#endif

static void stitch_reconfig_param_stage2_middle(STITCH_BASIC_TEST_PARAM *param)
{
	param->srcAttr.size[0].u32Width = 1536;
	param->srcAttr.size[0].u32Height = 1024;
	param->srcAttr.size[1].u32Width = 1536;
	param->srcAttr.size[1].u32Height = 1024;

	param->chnAttr.size.u32Width = 2560;
	param->chnAttr.size.u32Height = 1024;

	param->srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param->srcAttr.bd_attr.bd_rx[0] = 0;
	param->srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param->srcAttr.bd_attr.bd_rx[1] = 0;

	param->srcAttr.ovlap_attr.ovlp_lx[0] = 1024;//ovlap_attr from algo
	param->srcAttr.ovlap_attr.ovlp_rx[0] = 1535;
}

static CVI_S32 stitch_test_4way_fake_middle(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i= 0, j = 0, times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM] = {0};
	CVI_VOID *VirAddr[STITCH_MAX_SRC_NUM] = {0};
	char *wgt_name[3][2] = {
		{STITCH_MIDDLE_SIZE_FILE_IN_WGT_ALPHA, STITCH_MIDDLE_SIZE_FILE_IN_WGT_BETA},
		{STITCH_MIDDLE_SIZE_FILE_IN_WGT_ALPHA, STITCH_MIDDLE_SIZE_FILE_IN_WGT_BETA},
		{STITCH_MIDDLE_SIZE_FILE_IN_WGT_ALPHA, STITCH_MIDDLE_SIZE_FILE_IN_WGT_BETA},
	};
	char *filename_in[3][2] = {
		{STITCH_MIDDLE_SIZE_FILE_IN_LFT, STITCH_MIDDLE_SIZE_FILE_IN_RHT},
		{STITCH_MIDDLE_SIZE_FILE_IN_LFT, STITCH_MIDDLE_SIZE_FILE_IN_RHT},
		{STITCH_MIDDLE_SIZE_FILE_PEF, STITCH_MIDDLE_SIZE_FILE_PEF},
	};
	char *filename_out[3] = {STITCH_MIDDLE_SIZE_FILE_OUT, STITCH_MIDDLE_SIZE_FILE_OUT, STITCH_MIDDLE_SIZE_FILE_OUT2};
	char *filename_pef[3] = {STITCH_MIDDLE_SIZE_FILE_PEF, STITCH_MIDDLE_SIZE_FILE_PEF, STITCH_MIDDLE_SIZE_FILE_PEF2};

	for (j = 0; j < 3; j++) {
		param.needPef = CVI_TRUE;
		param.needDumpReg = CVI_FALSE;
		param.srcNum = 2;
		param.srcAttr.size[0].u32Width = 1024;
		param.srcAttr.size[0].u32Height = 1024;
		param.srcAttr.size[1].u32Width = 1024;
		param.srcAttr.size[1].u32Height = 1024;

		param.chnAttr.size.u32Width = 1536;
		param.chnAttr.size.u32Height = 1024;

		param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_420;
		param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_420;

		param.srcAttr.way_num = STITCH_2_WAY;

		param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
		param.srcAttr.bd_attr.bd_rx[0] = 0;
		param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
		param.srcAttr.bd_attr.bd_rx[1] = 0;

		param.srcAttr.ovlap_attr.ovlp_lx[0] = 512;//ovlap_attr from algo
		param.srcAttr.ovlap_attr.ovlp_rx[0] = 1023;

		if (j == 2)
			stitch_reconfig_param_stage2_middle(&param);

		param.opAttr.data_src = STITCH_DATA_SRC_DDR;
		param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

		param.wgtAttr.size_wgt[0].u32Width =
			ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[0] - param.srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
		param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

		strcpy(param.filename_out, filename_out[j]);
		strcpy(param.filename_pef, filename_pef[j]);
		for (i = 0; i < param.srcNum; i++) {
			strcpy(param.wgt_name[i], wgt_name[j][i]);
			strcpy(param.filename_in[i], filename_in[j][i]);

			s32Ret = cfg_wgt_image(param.wgtAttr.size_wgt[0], param.opAttr.wgt_mode
				, param.wgt_name[i], &u64PhyAddr[i], &VirAddr[i]);
			if (s32Ret != CVI_SUCCESS) {
				STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
				return s32Ret;
			}
			param.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
		}

		s32Ret = basic(&param, times);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("Test failed.\n");
			goto free;
		}

		for (i = 0; i < param.srcNum; i++) {
			if (u64PhyAddr[i] && VirAddr[i])
				CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
		}
	}

free:
	for (i = 0; i < param.srcNum; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
	}

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 basic3(STITCH_BASIC_TEST_PARAM *pParam, CVI_U8 times)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 timout_ms = STITCH_TIMEOUT;
	VB_POOL chn_pool_id = 1;
	UNUSED(times);

	s32Ret = CVI_STITCH_Init();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_STITCH_Reset();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_Reset failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetSrcAttr(&pParam->srcAttr);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetSrcAttr failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetChnAttr(&pParam->chnAttr);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetChnAttr failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetOpAttr(&pParam->opAttr);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetOpAttr failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetWgtAttr(&pParam->wgtAttr);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetWgtAttr failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetRegX(16);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetRegX failed!\n");
		goto exit2;
	}

	/*start stitch*/
	s32Ret = CVI_STITCH_EnableDev();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_EnableDev failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_AttachVbPool(chn_pool_id);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_AttachVbPool failed!\n");
		goto exit2;
	}

	do {
		//send frame
		for (int i = 0; i < pParam->srcNum; i++) {
		//for (int i = 0; i < 2; i++) {
			STITCH_UT_PRT("start send src frame[%d]\n", i);

			s32Ret = FileSendToStitch2((STITCH_SRC_IDX)i, &pParam->srcAttr.size[i], pParam->srcAttr.fmt_in, pParam->filename_in[i]);
			if (s32Ret != CVI_SUCCESS) {
				STITCH_UT_PRT("FileSendToStitch[%d] fail, s32Ret: 0x%x !\n", i, s32Ret);
				goto exit3;
			}
			//sleep(5);
		}



		memset(&pParam->stVideoFrameOut, 0, sizeof(pParam->stVideoFrameOut));
		s32Ret = CVI_STITCH_GetChnFrame(&pParam->stVideoFrameOut, timout_ms*10);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("CVI_STITCH_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
			//pParam->needDumpReg = CVI_TRUE;
			goto exit3;
		}

		STITCH_UT_PRT("***CVI_STITCH_GetChnFrame Success, start save and compare with golden***\n");
		STITCH_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", pParam->stVideoFrameOut.stVFrame.u64PhyAddr[0]
			, pParam->stVideoFrameOut.stVFrame.u64PhyAddr[1], pParam->stVideoFrameOut.stVFrame.u64PhyAddr[2]);

		if (g_stitch_save_file) {
			if (strlen(pParam->filename_out)) {
				s32Ret = FrameSaveToFile(pParam->filename_out, &pParam->stVideoFrameOut);
				if (s32Ret != CVI_SUCCESS) {
					STITCH_UT_PRT("FrameSaveToFile. s32Ret: 0x%x !\n", s32Ret);
					CVI_STITCH_ReleaseChnFrame(&pParam->stVideoFrameOut);
					goto exit3;
				}
				STITCH_UT_PRT("output file:%s\n", pParam->filename_out);
			}

			if (pParam->needPef) {
				s32Ret = CompareWithFile(pParam->filename_pef, &pParam->stVideoFrameOut);
				if (s32Ret != CVI_SUCCESS) {
					STITCH_UT_PRT("CompareWithFile fail.\n");
					CVI_STITCH_ReleaseChnFrame(&pParam->stVideoFrameOut);
					goto exit3;
				}
			}
		}

		s32Ret = CVI_STITCH_ReleaseChnFrame(&pParam->stVideoFrameOut);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("CVI_STITCH_ReleaseChnFrame fail s32Ret: 0x%x !\n", s32Ret);
			goto exit3;
		}
	} while (times--);
exit3:
	if (pParam->needDumpReg)
		CVI_STITCH_DumpRegInfo();

	CVI_STITCH_DisableDev();

if (g_stitch_need_rst)
	CVI_STITCH_Reset();
exit2:
	CVI_STITCH_DeInit();
	return s32Ret;
}

static CVI_S32 stitch_test_4way(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	CVI_U64 u64PhyAddr[6] = {0};
	CVI_VOID *VirAddr[6] = {0};
	char *wgt_name[6] = {
		STITCH_FILE_IN_WGT_ALPHA_12, STITCH_FILE_IN_WGT_BETA_12,
		STITCH_FILE_IN_WGT_ALPHA_23, STITCH_FILE_IN_WGT_BETA_23,
		STITCH_FILE_IN_WGT_ALPHA_34, STITCH_FILE_IN_WGT_BETA_34,
	};
	char *filename_in[4] = {
		STITCH_FILE_IN_1, STITCH_FILE_IN_2,
		STITCH_FILE_IN_3, STITCH_FILE_IN_4,
	};
	char *filename_out = {STITCH_FILE_OUT1234_REAL};
	char *filename_pef = {STITCH_FILE_PEF1234_REAL};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize[STITCH_MAX_SRC_NUM + 1];

	param.needPef = CVI_FALSE;
	param.needDumpReg = CVI_FALSE;
	param.srcNum = 4;
	param.srcAttr.size[0].u32Width = 4608;
	param.srcAttr.size[0].u32Height = 288;
	param.srcAttr.size[1].u32Width = 4608;
	param.srcAttr.size[1].u32Height = 288;
	param.srcAttr.size[2].u32Width = 4608;
	param.srcAttr.size[2].u32Height = 288;
	param.srcAttr.size[3].u32Width = 4608;
	param.srcAttr.size[3].u32Height = 288;

	param.chnAttr.size.u32Width = 11520;
	param.chnAttr.size.u32Height = 288;

	param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_420;
	param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_420;

	param.srcAttr.way_num = STITCH_4_WAY;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//img1, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//img2, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;
	param.srcAttr.bd_attr.bd_lx[2] = 0;//img3, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[2] = 0;
	param.srcAttr.bd_attr.bd_lx[3] = 0;//img4, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[3] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 2304;//ovlap_attr img12
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 4607;
	param.srcAttr.ovlap_attr.ovlp_lx[1] = 4608;//ovlap_attr img23
	param.srcAttr.ovlap_attr.ovlp_rx[1] = 6911;
	param.srcAttr.ovlap_attr.ovlp_lx[2] = 6912;//ovlap_attr img34
	param.srcAttr.ovlap_attr.ovlp_rx[2] = 9215;

	param.opAttr.data_src = STITCH_DATA_SRC_DDR;
	param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSize[0] = COMMON_GetPicBufferSize(4608, 288, param.srcAttr.fmt_in
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, STITCH_ALIGN);

	u32BlkSize[1] = COMMON_GetPicBufferSize(11520, 288, param.chnAttr.fmt_out
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, STITCH_ALIGN);

	stVbConf.u32MaxPoolCnt				= 2;

	for (int i = 0; i < (int)stVbConf.u32MaxPoolCnt; i++) {
		stVbConf.astCommPool[i].u32BlkSize	= u32BlkSize[i];
		stVbConf.astCommPool[i].u32BlkCnt	= (i == 0) ? 4 : 1;
		stVbConf.astCommPool[i].enRemapMode = VB_REMAP_MODE_CACHED;
		STITCH_UT_PRT("common pool[%d] BlkSize %d\n", i, stVbConf.astCommPool[i].u32BlkSize);
	}

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	strcpy(param.filename_out, filename_out);
	strcpy(param.filename_pef, filename_pef);

	for (int i = 0; i < 3; i++) {
		param.wgtAttr.size_wgt[i].u32Width =
			ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[i] - param.srcAttr.ovlap_attr.ovlp_lx[i] + 1, STITCH_ALIGN);
		param.wgtAttr.size_wgt[i].u32Height = param.srcAttr.size[i].u32Height;
	}

	for (int i = 0, j = 0; i < 6; i++, j = i >> 1) {
		strcpy(param.wgt_name[i], wgt_name[i]);
		s32Ret = cfg_wgt_image(param.wgtAttr.size_wgt[j], param.opAttr.wgt_mode
			, param.wgt_name[i], &u64PhyAddr[i], &VirAddr[i]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
			goto exit1;
		}
		if (i % 2)
			param.wgtAttr.phy_addr_wgt[j][1] = (__u64)u64PhyAddr[i];
		else
			param.wgtAttr.phy_addr_wgt[j][0] = (__u64)u64PhyAddr[i];
	}

	for (int i = 0; i < 4; i++)
		strcpy(param.filename_in[i], filename_in[i]);

	s32Ret = basic3(&param, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		goto exit1;
	}

exit1:
	for (int i = 0; i < 6; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
	}

	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 stitch_test_4way_middle(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	CVI_U64 u64PhyAddr[6] = {0};
	CVI_VOID *VirAddr[6] = {0};
	char *wgt_name[6] = {
		STITCH_MIDDLE_SIZE_FILE_IN_WGT_ALPHA_1P4, STITCH_MIDDLE_SIZE_FILE_IN_WGT_BETA_1P4,
		STITCH_MIDDLE_SIZE_FILE_IN_WGT_ALPHA, STITCH_MIDDLE_SIZE_FILE_IN_WGT_BETA,
		STITCH_MIDDLE_SIZE_FILE_IN_WGT_ALPHA_1P4, STITCH_MIDDLE_SIZE_FILE_IN_WGT_BETA_1P4,
	};
	char *filename_in[4] = {
		STITCH_MIDDLE_SIZE_FILE_IN_LFT, STITCH_MIDDLE_SIZE_FILE_IN_RHT,
		STITCH_MIDDLE_SIZE_FILE_IN_LFT, STITCH_MIDDLE_SIZE_FILE_IN_RHT,
	};
	char *filename_out = {STITCH_FILE_OUT1234_REAL_MIDDLE};
	char *filename_pef = {STITCH_FILE_PEF1234_REAL_MIDDLE};
	VB_CONFIG_S stVbConf;
	CVI_U32 u32BlkSize[STITCH_MAX_SRC_NUM + 1];

	param.needPef = CVI_FALSE;
	param.needDumpReg = CVI_FALSE;
	param.srcNum = 4;
	param.srcAttr.size[0].u32Width = 1024;
	param.srcAttr.size[0].u32Height = 1024;
	param.srcAttr.size[1].u32Width = 1024;
	param.srcAttr.size[1].u32Height = 1024;
	param.srcAttr.size[2].u32Width = 1024;
	param.srcAttr.size[2].u32Height = 1024;
	param.srcAttr.size[3].u32Width = 1024;
	param.srcAttr.size[3].u32Height = 1024;

	param.chnAttr.size.u32Width = 3072;
	param.chnAttr.size.u32Height = 1024;

	param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_420;
	param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_420;

	param.srcAttr.way_num = STITCH_4_WAY;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//img1, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//img2, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;
	param.srcAttr.bd_attr.bd_lx[2] = 0;//img3, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[2] = 0;
	param.srcAttr.bd_attr.bd_lx[3] = 0;//img4, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[3] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 768;//ovlap_attr img12
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 1023;
	param.srcAttr.ovlap_attr.ovlp_lx[1] = 1280;//ovlap_attr img23
	param.srcAttr.ovlap_attr.ovlp_rx[1] = 1791;
	param.srcAttr.ovlap_attr.ovlp_lx[2] = 2048;//ovlap_attr img34
	param.srcAttr.ovlap_attr.ovlp_rx[2] = 2303;

	param.opAttr.data_src = STITCH_DATA_SRC_DDR;
	param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

	/************************************************
	 * step1:  Init SYS and common VB
	 ************************************************/
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	u32BlkSize[0] = COMMON_GetPicBufferSize(1024, 1024, param.srcAttr.fmt_in
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, STITCH_ALIGN);

	u32BlkSize[1] = COMMON_GetPicBufferSize(3072, 1024, param.chnAttr.fmt_out
		, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, STITCH_ALIGN);

	stVbConf.u32MaxPoolCnt				= 2;

	for (int i = 0; i < (int)stVbConf.u32MaxPoolCnt; i++) {
		stVbConf.astCommPool[i].u32BlkSize	= u32BlkSize[i];
		stVbConf.astCommPool[i].u32BlkCnt	= (i == 0) ? 4 : 1;
		stVbConf.astCommPool[i].enRemapMode = VB_REMAP_MODE_CACHED;
		STITCH_UT_PRT("common pool[%d] BlkSize %d\n", i, stVbConf.astCommPool[i].u32BlkSize);
	}

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_SYS_Init failed!\n");
		goto exit0;
	}

	strcpy(param.filename_out, filename_out);
	strcpy(param.filename_pef, filename_pef);

	for (int i = 0; i < 3; i++) {
		param.wgtAttr.size_wgt[i].u32Width =
			ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[i] - param.srcAttr.ovlap_attr.ovlp_lx[i] + 1, STITCH_ALIGN);
		param.wgtAttr.size_wgt[i].u32Height = param.srcAttr.size[i].u32Height;
	}

	for (int i = 0, j = 0; i < 6; i++, j = i >> 1) {
		strcpy(param.wgt_name[i], wgt_name[i]);
		s32Ret = cfg_wgt_image(param.wgtAttr.size_wgt[j], param.opAttr.wgt_mode
			, param.wgt_name[i], &u64PhyAddr[i], &VirAddr[i]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
			goto exit1;
		}
		if (i % 2)
			param.wgtAttr.phy_addr_wgt[j][1] = (__u64)u64PhyAddr[i];
		else
			param.wgtAttr.phy_addr_wgt[j][0] = (__u64)u64PhyAddr[i];
	}

	for (int i = 0; i < 4; i++)
		strcpy(param.filename_in[i], filename_in[i]);

	s32Ret = basic3(&param, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		goto exit1;
	}

exit1:
	for (int i = 0; i < 6; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
	}

	CVI_SYS_Exit();
exit0:
	CVI_VB_Exit();

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 stitch_test_size_min1(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i, times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM] = {0};
	CVI_VOID *VirAddr[STITCH_MAX_SRC_NUM] = {0};
	char *wgt_name[STITCH_MAX_SRC_NUM] = {STITCH_MIN_SIZE_FILE_IN_WGT_ALPHA1, STITCH_MIN_SIZE_FILE_IN_WGT_BETA1};
	char *filename_in[STITCH_MAX_SRC_NUM] = {STITCH_MIN_SIZE_FILE_IN_LFT1, STITCH_MIN_SIZE_FILE_IN_RHT1};
	char *filename_out = STITCH_MIN_SIZE_FILE_OUT1;
	char *filename_pef = STITCH_MIN_SIZE_FILE_PEF1;

	param.needPef = CVI_TRUE;
	param.needDumpReg = CVI_FALSE;
	param.srcNum = 2;
	param.srcAttr.size[0].u32Width = 64;
	param.srcAttr.size[0].u32Height = 64;
	param.srcAttr.size[1].u32Width = 64;
	param.srcAttr.size[1].u32Height = 64;

	param.chnAttr.size.u32Width = 96;
	param.chnAttr.size.u32Height = 64;

	param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_420;
	param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_420;

	param.srcAttr.way_num = STITCH_2_WAY;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 32;//ovlap_attr from algo
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 63;

	param.opAttr.data_src = STITCH_DATA_SRC_DDR;
	param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

	param.wgtAttr.size_wgt[0].u32Width =
		ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[0] - param.srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
	param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

	strcpy(param.filename_out, filename_out);
	strcpy(param.filename_pef, filename_pef);
	for (i = 0; i < param.srcNum; i++) {
		strcpy(param.wgt_name[i], wgt_name[i]);
		strcpy(param.filename_in[i], filename_in[i]);

		s32Ret = cfg_wgt_image(param.wgtAttr.size_wgt[0], param.opAttr.wgt_mode
			, param.wgt_name[i], &u64PhyAddr[i], &VirAddr[i]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
			return s32Ret;
		}
		param.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
	}

	s32Ret = basic(&param, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		goto free;
	}

free:
	for (i = 0; i < param.srcNum; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
	}

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 stitch_test_size_min2(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i, times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM] = {0};
	CVI_VOID *VirAddr[STITCH_MAX_SRC_NUM] = {0};
	char *wgt_name[STITCH_MAX_SRC_NUM] = {STITCH_MIN_SIZE_FILE_IN_WGT_ALPHA2, STITCH_MIN_SIZE_FILE_IN_WGT_BETA2};
	char *filename_in[STITCH_MAX_SRC_NUM] = {STITCH_MIN_SIZE_FILE_IN_LFT2, STITCH_MIN_SIZE_FILE_IN_RHT2};
	char *filename_out = STITCH_MIN_SIZE_FILE_OUT2;
	char *filename_pef = STITCH_MIN_SIZE_FILE_PEF2;

	param.needPef = CVI_TRUE;
	param.needDumpReg = CVI_FALSE;
	param.srcNum = 2;
	param.srcAttr.size[0].u32Width = 128;
	param.srcAttr.size[0].u32Height = 128;
	param.srcAttr.size[1].u32Width = 128;
	param.srcAttr.size[1].u32Height = 128;

	param.chnAttr.size.u32Width = 192;
	param.chnAttr.size.u32Height = 128;

	param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_420;
	param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_420;

	param.srcAttr.way_num = STITCH_2_WAY;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 64;//ovlap_attr from algo
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 127;

	param.opAttr.data_src = STITCH_DATA_SRC_DDR;
	param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

	param.wgtAttr.size_wgt[0].u32Width =
		ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[0] - param.srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
	param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

	strcpy(param.filename_out, filename_out);
	strcpy(param.filename_pef, filename_pef);
	for (i = 0; i < param.srcNum; i++) {
		strcpy(param.wgt_name[i], wgt_name[i]);
		strcpy(param.filename_in[i], filename_in[i]);

		s32Ret = cfg_wgt_image(param.wgtAttr.size_wgt[0], param.opAttr.wgt_mode
			, param.wgt_name[i], &u64PhyAddr[i], &VirAddr[i]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
			return s32Ret;
		}
		param.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
	}

	s32Ret = basic(&param, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		goto free;
	}

free:
	for (i = 0; i < param.srcNum; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
	}

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 stitch_test_size_middle(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i, times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM] = {0};
	CVI_VOID *VirAddr[STITCH_MAX_SRC_NUM] = {0};
	char *wgt_name[STITCH_MAX_SRC_NUM] = {STITCH_MIDDLE_SIZE_FILE_IN_WGT_ALPHA, STITCH_MIDDLE_SIZE_FILE_IN_WGT_BETA};
	char *filename_in[STITCH_MAX_SRC_NUM] = {STITCH_MIDDLE_SIZE_FILE_IN_LFT, STITCH_MIDDLE_SIZE_FILE_IN_RHT};
	char *filename_out = STITCH_MIDDLE_SIZE_FILE_OUT;
	char *filename_pef = STITCH_MIDDLE_SIZE_FILE_PEF;

	param.needPef = CVI_TRUE;
	param.needDumpReg = CVI_FALSE;
	param.srcNum = 2;
	param.srcAttr.size[0].u32Width = 1024;
	param.srcAttr.size[0].u32Height = 1024;
	param.srcAttr.size[1].u32Width = 1024;
	param.srcAttr.size[1].u32Height = 1024;

	param.chnAttr.size.u32Width = 1536;
	param.chnAttr.size.u32Height = 1024;

	param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_420;
	param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_420;

	param.srcAttr.way_num = STITCH_2_WAY;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 512;//ovlap_attr from algo
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 1023;

	param.opAttr.data_src = STITCH_DATA_SRC_DDR;
	param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

	param.wgtAttr.size_wgt[0].u32Width =
		ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[0] - param.srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
	param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

	strcpy(param.filename_out, filename_out);
	strcpy(param.filename_pef, filename_pef);
	for (i = 0; i < param.srcNum; i++) {
		strcpy(param.wgt_name[i], wgt_name[i]);
		strcpy(param.filename_in[i], filename_in[i]);

		s32Ret = cfg_wgt_image(param.wgtAttr.size_wgt[0], param.opAttr.wgt_mode
			, param.wgt_name[i], &u64PhyAddr[i], &VirAddr[i]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
			return s32Ret;
		}
		param.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
	}

	s32Ret = basic(&param, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		goto free;
	}

free:
	for (i = 0; i < param.srcNum; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
	}

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}


/*same with full ovlp case, img height is not restriction*/
static CVI_S32 stitch_test_size_max(CVI_VOID)
{
	CVI_S32 s32Ret;

	s32Ret = stitch_test_full_ovlp();

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 stitch_test_rst(CVI_VOID)
{
	CVI_S32 s32Ret;

	g_stitch_need_rst = CVI_TRUE;
	s32Ret = stitch_test_basic();
	g_stitch_need_rst = CVI_FALSE;

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 stitch_test_online(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 stitch_test_multi_thread(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 stitch_test_pef(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i, times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM] = {0};
	CVI_VOID *VirAddr[STITCH_MAX_SRC_NUM] = {0};
	char *filename_in[STITCH_MAX_SRC_NUM] = {STITCH_FILE_4K, STITCH_FILE_4K};

	param.needPef = CVI_FALSE;
	param.needDumpReg = CVI_FALSE;
	param.srcNum = 2;
	param.srcAttr.size[0].u32Width = 3840;
	param.srcAttr.size[0].u32Height = 2160;
	param.srcAttr.size[1].u32Width = 3840;
	param.srcAttr.size[1].u32Height = 2160;

	param.chnAttr.size.u32Width = 5760;
	param.chnAttr.size.u32Height = 2160;

	param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_420;
	param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_420;

	param.srcAttr.way_num = STITCH_2_WAY;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 1920;//ovlap_attr from algo
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 3839;

	param.opAttr.data_src = STITCH_DATA_SRC_DDR;
	param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

	param.wgtAttr.size_wgt[0].u32Width =
		ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[0] - param.srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
	param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

	for (i = 0; i < param.srcNum; i++) {
		strcpy(param.filename_in[i], filename_in[i]);
		s32Ret = cfg_wgt_image2(param.wgtAttr.size_wgt[0], param.opAttr.wgt_mode
			, &u64PhyAddr[i], &VirAddr[i]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
			return s32Ret;
		}
		param.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
	}

	s32Ret = basic(&param, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		goto free;
	}

free:
	for (i = 0; i < param.srcNum; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
	}

	STITCH_TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 cfg_chn_frame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VB_CAL_CONFIG_S stVbCalConfig;

	COMMON_GetPicBufferConfig(stSize->u32Width, stSize->u32Height, enPixelFormat, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, STITCH_ALIGN, &stVbCalConfig);

	pstVideoFrame->stVFrame.enCompressMode = COMPRESS_MODE_NONE;
	pstVideoFrame->stVFrame.enPixelFormat = enPixelFormat;
	pstVideoFrame->stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
	pstVideoFrame->stVFrame.enColorGamut = COLOR_GAMUT_BT709;
	pstVideoFrame->stVFrame.u32Width = stSize->u32Width;
	pstVideoFrame->stVFrame.u32Height = stSize->u32Height;
	pstVideoFrame->stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
	pstVideoFrame->stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
	pstVideoFrame->stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
	pstVideoFrame->stVFrame.u32TimeRef = 0;
	pstVideoFrame->stVFrame.u64PTS = 0;
	pstVideoFrame->stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;

	if (CVI_SYS_IonAlloc(pu64PhyAddr, ppVirAddr, "stitch_chn_frm", stVbCalConfig.u32VBSize)) {
		STITCH_UT_PRT("CVI_SYS_IonAlloc_Cached NG.\n");
		return CVI_FAILURE;
	}

	pstVideoFrame->stVFrame.u32Length[0] = stVbCalConfig.u32MainYSize;
	pstVideoFrame->stVFrame.u32Length[1] = stVbCalConfig.u32MainCSize;
	pstVideoFrame->stVFrame.u64PhyAddr[0] = *pu64PhyAddr;
	pstVideoFrame->stVFrame.u64PhyAddr[1] = pstVideoFrame->stVFrame.u64PhyAddr[0]
		+ ALIGN(stVbCalConfig.u32MainYSize, stVbCalConfig.u16AddrAlign);
	if (stVbCalConfig.plane_num == 3) {
		pstVideoFrame->stVFrame.u32Length[2] = stVbCalConfig.u32MainCSize;
		pstVideoFrame->stVFrame.u64PhyAddr[2] = pstVideoFrame->stVFrame.u64PhyAddr[1]
			+ ALIGN(stVbCalConfig.u32MainCSize, stVbCalConfig.u16AddrAlign);
	}
	pstVideoFrame->stVFrame.u32Align = STITCH_ALIGN;

	STITCH_UT_PRT("length of buffer(%d, %d, %d)\n", pstVideoFrame->stVFrame.u32Length[0]
		, pstVideoFrame->stVFrame.u32Length[1], pstVideoFrame->stVFrame.u32Length[2]);
	STITCH_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", pstVideoFrame->stVFrame.u64PhyAddr[0]
		, pstVideoFrame->stVFrame.u64PhyAddr[1], pstVideoFrame->stVFrame.u64PhyAddr[2]);
	STITCH_UT_PRT("vir addr(%p, %p, %p)\n", pstVideoFrame->stVFrame.pu8VirAddr[0]
		, pstVideoFrame->stVFrame.pu8VirAddr[1], pstVideoFrame->stVFrame.pu8VirAddr[2]);

	return s32Ret;
}

static CVI_S32 basic4(STITCH_BASIC_TEST_PARAM *pParam, CVI_U8 times)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 timout_ms = STITCH_TIMEOUT;
	CVI_U64 u64PhyAddr[3] = {0};
	CVI_VOID *pVirAddr[3] = {0};

	UNUSED(times);

	s32Ret = CVI_STITCH_Init();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_STITCH_Reset();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_Reset failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetSrcAttr(&pParam->srcAttr);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetSrcAttr failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetChnAttr(&pParam->chnAttr);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetChnAttr failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetOpAttr(&pParam->opAttr);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetOpAttr failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetWgtAttr(&pParam->wgtAttr);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetWgtAttr failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetRegX(16);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetRegX failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_EnableDev();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_EnableDev failed!\n");
		goto exit2;
	}

	do {
		memset(&pParam->stVideoFrameOut, 0, sizeof(pParam->stVideoFrameOut));
		s32Ret = cfg_chn_frame(&pParam->chnAttr.size, pParam->chnAttr.fmt_out, &pParam->stVideoFrameOut, &u64PhyAddr[2], &pVirAddr[2]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_chn_frame NG.\n");
			goto exit3;
		}

		s32Ret = CVI_STITCH_SendChnFrame(&pParam->stVideoFrameOut, 1000);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("CVI_STITCH_SendChnFrame fail.\n");
			goto exit3;
		}
		for (int i = 0; i < pParam->srcNum; i++) {
			STITCH_UT_PRT("start send src frame[%d]\n", i);

			s32Ret = FileSendToStitchNoVb((STITCH_SRC_IDX)i, &pParam->srcAttr.size[i], pParam->srcAttr.fmt_in, pParam->filename_in[i], &u64PhyAddr[i], &pVirAddr[i]);
			if (s32Ret != CVI_SUCCESS) {
				STITCH_UT_PRT("FileSendToStitch[%d] fail, s32Ret: 0x%x !\n", i, s32Ret);
				goto exit3;
			}
		}



		s32Ret = CVI_STITCH_GetChnFrame(&pParam->stVideoFrameOut, timout_ms*10);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("CVI_STITCH_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
			//pParam->needDumpReg = CVI_TRUE;
			goto exit3;
		}

		STITCH_UT_PRT("***CVI_STITCH_GetChnFrame Success, start save and compare with golden***\n");
		STITCH_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", pParam->stVideoFrameOut.stVFrame.u64PhyAddr[0]
			, pParam->stVideoFrameOut.stVFrame.u64PhyAddr[1], pParam->stVideoFrameOut.stVFrame.u64PhyAddr[2]);

		if (g_stitch_save_file) {
			if (strlen(pParam->filename_out)) {
				s32Ret = FrameSaveToFile(pParam->filename_out, &pParam->stVideoFrameOut);
				if (s32Ret != CVI_SUCCESS) {
					STITCH_UT_PRT("FrameSaveToFile. s32Ret: 0x%x !\n", s32Ret);
					CVI_STITCH_ReleaseChnFrame(&pParam->stVideoFrameOut);
					goto exit3;
				}
				STITCH_UT_PRT("output file:%s\n", pParam->filename_out);
			}

			if (pParam->needPef) {
				s32Ret = CompareWithFile(pParam->filename_pef, &pParam->stVideoFrameOut);
				if (s32Ret != CVI_SUCCESS) {
					STITCH_UT_PRT("CompareWithFile fail.\n");
					CVI_STITCH_ReleaseChnFrame(&pParam->stVideoFrameOut);
					goto exit3;
				}
			}
		}

		for (int i = 0; i < 3; ++i) {
			if (u64PhyAddr[i] && pVirAddr[i])
				CVI_SYS_IonFree(u64PhyAddr[i], pVirAddr[i]);
			u64PhyAddr[i] = 0;
			pVirAddr[i] = NULL;
		}
	} while (times--);
exit3:
	if (pParam->needDumpReg)
		CVI_STITCH_DumpRegInfo();

	CVI_STITCH_DisableDev();
	for (int i = 0; i < 3; ++i) {
		if (u64PhyAddr[i] && pVirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], pVirAddr[i]);
	}

	if (g_stitch_need_rst)
		CVI_STITCH_Reset();
exit2:
	CVI_STITCH_DeInit();
	return s32Ret;
}


static CVI_S32 basic5(STITCH_BASIC_TEST_PARAM *pParam, CVI_U8 times)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 timout_ms = STITCH_TIMEOUT;
	CVI_U64 u64PhyAddr[3] = {0};
	CVI_VOID *pVirAddr[3] = {0};

	s32Ret = CVI_STITCH_Init();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_STITCH_Reset();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_Reset failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetSrcAttr(&pParam->srcAttr);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetSrcAttr failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetChnAttr(&pParam->chnAttr);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetChnAttr failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetOpAttr(&pParam->opAttr);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetOpAttr failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetWgtAttr(&pParam->wgtAttr);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetWgtAttr failed!\n");
		goto exit2;
	}

	s32Ret = CVI_STITCH_SetRegX(16);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_SetRegX failed!\n");
		goto exit2;
	}

	do {
		s32Ret = CVI_STITCH_EnableDev();
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("CVI_STITCH_EnableDev failed!\n");
			goto exit3;
		}

		memset(&pParam->stVideoFrameOut, 0, sizeof(pParam->stVideoFrameOut));
		s32Ret = cfg_chn_frame(&pParam->chnAttr.size, pParam->chnAttr.fmt_out, &pParam->stVideoFrameOut, &u64PhyAddr[2], &pVirAddr[2]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_chn_frame NG.\n");
			goto exit3;
		}

		s32Ret = CVI_STITCH_SendChnFrame(&pParam->stVideoFrameOut, 1000);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("CVI_STITCH_SendChnFrame fail.\n");
			goto exit3;
		}

		for (int i = 0; i < pParam->srcNum; i++) {
			STITCH_UT_PRT("start send src frame[%d]\n", i);

			s32Ret = FileSendToStitchNoVb((STITCH_SRC_IDX)i, &pParam->srcAttr.size[i], pParam->srcAttr.fmt_in, pParam->filename_in[i], &u64PhyAddr[i], &pVirAddr[i]);
			if (s32Ret != CVI_SUCCESS) {
				STITCH_UT_PRT("FileSendToStitch[%d] fail, s32Ret: 0x%x !\n", i, s32Ret);
				goto exit3;
			}
		}

		s32Ret = CVI_STITCH_GetChnFrame(&pParam->stVideoFrameOut, timout_ms*10);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("CVI_STITCH_GetChnFrame fail. s32Ret: 0x%x !\n", s32Ret);
			//pParam->needDumpReg = CVI_TRUE;
			goto exit3;
		}

		s32Ret = CVI_STITCH_DisableDev();
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("CVI_STITCH_DisableDev failed!\n");
			goto exit3;
		}

		STITCH_UT_PRT("***CVI_STITCH_GetChnFrame Success, start save and compare with golden***\n");
		STITCH_UT_PRT("phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64")\n", pParam->stVideoFrameOut.stVFrame.u64PhyAddr[0]
			, pParam->stVideoFrameOut.stVFrame.u64PhyAddr[1], pParam->stVideoFrameOut.stVFrame.u64PhyAddr[2]);

		if (g_stitch_save_file) {
			if (strlen(pParam->filename_out)) {
				s32Ret = FrameSaveToFile(pParam->filename_out, &pParam->stVideoFrameOut);
				if (s32Ret != CVI_SUCCESS) {
					STITCH_UT_PRT("FrameSaveToFile. s32Ret: 0x%x !\n", s32Ret);
					CVI_STITCH_ReleaseChnFrame(&pParam->stVideoFrameOut);
					goto exit3;
				}
				STITCH_UT_PRT("output file:%s\n", pParam->filename_out);
			}

			if (pParam->needPef) {
				s32Ret = CompareWithFile(pParam->filename_pef, &pParam->stVideoFrameOut);
				if (s32Ret != CVI_SUCCESS) {
					STITCH_UT_PRT("CompareWithFile fail.\n");
					CVI_STITCH_ReleaseChnFrame(&pParam->stVideoFrameOut);
					goto exit3;
				}
			}
		}

		for (int i = 0; i < 3; ++i) {
			if (u64PhyAddr[i] && pVirAddr[i])
				CVI_SYS_IonFree(u64PhyAddr[i], pVirAddr[i]);
			u64PhyAddr[i] = 0;
			pVirAddr[i] = NULL;
		}
	} while (times--);
exit3:
	if (pParam->needDumpReg)
		CVI_STITCH_DumpRegInfo();

	for (int i = 0; i < 3; ++i) {
		if (u64PhyAddr[i] && pVirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], pVirAddr[i]);
	}

	if (g_stitch_need_rst)
		CVI_STITCH_Reset();
exit2:
	CVI_STITCH_DeInit();
	return s32Ret;
}

static CVI_S32 stitch_test_en_dis_dev_loop(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i, times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM] = {0};
	CVI_VOID *VirAddr[STITCH_MAX_SRC_NUM] = {0};
	char *wgt_name[STITCH_MAX_SRC_NUM] = {STITCH_FILE_IN_WGT_ALPHA, STITCH_FILE_IN_WGT_BETA};
	char *filename_in[STITCH_MAX_SRC_NUM] = {STITCH_FILE_IN_LFT, STITCH_FILE_IN_RHT};
	char *filename_out = STITCH_FILE_OUT;
	char *filename_pef = STITCH_FILE_PEF;

	param.needPef = CVI_TRUE;
	param.needDumpReg = CVI_FALSE;
	param.srcNum = 2;
	param.srcAttr.size[0].u32Width = 4608;
	param.srcAttr.size[0].u32Height = 288;
	param.srcAttr.size[1].u32Width = 4608;
	param.srcAttr.size[1].u32Height = 288;

	param.chnAttr.size.u32Width = 6912;
	param.chnAttr.size.u32Height = 288;

	param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_420;
	param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_420;

	param.srcAttr.way_num = STITCH_2_WAY;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 2304;//ovlap_attr from algo
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 4607;

	param.opAttr.data_src = STITCH_DATA_SRC_DDR;
	param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

	param.wgtAttr.size_wgt[0].u32Width =
		ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[0] - param.srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
	param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

	strcpy(param.filename_out, filename_out);
	strcpy(param.filename_pef, filename_pef);
	for (i = 0; i < param.srcNum; i++) {
		strcpy(param.wgt_name[i], wgt_name[i]);
		strcpy(param.filename_in[i], filename_in[i]);

		s32Ret = cfg_wgt_image(param.wgtAttr.size_wgt[0], param.opAttr.wgt_mode
			, param.wgt_name[i], &u64PhyAddr[i], &VirAddr[i]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
			return s32Ret;
		}
		param.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
	}

	s32Ret = basic5(&param, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		goto free;
	}

free:
	for (i = 0; i < param.srcNum; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
	}

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 stitch_test_no_vb(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i, times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM] = {0};
	CVI_VOID *VirAddr[STITCH_MAX_SRC_NUM] = {0};
	char *wgt_name[STITCH_MAX_SRC_NUM] = {STITCH_FILE_IN_WGT_ALPHA, STITCH_FILE_IN_WGT_BETA};
	char *filename_in[STITCH_MAX_SRC_NUM] = {STITCH_FILE_IN_LFT, STITCH_FILE_IN_RHT};
	char *filename_out = STITCH_FILE_OUT;
	char *filename_pef = STITCH_FILE_PEF;

	param.needPef = CVI_TRUE;
	param.needDumpReg = CVI_FALSE;
	param.srcNum = 2;
	param.srcAttr.size[0].u32Width = 4608;
	param.srcAttr.size[0].u32Height = 288;
	param.srcAttr.size[1].u32Width = 4608;
	param.srcAttr.size[1].u32Height = 288;

	param.chnAttr.size.u32Width = 6912;
	param.chnAttr.size.u32Height = 288;

	param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_420;
	param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_420;

	param.srcAttr.way_num = STITCH_2_WAY;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 2304;//ovlap_attr from algo
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 4607;

	param.opAttr.data_src = STITCH_DATA_SRC_DDR;
	param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

	param.wgtAttr.size_wgt[0].u32Width =
		ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[0] - param.srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
	param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

	strcpy(param.filename_out, filename_out);
	strcpy(param.filename_pef, filename_pef);
	for (i = 0; i < param.srcNum; i++) {
		strcpy(param.wgt_name[i], wgt_name[i]);
		strcpy(param.filename_in[i], filename_in[i]);

		s32Ret = cfg_wgt_image(param.wgtAttr.size_wgt[0], param.opAttr.wgt_mode
			, param.wgt_name[i], &u64PhyAddr[i], &VirAddr[i]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
			return s32Ret;
		}
		param.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
	}

	s32Ret = basic4(&param, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		goto exit1;
	}

exit1:
	for (int i = 0; i < 3; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
	}

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 stitch_test_check_sum(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i, times = 1000;
	STITCH_BASIC_TEST_PARAM param = {0};
	CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM] = {0};
	CVI_VOID *VirAddr[STITCH_MAX_SRC_NUM] = {0};
	char *filename_in[STITCH_MAX_SRC_NUM] = {STITCH_FILE_4K, STITCH_FILE_4K};
	char *filename_out = STITCH_FILE_OUT_4K;
	char *filename_pef = STITCH_FILE_PEF_4K;

	param.needPef = CVI_TRUE;
	param.needDumpReg = CVI_FALSE;
	param.srcNum = 2;
	param.srcAttr.size[0].u32Width = 3840;
	param.srcAttr.size[0].u32Height = 2160;
	param.srcAttr.size[1].u32Width = 3840;
	param.srcAttr.size[1].u32Height = 2160;

	param.chnAttr.size.u32Width = 3840;
	param.chnAttr.size.u32Height = 2160;

	param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_420;
	param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_420;

	param.srcAttr.way_num = STITCH_2_WAY;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 0;//ovlap_attr from algo
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 3839;

	param.opAttr.data_src = STITCH_DATA_SRC_DDR;
	param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

	param.wgtAttr.size_wgt[0].u32Width =
		ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[0] - param.srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
	param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

	strcpy(param.filename_out, filename_out);
	strcpy(param.filename_pef, filename_pef);
	for (i = 0; i < param.srcNum; i++) {
		strcpy(param.filename_in[i], filename_in[i]);
		s32Ret = cfg_wgt_image2(param.wgtAttr.size_wgt[0], param.opAttr.wgt_mode
			, &u64PhyAddr[i], &VirAddr[i]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
			return s32Ret;
		}

		param.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
	}
#if 0
	unsigned int *addr[2];

	addr[0] = (unsigned int *)0x25200000;
	addr[1] = (unsigned int *)0x25300000;
	for (unsigned int idx = 0; idx < 0x100000/4; idx++) {
		*(addr[0] + idx) = 0xffffffff;
		*(addr[1] + idx) = 0x0;
	}

	param.wgtAttr.phy_addr_wgt[0][0] = (__u64)addr[0];
	param.wgtAttr.phy_addr_wgt[0][1] = (__u64)addr[1];
#endif

	int wgt;
	printf("input left wgt:[0-256]");
	scanf("%d", &wgt);

	memset(VirAddr[1], 255-wgt, param.wgtAttr.size_wgt[0].u32Width * param.wgtAttr.size_wgt[0].u32Height);
	memset(VirAddr[0], wgt, param.wgtAttr.size_wgt[0].u32Width * param.wgtAttr.size_wgt[0].u32Height);

	s32Ret = basic(&param, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		goto free;
	}

free:
	for (i = 0; i < param.srcNum; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
	}

	STITCH_TEST_CHECK_RET(s32Ret);

	return s32Ret;
}

static CVI_S32 stitch_test_suspend(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_STITCH_Init();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_STITCH_Suspend();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_Suspend failed!\n");
		return s32Ret;
	}

	return s32Ret;
}

static CVI_S32 stitch_test_resume(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_STITCH_Resume();
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("CVI_STITCH_Resume failed!\n");
		return s32Ret;
	}

	return s32Ret;
}

static CVI_S32 stitch_test_running_suspend(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int i, times = STITCH_REPECT_TIMES;
	STITCH_BASIC_TEST_PARAM param = {0};
	CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM] = {0};
	CVI_VOID *VirAddr[STITCH_MAX_SRC_NUM] = {0};
	char *wgt_name[STITCH_MAX_SRC_NUM] = {STITCH_FILE_IN_WGT_ALPHA, STITCH_FILE_IN_WGT_BETA};
	char *filename_in[STITCH_MAX_SRC_NUM] = {STITCH_FILE_IN_LFT, STITCH_FILE_IN_RHT};
	char *filename_out = STITCH_FILE_OUT;
	char *filename_pef = STITCH_FILE_PEF;

	param.needSuspend = CVI_TRUE;
	param.needPef = CVI_FALSE;
	param.needDumpReg = CVI_FALSE;
	param.srcNum = 2;
	param.srcAttr.size[0].u32Width = 4608;
	param.srcAttr.size[0].u32Height = 288;
	param.srcAttr.size[1].u32Width = 4608;
	param.srcAttr.size[1].u32Height = 288;

	param.chnAttr.size.u32Width = 6912;
	param.chnAttr.size.u32Height = 288;

	param.srcAttr.fmt_in = PIXEL_FORMAT_YUV_PLANAR_420;
	param.chnAttr.fmt_out = PIXEL_FORMAT_YUV_PLANAR_420;

	param.srcAttr.way_num = STITCH_2_WAY;

	param.srcAttr.bd_attr.bd_lx[0] = 0;//left img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[0] = 0;
	param.srcAttr.bd_attr.bd_lx[1] = 0;//right img, bd_attr from algo
	param.srcAttr.bd_attr.bd_rx[1] = 0;

	param.srcAttr.ovlap_attr.ovlp_lx[0] = 2304;//ovlap_attr from algo
	param.srcAttr.ovlap_attr.ovlp_rx[0] = 4607;

	param.opAttr.data_src = STITCH_DATA_SRC_DDR;
	param.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

	param.wgtAttr.size_wgt[0].u32Width =
		ALIGN(param.srcAttr.ovlap_attr.ovlp_rx[0] - param.srcAttr.ovlap_attr.ovlp_lx[0] + 1, STITCH_ALIGN);
	param.wgtAttr.size_wgt[0].u32Height = param.srcAttr.size[0].u32Height;

	strcpy(param.filename_out, filename_out);
	strcpy(param.filename_pef, filename_pef);
	for (i = 0; i < param.srcNum; i++) {
		strcpy(param.wgt_name[i], wgt_name[i]);
		strcpy(param.filename_in[i], filename_in[i]);

		s32Ret = cfg_wgt_image(param.wgtAttr.size_wgt[0], param.opAttr.wgt_mode
			, param.wgt_name[i], &u64PhyAddr[i], &VirAddr[i]);
		if (s32Ret != CVI_SUCCESS) {
			STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
			return s32Ret;
		}
		param.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
	}

	s32Ret = basic(&param, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		goto free;
	}

free:
	for (i = 0; i < param.srcNum; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
	}

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static CVI_S32 stitch_test_auto_regression(CVI_VOID)
{
	CVI_S32 s32Ret[100] = {[0 ... 99] = CVI_SUCCESS};
	CVI_S32 Ret = CVI_SUCCESS;
	p_func test_func[MAX_FUNC_CNT] = {
		stitch_test_basic,
		stitch_test_full_ovlp,
		stitch_test_none_ovlp,
		stitch_test_none_ovlp1,
		stitch_test_lft_ovlp,
		stitch_test_rht_ovlp,
		stitch_test_pure_color,
		stitch_test_pure_color_mode1,
		stitch_test_fmt_400,
		stitch_test_fmt_422p,
		stitch_test_fmt_444p,
		stitch_test_fmt_rgb888,
		stitch_test_wgt_mode1,
		stitch_test_4way_fake,
		stitch_test_4way_fake_middle,
		stitch_test_4way,
		stitch_test_4way_middle,
		stitch_test_size_min1,
		stitch_test_size_min2,
		stitch_test_size_middle,
		stitch_test_size_max,
		stitch_test_online,
		stitch_test_multi_thread,
		stitch_test_pef,
		stitch_test_no_vb,
		stitch_test_en_dis_dev_loop,
		stitch_test_rst,
	};

	for (int i = 0; i < MAX_FUNC_CNT; i++) {
		if (test_func[i]) {
			s32Ret[i] = test_func[i]();
			Ret |= s32Ret[i];
		}
	}

	for (int i = 0; i < MAX_FUNC_CNT; i++) {
		if (s32Ret[i])
			STITCH_UT_PRT("op[%d] fail, ret[%d]\n", i, s32Ret[i]);
	}

	STITCH_TEST_CHECK_RET(Ret);
	return Ret;
}

static CVI_S32 stitch_test_user_config(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int tmp, i, times = STITCH_REPECT_TIMES;
	CVI_U32 u32WidthIn[STITCH_MAX_SRC_NUM], u32HeightIn[STITCH_MAX_SRC_NUM], u32WidthOut, u32HeightOut;
	CVI_U32 u32FormatIn, u32FormatOut;
	STITCH_BASIC_TEST_PARAM stTestParam;
	CVI_U64 u64PhyAddr[STITCH_MAX_SRC_NUM] = {0};
	CVI_VOID *VirAddr[STITCH_MAX_SRC_NUM] = {0};

	memset(&stTestParam, 0, sizeof(stTestParam));
	printf("\n---stitch config---\n");

	printf("format list:\n");
	for (i = PIXEL_FORMAT_RGB_888; i < PIXEL_FORMAT_MAX; i++) {
		if (strncmp(GetFmtName(i), "unknown", sizeof("unknown")))
			printf("%2d : %s\n", i, GetFmtName(i));
	}
	printf("input format:");
	scanf("%d", &u32FormatIn);
	printf("output format:");
	scanf("%d", &u32FormatOut);

	printf("input way num:");
	scanf("%d", &tmp);
	stTestParam.srcNum = tmp;

	//src attr
	for (i = 0; i < stTestParam.srcNum; i++) {
		printf("input[%d] width:", i);
		scanf("%u", &u32WidthIn[i]);
		printf("input[%d] height:", i);
		scanf("%u", &u32HeightIn[i]);
		printf("input file_in[%d]:", i);
		scanf("%s", stTestParam.filename_in[i]);
		stTestParam.srcAttr.size[i].u32Width = u32WidthIn[i];
		stTestParam.srcAttr.size[i].u32Height = u32HeightIn[i];

		stTestParam.srcAttr.bd_attr.bd_lx[i] = 0;//left img, bd_attr from algo
		stTestParam.srcAttr.bd_attr.bd_rx[i] = 0;//right img, bd_attr from algo

		if (i == stTestParam.srcNum)
			break;
		printf("input[%d-%d] ovlp lx:", i, i+1);
		scanf("%d", &tmp);
		stTestParam.srcAttr.ovlap_attr.ovlp_lx[i] = tmp;//ovlap_attr from algo
		printf("input[%d-%d] ovlp rx:", i, i+1);
		scanf("%d", &tmp);
		stTestParam.srcAttr.ovlap_attr.ovlp_rx[i] = tmp;
	}
	stTestParam.srcAttr.way_num = (stTestParam.srcNum == 4 ? STITCH_4_WAY : STITCH_2_WAY);
	stTestParam.srcAttr.fmt_in = u32FormatIn;

	//Chn attr
	printf("output width:");
	scanf("%d", &u32WidthOut);
	printf("output height:");
	scanf("%d", &u32HeightOut);

	stTestParam.chnAttr.size.u32Width = u32WidthOut;
	stTestParam.chnAttr.size.u32Height = u32HeightOut;
	stTestParam.chnAttr.fmt_out = u32FormatOut;

	//op attr
	stTestParam.opAttr.data_src = STITCH_DATA_SRC_DDR;
	stTestParam.opAttr.wgt_mode = STITCH_WGT_YUV_SHARE;

	//wgt attr
	for (int i = 0; i < stTestParam.srcNum - 1; i++) {
		stTestParam.wgtAttr.size_wgt[i].u32Width =
			ALIGN(stTestParam.srcAttr.ovlap_attr.ovlp_rx[i] - stTestParam.srcAttr.ovlap_attr.ovlp_lx[i] + 1, STITCH_ALIGN);
		stTestParam.wgtAttr.size_wgt[i].u32Height = stTestParam.srcAttr.size[i].u32Height;
	}

	for (i = 0; i < stTestParam.srcNum; i++) {
		if (stTestParam.wgtAttr.size_wgt[0].u32Width) {
			printf("input wgt [%d] filename:", i);
			scanf("%s", stTestParam.wgt_name[i]);

			s32Ret = cfg_wgt_image(stTestParam.wgtAttr.size_wgt[0], stTestParam.opAttr.wgt_mode
				, stTestParam.wgt_name[i], &u64PhyAddr[i], &VirAddr[i]);
			if (s32Ret != CVI_SUCCESS) {
				STITCH_UT_PRT("cfg_wgt_image src[%d] failed!\n", i);
				goto free;
			}

			stTestParam.wgtAttr.phy_addr_wgt[0][i] = (__u64)u64PhyAddr[i];
		}
	}

	snprintf(stTestParam.filename_out, 64, "%s_%d_%d_%s.bin.yuv", __func__,
		u32WidthOut,
		u32HeightOut,
		GetFmtName(u32FormatOut));

	s32Ret = basic(&stTestParam, times);
	if (s32Ret != CVI_SUCCESS) {
		STITCH_UT_PRT("Test failed.\n");
		goto free;
	}

free:
	for (i = 0; i < stTestParam.srcNum; i++) {
		if (u64PhyAddr[i] && VirAddr[i])
			CVI_SYS_IonFree(u64PhyAddr[i], VirAddr[i]);
	}

	STITCH_TEST_CHECK_RET(s32Ret);
	return s32Ret;
}

static int dup_fd;
static int dup_fd_bak = 1000;

static CVI_S32 stitch_dup_fd(CVI_VOID)
{
	dup_fd = open( "./printf_dup_log.txt ", O_CREAT | O_RDWR | O_TRUNC);
	dup2(STDOUT_FILENO, dup_fd_bak);/*backup stdout*/
	dup2(dup_fd, STDOUT_FILENO);
	return CVI_SUCCESS;
}

static CVI_S32 stitch_rst_fd(CVI_VOID)
{
	dup2(dup_fd_bak, fileno(stdout));/*recover stdout*/
	close(dup_fd);
	return CVI_SUCCESS;
}

static CVI_S32 _stitch_handle_op(CVI_S32 op)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	switch (op) {
	case STITCH_TEST_BASIC:
		s32Ret = stitch_test_basic();
		break;
	case STITCH_TEST_FULL_OVLP:
		s32Ret = stitch_test_full_ovlp();
		break;
	case STITCH_TEST_NONE_OVLP:
		s32Ret = stitch_test_none_ovlp();
		break;
	case STITCH_TEST_NONE_OVLP1:
		s32Ret = stitch_test_none_ovlp1();
		break;
	case STITCH_TEST_LFT_OVLP:
		s32Ret = stitch_test_lft_ovlp();
		break;
	case STITCH_TEST_RHT_OVLP:
		s32Ret = stitch_test_rht_ovlp();
		break;
	case STITCH_TEST_PURE_COLOR:
		s32Ret = stitch_test_pure_color();
		break;
	case STITCH_TEST_PURE_COLOR_MODE1:
		s32Ret = stitch_test_pure_color_mode1();
		break;
	case STITCH_TEST_FMT_400:
		s32Ret = stitch_test_fmt_400();
		break;
	case STITCH_TEST_FMT_422P:
		s32Ret = stitch_test_fmt_422p();
		break;
	case STITCH_TEST_FMT_444P:
		s32Ret = stitch_test_fmt_444p();
		break;
	case STITCH_TEST_FMT_RGB:
		s32Ret = stitch_test_fmt_rgb888();
		break;
	case STITCH_TEST_WGT_MODE:
		s32Ret = stitch_test_wgt_mode1();
		break;
	case STITCH_TEST_4WAY_FAKE:
		s32Ret = stitch_test_4way_fake();
		break;
	case STITCH_TEST_4WAY_FAKE_MIDDLE:
		s32Ret = stitch_test_4way_fake_middle();
		break;
	case STITCH_TEST_4WAY:
		s32Ret = stitch_test_4way();
		break;
	case STITCH_TEST_4WAY_MIDDLE:
		s32Ret = stitch_test_4way_middle();
		break;
	case STITCH_TEST_SIZE_MIN1:
		s32Ret = stitch_test_size_min1();
		break;
	case STITCH_TEST_SIZE_MIN2:
		s32Ret = stitch_test_size_min2();
		break;
	case STITCH_TEST_SIZE_MIDDLE:
		s32Ret = stitch_test_size_middle();
		break;
	case STITCH_TEST_SIZE_MAX:
		s32Ret = stitch_test_size_max();
		break;
	case STITCH_TEST_ONLINE:
		s32Ret = stitch_test_online();
		break;
	case STITCH_TEST_MULTI_THREAD:
		s32Ret = stitch_test_multi_thread();
		break;
	case STITCH_TEST_PEF:
		s32Ret = stitch_test_pef();
		break;
	case STITCH_TEST_NO_VB:
		s32Ret = stitch_test_no_vb();
		break;
	case STITCH_TEST_EN_DIS_DEV_LOOP:
		s32Ret = stitch_test_en_dis_dev_loop();
		break;
	case STITCH_TEST_RST:
		s32Ret = stitch_test_rst();
		break;
	case STITCH_TEST_PRESURE_SIZE_FOR_EACH:
		break;
	case STITCH_TEST_AUTO_REGRESSION:
		s32Ret = stitch_test_auto_regression();
		break;
	case STITCH_TEST_USER_CONFIG:
		s32Ret = stitch_test_user_config();
		break;
	case STITCH_TEST_DUP_FD:
		s32Ret = stitch_dup_fd();
		break;
	case STITCH_TEST_RST_FD:
		s32Ret = stitch_rst_fd();
		break;
	case STITCH_TEST_CHECK_SUM:
		s32Ret = stitch_test_check_sum();
		break;
	case STITCH_TEST_SUSPEND:
		s32Ret = stitch_test_suspend();
		break;
	case STITCH_TEST_RESUME:
		s32Ret = stitch_test_resume();
		break;
	case STITCH_TEST_RUN_SUSPEND:
		s32Ret = stitch_test_running_suspend();
		break;
	case STITCH_TEST_AUTO_LOOP:
		while(1) {
			s32Ret = stitch_test_auto_regression();
			if (s32Ret)
				break;
		}
		break;
	default:
		s32Ret = CVI_FAILURE;
		break;
	}

	return s32Ret;
}

static void stitch_show_help(void)
{
	STITCH_UT_PRT("%4d: stitch basic test\n", STITCH_TEST_BASIC);
	STITCH_UT_PRT("%4d: stitch full ovlp test\n", STITCH_TEST_FULL_OVLP);
	STITCH_UT_PRT("%4d: stitch none ovlp test\n", STITCH_TEST_NONE_OVLP);
	STITCH_UT_PRT("%4d: stitch none ovlp1 test\n", STITCH_TEST_NONE_OVLP1);
	STITCH_UT_PRT("%4d: stitch lft ovlp test\n", STITCH_TEST_LFT_OVLP);
	STITCH_UT_PRT("%4d: stitch rht ovlp test\n", STITCH_TEST_RHT_OVLP);
	STITCH_UT_PRT("%4d: stitch pure color test\n", STITCH_TEST_PURE_COLOR);
	STITCH_UT_PRT("%4d: stitch pure color test mode1\n", STITCH_TEST_PURE_COLOR_MODE1);
	STITCH_UT_PRT("%4d: stitch fmt 400 test\n", STITCH_TEST_FMT_400);
	STITCH_UT_PRT("%4d: stitch fmt 422p test\n", STITCH_TEST_FMT_422P);
	STITCH_UT_PRT("%4d: stitch fmt 444p test\n", STITCH_TEST_FMT_444P);
	STITCH_UT_PRT("%4d: stitch fmt rgb888 test\n", STITCH_TEST_FMT_RGB);
	STITCH_UT_PRT("%4d: stitch wgt mode test\n", STITCH_TEST_WGT_MODE);
	STITCH_UT_PRT("%4d: stitch 4 way fake test\n", STITCH_TEST_4WAY_FAKE);
	STITCH_UT_PRT("%4d: stitch 4 way fake middle test\n", STITCH_TEST_4WAY_FAKE_MIDDLE);
	STITCH_UT_PRT("%4d: stitch 4 way test\n", STITCH_TEST_4WAY);
	STITCH_UT_PRT("%4d: stitch 4 way middle test\n", STITCH_TEST_4WAY_MIDDLE);
	STITCH_UT_PRT("%4d: stitch size min1 test\n", STITCH_TEST_SIZE_MIN1);
	STITCH_UT_PRT("%4d: stitch size min2 test\n", STITCH_TEST_SIZE_MIN2);
	STITCH_UT_PRT("%4d: stitch size middle test\n", STITCH_TEST_SIZE_MIDDLE);
	STITCH_UT_PRT("%4d: stitch size max test\n", STITCH_TEST_SIZE_MAX);
	STITCH_UT_PRT("%4d: stitch online test\n", STITCH_TEST_ONLINE);
	STITCH_UT_PRT("%4d: stitch multi thread test\n", STITCH_TEST_MULTI_THREAD);
	STITCH_UT_PRT("%4d: stitch pef test\n", STITCH_TEST_PEF);
	STITCH_UT_PRT("%4d: stitch no vb test\n", STITCH_TEST_NO_VB);
	STITCH_UT_PRT("%4d: stitch reset test\n", STITCH_TEST_RST);
	STITCH_UT_PRT("%4d: stitch size presure test\n", STITCH_TEST_PRESURE_SIZE_FOR_EACH);
	STITCH_UT_PRT("%4d: stitch auto test\n", STITCH_TEST_AUTO_REGRESSION);
	STITCH_UT_PRT("%4d: stitch user config test\n", STITCH_TEST_USER_CONFIG);
	STITCH_UT_PRT("%4d: stitch dup fd\n", STITCH_TEST_DUP_FD);
	STITCH_UT_PRT("%4d: stitch dup fd\n", STITCH_TEST_RST_FD);
	STITCH_UT_PRT("%4d: stitch enable disable dev loop\n", STITCH_TEST_EN_DIS_DEV_LOOP);
	STITCH_UT_PRT("255: exit\n");
}

int main(int argc, char **argv)
{
	CVI_S32 s32Ret;
	CVI_S32 op = 255;

	system("stty erase ^H");

	signal(SIGINT, stitch_ut_HandleSig);
	signal(SIGTERM, stitch_ut_HandleSig);

	if (argc >= 2) {
		op = (CVI_S32)atoi(argv[1]);
		g_stitch_save_file = (CVI_BOOL)atoi(argv[2]);

		s32Ret = _stitch_handle_op(op);
		STITCH_UT_PRT("stitch ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	} else {
		g_stitch_save_file = CVI_TRUE;
		do {
			stitch_show_help();
			scanf("%d", &op);

			s32Ret = _stitch_handle_op(op);
			if (op != 255)
				STITCH_UT_PRT("stitch ut op[%d] %s\n", op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
		} while (op != 255);
	}

	return s32Ret;
}

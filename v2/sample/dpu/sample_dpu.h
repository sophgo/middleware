/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: sample_dpu.h
 * Description:
 */

#ifndef __SAMPLE_DPU_H__
#define __SAMPLE_DPU_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
// #include "../../modules/sys/include/dwa_mesh.h"
#include <linux/cvi_common.h>
#include <linux/cvi_comm_video.h>
#include "sample_common_dpu.h"


typedef enum _DPU_TEST_OP {
	DPU_MODE_SGBM_MUX0_TEST,
	DPU_MODE_SGBM_MUX1_TEST,
	DPU_MODE_SGBM_MUX2_TEST,
	DPU_MODE_ONLINE_MUX0_TEST,
	DPU_MODE_ONLINE_MUX1_TEST,
	DPU_MODE_ONLINE_MUX2_TEST,
	DPU_MODE_FGS_MUX0_TEST,
	DPU_MODE_FGS_MUX1_TEST,
	DPU_MODE_BTCOST_TEST
} DPU_TEST_OP;

CVI_S32 SAMPLE_DPU_SGBM_MUX0(SIZE_S stSize , CVI_CHAR *filenameL,CVI_CHAR *filenameR,CVI_CHAR *filenameOut);
CVI_S32 SAMPLE_DPU_SGBM_MUX1(SIZE_S stSize , CVI_CHAR *filenameL,CVI_CHAR *filenameR,CVI_CHAR *filenameOut);
CVI_S32 SAMPLE_DPU_SGBM_MUX2(SIZE_S stSize , CVI_CHAR *filenameL,CVI_CHAR *filenameR,CVI_CHAR *filenameOut);
CVI_S32 SAMPLE_DPU_SGBM_MUX3(SIZE_S stSize , CVI_CHAR *filenameL,CVI_CHAR *filenameR,CVI_CHAR *filenameOut);
CVI_S32 SAMPLE_DPU_ONLINE_MUX0(SIZE_S stSize , CVI_CHAR *filenameL,CVI_CHAR *filenameR,CVI_CHAR *filenameOut);
CVI_S32 SAMPLE_DPU_ONLINE_MUX1(SIZE_S stSize , CVI_CHAR *filenameL,CVI_CHAR *filenameR,CVI_CHAR *filenameOut);
CVI_S32 SAMPLE_DPU_ONLINE_MUX2(SIZE_S stSize , CVI_CHAR *filenameL,CVI_CHAR *filenameR,CVI_CHAR *filenameOut);
CVI_S32 SAMPLE_DPU_FGS_MUX0(SIZE_S stSize , CVI_CHAR *filenameL,CVI_CHAR *filenameR,CVI_CHAR *filenameOut);
CVI_S32 SAMPLE_DPU_FGS_MUX1(SIZE_S stSize , CVI_CHAR *filenameL,CVI_CHAR *filenameR,CVI_CHAR *filenameOut);

CVI_S32 SAMPLE_DPU_DWA_TEST(SIZE_S stSizeIn , SIZE_S stSizeOut,CVI_CHAR *filenameL,CVI_CHAR *filenameR,CVI_CHAR *filenameOut,
							CVI_CHAR * fileGridInfoL,CVI_CHAR * fileGridInfoR);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __SAMPLE_DPU_H__*/

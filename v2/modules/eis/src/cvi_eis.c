/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * Description: Electronic Image Stabilization (EIS) SDK implementation.
 *
 * Author: binshan.huang@sophgo.com
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "cvi_debug.h"
#include "cvi_sys.h"
#include "cvi_type.h"

#include "cvi_cpu_eis.h"
#include "cvi_eis.h"


typedef struct _EIS_HANDLE_S {

	EIS_CFG_S params;
	CVI_VOID *pHandle;

} _EIS_HANDLE_S;

CVI_S32 CVI_EIS_Init(EIS_CFG_S *pEisCfg, EIS_HANDLE_S **ppeis)
{

	if (pEisCfg == NULL) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "%s eis config is NULL.\n", __func__);
		return CVI_FAILURE;
	}

	*ppeis = (EIS_HANDLE_S *)malloc(sizeof(EIS_HANDLE_S));
	if (*ppeis == NULL) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "%s eis handle malloc failed.\n", __func__);
		return CVI_FAILURE;
	}
	memset(*ppeis, 0, sizeof(EIS_HANDLE_S));

	EIS_HANDLE_S *peis = *ppeis;

	peis->params = *pEisCfg;

	// TODO: maybe do some parameter check here

	if (cpu_eis_init(&peis->params, &peis->pHandle) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "%s cpu_eis_init failed.\n", __func__);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_EIS_DeInit(EIS_HANDLE_S **ppeis)
{

	if (*ppeis == NULL) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "%s eis handle is NULL.\n", __func__);
		return CVI_FAILURE;
	}

	EIS_HANDLE_S *peis = *ppeis;

	if (cpu_eis_free(&peis->pHandle) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "%s cpu_eis_free failed.\n", __func__);
		return CVI_FAILURE;
	}

	free(*ppeis);
	*ppeis = NULL;

	return CVI_SUCCESS;
}

CVI_S32 CVI_EIS_Process(EIS_HANDLE_S *peis, EIS_INPUT_INFO_S *stInput, EIS_OUTPUT_INFO_S *stOutput)
{

	if (peis == NULL) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "%s eis handle is NULL.\n", __func__);
		return CVI_FAILURE;
	}

	CVI_S32 s32Ret = CVI_FAILURE;

	CVI_S32(*pMeshX)[MAX_MESH_NUM][4] = stOutput->pMeshX;
	CVI_S32(*pMeshY)[MAX_MESH_NUM][4] = stOutput->pMeshY;
	CVI_U32 *pMeshNum = &stOutput->meshNum;

	CVI_U64 framePts = stInput->pts;
	CVI_U32 exposureTime = stInput->expTime;
	EIS_MOTION_BUF_S *pGyroData = &stInput->GyroData;

	s32Ret = cpu_eis(peis->pHandle, pGyroData, framePts, exposureTime,
						*pMeshX, *pMeshY, pMeshNum);
	stOutput->pFrame = stInput->pFrame;

	return s32Ret;
}

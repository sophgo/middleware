/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_dwa.h
 * Description:
 *   dwa interfaces.
 */

#ifndef __CVI_DWA_H__
#define __CVI_DWA_H__

#include <linux/cvi_common.h>
#include <linux/cvi_comm_video.h>
#include <linux/cvi_comm_gdc.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* Begin a dwa job,then add task into the job,dwa will finish all the task in the job.
 *
 * @param phHandle: GDC_HANDLE *phHandle
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_DWA_BeginJob(GDC_HANDLE *phHandle);

CVI_S32 CVI_DWA_SetJobIdentity(GDC_HANDLE hHandle, GDC_IDENTITY_ATTR_S *identity_attr);

/* End a job,all tasks in the job will be submmitted to dwa
 *
 * @param phHandle: GDC_HANDLE *phHandle
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_DWA_EndJob(GDC_HANDLE hHandle);

/* Cancel a job ,then all tasks in the job will not be submmitted to dwa
 *
 * @param phHandle: GDC_HANDLE *phHandle
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_DWA_CancelJob(GDC_HANDLE hHandle);

/* Add a fisheye task to a dwa job
 *
 * @param phHandle: GDC_HANDLE *phHandle
 * @param pstTask(RW): to describe what to do
 * @param pstFishEyeAttr: for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_DWA_AddCorrectionTask(GDC_HANDLE hHandle, GDC_TASK_ATTR_S *pstTask,
				  const FISHEYE_ATTR_S *pstFishEyeAttr);

/* Add a rotation task to a dwa job
 *
 * @param phHandle: GDC_HANDLE *phHandle
 * @param pstTask(RW): to describe what to do
 * @param enRotation: for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_DWA_AddRotationTask(GDC_HANDLE hHandle, GDC_TASK_ATTR_S *pstTask, ROTATION_E enRotation);

CVI_S32 CVI_DWA_AddAffineTask(GDC_HANDLE hHandle, GDC_TASK_ATTR_S *pstTask, const AFFINE_ATTR_S *pstAffineAttr);

CVI_S32 CVI_DWA_AddDewarpTask(GDC_HANDLE hHandle, GDC_TASK_ATTR_S *pstTask, const WARP_ATTR_S *pstWarpAttr);

CVI_S32 CVI_DWA_AddLDCTask(GDC_HANDLE hHandle, GDC_TASK_ATTR_S *pstTask,
	const LDC_ATTR_S *pstLDCAttr, ROTATION_E enRotation);

CVI_S32 CVI_DWA_GetWorkJob(GDC_HANDLE* phHandle);

CVI_S32 CVI_DWA_GetChnFrame(GDC_IDENTITY_ATTR_S *identity, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec);

/* set meshsize for rotation only
 *
 * @param nMeshHor: mesh counts horizontal
 * @param nMeshVer: mesh counts vertical
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_DWA_SetMeshSize(int nMeshHor, int nMeshVer);

CVI_S32 CVI_DWA_GetDevFd(void);

CVI_S32 CVI_DWA_Init(void);

CVI_S32 CVI_DWA_DeInit(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __CVI_DWA_H__ */

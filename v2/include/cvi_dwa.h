/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_dwa.h
 * Description:
 *   dwa interfaces.
 */

#ifndef __CVI_DWA_H__
#define __CVI_DWA_H__

#include <cvi_common.h>
#include <cvi_comm_video.h>
#include <cvi_comm_gdc.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* Begin a dwa job,then add task into the job,dwa will finish all the task in the job.
 *
 * @param phHandle(Out): GDC_HANDLE *phHandle
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_DWA_BeginJob(GDC_HANDLE *phHandle);

/* set identity attribution for a job ,identity attribution is unique id for a job.
 *
 * @param hHandle(In): GDC_HANDLE hHandle
 * @param identity_attr(In): GDC_IDENTITY_ATTR_S *identity_attr
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_DWA_SetJobIdentity(GDC_HANDLE hHandle, GDC_IDENTITY_ATTR_S *identity_attr);

/* End a job,all tasks in the job will be submmitted to dwa
 *
 * @param hHandle(In): GDC_HANDLE hHandle
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_DWA_EndJob(GDC_HANDLE hHandle);

/* Cancel a job ,then all tasks in the job will not be submmitted to dwa
 *
 * @param hHandle(In): GDC_HANDLE hHandle
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_DWA_CancelJob(GDC_HANDLE hHandle);

/* Add a fisheye task to a dwa job
 *
 * @param hHandle(In): GDC_HANDLE hHandle
 * @param pstTask(In): to describe what to do
 * @param pstFishEyeAttr(In): for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_DWA_AddCorrectionTask(GDC_HANDLE hHandle, GDC_TASK_ATTR_S *pstTask,
				  const FISHEYE_ATTR_S *pstFishEyeAttr);

/* Add a rotation task to a dwa job
 *
 * @param hHandle: GDC_HANDLE hHandle
 * @param pstTask(RW): to describe what to do
 * @param enRotation: for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_DWA_AddRotationTask(GDC_HANDLE hHandle, GDC_TASK_ATTR_S *pstTask, ROTATION_E enRotation);

/* Add a affine task to a dwa job.
 *
 * @param hHandle(In): GDC_HANDLE hHandle
 * @param pstTask(In): to describe what to do
 * @param pstAffineAttr(In): for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_DWA_AddAffineTask(GDC_HANDLE hHandle, GDC_TASK_ATTR_S *pstTask, const AFFINE_ATTR_S *pstAffineAttr);

/* Add a dewarp task to a dwa job.
 *
 * @param hHandle(In): GDC_HANDLE hHandle
 * @param pstTask(In): to describe what to do
 * @param pstWarpAttr(In): for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_DWA_AddDewarpTask(GDC_HANDLE hHandle, GDC_TASK_ATTR_S *pstTask, const WARP_ATTR_S *pstWarpAttr);

/* Add a ldc task to a dwa job.
 *
 * @param hHandle(In): GDC_HANDLE hHandle
 * @param pstTask(In): to describe what to do
 * @param pstLDCAttr(In): for further settings
 * @param enRotation(In): for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_DWA_AddLDCTask(GDC_HANDLE hHandle, GDC_TASK_ATTR_S *pstTask,
	const LDC_ATTR_S *pstLDCAttr, ROTATION_E enRotation);

/* get the job currently being used by dwa dev.
 *
 * @param phHandle(Out): GDC_HANDLE* phHandle
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_DWA_GetWorkJob(GDC_HANDLE* phHandle);

/* get frame for specific dwa job, It only works with async io.
 *
 * @param identity(In): GDC_IDENTITY_ATTR_S *identity
 * @param pstFrameInfo(Out): VIDEO_FRAME_INFO_S *pstFrameInfo
 * @param s32MilliSec(In): timeout
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_DWA_GetChnFrame(GDC_IDENTITY_ATTR_S *identity, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec);

/* set meshsize for dwa task
 *
 * @param nMeshHor(In): mesh counts horizontal
 * @param nMeshVer(In): mesh counts vertical
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_DWA_SetMeshSize(int nMeshHor, int nMeshVer);

/* get fd for dwa dev.
 *
 * @return fd
 */
CVI_S32 CVI_DWA_GetDevFd(void);

/* init dwa moudle, include dev and dwa software subsystem.
 *
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_DWA_Init(void);

/* deinit dwa moudle, include dev and dwa software subsystem.
 *
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_DWA_DeInit(void);

/* Suspend dwa dev and dwa software subsystem.
 *
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_DWA_Suspend(void);

/* Resume dwa dev and dwa software subsystem.
 *
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_DWA_Resume(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __CVI_DWA_H__ */

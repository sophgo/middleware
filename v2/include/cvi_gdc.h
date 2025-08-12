/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_gdc.h
 * Description:
 *   gdc interfaces.
 */

#ifndef __CVI_GDC_H__
#define __CVI_GDC_H__

#include <cvi_common.h>
#include <cvi_comm_video.h>
#include <cvi_comm_gdc.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* init gdc moudle, include dev and gdc software subsystem.
 *
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_Init(void);

/* deinit gdc moudle, include dev and gdc software subsystem.
 *
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_DeInit(void);

/* Begin a gdc job,then add task into the job,gdc will finish all the task in the job.
 *
 * @param phHandle(Out): GDC_HANDLE *phHandle
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_BeginJob(GDC_HANDLE *phHandle);

/* End a job,all tasks in the job will be submmitted to gdc.
 *
 * @param hHandle(In): GDC_HANDLE hHandle
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_EndJob(GDC_HANDLE hHandle);

/* Cancel a job ,then all tasks in the job will not be submmitted to gdc.
 *
 * @param hHandle(In): GDC_HANDLE hHandle
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_CancelJob(GDC_HANDLE hHandle);

/* set identity attribution for a job ,identity attribution is unique id for a job.
 *
 * @param hHandle(In): GDC_HANDLE hHandle
 * @param identity_attr(In): GDC_IDENTITY_ATTR_S *identity_attr
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_SetJobIdentity(GDC_HANDLE hHandle, GDC_IDENTITY_ATTR_S *identity_attr);

/* Add a fisheye task to a gdc job.
 *
 * @param hHandle(In): GDC_HANDLE hHandle
 * @param pstTask(In): to describe what to do
 * @param pstFisheyeAttr(In): for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_AddCorrectionTask(GDC_HANDLE hHandle, GDC_TASK_ATTR_S *pstTask,
				  const FISHEYE_ATTR_S *pstFisheyeAttr);

/* Add a rotation task to a gdc job.
 *
 * @param hHandle(In): GDC_HANDLE hHandle
 * @param pstTask(In): to describe what to do
 * @param enRotation(In): for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_AddRotationTask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask, ROTATION_E enRotation);

/* Add a affine task to a gdc job.
 *
 * @param hHandle(In): GDC_HANDLE hHandle
 * @param pstTask(In): to describe what to do
 * @param pstAffineAttr(In): for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_AddAffineTask(GDC_HANDLE hHandle, GDC_TASK_ATTR_S *pstTask, const AFFINE_ATTR_S *pstAffineAttr);

/* Add a ldc task to a gdc job.
 *
 * @param hHandle(In): GDC_HANDLE hHandle
 * @param pstTask(In): to describe what to do
 * @param pstLDCAttr(In): for further settings
 * @param enRotation(In): for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_AddLDCTask(GDC_HANDLE hHandle, GDC_TASK_ATTR_S *pstTask,
	const LDC_ATTR_S *pstLDCAttr, ROTATION_E enRotation);

/* Add a dewarp task to a gdc job.
 *
 * @param hHandle(In): GDC_HANDLE hHandle
 * @param pstTask(In): to describe what to do
 * @param pstWarpAttr(In): for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_AddDewarpTask(GDC_HANDLE hHandle, GDC_TASK_ATTR_S *pstTask,
				const WARP_ATTR_S *pstWarpAttr);

/* get the job currently being used by gdc dev.
 *
 * @param phHandle(Out): GDC_HANDLE* phHandle
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_GetWorkJob(GDC_HANDLE* phHandle);

/* get frame for specific gdc job, It only works with async io.
 *
 * @param identity(In): GDC_IDENTITY_ATTR_S *identity
 * @param pstFrameInfo(Out): VIDEO_FRAME_INFO_S *pstFrameInfo
 * @param s32MilliSec(In): timeout
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_GetChnFrame(GDC_IDENTITY_ATTR_S *identity, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec);

/* get fd for gdc dev.
 *
 * @return fd
 */
CVI_S32 CVI_GDC_GetDevFd(void);

/* Suspend gdc dev and gdc software subsystem.
 *
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_Suspend(void);

/* Resume gdc dev and gdc software subsystem.
 *
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_Resume(void);

/* attach vb pool for specific gdc callback, It only works with internal mode.
 *
 * @param pChn(In): chn which to call gdc
 * @param u32VbPool(In): vb pool to attach
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_AttachVbPool(MMF_CHN_S *pChn, VB_POOL u32VbPool);

/* detach vb pool for specific gdc callback, It only works with internal mode.
 *
 * @param pChn(In): chn which to call gdc
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_DetachVbPool(MMF_CHN_S *pChn);

/* Update dwa mesh.
 *
 * @param bindName(In): bind name, compatible GRID_INFO_ATTR_S gridBindName
 * @param src_x_mesh(In): src mesh x coordinate
 * @param src_y_mesh(In): src mesh y coordinate
 * @param dst_x_mesh(In): dst mesh x coordinate
 * @param dst_y_mesh(In): dst mesh x coordinate
 * @param len(nbr_mesh): mesh coordinate grid num
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_UpdateMeshCoordinate(char *bindName,
	int src_x_mesh[][4], int src_y_mesh[][4], int dst_x_mesh[][4], int dst_y_mesh[][4], int nbr_mesh);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __CVI_GDC_H__ */

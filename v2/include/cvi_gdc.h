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
CVI_S32 CVI_GDC_AddCorrectionTask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask,
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
CVI_S32 CVI_GDC_AddAffineTask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask, const AFFINE_ATTR_S *pstAffineAttr);

/* Add a ldc task to a gdc job.
 *
 * @param hHandle(In): GDC_HANDLE hHandle
 * @param pstTask(In): to describe what to do
 * @param pstLDCAttr(In): for further settings
 * @param enRotation(In): for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_AddLDCTask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask,
	const LDC_ATTR_S *pstLDCAttr, ROTATION_E enRotation);

// color night vision (NA)
CVI_S32 CVI_GDC_AddCnvWarpTask(const float *pfmesh_data, GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask,
			       const FISHEYE_ATTR_S *pstAffineAttr, bool *bReNew);

// color night vision (NA)
CVI_S32 CVI_GDC_AddCorrectionTaskCNV(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask,
		const FISHEYE_ATTR_S *pstFishEyeAttr, uint8_t *p_tbl, uint8_t *p_idl, uint32_t *tbl_param);

/* set meshsize for gdc task(NA)
 *
 * @param nMeshHor(In): mesh counts horizontal
 * @param nMeshVer(In): mesh counts vertical
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_SetMeshSize(int nMeshHor, int nMeshVer);

/* set slice buffer attribution for gdc task(NA)
 *
 * @param hHandle(In): GDC_HANDLE *phHandlel
 * @param pstTask(In): GDC_TASK_ATTR_S *pstTask
 * @param pstBufWrap(In): LDC_BUF_WRAP_S *pstBufWrap
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_SetBufWrapAttr(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask, const LDC_BUF_WRAP_S *pstBufWrap);

/* get slice buffer attribution for gdc task(NA)
 *
 * @param hHandle(In): GDC_HANDLE *phHandlel
 * @param pstTask(In): GDC_TASK_ATTR_S *pstTask
 * @param pstBufWrap(Out): LDC_BUF_WRAP_S *pstBufWrap
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_GetBufWrapAttr(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask, LDC_BUF_WRAP_S *pstBufWrap);

/* dump mesh data for gdc task, mesh specific that
 * coordinate mapping relationship between the original image and the target image.
 *
 * @param pMeshDumpAttr(In): MESH_DUMP_ATTR_S *pMeshDumpAttr
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_DumpMesh(MESH_DUMP_ATTR_S *pMeshDumpAttr);

/* load mesh data for gdc task, mesh specific that
 * coordinate mapping relationship between the original image and the target image.
 *
 * @param pMeshDumpAttr(In): MESH_DUMP_ATTR_S *pMeshDumpAttr
 * @param LDC_ATTR_S(In): LDC_ATTR_S *pstLDCAttr
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_LoadMesh(MESH_DUMP_ATTR_S *pMeshDumpAttr, const LDC_ATTR_S *pstLDCAttr);

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

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __CVI_GDC_H__ */

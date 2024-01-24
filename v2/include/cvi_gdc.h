/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_gdc.h
 * Description:
 *   gdc interfaces.
 */

#ifndef __CVI_GDC_H__
#define __CVI_GDC_H__

#include <linux/cvi_common.h>
#include <linux/cvi_comm_video.h>
#include <linux/cvi_comm_gdc.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

CVI_S32 CVI_GDC_Init(void);

CVI_S32 CVI_GDC_DeInit(void);


/* Begin a gdc job,then add task into the job,gdc will finish all the task in the job.
 *
 * @param phHandle: GDC_HANDLE *phHandle
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_BeginJob(GDC_HANDLE *phHandle);

/* End a job,all tasks in the job will be submmitted to gdc
 *
 * @param phHandle: GDC_HANDLE *phHandle
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_EndJob(GDC_HANDLE hHandle);

/* Cancel a job ,then all tasks in the job will not be submmitted to gdc
 *
 * @param phHandle: GDC_HANDLE *phHandle
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_CancelJob(GDC_HANDLE hHandle);

CVI_S32 CVI_GDC_SetJobIdentity(GDC_HANDLE hHandle, GDC_IDENTITY_ATTR_S *identity_attr);

/* Add a fisheye task to a gdc job
 *
 * @param phHandle: GDC_HANDLE *phHandle
 * @param pstTask: to describe what to do
 * @param pstFisheyeAttr: for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_AddCorrectionTask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask,
				  const FISHEYE_ATTR_S *pstFisheyeAttr);

/* Add a rotation task to a gdc job
 *
 * @param phHandle: GDC_HANDLE *phHandle
 * @param pstTask: to describe what to do
 * @param enRotation: for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_AddRotationTask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask, ROTATION_E enRotation);

CVI_S32 CVI_GDC_AddAffineTask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask, const AFFINE_ATTR_S *pstAffineAttr);

CVI_S32 CVI_GDC_AddLDCTask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask,
	const LDC_ATTR_S *pstLDCAttr, ROTATION_E enRotation);

// color night vision
CVI_S32 CVI_GDC_AddCnvWarpTask(const float *pfmesh_data, GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask,
			       const FISHEYE_ATTR_S *pstAffineAttr, bool *bReNew);

CVI_S32 CVI_GDC_AddCorrectionTaskCNV(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask,
		const FISHEYE_ATTR_S *pstFishEyeAttr, uint8_t *p_tbl, uint8_t *p_idl, uint32_t *tbl_param);

/* set meshsize for rotation only
 *
 * @param nMeshHor: mesh counts horizontal
 * @param nMeshVer: mesh counts vertical
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_SetMeshSize(int nMeshHor, int nMeshVer);

CVI_S32 CVI_GDC_SetBufWrapAttr(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask, const LDC_BUF_WRAP_S *pstBufWrap);
CVI_S32 CVI_GDC_GetBufWrapAttr(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask, LDC_BUF_WRAP_S *pstBufWrap);

CVI_S32 CVI_GDC_DumpMesh(MESH_DUMP_ATTR_S *pMeshDumpAttr);
CVI_S32 CVI_GDC_LoadMesh(MESH_DUMP_ATTR_S *pMeshDumpAttr, const LDC_ATTR_S *pstLDCAttr);

CVI_S32 CVI_GDC_GetWorkJob(GDC_HANDLE* phHandle);

CVI_S32 CVI_GDC_GetChnFrame(GDC_IDENTITY_ATTR_S *identity, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec);

CVI_S32 CVI_GDC_GetDevFd(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __CVI_GDC_H__ */

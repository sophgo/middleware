/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: module/vpu/include/dwa_mesh.h
 * Description:
 *   DWA's mesh generator for hw.
 */
#include <sys/queue.h>

#ifndef MODULES_VPU_INCLUDE_DWA_MESH_H_
#define MODULES_VPU_INCLUDE_DWA_MESH_H_

#define CVI_DWA_MAGIC 0xbabeface

#define CVI_DWA_MESH_SIZE_ROT 0x60000
#define CVI_DWA_MESH_SIZE_AFFINE 0x20000
#define CVI_DWA_MESH_SIZE_FISHEYE 0xB0000
#define DWA_MAX_TSK_MESH (32)

#define DWA_DUMP_MESH 0

enum dwa_task_type {
	DWA_TASK_TYPE_ROT = 0,
	DWA_TASK_TYPE_FISHEYE,
	DWA_TASK_TYPE_AFFINE,
	DWA_TASK_TYPE_LDC,
	DWA_TASK_TYPE_WARP,
	DWA_TASK_TYPE_MAX,
};

/* dwa_task_param: the dwa task.
 *
 * stTask: define the in/out image info.
 * type: the type of dwa task.
 * param: the parameters for dwa task.
 */
struct dwa_task_param {
	STAILQ_ENTRY(dwa_task_param) stailq;

	GDC_TASK_ATTR_S stTask;
	enum dwa_task_type type;
	union {
		ROTATION_E enRotation;
		FISHEYE_ATTR_S stFishEyeAttr;
		AFFINE_ATTR_S stAffineAttr;
		LDC_ATTR_S stLDCAttr;
	};
};

/* dwa_job: the handle of dwa.
 *
 * ctx: the list of dwa task in the dwa job.
 * mutex: used if this job is sync-io.
 * cond: used if this job is sync-io.
 * sync_io: CVI_DWA_EndJob() will blocked until done is this is true.
 *          only meaningful if internal module use dwa.
 *          Default true;
 */
struct dwa_job {
	STAILQ_ENTRY(dwa_job) stailq;

	STAILQ_HEAD(dwa_job_ctx, dwa_task_param) ctx;
	pthread_cond_t cond;
	CVI_BOOL sync_io;
};

enum dwa_job_state {
	DWA_JOB_SUCCESS = 0,
	DWA_JOB_FAIL,
	DWA_JOB_WORKING,
};

struct dwa_job_info {
	CVI_S64 hHandle;
	MOD_ID_E enModId; // the module submitted dwa job
	CVI_U32 u32TaskNum; // number of tasks
	enum dwa_job_state eState; // job state
	CVI_U32 u32InSize;
	CVI_U32 u32OutSize;
	CVI_U32 u32CostTime; // From job submitted to job done
	CVI_U32 u32HwTime; // HW cost time
	CVI_U32 u32BusyTime; // From job submitted to job commit to driver
	CVI_U64 u64SubmitTime; // us
};

struct dwa_job_status {
	CVI_U32 u32Success;
	CVI_U32 u32Fail;
	CVI_U32 u32Cancel;
	CVI_U32 u32BeginNum;
	CVI_U32 u32BusyNum;
	CVI_U32 u32ProcingNum;
};

struct dwa_task_status {
	CVI_U32 u32Success;
	CVI_U32 u32Fail;
	CVI_U32 u32Cancel;
	CVI_U32 u32BusyNum;
};

struct dwa_operation_status {
	CVI_U32 u32AddTaskSuc;
	CVI_U32 u32AddTaskFail;
	CVI_U32 u32EndSuc;
	CVI_U32 u32EndFail;
	CVI_U32 u32CbCnt;
};

int dwa_get_mesh_size(int *p_mesh_hor, int *p_mesh_ver);
int dwa_set_mesh_size(int mesh_hor, int mesh_ver);
void dwa_mesh_gen_get_size(SIZE_S in_size, SIZE_S out_size, CVI_U32 *mesh_id_size, CVI_U32 *mesh_tbl_size);
void dwa_mesh_gen_rotation(SIZE_S in_size, SIZE_S out_size, ROTATION_E rot, uint64_t mesh_phy_addr, void *mesh_vir_addr);
void dwa_mesh_gen_affine(SIZE_S in_size, SIZE_S out_size, const AFFINE_ATTR_S *pstAffineAttr, uint64_t mesh_phy_addr,
		     void *mesh_vir_addr);
CVI_S32 dwa_mesh_gen_fisheye(SIZE_S in_size, SIZE_S out_size, const FISHEYE_ATTR_S *pstFishEyeAttr, uint64_t mesh_phy_addr,
		      void *mesh_vir_addr, ROTATION_E rot);
CVI_S32 dwa_mesh_gen_ldc(SIZE_S in_size, SIZE_S out_size, const LDC_ATTR_S *pstLDCAttr,
		     uint64_t mesh_phy_addr, void *mesh_vir_addr);
CVI_S32 dwa_mesh_gen_warp(SIZE_S in_size, SIZE_S out_size, const WARP_ATTR_S *pstWarpAttr, uint64_t mesh_phy_addr,
		      void *mesh_vir_addr);
// cnv
void dwa_mesh_gen_cnv(const float *pfmesh_data, SIZE_S in_size, SIZE_S out_size, const FISHEYE_ATTR_S *pstFishEyeAttr,
		  uint64_t mesh_phy_addr, void *mesh_vir_addr);

void dwa_get_cnv_warp_mesh_tbl(SIZE_S in_size, SIZE_S out_size, const AFFINE_ATTR_S *pstAffineAttr, uint64_t mesh_phy_addr,
			   void *mesh_vir_addr);

CVI_U8 get_valid_tsk_mesh_by_name(const CVI_CHAR *name);
CVI_U8 get_valid_tsk_mesh_by_name2(const CVI_CHAR *name);
CVI_U8 get_idle_tsk_mesh(void);
void free_cur_tsk_mesh(CVI_CHAR *meshName);
void free_all_tsk_mesh(void);

CVI_S32 CVI_DWA_GenLDCMesh(CVI_U32 u32Width, CVI_U32 u32Height, const LDC_ATTR_S *pstLDCAttr,
		const char *name, CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr);
CVI_VOID CVI_DWA_FreeCurTaskMesh(CVI_CHAR *tskName);

CVI_S32 CVI_DWA_GenFishEyeMesh(CVI_U32 u32Width, CVI_U32 u32Height, const FISHEYE_ATTR_S *pstFishEyeAttr,
		const char *name, CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr);

#endif // MODULES_VPU_INCLUDE_DWA_MESH_H_

/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: module/vpu/include/gdc_mesh.h
 * Description:
 *   GDC's mesh generator for hw.
 */
#include <sys/queue.h>
#include "grid_info.h"

#ifndef MODULES_VPU_INCLUDE_GDC_MESH_H_
#define MODULES_VPU_INCLUDE_GDC_MESH_H_

#define CVI_GDC_MESH_SIZE_AFFINE 0x20000
#define CVI_GDC_MESH_SIZE_FISHEYE 0x150000
#define GDC_MAX_TSK_MESH (32)

typedef struct _TSK_MESH_ATTR_S {
	CVI_CHAR Name[32];
	CVI_U64 paddr;
	CVI_VOID *vaddr;
} TSK_MESH_ATTR_S;

void mesh_gen_get_size(SIZE_S in_size, SIZE_S out_size, CVI_U32 *mesh_id_size, CVI_U32 *mesh_tbl_size);

void gdc_mesh_gen_affine(SIZE_S in_size, SIZE_S out_size, const AFFINE_ATTR_S *pstAffineAttr, uint64_t mesh_phy_addr,
		     void *mesh_vir_addr);
CVI_S32 gdc_mesh_gen_fisheye(SIZE_S in_size, SIZE_S out_size, const FISHEYE_ATTR_S *pstFishEyeAttr, uint64_t mesh_phy_addr,
		      void *mesh_vir_addr, ROTATION_E rot);
CVI_S32 gdc_mesh_gen_ldc(SIZE_S in_size, SIZE_S out_size, const LDC_ATTR_S *pstLDCAttr,
		     uint64_t mesh_phy_addr, void *mesh_vir_addr);
CVI_S32 gdc_mesh_gen_warp(SIZE_S in_size, SIZE_S out_size, const WARP_ATTR_S *pstWarpAttr, uint64_t mesh_phy_addr,
		      void *mesh_vir_addr);

CVI_U8 get_valid_tsk_mesh_by_name(const CVI_CHAR *name);
CVI_U8 get_valid_tsk_mesh_by_name2(const CVI_CHAR *name);
CVI_U8 get_idle_tsk_mesh(void);
void free_cur_tsk_mesh(CVI_CHAR *meshName);
void free_all_tsk_mesh(void);

int gdc_mesh_update_src_coordinate(int isrc_x_mesh_tbl[][4], int isrc_y_mesh_tbl[][4], MESH_DATA_EIS_S *p_MeshEIS);

CVI_S32 CVI_GDC_GenLDCMesh(CVI_U32 u32Width, CVI_U32 u32Height, const LDC_ATTR_S *pstLDCAttr,
		const char *name, CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr);

CVI_VOID CVI_GDC_FreeCurTaskMesh(CVI_CHAR *tskName);

CVI_S32 CVI_GDC_GenFishEyeMesh(CVI_U32 u32Width, CVI_U32 u32Height, const FISHEYE_ATTR_S *pstFishEyeAttr,
		const char *name, CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr);

extern TSK_MESH_ATTR_S tskMesh[GDC_MAX_TSK_MESH];

#endif // MODULES_VPU_INCLUDE_GDC_MESH_H_

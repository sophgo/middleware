#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "dwa_uapi.h"
#include "cvi_buffer.h"
#include "cvi_base.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_dwa.h"
#include "dwa_mesh.h"
#include "gdc_mesh.h"
#include "dwa_ioctl.h"
#include "grid_info.h"

#define DWA_YUV_BLACK 0x808000
#define DWA_RGB_BLACK 0x0

#define CHECK_DWA_FORMAT(imgIn, imgOut)                                                                                \
	do {                                                                                                           \
		if (imgIn.stVFrame.enPixelFormat != imgOut.stVFrame.enPixelFormat) {                                   \
			CVI_TRACE_DWA(CVI_DBG_ERR, "in/out pixelformat(%d-%d) mismatch\n",                             \
				      imgIn.stVFrame.enPixelFormat, imgOut.stVFrame.enPixelFormat);                    \
			return CVI_ERR_DWA_ILLEGAL_PARAM;                                                              \
		}                                                                                                      \
		if (!DWA_SUPPORT_FMT(imgIn.stVFrame.enPixelFormat)) {                                                  \
			CVI_TRACE_DWA(CVI_DBG_ERR, "pixelformat(%d) unsupported\n", imgIn.stVFrame.enPixelFormat);     \
			return CVI_ERR_DWA_ILLEGAL_PARAM;                                                              \
		}                                                                                                      \
	} while (0)

#define DEV_DWA_NAME "/dev/soph-dwa"

extern TSK_MESH_ATTR_S dwa_tskMesh[DWA_MAX_TSK_MESH];
static CVI_S32 dwa_fd = -1;
static pthread_mutex_t dwa_fd_lock = PTHREAD_MUTEX_INITIALIZER;

static CVI_S32 dwa_dev_close(CVI_VOID)
{
	pthread_mutex_lock(&dwa_fd_lock);
	close_device(&dwa_fd);
	pthread_mutex_unlock(&dwa_fd_lock);

	return CVI_SUCCESS;
}

CVI_S32 get_dwa_fd(CVI_VOID)
{
	pthread_mutex_lock(&dwa_fd_lock);
	if (dwa_fd <= 0) {
		if (open_device(DWA_DEV_NAME, &dwa_fd) == -1) {
			perror("DWA open fail\n");
			dwa_fd = -1;
		}
	}
	pthread_mutex_unlock(&dwa_fd_lock);

	return dwa_fd;
}

static CVI_S32 dwa_rotation_check_size(ROTATION_E enRotation, const GDC_TASK_ATTR_S *pstTask)
{
	if (enRotation >= ROTATION_MAX) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "invalid rotation(%d).\n", enRotation);
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}

	if (enRotation == ROTATION_90 || enRotation == ROTATION_270 || enRotation == ROTATION_XY_FLIP) {
		if (pstTask->stImgOut.stVFrame.u32Width < pstTask->stImgIn.stVFrame.u32Height) {
			CVI_TRACE_DWA(CVI_DBG_ERR, "rotation(%d) invalid: 'output width(%d) < input height(%d)'\n",
				      enRotation, pstTask->stImgOut.stVFrame.u32Width,
				      pstTask->stImgIn.stVFrame.u32Height);
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
		if (pstTask->stImgOut.stVFrame.u32Height < pstTask->stImgIn.stVFrame.u32Width) {
			CVI_TRACE_DWA(CVI_DBG_ERR, "rotation(%d) invalid: 'output height(%d) < input width(%d)'\n",
				      enRotation, pstTask->stImgOut.stVFrame.u32Height,
				      pstTask->stImgIn.stVFrame.u32Width);
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
	} else {
		if (pstTask->stImgOut.stVFrame.u32Width < pstTask->stImgIn.stVFrame.u32Width) {
			CVI_TRACE_DWA(CVI_DBG_ERR, "rotation(%d) invalid: 'output width(%d) < input width(%d)'\n",
				      enRotation, pstTask->stImgOut.stVFrame.u32Width,
				      pstTask->stImgIn.stVFrame.u32Width);
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
		if (pstTask->stImgOut.stVFrame.u32Height < pstTask->stImgIn.stVFrame.u32Height) {
			CVI_TRACE_DWA(CVI_DBG_ERR, "rotation(%d) invalid: 'output height(%d) < input height(%d)'\n",
				      enRotation, pstTask->stImgOut.stVFrame.u32Height,
				      pstTask->stImgIn.stVFrame.u32Height);
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
	}

	return CVI_SUCCESS;
}

/**************************************************************************
 *   Public APIs.
 **************************************************************************/

CVI_S32 CVI_DWA_Suspend(void)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_dwa_fd();

	s32Ret = dwa_suspend(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "suspend fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_DWA_Resume(void)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_dwa_fd();

	s32Ret = dwa_resume(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "resume fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_DWA_BeginJob(GDC_HANDLE *phHandle)
{
	CVI_S32 fd = get_dwa_fd();
	struct dwa_handle_data cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_DWA, phHandle);


	memset(&cfg, 0, sizeof(cfg));
	if (dwa_begin_job(fd, &cfg))
		return CVI_FAILURE;

	*phHandle = cfg.handle;
	return CVI_SUCCESS;
}

CVI_S32 CVI_DWA_SetJobIdentity(GDC_HANDLE hHandle, GDC_IDENTITY_ATTR_S *identity_attr)
{
	CVI_S32 fd = get_dwa_fd();
	struct dwa_identity_attr cfg = {0};

	MOD_CHECK_NULL_PTR(CVI_ID_DWA, identity_attr);

	if (!hHandle) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "null hHandle");
		return CVI_ERR_DWA_NULL_PTR;
	}

	cfg.handle = hHandle;
	memcpy(&cfg.attr, identity_attr, sizeof(*identity_attr));

	return dwa_set_job_identity(fd, &cfg);
}

CVI_S32 CVI_DWA_EndJob(GDC_HANDLE hHandle)
{
	CVI_S32 fd = get_dwa_fd();
	struct dwa_handle_data cfg;

	if (!hHandle) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "null hHandle");
		return CVI_ERR_DWA_NULL_PTR;
	}

	memset(&cfg, 0, sizeof(cfg));
	cfg.handle = hHandle;
	return dwa_end_job(fd, &cfg);
}

CVI_S32 CVI_DWA_CancelJob(GDC_HANDLE hHandle)
{
	CVI_S32 fd = get_dwa_fd();
	struct dwa_handle_data cfg;

	if (!hHandle) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "null hHandle");
		return CVI_ERR_DWA_NULL_PTR;
	}

	memset(&cfg, 0, sizeof(cfg));
	cfg.handle = hHandle;
	return dwa_cancel_job(fd, &cfg);
}

CVI_S32 CVI_DWA_AddCorrectionTask(GDC_HANDLE hHandle, GDC_TASK_ATTR_S *pstTask,
				const FISHEYE_ATTR_S *pstFishEyeAttr)
{
	CVI_S32 fd = get_dwa_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_DWA, pstTask);
	MOD_CHECK_NULL_PTR(CVI_ID_DWA, pstFishEyeAttr);
	CHECK_DWA_FORMAT(pstTask->stImgIn, pstTask->stImgOut);

	if (!hHandle) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "null hHandle");
		return CVI_ERR_DWA_NULL_PTR;
	}

	if (pstFishEyeAttr->bEnable) {
		if(!pstFishEyeAttr->stGridInfoAttr.Enable) {
			if (pstFishEyeAttr->u32RegionNum == 0) {
			CVI_TRACE_DWA(CVI_DBG_ERR, "RegionNum(%d) can't be 0 if enable fisheye.\n",
				      pstFishEyeAttr->u32RegionNum);
			return CVI_ERR_DWA_ILLEGAL_PARAM;
			}
			if (pstFishEyeAttr->enUseMode == MODE_01_1O || pstFishEyeAttr->enUseMode == MODE_STEREO_FIT) {
				CVI_TRACE_DWA(CVI_DBG_ERR, "FISHEYE not support MODE_01_1O and MODE_STEREO_FIT.\n");
				return CVI_ERR_DWA_ILLEGAL_PARAM;
			}
			if (((CVI_U32)pstFishEyeAttr->s32HorOffset > pstTask->stImgIn.stVFrame.u32Width) ||
				((CVI_U32)pstFishEyeAttr->s32VerOffset > pstTask->stImgIn.stVFrame.u32Height)) {
				CVI_TRACE_DWA(CVI_DBG_ERR, "center pos(%d %d) out of frame size(%d %d).\n",
						pstFishEyeAttr->s32HorOffset, pstFishEyeAttr->s32VerOffset,
						pstTask->stImgIn.stVFrame.u32Width, pstTask->stImgIn.stVFrame.u32Height);
				return CVI_ERR_DWA_ILLEGAL_PARAM;
			}
			for (CVI_U32 i = 0; i < pstFishEyeAttr->u32RegionNum; ++i) {
				if ((pstFishEyeAttr->enMountMode == FISHEYE_WALL_MOUNT) &&
					(pstFishEyeAttr->astFishEyeRegionAttr[i].enViewMode == FISHEYE_VIEW_360_PANORAMA)) {
					CVI_TRACE_DWA(CVI_DBG_ERR, "Rgn(%d): WALL_MOUNT not support Panorama_360.\n", i);
					return CVI_ERR_DWA_ILLEGAL_PARAM;
				}
				if ((pstFishEyeAttr->enMountMode == FISHEYE_CEILING_MOUNT) &&
					(pstFishEyeAttr->astFishEyeRegionAttr[i].enViewMode == FISHEYE_VIEW_180_PANORAMA)) {
					CVI_TRACE_DWA(CVI_DBG_ERR, "Rgn(%d): CEILING_MOUNT not support Panorama_180.\n", i);
					return CVI_ERR_DWA_ILLEGAL_PARAM;
				}
				if ((pstFishEyeAttr->enMountMode == FISHEYE_DESKTOP_MOUNT) &&
					(pstFishEyeAttr->astFishEyeRegionAttr[i].enViewMode == FISHEYE_VIEW_180_PANORAMA)) {
					CVI_TRACE_DWA(CVI_DBG_ERR, "Rgn(%d): DESKTOP_MOUNT not support Panorama_180.\n", i);
					return CVI_ERR_DWA_ILLEGAL_PARAM;
				}
			}
		}
	} else {
		CVI_TRACE_DWA(CVI_DBG_ERR, "FishEyeAttr is not be enabled.\n");
		return CVI_ERR_DWA_ILLEGAL_PARAM;
	}

	struct dwa_task_attr attr;
	CVI_U64 paddr;
	CVI_VOID *vaddr;
	SIZE_S in_size, out_size;

	in_size.u32Width = pstTask->stImgIn.stVFrame.u32Width;
	in_size.u32Height = pstTask->stImgIn.stVFrame.u32Height;
	out_size.u32Width = pstTask->stImgOut.stVFrame.u32Width;
	out_size.u32Height = pstTask->stImgOut.stVFrame.u32Height;

	CVI_U8 idx = get_valid_tsk_mesh_by_name(pstTask->name);
	if (idx >= DWA_MAX_TSK_MESH) {
		if (CVI_SYS_IonAlloc_Cached(&paddr, &vaddr, pstTask->name, CVI_DWA_MESH_SIZE_FISHEYE) != CVI_SUCCESS) {
			CVI_TRACE_DWA(CVI_DBG_ERR, "Can't acquire memory for mesh.\n");
			return CVI_ERR_DWA_NOBUF;
		}

		if (dwa_mesh_gen_fisheye(in_size, out_size, pstFishEyeAttr, paddr, vaddr, ROTATION_0)) {
			CVI_TRACE_DWA(CVI_DBG_ERR, "dwa_mesh_gen_fisheye failed\n");
			goto MESH_GEN_FAIL;
		}

		CVI_SYS_IonFlushCache(paddr, vaddr, CVI_DWA_MESH_SIZE_FISHEYE);

#if DWA_DUMP_MESH
		FILE * fp;

		fp = fopen(pstTask->name, "wb");
		if (!fp) {
			CVI_TRACE_DWA(CVI_DBG_ERR, "open file failed.\n");
			return CVI_ERR_DWA_ILLEGAL_PARAM;
		}
		fwrite(vaddr, CVI_DWA_MESH_SIZE_FISHEYE, 1, fp);
		fflush(fp);
		fclose(fp);
#endif
		idx = get_idle_tsk_mesh();
		if (idx >= DWA_MAX_TSK_MESH) {
			CVI_TRACE_DWA(CVI_DBG_ERR, "tsk mesh count(%d) is out of range(%d)\n", idx + 1, DWA_MAX_TSK_MESH);
			CVI_SYS_IonFree(paddr, vaddr);
			return CVI_ERR_DWA_NOT_PERMITTED;
		}

		strcpy(dwa_tskMesh[idx].Name, pstTask->name);
		dwa_tskMesh[idx].paddr = paddr;
		dwa_tskMesh[idx].vaddr = vaddr;
	}

	memset(&attr, 0, sizeof(attr));
	attr.handle = hHandle;
	memcpy(&attr.stImgIn, &pstTask->stImgIn, sizeof(attr.stImgIn));
	memcpy(&attr.stImgOut, &pstTask->stImgOut, sizeof(attr.stImgOut));
	//memcpy(attr.au64privateData, pstTask->au64privateData, sizeof(attr.au64privateData));
	memcpy(&attr.stFishEyeAttr, pstFishEyeAttr, sizeof(*pstFishEyeAttr));
	attr.reserved = pstTask->reserved;
	attr.au64privateData[0] = dwa_tskMesh[idx].paddr;

	pstTask->au64privateData[0] = dwa_tskMesh[idx].paddr;
	pstTask->au64privateData[1] = (CVI_U64)((uintptr_t)dwa_tskMesh[idx].vaddr);
	return dwa_add_correction_task(fd, &attr);

MESH_GEN_FAIL:
	if (paddr && vaddr)
		CVI_SYS_IonFree(paddr, vaddr);
	return CVI_FAILURE;
}

CVI_S32 CVI_DWA_AddDewarpTask(GDC_HANDLE hHandle, GDC_TASK_ATTR_S *pstTask,
				const WARP_ATTR_S *pstWarpAttr)
{
	CVI_S32 fd = get_dwa_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_DWA, pstTask);
	MOD_CHECK_NULL_PTR(CVI_ID_DWA, pstWarpAttr);
	CHECK_DWA_FORMAT(pstTask->stImgIn, pstTask->stImgOut);

	if (!hHandle) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "null hHandle");
		return CVI_ERR_DWA_NULL_PTR;
	}

	if (!pstWarpAttr->bEnable) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "WarpAttr is not be enabled.");
		return CVI_ERR_DWA_ILLEGAL_PARAM;
	}

	if (!pstWarpAttr->stGridInfoAttr.Enable) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "GridInfoAttr is not be enabled.");
		return CVI_ERR_DWA_ILLEGAL_PARAM;
	}

	struct dwa_task_attr attr;
	CVI_U64 paddr;
	CVI_VOID *vaddr;
	SIZE_S in_size, out_size;

	in_size.u32Width = pstTask->stImgIn.stVFrame.u32Width;
	in_size.u32Height = pstTask->stImgIn.stVFrame.u32Height;
	out_size.u32Width = pstTask->stImgOut.stVFrame.u32Width;
	out_size.u32Height = pstTask->stImgOut.stVFrame.u32Height;

	CVI_U8 idx = get_valid_tsk_mesh_by_name(pstTask->name);
	if (idx >= DWA_MAX_TSK_MESH) {
		if (CVI_SYS_IonAlloc_Cached(&paddr, &vaddr, pstTask->name, CVI_DWA_MESH_SIZE_FISHEYE) != CVI_SUCCESS) {
			CVI_TRACE_DWA(CVI_DBG_ERR, "Can't acquire memory for mesh.\n");
			return CVI_ERR_DWA_NOBUF;
		}

		if (dwa_mesh_gen_warp(in_size, out_size, pstWarpAttr, paddr, vaddr)) {
			CVI_TRACE_DWA(CVI_DBG_ERR, "dwa_mesh_gen_warp failed\n");
			goto MESH_GEN_FAIL;
		}

		CVI_SYS_IonFlushCache(paddr, vaddr, CVI_DWA_MESH_SIZE_FISHEYE);

		idx = get_idle_tsk_mesh();
		if (idx >= DWA_MAX_TSK_MESH) {
			CVI_TRACE_DWA(CVI_DBG_ERR, "tsk mesh count(%d) is out of range(%d)\n", idx + 1, DWA_MAX_TSK_MESH);
			CVI_SYS_IonFree(paddr, vaddr);
			return CVI_ERR_DWA_NOT_PERMITTED;
		}

		strcpy(dwa_tskMesh[idx].Name, pstTask->name);
		dwa_tskMesh[idx].paddr = paddr;
		dwa_tskMesh[idx].vaddr = vaddr;
	}

	memset(&attr, 0, sizeof(attr));
	attr.handle = hHandle;
	memcpy(&attr.stImgIn, &pstTask->stImgIn, sizeof(attr.stImgIn));
	memcpy(&attr.stImgOut, &pstTask->stImgOut, sizeof(attr.stImgOut));
	//memcpy(attr.au64privateData, pstTask->au64privateData, sizeof(attr.au64privateData));
	memcpy(&attr.stWarpAttr,  pstWarpAttr, sizeof(*pstWarpAttr));
	attr.reserved = pstTask->reserved;
	attr.au64privateData[0] = dwa_tskMesh[idx].paddr;

	pstTask->au64privateData[0] = dwa_tskMesh[idx].paddr;
	pstTask->au64privateData[1] = (CVI_U64)((uintptr_t)dwa_tskMesh[idx].vaddr);
	return dwa_add_warp_task(fd, &attr);

MESH_GEN_FAIL:
	if (paddr && vaddr)
		CVI_SYS_IonFree(paddr, vaddr);
	return CVI_FAILURE;
}

CVI_S32 CVI_DWA_AddRotationTask(GDC_HANDLE hHandle, GDC_TASK_ATTR_S *pstTask, ROTATION_E enRotation)
{
	CVI_S32 fd = get_dwa_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_DWA, pstTask);
	CHECK_DWA_FORMAT(pstTask->stImgIn, pstTask->stImgOut);

	if (!hHandle) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "null hHandle");
		return CVI_ERR_DWA_NULL_PTR;
	}

	if (enRotation < ROTATION_0 || enRotation >= ROTATION_MAX) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "dwa(%d) param invalid\n", enRotation);
		return CVI_ERR_DWA_ILLEGAL_PARAM;
	}

	if (dwa_rotation_check_size(enRotation, pstTask) != CVI_SUCCESS) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "dwa_rotation_check_size fail\n");
		return CVI_ERR_DWA_ILLEGAL_PARAM;
	}

	struct dwa_task_attr attr;
	CVI_U64 paddr;
	CVI_VOID *vaddr;
	SIZE_S in_size, out_size;

	in_size.u32Width = pstTask->stImgIn.stVFrame.u32Width;
	in_size.u32Height = pstTask->stImgIn.stVFrame.u32Height;
	out_size.u32Width = pstTask->stImgOut.stVFrame.u32Width;
	out_size.u32Height = pstTask->stImgOut.stVFrame.u32Height;

	CVI_U8 idx = get_valid_tsk_mesh_by_name(pstTask->name);
	if (idx >= DWA_MAX_TSK_MESH) {
		if (CVI_SYS_IonAlloc_Cached(&paddr, &vaddr, pstTask->name, CVI_DWA_MESH_SIZE_ROT) != CVI_SUCCESS) {
			CVI_TRACE_DWA(CVI_DBG_ERR, "Can't acquire memory for mesh.\n");
			return CVI_ERR_GDC_NOBUF;
		}

		if (dwa_mesh_gen_rotation(in_size, out_size, enRotation, paddr, vaddr)) {
			CVI_TRACE_DWA(CVI_DBG_ERR, "dwa_mesh_gen_rotation failed\n");
			goto MESH_GEN_FAIL;
		}

		CVI_SYS_IonFlushCache(paddr, vaddr, CVI_DWA_MESH_SIZE_ROT);

		idx = get_idle_tsk_mesh();
		if (idx >= DWA_MAX_TSK_MESH) {
			CVI_TRACE_DWA(CVI_DBG_ERR, "tsk mesh count(%d) is out of range(%d)\n", idx + 1, DWA_MAX_TSK_MESH);
			CVI_SYS_IonFree(paddr, vaddr);
			return CVI_ERR_DWA_NOT_PERMITTED;
		}

		strcpy(dwa_tskMesh[idx].Name, pstTask->name);
		dwa_tskMesh[idx].paddr = paddr;
		dwa_tskMesh[idx].vaddr = vaddr;
	}

	memset(&attr, 0, sizeof(attr));
	attr.handle = hHandle;
	memcpy(&attr.stImgIn, &pstTask->stImgIn, sizeof(attr.stImgIn));
	memcpy(&attr.stImgOut, &pstTask->stImgOut, sizeof(attr.stImgOut));
	//memcpy(attr.au64privateData, pstTask->au64privateData, sizeof(attr.au64privateData));
	attr.reserved = pstTask->reserved;
	attr.enRotation = enRotation;
	attr.au64privateData[0] = dwa_tskMesh[idx].paddr;

	pstTask->au64privateData[0] = dwa_tskMesh[idx].paddr;
	pstTask->au64privateData[1] = (CVI_U64)((uintptr_t)dwa_tskMesh[idx].vaddr);
	return dwa_add_rotation_task(fd, &attr);

MESH_GEN_FAIL:
	if (paddr && vaddr)
		CVI_SYS_IonFree(paddr, vaddr);
	return CVI_FAILURE;
}

CVI_S32 CVI_DWA_AddAffineTask(GDC_HANDLE hHandle, GDC_TASK_ATTR_S *pstTask, const AFFINE_ATTR_S *pstAffineAttr)
{
	CVI_S32 fd = get_dwa_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_DWA, pstTask);
	MOD_CHECK_NULL_PTR(CVI_ID_DWA, pstAffineAttr);
	CHECK_DWA_FORMAT(pstTask->stImgIn, pstTask->stImgOut);

	if (!hHandle) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "null hHandle");
		return CVI_ERR_DWA_NULL_PTR;
	}

	if (pstAffineAttr->u32RegionNum == 0) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "u32RegionNum(%d) can't be zero.\n", pstAffineAttr->u32RegionNum);
		return CVI_ERR_DWA_ILLEGAL_PARAM;
	}

	if (pstAffineAttr->stDestSize.u32Width > pstTask->stImgOut.stVFrame.u32Width) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "dest's width(%d) can't be larger than frame's width(%d)\n",
			      pstAffineAttr->stDestSize.u32Width, pstTask->stImgOut.stVFrame.u32Width);
		return CVI_ERR_DWA_ILLEGAL_PARAM;
	}
	for (CVI_U32 i = 0; i < pstAffineAttr->u32RegionNum; ++i) {
		CVI_TRACE_DWA(CVI_DBG_INFO, "u32RegionNum(%d) (%f, %f) (%f, %f) (%f, %f) (%f, %f)\n", i,
			      pstAffineAttr->astRegionAttr[i][0].x, pstAffineAttr->astRegionAttr[i][0].y,
			      pstAffineAttr->astRegionAttr[i][1].x, pstAffineAttr->astRegionAttr[i][1].y,
			      pstAffineAttr->astRegionAttr[i][2].x, pstAffineAttr->astRegionAttr[i][2].y,
			      pstAffineAttr->astRegionAttr[i][3].x, pstAffineAttr->astRegionAttr[i][3].y);
		if ((pstAffineAttr->astRegionAttr[i][0].x < 0) || (pstAffineAttr->astRegionAttr[i][0].y < 0) ||
		    (pstAffineAttr->astRegionAttr[i][1].x < 0) || (pstAffineAttr->astRegionAttr[i][1].y < 0) ||
		    (pstAffineAttr->astRegionAttr[i][2].x < 0) || (pstAffineAttr->astRegionAttr[i][2].y < 0) ||
		    (pstAffineAttr->astRegionAttr[i][3].x < 0) || (pstAffineAttr->astRegionAttr[i][3].y < 0)) {
			CVI_TRACE_DWA(CVI_DBG_ERR, "u32RegionNum(%d) affine point can't be negative\n", i);
			return CVI_ERR_DWA_ILLEGAL_PARAM;
		}
		if ((pstAffineAttr->astRegionAttr[i][1].x < pstAffineAttr->astRegionAttr[i][0].x) ||
		    (pstAffineAttr->astRegionAttr[i][3].x < pstAffineAttr->astRegionAttr[i][2].x)) {
			CVI_TRACE_DWA(CVI_DBG_ERR, "u32RegionNum(%d) point1/3's x should be bigger thant 0/2's\n", i);
			return CVI_ERR_DWA_ILLEGAL_PARAM;
		}
		if ((pstAffineAttr->astRegionAttr[i][2].y < pstAffineAttr->astRegionAttr[i][0].y) ||
		    (pstAffineAttr->astRegionAttr[i][3].y < pstAffineAttr->astRegionAttr[i][1].y)) {
			CVI_TRACE_DWA(CVI_DBG_ERR, "u32RegionNum(%d) point2/3's y should be bigger thant 0/1's\n", i);
			return CVI_ERR_DWA_ILLEGAL_PARAM;
		}
	}

	struct dwa_task_attr attr;
	CVI_U64 paddr;
	CVI_VOID *vaddr;
	SIZE_S in_size, out_size;
	in_size.u32Width = pstTask->stImgIn.stVFrame.u32Width;
	in_size.u32Height = pstTask->stImgIn.stVFrame.u32Height;
	out_size.u32Width = pstTask->stImgOut.stVFrame.u32Width;
	out_size.u32Height = pstTask->stImgOut.stVFrame.u32Height;

	CVI_U8 idx = get_valid_tsk_mesh_by_name(pstTask->name);
	if (idx >= DWA_MAX_TSK_MESH) {
		if (CVI_SYS_IonAlloc_Cached(&paddr, &vaddr, pstTask->name, CVI_DWA_MESH_SIZE_AFFINE) != CVI_SUCCESS) {
			CVI_TRACE_DWA(CVI_DBG_ERR, "Can't acquire memory for mesh.\n");
			return CVI_ERR_GDC_NOBUF;
		}

		dwa_mesh_gen_affine(in_size, out_size, pstAffineAttr, paddr, vaddr);

		CVI_SYS_IonFlushCache(paddr, vaddr, CVI_DWA_MESH_SIZE_AFFINE);

		idx = get_idle_tsk_mesh();
		if (idx >= DWA_MAX_TSK_MESH) {
			CVI_TRACE_DWA(CVI_DBG_ERR, "tsk mesh count(%d) is out of range(%d)\n", idx + 1, DWA_MAX_TSK_MESH);
			CVI_SYS_IonFree(paddr, vaddr);
			return CVI_ERR_DWA_NOT_PERMITTED;
		}

		strcpy(dwa_tskMesh[idx].Name, pstTask->name);
		dwa_tskMesh[idx].paddr = paddr;
		dwa_tskMesh[idx].vaddr = vaddr;
	}

	memset(&attr, 0, sizeof(attr));
	attr.handle = hHandle;
	memcpy(&attr.stImgIn, &pstTask->stImgIn, sizeof(attr.stImgIn));
	memcpy(&attr.stImgOut, &pstTask->stImgOut, sizeof(attr.stImgOut));
	//memcpy(attr.au64privateData, pstTask->au64privateData, sizeof(attr.au64privateData));
	memcpy(&attr.stAffineAttr, pstAffineAttr, sizeof(*pstAffineAttr));
	attr.reserved = pstTask->reserved;
	attr.au64privateData[0] = dwa_tskMesh[idx].paddr;

	pstTask->au64privateData[0] = dwa_tskMesh[idx].paddr;
	pstTask->au64privateData[1] = (CVI_U64)((uintptr_t)dwa_tskMesh[idx].vaddr);
	return dwa_add_affine_task(fd, &attr);
}

CVI_S32 CVI_DWA_AddLDCTask(GDC_HANDLE hHandle, GDC_TASK_ATTR_S *pstTask
	, const LDC_ATTR_S *pstLDCAttr, ROTATION_E enRotation)
{
	UNUSED(enRotation);
	CVI_S32 fd = get_dwa_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_DWA, pstTask);
	MOD_CHECK_NULL_PTR(CVI_ID_DWA, pstLDCAttr);
	CHECK_DWA_FORMAT(pstTask->stImgIn, pstTask->stImgOut);

	if (!hHandle) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "null hHandle");
		return CVI_ERR_DWA_NULL_PTR;
	}

	if (pstLDCAttr->enRotation < ROTATION_0 || pstLDCAttr->enRotation >= ROTATION_MAX) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "dwa(%d) param invalid\n", pstLDCAttr->enRotation);
		return CVI_ERR_DWA_ILLEGAL_PARAM;
	}

	struct dwa_task_attr attr;
	SIZE_S in_size, out_size;
	CVI_U32 mesh_1st_size, mesh_2nd_size, mesh_size;
	CVI_U64 paddr;
	CVI_VOID *vaddr;

	in_size.u32Width = pstTask->stImgIn.stVFrame.u32Width;
	in_size.u32Height = pstTask->stImgIn.stVFrame.u32Height;
	out_size.u32Width = pstTask->stImgOut.stVFrame.u32Width;
	out_size.u32Height = pstTask->stImgOut.stVFrame.u32Height;

	CVI_U8 idx = get_valid_tsk_mesh_by_name(pstTask->name);
	if (idx >= DWA_MAX_TSK_MESH) {
		dwa_mesh_gen_get_size(in_size, out_size, &mesh_1st_size, &mesh_2nd_size);
		mesh_size = mesh_1st_size + mesh_2nd_size;

		// acquire memory space for mesh.
		if (CVI_SYS_IonAlloc_Cached(&paddr, &vaddr, pstTask->name, mesh_size) != CVI_SUCCESS) {
			CVI_TRACE_DWA(CVI_DBG_ERR, "Can't acquire memory for mesh.\n");
			return CVI_ERR_DWA_NOMEM;
		}

		if (dwa_mesh_gen_ldc(in_size, out_size, pstLDCAttr, paddr, vaddr)) {
			CVI_TRACE_DWA(CVI_DBG_ERR, "dwa_mesh_gen_ldc failed\n");
			goto MESH_GEN_FAIL;
		}

		CVI_SYS_IonFlushCache(paddr, vaddr, mesh_size);

		idx = get_idle_tsk_mesh();
		if (idx >= DWA_MAX_TSK_MESH) {
			CVI_TRACE_DWA(CVI_DBG_ERR, "tsk mesh count(%d) is out of range(%d)\n", idx + 1, DWA_MAX_TSK_MESH);
			CVI_SYS_IonFree(paddr, vaddr);
			return CVI_ERR_DWA_NOT_PERMITTED;
		}

		strcpy(dwa_tskMesh[idx].Name, pstTask->name);
		dwa_tskMesh[idx].paddr = paddr;
		dwa_tskMesh[idx].vaddr = vaddr;
	}

	memset(&attr, 0, sizeof(attr));
	attr.handle = hHandle;
	memcpy(&attr.stImgIn, &pstTask->stImgIn, sizeof(attr.stImgIn));
	memcpy(&attr.stImgOut, &pstTask->stImgOut, sizeof(attr.stImgOut));
	//memcpy(attr.au64privateData, pstTask->au64privateData, sizeof(attr.au64privateData));
	memcpy(&attr.stLdcAttr, pstLDCAttr, sizeof(*pstLDCAttr));
	attr.reserved = pstTask->reserved;
	attr.au64privateData[0] = dwa_tskMesh[idx].paddr;

	pstTask->au64privateData[0] = dwa_tskMesh[idx].paddr;
	pstTask->au64privateData[1] = (CVI_U64)((uintptr_t)dwa_tskMesh[idx].vaddr);
	return dwa_add_ldc_task(fd, &attr);

MESH_GEN_FAIL:
	if (paddr && vaddr)
		CVI_SYS_IonFree(paddr, vaddr);
	return CVI_FAILURE;
}

CVI_S32 CVI_DWA_GetWorkJob(GDC_HANDLE* phHandle)
{
	CVI_S32 fd = get_dwa_fd();
	struct dwa_handle_data cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_DWA, phHandle);

	memset(&cfg, 0, sizeof(cfg));
	if (dwa_get_work_job(fd, &cfg))
		return CVI_FAILURE;

	*phHandle = cfg.handle;
	return CVI_SUCCESS;
}

CVI_S32 CVI_DWA_GetChnFrame(GDC_IDENTITY_ATTR_S *identity, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret;
	struct dwa_chn_frm_cfg cfg;
	CVI_S32 fd = get_dwa_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_DWA, identity);
	MOD_CHECK_NULL_PTR(CVI_ID_DWA, pstFrameInfo);

	memset(&cfg, 0, sizeof(cfg));
	memcpy(&cfg.identity.attr, identity, sizeof(*identity));
	cfg.MilliSec = s32MilliSec;

	s32Ret = dwa_get_chn_frm(fd, &cfg);
	if (s32Ret) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "identity[%s-%d-%d] get chn frame fail, Ret[%d]\n"
			, identity->Name , identity->enModId, identity->u32ID, s32Ret);
		return s32Ret;
	}
	memcpy(pstFrameInfo, &cfg.VideoFrame, sizeof(*pstFrameInfo));

	return s32Ret;
}

CVI_S32 CVI_DWA_SetMeshSize(int nMeshHor, int nMeshVer)
{
	return dwa_set_mesh_size(nMeshHor, nMeshVer);
}

CVI_S32 CVI_DWA_GetDevFd(void)
{
	return get_dwa_fd();
}

CVI_S32 CVI_DWA_Init(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = get_dwa_fd();

	s32Ret = dwa_init(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "init fail\n");
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 CVI_DWA_DeInit(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = get_dwa_fd();

	s32Ret = dwa_deinit(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "deinit fail\n");
		return s32Ret;
	}

	s32Ret = dwa_dev_close();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "dwa_dev_close fail\n");
		return s32Ret;
	}

	free_all_tsk_mesh();
	free_all_meshdata();

	return s32Ret;
}


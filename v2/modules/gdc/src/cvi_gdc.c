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
#include <vi_uapi.h>
#include <vpss_uapi.h>

#include "cvi_buffer.h"
#include "cvi_base.h"
#include "cvi_sys.h"
#include "cvi_vb.h"

#include "cvi_gdc.h"
#include "gdc_mesh.h"
#include "ldc_ioctl.h"
#include "vi_ioctl.h"
#include "vpss_ioctl.h"
#include "gdc_ctx.h"


#define LDC_YUV_BLACK 0x808000
#define LDC_RGB_BLACK 0x0

#define CHECK_GDC_FORMAT(imgIn, imgOut)                                                                                \
	do {                                                                                                           \
		if (imgIn.stVFrame.enPixelFormat != imgOut.stVFrame.enPixelFormat) {                                   \
			CVI_TRACE_GDC(CVI_DBG_ERR, "in/out pixelformat(%d-%d) mismatch\n",                             \
				      imgIn.stVFrame.enPixelFormat, imgOut.stVFrame.enPixelFormat);                    \
			return CVI_ERR_GDC_ILLEGAL_PARAM;                                                              \
		}                                                                                                      \
		if (!GDC_SUPPORT_FMT(imgIn.stVFrame.enPixelFormat)) {                                                  \
			CVI_TRACE_GDC(CVI_DBG_ERR, "pixelformat(%d) unsupported\n", imgIn.stVFrame.enPixelFormat);     \
			return CVI_ERR_GDC_ILLEGAL_PARAM;                                                              \
		}                                                                                                      \
	} while (0)

static CVI_S32 ldc_fd = -1;
static pthread_mutex_t ldc_fd_lock = PTHREAD_MUTEX_INITIALIZER;

extern CVI_S32 get_vi_fd(CVI_VOID);
extern CVI_S32 get_vpss_fd(CVI_VOID);


CVI_S32 get_ldc_fd(CVI_VOID)
{
	pthread_mutex_lock(&ldc_fd_lock);
	if (ldc_fd <= 0) {
		if (open_device(LDC_DEV_NAME, &ldc_fd) == -1) {
			perror("GDC open fail\n");
			ldc_fd = -1;
		}
	}
	pthread_mutex_unlock(&ldc_fd_lock);

	return ldc_fd;
}


static CVI_S32 gdc_rotation_check_size(ROTATION_E enRotation, const GDC_TASK_ATTR_S *pstTask)
{
	if (enRotation >= ROTATION_MAX) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "invalid rotation(%d).\n", enRotation);
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}

	if (enRotation == ROTATION_90 || enRotation == ROTATION_270 || enRotation == ROTATION_XY_FLIP) {
		if (pstTask->stImgOut.stVFrame.u32Width < pstTask->stImgIn.stVFrame.u32Height) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "rotation(%d) invalid: 'output width(%d) < input height(%d)'\n",
				      enRotation, pstTask->stImgOut.stVFrame.u32Width,
				      pstTask->stImgIn.stVFrame.u32Height);
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
		if (pstTask->stImgOut.stVFrame.u32Height < pstTask->stImgIn.stVFrame.u32Width) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "rotation(%d) invalid: 'output height(%d) < input width(%d)'\n",
				      enRotation, pstTask->stImgOut.stVFrame.u32Height,
				      pstTask->stImgIn.stVFrame.u32Width);
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
	} else {
		if (pstTask->stImgOut.stVFrame.u32Width < pstTask->stImgIn.stVFrame.u32Width) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "rotation(%d) invalid: 'output width(%d) < input width(%d)'\n",
				      enRotation, pstTask->stImgOut.stVFrame.u32Width,
				      pstTask->stImgIn.stVFrame.u32Width);
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
		if (pstTask->stImgOut.stVFrame.u32Height < pstTask->stImgIn.stVFrame.u32Height) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "rotation(%d) invalid: 'output height(%d) < input height(%d)'\n",
				      enRotation, pstTask->stImgOut.stVFrame.u32Height,
				      pstTask->stImgIn.stVFrame.u32Height);
			return CVI_ERR_GDC_ILLEGAL_PARAM;
		}
	}

	return CVI_SUCCESS;
}

static CVI_S32 gdc_comm_cfg_frame(SIZE_S *stSize, PIXEL_FORMAT_E enPixelFormat, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VB_BLK blk;
	VB_CAL_CONFIG_S stVbCalConfig;

	if (pstVideoFrame == CVI_NULL) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "Null pointer!\n");
		return CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(stSize->u32Width, stSize->u32Height, enPixelFormat, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	memset(pstVideoFrame, 0, sizeof(*pstVideoFrame));
	pstVideoFrame->stVFrame.enCompressMode = COMPRESS_MODE_NONE;
	pstVideoFrame->stVFrame.enPixelFormat = enPixelFormat;
	pstVideoFrame->stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
	pstVideoFrame->stVFrame.enColorGamut = COLOR_GAMUT_BT601;
	pstVideoFrame->stVFrame.u32Width = stSize->u32Width;
	pstVideoFrame->stVFrame.u32Height = stSize->u32Height;
	pstVideoFrame->stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
	pstVideoFrame->stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
	pstVideoFrame->stVFrame.u32TimeRef = 0;
	pstVideoFrame->stVFrame.u64PTS = 0;
	pstVideoFrame->stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;

	blk = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCalConfig.u32VBSize);
	if (blk == VB_INVALID_HANDLE) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "Can't acquire vb block\n");
		return CVI_FAILURE;
	}

	pstVideoFrame->u32PoolId = CVI_VB_Handle2PoolId(blk);
	pstVideoFrame->stVFrame.u32Length[0] = stVbCalConfig.u32MainYSize;
	pstVideoFrame->stVFrame.u32Length[1] = stVbCalConfig.u32MainCSize;
	pstVideoFrame->stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
	pstVideoFrame->stVFrame.u64PhyAddr[1] = pstVideoFrame->stVFrame.u64PhyAddr[0]
		+ ALIGN(stVbCalConfig.u32MainYSize, stVbCalConfig.u16AddrAlign);
	if (stVbCalConfig.plane_num == 3) {
		pstVideoFrame->stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
		pstVideoFrame->stVFrame.u32Length[2] = stVbCalConfig.u32MainCSize;
		pstVideoFrame->stVFrame.u64PhyAddr[2] = pstVideoFrame->stVFrame.u64PhyAddr[1]
			+ ALIGN(stVbCalConfig.u32MainCSize, stVbCalConfig.u16AddrAlign);
	}

	return CVI_SUCCESS;
}

/**************************************************************************
 *   Public APIs.
 **************************************************************************/

CVI_S32 CVI_GDC_Suspend(void)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_ldc_fd();

	s32Ret = gdc_suspend(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "suspend fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_GDC_Resume(void)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_ldc_fd();

	s32Ret = gdc_resume(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "resume fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_GDC_Init(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = get_ldc_fd();

	s32Ret = gdc_init(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "init fail\n");
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 CVI_GDC_DeInit(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = get_ldc_fd();

	s32Ret = gdc_deinit(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "deinit fail\n");
		return s32Ret;
	}

	gdc_free_all_tsk_mesh();

	return s32Ret;
}

CVI_S32 CVI_GDC_BeginJob(GDC_HANDLE *phHandle)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, phHandle);

	CVI_S32 fd = get_ldc_fd();

	struct gdc_handle_data cfg;

	memset(&cfg, 0, sizeof(cfg));
	if (gdc_begin_job(fd, &cfg))
		return CVI_FAILURE;

	*phHandle = cfg.handle;

	return CVI_SUCCESS;
}

CVI_S32 CVI_GDC_SetJobIdentity(GDC_HANDLE hHandle, GDC_IDENTITY_ATTR_S *identity_attr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, identity_attr);

	if (!hHandle) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "null hHandle");
		return CVI_ERR_GDC_NULL_PTR;
	}

	CVI_S32 fd = get_ldc_fd();

	struct gdc_identity_attr cfg = {0};

	cfg.handle = hHandle;
	memcpy(&cfg.attr, identity_attr, sizeof(*identity_attr));

	return gdc_set_job_identity(fd, &cfg);
}

CVI_S32 CVI_GDC_EndJob(GDC_HANDLE hHandle)
{
	if (!hHandle) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "null hHandle");
		return CVI_ERR_DWA_NULL_PTR;
	}

	CVI_S32 fd = get_ldc_fd();

	struct gdc_handle_data cfg;

	memset(&cfg, 0, sizeof(cfg));
	cfg.handle = hHandle;
	return gdc_end_job(fd, &cfg);
}

CVI_S32 CVI_GDC_CancelJob(GDC_HANDLE hHandle)
{
	if (!hHandle) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "null hHandle");
		return CVI_ERR_DWA_NULL_PTR;
	}

	CVI_S32 fd = get_ldc_fd();

	struct gdc_handle_data cfg;

	memset(&cfg, 0, sizeof(cfg));
	cfg.handle = hHandle;
	return gdc_cancel_job(fd, &cfg);
}

CVI_S32 CVI_GDC_AddCorrectionTask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask,
				  const FISHEYE_ATTR_S *pstFishEyeAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pstTask);
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pstFishEyeAttr);
	CHECK_GDC_FORMAT(pstTask->stImgIn, pstTask->stImgOut);
	UNUSED(hHandle);

	CVI_TRACE_GDC(CVI_DBG_NOTICE, "not supported\n");
	return CVI_ERR_GDC_NOT_SUPPORT;
}

CVI_S32 CVI_GDC_AddRotationTask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask, ROTATION_E enRotation)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pstTask);
	CHECK_GDC_FORMAT(pstTask->stImgIn, pstTask->stImgOut);

	if (!hHandle) {
		CVI_TRACE_DWA(CVI_DBG_ERR, "null hHandle");
		return CVI_ERR_DWA_NULL_PTR;
	}

	if (enRotation == ROTATION_180) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "do not support rotation 180\n");
		return CVI_ERR_GDC_NOT_SUPPORT;
	}

	if (gdc_rotation_check_size(enRotation, pstTask) != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "gdc_rotation_check_size fail\n");
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}

	CVI_S32 fd = get_ldc_fd();

	struct gdc_task_attr attr;

	memset(&attr, 0, sizeof(attr));
	attr.handle = hHandle;
	memcpy(&attr.stImgIn, &pstTask->stImgIn, sizeof(attr.stImgIn));
	memcpy(&attr.stImgOut, &pstTask->stImgOut, sizeof(attr.stImgOut));
	//memcpy(attr.au64privateData, pstTask->au64privateData, sizeof(attr.au64privateData));
	//attr.reserved = pstTask->reserved;
	attr.enRotation = enRotation;
	return gdc_add_rotation_task(fd, &attr);
}

CVI_S32 CVI_GDC_AddAffineTask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask, const AFFINE_ATTR_S *pstAffineAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pstTask);
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pstAffineAttr);
	CHECK_GDC_FORMAT(pstTask->stImgIn, pstTask->stImgOut);
	UNUSED(hHandle);

	CVI_TRACE_GDC(CVI_DBG_NOTICE, "not supported\n");
	return CVI_ERR_GDC_NOT_SUPPORT;
}

CVI_S32 CVI_GDC_AddLDCTask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask
	, const LDC_ATTR_S *pstLDCAttr, ROTATION_E enRotation)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pstTask);
	CHECK_GDC_FORMAT(pstTask->stImgIn, pstTask->stImgOut);
	CVI_S32 s32Ret;
	ROTATION_E rot[2];
	UNUSED(enRotation);
	if (pstLDCAttr->enRotation < ROTATION_0 || pstLDCAttr->enRotation >= ROTATION_MAX) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "ldc(%d) param invalid\n", pstLDCAttr->enRotation);
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}

	if (pstLDCAttr->enRotation == 1) {
		rot[0] = ROTATION_90;
		rot[1] = ROTATION_0;
	} else if (pstLDCAttr->enRotation == 2) {
		rot[0] = ROTATION_90;
		rot[1] = ROTATION_90;
	} else if (pstLDCAttr->enRotation == 3) {
		rot[0] = ROTATION_270;
		rot[1] = ROTATION_0;
	} else {
		rot[0] = ROTATION_90;
		rot[1] = ROTATION_270;
	}

	if (!hHandle) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "null hHandle");
		return CVI_ERR_GDC_NULL_PTR;
	}

	if (!pstLDCAttr) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "null pstLDCAttr");
		return CVI_ERR_GDC_NULL_PTR;
	}

	CVI_S32 fd = get_ldc_fd();

	if (pstLDCAttr->stGridInfoAttr.Enable) {
		rot[0] = ROTATION_270;
		rot[1] = ROTATION_90;
	}

	struct gdc_task_attr attr;
	SIZE_S stSizeTmp;
	PIXEL_FORMAT_E enPixelFormatTmp = pstTask->stImgIn.stVFrame.enPixelFormat;
	VIDEO_FRAME_INFO_S stVideoFrameTmp;
	CVI_U32 mesh_1st_size;
	VB_BLK blkTmp;

	stSizeTmp.u32Width = ALIGN(pstTask->stImgIn.stVFrame.u32Height, DEFAULT_ALIGN);
	stSizeTmp.u32Height = ALIGN(pstTask->stImgIn.stVFrame.u32Width, DEFAULT_ALIGN);

	s32Ret = gdc_comm_cfg_frame(&stSizeTmp, enPixelFormatTmp, &stVideoFrameTmp);
	if (s32Ret) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "gdc_comm_cfg_frame fail\n");
		return CVI_ERR_GDC_NOBUF;
	}

	memset(&attr, 0, sizeof(attr));
	attr.handle = hHandle;
	memcpy(&attr.stImgIn, &pstTask->stImgIn, sizeof(attr.stImgIn));
	memcpy(&attr.stImgOut, &stVideoFrameTmp, sizeof(attr.stImgOut));
	attr.au64privateData[0] = pstTask->au64privateData[0];
	attr.reserved = pstTask->reserved;
	attr.enRotation = rot[0];
	s32Ret = gdc_add_ldc_task(fd, &attr);
	if (s32Ret) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "gdc_add_ldc_task 1st fail\n");
		goto FREE_TMP_FRAME;
	}

	mesh_gen_get_1st_size(stSizeTmp, &mesh_1st_size);
	memcpy(&attr.stImgIn, &stVideoFrameTmp, sizeof(attr.stImgIn));
	memcpy(&attr.stImgOut, &pstTask->stImgOut, sizeof(attr.stImgOut));
	attr.au64privateData[0] = pstTask->au64privateData[0] + mesh_1st_size;
	attr.enRotation = rot[1];

	s32Ret = gdc_add_ldc_task(fd, &attr);
	if (s32Ret) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "gdc_add_ldc_task 2nd fail\n");
		goto FREE_TMP_FRAME;
	}

FREE_TMP_FRAME:
	blkTmp = CVI_VB_PhysAddr2Handle(stVideoFrameTmp.stVFrame.u64PhyAddr[0]);
	if (blkTmp != VB_INVALID_HANDLE)
		CVI_VB_ReleaseBlock(blkTmp);

	return s32Ret;
}

CVI_S32 CVI_GDC_AddCorrectionTaskCNV(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask
	, const FISHEYE_ATTR_S *pstFishEyeAttr, uint8_t *p_tbl, uint8_t *p_idl, uint32_t *tbl_param)
{
	CHECK_GDC_FORMAT(pstTask->stImgIn, pstTask->stImgOut);
	UNUSED(hHandle);
	UNUSED(pstFishEyeAttr);
	UNUSED(p_tbl);
	UNUSED(p_idl);
	UNUSED(tbl_param);

	CVI_TRACE_GDC(CVI_DBG_NOTICE, "not supported\n");
	return CVI_ERR_GDC_NOT_SUPPORT;
}

CVI_S32 CVI_GDC_AddCnvWarpTask(const float *pfmesh_data, GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask,
	const FISHEYE_ATTR_S *pstAffineAttr, bool *bReNew)
{
	CHECK_GDC_FORMAT(pstTask->stImgIn, pstTask->stImgOut);
	UNUSED(pfmesh_data);
	UNUSED(hHandle);
	UNUSED(pstAffineAttr);
	UNUSED(bReNew);

	CVI_TRACE_GDC(CVI_DBG_NOTICE, "not supported\n");
	return CVI_ERR_GDC_NOT_SUPPORT;
}

CVI_S32 CVI_GDC_SetBufWrapAttr(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask, const LDC_BUF_WRAP_S *pstBufWrap)
{
	struct ldc_buf_wrap_cfg *cfg;
	CVI_S32 s32Ret;

	CVI_S32 fd = get_ldc_fd();
	cfg = malloc(sizeof(*cfg));
	if (!cfg) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "gdc malloc fails.\n");
		return CVI_FAILURE;
	}

	memset(cfg, 0, sizeof(*cfg));
	cfg->handle = hHandle;
	memcpy(&cfg->stTask.stImgIn, &pstTask->stImgIn, sizeof(cfg->stTask.stImgIn));
	memcpy(&cfg->stTask.stImgOut, &pstTask->stImgOut, sizeof(cfg->stTask.stImgOut));
	memcpy(&cfg->stBufWrap, pstBufWrap, sizeof(cfg->stBufWrap));

	s32Ret = gdc_set_chn_buf_wrap(fd, cfg);

	free(cfg);

	return s32Ret;
}

CVI_S32 CVI_GDC_GetBufWrapAttr(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask, LDC_BUF_WRAP_S *pstBufWrap)
{
	struct ldc_buf_wrap_cfg *cfg;
	CVI_S32 s32Ret;

	CVI_S32 fd = get_ldc_fd();

	cfg = malloc(sizeof(*cfg));
	if (!cfg) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "gdc malloc fails.\n");
		return CVI_FAILURE;
	}

	memset(cfg, 0, sizeof(*cfg));
	cfg->handle = hHandle;
	memcpy(&cfg->stTask, pstTask, sizeof(cfg->stTask));

	s32Ret = gdc_get_chn_buf_wrap(fd, cfg);

	free(cfg);

	if (s32Ret == CVI_SUCCESS)
		memcpy(pstBufWrap, &cfg->stBufWrap, sizeof(*pstBufWrap));

	return s32Ret;
}

CVI_S32 CVI_GDC_DumpMesh(MESH_DUMP_ATTR_S *pMeshDumpAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pMeshDumpAttr);

	CVI_U64 phyMesh;
	CVI_VOID *virMesh;
	CVI_U32 u32Width, u32Height, vpssGrp, vpssChn, viChn;
	SIZE_S in_size, out_size;
	CVI_U32 mesh_1st_size, mesh_2nd_size, meshSize;
	CVI_S32 s32Ret;
	CVI_S32 fd;
	struct vpss_chn_attr attr;
	VI_CHN_ATTR_S stChnAttr;

	FILE *fp;
	MOD_ID_E mod = pMeshDumpAttr->enModId;
	CVI_CHAR *filePath = pMeshDumpAttr->binFileName;

	switch (mod) {
	case CVI_ID_VI:
		fd = get_vi_fd();
		viChn = pMeshDumpAttr->viMeshAttr.chn;
		phyMesh = g_vi_mesh[viChn].paddr;
		virMesh = g_vi_mesh[viChn].vaddr;
		s32Ret = vi_sdk_get_chn_attr(fd, 0, viChn, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_attr ioctl failed. errno 0x%x\n", s32Ret);
			return s32Ret;
		}
		u32Width = stChnAttr.stSize.u32Width;
		u32Height = stChnAttr.stSize.u32Height;
		in_size.u32Width = ALIGN(u32Width, DEFAULT_ALIGN);
		in_size.u32Height = ALIGN(u32Height, DEFAULT_ALIGN);
		out_size.u32Width = in_size.u32Width;
		out_size.u32Height = in_size.u32Height;
		mesh_gen_get_size(in_size, out_size, &mesh_1st_size, &mesh_2nd_size);
		meshSize = mesh_1st_size + mesh_2nd_size;
		break;
	case CVI_ID_VPSS:
		fd = get_vpss_fd();
		attr.VpssGrp = vpssGrp = pMeshDumpAttr->vpssMeshAttr.grp;
		attr.VpssChn = vpssChn = pMeshDumpAttr->vpssMeshAttr.chn;
		phyMesh = mesh[vpssGrp][vpssChn].paddr;
		virMesh = mesh[vpssGrp][vpssChn].vaddr;

		s32Ret = vpss_get_chn_attr(fd, &attr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn attr fail\n", vpssGrp, vpssChn);
			return s32Ret;
		}
		u32Width = attr.stChnAttr.u32Width;
		u32Height = attr.stChnAttr.u32Height;
		in_size.u32Width = ALIGN(u32Width, DEFAULT_ALIGN);
		in_size.u32Height = ALIGN(u32Height, DEFAULT_ALIGN);
		out_size.u32Width = in_size.u32Width;
		out_size.u32Height = in_size.u32Height;
		mesh_gen_get_size(in_size, out_size, &mesh_1st_size, &mesh_2nd_size);
		meshSize = mesh_1st_size + mesh_2nd_size;
		break;
	default:
		CVI_TRACE_GDC(CVI_DBG_ERR, "not supported\n");
		return CVI_ERR_GDC_NOT_SUPPORT;
	}

	CVI_TRACE_GDC(CVI_DBG_DEBUG, "dump mesh size:%d, mesh phy addr:%#"PRIx64", vir addr:%p.\n",
		meshSize, phyMesh, virMesh);

	fp = fopen(filePath, "wb");
	if (!fp) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "open file:%s failed.\n", filePath);
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}
	fwrite(virMesh, meshSize, 1, fp);
	fflush(fp);
	fclose(fp);
	return CVI_SUCCESS;
}

CVI_S32 CVI_GDC_LoadMesh(MESH_DUMP_ATTR_S *pMeshDumpAttr, const LDC_ATTR_S *pstLDCAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pMeshDumpAttr);

	CVI_U64 phyMesh;
	CVI_VOID *virMesh;
	CVI_U32 vpssGrp = 0, vpssChn = 0, viChn = 0;
	SIZE_S in_size, out_size;
	CVI_U32 mesh_1st_size, mesh_2nd_size, mesh_size;
	CVI_U32 u32Width, u32Height;
	struct cvi_gdc_mesh *pmesh;
	FILE *fp;
	CVI_S32 fd;
	MOD_ID_E mod = pMeshDumpAttr->enModId;
	CVI_CHAR *filePath = pMeshDumpAttr->binFileName;
	CVI_S32 s32Ret;
	struct vpss_chn_attr attr;
	VI_CHN_ATTR_S stChnAttr;
	CVI_CHAR mesh_name[128];

	switch (mod) {
	case CVI_ID_VI:
		fd = get_vi_fd();
		viChn = pMeshDumpAttr->viMeshAttr.chn;
		pmesh = &g_vi_mesh[viChn];
		s32Ret = vi_sdk_get_chn_attr(fd, 0, viChn, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_attr ioctl failed. errno 0x%x\n", s32Ret);
			return s32Ret;
		}

		u32Width = stChnAttr.stSize.u32Width;
		u32Height = stChnAttr.stSize.u32Height;
		in_size.u32Width = ALIGN(u32Width, DEFAULT_ALIGN);
		in_size.u32Height = ALIGN(u32Height, DEFAULT_ALIGN);
		snprintf(mesh_name, 128, "vi_%d", viChn);
		break;
	case CVI_ID_VPSS:
		fd = get_vpss_fd();
		attr.VpssGrp = vpssGrp = pMeshDumpAttr->vpssMeshAttr.grp;
		attr.VpssChn = vpssChn = pMeshDumpAttr->vpssMeshAttr.chn;
		pmesh = &mesh[vpssGrp][vpssChn];

		s32Ret = vpss_get_chn_attr(fd, &attr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn attr fail\n", vpssGrp, vpssChn);
			return s32Ret;
		}
		u32Width = attr.stChnAttr.u32Width;
		u32Height = attr.stChnAttr.u32Height;
		in_size.u32Width = ALIGN(u32Width, DEFAULT_ALIGN);
		in_size.u32Height = ALIGN(u32Height, DEFAULT_ALIGN);
		snprintf(mesh_name, 128, "vpss_%d_%d", vpssGrp, vpssChn);
		break;
	default:
		CVI_TRACE_GDC(CVI_DBG_ERR, "not supported\n");
		return CVI_ERR_GDC_NOT_SUPPORT;
	}

	out_size.u32Width = in_size.u32Width;
	out_size.u32Height = in_size.u32Height;

	mesh_gen_get_size(in_size, out_size, &mesh_1st_size, &mesh_2nd_size);
	mesh_size = mesh_1st_size + mesh_2nd_size;

	fp = fopen(filePath, "rb");
	if (!fp) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "open file:%s failed.\n", filePath);
		return CVI_ERR_GDC_ILLEGAL_PARAM;
	}
	fseek(fp, 0, SEEK_END);
	int fileSize = ftell(fp);

	if (mesh_size != (CVI_U32)fileSize) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "loadmesh file:(%s) size is not match.\n", filePath);
		fclose(fp);
		return CVI_FAILURE;
	}
	rewind(fp);

	// acquire memory space for mesh.
	if (CVI_SYS_IonAlloc_Cached(&phyMesh, &virMesh, mesh_name, mesh_size) != CVI_SUCCESS) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "Can't acquire memory for gdc mesh.\n");
		fclose(fp);
		return CVI_ERR_GDC_NOMEM;
	}

	CVI_TRACE_GDC(CVI_DBG_DEBUG, "load mesh size:%d, mesh phy addr:%#"PRIx64", vir addr:%p.\n",
		mesh_size, phyMesh, virMesh);
	pmesh->paddr = phyMesh;
	pmesh->vaddr = virMesh;

	fread(virMesh, mesh_size, 1, fp);
	CVI_SYS_IonFlushCache(phyMesh, virMesh, mesh_size);

	if (gdc_set_tsk_mesh_by_name(mesh_name, phyMesh, virMesh)) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "gdc_set_tsk_mesh_by_name fail.\n");
		fclose(fp);
		return CVI_ERR_GDC_NOMEM;
	}

	switch (mod) {
	case CVI_ID_VI:
//		g_vi_mesh[viChn].meshSize = mesh_size;
		//vi_ctx.stLDCAttr[viChn].bEnable = CVI_TRUE;

		fd = get_vi_fd();
		UNUSED(viChn);

		struct vi_chn_ldc_cfg vi_cfg;

		vi_cfg.ViChn = viChn;
		// vi_cfg.enRotation = ROTATION_0;
		//vi_cfg.stLDCAttr = *pstLDCAttr;
		vi_cfg.stLDCAttr.bEnable = CVI_TRUE;
		memcpy(&vi_cfg.stLDCAttr.stAttr, pstLDCAttr, sizeof(*pstLDCAttr));
		vi_cfg.meshHandle = pmesh->paddr;
		if (vi_sdk_set_chn_ldc(fd, &vi_cfg) != CVI_SUCCESS) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "VI Set Chn(%d) LDC fail\n", viChn);
			fclose(fp);
			return CVI_FAILURE;
		}

		break;
	case CVI_ID_VPSS:
		mesh[vpssGrp][vpssChn].meshSize = mesh_size;
		//vpssCtx[vpssGrp].stChnCfgs[vpssChn].stLDCAttr.bEnable = CVI_TRUE;
		fd = get_vpss_fd();
		struct vpss_chn_ldc_cfg vpss_cfg;

		vpss_cfg.VpssGrp = vpssGrp;
		vpss_cfg.VpssChn = vpssChn;
		// vpss_cfg.enRotation = ROTATION_0;
		//vpss_cfg.stLDCAttr = *pstLDCAttr;
		vpss_cfg.stLDCAttr.bEnable = CVI_TRUE;
		memcpy(&vpss_cfg.stLDCAttr.stAttr, pstLDCAttr, sizeof(*pstLDCAttr));
		vpss_cfg.meshHandle = pmesh->paddr;
		if (vpss_set_chn_ldc(fd, &vpss_cfg) != CVI_SUCCESS) {
			CVI_TRACE_GDC(CVI_DBG_ERR, "VPSS Set Chn(%d) LDC fail\n", vpssChn);
			fclose(fp);
			return CVI_FAILURE;
		}
		break;
	default:
		CVI_TRACE_GDC(CVI_DBG_ERR, "not supported\n");
		fclose(fp);
		return CVI_ERR_GDC_NOT_SUPPORT;
	}
	fclose(fp);
	return CVI_SUCCESS;
}

CVI_S32 CVI_GDC_GetWorkJob(GDC_HANDLE* phHandle)
{
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, phHandle);
	CVI_S32 fd = get_ldc_fd();

	struct gdc_handle_data cfg;

	memset(&cfg, 0, sizeof(cfg));
	if (gdc_get_work_job(fd, &cfg))
		return CVI_FAILURE;

	*phHandle = cfg.handle;
	return CVI_SUCCESS;
}

CVI_S32 CVI_GDC_GetChnFrame(GDC_IDENTITY_ATTR_S *identity, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_GDC, identity);
	MOD_CHECK_NULL_PTR(CVI_ID_GDC, pstFrameInfo);

	CVI_S32 fd = get_ldc_fd();

	struct gdc_chn_frm_cfg cfg;

	memset(&cfg, 0, sizeof(cfg));
	memcpy(&cfg.identity.attr, identity, sizeof(*identity));
	cfg.MilliSec = s32MilliSec;

	s32Ret = gdc_get_chn_frm(fd, &cfg);
	if (s32Ret) {
		CVI_TRACE_GDC(CVI_DBG_ERR, "identity[%s-%d-%d] get chn frame fail, Ret[%d]\n"
			, identity->Name , identity->enModId, identity->u32ID, s32Ret);
		return s32Ret;
	}
	memcpy(pstFrameInfo, &cfg.VideoFrame, sizeof(*pstFrameInfo));

	return s32Ret;
}

CVI_S32 CVI_GDC_SetMeshSize(int nMeshHor, int nMeshVer)
{
	UNUSED(nMeshHor);
	UNUSED(nMeshVer);

	CVI_TRACE_GDC(CVI_DBG_NOTICE, "not supported\n");
	return CVI_ERR_GDC_NOT_SUPPORT;
}

CVI_S32 CVI_GDC_GetDevFd(void)
{
	return get_ldc_fd();
}


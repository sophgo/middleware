#include "cvi_dpu.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/param.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <math.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/mman.h>
#include "cvi_debug.h"
#include "../include/dpu_ioctl.h"
#include "cvi_base.h"
#include "dwa_mesh.h"

#define CHECK_DPU_FMT(grp, chn, fmt)									\													\
		if (!DPU_SUPPORT_FMT(fmt)) {		\
			CVI_TRACE_DPU(CVI_DBG_ERR, "Grp(%d) Chn(%d) invalid PixFormat(%d) for DPU.\n"		\
				      , grp, chn, (fmt));							\
			return CVI_ERR_DPU_ILLEGAL_PARAM;							\
		}

static CVI_S32 dpu_fd = -1;
static pthread_mutex_t dpu_fd_lock = PTHREAD_MUTEX_INITIALIZER;

static inline CVI_S32 CHECK_DPU_GRP_VALID(DPU_GRP grp)
{
	if ((grp >= DPU_MAX_GRP_NUM) || (grp < 0)) {
		CVI_TRACE_DPU(CVI_DBG_ERR, "DpuGrp(%d) exceeds Max(%d)\n", grp, DPU_MAX_GRP_NUM);
		return CVI_ERR_DPU_ILLEGAL_PARAM;
	}
	return CVI_SUCCESS;
}

static inline CVI_S32 CHECK_DPU_CHN_VALID(DPU_CHN DpuChn)
{
	if ((DpuChn >= DPU_MAX_CHN_NUM) || (DpuChn < 0)) {
		CVI_TRACE_DPU(CVI_DBG_ERR, "Chn(%d) invalid.\n", DpuChn);
		return CVI_ERR_DPU_ILLEGAL_PARAM;
	}
	return CVI_SUCCESS;
}


static CVI_S32 get_dpu_fd(CVI_VOID)
{
	pthread_mutex_lock(&dpu_fd_lock);
	if (dpu_fd <= 0) {
		if (open_device(DPU_DEV_NAME, &dpu_fd) == -1) {
			perror("dpu open fail\n");
			dpu_fd = -1;
		}
	}
	pthread_mutex_unlock(&dpu_fd_lock);

	return dpu_fd;
}

// CVI_S32 CVI_DPU_GetAssistBufSize(CVI_U16 u16_disp_num,CVI_U32 u32_dst_height,CVI_U32 * pu32_size)
// {
//     return 0;
// }

CVI_S32 CVI_DPU_CreateGrp(DPU_GRP DpuGrp, const DPU_GRP_ATTR_S *pstGrpAttr)
{
    CVI_S32 fd = get_dpu_fd();
	CVI_S32 s32Ret;
	struct dpu_grp_attr grp_att;

	MOD_CHECK_NULL_PTR(CVI_ID_DPU, pstGrpAttr);

	s32Ret = CHECK_DPU_GRP_VALID(DpuGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	grp_att.DpuGrp = DpuGrp;
	memcpy(&grp_att.stGrpAttr, pstGrpAttr, sizeof(grp_att.stGrpAttr));

	s32Ret = dpu_create_grp(fd, &grp_att);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_DPU(CVI_DBG_ERR, "Grp(%d) create group fail\n", DpuGrp);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_DPU_DestroyGrp(DPU_GRP DpuGrp)
{
    CVI_S32 fd = get_dpu_fd();
	CVI_S32 s32Ret;
	struct dpu_grp_cfg grp_cfg;

	s32Ret = CHECK_DPU_GRP_VALID(DpuGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	grp_cfg.DpuGrp = DpuGrp;

	s32Ret = dpu_destroy_grp(fd, &grp_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_DPU(CVI_DBG_ERR, "Grp(%d) destroy group fail\n", DpuGrp);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_DPU_SetGrpAttr(DPU_GRP DpuGrp,const DPU_GRP_ATTR_S *pstGrpAttr)
{
    CVI_S32 fd = get_dpu_fd();
	CVI_S32 s32Ret;
	struct dpu_grp_attr grp_att;

	MOD_CHECK_NULL_PTR(CVI_ID_DPU, pstGrpAttr);

	s32Ret = CHECK_DPU_GRP_VALID(DpuGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	grp_att.DpuGrp = DpuGrp;
	memcpy(&grp_att.stGrpAttr, pstGrpAttr, sizeof(grp_att.stGrpAttr));

	s32Ret = dpu_set_grp_attr(fd, &grp_att);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_DPU(CVI_DBG_ERR, "Grp(%d) set group attr fail\n", DpuGrp);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_DPU_GetGrpAttr(DPU_GRP DpuGrp,DPU_GRP_ATTR_S *pstGrpAttr)
{
    CVI_S32 fd = get_dpu_fd();
	CVI_S32 s32Ret;
	struct dpu_grp_attr grp_att;

	s32Ret = CHECK_DPU_GRP_VALID(DpuGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	grp_att.DpuGrp = DpuGrp;

	s32Ret = dpu_get_grp_attr(fd, &grp_att);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_DPU(CVI_DBG_ERR, "Grp(%d) get group  attr fail\n", DpuGrp);
		return s32Ret;
	}

	memcpy(pstGrpAttr, &grp_att.stGrpAttr,sizeof(*pstGrpAttr));

	return CVI_SUCCESS;
}

CVI_S32 CVI_DPU_StartGrp(DPU_GRP DpuGrp)
{
    CVI_S32 fd = get_dpu_fd();
	CVI_S32 s32Ret;
	struct dpu_grp_cfg grp_cfg;

	s32Ret = CHECK_DPU_GRP_VALID(DpuGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	grp_cfg.DpuGrp = DpuGrp;

	s32Ret = dpu_start_grp(fd, &grp_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_DPU(CVI_DBG_ERR, "Grp(%d) start group fail\n", DpuGrp);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_DPU_StopGrp(DPU_GRP DpuGrp)
{
    CVI_S32 fd = get_dpu_fd();
	CVI_S32 s32Ret;
	struct dpu_grp_cfg grp_cfg;

	s32Ret = CHECK_DPU_GRP_VALID(DpuGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	grp_cfg.DpuGrp = DpuGrp;

	s32Ret = dpu_stop_grp(fd, &grp_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_DPU(CVI_DBG_ERR, "Grp(%d) stop group fail\n", DpuGrp);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_DPU_SetChnAttr(DPU_GRP DpuGrp,DPU_CHN  DpuChn,const DPU_CHN_ATTR_S *pstChnAttr)
{
    CVI_S32 fd = get_dpu_fd();
	CVI_S32 s32Ret;
	struct dpu_chn_attr chn_att;

	MOD_CHECK_NULL_PTR(CVI_ID_DPU,pstChnAttr);

	s32Ret = CHECK_DPU_GRP_VALID(DpuGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_DPU_CHN_VALID(DpuChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	chn_att.DpuGrp = DpuGrp;
	chn_att.DpuChn = DpuChn;
	memcpy(&chn_att.stChnAttr, pstChnAttr, sizeof(chn_att.stChnAttr));

	s32Ret = dpu_set_chn_attr(fd, &chn_att);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_DPU(CVI_DBG_ERR, "Grp(%d) Chn(%d) set chn attr fail\n", DpuGrp,DpuChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_DPU_GetChnAttr(DPU_GRP DpuGrp,DPU_CHN DpuChn,DPU_CHN_ATTR_S *pstChnAttr)
{
    CVI_S32 fd = get_dpu_fd();
	CVI_S32 s32Ret;
	struct dpu_chn_attr chn_att;

	MOD_CHECK_NULL_PTR(CVI_ID_DPU,pstChnAttr);

	s32Ret = CHECK_DPU_GRP_VALID(DpuGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_DPU_CHN_VALID(DpuChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	chn_att.DpuGrp = DpuGrp;
	chn_att.DpuChn = DpuChn;

	s32Ret = dpu_set_chn_attr(fd, &chn_att);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_DPU(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn attr fail\n", DpuGrp,DpuChn);
		return s32Ret;
	}

	memcpy(pstChnAttr, &chn_att.stChnAttr, sizeof(*pstChnAttr));
	return CVI_SUCCESS;
}

CVI_S32 CVI_DPU_EnableChn(DPU_GRP DpuGrp,DPU_CHN DpuChn)
{
    CVI_S32 fd = get_dpu_fd();
	CVI_S32 s32Ret;
	struct dpu_chn_cfg chn_cfg;

	s32Ret = CHECK_DPU_GRP_VALID(DpuGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_DPU_CHN_VALID(DpuChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	chn_cfg.DpuGrp = DpuGrp;
	chn_cfg.DpuChn = DpuChn;

	s32Ret = dpu_enable_chn(fd, &chn_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_DPU(CVI_DBG_ERR, "Grp(%d) Chn(%d) set enable chn fail\n", DpuGrp,DpuChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_DPU_DisableChn(DPU_GRP DpuGrp,DPU_CHN DpuChn)
{
    CVI_S32 fd = get_dpu_fd();
	CVI_S32 s32Ret;
	struct dpu_chn_cfg chn_cfg;

	s32Ret = CHECK_DPU_GRP_VALID(DpuGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_DPU_CHN_VALID(DpuChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	chn_cfg.DpuGrp = DpuGrp;
	chn_cfg.DpuChn = DpuChn;

	s32Ret = dpu_disable_chn(fd, &chn_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_DPU(CVI_DBG_ERR, "Grp(%d) Chn(%d) disable chn fail\n", DpuGrp,DpuChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_DPU_SendFrame(DPU_GRP DpuGrp,\
                                const VIDEO_FRAME_INFO_S *pst_left_frame,\
                                const VIDEO_FRAME_INFO_S *pst_right_frame,\
                                CVI_S32 s32Millisec)
{
    CVI_S32 fd = get_dpu_fd();
	CVI_S32 s32Ret;
	struct dpu_set_frame_cfg frame_cfg;

	s32Ret = CHECK_DPU_GRP_VALID(DpuGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	frame_cfg.DpuGrp = DpuGrp;
	frame_cfg.s32MilliSec=s32Millisec;

	memcpy(&frame_cfg.stSrcLeftFrame, pst_left_frame, sizeof(frame_cfg.stSrcLeftFrame));
	memcpy(&frame_cfg.stSrcRightFrame, pst_right_frame, sizeof(frame_cfg.stSrcRightFrame));
	s32Ret = dpu_send_frame(fd, &frame_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_DPU(CVI_DBG_ERR, "Grp(%d)  send frame fail\n", DpuGrp);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_DPU_GetFrame(DPU_GRP DpuGrp,\
							DPU_CHN DpuChn,\
							VIDEO_FRAME_INFO_S *pstFrameInfo,\
							CVI_S32 s32Millisec)
{
    CVI_S32 fd = get_dpu_fd();
	CVI_S32 s32Ret;
	struct dpu_get_frame_cfg frame_cfg;

	s32Ret = CHECK_DPU_GRP_VALID(DpuGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CHECK_DPU_CHN_VALID(DpuChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	frame_cfg.DpuGrp = DpuGrp;
	frame_cfg.DpuChn = DpuChn;
	frame_cfg.s32MilliSec=s32Millisec;

	s32Ret = dpu_get_frame(fd, &frame_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_DPU(CVI_DBG_ERR, "Grp(%d)  get frame fail\n", DpuGrp);
		return s32Ret;
	}
	memcpy(pstFrameInfo, &frame_cfg.stFrameInfo,sizeof(*pstFrameInfo));

	return CVI_SUCCESS;
}

CVI_S32 CVI_DPU_ReleaseFrame(DPU_GRP DpuGrp,\
							DPU_CHN DpuChn,\
                            const VIDEO_FRAME_INFO_S *pstVideoFrame)
{
    CVI_S32 fd = get_dpu_fd();
	CVI_S32 s32Ret;
	struct dpu_release_frame_cfg frame_cfg;

	s32Ret = CHECK_DPU_GRP_VALID(DpuGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CHECK_DPU_CHN_VALID(DpuChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	frame_cfg.DpuGrp = DpuGrp;
	frame_cfg.DpuChn = DpuChn;

	memcpy(&frame_cfg.stFrameInfo,pstVideoFrame, sizeof(frame_cfg.stFrameInfo));

	s32Ret = dpu_release_frame(fd, &frame_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_DPU(CVI_DBG_ERR, "Grp(%d) Chn(%d)release frame fail\n", DpuGrp,DpuChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

void CVI_DPU_CheckRegWrite(void)
{
	CVI_S32 fd = get_dpu_fd();
	dpu_check_reg_write(fd);
}

void CVI_DPU_CheckRegRead(void)
{
	CVI_S32 fd = get_dpu_fd();
	dpu_check_reg_read(fd);
}

void CVI_DPU_CheckSgbmStatus(void)
{
	CVI_S32 fd = get_dpu_fd();
	dpu_get_sgbm_status(fd);
}

void CVI_DPU_CheckFgsStatus(void)
{
	CVI_S32 fd = get_dpu_fd();
	dpu_get_fgs_status(fd);
}

void CVI_DPU_Reset(void)
{
	CVI_S32 fd = get_dpu_fd();
	dpu_reset(fd);
}

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

#include "cvi_errno.h"

#include "cvi_buffer.h"
#include "cvi_base.h"
#include "cvi_sys.h"
#include "cvi_stitch.h"
#include "stitch_ioctl.h"


static CVI_S32 stitch_fd = -1;
static pthread_mutex_t stitch_fd_lock = PTHREAD_MUTEX_INITIALIZER;

#define CHECK_STITCH_SRC_ID_VALID(idx)                                                              \
	do {                                                                                             \
		if (idx >= STITCH_MAX_SRC_NUM || idx < 0) {                                                   \
			CVI_TRACE_STITCH(CVI_DBG_ERR, "src_id(%d) exceeds Max(%d)\n", idx, STITCH_MAX_SRC_NUM - 1);   \
			return CVI_ERR_STITCH_ILLEGAL_PARAM;                                                          \
		}                                                                                                 \
	} while (0)

#define CHECK_GRP_ID_VALID(grp)                                                                                   \
	do {                                                                                                          \
		if (grp >= STITCH_MAX_GRP_NUM || grp < 0) {                                                              \
			CVI_TRACE_STITCH(CVI_DBG_ERR, "grp(%d) invalid, out of range[0-%d]\n", grp, STITCH_MAX_GRP_NUM - 1);  \
			return CVI_ERR_STITCH_ILLEGAL_PARAM;                                                                  \
		}                                                                                                         \
	} while (0)

static CVI_S32 stitch_dev_close(CVI_VOID)
{
	pthread_mutex_lock(&stitch_fd_lock);
	close_device(&stitch_fd);
	pthread_mutex_unlock(&stitch_fd_lock);

	return CVI_SUCCESS;
}

CVI_S32 get_stitch_fd(CVI_VOID)
{
	pthread_mutex_lock(&stitch_fd_lock);
	if (stitch_fd <= 0) {
		if (open_device(STITCH_DEV_NAME, &stitch_fd) == -1) {
			perror("DWA open fail\n");
			stitch_fd = -1;
		}
	}
	pthread_mutex_unlock(&stitch_fd_lock);

	return stitch_fd;
}

CVI_S32 CVI_STITCH_Suspend(void)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_stitch_fd();

	s32Ret = cvi_stitch_suspend(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "suspend fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_Resume(void)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_stitch_fd();

	s32Ret = cvi_stitch_resume(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "resume fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_GetDevFd()
{
	return get_stitch_fd();
}

CVI_S32 CVI_STITCH_Init()
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_stitch_fd();

	s32Ret = cvi_stitch_init(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "init fail\n");
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 CVI_STITCH_DeInit()
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_stitch_fd();

	s32Ret = cvi_stitch_deinit(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "deinit fail\n");
		return s32Ret;
	}

	s32Ret = stitch_dev_close();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "stitch_dev_close fail\n");
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 CVI_STITCH_SetSrcAttr(STITCH_GRP stitchGrpIdx, STITCH_SRC_ATTR_S *srcAttr)
{
	CVI_S32 s32Ret;
	STITCH_GRP_SRC_ATTR_S cfg;
	CVI_S32 fd = get_stitch_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, srcAttr);
	CHECK_GRP_ID_VALID(stitchGrpIdx);

	cfg.StitchGrp = stitchGrpIdx;
	cfg.src_attr = *srcAttr;
	s32Ret = cvi_stitch_set_src_attr(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "set src attr fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_GetSrcAttr(STITCH_GRP stitchGrpIdx, STITCH_SRC_ATTR_S *srcAttr)
{
	CVI_S32 s32Ret;
	STITCH_GRP_SRC_ATTR_S cfg;
	CVI_S32 fd = get_stitch_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, srcAttr);
	CHECK_GRP_ID_VALID(stitchGrpIdx);

	cfg.StitchGrp = stitchGrpIdx;
	cfg.src_attr = *srcAttr;
	s32Ret = cvi_stitch_get_src_attr(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "get src attr fail, s32Ret[%d]\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_SetChnAttr(STITCH_GRP stitchGrpIdx, STITCH_CHN_ATTR_S *chnAttr)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_stitch_fd();
	STITCH_GRP_CHN_ATTR_S cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, chnAttr);
	CHECK_GRP_ID_VALID(stitchGrpIdx);

	cfg.StitchGrp = stitchGrpIdx;
	cfg.chn_attr = *chnAttr;
	s32Ret = cvi_stitch_set_chn_attr(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "set chn attr fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_GetChnAttr(STITCH_GRP stitchGrpIdx, STITCH_CHN_ATTR_S *chnAttr)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_stitch_fd();
	STITCH_GRP_CHN_ATTR_S cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, chnAttr);
	CHECK_GRP_ID_VALID(stitchGrpIdx);

	cfg.StitchGrp = stitchGrpIdx;
	cfg.chn_attr = *chnAttr;
	s32Ret = cvi_stitch_get_chn_attr(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "get chn attr fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_SetOpAttr(STITCH_GRP stitchGrpIdx, STITCH_OP_ATTR_S *opAttr)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_stitch_fd();
	STITCH_GRP_OP_ATTR_S cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, opAttr);
	CHECK_GRP_ID_VALID(stitchGrpIdx);

	cfg.StitchGrp = stitchGrpIdx;
	cfg.opt_attr = *opAttr;
	s32Ret = cvi_stitch_set_op_attr(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "set op attr fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_GetOpAttr(STITCH_GRP stitchGrpIdx, STITCH_OP_ATTR_S *opAttr)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_stitch_fd();
	STITCH_GRP_OP_ATTR_S cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, opAttr);
	CHECK_GRP_ID_VALID(stitchGrpIdx);

	cfg.StitchGrp = stitchGrpIdx;
	cfg.opt_attr = *opAttr;
	s32Ret = cvi_stitch_get_op_attr(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "get op attr fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_SetWgtAttr(STITCH_GRP stitchGrpIdx, STITCH_WGT_ATTR_S *wgtAttr)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_stitch_fd();
	STITCH_GRP_WGT_ATTR_S cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, wgtAttr);
	CHECK_GRP_ID_VALID(stitchGrpIdx);

	cfg.StitchGrp = stitchGrpIdx;
	cfg.bld_wgt_attr = *wgtAttr;
	s32Ret = cvi_stitch_set_wgt_attr(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "set wgt attr fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_GetWgtAttr(STITCH_GRP stitchGrpIdx, STITCH_WGT_ATTR_S *wgtAttr)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_stitch_fd();
	STITCH_GRP_WGT_ATTR_S cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, wgtAttr);
	CHECK_GRP_ID_VALID(stitchGrpIdx);

	cfg.StitchGrp = stitchGrpIdx;
	cfg.bld_wgt_attr = *wgtAttr;
	s32Ret = cvi_stitch_get_wgt_attr(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "get wgt attr fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_SetRegX(CVI_U8 regX)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_stitch_fd();

	s32Ret = cvi_stitch_set_regx(fd, regX);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "set reg x fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_EnableGrp(STITCH_GRP stitchGrpIdx)
{
	CVI_S32 s32Ret;
	STITCH_GRP_ATTR_S cfg;
	CVI_S32 fd = get_stitch_fd();

	CHECK_GRP_ID_VALID(stitchGrpIdx);

	cfg.StitchGrp = stitchGrpIdx;
	s32Ret = cvi_stitch_grp_enable(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "enable dev fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_DisableGrp(STITCH_GRP stitchGrpIdx)
{
	CVI_S32 s32Ret;
	STITCH_GRP_ATTR_S cfg;
	CVI_S32 fd = get_stitch_fd();

	CHECK_GRP_ID_VALID(stitchGrpIdx);

	cfg.StitchGrp = stitchGrpIdx;
	s32Ret = cvi_stitch_grp_disable(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "disable dev fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_Reset(void)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_stitch_fd();

	s32Ret = cvi_stitch_reset(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "reset fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_SendFrame(STITCH_GRP stitchGrpIdx, STITCH_SRC_IDX srcIdx, const VIDEO_FRAME_INFO_S *VideoFrame, CVI_S32 MilliSec)
{
	CVI_S32 s32Ret;
	struct stitch_grp_src_frm_cfg cfg;
	CVI_S32 fd = get_stitch_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, VideoFrame);
	CHECK_GRP_ID_VALID(stitchGrpIdx);
	CHECK_STITCH_SRC_ID_VALID(srcIdx);

	cfg.StitchGrp = stitchGrpIdx;
	cfg.cfg.src_id = srcIdx;
	cfg.cfg.MilliSec = MilliSec;
	memcpy(&cfg.cfg.VideoFrame, VideoFrame, sizeof(cfg.cfg.VideoFrame));
	s32Ret = cvi_stitch_send_src_frame(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "src_id(%d) send frame fail\n", srcIdx);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_SendChnFrame(STITCH_GRP stitchGrpIdx, const VIDEO_FRAME_INFO_S *VideoFrame, CVI_S32 MilliSec)
{
	CVI_S32 s32Ret;
	struct stitch_grp_chn_frm_cfg cfg;
	CVI_S32 fd = get_stitch_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, VideoFrame);
	CHECK_GRP_ID_VALID(stitchGrpIdx);

	cfg.StitchGrp = stitchGrpIdx;
	cfg.cfg.MilliSec = MilliSec;
	memcpy(&cfg.cfg.VideoFrame, VideoFrame, sizeof(cfg.cfg.VideoFrame));
	s32Ret = cvi_stitch_send_chn_frame(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "chn send frame fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_GetChnFrame(STITCH_GRP stitchGrpIdx, VIDEO_FRAME_INFO_S *VideoFrame, CVI_S32 MilliSec)
{
	CVI_S32 s32Ret;
	struct stitch_grp_chn_frm_cfg cfg;
	CVI_S32 fd = get_stitch_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, VideoFrame);
	CHECK_GRP_ID_VALID(stitchGrpIdx);

	memset(&cfg, 0, sizeof(cfg));
	cfg.StitchGrp = stitchGrpIdx;
	cfg.cfg.MilliSec = MilliSec;
	s32Ret = cvi_stitch_get_chn_frame(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, " get chn frame fail\n");
		return s32Ret;
	}
	memcpy(VideoFrame, &cfg.cfg.VideoFrame, sizeof(*VideoFrame));

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_ReleaseChnFrame(STITCH_GRP stitchGrpIdx, VIDEO_FRAME_INFO_S *VideoFrame)
{
	CVI_S32 s32Ret;
	struct stitch_grp_chn_frm_cfg cfg;
	CVI_S32 fd = get_stitch_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, VideoFrame);
	CHECK_GRP_ID_VALID(stitchGrpIdx);

	cfg.StitchGrp = stitchGrpIdx;
	memset(&cfg, 0, sizeof(cfg));
	memcpy(&cfg.cfg.VideoFrame, VideoFrame, sizeof(*VideoFrame));
	s32Ret = cvi_stitch_release_chn_frame(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "release chn frame fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_AttachVbPool(STITCH_GRP stitchGrpIdx, VB_POOL VbPool)
{
	CVI_S32 s32Ret;
	struct stitch_grp_vb_pool_cfg cfg;
	CVI_S32 fd = get_stitch_fd();

	CHECK_GRP_ID_VALID(stitchGrpIdx);

	memset(&cfg, 0, sizeof(cfg));
	cfg.StitchGrp = stitchGrpIdx;
	cfg.vb_pool_cfg.VbPool = VbPool;
	s32Ret = cvi_stitch_attach_vbpool(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "attach vbpool fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_DetachVbPool(STITCH_GRP stitchGrpIdx)
{
	CVI_S32 s32Ret;
	struct stitch_grp_vb_pool_cfg cfg;
	CVI_S32 fd = get_stitch_fd();

	CHECK_GRP_ID_VALID(stitchGrpIdx);

	cfg.StitchGrp = stitchGrpIdx;
	s32Ret = cvi_stitch_detach_vbpool(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "detach vbpool fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_DumpRegInfo(void)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_stitch_fd();

	s32Ret = cvi_stitch_dump_reginfo(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "dump reg info fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_InitGrp(STITCH_GRP stitchGrpIdx)
{
	CVI_S32 s32Ret;
	STITCH_GRP_ATTR_S cfg;
	CVI_S32 fd = get_stitch_fd();

	CHECK_GRP_ID_VALID(stitchGrpIdx);

	cfg.StitchGrp = stitchGrpIdx;
	s32Ret = cvi_stitch_init_grp(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "grp[%d] init fail\n", stitchGrpIdx);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 CVI_STITCH_DeInitGrp(STITCH_GRP stitchGrpIdx)
{
	CVI_S32 s32Ret;
	STITCH_GRP_ATTR_S cfg;
	CVI_S32 fd = get_stitch_fd();

	CHECK_GRP_ID_VALID(stitchGrpIdx);

	cfg.StitchGrp = stitchGrpIdx;
	s32Ret = cvi_stitch_deinit_grp(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "grp[%d] deinit fail\n", stitchGrpIdx);
		return s32Ret;
	}

	return s32Ret;
}

STITCH_GRP CVI_STITCH_GetAvailableGrp(void)
{
	CVI_S32 s32Ret;
	STITCH_GRP grp = STITCH_INVALID_GRP;
	CVI_S32 fd = get_stitch_fd();

	s32Ret = cvi_stitch_get_available_grp(fd, &grp);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "GetAvailableGrp fail, s32Ret:%d\n", s32Ret);
		return grp;
	}
	CHECK_GRP_ID_VALID(grp);

	return grp;
}

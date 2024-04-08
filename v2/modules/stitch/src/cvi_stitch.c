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

#include "linux/cvi_errno.h"

#include "cvi_buffer.h"
#include "cvi_base.h"
#include "cvi_sys.h"
#include "cvi_stitch.h"
#include "stitch_ioctl.h"


static CVI_S32 stitch_fd = -1;
static pthread_mutex_t stitch_fd_lock = PTHREAD_MUTEX_INITIALIZER;

static inline CVI_S32 CHECK_STITCH_SRC_ID_VALID(STITCH_SRC_IDX idx)
{
	if ((idx >= STITCH_MAX_SRC_NUM) || (idx < 0)) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "src_id(%d) exceeds Max(%d)\n", idx, STITCH_MAX_SRC_NUM - 1);
		return CVI_ERR_STITCH_ILLEGAL_PARAM;
	}
	return CVI_SUCCESS;
}

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
	CVI_TRACE_STITCH(CVI_DBG_DEBUG, "+\n");
	CVI_TRACE_STITCH(CVI_DBG_NOTICE, "Not Support\n");
	CVI_TRACE_STITCH(CVI_DBG_DEBUG, "-\n");
	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_Resume(void)
{
	CVI_TRACE_STITCH(CVI_DBG_DEBUG, "+\n");
	CVI_TRACE_STITCH(CVI_DBG_NOTICE, "Not Support\n");
	CVI_TRACE_STITCH(CVI_DBG_DEBUG, "-\n");
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

CVI_S32 CVI_STITCH_SetSrcAttr(STITCH_SRC_ATTR_S *srcAttr)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_stitch_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, srcAttr);

	s32Ret = cvi_stitch_set_src_attr(fd, srcAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "set src attr fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_GetSrcAttr(STITCH_SRC_ATTR_S *srcAttr)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_stitch_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, srcAttr);

	s32Ret = cvi_stitch_get_src_attr(fd, srcAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "get src attr fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_SetChnAttr(STITCH_CHN_ATTR_S *chnAttr)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_stitch_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, chnAttr);

	s32Ret = cvi_stitch_set_chn_attr(fd, chnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "set chn attr fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_GetChnAttr(STITCH_CHN_ATTR_S *chnAttr)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_stitch_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, chnAttr);

	s32Ret = cvi_stitch_get_chn_attr(fd, chnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "get chn attr fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_SetOpAttr(STITCH_OP_ATTR_S *opAttr)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_stitch_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, opAttr);

	s32Ret = cvi_stitch_set_op_attr(fd, opAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "set op attr fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_GetOpAttr(STITCH_OP_ATTR_S *opAttr)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_stitch_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, opAttr);

	s32Ret = cvi_stitch_get_op_attr(fd, opAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "get op attr fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_SetWgtAttr(STITCH_WGT_ATTR_S *wgtAttr)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_stitch_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, wgtAttr);

	s32Ret = cvi_stitch_set_wgt_attr(fd, wgtAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "set wgt attr fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_GetWgtAttr(STITCH_WGT_ATTR_S *wgtAttr)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_stitch_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, wgtAttr);

	s32Ret = cvi_stitch_get_wgt_attr(fd, wgtAttr);
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

CVI_S32 CVI_STITCH_EnableDev(void)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_stitch_fd();

	s32Ret = cvi_stitch_dev_enable(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "enable dev fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_DisableDev(void)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_stitch_fd();

	s32Ret = cvi_stitch_dev_disable(fd);
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

CVI_S32 CVI_STITCH_SendFrame(STITCH_SRC_IDX srcIdx, const VIDEO_FRAME_INFO_S *VideoFrame, CVI_S32 MilliSec)
{
	CVI_S32 s32Ret;
	struct stitch_src_frm_cfg cfg;
	CVI_S32 fd = get_stitch_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, VideoFrame);
	s32Ret = CHECK_STITCH_SRC_ID_VALID(srcIdx);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	cfg.src_id = srcIdx;
	cfg.MilliSec = MilliSec;
	memcpy(&cfg.VideoFrame, VideoFrame, sizeof(cfg.VideoFrame));

	s32Ret = cvi_stitch_send_src_frame(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "src_id(%d) send frame fail\n", srcIdx);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_SendChnFrame(const VIDEO_FRAME_INFO_S *VideoFrame, CVI_S32 MilliSec)
{
	CVI_S32 s32Ret;
	struct stitch_chn_frm_cfg cfg;
	CVI_S32 fd = get_stitch_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, VideoFrame);

	cfg.MilliSec = MilliSec;
	memcpy(&cfg.VideoFrame, VideoFrame, sizeof(cfg.VideoFrame));

	s32Ret = cvi_stitch_send_chn_frame(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "chn send frame fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_GetChnFrame(VIDEO_FRAME_INFO_S *VideoFrame, CVI_S32 MilliSec)
{
	CVI_S32 s32Ret;
	struct stitch_chn_frm_cfg cfg;
	CVI_S32 fd = get_stitch_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, VideoFrame);

	memset(&cfg, 0, sizeof(cfg));
	cfg.MilliSec = MilliSec;

	s32Ret = cvi_stitch_get_chn_frame(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, " get chn frame fail\n");
		return s32Ret;
	}
	memcpy(VideoFrame, &cfg.VideoFrame, sizeof(*VideoFrame));

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_ReleaseChnFrame(VIDEO_FRAME_INFO_S *VideoFrame)
{
	CVI_S32 s32Ret;
	struct stitch_chn_frm_cfg cfg;
	CVI_S32 fd = get_stitch_fd();

	MOD_CHECK_NULL_PTR(CVI_ID_STITCH, VideoFrame);

	memset(&cfg, 0, sizeof(cfg));
	memcpy(&cfg.VideoFrame, VideoFrame, sizeof(*VideoFrame));

	s32Ret = cvi_stitch_release_chn_frame(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "release chn frame fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_AttachVbPool(VB_POOL VbPool)
{
	CVI_S32 s32Ret;
	struct stitch_vb_pool_cfg cfg;
	CVI_S32 fd = get_stitch_fd();

	memset(&cfg, 0, sizeof(cfg));
	cfg.VbPool = VbPool;

	s32Ret = cvi_stitch_attach_vbpool(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_STITCH(CVI_DBG_ERR, "attach vbpool fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_STITCH_DetachVbPool(void)
{
	CVI_S32 s32Ret;
	struct stitch_vb_pool_cfg cfg;
	CVI_S32 fd = get_stitch_fd();

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


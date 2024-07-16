#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "stitch_uapi.h"
#include "stitch_ioctl.h"

CVI_S32 cvi_stitch_init(CVI_S32 fd)
{
	return ioctl(fd, CVI_STITCH_INIT);
}

CVI_S32 cvi_stitch_deinit(CVI_S32 fd)
{
	return ioctl(fd, CVI_STITCH_DEINIT);
}

CVI_S32 cvi_stitch_set_src_attr(CVI_S32 fd, STITCH_SRC_ATTR_S *attr)
{
	return ioctl(fd, CVI_STITCH_SET_SRC_ATTR, attr);
}

CVI_S32 cvi_stitch_get_src_attr(CVI_S32 fd, STITCH_SRC_ATTR_S *attr)
{
	return ioctl(fd, CVI_STITCH_GET_SRC_ATTR, attr);
}

CVI_S32 cvi_stitch_set_chn_attr(CVI_S32 fd, STITCH_CHN_ATTR_S *attr)
{
	return ioctl(fd, CVI_STITCH_SET_CHN_ATTR, attr);
}

CVI_S32 cvi_stitch_get_chn_attr(CVI_S32 fd, STITCH_CHN_ATTR_S *attr)
{
	return ioctl(fd, CVI_STITCH_GET_CHN_ATTR, attr);
}

CVI_S32 cvi_stitch_set_op_attr(CVI_S32 fd, STITCH_OP_ATTR_S *attr)
{
	return ioctl(fd, CVI_STITCH_SET_OP_ATTR, attr);
}

CVI_S32 cvi_stitch_get_op_attr(CVI_S32 fd, STITCH_OP_ATTR_S *attr)
{
	return ioctl(fd, CVI_STITCH_GET_OP_ATTR, attr);
}

CVI_S32 cvi_stitch_set_wgt_attr(CVI_S32 fd, STITCH_WGT_ATTR_S *attr)
{
	return ioctl(fd, CVI_STITCH_SET_WGT_ATTR, attr);
}

CVI_S32 cvi_stitch_get_wgt_attr(CVI_S32 fd, STITCH_WGT_ATTR_S *attr)
{
	return ioctl(fd, CVI_STITCH_GET_WGT_ATTR, attr);
}

CVI_S32 cvi_stitch_set_regx(CVI_S32 fd, CVI_U8 regx)
{
	return ioctl(fd, CVI_STITCH_SET_REGX, &regx);
}

CVI_S32 cvi_stitch_dev_enable(CVI_S32 fd)
{
	return ioctl(fd, CVI_STITCH_DEV_ENABLE);
}

CVI_S32 cvi_stitch_dev_disable(CVI_S32 fd)
{
	return ioctl(fd, CVI_STITCH_DEV_DISABLE);
}

CVI_S32 cvi_stitch_reset(CVI_S32 fd)
{
	return ioctl(fd, CVI_STITCH_RST);
}

CVI_S32 cvi_stitch_send_src_frame(CVI_S32 fd, struct stitch_src_frm_cfg *cfg)
{
	return ioctl(fd, CVI_STITCH_SEND_SRC_FRM, cfg);
}

CVI_S32 cvi_stitch_send_chn_frame(CVI_S32 fd, struct stitch_chn_frm_cfg *cfg)
{
	return ioctl(fd, CVI_STITCH_SEND_CHN_FRM, cfg);
}

CVI_S32 cvi_stitch_get_chn_frame(CVI_S32 fd, struct stitch_chn_frm_cfg *cfg)
{
	return ioctl(fd, CVI_STITCH_GET_CHN_FRM, cfg);
}

CVI_S32 cvi_stitch_release_chn_frame(CVI_S32 fd, struct stitch_chn_frm_cfg *cfg)
{
	return ioctl(fd, CVI_STITCH_RLS_CHN_FRM, cfg);
}

CVI_S32 cvi_stitch_attach_vbpool(CVI_S32 fd, const struct stitch_vb_pool_cfg *cfg)
{
	return ioctl(fd, CVI_STITCH_ATTACH_VB_POOL, cfg);
}

CVI_S32 cvi_stitch_detach_vbpool(CVI_S32 fd, const struct stitch_vb_pool_cfg *cfg)
{
	return ioctl(fd, CVI_STITCH_DETACH_VB_POOL, cfg);
}

CVI_S32 cvi_stitch_dump_reginfo(CVI_S32 fd)
{
	return ioctl(fd, CVI_STITCH_DUMP_REGS);
}

CVI_S32 cvi_stitch_suspend(CVI_S32 fd)
{
	return ioctl(fd, CVI_STITCH_SUSPEND);
}

CVI_S32 cvi_stitch_resume(CVI_S32 fd)
{
	return ioctl(fd, CVI_STITCH_RESUME);
}


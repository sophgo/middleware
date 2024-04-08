#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <errno.h>

#include "../include/dpu_ioctl.h"

CVI_S32 dpu_create_grp(CVI_S32 fd, struct dpu_grp_attr *grp_attr)
{
	return ioctl(fd, CVI_DPU_CREATE_GROUP, grp_attr);
}

CVI_S32 dpu_destroy_grp(CVI_S32 fd, struct dpu_grp_cfg *grp_cfg)
{
	return ioctl(fd, CVI_DPU_DESTROY_GROUP, grp_cfg);
}
// CVI_S32 dpu_get_available_grp(CVI_S32 fd, DPU_GRP *pVpssGrp)
// {
// 	return 0;
// }

CVI_S32 dpu_start_grp(CVI_S32 fd, struct dpu_grp_cfg *grp_cfg)
{
	return ioctl(fd, CVI_DPU_START_GROUP, grp_cfg);
}

CVI_S32 dpu_stop_grp(CVI_S32 fd, struct dpu_grp_cfg *grp_cfg)
{
	return ioctl(fd, CVI_DPU_STOP_GROUP, grp_cfg);
}

CVI_S32 dpu_set_grp_attr(CVI_S32 fd, struct dpu_grp_attr *grp_attr)
{
	return ioctl(fd, CVI_DPU_SET_GRP_ATTR, grp_attr);
}

CVI_S32 dpu_get_grp_attr(CVI_S32 fd, struct dpu_grp_attr *grp_attr)
{
	return ioctl(fd, CVI_DPU_GET_GRP_ATTR, grp_attr);
}

CVI_S32 dpu_set_chn_attr(CVI_S32 fd, struct dpu_chn_attr *chn_attr)
{
	return ioctl(fd, CVI_DPU_SET_CHN_ATTR, chn_attr);
}

CVI_S32 dpu_get_chn_attr(CVI_S32 fd, struct dpu_chn_attr *chn_attr)
{
	return ioctl(fd, CVI_DPU_GET_CHN_ATTR, chn_attr);
}

CVI_S32 dpu_enable_chn(CVI_S32 fd, struct dpu_chn_cfg *chn_cfg)
{
	return ioctl(fd, CVI_DPU_ENABLE_CHN, chn_cfg);
}

CVI_S32 dpu_disable_chn(CVI_S32 fd, struct dpu_chn_cfg *chn_cfg)
{
	return ioctl(fd, CVI_DPU_DISABLE_CHN, chn_cfg);
}

CVI_S32 dpu_get_frame(CVI_S32 fd, struct dpu_get_frame_cfg *frame_cfg)
{
	return ioctl(fd, CVI_DPU_GET_FRAME, frame_cfg);
}

CVI_S32 dpu_release_frame(CVI_S32 fd, struct dpu_release_frame_cfg *frame_cfg)
{
	return ioctl(fd, CVI_DPU_RELEASE_FRAME, frame_cfg);
}

CVI_S32 dpu_send_frame(CVI_S32 fd, struct dpu_set_frame_cfg *frame_cfg)
{
	return ioctl(fd, CVI_DPU_SEND_FRAME, frame_cfg);
}

void dpu_check_reg_read(CVI_S32 fd)
{
	ioctl(fd, CVI_DPU_CHECK_REG_READ,NULL);
}

void dpu_check_reg_write(CVI_S32 fd)
{
	ioctl(fd, CVI_DPU_CHECK_REG_WRITE,NULL);
}

void dpu_get_sgbm_status(CVI_S32 fd)
{
	ioctl(fd, CVI_DPU_GET_SGBM_STATUS,NULL);
}

void dpu_get_fgs_status(CVI_S32 fd)
{
	ioctl(fd, CVI_DPU_GET_FGS_STATUS,NULL);
}
void dpu_reset(CVI_S32 fd)
{
	ioctl(fd, CVI_DPU_RESET,NULL);
}


#ifndef MODULES_DPU_IOCTL_H_
#define MODULES_DPU_IOCTL_H_

#include <sys/ioctl.h>

#include "dpu_uapi.h"
#include <linux/cvi_comm_dpu.h>
/* Configured from user  */
CVI_S32 dpu_create_grp(CVI_S32 fd, struct dpu_grp_attr *grp_attr);
CVI_S32 dpu_destroy_grp(CVI_S32 fd, struct dpu_grp_cfg *grp_cfg);
CVI_S32 dpu_get_available_grp(CVI_S32 fd, DPU_GRP *pVpssGrp);
CVI_S32 dpu_start_grp(CVI_S32 fd, struct dpu_grp_cfg *grp_cfg);
CVI_S32 dpu_stop_grp(CVI_S32 fd, struct dpu_grp_cfg *grp_cfg);
CVI_S32 dpu_set_grp_attr(CVI_S32 fd, struct dpu_grp_attr *grp_attr);
CVI_S32 dpu_get_grp_attr(CVI_S32 fd, struct dpu_grp_attr *grp_attr);
CVI_S32 dpu_set_chn_attr(CVI_S32 fd, struct dpu_chn_attr *chn_attr);
CVI_S32 dpu_get_chn_attr(CVI_S32 fd, struct dpu_chn_attr *chn_attr);
CVI_S32 dpu_enable_chn(CVI_S32 fd, struct dpu_chn_cfg *chn_cfg);
CVI_S32 dpu_disable_chn(CVI_S32 fd, struct dpu_chn_cfg *chn_cfg);
CVI_S32 dpu_get_frame(CVI_S32 fd, struct dpu_get_frame_cfg *frame_cfg);
CVI_S32 dpu_release_frame(CVI_S32 fd, struct dpu_release_frame_cfg *frame_cfg);
CVI_S32 dpu_send_frame(CVI_S32 fd, struct dpu_set_frame_cfg *frame_cfg);
void dpu_check_reg_read(CVI_S32 fd);
void dpu_check_reg_write(CVI_S32 fd);
void dpu_get_sgbm_status(CVI_S32 fd);
void dpu_get_fgs_status(CVI_S32 fd);
void dpu_reset(CVI_S32 fd);


#endif /* MODULES_DPU_IOCTL_H_*/

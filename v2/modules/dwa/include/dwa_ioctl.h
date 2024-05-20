#ifndef MODULES_VPU_INCLUDE_DWA_IOCTL_H_
#define MODULES_VPU_INCLUDE_DWA_IOCTL_H_

#include <sys/ioctl.h>

#include <linux/dwa_uapi.h>

/* Configured from user  */
CVI_S32 dwa_init(CVI_S32 fd);
CVI_S32 dwa_deinit(CVI_S32 fd);
CVI_S32 dwa_begin_job(CVI_S32 fd, struct dwa_handle_data *cfg);
CVI_S32 dwa_end_job(CVI_S32 fd, struct dwa_handle_data *cfg);
CVI_S32 dwa_cancel_job(CVI_S32 fd, struct dwa_handle_data *cfg);
CVI_S32 dwa_add_rotation_task(CVI_S32 fd, struct dwa_task_attr *attr);
CVI_S32 dwa_add_ldc_task(CVI_S32 fd, struct dwa_task_attr *attr);
CVI_S32 dwa_add_warp_task(CVI_S32 fd, struct dwa_task_attr *attr);
CVI_S32 dwa_add_correction_task(CVI_S32 fd, struct dwa_task_attr *attr);
CVI_S32 dwa_add_affine_task(CVI_S32 fd, struct dwa_task_attr *attr);
CVI_S32 dwa_set_job_identity(CVI_S32 fd, struct dwa_identity_attr *indentity);
CVI_S32 dwa_get_work_job(CVI_S32 fd, struct dwa_handle_data *cfg);
CVI_S32 dwa_get_chn_frm(CVI_S32 fd, struct dwa_chn_frm_cfg *cfg);
CVI_S32 dwa_suspend(CVI_S32 fd);
CVI_S32 dwa_resume(CVI_S32 fd);

#endif /* MODULES_VPU_INCLUDE_DWA_IOCTL_H_ */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "dwa_ioctl.h"

CVI_S32 dwa_init(CVI_S32 fd)
{
	return ioctl(fd, CVI_DWA_INIT);
}

CVI_S32 dwa_deinit(CVI_S32 fd)
{
	return ioctl(fd, CVI_DWA_DEINIT);
}

CVI_S32 dwa_begin_job(CVI_S32 fd, struct dwa_handle_data *cfg)
{
	return ioctl(fd, CVI_DWA_BEGIN_JOB, cfg);
}

CVI_S32 dwa_end_job(CVI_S32 fd, struct dwa_handle_data *cfg)
{
	return ioctl(fd, CVI_DWA_END_JOB, cfg);
}

CVI_S32 dwa_cancel_job(CVI_S32 fd, struct dwa_handle_data *cfg)
{
	return ioctl(fd, CVI_DWA_CANCEL_JOB, cfg);
}

CVI_S32 dwa_add_rotation_task(CVI_S32 fd, struct dwa_task_attr *attr)
{
	return ioctl(fd, CVI_DWA_ADD_ROT_TASK, attr);
}

CVI_S32 dwa_add_ldc_task(CVI_S32 fd, struct dwa_task_attr *attr)
{
	return ioctl(fd, CVI_DWA_ADD_LDC_TASK, attr);
}

CVI_S32 dwa_add_correction_task(CVI_S32 fd, struct dwa_task_attr *attr)
{
	return ioctl(fd, CVI_DWA_ADD_COR_TASK, attr);
}

CVI_S32 dwa_add_warp_task(CVI_S32 fd, struct dwa_task_attr *attr)
{
	return ioctl(fd, CVI_DWA_ADD_WAR_TASK, attr);
}

CVI_S32 dwa_add_affine_task(CVI_S32 fd, struct dwa_task_attr *attr)
{
	return ioctl(fd, CVI_DWA_ADD_AFF_TASK, attr);
}

CVI_S32 dwa_set_job_identity(CVI_S32 fd, struct dwa_identity_attr *indentity)
{
	return ioctl(fd, CVI_DWA_SET_JOB_IDENTITY, indentity);
}

CVI_S32 dwa_get_work_job(CVI_S32 fd, struct dwa_handle_data *cfg)
{
	return ioctl(fd, CVI_DWA_GET_WORK_JOB, cfg);
}

CVI_S32 dwa_get_chn_frm(CVI_S32 fd, struct dwa_chn_frm_cfg *cfg)
{
	return ioctl(fd, CVI_DWA_GET_CHN_FRM, cfg);
}


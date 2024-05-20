#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "ldc_ioctl.h"

CVI_S32 gdc_init(CVI_S32 fd)
{
	return ioctl(fd, CVI_LDC_INIT);
}

CVI_S32 gdc_deinit(CVI_S32 fd)
{
	return ioctl(fd, CVI_LDC_DEINIT);
}

CVI_S32 gdc_begin_job(CVI_S32 fd, struct gdc_handle_data *cfg)
{
	return ioctl(fd, CVI_LDC_BEGIN_JOB, cfg);
}

CVI_S32 gdc_end_job(CVI_S32 fd, struct gdc_handle_data *cfg)
{
	return ioctl(fd, CVI_LDC_END_JOB, cfg);
}

CVI_S32 gdc_cancel_job(CVI_S32 fd, struct gdc_handle_data *cfg)
{
	return ioctl(fd, CVI_LDC_CANCEL_JOB, cfg);
}

CVI_S32 gdc_add_rotation_task(CVI_S32 fd, struct gdc_task_attr *attr)
{
	return ioctl(fd, CVI_LDC_ADD_ROT_TASK, attr);
}

CVI_S32 gdc_add_ldc_task(CVI_S32 fd, struct gdc_task_attr *attr)
{
	return ioctl(fd, CVI_LDC_ADD_LDC_TASK, attr);
}

CVI_S32 gdc_set_chn_buf_wrap(CVI_S32 fd, const struct ldc_buf_wrap_cfg *cfg)
{
	return ioctl(fd, CVI_LDC_SET_BUF_WRAP, cfg);
}

CVI_S32 gdc_get_chn_buf_wrap(CVI_S32 fd, struct ldc_buf_wrap_cfg *cfg)
{
	return ioctl(fd, CVI_LDC_GET_BUF_WRAP, cfg);
}

CVI_S32 gdc_set_job_identity(CVI_S32 fd, struct gdc_identity_attr *indentity)
{
	return ioctl(fd, CVI_LDC_SET_JOB_IDENTITY, indentity);
}

CVI_S32 gdc_get_work_job(CVI_S32 fd, struct gdc_handle_data *cfg)
{
	return ioctl(fd, CVI_LDC_GET_WORK_JOB, cfg);
}

CVI_S32 gdc_get_chn_frm(CVI_S32 fd, struct gdc_chn_frm_cfg *cfg)
{
	return ioctl(fd, CVI_LDC_GET_CHN_FRM, cfg);
}

CVI_S32 gdc_suspend(CVI_S32 fd)
{
	return ioctl(fd, CVI_LDC_SUSPEND);
}

CVI_S32 gdc_resume(CVI_S32 fd)
{
	return ioctl(fd, CVI_LDC_RESUME);
}


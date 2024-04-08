#include "vo_ut.h"

int _vo_set_pattern(int fd, int vodev)
{
	long pattern = 0;

	return vo_set_pattern(fd, pattern, vodev);
}

int _vo_set_frame_bgcolor(int fd, int vodev)
{
	CVI_U16 rgb[3] = {0xFF, 0x00, 0x00};
	VO_LAYER volayer = vodev;

	return vo_set_frame_bgcolor(fd, rgb, volayer);
}

int _vo_set_window_bgcolor(int fd, int vodev)
{
	CVI_U16 rgb[3] = {0x00, 0xFF, 0x00};
	VO_LAYER volayer = vodev;

	return vo_set_window_bgcolor(fd, rgb, volayer);
}

int _vo_enable_window_bgcolor(int fd, int vodev)
{
	int enable = 0;
	VO_LAYER volayer = vodev;

	return vo_enable_window_bgcolor(fd, enable, volayer);
}

int _vo_set_align(int fd, int vodev)
{
	int align = 0;
	VO_LAYER volayer = vodev;

	return vo_set_align(fd, align, volayer);
}

int _vo_get_videolayer_size(int fd, int vodev)
{
	SIZE_S vsize;
	VO_LAYER volayer = vodev;

	return vo_get_videolayer_size(fd, &vsize, volayer);
}

int _vo_get_intf_type(int fd, int vodev)
{
	CVI_U32 intf = 0;

	return vo_get_intf_type(fd, &intf, vodev);
}

int _vo_get_panel_status(int fd, int vodev)
{
	struct vo_panel_status_cfg cfg;

	cfg.VoLayer = vodev;
	cfg.VoChn = 0;

	return vo_get_panel_status(fd, &cfg);
}

int _vo_get_gamma_ctrl(int fd, int vodev)
{
	int s32Ret;
	VO_GAMMA_INFO_S gamma_attr;

	s32Ret = vo_get_gamma_ctrl(fd, &gamma_attr, vodev);
	if (s32Ret != 0)
		return s32Ret;

	return vo_set_gamma_ctrl(fd, &gamma_attr, vodev);
}

int _vo_get_dv_timings(int fd, int vodev)
{
	int s32Ret = 0;
	struct vo_dv_timings timings;

	s32Ret =  vo_get_dv_timings(fd, &timings, vodev);
	if (s32Ret != 0)
		return s32Ret;

	return vo_set_dv_timings(fd, &timings, vodev);
}

static int (*ioctl_test[])(int, int) = {
	_vo_set_pattern,
	_vo_set_frame_bgcolor,
	_vo_set_window_bgcolor,
	_vo_enable_window_bgcolor,
	_vo_set_align,
	_vo_get_videolayer_size,
	_vo_get_panel_status,
	_vo_get_intf_type,
	_vo_get_gamma_ctrl,
	_vo_get_dv_timings
};

CVI_S32 vo_ioctl_test(int fd, int vodev)
{
	CVI_S32 ret = 0;
	CVI_U32 i = 0;

	for (i = 0; i < (int)ARRAY_SIZE(ioctl_test); ++i) {
		if (ioctl_test[i] != NULL) {
			ret = ioctl_test[i](fd, vodev);
			if (ret != 0)
				return ret;
		} else {
			printf("ioctl no[%d] = NULL\n", i);
		}
	}

	return ret;
}

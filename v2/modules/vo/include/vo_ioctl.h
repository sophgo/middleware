#ifndef __VO_IOCTL_H__
#define __VO_IOCTL_H__

#include "vo_uapi.h"

#define VO_S_CTRL_VALUE(_fd, _cfg, _ioctl)\
	do {\
		struct vo_ext_control ec1;\
		memset(&ec1, 0, sizeof(ec1));\
		ec1.id = _ioctl;\
		ec1.value = _cfg;\
		return ioctl(_fd, VO_IOC_S_CTRL, &ec1);\
	} while (0)

#define VO_S_CTRL_PTR(_fd, _cfg, _ioctl)\
	do {\
		struct vo_ext_control ec1;\
		memset(&ec1, 0, sizeof(ec1));\
		ec1.id = _ioctl;\
		ec1.ptr = (void *)_cfg;\
		return ioctl(_fd, VO_IOC_S_CTRL, &ec1);\
	} while (0)

#define VO_S_CTRL_RESERVE_VALUE(_fd, _cfg, _reserve, _ioctl)\
	do {\
		struct vo_ext_control ec1;\
		memset(&ec1, 0, sizeof(ec1));\
		ec1.id = _ioctl;\
		ec1.value = _cfg;\
		ec1.reserved[0] = _reserve;\
		return ioctl(_fd, VO_IOC_S_CTRL, &ec1);\
	} while (0)

#define VO_S_CTRL_RESERVE_PTR(_fd, _cfg, _reserved, _ioctl)\
	do {\
		struct vo_ext_control ec1;\
		memset(&ec1, 0, sizeof(ec1));\
		ec1.id = _ioctl;\
		ec1.ptr = (void *)_cfg;\
		ec1.reserved[0] = _reserved;\
		if (_cfg != NULL)\
			ec1.size = sizeof(*_cfg);\
		return ioctl(_fd, VO_IOC_S_CTRL, &ec1);\
	} while (0)

#define VO_G_CTRL_PTR(_fd, _cfg, _ioctl)\
	do {\
		struct vo_ext_control ec1;\
		memset(&ec1, 0, sizeof(ec1));\
		ec1.id = _ioctl;\
		ec1.ptr = (void *)_cfg;\
		return ioctl(_fd, VO_IOC_G_CTRL, &ec1);\
	} while (0)

#define VO_G_CTRL_RESERVE_PTR(_fd, _cfg, _reserve, _ioctl)\
	do {\
		struct vo_ext_control ec1;\
		memset(&ec1, 0, sizeof(ec1));\
		ec1.id = _ioctl;\
		ec1.ptr = (void *)_cfg;\
		ec1.reserved[0] = _reserve;\
		return ioctl(_fd, VO_IOC_G_CTRL, &ec1);\
	} while (0)

#define VO_SDK_CTRL_PTR(_fd, _cfg, _ioctl, _sdk_id)\
	do {\
		struct vo_ext_control ec1;\
		memset(&ec1, 0, sizeof(ec1));\
		ec1.id = _ioctl;\
		ec1.sdk_id = _sdk_id;\
		ec1.ptr = (void *)_cfg;\
		if (_cfg != NULL)\
			ec1.size = sizeof(*_cfg);\
		return ioctl(_fd, VO_IOC_S_CTRL, &ec1);\
	} while (0)

//self ioctl cmd test
int vo_set_pattern(int fd, VO_PATTERN_MODE pattern, unsigned int vodev);
int vo_set_frame_bgcolor(int fd, void *rgb, unsigned int vodev);
int vo_set_window_bgcolor(int fd, void *rgb, unsigned int vodev);
int vo_enable_window_bgcolor(int fd, int enable, unsigned int vodev);
int vo_get_videolayer_size(int fd, SIZE_S *vsize, unsigned int vodev);
int vo_get_intf_type(int fd, CVI_U32 *vo_sel, unsigned int vodev);
int vo_set_gamma_ctrl(int fd, VO_GAMMA_INFO_S *gamma_attr, unsigned int vodev);
int vo_get_gamma_ctrl(int fd, VO_GAMMA_INFO_S *gamma_attr, unsigned int vodev);
int vo_set_tgt_compose(int fd, struct vo_rect *area, unsigned int vodev);
int vo_set_tgt_crop(int fd, struct vo_rect *area, unsigned int vodev);
int vo_set_dv_timings(int fd, struct vo_dv_timings *timings, unsigned int vodev);
int vo_get_dv_timings(int fd, struct vo_dv_timings *timings, unsigned int vodev);
int vo_set_stop_streaming(int fd, unsigned int vodev);
int vo_set_start_streaming(int fd, unsigned int vodev);
int vo_set_csc(int fd, struct disp_csc_matrix *cfg, unsigned int volayer);

//vo sdk layer apis
int vo_sdk_enable(int fd, struct vo_dev_cfg *cfg);
int vo_sdk_disable(int fd, struct vo_dev_cfg *cfg);
int vo_sdk_isenable(int fd, struct vo_dev_cfg *cfg);
int vo_sdk_get_panelstatue(int fd, struct vo_panel_status_cfg *cfg);
int vo_sdk_get_pubattr(int fd, struct vo_pub_attr_cfg *cfg);
int vo_sdk_set_pubattr(int fd, struct vo_pub_attr_cfg *cfg);
int vo_sdk_set_lvdsparam(int fd, struct vo_lvds_param_cfg *cfg);
int vo_sdk_get_lvdsparam(int fd, struct vo_lvds_param_cfg *cfg);
int vo_sdk_set_btparam(int fd, struct vo_bt_param_cfg *cfg);
int vo_sdk_get_btparam(int fd, struct vo_bt_param_cfg *cfg);
int vo_sdk_set_hdmiparam(int fd, struct vo_hdmi_param_cfg *cfg);
int vo_sdk_get_hdmiparam(int fd, struct vo_hdmi_param_cfg *cfg);
int vo_sdk_get_displaybuflen(int fd, struct vo_display_buflen_cfg *cfg);
int vo_sdk_set_displaybuflen(int fd, struct vo_display_buflen_cfg *cfg);
int vo_sdk_set_videolayerattr(int fd, struct vo_video_layer_attr_cfg *cfg);
int vo_sdk_get_videolayerattr(int fd, struct vo_video_layer_attr_cfg *cfg);
int vo_sdk_set_layer_proc_amp(int fd, struct vo_layer_proc_amp_cfg *cfg);
int vo_sdk_get_layer_proc_amp(int fd, struct vo_layer_proc_amp_cfg *cfg);
int vo_sdk_set_layer_csc(int fd, struct vo_layer_csc_cfg *cfg);
int vo_sdk_get_layer_csc(int fd, struct vo_layer_csc_cfg *cfg);
int vo_sdk_enable_videolayer(int fd, struct vo_video_layer_cfg *cfg);
int vo_sdk_disable_videolayer(int fd, struct vo_video_layer_cfg *cfg);
int vo_sdk_set_layer_toleration(int fd, struct vo_layer_toleration_cfg *cfg);
int vo_sdk_get_layer_toleration(int fd, struct vo_layer_toleration_cfg *cfg);
int vo_sdk_set_layer_priority(int fd, struct vo_layer_priority_cfg *cfg);
int vo_sdk_get_layer_priority(int fd, struct vo_layer_priority_cfg *cfg);
int vo_sdk_get_screen_frame(int fd, struct vo_screen_frame *cfg);
int vo_sdk_release_screen_frame(int fd, struct vo_screen_frame *cfg);
int vo_sdk_bind_layer(int fd, struct vo_video_layer_bind_cfg *cfg);
int vo_sdk_unbind_layer(int fd, struct vo_video_layer_bind_cfg *cfg);
int vo_sdk_enable_chn(int fd, struct vo_chn_cfg *cfg);
int vo_sdk_disable_chn(int fd, struct vo_chn_cfg *cfg);
int vo_sdk_send_frame(int fd, struct vo_snd_frm_cfg *cfg);
int vo_sdk_clearchnbuf(int fd, struct vo_clear_chn_buf_cfg *cfg);
int vo_sdk_set_chnattr(int fd, struct vo_chn_attr_cfg *cfg);
int vo_sdk_get_chnattr(int fd, struct vo_chn_attr_cfg *cfg);
int vo_sdk_set_chnparam(int fd, struct vo_chn_param_cfg *cfg);
int vo_sdk_get_chnparam(int fd, struct vo_chn_param_cfg *cfg);
int vo_sdk_set_chnzoom(int fd, struct vo_chn_zoom_cfg *cfg);
int vo_sdk_get_chnzoom(int fd, struct vo_chn_zoom_cfg *cfg);
int vo_sdk_set_chnborder(int fd, struct vo_chn_border_cfg *cfg);
int vo_sdk_get_chnborder(int fd, struct vo_chn_border_cfg *cfg);
int vo_sdk_set_chnmirror(int fd, struct vo_chn_mirror_cfg *cfg);
int vo_sdk_get_chnmirror(int fd, struct vo_chn_mirror_cfg *cfg);
int vo_sdk_set_chn_frmrate(int fd, struct vo_chn_frmrate_cfg *cfg);
int vo_sdk_get_chn_frmrate(int fd, struct vo_chn_frmrate_cfg *cfg);
int vo_sdk_get_chn_frame(int fd, struct vo_chn_frame_cfg *cfg);
int vo_sdk_release_chn_frame(int fd, struct vo_chn_frame_cfg *cfg);
int vo_sdk_get_chn_pts(int fd, struct vo_chn_pts_cfg *cfg);
int vo_sdk_get_chn_status(int fd, struct vo_chn_status_cfg *cfg);
int vo_sdk_set_chn_threshold(int fd, struct vo_chn_threshold_cfg *cfg);
int vo_sdk_get_chn_threshold(int fd, struct vo_chn_threshold_cfg *cfg);
int vo_sdk_set_chnrotation(int fd, struct vo_chn_rotation_cfg *cfg);
int vo_sdk_get_chnrotation(int fd, struct vo_chn_rotation_cfg *cfg);
int vo_sdk_showchn(int fd, struct vo_chn_cfg *cfg);
int vo_sdk_hidechn(int fd, struct vo_chn_cfg *cfg);
int vo_sdk_pausechn(int fd, struct vo_chn_cfg *cfg);
int vo_sdk_stepchn(int fd, struct vo_chn_cfg *cfg);
int vo_sdk_refreshchn(int fd, struct vo_chn_cfg *cfg);
int vo_sdk_resumechn(int fd, struct vo_chn_cfg *cfg);
int vo_sdk_set_wbcsrc(int fd, struct vo_wbc_src_cfg *cfg);
int vo_sdk_get_wbcsrc(int fd, struct vo_wbc_src_cfg *cfg);
int vo_sdk_enable_wbc(int fd, struct vo_wbc_cfg *cfg);
int vo_sdk_disable_wbc(int fd, struct vo_wbc_cfg *cfg);
int vo_sdk_set_wbcattr(int fd, struct vo_wbc_attr_cfg *cfg);
int vo_sdk_get_wbcattr(int fd, struct vo_wbc_attr_cfg *cfg);
int vo_sdk_set_wbcmode(int fd, struct vo_wbc_mode_cfg *cfg);
int vo_sdk_get_wbcmode(int fd, struct vo_wbc_mode_cfg *cfg);
int vo_sdk_set_wbcdepth(int fd, struct vo_wbc_depth_cfg *cfg);
int vo_sdk_get_wbcdepth(int fd, struct vo_wbc_depth_cfg *cfg);
int vo_sdk_get_wbcframe(int fd, struct vo_wbc_frame_cfg *cfg);
int vo_sdk_release_wbcframe(int fd, struct vo_wbc_frame_cfg *cfg);
int vo_sdk_attach_layer_vb_pool(int fd, struct vo_layer_vb_pool_cfg *cfg);
int vo_sdk_detach_layer_vb_pool(int fd, struct vo_layer_vb_pool_cfg *cfg);
int vo_sdk_attach_wbc_vb_pool(int fd, struct vo_wbc_vb_pool_cfg *cfg);
int vo_sdk_detach_wbc_vb_pool(int fd, struct vo_wbc_vb_pool_cfg *cfg);
int vo_sdk_suspend(int fd);
int vo_sdk_resume(int fd);

#endif

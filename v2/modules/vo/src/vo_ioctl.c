#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#include <fcntl.h>		/* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <cvi_comm_vo.h>
#include "vo_uapi.h"
#include "vo_ioctl.h"

int vo_set_pattern(int fd, VO_PATTERN_MODE pattern, unsigned int vodev)
{
	VO_S_CTRL_RESERVE_VALUE(fd, pattern, vodev, VO_IOCTL_PATTERN);
}

int vo_set_frame_bgcolor(int fd, void *rgb, unsigned int vodev)
{
	VO_S_CTRL_RESERVE_PTR(fd, rgb, vodev, VO_IOCTL_FRAME_BGCOLOR);
}

int vo_set_window_bgcolor(int fd, void *rgb, unsigned int vodev)
{
	VO_S_CTRL_RESERVE_PTR(fd, rgb, vodev, VO_IOCTL_WINDOW_BGCOLOR);
}

int vo_enable_window_bgcolor(int fd, int enable, unsigned int vodev)
{
	VO_S_CTRL_RESERVE_VALUE(fd, enable, vodev, VO_IOCTL_ENABLE_WIN_BGCOLOR);
}

int vo_set_csc(int fd, struct disp_csc_matrix *cfg, unsigned int volayer)
{
	VO_S_CTRL_RESERVE_PTR(fd, cfg, volayer, VO_IOCTL_SET_CUSTOM_CSC);
}

int vo_get_videolayer_size(int fd, SIZE_S *vsize, unsigned int vodev)
{
	VO_G_CTRL_RESERVE_PTR(fd, vsize, vodev, VO_IOCTL_GET_VLAYER_SIZE);
}

int vo_get_intf_type(int fd, CVI_U32 *intf, unsigned int vodev)
{
	VO_G_CTRL_RESERVE_PTR(fd, intf, vodev, VO_IOCTL_GET_INTF_TYPE);
}

int vo_set_gamma_ctrl(int fd, VO_GAMMA_INFO_S *gamma_attr, unsigned int vodev)
{
	VO_S_CTRL_RESERVE_PTR(fd, gamma_attr, vodev, VO_IOCTL_GAMMA_LUT_UPDATE);
}

int vo_get_gamma_ctrl(int fd, VO_GAMMA_INFO_S *gamma_attr, unsigned int vodev)
{
	VO_G_CTRL_RESERVE_PTR(fd, gamma_attr, vodev, VO_IOCTL_GAMMA_LUT_READ);
}

int vo_set_tgt_compose(int fd, struct vo_rect *area, unsigned int vodev)
{
	VO_S_CTRL_RESERVE_PTR(fd, area, vodev, VO_IOCTL_SEL_TGT_COMPOSE);
}

int vo_set_tgt_crop(int fd, struct vo_rect *area, unsigned int vodev)
{
	VO_S_CTRL_RESERVE_PTR(fd, area, vodev, VO_IOCTL_SEL_TGT_CROP);
}

int vo_set_dv_timings(int fd, struct vo_dv_timings *timings, unsigned int vodev)
{
	VO_S_CTRL_RESERVE_PTR(fd, timings, vodev, VO_IOCTL_SET_DV_TIMINGS);
}

int vo_get_dv_timings(int fd, struct vo_dv_timings *timings, unsigned int vodev)
{
	VO_G_CTRL_RESERVE_PTR(fd, timings, vodev, VO_IOCTL_GET_DV_TIMINGS);
}

int vo_set_stop_streaming(int fd, unsigned int vodev)
{
	VO_S_CTRL_RESERVE_VALUE(fd, 0, vodev, VO_IOCTL_STOP_STREAMING);
}

int vo_set_start_streaming(int fd, unsigned int vodev)
{
	VO_S_CTRL_RESERVE_VALUE(fd, 0, vodev, VO_IOCTL_START_STREAMING);
}

//vo sdk API list
int vo_sdk_clearchnbuf(int fd, struct vo_clear_chn_buf_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_CLEAR_CHNBUF);
}

int vo_sdk_send_frame(int fd, struct vo_snd_frm_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SEND_FRAME);
}

int vo_sdk_get_panelstatue(int fd, struct vo_panel_status_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_PANELSTATUE);
}

int vo_sdk_get_pubattr(int fd, struct vo_pub_attr_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_PUBATTR);
}

int vo_sdk_set_pubattr(int fd, struct vo_pub_attr_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SET_PUBATTR);
}

int vo_sdk_set_lvdsparam(int fd, struct vo_lvds_param_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SET_LVDSPARAM);
}

int vo_sdk_get_lvdsparam(int fd, struct vo_lvds_param_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_LVDSPARAM);
}

int vo_sdk_set_btparam(int fd, struct vo_bt_param_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SET_BTPARAM);
}

int vo_sdk_get_btparam(int fd, struct vo_bt_param_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_BTPARAM);
}

int vo_sdk_set_hdmiparam(int fd, struct vo_hdmi_param_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SET_HDMIPARAM);
}

int vo_sdk_get_hdmiparam(int fd, struct vo_hdmi_param_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_HDMIPARAM);
}

int vo_sdk_suspend(int fd)
{
	VO_SDK_CTRL_PTR(fd, NULL, VO_IOCTL_SDK_CTRL, VO_SDK_SUSPEND);
}

int vo_sdk_resume(int fd)
{
	VO_SDK_CTRL_PTR(fd, NULL, VO_IOCTL_SDK_CTRL, VO_SDK_RESUME);
}

int vo_sdk_get_displaybuflen(int fd, struct vo_display_buflen_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_DISPLAYBUFLEN);
}

int vo_sdk_set_displaybuflen(int fd, struct vo_display_buflen_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SET_DISPLAYBUFLEN);
}

int vo_sdk_set_videolayerattr(int fd, struct vo_video_layer_attr_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SET_VIDEOLAYERATTR);
}

int vo_sdk_get_videolayerattr(int fd, struct vo_video_layer_attr_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_VIDEOLAYERATTR);
}

int vo_sdk_set_layer_proc_amp(int fd, struct vo_layer_proc_amp_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SET_LAYER_PROC_AMP);
}

int vo_sdk_get_layer_proc_amp(int fd, struct vo_layer_proc_amp_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_LAYER_PROC_AMP);
}

int vo_sdk_set_layer_csc(int fd, struct vo_layer_csc_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SET_LAYERCSC);
}

int vo_sdk_get_layer_csc(int fd, struct vo_layer_csc_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_LAYERCSC);
}

int vo_sdk_enable_videolayer(int fd, struct vo_video_layer_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_ENABLE_VIDEOLAYER);
}

int vo_sdk_disable_videolayer(int fd, struct vo_video_layer_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_DISABLE_VIDEOLAYER);
}

int vo_sdk_set_layer_toleration(int fd, struct vo_layer_toleration_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SET_LAYERTOLERATION);
}

int vo_sdk_get_layer_toleration(int fd, struct vo_layer_toleration_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_LAYERTOLERATION);
}

int vo_sdk_set_layer_priority(int fd, struct vo_layer_priority_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SET_LAYERPRRIORITY);
}

int vo_sdk_get_layer_priority(int fd, struct vo_layer_priority_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_LAYERPRRIORITY);
}

int vo_sdk_bind_layer(int fd, struct vo_video_layer_bind_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_BIND_LAYER);
}

int vo_sdk_unbind_layer(int fd, struct vo_video_layer_bind_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_UNBIND_LAYER);
}

int vo_sdk_get_screen_frame(int fd, struct vo_screen_frame *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_SCREENFRAME);
}

int vo_sdk_release_screen_frame(int fd, struct vo_screen_frame *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_RELEASE_SCREENFRAME);
}

int vo_sdk_set_chnattr(int fd, struct vo_chn_attr_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SET_CHNATTR);
}

int vo_sdk_get_chnattr(int fd, struct vo_chn_attr_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_CHNATTR);
}

int vo_sdk_set_chnparam(int fd, struct vo_chn_param_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SET_CHNPARAM);
}

int vo_sdk_get_chnparam(int fd, struct vo_chn_param_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_CHNPARAM);
}

int vo_sdk_set_chnzoom(int fd, struct vo_chn_zoom_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SET_CHNZOOM);
}

int vo_sdk_get_chnzoom(int fd, struct vo_chn_zoom_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_CHNZOOM);
}

int vo_sdk_set_chnborder(int fd, struct vo_chn_border_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SET_CHNBORDER);
}

int vo_sdk_get_chnborder(int fd, struct vo_chn_border_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_CHNBORDER);
}

int vo_sdk_set_chnmirror(int fd, struct vo_chn_mirror_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SET_CHNMIRROR);
}

int vo_sdk_get_chnmirror(int fd, struct vo_chn_mirror_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_CHNMIRROR);
}

int vo_sdk_set_chn_frmrate(int fd, struct vo_chn_frmrate_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SET_CHNFRAMERATE);
}

int vo_sdk_get_chn_frmrate(int fd, struct vo_chn_frmrate_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_CHNFRAMERATE);
}

int vo_sdk_get_chn_frame(int fd, struct vo_chn_frame_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_CHNFRAME);
}

int vo_sdk_release_chn_frame(int fd, struct vo_chn_frame_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_RELEASE_CHNFRAME);
}

int vo_sdk_get_chn_pts(int fd, struct vo_chn_pts_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_CHNPTS);
}

int vo_sdk_get_chn_status(int fd, struct vo_chn_status_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_CHNSTATUS);
}

int vo_sdk_set_chn_threshold(int fd, struct vo_chn_threshold_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SET_CHNTHRESHOLD);
}

int vo_sdk_get_chn_threshold(int fd, struct vo_chn_threshold_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_CHNTHRESHOLD);
}

int vo_sdk_set_chnrotation(int fd, struct vo_chn_rotation_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SET_CHNROTATION);
}

int vo_sdk_get_chnrotation(int fd, struct vo_chn_rotation_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_CHNROTATION);
}

int vo_sdk_enable_chn(int fd, struct vo_chn_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_ENABLE_CHN);
}

int vo_sdk_disable_chn(int fd, struct vo_chn_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_DISABLE_CHN);
}

int vo_sdk_enable(int fd, struct vo_dev_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_ENABLE);
}

int vo_sdk_disable(int fd, struct vo_dev_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_DISABLE);
}

int vo_sdk_isenable(int fd, struct vo_dev_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_ISENABLE);
}

int vo_sdk_showchn(int fd, struct vo_chn_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SHOW_CHN);
}

int vo_sdk_hidechn(int fd, struct vo_chn_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_HIDE_CHN);
}

int vo_sdk_pausechn(int fd, struct vo_chn_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_PAUSE_CHN);
}

int vo_sdk_stepchn(int fd, struct vo_chn_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_STEP_CHN);
}

int vo_sdk_refreshchn(int fd, struct vo_chn_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_REFRESH_CHN);
}

int vo_sdk_resumechn(int fd, struct vo_chn_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_RESUME_CHN);
}

int vo_sdk_set_wbcsrc(int fd, struct vo_wbc_src_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SET_WBCSRC);
}

int vo_sdk_get_wbcsrc(int fd, struct vo_wbc_src_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_WBCSRC);
}

int vo_sdk_enable_wbc(int fd, struct vo_wbc_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_ENABLE_WBC);
}

int vo_sdk_disable_wbc(int fd, struct vo_wbc_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_DISABLE_WBC);
}

int vo_sdk_set_wbcattr(int fd, struct vo_wbc_attr_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SET_WBCATTR);
}

int vo_sdk_get_wbcattr(int fd, struct vo_wbc_attr_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_WBCATTR);
}

int vo_sdk_set_wbcmode(int fd, struct vo_wbc_mode_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SET_WBCMODE);
}

int vo_sdk_get_wbcmode(int fd, struct vo_wbc_mode_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_WBCMODE);
}

int vo_sdk_set_wbcdepth(int fd, struct vo_wbc_depth_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_SET_WBCDEPTH);
}

int vo_sdk_get_wbcdepth(int fd, struct vo_wbc_depth_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_WBCDEPTH);
}

int vo_sdk_get_wbcframe(int fd, struct vo_wbc_frame_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_GET_WBCFRAME);
}

int vo_sdk_release_wbcframe(int fd, struct vo_wbc_frame_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_RELEASE_WBCFRAME);
}

int vo_sdk_attach_layer_vb_pool(int fd, struct vo_layer_vb_pool_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_ATTACH_LAYER_VBPOOL);
}

int vo_sdk_detach_layer_vb_pool(int fd, struct vo_layer_vb_pool_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_DETACH_LAYER_VBPOOL);
}

int vo_sdk_attach_wbc_vb_pool(int fd, struct vo_wbc_vb_pool_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_ATTACH_WBC_VBPOOL);
}

int vo_sdk_detach_wbc_vb_pool(int fd, struct vo_wbc_vb_pool_cfg *cfg)
{
	VO_SDK_CTRL_PTR(fd, cfg, VO_IOCTL_SDK_CTRL, VO_SDK_DETACH_WBC_VBPOOL);
}

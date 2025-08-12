#ifndef __U_VO_UAPI_H__
#define __U_VO_UAPI_H__

#include <cvi_comm_vo.h>
#ifdef __cplusplus
	extern "C" {
#endif

#define VO_IOC_MAGIC	'o'
#define VO_IOC_BASE	0x20

#define VO_IOC_G_CTRL		_IOWR(VO_IOC_MAGIC, VO_IOC_BASE, struct vo_ext_control)
#define VO_IOC_S_CTRL		_IOWR(VO_IOC_MAGIC, VO_IOC_BASE + 1, struct vo_ext_control)

enum VO_IOCTL_CMD {
	//VO_IOC_S_CTRL
	VO_IOCTL_BASE = 0,
	VO_IOCTL_VB_DONE,
	VO_IOCTL_INTR,
	VO_IOCTL_OUT_CSC,
	VO_IOCTL_PATTERN,
	VO_IOCTL_FRAME_BGCOLOR,
	VO_IOCTL_WINDOW_BGCOLOR,
	VO_IOCTL_ONLINE,
	VO_IOCTL_INTF,
	VO_IOCTL_ENABLE_WIN_BGCOLOR,
	VO_IOCTL_SET_ALIGN = 10,
	VO_IOCTL_SET_RGN,
	VO_IOCTL_SET_CUSTOM_CSC,
	VO_IOCTL_SET_CLK,
	VO_IOCTL_GAMMA_LUT_UPDATE,

	//VO_IOC_G_CTRL
	VO_IOCTL_GET_VLAYER_SIZE,
	VO_IOCTL_GET_INTF_TYPE,
	VO_IOCTL_GAMMA_LUT_READ = 20,

	//VO_IOC_S_SELECTION
	VO_IOCTL_SEL_TGT_COMPOSE,
	VO_IOCTL_SEL_TGT_CROP,

	//VO_IOC_S_DV_TIMINGS
	VO_IOCTL_SET_DV_TIMINGS,
	VO_IOCTL_GET_DV_TIMINGS,
	VO_IOCTL_SET_FMT,

	VO_IOCTL_START_STREAMING,
	VO_IOCTL_STOP_STREAMING,
	VO_IOCTL_ENQ_WAITQ,
	VO_IOCTL_SDK_CTRL,
	VO_IOCTL_MAX,
};

enum VO_SDK_CTRL {
	//DEV CTRL
	VO_SDK_SET_PUBATTR,
	VO_SDK_GET_PUBATTR,
	VO_SDK_SET_HDMIPARAM,
	VO_SDK_GET_HDMIPARAM,
	VO_SDK_SET_LVDSPARAM,
	VO_SDK_GET_LVDSPARAM,
	VO_SDK_ENABLE,
	VO_SDK_DISABLE,
	VO_SDK_SUSPEND,
	VO_SDK_RESUME,
	VO_SDK_ISENABLE,
	VO_SDK_GET_PANELSTATUE,
	//LAYER CTRL
	VO_SDK_SET_VIDEOLAYERATTR,
	VO_SDK_GET_VIDEOLAYERATTR,
	VO_SDK_ENABLE_VIDEOLAYER,
	VO_SDK_DISABLE_VIDEOLAYER,
	VO_SDK_SET_DISPLAYBUFLEN,
	VO_SDK_GET_DISPLAYBUFLEN,
	VO_SDK_GET_SCREENFRAME,
	VO_SDK_RELEASE_SCREENFRAME,
	VO_SDK_SET_LAYER_PROC_AMP,
	VO_SDK_GET_LAYER_PROC_AMP,
	VO_SDK_SET_LAYERCSC,
	VO_SDK_GET_LAYERCSC,
	VO_SDK_SET_LAYERTOLERATION,
	VO_SDK_GET_LAYERTOLERATION,
	VO_SDK_SET_LAYERPRRIORITY,
	VO_SDK_GET_LAYERPRRIORITY,
	VO_SDK_BIND_LAYER,
	VO_SDK_UNBIND_LAYER,
	VO_SDK_ATTACH_LAYER_VBPOOL,
	VO_SDK_DETACH_LAYER_VBPOOL,
	//CHN CTRL
	VO_SDK_SET_CHNATTR,
	VO_SDK_GET_CHNATTR,
	VO_SDK_ENABLE_CHN,
	VO_SDK_DISABLE_CHN,
	VO_SDK_SET_CHNPARAM,
	VO_SDK_GET_CHNPARAM,
	VO_SDK_SET_CHNZOOM,
	VO_SDK_GET_CHNZOOM,
	VO_SDK_SET_CHNBORDER,
	VO_SDK_GET_CHNBORDER,
	VO_SDK_SET_CHNMIRROR,
	VO_SDK_GET_CHNMIRROR,
	VO_SDK_SET_CHNROTATION,
	VO_SDK_GET_CHNROTATION,
	VO_SDK_GET_CHNFRAME,
	VO_SDK_RELEASE_CHNFRAME,
	VO_SDK_SET_CHNFRAMERATE,
	VO_SDK_GET_CHNFRAMERATE,
	VO_SDK_GET_CHNPTS,
	VO_SDK_GET_CHNSTATUS,
	VO_SDK_SET_CHNTHRESHOLD,
	VO_SDK_GET_CHNTHRESHOLD,
	VO_SDK_SHOW_CHN,
	VO_SDK_HIDE_CHN,
	VO_SDK_RESUME_CHN,
	VO_SDK_PAUSE_CHN,
	VO_SDK_STEP_CHN,
	VO_SDK_REFRESH_CHN,
	VO_SDK_SEND_FRAME,
	VO_SDK_CLEAR_CHNBUF,
	//WBC CTRL
	VO_SDK_SET_WBCSRC,
	VO_SDK_GET_WBCSRC,
	VO_SDK_ENABLE_WBC,
	VO_SDK_DISABLE_WBC,
	VO_SDK_SET_WBCATTR,
	VO_SDK_GET_WBCATTR,
	VO_SDK_SET_WBCMODE,
	VO_SDK_GET_WBCMODE,
	VO_SDK_SET_WBCDEPTH,
	VO_SDK_GET_WBCDEPTH,
	VO_SDK_GET_WBCFRAME,
	VO_SDK_RELEASE_WBCFRAME,
	VO_SDK_ATTACH_WBC_VBPOOL,
	VO_SDK_DETACH_WBC_VBPOOL,
	VO_SDK_SET_BTPARAM,
	VO_SDK_GET_BTPARAM,
};

struct vo_ext_control {
	CVI_U32 id;
	CVI_U32 sdk_id;
	CVI_U32 size;
	CVI_U32 reserved[1];
	union {
		CVI_S32 value;
		CVI_S64 value64;
		void *ptr;
	};
};

struct vo_capability {
	CVI_U8 driver[16];
	CVI_U8 card[32];
	CVI_U8 bus_info[32];
	CVI_U32 version;
	CVI_U32 capabilities;
	CVI_U32 vdevice_caps;
	CVI_U32 reserved[3];
};

struct vo_bt_timings {
	CVI_U32 width;
	CVI_U32 height;
	CVI_U32 interlaced;
	CVI_U32 polarities;
	CVI_U64 pixelclock;
	CVI_U32 hfrontporch;
	CVI_U32 hsync;
	CVI_U32 hbackporch;
	CVI_U32 vfrontporch;
	CVI_U32 vsync;
	CVI_U32 vbackporch;
	CVI_U32 il_vfrontporch;
	CVI_U32 il_vsync;
	CVI_U32 il_vbackporch;
	CVI_U32 standards;
	CVI_U32 flags;
	CVI_U32 reservedd[14];
};

struct vo_dv_timings {
	CVI_U32 type;
	struct vo_bt_timings bt;
};

struct vo_rect {
	CVI_U32 left;
	CVI_U32 top;
	CVI_U32 width;
	CVI_U32 height;
};

struct disp_csc_matrix {
	CVI_U16 coef[3][3];
	CVI_U8 sub[3];
	CVI_U8 add[3];
};

//vo sdk layer config
struct vo_video_layer_cfg {
	CVI_U8 VoLayer;
};

struct vo_video_layer_bind_cfg {
	CVI_U8 VoDev;
	CVI_U8 VoLayer;
};

struct vo_video_layer_attr_cfg {
	CVI_U8 VoLayer;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
};

struct vo_layer_proc_amp_cfg {
	CVI_U8 VoLayer;
	CVI_S32 proc_amp[PROC_AMP_MAX];
};

struct vo_layer_csc_cfg {
	CVI_U8 VoLayer;
	VO_CSC_S stVideoCSC;
};

struct vo_layer_toleration_cfg {
	CVI_U8 VoLayer;
	CVI_U32 u32Toleration;
};

struct vo_layer_priority_cfg {
	CVI_U8 VoLayer;
	CVI_U32 u32Priority;
};

struct vo_clear_chn_buf_cfg {
	CVI_U8 VoLayer;
	CVI_U8 VoChn;
	CVI_BOOL bClrAll;
};

struct vo_snd_frm_cfg {
	CVI_U8 VoLayer;
	CVI_U8 VoChn;
	VIDEO_FRAME_INFO_S stVideoFrame;
	CVI_S32 s32MilliSec;
};

struct vo_display_buflen_cfg {
	CVI_U8 VoLayer;
	CVI_U32 u32BufLen;
};

struct vo_chn_attr_cfg {
	CVI_U8 VoLayer;
	CVI_U8 VoChn;
	VO_CHN_ATTR_S stChnAttr;
};

struct vo_chn_param_cfg {
	CVI_U8 VoLayer;
	CVI_U8 VoChn;
	VO_CHN_PARAM_S stChnParam;
};

struct vo_chn_zoom_cfg {
	CVI_U8 VoLayer;
	CVI_U8 VoChn;
	VO_CHN_ZOOM_ATTR_S stChnZoomAttr;
};

struct vo_chn_border_cfg {
	CVI_U8 VoLayer;
	CVI_U8 VoChn;
	VO_CHN_BORDER_ATTR_S stChnBorder;
};

struct vo_chn_mirror_cfg {
	CVI_U8 VoLayer;
	CVI_U8 VoChn;
	VO_CHN_MIRROR_TYPE enChnMirror;
};

struct vo_chn_cfg {
	CVI_U8 VoLayer;
	CVI_U8 VoChn;
};

struct vo_chn_frame_cfg {
	CVI_U8 VoLayer;
	CVI_U8 VoChn;
	VIDEO_FRAME_INFO_S stVideoFrame;
	CVI_S32 s32MilliSec;
};

struct vo_chn_frmrate_cfg {
	CVI_U8 VoLayer;
	CVI_U8 VoChn;
	CVI_U32 u32FrameRate;
};

struct vo_chn_pts_cfg {
	CVI_U8 VoLayer;
	CVI_U8 VoChn;
	CVI_U64 u64ChnPTS;
};

struct vo_chn_status_cfg {
	CVI_U8 VoLayer;
	CVI_U8 VoChn;
	VO_QUERY_STATUS_S stStatus;
};

struct vo_chn_threshold_cfg {
	CVI_U8 VoLayer;
	CVI_U8 VoChn;
	CVI_U32 u32Threshold;
};

struct vo_chn_rotation_cfg {
	CVI_U8 VoLayer;
	CVI_U8 VoChn;
	ROTATION_E enRotation;
};

struct vo_panel_status_cfg {
	CVI_U8 VoLayer;
	CVI_U8 VoChn;
	CVI_U32 is_init;
};

struct vo_dev_cfg {
	CVI_U8 VoDev;
	CVI_U8 isEnable;
};

struct vo_pub_attr_cfg {
	CVI_U8 VoDev;
	VO_PUB_ATTR_S stPubAttr;
};

struct vo_lvds_param_cfg {
	CVI_U8 VoDev;
	VO_LVDS_ATTR_S stLVDSParam;
};

struct vo_bt_param_cfg {
	CVI_U8 VoDev;
	VO_BT_ATTR_S stBTParam;
};

struct vo_hdmi_param_cfg {
	CVI_U8 VoDev;
	VO_HDMI_PARAM_S stHDMIParam;
};

struct vo_screen_frame {
	CVI_U8 VoLayer;
	VIDEO_FRAME_INFO_S stVideoFrame;
	CVI_S32 s32MilliSec;
};

struct vo_wbc_src_cfg {
	CVI_U8 VoWbc;
	VO_WBC_SRC_S stWbcSrc;
};

struct vo_wbc_cfg {
	CVI_U8 VoWbc;
};

struct vo_wbc_attr_cfg {
	CVI_U8 VoWbc;
	VO_WBC_ATTR_S stWbcAttr;
};

struct vo_wbc_mode_cfg {
	CVI_U8 VoWbc;
	VO_WBC_MODE_E enWbcMode;
};

struct vo_wbc_depth_cfg {
	CVI_U8 VoWbc;
	CVI_U32 u32Depth;
};

struct vo_wbc_frame_cfg {
	CVI_U8 VoWbc;
	VIDEO_FRAME_INFO_S stVideoFrame;
	CVI_S32 s32MilliSec;
};

struct vo_gamma_info_cfg {
	VO_GAMMA_INFO_S *pinfo;
};

struct vo_layer_vb_pool_cfg {
	CVI_U8 VoLayer;
	VB_POOL u32VbPool;
};

struct vo_wbc_vb_pool_cfg {
	CVI_U8 VoWbc;
	VB_POOL u32VbPool;
};

#ifdef __cplusplus
	}
#endif

#endif /* __U_VO_UAPI_H__ */


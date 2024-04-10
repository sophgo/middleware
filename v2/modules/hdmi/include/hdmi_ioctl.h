#ifndef MODULES_HDMI_IOCTL_H_
#define MODULES_HDMI_IOCTL_H_

#include <sys/ioctl.h>
#include <linux/hdmi_uapi.h>

/* Configured from user  */

CVI_S32 _cvi_hdmi_init(CVI_S32 fd);
CVI_S32 _cvi_hdmi_deinit(CVI_S32 fd);
CVI_S32 _cvi_hdmi_open(CVI_S32 fd);
CVI_S32 _cvi_hdmi_close(CVI_S32 fd);
CVI_S32 _cvi_hdmi_get_sink_capability(CVI_S32 fd, CVI_HDMI_SINK_CAPABILITY* capability);
CVI_S32 _cvi_hdmi_set_attr(CVI_S32 fd, const CVI_HDMI_ATTR* attr);
CVI_S32 _cvi_hdmi_get_attr(CVI_S32 fd, CVI_HDMI_ATTR* attr);
CVI_S32 _cvi_hdmi_start(CVI_S32 fd);
CVI_S32 _cvi_hdmi_stop(CVI_S32 fd);
CVI_S32 _cvi_hdmi_force_get_edid(CVI_S32 fd, CVI_HDMI_EDID* edid_data);
CVI_S32 _cvi_hdmi_get_event_id(CVI_S32 fd, CVI_U32* event_id);
CVI_S32 _cvi_hdmi_unregister_callback(CVI_S32 fd);
CVI_S32 _cvi_hdmi_set_infoframe(CVI_S32 fd, const CVI_HDMI_INFOFRAME* infoframe);
CVI_S32 _cvi_hdmi_get_infoframe(CVI_S32 fd, CVI_HDMI_INFOFRAME* infoframe);
CVI_S32 _cvi_hdmi_set_hw_spec(CVI_S32 fd, const CVI_HDMI_HW_SPEC* hw_spec);
CVI_S32 _cvi_hdmi_get_hw_spec(CVI_S32 fd, CVI_HDMI_HW_SPEC* hw_spec);
CVI_S32 _cvi_hdmi_set_avmute(CVI_S32 fd,  const CVI_BOOL* avmute_en);
CVI_S32 _cvi_hdmi_set_audio_mute(CVI_S32 fd, const CVI_BOOL* audio_mute_en);

#endif /* MODULES_HDMI_IOCTL_H_*/

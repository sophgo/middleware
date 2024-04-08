#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include <fcntl.h> /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "cvi_sys.h"
#include <linux/hdmi_uapi.h>
#include "hdmi_ioctl.h"

CVI_S32 _cvi_hdmi_init(CVI_S32 fd)
{
    return ioctl(fd, CVI_HDMI_INIT, NULL);
}

CVI_S32 _cvi_hdmi_deinit(CVI_S32 fd)
{
    return ioctl(fd, CVI_HDMI_DEINIT, NULL);
}

CVI_S32 _cvi_hdmi_open(CVI_S32 fd)
{
    return ioctl(fd, CVI_HDMI_OPEN, NULL);
}

CVI_S32 _cvi_hdmi_close(CVI_S32 fd)
{
    return ioctl(fd, CVI_HDMI_CLOSE, NULL);
}

CVI_S32 _cvi_hdmi_get_sink_capability(CVI_S32 fd, CVI_HDMI_SINK_CAPABILITY* capability)
{
    return ioctl(fd, CVI_HDMI_GET_SINK_CAPABILITY, capability);
}

CVI_S32 _cvi_hdmi_set_attr(CVI_S32 fd, const CVI_HDMI_ATTR* attr)
{
    return ioctl(fd, CVI_HDMI_SET_ATTR, attr);
}

CVI_S32 _cvi_hdmi_get_attr(CVI_S32 fd, CVI_HDMI_ATTR* attr)
{
    return ioctl(fd, CVI_HDMI_GET_ATTR, attr);
}

CVI_S32 _cvi_hdmi_start(CVI_S32 fd)
{
    return ioctl(fd, CVI_HDMI_START, NULL);
}

CVI_S32 _cvi_hdmi_stop(CVI_S32 fd)
{
    return ioctl(fd, CVI_HDMI_STOP, NULL);
}

CVI_S32 _cvi_hdmi_force_get_edid(CVI_S32 fd, CVI_HDMI_EDID* edid_data)
{
    return ioctl(fd, CVI_HDMI_FORCE_GET_EDID, edid_data);
}

CVI_S32 _cvi_hdmi_get_event_id(CVI_S32 fd, CVI_U32* event_id)
{
    return ioctl(fd, CVI_HDMI_GET_EVENT_ID, event_id);
}

CVI_S32 _cvi_hdmi_unregister_callback(CVI_S32 fd)
{
    return ioctl(fd, CVI_HDMI_UNREGISTER_CALLBACK, NULL);
}

CVI_S32 _cvi_hdmi_set_infoframe(CVI_S32 fd, const CVI_HDMI_INFOFRAME* infoframe)
{
    return ioctl(fd, CVI_HDMI_SET_INFOFRAME, infoframe);
}

CVI_S32 _cvi_hdmi_get_infoframe(CVI_S32 fd, CVI_HDMI_INFOFRAME* infoframe)
{
    return ioctl(fd, CVI_HDMI_GET_INFOFRAME, infoframe);
}

CVI_S32 _cvi_hdmi_set_hw_spec(CVI_S32 fd, const CVI_HDMI_HW_SPEC* hw_spec)
{
    return ioctl(fd, CVI_HDMI_SET_HW_SPEC, hw_spec);
}

CVI_S32 _cvi_hdmi_get_hw_spec(CVI_S32 fd, CVI_HDMI_HW_SPEC* hw_spec)
{
    return ioctl(fd, CVI_HDMI_GET_HW_SPEC, hw_spec);
}

CVI_S32 _cvi_hdmi_set_avmute(CVI_S32 fd,  const CVI_BOOL* avmute_en)
{
    return ioctl(fd, CVI_HDMI_SET_AVMUTE, avmute_en);
}

CVI_S32 _cvi_hdmi_set_audio_mute(CVI_S32 fd, const CVI_BOOL* audio_mute_en)
{
    return ioctl(fd, CVI_HDMI_SET_AUDIO_MUTE, audio_mute_en);
}
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include <fcntl.h> /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "cvi_base.h"
#include "cvi_sys.h"
#include "cvi_hdmi.h"
#include "hdmi_ioctl.h"

const CVI_HDMI_CALLBACK_FUNC* _CallBack_func;
static CVI_S32 hdmi_fd = -1;
static pthread_mutex_t hdmi_fd_lock = PTHREAD_MUTEX_INITIALIZER;


CVI_S32 get_hdmi_fd(CVI_VOID)
{
	pthread_mutex_lock(&hdmi_fd_lock);
	if (hdmi_fd <= 0) {
		if (open_device(HDMI_DEV_NAME, &hdmi_fd) == -1) {
			perror("HDMI open fail\n");
			hdmi_fd = -1;
		}
	}
	pthread_mutex_unlock(&hdmi_fd_lock);

	return hdmi_fd;
}

CVI_S32 CVI_HDMI_Init(CVI_VOID)
{
    CVI_S32 s32Ret;
    CVI_S32 fd = get_hdmi_fd();
    s32Ret =  _cvi_hdmi_init(fd);

    if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_HDMI(CVI_DBG_ERR, "HDMI init failed\n");
		return s32Ret;
	}

    return CVI_SUCCESS;
}

CVI_S32 CVI_HDMI_DeInit(CVI_VOID)
{
    CVI_S32 s32Ret;
    CVI_S32 fd = get_hdmi_fd();

    s32Ret =  _cvi_hdmi_deinit(fd);
    if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_HDMI(CVI_DBG_ERR, "HDMI deinit failed\n");
		return s32Ret;
	}
     return CVI_SUCCESS;
}

CVI_S32 CVI_HDMI_Open(CVI_VOID)
{
    CVI_S32 s32Ret;
    CVI_S32 fd = get_hdmi_fd();
    s32Ret = _cvi_hdmi_open(fd);
     if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_HDMI(CVI_DBG_ERR, "HDMI open failed\n");
		return s32Ret;
	}
    return CVI_SUCCESS;
}

CVI_S32 CVI_HDMI_Close(CVI_VOID)
{
    CVI_S32 s32Ret;
    CVI_S32 fd = get_hdmi_fd();

    s32Ret =  _cvi_hdmi_close(fd);
    if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_HDMI(CVI_DBG_ERR, "HDMI close failed\n");
		return s32Ret;
	}

    return CVI_SUCCESS;
}

CVI_S32 CVI_HDMI_GetSinkCapability(CVI_HDMI_SINK_CAPABILITY* capability)
{
    CVI_S32 s32Ret;
    CVI_S32 fd = get_hdmi_fd();

    s32Ret = _cvi_hdmi_get_sink_capability(fd, capability);
     if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_HDMI(CVI_DBG_ERR, "HDMI get sink capability failed\n");
		return s32Ret;
	}

    return CVI_SUCCESS;
}

CVI_S32 CVI_HDMI_SetAttr(const CVI_HDMI_ATTR* attr)
{
    CVI_S32 s32Ret;
    CVI_S32 fd = get_hdmi_fd();

    s32Ret =  _cvi_hdmi_set_attr(fd, attr);
    if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_HDMI(CVI_DBG_ERR, "HDMI set attr failed\n");
		return s32Ret;
	}

    return CVI_SUCCESS;
}

CVI_S32 CVI_HDMI_GetAttr(CVI_HDMI_ATTR* attr)
{
    CVI_S32 s32Ret;
    CVI_S32 fd = get_hdmi_fd();

    s32Ret = _cvi_hdmi_get_attr(fd, attr);
    if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_HDMI(CVI_DBG_ERR, "HDMI get attr failed\n");
		return s32Ret;
	}

    return CVI_SUCCESS;
}

CVI_S32 CVI_HDMI_Start(CVI_VOID)
{
    CVI_S32 s32Ret;
    CVI_S32 fd = get_hdmi_fd();

    s32Ret =  _cvi_hdmi_start(fd);
    if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_HDMI(CVI_DBG_ERR, "HDMI start failed\n");
		return s32Ret;
	}

    return CVI_SUCCESS;
}

CVI_S32 CVI_HDMI_Stop(CVI_VOID)
{
    CVI_S32 s32Ret;
    CVI_S32 fd = get_hdmi_fd();

    s32Ret =  _cvi_hdmi_stop(fd);
    if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_HDMI(CVI_DBG_ERR, "HDMI stop failed\n");
		return s32Ret;
	}

    return CVI_SUCCESS;
}

CVI_S32 CVI_HDMI_ForceGetEdid(CVI_HDMI_EDID* edid_data)
{
    CVI_S32 s32Ret;
    CVI_S32 fd = get_hdmi_fd();

    s32Ret =  _cvi_hdmi_force_get_edid(fd, edid_data);
    if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_HDMI(CVI_DBG_ERR, "HDMI force get edid failed\n");
		return s32Ret;
	}

    return CVI_SUCCESS;
}

static void CVI_HDMI_SigHandle(int sigNum)
{
    (void)sigNum;
    CVI_S32 s32Ret;
    CVI_U32 event_id;
    CVI_S32 fd = get_hdmi_fd();

    s32Ret = _cvi_hdmi_get_event_id(fd, &event_id);
    if (s32Ret != CVI_SUCCESS) {
        CVI_TRACE_HDMI(CVI_DBG_ERR, "HDMI get event id failed\n");
    }

    if(_CallBack_func->hdmi_event_callback && _CallBack_func->private_data){
         _CallBack_func->hdmi_event_callback(event_id, _CallBack_func->private_data);
    } else {
        CVI_TRACE_HDMI(CVI_DBG_ERR, "HDMI_CallBack_Func or Private_Data is NULL\n");
    }

}

CVI_S32 CVI_HDMI_RegisterCallback(const CVI_HDMI_CALLBACK_FUNC* callback_func)
{
    CVI_S32 flags;
	CVI_S32 fd = get_hdmi_fd();

	if (fd < 0) {
		CVI_TRACE_HDMI(CVI_DBG_ERR, "CVI_HDMI_GetDevFd fail!\n");
		return -1;
	}

    _CallBack_func = callback_func;

    signal(SIGIO, CVI_HDMI_SigHandle);

	fcntl(fd, F_SETOWN, getpid());
	flags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, flags | FASYNC);

    return CVI_SUCCESS;
}

CVI_S32 CVI_HDMI_UnRegisterCallback(const CVI_HDMI_CALLBACK_FUNC* callback_func)
{
    CVI_S32 flags;
	CVI_S32 fd = get_hdmi_fd();

    if(!callback_func->hdmi_event_callback){
        CVI_TRACE_HDMI(CVI_DBG_ERR, "hdmi_event_callback is NULL! No need to Unregister\n");
        return -1;
    }

	if (fd < 0) {
		CVI_TRACE_HDMI(CVI_DBG_ERR, "CVI_HDMI_GetDevFd fail!\n");
		return -1;
	}

    signal(SIGIO, NULL);

	flags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, flags & (~FASYNC));

    return CVI_SUCCESS;
}

CVI_S32 CVI_HDMI_SetInfoFrame(const CVI_HDMI_INFOFRAME* infoframe)
{
    CVI_S32 s32Ret;
    CVI_S32 fd = get_hdmi_fd();

    s32Ret = _cvi_hdmi_set_infoframe(fd, infoframe);
    if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_HDMI(CVI_DBG_ERR, "HDMI set infoframe failed\n");
		return s32Ret;
	}

    return CVI_SUCCESS;
}

CVI_S32 CVI_HDMI_GetInfoFrame(CVI_HDMI_INFOFRAME* infoframe)
{
    CVI_S32 s32Ret;
    CVI_S32 fd = get_hdmi_fd();

    s32Ret =  _cvi_hdmi_get_infoframe(fd, infoframe);
    if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_HDMI(CVI_DBG_ERR, "HDMI get infoframe failed\n");
		return s32Ret;
	}

    return CVI_SUCCESS;
}

CVI_S32 CVI_HDMI_SetHwSpec(const CVI_HDMI_HW_SPEC* hw_spec)
{
    CVI_S32 s32Ret;
    CVI_S32 fd = get_hdmi_fd();

    s32Ret =  _cvi_hdmi_set_hw_spec(fd, hw_spec);
    if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_HDMI(CVI_DBG_ERR, "HDMI set hw spec failed\n");
		return s32Ret;
	}

    return CVI_SUCCESS;
}

CVI_S32 CVI_HDMI_GetHwSpec(CVI_HDMI_HW_SPEC* hw_spec)
{
    CVI_S32 s32Ret;
    CVI_S32 fd = get_hdmi_fd();

    s32Ret = _cvi_hdmi_get_hw_spec(fd, hw_spec);
    if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_HDMI(CVI_DBG_ERR, "HDMI get hw spec failed\n");
		return s32Ret;
	}

    return CVI_SUCCESS;
}

CVI_S32 CVI_HDMI_SetAvmute(const CVI_BOOL* aumute_en)
{
    CVI_S32 s32Ret;
    CVI_S32 fd = get_hdmi_fd();

    s32Ret =  _cvi_hdmi_set_avmute(fd, aumute_en);
    if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_HDMI(CVI_DBG_ERR, "HDMI set avmute failed\n");
		return s32Ret;
	}

    return CVI_SUCCESS;
}

CVI_S32 CVI_HDMI_SetAudioMute(const CVI_BOOL* audio_mute_en)
{
    CVI_S32 s32Ret;
    CVI_S32 fd = get_hdmi_fd();

    s32Ret = _cvi_hdmi_set_audio_mute(fd, audio_mute_en);
    if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_HDMI(CVI_DBG_ERR, "HDMI set audio mute failed\n");
		return s32Ret;
	}

    return CVI_SUCCESS;
}
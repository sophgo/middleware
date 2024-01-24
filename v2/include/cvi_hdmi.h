#ifndef _CVI_HDMI_H_
#define _CVI_HDMI_H_

#include <linux/cvi_common.h>
#include <linux/cvi_errno.h>
#include <linux/cvi_defines.h>
#include <linux/cvi_comm_hdmi.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

CVI_S32 CVI_HDMI_Init(CVI_VOID);
CVI_S32 CVI_HDMI_DeInit(CVI_VOID);
CVI_S32 CVI_HDMI_Open(CVI_VOID);
CVI_S32 CVI_HDMI_Close(CVI_VOID);
CVI_S32 CVI_HDMI_GetSinkCapability(CVI_HDMI_SINK_CAPABILITY* capability);
CVI_S32 CVI_HDMI_SetAttr(const CVI_HDMI_ATTR* attr);
CVI_S32 CVI_HDMI_GetAttr(CVI_HDMI_ATTR* attr);
CVI_S32 CVI_HDMI_Start(CVI_VOID);
CVI_S32 CVI_HDMI_Stop(CVI_VOID);
CVI_S32 CVI_HDMI_ForceGetEdid(CVI_HDMI_EDID* edid_data);
CVI_S32 CVI_HDMI_RegisterCallback(const CVI_HDMI_CALLBACK_FUNC* callback_func);
CVI_S32 CVI_HDMI_UnRegisterCallback(const CVI_HDMI_CALLBACK_FUNC* callback_func);
CVI_S32 CVI_HDMI_SetInfoFrame(const CVI_HDMI_INFOFRAME* infoframe);
CVI_S32 CVI_HDMI_GetInfoFrame(CVI_HDMI_INFOFRAME* infoframe);
CVI_S32 CVI_HDMI_SetHwSpec(const CVI_HDMI_HW_SPEC* hw_spec);
CVI_S32 CVI_HDMI_GetHwSpec(CVI_HDMI_HW_SPEC* hw_spec);
CVI_S32 CVI_HDMI_SetAvmute(const CVI_BOOL* aumute_en);
CVI_S32 CVI_HDMI_SetAudioMute(const CVI_BOOL* audio_mute_en);
CVI_S32 CVI_HDMI_SetModParam(const CVI_HDMI_MOD_PARAM* mod_param);
CVI_S32 CVI_HDMI_GetModParam(CVI_HDMI_MOD_PARAM* mod_param);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* _CVI_HDMI_H_ */

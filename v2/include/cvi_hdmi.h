#ifndef _CVI_HDMI_H_
#define _CVI_HDMI_H_

#include <cvi_common.h>
#include <cvi_errno.h>
#include <cvi_defines.h>
#include <cvi_comm_hdmi.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/**
 * @brief Init Hdmi_Tx.
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_HDMI_Init(CVI_VOID);

/**
 * @brief DeInit Hdmi_Tx.
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_HDMI_DeInit(CVI_VOID);

/**
 * @brief Open Hdmi_Tx.
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_HDMI_Open(CVI_VOID);

/**
 * @brief Close Hdmi_Tx.
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_HDMI_Close(CVI_VOID);

/**
 * @brief Get Sink's Capability.
 *
 * @param capability(Out), Sink's Capability.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_HDMI_GetSinkCapability(CVI_HDMI_SINK_CAPABILITY* capability);

/**
 * @brief Set Hdmi_Tx Attribute.
 *
 * @param attr(In), Hdmi_Tx Attribute.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_HDMI_SetAttr(const CVI_HDMI_ATTR* attr);

/**
 * @brief Get Hdmi_Tx Attribute.
 *
 * @param attr(Out), Hdmi_Tx Attribute.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_HDMI_GetAttr(CVI_HDMI_ATTR* attr);

/**
 * @brief Start Hdmi_Tx.
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_HDMI_Start(CVI_VOID);

/**
 * @brief Stop Hdmi_Tx.
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_HDMI_Stop(CVI_VOID);

/**
 * @brief Fore Get Hdmi Sink's Edid Data.
 *
 * @param edid_data(Out), Hdmi Sink's Edid Data.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_HDMI_ForceGetEdid(CVI_HDMI_EDID* edid_data);

/**
 * @brief Register Callback Function.
 *
 * @param callback_func(In), Callback Function Point.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_HDMI_RegisterCallback(const CVI_HDMI_CALLBACK_FUNC* callback_func);

/**
 * @brief UnRegister Callback Function.
 *
 * @param callback_func(In), Callback Function Point.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_HDMI_UnRegisterCallback(const CVI_HDMI_CALLBACK_FUNC* callback_func);

/**
 * @brief Set Hdmi_Tx Frame Info.
 *
 * @param infoframe(In), Hdmi_Tx Frame Info.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_HDMI_SetInfoFrame(const CVI_HDMI_INFOFRAME* infoframe);

/**
 * @brief Get Hdmi_Tx Frame Info.
 *
 * @param infoframe(Out), Hdmi_Tx Frame Info.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_HDMI_GetInfoFrame(CVI_HDMI_INFOFRAME* infoframe);

/**
 * @brief Set Hdmi_Tx Hardware Spec.
 *
 * @param hw_spec(In), Hdmi_Tx Hardware Spec.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_HDMI_SetHwSpec(const CVI_HDMI_HW_SPEC* hw_spec);

/**
 * @brief Get Hdmi_Tx Hardware Spec.
 *
 * @param hw_spec(Out), Hdmi_Tx Hardware Spec.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_HDMI_GetHwSpec(CVI_HDMI_HW_SPEC* hw_spec);

/**
 * @brief Set Hdmi_Tx Avmute Value.
 *
 * @param avmute_en(In), TRUE for mute, FALSE for unmute.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_HDMI_SetAvmute(const CVI_BOOL* avmute_en);

/**
 * @brief Set Hdmi_Tx Audio Mute Value.
 *
 * @param audio_mute_en(In), TRUE for mute, FALSE for unmute.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_HDMI_SetAudioMute(const CVI_BOOL* audio_mute_en);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* _CVI_HDMI_H_ */

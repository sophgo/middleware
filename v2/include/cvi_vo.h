/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_vo.h
 * Description:
 *   MMF Programe Interface for video output management moudle
 */

#ifndef __CVI_VO_H__
#define __CVI_VO_H__

#include <cvi_comm_vo.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/* Device Relative Settings */

/* CVI_VO_SetPubAttr: Configure VO public properties
 *
 * @param VoDev: Video output device
 * @param pstPubAttr: public properties
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetPubAttr(VO_DEV VoDev, const VO_PUB_ATTR_S *pstPubAttr);

/* CVI_VO_GetPubAttr: get VO public properties
 *
 * @param VoDev: Video output device
 * @param pstPubAttr: public properties
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetPubAttr(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr);

/* CVI_VO_SetHDMIParam: set hdmi csc param
 *
 * @param VoDev: Video output device
 * @param pstHDMIParam: hdmi csc param
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetHDMIParam(VO_DEV VoDev, const VO_HDMI_PARAM_S *pstHDMIParam);

/* CVI_VO_GetHDMIParam: get hdmi csc param
 *
 * @param VoDev: Video output device
 * @param pstHDMIParam: hdmi csc param
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetHDMIParam(VO_DEV VoDev, VO_HDMI_PARAM_S *pstHDMIParam);

/* CVI_VO_SetLVDSParam: set lvds param
 *
 * @param VoDev: Video output device
 * @param pstLVDSParam: lvds param
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetLVDSParam(VO_DEV VoDev, const VO_LVDS_ATTR_S *pstLVDSParam);

/* CVI_VO_GetLVDSParam: get lvds param
 *
 * @param VoDev: Video output device
 * @param pstLVDSParam: lvds param
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetLVDSParam(VO_DEV VoDev, VO_LVDS_ATTR_S *pstLVDSParam);

/* CVI_VO_SetBTParam: set bt param
 *
 * @param VoDev: Video output device
 * @param pstBTParam: bt param
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetBTParam(VO_DEV VoDev, const VO_BT_ATTR_S *pstBTParam);

/* CVI_VO_GetBTSParam: get bt param
 *
 * @param VoDev: Video output device
 * @param pstBTParam: bt param
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetBTParam(VO_DEV VoDev, VO_BT_ATTR_S *pstBTParam);

/* CVI_VO_IsEnabled: Check if VO is enabled
 *
 * @param VoDev: Video output device
 * @return: Is it enabled.
 */
CVI_BOOL CVI_VO_IsEnabled(VO_DEV VoDev);

/* CVI_VO_Enable: enable vo
 *
 * @param VoDev: Video output device
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_Enable(VO_DEV VoDev);

/* CVI_VO_Disable: disable vo
 *
 * @param VoDev: Video output device
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_Disable(VO_DEV VoDev);

/* CVI_VO_ShowPattern: show internal test patterns
 *
 * @param VoDev: Video output device
 * @param PatternId: pattern id
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_ShowPattern(VO_DEV VoDev, VO_PATTERN_MODE PatternId);

/* CVI_VO_CloseFd: close vo fd
 *
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_CloseFd(void);

/* Video Relative Settings */

/* CVI_VO_SetVideoLayerAttr: Set video layer properties
 *
 * @param VoLayer: Video layer
 * @param pstLayerAttr: layer properties
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetVideoLayerAttr(VO_LAYER VoLayer, const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr);

/* CVI_VO_GetVideoLayerAttr: Get video layer properties
 *
 * @param VoLayer: Video layer
 * @param pstLayerAttr: layer properties
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetVideoLayerAttr(VO_LAYER VoLayer, VO_VIDEO_LAYER_ATTR_S *pstLayerAttr);

/* CVI_VO_EnableVideoLayer: enable video layer
 *
 * @param VoLayer: Video layer
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_EnableVideoLayer(VO_LAYER VoLayer);

/* CVI_VO_DisableVideoLayer: disable video layer
 *
 * @param VoLayer: Video layer
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_DisableVideoLayer(VO_LAYER VoLayer);

/* CVI_VO_SetLayerPriority: set layer priority
 *
 * @param VoLayer: Graphic layer
 * @param u32Priority: priority
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetLayerPriority(VO_LAYER VoLayer, CVI_U32 u32Priority);

/* CVI_VO_SetLayerPriority: get layer priority
 *
 * @param VoLayer: Graphic layer
 * @param u32Priority: priority
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetLayerPriority(VO_LAYER VoLayer, CVI_U32 *pu32Priority);

/* CVI_VO_SetVideoLayerCSC: set layer csc matrix
 *
 * @param VoLayer: Video layer
 * @param pstVideoCSC: csc matrix
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetVideoLayerCSC(VO_LAYER VoLayer, const VO_CSC_S *pstVideoCSC);

/* CVI_VO_GetVideoLayerCSC: get layer csc matrix
 *
 * @param VoLayer: Video layer
 * @param pstVideoCSC: csc matrix
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetVideoLayerCSC(VO_LAYER VoLayer, VO_CSC_S *pstVideoCSC);

/* CVI_VO_GetLayerProcAmpCtrl: get layer proc ctrl parameters
 *
 * @param VoLayer: Video layer
 * @param type: Brightness, hue, saturation, contrast
 * @param ctrl: Control parameters
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetLayerProcAmpCtrl(VO_LAYER VoLayer, PROC_AMP_E type, PROC_AMP_CTRL_S *ctrl);

/* CVI_VO_GetLayerProcAmp: get layer proc value
 *
 * @param VoLayer: Video layer
 * @param type: Brightness, hue, saturation, contrast
 * @param value: value
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetLayerProcAmp(VO_LAYER VoLayer, PROC_AMP_E type, CVI_S32 *value);

/* CVI_VO_SetLayerProcAmp: set layer proc value
 *
 * @param VoLayer: Video layer
 * @param type: Brightness, hue, saturation, contrast
 * @param value: value
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetLayerProcAmp(VO_LAYER VoLayer, PROC_AMP_E type, CVI_S32 value);

/* CVI_VO_BindLayer: bind layer to dev
 *
 * @param VoLayer: Video/Graphic layer
 * @param VoDev: Video output device
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_BindLayer(VO_LAYER VoLayer, VO_DEV VoDev);

/* CVI_VO_UnBindLayer: unbind layer to dev
 *
 * @param VoLayer: Video/Graphic layer
 * @param VoDev: Video output device
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_UnBindLayer(VO_LAYER VoLayer, VO_DEV VoDev);

/* Display relative operations */

/* CVI_VO_SetPlayToleration: Set playback tolerance
 *
 * @param VoLayer: Video layer
 * @param u32Toleration: Play tolerance
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetPlayToleration(VO_LAYER VoLayer, CVI_U32 u32Toleration);

/* CVI_VO_GetPlayToleration: Get playback tolerance
 *
 * @param VoLayer: Video layer
 * @param u32Toleration: Play tolerance
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetPlayToleration(VO_LAYER VoLayer, CVI_U32 *pu32Toleration);

/* CVI_VO_GetScreenFrame: Get layer frame
 *
 * @param VoLayer: Video layer
 * @param pstVideoFrame: frame info
 * @param s32MilliSec: timeout
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetScreenFrame(VO_LAYER VoLayer, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec);

/* CVI_VO_ReleaseScreenFrame: release layer frame
 *
 * @param VoLayer: Video layer
 * @param pstVideoFrame: frame info
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_ReleaseScreenFrame(VO_LAYER VoLayer, const VIDEO_FRAME_INFO_S *pstVideoFrame);

/* CVI_VO_SetDisplayBufLen: set layer display buf depth
 *
 * @param VoLayer: Video layer
 * @param u32BufLen: How many VBs
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetDisplayBufLen(VO_LAYER VoLayer, CVI_U32 u32BufLen);

/* CVI_VO_GetDisplayBufLen: set layer display buf depth
 *
 * @param VoLayer: Video layer
 * @param u32BufLen: How many VBs
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetDisplayBufLen(VO_LAYER VoLayer, CVI_U32 *pu32BufLen);

/* Channel Relative Operations */

/* CVI_VO_SetChnAttr: set chn attr
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param pstChnAttr: chn attr
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetChnAttr(VO_LAYER VoLayer, VO_CHN VoChn, const VO_CHN_ATTR_S *pstChnAttr);

/* CVI_VO_GetChnAttr: get chn attr
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param pstChnAttr: chn attr
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetChnAttr(VO_LAYER VoLayer, VO_CHN VoChn, VO_CHN_ATTR_S *pstChnAttr);

/* CVI_VO_SetChnParam: set chn aspect ratio param
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param pstChnParam: aspect ratio param
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetChnParam(VO_LAYER VoLayer, VO_CHN VoChn, const VO_CHN_PARAM_S *pstChnParam);

/* CVI_VO_GetChnParam: get chn aspect ratio param
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param pstChnParam: aspect ratio param
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetChnParam(VO_LAYER VoLayer, VO_CHN VoChn, VO_CHN_PARAM_S *pstChnParam);

/* CVI_VO_SetChnZoomInWindow: set chn Local amplification parameters
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param pstChnZoomAttr: Local amplification parameters
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetChnZoomInWindow(VO_LAYER VoLayer, VO_CHN VoChn, const VO_CHN_ZOOM_ATTR_S *pstChnZoomAttr);

/* CVI_VO_GetChnZoomInWindow: get chn Local amplification parameters
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param pstChnZoomAttr: Local amplification parameters
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetChnZoomInWindow(VO_LAYER VoLayer, VO_CHN VoChn, VO_CHN_ZOOM_ATTR_S *pstChnZoomAttr);

/* CVI_VO_SetChnBorder: set chn Border Properties
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param pstChnBorder: Border Properties
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetChnBorder(VO_LAYER VoLayer, VO_CHN VoChn, const VO_CHN_BORDER_ATTR_S *pstChnBorder);

/* CVI_VO_GetChnBorder: get chn Border Properties
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param pstChnBorder: Border Properties
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetChnBorder(VO_LAYER VoLayer, VO_CHN VoChn, VO_CHN_BORDER_ATTR_S *pstChnBorder);

/* CVI_VO_SetChnMirror: set chn Mirror type
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param enChnMirror: Mirror type
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetChnMirror(VO_LAYER VoLayer, VO_CHN VoChn, VO_CHN_MIRROR_TYPE enChnMirror);

/* CVI_VO_GetChnMirror: get chn Mirror type
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param penChnMirror: Mirror type
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetChnMirror(VO_LAYER VoLayer, VO_CHN VoChn, VO_CHN_MIRROR_TYPE *penChnMirror);

/* CVI_VO_EnableChn: enable chn
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_EnableChn(VO_LAYER VoLayer, VO_CHN VoChn);

/* CVI_VO_DisableChn: disable chn
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_DisableChn(VO_LAYER VoLayer, VO_CHN VoChn);

/* CVI_VO_SetChnFrameRate: set chn framerate
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param s32ChnFrmRate: framerate
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetChnFrameRate(VO_LAYER VoLayer, VO_CHN VoChn, CVI_S32 s32ChnFrmRate);

/* CVI_VO_GetChnMirror: get chn framerate
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param ps32ChnFrmRate: framerate
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetChnFrameRate(VO_LAYER VoLayer, VO_CHN VoChn, CVI_S32 *ps32ChnFrmRate);

/* CVI_VO_GetChnFrame: Get chn frame
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param pstVideoFrame: frame info
 * @param s32MilliSec: timeout
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetChnFrame(VO_LAYER VoLayer, VO_CHN VoChn, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec);

/* CVI_VO_ReleaseChnFrame: release chn frame
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param pstVideoFrame: frame info
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_ReleaseChnFrame(VO_LAYER VoLayer, VO_CHN VoChn, const VIDEO_FRAME_INFO_S *pstVideoFrame);

/* CVI_VO_PauseChn: pause chn
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_PauseChn(VO_LAYER VoLayer, VO_CHN VoChn);

/* CVI_VO_PauseChn: step play chn
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_StepChn(VO_LAYER VoLayer, VO_CHN VoChn);

/* CVI_VO_PauseChn: refresh chn
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_RefreshChn(VO_LAYER VoLayer, VO_CHN VoChn);

/* CVI_VO_PauseChn: resume chn
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_ResumeChn(VO_LAYER VoLayer, VO_CHN VoChn);

/* CVI_VO_PauseChn: show chn
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_ShowChn(VO_LAYER VoLayer, VO_CHN VoChn);

/* CVI_VO_PauseChn: hide chn
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_HideChn(VO_LAYER VoLayer, VO_CHN VoChn);

/* CVI_VO_GetChnPTS: get chn pts
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param pu64ChnPTS: pts
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetChnPTS(VO_LAYER VoLayer, VO_CHN VoChn, CVI_U64 *pu64ChnPTS);

/* CVI_VO_QueryChnStatus: get chn status
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param pstStatus: status
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_QueryChnStatus(VO_LAYER VoLayer, VO_CHN VoChn, VO_QUERY_STATUS_S *pstStatus);

/* CVI_VO_SendFrame: send to chn frame
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param pstVideoFrame: frame info
 * @param s32MilliSec: timeout
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SendFrame(VO_LAYER VoLayer, VO_CHN VoChn, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec);

/* CVI_VO_ClearChnBuf: clear chn buffer
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param bClrAll: Do you want to clear all
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_ClearChnBuf(VO_LAYER VoLayer, VO_CHN VoChn, CVI_BOOL bClrAll);

/* CVI_VO_SetChnRecvThreshold: set chn Threshold
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param u32Threshold: Threshold
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetChnRecvThreshold(VO_LAYER VoLayer, VO_CHN VoChn, CVI_U32 u32Threshold);

/* CVI_VO_GetChnRecvThreshold: get chn Threshold
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param u32Threshold: Threshold
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetChnRecvThreshold(VO_LAYER VoLayer, VO_CHN VoChn, CVI_U32 *pu32Threshold);

/* CVI_VO_SetChnRotation: set chn Rotation type
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param enRotation: Rotation type
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetChnRotation(VO_LAYER VoLayer, VO_CHN VoChn, ROTATION_E enRotation);

/* CVI_VO_GetChnRotation: get chn Rotation type
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param penRotation: Rotation type
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetChnRotation(VO_LAYER VoLayer, VO_CHN VoChn, ROTATION_E *penRotation);

/* CVI_VO_SetGammaInfo: set gamma attr
 *
 * @param VO_GAMMA_INFO_S: gamma attr
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetGammaInfo(VO_GAMMA_INFO_S *pinfo);

/* CVI_VO_GetGammaInfo: get gamma attr
 *
 * @param VO_GAMMA_INFO_S: gamma attr
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetGammaInfo(VO_GAMMA_INFO_S *pinfo);

/* CVI_VO_SetWbcSrc: set wbc src
 *
 * @param VoWbc: wbc dev
 * @param pstWbcSrc: wbc src
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetWbcSrc(VO_WBC VoWbc, const VO_WBC_SRC_S *pstWbcSrc);

/* CVI_VO_GetWbcSrc: get wbc src
 *
 * @param VoWbc: wbc dev
 * @param pstWbcSrc: wbc src
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetWbcSrc(VO_WBC VoWbc, VO_WBC_SRC_S *pstWbcSrc);

/* CVI_VO_EnableWbc: enable wbc
 *
 * @param VoWbc: wbc dev
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_EnableWbc(VO_WBC VoWbc);

/* CVI_VO_DisableWbc: disable wbc
 *
 * @param VoWbc: wbc dev
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_DisableWbc(VO_WBC VoWbc);

/* CVI_VO_SetWbcAttr: set wbc attr
 *
 * @param VoWbc: wbc dev
 * @param pstWbcAttr: wbc attr
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetWbcAttr(VO_WBC VoWbc, const VO_WBC_ATTR_S *pstWbcAttr);

/* CVI_VO_GetWbcAttr: get wbc attr
 *
 * @param VoWbc: wbc dev
 * @param pstWbcAttr: wbc attr
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetWbcAttr(VO_WBC VoWbc, VO_WBC_ATTR_S *pstWbcAttr);

/* CVI_VO_SetWbcMode: set wbc mode
 *
 * @param VoWbc: wbc dev
 * @param enWbcMode: wbc mode
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetWbcMode(VO_WBC VoWbc, VO_WBC_MODE_E enWbcMode);

/* CVI_VO_GetWbcMode: get wbc mode
 *
 * @param VoWbc: wbc dev
 * @param penWbcMode: wbc mode
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetWbcMode(VO_WBC VoWbc, VO_WBC_MODE_E *penWbcMode);

/* CVI_VO_SetWbcDepth: set wbc doneq depth
 *
 * @param VoWbc: wbc dev
 * @param u32Depth: doneq depth
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetWbcDepth(VO_WBC VoWbc, CVI_U32 u32Depth);

/* CVI_VO_GetWbcDepth: get wbc doneq depth
 *
 * @param VoWbc: wbc dev
 * @param pu32Depth: doneq depth
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetWbcDepth(VO_WBC VoWbc, CVI_U32 *pu32Depth);

/* CVI_VO_GetWbcFrame: get wbc frame
 *
 * @param VoWbc: wbc dev
 * @param pstVideoFrame: frame info
 * @param s32MilliSec: timeout
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetWbcFrame(VO_WBC VoWbc, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec);

/* CVI_VO_ReleaseWbcFrame: release wbc frame
 *
 * @param VoWbc: wbc dev
 * @param pstVideoFrame: frame info
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_ReleaseWbcFrame(VO_WBC VoWbc, const VIDEO_FRAME_INFO_S *pstVideoFrame);

/* Module Parameter Settings */
CVI_S32 CVI_VO_Get_Panel_Status(VO_LAYER VoLayer, VO_CHN VoChn, CVI_U32 *is_init);
CVI_S32 CVI_VO_RegPmCallBack(VO_DEV VoDev, VO_PM_OPS_S *pstPmOps, void *pvData);
CVI_S32 CVI_VO_UnRegPmCallBack(VO_DEV VoDev);
CVI_S32 CVI_VO_Suspend(void);
CVI_S32 CVI_VO_Resume(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif // __CVI_VO_H__

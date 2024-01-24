/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_vpss.h
 * Description:
 *   MMF Programe Interface for video processing moudle
 */

#ifndef __CVI_VPSS_H__
#define __CVI_VPSS_H__

#include <linux/cvi_common.h>
#include <linux/cvi_comm_video.h>
#include <linux/cvi_comm_vpss.h>
#include <linux/cvi_comm_vb.h>
#include <linux/cvi_comm_gdc.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/****** Group Settings ******/

/**
 * @brief Create vpss group.
 *
 * @param VpssGrp(In), group ID.
 * @param pstGrpAttr(In), group attribute.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_CreateGrp(VPSS_GRP VpssGrp, const VPSS_GRP_ATTR_S *pstGrpAttr);

/**
 * @brief Destroy vpss group.
 *
 * @param VpssGrp(In), group ID.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_DestroyGrp(VPSS_GRP VpssGrp);

/**
 * @brief Get Available group ID.
 *
 * @return Return group ID.
 */
VPSS_GRP CVI_VPSS_GetAvailableGrp(void);

/**
 * @brief Start vpss group.
 *
 * @param VpssGrp(In), group ID.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_StartGrp(VPSS_GRP VpssGrp);

/**
 * @brief Stop vpss group.
 *
 * @param VpssGrp(In), group ID.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_StopGrp(VPSS_GRP VpssGrp);

/**
 * @brief Reset vpss group.
 *
 * @param VpssGrp(In), group ID.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_ResetGrp(VPSS_GRP VpssGrp);

/**
 * @brief Get vpss group attribute.
 *
 * @param VpssGrp(In), group ID.
 * @param pstGrpAttr(Out), group attribute.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_GetGrpAttr(VPSS_GRP VpssGrp, VPSS_GRP_ATTR_S *pstGrpAttr);

/**
 * @brief Set vpss group attribute.
 *
 * @param VpssGrp(In), group ID.
 * @param pstGrpAttr(In), group attribute.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_SetGrpAttr(VPSS_GRP VpssGrp, const VPSS_GRP_ATTR_S *pstGrpAttr);

/**
 * @brief Set crop info of vpss group.
 *
 * @param VpssGrp(In), group ID.
 * @param pstCropInfo(In), crop info.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_SetGrpCrop(VPSS_GRP VpssGrp, const VPSS_CROP_INFO_S *pstCropInfo);

/**
 * @brief Get crop info of vpss group.
 *
 * @param VpssGrp(In), group ID.
 * @param pstCropInfo(Out), crop info.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_GetGrpCrop(VPSS_GRP VpssGrp, VPSS_CROP_INFO_S *pstCropInfo);

/**
 * @brief Send frame to vpss group.
 *
 * @param VpssGrp(In), group ID.
 * @param pstVideoFrame(In), frame info.
 * @param s32MilliSec(In), timeouts.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_SendFrame(VPSS_GRP VpssGrp, const VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec);

/**
 * @brief Image quality control.
 *
 * @param VpssGrp(In), group ID.
 * @param type(In), brightness/contrast/saturation/hue.
 * @param ctrl(In), control info.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_GetGrpProcAmpCtrl(VPSS_GRP VpssGrp, PROC_AMP_E type, PROC_AMP_CTRL_S *ctrl);

/**
 * @brief Get the brightness/contrast/saturation/hue value.
 *
 * @param VpssGrp(In), group ID.
 * @param type(In), brightness/contrast/saturation/hue.
 * @param value(Out), value.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_GetGrpProcAmp(VPSS_GRP VpssGrp, PROC_AMP_E type, CVI_S32 *value);

/**
 * @brief Set the brightness/contrast/saturation/hue value.
 *
 * @param VpssGrp(In), group ID.
 * @param type(In), brightness/contrast/saturation/hue.
 * @param value(In), value.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_SetGrpProcAmp(VPSS_GRP VpssGrp, PROC_AMP_E type, CVI_S32 value);

/* Apply the settings of scene from bin
 *
 * @param VpssGrp: the vpss grp to apply
 * @param scene: the scene of settings stored in bin to use
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_SetGrpParamfromBin(VPSS_GRP VpssGrp, CVI_U8 scene);

/*for ISP tool get bin scene*/
CVI_S32 CVI_VPSS_GetBinScene(VPSS_GRP VpssGrp, CVI_U8 *scene);


/****** Chn Settings ******/

/**
 * @brief Set vpss channel attribute.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param pstChnAttr(In), channel attribute.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_SetChnAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CHN_ATTR_S *pstChnAttr);

/**
 * @brief Get vpss channel attribute.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param pstChnAttr(Out), channel attribute.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_GetChnAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CHN_ATTR_S *pstChnAttr);

/**
 * @brief Enable vpss channel.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_EnableChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn);

/**
 * @brief Disable vpss channel.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_DisableChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn);

/**
 * @brief Set crop info of vpss channel.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param pstCropInfo(In), crop info.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_SetChnCrop(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CROP_INFO_S *pstCropInfo);

/**
 * @brief Get crop info of vpss channel.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param pstCropInfo(Out), crop info.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_GetChnCrop(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CROP_INFO_S *pstCropInfo);

/**
 * @brief Set vpss channel rotation.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param enRotation(In), Rotation 0/90/180/270.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_SetChnRotation(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, ROTATION_E enRotation);

/**
 * @brief Get vpss channel rotation.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param enRotation(Out), Rotation value.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_GetChnRotation(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, ROTATION_E *penRotation);

/**
 * @brief Set vpss channel LDC attribute.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param pstLDCAttr(In), LDC attribute.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_SetChnLDCAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_LDC_ATTR_S *pstLDCAttr);

/**
 * @brief Get vpss channel LDC attribute.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param pstLDCAttr(Out), LDC attribute.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_GetChnLDCAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_LDC_ATTR_S *pstLDCAttr);

/**
 * @brief Set vpss channel FishEye attribute.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param pstFishEyeAttr(In), FishEye attribute.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_SetChnFisheye(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const FISHEYE_ATTR_S *pstFishEyeAttr);

/**
 * @brief Get vpss channel FishEye attribute.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param pstFishEyeAttr(Out), FishEye attribute.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_GetChnFisheye(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, FISHEYE_ATTR_S *pstFishEyeAttr);

/**
 * @brief Send frame to vpss channel.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param pstVideoFrame(In), frame info.
 * @param s32MilliSec(In), timeout.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_SendChnFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
	const VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec);

/**
 * @brief Get frame in vpss channel.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param pstVideoFrame(Out), frame info.
 * @param s32MilliSec(In), timeout.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_GetChnFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VIDEO_FRAME_INFO_S *pstVideoFrame,
				 CVI_S32 s32MilliSec);

/**
 * @brief Release vpss channel frame.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param pstVideoFrame(In), frame info.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_ReleaseChnFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VIDEO_FRAME_INFO_S *pstVideoFrame);

/**
 * @brief Trigger venc encode.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param u32FrameCnt(In), frame count.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_TriggerSnapFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 u32FrameCnt);

/**
 * @brief Set channel to fetch VB from a specified pool.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param hVbPool(In), VB pool ID.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_AttachVbPool(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VB_POOL hVbPool);

/**
 * @brief Cancel specified pool.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_DetachVbPool(VPSS_GRP VpssGrp, VPSS_CHN VpssChn);

/**
 * @brief Set frame buffer stride.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param u32Align(In), buffer stride.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_SetChnAlign(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 u32Align);

/**
 * @brief Get frame buffer stride.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param u32Align(Out), buffer stride.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_GetChnAlign(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 *pu32Align);

/**
 * @brief Set Y ratio for YUV format
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param YRatio(In), Y ratio.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_SetChnYRatio(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_FLOAT YRatio);

/**
 * @brief Get Y ratio
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param YRatio(Out), Y ratio.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_GetChnYRatio(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_FLOAT *pYRatio);

/**
 * @brief Set Scale coefficient level for VPSS.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param enCoef(In), coef enum.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_SetChnScaleCoefLevel(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_SCALE_COEF_E enCoef);

/**
 * @brief Get Scale coefficient level for VPSS.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param enCoef(Out), coef enum.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_GetChnScaleCoefLevel(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_SCALE_COEF_E *penCoef);

/**
 * @brief Set Draw rectangle for VPSS.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param pstDrawRect(In), Draw rectangle param.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_SetChnDrawRect(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_DRAW_RECT_S *pstDrawRect);

/**
 * @brief Get Draw rectangle for VPSS.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param pstDrawRect(Out), Draw rectangle param.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_GetChnDrawRect(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_DRAW_RECT_S *pstDrawRect);

/**
 * @brief Set Convert for VPSS.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param pstConvert(In), Convert param.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_SetChnConvert(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CONVERT_S *pstConvert);

/**
 * @brief Get Convert for VPSS.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param pstConvert(Out), Convert param.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_GetChnConvert(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CONVERT_S *pstConvert);

/**
 * @brief channel image display.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_ShowChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn);

/**
 * @brief channel image hidden.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_HideChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn);


/**
 * @brief Get chn file descriptor.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @return file descriptor.
 */
CVI_S32 CVI_VPSS_GetChnFd(VPSS_GRP VpssGrp, VPSS_CHN VpssChn);

/**
 * @brief Close chn file descriptor.
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_CloseFd(void);

/**
 * @brief Get region luma in vpss chn.
 *
 * @param VpssGrp(In), group ID.
 * @param VpssChn(In), channel ID.
 * @param pstRegionInfo(In), region Info.
 * @param pu64LumaData(Out), Get the Luma value.
 * @param s32MilliSec(In), timeout.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_GetRegionLuma(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VIDEO_REGION_INFO_S *pstRegionInfo,
								CVI_U64 *pu64LumaData, CVI_S32 s32MilliSec);

/**
 * @brief Vpss stitch.
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_VPSS_Stitch(CVI_U32 u32ChnNum, VPSS_STITCH_CHN_ATTR_S *pstInput,
			VPSS_STITCH_OUTPUT_ATTR_S *pstOutput, VIDEO_FRAME_INFO_S *pstVideoFrame);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __CVI_VPSS_H__ */

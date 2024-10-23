/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_stitch.h
 * Description:
 *   MMF Programe Interface for video processing moudle
 */

#ifndef __CVI_STITCH_H__
#define __CVI_STITCH_H__

#include <cvi_common.h>
#include <cvi_comm_video.h>
#include <cvi_comm_stitch.h>
#include <cvi_comm_vb.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* init stitch moudle, include dev and stitch software subsystem.
 *
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_Init();

/* deinit stitch module, include dev and stitch software subsystem.
 *
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_DeInit();

/* init stitch grp
 *
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_InitGrp(STITCH_GRP stitchGrpIdx);

/* deinit stitch grp
 *
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_DeInitGrp(STITCH_GRP stitchGrpIdx);

/* set src attribution, specify input info.
 *
 * @param srcAttr(In): for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_SetSrcAttr(STITCH_GRP stitchGrpIdx, STITCH_SRC_ATTR_S *srcAttr);

/* get src attribution, specify input info.
 *
 * @param srcAttr(Out): for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_GetSrcAttr(STITCH_GRP stitchGrpIdx, STITCH_SRC_ATTR_S *srcAttr);

/* set chn attribution, specify output info.
 *
 * @param chnAttr(In): for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_SetChnAttr(STITCH_GRP stitchGrpIdx, STITCH_CHN_ATTR_S *chnAttr);

/* get chn attribution, specify output info.
 *
 * @param chnAttr(Out): for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_GetChnAttr(STITCH_GRP stitchGrpIdx, STITCH_CHN_ATTR_S *chnAttr);

/* set op attribution, specify operation info.
 *
 * @param opAttr(In): for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_SetOpAttr(STITCH_GRP stitchGrpIdx, STITCH_OP_ATTR_S *opAttr);

/* get op attribution, specify operation info.
 *
 * @param opAttr(Out): for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_GetOpAttr(STITCH_GRP stitchGrpIdx, STITCH_OP_ATTR_S *opAttr);

/* set wgt attribution, specify aplha beta weight info.
 *
 * @param wgtAttr(In): for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_SetWgtAttr(STITCH_GRP stitchGrpIdx, STITCH_WGT_ATTR_S *wgtAttr);

/* get wgt attribution, specify aplha beta weight info.
 *
 * @param wgtAttr(Out): for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_GetWgtAttr(STITCH_GRP stitchGrpIdx, STITCH_WGT_ATTR_S *wgtAttr);

/* set regx, specify weight depth.
 * alpha_pixel + beta_pixel = 8’d255
 * blend_out = (alpha_pixel*(left_pixel<<8) + beta_pixel*(right_pixel<<8))>>reg_x
 *
 * @param regX(In): weight depth
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_SetRegX(CVI_U8 regX);

/* enable stitch dev
 *
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_EnableGrp(STITCH_GRP stitchGrpIdx);

/* disable stitch dev
 *
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_DisableGrp(STITCH_GRP stitchGrpIdx);

/* reset stitch dev
 *
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_Reset(void);

/**
 * @brief Send src frame to stitch.
 *
 * @param srcIdx(In), src ID.
 * @param VideoFrame(In), frame info.
 * @param MilliSec(In), timeouts.
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_SendFrame(STITCH_GRP stitchGrpIdx, STITCH_SRC_IDX srcIdx, const VIDEO_FRAME_INFO_S *VideoFrame, CVI_S32 MilliSec);

/**
 * @brief Send chn frame to stitch, chn frame specify output frame.
 *
 * @param VideoFrame(In), frame info.
 * @param MilliSec(In), timeouts.
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_SendChnFrame(STITCH_GRP stitchGrpIdx, const VIDEO_FRAME_INFO_S *VideoFrame, CVI_S32 MilliSec);

/**
 * @brief Get chn frame to stitch, chn frame specify output frame.
 *
 * @param VideoFrame(Out), frame info.
 * @param MilliSec(In), timeouts.
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_GetChnFrame(STITCH_GRP stitchGrpIdx, VIDEO_FRAME_INFO_S *VideoFrame, CVI_S32 MilliSec);

/**
 * @brief release chn frame to stitch, chn frame specify output frame.
 *
 * @param VideoFrame(In), frame info.
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_ReleaseChnFrame(STITCH_GRP stitchGrpIdx, VIDEO_FRAME_INFO_S *VideoFrame);

/**
 * @brief Set chn frame to fetch VB from a specified pool.
 *
 * @param VbPool(In), VB pool ID.
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_AttachVbPool(STITCH_GRP stitchGrpIdx, VB_POOL VbPool);

/**
 * @brief Cancel specified pool.
 *
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_DetachVbPool(STITCH_GRP stitchGrpIdx);

/* Suspend stitch dev and stitch software subsystem.
 *
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_Suspend(void);

/* Resume stitch dev and stitch software subsystem.
 *
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_Resume(void);

/* get fd for stitch dev.
 *
 * @return fd
 */
CVI_S32 CVI_STITCH_GetDevFd();

/* dump stitch dev register info.
 *
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_STITCH_DumpRegInfo(void);


/* get stitch available grp.
 *
 * @return Error code (0 if successful)
 */

STITCH_GRP CVI_STITCH_GetAvailableGrp(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __CVI_STITCH_H__ */

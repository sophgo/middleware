/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_stitch.h
 * Description:
 *   MMF Programe Interface for video processing moudle
 */

#ifndef __CVI_STITCH_H__
#define __CVI_STITCH_H__

#include <linux/cvi_common.h>
#include <linux/cvi_comm_video.h>
#include <linux/cvi_comm_stitch.h>
#include <linux/cvi_comm_vb.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

CVI_S32 CVI_STITCH_Init();
CVI_S32 CVI_STITCH_DeInit();
CVI_S32 CVI_STITCH_SetSrcAttr(STITCH_SRC_ATTR_S *srcAttr);
CVI_S32 CVI_STITCH_GetSrcAttr(STITCH_SRC_ATTR_S *srcAttr);
CVI_S32 CVI_STITCH_SetChnAttr(STITCH_CHN_ATTR_S *chnAttr);
CVI_S32 CVI_STITCH_GetChnAttr(STITCH_CHN_ATTR_S *chnAttr);
CVI_S32 CVI_STITCH_SetOpAttr(STITCH_OP_ATTR_S *opAttr);
CVI_S32 CVI_STITCH_GetOpAttr(STITCH_OP_ATTR_S *opAttr);
CVI_S32 CVI_STITCH_SetWgtAttr(STITCH_WGT_ATTR_S *wgtAttr);
CVI_S32 CVI_STITCH_GetWgtAttr(STITCH_WGT_ATTR_S *wgtAttr);
CVI_S32 CVI_STITCH_SetRegX(CVI_U8 regX);
CVI_S32 CVI_STITCH_EnableDev(void);
CVI_S32 CVI_STITCH_DisableDev(void);
CVI_S32 CVI_STITCH_Reset(void);
CVI_S32 CVI_STITCH_SendFrame(STITCH_SRC_IDX srcIdx, const VIDEO_FRAME_INFO_S *VideoFrame, CVI_S32 MilliSec);
CVI_S32 CVI_STITCH_SendChnFrame(const VIDEO_FRAME_INFO_S *VideoFrame, CVI_S32 MilliSec);
CVI_S32 CVI_STITCH_GetChnFrame(VIDEO_FRAME_INFO_S *VideoFrame, CVI_S32 MilliSec);
CVI_S32 CVI_STITCH_ReleaseChnFrame(VIDEO_FRAME_INFO_S *VideoFrame);
CVI_S32 CVI_STITCH_AttachVbPool(VB_POOL VbPool);
CVI_S32 CVI_STITCH_DetachVbPool(void);
CVI_S32 CVI_STITCH_Suspend(void);
CVI_S32 CVI_STITCH_Resume(void);
CVI_S32 CVI_STITCH_GetDevFd();
CVI_S32 CVI_STITCH_DumpRegInfo(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __CVI_STITCH_H__ */

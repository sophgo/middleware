#ifndef _DPU_H_
#define _DPU_H_
#include "cvi_comm_dpu.h"
#include <linux/cvi_common.h>
#include <linux/cvi_errno.h>
#include <linux/cvi_defines.h>
CVI_S32 CVI_DPU_GetAssistBufSize(CVI_U16 u16_disp_num,CVI_U32 u32_dst_height,CVI_U32 * pu32_size);

CVI_S32 CVI_DPU_CreateGrp(DPU_GRP DpuGrp, const DPU_GRP_ATTR_S *pstGrpAttr);

CVI_S32 CVI_DPU_DestroyGrp(DPU_GRP DpuGrp);

CVI_S32 CVI_DPU_SetGrpAttr(DPU_GRP DpuGrp,const DPU_GRP_ATTR_S *pstGrpAttr);

CVI_S32 CVI_DPU_GetGrpAttr(DPU_GRP DpuGrp,DPU_GRP_ATTR_S *pstGrpAttr);

CVI_S32 CVI_DPU_StartGrp(DPU_GRP DpuGrp);

CVI_S32 CVI_DPU_StopGrp(DPU_GRP DpuGrp);

CVI_S32 CVI_DPU_SetChnAttr(DPU_GRP DpuGrp,DPU_CHN  DpuChn,const DPU_CHN_ATTR_S *pstChnAttr);

CVI_S32 CVI_DPU_GetChnAttr(DPU_GRP DpuGrp,DPU_CHN DpuChn,DPU_CHN_ATTR_S *pstChnAttr);

CVI_S32 CVI_DPU_EnableChn(DPU_GRP DpuGrp,DPU_CHN DpuChn);

CVI_S32 CVI_DPU_DisableChn(DPU_GRP DpuGrp,DPU_CHN DpuChn);

CVI_S32 CVI_DPU_SendFrame(DPU_GRP DpuGrp,\
                                const VIDEO_FRAME_INFO_S *pst_left_frame,\
                                const VIDEO_FRAME_INFO_S *pst_right_frame,\
                                CVI_S32 s32Millisec);

CVI_S32 CVI_DPU_GetFrame(DPU_GRP DpuGrp,\
							DPU_CHN DpuChn,\
							VIDEO_FRAME_INFO_S *pstFrameInfo,\
							CVI_S32 s32Millisec);

CVI_S32 CVI_DPU_ReleaseFrame(DPU_GRP DpuGrp,\
							DPU_CHN DpuChn,\
                            const VIDEO_FRAME_INFO_S *pstVideoFrame);


void CVI_DPU_CheckRegWrite(void);

void CVI_DPU_CheckRegRead(void);

void CVI_DPU_CheckSgbmStatus(void);

void CVI_DPU_CheckFgsStatus(void);

void CVI_DPU_Reset(void);

#endif /* _DPU_H_ */
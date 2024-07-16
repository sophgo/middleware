#ifndef _DPU_H_
#define _DPU_H_
#include "cvi_comm_dpu.h"
#include <cvi_common.h>
#include <cvi_errno.h>
#include <cvi_defines.h>

/**
 * @brief Create dpu group.
 *
 * @param DpuGrp(In), group ID.
 * @param pstGrpAttr(In), group attribute.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_DPU_CreateGrp(DPU_GRP DpuGrp, const DPU_GRP_ATTR_S *pstGrpAttr);

/**
 * @brief Destroy dpu group.
 *
 * @param DpuGrp(In), group ID.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_DPU_DestroyGrp(DPU_GRP DpuGrp);

/**
 * @brief Set dpu group attribute.
 *
 * @param DpuGrp(In), group ID.
 * @param pstGrpAttr(In), group attribute.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_DPU_SetGrpAttr(DPU_GRP DpuGrp,const DPU_GRP_ATTR_S *pstGrpAttr);

/**
 * @brief Get dpu group attribute.
 *
 * @param DpuGrp(In), group ID.
 * @param pstGrpAttr(Out), group attribute.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_DPU_GetGrpAttr(DPU_GRP DpuGrp,DPU_GRP_ATTR_S *pstGrpAttr);

/**
 * @brief Start dpu group.
 *
 * @param DpuGrp(In), group ID.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_DPU_StartGrp(DPU_GRP DpuGrp);

/**
 * @brief Stop dpu group.
 *
 * @param DpuGrp(In), group ID.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_DPU_StopGrp(DPU_GRP DpuGrp);

/**
 * @brief Set dpu channel attribute.
 *
 * @param DpuGrp(In), group ID.
 * @param DpuChn(In), channel ID.
 * @param pstChnAttr(In), channel attribute.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_DPU_SetChnAttr(DPU_GRP DpuGrp,DPU_CHN  DpuChn,const DPU_CHN_ATTR_S *pstChnAttr);

/**
 * @brief Get dpu channel attribute.
 *
 * @param DpuGrp(In), group ID.
 * @param DpuChn(In), channel ID.
 * @param pstChnAttr(Out), channel attribute.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_DPU_GetChnAttr(DPU_GRP DpuGrp,DPU_CHN DpuChn,DPU_CHN_ATTR_S *pstChnAttr);

/**
 * @brief Enable dpu channel.
 *
 * @param DpuGrp(In), group ID.
 * @param DpuChn(In), channel ID.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_DPU_EnableChn(DPU_GRP DpuGrp,DPU_CHN DpuChn);

/**
 * @brief Disable dpu channel.
 *
 * @param DpuGrp(In), group ID.
 * @param DpuChn(In), channel ID.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_DPU_DisableChn(DPU_GRP DpuGrp,DPU_CHN DpuChn);

/**
 * @brief Send frame to dpu group.
 *
 * @param DpuGrp(In), group ID.
 * @param pst_left_frame(In),left frame info.
 * @param pst_right_frame(In),right frame info.
 * @param s32MilliSec(In), timeouts.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_DPU_SendFrame(DPU_GRP DpuGrp,\
                                const VIDEO_FRAME_INFO_S *pst_left_frame,\
                                const VIDEO_FRAME_INFO_S *pst_right_frame,\
                                CVI_S32 s32Millisec);

/**
 * @brief Get frame in dpu channel.
 *
 * @param DpuGrp(In), group ID.
 * @param DpuChn(In), channel ID.
 * @param pstFrameInfo(Out), frame info.
 * @param s32MilliSec(In), timeout.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_DPU_GetFrame(DPU_GRP DpuGrp,\
							DPU_CHN DpuChn,\
							VIDEO_FRAME_INFO_S *pstFrameInfo,\
							CVI_S32 s32Millisec);

/**
 * @brief Release dpu channel frame.
 *
 * @param DpuGrp(In), group ID.
 * @param DpuChn(In), channel ID.
 * @param pstVideoFrame(In), frame info.
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_DPU_ReleaseFrame(DPU_GRP DpuGrp,\
							DPU_CHN DpuChn,\
                            const VIDEO_FRAME_INFO_S *pstVideoFrame);


#endif /* _DPU_H_ */

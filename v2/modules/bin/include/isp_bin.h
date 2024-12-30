/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: isp_bin.h
 * Description:
 *
 */

#ifndef _ISP_BIN_H
#define _ISP_BIN_H

#include <cvi_common.h>
#include "cvi_bin.h"
#include "cvi_bin_autogen.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef CVI_S32 (*pfn_cvi_bin_getbinsize)(enum CVI_BIN_SECTION_ID id, CVI_U32 *size);
typedef CVI_S32 (*pfn_cvi_bin_getparamfrombin)(enum CVI_BIN_SECTION_ID id, CVI_U8 *addr, CVI_U32 size);
typedef CVI_S32 (*pfn_cvi_bin_setparamtobin)(enum CVI_BIN_SECTION_ID id, FILE *fp);
typedef CVI_S32 (*pfn_cvi_bin_setparamtobuf)(enum CVI_BIN_SECTION_ID id, unsigned char *buffer);
typedef CVI_S32 (*pfn_cvi_indexbin_getparamfrombin)(CVI_U8 *buffer, enum CVI_BIN_SECTION_ID id,
													CVI_U32 binSize, CVI_U32 indexOffset);

CVI_S32 header_bin_getBinSize(enum CVI_BIN_SECTION_ID id, CVI_U32 *binSize);
CVI_S32 header_bin_getBinParam(enum CVI_BIN_SECTION_ID id, CVI_U8 *addr, CVI_U32 binSize);
CVI_S32 header_bin_GetHeaderDescInfo(CVI_CHAR *pOutDesc, enum CVI_BIN_CREATMODE inCreatMode);

CVI_S32 isp_bin_getBinSize(enum CVI_BIN_SECTION_ID id, CVI_U32 *binSize);
CVI_S32 isp_bin_getBinParam(enum CVI_BIN_SECTION_ID id, CVI_U8 *addr, CVI_U32 binSize);
CVI_S32 header_bin_setBinParam(enum CVI_BIN_SECTION_ID id, FILE *fp);
CVI_S32 isp_bin_setBinParam(enum CVI_BIN_SECTION_ID id, FILE *fp);
CVI_S32 header_bin_setBinParambuf(enum CVI_BIN_SECTION_ID id, CVI_U8 *buffer);
CVI_S32 isp_bin_setBinParambuf(enum CVI_BIN_SECTION_ID id, CVI_U8 *buffer);

CVI_S32 isp_bin_setBinBypassParams(VI_PIPE ViPipe, ISP_BIN_BYPASS_U *ispBinBypass);
CVI_S32 isp_bin_getBinBypassParams(VI_PIPE ViPipe, ISP_BIN_BYPASS_U *ispBinBypass);

/*----------Index table bin related APIs----------*/
CVI_S32 cvi_bin_getIndexInitHeader(CVI_BIN_INDEX_HEADER *idx_header);
CVI_S32 cvi_bin_getIndexTable(Entry *entrys);
CVI_S32 isp_index_bin_getparamfrombin(CVI_U8 *buffer, enum CVI_BIN_SECTION_ID id,
										CVI_U32 binSize, CVI_U32 indexOffset);
CVI_S32 cvi_bin_computeIndexHeaderOffset(CVI_BIN_INDEX_HEADER *idx_header, CVI_U32 bin_header_size);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif // _ISP_BIN_H

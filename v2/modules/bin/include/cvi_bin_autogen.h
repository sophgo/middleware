#ifndef __CVI_BIN_AUTOGEN_H__
#define __CVI_BIN_AUTOGEN_H__

#include <stdio.h>

#include "cvi_bin.h"
#include "cvi_common.h"
#include "cvi_bin_param_header.h"

typedef struct _CVI_BIN_INDEX_HEADER {
	CVI_U32 offsetIspEntry;
	CVI_U32 offsetVpssEntry;
	CVI_U32 offsetVoEntry;
	CVI_U32 offsetData[CVI_BIN_ID_MAX];
	CVI_U32 size[CVI_BIN_ID_MAX]; /*size info for every modules, 0 for header and index*/
} CVI_BIN_INDEX_HEADER;

typedef struct _Entry {
	CVI_U16 ID;
	CVI_U16 offset;
	CVI_U16 size;
	CVI_U16 unitsize;
} Entry;

CVI_S32 isp_getBinParam_autogen(CVI_U32 id, CVI_U8 *buf, ISP_Parameter_Structures *pst, CVI_U32 indexOffset);
CVI_S32 vpss_getBinParam_autogen(CVI_U32 id, CVI_U8 *buf, VPSS_Parameter_Structures *pst, CVI_U32 indexOffset);
CVI_S32 vo_getBinParam_autogen(CVI_U32 id, CVI_U8 *buf, VO_Parameter_Structures *pst, CVI_U32 indexOffset);

CVI_S32 cvi_bin_getIndexInitHeader_autogen(CVI_BIN_INDEX_HEADER *idx_header);
CVI_S32 cvi_bin_setIndextoBuf_autogen(CVI_BIN_INDEX_HEADER *idx_header, CVI_U8 **buffer);
CVI_S32 cvi_bin_setIndex_autogen(CVI_BIN_INDEX_HEADER *idx_header, FILE *fp);
CVI_S32 cvi_bin_getBinSize_autogen(CVI_U32 *binSize);
CVI_S32 cvi_bin_getIndexTable_autogen(Entry *entrys);
CVI_S32 cvi_bin_computeIndexHeaderOffset_autogen(CVI_BIN_INDEX_HEADER *idx_header, CVI_U32 bin_header_size);

#endif
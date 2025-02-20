#ifndef _VO_BIN_H
#define _VO_BIN_H

#include <cvi_type.h>
#include "cvi_bin.h"
#include <cvi_common.h>
#include <cvi_comm_vo.h>
#include "cvi_bin_autogen.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

CVI_S32 vo_bin_getbinsize(enum CVI_BIN_SECTION_ID id, CVI_U32 *size);
CVI_S32 vo_bin_getparamfrombin(enum CVI_BIN_SECTION_ID id, CVI_U8 *addr, CVI_U32 size);
CVI_S32 vo_bin_setparamtobuf(enum CVI_BIN_SECTION_ID id, CVI_U8 *buffer);
CVI_S32 vo_index_bin_getparamfrombin(CVI_U8 *buffer, enum CVI_BIN_SECTION_ID id,
										CVI_U32 binSize, CVI_U32 indexOffset);

VO_BIN_INFO_S *get_vo_bin_info_addr(void);
CVI_U32 get_vo_bin_guardmagic_code(void);

#endif
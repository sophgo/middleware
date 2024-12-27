#ifndef _VPSS_BIN_H
#define _VPSS_BIN_H

#include <cvi_type.h>
#include "cvi_base.h"
#include "cvi_bin_autogen.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

CVI_S32 vpss_index_bin_getparamfrombin(CVI_U8 *buffer, enum CVI_BIN_SECTION_ID id,
										CVI_U32 binSize, CVI_U32 indexOffset);
CVI_S32 vpss_bin_getbinsize(enum CVI_BIN_SECTION_ID id, CVI_U32 *size);
CVI_S32 vpss_bin_getparamfrombin(enum CVI_BIN_SECTION_ID id, CVI_U8 *addr, CVI_U32 size);

CVI_S32 vpss_bin_setparamtobuf(enum CVI_BIN_SECTION_ID id, CVI_U8 *buffer);

CVI_VOID set_loadbin_state(CVI_BOOL done);
VPSS_BIN_DATA *get_vpssbindata_addr(void);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif

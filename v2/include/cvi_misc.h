/*
 * Copyright (C) Cvitek Co., Ltd. 2023-2025. All rights reserved.
 *
 * File Name: include/cvi_misc.h
 * Description:
 *   MMF Programe Interface for system
 */

#ifndef __CVI_MISC_H__
#define __CVI_MISC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "cvi_debug.h"

/* OTP interface */
typedef enum {
	CVI_OTP_SECUREBOOT_DISABLE = 0,
	CVI_OTP_SECUREBOOT_SIGN,
	CVI_OTP_SECUREBOOT_SIGN_ENCRYPT,
} CVI_OTP_SECUREBOOT_E;

/*
 * Describe: read from OTP2 block
 * Param   : segment[in]				segment id of otp2 block
 * Param   : cell[in]					cell id in segment
 * Param   : size[in]					cell counts
 * Param   : value[out]					read result
 * Return  :
 *		# 0                     success
 *		# -1/other              fail
 */
CVI_S32 CVI_OTP2_Read(CVI_U32 segment, CVI_U32 cell, CVI_U32 size, CVI_U32 *value);
/*
 * Describe: write to OTP2 block
 * Param   : segment[in]				segment id of otp2 block
 * Param   : cell[in]					cell id in segment
 * Param   : size[in]					cell counts
 * Param   : value[in]					data to write
 * Return  :
 *		# 0                     success
 *		# -1/other              fail
 */
CVI_S32 CVI_OTP2_Write(CVI_U32 segment, CVI_U32 cell, CVI_U32 size, CVI_U32 *value);
/*
 * Describe: read from OTP3 block
 * Param   : segment[in]				segment id of otp3 block
 * Param   : cell[in]					cell id in segment
 * Param   : size[in]					cell counts
 * Param   : value[out]					read result
 * Return  :
 *		# 0                     success
 *		# -1/other              fail
 */
CVI_S32 CVI_OTP3_Read(CVI_U32 segment, CVI_U32 cell, CVI_U32 size, CVI_U32 *value);
/*
 * Describe: write to OTP3 block
 * Param   : segment[in]				segment id of otp3 block
 * Param   : cell[in]					cell id in segment
 * Param   : size[in]					cell counts
 * Param   : value[in]					data to write
 * Return  :
 *		# 0                     success
 *		# -1/other              fail
 */
CVI_S32 CVI_OTP3_Write(CVI_U32 segment, CVI_U32 cell, CVI_U32 size, CVI_U32 *value);
/*
 * Describe: enable secure boot
 * Param   : sel[in]				param for secure boot
 * Return  :
 *		# 0                     success
 *		# -1/other              fail
 */
CVI_S32 CVI_OTP_EnableSecureBoot(CVI_OTP_SECUREBOOT_E sel);
/*
 * Describe: Is secure boot enable
 * Return  :
 *		# 2			enable(sign and encrypt)
 *		# 1			enable(sign)
 *		# 0                     disable
 *		# -1/other              fail
 */
CVI_S32 CVI_OTP_IsSecureBootEnabled(void);

/*
 * Describe: get chip serial number
 * Param   : pu8SN[out]				buffer for chip sn
 * Param   : u32SNSize[in]			buffer size
 * Return  :
 *		# 8                     success
 *		# -1/other              fail
 */
CVI_S32 CVI_MISC_GetChipSN(CVI_U8 *pu8SN, CVI_U32 u32SNSize);
/*
 * Describe: get chip serial number size
 * Param   : pu32SNSize[out]			buffer for size
 * Return  :
 *		# 0			success
 *		# -1			fail
 */
CVI_S32 CVI_MISC_GetChipSNSize(CVI_U32 *pu32SNSize);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
#endif /*__CVI_SYS_H__ */

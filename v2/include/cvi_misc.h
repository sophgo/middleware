/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2022. All rights reserved.
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

typedef enum {
	CVI_EFUSE_AREA_USER,
	CVI_EFUSE_AREA_DEVICE_ID,
	CVI_EFUSE_AREA_HASH0_PUBLIC,
	CVI_EFUSE_AREA_LOADER_EK,
	CVI_EFUSE_AREA_DEVICE_EK,
	CVI_EFUSE_AREA_CHIP_SN,
	CVI_EFUSE_AREA_LAST
} CVI_EFUSE_AREA_E;

typedef enum {
	CVI_EFUSE_LOCK_HASH0_PUBLIC,
	CVI_EFUSE_LOCK_LOADER_EK,
	CVI_EFUSE_LOCK_DEVICE_EK,
	CVI_EFUSE_LOCK_LAST
} CVI_EFUSE_LOCK_E;

CVI_S32 CVI_MISC_StartPMThread(void);
CVI_S32 CVI_MISC_StopPMThread(void);
/** <!-- [EFUSE] */
CVI_S32 CVI_EFUSE_GetSize(CVI_EFUSE_AREA_E area, CVI_U32 *size);
CVI_S32 CVI_EFUSE_Read(CVI_EFUSE_AREA_E area, CVI_U8 *buf, CVI_U32 buf_size);
CVI_S32 CVI_EFUSE_Write(CVI_EFUSE_AREA_E area, const CVI_U8 *buf, CVI_U32 buf_size);
CVI_S32 CVI_EFUSE_EnableSecureBoot(void);
CVI_S32 CVI_EFUSE_IsSecureBootEnabled(void);
CVI_S32 CVI_EFUSE_EnableFastBoot(void);
CVI_S32 CVI_EFUSE_IsFastBootEnabled(void);
/**
 * @brief Lock eFuse area for both READING and WRITING.
 *
 * @param[in] lock - The area to be locked.
 *
 * @return CVI_SUCCESS (0) on success.
 * @return Non-zero error code on failure.
 */
CVI_S32 CVI_EFUSE_Lock(CVI_EFUSE_LOCK_E lock);
CVI_S32 CVI_EFUSE_IsLocked(CVI_EFUSE_LOCK_E lock);
/**
 * @brief Lock eFuse area for WRITING only.
 *
 * @param[in] lock - The area to be locked.
 *
 * @return CVI_SUCCESS (0) on success.
 * @return Non-zero error code on failure.
 */
CVI_S32 CVI_EFUSE_LockWrite(CVI_EFUSE_LOCK_E lock);
CVI_S32 CVI_EFUSE_IsWriteLocked(CVI_EFUSE_LOCK_E lock);

CVI_S32 CVI_MISC_SysSuspend(void);
CVI_S32 CVI_MISC_SysResume(void);

typedef enum {
    CVI_OTP_SECUREBOOT_DISABLE = 0,
    CVI_OTP_SECUREBOOT_SIGN,
    CVI_OTP_SECUREBOOT_SIGN_ENCRYPT,
} CVI_OTP_SECUREBOOT_E;

// OTP
CVI_S32 CVI_OTP2_Read(CVI_U32 segment, CVI_U32 addr, CVI_U32 size, CVI_U32 *value);
CVI_S32 CVI_OTP2_Write(CVI_U32 segment, CVI_U32 addr, CVI_U32 size, CVI_U32 *value);
CVI_S32 CVI_OTP3_Read(CVI_U32 segment, CVI_U32 addr, CVI_U32 size, CVI_U32 *value);
CVI_S32 CVI_OTP3_Write(CVI_U32 segment, CVI_U32 addr, CVI_U32 size, CVI_U32 *value);
CVI_S32 CVI_OTP_EnableSecureBoot(CVI_OTP_SECUREBOOT_E sel);
CVI_S32 CVI_OTP_IsSecureBootEnabled(void);

/*
 * Describe  get chip serial number
 * Param  pu8SN[out]			buffer for result
 * param  u32SNSize[in]			buffer size must >= 8
 * return the len of chip sn
        # = 8                   ok
        # other                 fail
*/
CVI_S32 CVI_MISC_GetChipSN(CVI_U8 *pu8SN, CVI_U32 u32SNSize);
// Deprecated
CVI_S32 CVI_MISC_GetChipSNSize(CVI_U32 *pu32SNSize);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*__CVI_SYS_H__ */

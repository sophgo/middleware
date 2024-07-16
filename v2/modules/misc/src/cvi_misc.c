#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>		/* low-level i/o */
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <cvi_sys.h>
#include <cvi_misc.h>
#include <cvi_errno.h>
#include "cvi_base.h"
#include <linux/sophon-otp.h>

#define CVI_OTP_CHIP_SN_SIZE				8

// for secure boot sign
#define CVI_OTP_TEE_SCS_ENABLE_SHIFT                    2
#define CVI_OTP_ROOT_PUBLIC_KEY_SELECTION_SHIFT         20
// for secure boot encryption
#define CVI_OTP_BOOT_LOADER_ENCRYPTION                  6
#define CVI_OTP_LDR_KEY_SELECTION_SHIFT                 23

static CVI_S32 _CVI_OTP_Read(unsigned long request, CVI_U32 segment, CVI_U32 cell, CVI_U32 size, CVI_U32 *value)
{
	struct otp_config config;
	CVI_S32 fd;

	fd = open("/dev/otp", O_RDWR);
	if (fd <= 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "otp open failed\n");
		return -1;
	}

	config.segment = segment;
	config.addr = cell;
	config.size = size;
	memset(config.value, 0, sizeof(uint32_t) * size);

	if (ioctl(fd, request, &config)) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "otp3 read ioctl failed\n");
		close(fd);
		return -1;
	}

	memcpy(value, config.value, sizeof(uint32_t) * size);
	close(fd);
	return 0;
}

static CVI_S32 _CVI_OTP_Write(unsigned long request, CVI_U32 segment, CVI_U32 cell, CVI_U32 size, CVI_U32 *value)
{
	struct otp_config config;
	CVI_S32 fd;

	fd = open("/dev/otp", O_RDWR);
	if (fd <= 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "otp open failed\n");
		return -1;
	}

	config.segment = segment;
	config.addr = cell;
	config.size = size;
	memcpy(config.value, value, sizeof(uint32_t) * size);

	if (ioctl(fd, request, &config)) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "otp3 write ioctl failed\n");
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}

static inline CVI_S32 CVI_OTP2_ArgmentsCheck(CVI_U32 segment, CVI_U32 cell, CVI_U32 size, CVI_U32 *value)
{
	if ((NULL == value)
		|| (segment > 27)
		|| (cell > 31)
		|| (size > 32)
		|| ((cell + size) > 32)) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "otp argments invailed\n");
		return -1;
	}

	if (segment == 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "otp argments is reserved\n");
		return -1;
	}

	return 0;
}

CVI_S32 CVI_OTP2_Read(CVI_U32 segment, CVI_U32 cell, CVI_U32 size, CVI_U32 *value)
{
	if (CVI_OTP2_ArgmentsCheck(segment, cell, size, value) < 0) {
		return -1;
	}

	return _CVI_OTP_Read(IOCTL_OTP2_READ_CONFIG, segment, cell, size, value);
}

CVI_S32 CVI_OTP2_Write(CVI_U32 segment, CVI_U32 cell, CVI_U32 size, CVI_U32 *value)
{
	if (CVI_OTP2_ArgmentsCheck(segment, cell, size, value) < 0) {
		return -1;
	}

	return _CVI_OTP_Write(IOCTL_OTP2_WRITE_CONFIG, segment, cell, size, value);
}

static inline CVI_S32 CVI_OTP3_ArgmentsCheck(CVI_U32 segment, CVI_U32 cell, CVI_U32 size, CVI_U32 *value)
{
	if ((NULL == value)
		|| (segment > 3)
		|| (cell > 31)
		|| (size > 32)
		|| ((cell + size) > 32)) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "otp argments invailed\n");
		return -1;
	}

	if ((segment == 2)
		|| (segment == 3)) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "otp argments is reserved\n");
		return -1;
	}

	return 0;
}

CVI_S32 CVI_OTP3_Read(CVI_U32 segment, CVI_U32 cell, CVI_U32 size, CVI_U32 *value)
{
	if (CVI_OTP3_ArgmentsCheck(segment, cell, size, value) < 0) {
		return -1;
	}

	return _CVI_OTP_Read(IOCTL_OTP3_READ_CONFIG, segment, cell, size, value);
}

CVI_S32 CVI_OTP3_Write(CVI_U32 segment, CVI_U32 cell, CVI_U32 size, CVI_U32 *value)
{
	if (CVI_OTP3_ArgmentsCheck(segment, cell, size, value) < 0) {
		return -1;
	}

	return _CVI_OTP_Write(IOCTL_OTP3_WRITE_CONFIG, segment, cell, size, value);
}

CVI_S32 CVI_OTP_EnableSecureBoot(CVI_OTP_SECUREBOOT_E sel)
{
	uint32_t value = 0;

	value |= (0x3 << CVI_OTP_TEE_SCS_ENABLE_SHIFT);
	value |= (0x4 << CVI_OTP_ROOT_PUBLIC_KEY_SELECTION_SHIFT);

	if (sel == CVI_OTP_SECUREBOOT_SIGN_ENCRYPT) {
		value |= (0x3 << CVI_OTP_BOOT_LOADER_ENCRYPTION);
		value |= (0x4 << CVI_OTP_LDR_KEY_SELECTION_SHIFT);
	}

	return _CVI_OTP_Write(IOCTL_OTP3_WRITE_CONFIG, 3, 8, 1, &value);
}

CVI_S32 CVI_OTP_IsSecureBootEnabled(void)
{
	uint32_t value = 0;
	int ret = -1;

	ret = _CVI_OTP_Read(IOCTL_OTP3_READ_CONFIG, 3, 8, 1, &value);
	if (ret < 0)
		return ret;

	if (value & (0x3 << CVI_OTP_TEE_SCS_ENABLE_SHIFT)) {
		if (value & (0x3 << CVI_OTP_BOOT_LOADER_ENCRYPTION)) {
			return CVI_OTP_SECUREBOOT_SIGN_ENCRYPT;
		} else {
			return CVI_OTP_SECUREBOOT_SIGN;
		}
	}

	return CVI_OTP_SECUREBOOT_DISABLE;
}

CVI_S32 CVI_MISC_GetChipSNSize(CVI_U32 *pu32SNSize)
{
	if (pu32SNSize) {
		*pu32SNSize = CVI_OTP_CHIP_SN_SIZE;
		return CVI_SUCCESS;
	}

	return CVI_FAILURE;
}

CVI_S32 CVI_MISC_GetChipSN(CVI_U8 *pu8SN, CVI_U32 u32SNSize)
{
	CVI_S32 ret;

	if (!pu8SN || (u32SNSize < 8))
		return CVI_ERR_SYS_ILLEGAL_PARAM;

	ret = _CVI_OTP_Read(IOCTL_OTP2_READ_CONFIG, 0, 0, 2, (CVI_U32 *)pu8SN);
	if (ret)
		return CVI_FAILURE;

	return CVI_OTP_CHIP_SN_SIZE;
}

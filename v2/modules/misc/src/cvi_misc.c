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
#include <linux/cvi_errno.h>
#include "cvi_base.h"

#define CVI_OTP_CHIP_SN_SIZE 8

CVI_S32 CVI_MISC_SysSuspend(void)
{
	CVI_VPSS_Suspend();
	//CVI_GDC_Suspend();
	CVI_VO_Suspend();

	//TODO
	//v4l2_dev_close(false, true, true, true);
	return CVI_SUCCESS;
}

CVI_S32 CVI_MISC_SysResume(void)
{
	//TODO
	//v4l2_dev_open(false, true, true, true);

	VI_VPSS_MODE_S stVIVPSSMode;

	CVI_SYS_GetVIVPSSMode(&stVIVPSSMode);
//TODO
#if 0
	if (init_vpss_device() != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "init vpss failed.\n");
		return CVI_FAILURE;
	}

	if (init_disp_device() != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "init disp failed.\n");
		return CVI_FAILURE;
	}

	if (init_ldc_device() != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "init ldc failed.\n");
		return CVI_FAILURE;
	}

	if (init_isp_device() != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "init isp failed.\n");
		return CVI_FAILURE;
	}
#endif
	CVI_SYS_SetVIVPSSMode(&stVIVPSSMode);

	CVI_VO_Resume();
	//CVI_GDC_Resume();
	CVI_VPSS_Resume();

	return CVI_SUCCESS;
}

#if 0
#define SIGBASE_STATE_CHANGE		44
pthread_t pm_thread;
CVI_BOOL g_is_pm_running = CVI_FALSE;

static CVI_S32 register_pm_ioctl(int io_fd)
{
	struct base_statesignal cs;
	int ret;

	memset(&cs, 0, sizeof(cs));
	cs.signr = SIGBASE_STATE_CHANGE;
	ret = ioctl(io_fd, IOCTL_STATESIG, &cs);
	if (ret < 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "register connect signal fail %d", ret);
		return -1;
	}
	return 0;
}

static CVI_VOID *pm_event_handler(CVI_VOID *data)
{
	int base_pm_fd;
	sigset_t mask;
	siginfo_t info;
	struct timespec ts;
	int ret;

	prctl(PR_SET_NAME, "pm_event_handler");
	(void)(data);

	if (open_device(BASE_DEV_NAME, &base_pm_fd) == -1) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "base dev open fail!\n");
		return NULL;
	}

	/* BLOCK  signal. */
	sigemptyset(&mask);
	sigaddset(&mask, SIGBASE_STATE_CHANGE);
	if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Cannot block SIGBASE_STATE_CHANGE: %s.\n", strerror(errno));
		return NULL;
	}

	/* register signal */
	if (register_pm_ioctl(base_pm_fd) < 0)
		return NULL;
	/* configure the timeout 50ms */
	ts.tv_sec = 0;
	ts.tv_nsec = 50000000;
	while (g_is_pm_running) {
		sigemptyset(&mask);
		sigaddset(&mask, SIGBASE_STATE_CHANGE);
		ret = sigtimedwait(&mask, &info, &ts);
		if (ret == -1) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			CVI_TRACE_SYS(CVI_DBG_ERR, "sigwaitinfo() failed: %s.\n", strerror(errno));
			goto exit_loop;
		}
		if (info.si_signo == SIGBASE_STATE_CHANGE) {
			CVI_U32 pm_state;
			/* get current state. */
			ret = ioctl(base_pm_fd, IOCTL_READ_STATE, &pm_state);
			if (ret < 0) {
				CVI_TRACE_SYS(CVI_DBG_ERR, "get currenct state fail %d\n", ret);
				goto exit_loop;
			}
			CVI_TRACE_SYS(CVI_DBG_INFO, "receive state %d\n", pm_state);
			printf("receive state %d\n", pm_state);
			switch (pm_state) {
			case BASE_STATE_SUSPEND_PREPARE: {
				/* call VIP user space suspend. */
				CVI_VI_Suspend();

				CVI_MISC_SysSuspend();

				/* notify the user space suspend is complete*/
				ret = ioctl(base_pm_fd, IOCTL_USER_SUSPEND_DONE, 0);
				if (ret < 0) {
					CVI_TRACE_SYS(CVI_DBG_ERR, "set suspend done fail %d\n", ret);
					goto exit_loop;
				}
				break;
			}
			case BASE_STATE_RESUME: {
				CVI_MISC_SysResume();

				/* call VIP user space resume. */
				CVI_VI_Resume();
				/* notify the user space resume is complete*/
				ret = ioctl(base_pm_fd, IOCTL_USER_RESUME_DONE, 0);
				if (ret < 0) {
					CVI_TRACE_SYS(CVI_DBG_ERR, "set resume done fail %d\n", ret);
					goto exit_loop;
				}
				break;
			}
			default:
				/* receive other state change */
				break;
			}

		} else {
			CVI_TRACE_SYS(CVI_DBG_DEBUG, "other signal number %d, %d\n", ret, info.si_signo);
		}

	}
	close(base_pm_fd);

exit_loop:
	g_is_pm_running = CVI_FALSE;
	pthread_exit(NULL);
}

CVI_S32 CVI_MISC_StartPMThread(void)
{
	struct sched_param param;
	pthread_attr_t attr;

	if (g_is_pm_running == CVI_TRUE) {
		CVI_TRACE_SYS(CVI_DBG_WARN, "already started\n");
		return CVI_FAILURE;
	}
	g_is_pm_running = CVI_TRUE;

	param.sched_priority = 70;

	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	pthread_create(&pm_thread, &attr, (void *)pm_event_handler, NULL);
	CVI_TRACE_SYS(CVI_DBG_INFO, "CVI_SYS_StartPMThread\n");
	return CVI_SUCCESS;
}

CVI_S32 CVI_MISC_StopPMThread(void)
{
	if (g_is_pm_running == CVI_FALSE) {
		CVI_TRACE_SYS(CVI_DBG_WARN, "not start yet\n");
		return CVI_FAILURE;
	}

	g_is_pm_running = CVI_FALSE;
	if (pm_thread) {
		pthread_join(pm_thread, NULL);
		pm_thread = 0;
	}
	CVI_TRACE_SYS(CVI_DBG_INFO, "CVI_SYS_StopPMThread\n");
	return CVI_SUCCESS;
}
#endif

// ===========================================================================
// EFUSE API
// ===========================================================================
static struct _CVI_EFUSE_AREA_S {
	CVI_U32 addr;
	CVI_U32 size;
} cvi_efuse_area[] = { [CVI_EFUSE_AREA_USER] = { 0x40, 40 },
		       [CVI_EFUSE_AREA_DEVICE_ID] = { 0x8c, 8 },
		       [CVI_EFUSE_AREA_HASH0_PUBLIC] = { 0xA8, 32 },
		       [CVI_EFUSE_AREA_LOADER_EK] = { 0xD8, 16 },
		       [CVI_EFUSE_AREA_DEVICE_EK] = { 0xE8, 16 },
		       [CVI_EFUSE_AREA_CHIP_SN] = { 0x0C, 8 } };

static struct _CVI_EFUSE_LOCK_S {
	CVI_U32 wlock_shift;
	CVI_U32 rlock_shift;
} cvi_efuse_lock[] = { [CVI_EFUSE_LOCK_HASH0_PUBLIC] = { 0, 8 },
		       [CVI_EFUSE_LOCK_LOADER_EK] = { 4, 12 },
		       [CVI_EFUSE_LOCK_DEVICE_EK] = { 6, 14 } };

static struct _CVI_EFUSE_USER_S {
	CVI_U32 addr;
	CVI_U32 size;
} cvi_efuse_user[] = {
	{ 0x40, 4 },
	{ 0x48, 4 },
	{ 0x50, 4 },
	{ 0x58, 4 },
	{ 0x60, 4 },
	{ 0x68, 4 },
	{ 0x70, 4 },
	{ 0x78, 4 },
	{ 0x80, 4 },
	{ 0x88, 4 },
};

#define CVI_EFUSE_TOTAL_SIZE 0x100

#define CVI_EFUSE_LOCK_ADDR 0xF8
#define CVI_EFUSE_SECURE_CONF_ADDR 0xA0
#define CVI_EFUSE_SCS_ENABLE_SHIFT 0
#define CVI_EFUSE_SW_INFO 0x2C
#define CVI_EFUSE_CUSTOMER_ADDR 0x4

#define CVI_EFUSE_PATH_PROG "/sys/class/cvi-base/base_efuse_prog"
#define CVI_EFUSE_PATH_SHADOW "/sys/class/cvi-base/base_efuse_shadow"

CVI_S32 CVI_EFUSE_GetSize(CVI_EFUSE_AREA_E area, CVI_U32 *size)
{
	if (area >= ARRAY_SIZE(cvi_efuse_area) ||
	    cvi_efuse_area[area].size == 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "area (%d) is not found\n", area);
		return CVI_ERR_SYS_NOMEM;
	}

	if (size)
		*size = cvi_efuse_area[area].size;

	return 0;
}

static CVI_S32 _CVI_EFUSE_Read(CVI_U32 addr, void *buf, CVI_U32 buf_size)
{
	CVI_S32 ret = -1;

	CVI_TRACE_SYS(CVI_DBG_DEBUG, "addr=0x%02x\n", addr);

	if (!buf)
		return CVI_ERR_SYS_ILLEGAL_PARAM;

	FILE *fp = fopen(CVI_EFUSE_PATH_SHADOW, "r");

	if (!fp) {
		ret = errno;
		CVI_TRACE_SYS(CVI_DBG_ERR, "fopen(%s)\n",
			      CVI_EFUSE_PATH_SHADOW);
		return ret;
	}

	fseek(fp, addr, SEEK_SET);
	ret = fread(buf, buf_size, 1, fp);
	if (ret < 0)
		CVI_TRACE_SYS(CVI_DBG_ERR, "ret=%d\n", ret);

	fclose(fp);

	return ret;
}

static CVI_S32 _CVI_EFUSE_Write(CVI_U32 addr, const void *buf, CVI_U32 buf_size)
{
	CVI_TRACE_SYS(CVI_DBG_DEBUG, "addr=0x%02x\n", addr);

	char cmd[64];
	CVI_U32 value;
	CVI_U32 aligned_addr = addr;
	CVI_U32 aligned_size = buf_size;
	void *aligned_buf = NULL;
	size_t i;

	if (!buf)
		return CVI_ERR_SYS_ILLEGAL_PARAM;

	if (aligned_addr % 4) {
		aligned_addr -= aligned_addr % 4;
	}

	if (aligned_size % 4) {
		aligned_size += 4 - aligned_size % 4;
	}

	aligned_buf = malloc(aligned_size);
	memset(aligned_buf, 0, aligned_size);
	memcpy((CVI_U8 *)aligned_buf + (addr - aligned_addr), buf, buf_size);

	for (i = 0; i < aligned_size; i += 4) {
		memcpy(&value, (CVI_U8 *)aligned_buf + i, sizeof(value));
		snprintf(cmd, sizeof(cmd), "0x%04zx=0x%08x", addr + i, value);
		CVI_TRACE_SYS(CVI_DBG_DEBUG, "cmd=%s\n", cmd);

		FILE *fp = fopen(CVI_EFUSE_PATH_PROG, "w");
		int ret;

		if (!fp) {
			ret = errno;
			CVI_TRACE_SYS(CVI_DBG_ERR, "fopen(%s)\n",
				      CVI_EFUSE_PATH_PROG);
			return ret;
		}

		ret = fwrite(cmd, strlen(cmd), 1, fp);
		if (ret < 0)
			CVI_TRACE_SYS(CVI_DBG_ERR, "ret=%d\n", ret);
		fclose(fp);
	}

	return 0;
}

CVI_S32 CVI_EFUSE_Read(CVI_EFUSE_AREA_E area, CVI_U8 *buf, CVI_U32 buf_size)
{
	CVI_U32 user_size = cvi_efuse_area[CVI_EFUSE_AREA_USER].size;
	CVI_U8 user[user_size], *p;
	CVI_S32 ret;
	size_t i;

	if (area >= ARRAY_SIZE(cvi_efuse_area) ||
	    cvi_efuse_area[area].size == 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "area (%d) is not found\n", area);
		return CVI_ERR_SYS_NOMEM;
	}

	if (!buf)
		return CVI_ERR_SYS_ILLEGAL_PARAM;

	memset(buf, 0, buf_size);

	if (buf_size > cvi_efuse_area[area].size)
		buf_size = cvi_efuse_area[area].size;

	if (area != CVI_EFUSE_AREA_USER)
		return _CVI_EFUSE_Read(cvi_efuse_area[area].addr, buf,
				       buf_size);

	memset(user, 0, user_size);

	p = user;
	for (i = 0; i < ARRAY_SIZE(cvi_efuse_user); i++) {
		ret = _CVI_EFUSE_Read(cvi_efuse_user[i].addr, p,
				      cvi_efuse_user[i].size);
		if (ret < 0)
			return ret;
		p += cvi_efuse_user[i].size;
	}

	memcpy(buf, user, buf_size);

	return CVI_SUCCESS;
}

CVI_S32 CVI_EFUSE_Write(CVI_EFUSE_AREA_E area, const CVI_U8 *buf,
			CVI_U32 buf_size)
{
	CVI_U32 user_size = cvi_efuse_area[CVI_EFUSE_AREA_USER].size;
	CVI_U8 user[user_size], *p;
	CVI_S32 ret;
	size_t i;

	if (area >= ARRAY_SIZE(cvi_efuse_area) ||
	    cvi_efuse_area[area].size == 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "area (%d) is not found\n", area);
		return CVI_ERR_SYS_NOMEM;
	}
	if (!buf)
		return CVI_ERR_SYS_ILLEGAL_PARAM;

	if (buf_size > cvi_efuse_area[area].size)
		buf_size = cvi_efuse_area[area].size;

	if (area != CVI_EFUSE_AREA_USER) {
		return _CVI_EFUSE_Write(cvi_efuse_area[area].addr, buf,
					buf_size);
	}

	memset(user, 0, user_size);
	memcpy(user, buf, buf_size);

	p = user;
	for (i = 0; i < ARRAY_SIZE(cvi_efuse_user); i++) {
		ret = _CVI_EFUSE_Write(cvi_efuse_user[i].addr, p,
				       cvi_efuse_user[i].size);
		if (ret < 0)
			return ret;
		p += cvi_efuse_user[i].size;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_EFUSE_EnableSecureBoot(void)
{
	CVI_U32 value = 0x3 << CVI_EFUSE_SCS_ENABLE_SHIFT;

	return _CVI_EFUSE_Write(CVI_EFUSE_SECURE_CONF_ADDR, &value,
				sizeof(value));
}

CVI_S32 CVI_EFUSE_IsSecureBootEnabled(void)
{
	CVI_U32 value = 0;
	CVI_S32 ret = 0;

	ret = _CVI_EFUSE_Read(CVI_EFUSE_SECURE_CONF_ADDR, &value,
			      sizeof(value));
	CVI_TRACE_SYS(CVI_DBG_DEBUG, "ret=%d value=%u\n", ret, value);
	if (ret < 0)
		return ret;

	value &= 0x3 << CVI_EFUSE_SCS_ENABLE_SHIFT;
	return !!value;
}

CVI_S32 CVI_EFUSE_EnableFastBoot(void)
{
	CVI_U32 value = 0x1 << 22;
	CVI_S32 ret = 0;
	CVI_U32 chip = 0;

	// set sd dl button
	ret = _CVI_EFUSE_Write(CVI_EFUSE_SW_INFO, &value, sizeof(value));
	if (ret < 0)
		return ret;

	CVI_SYS_GetChipId(&chip);
	if (!IS_CHIP_CV181X(chip)) {
		CVI_TRACE_SYS(CVI_DBG_DEBUG, "IS_CHIP_CV181X=%d\n", IS_CHIP_CV181X(chip));
		CVI_TRACE_SYS(CVI_DBG_DEBUG, "IS_CHIP_PKG_TYPE_QFN=%d\n", IS_CHIP_PKG_TYPE_QFN(chip));

		return CVI_FAILURE;
	}

	if (IS_CHIP_PKG_TYPE_QFN(chip))
		value = 0x1E1E64; // AUX0
	else
		value = 0x1; // USB_ID

	return _CVI_EFUSE_Write(CVI_EFUSE_CUSTOMER_ADDR, &value, sizeof(value));
}

CVI_S32 CVI_EFUSE_IsFastBootEnabled(void)
{
	CVI_U32 value = 0;
	CVI_S32 ret = 0;
	CVI_U32 chip = 0;

	ret = _CVI_EFUSE_Read(CVI_EFUSE_SW_INFO, &value, sizeof(value));
	CVI_TRACE_SYS(CVI_DBG_DEBUG, "ret=%d value=%u\n", ret, value);
	if (ret < 0)
		return ret;

	if (((value & (0x1 << 22)) == 0) || ((value & (0x1 << 23)) != 0))
		return 0;

	ret = _CVI_EFUSE_Read(CVI_EFUSE_CUSTOMER_ADDR, &value, sizeof(value));
	CVI_TRACE_SYS(CVI_DBG_DEBUG, "ret=%d value=%u\n", ret, value);
	if (ret < 0)
		return ret;

	CVI_SYS_GetChipId(&chip);
	if (IS_CHIP_CV181X(chip) && IS_CHIP_PKG_TYPE_QFN(chip)) {
		if (value == 0x1E1E64)
			return CVI_SUCCESS; // AUX0
		else if (value == 0x1)
			return CVI_SUCCESS; // USB_ID
		else
			return CVI_FAILURE;
	}

	return CVI_FAILURE;
}

CVI_S32 CVI_EFUSE_Lock(CVI_EFUSE_LOCK_E lock)
{
	CVI_U32 value = 0;
	CVI_S32 ret = 0;

	if (lock >= ARRAY_SIZE(cvi_efuse_lock)) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "lock (%d) is not found\n", lock);
		return CVI_ERR_SYS_NOMEM;
	}

	value = 0x3 << cvi_efuse_lock[lock].wlock_shift;
	ret = _CVI_EFUSE_Write(CVI_EFUSE_LOCK_ADDR, &value, sizeof(value));
	if (ret < 0)
		return ret;

	value = 0x3 << cvi_efuse_lock[lock].rlock_shift;
	ret = _CVI_EFUSE_Write(CVI_EFUSE_LOCK_ADDR, &value, sizeof(value));
	return ret;
}

CVI_S32 CVI_EFUSE_IsLocked(CVI_EFUSE_LOCK_E lock)
{
	CVI_S32 ret = 0;
	CVI_U32 value = 0;

	if (lock >= ARRAY_SIZE(cvi_efuse_lock)) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "lock (%d) is not found\n", lock);
		return CVI_ERR_SYS_NOMEM;
	}

	ret = _CVI_EFUSE_Read(CVI_EFUSE_LOCK_ADDR, &value, sizeof(value));
	CVI_TRACE_SYS(CVI_DBG_DEBUG, "ret=%d value=%u\n", ret, value);
	if (ret < 0)
		return ret;

	value &= 0x3 << cvi_efuse_lock[lock].wlock_shift;
	return !!value;
}

CVI_S32 CVI_EFUSE_LockWrite(CVI_EFUSE_LOCK_E lock)
{
	CVI_U32 value = 0;
	CVI_S32 ret = 0;

	if (lock >= ARRAY_SIZE(cvi_efuse_lock)) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "lock (%d) is not found\n", lock);
		return CVI_ERR_SYS_NOMEM;
	}

	value = 0x3 << cvi_efuse_lock[lock].wlock_shift;
	ret = _CVI_EFUSE_Write(CVI_EFUSE_LOCK_ADDR, &value, sizeof(value));
	return ret;
}

CVI_S32 CVI_EFUSE_IsWriteLocked(CVI_EFUSE_LOCK_E lock)
{
	CVI_S32 ret = 0;
	CVI_U32 value = 0;

	if (lock >= ARRAY_SIZE(cvi_efuse_lock)) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "lock (%d) is not found\n", lock);
		return CVI_ERR_SYS_NOMEM;
	}

	ret = _CVI_EFUSE_Read(CVI_EFUSE_LOCK_ADDR, &value, sizeof(value));
	CVI_TRACE_SYS(CVI_DBG_DEBUG, "ret=%d value=%u\n", ret, value);
	if (ret < 0)
		return ret;

	value &= 0x3 << cvi_efuse_lock[lock].wlock_shift;
	return !!value;
}


struct otp_config {
    uint32_t segment;
    uint32_t addr;
    uint32_t size;
    uint32_t value[32];
};

#define IOCTL_OTP_BASE                          'O'
#define IOCTL_OTP_VERSION                       _IOR(IOCTL_OTP_BASE, 0, uint32_t)
#define IOCTL_OTP2_READ_CONFIG                  _IOWR(IOCTL_OTP_BASE, 1, struct otp_config)
#define IOCTL_OTP2_WRITE_CONFIG                 _IOW(IOCTL_OTP_BASE, 2, struct otp_config)
#define IOCTL_OTP3_READ_CONFIG                  _IOWR(IOCTL_OTP_BASE, 3, struct otp_config)
#define IOCTL_OTP3_WRITE_CONFIG                 _IOW(IOCTL_OTP_BASE, 4, struct otp_config)

// for secure boot sign
#define CVI_OTP_TEE_SCS_ENABLE_SHIFT                    2
#define CVI_OTP_ROOT_PUBLIC_KEY_SELECTION_SHIFT         20
// for secure boot encryption
#define CVI_OTP_BOOT_LOADER_ENCRYPTION                  6
#define CVI_OTP_LDR_KEY_SELECTION_SHIFT                 23

static CVI_S32 _CVI_OTP_Read(unsigned long request, CVI_U32 segment, CVI_U32 addr, CVI_U32 size, CVI_U32 *value)
{
	struct otp_config config;
	CVI_S32 fd;

	fd = open("/dev/otp", O_RDWR);
	if (fd <= 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "otp open failed\n");
		return -1;
	}

	config.segment = segment;
	config.addr = addr;
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

static CVI_S32 _CVI_OTP_Write(unsigned long request, CVI_U32 segment, CVI_U32 addr, CVI_U32 size, CVI_U32 *value)
{
	struct otp_config config;
	CVI_S32 fd;

	fd = open("/dev/otp", O_RDWR);
	if (fd <= 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "otp open failed\n");
		return -1;
	}

	config.segment = segment;
	config.addr = addr;
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

static inline CVI_S32 CVI_OTP2_ArgmentsCheck(CVI_U32 segment, CVI_U32 addr, CVI_U32 size, CVI_U32 *value)
{
	if ((NULL == value)
		|| (segment > 27)
		|| (addr > 31)
		|| (size > 32)
		|| ((addr + size) > 32)) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "otp argments invailed\n");
		return -1;
	}

	if (segment == 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "otp argments is reserved\n");
		return -1;
	}

	return 0;
}

CVI_S32 CVI_OTP2_Read(CVI_U32 segment, CVI_U32 addr, CVI_U32 size, CVI_U32 *value)
{
	if (CVI_OTP2_ArgmentsCheck(segment, addr, size, value) < 0) {
		return -1;
	}

	return _CVI_OTP_Read(IOCTL_OTP2_READ_CONFIG, segment, addr, size, value);
}

CVI_S32 CVI_OTP2_Write(CVI_U32 segment, CVI_U32 addr, CVI_U32 size, CVI_U32 *value)
{
	if (CVI_OTP2_ArgmentsCheck(segment, addr, size, value) < 0) {
		return -1;
	}

	return _CVI_OTP_Write(IOCTL_OTP2_WRITE_CONFIG, segment, addr, size, value);
}

static inline CVI_S32 CVI_OTP3_ArgmentsCheck(CVI_U32 segment, CVI_U32 addr, CVI_U32 size, CVI_U32 *value)
{
	if ((NULL == value)
		|| (segment > 3)
		|| (addr > 31)
		|| (size > 32)
		|| ((addr + size) > 32)) {
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

CVI_S32 CVI_OTP3_Read(CVI_U32 segment, CVI_U32 addr, CVI_U32 size, CVI_U32 *value)
{
	if (CVI_OTP3_ArgmentsCheck(segment, addr, size, value) < 0) {
		return -1;
	}

	return _CVI_OTP_Read(IOCTL_OTP3_READ_CONFIG, segment, addr, size, value);
}

CVI_S32 CVI_OTP3_Write(CVI_U32 segment, CVI_U32 addr, CVI_U32 size, CVI_U32 *value)
{
	if (CVI_OTP3_ArgmentsCheck(segment, addr, size, value) < 0) {
		return -1;
	}

	return _CVI_OTP_Write(IOCTL_OTP3_WRITE_CONFIG, segment, addr, size, value);
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

// Deprecated
CVI_S32 CVI_MISC_GetChipSNSize(CVI_U32 *pu32SNSize)
{
	if (pu32SNSize)
		*pu32SNSize = CVI_OTP_CHIP_SN_SIZE;

	return CVI_SUCCESS;
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

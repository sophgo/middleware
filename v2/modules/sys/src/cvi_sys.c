#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/queue.h>
#include <pthread.h>
#include <stdatomic.h>
#include <inttypes.h>
#include <sys/mman.h>

#include <fcntl.h>		/* low-level i/o */
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "devmem.h"
#include "cvi_base.h"
#include "cvi_sys.h"
#include "hashmap.h"
#include "base_uapi.h"
#include "sys_uapi.h"

#define MMF_VERSION  (CVI_CHIP_NAME MMF_VER_PRIX MK_VERSION(VER_X, VER_Y, VER_Z) VER_D)


static int devm_fd = -1, devm_cached_fd = -1;
static int ionFd = -1;
static void *shared_mem;
static MMF_VERSION_S *mmf_version;
CVI_S32 *log_levels;
CVI_CHAR const *log_name[8] = {
	(CVI_CHAR *)"EMG", (CVI_CHAR *)"ALT", (CVI_CHAR *)"CRI", (CVI_CHAR *)"ERR",
	(CVI_CHAR *)"WRN", (CVI_CHAR *)"NOT", (CVI_CHAR *)"INF", (CVI_CHAR *)"DBG"
};

static CVI_S32 _SYS_MMAP(void)
{
	if (shared_mem != NULL) {
		CVI_TRACE_SYS(CVI_DBG_INFO, "already done mmap\n");
		return CVI_SUCCESS;
	}

	shared_mem = base_get_shm();
	if (shared_mem == NULL) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "base_get_shm failed!\n");
		return CVI_ERR_SYS_NOMEM;
	}

	log_levels = (CVI_S32 *)(shared_mem + BASE_LOG_LEVEL_OFFSET);

	mmf_version = (MMF_VERSION_S *)(shared_mem + BASE_VERSION_INFO_OFFSET);
	memset(mmf_version, 0, VERSION_INFO_RSV_SIZE);
	CVI_SYS_GetVersion(mmf_version);

	return CVI_SUCCESS;
}

static CVI_S32 _SYS_UNMMAP(void)
{
	if (shared_mem == NULL) {
		CVI_TRACE_SYS(CVI_DBG_INFO, "No need to unmap\n");
		return CVI_SUCCESS;
	}

	base_release_shm();
	shared_mem = NULL;
	log_levels = NULL;
	mmf_version = NULL;

	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_DevMem_Open(void)
{
	if (devm_fd < 0)
		devm_fd = devm_open();

	if (devm_cached_fd < 0)
		devm_cached_fd = devm_open_cached();

	if (devm_fd < 0 || devm_cached_fd < 0) {
		perror("devmem open failed\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_DevMem_Close(void)
{
	if (devm_fd < 0 || devm_cached_fd < 0)
		return CVI_SUCCESS;

	devm_close(devm_fd);
	devm_fd = -1;
	devm_close(devm_cached_fd);
	devm_cached_fd = -1;
	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_Init(void)
{
	CVI_S32 s32ret = CVI_SUCCESS, _sys_fd = -1;
	CVI_U32 sys_init = 0;
	VI_VPSS_MODE_S stVIVPSSMode;

	//pthread_once(&lp_off_once, _sys_low_power_off);

	s32ret = base_dev_open();
	if (s32ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "base_dev_open failed\n");
		return CVI_ERR_SYS_NOTREADY;
	}

	s32ret = sys_dev_open();
	if (s32ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "sys_dev_open failed\n");
		return CVI_ERR_SYS_NOTREADY;
	}

	CVI_TRACE_SYS(CVI_DBG_INFO, "+\n");

	if (CVI_SYS_DevMem_Open() != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "devmem open failed.\n");
		return CVI_ERR_SYS_NOTREADY;
	}

	if (_SYS_MMAP() != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "_SYS_MMAP failed.\n");
		return CVI_ERR_SYS_NOMEM;
	}


#if 0 //TODO: need remove to vo module
	// check fb parameters to see if fb on vpss or vo.
	memset(&vo_ctx, 0, sizeof(vo_ctx));
	file = fopen("/sys/module/cvi_fb/parameters/option", "r");
	if (file == NULL)
		vo_ctx.fb_on_vpss = CVI_FALSE;
	else {
		CVI_S32 option = 0;

		fscanf(file, "%d", &option);
		vo_ctx.fb_on_vpss = option & BIT(1);
		fclose(file);
	}
#endif
	_sys_fd = get_sys_fd();
	ioctl(_sys_fd, SYS_IOC_GET_INIT, &sys_init);

	if (sys_init == 0) {
		for (CVI_U8 i = 0; i < VI_MAX_PIPE_NUM; ++i)
			stVIVPSSMode.aenMode[i] = VI_OFFLINE_VPSS_OFFLINE;
		CVI_SYS_SetVIVPSSMode(&stVIVPSSMode);
		//CVI_SYS_StartThermalThread();
	}

	ioctl(_sys_fd, SYS_IOC_SET_INIT, NULL);

	CVI_TRACE_SYS(CVI_DBG_INFO, "-\n");

	return s32ret;
}

CVI_S32 CVI_SYS_Exit(void)
{
	CVI_S32 s32ret = CVI_SUCCESS, _sys_fd = -1;
	CVI_U32 sys_init = 0;

	CVI_TRACE_SYS(CVI_DBG_INFO, "+\n");

	_sys_fd = get_sys_fd();
	ioctl(_sys_fd, SYS_IOC_GET_INIT, &sys_init);
	if (sys_init == 1) {
		//CVI_SYS_StopThermalThread();
	}

	if (ionFd > 0) {
		close(ionFd);
		ionFd = -1;
	}

	_SYS_UNMMAP();
	s32ret = CVI_SYS_DevMem_Close();
	if (s32ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "devmem close failed\n");
		return CVI_ERR_SYS_NOTREADY;
	}
	sys_dev_close();
	base_dev_close();

	CVI_TRACE_SYS(CVI_DBG_INFO, "-\n");

	return s32ret;
}

CVI_S32 _CVI_SYS_BindIOCtl(const MMF_CHN_S *pstSrcChn, const MMF_CHN_S *pstDestChn, CVI_U8 is_bind)
{
	CVI_S32 fd = 0;
	CVI_S32 ret = 0;
	struct sys_bind_cfg bind_cfg;

	if ((fd = get_base_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;

	memset(&bind_cfg, 0, sizeof(struct sys_bind_cfg));
	bind_cfg.is_bind = is_bind;
	bind_cfg.mmf_chn_src = *pstSrcChn;
	bind_cfg.mmf_chn_dst = *pstDestChn;

	ret = ioctl(fd, BASE_SET_BINDCFG, &bind_cfg);

	if (ret)
		CVI_TRACE_SYS(CVI_DBG_ERR, "_CVI_SYS_BindIOCtl()failed\n");

	return ret;
}

CVI_S32 CVI_SYS_Bind(const MMF_CHN_S *pstSrcChn, const MMF_CHN_S *pstDestChn)
{
#ifdef __CV180X__
	if (pstDestChn && (pstDestChn->enModId == CVI_ID_VO)) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "No vo device, vo cannot be bind!\n");
		return CVI_ERR_SYS_ILLEGAL_PARAM;
	}
#endif
	return _CVI_SYS_BindIOCtl(pstSrcChn, pstDestChn, 1);
}

CVI_S32 CVI_SYS_UnBind(const MMF_CHN_S *pstSrcChn, const MMF_CHN_S *pstDestChn)
{
#ifdef __CV180X__
	if (pstDestChn && (pstDestChn->enModId == CVI_ID_VO)) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "No vo device, cannot unbind vo!\n");
		return CVI_ERR_SYS_ILLEGAL_PARAM;
	}
#endif
	return _CVI_SYS_BindIOCtl(pstSrcChn, pstDestChn, 0);
}

CVI_S32 CVI_SYS_GetBindbyDest(const MMF_CHN_S *pstDestChn, MMF_CHN_S *pstSrcChn)
{
	CVI_S32 fd = 0;
	CVI_S32 ret = 0;
	struct sys_bind_cfg bind_cfg;

	if ((fd = get_base_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;

	memset(&bind_cfg, 0, sizeof(struct sys_bind_cfg));
	bind_cfg.get_by_src = 0;
	bind_cfg.mmf_chn_dst = *pstDestChn;

	ret = ioctl(fd, BASE_GET_BINDCFG, &bind_cfg);

	if (ret) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "CVI_SYS_GetBindbyDest() failed\n");
		return ret;
	}

	memcpy(pstSrcChn, &bind_cfg.mmf_chn_src, sizeof(MMF_CHN_S));
	return CVI_SUCCESS;

}

CVI_S32 CVI_SYS_GetBindbySrc(const MMF_CHN_S *pstSrcChn, MMF_BIND_DEST_S *pstBindDest)
{
	CVI_S32 fd = 0;
	CVI_S32 ret = 0;
	struct sys_bind_cfg bind_cfg;

	if ((fd = get_base_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;

	memset(&bind_cfg, 0, sizeof(struct sys_bind_cfg));
	bind_cfg.get_by_src = 1;
	bind_cfg.mmf_chn_src = *pstSrcChn;

	ret = ioctl(fd, BASE_GET_BINDCFG, &bind_cfg);

	if (ret) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "CVI_SYS_GetBindbySrc() failed\n");
		return ret;
	}

	memcpy(pstBindDest, &bind_cfg.bind_dst, sizeof(MMF_BIND_DEST_S));
	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_GetVersion(MMF_VERSION_S *pstVersion)
{
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstVersion);

	snprintf(pstVersion->version, VERSION_NAME_MAXLEN, "%s-%s", MMF_VERSION, SDK_VER);
	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_GetChipId(CVI_U32 *pu32ChipId)
{
	static CVI_U32 id = 0xffffffff;
	int fd;

	if (id == 0xffffffff) {
		CVI_U32 tmp = 0;

		fd = get_sys_fd();
		if (fd == -1) {
			CVI_TRACE_SYS(CVI_DBG_ERR, "Can't open device, cvi-sys.\n");
			return CVI_ERR_SYS_NOTREADY;
		}

		if (ioctl(fd, SYS_IOC_READ_CHIP_ID, &tmp) < 0) {
			CVI_TRACE_SYS(CVI_DBG_ERR, "ioctl SYS_IOC_READ_CHIP_ID failed\n");
			return CVI_FAILURE;
		}

		id = tmp;
	}

	*pu32ChipId = id;
	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_GetPowerOnReason(CVI_U32 *pu32PowerOnReason)
{
	int fd;
	CVI_U32 ret_val = 0x0;
	CVI_U32 reason = 0x0;

	fd = get_sys_fd();
	if (fd == -1) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Can't open device, cvi-sys.\n");
		return CVI_ERR_SYS_NOTREADY;
	}

	if (ioctl(fd, SYS_IOC_READ_CHIP_PWR_ON_REASON, &reason) < 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "SYS_IOC_READ_CHIP_PWR_ON_REASON failed\n");
		return CVI_FAILURE;
	}

	switch (reason) {
	case E_CHIP_PWR_ON_COLDBOOT:
		ret_val = CVI_COLDBOOT;
	break;
	case E_CHIP_PWR_ON_WDT:
		ret_val = CVI_WDTBOOT;
	break;
	case E_CHIP_PWR_ON_SUSPEND:
		ret_val = CVI_SUSPENDBOOT;
	break;
	case E_CHIP_PWR_ON_WARM_RST:
		ret_val = CVI_WARMBOOT;
	break;
	default:
		CVI_TRACE_SYS(CVI_DBG_ERR, "unknown reason (%#x)\n", reason);
		return CVI_ERR_SYS_NOT_PERM;
	break;
	}

	*pu32PowerOnReason = ret_val;
	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_GetChipVersion(CVI_U32 *pu32ChipVersion)
{
	static CVI_U32 version = 0xffffffff;
	int fd;

	if (version == 0xffffffff) {
		CVI_U32 tmp = 0;

		fd = get_sys_fd();
		if (fd == -1) {
			CVI_TRACE_SYS(CVI_DBG_ERR, "Can't open device, cvi-sys.\n");
			return CVI_ERR_SYS_NOTREADY;
		}

		if (ioctl(fd, SYS_IOC_READ_CHIP_VERSION, &tmp) < 0) {
			CVI_TRACE_SYS(CVI_DBG_ERR, "ioctl SYS_IOC_READ_CHIP_VERSION failed\n");
			return CVI_FAILURE;
		}

		switch (tmp) {
		case E_CHIPVERSION_U01:
			version = CVIU01;
		break;
		case E_CHIPVERSION_U02:
			version = CVIU02;
		break;
		default:
			CVI_TRACE_SYS(CVI_DBG_ERR, "unknown version(%#x)\n", tmp);
			return CVI_ERR_SYS_NOT_PERM;
		break;
		}
	}

	*pu32ChipVersion = version;
	return CVI_SUCCESS;
}

void *CVI_SYS_Mmap(CVI_U64 u64PhyAddr, CVI_U32 u32Size)
{
	CVI_SYS_DevMem_Open();

	return devm_map(devm_fd, u64PhyAddr, u32Size);
}

/* CVI_SYS_MmapCache - mmap the physical address to cached virtual-address
 *
 * @param pu64PhyAddr: the phy-address of the buffer
 * @param u32Size: the length of the buffer
 * @return virtual-address if success; 0 if fail.
 */
void *CVI_SYS_MmapCache(CVI_U64 u64PhyAddr, CVI_U32 u32Size)
{
	CVI_SYS_DevMem_Open();

	void *addr = devm_map(devm_cached_fd, u64PhyAddr, u32Size);

	if (addr)
		CVI_SYS_IonInvalidateCache(u64PhyAddr, addr, u32Size);
	return addr;
}

CVI_S32 CVI_SYS_Munmap(void *pVirAddr, CVI_U32 u32Size)
{
	devm_unmap(pVirAddr, u32Size);
	return CVI_SUCCESS;
}

CVI_S32 ionMalloc(struct sys_ion_data *para)
{
	CVI_S32 fd = -1;
	CVI_S32 ret;

	if ((fd = get_base_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;


	ret = ioctl(fd, BASE_ION_ALLOC, para);
	if (ret) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "ioctl BASE_ION_ALLOC failed\n");
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 ionFree(struct sys_ion_data *para)
{
	CVI_S32 fd = -1;
	CVI_S32 ret;

	if ((fd = get_base_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;

	ret = ioctl(fd, BASE_ION_FREE, para);
	if (ret) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "ioctl BASE_ION_FREE failed\n");
		return ret;
	}

	return CVI_SUCCESS;
}

static CVI_S32 _SYS_IonAlloc(CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr,
			     CVI_U32 u32Len, CVI_BOOL cached, const CVI_CHAR *name)
{
	struct sys_ion_data ion_data;

	ion_data.size = u32Len;
	ion_data.cached = cached;
	// Set buffer as "anonymous" when user is passing null pointer.
	if (name)
		strncpy((char *)(ion_data.name), name, MAX_ION_BUFFER_NAME);
	else
		strncpy((char *)(ion_data.name), "anonymous", MAX_ION_BUFFER_NAME);

	if (ionMalloc(&ion_data) != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "alloc failed.\n");
		return CVI_ERR_SYS_NOMEM;
	}

	*pu64PhyAddr = ion_data.addr_p;

	if (ppVirAddr) {
		if (cached)
			*ppVirAddr = CVI_SYS_MmapCache(*pu64PhyAddr, u32Len);
		else
			*ppVirAddr = CVI_SYS_Mmap(*pu64PhyAddr, u32Len);
		if (*ppVirAddr == NULL) {
			ionFree(&ion_data);
			CVI_TRACE_SYS(CVI_DBG_ERR, "mmap failed. (%s)\n", strerror(errno));
			return CVI_ERR_SYS_REMAPPING;
		}
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_IonAlloc(CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr, const CVI_CHAR *strName, CVI_U32 u32Len)
{
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pu64PhyAddr);

	return _SYS_IonAlloc(pu64PhyAddr, ppVirAddr, u32Len, CVI_FALSE, strName);
}

/* CVI_SYS_IonAlloc_Cached - acquire buffer of u32Len from ion
 *
 * @param pu64PhyAddr: the phy-address of the buffer
 * @param ppVirAddr: the cached vir-address of the buffer
 * @param strName: the name of the buffer
 * @param u32Len: the length of the buffer acquire
 * @return CVI_SUCCES if ok
 */
CVI_S32 CVI_SYS_IonAlloc_Cached(CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr,
				 const CVI_CHAR *strName, CVI_U32 u32Len)
{
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pu64PhyAddr);

	return _SYS_IonAlloc(pu64PhyAddr, ppVirAddr, u32Len, CVI_TRUE, strName);
}

CVI_S32 CVI_SYS_IonFree(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr)
{
	struct sys_ion_data ion_data;
	int ret;

	ion_data.addr_p = u64PhyAddr;
	ret = ionFree(&ion_data);
	if (ret) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "ionFree failed\n");
		return ret;
	}
	if (pVirAddr)
		devm_unmap(pVirAddr, ion_data.size);

	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_IonFlushCache(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr, CVI_U32 u32Len)
{
	CVI_S32 fd = -1;
	CVI_S32 ret = CVI_SUCCESS;
	struct sys_cache_op cache_cfg;

	if ((fd = get_base_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;

	if (pVirAddr == NULL) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "pVirAddr Null.\n");
		return CVI_ERR_SYS_NULL_PTR;
	}

	cache_cfg.addr_p = u64PhyAddr;
	cache_cfg.addr_v = pVirAddr;
	cache_cfg.size = u32Len;

	ret = ioctl(fd, BASE_CACHE_FLUSH, &cache_cfg);
	if (ret < 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "ion flush err.\n");
		ret = CVI_ERR_SYS_NOTREADY;
	}
	return ret;
}

CVI_S32 CVI_SYS_IonInvalidateCache(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr, CVI_U32 u32Len)
{
	CVI_S32 fd = -1;
	CVI_S32 ret = CVI_SUCCESS;
	struct sys_cache_op cache_cfg;

	if ((fd = get_base_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;

	if (pVirAddr == NULL) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "pVirAddr Null.\n");
		return CVI_ERR_SYS_NULL_PTR;
	}

	cache_cfg.addr_p = u64PhyAddr;
	cache_cfg.addr_v = pVirAddr;
	cache_cfg.size = u32Len;

	ret = ioctl(fd, BASE_CACHE_INVLD, &cache_cfg);
	if (ret < 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "ion invalid err.\n");
		ret = CVI_ERR_SYS_NOTREADY;
	}
	return ret;
}

CVI_S32 CVI_SYS_SetVIVPSSMode(const VI_VPSS_MODE_S *pstVIVPSSMode)
{
	CVI_S32 fd = 0;
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstVIVPSSMode);

	if ((fd = get_sys_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;

	return ioctl(fd, SYS_IOC_SET_VIVPSSMODE, pstVIVPSSMode);
}

CVI_S32 CVI_SYS_GetVIVPSSMode(VI_VPSS_MODE_S *pstVIVPSSMode)
{
	CVI_S32 fd = 0;
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstVIVPSSMode);

	if ((fd = get_sys_fd()) == -1)
		return CVI_ERR_SYS_NOTREADY;

	return ioctl(fd, SYS_IOC_GET_VIVPSSMODE, pstVIVPSSMode);
}

const CVI_CHAR *CVI_SYS_GetModName(MOD_ID_E id)
{
	return CVI_GET_MOD_NAME(id);
}

CVI_S32 CVI_LOG_SetLevelConf(LOG_LEVEL_CONF_S *pstConf)
{
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstConf);

	if (pstConf->enModId >= CVI_ID_BUTT) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Invalid ModId(%d)\n", pstConf->enModId);
		return CVI_ERR_SYS_ILLEGAL_PARAM;
	}

	if (_SYS_MMAP() != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "_SYS_MMAP failed.\n");
		return CVI_FAILURE;
	}

	log_levels[pstConf->enModId] = pstConf->s32Level;
	return CVI_SUCCESS;
}

CVI_S32 CVI_LOG_GetLevelConf(LOG_LEVEL_CONF_S *pstConf)
{
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstConf);

	if (pstConf->enModId >= CVI_ID_BUTT) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Invalid ModId(%d)\n", pstConf->enModId);
		return CVI_ERR_SYS_ILLEGAL_PARAM;
	}

	if (_SYS_MMAP() != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "_SYS_MMAP failed.\n");
		return CVI_FAILURE;
	}

	pstConf->s32Level = log_levels[pstConf->enModId];
	return CVI_SUCCESS;
}

CVI_S32 CVI_SYS_GetCurPTS(CVI_U64 *pu64CurPTS)
{
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pu64CurPTS);

	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	*pu64CurPTS = ts.tv_sec*1000000 + ts.tv_nsec/1000;

	return CVI_SUCCESS;

}

CVI_S32 CVI_SYS_CDMACopy(CVI_U64 u64PhyDst, CVI_U64 u64PhySrc, CVI_U32 u32Len)
{
	CVI_S32 fd = 0;
	struct sys_cdma_copy cfg;

	if (u32Len == 0)
		return CVI_SUCCESS;

	fd = get_sys_fd();
	if (fd == -1) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Can't open device, cvi-sys.\n");
		return CVI_ERR_SYS_NOTREADY;
	}
	cfg.phy_addr_dst = u64PhyDst;
	cfg.phy_addr_src = u64PhySrc;
	cfg.len = u32Len;

	return ioctl(fd, SYS_IOC_CDMA_COPY, &cfg);
}

CVI_S32 CVI_SYS_CDMACopy2D(const CVI_CDMA_2D_S *param)
{
	CVI_S32 fd = 0;

	fd = get_sys_fd();
	if (fd == -1) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Can't open device, cvi-sys.\n");
		return CVI_ERR_SYS_NOTREADY;
	}

	return ioctl(fd, SYS_IOC_CDMA_COPY2D, param);
}


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/param.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <math.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/mman.h>

#include "cvi_buffer.h"
#include "cvi_base.h"
#include "cvi_vpss.h"
#include "cvi_sys.h"
#include "gdc_mesh.h"
#include "dwa_mesh.h"
#include "vpss_ioctl.h"


#define CHECK_VPSS_GDC_FMT(grp, chn, fmt)									\
	do {													\
		if (!GDC_SUPPORT_FMT(fmt)) {		\
			CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) invalid PixFormat(%d) for GDC.\n"		\
				      , grp, chn, (fmt));							\
			return CVI_ERR_VPSS_ILLEGAL_PARAM;							\
		}												\
	} while (0)
#define CHECK_VPSS_DWA_FMT(grp, chn, fmt)									\
	do {													\
		if (!DWA_SUPPORT_FMT(fmt)) {		\
			CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) invalid PixFormat(%d) for DWA.\n"		\
				      , grp, chn, (fmt));							\
			return CVI_ERR_VPSS_ILLEGAL_PARAM;							\
		}												\
	} while (0)


struct cvi_gdc_mesh mesh[VPSS_MAX_GRP_NUM][VPSS_MAX_CHN_NUM];

static CVI_S32 vpss_fd = -1;
static pthread_mutex_t vpss_fd_lock = PTHREAD_MUTEX_INITIALIZER;


static CVI_S32 vpss_dev_close(CVI_VOID)
{
	pthread_mutex_lock(&vpss_fd_lock);
	close_device(&vpss_fd);
	pthread_mutex_unlock(&vpss_fd_lock);

	return CVI_SUCCESS;
}

CVI_S32 get_vpss_fd(CVI_VOID)
{
	pthread_mutex_lock(&vpss_fd_lock);
	if (vpss_fd <= 0) {
		if (open_device(VPSS_DEV_NAME, &vpss_fd) == -1) {
			perror("VPSS open fail\n");
			vpss_fd = -1;
		}
	}
	pthread_mutex_unlock(&vpss_fd_lock);

	return vpss_fd;
}

static inline CVI_S32 CHECK_VPSS_GRP_VALID(VPSS_GRP grp)
{
	if ((grp >= VPSS_MAX_GRP_NUM) || (grp < 0)) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "VpssGrp(%d) exceeds Max(%d)\n", grp, VPSS_MAX_GRP_NUM);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}
	return CVI_SUCCESS;
}

static inline CVI_S32 CHECK_VPSS_CHN_VALID(VPSS_CHN VpssChn)
{
	if ((VpssChn >= VPSS_MAX_CHN_NUM) || (VpssChn < 0)) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Chn(%d) invalid.\n", VpssChn);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}
	return CVI_SUCCESS;
}


static VPSS_BIN_DATA vpss_bin_data[VPSS_MAX_GRP_NUM];
static CVI_BOOL g_bLoadBinDone = CVI_FALSE;

VPSS_BIN_DATA *get_vpssbindata_addr(void)
{
	return vpss_bin_data;
}

CVI_BOOL get_loadbin_state(void)
{
	return g_bLoadBinDone;
}

CVI_VOID set_loadbin_state(CVI_BOOL done)
{
	g_bLoadBinDone = done;
}

static CVI_S32 _vpss_update_rotation_mesh(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
	ROTATION_E enRotation, CVI_U32 u32Width, CVI_U32 u32Height)
{
#ifdef ARCH_CV183X
	CVI_U64 paddr, paddr_old;
	CVI_VOID *vaddr, *vaddr_old;
	struct cvi_gdc_mesh *pmesh = &mesh[VpssGrp][VpssChn];

	// acquire memory space for mesh.
	if (CVI_SYS_IonAlloc_Cached(&paddr, &vaddr, "vpss_mesh", CVI_GDC_MESH_SIZE_ROT) != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) Can't acquire memory for rotation mesh.\n"
			, VpssGrp, VpssChn);
		return CVI_ERR_VPSS_NOMEM;
	}

	SIZE_S in_size, out_size;

	in_size.u32Width = u32Width;
	in_size.u32Height = u32Height;
	if (enRotation == ROTATION_180) {
		out_size = in_size;
	} else {
		out_size.u32Width = in_size.u32Height;
		out_size.c = in_size.u32Width;
	}
	mesh_gen_rotation(in_size, out_size, enRotation, paddr, vaddr);
	CVI_SYS_IonFlushCache(paddr, vaddr, CVI_GDC_MESH_SIZE_ROT);

	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "Grp(%d) Chn(%d) mesh base(%#"PRIx64") vaddr(%p)\n"
		, VpssGrp, VpssChn, paddr, vaddr);

	pthread_mutex_lock(&pmesh->lock);
	if (pmesh->paddr != 0) {
		paddr_old = pmesh->paddr;
		vaddr_old = pmesh->vaddr;
	} else {
		paddr_old = 0;
		vaddr_old = NULL;
	}
	pmesh->paddr = paddr;
	pmesh->vaddr = vaddr;
	pthread_mutex_unlock(&pmesh->lock);
	if (paddr_old)
		CVI_SYS_IonFree(paddr_old, vaddr_old);
#elif defined(ARCH_CV182X)
	struct cvi_gdc_mesh *pmesh = &mesh[VpssGrp][VpssChn];

	UNUSED(u32Width);
	UNUSED(u32Height);
	UNUSED(enRotation);
	// TODO: dummy settings
	pmesh->paddr = DEFAULT_MESH_PADDR;
#elif defined(__CV181X__) || defined(__CV180X__) || defined(__CV186X__)
	CVI_S32 fd = get_vpss_fd();
	struct cvi_gdc_mesh *pmesh = &mesh[VpssGrp][VpssChn];
	struct vpss_chn_rot_cfg cfg;

	UNUSED(u32Width);
	UNUSED(u32Height);
	// TODO: dummy settings
	pmesh->paddr = DEFAULT_MESH_PADDR;

	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.enRotation = enRotation;
	return vpss_set_chn_rotation(fd, &cfg);
#endif
	return CVI_SUCCESS;
}

static CVI_S32 _vpss_update_ldc_mesh(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
	const VPSS_LDC_ATTR_S *pstLDCAttr, CVI_U32 u32Width, CVI_U32 u32Height)
{
	CVI_U64 paddr = 0;
	CVI_VOID *vaddr;
	CVI_S32 s32Ret;
	char mesh_name[128];

	snprintf(mesh_name, 128, "vpss_%d_%d", VpssGrp, VpssChn);
	if (pstLDCAttr->stAttr.bEnHWLDC)
		s32Ret = CVI_GDC_GenLDCMesh(u32Width, u32Height, &pstLDCAttr->stAttr, mesh_name, &paddr, &vaddr);
	else
		s32Ret = CVI_DWA_GenLDCMesh(u32Width, u32Height, &pstLDCAttr->stAttr, mesh_name, &paddr, &vaddr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) gen mesh fail\n",
				VpssGrp, VpssChn);
		return s32Ret;
	}

	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "Grp(%d) Chn(%d) mesh base(%#"PRIx64") vaddr(%p)\n"
		      , VpssGrp, VpssChn, paddr, vaddr);

#if defined(__CV181X__) || defined(__CV180X__) || defined(__CV186X__)
	CVI_S32 fd = get_vpss_fd();
	struct vpss_chn_ldc_cfg cfg;

	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.stLDCAttr = *pstLDCAttr;
	cfg.meshHandle = paddr;
	return vpss_set_chn_ldc(fd, &cfg);
#endif

	return CVI_SUCCESS;
}

static CVI_S32 _vpss_update_fisheye_mesh(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
	const FISHEYE_ATTR_S *pstFishEyeAttr, ROTATION_E enRotation, CVI_U32 u32Width, CVI_U32 u32Height)
{
	CVI_U64 paddr = 0;
	CVI_VOID *vaddr;
	struct cvi_gdc_mesh *pmesh = &mesh[VpssGrp][VpssChn];
	CVI_S32 s32Ret;
	char mesh_name[128];

	snprintf(mesh_name, 128, "vpss_%d_%d", VpssGrp, VpssChn);
	if (pstFishEyeAttr->bEnable) {
		s32Ret = CVI_DWA_GenFishEyeMesh(u32Width, u32Height, pstFishEyeAttr, mesh_name, &paddr, &vaddr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) gen mesh fail\n",
					VpssGrp, VpssChn);
			return s32Ret;
		}

		pthread_mutex_lock(&pmesh->lock);
		pmesh->paddr = paddr;
		pmesh->vaddr = vaddr;
		pthread_mutex_unlock(&pmesh->lock);
	}

	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "Grp(%d) Chn(%d) mesh base(%#"PRIx64") vaddr(%p)\n", VpssGrp, VpssChn, paddr, vaddr);

#if defined(__CV181X__) || defined(__CV180X__) || defined(__CV186X__)
	CVI_S32 fd = get_vpss_fd();
	struct vpss_chn_fisheye_cfg cfg;

	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.enRotation = enRotation;
	cfg.stFishEyeAttr = *pstFishEyeAttr;
	cfg.meshHandle = paddr;
	return vpss_set_chn_fisheye(fd, &cfg);
#endif

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_Suspend(void)
{
	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "+\n");

	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "-\n");
	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_Resume(void)
{
	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "+\n");

	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "-\n");
	return CVI_SUCCESS;
}
/**************************************************************************
 *   Public APIs.
 **************************************************************************/
CVI_S32 CVI_VPSS_CreateGrp(VPSS_GRP VpssGrp, const VPSS_GRP_ATTR_S *pstGrpAttr)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_crt_grp_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstGrpAttr);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	cfg.VpssGrp = VpssGrp;
	memcpy(&cfg.stGrpAttr, pstGrpAttr, sizeof(cfg.stGrpAttr));

	s32Ret = vpss_create_grp(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) create group fail\n", VpssGrp);
		return s32Ret;
	}
	// for chn rotation, ldc mesh gen
	for (CVI_U8 i = 0; i < VPSS_MAX_CHN_NUM; ++i)
		pthread_mutex_init(&mesh[VpssGrp][i].lock, NULL);

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_DestroyGrp(VPSS_GRP VpssGrp)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = vpss_destroy_grp(fd, VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) destroy group fail\n", VpssGrp);
		return s32Ret;
	}
	for (CVI_U8 i = 0; i < VPSS_MAX_CHN_NUM; ++i)
		pthread_mutex_destroy(&mesh[VpssGrp][i].lock);

	return CVI_SUCCESS;
}

VPSS_GRP CVI_VPSS_GetAvailableGrp(void)
{
	CVI_S32 fd = get_vpss_fd();
	VPSS_GRP grp = VPSS_INVALID_GRP;

	vpss_get_available_grp(fd, &grp);

	return grp;
}

CVI_S32 CVI_VPSS_StartGrp(VPSS_GRP VpssGrp)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_str_grp_cfg cfg;

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	cfg.VpssGrp = VpssGrp;
	s32Ret = vpss_start_grp(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) start group fail\n",
				VpssGrp);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_StopGrp(VPSS_GRP VpssGrp)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = vpss_stop_grp(fd, VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) stop group fail\n", VpssGrp);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_ResetGrp(VPSS_GRP VpssGrp)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = vpss_reset_grp(fd, VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) reset group fail\n", VpssGrp);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetGrpAttr(VPSS_GRP VpssGrp, VPSS_GRP_ATTR_S *pstGrpAttr)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_grp_attr cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstGrpAttr);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;

	s32Ret = vpss_get_grp_attr(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) get grp attr fail\n", VpssGrp);
		return s32Ret;
	}

	memcpy(pstGrpAttr, &cfg.stGrpAttr, sizeof(*pstGrpAttr));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetGrpAttr(VPSS_GRP VpssGrp, const VPSS_GRP_ATTR_S *pstGrpAttr)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_grp_attr cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstGrpAttr);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	memcpy(&cfg.stGrpAttr, pstGrpAttr, sizeof(cfg.stGrpAttr));

	s32Ret = vpss_set_grp_attr(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) set grp attr fail\n",
				VpssGrp);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetGrpProcAmpCtrl(VPSS_GRP VpssGrp, PROC_AMP_E type, PROC_AMP_CTRL_S *ctrl)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_proc_amp_ctrl_cfg cfg = {0};

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, ctrl);

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	cfg.type = type;
	s32Ret = vpss_get_proc_amp_ctrl(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) get proc amp ctrl fail\n", VpssGrp);
		return s32Ret;
	}
	*ctrl = cfg.ctrl;

	return CVI_SUCCESS;
}

static void _vpss_proamp_2_csc(struct vpss_grp_csc_cfg *csc_cfg)
{
	// for grp proc-amp.
	CVI_S32 *proc_amp = csc_cfg->proc_amp;
	float h = (float)(proc_amp[PROC_AMP_HUE] - 50) * PI / 360;
	float b_off = (proc_amp[PROC_AMP_BRIGHTNESS] - 50) * 2.56;
	float C_gain = 1 + (proc_amp[PROC_AMP_CONTRAST] - 50) * 0.02;
	float S = 1 + (proc_amp[PROC_AMP_SATURATION] - 50) * 0.02;
	float A = cos(h) * C_gain * S;
	float B = sin(h) * C_gain * S;
	float C_diff, c_off, tmp;
	CVI_U8 sub_0_l, add_0_l, add_1_l, add_2_l;

	if (proc_amp[PROC_AMP_CONTRAST] > 50)
		C_diff = 256 / C_gain;
	else
		C_diff = 256 * C_gain;
	c_off = 128 - (C_diff/2);

	if (b_off < 0) {
		sub_0_l = abs(proc_amp[PROC_AMP_BRIGHTNESS] - 50) * 2.56;
		add_0_l = 0;
		add_1_l = 0;
		add_2_l = 0;
	} else {
		sub_0_l = 0;
		if ((C_gain * b_off) > 255) {
			add_0_l = 255;
		} else {
			add_0_l = C_gain * b_off;
		}
		add_1_l = add_0_l;
		add_2_l = add_0_l;
	}

	if (proc_amp[PROC_AMP_CONTRAST] > 50) {
		csc_cfg->sub[0] = sub_0_l + c_off;
		csc_cfg->add[0] = add_0_l;
		csc_cfg->add[1] = add_1_l;
		csc_cfg->add[2] = add_2_l;
	} else {
		csc_cfg->sub[0] = sub_0_l;
		csc_cfg->add[0] = add_0_l + c_off;
		csc_cfg->add[1] = add_1_l + c_off;
		csc_cfg->add[2] = add_2_l + c_off;
	}
	csc_cfg->sub[1] = 128;
	csc_cfg->sub[2] = 128;

	csc_cfg->coef[0][0] = C_gain * BIT(10);
	tmp = B * -1.402;
	csc_cfg->coef[0][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
	tmp = A * 1.402;
	csc_cfg->coef[0][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
	csc_cfg->coef[1][0] = C_gain * BIT(10);
	tmp = A * -0.344 + B * 0.714;
	csc_cfg->coef[1][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
	tmp = B * -0.344 + A * -0.714;
	csc_cfg->coef[1][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
	csc_cfg->coef[2][0] = C_gain * BIT(10);
	tmp = A * 1.772;
	csc_cfg->coef[2][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
	tmp = B * 1.772;
	csc_cfg->coef[2][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "coef[0][0]: %#4x coef[0][1]: %#4x coef[0][2]: %#4x\n"
		, csc_cfg->coef[0][0], csc_cfg->coef[0][1]
		, csc_cfg->coef[0][2]);
	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "coef[1][0]: %#4x coef[1][1]: %#4x coef[1][2]: %#4x\n"
		, csc_cfg->coef[1][0], csc_cfg->coef[1][1]
		, csc_cfg->coef[1][2]);
	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "coef[2][0]: %#4x coef[2][1]: %#4x coef[2][2]: %#4x\n"
		, csc_cfg->coef[2][0], csc_cfg->coef[2][1]
		, csc_cfg->coef[2][2]);
	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "sub[0]: %3d sub[1]: %3d sub[2]: %3d\n"
		, csc_cfg->sub[0], csc_cfg->sub[1], csc_cfg->sub[2]);
	CVI_TRACE_VPSS(CVI_DBG_DEBUG, "add[0]: %3d add[1]: %3d add[2]: %3d\n"
		, csc_cfg->add[0], csc_cfg->add[1], csc_cfg->add[2]);
}

CVI_S32 CVI_VPSS_GetGrpProcAmp(VPSS_GRP VpssGrp, PROC_AMP_E type, CVI_S32 *value)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_vpss_fd();
	struct vpss_proc_amp_cfg cfg = {0};

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, value);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (type >= PROC_AMP_MAX) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) ProcAmp type(%d) invalid.\n", VpssGrp, type);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}

	cfg.VpssGrp = VpssGrp;
	s32Ret = vpss_get_proc_amp(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) get proc amp fail\n", VpssGrp);
		return s32Ret;
	}
	*value = cfg.proc_amp[type];
	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetGrpProcAmp(VPSS_GRP VpssGrp, PROC_AMP_E type, const CVI_S32 value)
{
	CVI_S32 s32Ret;
	PROC_AMP_CTRL_S ctrl;
	struct vpss_grp_csc_cfg csc_cfg;
	struct vpss_proc_amp_cfg amp_cfg;
	CVI_S32 fd = get_vpss_fd();

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	if (type >= PROC_AMP_MAX) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) ProcAmp type(%d) invalid.\n", VpssGrp, type);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}

	CVI_VPSS_GetGrpProcAmpCtrl(VpssGrp, type, &ctrl);
	if ((value > ctrl.maximum) || (value < ctrl.minimum)) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) new value(%d) out of range(%d ~ %d).\n"
			, VpssGrp, value, ctrl.minimum, ctrl.maximum);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}

	amp_cfg.VpssGrp = VpssGrp;
	s32Ret = vpss_get_proc_amp(fd, &amp_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) get proc amp fail\n", VpssGrp);
		return s32Ret;
	}
	amp_cfg.proc_amp[type] = value;

	memset(&csc_cfg, 0, sizeof(csc_cfg));
	csc_cfg.VpssGrp = VpssGrp;
	memcpy(csc_cfg.proc_amp, amp_cfg.proc_amp, sizeof(csc_cfg.proc_amp));
	_vpss_proamp_2_csc(&csc_cfg);

	s32Ret = vpss_set_grp_csc(fd, &csc_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) set group csc fail\n", VpssGrp);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static CVI_VOID _vpss_check_normalize(VPSS_CHN_ATTR_S *pstChnAttr)
{
	if (pstChnAttr->stNormalize.bEnable) {
		for (CVI_U8 i = 0; i < 3; ++i) {
			if (pstChnAttr->stNormalize.factor[i] >= 1.0f) {
				pstChnAttr->stNormalize.factor[i] = 1.0f - 1.0f/8192;
				CVI_TRACE_VPSS(CVI_DBG_WARN, "factor%d replaced with max value 8191/8192\n", i);
			}
			if (pstChnAttr->stNormalize.factor[i] < (1.0f/8192)) {
				pstChnAttr->stNormalize.factor[i] = (1.0f/8192);
				CVI_TRACE_VPSS(CVI_DBG_WARN, "factor%d replaced with min value 1/8192\n", i);
			}
			if (pstChnAttr->stNormalize.mean[i] > 255.0f) {
				pstChnAttr->stNormalize.mean[i] = 255.0f;
				CVI_TRACE_VPSS(CVI_DBG_WARN, "mean%d replaced with max value 255\n", i);
			}
			if (pstChnAttr->stNormalize.mean[i] < 0) {
				pstChnAttr->stNormalize.mean[i] = 0;
				CVI_TRACE_VPSS(CVI_DBG_WARN, "mean%d replaced with min value 0\n", i);
			}
		}
	}
}

/* Chn Settings */
CVI_S32 CVI_VPSS_SetChnAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_chn_attr attr = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstChnAttr);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memcpy(&attr.stChnAttr,  pstChnAttr, sizeof(attr.stChnAttr));
	// Handle float poing in user space
	_vpss_check_normalize(&attr.stChnAttr);

	s32Ret = vpss_set_chn_attr(fd, &attr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set chn attr fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_chn_attr attr = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstChnAttr);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = vpss_get_chn_attr(fd, &attr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn attr fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	*pstChnAttr = attr.stChnAttr;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_EnableChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_en_chn_cfg cfg = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = vpss_enable_chn(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) enable fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_DisableChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_en_chn_cfg cfg = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};
	struct vpss_chn_ldc_cfg ldcCfg = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};
	CVI_CHAR mesh_name[128];

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = vpss_disable_chn(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) disable fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	snprintf(mesh_name, 128, "vpss_%d_%d", VpssGrp, VpssChn);
	vpss_get_chn_ldc(fd, &ldcCfg);
	if (ldcCfg.stLDCAttr.stAttr.bEnHWLDC)
		CVI_GDC_FreeCurTaskMesh(mesh_name);
	else
		CVI_DWA_FreeCurTaskMesh(mesh_name);

	if (mesh[VpssGrp][VpssChn].paddr && mesh[VpssGrp][VpssChn].vaddr
		&& mesh[VpssGrp][VpssChn].paddr != DEFAULT_MESH_PADDR) {
		CVI_SYS_IonFree(mesh[VpssGrp][VpssChn].paddr, mesh[VpssGrp][VpssChn].vaddr);
		mesh[VpssGrp][VpssChn].paddr = CVI_NULL;
		mesh[VpssGrp][VpssChn].vaddr = CVI_NULL;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetChnCrop(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CROP_INFO_S *pstCropInfo)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_chn_crop_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstCropInfo);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.stCropInfo = *pstCropInfo;

	s32Ret = vpss_set_chn_crop(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set chn crop fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnCrop(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CROP_INFO_S *pstCropInfo)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_chn_crop_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstCropInfo);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	s32Ret = vpss_get_chn_crop(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn crop fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	*pstCropInfo = cfg.stCropInfo;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_ShowChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_en_chn_cfg cfg;

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	s32Ret = vpss_show_chn(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) show chn fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_HideChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_en_chn_cfg cfg;

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	s32Ret = vpss_hide_chn(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) hide chn fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetGrpCrop(VPSS_GRP VpssGrp, VPSS_CROP_INFO_S *pstCropInfo)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_grp_crop_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstCropInfo);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;

	s32Ret = vpss_get_grp_crop(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) get crop fail\n", VpssGrp);
		return s32Ret;
	}

	*pstCropInfo = cfg.stCropInfo;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetGrpCrop(VPSS_GRP VpssGrp, const VPSS_CROP_INFO_S *pstCropInfo)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_grp_crop_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstCropInfo);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.stCropInfo = *pstCropInfo;

	s32Ret = vpss_set_grp_crop(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) set crop fail\n", VpssGrp);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SendFrame(VPSS_GRP VpssGrp, const VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_snd_frm_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVideoFrame);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	cfg.VpssGrp = VpssGrp;
	memcpy(&cfg.stVideoFrame, pstVideoFrame, sizeof(cfg.stVideoFrame));
	cfg.s32MilliSec = s32MilliSec;

	s32Ret = vpss_send_frame(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) send frame fail\n", VpssGrp);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SendChnFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn
	, const VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_chn_frm_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVideoFrame);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	memcpy(&cfg.stVideoFrame, pstVideoFrame, sizeof(cfg.stVideoFrame));
	cfg.s32MilliSec = s32MilliSec;

	s32Ret = vpss_send_chn_frame(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) send chn frame fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VIDEO_FRAME_INFO_S *pstFrameInfo,
			     CVI_S32 s32MilliSec)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_chn_frm_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstFrameInfo);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.s32MilliSec = s32MilliSec;

	s32Ret = vpss_get_chn_frame(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn frame fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}
	memcpy(pstFrameInfo, &cfg.stVideoFrame, sizeof(*pstFrameInfo));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_ReleaseChnFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_chn_frm_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstVideoFrame);
	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	memcpy(&cfg.stVideoFrame, pstVideoFrame, sizeof(cfg.stVideoFrame));

	s32Ret = vpss_release_chn_frame(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) release chn frame fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnFd(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	return fd;
}

CVI_S32 CVI_VPSS_CloseFd(void)
{
	vpss_dev_close();
	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetModParam(const VPSS_MOD_PARAM_S *pstModParam)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstModParam);

	s32Ret = vpss_set_mod_param(fd, pstModParam);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Set module param fail\n");
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetModParam(VPSS_MOD_PARAM_S *pstModParam)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstModParam);

	s32Ret = vpss_get_mod_param(fd, pstModParam);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Get module param fail\n");
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetChnRotation(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, ROTATION_E enRotation)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_vpss_fd();
	struct vpss_chn_attr attr = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};
	struct vpss_chn_ldc_cfg ldc_cfg = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = vpss_get_chn_attr(fd, &attr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn attr fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}
	CHECK_VPSS_GDC_FMT(VpssGrp, VpssChn, attr.stChnAttr.enPixelFormat);

	s32Ret = vpss_get_chn_ldc(fd, &ldc_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn LDC attr fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	if (enRotation == ROTATION_180) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "not support rotation(%d).\n", enRotation);
		return CVI_ERR_VI_NOT_SUPPORT;
	} else if (enRotation >= ROTATION_MAX) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) invalid rotation(%d).\n"
			, VpssGrp, VpssChn, enRotation);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}

	if (ldc_cfg.stLDCAttr.bEnable) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set chn rotation fail, please add rotation to ldc.\n", VpssGrp, VpssChn);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	} else
		return _vpss_update_rotation_mesh(VpssGrp, VpssChn, enRotation,
			attr.stChnAttr.u32Width, attr.stChnAttr.u32Height);
	return s32Ret;
}

CVI_S32 CVI_VPSS_GetChnRotation(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, ROTATION_E *penRotation)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_chn_rot_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, penRotation);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	s32Ret = vpss_get_chn_rotation(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn rotation fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	*penRotation = cfg.enRotation;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetChnAlign(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 u32Align)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_chn_align_cfg cfg;

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.u32Align = u32Align;

	s32Ret = vpss_set_chn_align(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set chn align fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnAlign(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 *pu32Align)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_chn_align_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pu32Align);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	s32Ret = vpss_get_chn_align(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn align fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	*pu32Align = cfg.u32Align;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetChnScaleCoefLevel(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_SCALE_COEF_E enCoef)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_chn_coef_level_cfg cfg;

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.enCoef = enCoef;

	s32Ret = vpss_set_coef_level(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set chn ScaleCoefLevel fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnScaleCoefLevel(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_SCALE_COEF_E *penCoef)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_chn_coef_level_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, penCoef);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	s32Ret = vpss_get_coef_level(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn ScaleCoefLevel fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	*penCoef = cfg.enCoef;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetChnDrawRect(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_DRAW_RECT_S *pstDrawRect)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_chn_draw_rect_cfg cfg;

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.stDrawRect = *pstDrawRect;

	s32Ret = vpss_set_draw_rect(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set draw rect fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnDrawRect(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_DRAW_RECT_S *pstDrawRect)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_chn_draw_rect_cfg cfg;

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	s32Ret = vpss_get_draw_rect(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get draw rect fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}
	*pstDrawRect = cfg.stDrawRect;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetChnConvert(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_CONVERT_S *pstConvert)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_chn_convert_cfg cfg;

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.stConvert = *pstConvert;

	s32Ret = vpss_set_convert(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set convert fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnConvert(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_CONVERT_S *pstConvert)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_chn_convert_cfg cfg;

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	s32Ret = vpss_set_convert(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get convert fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}
	*pstConvert = cfg.stConvert;

	return CVI_SUCCESS;
}

/* CVI_VPSS_SetChnYRatio: Modify the y ratio of chn output. Only work for yuv format.
 *
 * @param VpssGrp: The Vpss Grp to work.
 * @param VpssChn: The Vpss Chn to work.
 * @param YRatio: Output's Y will be sacled by this ratio.
 * @return: CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VPSS_SetChnYRatio(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_FLOAT YRatio)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_chn_yratio_cfg cfg;

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.YRatio = (CVI_U32)(YRatio * 100);

	s32Ret = vpss_set_chn_yratio(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set chn Y Ratio fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_GetChnYRatio(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_FLOAT *pYRatio)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_chn_yratio_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pYRatio);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	s32Ret = vpss_get_chn_yratio(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn Y Ratio fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}
	*pYRatio = (1.0f * cfg.YRatio) / 100.0;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetChnLDCAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VPSS_LDC_ATTR_S *pstLDCAttr)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_vpss_fd();
	struct vpss_chn_rot_cfg rot_cfg = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};
	struct vpss_chn_attr attr = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstLDCAttr);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = vpss_get_chn_attr(fd, &attr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn attr fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	if (pstLDCAttr->stAttr.bEnHWLDC)
		CHECK_VPSS_GDC_FMT(VpssGrp, VpssChn, attr.stChnAttr.enPixelFormat);
	else
		CHECK_VPSS_DWA_FMT(VpssGrp, VpssChn, attr.stChnAttr.enPixelFormat);

	s32Ret = vpss_get_chn_rotation(fd, &rot_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn rotation fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}
	if (rot_cfg.enRotation > 0) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) set chn ldc fail, please add rotation to ldc.\n", VpssGrp, VpssChn);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}

	if (pstLDCAttr->stAttr.enRotation == ROTATION_180) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "not support ldc rotation(%d).\n", pstLDCAttr->stAttr.enRotation);
		return CVI_ERR_VI_NOT_SUPPORT;
	} else if (pstLDCAttr->stAttr.enRotation >= ROTATION_MAX) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) invalid ldc rotation(%d).\n"
			, VpssGrp, VpssChn, pstLDCAttr->stAttr.enRotation);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}

	return _vpss_update_ldc_mesh(VpssGrp, VpssChn, pstLDCAttr,
				attr.stChnAttr.u32Width, attr.stChnAttr.u32Height);
}

CVI_S32 CVI_VPSS_GetChnLDCAttr(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VPSS_LDC_ATTR_S *pstLDCAttr)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_chn_ldc_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstLDCAttr);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	s32Ret = vpss_get_chn_ldc(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn LDC attr fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	memcpy(pstLDCAttr, &cfg.stLDCAttr, sizeof(*pstLDCAttr));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_SetChnFisheye(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const FISHEYE_ATTR_S *pstFishEyeAttr)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = get_vpss_fd();
	struct vpss_chn_rot_cfg rot_cfg = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};
	struct vpss_chn_attr attr = {.VpssGrp = VpssGrp, .VpssChn = VpssChn};
	struct vpss_grp_attr cfg = {.VpssGrp = VpssGrp};

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstFishEyeAttr);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = vpss_get_chn_attr(fd, &attr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn attr fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	if (pstFishEyeAttr->bEnable)
		CHECK_VPSS_DWA_FMT(VpssGrp, VpssChn, attr.stChnAttr.enPixelFormat);

	s32Ret = vpss_get_chn_rotation(fd, &rot_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn rotation fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	if (rot_cfg.enRotation != 0) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) not support rotation when fisheye\n", VpssGrp, VpssChn);
		return CVI_ERR_DWA_ILLEGAL_PARAM;
	}

	s32Ret = vpss_get_grp_attr(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get grp attr fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	return _vpss_update_fisheye_mesh(VpssGrp, VpssChn, pstFishEyeAttr, rot_cfg.enRotation,
				attr.stChnAttr.u32Width, attr.stChnAttr.u32Height);
}

CVI_S32 CVI_VPSS_GetChnFisheye(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, FISHEYE_ATTR_S *pstFishEyeAttr)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_chn_fisheye_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstFishEyeAttr);
	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;

	s32Ret = vpss_get_chn_fisheye(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get chn FishEye attr fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	memcpy(pstFishEyeAttr, &cfg.stFishEyeAttr, sizeof(*pstFishEyeAttr));

	return CVI_SUCCESS;
}

/* CVI_VPSS_SetGrpParamfromBin: Apply the settings of scene from bin
 *
 * @param VpssGrp: the vpss grp to apply
 * @param scene: the scene of settings stored in bin to use
 * @return: result of the API
 */
CVI_S32 CVI_VPSS_SetGrpParamfromBin(VPSS_GRP VpssGrp, CVI_U8 scene)
{
	CVI_S32 s32Ret;
	VPSS_BIN_DATA *pBinData;
	struct vpss_grp_csc_cfg csc_cfg = {0};
	CVI_S32 fd = get_vpss_fd();

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	if (scene > VPSS_MAX_GRP_NUM) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "scene(%d) is over max(%d)\n", scene, VPSS_MAX_GRP_NUM);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}
	if (get_loadbin_state()) {
		pBinData = get_vpssbindata_addr();
		csc_cfg.VpssGrp = VpssGrp;
		memcpy(csc_cfg.proc_amp, pBinData[scene].proc_amp, sizeof(csc_cfg.proc_amp));
		_vpss_proamp_2_csc(&csc_cfg);

		s32Ret = vpss_set_grp_csc(fd, &csc_cfg);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) set group csc fail\n", VpssGrp);
			return s32Ret;
		}
		CVI_TRACE_VPSS(CVI_DBG_INFO, "PqBin is exist, vpss grp param use pqbin value !!\n");
	} else {
		CVI_TRACE_VPSS(CVI_DBG_INFO, "PqBin is not find, vpss grp param use default !!\n");
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_AttachVbPool(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VB_POOL hVbPool)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_vb_pool_cfg cfg;

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	cfg.hVbPool = hVbPool;
	return vpss_attach_vbpool(fd, &cfg);
}

CVI_S32 CVI_VPSS_DetachVbPool(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_vb_pool_cfg cfg;

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(&cfg, 0, sizeof(cfg));
	cfg.VpssGrp = VpssGrp;
	cfg.VpssChn = VpssChn;
	return vpss_detach_vbpool(fd, &cfg);
}

CVI_S32 CVI_VPSS_GetRegionLuma(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, const VIDEO_REGION_INFO_S *pstRegionInfo,
								CVI_U64 *pu64LumaData, CVI_S32 s32MilliSec)
{
	CVI_S32 ret = 0;
	VIDEO_FRAME_INFO_S stVideoFrame;
	CVI_U8 *pstVirAddr;
	SIZE_S stSize;
	CVI_U32 u32X, u32Y, u32XStep, u32YStep, u32Num;
	CVI_U32 u32MainStride;
	CVI_S32 s32StartX, s32StartY;

	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pstRegionInfo);
	MOD_CHECK_NULL_PTR(CVI_ID_VPSS, pu64LumaData);

	ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (ret != CVI_SUCCESS)
		return ret;
	ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (ret != CVI_SUCCESS)
		return ret;

	s32StartX = pstRegionInfo->pstRegion->s32X;
	s32StartY = pstRegionInfo->pstRegion->s32Y;
	stSize.u32Width = pstRegionInfo->pstRegion->u32Width;
	stSize.u32Height = pstRegionInfo->pstRegion->u32Height;
	if ((s32StartX < 0) || (s32StartY < 0)) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "region info(%d %d %d %d) invalid.\n"
					, s32StartX, s32StartY, stSize.u32Width, stSize.u32Height);
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	}

	ret = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, s32MilliSec);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) get buf fail\n", VpssGrp, VpssChn);
		return CVI_ERR_VPSS_BUF_EMPTY;
	}

	if ((s32StartX + stSize.u32Width > stVideoFrame.stVFrame.u32Width) ||
		(s32StartY + stSize.u32Height > stVideoFrame.stVFrame.u32Height) ||
		((CVI_U32)s32StartX >= stVideoFrame.stVFrame.u32Width) ||
		((CVI_U32)s32StartY >= stVideoFrame.stVFrame.u32Height) ||
		(stSize.u32Width > stVideoFrame.stVFrame.u32Width) ||
		(stSize.u32Height > stVideoFrame.stVFrame.u32Height)) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "size(%d %d %d %d) out of range.\n"
					, s32StartX, s32StartY, stSize.u32Width, stSize.u32Height);
		ret = CVI_ERR_VPSS_ILLEGAL_PARAM;
		goto release_blk;
	}

	if (!IS_FMT_YUV(stVideoFrame.stVFrame.enPixelFormat)) {
		ret = CVI_ERR_VPSS_NOT_SUPPORT;
		CVI_TRACE_VPSS(CVI_DBG_ERR, "only support yuv-fmt(%d).\n"
					, stVideoFrame.stVFrame.enPixelFormat);
		goto release_blk;
	}

	size_t Luma_size = stVideoFrame.stVFrame.u32Length[0];

	pstVirAddr = CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0], Luma_size);
	if (pstVirAddr == NULL) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "mmap for stVideoFrame failed.\n");
		ret = CVI_FAILURE;
		goto release_blk;
	}

	u32MainStride = stVideoFrame.stVFrame.u32Stride[0];

	u32Num = 0;
	*pu64LumaData = 0;
	u32XStep = stSize.u32Width > 9 ? stSize.u32Width / 9 : 1;
	u32YStep = stSize.u32Height > 9 ? stSize.u32Height / 9 : 1;

	for (u32Y = s32StartY; u32Y < s32StartY + stSize.u32Height; u32Y += u32YStep) {
		for (u32X = s32StartX; u32X < (s32StartX + stSize.u32Width); u32X += u32XStep) {
			*pu64LumaData += *(pstVirAddr + u32X + u32Y * u32MainStride);
			u32Num++;
		}
	}

	for (u32X = s32StartX + u32XStep / 2; u32X < (s32StartX + stSize.u32Width); u32X += u32XStep) {
		for (u32Y = s32StartY + u32YStep / 2; u32Y < (s32StartY + stSize.u32Height); u32Y += u32YStep) {
			*pu64LumaData += *(pstVirAddr + u32X + u32Y * u32MainStride);
			u32Num++;
		}
	}

	*pu64LumaData = *pu64LumaData / u32Num;

	CVI_SYS_Munmap(pstVirAddr, Luma_size);
release_blk:
	if (CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, &stVideoFrame) != CVI_SUCCESS)
		return CVI_FAILURE;
	return ret;
}

CVI_S32 CVI_VPSS_TriggerSnapFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, CVI_U32 u32FrameCnt)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct vpss_snap_cfg cfg = {.VpssGrp = VpssGrp, .VpssChn = VpssChn, .frame_cnt = u32FrameCnt};

	s32Ret = CHECK_VPSS_GRP_VALID(VpssGrp);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_VPSS_CHN_VALID(VpssChn);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = vpss_trigger_snap_frame(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Grp(%d) Chn(%d) Trigger Snap Frame fail\n", VpssGrp, VpssChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VPSS_Stitch(CVI_U32 u32ChnNum, VPSS_STITCH_CHN_ATTR_S *pstInput,
			VPSS_STITCH_OUTPUT_ATTR_S *pstOutput, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	CVI_S32 fd = get_vpss_fd();
	CVI_S32 s32Ret;
	struct _vpss_stitch_cfg cfg;

	cfg.u32ChnNum = u32ChnNum;
	cfg.pstInput = pstInput;
	cfg.stOutput = *pstOutput;

	s32Ret = vpss_stitch(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "Vpss Stitch fail\n");
		return s32Ret;
	}
	memcpy(pstVideoFrame, &cfg.stVideoFrame, sizeof(*pstVideoFrame));

	return CVI_SUCCESS;
}


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/param.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <inttypes.h>
#include <math.h>
#include <sys/prctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cvi_defines.h>
#include <cvi_base.h>
#include <cvi_math.h>
#include <cvi_comm_vo.h>
#include <cvi_errno.h>
#include "vo_uapi.h"
#include "vo_ioctl.h"

#define CHECK_VO_DEV_VALID(VoDev)\
	do {\
		if ((VoDev >= VO_MAX_DEV_NUM) || (VoDev < 0)) {\
			CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) invalid.\n", VoDev);\
			return CVI_ERR_VO_INVALID_DEVID;\
		} \
	} while (0)

#define CHECK_VIDEO_LAYER_VALID(VoLayer)\
	do {\
		if ((VoLayer >= VO_MAX_VIDEO_LAYER_NUM) || (VoLayer < 0)) {\
			CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) invalid.\n", VoLayer);\
			return CVI_ERR_VO_INVALID_LAYERID;\
		} \
	} while (0)

#define CHECK_VO_CHN_VALID(VoLayer, VoChn)\
	do {\
		if ((VoLayer >= VO_MAX_VIDEO_LAYER_NUM) || (VoLayer < 0)) {\
			CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) invalid.\n", VoLayer);\
			return CVI_ERR_VO_INVALID_LAYERID;\
		} \
		if ((VoChn >= VO_MAX_CHN_NUM) || (VoChn < 0)) {\
			CVI_TRACE_VO(CVI_DBG_ERR, "VoChn(%d) invalid.\n", VoChn);\
			return CVI_ERR_VO_INVALID_CHNID;\
		} \
	} while (0)

#define CHECK_VO_WBC_VALID(VoWbc)\
	do {\
		if ((VoWbc >= VO_MAX_DEV_NUM) || (VoWbc < 0)) {\
			CVI_TRACE_VO(CVI_DBG_ERR, "VoWbc(%d) invalid.\n", VoWbc);\
			return CVI_ERR_VO_INVALID_DEVID;\
		} \
	} while (0)


struct vo_pm_s {
	VO_PM_OPS_S stOps;
	CVI_VOID	*pvData;
};

static pthread_once_t once0 = PTHREAD_ONCE_INIT;
static pthread_once_t once1 = PTHREAD_ONCE_INIT;

static CVI_S32 vo_fd = -1;
static pthread_mutex_t vo_fd_lock = PTHREAD_MUTEX_INITIALIZER;

static struct vo_pm_s apstVoPm[VO_MAX_DEV_NUM] = { 0 };

static CVI_S32 vo_dev_close(CVI_VOID)
{
	pthread_mutex_lock(&vo_fd_lock);
	close_device(&vo_fd);
	pthread_mutex_unlock(&vo_fd_lock);

	return CVI_SUCCESS;
}

CVI_S32 get_vo_fd(CVI_VOID)
{
	pthread_mutex_lock(&vo_fd_lock);
	if (vo_fd <= 0) {
		if (open_device(VO_DEV_NAME, &vo_fd) == -1) {
			perror("VO open fail\n");
			vo_fd = -1;
		}
	}
	pthread_mutex_unlock(&vo_fd_lock);

	return vo_fd;
}

static CVI_S32 _check_vo_exist(CVI_S32 *pfd)
{
	*pfd = get_vo_fd();
	if (*pfd <= 0) {
		CVI_TRACE_VO(CVI_DBG_ERR, "Maybe no VO dev?\n");
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}


/**************************************************************************
 *   Bin related APIs.
 **************************************************************************/
#define VO_BIN_GUARDMAGIC 0x12345678
static VO_BIN_INFO_S vo_bin_info[VO_MAX_DEV_NUM] = {
	{
		.gamma_info = {
			.s32VoDev = 0,
			.enable = CVI_FALSE,
			.osd_apply = CVI_FALSE,
			.value = {
				0,   3,   7,   11,  15,  19,  23,  27,
				31,  35,  39,  43,  47,  51,  55,  59,
				63,  67,  71,  75,  79,  83,  87,  91,
				95,  99,  103, 107, 111, 115, 119, 123,
				127, 131, 135, 139, 143, 147, 151, 155,
				159, 163, 167, 171, 175, 179, 183, 187,
				191, 195, 199, 203, 207, 211, 215, 219,
				223, 227, 231, 235, 239, 243, 247, 251,
				255
			}
		},
		.guard_magic = VO_BIN_GUARDMAGIC
	},
	{
		.gamma_info = {
			.s32VoDev = 1,
			.enable = CVI_FALSE,
			.osd_apply = CVI_FALSE,
			.value = {
				0,   3,   7,   11,  15,  19,  23,  27,
				31,  35,  39,  43,  47,  51,  55,  59,
				63,  67,  71,  75,  79,  83,  87,  91,
				95,  99,  103, 107, 111, 115, 119, 123,
				127, 131, 135, 139, 143, 147, 151, 155,
				159, 163, 167, 171, 175, 179, 183, 187,
				191, 195, 199, 203, 207, 211, 215, 219,
				223, 227, 231, 235, 239, 243, 247, 251,
				255
			}
		},
		.guard_magic = VO_BIN_GUARDMAGIC
	}
};

static PROC_AMP_CTRL_S vp_proc_amp_ctrls[PROC_AMP_MAX] = {
	{ .minimum = 0, .maximum = 255, .step = 1, .default_value = 128 },
	{ .minimum = 0, .maximum = 255, .step = 1, .default_value = 128 },
	{ .minimum = 0, .maximum = 255, .step = 1, .default_value = 128 },
	{ .minimum = 0, .maximum = 359, .step = 1, .default_value = 0 },
};

VO_BIN_INFO_S *get_vo_bin_info_addr(void)
{
	return vo_bin_info;
}

CVI_U32 get_vo_bin_guardmagic_code(void)
{
	return VO_BIN_GUARDMAGIC;
}

void vo_layer_init0(void)
{
	CVI_S32 fd = -1, s32Ret;
	struct vo_layer_proc_amp_cfg cfg;
	VO_LAYER VoLayer = 0;

	if (_check_vo_exist(&fd)) {
		return;
	}

	cfg.VoLayer = VoLayer;
	for (CVI_U8 i = PROC_AMP_BRIGHTNESS; i < PROC_AMP_MAX; ++i)
		cfg.proc_amp[i] = vp_proc_amp_ctrls[i].default_value;

	s32Ret = vo_sdk_set_layer_proc_amp(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Set Layer proc_amp fail\n", VoLayer);
		return;
	}
}

void vo_layer_init1(void)
{
	CVI_S32 fd = -1, s32Ret;
	struct vo_layer_proc_amp_cfg cfg;
	VO_LAYER VoLayer = 1;

	if (_check_vo_exist(&fd)) {
		return;
	}

	cfg.VoLayer = VoLayer;
	for (CVI_U8 i = PROC_AMP_BRIGHTNESS; i < PROC_AMP_MAX; ++i)
		cfg.proc_amp[i] = vp_proc_amp_ctrls[i].default_value;

	s32Ret = vo_sdk_set_layer_proc_amp(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Set Layer proc_amp fail\n", VoLayer);
		return;
	}
}

CVI_S32 CVI_VO_Suspend(void)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = -1;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	s32Ret = vo_sdk_suspend(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "vo sdk suspend fail\n");
		return s32Ret;
	}

	for (VO_DEV VoDev = 0; VoDev < VO_MAX_DEV_NUM; ++VoDev) {
		if (apstVoPm[VoDev].stOps.pfnPanelSuspend) {
			s32Ret = apstVoPm[VoDev].stOps.pfnPanelSuspend(apstVoPm[VoDev].pvData);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_VO(CVI_DBG_ERR, "Panel[%d] suspend failed with %#x!\n", VoDev, s32Ret);
				return s32Ret;
			}
		}
	}

	CVI_TRACE_VO(CVI_DBG_DEBUG, "-\n");
	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_Resume(void)
{
	CVI_S32 s32Ret;
	CVI_S32 fd = -1;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	for (VO_DEV VoDev = 0; VoDev < VO_MAX_DEV_NUM; ++VoDev) {
		if (apstVoPm[VoDev].stOps.pfnPanelResume) {
			s32Ret = apstVoPm[VoDev].stOps.pfnPanelResume(apstVoPm[VoDev].pvData);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_VO(CVI_DBG_ERR, "Panel[%d] resume failed with %#x!\n", VoDev, s32Ret);
				return s32Ret;
			}
		}
	}

	s32Ret = vo_sdk_resume(fd);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "vo sdk resume fail\n");
		return s32Ret;
	}

	CVI_TRACE_VO(CVI_DBG_DEBUG, "-\n");
	return CVI_SUCCESS;
}
/**************************************************************************
 *   Public APIs.
 **************************************************************************/
CVI_S32 CVI_VO_SetPubAttr(VO_DEV VoDev, const VO_PUB_ATTR_S *pstPubAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstPubAttr);
	CHECK_VO_DEV_VALID(VoDev);

	CVI_S32 fd = -1, s32Ret;
	struct vo_pub_attr_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoDev = VoDev;
	CVI_TRACE_VO(CVI_DBG_DEBUG, "VoDev(%d) INTF type(0x%x) Sync(%d)\n", VoDev
		    , pstPubAttr->enIntfType, pstPubAttr->enIntfSync);

	memcpy(&cfg.stPubAttr, pstPubAttr, sizeof(VO_PUB_ATTR_S));

	s32Ret = vo_sdk_set_pubattr(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) Set Pub Attr fail\n", VoDev);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetPubAttr(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstPubAttr);
	CHECK_VO_DEV_VALID(VoDev);

	CVI_S32 fd = -1, s32Ret;
	struct vo_pub_attr_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoDev = VoDev;
	s32Ret = vo_sdk_get_pubattr(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) Get Pub Attr fail\n", VoDev);
		return s32Ret;
	}

	memcpy(pstPubAttr, &cfg.stPubAttr, sizeof(VO_PUB_ATTR_S));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetHDMIParam(VO_DEV VoDev, const VO_HDMI_PARAM_S *pstHDMIParam)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstHDMIParam);
	CHECK_VO_DEV_VALID(VoDev);

	CVI_S32 fd = -1, s32Ret;
	struct vo_hdmi_param_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoDev = VoDev;
	memcpy(&cfg.stHDMIParam, pstHDMIParam, sizeof(VO_HDMI_PARAM_S));

	if (VoDev != VO_HDMI_DEVICE) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) Not Support HDMI\n", VoDev);
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	s32Ret = vo_sdk_set_hdmiparam(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) Set HDMI param fail\n", VoDev);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetHDMIParam(VO_DEV VoDev, VO_HDMI_PARAM_S *pstHDMIParam)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstHDMIParam);
	CHECK_VO_DEV_VALID(VoDev);

	CVI_S32 fd = -1, s32Ret;
	struct vo_hdmi_param_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoDev = VoDev;

	if (VoDev != VO_HDMI_DEVICE) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) Not Support HDMI\n", VoDev);
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	s32Ret = vo_sdk_get_hdmiparam(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) Get HDMI param fail\n", VoDev);
		return s32Ret;
	}

	memcpy(pstHDMIParam, &cfg.stHDMIParam, sizeof(VO_HDMI_PARAM_S));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetLVDSParam(VO_DEV VoDev, const VO_LVDS_ATTR_S *pstLVDSParam)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstLVDSParam);
	CHECK_VO_DEV_VALID(VoDev);

	CVI_S32 fd = -1, s32Ret;
	struct vo_lvds_param_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoDev = VoDev;
	memcpy(&cfg.stLVDSParam, pstLVDSParam, sizeof(VO_LVDS_ATTR_S));

	s32Ret = vo_sdk_set_lvdsparam(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) Set LVDS param fail\n", VoDev);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetLVDSParam(VO_DEV VoDev, VO_LVDS_ATTR_S *pstLVDSParam)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstLVDSParam);
	CHECK_VO_DEV_VALID(VoDev);

	CVI_S32 fd = -1, s32Ret;
	struct vo_lvds_param_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoDev = VoDev;

	s32Ret = vo_sdk_get_lvdsparam(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) Get LVDS param fail\n", VoDev);
		return s32Ret;
	}

	memcpy(pstLVDSParam, &cfg.stLVDSParam, sizeof(VO_LVDS_ATTR_S));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetBTParam(VO_DEV VoDev, const VO_BT_ATTR_S *pstBTParam)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstBTParam);
	CHECK_VO_DEV_VALID(VoDev);

	CVI_S32 fd = -1, s32Ret;
	struct vo_bt_param_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoDev = VoDev;
	memcpy(&cfg.stBTParam, pstBTParam, sizeof(VO_BT_ATTR_S));

	s32Ret = vo_sdk_set_btparam(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) Get BT param fail\n", VoDev);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetBTParam(VO_DEV VoDev, VO_BT_ATTR_S *pstBTParam)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstBTParam);
	CHECK_VO_DEV_VALID(VoDev);

	CVI_S32 fd = -1, s32Ret;
	struct vo_bt_param_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoDev = VoDev;

	s32Ret = vo_sdk_get_btparam(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) Get BT param fail\n", VoDev);
		return s32Ret;
	}

	memcpy(pstBTParam, &cfg.stBTParam, sizeof(VO_BT_ATTR_S));

	return CVI_SUCCESS;
}


CVI_S32 CVI_VO_Enable(VO_DEV VoDev)
{
	CHECK_VO_DEV_VALID(VoDev);

	CVI_S32 fd = -1, s32Ret;
	struct vo_dev_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoDev = VoDev;
	s32Ret = vo_sdk_enable(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) Enable fail\n", VoDev);
		return s32Ret;
	}

#if 0 // skip in FPGA test
	//set vo parameter if bin has parameters
	CVI_VO_SetVOParamFromBin();
#endif
	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_Disable(VO_DEV VoDev)
{
	CHECK_VO_DEV_VALID(VoDev);

	CVI_S32 fd = -1, s32Ret;
	struct vo_dev_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoDev = VoDev;
	s32Ret = vo_sdk_disable(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) Disable fail\n", VoDev);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetVideoLayerCSC(VO_LAYER VoLayer, const VO_CSC_S *pstVideoCSC)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstVideoCSC);
	CHECK_VIDEO_LAYER_VALID(VoLayer);
	CVI_S32 fd = -1, s32Ret;
	struct vo_layer_csc_cfg csc_cfg;
	struct vo_layer_proc_amp_cfg proc_cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	csc_cfg.VoLayer = VoLayer;
	memcpy(&csc_cfg.stVideoCSC, pstVideoCSC, sizeof(VO_CSC_S));
	s32Ret = vo_sdk_set_layer_csc(fd, &csc_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Set layer csc fail\n", VoLayer);
		return s32Ret;
	}

	for (CVI_U8 i = PROC_AMP_BRIGHTNESS; i < PROC_AMP_MAX; ++i)
		proc_cfg.proc_amp[i] = vp_proc_amp_ctrls[i].default_value;

	proc_cfg.VoLayer = VoLayer;
	s32Ret = vo_sdk_set_layer_proc_amp(fd, &proc_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Set Layer proc_amp fail\n", VoLayer);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetVideoLayerCSC(VO_LAYER VoLayer, VO_CSC_S *pstVideoCSC)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstVideoCSC);
	CHECK_VIDEO_LAYER_VALID(VoLayer);
	CVI_S32 fd = -1, s32Ret;
	struct vo_layer_csc_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	s32Ret = vo_sdk_get_layer_csc(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Get Layer csc fail\n", VoLayer);
		return s32Ret;
	}
	memcpy(pstVideoCSC, &cfg.stVideoCSC, sizeof(VO_CSC_S));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_EnableVideoLayer(VO_LAYER VoLayer)
{
	CHECK_VIDEO_LAYER_VALID(VoLayer);
	CVI_S32 fd = -1, s32Ret;
	struct vo_video_layer_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	s32Ret = vo_sdk_enable_videolayer(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Enable Video Layer fail\n", VoLayer);
		return s32Ret;
	}

	if (VoLayer == 0) {
		pthread_once(&once0, vo_layer_init0);
	} else if (VoLayer == 1) {
		pthread_once(&once1, vo_layer_init1);
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_DisableVideoLayer(VO_LAYER VoLayer)
{
	CHECK_VIDEO_LAYER_VALID(VoLayer);
	CVI_S32 fd = -1, s32Ret;
	struct vo_video_layer_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	s32Ret = vo_sdk_disable_videolayer(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Disable Video Layer fail\n", VoLayer);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetVideoLayerAttr(VO_LAYER VoLayer, const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstLayerAttr);
	CHECK_VIDEO_LAYER_VALID(VoLayer);
	CVI_S32 fd = -1, s32Ret;
	struct vo_video_layer_attr_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	memcpy(&cfg.stLayerAttr, pstLayerAttr, sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = vo_sdk_set_videolayerattr(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Set Video Layer Attr fail\n", VoLayer);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetVideoLayerAttr(VO_LAYER VoLayer, VO_VIDEO_LAYER_ATTR_S *pstLayerAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstLayerAttr);
	CHECK_VIDEO_LAYER_VALID(VoLayer);
	CVI_S32 fd = -1, s32Ret;
	struct vo_video_layer_attr_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	s32Ret = vo_sdk_get_videolayerattr(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Get Video Layer Attr fail\n", VoLayer);
		return s32Ret;
	}
	memcpy(pstLayerAttr, &cfg.stLayerAttr, sizeof(VO_VIDEO_LAYER_ATTR_S));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetLayerProcAmpCtrl(VO_LAYER VoLayer, PROC_AMP_E type, PROC_AMP_CTRL_S *ctrl)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, ctrl);
	CHECK_VIDEO_LAYER_VALID(VoLayer);

	if (type >= PROC_AMP_MAX) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) ProcAmp type(%d) invalid.\n", VoLayer, type);
		return CVI_ERR_VO_ILLEGAL_PARAM;
	}

	*ctrl = vp_proc_amp_ctrls[type];

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetLayerProcAmp(VO_LAYER VoLayer, PROC_AMP_E type, CVI_S32 *value)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, value);
	CHECK_VIDEO_LAYER_VALID(VoLayer);
	CVI_S32 fd = -1, s32Ret;
	struct vo_layer_proc_amp_cfg cfg;

	if (type >= PROC_AMP_MAX) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) ProcAmp type(%d) invalid.\n", VoLayer, type);
		return CVI_ERR_VO_ILLEGAL_PARAM;
	}

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	s32Ret = vo_sdk_get_layer_proc_amp(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Get Layer proc_amp fail\n", VoLayer);
		return s32Ret;
	}

	*value = cfg.proc_amp[type];

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetLayerProcAmp(VO_LAYER VoLayer, PROC_AMP_E type, CVI_S32 value)
{
	CHECK_VIDEO_LAYER_VALID(VoLayer);
	CVI_S32 fd = -1, s32Ret;
	struct vo_layer_proc_amp_cfg cfg;
	struct vo_layer_csc_cfg layer_csc_cfg;

	if (type >= PROC_AMP_MAX) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) ProcAmp type(%d) invalid.\n", VoLayer, type);
		return CVI_ERR_VO_ILLEGAL_PARAM;
	}

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	if ((value > vp_proc_amp_ctrls[type].maximum) || (value < vp_proc_amp_ctrls[type].minimum)) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) new value(%d) out of range(%d ~ %d).\n"
			, VoLayer, value, vp_proc_amp_ctrls[type].minimum, vp_proc_amp_ctrls[type].maximum);
		return CVI_ERR_VO_ILLEGAL_PARAM;
	}

	cfg.VoLayer = VoLayer;
	s32Ret = vo_sdk_get_layer_proc_amp(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Get Layer proc_amp fail\n", VoLayer);
		return s32Ret;
	}

	cfg.proc_amp[type] = value;
	s32Ret = vo_sdk_set_layer_proc_amp(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Set Layer proc_amp fail\n", VoLayer);
		return s32Ret;
	}

	layer_csc_cfg.VoLayer = VoLayer;
	s32Ret = vo_sdk_get_layer_csc(fd, &layer_csc_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Get Layer csc fail\n", VoLayer);
		return s32Ret;
	}

	struct disp_csc_matrix csc_cfg;
	CVI_S32 b = (cfg.proc_amp[PROC_AMP_BRIGHTNESS] >> 1) - 64;
	float c = (float)cfg.proc_amp[PROC_AMP_CONTRAST] / 128;
	float s = (float)cfg.proc_amp[PROC_AMP_SATURATION] / 128;
	float h = (float)cfg.proc_amp[PROC_AMP_HUE] * 2 * PI / 360;
	float A = cos(h) * c * s;
	float B = sin(h) * c * s;
	float tmp;

	if (layer_csc_cfg.stVideoCSC.enCscMatrix == VO_CSC_MATRIX_601_LIMIT_YUV2RGB) {
		if (b > 0) {
			csc_cfg.sub[0] = 16;
			csc_cfg.add[0] = b;
			csc_cfg.add[1] = csc_cfg.add[0];
			csc_cfg.add[2] = csc_cfg.add[0];
		} else {
			csc_cfg.sub[0] = 16 + abs(b);
			csc_cfg.add[0] = 0;
			csc_cfg.add[1] = 0;
			csc_cfg.add[2] = 0;
		}
		csc_cfg.sub[1] = 128;
		csc_cfg.sub[2] = 128;

		csc_cfg.coef[0][0] = c * 1192;
		tmp = B * -1.596;
		csc_cfg.coef[0][1] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
		tmp = A * 1.596;
		csc_cfg.coef[0][2] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
		csc_cfg.coef[1][0] = c * 1192;
		tmp = A * -0.392 + B * 0.812;
		csc_cfg.coef[1][1] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
		tmp = B * -0.392 + A * -0.812;
		csc_cfg.coef[1][2] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
		csc_cfg.coef[2][0] = c * 1192;
		tmp = A * 2.016;
		csc_cfg.coef[2][1] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
		tmp = B * 2.016;
		csc_cfg.coef[2][2] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
	} else if (layer_csc_cfg.stVideoCSC.enCscMatrix == VO_CSC_MATRIX_601_FULL_YUV2RGB) {
		if (b > 0) {
			csc_cfg.sub[0] = 0;
			csc_cfg.add[0] = b;
			csc_cfg.add[1] = csc_cfg.add[0];
			csc_cfg.add[2] = csc_cfg.add[0];
		} else {
			csc_cfg.sub[0] = abs(b);
			csc_cfg.add[0] = 0;
			csc_cfg.add[1] = 0;
			csc_cfg.add[2] = 0;
		}
		csc_cfg.sub[1] = 128;
		csc_cfg.sub[2] = 128;

		csc_cfg.coef[0][0] = c * BIT(10);
		tmp = B * -1.4075;
		csc_cfg.coef[0][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
		tmp = A * 1.4075;
		csc_cfg.coef[0][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
		csc_cfg.coef[1][0] = c * BIT(10);
		tmp = A * -0.3455 + B * 0.7169;
		csc_cfg.coef[1][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
		tmp = B * -0.3455 + A * -0.7169;
		csc_cfg.coef[1][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
		csc_cfg.coef[2][0] = c * BIT(10);
		tmp = A * 1.779;
		csc_cfg.coef[2][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
		tmp = B * 1.779;
		csc_cfg.coef[2][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
	} else if (layer_csc_cfg.stVideoCSC.enCscMatrix == VO_CSC_MATRIX_709_LIMIT_YUV2RGB) {
		if (b > 0) {
			csc_cfg.sub[0] = 16;
			csc_cfg.add[0] = b;
			csc_cfg.add[1] = csc_cfg.add[0];
			csc_cfg.add[2] = csc_cfg.add[0];
		} else {
			csc_cfg.sub[0] = 16 + abs(b);
			csc_cfg.add[0] = 0;
			csc_cfg.add[1] = 0;
			csc_cfg.add[2] = 0;
		}
		csc_cfg.sub[1] = 128;
		csc_cfg.sub[2] = 128;

		csc_cfg.coef[0][0] = c * 1192;
		tmp = B * -1.792;
		csc_cfg.coef[0][1] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
		tmp = A * 1.792;
		csc_cfg.coef[0][2] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
		csc_cfg.coef[1][0] = c * 1192;
		tmp = A * -0.213 + B * 0.534;
		csc_cfg.coef[1][1] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
		tmp = B * -0.213 + A * -0.534;
		csc_cfg.coef[1][2] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
		csc_cfg.coef[2][0] = c * 1192;
		tmp = A * 2.114;
		csc_cfg.coef[2][1] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
		tmp = B * 2.114;
		csc_cfg.coef[2][2] = (tmp >= 0) ? tmp * 1192 : (CVI_U16)((-tmp) * 1192) | BIT(13);
	} else if (layer_csc_cfg.stVideoCSC.enCscMatrix == VO_CSC_MATRIX_709_FULL_YUV2RGB) {
		if (b > 0) {
			csc_cfg.sub[0] = 0;
			csc_cfg.add[0] = b;
			csc_cfg.add[1] = csc_cfg.add[0];
			csc_cfg.add[2] = csc_cfg.add[0];
		} else {
			csc_cfg.sub[0] = abs(b);
			csc_cfg.add[0] = 0;
			csc_cfg.add[1] = 0;
			csc_cfg.add[2] = 0;
		}
		csc_cfg.sub[1] = 128;
		csc_cfg.sub[2] = 128;

		csc_cfg.coef[0][0] = c * BIT(10);
		tmp = B * -1.5748;
		csc_cfg.coef[0][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
		tmp = A * 1.5748;
		csc_cfg.coef[0][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
		csc_cfg.coef[1][0] = c * BIT(10);
		tmp = A * -0.1868 + B * 0.468;
		csc_cfg.coef[1][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
		tmp = B * -0.1868 + A * -0.468;
		csc_cfg.coef[1][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
		csc_cfg.coef[2][0] = c * BIT(10);
		tmp = A * 1.856;
		csc_cfg.coef[2][1] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
		tmp = B * 1.856;
		csc_cfg.coef[2][2] = (tmp >= 0) ? tmp * BIT(10) : (CVI_U16)((-tmp) * BIT(10)) | BIT(13);
	}

	vo_set_csc(fd, &csc_cfg, VoLayer);
	CVI_TRACE_VO(CVI_DBG_DEBUG, "coef[0][0]: %#4x coef[0][1]: %#4x coef[0][2]: %#4x\n"
		, csc_cfg.coef[0][0], csc_cfg.coef[0][1]
		, csc_cfg.coef[0][2]);
	CVI_TRACE_VO(CVI_DBG_DEBUG, "coef[1][0]: %#4x coef[1][1]: %#4x coef[1][2]: %#4x\n"
		, csc_cfg.coef[1][0], csc_cfg.coef[1][1]
		, csc_cfg.coef[1][2]);
	CVI_TRACE_VO(CVI_DBG_DEBUG, "coef[2][0]: %#4x coef[2][1]: %#4x coef[2][2]: %#4x\n"
		, csc_cfg.coef[2][0], csc_cfg.coef[2][1]
		, csc_cfg.coef[2][2]);
	CVI_TRACE_VO(CVI_DBG_DEBUG, "sub[0]: %3d sub[1]: %3d sub[2]: %3d\n"
		, csc_cfg.sub[0], csc_cfg.sub[1], csc_cfg.sub[2]);
	CVI_TRACE_VO(CVI_DBG_DEBUG, "add[0]: %3d add[1]: %3d add[2]: %3d\n"
		, csc_cfg.add[0], csc_cfg.add[1], csc_cfg.add[2]);

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetPlayToleration(VO_LAYER VoLayer, CVI_U32 u32Toleration)
{
	CHECK_VIDEO_LAYER_VALID(VoLayer);
	CVI_S32 fd = -1, s32Ret;
	struct vo_layer_toleration_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.u32Toleration = u32Toleration;

	s32Ret = vo_sdk_set_layer_toleration(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Set Toleration fail\n", VoLayer);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetPlayToleration(VO_LAYER VoLayer, CVI_U32 *pu32Toleration)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pu32Toleration);
	CHECK_VIDEO_LAYER_VALID(VoLayer);
	CVI_S32 fd = -1, s32Ret;
	struct vo_layer_toleration_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;

	s32Ret = vo_sdk_get_layer_toleration(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Get Toleration fail\n", VoLayer);
		return s32Ret;
	}

	*pu32Toleration = cfg.u32Toleration;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetLayerPriority(VO_LAYER VoLayer, CVI_U32 u32Priority)
{
	CVI_S32 fd = -1, s32Ret;
	struct vo_layer_priority_cfg cfg;

	if ((VoLayer >= VO_MAX_LAYER_NUM) || (VoLayer < 0)) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) invalid.\n", VoLayer);
		return CVI_ERR_VO_INVALID_LAYERID;
	}

	if (VoLayer < VO_MAX_VIDEO_LAYER_NUM) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Video layer unsurpport set priority.\n", VoLayer);
		return CVI_ERR_VO_INVALID_LAYERID;
	}

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.u32Priority = u32Priority;

	s32Ret = vo_sdk_set_layer_priority(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Set Priority fail\n", VoLayer);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetLayerPriority(VO_LAYER VoLayer, CVI_U32 *pu32Priority)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pu32Priority);
	CVI_S32 fd = -1, s32Ret;
	struct vo_layer_priority_cfg cfg;

	if ((VoLayer >= VO_MAX_LAYER_NUM) || (VoLayer < 0)) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) invalid.\n", VoLayer);
		return CVI_ERR_VO_INVALID_LAYERID;
	}

	if (VoLayer < VO_MAX_VIDEO_LAYER_NUM) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Video layer unsurpport get priority.\n", VoLayer);
		return CVI_ERR_VO_INVALID_LAYERID;
	}

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;

	s32Ret = vo_sdk_get_layer_priority(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Get Priority fail\n", VoLayer);
		return s32Ret;
	}

	*pu32Priority = cfg.u32Priority;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_BindLayer(VO_LAYER VoLayer, VO_DEV VoDev)
{
	CHECK_VO_DEV_VALID(VoDev);
	CVI_S32 fd = -1, s32Ret;
	struct vo_video_layer_bind_cfg cfg;

	if ((VoLayer >= VO_MAX_LAYER_NUM) || (VoLayer < 0)) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) invalid.\n", VoLayer);
		return CVI_ERR_VO_INVALID_LAYERID;
	}

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoDev = VoDev;

	s32Ret = vo_sdk_bind_layer(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Bind VoDev(%d)\n", VoLayer, VoDev);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_UnBindLayer(VO_LAYER VoLayer, VO_DEV VoDev)
{
	CHECK_VO_DEV_VALID(VoDev);
	CVI_S32 fd = -1, s32Ret;
	struct vo_video_layer_bind_cfg cfg;

	if ((VoLayer >= VO_MAX_LAYER_NUM) || (VoLayer < 0)) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) invalid.\n", VoLayer);
		return CVI_ERR_VO_INVALID_LAYERID;
	}

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoDev = VoDev;

	s32Ret = vo_sdk_unbind_layer(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) UnBind VoDev(%d)\n", VoLayer, VoDev);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetChnAttr(VO_LAYER VoLayer, VO_CHN VoChn, const VO_CHN_ATTR_S *pstChnAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstChnAttr);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_attr_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	memcpy(&cfg.stChnAttr, pstChnAttr ,sizeof(VO_CHN_ATTR_S));
	s32Ret = vo_sdk_set_chnattr(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) Set Chn Attr fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetChnAttr(VO_LAYER VoLayer, VO_CHN VoChn, VO_CHN_ATTR_S *pstChnAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstChnAttr);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_attr_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;

	s32Ret = vo_sdk_get_chnattr(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) Get Chn Attr fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	memcpy(pstChnAttr, &cfg.stChnAttr, sizeof(VO_CHN_ATTR_S));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetChnParam(VO_LAYER VoLayer, VO_CHN VoChn, const VO_CHN_PARAM_S *pstChnParam)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstChnParam);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_param_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	memcpy(&cfg.stChnParam, pstChnParam ,sizeof(VO_CHN_PARAM_S));
	s32Ret = vo_sdk_set_chnparam(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) Set Chn Param fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetChnParam(VO_LAYER VoLayer, VO_CHN VoChn, VO_CHN_PARAM_S *pstChnParam)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstChnParam);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_param_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;

	s32Ret = vo_sdk_get_chnparam(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) Get Chn Param fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	memcpy(pstChnParam, &cfg.stChnParam, sizeof(VO_CHN_PARAM_S));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetChnZoomInWindow(VO_LAYER VoLayer, VO_CHN VoChn, const VO_CHN_ZOOM_ATTR_S *pstChnZoomAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstChnZoomAttr);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_zoom_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	memcpy(&cfg.stChnZoomAttr, pstChnZoomAttr ,sizeof(VO_CHN_ZOOM_ATTR_S));
	s32Ret = vo_sdk_set_chnzoom(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) Set Chn Zoom fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetChnZoomInWindow(VO_LAYER VoLayer, VO_CHN VoChn, VO_CHN_ZOOM_ATTR_S *pstChnZoomAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstChnZoomAttr);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_zoom_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;

	s32Ret = vo_sdk_get_chnzoom(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) Get Chn Zoom fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	memcpy(pstChnZoomAttr, &cfg.stChnZoomAttr, sizeof(VO_CHN_ZOOM_ATTR_S));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetChnBorder(VO_LAYER VoLayer, VO_CHN VoChn, const VO_CHN_BORDER_ATTR_S *pstChnBorder)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstChnBorder);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_border_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	memcpy(&cfg.stChnBorder, pstChnBorder ,sizeof(VO_CHN_BORDER_ATTR_S));
	s32Ret = vo_sdk_set_chnborder(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) Set Chn Border fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetChnBorder(VO_LAYER VoLayer, VO_CHN VoChn, VO_CHN_BORDER_ATTR_S *pstChnBorder)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstChnBorder);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_border_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;

	s32Ret = vo_sdk_get_chnborder(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) Get Chn Border fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	memcpy(pstChnBorder, &cfg.stChnBorder, sizeof(VO_CHN_BORDER_ATTR_S));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetChnMirror(VO_LAYER VoLayer, VO_CHN VoChn, VO_CHN_MIRROR_TYPE enChnMirror)
{
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_mirror_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	cfg.enChnMirror = enChnMirror;
	s32Ret = vo_sdk_set_chnmirror(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) Set Chn Mirror fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetChnMirror(VO_LAYER VoLayer, VO_CHN VoChn, VO_CHN_MIRROR_TYPE *penChnMirror)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, penChnMirror);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_mirror_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;

	s32Ret = vo_sdk_get_chnmirror(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) Get Chn Mirror fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	*penChnMirror = cfg.enChnMirror;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetChnFrameRate(VO_LAYER VoLayer, VO_CHN VoChn, CVI_S32 s32ChnFrmRate)
{
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_frmrate_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	cfg.u32FrameRate = s32ChnFrmRate;
	s32Ret = vo_sdk_set_chn_frmrate(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) Set Chn FrmRate fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetChnFrameRate(VO_LAYER VoLayer, VO_CHN VoChn, CVI_S32 *ps32ChnFrmRate)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, ps32ChnFrmRate);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_frmrate_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;

	s32Ret = vo_sdk_get_chn_frmrate(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) Get Chn FrmRate fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	*ps32ChnFrmRate = cfg.u32FrameRate;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetChnFrame(VO_LAYER VoLayer, VO_CHN VoChn, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstVideoFrame);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_frame_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	cfg.s32MilliSec = s32MilliSec;

	s32Ret = vo_sdk_get_chn_frame(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) get chn frame fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	memcpy(pstVideoFrame, &cfg.stVideoFrame, sizeof(VIDEO_FRAME_INFO_S));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_ReleaseChnFrame(VO_LAYER VoLayer, VO_CHN VoChn, const VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstVideoFrame);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_frame_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	memcpy(&cfg.stVideoFrame, pstVideoFrame, sizeof(VIDEO_FRAME_INFO_S));

	s32Ret = vo_sdk_release_chn_frame(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) release chn frame fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetChnPTS(VO_LAYER VoLayer, VO_CHN VoChn, CVI_U64 *pu64ChnPTS)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pu64ChnPTS);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_pts_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;

	s32Ret = vo_sdk_get_chn_pts(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) get chn pts fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	*pu64ChnPTS = cfg.u64ChnPTS;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_QueryChnStatus(VO_LAYER VoLayer, VO_CHN VoChn, VO_QUERY_STATUS_S *pstStatus)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstStatus);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_status_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;

	s32Ret = vo_sdk_get_chn_status(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) get chn status fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	memcpy(pstStatus, &cfg.stStatus, sizeof(VO_QUERY_STATUS_S));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetScreenFrame(VO_LAYER VoLayer, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstVideoFrame);
	CHECK_VIDEO_LAYER_VALID(VoLayer);
	CVI_S32 fd = -1, s32Ret;
	struct vo_screen_frame cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.s32MilliSec = s32MilliSec;

	s32Ret = vo_sdk_get_screen_frame(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) get screen frame fail\n", VoLayer);
		return s32Ret;
	}

	memcpy(pstVideoFrame, &cfg.stVideoFrame, sizeof(VIDEO_FRAME_INFO_S));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_ReleaseScreenFrame(VO_LAYER VoLayer, const VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstVideoFrame);
	CHECK_VIDEO_LAYER_VALID(VoLayer);
	CVI_S32 fd = -1, s32Ret;
	struct vo_screen_frame cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	memcpy(&cfg.stVideoFrame, pstVideoFrame, sizeof(VIDEO_FRAME_INFO_S));

	s32Ret = vo_sdk_release_screen_frame(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) get screen frame fail\n", VoLayer);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetChnRecvThreshold(VO_LAYER VoLayer, VO_CHN VoChn, CVI_U32 u32Threshold)
{
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_threshold_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	cfg.u32Threshold = u32Threshold;

	s32Ret = vo_sdk_set_chn_threshold(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) Set Chn Threshold fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetChnRecvThreshold(VO_LAYER VoLayer, VO_CHN VoChn, CVI_U32 *pu32Threshold)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pu32Threshold);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_threshold_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;

	s32Ret = vo_sdk_get_chn_threshold(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) Get Chn Threshold fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	*pu32Threshold = cfg.u32Threshold;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetDisplayBufLen(VO_LAYER VoLayer, CVI_U32 u32BufLen)
{
	CHECK_VIDEO_LAYER_VALID(VoLayer);
	CVI_S32 fd = -1, s32Ret;
	struct vo_display_buflen_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.u32BufLen = u32BufLen;
	s32Ret = vo_sdk_set_displaybuflen(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Set Display BufLen (%d)fail\n", VoLayer, u32BufLen);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetDisplayBufLen(VO_LAYER VoLayer, CVI_U32 *pu32BufLen)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pu32BufLen);
	CHECK_VIDEO_LAYER_VALID(VoLayer);
	CVI_S32 fd = -1, s32Ret;
	struct vo_display_buflen_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	s32Ret = vo_sdk_get_displaybuflen(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Get Display BufLen (%d)fail\n", VoLayer, cfg.u32BufLen);
		return s32Ret;
	}

	*pu32BufLen = cfg.u32BufLen;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_EnableChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	s32Ret = vo_sdk_enable_chn(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Enable Chn(%d) fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_DisableChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	s32Ret = vo_sdk_disable_chn(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Disable Chn(%d) fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetChnRotation(VO_LAYER VoLayer, VO_CHN VoChn, ROTATION_E enRotation)
{
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_rotation_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	cfg.enRotation = enRotation;
	if (enRotation == ROTATION_180) {
		CVI_TRACE_VO(CVI_DBG_ERR, "not support rotation(%d).\n", enRotation);
		return CVI_ERR_VO_NOT_SUPPORT;
	} else if (enRotation >= ROTATION_MAX) {
		CVI_TRACE_VO(CVI_DBG_ERR, "invalid rotation(%d).\n", enRotation);
		return CVI_ERR_VO_ILLEGAL_PARAM;
	}
	s32Ret = vo_sdk_set_chnrotation(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Set Chn(%d) Rotation fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetChnRotation(VO_LAYER VoLayer, VO_CHN VoChn, ROTATION_E *penRotation)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, penRotation);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_rotation_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	s32Ret = vo_sdk_get_chnrotation(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Set Chn(%d) Rotation fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	*penRotation = cfg.enRotation;
	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SendFrame(VO_LAYER VoLayer, VO_CHN VoChn, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstVideoFrame);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_snd_frm_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	cfg.s32MilliSec = s32MilliSec;

	memcpy(&cfg.stVideoFrame, pstVideoFrame, sizeof(VIDEO_FRAME_INFO_S));
	s32Ret = vo_sdk_send_frame(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Chn(%d) send frame fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_ShowPattern(VO_DEV VoDev, VO_PATTERN_MODE PatternId)
{
	CHECK_VO_DEV_VALID(VoDev);
	CVI_S32 fd = -1, s32Ret;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	s32Ret = vo_set_pattern(fd, PatternId, VoDev);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) set Pattern failed.\n", VoDev);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_ClearChnBuf(VO_LAYER VoLayer, VO_CHN VoChn, CVI_BOOL bClrAll)
{
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_clear_chn_buf_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	cfg.bClrAll = bClrAll;
	s32Ret = vo_sdk_clearchnbuf(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Chn(%d) clean buf fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_ShowChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	s32Ret = vo_sdk_showchn(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) show chn failed.\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_HideChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	s32Ret = vo_sdk_hidechn(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) hide chn failed.\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_CloseFd(void)
{
	vo_dev_close();
	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_PauseChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CHECK_VO_CHN_VALID(VoLayer, VoChn);
	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	s32Ret = vo_sdk_pausechn(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) pause chn failed.\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_StepChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CHECK_VO_CHN_VALID(VoLayer, VoChn);

	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	s32Ret = vo_sdk_stepchn(fd, &cfg);
	if (s32Ret != 0) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) step chn failed.\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_RefreshChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CHECK_VO_CHN_VALID(VoLayer, VoChn);

	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	s32Ret = vo_sdk_refreshchn(fd, &cfg);
	if (s32Ret != 0) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) step chn failed.\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_ResumeChn(VO_LAYER VoLayer, VO_CHN VoChn)
{
	CHECK_VO_CHN_VALID(VoLayer, VoChn);

	CVI_S32 fd = -1, s32Ret;
	struct vo_chn_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	s32Ret = vo_sdk_resumechn(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) VoChn(%d) resume chn failed.\n", VoLayer, VoChn);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_Get_Panel_Status(VO_LAYER VoLayer, VO_CHN VoChn, CVI_U32 *is_init)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, is_init);
	CHECK_VO_CHN_VALID(VoLayer, VoChn);

	CVI_S32 fd = -1, s32Ret;
	struct vo_panel_status_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoLayer = VoLayer;
	cfg.VoChn = VoChn;
	s32Ret = vo_sdk_get_panelstatue(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoLayer(%d) Chn(%d) get panel status fail\n", VoLayer, VoChn);
		return s32Ret;
	}

	memcpy(&is_init, &cfg.is_init, sizeof(is_init));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_RegPmCallBack(VO_DEV VoDev, VO_PM_OPS_S *pstPmOps, void *pvData)
{
	CHECK_VO_DEV_VALID(VoDev);
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstPmOps);

	apstVoPm[VoDev].stOps = *pstPmOps;
	apstVoPm[VoDev].pvData = pvData;
	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_UnRegPmCallBack(VO_DEV VoDev)
{
	CHECK_VO_DEV_VALID(VoDev);

	memset(&apstVoPm[VoDev].stOps, 0, sizeof(apstVoPm[VoDev].stOps));
	apstVoPm[VoDev].pvData = NULL;
	return CVI_SUCCESS;
}

CVI_BOOL CVI_VO_IsEnabled(VO_DEV VoDev)
{
	CVI_S32 fd = -1, s32Ret;
	struct vo_dev_cfg cfg;

	if ((VoDev >= VO_MAX_DEV_NUM) || (VoDev < 0)) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) invalid.\n", VoDev);
		return 0;
	}

	if (_check_vo_exist(&fd)) {
		return CVI_FALSE;
	}

	cfg.VoDev = VoDev;
	s32Ret = vo_sdk_isenable(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) get vo dev status fail\n", VoDev);
		return CVI_FALSE;
	}

	return (CVI_BOOL)cfg.isEnable;
}

CVI_S32 CVI_VO_SetGammaInfo(VO_GAMMA_INFO_S *pinfo)
{
	CVI_S32 fd = -1, s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, pinfo);

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}
	if (!CVI_VO_IsEnabled(pinfo->s32VoDev)) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) not ready yet!\n", pinfo->s32VoDev);
		return CVI_ERR_VO_SYS_NOTREADY;
	}

	s32Ret = vo_set_gamma_ctrl(fd, pinfo, pinfo->s32VoDev);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "Device(%d) set gamma fail\n", pinfo->s32VoDev);
		return s32Ret;
	}

	memcpy(&vo_bin_info[pinfo->s32VoDev].gamma_info, pinfo, sizeof(VO_GAMMA_INFO_S));
	vo_bin_info[pinfo->s32VoDev].guard_magic = VO_BIN_GUARDMAGIC;
	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetGammaInfo(VO_GAMMA_INFO_S *pinfo)
{
	CVI_S32 fd = -1, s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_VO, pinfo);

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	if (!CVI_VO_IsEnabled(pinfo->s32VoDev)) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoDev(%d) not ready yet!\n", pinfo->s32VoDev);
		return CVI_ERR_VO_SYS_NOTREADY;
	}

	//calling HW
	s32Ret = vo_get_gamma_ctrl(fd, pinfo, pinfo->s32VoDev);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "Device(%d) get gamma fail\n", pinfo->s32VoDev);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetWbcSrc(VO_WBC VoWbc, const VO_WBC_SRC_S *pstWbcSrc)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstWbcSrc);
	CHECK_VO_WBC_VALID(VoWbc);
	CVI_S32 fd = -1, s32Ret;
	struct vo_wbc_src_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoWbc = VoWbc;
	memcpy(&cfg.stWbcSrc, pstWbcSrc ,sizeof(VO_WBC_SRC_S));

	s32Ret = vo_sdk_set_wbcsrc(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoWbc(%d) Set Wbc Src fail\n", VoWbc);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetWbcSrc(VO_WBC VoWbc, VO_WBC_SRC_S *pstWbcSrc)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstWbcSrc);
	CHECK_VO_WBC_VALID(VoWbc);
	CVI_S32 fd = -1, s32Ret;
	struct vo_wbc_src_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoWbc = VoWbc;

	s32Ret = vo_sdk_get_wbcsrc(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoWbc(%d) Get Wbc Src fail\n", VoWbc);
		return s32Ret;
	}

	memcpy(pstWbcSrc, &cfg.stWbcSrc, sizeof(VO_WBC_SRC_S));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_EnableWbc(VO_WBC VoWbc)
{
	CHECK_VO_WBC_VALID(VoWbc);
	CVI_S32 fd = -1, s32Ret;
	struct vo_wbc_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoWbc = VoWbc;
	s32Ret = vo_sdk_enable_wbc(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoWbc(%d) Enable fail\n", VoWbc);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_DisableWbc(VO_WBC VoWbc)
{
	CHECK_VO_WBC_VALID(VoWbc);
	CVI_S32 fd = -1, s32Ret;
	struct vo_wbc_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoWbc = VoWbc;
	s32Ret = vo_sdk_disable_wbc(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoWbc(%d) Disable fail\n", VoWbc);
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetWbcAttr(VO_WBC VoWbc, const VO_WBC_ATTR_S *pstWbcAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstWbcAttr);
	CHECK_VO_WBC_VALID(VoWbc);
	CVI_S32 fd = -1, s32Ret;
	struct vo_wbc_attr_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoWbc = VoWbc;
	memcpy(&cfg.stWbcAttr, pstWbcAttr, sizeof(VO_WBC_ATTR_S));
	s32Ret = vo_sdk_set_wbcattr(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoWbc(%d) Set Wbc Attr fail\n", VoWbc);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetWbcAttr(VO_WBC VoWbc, VO_WBC_ATTR_S *pstWbcAttr)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstWbcAttr);
	CHECK_VO_WBC_VALID(VoWbc);
	CVI_S32 fd = -1, s32Ret;
	struct vo_wbc_attr_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoWbc = VoWbc;
	s32Ret = vo_sdk_get_wbcattr(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoWbc(%d) Get Wbc Attr fail\n", VoWbc);
		return s32Ret;
	}
	memcpy(pstWbcAttr, &cfg.stWbcAttr, sizeof(VO_WBC_ATTR_S));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetWbcMode(VO_WBC VoWbc, VO_WBC_MODE_E enWbcMode)
{
	CHECK_VO_WBC_VALID(VoWbc);
	CVI_S32 fd = -1, s32Ret;
	struct vo_wbc_mode_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoWbc = VoWbc;
	cfg.enWbcMode = enWbcMode;

	s32Ret = vo_sdk_set_wbcmode(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoWbc(%d) Set Mode(%d) fail\n", VoWbc, enWbcMode);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetWbcMode(VO_WBC VoWbc, VO_WBC_MODE_E *penWbcMode)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, penWbcMode);
	CHECK_VO_WBC_VALID(VoWbc);
	CVI_S32 fd = -1, s32Ret;
	struct vo_wbc_mode_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoWbc = VoWbc;
	s32Ret = vo_sdk_get_wbcmode(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoWbc(%d) Get Mode fail\n", VoWbc);
		return s32Ret;
	}

	*penWbcMode = cfg.enWbcMode;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_SetWbcDepth(VO_WBC VoWbc, CVI_U32 u32Depth)
{
	CHECK_VO_WBC_VALID(VoWbc);
	CVI_S32 fd = -1, s32Ret;
	struct vo_wbc_depth_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoWbc = VoWbc;
	cfg.u32Depth = u32Depth;

	s32Ret = vo_sdk_set_wbcdepth(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoWbc(%d) Set Depth(%d) fail\n", VoWbc, u32Depth);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetWbcDepth(VO_WBC VoWbc, CVI_U32 *u32Depth)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, u32Depth);
	CHECK_VO_WBC_VALID(VoWbc);
	CVI_S32 fd = -1, s32Ret;
	struct vo_wbc_depth_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoWbc = VoWbc;
	s32Ret = vo_sdk_get_wbcdepth(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoWbc(%d) Get Depth fail\n", VoWbc);
		return s32Ret;
	}

	*u32Depth = cfg.u32Depth;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_GetWbcFrame(VO_WBC VoWbc, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstVideoFrame);
	CHECK_VO_WBC_VALID(VoWbc);
	CVI_S32 fd = -1, s32Ret;
	struct vo_wbc_frame_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoWbc = VoWbc;
	cfg.s32MilliSec = s32MilliSec;

	s32Ret = vo_sdk_get_wbcframe(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoWbc(%d) get wbc frame fail\n", VoWbc);
		return s32Ret;
	}

	memcpy(pstVideoFrame, &cfg.stVideoFrame, sizeof(VIDEO_FRAME_INFO_S));

	return CVI_SUCCESS;
}

CVI_S32 CVI_VO_ReleaseWbcFrame(VO_WBC VoWbc, const VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	MOD_CHECK_NULL_PTR(CVI_ID_VO, pstVideoFrame);
	CHECK_VO_WBC_VALID(VoWbc);
	CVI_S32 fd = -1, s32Ret;
	struct vo_wbc_frame_cfg cfg;

	if (_check_vo_exist(&fd)) {
		return CVI_ERR_VO_NOT_SUPPORT;
	}

	cfg.VoWbc = VoWbc;
	memcpy(&cfg.stVideoFrame, pstVideoFrame, sizeof(VIDEO_FRAME_INFO_S));

	s32Ret = vo_sdk_release_wbcframe(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VO(CVI_DBG_ERR, "VoWbc(%d) release wbc frame fail\n", VoWbc);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

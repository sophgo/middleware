#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/select.h>
#include <inttypes.h>

#include "cvi_buffer.h"
#include "cvi_base.h"
#include "cvi_vi.h"
#include "cvi_vb.h"
#include "cvi_sys.h"
#include "gdc_mesh.h"
#include "vi_ioctl.h"
#include "cvi_sns_ctrl.h"
#include "dump_register.h"


#define CHECK_VI_PIPEID_VALID(x)						\
	do {									\
		if ((x) > (VI_MAX_PIPE_NUM - 1) || (x) < 0) {			\
			CVI_TRACE_VI(CVI_DBG_ERR, " invalid pipe-id(%d)\n", x);	\
			return CVI_ERR_VI_INVALID_PIPEID;			\
		}								\
	} while (0)

#define CHECK_VI_DEVID_VALID(x)							\
	do {									\
		if ((x) > (VI_MAX_DEV_NUM - 1) || (x) < 0) {			\
			CVI_TRACE_VI(CVI_DBG_ERR, " invalid dev-id(%d)\n", x);	\
			return CVI_ERR_VI_INVALID_DEVID;			\
		}								\
	} while (0)

#define CHECK_VI_CHNID_VALID(x)							\
	do {									\
		if ((x) > (VI_MAX_CHN_NUM - 1) || (x) < 0) {			\
			CVI_TRACE_VI(CVI_DBG_ERR, " invalid chn-id(%d)\n", x);	\
			return CVI_ERR_VI_INVALID_CHNID;			\
		}								\
	} while (0)

#define CHECK_VI_NULL_PTR(ptr)							\
	do {									\
		if (ptr == NULL) {						\
			CVI_TRACE_VI(CVI_DBG_ERR, " Invalid null pointer\n");	\
			return CVI_ERR_VI_INVALID_NULL_PTR;			\
		}								\
	} while (0)

#define VDEV_CLOSED_CHK(_dev_type, _dev_id)					\
	if (_dev_type == VDEV_TYPE_ISP && vi_is_closed()) {						\
		CVI_TRACE_VI(CVI_DBG_ERR, "vi dev(%d) state incorrect.",	\
					_dev_id);			\
		return CVI_ERR_VI_SYS_NOTREADY;					\
	}

#define VIPIPE_TO_DEV(_pipe_id, _dev_id)					\
	do {									\
		if (_pipe_id == 0 || _pipe_id == 1)				\
			_dev_id = 0;						\
		else if (_pipe_id == 2 || _pipe_id == 3)			\
			_dev_id = 1;						\
	} while (0)

#define CHECK_VI_EXTCHNID_VALID(x)										\
	do {													\
		if (((x) < VI_EXT_CHN_START) || ((x) >= (VI_EXT_CHN_START + VI_MAX_EXT_CHN_NUM))) {		\
			CVI_TRACE_VI(CVI_DBG_ERR, " invalid extchn-id(%d)\n", x);				\
			return CVI_ERR_VI_INVALID_CHNID;							\
		}												\
	} while (0)

#define CHECK_VI_GDC_FMT(x)											\
	do {													\
		if (!((x) == PIXEL_FORMAT_NV21 ||								\
			(x) == PIXEL_FORMAT_YUYV || (x) == PIXEL_FORMAT_UYVY) ||				\
			((x) == PIXEL_FORMAT_YVYU) || ((x) == PIXEL_FORMAT_VYUY)) {				\
			CVI_TRACE_VI(CVI_DBG_ERR, "invalid PixFormat(%d) for gdc.\n", (x));			\
			return CVI_ERR_VI_INVALID_PARA;								\
		}												\
	} while (0)

static inline CVI_S32 CHECK_VI_CTX_NULL_PTR(void *ptr)
{
	CVI_S32 ret = CVI_SUCCESS;

	if (ptr == NULL) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Call SetDevAttr first\n");
		ret = CVI_ERR_VI_FAILED_NOTCONFIG;
	}

	return ret;
}

typedef CVI_VOID (*pfnChnMirrorFlip) (VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eChnMirrorFlip);
static pfnChnMirrorFlip s_pfnDevMirrorFlip[VI_MAX_DEV_NUM];

struct vi_pm_s {
	VI_PM_OPS_S	stOps;
	CVI_VOID	*pvData;
};
static struct vi_pm_s apstViPm[VI_MAX_DEV_NUM] = { 0 };

//static struct cvi_vi_ctx vi_ctx_bak;
//VI dma buffer addr
static CVI_U64 dmaBufpAddr;

//TODO wait robin replace;
struct cvi_gdc_mesh g_vi_mesh[VI_MAX_CHN_NUM];

struct vi_dbg_th_info_s {
	CVI_U8    th_enable;
	pthread_t vi_dbg_thread;
};
struct vi_dbg_th_info_s gViDbgTH;

static CVI_S32 vi_fd = -1;
static pthread_mutex_t vi_fd_lock = PTHREAD_MUTEX_INITIALIZER;

static CVI_S32 vi_dev_close(CVI_VOID)
{
	pthread_mutex_lock(&vi_fd_lock);
	close_device(&vi_fd);
	pthread_mutex_unlock(&vi_fd_lock);

	return CVI_SUCCESS;
}

CVI_S32 get_vi_fd(CVI_VOID)
{
	pthread_mutex_lock(&vi_fd_lock);
	if (vi_fd <= 0) {
		if (open_device(VI_DEV_NAME, &vi_fd) == -1) {
			perror("VI open fail\n");
			vi_fd = -1;
		}
	}
	pthread_mutex_unlock(&vi_fd_lock);

	return vi_fd;
}

CVI_S32 vi_is_closed()
{
	CVI_S32 s32Ret = CVI_FALSE;

	pthread_mutex_lock(&vi_fd_lock);
	if (vi_fd <= 0)
		s32Ret = CVI_TRUE;
	pthread_mutex_unlock(&vi_fd_lock);

	return s32Ret;
}

/**************************************************************************
 *   Internal APIs for vi only
 **************************************************************************/

static CVI_VOID *vi_dbg_handler(CVI_VOID *data)
{
	CVI_S32 fd = -1;
	fd_set rfds;
	//struct timeval tv;
	CVI_S32 ret = CVI_SUCCESS;
	UNUSED(data);

	prctl(PR_SET_NAME, "vi_dbg_handler");

	fd = get_vi_fd();

	gViDbgTH.th_enable = CVI_TRUE;

	while (gViDbgTH.th_enable) {
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);

		//tv.tv_sec = 10;
		//tv.tv_usec = 0;

		ret = select(fd + 1, &rfds, NULL, NULL, NULL);
		if (ret == -1) {
			if (errno == EINTR)
				continue;
			CVI_TRACE_VI(CVI_DBG_ERR, "vi_dbg_thread select error\n");
			break;
		}

		//Cat vi_dbg/mipi_rx if error
		if (FD_ISSET(fd, &rfds) && gViDbgTH.th_enable) {
			system("cat /proc/soph/mipi-rx");
			system("cat /proc/soph/vi_dbg");
		}
	}
	CVI_TRACE_VI(CVI_DBG_INFO, "-\n");

	pthread_exit(NULL);
}

#if defined(POSSIBLE_DEAD_CODE)
static CVI_S32 _vi_proc_mmap(void)
{
	CVI_S32 fd = get_vi_fd()

	if (gViCtx != NULL) {
		CVI_TRACE_VI(CVI_DBG_DEBUG, "vi proc has already done mmap\n");
		return CVI_SUCCESS;
	}

	gViCtx = (struct cvi_vi_ctx *)mmap(NULL, VI_SHARE_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (gViCtx == MAP_FAILED) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi proc mmap fail!\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 _vi_proc_unmap(void)
{
	if (gViCtx == NULL) {
		CVI_TRACE_VI(CVI_DBG_DEBUG, "VI proc no need to unmap\n");
		return CVI_SUCCESS;
	}

	munmap((void *)gViCtx, VI_SHARE_MEM_SIZE);

	gViCtx = NULL;

	return CVI_SUCCESS;
}

#endif

static CVI_S32 _vi_chn_enable_mirror_flip(VI_PIPE ViPipe, VI_CHN ViChn, bool bFlip, bool bMirror)
{
	ISP_SNS_MIRRORFLIP_TYPE_E eChnMirrorFlip;

	if (bMirror && bFlip)
		eChnMirrorFlip = ISP_SNS_MIRROR_FLIP;
	else if (bMirror)
		eChnMirrorFlip = ISP_SNS_MIRROR;
	else if (bFlip)
		eChnMirrorFlip = ISP_SNS_FLIP;
	else
		eChnMirrorFlip = ISP_SNS_NORMAL;

	if (eChnMirrorFlip != ISP_SNS_NORMAL && !s_pfnDevMirrorFlip[ViChn]) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI chn mirror/flip do not support this sensor.");
		return CVI_ERR_VI_NOT_SUPPORT;
	}

	if (s_pfnDevMirrorFlip[ViPipe])
		s_pfnDevMirrorFlip[ViPipe](ViPipe, eChnMirrorFlip);

	return CVI_SUCCESS;
}

CVI_S32 _vi_update_rotation_mesh(VI_PIPE ViPipe, VI_CHN ViChn, ROTATION_E enRotation)
{
	struct vi_chn_rot_cfg cfg;
	CVI_S32 fd = get_vi_fd();

	cfg.ViPipe = ViPipe;
	cfg.ViChn = ViChn;
	cfg.enRotation = enRotation;
	if (vi_sdk_set_chn_rotation(fd, &cfg) != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI Set Chn(%d) Rotation(%d) fail\n", ViChn, enRotation);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

// static CVI_S32 _vi_update_ldc_mesh(VI_PIPE ViPipe, VI_CHN ViChn,
// 	const VI_LDC_ATTR_S *pstLDCAttr, CVI_U32 u32Width, CVI_U32 u32Height)
// {
// 	CVI_U64 paddr;
// 	CVI_VOID *vaddr;
// 	CVI_S32 fd = get_vi_fd();
// 	struct vi_chn_ldc_cfg cfg;
// 	char mesh_name[128];
// 	CVI_S32 s32Ret = CVI_SUCCESS;
// 	struct cvi_gdc_mesh *pmesh = &g_vi_mesh[ViChn];

// 	cfg.ViPipe = ViPipe;
// 	cfg.ViChn = ViChn;
// 	cfg.stLDCAttr = *pstLDCAttr;

// 	snprintf(mesh_name, 128, "vi_%d_%d", ViPipe, ViChn);
// 	s32Ret = CVI_GDC_GenLDCMesh(u32Width, u32Height, &pstLDCAttr->stAttr,
// 			mesh_name, &paddr, &vaddr);
// 	if (s32Ret != CVI_SUCCESS) {
// 		CVI_TRACE_VI(CVI_DBG_ERR, "Chn(%d) gen mesh fail\n", ViChn);
// 		return s32Ret;
// 	}

// 	CVI_TRACE_VI(CVI_DBG_DEBUG, "ViPipe(%d) ViChn(%d) mesh base(%#"PRIx64") vaddr(%p)\n"
// 		, ViPipe, ViChn, paddr, vaddr);

// 	if (pmesh->paddr && pmesh->vaddr) {
// 		CVI_SYS_IonFree(pmesh->paddr, pmesh->vaddr);
// 		pmesh->paddr = 0;
// 		pmesh->vaddr = CVI_NULL;
// 	}

// 	pmesh->paddr = paddr;
// 	pmesh->vaddr = vaddr;

// 	cfg.meshHandle = paddr;
// 	if (vi_sdk_set_chn_ldc(fd, &cfg) != CVI_SUCCESS) {
// 		CVI_TRACE_VI(CVI_DBG_ERR, "VI Set Chn(%d) LDC fail\n", ViChn);
// 		return CVI_FAILURE;
// 	}

// 	return CVI_SUCCESS;
// }

CVI_S32 _cvi_vi_freeIonBuf(void)
{
	CVI_S32 ret = CVI_SUCCESS;

	if (dmaBufpAddr) {
		ret = CVI_SYS_IonFree(dmaBufpAddr, NULL);
		if (ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "Free Ion dmaBufAddr failed\n");
			return CVI_ERR_SYS_ILLEGAL_PARAM;
		}
	}

	CVI_TRACE_VI(CVI_DBG_DEBUG, "VI FreeIonBuf success\n");

	dmaBufpAddr = 0;
	return ret;
}

CVI_S32 _cvi_vi_getIonBuf(void)
{
	CVI_U64 pAddr = 0;
	CVI_S32 ret = CVI_SUCCESS;
	CVI_U32 size = 0;
	struct cvi_vi_dma_buf_info info = {.paddr = 0, .size = 0};

	CVI_S32 fd = get_vi_fd();

	//ioctl to isp driver to get size
	ret = vi_get_dma_size(fd, &size);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_get_dma_size ioctl failed\n");
		return CVI_ERR_VI_NOMEM;
	}

	if (size == 0) {
		//return success and size = 0, it's means yuv sensor;
		CVI_TRACE_VI(CVI_DBG_DEBUG, "yuv sensor not need dma size\n");
		return CVI_SUCCESS;
	}

	ret = CVI_SYS_IonAlloc_Cached(&pAddr, NULL, "VI_DMA_BUF", size);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI ion alloc size(%d) failed.\n", size);
		return CVI_ERR_VI_NOMEM;
	}

	//set paddr and size to isp driver
	info.paddr = pAddr;
	info.size  = size;
	ret = vi_set_dma_buf_info(fd, &info);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_set_dma_buf_info ioctl failed\n");
		CVI_SYS_IonFree(pAddr, NULL);
		return CVI_ERR_VI_NOT_SUPPORT;
	}

	dmaBufpAddr = pAddr;

	CVI_TRACE_VI(CVI_DBG_INFO, "VI ion alloc size(%d) success\n", info.size);

	return ret;
}

/**************************************************************************
 *   Internal APIs for other modules
 **************************************************************************/

CVI_S32 CVI_VI_Suspend(void)
{
#if defined(POSSIBLE_DEAD_CODE)
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U8  i = 0;

	if (CHECK_VI_CTX_NULL_PTR(gViCtx) != CVI_SUCCESS)
		return CVI_ERR_VI_FAILED_NOTCONFIG;

	gViCtx->vi_stt = VI_SUSPEND;

	memcpy(&vi_ctx_bak, gViCtx, sizeof(struct cvi_vi_ctx));

	for (i = 0; i < vi_ctx_bak.total_dev_num; i++) {
		s32Ret = CVI_VI_DisableChn(0, i);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "CVI_VI_DisableChn failed with %#x!\n", s32Ret);
			return s32Ret;
		}
		/* suspend sensor. */
		if (apstViPm[i].stOps.pfnSnsSuspend) {
			s32Ret = apstViPm[i].stOps.pfnSnsSuspend(apstViPm[i].pvData);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_VI(CVI_DBG_ERR, "sensor[%d] suspend failed with %#x!\n", i, s32Ret);
				return s32Ret;
			}
		}
		/* suspend mipi. */
		if (apstViPm[i].stOps.pfnMipiSuspend) {
			s32Ret = apstViPm[i].stOps.pfnMipiSuspend(apstViPm[i].pvData);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_VI(CVI_DBG_ERR, "mipi[%d] suspend failed with %#x!\n", i, s32Ret);
				return s32Ret;
			}
		}
	}

	for (i = 0; i < vi_ctx_bak.total_dev_num; i++) {
		s32Ret = CVI_VI_DestroyPipe(i);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "CVI_VI_DestroyPipe failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	for (i = 0; i < vi_ctx_bak.total_dev_num; i++) {
		s32Ret  = CVI_VI_DisableDev(i);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "CVI_VI_DisableDev failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	//CVI_SYS_VI_Close();
#endif
	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_Resume(void)
{
#if defined(POSSIBLE_DEAD_CODE)
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U8  i = 0;
	VI_DEV_ATTR_S	    stViDevAttr;
	VI_PIPE_ATTR_S	    stPipeAttr;
	VI_CHN_ATTR_S	    stChnAttr;

	//CVI_SYS_VI_Open();
	CVI_S32 fd = get_vi_fd();

	s32Ret = vi_set_clk(fd, 1);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Set isp clk ioctl\n");
		return CVI_FAILURE;
	}

	for (i = 0; i < vi_ctx_bak.total_dev_num; i++) {
		/* resume mipi. */
		if (apstViPm[i].stOps.pfnMipiResume) {
			s32Ret = apstViPm[i].stOps.pfnMipiResume(apstViPm[i].pvData);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_VI(CVI_DBG_ERR, "mipi[%d] resume failed with %#x!\n", i, s32Ret);
				return s32Ret;
			}
		}
		/* resume sensor. */
		if (apstViPm[i].stOps.pfnSnsResume) {
			s32Ret = apstViPm[i].stOps.pfnSnsResume(apstViPm[i].pvData);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_VI(CVI_DBG_ERR, "sensor[%d] resume failed with %#x!\n", i, s32Ret);
				return s32Ret;
			}
		}

		memcpy(&stViDevAttr, &vi_ctx_bak.devAttr[i], sizeof(VI_DEV_ATTR_S));

		s32Ret = CVI_VI_SetDevAttr(i, &stViDevAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "CVI_VI_SetDevAttr failed with %#x!\n", s32Ret);
			return s32Ret;
		}

		s32Ret = CVI_VI_EnableDev(i);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "CVI_VI_EnableDev failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	for (i = 0; i < vi_ctx_bak.total_dev_num; i++) {
		memcpy(&stPipeAttr, &vi_ctx_bak.pipeAttr[i], sizeof(VI_PIPE_ATTR_S));

		s32Ret = CVI_VI_CreatePipe(i, &stPipeAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "CVI_VI_CreatePipe failed with %#x!\n", s32Ret);
			return s32Ret;
		}

		s32Ret = CVI_VI_StartPipe(i);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "CVI_VI_StartPipe failed with %#x!\n", s32Ret);
			return s32Ret;
		}
	}

	for (i = 0; i < vi_ctx_bak.total_dev_num; i++) {
		memcpy(&stChnAttr, &vi_ctx_bak.chnAttr[i], sizeof(VI_CHN_ATTR_S));

		s32Ret = CVI_VI_SetChnAttr(0, i, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "CVI_VI_SetChnAttr failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}

		s32Ret = CVI_VI_EnableChn(0, i);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "CVI_VI_EnableChn failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}
	}

	//gViCtx->vi_stt = vi_prc_ctx->vi_stt = VI_RUNNING;
#endif
	return CVI_SUCCESS;
}

CVI_VOID CVI_VI_SetMotionLV(struct mlv_info mlevel_i)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	struct mlv_info_s mlv_i_tmp;

	CVI_S32 fd = get_vi_fd();

	if (s32Ret == CVI_SUCCESS) {
		mlv_i_tmp.sensor_num = mlevel_i.sensor_num;
		mlv_i_tmp.frm_num    = mlevel_i.frm_num;
		mlv_i_tmp.mlv        = mlevel_i.mlv;
		memcpy(mlv_i_tmp.mtable, mlevel_i.mtable, MO_TBL_SIZE);

		s32Ret = vi_sdk_set_motion_lv(fd, &mlv_i_tmp);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_motion_lv ioctl failed\n");
		}
	}
}

CVI_VOID CVI_VI_SET_DIS_INFO(struct dis_info dis_i)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	struct dis_info_s dis_i_tmp;

	CVI_S32 fd = get_vi_fd();

	if (s32Ret == CVI_SUCCESS) {
		dis_i_tmp.sensor_num    = dis_i.sensor_num;
		dis_i_tmp.frm_num       = dis_i.frm_num;
		dis_i_tmp.dis_i.start_x = dis_i.dis_i.start_x;
		dis_i_tmp.dis_i.start_y = dis_i.dis_i.start_y;
		dis_i_tmp.dis_i.end_x   = dis_i.dis_i.end_x;
		dis_i_tmp.dis_i.end_y   = dis_i.dis_i.end_y;

		s32Ret = vi_sdk_set_dis_info(fd, &dis_i_tmp);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_dis_info ioctl failed\n");
		}
	}
}

CVI_S32 CVI_VI_SetBypassFrm(CVI_U32 snr_num, CVI_U8 bypass_num)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = get_vi_fd();

	if (snr_num > VI_MAX_PIPE_NUM - 1) {
		CVI_TRACE_VI(CVI_DBG_ERR, " invalid dev-id(%d)\n", snr_num);
		return CVI_ERR_VI_INVALID_DEVID;
	}

	s32Ret = vi_sdk_set_bypass_frm(fd, snr_num, bypass_num);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_bypass_frm ioctl failed errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

/**************************************************************************
 *   Public APIs.
 **************************************************************************/
CVI_S32 CVI_VI_SetDevNum(CVI_U32 devNum)
{
	UNUSED(devNum);

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetDevNum(CVI_U32 *devNum)
{
	CVI_S32 fd = get_vi_fd();
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_NULL_PTR(devNum);

	s32Ret = vi_get_dev_num(fd, devNum);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_get_dev_num ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 CVI_VI_QueryDevStatus(VI_PIPE ViPipe)
{
	CVI_S32 fd = get_vi_fd();
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_BOOL bStatus;

	CHECK_VI_PIPEID_VALID(ViPipe);

	s32Ret = vi_sdk_get_dev_status(fd, ViPipe, &bStatus);
	if (s32Ret != CVI_SUCCESS)
		return CVI_FAILURE;

	return bStatus == CVI_TRUE ? CVI_SUCCESS : CVI_FAILURE;
}

CVI_S32 CVI_VI_SetDevAttr(VI_DEV ViDev, const VI_DEV_ATTR_S *pstDevAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;
	VI_DEV_ATTR_S devAttr;

	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pstDevAttr);

	fd = get_vi_fd();

	devAttr = *pstDevAttr;

	s32Ret = vi_sdk_set_dev_attr(fd, ViDev, &devAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "setDevAttr ioctl failed\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetDevAttr(VI_DEV ViDev, VI_DEV_ATTR_S *pstDevAttr)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pstDevAttr);

	fd = get_vi_fd();
	if (fd < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "get_vi_fd open failed\n");
		return CVI_ERR_VI_BUSY;
	}

	s32Ret = vi_sdk_get_dev_attr(fd, ViDev, pstDevAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_dev_attr ioctl failed 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_SetDevBindAttr(VI_DEV ViDev, const VI_DEV_BIND_PIPE_S *pstDevBindAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;
	VI_DEV_BIND_PIPE_S devBindAttr;

	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pstDevBindAttr);

	fd = get_vi_fd();

	devBindAttr = *pstDevBindAttr;

	s32Ret = vi_sdk_set_dev_bind_attr(fd, ViDev, &devBindAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "setDevBindAttr ioctl failed\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetDevBindAttr(VI_DEV ViDev, VI_DEV_BIND_PIPE_S *pstDevBindAttr)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pstDevBindAttr);

	fd = get_vi_fd();
	if (fd < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "get_vi_fd open failed\n");
		return CVI_ERR_VI_BUSY;
	}

	s32Ret = vi_sdk_get_dev_bind_attr(fd, ViDev, pstDevBindAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_dev_bind_attr ioctl failed 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_SetDevUnbindAttr(VI_DEV ViDev)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_DEVID_VALID(ViDev);

	fd = get_vi_fd();
	if (fd < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "get_vi_fd open failed\n");
		return CVI_ERR_VI_BUSY;
	}

	s32Ret = vi_sdk_set_dev_unbind_attr(fd, ViDev);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_dev_unbind_attr ioctl failed 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_EnableDev(VI_DEV ViDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;

	CHECK_VI_DEVID_VALID(ViDev);

	fd = get_vi_fd();
	if (fd < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "get_vi_fd open failed\n");
		return CVI_ERR_VI_BUSY;
	}

	s32Ret = vi_sdk_enable_dev(fd, ViDev);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "enable_dev ioctl failed errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_DisableDev(VI_DEV ViDev)
{
	CVI_S32 ret = CVI_SUCCESS;
	CVI_S32 fd = -1;

	CHECK_VI_DEVID_VALID(ViDev);

	fd = get_vi_fd();
	if (fd < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "get_vi_fd open failed\n");
		return CVI_ERR_VI_BUSY;
	}

	ret = vi_sdk_disable_dev(fd, ViDev);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "disable_dev ioctl failed 0x%x\n", ret);
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_SetDevTimingAttr(VI_DEV ViDev, const VI_DEV_TIMING_ATTR_S *pstTimingAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;
	VI_DEV_TIMING_ATTR_S stTimingAttr;

	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pstTimingAttr);

	fd = get_vi_fd();

	stTimingAttr = *pstTimingAttr;
	s32Ret = vi_sdk_set_dev_timing_attr(fd, ViDev, &stTimingAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_dev_timing_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetDevTimingAttr(VI_DEV ViDev, VI_DEV_TIMING_ATTR_S *pstTimingAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;

	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pstTimingAttr);

	fd = get_vi_fd();

	s32Ret = vi_sdk_get_dev_timing_attr(fd, ViDev, pstTimingAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_dev_timing_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

/* 2 for vi pipe */
CVI_S32 CVI_VI_CreatePipe(VI_PIPE ViPipe, const VI_PIPE_ATTR_S *pstPipeAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;
	VI_PIPE_ATTR_S stPipeAttr;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstPipeAttr);

	fd = get_vi_fd();

	stPipeAttr = *pstPipeAttr;

	s32Ret = vi_sdk_create_pipe(fd, ViPipe, &stPipeAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_create_pipe ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_DestroyPipe(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;

	CHECK_VI_PIPEID_VALID(ViPipe);

	fd = get_vi_fd();

	s32Ret = vi_sdk_destroy_pipe(fd, ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_destroy_pipe ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_StartPipe(VI_PIPE ViPipe)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;

	CHECK_VI_PIPEID_VALID(ViPipe);

	fd = get_vi_fd();

	s32Ret = vi_sdk_start_pipe(fd, ViPipe);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_start_pipe ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 CVI_VI_StopPipe(VI_PIPE ViPipe)
{
	CHECK_VI_PIPEID_VALID(ViPipe);

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_SetPipeAttr(VI_PIPE ViPipe, const VI_PIPE_ATTR_S *pstPipeAttr)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_PIPE_ATTR_S pipeAttr;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstPipeAttr);

	if (pstPipeAttr->u32MaxW > VI_PIPE_ONLINE_MAX_WIDTH || pstPipeAttr->u32MaxH > VI_PIPE_ONLINE_MAX_HEIGHT) {
		CVI_TRACE_VI(CVI_DBG_ERR, "u32MaxW(%d) or u32MaxH(%d) too large\n",
			pstPipeAttr->u32MaxW, pstPipeAttr->u32MaxH);
		return CVI_ERR_VI_INVALID_PARA;
	}

	fd = get_vi_fd();
	if (fd < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "get_vi_fd open failed\n");
		return CVI_ERR_VI_BUSY;
	}

	pipeAttr = *pstPipeAttr;

	s32Ret = vi_sdk_set_pipe_attr(fd, ViPipe, &pipeAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_pipe_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetPipeAttr(VI_PIPE ViPipe, VI_PIPE_ATTR_S *pstPipeAttr)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstPipeAttr);

	fd = get_vi_fd();
	if (fd < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "get_vi_fd open failed\n");
		return CVI_ERR_VI_BUSY;
	}

	s32Ret = vi_sdk_get_pipe_attr(fd, ViPipe, pstPipeAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_pipe_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_SetPipeDumpAttr(VI_PIPE ViPipe, const VI_DUMP_ATTR_S *pstDumpAttr)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_DUMP_ATTR_S dumpAttr;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstDumpAttr);

	fd = get_vi_fd();
	if (fd < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "get_vi_fd open failed\n");
		return CVI_ERR_VI_BUSY;
	}

	dumpAttr = *pstDumpAttr;
	s32Ret = vi_sdk_set_pipe_dump_attr(fd, ViPipe, &dumpAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_pipe_dump_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetPipeDumpAttr(VI_PIPE ViPipe, VI_DUMP_ATTR_S *pstDumpAttr)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstDumpAttr);

	fd = get_vi_fd();
	if (fd < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "get_vi_fd open failed\n");
		return CVI_ERR_VI_BUSY;
	}

	s32Ret = vi_sdk_get_pipe_dump_attr(fd, ViPipe, pstDumpAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_pipe_dump_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

// get bayer from preraw
CVI_S32 CVI_VI_GetPipeFrame(VI_PIPE ViPipe, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstFrameInfo);

	fd = get_vi_fd();
	if (fd < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "get_vi_fd open failed\n");
		return CVI_ERR_VI_BUSY;
	}

	s32Ret = vi_sdk_get_pipe_frame(fd, ViPipe, pstFrameInfo, s32MilliSec);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_pipe_frame ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_ReleasePipeFrame(VI_PIPE ViPipe, const VIDEO_FRAME_INFO_S *pstFrameInfo)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;
	VIDEO_FRAME_INFO_S stVideoFrame;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstFrameInfo);

	fd = get_vi_fd();
	if (fd < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "get_vi_fd open failed\n");
		return CVI_ERR_VI_BUSY;
	}

	stVideoFrame = *pstFrameInfo;
	s32Ret = vi_sdk_release_pipe_frame(fd, ViPipe, &stVideoFrame);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_release_pipe_frame ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_StartSmoothRawDump(const VI_SMOOTH_RAW_DUMP_INFO_S *pstDumpInfo)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_NULL_PTR(pstDumpInfo);
	CHECK_VI_NULL_PTR(pstDumpInfo->phy_addr_list);
	CHECK_VI_PIPEID_VALID(pstDumpInfo->ViPipe);

	VI_PIPE ViPipe = pstDumpInfo->ViPipe;
	CVI_U8 frm_num = 0;
	CVI_U64 phy_addr = 0;
	struct cvi_vip_isp_smooth_raw_param param;
	struct cvi_vip_isp_raw_blk *raw_blk = CVI_NULL;
	VI_DEV_ATTR_S stDevAttr;

	fd = get_vi_fd();

	if (pstDumpInfo->u8BlkCnt < 2) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Need two ring buffer at least, now is %d\n", pstDumpInfo->u8BlkCnt);
		return CVI_ERR_VI_INVALID_PARA;
	}

	s32Ret = vi_sdk_get_dev_attr(fd, ViPipe, &stDevAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "sys busy\n");
		return s32Ret;
	}

	if ((stDevAttr.stWDRAttr.enWDRMode == WDR_MODE_2To1_LINE) ||
		(stDevAttr.stWDRAttr.enWDRMode == WDR_MODE_2To1_FRAME) ||
		(stDevAttr.stWDRAttr.enWDRMode == WDR_MODE_2To1_FRAME_FULL_RATE)) {
		frm_num = 2;
	} else {
		frm_num = 1;
	}

	frm_num = (pstDumpInfo->u8BlkCnt) * frm_num;

	raw_blk = calloc(frm_num, sizeof(struct cvi_vip_isp_raw_blk));
	if (raw_blk == CVI_NULL) {
		CVI_TRACE_VI(CVI_DBG_ERR, "malloc raw_blk failed\n");
		return CVI_ERR_VI_NOMEM;
	}

	for (CVI_U8 i = 0; i < frm_num; i++) {
		phy_addr = *(pstDumpInfo->phy_addr_list + i);
		if (phy_addr == 0) {
			if (raw_blk != CVI_NULL) {
				free(raw_blk);
				raw_blk = CVI_NULL;
			}
			CVI_TRACE_VI(CVI_DBG_ERR, "phy_addr is invalid\n");
			return CVI_ERR_VI_INVALID_PARA;
		}

		(raw_blk + i)->raw_dump.phy_addr = phy_addr;
		// CVI_TRACE_VI(CVI_DBG_DEBUG, "i=%d, phy_paddr(%#"PRIx64")\n", i, (raw_blk+i)->raw_dump.phy_addr);

		// set rawdump crop info
		(raw_blk + i)->crop_x = pstDumpInfo->stCropRect.s32X;
		(raw_blk + i)->crop_y = pstDumpInfo->stCropRect.s32Y;
		(raw_blk + i)->src_w = pstDumpInfo->stCropRect.u32Width;
		(raw_blk + i)->src_h = pstDumpInfo->stCropRect.u32Height;
		//CVI_TRACE_VI(CVI_DBG_DEBUG, "i (%d), crop_x(%d), crop_y(%d), src_w(%d), src_h(%d)\n",
		//	i, (raw_blk + i)->crop_x, (raw_blk + i)->crop_y,
		//	(raw_blk + i)->src_w, (raw_blk + i)->src_h);
	}

	param.raw_num = ViPipe;
	param.frm_num = frm_num;
	param.raw_blk = raw_blk;

	s32Ret = vi_sdk_start_smooth_rawdump(fd, ViPipe, &param);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_start_smooth_rawdump ioctl failed. errno 0x%x\n", s32Ret);
		free(raw_blk);
		return CVI_ERR_VI_FAILED_NOT_ENABLED;
	}

	if (raw_blk != CVI_NULL) {
		free(raw_blk);
		raw_blk = CVI_NULL;
	}

	return s32Ret;
}

CVI_S32 CVI_VI_StopSmoothRawDump(const VI_SMOOTH_RAW_DUMP_INFO_S *pstDumpInfo)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_NULL_PTR(pstDumpInfo);
	CHECK_VI_PIPEID_VALID(pstDumpInfo->ViPipe);

	VI_PIPE ViPipe = pstDumpInfo->ViPipe;
	struct cvi_vip_isp_smooth_raw_param param;

	fd = get_vi_fd();

	param.raw_num = ViPipe;

	s32Ret = vi_sdk_stop_smooth_rawdump(fd, ViPipe, &param);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_stop_smooth_rawdump ioctl failed. errno 0x%x\n", s32Ret);
		return CVI_ERR_VI_FAILED_NOT_ENABLED;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetSmoothRawDump(VI_PIPE ViPipe, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_NULL_PTR(pstVideoFrame);

	fd = get_vi_fd();

	s32Ret = vi_sdk_get_smooth_rawdump(fd, ViPipe, pstVideoFrame, s32MilliSec);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_smooth_rawdump ioctl failed. errno 0x%x\n", s32Ret);
		return CVI_ERR_VI_FAILED_NOT_ENABLED;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_PutSmoothRawDump(VI_PIPE ViPipe, const VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;
	VIDEO_FRAME_INFO_S stVideoFrame[2];

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstVideoFrame);

	fd = get_vi_fd();

	stVideoFrame[0] = pstVideoFrame[0];
	stVideoFrame[1] = pstVideoFrame[1];
	s32Ret = vi_sdk_put_smooth_rawdump(fd, ViPipe, stVideoFrame);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_put_smooth_rawdump ioctl failed. errno 0x%x\n", s32Ret);
		return CVI_ERR_VI_FAILED_NOT_ENABLED;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_QueryPipeStatus(VI_PIPE ViPipe, VI_PIPE_STATUS_S *pstStatus)
{
	CVI_S32 fd = get_vi_fd();
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstStatus);

	s32Ret = vi_sdk_get_pipe_status(fd, ViPipe, pstStatus);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_pipe_status ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_SetPipeFrameSource(VI_PIPE ViPipe, const VI_PIPE_FRAME_SOURCE_E enSource)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_PIPE_FRAME_SOURCE_E src = enSource;

	CHECK_VI_PIPEID_VALID(ViPipe);

	if (enSource < 0 || enSource >= VI_PIPE_FRAME_SOURCE_BUTT) {
		CVI_TRACE_VI(CVI_DBG_ERR, "enSource(%d)is invalid\n", enSource);
		return CVI_ERR_VI_INVALID_PARA;
	}

	fd = get_vi_fd();

	s32Ret = vi_sdk_set_pipe_frm_src(fd, ViPipe, &src);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_pipe_frm_src ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetPipeFrameSource(VI_PIPE ViPipe, VI_PIPE_FRAME_SOURCE_E *penSource)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(penSource);

	fd = get_vi_fd();

	s32Ret = vi_sdk_get_pipe_frm_src(fd, ViPipe, penSource);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_pipe_frm_src ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_SendPipeRaw(CVI_U32 u32PipeNum, VI_PIPE PipeId[], const VIDEO_FRAME_INFO_S *pstVideoFrame[],
			   CVI_S32 s32MilliSec)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;

	CHECK_VI_NULL_PTR(PipeId);
	CHECK_VI_NULL_PTR(pstVideoFrame);

	if (u32PipeNum > VI_MAX_PIPE_NUM - 1) {
		CVI_TRACE_VI(CVI_DBG_ERR, "only support %u pipe\n", VI_MAX_PIPE_NUM);
		return CVI_ERR_VI_INVALID_PIPEID;
	}

	UNUSED(s32MilliSec);

	fd = get_vi_fd();

	for (CVI_U32 i = 0; i < u32PipeNum; ++i) {
		VI_PIPE pipeid = PipeId[i];

		CHECK_VI_NULL_PTR(pstVideoFrame[i]);
		VIDEO_FRAME_INFO_S stVideoFrm = *pstVideoFrame[i];

		s32Ret = vi_sdk_send_pipe_raw(fd, pipeid, &stVideoFrm);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_send_pipe_raw ioctl failed. errno 0x%x\n", s32Ret);
			return s32Ret;
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetPipeFd(VI_PIPE ViPipe)
{
	CVI_S32 fd = -1;

	CHECK_VI_PIPEID_VALID(ViPipe);

	fd = get_vi_fd();

	if (fd <= 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "Get pipe fd fail\n");
		return CVI_FAILURE;
	}

	return fd;
}

CVI_S32 CVI_VI_CloseFd(void)
{
	vi_dev_close();
	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_SetPipeCrop(VI_PIPE ViPipe, const CROP_INFO_S *pstCropInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;
	CROP_INFO_S stCropInfo;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstCropInfo);

	fd = get_vi_fd();

	if (pstCropInfo->stRect.s32X % 2 || pstCropInfo->stRect.s32Y % 2 ||
		pstCropInfo->stRect.u32Width % 2 || pstCropInfo->stRect.u32Height % 2) {
		CVI_TRACE_VI(CVI_DBG_ERR, "crop_x(%d)_y(%d)_w(%d)_h(%d) must be multiple of 2.\n",
					pstCropInfo->stRect.s32X,
					pstCropInfo->stRect.s32Y,
					pstCropInfo->stRect.u32Width,
					pstCropInfo->stRect.u32Height);
		return CVI_ERR_VI_INVALID_PARA;
	}

	if (pstCropInfo->stRect.s32X < 0 || pstCropInfo->stRect.s32Y < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "crop_x(%d)_y(%d) is invalid.\n",
					pstCropInfo->stRect.s32X,
					pstCropInfo->stRect.s32Y);
		return CVI_ERR_VI_INVALID_PARA;
	}

	stCropInfo = *pstCropInfo;
	if (pstCropInfo->bEnable == CVI_TRUE) {
		s32Ret = vi_sdk_set_pipe_crop(fd, ViPipe, &stCropInfo);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_pipe_crop ioctl failed. errno 0x%x\n", s32Ret);
			return s32Ret;
		}
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetPipeCrop(VI_PIPE ViPipe, CROP_INFO_S *pstCropInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstCropInfo);

	fd = get_vi_fd();

	s32Ret = vi_sdk_get_pipe_crop(fd, ViPipe, pstCropInfo);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_pipe_crop ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_AttachVbPool(VI_PIPE ViPipe, VI_CHN ViChn, VB_POOL VbPool)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);

	CVI_S32 fd = get_vi_fd();
	struct vi_vb_pool_cfg cfg;

	memset(&cfg, 0, sizeof(cfg));
	cfg.ViPipe = ViPipe;
	cfg.ViChn = ViChn;
	cfg.VbPool = VbPool;
	return vi_sdk_attach_vbpool(fd, &cfg);
}

CVI_S32 CVI_VI_DetachVbPool(VI_PIPE ViPipe, VI_CHN ViChn)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);

	CVI_S32 fd = get_vi_fd();
	struct vi_vb_pool_cfg cfg;

	memset(&cfg, 0, sizeof(cfg));
	cfg.ViPipe = ViPipe;
	cfg.ViChn = ViChn;
	return vi_sdk_detach_vbpool(fd, &cfg);
}

CVI_S32 CVI_VI_SetChnAttr(VI_PIPE ViPipe, VI_CHN ViChn, VI_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;

	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstChnAttr);

	fd = get_vi_fd();

	if (pstChnAttr->stFrameRate.s32SrcFrameRate != pstChnAttr->stFrameRate.s32DstFrameRate)
		CVI_TRACE_VI(CVI_DBG_WARN, "FrameRate ctrl, src(%d) dst(%d), not support yet.\n"
				, pstChnAttr->stFrameRate.s32SrcFrameRate, pstChnAttr->stFrameRate.s32DstFrameRate);

	if (pstChnAttr->enPixelFormat != PIXEL_FORMAT_NV21 &&
		(pstChnAttr->enPixelFormat < PIXEL_FORMAT_YUYV || pstChnAttr->enPixelFormat > PIXEL_FORMAT_VYUY)) {
		CVI_TRACE_VI(CVI_DBG_ERR, "not support %d\n", pstChnAttr->enPixelFormat);
		return CVI_ERR_VI_NOT_SUPPORT;
	}

	s32Ret = vi_sdk_set_chn_attr(fd, ViPipe, ViChn, pstChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_chn_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetChnAttr(VI_PIPE ViPipe, VI_CHN ViChn, VI_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;

	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstChnAttr);

	fd = get_vi_fd();

	s32Ret = vi_sdk_get_chn_attr(fd, ViPipe, ViChn, pstChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_EnableChn(VI_PIPE ViPipe, VI_CHN ViChn)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_CHN_ATTR_S stChnAttr;

	CHECK_VI_PIPEID_VALID(ViPipe);

	if (ViChn >= (VI_MAX_CHN_NUM + VI_MAX_EXT_CHN_NUM) || ViChn < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, " invalid chn-id(%d)\n", ViChn);
		return CVI_ERR_VI_INVALID_CHNID;
	}

	fd = get_vi_fd();

	s32Ret = vi_sdk_get_chn_attr(fd, ViPipe, ViChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	if (stChnAttr.stSize.u32Width == 0 &&
		stChnAttr.stSize.u32Height == 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, " Call SetChnAttr first(%d)\n", ViChn);
		return CVI_ERR_VI_FAILED_NOTCONFIG;
	}

	if (ViChn >= VI_EXT_CHN_START) {
		CVI_TRACE_VI(CVI_DBG_ERR, " not support ext chn(%d)\n", ViChn);
	} else {
		if (ViChn < VI_MAX_CHN_NUM)
			_vi_chn_enable_mirror_flip(ViPipe, ViChn,
				stChnAttr.bFlip, stChnAttr.bMirror);

		s32Ret = vi_sdk_enable_chn(fd, ViPipe, ViChn);
		if (s32Ret < 0) {
			CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_enable_chn ioctl failed. errno 0x%x\n", s32Ret);
			return s32Ret;
		}

		if (s32Ret == CVI_SUCCESS_ALL_CHN) {
			struct sched_param param;
			pthread_attr_t attr;

			param.sched_priority = 85;

			pthread_attr_init(&attr);
			pthread_attr_setschedpolicy(&attr, SCHED_RR);
			pthread_attr_setschedparam(&attr, &param);
			pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

			pthread_create(&gViDbgTH.vi_dbg_thread, &attr, (void *)vi_dbg_handler, NULL);
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_DisableChn(VI_PIPE ViPipe, VI_CHN ViChn)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_CHAR mesh_name[128];
	struct vi_chn_ldc_cfg ldc_cfg = {.ViPipe = ViPipe, .ViChn = ViChn};;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);

	if (ViChn >= (VI_MAX_CHN_NUM + VI_MAX_EXT_CHN_NUM)) {
		CVI_TRACE_VI(CVI_DBG_ERR, " invalid chn-id(%d)\n", ViChn);
		return CVI_ERR_VI_INVALID_CHNID;
	}

	fd = get_vi_fd();

	if (ViChn < VI_MAX_PHY_CHN_NUM) {
		gViDbgTH.th_enable = CVI_FALSE;
		s32Ret = vi_sdk_disable_chn(fd, ViPipe, ViChn);
		if (s32Ret < 0) {
			CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_disable_chn ioctl failed. errno 0x%x\n", s32Ret);
			return s32Ret;
		}

		if (s32Ret == CVI_SUCCESS_ALL_CHN) {
			pthread_join(gViDbgTH.vi_dbg_thread, NULL);
		}

		snprintf(mesh_name, 128, "vi_%d", ViChn);
		vi_sdk_get_chn_ldc(fd, ViPipe, ViChn, &ldc_cfg);
		CVI_GDC_FreeCurTaskMesh(mesh_name);

		g_vi_mesh[ViChn].paddr = CVI_NULL;
		g_vi_mesh[ViChn].vaddr = CVI_NULL;
	} else if (ViChn >= VI_EXT_CHN_START) {
		CVI_TRACE_VI(CVI_DBG_ERR, " not support ext chn(%d)\n", ViChn);
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_SetChnCrop(VI_PIPE ViPipe, VI_CHN ViChn, const VI_CROP_INFO_S  *pstCropInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;
	VI_CROP_INFO_S cropInfo;

	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstCropInfo);

	fd = get_vi_fd();

	if (pstCropInfo->stCropRect.s32X % 2 || pstCropInfo->stCropRect.s32Y % 2 ||
		pstCropInfo->stCropRect.u32Width % 2 || pstCropInfo->stCropRect.u32Height % 2) {
		CVI_TRACE_VI(CVI_DBG_ERR, "crop_x(%d)_y(%d)_w(%d)_h(%d) must be multiple of 2.\n",
					pstCropInfo->stCropRect.s32X,
					pstCropInfo->stCropRect.s32Y,
					pstCropInfo->stCropRect.u32Width,
					pstCropInfo->stCropRect.u32Height);
		return CVI_ERR_VI_INVALID_PARA;
	}

	if (pstCropInfo->stCropRect.s32X < 0 || pstCropInfo->stCropRect.s32Y < 0) {
		CVI_TRACE_VI(CVI_DBG_ERR, "crop_x(%d)_y(%d) is invalid.\n",
					pstCropInfo->stCropRect.s32X,
					pstCropInfo->stCropRect.s32Y);
		return CVI_ERR_VI_INVALID_PARA;
	}

	if (pstCropInfo->bEnable == CVI_TRUE) {
		cropInfo = *pstCropInfo;
		s32Ret = vi_sdk_set_chn_crop(fd, ViPipe, ViChn, &cropInfo);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_chn_crop ioctl failed. errno 0x%x\n", s32Ret);
			return s32Ret;
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetChnCrop(VI_PIPE ViPipe, VI_CHN ViChn, VI_CROP_INFO_S  *pstCropInfo)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;

	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstCropInfo);

	fd = get_vi_fd();

	s32Ret = vi_sdk_get_chn_crop(fd, ViPipe, ViChn, pstCropInfo);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_crop ioctl failed. errno 0x%x\n", s32Ret);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_GetChnFrame(VI_PIPE ViPipe, VI_CHN ViChn, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(pstFrameInfo);

	fd = get_vi_fd();

	s32Ret = vi_sdk_get_chn_frame(fd, ViPipe, ViChn, pstFrameInfo, s32MilliSec);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_frame ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_ReleaseChnFrame(VI_PIPE ViPipe, VI_CHN ViChn, const VIDEO_FRAME_INFO_S *pstFrameInfo)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;
	VIDEO_FRAME_INFO_S stFrameInfo;

	CHECK_VI_NULL_PTR(pstFrameInfo);
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);

	fd = get_vi_fd();

	stFrameInfo = *pstFrameInfo;
	s32Ret = vi_sdk_release_chn_frame(fd, ViPipe, ViChn, &stFrameInfo);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_release_chn_frame ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_QueryChnStatus(VI_PIPE ViPipe, VI_CHN ViChn, VI_CHN_STATUS_S *pstChnStatus)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_NULL_PTR(pstChnStatus);

	fd = get_vi_fd();

	s32Ret = vi_sdk_get_chn_status(fd, ViPipe, ViChn, pstChnStatus);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_status ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_SetChnRotation(VI_PIPE ViPipe, VI_CHN ViChn, const ROTATION_E enRotation)
{
	CVI_S32 fd = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;
	struct cvi_isp_sc_online online;
	VI_CHN_ATTR_S stChnAttr;
	struct vi_chn_ldc_cfg ldc_cfg;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);

	fd = get_vi_fd();

	online.raw_num = ViPipe;
	s32Ret = vi_get_online2sc(fd, &online);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "sys busy.\n");
		return CVI_ERR_VI_BUSY;
	}

	if (online.is_sc_online) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI Rotation not support online2sc.\n");
		return CVI_ERR_VI_NOT_SUPPORT;
	}

	s32Ret = vi_sdk_get_chn_attr(fd, ViPipe, ViChn, &stChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_attr ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = vi_sdk_get_chn_ldc(fd, ViPipe, ViChn, &ldc_cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_ldc ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	CHECK_VI_GDC_FMT(stChnAttr.enPixelFormat);

	if (enRotation == ROTATION_180) {
		CVI_TRACE_VI(CVI_DBG_ERR, "not support rotation(%d).\n", enRotation);
		return CVI_ERR_VI_NOT_SUPPORT;
	} else if (enRotation >= ROTATION_MAX) {
		CVI_TRACE_VI(CVI_DBG_ERR, "invalid rotation(%d).\n", enRotation);
		return CVI_ERR_VI_INVALID_PARA;
	}

	if (ldc_cfg.stLDCAttr.bEnable) {
		CVI_TRACE_VI(CVI_DBG_ERR, "set rotation fail, please add rotation to ldc.\n");
		return CVI_ERR_VPSS_ILLEGAL_PARAM;
	} else
		return _vi_update_rotation_mesh(ViPipe, ViChn, enRotation);
	return s32Ret;
}

CVI_S32 CVI_VI_GetChnRotation(VI_PIPE ViPipe, VI_CHN ViChn, ROTATION_E *penRotation)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;
	struct vi_chn_rot_cfg rotCfg;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_NULL_PTR(penRotation);

	fd = get_vi_fd();

	rotCfg.ViPipe = ViPipe;
	rotCfg.ViChn = ViChn;
	s32Ret = vi_sdk_get_chn_rotation(fd, &rotCfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_rotation ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	*penRotation = rotCfg.enRotation;

	return s32Ret;
}

CVI_S32 CVI_VI_SetChnLDCAttr(VI_PIPE ViPipe, VI_CHN ViChn, const VI_LDC_ATTR_S *pstLDCAttr)
{
	// CVI_S32 fd = -1;
	// CVI_S32 s32Ret = CVI_SUCCESS;
	// struct cvi_isp_sc_online online;
	// VI_CHN_ATTR_S stChnAttr;
	// struct vi_chn_rot_cfg rotCfg;

	// CHECK_VI_NULL_PTR(pstLDCAttr);
	// CHECK_VI_PIPEID_VALID(ViPipe);
	// CHECK_VI_CHNID_VALID(ViChn);

	// fd = get_vi_fd();

	// s32Ret = vi_sdk_get_chn_attr(fd, ViPipe, ViChn, &stChnAttr);
	// if (s32Ret != CVI_SUCCESS) {
	// 	CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_attr ioctl failed. errno 0x%x\n", s32Ret);
	// 	return s32Ret;
	// }

	// CHECK_VI_GDC_FMT(stChnAttr.enPixelFormat);

	// online.raw_num = ViPipe;
	// s32Ret = vi_get_online2sc(fd, &online);
	// if (s32Ret != CVI_SUCCESS) {
	// 	CVI_TRACE_VI(CVI_DBG_ERR, "sys busy.\n");
	// 	return CVI_ERR_VI_BUSY;
	// }

	// if (online.is_sc_online) {
	// 	CVI_TRACE_VI(CVI_DBG_ERR, "VI Rotation not support online2sc.\n");
	// 	return CVI_ERR_VI_NOT_SUPPORT;
	// }

	// rotCfg.ViPipe = ViPipe;
	// rotCfg.ViChn = ViChn;
	// s32Ret = vi_sdk_get_chn_rotation(fd, &rotCfg);
	// if (s32Ret != CVI_SUCCESS) {
	// 	CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_rotation ioctl failed. errno 0x%x\n", s32Ret);
	// 	return s32Ret;
	// }
	// if (rotCfg.enRotation > 0) {
	// 	CVI_TRACE_VI(CVI_DBG_ERR, "set ldc fail, please add rotation to ldc.\n");
	// 	return CVI_ERR_VPSS_ILLEGAL_PARAM;
	// }

	// if (pstLDCAttr->stAttr.enRotation == ROTATION_180) {
	// 	CVI_TRACE_VI(CVI_DBG_ERR, "not support ldc rotation(%d).\n", pstLDCAttr->stAttr.enRotation);
	// 	return CVI_ERR_VI_NOT_SUPPORT;
	// } else if (pstLDCAttr->stAttr.enRotation >= ROTATION_MAX) {
	// 	CVI_TRACE_VI(CVI_DBG_ERR, "Pipe(%d) Chn(%d) invalid ldc rotation(%d).\n"
	// 		, ViPipe, ViChn, pstLDCAttr->stAttr.enRotation);
	// 	return CVI_ERR_VPSS_ILLEGAL_PARAM;
	// }

	// return _vi_update_ldc_mesh(ViPipe, ViChn, pstLDCAttr,
	// 		stChnAttr.stSize.u32Width, stChnAttr.stSize.u32Height);
	UNUSED(ViPipe);
	UNUSED(ViChn);
	UNUSED(pstLDCAttr);

	CVI_TRACE_VI(CVI_DBG_ERR, "vi not support ldc \n");

	return CVI_ERR_VI_NOT_SUPPORT;
}

CVI_S32 CVI_VI_GetChnLDCAttr(VI_PIPE ViPipe, VI_CHN ViChn, VI_LDC_ATTR_S *pstLDCAttr)
{
	// CVI_S32 fd = -1;
	// CVI_S32 s32Ret = CVI_SUCCESS;
	// struct vi_chn_ldc_cfg ldc_cfg;

	// CHECK_VI_NULL_PTR(pstLDCAttr);
	// CHECK_VI_PIPEID_VALID(ViPipe);
	// CHECK_VI_CHNID_VALID(ViChn);

	// fd = get_vi_fd();

	// s32Ret = vi_sdk_get_chn_ldc(fd, ViPipe, ViChn, &ldc_cfg);
	// if (s32Ret != CVI_SUCCESS) {
	// 	CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_ldc ioctl failed. errno 0x%x\n", s32Ret);
	// 	return s32Ret;
	// }
	UNUSED(ViPipe);
	UNUSED(ViChn);
	UNUSED(pstLDCAttr);

	CVI_TRACE_VI(CVI_DBG_ERR, "vi not support ldc \n");

	return CVI_ERR_VI_NOT_SUPPORT;
}

CVI_S32 CVI_VI_RegChnFlipMirrorCallBack(VI_PIPE ViPipe, VI_DEV ViDev, void *pvData)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pvData);

	s_pfnDevMirrorFlip[ViDev] = (pfnChnMirrorFlip)pvData;
	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_UnRegChnFlipMirrorCallBack(VI_PIPE ViPipe, VI_DEV ViDev)
{
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_DEVID_VALID(ViDev);

	s_pfnDevMirrorFlip[ViDev] = CVI_NULL;
	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_SetChnFlipMirror(VI_PIPE ViPipe, VI_CHN ViChn, CVI_BOOL bFlip, CVI_BOOL bMirror)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;
	struct vi_chn_flip_mirror_cfg cfg;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);

	fd = get_vi_fd();

	cfg.ViPipe = ViPipe;
	cfg.ViChn = ViChn;
	cfg.bFlip = bFlip;
	cfg.bMirror = bMirror;
	s32Ret = vi_sdk_set_chn_flip_mirror(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_set_chn_flip_mirror ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	return _vi_chn_enable_mirror_flip(ViPipe, ViChn, bFlip, bMirror);
}

CVI_S32 CVI_VI_GetChnFlipMirror(VI_PIPE ViPipe, VI_CHN ViChn, CVI_BOOL *pbFlip, CVI_BOOL *pbMirror)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 fd = -1;
	struct vi_chn_flip_mirror_cfg cfg;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_CHNID_VALID(ViChn);
	CHECK_VI_NULL_PTR(pbFlip);
	CHECK_VI_NULL_PTR(pbMirror);

	fd = get_vi_fd();

	cfg.ViPipe = ViPipe;
	cfg.ViChn = ViChn;
	s32Ret = vi_sdk_get_chn_flip_mirror(fd, &cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "vi_sdk_get_chn_flip_mirror ioctl failed. errno 0x%x\n", s32Ret);
		return s32Ret;
	}

	*pbFlip = cfg.bFlip;
	*pbMirror = cfg.bMirror;

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_DumpHwRegisterToFile(VI_PIPE ViPipe, FILE *fp, VI_DUMP_REGISTER_TABLE_S *pstRegTbl)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_NULL_PTR(fp);
	CHECK_VI_NULL_PTR(pstRegTbl);

	s32Ret = dump_register_cv186x(ViPipe, fp, pstRegTbl);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VI(CVI_DBG_ERR, "dump_register_cv186x fail\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_RegPmCallBack(VI_DEV ViDev, VI_PM_OPS_S *pstPmOps, void *pvData)
{
	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pstPmOps);
	CHECK_VI_NULL_PTR(pvData);

	memcpy(&apstViPm[ViDev].stOps, pstPmOps, sizeof(VI_PM_OPS_S));
	apstViPm[ViDev].pvData = pvData;
	return CVI_SUCCESS;
}

CVI_S32 CVI_VI_UnRegPmCallBack(VI_DEV ViDev)
{
	CHECK_VI_DEVID_VALID(ViDev);

	memset(&apstViPm[ViDev].stOps, 0, sizeof(VI_PM_OPS_S));
	apstViPm[ViDev].pvData = NULL;
	return CVI_SUCCESS;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_Trig_AHD(VI_PIPE ViPipe, CVI_U8 u8AHDSignal)
{
	UNUSED(ViPipe);
	UNUSED(u8AHDSignal);
#if defined(POSSIBLE_DEAD_CODE)
	CVI_S32 fd = -1;
	VI_CHN ViChn = ViPipe;

	CHECK_VI_PIPEID_VALID(ViPipe);
	if (CHECK_VI_CTX_NULL_PTR(gViCtx) != CVI_SUCCESS)
		return CVI_ERR_VI_FAILED_NOTCONFIG;

	fd = get_vi_fd();

	if (gViCtx->pipeAttr[ViPipe].bYuvBypassPath == CVI_FALSE) {
		CVI_TRACE_VI(CVI_DBG_ERR, "VI Pipe(%d) is not yuv bypass mode.", ViPipe);
		return CVI_ERR_VI_NOT_SUPPORT;
	}

	gViCtx->chnStatus[ViChn].bEnable = (u8AHDSignal == 0) ? CVI_FALSE : CVI_TRUE;
	if (gViCtx->chnStatus[ViChn].bEnable == CVI_TRUE) {
		vi_set_trig_preraw(fd, ViPipe);
	}
#endif
	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_SetExtChnFisheye(VI_PIPE ViPipe, VI_CHN ViChn, const FISHEYE_ATTR_S *pstFishEyeAttr)
{
	CHECK_VI_NULL_PTR(pstFishEyeAttr);
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_EXTCHNID_VALID(ViChn);

	//ToDo fisheye

	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_GetExtChnFisheye(VI_PIPE ViPipe, VI_CHN ViChn, FISHEYE_ATTR_S *pstFishEyeAttr)
{
	CHECK_VI_NULL_PTR(pstFishEyeAttr);
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_EXTCHNID_VALID(ViChn);

	//ToDo fisheye

	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_SetExtChnAttr(VI_PIPE ViPipe, VI_CHN ViChn, const VI_EXT_CHN_ATTR_S *pstExtChnAttr)
{
	CHECK_VI_NULL_PTR(pstExtChnAttr);
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_EXTCHNID_VALID(ViChn);
#if defined(POSSIBLE_DEAD_CODE)
	if (CHECK_VI_CTX_NULL_PTR(gViCtx) != CVI_SUCCESS)
		return CVI_ERR_VI_FAILED_NOTCONFIG;

	if (pstExtChnAttr->s32BindChn >= VI_MAX_CHN_NUM) {
		CVI_TRACE_VI(CVI_DBG_ERR, " invalid bind chn-id(%d)\n", pstExtChnAttr->s32BindChn);
		return CVI_ERR_VI_INVALID_PARA;
	}

	VI_CHN_ATTR_S *pstChnAttr = &gViCtx->chnAttr[pstExtChnAttr->s32BindChn];

	if (_vpss_is_online()) {
		CVI_TRACE_VPSS(CVI_DBG_ERR, "ViPipe(%d) Chn(%d) not supported if online.\n"
			      , ViPipe, ViChn);
		return CVI_ERR_VI_INVALID_PARA;
	}

	if (!gViCtx->is_enable[pstExtChnAttr->s32BindChn]) {
		CVI_TRACE_VI(CVI_DBG_ERR, "bindchn(%d) not enabled yet.\n", pstExtChnAttr->s32BindChn);
		return CVI_ERR_VI_INVALID_PARA;
	}

	//ToDo ldc/fisheye

	if (pstChnAttr->enPixelFormat != pstExtChnAttr->enPixelFormat) {
		CVI_TRACE_VI(CVI_DBG_ERR, "PixelFormat mismatch extchn(%d) - bindchn(%d)\n"
			, pstExtChnAttr->enPixelFormat, pstChnAttr->enPixelFormat);
		return CVI_ERR_VI_INVALID_PARA;
	}
	gViCtx->stExtChnAttr[ViChn - VI_EXT_CHN_START] = *pstExtChnAttr;
#endif
	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_GetExtChnAttr(VI_PIPE ViPipe, VI_CHN ViChn, VI_EXT_CHN_ATTR_S *pstExtChnAttr)
{
	CHECK_VI_NULL_PTR(pstExtChnAttr);
	CHECK_VI_PIPEID_VALID(ViPipe);
	CHECK_VI_EXTCHNID_VALID(ViChn);
#if defined(POSSIBLE_DEAD_CODE)
	if (CHECK_VI_CTX_NULL_PTR(gViCtx) != CVI_SUCCESS)
		return CVI_ERR_VI_FAILED_NOTCONFIG;

	*pstExtChnAttr = gViCtx->stExtChnAttr[ViChn - VI_EXT_CHN_START];
#endif
	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_SetMipiBindDev(VI_DEV ViDev, MIPI_DEV MipiDev)
{
	CHECK_VI_DEVID_VALID(ViDev);

	UNUSED(MipiDev);
	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_GetMipiBindDev(VI_DEV ViDev, MIPI_DEV *pMipiDev)
{
	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pMipiDev);

	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_SetDevAttrEx(VI_DEV ViDev, const VI_DEV_ATTR_EX_S *pstDevAttrEx)
{
	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pstDevAttrEx);
#if defined(POSSIBLE_DEAD_CODE)
	memset(&gViCtx->devAttrEx[ViDev], 0, sizeof(gViCtx->devAttrEx[ViDev]));
	gViCtx->devAttrEx[ViDev] = *pstDevAttrEx;
#endif
	return CVI_ERR_VI_NOT_SUPPORT;
}

/**
 * @deprecated
 */
CVI_S32 CVI_VI_GetDevAttrEx(VI_DEV ViDev, VI_DEV_ATTR_EX_S *pstDevAttrEx)
{
	CHECK_VI_DEVID_VALID(ViDev);
	CHECK_VI_NULL_PTR(pstDevAttrEx);
#if defined(POSSIBLE_DEAD_CODE)
	if (CHECK_VI_CTX_NULL_PTR(gViCtx) != CVI_SUCCESS)
		return CVI_ERR_VI_FAILED_NOTCONFIG;

	*pstDevAttrEx = gViCtx->devAttrEx[ViDev];
#endif
	return CVI_ERR_VI_NOT_SUPPORT;
}

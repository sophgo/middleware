#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include <fcntl.h> /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include "cvi_sys.h"
#include "cvi_ive.h"
#include "ive_uapi.h"

#define IVE_DEV_NODE "/dev/soph-ive"
#define STRFY(s) #s

typedef void *IVE_HANDLE;

enum DIRECTION {
	VERTICAL = 0,
	HORIZON = 1,
	SLASH = 2,
	BACK_SLASH = 3,
	ZERO = 4,
};

struct IVE_HANDLE_CTX {
	int devfd;
	int used_time;
};

struct IVE_HANDLE_CTX handle_ctx;

const char *cviIveImgEnTypeStr[] = {
	STRFY(IVE_IMAGE_TYPE_U8C1),	    STRFY(IVE_IMAGE_TYPE_S8C1),
	STRFY(IVE_IMAGE_TYPE_YUV420SP),	    STRFY(IVE_IMAGE_TYPE_YUV422SP),
	STRFY(IVE_IMAGE_TYPE_YUV420P),	    STRFY(IVE_IMAGE_TYPE_YUV422P),
	STRFY(IVE_IMAGE_TYPE_S8C2_PACKAGE), STRFY(IVE_IMAGE_TYPE_S8C2_PLANAR),
	STRFY(IVE_IMAGE_TYPE_S16C1),	    STRFY(IVE_IMAGE_TYPE_U16C1),
	STRFY(IVE_IMAGE_TYPE_U8C3_PACKAGE), STRFY(IVE_IMAGE_TYPE_U8C3_PLANAR),
	STRFY(IVE_IMAGE_TYPE_S32C1),	    STRFY(IVE_IMAGE_TYPE_U32C1),
	STRFY(IVE_IMAGE_TYPE_S64C1),	    STRFY(IVE_IMAGE_TYPE_U64C1),
	STRFY(IVE_IMAGE_TYPE_BF16C1),	    STRFY(IVE_IMAGE_TYPE_FP32C1)
};

inline CVI_S32 CHECK_NULL_PTR(void *ptr)
{
	CVI_S32 s32Ret;

	if (!ptr) {
		printf("input ptr is NULL, please check!\n");
		s32Ret = CVI_ERR_IVE_NULL_PTR;
	} else {
		s32Ret = CVI_SUCCESS;
	}

	return s32Ret;
}

CVI_S32 CHECK_ARG_IN_RANGE(unsigned int cmd, void *pstctrl)
{
	CVI_S32 s32Ret;

	switch (cmd) {
	case CVI_IVE_IOC_ORD_STAT_FILTER: {
		IVE_ORD_STAT_FILTER_CTRL_S *ctrl;
		ctrl = (IVE_ORD_STAT_FILTER_CTRL_S *)pstctrl;

		if (ctrl->enMode > IVE_ORD_STAT_FILTER_MODE_MIN ||
			ctrl->enMode < IVE_ORD_STAT_FILTER_MODE_MEDIAN) {
			printf("ORD_STAT_FILTER input arg csc_mode=%d out of range\n",
					ctrl->enMode);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}
	case CVI_IVE_IOC_SUB: {
		IVE_SUB_CTRL_S *ctrl;
		ctrl = (IVE_SUB_CTRL_S *)pstctrl;

		if (ctrl->enMode > IVE_SUB_MODE_SHIFT ||
			ctrl->enMode < IVE_SUB_MODE_ABS) {
			printf("SUB input arg csc_mode=%d out of range\n",
					ctrl->enMode);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}
	case CVI_IVE_IOC_DMA: {
		IVE_DMA_CTRL_S *dma_ctrl;
		dma_ctrl = (IVE_DMA_CTRL_S *)pstctrl;

		if (dma_ctrl->enMode > 3 || dma_ctrl->enMode < 0) {
			printf("DMA input arg  dma_mode=%d out of range\n",
					dma_ctrl->enMode);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}

		if (dma_ctrl->enMode == IVE_DMA_MODE_INTERVAL_COPY &&
			dma_ctrl->u8HorSegSize != 2 &&
			dma_ctrl->u8HorSegSize != 3 &&
			dma_ctrl->u8HorSegSize != 4 &&
			dma_ctrl->u8HorSegSize != 8 &&
			dma_ctrl->u8HorSegSize != 16) {
			printf("DMA input arg  hor_seg_size=%d out of range\n",
					dma_ctrl->u8HorSegSize);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}
	case CVI_IVE_IOC_FILTER_AND_CSC: {
		IVE_FILTER_AND_CSC_CTRL_S *ctrl;
		ctrl = (IVE_FILTER_AND_CSC_CTRL_S *)pstctrl;

		if (ctrl->enMode > IVE_CSC_MODE_PIC_BT709_YUV2LAB ||
			ctrl->enMode < IVE_CSC_MODE_VIDEO_BT601_YUV2RGB) {
			printf("FILTER_AND_CSC input arg csc_mode=%d out of range\n",
					ctrl->enMode);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}
	case CVI_IVE_IOC_CSC: {
		IVE_CSC_CTRL_S *ctrl;
		ctrl = (IVE_CSC_CTRL_S *)pstctrl;

		if (ctrl->enMode > IVE_CSC_MODE_PIC_BT709_RGB2YUV ||
			ctrl->enMode < IVE_CSC_MODE_VIDEO_BT601_YUV2RGB) {
			printf("CSC input arg csc_mode=%d out of range\n",
					ctrl->enMode);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}
	case CVI_IVE_IOC_SOBEL: {
		IVE_SOBEL_CTRL_S *ctrl;
		ctrl = (IVE_SOBEL_CTRL_S *)pstctrl;

		if (ctrl->enOutCtrl > IVE_SOBEL_OUT_CTRL_VER ||
			ctrl->enOutCtrl < IVE_SOBEL_OUT_CTRL_BOTH) {
			printf("SOBEL input arg out_ctrl=%d out of range\n",
					ctrl->enOutCtrl);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}
	case CVI_IVE_IOC_NORMGRAD: {
		IVE_NORM_GRAD_CTRL_S *ctrl;
		ctrl = (IVE_NORM_GRAD_CTRL_S *)pstctrl;

		if (ctrl->enOutCtrl > IVE_NORM_GRAD_OUT_CTRL_COMBINE ||
			ctrl->enOutCtrl < IVE_NORM_GRAD_OUT_CTRL_HOR_AND_VER) {
			printf("NORMGRAD input arg out_ctrl=%d out of range\n",
					ctrl->enOutCtrl);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}
	case CVI_IVE_IOC_MAG_AND_ANG: {
		IVE_MAG_AND_ANG_CTRL_S *ctrl;
		ctrl = (IVE_MAG_AND_ANG_CTRL_S *)pstctrl;

		if (ctrl->enOutCtrl > IVE_MAG_AND_ANG_OUT_CTRL_MAG_AND_ANG ||
			ctrl->enOutCtrl < IVE_MAG_AND_ANG_OUT_CTRL_MAG) {
			printf("MAG_AND_ANG input arg out_ctrl=%d out of range\n",
					ctrl->enOutCtrl);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}
	case CVI_IVE_IOC_THRESH: {
		IVE_THRESH_CTRL_S *ctrl;
		ctrl = (IVE_THRESH_CTRL_S *)pstctrl;

		if (ctrl->enMode > IVE_THRESH_MODE_ORI_MID_ORI ||
			ctrl->enMode < IVE_THRESH_MODE_BINARY) {
			printf("THRESH input arg out_ctrl=%d out of range\n",
					ctrl->enMode);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}
	case CVI_IVE_IOC_MAP: {
		IVE_MAP_CTRL_S *ctrl;
		ctrl = (IVE_MAP_CTRL_S *)pstctrl;

		if (ctrl->enMode > IVE_MAP_MODE_U16 ||
			ctrl->enMode < IVE_MAP_MODE_U8) {
			printf("MAP input arg mode=%d out of range\n",
					ctrl->enMode);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}
	case CVI_IVE_IOC_INTEG: {
		IVE_INTEG_CTRL_S *ctrl;
		ctrl = (IVE_INTEG_CTRL_S *)pstctrl;

		if (ctrl->enOutCtrl > IVE_INTEG_OUT_CTRL_SQSUM ||
			ctrl->enOutCtrl < IVE_INTEG_OUT_CTRL_COMBINE) {
			printf("INTEG input arg out_ctrl=%d out of range\n",
					ctrl->enOutCtrl);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}
	case CVI_IVE_IOC_MATCH_BGMODEM: {
		IVE_MATCH_BG_MODEL_CTRL_S *ctrl;
		ctrl = (IVE_MATCH_BG_MODEL_CTRL_S *)pstctrl;

		if (ctrl->u16TimeThr > 100 ||
			ctrl->u16TimeThr < 2) {
			printf("MATCH_BGMODEL input arg time_thr=%d out of range\n",
					ctrl->u16TimeThr);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u8DiffMaxThr > 15 ||
					ctrl->u8DiffMaxThr < 3 ) {
			printf("MATCH_BGMODEL input arg diff_max_thr=%d out of range\n",
					ctrl->u8DiffMaxThr);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u8DiffThrCrlCoef > 5) {
			printf("MATCH_BGMODEL input arg diff_thr_crl_coef=%d out of range\n",
					ctrl->u8DiffThrCrlCoef);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u8DiffMinThr > 15 || ctrl->u8DiffMinThr < 3) {
			printf("MATCH_BGMODEL input arg diff_min_thr=%d out of range\n",
					ctrl->u8DiffMinThr);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u8DiffThrInc > 6 ) {
			printf("MATCH_BGMODEL input arg diff_thr_inc=%d out of range\n",
					ctrl->u8DiffThrInc);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u8FastLearnRate > 4) {
			printf("MATCH_BGMODEL input arg fast_learn_rate=%d out of range\n",
					ctrl->u8FastLearnRate);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u8DetChgRegion > 1) {
			printf("MATCH_BGMODEL input arg det_chg_region=%d out of range\n",
					ctrl->u8DetChgRegion);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}
	case CVI_IVE_IOC_UPDATE_BGMODEL: {
		IVE_UPDATE_BG_MODEL_CTRL_S *ctrl;
		ctrl = (IVE_UPDATE_BG_MODEL_CTRL_S *)pstctrl;

		if (ctrl->u32FrmChkPeriod > 2000) {
			printf("UPDATE_BGMODEL input arg frm_chk_period=%d out of range\n",
					ctrl->u32FrmChkPeriod);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u32InitMinTime > 6000 ||
					ctrl->u32InitMinTime < 20) {
			printf("UPDATE_BGMODEL input arg init_min_time=%d out of range\n",
					ctrl->u32InitMinTime);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u32StyBgMinBlendTime > 6000||
					ctrl->u32StyBgMinBlendTime < 20) {
			printf("UPDATE_BGMODEL input arg sty_bg_min_blend_time=%d out of range\n",
					ctrl->u32StyBgMinBlendTime);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u32StyBgMaxBlendTime > 40000 ||
					ctrl->u32StyBgMaxBlendTime < 20) {
			printf("UPDATE_BGMODEL input arg sty_bg_max_blend_time=%d out of range\n",
					ctrl->u32StyBgMaxBlendTime);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u32DynBgMinBlendTime > 6000) {
			printf("UPDATE_BGMODEL input arg dyn_bg_min_blend_time=%d out of range\n",
					ctrl->u32DynBgMinBlendTime);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u32StaticDetMinTime > 6000 ||
					ctrl->u32StaticDetMinTime < 20) {
			printf("UPDATE_BGMODEL input arg static_det_min_time=%d out of range\n",
					ctrl->u32StaticDetMinTime);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u16FgMaxFadeTime > 255 ||
					ctrl->u16FgMaxFadeTime < 1) {
			printf("UPDATE_BGMODEL input arg fg_max_fade_time=%d out of range\n",
					ctrl->u16FgMaxFadeTime);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u16BgMaxFadeTime > 255 ||
					ctrl->u16BgMaxFadeTime < 1) {
			printf("UPDATE_BGMODEL input arg bg_max_fade_time=%d out of range\n",
					ctrl->u16BgMaxFadeTime);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u8StyBgAccTimeRateThr > 100 ||
					ctrl->u8StyBgAccTimeRateThr < 10) {
			printf("UPDATE_BGMODEL input arg sty_bg_acc_time_rate_thr=%d out of range\n",
					ctrl->u8StyBgAccTimeRateThr);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u8ChgBgAccTimeRateThr > 100 ||
					ctrl->u8ChgBgAccTimeRateThr < 10) {
			printf("UPDATE_BGMODEL input arg chg_bg_acc_time_rate_thr=%d out of range\n",
					ctrl->u8ChgBgAccTimeRateThr);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u8DynBgAccTimeThr > 50) {
			printf("UPDATE_BGMODEL input arg dyn_bg_acc_time_thr=%d out of range\n",
					ctrl->u8DynBgAccTimeThr);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u8DynBgDepth > 3) {
			printf("UPDATE_BGMODEL input arg dyn_bg_depth=%d out of range\n",
					ctrl->u8DynBgDepth);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u8BgEffStaRateThr > 100 ||
					ctrl->u8BgEffStaRateThr < 90) {
			printf("UPDATE_BGMODEL input arg bg_eff_sta_rate_thr=%d out of range\n",
					ctrl->u8BgEffStaRateThr);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u8AcceBgLearn > 1) {
			printf("UPDATE_BGMODEL input arg acce_bg_learn=%d out of range\n",
					ctrl->u8AcceBgLearn);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u8DetChgRegion > 1) {
			printf("UPDATE_BGMODEL input arg det_chg_region=%d out of range\n",
					ctrl->u8DetChgRegion);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}
	case CVI_IVE_IOC_GRADFG: {
		IVE_GRAD_FG_CTRL_S *ctrl;
		ctrl = (IVE_GRAD_FG_CTRL_S *)pstctrl;

		if (ctrl->enMode > IVE_GRAD_FG_MODE_FIND_MIN_GRAD ||
			ctrl->enMode < IVE_GRAD_FG_MODE_USE_CUR_GRAD) {
			printf("GRADFG input arg mode=%d out of range\n",
					ctrl->enMode);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u16EdwFactor > 2000 || ctrl->u16EdwFactor < 500) {
			printf("GRADFG input arg edw_factor=%d out of range\n",
					ctrl->u16EdwFactor);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u8CrlCoefThr > 100 || ctrl->u8CrlCoefThr < 50) {
			printf("GRADFG input arg crl_coe_thr=%d out of range\n",
					ctrl->u8CrlCoefThr);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u8MagCrlThr > 20) {
			printf("GRADFG input arg mag_crl_thr=%d out of range\n",
					ctrl->u8MagCrlThr);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u8MinMagDiff > 8 || ctrl->u8MinMagDiff < 2) {
			printf("GRADFG input arg min_mag_diff=%d out of range\n",
					ctrl->u8MinMagDiff);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u8NoiseVal > 8 || ctrl->u8NoiseVal < 1) {
			printf("GRADFG input arg noise_val=%d out of range\n",
					ctrl->u8NoiseVal);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u8EdwDark > 8 || ctrl->u8EdwDark < 1) {
			printf("GRADFG input arg edw_dark=%d out of range\n",
					ctrl->u8EdwDark);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}
	case CVI_IVE_IOC_GMM: {
		IVE_GMM_CTRL_S *ctrl;
		ctrl = (IVE_GMM_CTRL_S *)pstctrl;

		if (ctrl->u8ModelNum != 5 &&
			ctrl->u8ModelNum != 3) {
			printf("GMM input arg model_num=%d out of range\n",
					ctrl->u8ModelNum);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}
	case CVI_IVE_IOC_GMM2: {
		IVE_GMM2_CTRL_S *ctrl;
		ctrl = (IVE_GMM2_CTRL_S *)pstctrl;

		if (ctrl->enSnsFactorMode > IVE_GMM2_SNS_FACTOR_MODE_PIX ||
			ctrl->enSnsFactorMode < IVE_GMM2_SNS_FACTOR_MODE_GLB) {
			printf("GMM2 input arg sns_factor_mode=%d out of range\n",
					ctrl->enSnsFactorMode);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->enLifeUpdateFactorMode > IVE_GMM2_LIFE_UPDATE_FACTOR_MODE_PIX ||
					ctrl->enLifeUpdateFactorMode < IVE_GMM2_LIFE_UPDATE_FACTOR_MODE_GLB) {
			printf("GMM2 input arg life_update_factor_mode=%d out of range\n",
					ctrl->enLifeUpdateFactorMode);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->u8ModelNum > 5 ||
					ctrl->u8ModelNum < 1) {
			printf("GMM2 input arg model_num=%d out of range\n",
					ctrl->u8ModelNum);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}
	case CVI_IVE_IOC_BERNSEN: {
		IVE_BERNSEN_CTRL_S *ctrl;
		ctrl = (IVE_BERNSEN_CTRL_S *)pstctrl;

		if (ctrl->enMode > IVE_BERNSEN_MODE_PAPER ||
			ctrl->enMode < IVE_BERNSEN_MODE_NORMAL) {
			printf("BERNSEN input arg mode=%d out of range\n",
					ctrl->enMode);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}
	case CVI_IVE_IOC_CCL: {
		IVE_CCL_CTRL_S *ctrl;
		ctrl = (IVE_CCL_CTRL_S *)pstctrl;

		if (ctrl->enMode > IVE_CCL_MODE_8C ||
			ctrl->enMode < IVE_CCL_MODE_4C) {
			printf("CCL input arg mode=%d out of range\n",
					ctrl->enMode);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}
	case CVI_IVE_IOC_LBP: {
		IVE_LBP_CTRL_S *ctrl;
		ctrl = (IVE_LBP_CTRL_S *)pstctrl;

		if (ctrl->enMode > IVE_LBP_CMP_MODE_ABS ||
			ctrl->enMode < IVE_LBP_CMP_MODE_NORMAL) {
			printf("LBP input arg mode=%d out of range\n",
					ctrl->enMode);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}
	case CVI_IVE_IOC_THRESH_S16: {
		IVE_THRESH_S16_CTRL_S *ctrl;
		ctrl = (IVE_THRESH_S16_CTRL_S *)pstctrl;

		if (ctrl->enMode > IVE_THRESH_S16_MODE_S16_TO_U8_MIN_ORI_MAX ||
			ctrl->enMode < IVE_THRESH_S16_MODE_S16_TO_S8_MIN_MID_MAX) {
			printf("THRESH_S16 input arg mode=%d out of range\n",
					ctrl->enMode);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}
	case CVI_IVE_IOC_THRESH_U16: {
		IVE_THRESH_U16_CTRL_S *ctrl;
		ctrl = (IVE_THRESH_U16_CTRL_S *)pstctrl;

		if (ctrl->enMode > IVE_THRESH_U16_MODE_U16_TO_U8_MIN_ORI_MAX ||
			ctrl->enMode < IVE_THRESH_U16_MODE_U16_TO_U8_MIN_MID_MAX) {
			printf("THRESH_U16 input arg mode=%d out of range\n",
					ctrl->enMode);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}
	case CVI_IVE_IOC_16BIT_TO_8BIT: {
		IVE_16BIT_TO_8BIT_CTRL_S *ctrl;
		ctrl = (IVE_16BIT_TO_8BIT_CTRL_S *)pstctrl;

		if (ctrl->enMode > IVE_16BIT_TO_8BIT_MODE_U16_TO_U8 ||
			ctrl->enMode < IVE_16BIT_TO_8BIT_MODE_S16_TO_S8) {
			printf("16BIT_TO_8BIT input arg mode=%d out of range\n",
					ctrl->enMode);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}
	case CVI_IVE_IOC_MD: {
		IVE_FRAME_DIFF_MOTION_CTRL_S *ctrl;
		ctrl = (IVE_FRAME_DIFF_MOTION_CTRL_S *)pstctrl;

		if (ctrl->enSubMode > IVE_SUB_MODE_SHIFT ||
			ctrl->enSubMode < IVE_SUB_MODE_ABS) {
			printf("MD input arg sub_mode=%d out of range\n",
					ctrl->enSubMode);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->enThrMode > IVE_THRESH_MODE_ORI_MID_ORI ||
					ctrl->enThrMode < IVE_THRESH_MODE_BINARY) {
			printf("MD input arg thr_mode=%d out of range\n",
					ctrl->enThrMode);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}
	case CVI_IVE_IOC_SAD: {
		IVE_SAD_CTRL_S * ctrl;
		ctrl = (IVE_SAD_CTRL_S *)pstctrl;
		if (ctrl->enMode > IVE_SAD_MODE_MB_16X16 ||
			ctrl->enMode < IVE_SAD_MODE_MB_4X4) {
			printf("SAD input arg mode=%d out of range\n",
					ctrl->enMode);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		} else if (ctrl->enOutCtrl > IVE_SAD_OUT_CTRL_THRESH ||
					ctrl->enOutCtrl < IVE_SAD_OUT_CTRL_16BIT_BOTH) {
			printf("SAD input arg enOutCtrl=%d out of range\n",
					ctrl->enOutCtrl);
			s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
			return s32Ret;
		}
		break;
	}

	default: {
		// printf("input cmd[%d] has not arg to detect\n", cmd);
		s32Ret = CVI_SUCCESS;
		return s32Ret;
	}
	}
	s32Ret = CVI_SUCCESS;
	return s32Ret;
}

inline CVI_S32 CHECK_NULL_PTR_ARRAY(void* ptr[], int num)
{
	CVI_S32 s32Ret;

	for(int i = 0; i < num; i++) {
		s32Ret = CHECK_NULL_PTR(ptr[i]);
		if (s32Ret != CVI_SUCCESS)
			break;
	}
	return s32Ret;
}

CVI_S32 CHECK_INPUT_IMAGE_FORMAT(IVE_IMAGE_S *pstimage)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	if (pstimage->enType < IVE_IMAGE_TYPE_U8C1 ||
		pstimage->enType > IVE_IMAGE_TYPE_U64C1) {
		printf("input image[%p] format error\n", pstimage);
		s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
	}
	return s32Ret;
}

CVI_S32 CHECK_INPUT_IMAGE_SIZE(IVE_IMAGE_S *pstimage)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	if (pstimage->u32Width < 64 || pstimage->u32Width > 1920 ||
		pstimage->u32Height < 64 || pstimage->u32Height > 1080) {
		printf("input image[%p] width[%d] height[%d] over range\n",
				pstimage, pstimage->u32Width, pstimage->u32Height);
		s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
	}
	return s32Ret;
}

CVI_S32 CHECK_INPUT_FORMAT(IVE_IMAGE_TYPE_E enType)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (enType < IVE_IMAGE_TYPE_U8C1 ||
		enType > IVE_IMAGE_TYPE_U64C1) {
		printf("input enType[%d] out of range\n", enType);
		s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
	}
	return s32Ret;
}

CVI_S32 CHECK_FUNC_ALL(void* ptr[], int num, IVE_IMAGE_S *pst_image,
						void *ptr_ctrl, unsigned int cmd)
{
	CVI_S32 s32Ret;

	s32Ret = CHECK_NULL_PTR_ARRAY(ptr, num);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	if (pst_image) {
		s32Ret = CHECK_INPUT_IMAGE_FORMAT(pst_image);
		if (s32Ret != CVI_SUCCESS)
			return s32Ret;
		s32Ret = CHECK_INPUT_IMAGE_SIZE(pst_image);
		if (s32Ret != CVI_SUCCESS)
			return s32Ret;
	}

	if (ptr_ctrl) {
		s32Ret = CHECK_ARG_IN_RANGE(cmd, ptr_ctrl);
		if (s32Ret != CVI_SUCCESS)
			return s32Ret;
	}

	return s32Ret;
}

int open_ive_dev()
{
	if (handle_ctx.devfd != 0)
		return handle_ctx.devfd;

	int devfd;

	do {
		if (handle_ctx.devfd <= 0)
			devfd = open(IVE_DEV_NODE, O_RDWR);
		else
			devfd = handle_ctx.devfd;
		if (devfd < 0) {
			printf("Can't open %s\n", IVE_DEV_NODE);
			return ERR_IVE_OPEN_FILE;
		}
	} while (0);
	return devfd;
}

uint32_t WidthAlign(const uint32_t width, const uint32_t align)
{
	uint32_t stride = (uint32_t)(width / align) * align;
	if (stride < width) {
		stride += align;
	}
	return stride;
}

void dump_data_u8(const CVI_U8 *data, CVI_S32 count, const CVI_CHAR *desc)
{
	CVI_S32 i;

	printf("%s\n", desc);
	for (i = 0; i < count; ++i) {
		printf("%02X ", data[i]);
		if ((i + 1) % 8 == 0 || i + 1 == count) {
			printf(" ");
			if ((i + 1) % 16 == 0) {
				printf("\n");
			} else if (i + 1 == count) {
				printf("\n");
			}
		}
	}
}

static CVI_S32 find_first_diff_u8(const CVI_U8 *A, const CVI_U8 *B,
				  CVI_S32 count)
{
	CVI_S32 i;
	CVI_S32 result = -1;
	for (i = 0; i < count; ++i) {
		if (A[i] != B[i]) {
			result = i;
			break;
		}
	}
	return result;
}

static CVI_S32 find_first_diff_u16(const CVI_U16 *A, const CVI_U16 *B,
				   CVI_S32 count)
{
	CVI_S32 i;
	CVI_S32 result = -1;
	for (i = 0; i < count; ++i) {
		if (A[i] != B[i]) {
			result = i;
			break;
		}
	}
	return result;
}

void dump_data_u16(const CVI_U16 *data, CVI_S32 count, const CVI_CHAR *desc)
{
	CVI_S32 i;
	printf("%s\n", desc);
	for (i = 0; i < count; ++i) {
		printf("%04X ", data[i]);
		if ((i + 1) % 8 == 0 || i + 1 == count) {
			printf(" ");
			if ((i + 1) % 16 == 0) {
				printf("\n");
			} else if (i + 1 == count) {
				printf("\n");
			}
		}
	}
}

static CVI_S32 find_first_diff_u32(const CVI_U32 *A, const CVI_U32 *B,
				   CVI_S32 count)
{
	CVI_S32 i;
	CVI_S32 result = -1;
	for (i = 0; i < count; ++i) {
		if (A[i] != B[i]) {
			result = i;
			break;
		}
	}
	return result;
}
void dump_data_u32(const CVI_U32 *data, CVI_S32 count, const CVI_CHAR *desc)
{
	CVI_S32 i;
	printf("%s\n", desc);
	for (i = 0; i < count; ++i) {
		printf("%08X ", data[i]);
		if ((i + 1) % 4 == 0 || i + 1 == count) {
			printf(" ");
			if ((i + 1) % 8 == 0) {
				printf("\n");
			} else if (i + 1 == count) {
				printf("\n");
			}
		}
	}
}

static CVI_S32 find_first_diff_u64(const CVI_U64 *A, const CVI_U64 *B,
				   CVI_S32 count)
{
	CVI_S32 i;
	CVI_S32 result = -1;
	for (i = 0; i < count; ++i) {
		if (A[i] != B[i]) {
			result = i;
			break;
		}
	}
	return result;
}

void dump_data_u64(const CVI_U64 *data, CVI_S32 count, const CVI_CHAR *desc)
{
	CVI_S32 i;
	printf("%s\n", desc);
	for (i = 0; i < count; ++i) {
		printf("%016jX ", data[i]);
		if ((i + 1) % 2 == 0 || i + 1 == count) {
			printf(" ");
			if ((i + 1) % 4 == 0) {
				printf("\n");
			} else if (i + 1 == count) {
				printf("\n");
			}
		}
	}
}

void boundary_check(IVE_POINT_U16_S *point, int width, int height)
{
	if ((CVI_S16)point->u16X < 0) {
		point->u16X = 0;
	}
	if ((CVI_S16)point->u16Y < 0) {
		point->u16Y = 0;
	}
	if (point->u16X >= width) {
		point->u16X = width - 1;
	}
	if (point->u16Y >= height) {
		point->u16Y = height - 1;
	}
}

int do_hysteresis_wo_ang(unsigned char *pEdgeMap,
			 unsigned char *dst, int imw, int imh, int stride)
{
	IVE_POINT_U16_S p1, p2, p;
	unsigned char *visited =
		(unsigned char *)calloc(imh * stride, sizeof(unsigned char));
	unsigned char *pdst =
		(unsigned char *)calloc(imh * stride, sizeof(unsigned char));

	for (int i = 0; i < imh; i++) {
		for (int j = 0; j < imw; j++) {
			int index = i * stride + j;

			if (pEdgeMap[index] == 2) {
				pdst[index] = 255;
			} else {
				pdst[index] = 0;
			}
		}
	}

	for (int y = 1; y < imh - 1; y++) {
		for (int x = 1; x < imw - 1; x++) {
			// if pixel not in upper or had visited, skip
			if (pEdgeMap[y * stride + x] != 2 ||
				visited[y * stride + x] > 0)
				continue;
			p.u16X = x;
			p.u16Y = y;
			visited[p.u16Y * stride + p.u16X] = 255;
			pdst[p.u16Y * stride + p.u16X] = 255;
			for (unsigned char dir = 0; dir < 4; dir++) {
				if (dir == VERTICAL) {
					p1.u16X = p.u16X + 1;
					p1.u16Y = p.u16Y;
					p2.u16X = p.u16X - 1;
					p2.u16Y = p.u16Y;
				} else if (dir == HORIZON) {
					p1.u16X = p.u16X;
					p1.u16Y = p.u16Y + 1;
					p2.u16X = p.u16X;
					p2.u16Y = p.u16Y - 1;
				} else if (dir == SLASH) {
					p1.u16X = p.u16X + 1;
					p1.u16Y = p.u16Y - 1;
					p2.u16X = p.u16X - 1;
					p2.u16Y = p.u16Y + 1;
				} else if (dir == BACK_SLASH) {
					p1.u16X = p.u16X + 1;
					p1.u16Y = p.u16Y + 1;
					p2.u16X = p.u16X - 1;
					p2.u16Y = p.u16Y - 1;
				} else {
					p1.u16X = p.u16X;
					p1.u16Y = p.u16Y;
					p2.u16X = p.u16X;
					p2.u16Y = p.u16Y;
				}
				boundary_check(&p1, imw, imh);
				boundary_check(&p2, imw, imh);
				if (pEdgeMap[p1.u16Y * stride + p1.u16X] == 0 &&
					visited[p1.u16Y * stride + p1.u16X] == 0) {
					visited[p1.u16Y * stride + p1.u16X] = 255;
					pdst[p1.u16Y * stride + p1.u16X] = 255;
				}
				if (pEdgeMap[p2.u16Y * stride + p2.u16X] == 0 &&
					visited[p2.u16Y * stride + p2.u16X] == 0) {
					visited[p2.u16Y * stride + p2.u16X] = 255;
					pdst[p2.u16Y * stride + p2.u16X] = 255;
				}
			}
		}
	}

	memcpy(dst, pdst, stride * imh);
	free(visited);
	free(pdst);
	return 0;
}

IVE_HANDLE CVI_IVE_CreateHandle()
{
	handle_ctx.devfd = open_ive_dev();
	handle_ctx.used_time++;
	return &handle_ctx;
}

CVI_S32 CVI_IVE_DestroyHandle(IVE_HANDLE pIveHandle)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 1;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, NULL, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;

	p->used_time--;
	if (p->used_time <= 0 && p->devfd > 0) {
		close(p->devfd);
		p->devfd = 0;
	}
	return s32Ret;
}

CVI_S32 CVI_IVE_CompareIveData(IVE_DATA_S *pstData1, IVE_DATA_S *pstData2)
{
	int n;
	CVI_U32 y;

	if (pstData1->u32Width != pstData2->u32Width) {
		printf("Not same u32Width, %d vs %d\n", pstData1->u32Width,
			   pstData2->u32Width);
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	if (pstData1->u32Height != pstData2->u32Height) {
		printf("Not same u32Height, %d vs %d\n", pstData1->u32Height,
			   pstData2->u32Height);
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	printf("compare A at %p, B at %p\n",
		   (char *)(uintptr_t)pstData1->u64VirAddr,
		   (char *)(uintptr_t)pstData2->u64VirAddr);


	for (y = 0; y < pstData1->u32Height; y++) {
		n = memcmp((char *)(uintptr_t)pstData1->u64VirAddr + y * pstData1->u32Stride,
		(char *)(uintptr_t)pstData2->u64VirAddr + y * pstData1->u32Stride,
		pstData1->u32Width);
		if (n != 0) {
			printf("Not same content\n");
			return CVI_FAILURE;
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_IVE_CompareIveMem(IVE_MEM_INFO_S *pstMem1, IVE_MEM_INFO_S *pstMem2)
{
	int n;

	if (pstMem1->u32Size != pstMem2->u32Size || pstMem1->u32Size == 0) {
		printf("Not same u32Size, %d vs %d\n", pstMem1->u32Size,
			   pstMem2->u32Size);
		return CVI_ERR_IVE_INVALID_DEVID;
	}

	if ((char *)(uintptr_t)pstMem1->u64VirAddr == NULL ||
		(char *)(uintptr_t)pstMem2->u64VirAddr == NULL) {
		printf("invalid address\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}

	printf("compare A at %p, B at %p\n",
		   (char *)(uintptr_t)pstMem1->u64VirAddr,
		   (char *)(uintptr_t)pstMem2->u64VirAddr);
	n = memcmp((char *)(uintptr_t)pstMem1->u64VirAddr,
		   (char *)(uintptr_t)pstMem2->u64VirAddr, pstMem1->u32Size);
	if (n != 0) {
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_IVE_CompareIveImage(IVE_IMAGE_S *pstImage1, IVE_IMAGE_S *pstImage2)
{
	CVI_U32 u32Stride;
	CVI_S32 s32Succ;
	IVE_IMAGE_TYPE_E enType;
	CVI_U32 u32Width;
	CVI_U32 u32Height;
	CVI_U8 *pData1;
	CVI_U8 *pData2;
	CVI_U16 *pData1_U16;
	CVI_U16 *pData2_U16;
	CVI_U32 *pData1_U32;
	CVI_U32 *pData2_U32;
	CVI_U64 *pData1_U64;
	CVI_U64 *pData2_U64;
	CVI_U16 y;
	int n;

	if (pstImage1->enType != pstImage2->enType) {
		printf("Not same IMAGE_TYPE, %d vs %d\n", pstImage1->enType,
			   pstImage2->enType);
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	if (pstImage1->u32Width != pstImage2->u32Width) {
		printf("Not same u32Width, %d vs %d\n", pstImage1->u32Width,
			   pstImage2->u32Width);
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	if (pstImage1->u32Height != pstImage2->u32Height) {
		printf("Not same u32Height, %d vs %d\n", pstImage1->u32Height,
			   pstImage2->u32Height);
		return CVI_ERR_IVE_INVALID_DEVID;
	}

	enType = pstImage1->enType;
	u32Width = pstImage1->u32Width;
	u32Height = pstImage1->u32Height;

	//u32Stride = CVI_CalcStride(u32Width, CVI_IVE2_STRIDE_ALIGN);
	u32Stride = pstImage1->u32Stride[0];//WidthAlign(u32Width, IVE_DEFAULT_ALIGN);
	s32Succ = CVI_SUCCESS;

	switch (enType) {
	case IVE_IMAGE_TYPE_U8C1:
	case IVE_IMAGE_TYPE_S8C1: {
		pData1 = (CVI_U8 *)(uintptr_t)pstImage1->u64VirAddr[0];
		pData2 = (CVI_U8 *)(uintptr_t)pstImage2->u64VirAddr[0];
		printf("compare A at %p, B at %p\n", pData1, pData2);
		for (y = 0; y < u32Height;
			 y++, pData1 += u32Stride, pData2 += u32Stride) {
			n = memcmp(pData1, pData2, u32Width);
			if (n != 0) {
				int idx = find_first_diff_u8(pData1, pData2,
								 u32Width);
				CVI_ASSERT(idx >= 0);
				printf("Compare failed, at y = %d, x = %d, n = %d\n",
					   y, idx, n);
				printf("  A = 0x%02X, B = 0x%02X\n",
					   pData1[idx], pData2[idx]);
				dump_data_u8(pData1, u32Width, "A");
				dump_data_u8(pData2, u32Width, "B");
				s32Succ = CVI_FAILURE;
				break;
			}
		}
	} break;
	case IVE_IMAGE_TYPE_YUV420SP: {
		pData1 = (CVI_U8 *)(uintptr_t)pstImage1->u64VirAddr[0];
		pData2 = (CVI_U8 *)(uintptr_t)pstImage2->u64VirAddr[0];
		printf("compareY A at %p, B at %p\n", pData1, pData2);
		for (y = 0; y < u32Height;
			 y++, pData1 += u32Stride, pData2 += u32Stride) {
			n = memcmp(pData1, pData2, u32Width);
			if (n != 0) {
				int idx = find_first_diff_u8(pData1, pData2,
								 u32Width);
				CVI_ASSERT(idx >= 0);
				printf("Compare Y failed, at y = %d, x = %d, n = %d\n",
					   y, idx, n);
				printf("  A = 0x%02X, B = 0x%02X\n",
					   pData1[idx], pData2[idx]);
				dump_data_u8(pData1, u32Width, "A");
				dump_data_u8(pData2, u32Width, "B");
				s32Succ = CVI_FAILURE;
				break;
			}
		}

		pData1 = (CVI_U8 *)(uintptr_t)pstImage1->u64VirAddr[1];
		pData2 = (CVI_U8 *)(uintptr_t)pstImage2->u64VirAddr[1];
		printf("compareUV A at %p, B at %p\n", pData1, pData2);
		for (y = 0; y < u32Height / 2;
			 y++, pData1 += u32Stride, pData2 += u32Stride) {
			n = memcmp(pData1, pData2, u32Width);
			if (n != 0) {
				int idx = find_first_diff_u8(pData1, pData2,
								 u32Width);
				CVI_ASSERT(idx >= 0);
				printf("Compare UV failed, at y = %d, x = %d, n = %d\n",
					   y, idx, n);
				printf("  A = 0x%02X, B = 0x%02X\n",
					   pData1[idx], pData2[idx]);
				dump_data_u8(pData1, u32Width, "A");
				dump_data_u8(pData2, u32Width, "B");
				s32Succ = CVI_FAILURE;
				break;
			}
		}
	} break;
	case IVE_IMAGE_TYPE_YUV422SP: {
		pData1 = (CVI_U8 *)(uintptr_t)pstImage1->u64VirAddr[0];
		pData2 = (CVI_U8 *)(uintptr_t)pstImage2->u64VirAddr[0];
		printf("compareY A at %p, B at %p\n", pData1, pData2);
		for (y = 0; y < u32Height;
			 y++, pData1 += u32Stride, pData2 += u32Stride) {
			n = memcmp(pData1, pData2, u32Width);
			if (n != 0) {
				int idx = find_first_diff_u8(pData1, pData2,
								 u32Width);
				CVI_ASSERT(idx >= 0);
				printf("Compare Y failed, at y = %d, x = %d, n = %d\n",
					   y, idx, n);
				printf("  A = 0x%02X, B = 0x%02X\n",
					   pData1[idx], pData2[idx]);
				dump_data_u8(pData1, u32Width, "A");
				dump_data_u8(pData2, u32Width, "B");
				s32Succ = CVI_FAILURE;
				break;
			}
		}

		pData1 = (CVI_U8 *)(uintptr_t)pstImage1->u64VirAddr[1];
		pData2 = (CVI_U8 *)(uintptr_t)pstImage2->u64VirAddr[1];
		printf("compareUV A at %p, B at %p\n", pData1, pData2);
		for (y = 0; y < u32Height;
			 y++, pData1 += u32Stride, pData2 += u32Stride) {
			n = memcmp(pData1, pData2, u32Width);
			if (n != 0) {
				int idx = find_first_diff_u8(pData1, pData2,
								 u32Width);
				CVI_ASSERT(idx >= 0);
				printf("Compare UV failed, at y = %d, x = %d, n = %d\n",
					   y, idx, n);
				printf("  A = 0x%02X, B = 0x%02X\n",
					   pData1[idx], pData2[idx]);
				dump_data_u8(pData1, u32Width, "A");
				dump_data_u8(pData2, u32Width, "B");
				s32Succ = CVI_FAILURE;
				break;
			}
		}
	} break;
	case IVE_IMAGE_TYPE_YUV420P:
	case IVE_IMAGE_TYPE_YUV422P:
	case IVE_IMAGE_TYPE_S8C2_PACKAGE:
	case IVE_IMAGE_TYPE_S8C2_PLANAR: {
		printf("Unsupported IMAGE_TYPE %d\n", enType);
		s32Succ = CVI_FAILURE;
	} break;
	case IVE_IMAGE_TYPE_U8C3_PACKAGE: {
		pData1 = (CVI_U8 *)(uintptr_t)pstImage1->u64VirAddr[0];
		pData2 = (CVI_U8 *)(uintptr_t)pstImage2->u64VirAddr[0];
		printf("compare U8C3_PACKAGE A at %p, B at %p\n", pData1,
			   pData2);
		for (y = 0; y < u32Height;
			 y++, pData1 += u32Stride, pData2 += u32Stride) {
			n = memcmp(pData1, pData2, u32Width * 3);
			//dump_data_u8(pData1, u32Width, "dst");
			//dump_data_u8(pData2, u32Width, "ref");
			if (n != 0) {
				int idx = find_first_diff_u8(pData1, pData2,
								 u32Width * 3);
				CVI_ASSERT(idx >= 0);
				printf("Compare U8C3_PACKAGE failed, at y = %d, x = %d, n = %d\n",
					   y, idx, n);
				printf("  A = 0x%02X, B = 0x%02X\n",
					   pData1[idx], pData2[idx]);
				dump_data_u8(pData1, u32Width * 3, "A");
				dump_data_u8(pData2, u32Width * 3, "B");
				CVI_IVE_WriteImg(0, "sample_CSC_YUV4442RGB_1.yuv", pstImage1);
				CVI_IVE_WriteImg(0, "sample_CSC_YUV4442RGB_2.yuv", pstImage2);
				s32Succ = CVI_FAILURE;
				break;
			}
		}
	} break;
	case IVE_IMAGE_TYPE_U8C3_PLANAR: {
		for (int i = 0; i < 3; i++) {
			char YUVStr[4] = { 'Y', 'U', 'V' };
			pData1 = (CVI_U8 *)(uintptr_t)pstImage1->u64VirAddr[i];
			pData2 = (CVI_U8 *)(uintptr_t)pstImage2->u64VirAddr[i];
			printf("compare U8C3_PLANAR[%c] A at %p, B at %p\n",
				   YUVStr[i], pData1, pData2);
			for (y = 0; y < u32Height;
				 y++, pData1 += u32Stride, pData2 += u32Stride) {
				n = memcmp(pData1, pData2, u32Width);
				if (n != 0) {
					int idx = find_first_diff_u8(
						pData1, pData2, u32Width);
					CVI_ASSERT(idx >= 0);
					printf("Compare %c failed, at y = %d, x = %d, n = %d\n",
						   YUVStr[i], y, idx, n);
					printf("  A = 0x%02X, B = 0x%02X\n",
						   pData1[idx], pData2[idx]);
					dump_data_u8(pData1, u32Width, "A");
					dump_data_u8(pData2, u32Width, "B");
					s32Succ = CVI_FAILURE;
					break;
				}
			}
		}
	} break;
	case IVE_IMAGE_TYPE_S16C1:
	case IVE_IMAGE_TYPE_U16C1: {
		pData1 = (CVI_U8 *)(uintptr_t)pstImage1->u64VirAddr[0];
		pData2 = (CVI_U8 *)(uintptr_t)pstImage2->u64VirAddr[0];
		printf("compare A at %p, B at %p\n", pData1, pData2);
		for (y = 0; y < u32Height;
			 y++, pData1 += u32Stride, pData2 += u32Stride) {
			n = memcmp(pData1, pData2, u32Width * 2);
			if (n != 0) {
				pData1_U16 = (CVI_U16 *)pData1;
				pData2_U16 = (CVI_U16 *)pData2;
				int idx = find_first_diff_u16(
					pData1_U16, pData2_U16, u32Width);
				CVI_ASSERT(idx >= 0);
				printf("Compare failed, at y = %d, x = %d, n = %d\n",
					   y, idx, n);
				printf("  A = 0x%04X, B = 0x%04X\n",
					   pData1_U16[idx], pData2_U16[idx]);
				dump_data_u16(pData1_U16, u32Width, "A");
				dump_data_u16(pData2_U16, u32Width, "B");
				s32Succ = CVI_FAILURE;
				break;
			}
		}
	} break;
	case IVE_IMAGE_TYPE_S32C1:
	case IVE_IMAGE_TYPE_U32C1: {
		pData1 = (CVI_U8 *)(uintptr_t)pstImage1->u64VirAddr[0];
		pData2 = (CVI_U8 *)(uintptr_t)pstImage2->u64VirAddr[0];
		printf("compareU32C1 A at %p, B at %p\n", pData1, pData2);
		for (y = 0; y < u32Height;
			 y++, pData1 += u32Stride, pData2 += u32Stride) {
			n = memcmp(pData1, pData2, u32Width * 4);
			if (n != 0) {
				pData1_U32 = (CVI_U32 *)pData1;
				pData2_U32 = (CVI_U32 *)pData2;
				int idx = find_first_diff_u32(
					pData1_U32, pData2_U32, u32Width);
				CVI_ASSERT(idx >= 0);
				printf("Compare failed, at y = %d, x = %d, n = %d\n",
					   y, idx, n);
				printf("  A = 0x%08X, B = 0x%08X\n",
					   pData1_U32[idx], pData2_U32[idx]);
				dump_data_u32(pData1_U32, u32Width, "A");
				dump_data_u32(pData2_U32, u32Width, "B");
				s32Succ = CVI_FAILURE;
				break;
			}
		}
	} break;
	case IVE_IMAGE_TYPE_S64C1:
	case IVE_IMAGE_TYPE_U64C1: {
		pData1 = (CVI_U8 *)(uintptr_t)pstImage1->u64VirAddr[0];
		pData2 = (CVI_U8 *)(uintptr_t)pstImage2->u64VirAddr[0];
		printf("compareU64C1 A at %p, B at %p\n", pData1, pData2);
		for (y = 0; y < u32Height;
			 y++, pData1 += u32Stride, pData2 += u32Stride) {
			int m = memcmp(pData1, pData2, u32Width * 4);

			n = memcmp(pData1 + u32Width * 4, pData2 + u32Width * 4,
				   u32Width * 4);
			if (n != 0 || m != 0) {
				pData1_U64 = (CVI_U64 *)pData1;
				pData2_U64 = (CVI_U64 *)pData2;
				int idx = find_first_diff_u64(
					pData1_U64, pData2_U64, u32Width);
				CVI_ASSERT(idx >= 0);
				printf("Compare failed, at y = %d, x = %d, n = %d\n",
					   y, idx, n);
				printf("  A = 0x%016jX, B = 0x%016jX\n",
					   pData1_U64[idx], pData2_U64[idx]);
				dump_data_u64(pData1_U64, u32Width, "A");
				dump_data_u64(pData2_U64, u32Width, "B");
				s32Succ = CVI_FAILURE;
				break;
			}
		}
	} break;
	default: {
		printf("Unknown IMAGE_TYPE %d\n", enType);
		s32Succ = CVI_ERR_IVE_ILLEGAL_PARAM;
	} break;
	}
	return s32Succ;
}

CVI_S32 CVI_IVE_CompareSADImage(IVE_IMAGE_S *pstImage1, IVE_IMAGE_S *pstImage2,
				IVE_SAD_MODE_E mode, CVI_BOOL isDMAhalf)
{
	CVI_U32 u32Stride;
	CVI_S32 s32Succ;
	IVE_IMAGE_TYPE_E enType;
	CVI_U32 u32Width, u32SADWidth;
	CVI_U32 u32Height, u32SADHeight;
	CVI_U32 u32SADStride;
	CVI_U32 u32ByteSize;
	CVI_U8 *pData1;
	CVI_U8 *pData2;
	CVI_U16 y;
	int n;

	if (pstImage1->enType != pstImage2->enType) {
		printf("Not same IMAGE_TYPE, %d vs %d\n", pstImage1->enType,
			   pstImage2->enType);
		return CVI_FAILURE;
	}
	if (pstImage1->u32Width != pstImage2->u32Width) {
		printf("Not same u32Width, %d vs %d\n", pstImage1->u32Width,
			   pstImage2->u32Width);
		return CVI_FAILURE;
	}
	if (pstImage1->u32Height != pstImage2->u32Height) {
		printf("Not same u32Height, %d vs %d\n", pstImage1->u32Height,
			   pstImage2->u32Height);
		return CVI_FAILURE;
	}

	if (pstImage1->u32Stride[0] != pstImage2->u32Stride[0]) {
		printf("Not same u32Stride, %d vs %d\n", pstImage1->u32Stride[0],
			   pstImage2->u32Stride[0]);
		return CVI_FAILURE;
	}

	enType = pstImage1->enType;
	u32Width = pstImage1->u32Width;
	u32Height = pstImage1->u32Height;

	u32Stride = pstImage1->u32Stride[0];
	s32Succ = CVI_SUCCESS;
	switch (enType) {
	case IVE_IMAGE_TYPE_S16C1:
	case IVE_IMAGE_TYPE_U16C1:
		u32ByteSize = 2;
		break;
	case IVE_IMAGE_TYPE_U8C1:
	case IVE_IMAGE_TYPE_S8C1:
		u32ByteSize = 1;
		break;
	default:
		printf("not support output type %d, return\n", enType);
		return CVI_FAILURE;
	}
	switch (mode) {
	case IVE_SAD_MODE_MB_4X4:
		u32SADWidth = u32Width * u32ByteSize / 4;
		u32SADHeight = u32Height / 4;
		break;
	case IVE_SAD_MODE_MB_8X8:
		u32SADWidth = u32Width * u32ByteSize / 8;
		u32SADHeight = u32Height / 8;
		break;
	case IVE_SAD_MODE_MB_16X16:
		u32SADWidth = u32Width * u32ByteSize / 16;
		u32SADHeight = u32Height / 16;
		break;
	default:
		printf("not support mode type %d, return\n", mode);
		return CVI_FAILURE;
	}
	u32SADStride = (isDMAhalf) ? pstImage1->u32Stride[0] / 2 : pstImage1->u32Stride[0];
	printf("SAD Width %u Height %u Stride %u %u\n", u32SADWidth,
		   u32SADHeight, u32SADStride, u32Stride);

	switch (enType) {
	case IVE_IMAGE_TYPE_U8C1:
	case IVE_IMAGE_TYPE_S8C1:
	case IVE_IMAGE_TYPE_S16C1:
	case IVE_IMAGE_TYPE_U16C1: {
		pData1 = (CVI_U8 *)(uintptr_t)pstImage1->u64VirAddr[0];
		pData2 = (CVI_U8 *)(uintptr_t)pstImage2->u64VirAddr[0];
		printf("compareSAD A at %p, B at %p\n", pData1, pData2);
		for (y = 0; y < u32SADHeight;
			 y++, pData1 += u32SADStride, pData2 += u32Stride) { //dst=96 ref=384
			n = memcmp(pData1, pData2, u32SADWidth);
			if (n != 0) {
				int idx = find_first_diff_u8(pData1, pData2,
								 u32SADWidth);
				CVI_ASSERT(idx >= 0);
				printf("Compare failed, at y = %d, x = %d, n = %d\n",
					   y, idx, n);
				printf("  A = 0x%02X, B = 0x%02X\n",
					   pData1[idx], pData2[idx]);
				dump_data_u8(pData1, u32SADWidth, "A");
				dump_data_u8(pData2, u32SADWidth, "B");
				s32Succ = CVI_FAILURE;
				break;
			}
		}
	} break;
	default: {
		printf("Unknown IMAGE_TYPE %d\n", enType);
		s32Succ = CVI_ERR_IVE_ILLEGAL_PARAM;
	} break;
	}

	return s32Succ;
}

CVI_S32 CVI_IVE_VideoFrameInfo2Image(VIDEO_FRAME_INFO_S *pstVFISrc, IVE_IMAGE_S *pstIIDst)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 2;
	void* ptr[input_num];

	ptr[0] = pstIIDst;
	ptr[1] = pstVFISrc;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, NULL, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	CVI_U32 u32Channel = 1;
	VIDEO_FRAME_S *pstVFSrc = &pstVFISrc->stVFrame;
	IVE_IMAGE_TYPE_E img_type = IVE_IMAGE_TYPE_U8C1;

	switch (pstVFSrc->enPixelFormat) {
	case PIXEL_FORMAT_YUV_400: {
		img_type = IVE_IMAGE_TYPE_U8C1;
	} break;
	case PIXEL_FORMAT_NV21:
	case PIXEL_FORMAT_NV12: {
		img_type = IVE_IMAGE_TYPE_YUV420SP;
		u32Channel = 2;
	} break;
	case PIXEL_FORMAT_YUV_PLANAR_420: {
		img_type = IVE_IMAGE_TYPE_YUV420P;
		u32Channel = 3;
	} break;
	case PIXEL_FORMAT_YUV_PLANAR_422: {
		img_type = IVE_IMAGE_TYPE_YUV422P;
		u32Channel = 3;
	} break;
	case PIXEL_FORMAT_RGB_888:
	case PIXEL_FORMAT_BGR_888: {
		img_type = IVE_IMAGE_TYPE_U8C3_PACKAGE;
	} break;
	case PIXEL_FORMAT_RGB_888_PLANAR: {
		img_type = IVE_IMAGE_TYPE_U8C3_PLANAR;
		u32Channel = 3;
	} break;
	case PIXEL_FORMAT_INT16_C1: {
		img_type = IVE_IMAGE_TYPE_S16C1;
	} break;
	case PIXEL_FORMAT_UINT16_C1: {
		img_type = IVE_IMAGE_TYPE_U16C1;
	} break;
	default: {
		printf("Unsupported conversion type: %u.\n", pstVFSrc->enPixelFormat);
		return CVI_FAILURE;
	}
	}

	for (CVI_U32 i = 0; i < u32Channel; i++) {
		pstIIDst->u64VirAddr[i] = (CVI_U64)(uintptr_t)pstVFSrc->pu8VirAddr[i];
		pstIIDst->u64PhyAddr[i] = pstVFSrc->u64PhyAddr[i];
		pstIIDst->u32Stride[i] = pstVFSrc->u32Stride[i];
	}
	pstIIDst->enType = img_type;
	pstIIDst->u32Width = pstVFSrc->u32Width;
	pstIIDst->u32Height = pstVFSrc->u32Height;
	return s32Ret;
}

CVI_S32 CVI_IVE_Image2VideoFrameInfo(IVE_IMAGE_S *pstIISrc, VIDEO_FRAME_INFO_S *pstVFIDst)
		//, CVI_BOOL invertPackage)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 2;
	void* ptr[input_num];

	ptr[0] = pstIISrc;
	ptr[1] = pstVFIDst;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstIISrc, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	CVI_U32 u32Channel = 1;
	VIDEO_FRAME_S *pstVFSrc = &pstVFIDst->stVFrame;
	PIXEL_FORMAT_E img_type = PIXEL_FORMAT_YUV_400;

	pstVFIDst->u32PoolId = -1;
	switch (pstIISrc->enType) {
	case IVE_IMAGE_TYPE_U8C1: {
		pstVFSrc->u32Stride[0] = WidthAlign(pstIISrc->u32Width, IVE_DEFAULT_ALIGN);
		img_type = PIXEL_FORMAT_YUV_400;
	} break;
	case IVE_IMAGE_TYPE_YUV420SP: {
		pstVFSrc->u32Stride[0] = WidthAlign(pstIISrc->u32Width, IVE_DEFAULT_ALIGN);
		pstVFSrc->u32Stride[1] = WidthAlign(pstIISrc->u32Width, IVE_DEFAULT_ALIGN);
		img_type = PIXEL_FORMAT_NV21;
		u32Channel = 2;
	} break;
	case IVE_IMAGE_TYPE_YUV420P: {
		pstVFSrc->u32Stride[0] = WidthAlign(pstIISrc->u32Width, IVE_DEFAULT_ALIGN);
		pstVFSrc->u32Stride[1] = WidthAlign(pstIISrc->u32Width >> 1, IVE_DEFAULT_ALIGN);
		pstVFSrc->u32Stride[2] = pstVFSrc->u32Stride[1];
		img_type = PIXEL_FORMAT_YUV_PLANAR_420;
		u32Channel = 3;
	} break;
	case IVE_IMAGE_TYPE_YUV422P: {
		pstVFSrc->u32Stride[0] = WidthAlign(pstIISrc->u32Width, IVE_DEFAULT_ALIGN);
		pstVFSrc->u32Stride[1] = WidthAlign(pstIISrc->u32Width >> 1, IVE_DEFAULT_ALIGN);
		pstVFSrc->u32Stride[2] = pstVFSrc->u32Stride[1];
		img_type = PIXEL_FORMAT_YUV_PLANAR_422;
		u32Channel = 3;
	} break;
	case IVE_IMAGE_TYPE_U8C3_PACKAGE: {
		pstVFSrc->u32Stride[0] = WidthAlign(pstIISrc->u32Width * 3, IVE_DEFAULT_ALIGN);
		img_type = PIXEL_FORMAT_RGB_888;
	} break;
	case IVE_IMAGE_TYPE_U8C3_PLANAR: {
		pstVFSrc->u32Stride[0] = WidthAlign(pstIISrc->u32Width, IVE_DEFAULT_ALIGN);
		pstVFSrc->u32Stride[1] = pstVFSrc->u32Stride[0];
		pstVFSrc->u32Stride[2] = pstVFSrc->u32Stride[0];
		img_type = PIXEL_FORMAT_RGB_888_PLANAR;
		u32Channel = 3;
	} break;
	case IVE_IMAGE_TYPE_S16C1: {
		pstVFSrc->u32Stride[0] =
			WidthAlign(pstIISrc->u32Width * sizeof(int16_t), IVE_DEFAULT_ALIGN);
		img_type = PIXEL_FORMAT_INT16_C1;
	} break;
	case IVE_IMAGE_TYPE_U16C1: {
		pstVFSrc->u32Stride[0] =
			WidthAlign(pstIISrc->u32Width * sizeof(uint16_t), IVE_DEFAULT_ALIGN);
		img_type = PIXEL_FORMAT_UINT16_C1;
	} break;
	default: {
		printf("Unsupported conversion type: %u.\n", pstIISrc->enType);
		return CVI_FAILURE;
	}
	}

	for (CVI_U32 i = 0; i < u32Channel; i++) {
		pstVFSrc->pu8VirAddr[i] = (CVI_U8 *)(uintptr_t)pstIISrc->u64VirAddr[i];
		pstVFSrc->u64PhyAddr[i] = pstIISrc->u64PhyAddr[i];
		pstVFSrc->u32Length[i] = pstIISrc->u32Height * pstIISrc->u32Stride[i];
	}
	for (CVI_U32 i = u32Channel; i < 3; i++) {
		pstVFSrc->pu8VirAddr[i] = 0;
		pstVFSrc->u64PhyAddr[i] = 0;
		pstVFSrc->u32Stride[i] = 0;
		pstVFSrc->u32Length[i] = 0;
	}
	pstVFSrc->enPixelFormat = img_type;
	pstVFSrc->u32Width = pstIISrc->u32Width;
	pstVFSrc->u32Height = pstIISrc->u32Height;

	return s32Ret;
}

CVI_S32 CVI_IVE_ReadData(IVE_HANDLE pIveHandle, IVE_DATA_S *pstData,
			 const char *filename, CVI_U16 u32Width,
			 CVI_U16 u32Height)
{
	CVI_U16 u32Stride = WidthAlign(u32Width, IVE_DEFAULT_ALIGN);
	int uSize = u32Stride * u32Height;
	FILE *fp;
	char pBuffer[u32Width * u32Height];
	char *ptr = pBuffer;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 3;
	void* ptr_input[input_num];

	ptr_input[0] = pIveHandle;
	ptr_input[1] = pstData;
	ptr_input[2] = (void *)filename;
	s32Ret = CHECK_FUNC_ALL(ptr_input, input_num, NULL, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Can't open %s\n", filename);
		return ERR_IVE_OPEN_FILE;
	}

	s32Ret = CVI_IVE_CreateDataInfo(pIveHandle, pstData, u32Width,
					 u32Height);
	if (s32Ret) {
		printf("Ion alloc failed\n");
		return s32Ret;
	}

	int readCnt =
		fread(pBuffer, 1, uSize, fp);

	if (readCnt == 0) {
		printf("Image %s read failed.\n", filename);
		return ERR_IVE_READ_FILE;
	}
	for (size_t j = 0; j < (size_t)u32Height; j++) {
		memcpy((char *)(uintptr_t)(pstData->u64VirAddr +
				(j * pstData->u32Stride)), ptr, u32Width);
		ptr += u32Width;
	}
	fclose(fp);
	return s32Ret;
}

CVI_S32 CVI_IVE_ReadDataArray(IVE_HANDLE pIveHandle, IVE_DATA_S *pstData,
				  char *pBuffer, CVI_U16 u32Width, CVI_U16 u32Height)
{
	char *ptr = NULL;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 3;
	void* ptr_input[input_num];

	ptr_input[0] = pIveHandle;
	ptr_input[1] = pstData;
	ptr_input[2] = pBuffer;
	s32Ret = CHECK_FUNC_ALL(ptr_input, input_num, NULL, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_IVE_CreateDataInfo(pIveHandle, pstData, u32Width,
					 u32Height);
	if (s32Ret) {
		printf("Ion alloc failed\n");
		return s32Ret;
	}
	printf("u32Stride = %d\n", pstData->u32Stride);
	ptr = pBuffer;
	for (size_t j = 0; j < (size_t)u32Height; j++) {
		memcpy((char *)(uintptr_t)(pstData->u64VirAddr +
				(j * pstData->u32Stride)), ptr, u32Width);
		ptr += u32Width;
	}
	return s32Ret;
}

CVI_S32 CVI_IVE_ReadMem(IVE_HANDLE pIveHandle, IVE_MEM_INFO_S *pstMem,
			const char *filename, CVI_U32 uSize)
{
	FILE *fp;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 3;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstMem;
	ptr[2] = (void *)filename;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, NULL, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Can't open %s\n", filename);
		return ERR_IVE_OPEN_FILE;
	}

	s32Ret = CVI_IVE_CreateMemInfo(pIveHandle, pstMem, uSize);
	if (s32Ret) {
		printf("Ion alloc failed\n");
		return s32Ret;
	}

	int readCnt =
		fread((char *)(uintptr_t)pstMem->u64VirAddr, 1, uSize, fp);

	if (readCnt == 0) {
		printf("Image %s read failed.\n", filename);
		return ERR_IVE_READ_FILE;
	}
	fclose(fp);
	return s32Ret;
}

CVI_S32 CVI_IVE_ReadMemArray(IVE_HANDLE pIveHandle, IVE_MEM_INFO_S *pstMem,
				 char *pBuffer, CVI_U32 uSize)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 3;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstMem;
	ptr[2] = pBuffer;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, NULL, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	s32Ret = CVI_IVE_CreateMemInfo(pIveHandle, pstMem, uSize);
	if (s32Ret) {
		printf("Ion alloc failed\n");
		return s32Ret;
	}
	memcpy((char *)(uintptr_t)pstMem->u64VirAddr, (void *)pBuffer, uSize);
	return s32Ret;
}

CVI_S32 CVI_IVE_ReadImageArray(IVE_HANDLE pIveHandle, IVE_IMAGE_S *pstImg,
				   char *pBuffer, IVE_IMAGE_TYPE_E enType,
				   CVI_U16 u32Width, CVI_U16 u32Height)
{
	char *ptr = NULL;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 3;
	void* ptr_input[input_num];

	ptr_input[0] = pstImg;
	ptr_input[1] = pBuffer;
	ptr_input[2] = pIveHandle;
	s32Ret = CHECK_FUNC_ALL(ptr_input, input_num, NULL, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_INPUT_FORMAT(enType);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memset(pstImg, 0, sizeof(IVE_IMAGE_S));

	CVI_IVE_CreateImage(pIveHandle, pstImg, enType, u32Width, u32Height);

	if (enType == IVE_IMAGE_TYPE_U8C3_PLANAR) {
		ptr = pBuffer;
		for (size_t j = 0; j < (size_t)u32Height; j++) {
			memcpy((char *)(uintptr_t)(pstImg->u64VirAddr[0] +
						   (j * pstImg->u32Stride[0])),
				   ptr, u32Width);
			ptr += u32Width;
		}
		for (size_t j = 0; j < (size_t)pstImg->u32Height; j++) {
			memcpy((char *)(uintptr_t)(pstImg->u64VirAddr[1] +
						   (j * pstImg->u32Stride[1])),
				   ptr, u32Width);
			ptr += u32Width;
		}
		for (size_t j = 0; j < (size_t)pstImg->u32Height; j++) {
			memcpy((char *)(uintptr_t)(pstImg->u64VirAddr[2] +
						   (j * pstImg->u32Stride[2])),
				   ptr, u32Width);
			ptr += u32Width;
		}
	} else if (enType == IVE_IMAGE_TYPE_U8C3_PACKAGE) {
		// yyy... uuu... vvv... to yyy... uuu... vvv...
		ptr = pBuffer;
		for (size_t i = 0; i < (size_t)u32Height; i++) {
			uint32_t stb_stride = i * u32Width * 3;
			uint32_t image_stride = (i * pstImg->u32Stride[0]);

			for (size_t j = 0; j < (size_t)u32Width; j++) {
				uint32_t buf_idx = stb_stride + (j * 3);
				uint32_t img_idx = image_stride + (j * 3);
				((char *)(uintptr_t)
					 pstImg->u64VirAddr[0])[img_idx] =
					ptr[buf_idx];
				((char *)(uintptr_t)
					 pstImg->u64VirAddr[0])[img_idx + 1] =
					ptr[buf_idx + 1];
				((char *)(uintptr_t)
					 pstImg->u64VirAddr[0])[img_idx + 2] =
					ptr[buf_idx + 2];
			}
		}
	} else if (enType == IVE_IMAGE_TYPE_YUV420SP) {
		ptr = pBuffer;
		for (size_t j = 0; j < (size_t)u32Height; j++) {
			memcpy((char *)(uintptr_t)(pstImg->u64VirAddr[0] +
						   (j * pstImg->u32Stride[0])),
				   ptr, u32Width);
			ptr += u32Width;
		}
		for (size_t j = 0; j < (size_t)pstImg->u32Height / 2; j++) {
			memcpy((char *)(uintptr_t)(pstImg->u64VirAddr[1] +
						   (j * pstImg->u32Stride[1])),
				   ptr, u32Width);
			ptr += u32Width;
		}
	} else if (enType == IVE_IMAGE_TYPE_YUV422SP) {
		ptr = pBuffer;
		for (size_t j = 0; j < (size_t)u32Height; j++) {
			memcpy((char *)(uintptr_t)(pstImg->u64VirAddr[0] +
						   (j * pstImg->u32Stride[0])),
				   ptr, u32Width);
			ptr += u32Width;
		}
		for (size_t j = 0; j < (size_t)pstImg->u32Height; j++) {
			memcpy((char *)(uintptr_t)(pstImg->u64VirAddr[1] +
						   (j * pstImg->u32Stride[1])),
				   ptr, u32Width);
			ptr += u32Width;
		}
	} else if (enType == IVE_IMAGE_TYPE_U16C1 ||
		   enType == IVE_IMAGE_TYPE_S16C1) {
		ptr = pBuffer;
		for (size_t j = 0; j < (size_t)u32Height; j++) {
			memcpy((char *)(uintptr_t)(pstImg->u64VirAddr[0] +
						   (j * pstImg->u32Stride[0])),
				   ptr, u32Width * (sizeof(uint16_t)));
			ptr += u32Width * (sizeof(uint16_t));
		}
	} else {
		ptr = pBuffer;
		for (size_t j = 0; j < (size_t)u32Height; j++) {
			memcpy((char *)(uintptr_t)(pstImg->u64VirAddr[0] +
						   (j * pstImg->u32Stride[0])),
				   ptr, u32Width);
			ptr += u32Width;
		}
	}

	return s32Ret;
}

CVI_S32 CVI_IVE_ReadRawImage(IVE_HANDLE pIveHandle, IVE_IMAGE_S *pstImg,
			  const char *filename, IVE_IMAGE_TYPE_E enType,
			  CVI_U16 u32Width, CVI_U16 u32Height)
{
	float desiredNChannels = -1;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 3;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstImg;
	ptr[2] = (void *)filename;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, NULL, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_INPUT_FORMAT(enType);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	switch (enType) {
	case IVE_IMAGE_TYPE_U8C1:
	case IVE_IMAGE_TYPE_S8C1:
		desiredNChannels = 1;
		break;
	case IVE_IMAGE_TYPE_YUV420SP:
		desiredNChannels = 1.5;
		break;
	case IVE_IMAGE_TYPE_U16C1:
	case IVE_IMAGE_TYPE_S16C1:
	case IVE_IMAGE_TYPE_YUV422SP:
		desiredNChannels = 2;
		break;
	case IVE_IMAGE_TYPE_U8C3_PLANAR:
	case IVE_IMAGE_TYPE_U8C3_PACKAGE:
		desiredNChannels = 3;
		break;
	default:
		printf("Not support channel %s.\n", cviIveImgEnTypeStr[enType]);
		return CVI_ERR_IVE_ILLEGAL_PARAM;
	}

	if (desiredNChannels > 0) {
		int buf_size = (int)((float)u32Width * (float)u32Height *
					 (float)desiredNChannels);
		char buffer[buf_size];
		FILE *fp;

		fp = fopen(filename, "r");
		int readCnt = fread(buffer, 1, buf_size, fp);

		if (readCnt == 0) {
			printf("Image %s read failed.\n", filename);
			return ERR_IVE_READ_FILE;
		}
		fclose(fp);

		CVI_IVE_ReadImageArray(pIveHandle, pstImg, buffer, enType,
					   u32Width, u32Height);
		return CVI_SUCCESS;
	}
	return CVI_FAILURE;
}

IVE_IMAGE_S CVI_IVE_ReadImage(IVE_HANDLE pIveHandle, const char *filename, IVE_IMAGE_TYPE_E enType)
{
	int desiredNChannels = -1;
	CVI_BOOL invertPackage = false;

	switch (enType) {
	case IVE_IMAGE_TYPE_U8C1:
		desiredNChannels = STBI_grey;
		break;
	case IVE_IMAGE_TYPE_U8C3_PLANAR:
		desiredNChannels = STBI_rgb;
		break;
	case IVE_IMAGE_TYPE_U8C3_PACKAGE:
		desiredNChannels = STBI_rgb;
		break;
	default:
		printf("Not support channel %s.\n", cviIveImgEnTypeStr[enType]);
		break;
	}
	IVE_IMAGE_S img;

	memset(&img, 0, sizeof(IVE_IMAGE_S));
	if (desiredNChannels >= 0) {
		int width, height, nChannels;
		stbi_uc *stbi_data = stbi_load(filename, &width, &height, &nChannels, desiredNChannels);

		if (stbi_data == NULL) {
			printf("Image %s read failed.\n", filename);
			return img;
		}
		CVI_IVE_CreateImage(pIveHandle, &img, enType, width, height);
		//printf("desiredNChannels, width, height: %d %d %d\n", desiredNChannels, width, height);
		if (enType == IVE_IMAGE_TYPE_U8C3_PLANAR) {
			for (size_t i = 0; i < (size_t)height; i++) {
				for (size_t j = 0; j < (size_t)width; j++) {
					size_t stb_idx = (i * width + j) * 3;
					size_t img_idx = (i * img.u32Stride[0] + j);
					((CVI_U8 *)(uintptr_t)img.u64VirAddr[0])[img_idx] = stbi_data[stb_idx];
					((CVI_U8 *)(uintptr_t)img.u64VirAddr[1])[img_idx] = stbi_data[stb_idx + 1];
					((CVI_U8 *)(uintptr_t)img.u64VirAddr[2])[img_idx] = stbi_data[stb_idx + 2];
				}
			}
		} else {
			if (invertPackage && enType == IVE_IMAGE_TYPE_U8C3_PACKAGE) {
				for (size_t i = 0; i < (size_t)height; i++) {
					uint32_t stb_stride = i * width * 3;
					uint32_t image_stride = (i * img.u32Stride[0]);

					for (size_t j = 0; j < (size_t)width; j++) {
						uint32_t stb_idx = stb_stride + (j * 3);
						uint32_t img_idx = image_stride + (j * 3);

						((CVI_U8 *)(uintptr_t)img.u64VirAddr[0])[img_idx] =
						stbi_data[stb_idx + 2];
						((CVI_U8 *)(uintptr_t)img.u64VirAddr[0])[img_idx + 1] =
						stbi_data[stb_idx + 1];
						((CVI_U8 *)(uintptr_t)img.u64VirAddr[0])[img_idx + 2] =
						stbi_data[stb_idx];
					}
				}
			} else {
				stbi_uc *ptr = stbi_data;

				for (size_t j = 0; j < (size_t)height; j++) {
					memcpy((void *)(uintptr_t)img.u64VirAddr[0] + (j * img.u32Stride[0]),
							ptr, width * desiredNChannels);
					ptr += width * desiredNChannels;
				}
			}
		}
		stbi_image_free(stbi_data);
	}
	return img;
}

CVI_S32 CVI_IVE_WriteImage(IVE_HANDLE pIveHandle, const char *filename, IVE_IMAGE_S *pstImg)
{
	UNUSED(pIveHandle);
	int desiredNChannels = -1;
	int stride = 1;
	uint8_t *arr = NULL;
	bool remove_buffer = false;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 2;
	void* ptr[input_num];

	ptr[0] = (void *)filename;
	ptr[1] = pstImg;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstImg, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	switch (pstImg->enType) {
	case IVE_IMAGE_TYPE_U8C1:
		desiredNChannels = STBI_grey;
		arr = (CVI_U8 *)(uintptr_t)pstImg->u64VirAddr[0];
		break;
	case IVE_IMAGE_TYPE_U8C3_PLANAR: {
		desiredNChannels = STBI_rgb;
		stride = 1;
		arr = (uint8_t *)calloc((size_t)pstImg->u32Stride[0] * pstImg->u32Height * desiredNChannels,
								(size_t)sizeof(uint8_t));
		size_t image_total = pstImg->u32Stride[0] * pstImg->u32Height;

		for (size_t i = 0; i < image_total; i++) {
			size_t stb_idx = i * 3;

			arr[stb_idx] = ((CVI_U8 *)(uintptr_t)pstImg->u64VirAddr[0])[i];
			arr[stb_idx + 1] = ((CVI_U8 *)(uintptr_t)pstImg->u64VirAddr[1])[i];
			arr[stb_idx + 2] = ((CVI_U8 *)(uintptr_t)pstImg->u64VirAddr[2])[i];
		}
		stride = 3;
		remove_buffer = true;
	} break;
	case IVE_IMAGE_TYPE_U8C3_PACKAGE:
		desiredNChannels = STBI_rgb;
		arr = (CVI_U8 *)(uintptr_t)pstImg->u64VirAddr[0];
		stride = 1;
		break;
	default:
		printf("Not supported channel %s.", cviIveImgEnTypeStr[pstImg->enType]);
		return CVI_FAILURE;
	}
	stbi_write_png(filename, pstImg->u32Width, pstImg->u32Height, desiredNChannels, arr,
					pstImg->u32Stride[0] * stride);
	if (remove_buffer) {
		free(arr);
	}
	return s32Ret;
}

CVI_S32 CVI_IVE_ResetImage(IVE_IMAGE_S *pstImage, CVI_U8 val)
{
	CVI_U32 u32Stride;
	IVE_IMAGE_TYPE_E enType;
	CVI_U32 u32Width;
	CVI_U32 u32Height;
	CVI_U8 *pData;
	CVI_U16 y;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 1;
	void* ptr[input_num];

	ptr[0] = pstImage;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstImage, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	enType = pstImage->enType;
	u32Width = pstImage->u32Width;
	u32Height = pstImage->u32Height;
	u32Stride = WidthAlign(u32Width, IVE_DEFAULT_ALIGN);

	switch (enType) {
	case IVE_IMAGE_TYPE_U8C1:
	case IVE_IMAGE_TYPE_S8C1: {
		pData = (CVI_U8 *)(uintptr_t)pstImage->u64VirAddr[0];
		for (y = 0; y < u32Height; y++, pData += u32Stride) {
			memset(pData, val, u32Width);
		}
	} break;
	case IVE_IMAGE_TYPE_YUV420SP: {
		pData = (CVI_U8 *)(uintptr_t)pstImage->u64VirAddr[0];
		for (y = 0; y < u32Height; y++, pData += u32Stride) {
			memset(pData, val, u32Width);
		}
		pData = (CVI_U8 *)(uintptr_t)pstImage->u64VirAddr[1];
		for (y = 0; y < u32Height / 2; y++, pData += u32Stride) {
			memset(pData, val, u32Width);
		}
	} break;
	case IVE_IMAGE_TYPE_YUV422SP: {
		pData = (CVI_U8 *)(uintptr_t)pstImage->u64VirAddr[0];
		for (y = 0; y < u32Height; y++, pData += u32Stride) {
			memset(pData, val, u32Width);
		}
		pData = (CVI_U8 *)(uintptr_t)pstImage->u64VirAddr[1];
		for (y = 0; y < u32Height; y++, pData += u32Stride) {
			memset(pData, val, u32Width);
		}
	} break;
	case IVE_IMAGE_TYPE_U8C3_PACKAGE: {
		pData = (CVI_U8 *)(uintptr_t)pstImage->u64VirAddr[0];
		for (y = 0; y < pstImage->u32Height;
			 y++, pData += pstImage->u32Stride[0] * 3) {
			memset(pData, val, pstImage->u32Width * 3);
		}
	} break;
	case IVE_IMAGE_TYPE_U8C3_PLANAR: {
		for (int i = 0; i < 3; i++) {
			pData = (CVI_U8 *)(uintptr_t)pstImage->u64VirAddr[i];
			for (y = 0; y < pstImage->u32Height;
				 y++, pData += pstImage->u32Stride[i]) {
				memset(pData, val, pstImage->u32Width);
			}
		}

	} break;
	case IVE_IMAGE_TYPE_YUV420P:
	case IVE_IMAGE_TYPE_YUV422P:
	case IVE_IMAGE_TYPE_S8C2_PACKAGE:
	case IVE_IMAGE_TYPE_S8C2_PLANAR:
	case IVE_IMAGE_TYPE_S16C1:
	case IVE_IMAGE_TYPE_U16C1:
	case IVE_IMAGE_TYPE_S32C1:
	case IVE_IMAGE_TYPE_U32C1:
	case IVE_IMAGE_TYPE_S64C1:
	case IVE_IMAGE_TYPE_U64C1: {
		printf("Unsupported IMAGE_TYPE %d\n", enType);
		s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
	} break;
	default: {
		printf("Unknown IMAGE_TYPE %d\n", enType);
		s32Ret = CVI_ERR_IVE_ILLEGAL_PARAM;
	} break;
	}

	return s32Ret;
}

CVI_S32 CVI_IVE_CreateMemInfo(IVE_HANDLE pIveHandle, IVE_MEM_INFO_S *pstMemInfo,
				  CVI_U32 u32ByteSize)
{
	UNUSED(pIveHandle);
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 1;
	void* ptr[input_num];

	ptr[0] = pstMemInfo;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, NULL, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CVI_SYS_IonAlloc((CVI_U64 *)&pstMemInfo->u64PhyAddr,
					  (CVI_VOID **)&pstMemInfo->u64VirAddr,
					  "ive_mesh", u32ByteSize);

	memset((char *)(uintptr_t)pstMemInfo->u64VirAddr, 0, u32ByteSize);
	pstMemInfo->u32Size = u32ByteSize;
	return s32Ret;
}

CVI_S32 CVI_IVE_CreateDataInfo(IVE_HANDLE pIveHandle, IVE_DATA_S *pstDataInfo,
				   CVI_U16 u32Width, CVI_U16 u32Height)
{
	UNUSED(pIveHandle);
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 1;
	void* ptr[input_num];

	ptr[0] = pstDataInfo;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, NULL, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	pstDataInfo->u32Stride = WidthAlign(u32Width, IVE_DEFAULT_ALIGN);
	s32Ret = CVI_SYS_IonAlloc((CVI_U64 *)&pstDataInfo->u64PhyAddr,
					  (CVI_VOID **)&pstDataInfo->u64VirAddr,
					  "ive_mesh",
					  pstDataInfo->u32Stride * u32Height);

	memset((char *)(uintptr_t)pstDataInfo->u64VirAddr, 0,
		   pstDataInfo->u32Stride * u32Height);
	pstDataInfo->u32Width = u32Width;
	pstDataInfo->u32Height = u32Height;
	return s32Ret;
}

CVI_S32 _CVI_IVE_CreateImage(IVE_HANDLE pIveHandle, IVE_IMAGE_S *pstImg,
				IVE_IMAGE_TYPE_E enType, uint16_t u32Width,
				uint16_t u32Height, CVI_BOOL bCached)
{
	UNUSED(pIveHandle);
	CVI_U32 u32Len = 0, u32Channel = 1;
	CVI_U32 u32Coffset[3] = { 0 };
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 1;
	void* ptr[input_num];

	ptr[0] = pstImg;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, NULL, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	s32Ret = CHECK_INPUT_FORMAT(enType);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (u32Width == 0 || u32Height == 0) {
		printf("Image width or height cannot be 0.\n");
		pstImg->enType = enType;
		pstImg->u32Width = 0;
		pstImg->u32Height = 0;
		pstImg->u32Reserved = 0;
		for (size_t i = 0; i < 3; i++) {
			pstImg->u64VirAddr[i] = 0;
			pstImg->u64PhyAddr[i] = 0;
			pstImg->u32Stride[i] = 0;
		}
		return CVI_FAILURE;
	}

	switch (enType) {
	case IVE_IMAGE_TYPE_S8C1: {
		pstImg->u32Stride[0] = WidthAlign(u32Width, IVE_DEFAULT_ALIGN);
		u32Len = pstImg->u32Stride[0] * u32Height;
	} break;
	case IVE_IMAGE_TYPE_U8C1: {
		pstImg->u32Stride[0] = WidthAlign(u32Width, IVE_DEFAULT_ALIGN);
		u32Len = pstImg->u32Stride[0] * u32Height;
	} break;
	case IVE_IMAGE_TYPE_YUV420SP: {
		pstImg->u32Stride[0] = WidthAlign(u32Width, IVE_DEFAULT_ALIGN);
		u32Len = pstImg->u32Stride[0] * u32Height;
		u32Coffset[1] = u32Len;
		pstImg->u32Stride[1] = WidthAlign(u32Width, IVE_DEFAULT_ALIGN);
		u32Len += pstImg->u32Stride[0] * u32Height >> 1;
		u32Channel = 2;
	} break;
	case IVE_IMAGE_TYPE_YUV420P: {
		pstImg->u32Stride[0] = WidthAlign(u32Width, IVE_DEFAULT_ALIGN);
		u32Len = pstImg->u32Stride[0] * u32Height;
		u32Coffset[1] = u32Len;
		pstImg->u32Stride[1] =
			WidthAlign(u32Width >> 1, IVE_DEFAULT_ALIGN);
		pstImg->u32Stride[2] = pstImg->u32Stride[1];
		u32Len += pstImg->u32Stride[1] * u32Height >> 1;
		u32Coffset[2] = u32Len;
		u32Len += pstImg->u32Stride[1] * u32Height >> 1;
		u32Channel = 3;
	} break;
	case IVE_IMAGE_TYPE_YUV422SP: {
		pstImg->u32Stride[0] = WidthAlign(u32Width, IVE_DEFAULT_ALIGN);
		pstImg->u32Stride[1] = pstImg->u32Stride[0];
		u32Len = pstImg->u32Stride[0] * u32Height;
		u32Coffset[1] = u32Len;
		u32Len += pstImg->u32Stride[0] * u32Height;
		u32Channel = 2;
	} break;
	case IVE_IMAGE_TYPE_YUV422P: {
		pstImg->u32Stride[0] = WidthAlign(u32Width, IVE_DEFAULT_ALIGN);
		pstImg->u32Stride[1] =
			WidthAlign(u32Width >> 1, IVE_DEFAULT_ALIGN);
		pstImg->u32Stride[2] = pstImg->u32Stride[1];
		u32Len = pstImg->u32Stride[0] * u32Height;
		u32Coffset[1] = u32Len;
		u32Len += pstImg->u32Stride[1] * u32Height;
		u32Coffset[2] = u32Len;
		u32Len += pstImg->u32Stride[1] * u32Height;
		u32Channel = 3;
	} break;
	case IVE_IMAGE_TYPE_U8C3_PACKAGE: {
		pstImg->u32Stride[0] = WidthAlign(u32Width * 3, IVE_DEFAULT_ALIGN);
		u32Len = pstImg->u32Stride[0] * u32Height;
	} break;
	case IVE_IMAGE_TYPE_U8C3_PLANAR: {
		pstImg->u32Stride[0] = WidthAlign(u32Width, IVE_DEFAULT_ALIGN);
		pstImg->u32Stride[1] = pstImg->u32Stride[0];
		pstImg->u32Stride[2] = pstImg->u32Stride[0];
		u32Len = pstImg->u32Stride[0] * u32Height;
		u32Coffset[1] = u32Len;
		u32Len += pstImg->u32Stride[0] * u32Height;
		u32Coffset[2] = u32Len;
		u32Len += pstImg->u32Stride[0] * u32Height;
		u32Channel = 3;
	} break;
	case IVE_IMAGE_TYPE_BF16C1: {
		pstImg->u32Stride[0] =
			WidthAlign(u32Width, IVE_DEFAULT_ALIGN) * sizeof(int16_t);
		u32Len = pstImg->u32Stride[0] * u32Height;
	} break;
	case IVE_IMAGE_TYPE_U16C1: {
		pstImg->u32Stride[0] =
			WidthAlign(u32Width, IVE_DEFAULT_ALIGN) * sizeof(uint16_t);
		u32Len = pstImg->u32Stride[0] * u32Height;
	} break;
	case IVE_IMAGE_TYPE_S16C1: {
		pstImg->u32Stride[0] =
			WidthAlign(u32Width, IVE_DEFAULT_ALIGN) * sizeof(uint16_t);
		u32Len = pstImg->u32Stride[0] * u32Height;
	} break;
	case IVE_IMAGE_TYPE_U32C1: {
		pstImg->u32Stride[0] =
			WidthAlign(u32Width, IVE_DEFAULT_ALIGN) * sizeof(uint32_t);
		u32Len = pstImg->u32Stride[0] * u32Height;
	} break;
	case IVE_IMAGE_TYPE_FP32C1: {
		pstImg->u32Stride[0] =
			WidthAlign(u32Width, IVE_DEFAULT_ALIGN) * sizeof(float);
		u32Len = pstImg->u32Stride[0] * u32Height;
	} break;
	default:
		printf("Not supported enType %s.\n",
			   cviIveImgEnTypeStr[enType]);
		return CVI_ERR_IVE_ILLEGAL_PARAM;
		break;
	}
	pstImg->enType = enType;
	if (pstImg->u32Stride[0] == 0 || u32Len == 0) {
		printf("[DEV] Stride not set.\n");
		return CVI_ERR_IVE_ILLEGAL_PARAM;
	}
	pstImg->u32Width = u32Width;
	pstImg->u32Height = u32Height;
	if (bCached) {
		s32Ret = CVI_SYS_IonAlloc_Cached(&pstImg->u64PhyAddr[0],
				  (CVI_VOID **)&pstImg->u64VirAddr[0],
				  "ive_mesh", u32Len);
	} else {
		s32Ret = CVI_SYS_IonAlloc(&pstImg->u64PhyAddr[0],
				  (CVI_VOID **)&pstImg->u64VirAddr[0],
				  "ive_mesh", u32Len);
	}
	memset((char *)(uintptr_t)pstImg->u64VirAddr[0], 0, u32Len);
	if (s32Ret) {
		printf("CVI_SYS_IonAlloc failed with %#x\n", s32Ret);
		return CVI_FAILURE;
	}

	for (size_t i = 1; i < u32Channel; i++) {
		pstImg->u64VirAddr[i] = pstImg->u64VirAddr[0] + u32Coffset[i];
		pstImg->u64PhyAddr[i] = pstImg->u64PhyAddr[0] + u32Coffset[i];
	}

	for (size_t i = u32Channel; i < 3; i++) {
		pstImg->u64VirAddr[i] = 0;
		pstImg->u64PhyAddr[i] = -1;
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_IVE_CreateImage(IVE_HANDLE pIveHandle, IVE_IMAGE_S *pstImg,
				IVE_IMAGE_TYPE_E enType, uint16_t u32Width,
				uint16_t u32Height)
{
	return _CVI_IVE_CreateImage(pIveHandle, pstImg, enType, u32Width, u32Height, false);
}

CVI_S32 CVI_IVE_CreateImage_Cached(IVE_HANDLE pIveHandle, IVE_IMAGE_S *pstImg,
				IVE_IMAGE_TYPE_E enType, uint16_t u32Width,
				uint16_t u32Height)
{
	return _CVI_IVE_CreateImage(pIveHandle, pstImg, enType, u32Width, u32Height, true);
}

CVI_S32 _CVI_IVE_BufFlush_Request(IVE_HANDLE pIveHandle, IVE_IMAGE_S *pstImg, CVI_BOOL bFlush)
{
	UNUSED(pIveHandle);
	CVI_U32 u32Len = 0;
	CVI_U32 u32Height;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 1;
	void* ptr[input_num];

	ptr[0] = pstImg;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstImg, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	u32Height = pstImg->u32Height;
	switch (pstImg->enType) {
	case IVE_IMAGE_TYPE_S8C1:
	case IVE_IMAGE_TYPE_U8C1:
	case IVE_IMAGE_TYPE_U8C3_PACKAGE:
	case IVE_IMAGE_TYPE_BF16C1:
	case IVE_IMAGE_TYPE_U16C1:
	case IVE_IMAGE_TYPE_S16C1:
	case IVE_IMAGE_TYPE_U32C1:
	case IVE_IMAGE_TYPE_FP32C1: {
		u32Len = pstImg->u32Stride[0] * u32Height;
	} break;
	case IVE_IMAGE_TYPE_YUV420SP: {
		u32Len = pstImg->u32Stride[0] * u32Height;
		u32Len += pstImg->u32Stride[0] * u32Height >> 1;
	} break;
	case IVE_IMAGE_TYPE_YUV420P: {
		u32Len = pstImg->u32Stride[0] * u32Height;
		u32Len += pstImg->u32Stride[1] * u32Height >> 1;
		u32Len += pstImg->u32Stride[1] * u32Height >> 1;
	} break;
	case IVE_IMAGE_TYPE_YUV422SP: {
		u32Len = pstImg->u32Stride[0] * u32Height;
		u32Len += pstImg->u32Stride[0] * u32Height;
	} break;
	case IVE_IMAGE_TYPE_YUV422P: {
		u32Len = pstImg->u32Stride[0] * u32Height;
		u32Len += pstImg->u32Stride[1] * u32Height;
		u32Len += pstImg->u32Stride[1] * u32Height;
	} break;
	case IVE_IMAGE_TYPE_U8C3_PLANAR: {
		u32Len = pstImg->u32Stride[0] * u32Height;
		u32Len += pstImg->u32Stride[0] * u32Height;
		u32Len += pstImg->u32Stride[0] * u32Height;
	} break;
	default:
		printf("Not supported enType %s.\n",
			   cviIveImgEnTypeStr[pstImg->enType]);
		return CVI_ERR_IVE_ILLEGAL_PARAM;
	}
	if (u32Len == 0) {
		printf("[DEV] BufFlush Stride not set.\n");
		return CVI_ERR_IVE_ILLEGAL_PARAM;
	}
	if (bFlush)
		return CVI_SYS_IonFlushCache(pstImg->u64PhyAddr[0], (char *)(uintptr_t)pstImg->u64VirAddr[0], u32Len);
	return CVI_SYS_IonInvalidateCache(pstImg->u64PhyAddr[0], (char *)(uintptr_t)pstImg->u64VirAddr[0], u32Len);
}

CVI_S32 CVI_IVE_BufFlush(IVE_HANDLE pIveHandle, IVE_IMAGE_S *pstImg)
{
	return _CVI_IVE_BufFlush_Request(pIveHandle, pstImg, true);
}

CVI_S32 CVI_IVE_BufRequest(IVE_HANDLE pIveHandle, IVE_IMAGE_S *pstImg)
{
	return _CVI_IVE_BufFlush_Request(pIveHandle, pstImg, false);
}

CVI_S32 _CVI_IVE_Write(const char *filename, CVI_U64 u64VirAddr, CVI_U32 len)
{
	FILE *fp;
	int readCnt = 0;
	CVI_S32 s32Ret = CVI_SUCCESS;

	fp = fopen(filename, "w");
	if (fp == NULL) {
		printf("Can't open %s\n", filename);
		return ERR_IVE_OPEN_FILE;
	}
	while (len) {
		readCnt = fwrite((char *)(uintptr_t)(u64VirAddr + readCnt), 1,
				 len, fp);
		len -= readCnt;
		if (readCnt == 0) {
			printf("Image %s write failed.\n", filename);
			return ERR_IVE_WRITE_FILE;
		}
	}

	fclose(fp);
	return s32Ret;
}

CVI_S32 CVI_IVE_WriteData(IVE_HANDLE pIveHandle, const char *filename,
			  IVE_DATA_S *pstData)
{
	UNUSED(pIveHandle);
	int input_num = 2;
	void* ptr[input_num];
	CVI_S32 s32Ret = CVI_SUCCESS;

	ptr[0] = (void *)filename;
	ptr[1] = pstData;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, NULL, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	return _CVI_IVE_Write(filename, pstData->u64VirAddr,
				  pstData->u32Stride * pstData->u32Height);
}

CVI_S32 CVI_IVE_WriteMem(IVE_HANDLE pIveHandle, const char *filename,
			 IVE_MEM_INFO_S *pstMem)
{
	UNUSED(pIveHandle);
	int input_num = 2;
	void* ptr[input_num];
	CVI_S32 s32Ret = CVI_SUCCESS;

	ptr[0] = (void *)filename;
	ptr[1] = pstMem;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, NULL, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	return _CVI_IVE_Write(filename, pstMem->u64VirAddr, pstMem->u32Size);
}

CVI_S32 CVI_IVE_WriteImg(IVE_HANDLE pIveHandle, const char *filename,
			 IVE_IMAGE_S *pstImg)
{
	UNUSED(pIveHandle);
	float desiredNChannels = 1.0;
	int input_num = 2;
	void* ptr[input_num];
	CVI_S32 s32Ret = CVI_SUCCESS;

	ptr[0] = (void *)filename;
	ptr[1] = pstImg;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, NULL, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	switch (pstImg->enType) {
	case IVE_IMAGE_TYPE_U8C1:
	case IVE_IMAGE_TYPE_S8C1:
	case IVE_IMAGE_TYPE_S16C1:
	case IVE_IMAGE_TYPE_U16C1:
	case IVE_IMAGE_TYPE_S32C1:
	case IVE_IMAGE_TYPE_U32C1:
	case IVE_IMAGE_TYPE_S64C1:
	case IVE_IMAGE_TYPE_U64C1:
	case IVE_IMAGE_TYPE_U8C3_PACKAGE: {
		desiredNChannels = 1;
	} break;
	case IVE_IMAGE_TYPE_YUV420P:
	case IVE_IMAGE_TYPE_YUV420SP: {
		desiredNChannels = 1.5;
	} break;
	case IVE_IMAGE_TYPE_YUV422P:
	case IVE_IMAGE_TYPE_YUV422SP: {
		desiredNChannels = 2;
	} break;
	case IVE_IMAGE_TYPE_U8C3_PLANAR: {
		desiredNChannels = 3;
	} break;
	default: {
		printf("Unsupported conversion type: %u.\n", pstImg->enType);
		return CVI_ERR_IVE_ILLEGAL_PARAM;
	} break;
	}
	int len = (int)((float)pstImg->u32Stride[0] * (float)pstImg->u32Height *
			desiredNChannels);

	return _CVI_IVE_Write(filename, pstImg->u64VirAddr[0], len);
}

CVI_S32 CVI_SYS_FreeM(IVE_HANDLE pIveHandle, IVE_MEM_INFO_S *pstMem)
{
	UNUSED(pIveHandle);
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 1;
	void* ptr[input_num];

	ptr[0] = pstMem;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, NULL, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	return CVI_SYS_IonFree(pstMem->u64PhyAddr,
				   (char *)(uintptr_t)pstMem->u64VirAddr);
}

CVI_S32 CVI_SYS_FreeI(IVE_HANDLE pIveHandle, IVE_IMAGE_S *pstImg)
{
	UNUSED(pIveHandle);
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 1;
	void* ptr[input_num];

	ptr[0] = pstImg;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, NULL, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	return CVI_SYS_IonFree(pstImg->u64PhyAddr[0],
				   (char *)(uintptr_t)pstImg->u64VirAddr[0]);
}

CVI_S32 CVI_SYS_FreeD(IVE_HANDLE pIveHandle, IVE_DATA_S *pstData)
{
	UNUSED(pIveHandle);
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 1;
	void* ptr[input_num];

	ptr[0] = pstData;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, NULL, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	return CVI_SYS_IonFree(pstData->u64PhyAddr,
				   (char *)(uintptr_t)pstData->u64VirAddr);
}

CVI_S32 CVI_IVE_DiffFg_Split(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstDiffFg,
				 IVE_DST_IMAGE_S *pstBgDiffFg,
				 IVE_DST_IMAGE_S *pstFrmDiffFg)
{
	UNUSED(pIveHandle);
	int i = 0;
	int size;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 3;
	void* ptr[input_num];

	ptr[0] = pstBgDiffFg;
	ptr[1] = pstDiffFg;
	ptr[2] = pstFrmDiffFg;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstDiffFg, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	size = (pstDiffFg->u32Stride[0] / sizeof(uint16_t)) *
		   pstDiffFg->u32Height;
	for (i = 0; i < size; i++) {
		((char *)(uintptr_t)pstBgDiffFg->u64VirAddr[0])[i] =
			((char *)(uintptr_t)pstDiffFg->u64VirAddr[0])[i * 2];
		((char *)(uintptr_t)pstFrmDiffFg->u64VirAddr[0])[i] =
			((char *)(uintptr_t)pstDiffFg->u64VirAddr[0])[i * 2 + 1];
	}
	return s32Ret;
}

CVI_S32 CVI_IVE_ChgSta_Split(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstChgSta,
				 IVE_DST_IMAGE_S *pstChgStaImg,
				 IVE_DST_IMAGE_S *pstChgStaFg,
				 IVE_DST_IMAGE_S *pstChStaLift)
{
	UNUSED(pIveHandle);
	int i = 0;
	int size;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 4;
	void* ptr[input_num];

	ptr[0] = pstChgSta;
	ptr[1] = pstChgStaImg;
	ptr[2] = pstChgStaFg;
	ptr[3] = pstChStaLift;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstChgSta, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	size = (pstChgSta->u32Stride[0] / sizeof(uint32_t)) *
		   pstChgSta->u32Height;
	for (i = 0; i < size; i++) {
		((char *)(uintptr_t)pstChgStaImg->u64VirAddr[0])[i] =
			((char *)(uintptr_t)pstChgSta->u64VirAddr[0])[i * 4];//0
		((char *)(uintptr_t)pstChgStaFg->u64VirAddr[0])[i] =
			((char *)(uintptr_t)pstChgSta->u64VirAddr[0])[i * 4 + 1];//1
		((char *)(uintptr_t)pstChStaLift->u64VirAddr[0])[i * 2] =
			((char *)(uintptr_t)pstChgSta->u64VirAddr[0])[i * 4 + 2];//2
		((char *)(uintptr_t)pstChStaLift->u64VirAddr[0])[i * 2 + 1] =
			((char *)(uintptr_t)pstChgSta->u64VirAddr[0])[i * 4 + 3];//3
	}
	return s32Ret;
}

CVI_S32 CVI_IVE_DUMP(IVE_HANDLE pIveHandle)
{
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 1;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, NULL, NULL, CVI_IVE_IOC_DUMP);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ioctl(p->devfd, CVI_IVE_IOC_DUMP);
	return CVI_SUCCESS;
}

CVI_S32 CVI_IVE_RESET(IVE_HANDLE pIveHandle, int s)
{
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 1;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, NULL, NULL, CVI_IVE_IOC_RESET);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ioctl_arg.input_data = (CVI_U64)&s;
	ioctl(p->devfd, CVI_IVE_IOC_RESET, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_QUERY(IVE_HANDLE pIveHandle, CVI_BOOL *pbFinish,
			  CVI_BOOL bBlock)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_QUERY_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 2;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pbFinish;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, NULL, NULL, CVI_IVE_IOC_QUERY);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.pbFinish = pbFinish;
	ive_arg.bBlock = bBlock;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_QUERY, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_DMA(IVE_HANDLE pIveHandle, IVE_DATA_S *pstSrc,
			IVE_DST_DATA_S *pstDst, IVE_DMA_CTRL_S *pstCtrl,
			CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_DMA_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 4;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstDst;
	ptr[3] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, NULL, pstCtrl, CVI_IVE_IOC_DMA);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	ive_arg.stDst = *pstDst;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_DMA, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_And(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc1,
			IVE_SRC_IMAGE_S *pstSrc2, IVE_DST_IMAGE_S *pstDst,
			CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_AND_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 4;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc1;
	ptr[2] = pstSrc2;
	ptr[3] = pstDst;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc1, NULL, CVI_IVE_IOC_AND);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc1 = *pstSrc1;
	ive_arg.stSrc2 = *pstSrc2;
	ive_arg.stDst = *pstDst;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_AND, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_Or(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc1,
		   IVE_SRC_IMAGE_S *pstSrc2, IVE_DST_IMAGE_S *pstDst,
		   CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_OR_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 4;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc1;
	ptr[2] = pstSrc2;
	ptr[3] = pstDst;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc1, NULL, CVI_IVE_IOC_OR);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc1 = *pstSrc1;
	ive_arg.stSrc2 = *pstSrc2;
	ive_arg.stDst = *pstDst;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_OR, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_Xor(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc1,
			IVE_SRC_IMAGE_S *pstSrc2, IVE_DST_IMAGE_S *pstDst,
			CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_XOR_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 4;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc1;
	ptr[2] = pstSrc2;
	ptr[3] = pstDst;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc1, NULL, CVI_IVE_IOC_XOR);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc1 = *pstSrc1;
	ive_arg.stSrc2 = *pstSrc2;
	ive_arg.stDst = *pstDst;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_XOR, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_Add(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc1,
			IVE_SRC_IMAGE_S *pstSrc2, IVE_DST_IMAGE_S *pstDst,
			IVE_ADD_CTRL_S *pstCtrl, CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_ADD_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 5;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc1;
	ptr[2] = pstSrc2;
	ptr[3] = pstDst;
	ptr[4] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc1, pstCtrl, CVI_IVE_IOC_ADD);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc1 = *pstSrc1;
	ive_arg.stSrc2 = *pstSrc2;
	ive_arg.stDst = *pstDst;
	ive_arg.pstCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	if (ioctl(p->devfd, CVI_IVE_IOC_ADD, &ioctl_arg) < 0) {
		fprintf(stderr, "SYS_IOC_S_CTRL - %s NG\n", __func__);
		return -1;
	}
	return s32Ret;
}

CVI_S32 CVI_IVE_Sub(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc1,
			IVE_SRC_IMAGE_S *pstSrc2, IVE_DST_IMAGE_S *pstDst,
			IVE_SUB_CTRL_S *pstCtrl, CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_SUB_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 5;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc1;
	ptr[2] = pstSrc2;
	ptr[3] = pstDst;
	ptr[4] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc1, pstCtrl, CVI_IVE_IOC_SUB);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc1 = *pstSrc1;
	ive_arg.stSrc2 = *pstSrc2;
	ive_arg.stDst = *pstDst;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_SUB, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_Erode(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
			  IVE_DST_IMAGE_S *pstDst, IVE_ERODE_CTRL_S *pstCtrl,
			  CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_ERODE_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 4;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstDst;
	ptr[3] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, pstCtrl, CVI_IVE_IOC_ERODE);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	ive_arg.stDst = *pstDst;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_ERODE, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_Dilate(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
			   IVE_DST_IMAGE_S *pstDst, IVE_DILATE_CTRL_S *pstctrl,
			   CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_DILATE_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 4;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstDst;
	ptr[3] = pstctrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, pstctrl, CVI_IVE_IOC_DILATE);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	ive_arg.stDst = *pstDst;
	ive_arg.stCtrl = *pstctrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_DILATE, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_Thresh(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
			   IVE_DST_IMAGE_S *pstDst, IVE_THRESH_CTRL_S *pstCtrl,
			   CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_THRESH_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 4;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstDst;
	ptr[3] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, pstCtrl, CVI_IVE_IOC_THRESH);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	ive_arg.stDst = *pstDst;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_THRESH, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_MatchBgModel(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstCurImg,
				 IVE_DATA_S *pstBgModel, IVE_IMAGE_S *pstFgFlag,
				 IVE_DST_IMAGE_S *pstDiffFg,
				 IVE_DST_MEM_INFO_S *pstStatData,
				 IVE_MATCH_BG_MODEL_CTRL_S *pstCtrl,
				 CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_MATCH_BGMODEL_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 7;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstCurImg;
	ptr[2] = pstBgModel;
	ptr[3] = pstFgFlag;
	ptr[4] = pstDiffFg;
	ptr[5] = pstStatData;
	ptr[6] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstCurImg, pstCtrl, CVI_IVE_IOC_MATCH_BGMODEM);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stCurImg = *pstCurImg;
	ive_arg.stBgModel = *pstBgModel;
	ive_arg.stFgFlag = *pstFgFlag;
	ive_arg.stDiffFg = *pstDiffFg;
	ive_arg.stStatData = *pstStatData;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl_arg.buffer = (void *)(uintptr_t)pstStatData->u64VirAddr;
	ioctl_arg.size = sizeof(IVE_BG_STAT_DATA_S);
	ioctl(p->devfd, CVI_IVE_IOC_MATCH_BGMODEM, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_UpdateBgModel(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstCurImg, IVE_DATA_S *pstBgModel,
				  IVE_IMAGE_S *pstFgFlag, IVE_DST_IMAGE_S *pstBgImg,
				  IVE_DST_IMAGE_S *pstChgSta,
				  IVE_DST_MEM_INFO_S *pstStatData,
				  IVE_UPDATE_BG_MODEL_CTRL_S *pstCtrl,
				  CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_UPDATE_BGMODEL_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 8;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstCurImg;
	ptr[2] = pstBgModel;
	ptr[3] = pstFgFlag;
	ptr[4] = pstBgImg;
	ptr[5] = pstChgSta;
	ptr[6] = pstStatData;
	ptr[7] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstCurImg, pstCtrl, CVI_IVE_IOC_UPDATE_BGMODEL);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stCurImg = *pstCurImg;
	ive_arg.stBgModel = *pstBgModel;
	ive_arg.stFgFlag = *pstFgFlag;
	ive_arg.stBgImg = *pstBgImg;
	ive_arg.stChgSta = *pstChgSta;
	ive_arg.stStatData = *pstStatData;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl_arg.buffer = (void *)(uintptr_t)pstStatData->u64VirAddr;
	ioctl_arg.size = sizeof(IVE_BG_STAT_DATA_S);
	ioctl(p->devfd, CVI_IVE_IOC_UPDATE_BGMODEL, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_GMM(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
			IVE_DST_IMAGE_S *pstFg, IVE_DST_IMAGE_S *pstBg,
			IVE_MEM_INFO_S *pstModel, IVE_GMM_CTRL_S *pstCtrl,
			CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_GMM_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 6;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstFg;
	ptr[3] = pstBg;
	ptr[4] = pstModel;
	ptr[5] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, pstCtrl, CVI_IVE_IOC_GMM);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	ive_arg.stFg = *pstFg;
	ive_arg.stBg = *pstBg;
	ive_arg.stModel = *pstModel;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_GMM, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_GMM2(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
			 IVE_SRC_IMAGE_S *pstFactor, IVE_DST_IMAGE_S *pstFg,
			 IVE_DST_IMAGE_S *pstBg, IVE_DST_IMAGE_S *pstMatchModelInfo,
			 IVE_MEM_INFO_S *pstModel, IVE_GMM2_CTRL_S *pstCtrl,
			 CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_GMM2_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 8;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstFactor;
	ptr[3] = pstFg;
	ptr[4] = pstBg;
	ptr[5] = pstMatchModelInfo;
	ptr[6] = pstModel;
	ptr[7] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, pstCtrl, CVI_IVE_IOC_GMM2);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	ive_arg.stFactor = *pstFactor;
	ive_arg.stFg = *pstFg;
	ive_arg.stBg = *pstBg;
	ive_arg.stInfo = *pstMatchModelInfo;
	ive_arg.stModel = *pstModel;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_GMM2, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_Bernsen(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
			IVE_DST_IMAGE_S *pstDst, IVE_BERNSEN_CTRL_S *pstCtrl,
			CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_BERNSEN_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 4;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstDst;
	ptr[3] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, pstCtrl, CVI_IVE_IOC_BERNSEN);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	ive_arg.stDst = *pstDst;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_BERNSEN, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_Filter(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
			   IVE_DST_IMAGE_S *pstDst, IVE_FILTER_CTRL_S *pstCtrl,
			   CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_FILTER_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 4;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstDst;
	ptr[3] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, pstCtrl, CVI_IVE_IOC_FILTER);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	ive_arg.stDst = *pstDst;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_FILTER, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_Sobel(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
			  IVE_DST_IMAGE_S *pstDstH, IVE_DST_IMAGE_S *pstDstV,
			  IVE_SOBEL_CTRL_S *pstCtrl, CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_SOBEL_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 3;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, pstCtrl, CVI_IVE_IOC_SOBEL);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	if (pstDstH != CVI_NULL)
		ive_arg.stDstH = *pstDstH;
	if (pstDstV != CVI_NULL)
		ive_arg.stDstV = *pstDstV;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_SOBEL, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_MagAndAng(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
			  IVE_DST_IMAGE_S *pstDstMag,
			  IVE_DST_IMAGE_S *pstDstAng,
			  IVE_MAG_AND_ANG_CTRL_S *pstCtrl, CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_MAGANANG_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 3;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, pstCtrl, CVI_IVE_IOC_MAG_AND_ANG);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	if (pstDstMag != CVI_NULL)
		ive_arg.stDstMag = *pstDstMag;
	if (pstDstAng != CVI_NULL)
		ive_arg.stDstAng = *pstDstAng;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_MAG_AND_ANG, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_CSC(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
			IVE_DST_IMAGE_S *pstDst, IVE_CSC_CTRL_S *pstCtrl,
			CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_CSC_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 4;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstDst;
	ptr[3] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, pstCtrl, CVI_IVE_IOC_CSC);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	ive_arg.stDst = *pstDst;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_CSC, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_FilterAndCSC(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
				 IVE_DST_IMAGE_S *pstDst,
				 IVE_FILTER_AND_CSC_CTRL_S *pstCtrl,
				 CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_FILTER_AND_CSC_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 4;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstDst;
	ptr[3] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, pstCtrl, CVI_IVE_IOC_FILTER_AND_CSC);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	ive_arg.stDst = *pstDst;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_FILTER_AND_CSC, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_Hist(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
			 IVE_DST_MEM_INFO_S *pstDst, CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_HIST_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 3;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstDst;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, NULL, CVI_IVE_IOC_HIST);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	ive_arg.stDst = *pstDst;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_HIST, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_Map(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
			IVE_SRC_MEM_INFO_S *pstMap, IVE_DST_IMAGE_S *pstDst,
			IVE_MAP_CTRL_S *pstCtrl, CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_MAP_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 5;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstMap;
	ptr[3] = pstDst;
	ptr[4] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, pstCtrl, CVI_IVE_IOC_MAP);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	ive_arg.stMap = *pstMap;
	ive_arg.stDst = *pstDst;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl_arg.buffer = (void *)pstMap->u64VirAddr;
	ioctl_arg.size = pstMap->u32Size;
	ioctl(p->devfd, CVI_IVE_IOC_MAP, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_NCC(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc1,
			IVE_SRC_IMAGE_S *pstSrc2, IVE_DST_MEM_INFO_S *pstDst,
			CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_NCC_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 4;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc1;
	ptr[2] = pstSrc2;
	ptr[3] = pstDst;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc1, NULL, CVI_IVE_IOC_NCC);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc1 = *pstSrc1;
	ive_arg.stSrc2 = *pstSrc2;
	ive_arg.stDst = *pstDst;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl_arg.buffer = (void *)pstDst->u64VirAddr;
	ioctl_arg.size = sizeof(IVE_NCC_DST_MEM_S);
	ioctl(p->devfd, CVI_IVE_IOC_NCC, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_Integ(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
			  IVE_DST_MEM_INFO_S *pstDst, IVE_INTEG_CTRL_S *pstCtrl,
			  CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_INTEG_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 4;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstDst;
	ptr[3] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, pstCtrl, CVI_IVE_IOC_INTEG);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	ive_arg.stDst = *pstDst;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_INTEG, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_LBP(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
			IVE_DST_IMAGE_S *pstDst, IVE_LBP_CTRL_S *pstCtrl,
			CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_LBP_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 4;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstDst;
	ptr[3] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, pstCtrl, CVI_IVE_IOC_LBP);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = p;
	ive_arg.stSrc = *pstSrc;
	ive_arg.stDst = *pstDst;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_LBP, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_Thresh_S16(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
			   IVE_DST_IMAGE_S *pstDst,
			   IVE_THRESH_S16_CTRL_S *pstCtrl, CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_THRESH_S16_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 4;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstDst;
	ptr[3] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, pstCtrl, CVI_IVE_IOC_THRESH_S16);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	ive_arg.stDst = *pstDst;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_THRESH_S16, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_Thresh_U16(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
			   IVE_DST_IMAGE_S *pstDst,
			   IVE_THRESH_U16_CTRL_S *pstCtrl, CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_THRESH_U16_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 4;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstDst;
	ptr[3] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, pstCtrl, CVI_IVE_IOC_THRESH_U16);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	ive_arg.stDst = *pstDst;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_THRESH_U16, &ioctl_arg);
	return 0;
}

CVI_S32 CVI_IVE_16BitTo8Bit(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
				IVE_DST_IMAGE_S *pstDst,
				IVE_16BIT_TO_8BIT_CTRL_S *pstCtrl,
				CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_16BIT_TO_8BIT_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 4;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstDst;
	ptr[3] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, pstCtrl, CVI_IVE_IOC_16BIT_TO_8BIT);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	ive_arg.stDst = *pstDst;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_16BIT_TO_8BIT, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_OrdStatFilter(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
				  IVE_DST_IMAGE_S *pstDst,
				  IVE_ORD_STAT_FILTER_CTRL_S *pstCtrl,
				  CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_ORD_STAT_FILTER_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 4;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstDst;
	ptr[3] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, pstCtrl, CVI_IVE_IOC_ORD_STAT_FILTER);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	ive_arg.stDst = *pstDst;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_ORD_STAT_FILTER, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_CannyEdge(IVE_IMAGE_S *pstEdge, IVE_MEM_INFO_S *pstStack)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 1;
	void* ptr[input_num];

	ptr[0] = pstEdge;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstEdge, NULL, 0);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	UNUSED(pstStack);
	do_hysteresis_wo_ang((unsigned char *)(uintptr_t)pstEdge->u64VirAddr[0],
				 (unsigned char *)(uintptr_t)pstEdge->u64VirAddr[0],
				 pstEdge->u32Width, pstEdge->u32Height, pstEdge->u32Stride[0]);

	return s32Ret;
}

CVI_S32 CVI_IVE_CannyHysEdge(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
				 IVE_DST_IMAGE_S *pstEdge,
				 IVE_DST_MEM_INFO_S *pstStack,
				 IVE_CANNY_HYS_EDGE_CTRL_S *pstCtrl,
				 CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_CANNY_HYS_EDGE_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 5;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstEdge;
	ptr[3] = pstStack;
	ptr[4] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, pstCtrl, CVI_IVE_IOC_CANNYHYSEDGE);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	ive_arg.stDst = *pstEdge;
	ive_arg.stStack = *pstStack;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_CANNYHYSEDGE, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_NormGrad(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
			 IVE_DST_IMAGE_S *pstDstH, IVE_DST_IMAGE_S *pstDstV,
			 IVE_DST_IMAGE_S *pstDstHV,
			 IVE_NORM_GRAD_CTRL_S *pstCtrl, CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_NORM_GRAD_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 3;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, pstCtrl, CVI_IVE_IOC_NORMGRAD);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	if (pstDstH != CVI_NULL)
		ive_arg.stDstH = *pstDstH;
	if (pstDstV != CVI_NULL)
		ive_arg.stDstV = *pstDstV;
	if (pstDstHV != CVI_NULL)
		ive_arg.stDstHV = *pstDstHV;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_NORMGRAD, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_GradFg(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstBgDiffFg,
			   IVE_SRC_IMAGE_S *pstCurGrad, IVE_SRC_IMAGE_S *pstBgGrad,
			   IVE_DST_IMAGE_S *pstGradFg, IVE_GRAD_FG_CTRL_S *pstCtrl,
			   CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_GRAD_FG_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 6;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstBgDiffFg;
	ptr[2] = pstCurGrad;
	ptr[3] = pstBgGrad;
	ptr[4] = pstGradFg;
	ptr[5] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstBgDiffFg, pstCtrl, CVI_IVE_IOC_GRADFG);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	if (pstBgDiffFg != CVI_NULL)
		ive_arg.stBgDiffFg = *pstBgDiffFg;
	if (pstCurGrad != CVI_NULL)
		ive_arg.stCurGrad = *pstCurGrad;
	ive_arg.stBgGrad = *pstBgGrad;
	ive_arg.stGradFg = *pstGradFg;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_GRADFG, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_SAD(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc1,
			IVE_SRC_IMAGE_S *pstSrc2, IVE_DST_IMAGE_S *pstSad,
			IVE_DST_IMAGE_S *pstThr, IVE_SAD_CTRL_S *pstCtrl,
			CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_SAD_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 6;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc1;
	ptr[2] = pstSrc2;
	ptr[3] = pstSad;
	ptr[4] = pstThr;
	ptr[5] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc1, pstCtrl, CVI_IVE_IOC_SAD);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc1 = *pstSrc1;
	ive_arg.stSrc2 = *pstSrc2;
	ive_arg.stSad = *pstSad;
	ive_arg.stThr = *pstThr;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_SAD, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_Resize(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
			   IVE_DST_IMAGE_S *pstDst, IVE_RESIZE_CTRL_S *pstCtrl,
			   CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_RESIZE_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 4;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstDst;
	ptr[3] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, pstCtrl, CVI_IVE_IOC_RESIZE);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	ive_arg.stDst = *pstDst;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_RESIZE, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_imgInToOdma(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
				IVE_DST_IMAGE_S *pstDst, IVE_FILTER_CTRL_S *pstCtrl,
				CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_FILTER_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;

	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	ive_arg.stDst = *pstDst;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_IMGIN_To_ODMA, &ioctl_arg);
	return 0;
}

CVI_S32 CVI_IVE_rgbPToYuvToErodeToDilate(IVE_HANDLE pIveHandle,
					 IVE_SRC_IMAGE_S *pstSrc,
					 IVE_DST_IMAGE_S *pstDst1,
					 IVE_DST_IMAGE_S *pstDst2,
					 IVE_FILTER_CTRL_S *pstCtrl,
					 CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_RGBP2YUV2ERODE2DILATE ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 5;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstDst1;
	ptr[3] = pstDst2;
	ptr[4] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, pstCtrl, CVI_IVE_IOC_RGBP2YUV2ERODE2DILATE);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	ive_arg.stDst1 = *pstDst1;
	ive_arg.stDst2 = *pstDst2;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_RGBP2YUV2ERODE2DILATE, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_STCandiCorner(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc,
				  IVE_DST_IMAGE_S *pstDst,
				  IVE_ST_CANDI_CORNER_CTRL_S *pstCtrl,
				  CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_STCANDICORNER ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 4;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc;
	ptr[2] = pstDst;
	ptr[3] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc, pstCtrl, CVI_IVE_IOC_ST_CANDI_CORNER);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc = *pstSrc;
	ive_arg.stDst = *pstDst;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_ST_CANDI_CORNER, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_FrameDiffMotion(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *pstSrc1,
				IVE_SRC_IMAGE_S *pstSrc2,
				IVE_DST_IMAGE_S *pstDst,
				IVE_FRAME_DIFF_MOTION_CTRL_S *pstCtrl,
				CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_MD ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 5;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = pstSrc1;
	ptr[2] = pstSrc2;
	ptr[3] = pstDst;
	ptr[4] = pstCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, pstSrc1, pstCtrl, CVI_IVE_IOC_MD);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}
	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrc1 = *pstSrc1;
	ive_arg.stSrc2 = *pstSrc2;
	ive_arg.stDst = *pstDst;
	ive_arg.stCtrl = *pstCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl(p->devfd, CVI_IVE_IOC_MD, &ioctl_arg);
	return s32Ret;
}

CVI_S32 CVI_IVE_CMDQ(IVE_HANDLE pIveHandle)
{
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;

	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}

	ioctl(p->devfd, CVI_IVE_IOC_CMDQ, NULL);
	return 0;
}
CVI_S32 CVI_IVE_CCL(IVE_HANDLE pIveHandle, IVE_SRC_IMAGE_S *stSrcDst,
			IVE_DST_MEM_INFO_S *stBlob,
			IVE_CCL_CTRL_S *stCclCtrl,
			CVI_BOOL bInstant)
{
	CVI_IVE_IOCTL_ARG ioctl_arg;
	CVI_IVE_IOCTL_CCL_ARG ive_arg;
	struct IVE_HANDLE_CTX *p = (struct IVE_HANDLE_CTX *)pIveHandle;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int input_num = 4;
	void* ptr[input_num];

	ptr[0] = pIveHandle;
	ptr[1] = stSrcDst;
	ptr[2] = stBlob;
	ptr[3] = stCclCtrl;
	s32Ret = CHECK_FUNC_ALL(ptr, input_num, stSrcDst, stCclCtrl, CVI_IVE_IOC_CCL);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;
	if (p->devfd <= 0) {
		printf("Device ive is not open, please check it\n");
		return CVI_ERR_IVE_INVALID_DEVID;
	}

	ive_arg.pIveHandle = pIveHandle;
	ive_arg.stSrcDst = *stSrcDst;
	ive_arg.stBlob = *stBlob;
	ive_arg.stCclCtrl = *stCclCtrl;
	ive_arg.bInstant = bInstant;

	ioctl_arg.input_data = (CVI_U64)&ive_arg;
	ioctl_arg.buffer = (void *)(uintptr_t)stBlob->u64VirAddr;
	ioctl_arg.size = sizeof(CVI_U16) + sizeof(CVI_S8) + sizeof(CVI_U8);
	ioctl(p->devfd, CVI_IVE_IOC_CCL, &ioctl_arg);
	return s32Ret;

}

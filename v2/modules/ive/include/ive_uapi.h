#ifndef __CVI_IVE_UAPI_H__
#define __CVI_IVE_UAPI_H__

#include "cvi_comm_ive.h"
/*
 * mw iotcl arg struct is differented from kernel
 * if update this file, please modify kernel ive_uapi.h as the same way
 *
*/
typedef struct _CVI_IVE_IOCTL_ARG {
	void *buffer;
	unsigned long input_data;
	uint32_t size;
} CVI_IVE_IOCTL_ARG;

typedef struct _CVI_IVE_TEST_ARG {
	IVE_IMAGE_TYPE_E enType;
	char *addr;
#ifdef __arm__
	__u32 padding;
#endif
	uint16_t u16Width;
	uint16_t u16Height;
} CVI_IVE_TEST_ARG;

typedef struct _CVI_IVE_QUERY_ARG {
	IVE_HANDLE pIveHandle;
	unsigned char *pbFinish;
#ifdef __arm__
	__u32 padding;
#endif
	unsigned char bBlock;
} CVI_IVE_QUERY_ARG;

typedef struct _CVI_IVE_IOCTL_ADD_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc1;
	IVE_SRC_IMAGE_S stSrc2;
	IVE_DST_IMAGE_S stDst;
	IVE_ADD_CTRL_S pstCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_ADD_ARG;

typedef struct _CVI_IVE_IOCTL_AND_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc1;
	IVE_SRC_IMAGE_S stSrc2;
	IVE_DST_IMAGE_S stDst;
	unsigned char bInstant;
} CVI_IVE_IOCTL_AND_ARG;

typedef struct _CVI_IVE_IOCTL_XOR_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc1;
	IVE_SRC_IMAGE_S stSrc2;
	IVE_DST_IMAGE_S stDst;
	unsigned char bInstant;
} CVI_IVE_IOCTL_XOR_ARG;

typedef struct _CVI_IVE_IOCTL_OR_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc1;
	IVE_SRC_IMAGE_S stSrc2;
	IVE_DST_IMAGE_S stDst;
	unsigned char bInstant;
} CVI_IVE_IOCTL_OR_ARG;

typedef struct _CVI_IVE_IOCTL_SUB_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc1;
	IVE_SRC_IMAGE_S stSrc2;
	IVE_DST_IMAGE_S stDst;
	IVE_SUB_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_SUB_ARG;

typedef struct _CVI_IVE_IOCTL_ERODE_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_ERODE_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_ERODE_ARG;

typedef struct _CVI_IVE_IOCTL_DILATE_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_DILATE_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_DILATE_ARG;

typedef struct _CVI_IVE_IOCTL_THRESH_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_THRESH_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_THRESH_ARG;

typedef struct _CVI_IVE_IOCTL_MATCH_BGMODEL_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stCurImg;
	IVE_DATA_S stBgModel;
	IVE_IMAGE_S stFgFlag;
	IVE_DST_IMAGE_S stDiffFg;
	IVE_DST_MEM_INFO_S stStatData;
	IVE_MATCH_BG_MODEL_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_MATCH_BGMODEL_ARG;

typedef struct _CVI_IVE_IOCTL_UPDATE_BGMODEL_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stCurImg;
	IVE_DATA_S stBgModel;
	IVE_IMAGE_S stFgFlag;
	IVE_DST_IMAGE_S stBgImg;
	IVE_DST_IMAGE_S stChgSta;
	IVE_DST_MEM_INFO_S stStatData;
	IVE_UPDATE_BG_MODEL_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_UPDATE_BGMODEL_ARG;

typedef struct _CVI_IVE_IOCTL_GMM_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stFg;
	IVE_DST_IMAGE_S stBg;
	IVE_MEM_INFO_S stModel;
	IVE_GMM_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_GMM_ARG;

typedef struct _CVI_IVE_IOCTL_GMM2_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_SRC_IMAGE_S stFactor;
	IVE_DST_IMAGE_S stFg;
	IVE_DST_IMAGE_S stBg;
	IVE_DST_IMAGE_S stInfo;
	IVE_MEM_INFO_S stModel;
	IVE_GMM2_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_GMM2_ARG;

typedef struct _CVI_IVE_IOCTL_DMA_ARG {
	IVE_HANDLE pIveHandle;
	IVE_DATA_S stSrc;
	IVE_DST_DATA_S stDst;
	IVE_DMA_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_DMA_ARG;

typedef struct _CVI_IVE_IOCTL_BERNSEN_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_BERNSEN_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_BERNSEN_ARG;

typedef struct _CVI_IVE_IOCTL_FILTER_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_FILTER_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_FILTER_ARG;

typedef struct _CVI_IVE_IOCTL_SOBEL_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDstH;
	IVE_DST_IMAGE_S stDstV;
	IVE_SOBEL_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_SOBEL_ARG;

typedef struct _CVI_IVE_IOCTL_MAGANANG_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDstMag;
	IVE_DST_IMAGE_S stDstAng;
	IVE_MAG_AND_ANG_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_MAGANANG_ARG;

typedef struct _CVI_IVE_IOCTL_CSC_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_CSC_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_CSC_ARG;

typedef struct _CVI_IVE_IOCTL_HIST_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_MEM_INFO_S stDst;
	unsigned char bInstant;
} CVI_IVE_IOCTL_HIST_ARG;

typedef struct _CVI_IVE_IOCTL_FILTER_AND_CSC_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_FILTER_AND_CSC_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_FILTER_AND_CSC_ARG;

typedef struct _CVI_IVE_IOCTL_MAP_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_SRC_MEM_INFO_S stMap;
	IVE_DST_IMAGE_S stDst;
	IVE_MAP_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_MAP_ARG;

typedef struct _CVI_IVE_IOCTL_NCC_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc1;
	IVE_SRC_IMAGE_S stSrc2;
	IVE_DST_MEM_INFO_S stDst;
	unsigned char bInstant;
}CVI_IVE_IOCTL_NCC_ARG;

typedef struct _CVI_IVE_IOCTL_INTEG_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_MEM_INFO_S stDst;
	IVE_INTEG_CTRL_S stCtrl;
	unsigned char bInstant;
}CVI_IVE_IOCTL_INTEG_ARG;

typedef struct _CVI_IVE_IOCTL_LBP_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_LBP_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_LBP_ARG;

typedef struct _CVI_IVE_IOCTL_THRESH_S16_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_THRESH_S16_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_THRESH_S16_ARG;

typedef struct _CVI_IVE_IOCTL_THRESH_U16_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_THRESH_U16_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_THRESH_U16_ARG;

typedef struct _CVI_IVE_IOCTL_16BIT_TO_8BIT_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_16BIT_TO_8BIT_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_16BIT_TO_8BIT_ARG;

typedef struct _CVI_IVE_IOCTL_ORD_STAT_FILTER_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_ORD_STAT_FILTER_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_ORD_STAT_FILTER_ARG;

typedef struct _CVI_IVE_IOCTL_CANNY_HYS_EDGE_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_DST_MEM_INFO_S stStack;
	IVE_CANNY_HYS_EDGE_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_CANNY_HYS_EDGE_ARG;

typedef struct _CVI_IVE_IOCTL_NORM_GRAD_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDstH;
	IVE_DST_IMAGE_S stDstV;
	IVE_DST_IMAGE_S stDstHV;
	IVE_NORM_GRAD_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_NORM_GRAD_ARG;

typedef struct _CVI_IVE_IOCTL_GRAD_FG_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stBgDiffFg;
	IVE_SRC_IMAGE_S stCurGrad;
	IVE_SRC_IMAGE_S stBgGrad;
	IVE_DST_IMAGE_S stGradFg;
	IVE_GRAD_FG_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_GRAD_FG_ARG;

typedef struct _CVI_IVE_IOCTL_SAD_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc1;
	IVE_SRC_IMAGE_S stSrc2;
	IVE_DST_IMAGE_S stSad;
	IVE_DST_IMAGE_S stThr;
	IVE_SAD_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_SAD_ARG;

typedef struct _CVI_IVE_IOCTL_RESIZE_ARG {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
#ifdef __arm__
		__u32 padding1;
#endif
	IVE_DST_IMAGE_S stDst;
#ifdef __arm__
		__u32 padding2;
#endif
	IVE_RESIZE_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_RESIZE_ARG;

typedef struct _CVI_IVE_IOCTL_CCL_ARG{
	IVE_HANDLE pIveHandle;
	IVE_IMAGE_S stSrcDst;
	IVE_DST_MEM_INFO_S stBlob;
	IVE_CCL_CTRL_S stCclCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_CCL_ARG;

typedef struct _CVI_IVE_IOCTL_RGBP2YUV2ERODE2DILATE {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst1;
	IVE_DST_IMAGE_S stDst2;
	IVE_FILTER_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_RGBP2YUV2ERODE2DILATE;

typedef struct _CVI_IVE_IOCTL_STCANDICORNER {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc;
	IVE_DST_IMAGE_S stDst;
	IVE_ST_CANDI_CORNER_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_STCANDICORNER;

typedef struct _CVI_IVE_IOCTL_MD {
	IVE_HANDLE pIveHandle;
	IVE_SRC_IMAGE_S stSrc1;
	IVE_SRC_IMAGE_S stSrc2;
	IVE_DST_IMAGE_S stDst;
	IVE_FRAME_DIFF_MOTION_CTRL_S stCtrl;
	unsigned char bInstant;
} CVI_IVE_IOCTL_MD;

#define CVI_IVE_IOC_MAGIC 'v'
#define CVI_IVE_IOC_TEST _IOW(CVI_IVE_IOC_MAGIC, 0x00, unsigned long long)
#define CVI_IVE_IOC_DMA _IOW(CVI_IVE_IOC_MAGIC, 0x01, unsigned long long)
#define CVI_IVE_IOC_ADD _IOW(CVI_IVE_IOC_MAGIC, 0x02, unsigned long long)
#define CVI_IVE_IOC_AND _IOW(CVI_IVE_IOC_MAGIC, 0x03, unsigned long long)
#define CVI_IVE_IOC_OR _IOW(CVI_IVE_IOC_MAGIC, 0x04, unsigned long long)
#define CVI_IVE_IOC_SUB _IOW(CVI_IVE_IOC_MAGIC, 0x05, unsigned long long)
#define CVI_IVE_IOC_XOR _IOW(CVI_IVE_IOC_MAGIC, 0x06, unsigned long long)
#define CVI_IVE_IOC_THRESH _IOW(CVI_IVE_IOC_MAGIC, 0x07, unsigned long long)
#define CVI_IVE_IOC_THRESH_S16 _IOW(CVI_IVE_IOC_MAGIC, 0x08, unsigned long long)
#define CVI_IVE_IOC_THRESH_U16 _IOW(CVI_IVE_IOC_MAGIC, 0x09, unsigned long long)
#define CVI_IVE_IOC_16BIT_TO_8BIT _IOW(CVI_IVE_IOC_MAGIC, 0x0a, unsigned long long)
#define CVI_IVE_IOC_CSC _IOW(CVI_IVE_IOC_MAGIC, 0x0b, unsigned long long)
#define CVI_IVE_IOC_GRADFG _IOW(CVI_IVE_IOC_MAGIC, 0x0c, unsigned long long)
#define CVI_IVE_IOC_NORMGRAD _IOW(CVI_IVE_IOC_MAGIC, 0x0d, unsigned long long)
#define CVI_IVE_IOC_FILTER _IOW(CVI_IVE_IOC_MAGIC, 0x0e, unsigned long long)
#define CVI_IVE_IOC_FILTER_AND_CSC _IOW(CVI_IVE_IOC_MAGIC, 0x0f, unsigned long long)
#define CVI_IVE_IOC_HIST _IOW(CVI_IVE_IOC_MAGIC, 0x10, unsigned long long)
#define CVI_IVE_IOC_EQUALIZE_HIST _IOW(CVI_IVE_IOC_MAGIC, 0x11, unsigned long long)
#define CVI_IVE_IOC_MAP _IOW(CVI_IVE_IOC_MAGIC, 0x12, unsigned long long)
#define CVI_IVE_IOC_NCC _IOWR(CVI_IVE_IOC_MAGIC, 0x13, unsigned long long)
#define CVI_IVE_IOC_ORD_STAT_FILTER _IOW(CVI_IVE_IOC_MAGIC, 0x14, unsigned long long)
#define CVI_IVE_IOC_RESIZE _IOW(CVI_IVE_IOC_MAGIC, 0x15, unsigned long long)
#define CVI_IVE_IOC_CANNYHYSEDGE _IOW(CVI_IVE_IOC_MAGIC, 0x16, unsigned long long)
#define CVI_IVE_IOC_CANNYEDGE _IOW(CVI_IVE_IOC_MAGIC, 0x17, unsigned long long)
#define CVI_IVE_IOC_INTEG _IOW(CVI_IVE_IOC_MAGIC, 0x18, unsigned long long)
#define CVI_IVE_IOC_LBP _IOW(CVI_IVE_IOC_MAGIC, 0x19, unsigned long long)
#define CVI_IVE_IOC_MAG_AND_ANG _IOW(CVI_IVE_IOC_MAGIC, 0x1a, unsigned long long)
#define CVI_IVE_IOC_ST_CANDI_CORNER _IOW(CVI_IVE_IOC_MAGIC, 0x1b, unsigned long long)
#define CVI_IVE_IOC_ST_CORNER _IOW(CVI_IVE_IOC_MAGIC, 0x1c, unsigned long long)
#define CVI_IVE_IOC_SOBEL _IOW(CVI_IVE_IOC_MAGIC, 0x1d, unsigned long long)
#define CVI_IVE_IOC_CCL _IOW(CVI_IVE_IOC_MAGIC, 0x1e, unsigned long long)
#define CVI_IVE_IOC_DILATE _IOW(CVI_IVE_IOC_MAGIC, 0x1f, unsigned long long)
#define CVI_IVE_IOC_ERODE _IOW(CVI_IVE_IOC_MAGIC, 0x20, unsigned long long)
#define CVI_IVE_IOC_MATCH_BGMODEM _IOWR(CVI_IVE_IOC_MAGIC, 0x21, unsigned long long)
#define CVI_IVE_IOC_UPDATE_BGMODEL _IOWR(CVI_IVE_IOC_MAGIC, 0x22, unsigned long long)
#define CVI_IVE_IOC_GMM _IOW(CVI_IVE_IOC_MAGIC, 0x23, unsigned long long)
#define CVI_IVE_IOC_GMM2 _IOW(CVI_IVE_IOC_MAGIC, 0x24, unsigned long long)
#define CVI_IVE_IOC_LK_OPTICAL_FLOW_PYR                                           \
	_IOW(CVI_IVE_IOC_MAGIC, 0x25, unsigned long long)
#define CVI_IVE_IOC_SAD _IOW(CVI_IVE_IOC_MAGIC, 0x26, unsigned long long)
#define CVI_IVE_IOC_BERNSEN _IOW(CVI_IVE_IOC_MAGIC, 0x27, unsigned long long)
#define CVI_IVE_IOC_IMGIN_To_ODMA _IOW(CVI_IVE_IOC_MAGIC, 0x28, unsigned long long)
#define CVI_IVE_IOC_RGBP2YUV2ERODE2DILATE                                   \
	_IOW(CVI_IVE_IOC_MAGIC, 0x29, unsigned long long)
#define CVI_IVE_IOC_MD _IOW(CVI_IVE_IOC_MAGIC, 0x2a, unsigned long long)
#define CVI_IVE_IOC_CMDQ _IOW(CVI_IVE_IOC_MAGIC, 0x2b, unsigned long long)
#define CVI_IVE_IOC_RESET _IOW(CVI_IVE_IOC_MAGIC, 0xF0, unsigned long long)
#define CVI_IVE_IOC_DUMP _IO(CVI_IVE_IOC_MAGIC, 0xF1)
#define CVI_IVE_IOC_QUERY _IOWR(CVI_IVE_IOC_MAGIC, 0xF2, unsigned long long)
#endif /* __CVI_IVE_UAPI_H__ */

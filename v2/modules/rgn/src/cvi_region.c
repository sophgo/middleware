#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/queue.h>
#include <pthread.h>
#include <stdatomic.h>
#include <inttypes.h>

#include <fcntl.h>		/* low-level i/o */
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "cvi_base.h"
#include <cvi_math.h>
#include "cvi_sys.h"
#include "cvi_vpss.h"
#include "cvi_vo.h"
#include "cvi_region.h"
#include "cvi_venc.h"
#include "hashmap.h"
#include "rgn_ioctl.h"
#include "cvi_comm_osdc.h"
#include "cvi_osdc.h"

struct rgn_canvas {
	STAILQ_ENTRY(rgn_canvas) stailq;
	RGN_HANDLE Handle;
	CVI_U64 u64PhyAddr;
	CVI_U8 *pu8VirtAddr;
	CVI_U32 u32Size;
};

static pthread_once_t once = PTHREAD_ONCE_INIT;
static pthread_mutex_t canvas_q_lock = PTHREAD_MUTEX_INITIALIZER;
STAILQ_HEAD(rgn_canvas_q, rgn_canvas) canvas_q;

static CVI_S32 rgn_fd = -1;
static pthread_mutex_t rgn_fd_lock = PTHREAD_MUTEX_INITIALIZER;
static CVI_U32	abGrpEnable[VPSS_MAX_GRP_NUM] = {999};



CVI_S32 get_rgn_fd(CVI_VOID)
{
	pthread_mutex_lock(&rgn_fd_lock);
	if (rgn_fd <= 0) {
		if (open_device(RGN_DEV_NAME, &rgn_fd) == -1) {
			perror("RGN open fail\n");
			rgn_fd = -1;
		}
	}
	pthread_mutex_unlock(&rgn_fd_lock);

	return rgn_fd;
}

static void rgn_init(void)
{
	STAILQ_INIT(&canvas_q);
}

/**************************************************************************
 *   Public APIs.
 **************************************************************************/
CVI_S32 CVI_RGN_Create(RGN_HANDLE Handle, const RGN_ATTR_S *pstRegion)
{
	CVI_S32 fd = -1, s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstRegion);

	// Driver control
	fd = get_rgn_fd();
	s32Ret = rgn_create(fd, Handle, pstRegion);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Create RGN fail.\n");
		return s32Ret;
	}

	pthread_mutex_lock(&canvas_q_lock);
	pthread_once(&once, rgn_init);
	pthread_mutex_unlock(&canvas_q_lock);

	return CVI_SUCCESS;
}

CVI_S32 CVI_RGN_Destroy(RGN_HANDLE Handle)
{
	CVI_S32 fd = -1, s32Ret;

	// Driver control
	fd = get_rgn_fd();
	s32Ret = rgn_destroy(fd, Handle);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Destroy RGN fail.\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static CVI_S32 _cvi_rgn_get_ion_len(CVI_S32 fd, RGN_HANDLE Handle, CVI_S32 *pS32Len)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pS32Len);
	s32Ret = rgn_get_ion_len(fd, Handle, pS32Len);

	return s32Ret;
}

CVI_S32 CVI_RGN_GetAttr(RGN_HANDLE Handle, RGN_ATTR_S *pstRegion)
{
	CVI_S32 fd = -1, s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstRegion);

	// Driver control
	fd = get_rgn_fd();
	s32Ret = rgn_get_attr(fd, Handle, pstRegion);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Get RGN attributes fail.\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_RGN_SetAttr(RGN_HANDLE Handle, const RGN_ATTR_S *pstRegion)
{
	CVI_S32 fd = -1, s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstRegion);
	// Driver control
	fd = get_rgn_fd();
	s32Ret = rgn_set_attr(fd, Handle, pstRegion);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Set RGN attributes fail.\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_RGN_SetBitMap(RGN_HANDLE Handle, const BITMAP_S *pstBitmap)
{
	CVI_S32 fd = -1, s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstBitmap);
	// Driver control
	fd = get_rgn_fd();
	s32Ret = rgn_set_bit_map(fd, Handle, pstBitmap);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Set RGN Bitmap fail.\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_RGN_AttachToChn(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, const RGN_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 fd = -1, s32Ret;
	VPSS_GRP	VpssGrp = 0;
	VPSS_GRP_ATTR_S	stVpssGrpAttr = {0};
	VPSS_CHN_ATTR_S	stVpssChnAttr = {0};
	PIXEL_FORMAT_E	pixelFormat = PIXEL_FORMAT_NV21;
	SIZE_S stSize = {1920, 1080};
	MMF_CHN_S stChn = {0};

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChn);
	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChnAttr);

	if (pstChn->enModId != CVI_ID_JPEGE && pstChn->enModId != CVI_ID_VPSS && pstChn->enModId != CVI_ID_VO) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Unsupported module attach to RGN!\n");
		return CVI_ERR_RGN_ILLEGAL_PARAM;
	}

#ifdef __CV180X__
	if (pstChn->enModId == CVI_ID_VO) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "No vo device, cannot attach to vo!\n");
		return CVI_ERR_RGN_ILLEGAL_PARAM;
	}
#endif
	if (pstChn->enModId == CVI_ID_JPEGE) {
		stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
		stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
		stVpssGrpAttr.enPixelFormat		     = pixelFormat;
		stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
		stVpssGrpAttr.u32MaxH			     = stSize.u32Height;

		stVpssChnAttr.u32Width		    = stSize.u32Width;
		stVpssChnAttr.u32Height		    = stSize.u32Height;
		stVpssChnAttr.enVideoFormat		    = VIDEO_FORMAT_LINEAR;
		stVpssChnAttr.enPixelFormat		    = pixelFormat;
		stVpssChnAttr.stFrameRate.s32SrcFrameRate = 30;
		stVpssChnAttr.stFrameRate.s32DstFrameRate = 30;
		stVpssChnAttr.u32Depth		    = 1;
		stVpssChnAttr.bMirror			    = CVI_FALSE;
		stVpssChnAttr.bFlip			    = CVI_FALSE;

		/*start vpss*/
		VpssGrp = CVI_VPSS_GetAvailableGrp();

		stChn.enModId = CVI_ID_VPSS;
		stChn.s32DevId = VpssGrp;
		stChn.s32ChnId = 0;

		abGrpEnable[VpssGrp] = Handle;

		s32Ret = CVI_VPSS_CreateGrp(VpssGrp,  &stVpssGrpAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
			return CVI_FAILURE;
		}

		s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, 0, &stVpssChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
			return CVI_FAILURE;
		}

		s32Ret = CVI_VPSS_EnableChn(VpssGrp, 0);

		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
			return CVI_FAILURE;
		}

		s32Ret = CVI_VPSS_StartGrp(VpssGrp);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}

		CVI_VENC_SetRgnAttachInfo(pstChn->s32ChnId, stChn.s32DevId, stChn.s32ChnId, CVI_TRUE);
	} else {
		stChn.enModId = pstChn->enModId;
		stChn.s32DevId = pstChn->s32DevId;
		stChn.s32ChnId = pstChn->s32ChnId;
	}

	// Driver control
	fd = get_rgn_fd();
	s32Ret = rgn_attach_to_chn(fd, Handle, &stChn, pstChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Attach RGN to channel fail.\n");
		return s32Ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_RGN_DetachFromChn(RGN_HANDLE Handle, const MMF_CHN_S *pstChn)
{
	CVI_S32 fd = -1, s32Ret;
	MMF_CHN_S stChn = {0};

#ifdef __CV180X__
	if (pstChn->enModId == CVI_ID_VO) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "No vo device, cannot detach from vo!\n");
		return CVI_ERR_RGN_ILLEGAL_PARAM;
	}
#endif

	if (pstChn->enModId != CVI_ID_JPEGE && pstChn->enModId != CVI_ID_VPSS && pstChn->enModId != CVI_ID_VO) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Unsupported module attach to RGN!\n");
		return CVI_ERR_RGN_ILLEGAL_PARAM;
	}

	if (pstChn->enModId == CVI_ID_JPEGE) {
		for (CVI_S32 i = 0; i < VPSS_MAX_GRP_NUM; i++) {
			if (abGrpEnable[i] == Handle) {
				stChn.enModId = CVI_ID_VPSS;
				stChn.s32DevId = i;
				stChn.s32ChnId = 0;
				abGrpEnable[i] = 999;
				break;
			}
		}
	} else {
		stChn.enModId = pstChn->enModId;
		stChn.s32DevId = pstChn->s32DevId;
		stChn.s32ChnId = pstChn->s32ChnId;
	}

	// Driver control
	fd = get_rgn_fd();
	s32Ret = rgn_detach_from_chn(fd, Handle, &stChn);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Detach RGN from channel fail.\n");
		return s32Ret;
	}

	if (pstChn->enModId == CVI_ID_JPEGE) {
		CVI_VENC_SetRgnAttachInfo(pstChn->s32ChnId, stChn.s32DevId, stChn.s32ChnId, CVI_FALSE);

		s32Ret = CVI_VPSS_DisableChn(stChn.s32DevId, stChn.s32ChnId);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "Vpss stop Grp %d channel %d failed! Please check param\n",
			stChn.s32DevId, stChn.s32ChnId);
			return CVI_FAILURE;
		}


		s32Ret = CVI_VPSS_StopGrp(stChn.s32DevId);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "Vpss Stop Grp %d failed! Please check param\n", stChn.s32DevId);
			return CVI_FAILURE;
		}

		s32Ret = CVI_VPSS_DestroyGrp(stChn.s32DevId);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "Vpss Destroy Grp %d failed! Please check\n", stChn.s32DevId);
			return CVI_FAILURE;
		}
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_RGN_SetDisplayAttr(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, const RGN_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 fd = -1, s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChn);
	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChnAttr);

	// Driver control
	fd = get_rgn_fd();
	s32Ret = rgn_set_display_attr(fd, Handle, pstChn, pstChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Set display RGN attributes fail.\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_RGN_GetDisplayAttr(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, RGN_CHN_ATTR_S *pstChnAttr)
{
	CVI_S32 fd = -1, s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChn);
	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChnAttr);

	// Driver control
	fd = get_rgn_fd();
	s32Ret = rgn_get_display_attr(fd, Handle, pstChn, pstChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Get display RGN attributes fail.\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_RGN_GetCanvasInfo(RGN_HANDLE Handle, RGN_CANVAS_INFO_S *pstCanvasInfo)
{
	CVI_S32 fd = -1, s32Ret, s32IonLen;
	struct rgn_canvas *canvas;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstCanvasInfo);

	// Driver control
	fd = get_rgn_fd();

	s32Ret = _cvi_rgn_get_ion_len(fd, Handle, &s32IonLen);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_WARN, "Get RGN ion length fail.\n");
		return s32Ret;
	}

	s32Ret = rgn_get_canvas_info(fd, Handle, pstCanvasInfo);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Get RGN canvas information fail.\n");
		return s32Ret;
	}

	canvas = calloc(1, sizeof(struct rgn_canvas));
	if (!canvas) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "malloc failed.\n");
		return CVI_ERR_RGN_NOMEM;
	}
	pthread_mutex_lock(&canvas_q_lock);
	canvas->u64PhyAddr = pstCanvasInfo->u64PhyAddr;
	canvas->u32Size = s32IonLen;
	pstCanvasInfo->pu8VirtAddr = canvas->pu8VirtAddr =
			CVI_SYS_MmapCache(pstCanvasInfo->u64PhyAddr, canvas->u32Size);
	if (pstCanvasInfo->pu8VirtAddr == NULL) {
		free(canvas);
		CVI_TRACE_RGN(CVI_DBG_ERR, "CVI_SYS_MmapCache NG.\n");
		return CVI_FAILURE;
	}
	canvas->Handle = Handle;
	pstCanvasInfo->pstCanvasCmprAttr = (RGN_CANVAS_CMPR_ATTR_S *)pstCanvasInfo->pu8VirtAddr;
	pstCanvasInfo->pstObjAttr = (RGN_CMPR_OBJ_ATTR_S *)(pstCanvasInfo->pu8VirtAddr +
			sizeof(RGN_CANVAS_CMPR_ATTR_S));
	STAILQ_INSERT_TAIL(&canvas_q, canvas, stailq);
	pthread_mutex_unlock(&canvas_q_lock);

	return CVI_SUCCESS;
}

struct cvi_rgn_bitmap {
	CVI_VOID *pBitmapVAddr;
	CVI_U32 u32BitmapSize;
};
CVI_S32 CVI_RGN_UpdateCanvas(RGN_HANDLE Handle)
{
	CVI_S32 fd = -1, s32Ret, s32IonLen;
	struct rgn_canvas *canvas;
	RGN_ATTR_S stRegion;
	struct cvi_rgn_bitmap *pstBitmaps;
	CVI_U32 u32Bpp, i = 0, j = 0;

	// Driver control
	fd = get_rgn_fd();

	s32Ret = _cvi_rgn_get_ion_len(fd, Handle, &s32IonLen);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_WARN, "Get RGN ion length fail.\n");
		return s32Ret;
	}

	s32Ret = rgn_get_attr(fd, Handle, &stRegion);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Get RGN attr fail.\n");
		return s32Ret;
	}
	if (stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode == OSD_COMPRESS_MODE_HW) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Not support OSD_COMPRESS_MODE_HW yet.\n");
		return CVI_ERR_RGN_ILLEGAL_PARAM;
	}

	pthread_mutex_lock(&canvas_q_lock);
	if (!STAILQ_EMPTY(&canvas_q)) {
		STAILQ_FOREACH(canvas, &canvas_q, stailq) {
			if (canvas->Handle == Handle) {
				break;
			}
		}
	} else {
		CVI_TRACE_RGN(CVI_DBG_ERR, "No corresponding Handle(%d) found.\n", Handle);
		pthread_mutex_unlock(&canvas_q_lock);
		return CVI_ERR_RGN_ILLEGAL_PARAM;
	}
	pthread_mutex_unlock(&canvas_q_lock);

	if (stRegion.unAttr.stOverlay.stCompressInfo.enOSDCompressMode == OSD_COMPRESS_MODE_SW) {
		RGN_CANVAS_CMPR_ATTR_S *pstCanvasCmprAttr = NULL;
		RGN_CMPR_OBJ_ATTR_S *pstObjAttr = NULL;
		CVI_S32 status;
		OSDC_Canvas_Attr_S osdc_canvas;
		OSDC_DRAW_OBJ_S *obj_vec;
		CVI_U32 bs_size;

		//cmpr canvas info is stored in ion which needs to be flush before reading
		CVI_SYS_IonFlushCache(canvas->u64PhyAddr,
			canvas->pu8VirtAddr, canvas->u32Size);
		pstCanvasCmprAttr = (RGN_CANVAS_CMPR_ATTR_S *)canvas->pu8VirtAddr;
		pstObjAttr = (RGN_CMPR_OBJ_ATTR_S *)(canvas->pu8VirtAddr +
			sizeof(RGN_CANVAS_CMPR_ATTR_S));

		for (i = 0; i < pstCanvasCmprAttr->u32ObjNum; ++i) {
			if (pstObjAttr[i].enObjType == RGN_CMPR_LINE) {
				CVI_TRACE_RGN(CVI_DBG_DEBUG, "start(%d %d) end(%d %d) Thick(%d) Color(0x%x)\n",
					pstObjAttr[i].stLine.stPointStart.s32X,
					pstObjAttr[i].stLine.stPointStart.s32Y,
					pstObjAttr[i].stLine.stPointEnd.s32X,
					pstObjAttr[i].stLine.stPointEnd.s32Y,
					pstObjAttr[i].stLine.u32Thick,
					pstObjAttr[i].stLine.u32Color);
			} else if (pstObjAttr[i].enObjType == RGN_CMPR_RECT) {
				CVI_TRACE_RGN(CVI_DBG_DEBUG,
					"xywh(%d %d %d %d) Thick(%d) Color(0x%x) is_fill(%d)\n",
					pstObjAttr[i].stRgnRect.stRect.s32X,
					pstObjAttr[i].stRgnRect.stRect.s32Y,
					pstObjAttr[i].stRgnRect.stRect.u32Width,
					pstObjAttr[i].stRgnRect.stRect.u32Height,
					pstObjAttr[i].stRgnRect.u32Thick,
					pstObjAttr[i].stRgnRect.u32Color,
					pstObjAttr[i].stRgnRect.u32IsFill);
			} else if (pstObjAttr[i].enObjType == RGN_CMPR_BIT_MAP) {
				CVI_TRACE_RGN(CVI_DBG_DEBUG, "xywh(%d %d %d %d) u64BitmapPAddr(%lx)\n",
					pstObjAttr[i].stBitmap.stRect.s32X,
					pstObjAttr[i].stBitmap.stRect.s32Y,
					pstObjAttr[i].stBitmap.stRect.u32Width,
					pstObjAttr[i].stBitmap.stRect.u32Height,
					pstObjAttr[i].stBitmap.u64BitmapPAddr);
			}
		}

		osdc_canvas.width = stRegion.unAttr.stOverlay.stSize.u32Width;
		osdc_canvas.height = stRegion.unAttr.stOverlay.stSize.u32Height;
		osdc_canvas.bg_color_code = stRegion.unAttr.stOverlay.u32BgColor;
		switch (stRegion.unAttr.stOverlay.enPixelFormat) {
		case PIXEL_FORMAT_ARGB_8888:
			osdc_canvas.format = OSD_ARGB8888;
			u32Bpp = 4;
			break;

		case PIXEL_FORMAT_ARGB_4444:
			osdc_canvas.format = OSD_ARGB4444;
			u32Bpp = 2;
			break;

		case PIXEL_FORMAT_ARGB_1555:
			osdc_canvas.format = OSD_ARGB1555;
			u32Bpp = 2;
			break;

		case PIXEL_FORMAT_8BIT_MODE:
			osdc_canvas.format = OSD_LUT8;
			u32Bpp = 1;
			break;

		default:
			osdc_canvas.format = OSD_ARGB1555;
			u32Bpp = 2;
			break;
		}

		obj_vec = calloc(sizeof(OSDC_DRAW_OBJ_S) * pstCanvasCmprAttr->u32ObjNum, 1);
		if (!obj_vec) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "calloc size (%zu) failed!\n",
							sizeof(OSDC_DRAW_OBJ_S) * pstCanvasCmprAttr->u32ObjNum);
			return CVI_ERR_RGN_NOBUF;
		}
		pstBitmaps = (struct cvi_rgn_bitmap *)calloc(pstCanvasCmprAttr->u32ObjNum,
						sizeof(struct cvi_rgn_bitmap));
		if (!pstBitmaps) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "calloc size (%zu) failed!\n",
							pstCanvasCmprAttr->u32ObjNum * sizeof(struct cvi_rgn_bitmap));
			free(obj_vec);
			return CVI_ERR_RGN_NOBUF;
		}

		for (i = 0; i < pstCanvasCmprAttr->u32ObjNum; ++i) {
			if (pstObjAttr[i].enObjType == RGN_CMPR_LINE) {
				CVI_OSDC_SetLineObjAttr(&osdc_canvas, &obj_vec[i],
				pstObjAttr[i].stLine.u32Color,
					pstObjAttr[i].stLine.stPointStart.s32X,
					pstObjAttr[i].stLine.stPointStart.s32Y,
					pstObjAttr[i].stLine.stPointEnd.s32X,
					pstObjAttr[i].stLine.stPointEnd.s32Y,
					pstObjAttr[i].stLine.u32Thick);
			} else if (pstObjAttr[i].enObjType == RGN_CMPR_RECT) {
				CVI_OSDC_SetRectObjAttr(&osdc_canvas, &obj_vec[i],
					pstObjAttr[i].stRgnRect.u32Color,
					pstObjAttr[i].stRgnRect.stRect.s32X,
					pstObjAttr[i].stRgnRect.stRect.s32Y,
					pstObjAttr[i].stRgnRect.stRect.u32Width,
					pstObjAttr[i].stRgnRect.stRect.u32Height,
					pstObjAttr[i].stRgnRect.u32IsFill,
					pstObjAttr[i].stRgnRect.u32Thick);
			} else if (pstObjAttr[i].enObjType == RGN_CMPR_BIT_MAP) {
				pstBitmaps[j].u32BitmapSize = pstObjAttr[i].stBitmap.stRect.u32Width *
								pstObjAttr[i].stBitmap.stRect.u32Height * u32Bpp;
				pstBitmaps[j].pBitmapVAddr = CVI_SYS_MmapCache(pstObjAttr[i].stBitmap.u64BitmapPAddr,
								pstBitmaps[j].u32BitmapSize);

				CVI_OSDC_SetBitmapObjAttr(&osdc_canvas, &obj_vec[i],
					pstBitmaps[j++].pBitmapVAddr,
					pstObjAttr[i].stBitmap.stRect.s32X,
					pstObjAttr[i].stBitmap.stRect.s32Y,
					pstObjAttr[i].stBitmap.stRect.u32Width,
					pstObjAttr[i].stBitmap.stRect.u32Height,
					false);
			}
		}

		status = CVI_OSDC_DrawCmprCanvas(&osdc_canvas, &obj_vec[0], pstCanvasCmprAttr->u32ObjNum,
			canvas->pu8VirtAddr, s32IonLen,  &bs_size);
		if (status != 1) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "Region(%d) needs ion size(%d), current size(%d)!\n",
				Handle, bs_size, s32IonLen);
			CVI_OSDC_DrawCmprCanvas(&osdc_canvas, &obj_vec[0], 0,
				canvas->pu8VirtAddr, s32IonLen,  &bs_size);
		}
		if (j) {
			for (i = 0; i < j; i++) {
				CVI_SYS_Munmap(pstBitmaps[i].pBitmapVAddr, pstBitmaps[i].u32BitmapSize);
			}
		}
		free(pstBitmaps);
		// store bitstream size in bit[32:63], after driver gets it,
		// it should be restored to image width and height
		/*use ioctl instead*/
		// *((unsigned int *)canvas->pu8VirtAddr + 1) = bs_size;

		s32Ret = rgn_set_compress_size(fd, Handle, bs_size);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_RGN(CVI_DBG_ERR, "Set compress size fail.\n");
			pthread_mutex_unlock(&canvas_q_lock);
			return s32Ret;
		}

		free(obj_vec);
	}

	CVI_SYS_IonFlushCache(canvas->u64PhyAddr, canvas->pu8VirtAddr, canvas->u32Size);

	s32Ret = rgn_update_canvas(fd, Handle);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Update RGN canvas fail.\n");
		pthread_mutex_unlock(&canvas_q_lock);
		return s32Ret;
	}

	pthread_mutex_lock(&canvas_q_lock);
	if (!STAILQ_EMPTY(&canvas_q)) {
		STAILQ_FOREACH(canvas, &canvas_q, stailq) {
			if (canvas->Handle == Handle) {
				STAILQ_REMOVE(&canvas_q, canvas, rgn_canvas, stailq);
				CVI_SYS_Munmap(canvas->pu8VirtAddr, canvas->u32Size);
				free(canvas);
				break;
			}
		}
	}
	pthread_mutex_unlock(&canvas_q_lock);

	return CVI_SUCCESS;
}

/* CVI_RGN_Invert_Color - invert color per luma statistics of video content
 *   Chns' pixel-format should be YUV.
 *   RGN's pixel-format should be ARGB1555.
 *
 * @param Handle: RGN Handle
 * @param pstChn: the chn which rgn attached
 * @param pu32Color: rgn's content
 */
CVI_S32 CVI_RGN_Invert_Color(RGN_HANDLE Handle, MMF_CHN_S *pstChn, CVI_U32 *pu32Color)
{
	CVI_S32 fd = -1, s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChn);
	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pu32Color);

	// Driver control
	fd = get_rgn_fd();
	s32Ret = rgn_invert_color(fd, Handle, pstChn, (void *)pu32Color);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Invert RGN color fail.\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_RGN_SetChnPalette(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, RGN_PALETTE_S *pstPalette)
{
	CVI_S32 fd = -1, s32Ret;

	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstChn);
	MOD_CHECK_NULL_PTR(CVI_ID_RGN, pstPalette);

	// Driver control
	fd = get_rgn_fd();

	s32Ret = rgn_set_chn_palette(fd, Handle, pstChn, pstPalette);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_RGN(CVI_DBG_ERR, "Set chn palette fail.\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

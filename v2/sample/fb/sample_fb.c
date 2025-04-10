#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <linux/fb.h>

#include "sample_comm.h"
#include "cvi_tde.h"
#include "loadbmp.h"


#define TEST_CNT	20

#define FB_RED_1555		0xFC00
#define FB_RED_8888		0xFFff0000

#define GRAPHICS_LAYER_G0	0
#define GRAPHICS_LAYER_G1	1

#define SAMPLE_IMAGE_ARGB8888_PATH		"./res/640_360_argb8888.bin"


static struct fb_bitfield s_r16 = {10, 5, 0};
static struct fb_bitfield s_g16 = {5, 5, 0};
static struct fb_bitfield s_b16 = {0, 5, 0};
static struct fb_bitfield s_a16 = {15, 1, 0};

static struct fb_bitfield s_a32 = {24, 8, 0};
static struct fb_bitfield s_r32 = {16, 8, 0};
static struct fb_bitfield s_g32 = {8,  8, 0};
static struct fb_bitfield s_b32 = {0,  8, 0};

static CVI_CHAR gs_cExitFlag;

pthread_t g_stFbThread;

typedef enum _SAMPLE_VO_INTF_TYPE_E {
	SAMPLE_VO_INTF_MIPI = 0,
	SAMPLE_VO_INTF_LVDS,
	SAMPLE_VO_INTF_HDMI,
	SAMPLE_VO_INTF_BT656,
	SAMPLE_VO_INTF_BT1120,
	SAMPLE_VO_INTF_BUTT
} SAMPLE_VO_INTF_TYPE_E;

typedef enum _SAMPLE_VO_INTF_SYNC_E {
	SAMPLE_VO_OUTPUT_720x1280_60 = 0,
	SAMPLE_VO_OUTPUT_720P60,
	SAMPLE_VO_OUTPUT_1080P30,
	SAMPLE_VO_OUTPUT_1080P60,
	SAMPLE_VO_OUTPUT_2160P30,
	SAMPLE_VO_OUTPUT_2160P60,
	SAMPLE_VO_OUTPUT_4096x2160P30,
	SAMPLE_VO_OUTPUT_4096x2160P60,
	SAMPLE_VO_OUTPUT_BUTT
} SAMPLE_VO_INTF_SYNC_E;

typedef struct PTHREAD_FB_SAMPLE {
	CVI_S32			fd;
	CVI_S32			layer;
	CVI_U32			width;
	CVI_U32			height;
	CVI_BOOL		bEnableTDE;
	CVI_BOOL		bCompress;
	OSD_COLOR_FMT_E enColorFmt;
} PTHREAD_FB_SAMPLE_INFO;

typedef struct _SAMPLE_TDE_INFO {
	cvi_tde_color_format	enSrcClrFmt;
	cvi_tde_color_format	enDstClrFmt;
	CVI_U32					u32SrcWidth;
	CVI_U32					u32SrcHeight;
	CVI_U32					u32SrcStride;
	CVI_U64					u64SrcPhyaddr;
	CVI_S32					s32DstPosX;
	CVI_S32					s32DstPosY;
	CVI_U32					u32DstWidth;
	CVI_U32					u32DstHeight;
	CVI_U32					u32DstStride;
	CVI_U64					u64DstPhyaddr;
} SAMPLE_TDE_INFO;

VO_INTF_TYPE_E IntfType_Transform(SAMPLE_VO_INTF_TYPE_E enVoIntfType)
{
	switch (enVoIntfType) {
	case SAMPLE_VO_INTF_MIPI:
		return VO_INTF_MIPI;
	case SAMPLE_VO_INTF_LVDS:
		return VO_INTF_LVDS;
	case SAMPLE_VO_INTF_HDMI:
		return VO_INTF_HDMI;
	case SAMPLE_VO_INTF_BT656:
		return VO_INTF_BT656;
	case SAMPLE_VO_INTF_BT1120:
		return VO_INTF_BT1120;
	default:
		return VO_INTF_HDMI;
	}

	return VO_INTF_HDMI;
}

VO_INTF_SYNC_E IntfSync_Transform(SAMPLE_VO_INTF_SYNC_E enIntfSync)
{
	switch (enIntfSync) {
	case SAMPLE_VO_OUTPUT_720x1280_60:
		return VO_OUTPUT_720x1280_60;
	case SAMPLE_VO_OUTPUT_720P60:
		return VO_OUTPUT_720P60;
	case SAMPLE_VO_OUTPUT_1080P30:
		return VO_OUTPUT_1080P30;
	case SAMPLE_VO_OUTPUT_1080P60:
		return VO_OUTPUT_1080P60;
	case SAMPLE_VO_OUTPUT_2160P30:
		return VO_OUTPUT_2160P30;
	case SAMPLE_VO_OUTPUT_2160P60:
		return VO_OUTPUT_2160P60;
	case SAMPLE_VO_OUTPUT_4096x2160P30:
		return VO_OUTPUT_4096x2160P30;
	case SAMPLE_VO_OUTPUT_4096x2160P60:
		return VO_OUTPUT_4096x2160P60;
	default:
		return VO_OUTPUT_1080P60;
	}

	return VO_OUTPUT_1080P60;
}

CVI_VOID convertARGB8888ToARGB1555(CVI_U8 *input, CVI_U16 *output, CVI_S32 width, CVI_S32 height)
{
	for (CVI_S32 y = 0; y < height; y++) {
		for (CVI_S32 x = 0; x < width; x++) {
			// 获取ARGB8888格式的像素
			CVI_U8 a = input[(y * width + x) * 4 + 0]; // Alpha
			CVI_U8 r = input[(y * width + x) * 4 + 1]; // Red
			CVI_U8 g = input[(y * width + x) * 4 + 2]; // Green
			CVI_U8 b = input[(y * width + x) * 4 + 3]; // Blue

			// 转换为ARGB1555格式
			CVI_U16 a1555 = (a > 0) ? 0x8000 : 0; // Alpha bit (1 bit)
			CVI_U16 r1555 = (r >> 3) & 0x1F;	  // Red (5 bits)
			CVI_U16 g1555 = (g >> 3) & 0x1F;	  // Green (5 bits)
			CVI_U16 b1555 = (b >> 3) & 0x1F;	  // Blue (5 bits)

			// 合并成一个16位的ARGB1555像素
			output[y * width + x] = a1555 | (r1555 << 10) | (g1555 << 5) | b1555;
		}
	}
}

CVI_S32 load_image(CVI_CHAR *pFileName, CVI_U32 u32ImageWidth, CVI_U32 u32ImageHeigth,
		OSD_COLOR_FMT_E enColorFmt, CVI_VOID *Viraddr)
{
	FILE *file;
	CVI_U32 u32Len;
	CVI_VOID *p, *buf = NULL;

	u32Len = u32ImageWidth * u32ImageHeigth * 4;

	file = fopen(pFileName, "rb");
	if (!file) {
		SAMPLE_PRT("open file:%s failed!\n", pFileName);
		return CVI_FAILURE;
	}

	if (enColorFmt == OSD_COLOR_FMT_RGB8888) {
		p = Viraddr;
	} else if (enColorFmt == OSD_COLOR_FMT_RGB1555) {
		buf = malloc(u32Len);
		if (!buf) {
			SAMPLE_PRT("malloc failed, size:%d!\n", u32Len);
			fclose(file);
			return CVI_FAILURE;
		}
		p = buf;
	} else {
		SAMPLE_PRT("Not support!\n");
		fclose(file);
		return CVI_FAILURE;
	}

	fread(p, u32Len, 1, file);
	fclose(file);

	if (enColorFmt == OSD_COLOR_FMT_RGB1555) {
		convertARGB8888ToARGB1555((CVI_U8 *)p, Viraddr, u32ImageWidth, u32ImageHeigth);
		free(buf);
	}

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_FB_VO_GetDefConfig(SAMPLE_VO_CONFIG_S *pstVoConfig, CVI_S32 s32VoDev,
	CVI_S32 s32IntfType, CVI_S32 s32IntfSync)
{
	RECT_S stDefDispRect = {0, 0, 1920, 1080};
	SIZE_S stDefImageSize = {1920, 1080};
	CVI_U32 u32W, u32H, u32Frm;


	if (pstVoConfig == NULL) {
		SAMPLE_PRT("Error:argument can not be NULL\n");
		return CVI_FAILURE;
	}

	pstVoConfig->VoDev			= s32VoDev;
	pstVoConfig->stVoPubAttr.enIntfType = IntfType_Transform(s32IntfType);
	pstVoConfig->stVoPubAttr.enIntfSync = IntfSync_Transform(s32IntfSync);

	SAMPLE_COMM_VO_GetWH(pstVoConfig->stVoPubAttr.enIntfSync, &u32W, &u32H, &u32Frm);
	SAMPLE_PRT("u32W:%d u32H:%d\n", u32W, u32H);

	stDefDispRect.u32Width = u32W;
	stDefDispRect.u32Height = u32H;
	stDefImageSize.u32Width = u32W;
	stDefImageSize.u32Height = u32H;

	pstVoConfig->stDispRect	= stDefDispRect;
	pstVoConfig->stImageSize	= stDefImageSize;
	pstVoConfig->enPixFormat	= PIXEL_FORMAT_NV21;
	pstVoConfig->stVoPubAttr.u32BgColor = COLOR_10_RGB_BLUE;
	pstVoConfig->u32DisBufLen	= 3;
	pstVoConfig->enVoMode		= VO_MODE_1MUX;

	return CVI_SUCCESS;
}

CVI_VOID SAMPLE_FB_TO_EXIT(CVI_VOID)
{
	CVI_CHAR ch = 0;

	while (1) {
		printf("\npress 'q' to exit this sample.\n");
		while (ch == '\n') {
			ch = (char)getchar();
		};

		getchar();
		if ('q' == ch) {
			gs_cExitFlag = ch;
			break;
		} else {
			printf("input invaild! please try again.\n");
		}
	}

	if (g_stFbThread != 0) {
		pthread_join(g_stFbThread, 0);
		g_stFbThread = 0;
	}
}

CVI_S32 SAMPLE_TDE_Proc(SAMPLE_TDE_INFO *pstTdeInfo)
{
	CVI_S32					s32Ret = CVI_SUCCESS;
	CVI_S32					s32Handle;
	cvi_tde_single_src		single_src;
	cvi_tde_rect			stSrcRect, stDstRect;
	cvi_tde_surface			stSrcSurface, stDstSurface;
	cvi_tde_none_src		none_src = {0};
	CVI_U32					fill_data = 0xC1CDC1;
	cvi_tde_rect			fill_dst_rect;
	cvi_tde_line			lines[] = {
		{0, 64, 256, 64, 3, 0xffff00},
		{0, 128, 256, 128, 3, 0xffff00},
		{0, 192, 256, 192, 3, 0xff00},
		{64, 0, 64, 256, 3, 0xffff00},
		{128, 0, 128, 256, 3, 0xffff00},
		{192, 0, 192, 256, 3, 0xff00},
		{128, 0, 256, 128, 3, 0xff},
		{0, 128, 128, 256, 3, 0xff},
		{128, 0, 0, 128, 3, 0xff},
		{256, 128, 128, 256, 3, 0xff},
		{0, 0, 256, 256, 3, 0xff},
		{256, 0, 0, 256, 3, 0xff},
		{128, 0, 256, 192, 3, 0xff00},
		{128, 0, 256, 256, 3, 0xff00},
		{128, 0, 192, 128, 3, 0xff00},
		{128, 0, 192, 192, 3, 0xff00},
		{128, 0, 192, 256, 3, 0xff00},
		{128, 0, 0, 192, 3, 0xff00},
		{128, 0, 0, 256, 3, 0xff00},
		{128, 0, 64, 128, 3, 0xff00},
		{128, 0, 64, 192, 3, 0xff00},
		{128, 0, 64, 256, 3, 0xff00},
		{0, 0, 256, 64, 3, 0xff},
		{0, 0, 256, 128, 3, 0xff},
		{256, 0, 0, 64, 3, 0xff},
		{256, 0, 0, 128, 3, 0xff},
		{192, 0, 256, 64, 3, 0xff00},
		{192, 0, 256, 128, 3, 0xff00},
		{64, 0, 0, 64, 3, 0xff00},
		{64, 0, 0, 128, 3, 0xff00}
	};

	/* TDE job step 0. open tde */
	stSrcSurface.color_format	  = pstTdeInfo->enSrcClrFmt;
	stSrcSurface.width	 = pstTdeInfo->u32SrcWidth;
	stSrcSurface.height	 = pstTdeInfo->u32SrcHeight;
	stSrcSurface.stride	 = pstTdeInfo->u32SrcStride;
	stSrcSurface.phys_addr	  = pstTdeInfo->u64SrcPhyaddr;
	stSrcSurface.support_alpha_ex_1555 = CVI_TRUE;
	stSrcSurface.alpha_max_is_255	= CVI_TRUE;
	stSrcSurface.alpha0	  = 0XFF;
	stSrcSurface.alpha1	  = 0XFF;

	stSrcRect.pos_x	= 0;
	stSrcRect.pos_y	= 0;
	stSrcRect.width = pstTdeInfo->u32SrcWidth;
	stSrcRect.height	= pstTdeInfo->u32SrcHeight;

	stDstRect.pos_x = pstTdeInfo->s32DstPosX;
	stDstRect.pos_y = pstTdeInfo->s32DstPosY;
	stDstRect.width = stSrcRect.width;
	stDstRect.height = stSrcRect.height;

	stDstSurface.color_format	  = pstTdeInfo->enDstClrFmt;
	stDstSurface.width		= pstTdeInfo->u32DstWidth;
	stDstSurface.height	= pstTdeInfo->u32DstHeight;
	stDstSurface.stride	= pstTdeInfo->u32DstStride;
	stDstSurface.phys_addr	= pstTdeInfo->u64DstPhyaddr;
	stDstSurface.support_alpha_ex_1555 = CVI_TRUE;
	stDstSurface.alpha_max_is_255	= CVI_TRUE;
	stDstSurface.alpha0 = 0XFF;
	stDstSurface.alpha1 = 0XFF;

	/* TDE job step 1. start job */
	s32Handle = cvi_tde_begin_job();
	if (s32Handle == CVI_ERR_TDE_INVALID_HANDLE) {
		SAMPLE_PRT("start job failed!\n");
		return s32Ret;
	}

	fill_dst_rect.width = pstTdeInfo->u32DstWidth;
	fill_dst_rect.height = pstTdeInfo->u32DstHeight;
	fill_dst_rect.pos_x = 0;
	fill_dst_rect.pos_y = 0;
	none_src.dst_surface = &stDstSurface;
	none_src.dst_rect = &fill_dst_rect;
	s32Ret = cvi_tde_quick_fill(s32Handle, &none_src, fill_data);
	if (s32Ret < 0) {
		SAMPLE_PRT("cvi_tde_quick_fill failed,ret=0x%x!\n", s32Ret);
		cvi_tde_cancel_job(s32Handle);
		return s32Ret;
	}

	single_src.src_surface = &stSrcSurface;
	single_src.dst_surface = &stDstSurface;
	single_src.src_rect = &stSrcRect;
	single_src.dst_rect = &stDstRect;
	s32Ret = cvi_tde_quick_copy(s32Handle, &single_src);
	if (s32Ret < 0) {
		SAMPLE_PRT("cvi_tde_quick_copy failed,ret=0x%x!\n", s32Ret);
		cvi_tde_cancel_job(s32Handle);
		return s32Ret;
	}

	s32Ret = cvi_tde_draw_line(s32Handle, &stDstSurface, lines, ARRAY_SIZE(lines));
	if (s32Ret < 0) {
		SAMPLE_PRT("cvi_tde_draw_line failed,ret=0x%x!\n", s32Ret);
		cvi_tde_cancel_job(s32Handle);
		return s32Ret;
	}

	/* TDE job step 2. submit job */
	s32Ret = cvi_tde_end_job(s32Handle, CVI_FALSE, CVI_TRUE, 100);
	if (s32Ret < 0) {
		SAMPLE_PRT("cvi_tde_end_job failed,ret=0x%x!\n", s32Ret);
		cvi_tde_cancel_job(s32Handle);
		return s32Ret;
	}

	return s32Ret;
}

CVI_VOID *SAMPLE_FB_ONE_BUF_DISPLAY(void *pData)
{
	CVI_U32						i, x, y;
	CVI_U8						*pShowScreen;
	CVI_VOID					*pShowLine;
	CVI_VOID					*ptemp = NULL;
	CVI_CHAR					file[12] = {0};
	PTHREAD_FB_SAMPLE_INFO		*pstInfo;
	CVI_U32						u32Color = FB_RED_1555;
	CVI_CHAR					thdname[64];
	struct fb_fix_screeninfo	fix;
	struct fb_var_screeninfo	var;

	if (pData == CVI_NULL) {
		return CVI_NULL;
	}
	pstInfo = (PTHREAD_FB_SAMPLE_INFO *)pData;
	snprintf(thdname, sizeof(thdname), "FB%d_PANDISPLAY", pstInfo->layer);
	prctl(PR_SET_NAME, thdname, 0, 0, 0);

	switch (pstInfo->layer) {
	case GRAPHICS_LAYER_G0:
		strncpy(file, "/dev/fb0", 12);
		break;
	case GRAPHICS_LAYER_G1:
		strncpy(file, "/dev/fb1", 12);
		break;
	default:
		strncpy(file, "/dev/fb0", 12);
		break;
	}

	/********************************
	* Step 1. open framebuffer device overlay 0
	**********************************/
	pstInfo->fd = open(file, O_RDWR, 0);
	if (pstInfo->fd < 0) {
		SAMPLE_PRT("open %s failed!\n", file);
		return CVI_NULL;
	}

	/********************************
	* Step 2. get the variable screen information
	**********************************/
	if (ioctl(pstInfo->fd, FBIOGET_VSCREENINFO, &var) < 0) {
		SAMPLE_PRT("Get variable screen info failed!\n");
		close(pstInfo->fd);
		return CVI_NULL;
	}

	/* **********************************************************
	*Step 3. modify the variable screen info
	*			 the screen size: IMAGE_WIDTH*IMAGE_HEIGHT
	*			 the virtual screen size: VIR_SCREEN_WIDTH*VIR_SCREEN_HEIGHT
	*			 (which equals to VIR_SCREEN_WIDTH*(IMAGE_HEIGHT*2))
	*			 the pixel format: ARGB1555
	**************************************************************/
	SAMPLE_PRT("[Begin]\n");
	SAMPLE_PRT("wait 3 seconds\n");
	sleep(3);

	switch (pstInfo->enColorFmt) {
	case OSD_COLOR_FMT_RGB8888:
		var.transp = s_a32;
		var.red    = s_r32;
		var.green  = s_g32;
		var.blue   = s_b32;
		var.bits_per_pixel = 32;
		u32Color		 = FB_RED_8888;
		break;
	default:
		var.transp = s_a16;
		var.red    = s_r16;
		var.green  = s_g16;
		var.blue   = s_b16;
		var.bits_per_pixel = 16;
		u32Color		 = FB_RED_1555;
		break;
	}

	var.yoffset = 0;
	var.xres		 = pstInfo->width;
	var.yres		 = pstInfo->height;
	var.xres_virtual = pstInfo->width;
	var.yres_virtual = pstInfo->height;
	var.activate	 = FB_ACTIVATE_NOW;

	/*********************************
	* Step 4. set the variable screen information
	***********************************/
	if (ioctl(pstInfo->fd, FBIOPUT_VSCREENINFO, &var) < 0) {
		SAMPLE_PRT("Put variable screen info failed!\n");
		close(pstInfo->fd);
		return CVI_NULL;
	}

	/**********************************
	* Step 5. get the fix screen information
	************************************/
	if (ioctl(pstInfo->fd, FBIOGET_FSCREENINFO, &fix) < 0) {
		SAMPLE_PRT("Get fix screen info failed!\n");
		close(pstInfo->fd);
		return CVI_NULL;
	}

	 /***************************************
	* Step 6. map the physical video memory for user use
	******************************************/
	pShowScreen = mmap(CVI_NULL, fix.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, pstInfo->fd, 0);
	if (pShowScreen == MAP_FAILED) {
		SAMPLE_PRT("mmap framebuffer failed!\n");
		close(pstInfo->fd);
		return CVI_NULL;
	}

	for (i = 0; i < TEST_CNT; i++) {
		SAMPLE_PRT("Display background colorr!\n");
		memset(pShowScreen, 0x0, fix.smem_len);
		CVI_SYS_IonFlushCache(fix.smem_start, pShowScreen, fix.smem_len);
		sleep(2);

		SAMPLE_PRT("expected: the red box will appear!\n");
		ptemp = pShowScreen;
		for (y = 100; y < 300; y++) {
			for (x = 0; x < 300; x++) {
				if (pstInfo->enColorFmt == OSD_COLOR_FMT_RGB8888) {
					*((CVI_U32 *)ptemp + y * var.xres + x) = FB_RED_8888;
				} else {
					*((CVI_U16 *)ptemp + y * var.xres + x) = FB_RED_1555;
				}
			}
		}
		CVI_SYS_IonFlushCache(fix.smem_start, pShowScreen, fix.smem_len);
		sleep(2);

		SAMPLE_PRT("expected: two red line will appear!\n");
		pShowLine = pShowScreen;
		for (y = (pstInfo->height / 2 - 2); y < (pstInfo->height / 2 + 2); y++) {
			for (x = 0; x < pstInfo->width; x++) {
				if (pstInfo->enColorFmt == OSD_COLOR_FMT_RGB8888) {
					*((CVI_U32 *)pShowLine + y * var.xres + x) = u32Color; //todo:u32FixScreenStride
				} else {
					*((CVI_U16 *)pShowLine + y * var.xres + x) = u32Color;
				}
			}
		}
		for (y = 0; y < pstInfo->height; y++) {
			for (x = (pstInfo->width / 2 - 2); x < (pstInfo->width / 2 + 2); x++) {
				if (pstInfo->enColorFmt == OSD_COLOR_FMT_RGB8888) {
					*((CVI_U32 *)pShowLine + y * var.xres + x) = u32Color;
				} else {
					*((CVI_U16 *)pShowLine + y * var.xres + x) = u32Color;
				}
			}
		}
		CVI_SYS_IonFlushCache(fix.smem_start, pShowScreen, fix.smem_len);
		sleep(2);

		if ('q' == gs_cExitFlag) {
			printf("process exit...\n");
			break;
		}
	}

	/* unmap the physical memory */
	munmap(pShowScreen, fix.smem_len);
	close(pstInfo->fd);
	SAMPLE_PRT("[End]\n");

	return CVI_NULL;
}

CVI_VOID *SAMPLE_FB_DOUBLE_BUF_DISPLAY(void *pData)
{
	CVI_S32						s32Ret;
	CVI_U32						x, y;
	CVI_U8						*pShowScreen, *pHideScreen;
	CVI_U32						u32FixScreenStride;
	CVI_VOID					*ptemp = NULL;
	CVI_CHAR					fb_path[12] = {0};
	PTHREAD_FB_SAMPLE_INFO		*pstInfo;
	CVI_CHAR					thdname[64];
	CVI_S32						x_point = 0, y_point = 0, s32Step = 8;
	CVI_BOOL					xAdd = CVI_TRUE, yAdd = CVI_TRUE;
	struct fb_fix_screeninfo	fix;
	struct fb_var_screeninfo	var;
	SAMPLE_TDE_INFO				stTdeInfo;
	CVI_U64						u64HideScreenPhy;
	CVI_CHAR					*pFileName = SAMPLE_IMAGE_ARGB8888_PATH;
	CVI_U32						u32ImageWidth = 640, u32ImageHeigth = 360;
	cvi_tde_color_format		enTdeClrFmt;
	CVI_U32						u32BytePerPixel, u32IonLen;
	CVI_U64						u64Phyaddr = 0;
	CVI_VOID					*Viraddr = NULL;
	CVI_U8						*pu8Temp;
	CVI_U16						*pu16Temp;

	if (pData == CVI_NULL) {
		return CVI_NULL;
	}
	pstInfo = (PTHREAD_FB_SAMPLE_INFO *)pData;
	snprintf(thdname, sizeof(thdname), "FB%d_PANDISPLAY", pstInfo->layer);
	prctl(PR_SET_NAME, thdname, 0, 0, 0);


	switch (pstInfo->layer) {
	case GRAPHICS_LAYER_G0:
		strncpy(fb_path, "/dev/fb0", 12);
		break;
	case GRAPHICS_LAYER_G1:
		strncpy(fb_path, "/dev/fb1", 12);
		break;
	default:
		strncpy(fb_path, "/dev/fb0", 12);
		break;
	}

	/********************************
	* Step 1. open framebuffer device overlay 0
	**********************************/
	pstInfo->fd = open(fb_path, O_RDWR, 0);
	if (pstInfo->fd < 0) {
		SAMPLE_PRT("open %s failed!\n", fb_path);
		return CVI_NULL;
	}

	/********************************
	* Step 2. get the variable screen information
	**********************************/
	if (ioctl(pstInfo->fd, FBIOGET_VSCREENINFO, &var) < 0) {
		SAMPLE_PRT("Get variable screen info failed!\n");
		close(pstInfo->fd);
		return CVI_NULL;
	}

	/* **********************************************************
	*Step 3. modify the variable screen info
	*			 the screen size: IMAGE_WIDTH*IMAGE_HEIGHT
	*			 the virtual screen size: VIR_SCREEN_WIDTH*VIR_SCREEN_HEIGHT
	*			 (which equals to VIR_SCREEN_WIDTH*(IMAGE_HEIGHT*2))
	*			 the pixel format: ARGB1555
	**************************************************************/
	SAMPLE_PRT("[Begin]\n");
	SAMPLE_PRT("wait 3 seconds\n");
	sleep(3);

	switch (pstInfo->enColorFmt) {
	case OSD_COLOR_FMT_RGB8888:
		var.transp = s_a32;
		var.red    = s_r32;
		var.green  = s_g32;
		var.blue   = s_b32;
		var.bits_per_pixel = 32;
		enTdeClrFmt = CVI_TDE_COLOR_FORMAT_ARGB8888;
		break;
	default:
		var.transp = s_a16;
		var.red    = s_r16;
		var.green  = s_g16;
		var.blue   = s_b16;
		var.bits_per_pixel = 16;
		enTdeClrFmt = CVI_TDE_COLOR_FORMAT_ARGB1555;
		break;
	}
	u32BytePerPixel = var.bits_per_pixel / 8;
	var.yoffset = 0;
	var.xres_virtual = pstInfo->width;
	var.yres_virtual = pstInfo->height * 2;
	var.xres		 = pstInfo->width;
	var.yres		 = pstInfo->height;
	var.activate	   = FB_ACTIVATE_NOW;

	/*********************************
	* Step 4. set the variable screen information
	***********************************/
	if (ioctl(pstInfo->fd, FBIOPUT_VSCREENINFO, &var) < 0) {
		SAMPLE_PRT("Put variable screen info failed!\n");
		close(pstInfo->fd);
		return CVI_NULL;
	}

	/**********************************
	* Step 5. get the fix screen information
	************************************/
	if (ioctl(pstInfo->fd, FBIOGET_FSCREENINFO, &fix) < 0) {
		SAMPLE_PRT("Get fix screen info failed!\n");
		close(pstInfo->fd);
		return CVI_NULL;
	}
	u32FixScreenStride = fix.line_length;	/*fix screen stride*/
	 /***************************************
	* Step 6. map the physical video memory for user use
	******************************************/
	pShowScreen = mmap(CVI_NULL, fix.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, pstInfo->fd, 0);
	if (pShowScreen == MAP_FAILED) {
		SAMPLE_PRT("mmap framebuffer failed!\n");
		close(pstInfo->fd);
		return CVI_NULL;
	}
	memset(pShowScreen, 0x0, fix.smem_len);
	CVI_SYS_IonFlushCache(fix.smem_start, pShowScreen, fix.smem_len);

	u32IonLen = u32ImageWidth * u32ImageHeigth * u32BytePerPixel;
	s32Ret = CVI_SYS_IonAlloc(&u64Phyaddr, &Viraddr, "BmpBuf", u32IonLen);
	if (s32Ret) {
		SAMPLE_PRT("allocate memory (maxW*maxH*%d bytes) failed\n", u32BytePerPixel);
		munmap(pShowScreen, fix.smem_len);
		close(pstInfo->fd);
		return CVI_NULL;
	}

	s32Ret = load_image(pFileName, u32ImageWidth, u32ImageHeigth, pstInfo->enColorFmt, Viraddr);
	if (s32Ret) {
		SAMPLE_PRT("load_image %s failed\n", pFileName);
		CVI_SYS_IonFree(u64Phyaddr, Viraddr);
		munmap(pShowScreen, fix.smem_len);
		close(pstInfo->fd);
		return CVI_NULL;
	}

	CVI_SYS_IonFlushCache(u64Phyaddr, Viraddr, u32IonLen);

	if (pstInfo->bEnableTDE) {
		if (cvi_tde_open()) {
			SAMPLE_PRT("cvi_tde_open failed!\n");
			CVI_SYS_IonFree(u64Phyaddr, Viraddr);
			munmap(pShowScreen, fix.smem_len);
			close(pstInfo->fd);
			return CVI_NULL;
		}
	}

	/* show */
	while (1) {
		if ('q' == gs_cExitFlag) {
			printf("process exit...\n");
			break;
		}
		if ((x_point + u32ImageWidth) >= pstInfo->width) {
			x_point = pstInfo->width - u32ImageWidth;
			xAdd = CVI_FALSE;
		} else if (x_point <= 0) {
			xAdd = CVI_TRUE;
			x_point = 0;
		}
		if ((y_point + u32ImageHeigth) >= pstInfo->height) {
			yAdd = CVI_FALSE;
			y_point = pstInfo->height - u32ImageHeigth;
		} else if (y_point <= 0) {
			yAdd = CVI_TRUE;
			y_point = 0;
		}

		//double buffer
		if (var.yoffset) {
			var.yoffset = 0;
		} else {
			var.yoffset = var.yres;
		}
		pHideScreen = pShowScreen + (u32FixScreenStride * var.yoffset);
		u64HideScreenPhy = fix.smem_start + (u32FixScreenStride * var.yoffset);

		//TDE copy
		if (pstInfo->bEnableTDE) {
			//Little-endian,
			//The order of the files is: A R G B, But in memory it is: 0xBGRA
			stTdeInfo.enSrcClrFmt =
				(pstInfo->enColorFmt == OSD_COLOR_FMT_RGB8888) ?
				CVI_TDE_COLOR_FORMAT_BGRA8888 : enTdeClrFmt;
			stTdeInfo.u32SrcWidth = u32ImageWidth;
			stTdeInfo.u32SrcHeight = u32ImageHeigth;
			stTdeInfo.u32SrcStride = u32ImageWidth * u32BytePerPixel;
			stTdeInfo.u64SrcPhyaddr = u64Phyaddr;

			stTdeInfo.enDstClrFmt = enTdeClrFmt;
			stTdeInfo.s32DstPosX = x_point;
			stTdeInfo.s32DstPosY = y_point;
			stTdeInfo.u32DstWidth = pstInfo->width;
			stTdeInfo.u32DstHeight = pstInfo->height;
			stTdeInfo.u32DstStride = u32FixScreenStride;
			stTdeInfo.u64DstPhyaddr = u64HideScreenPhy;

			if (SAMPLE_TDE_Proc(&stTdeInfo)) {
				SAMPLE_PRT("SAMPLE_TDE_Proc failed!\n");
				CVI_SYS_IonFree(u64Phyaddr, Viraddr);
				munmap(pShowScreen, fix.smem_len);
				close(pstInfo->fd);
				return CVI_NULL;
			}
		} else {
		// CPU copy
			memset(pHideScreen, 0x00, u32FixScreenStride * var.yres);
			ptemp = pHideScreen;
			pu8Temp = (CVI_U8 *)Viraddr;
			pu16Temp = (CVI_U16 *)Viraddr;
			for (y = y_point; y < (y_point + u32ImageHeigth); y++) {
				for (x = x_point; x < (x_point + u32ImageWidth); x++) {
					//Little-endian
					if (pstInfo->enColorFmt == OSD_COLOR_FMT_RGB8888) {
						//Need to reverse order
						*((CVI_U8 *)ptemp + 4 * (y * var.xres + x) + 3) = *pu8Temp++;
						*((CVI_U8 *)ptemp + 4 * (y * var.xres + x) + 2) = *pu8Temp++;
						*((CVI_U8 *)ptemp + 4 * (y * var.xres + x) + 1) = *pu8Temp++;
						*((CVI_U8 *)ptemp + 4 * (y * var.xres + x)) = *pu8Temp++;
					} else {
						//Adjusted order in convertARGB8888ToARGB1555
						*((CVI_U16 *)ptemp + y * var.xres + x) = *pu16Temp++;
					}
				}
			}
		}

		if (ioctl(pstInfo->fd, FBIOPAN_DISPLAY, &var) < 0) {
			SAMPLE_PRT("FBIOPAN_DISPLAY failed!\n");
			munmap(pShowScreen, fix.smem_len);
			close(pstInfo->fd);
			return CVI_NULL;
		}
		x_point = xAdd ? (x_point + s32Step) : (x_point - s32Step);
		y_point = yAdd ? (y_point + s32Step) : (y_point - s32Step);
		sleep(1);
	}

	if (pstInfo->bEnableTDE) {
		cvi_tde_close();
	}

	CVI_SYS_IonFree(u64Phyaddr, Viraddr);
	munmap(pShowScreen, fix.smem_len);
	close(pstInfo->fd);
	SAMPLE_PRT("[End]\n");

	return CVI_NULL;
}

CVI_VOID SAMPLE_FB_HandleSig(CVI_S32 signo)
{
	static int sig_handled;

	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	if (!sig_handled && (SIGINT == signo || SIGTERM == signo)) {
		sig_handled = 1;
		gs_cExitFlag = 'q';

		if (g_stFbThread) {
			pthread_join(g_stFbThread, 0);
			g_stFbThread = 0;
		}

		SAMPLE_COMM_SYS_Exit();

		printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
	}

	exit(-1);
}

CVI_VOID SAMPLE_FB_Usage2(CVI_VOID)
{
	printf("\n\n/****************index******************/\n");
	printf("please choose the case which you want to run:\n");
	printf("\t0:  ARGB8888 ONE BUF mode\n");
	printf("\t1:  ARGB1555 DOUBLE BUF mode\n");
	printf("\t2:  ARGB8888 fb + TDE\n");
	printf("\n/****************VoDev******************/\n");
	printf("\t0:  VO's device HD0\n");
	printf("\t1:  VO's device HD1\n");
	printf("\n/****************IntfType******************/\n");
	printf("\t%d:  MIPI\n", SAMPLE_VO_INTF_MIPI);
	printf("\t%d:  LVDS\n", SAMPLE_VO_INTF_LVDS);
	printf("\t%d:  HDMI\n", SAMPLE_VO_INTF_HDMI);
	printf("\t%d:  BT656\n", SAMPLE_VO_INTF_BT656);
	printf("\t%d:  BT1120\n", SAMPLE_VO_INTF_BT1120);
	printf("\n/****************IntfSync******************/\n");
	printf("\t%d:  720x1280@60\n", SAMPLE_VO_OUTPUT_720x1280_60);
	printf("\t%d:  1280x720@60\n", SAMPLE_VO_OUTPUT_720P60);
	printf("\t%d:  1920x1080@30\n", SAMPLE_VO_OUTPUT_1080P30);
	printf("\t%d:  1920x1080@60\n", SAMPLE_VO_OUTPUT_1080P60);
	printf("\t%d:  3840x2160@30\n", SAMPLE_VO_OUTPUT_2160P30);
	printf("\t%d:  3840x2160@30\n", SAMPLE_VO_OUTPUT_2160P60);
	printf("\t%d:  4096x2160@30\n", SAMPLE_VO_OUTPUT_4096x2160P30);
	printf("\t%d:  4096x2160@60\n", SAMPLE_VO_OUTPUT_4096x2160P60);
}

CVI_VOID SAMPLE_FB_Usage1(CVI_CHAR *sPrgNm)
{
	printf("Usage : %s <index> <VoDev> <IntfType> <IntfSync>\n", sPrgNm);
	SAMPLE_FB_Usage2();
	printf("\nexample: %s 0 1 %d %d\n", sPrgNm, SAMPLE_VO_INTF_HDMI, SAMPLE_VO_OUTPUT_1080P60);
}

CVI_S32 SAMPLE_FB_OneBufMode(SAMPLE_VO_CONFIG_S *pstVoDevInfo)
{
	CVI_S32					s32Ret = CVI_SUCCESS;
	PTHREAD_FB_SAMPLE_INFO	stInfo0;
	VO_DEV					VoDev = pstVoDevInfo->VoDev;
	VB_CONFIG_S				stVbConfig;
	CVI_U32					u32BlkSize;

	/******************************************
	 step 1: mpp system init.
	******************************************/
	u32BlkSize = COMMON_GetPicBufferSize(pstVoDevInfo->stImageSize.u32Width, pstVoDevInfo->stImageSize.u32Height,
		pstVoDevInfo->enPixFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	memset(&stVbConfig, 0, sizeof(VB_CONFIG_S));
	stVbConfig.u32MaxPoolCnt = 1;
	stVbConfig.astCommPool[0].u32BlkCnt = 5;
	stVbConfig.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConfig.astCommPool[0].enRemapMode = VB_REMAP_MODE_CACHED;

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_SYS_Init failed with %d!\n", s32Ret);
		goto SAMPLE_FB_StandarMode_0;
	}

	/******************************************
	 step 2: Start VO device.
	 NOTE: Step 3 is optional when VO is running on other system.
	******************************************/
	s32Ret = SAMPLE_COMM_VO_StartVO(pstVoDevInfo);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VO_StartVO failed with %d!\n", s32Ret);
		goto SAMPLE_FB_StandarMode_0;
	}

	/******************************************
	 step 3:  start fb.
	*****************************************/
	stInfo0.layer	  =  VoDev;    /* VO device number */
	stInfo0.fd		  = -1;
	stInfo0.width	  = pstVoDevInfo->stImageSize.u32Width;
	stInfo0.height	   = pstVoDevInfo->stImageSize.u32Height;
	stInfo0.bEnableTDE = CVI_FALSE;
	stInfo0.bCompress =  CVI_FALSE; /* Compress opened or not */
	stInfo0.enColorFmt = OSD_COLOR_FMT_RGB8888;
	if (pthread_create(&g_stFbThread, 0, SAMPLE_FB_ONE_BUF_DISPLAY, (void *)(&stInfo0))) {
		SAMPLE_PRT("start fb thread0 failed!\n");
		goto SAMPLE_FB_StandarMode_1;
	}

	SAMPLE_FB_TO_EXIT();

SAMPLE_FB_StandarMode_1:
	SAMPLE_COMM_VO_StopVO(pstVoDevInfo);
SAMPLE_FB_StandarMode_0:
	SAMPLE_COMM_SYS_Exit();

	return s32Ret;
}

CVI_S32 SAMPLE_FB_DoubleBufMode(SAMPLE_VO_CONFIG_S *pstVoDevInfo)
{
	CVI_S32					s32Ret = CVI_SUCCESS;
	PTHREAD_FB_SAMPLE_INFO	stInfo0;
	VO_DEV					VoDev = pstVoDevInfo->VoDev;
	VB_CONFIG_S				stVbConfig;
	CVI_U32					u32BlkSize;

	/******************************************
	 step 1: mpp system init.
	******************************************/
	u32BlkSize = COMMON_GetPicBufferSize(pstVoDevInfo->stImageSize.u32Width, pstVoDevInfo->stImageSize.u32Height,
		pstVoDevInfo->enPixFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	memset(&stVbConfig, 0, sizeof(VB_CONFIG_S));
	stVbConfig.u32MaxPoolCnt = 1;
	stVbConfig.astCommPool[0].u32BlkCnt = 5;
	stVbConfig.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConfig.astCommPool[0].enRemapMode = VB_REMAP_MODE_CACHED;

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_SYS_Init failed with %d!\n", s32Ret);
		goto SAMPLE_FB_StandarMode_0;
	}

	/******************************************
	 step 2: Start VO device.
	 NOTE: Step 3 is optional when VO is running on other system.
	******************************************/
	s32Ret = SAMPLE_COMM_VO_StartVO(pstVoDevInfo);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VO_StartVO failed with %d!\n", s32Ret);
		goto SAMPLE_FB_StandarMode_0;
	}

	/******************************************
	 step 3:  start fb.
	*****************************************/
	stInfo0.layer	  =  VoDev;    /* VO device number */
	stInfo0.fd		  = -1;
	stInfo0.width	  = pstVoDevInfo->stImageSize.u32Width;
	stInfo0.height	   = pstVoDevInfo->stImageSize.u32Height;
	stInfo0.bEnableTDE = CVI_FALSE;
	stInfo0.bCompress =  CVI_FALSE; /* Compress opened or not */
	stInfo0.enColorFmt = OSD_COLOR_FMT_RGB1555;
	if (pthread_create(&g_stFbThread, 0, SAMPLE_FB_DOUBLE_BUF_DISPLAY, (void *)(&stInfo0))) {
		SAMPLE_PRT("start fb thread0 failed!\n");
		goto SAMPLE_FB_StandarMode_1;
	}

	SAMPLE_FB_TO_EXIT();

SAMPLE_FB_StandarMode_1:
	SAMPLE_COMM_VO_StopVO(pstVoDevInfo);
SAMPLE_FB_StandarMode_0:
	SAMPLE_COMM_SYS_Exit();

	return s32Ret;
}

CVI_S32 SAMPLE_FB_TDE(SAMPLE_VO_CONFIG_S *pstVoDevInfo)
{
	CVI_S32					s32Ret = CVI_SUCCESS;
	PTHREAD_FB_SAMPLE_INFO	stInfo0;
	VO_DEV					VoDev = pstVoDevInfo->VoDev;
	VB_CONFIG_S				stVbConfig;
	CVI_U32					u32BlkSize;

	/******************************************
	 step 1: mpp system init.
	******************************************/
	u32BlkSize = COMMON_GetPicBufferSize(pstVoDevInfo->stImageSize.u32Width, pstVoDevInfo->stImageSize.u32Height,
		pstVoDevInfo->enPixFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	memset(&stVbConfig, 0, sizeof(VB_CONFIG_S));
	stVbConfig.u32MaxPoolCnt = 1;
	stVbConfig.astCommPool[0].u32BlkCnt = 5;
	stVbConfig.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConfig.astCommPool[0].enRemapMode = VB_REMAP_MODE_CACHED;

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_SYS_Init failed with %d!\n", s32Ret);
		goto SAMPLE_FB_StandarMode_0;
	}

	/******************************************
	 step 2: Start VO device.
	 NOTE: Step 3 is optional when VO is running on other system.
	******************************************/
	s32Ret = SAMPLE_COMM_VO_StartVO(pstVoDevInfo);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VO_StartVO failed with %d!\n", s32Ret);
		goto SAMPLE_FB_StandarMode_0;
	}

	/******************************************
	 step 3:  start fb.
	*****************************************/
	stInfo0.layer	  =  VoDev;    /* VO device number */
	stInfo0.fd		  = -1;
	stInfo0.width	  = pstVoDevInfo->stImageSize.u32Width;
	stInfo0.height	   = pstVoDevInfo->stImageSize.u32Height;
	stInfo0.bEnableTDE = CVI_TRUE;
	stInfo0.bCompress =  CVI_FALSE; /* Compress opened or not */
	stInfo0.enColorFmt = OSD_COLOR_FMT_RGB8888;
	if (pthread_create(&g_stFbThread, 0, SAMPLE_FB_DOUBLE_BUF_DISPLAY, (void *)(&stInfo0))) {
		SAMPLE_PRT("start fb thread0 failed!\n");
		goto SAMPLE_FB_StandarMode_1;
	}

	SAMPLE_FB_TO_EXIT();

SAMPLE_FB_StandarMode_1:
	SAMPLE_COMM_VO_StopVO(pstVoDevInfo);
SAMPLE_FB_StandarMode_0:
	SAMPLE_COMM_SYS_Exit();

	return s32Ret;
}

int main(int argc, char *argv[])
{
	CVI_S32		s32Ret = CVI_FAILURE;
	CVI_CHAR		ch;
	SAMPLE_VO_CONFIG_S stVoDevInfo;

	if (argc < 5) {
		SAMPLE_FB_Usage1(argv[0]);
		return CVI_FAILURE;
	}

	signal(SIGINT, SAMPLE_FB_HandleSig);
	signal(SIGTERM, SAMPLE_FB_HandleSig);

	s32Ret = SAMPLE_FB_VO_GetDefConfig(&stVoDevInfo, atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VO_GetDefConfig failed with %d!\n", s32Ret);
		return CVI_FAILURE;
	}
	/******************************************
	 1 choose the case
	******************************************/
	ch = *(argv[1]);
	gs_cExitFlag = 0;
	switch (ch) {
	case '0':
	{
		SAMPLE_PRT("\nindex 0 selected.\n");
		s32Ret = SAMPLE_FB_OneBufMode(&stVoDevInfo);
		break;
	}
	case '1':
	{
		SAMPLE_PRT("\nindex 1 selected.\n");
		s32Ret = SAMPLE_FB_DoubleBufMode(&stVoDevInfo);
		break;
	}
	case '2':
	{
		SAMPLE_PRT("\nindex 2 selected.\n");
		s32Ret = SAMPLE_FB_TDE(&stVoDevInfo);
		break;
	}

	default:
	{
		printf("index invaild! please try again.\n");
		break;
	}
	}

	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("program exit normally!\n");
	} else {
		SAMPLE_PRT("program exit abnormally!\n");
	}

	return (s32Ret);
}



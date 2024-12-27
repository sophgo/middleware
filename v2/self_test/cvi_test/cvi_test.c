#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <inttypes.h>

#include <fcntl.h>		/* low-level i/o */
#include "cvi_buffer.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_comm_isp.h"
#include "cvi_comm_sns.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_isp.h"
#include "cvi_misc.h"

#include "sample_comm.h"
#ifndef FPGA_PORTING
#include "isp_test.h"
#include "ae/ae_debug.h"
#include "awb/awb_debug.h"
#endif
#include "vpss_test.h"
#include "vi_test.h"
#include "sys_test.h"
#include "../../sample/venc/include/sample_venc_lib.h"
#include "../../sample/sensor_test/inc/ae_test.h"
#ifdef MTRACE
#include "mcheck.h"
#endif

#ifdef DDR_64MB_SIZE
#include "sys/param.h"
#endif


#define MAX_VENC_BIND	4

#define TEST_ISP_API(getApi, setApi, St) \
{ \
	St param; \
	getApi(0, &param); \
	setApi(0, &param); \
} \

typedef struct _SAMPLE_VENC_TEST_S_ {
	sampleVenc sv;
	CVI_S32 bind_mode;
	CVI_S32 numChn;
	CVI_S32 vpssGrp;
	CVI_S32 vpssChn[MAX_VENC_BIND];
	CVI_S32 codec;
} SAMPLE_VENC_TEST_S;

static SAMPLE_VI_CONFIG_S g_stViConfig;
static SAMPLE_INI_CFG_S g_stIniCfg;
SAMPLE_VENC_TEST_S sampleVencTest, *psvt = &sampleVencTest;

static CVI_S32 _handle_op(CVI_S32 op, SAMPLE_INI_CFG_S *pstIniCfg, SAMPLE_VI_CONFIG_S *pstViConfig);
static void ViConfigReInit(SAMPLE_VI_CONFIG_S *p_stViConfig, SAMPLE_INI_CFG_S *p_stIniCfg);

static void _vpss_start_bind_vi(VPSS_GRP VpssGrp, SIZE_S stSize, CVI_U8 chns)
{
	VPSS_GRP_ATTR_S    stVpssGrpAttr;
	VPSS_CHN           VpssChn        = VPSS_CHN0;
	CVI_BOOL           abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
	CVI_S32 s32Ret = CVI_SUCCESS;

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat                  = PIXEL_FORMAT_YUV_PLANAR_420;
	stVpssGrpAttr.u32MaxW                        = stSize.u32Width;
	stVpssGrpAttr.u32MaxH                        = stSize.u32Height;

	astVpssChnAttr[VpssChn].u32Width                    = 1280;
	astVpssChnAttr[VpssChn].u32Height                   = 720;

	astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat               = PIXEL_FORMAT_RGB_888_PLANAR;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth                    = 0;
	astVpssChnAttr[VpssChn].bMirror                     = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip                       = CVI_FALSE;
	#if 0
	astVpssChnAttr[VpssChn].stAspectRatio.enMode        = ASPECT_RATIO_MANUAL;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.s32X = 40;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.s32Y = 400;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.u32Width = 640;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.u32Height = 360;
	#else
	astVpssChnAttr[VpssChn].stAspectRatio.enMode        = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	//astVpssChnAttr[VpssChn].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
	#endif
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;

	if (chns == 2) {
		abChnEnable[1] = CVI_TRUE;
		VpssChn        = VPSS_CHN1;
		astVpssChnAttr[VpssChn].u32Width                    = 640;
		astVpssChnAttr[VpssChn].u32Height                   = 480;
		astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
		astVpssChnAttr[VpssChn].enPixelFormat               = PIXEL_FORMAT_YUV_PLANAR_420;
		astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
		astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
		astVpssChnAttr[VpssChn].u32Depth                    = 0;
		astVpssChnAttr[VpssChn].bMirror                     = CVI_FALSE;
		astVpssChnAttr[VpssChn].bFlip                       = CVI_FALSE;
		astVpssChnAttr[VpssChn].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
		astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;
	}

	/*start vpss*/
	abChnEnable[0] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return;
	}

	s32Ret = SAMPLE_COMM_VI_Bind_VPSS(0, 0, VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
		return;
	}
}

static void _vpss_stop_unbind_vi(VPSS_GRP VpssGrp)
{
	CVI_BOOL           abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {CVI_TRUE, };
	CVI_S32 s32Ret = CVI_SUCCESS;

	SAMPLE_COMM_VI_UnBind_VPSS(0, 0, VpssGrp);

	s32Ret = SAMPLE_COMM_VPSS_Stop(VpssGrp, abChnEnable);
	if (s32Ret != CVI_SUCCESS)
		CVI_TRACE_LOG(CVI_DBG_ERR, "stop vpss group failed. s32Ret: 0x%x !\n", s32Ret);
}

static CVI_S32 _vi_get_chn_frame(CVI_U8 chn)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	VI_CROP_INFO_S crop_info = {0};

	if (CVI_VI_GetChnFrame(0, chn, &stVideoFrame, 3000) == 0) {
		FILE *output;
		size_t image_size = stVideoFrame.stVFrame.u32Length[0] + stVideoFrame.stVFrame.u32Length[1]
				  + stVideoFrame.stVFrame.u32Length[2];
		CVI_VOID *vir_addr;
		CVI_U32 plane_offset, u32LumaSize, u32ChromaSize;
		CVI_CHAR img_name[128] = {0, };

		CVI_TRACE_LOG(CVI_DBG_WARN, "width: %d, height: %d, total_buf_length: %zu\n",
			   stVideoFrame.stVFrame.u32Width,
			   stVideoFrame.stVFrame.u32Height, image_size);

		snprintf(img_name, sizeof(img_name), "sample_%d.yuv", chn);

		output = fopen(img_name, "wb");
		if (output == NULL) {
			memset(img_name, 0x0, sizeof(img_name));
			snprintf(img_name, sizeof(img_name), "/mnt/data/sample_%d.yuv", chn);
			output = fopen(img_name, "wb");
			if (output == NULL) {
				CVI_VI_ReleaseChnFrame(0, chn, &stVideoFrame);
				CVI_TRACE_LOG(CVI_DBG_ERR, "fopen fail\n");
				return CVI_FAILURE;
			}
		}

		u32LumaSize =  stVideoFrame.stVFrame.u32Stride[0] * stVideoFrame.stVFrame.u32Height;
		u32ChromaSize =  stVideoFrame.stVFrame.u32Stride[1] * stVideoFrame.stVFrame.u32Height / 2;
		CVI_VI_GetChnCrop(0, chn, &crop_info);
		if (crop_info.bEnable) {
			u32LumaSize = ALIGN((crop_info.stCropRect.u32Width * 8 + 7) >> 3, DEFAULT_ALIGN) *
				ALIGN(crop_info.stCropRect.u32Height, 2);
			u32ChromaSize = (ALIGN(((crop_info.stCropRect.u32Width >> 1) * 8 + 7) >> 3, DEFAULT_ALIGN) *
				ALIGN(crop_info.stCropRect.u32Height, 2)) >> 1;
		}
		vir_addr = CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0], image_size);
		CVI_SYS_IonInvalidateCache(stVideoFrame.stVFrame.u64PhyAddr[0], vir_addr, image_size);
		plane_offset = 0;
		for (int i = 0; i < 3; i++) {
			if (stVideoFrame.stVFrame.u32Length[i] != 0) {
				stVideoFrame.stVFrame.pu8VirAddr[i] = vir_addr + plane_offset;
				plane_offset += stVideoFrame.stVFrame.u32Length[i];
				CVI_TRACE_LOG(CVI_DBG_WARN,
					   "plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d) length(%d)\n",
					   i, stVideoFrame.stVFrame.u64PhyAddr[i],
					   stVideoFrame.stVFrame.pu8VirAddr[i],
					   stVideoFrame.stVFrame.u32Stride[i],
					   stVideoFrame.stVFrame.u32Length[i]);
				fwrite((void *)stVideoFrame.stVFrame.pu8VirAddr[i]
					, (i == 0) ? u32LumaSize : u32ChromaSize, 1, output);
			}
		}
		CVI_SYS_Munmap(vir_addr, image_size);

		if (CVI_VI_ReleaseChnFrame(0, chn, &stVideoFrame) != 0)
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_ReleaseChnFrame NG\n");
		else
			CVI_TRACE_LOG(CVI_DBG_WARN, "CVI_VI_ReleaseChnFrame TEST-PASS\n");

		fclose(output);
		return CVI_SUCCESS;
	}
	CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_GetChnFrame NG\n");
	return CVI_FAILURE;
}

#define YUV_SAVE_BUF_NUM 140

static CVI_S32 _vi_get_multi_chn_frame(CVI_U32 loop)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	CVI_U32 imageHeight = 1920, imageWidth = 1080;
	CVI_U32 imageSize = imageHeight * imageWidth * 3 >> 1;
	CVI_U32 imageBufferNum = YUV_SAVE_BUF_NUM;
	CVI_U8 *imageBuffer = CVI_NULL, *ptr = CVI_NULL;
	CVI_U32 count = 0, i;
	//CVI_U32 timing[3][loop];
	CVI_VOID *addr[YUV_SAVE_BUF_NUM] = { 0 };
	CVI_U32 size[YUV_SAVE_BUF_NUM] = { 0 };
	CVI_CHAR fname[50] = "";
	FILE *output;
	//struct timeval t1, t2, t3, t4, t5;
	CVI_U32 plane_offset, u32LumaSize, u32ChromaSize;
	size_t image_size;
	CVI_VOID *vir_addr;

	if (loop > YUV_SAVE_BUF_NUM) {
		printf("sorry. Current can't store more than %d frame\n", YUV_SAVE_BUF_NUM);
		return CVI_FAILURE;
	}
	ptr = imageBuffer = calloc(imageBufferNum, imageSize);
	if (imageBuffer == CVI_NULL) {
		printf("buffer allocate failed\n");
		return CVI_FAILURE;
	}
	//gettimeofday(&t1, NULL);
	for (i = 0; i < loop; i++) {
		//gettimeofday(&t2, NULL);
		if (CVI_VI_GetChnFrame(0, 0, &stVideoFrame, 1000) == 0) {
			//gettimeofday(&t3, NULL);
			addr[i] = ptr;

			image_size = stVideoFrame.stVFrame.u32Length[0] + stVideoFrame.stVFrame.u32Length[1]
				   + stVideoFrame.stVFrame.u32Length[2];
			u32LumaSize =  stVideoFrame.stVFrame.u32Stride[0] * stVideoFrame.stVFrame.u32Height;
			u32ChromaSize =  stVideoFrame.stVFrame.u32Stride[1] * stVideoFrame.stVFrame.u32Height / 2;

			vir_addr = CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0], image_size);
			CVI_SYS_IonInvalidateCache(stVideoFrame.stVFrame.u64PhyAddr[0], vir_addr, image_size);
			plane_offset = 0;
			stVideoFrame.stVFrame.pu8VirAddr[0] = vir_addr;
			plane_offset += stVideoFrame.stVFrame.u32Length[0];
			stVideoFrame.stVFrame.pu8VirAddr[1] = vir_addr + plane_offset;
			plane_offset += stVideoFrame.stVFrame.u32Length[1];
			stVideoFrame.stVFrame.pu8VirAddr[2] = vir_addr + plane_offset;

			/*Todo@CF. Will modify to DMA after.*/
			memcpy((void *)ptr, (const void *)stVideoFrame.stVFrame.pu8VirAddr[0],
					u32LumaSize);
			count = u32LumaSize;
			ptr += count;
			size[i] += count;
			memcpy((void *)ptr, (const void *)stVideoFrame.stVFrame.pu8VirAddr[1],
					u32ChromaSize);
			count = u32ChromaSize;
			ptr += count;
			size[i] += count;
			memcpy((void *)ptr, (const void *)stVideoFrame.stVFrame.pu8VirAddr[2],
					u32ChromaSize);
			count = u32ChromaSize;
			ptr += count;
			size[i] += count;
			//timing[0][i] = t3.tv_usec - t2.tv_usec;
			//timing[1][i] = t4.tv_usec - t3.tv_usec;
			//timing[2][i] = t5.tv_usec - t4.tv_usec;
			CVI_SYS_Munmap((void *)stVideoFrame.stVFrame.pu8VirAddr[0],
					stVideoFrame.stVFrame.u32Length[0]);
			CVI_SYS_Munmap((void *)stVideoFrame.stVFrame.pu8VirAddr[1],
					stVideoFrame.stVFrame.u32Length[1]);
			CVI_SYS_Munmap((void *)stVideoFrame.stVFrame.pu8VirAddr[2],
					stVideoFrame.stVFrame.u32Length[2]);
		} else {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_GetChnFrame NG\n");
		}

		if (CVI_VI_ReleaseChnFrame(0, 0, &stVideoFrame) != 0)
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_ReleaseChnFrame NG\n");
	}
	/*Todo@CF. CY said 33ms 140frame is OK now. So first write card here for robust.*/
	count = 0;
	while (1) {
		sprintf(fname, "/mnt/sample%03d.yuv", count);
		output = fopen(fname, "wb");
		fwrite((void *)addr[count], size[count], 1, output);
		fclose(output);
		count++;
		//sleep(1);
		if (count == loop)
			break;
	}
	free(imageBuffer);
	printf("yuv save complete\n");
	return CVI_SUCCESS;
}

static CVI_S32 _vi_sdk_test(SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_U8	chn = 0;
	CVI_S32 ret = CVI_SUCCESS;

	ret = CVI_VI_GetPipeFd(0);
	if (ret < 0) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_GetPipeFd is fail\n");
		return ret;
	}

	ret = CVI_VI_CloseFd();
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_CloseFd is fail\n");
		return ret;
	}

	if (_vi_get_chn_frame(chn) == CVI_SUCCESS)
		CVI_TRACE_LOG(CVI_DBG_WARN, "CVI_VI_GetChnFrame TEST-PASS\n");
	else {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_GetChnFrame TEST-FAIL\n");
		return CVI_FAILURE;
	}

	VIDEO_FRAME_INFO_S stVideoFrame[2];
	VI_DUMP_ATTR_S attr;
	CVI_U32 dev = 0;

	memset(stVideoFrame, 0, sizeof(stVideoFrame));

	stVideoFrame[0].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;
	stVideoFrame[1].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;

	attr.bEnable = 1;
	attr.u32Depth = 0;
	attr.enDumpType = VI_DUMP_TYPE_RAW;
	CVI_VI_SetPipeDumpAttr(dev, &attr);

	attr.bEnable = 0;
	attr.enDumpType = VI_DUMP_TYPE_IR;
	CVI_VI_GetPipeDumpAttr(dev, &attr);

	ret = CVI_VI_GetPipeFrame(dev, stVideoFrame, 1000);
	if (ret == CVI_SUCCESS)
		CVI_TRACE_LOG(CVI_DBG_WARN, "CVI_VI_GetPipeFrame TEST-PASS\n");
	else {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_GetPipeFrame TEST-FAIL\n");
		return CVI_FAILURE;
	}

	ret = CVI_VI_ReleasePipeFrame(dev, stVideoFrame);
	if (ret == CVI_SUCCESS)
		CVI_TRACE_LOG(CVI_DBG_WARN, "CVI_VI_ReleasePipeFrame TEST-PASS\n");
	else {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_ReleasePipeFrame TEST-FAIL\n");
		return CVI_FAILURE;
	}

	ret = CVI_VI_SetPipeFrameSource(0, VI_PIPE_FRAME_SOURCE_DEV);
	if (ret  == CVI_SUCCESS)
		CVI_TRACE_LOG(CVI_DBG_WARN, "CVI_VI_SetPipeFrameSource TEST-PASS\n");
	else {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_SetPipeFrameSource TEST-FAIL\n");
		return CVI_FAILURE;
	}

	VI_PIPE_FRAME_SOURCE_E penSource = VI_PIPE_FRAME_SOURCE_USER_BE;

	ret = CVI_VI_GetPipeFrameSource(0, &penSource);
	if (ret == CVI_SUCCESS)
		if (penSource == VI_PIPE_FRAME_SOURCE_DEV)
			CVI_TRACE_LOG(CVI_DBG_WARN, "CVI_VI_GetPipeFrameSource TEST-PASS\n");
		else {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_GetPipeFrameSource TEST-FAIL\n");
			return CVI_FAILURE;
		}
	else {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_GetPipeFrameSource TEST-FAIL\n");
		return CVI_FAILURE;
	}

	VI_CHN_ATTR_S stChnAttr;

	ret = CVI_VI_GetChnAttr(0, 0, &stChnAttr);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_GetChnAttr fail\n");
		return CVI_FAILURE;
	}

	VI_CROP_INFO_S pstCropInfo;

	pstCropInfo.bEnable = CVI_TRUE;
	pstCropInfo.stCropRect.s32X = ALIGN_DOWN(stChnAttr.stSize.u32Width >> 2, 2);
	pstCropInfo.stCropRect.s32Y = ALIGN_DOWN(stChnAttr.stSize.u32Height >> 2, 2);
	pstCropInfo.stCropRect.u32Width = ALIGN_DOWN(stChnAttr.stSize.u32Width >> 1, 2);
	pstCropInfo.stCropRect.u32Height = ALIGN_DOWN(stChnAttr.stSize.u32Height >> 1, 2);

	ret = CVI_VI_SetChnCrop(0, 0, &pstCropInfo);
	if (ret == CVI_SUCCESS)
		CVI_TRACE_LOG(CVI_DBG_WARN, "CVI_VI_SetChnCrop TEST-PASS\n");
	else {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_SetChnCrop TEST-FAIL\n");
		return CVI_FAILURE;
	}

	VI_DEV_TIMING_ATTR_S stTimingAttr;

	stTimingAttr.bEnable = CVI_TRUE;
	stTimingAttr.s32FrmRate = 30;

	ret = CVI_VI_SetDevTimingAttr(0, &stTimingAttr);
	if (ret == CVI_SUCCESS)
		CVI_TRACE_LOG(CVI_DBG_WARN, "CVI_VI_SetDevTimingAttr TEST-PASS\n");
	else {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_SetDevTimingAttr TEST-FAIL\n");
		return CVI_FAILURE;
	}

	stTimingAttr.bEnable = CVI_FALSE;
	stTimingAttr.s32FrmRate = 20;

	ret = CVI_VI_GetDevTimingAttr(0, &stTimingAttr);
	if (ret == CVI_SUCCESS)
		if (stTimingAttr.bEnable == CVI_TRUE && stTimingAttr.s32FrmRate == 30)
			CVI_TRACE_LOG(CVI_DBG_WARN, "CVI_VI_GetDevTimingAttr TEST-PASS\n");
		else {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_GetDevTimingAttr TEST-FAIL\n");
			return CVI_FAILURE;
		}
	else {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_GetDevTimingAttr TEST-FAIL\n");
		return CVI_FAILURE;
	}

	CVI_S32           i;
	CVI_S32           s32ViNum;
	SAMPLE_VI_INFO_S *pstViInfo = CVI_NULL;

	for (i = 0; i < pstViConfig->s32WorkingViNum; i++) {
		s32ViNum  = pstViConfig->as32WorkingViId[i];
		pstViInfo = &pstViConfig->astViInfo[s32ViNum];

		if (SAMPLE_COMM_VI_StopViChn(pstViInfo) == CVI_SUCCESS)
			CVI_TRACE_LOG(CVI_DBG_WARN, "CVI_VI_DisableChn TEST-PASS\n");
		else {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_DisableChn TEST-FAIL\n");
			return CVI_FAILURE;
		}

		if (SAMPLE_COMM_VI_StopViPipe(pstViInfo) == CVI_SUCCESS)
			CVI_TRACE_LOG(CVI_DBG_WARN, "CVI_VI_StopPipe and CVI_VI_DestroyPipe TEST-PASS\n");
		else {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_StopPipe and CVI_VI_DestroyPipe TEST-FAIL\n");
			return CVI_FAILURE;
		}

		if (SAMPLE_COMM_VI_StopDev(pstViInfo) == CVI_SUCCESS)
			CVI_TRACE_LOG(CVI_DBG_WARN, "CVI_VI_DisableDev TEST-PASS\n");
		else {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_DisableDev TEST-FAIL\n");
			return CVI_FAILURE;
		}
	}

	return CVI_SUCCESS;
}

static CVI_S32 _get_input_frame_size(chnInputCfg *pIc,
		unsigned int *pWidth, unsigned int *pHeight)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	CVI_U8 chn = 0;
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (pIc->bind_mode == VENC_BIND_VI) {
		if (CVI_VI_GetChnFrame(0, chn, &stVideoFrame, 1000) == 0) {

			CVI_TRACE_LOG(CVI_DBG_INFO, "width: %d, height: %d\n",
					stVideoFrame.stVFrame.u32Width,
					stVideoFrame.stVFrame.u32Height);

			*pWidth = stVideoFrame.stVFrame.u32Width;
			*pHeight = stVideoFrame.stVFrame.u32Height;

			if (CVI_VI_ReleaseChnFrame(0, chn, &stVideoFrame) != 0)
				CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_ReleaseChnFrame NG\n");

			s32Ret = 0;
		}
	} else if (pIc->bind_mode == VENC_BIND_VPSS) {

		if (pIc->vpssChn >= MAX_VENC_BIND) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vpssChn = %d, >= MAX_VENC_BIND\n", pIc->vpssChn);
			return -1;
		}

		s32Ret = CVI_VPSS_GetChnFrame(pIc->vpssGrp, pIc->vpssChn,
				&stVideoFrame, 1000);
		if (s32Ret == CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_INFO, "width: %d, height: %d\n",
					stVideoFrame.stVFrame.u32Width,
					stVideoFrame.stVFrame.u32Height);

			*pWidth = stVideoFrame.stVFrame.u32Width;
			*pHeight = stVideoFrame.stVFrame.u32Height;

			if (CVI_VPSS_ReleaseChnFrame(pIc->vpssGrp, pIc->vpssChn,
						&stVideoFrame) != 0)
				CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_ReleaseChnFrame NG\n");

			s32Ret = 0;
		}
	} else {
		CVI_TRACE_LOG(CVI_DBG_ERR, "bind_mode = %d\n", pIc->bind_mode);
		return -1;
	}

	return s32Ret;
}

static CVI_S32 _get_frame_pixel_format(chnInputCfg *pIc, int chn_id)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;

	if (pIc->bind_mode == VENC_BIND_VI) {
		CVI_U8 chn = 0;
		VI_CHN_ATTR_S stViChnAttr;

		s32Ret = CVI_VI_GetChnAttr(0, chn, &stViChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_GetChnAttr, chn idx = %d\n", chn);
			return -1;
		}
		enPixelFormat = stViChnAttr.enPixelFormat;
	} else if (pIc->bind_mode == VENC_BIND_VPSS) {
		VPSS_CHN_ATTR_S stChnAttr;

		s32Ret = CVI_VPSS_GetChnAttr(0, chn_id, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnAttr, chn idx = %d\n", chn_id);
			return -1;
		}
		enPixelFormat = stChnAttr.enPixelFormat;
	}

	switch (enPixelFormat) {
	case PIXEL_FORMAT_YUV_PLANAR_422:
		pIc->pixel_format = 1;
		break;
	case PIXEL_FORMAT_NV12:
		pIc->pixel_format = 2;
		break;
	case PIXEL_FORMAT_NV21:
		pIc->pixel_format = 3;
		break;
	case PIXEL_FORMAT_YUV_PLANAR_420:
	default:
		pIc->pixel_format = 0;
		break;
	}

	return s32Ret;
}

static CVI_S32 _start_venc(SAMPLE_VENC_TEST_S *psvt)
{
	static const char * const argv[] = {
		"sample_venc",
		"-c", "265",
		"--getBsMode=0",
		"-i", "../yuv/coastguard_352x288_300.yuv",
		"-n", "1000000",
		"--ifInitVb=0"
	};
	sampleVenc *psv = &psvt->sv;
	int argc = sizeof(argv) / sizeof(char *);
	commonInputCfg *pcic = &psv->commonIc;
	chnInputCfg *pIc = &psv->chnCtx[0].chnIc;
	int ret = 0, idx;
	unsigned int width = 0, height = 0;


	memset(psv, 0, sizeof(*psv));

	initInputCfg(pcic, pIc);

	if (parseEncArgv(psv, pIc, argc, (char **)argv) < 0) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "parseEncArgv\n");
		return -1;
	}

	if (psvt->bind_mode == VENC_BIND_VI ||
		psvt->bind_mode == VENC_BIND_VPSS) {
		pcic->numChn = psvt->numChn;
	} else {
		CVI_TRACE_LOG(CVI_DBG_ERR, "bind_mode is not supported\n");
		return CVI_FAILURE;
	}

	for (idx = 0; idx < pcic->numChn; idx++) {
		pIc = &psv->chnCtx[idx].chnIc;

		pIc->bind_mode = psvt->bind_mode;
		pIc->vpssGrp = psvt->vpssGrp;
		pIc->vpssChn = psvt->vpssChn[idx];

		ret = _get_input_frame_size(pIc, &width, &height);
		if (ret < 0) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "_get_input_frame_size\n");
			return -1;
		}

		pIc->width = width;
		pIc->height = height;
		SAMPLE_PRT("size = %d x %d\n", width, height);

		ret = _get_frame_pixel_format(pIc, idx);
		if (ret < 0) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "_get_frame_pixel_format\n");
			return -1;
		}
		SAMPLE_PRT("pixel_format = %d\n", pIc->pixel_format);

		sprintf(pIc->output_path, "test-%d", idx);
		SAMPLE_PRT("output_path = %s\n", pIc->output_path);

		if (psvt->codec == PT_MJPEG) {
			sprintf(pIc->codec, "%s", "mjp");
			pIc->rcMode = 4;
			pIc->num_frames = 1000000;
		} else if (psvt->codec == PT_JPEG) {
			sprintf(pIc->codec, "%s", "jpg");
			pIc->quality = 70;
			pIc->num_frames = 1;
		} else {
			if (psvt->codec == PT_H264) {
				sprintf(pIc->codec, "%s", "264");
			} else {
				sprintf(pIc->codec, "%s", "265");
				SAMPLE_PRT("Set codec to H.265\n");
			}

			pIc->rcMode = 4;
			pIc->gop = 60;
			pIc->iqp = 30;
			pIc->pqp = 30;
			pIc->firstFrmstartQp = 30;
			pIc->num_frames = 1000000;
		}
	}

	pcic->ifInitVb = 0;

	ret = SAMPLE_VENC_START(psv);
	if (ret < 0) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_VENC_START\n");
		return CVI_FAILURE;
	}

	return ret;
}

static CVI_S32 _stop_venc(SAMPLE_VENC_TEST_S *psvt)
{
	SAMPLE_VENC_MOVE_TO_STOP_STATE(&psvt->sv);
	return SAMPLE_VENC_STOP(&psvt->sv);
}

static CVI_VOID _get_vpss_info(CVI_VOID)
{
	CVI_S32 numChns = 2;
	CVI_S32 idx, s32Ret;
	VPSS_CHN_ATTR_S stChnAttr;

	for (idx = 0; idx < numChns; idx++) {
		s32Ret = CVI_VPSS_GetChnAttr(0, idx, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnAttr, chn idx = %d\n", idx);
			break;
		}

		CVI_TRACE_LOG(CVI_DBG_WARN, "-- VPSS Chn info --\n");
		CVI_TRACE_LOG(CVI_DBG_WARN, "width(%d) height(%d)\n",
				stChnAttr.u32Width, stChnAttr.u32Height);

		CVI_TRACE_LOG(CVI_DBG_WARN, "VideoFormat(%d) PxlFormat(%d) Depth(%d)\n",
				stChnAttr.enVideoFormat, stChnAttr.enPixelFormat, stChnAttr.u32Depth);

		CVI_TRACE_LOG(CVI_DBG_WARN, "Flip(%d) Mirror(%d)\n", stChnAttr.bFlip, stChnAttr.bMirror);
		CVI_TRACE_LOG(CVI_DBG_WARN, "frame-rate src(%d) dst(%d)\n",
				stChnAttr.stFrameRate.s32SrcFrameRate, stChnAttr.stFrameRate.s32DstFrameRate);

		CVI_TRACE_LOG(CVI_DBG_WARN, "aspect ratio mode(%d) bgcolor(%#x)\n",
				stChnAttr.stAspectRatio.enMode, stChnAttr.stAspectRatio.u32BgColor);

		if (stChnAttr.stAspectRatio.enMode == ASPECT_RATIO_MANUAL)
			CVI_TRACE_LOG(CVI_DBG_WARN, "manual rect(%d %d %d %d)\n",
					stChnAttr.stAspectRatio.stVideoRect.s32X,
					stChnAttr.stAspectRatio.stVideoRect.s32Y,
					stChnAttr.stAspectRatio.stVideoRect.u32Width,
					stChnAttr.stAspectRatio.stVideoRect.u32Height);
	}
}

static CVI_VOID _get_max_vb_size(SIZE_S *pstSize, SIZE_S stSizeLevel)
{
	if (pstSize->u32Width < stSizeLevel.u32Width) {
		pstSize->u32Width = stSizeLevel.u32Width;
		pstSize->u32Height = stSizeLevel.u32Height;
	} else if (pstSize->u32Width == stSizeLevel.u32Width && pstSize->u32Height < stSizeLevel.u32Height) {
		pstSize->u32Height = stSizeLevel.u32Height;
	}
}

static CVI_S32 _vpss_test(CVI_VOID)
{
	VPSS_GRP VpssGrp = 0;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = { CVI_TRUE, };
	CVI_S32  s32Ret = CVI_SUCCESS;
	struct vpss_test_ops *ops;
	bool fail_pause = false;
	PIC_SIZE_E enPicSize;
	SIZE_S stSize;
	SIZE_S stSizeVbDefault = {.u32Height = 1920, .u32Width = 1080};

#if defined(ARCH_CV182X)
	ops = &vpss_test_ops_1822;
#elif defined(__CV181X__) || defined(__CV180X__) || defined(__CV186X__)
	ops = &vpss_test_ops_mars;
#else
	ops = &vpss_test_ops_1835;
#endif

	// Exit to reallocate resource for test
	SAMPLE_COMM_VI_DestroyIsp(&g_stViConfig);
	SAMPLE_COMM_VI_DestroyVi(&g_stViConfig);
	SAMPLE_COMM_SYS_Exit();

	/************************************************
	 * step2:  Get input size
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(g_stIniCfg.enSnsType[0], &enPicSize);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VI_GetSizeBySensor failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_SYS_GetPicSize failed with %#x\n", s32Ret);
		return s32Ret;
	}

	_get_max_vb_size(&stSizeVbDefault, stSize);

	/************************************************
	 * step3:  Init modules
	 ************************************************/
	VB_CONFIG_S stVbConf;
	CVI_U32        u32BlkSize, u32BlkRotSize;

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
#if !defined(DDR_64MB_SIZE)
#if defined(ARCH_CV182X) || defined(__CV181X__) || defined(__CV180X__) || defined(__CV186X__)
	stVbConf.u32MaxPoolCnt = 2;

	u32BlkSize = COMMON_GetPicBufferSize(stSizeVbDefault.u32Width, stSizeVbDefault.u32Height,
		SAMPLE_PIXEL_FORMAT, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkRotSize = COMMON_GetPicBufferSize(stSizeVbDefault.u32Height, stSizeVbDefault.u32Width,
		SAMPLE_PIXEL_FORMAT, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSize = MAX2(u32BlkSize, u32BlkRotSize);

	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 6;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	SAMPLE_PRT("common pool[0] BlkSize %d\n", u32BlkSize);

	u32BlkSize = COMMON_GetPicBufferSize(1920, 1080, PIXEL_FORMAT_RGB_888_PLANAR,
		DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[1].u32BlkCnt	= 2;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	SAMPLE_PRT("common pool[1] BlkSize %d\n", u32BlkSize);
#else
	stVbConf.u32MaxPoolCnt = 2;

	u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height, SAMPLE_PIXEL_FORMAT,
		DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkRotSize = COMMON_GetPicBufferSize(stSize.u32Height, stSize.u32Width, SAMPLE_PIXEL_FORMAT,
		DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSize = MAX2(u32BlkSize, u32BlkRotSize);

	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 12;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	SAMPLE_PRT("common pool[0] BlkSize %d\n", u32BlkSize);

	u32BlkSize = COMMON_GetPicBufferSize(4096, 2160, SAMPLE_PIXEL_FORMAT,
		DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[1].u32BlkCnt	= 5;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	SAMPLE_PRT("common pool[1] BlkSize %d\n", u32BlkSize);
#endif

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "system init failed with %#x\n", s32Ret);
		return s32Ret;
	}

	if (g_stIniCfg.enSource == VI_PIPE_FRAME_SOURCE_DEV) {
		ViConfigReInit(&g_stViConfig, &g_stIniCfg);

		s32Ret = SAMPLE_PLAT_VI_INIT(&g_stViConfig);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vi init failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

	/************************************************
	 * step4:  Test
	 ************************************************/
	if (ops->vi_test) {
		if (ops->vi_test(stSize, fail_pause) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*---------   VI-VPSS test failed   ---------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			s32Ret = CVI_FAILURE;
		} else {
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*---------   VI-VPSS test PASS   -----------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		}
	}

	if (ops->in_fmt_test) {
		if (ops->in_fmt_test(fail_pause) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*---------  VPSS InFmt test failed ---------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			s32Ret = CVI_FAILURE;
		} else {
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*---------  VPSS InFmt test PASS -----------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		}
	}

	if (ops->out_fmt_test) {
		if (ops->out_fmt_test(fail_pause) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*--------- VPSS OutFmt test failed ---------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			s32Ret = CVI_FAILURE;
		} else {
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*--------- VPSS OutFmt test PASS -----------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		}
	}

	if (ops->yuv_2k_test) {
		if (ops->yuv_2k_test(fail_pause) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*---------Golden YUV 2k test failed---------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			s32Ret = CVI_FAILURE;
		} else {
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*---------Golden YUV 2k test PASS-----------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		}
	}

	if (ops->yuv_4k_test) {
		if (ops->yuv_4k_test(fail_pause) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*---------Golden YUV 4k test failed---------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			s32Ret = CVI_FAILURE;
		} else {
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*---------Golden YUV 4k test PASS-----------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		}
	}

	if (ops->rgb_test) {
		if (ops->rgb_test(fail_pause) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*----------Golden RGB 2k test failed--------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			s32Ret = CVI_FAILURE;
		} else {
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*----------Golden RGB 2k test PASS----------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		}
	}

	// stop all vpss.
	for (VpssGrp = 0; VpssGrp < 6; ++VpssGrp) {
		if (SAMPLE_COMM_VPSS_Stop(VpssGrp, abChnEnable) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "stop vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		}
	}

	// dual mode test
	if (ops->dual_yuv_test) {
		if (ops->dual_yuv_test(fail_pause) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------Golden YUV 2k dual test failed------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			s32Ret = CVI_FAILURE;
		} else {
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------Golden YUV 2k dual test PASS--------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		}
	}

	if (ops->param_grp_test) {
		if (ops->param_grp_test(fail_pause) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*--------VPSS GRP PARAM test failed---------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			s32Ret = CVI_FAILURE;
		} else {
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*---------  VPSS GRP PARAM PASS   ----------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		}
	}

	if (ops->param_chn_test) {
		if (ops->param_chn_test(fail_pause) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*--------VPSS CHN PARAM test failed---------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			s32Ret = CVI_FAILURE;
		} else {
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*---------  VPSS CHN PARAM PASS   ----------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		}
	}

	if (ops->dual_combo_test) {
		if (ops->dual_combo_test(fail_pause) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "*---------------------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------Golden YUV422/BGR packed 2k dual test failed------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*---------------------------------------------------------*\n");
			s32Ret = CVI_FAILURE;
		} else {
			CVI_TRACE_LOG(CVI_DBG_WARN, "*---------------------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------Golden YUV422/BGR packed 2k dual test PASS--------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*---------------------------------------------------------*\n");
		}
	}

	// test fd ops.
	CVI_S32 fd = CVI_VPSS_GetChnFd(9, 0);

	if (fd < 0) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnFd is fail\n");
		return CVI_FAILURE;
	}
#else

	/************************************************
	 * step4:  Test
	 ************************************************/
	stVbConf.u32MaxPoolCnt = 2;
	u32BlkSize = COMMON_GetPicBufferSize(stSizeVbDefault.u32Width, stSizeVbDefault.u32Height,
		SAMPLE_PIXEL_FORMAT, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkRotSize = COMMON_GetPicBufferSize(stSizeVbDefault.u32Height, stSizeVbDefault.u32Width,
		SAMPLE_PIXEL_FORMAT, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSize = MAX2(u32BlkSize, u32BlkRotSize);

	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 3;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	SAMPLE_PRT("common pool[0] BlkSize %d\n", u32BlkSize);

	u32BlkSize = COMMON_GetPicBufferSize(1280, 720, SAMPLE_PIXEL_FORMAT,
		DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[1].u32BlkCnt	= 2;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	SAMPLE_PRT("common pool[1] BlkSize %d\n", u32BlkSize);

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "system init failed with %#x\n", s32Ret);
		return s32Ret;
	}

	if (g_stIniCfg.enSource == VI_PIPE_FRAME_SOURCE_DEV) {
		ViConfigReInit(&g_stViConfig, &g_stIniCfg);

		s32Ret = SAMPLE_PLAT_VI_INIT(&g_stViConfig);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vi init failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}

	if (ops->vi_test) {
		if (ops->vi_test(stSize, fail_pause) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*---------   VI-VPSS test failed   ---------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			s32Ret = CVI_FAILURE;
		} else {
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*---------   VI-VPSS test PASS   -----------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		}
	}
	/************************************************
	 * Restart, exit to reallocate resource for next test
	 ************************************************/
	SAMPLE_COMM_VI_DestroyIsp(&g_stViConfig);
	SAMPLE_COMM_VI_DestroyVi(&g_stViConfig);
	SAMPLE_COMM_SYS_Exit();

	s32Ret = SAMPLE_COMM_SYS_GetPicSize(PIC_1080P, &stSize);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_SYS_GetPicSize failed with %#x\n", s32Ret);
		return s32Ret;
	}

	_get_max_vb_size(&stSizeVbDefault, stSize);
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	stVbConf.u32MaxPoolCnt = 1;
	u32BlkSize = COMMON_GetPicBufferSize(stSizeVbDefault.u32Width, stSizeVbDefault.u32Height,
		PIXEL_FORMAT_RGB_888_PLANAR, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkRotSize = COMMON_GetPicBufferSize(stSizeVbDefault.u32Height, stSizeVbDefault.u32Width,
		PIXEL_FORMAT_RGB_888_PLANAR, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSize = MAX2(u32BlkSize, u32BlkRotSize);

	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 2;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	SAMPLE_PRT("common pool[0] BlkSize %d\n", u32BlkSize);
	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "system init failed with %#x\n", s32Ret);
		return s32Ret;
	}

	if (ops->in_fmt_test) {
		if (ops->in_fmt_test(fail_pause) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*---------  VPSS InFmt test failed ---------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			s32Ret = CVI_FAILURE;
		} else {
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*---------  VPSS InFmt test PASS -----------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		}
	}

	if (ops->out_fmt_test) {
		if (ops->out_fmt_test(fail_pause) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*--------- VPSS OutFmt test failed ---------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			s32Ret = CVI_FAILURE;
		} else {
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*--------- VPSS OutFmt test PASS -----------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		}
	}

	/************************************************
	 * Restart, exit to reallocate resource for next test
	 ************************************************/
	SAMPLE_COMM_SYS_Exit();

	s32Ret = SAMPLE_COMM_SYS_GetPicSize(PIC_1080P, &stSize);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_SYS_GetPicSize failed with %#x\n", s32Ret);
		return s32Ret;
	}

	_get_max_vb_size(&stSizeVbDefault, stSize);
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	stVbConf.u32MaxPoolCnt = 2;
	u32BlkSize = COMMON_GetPicBufferSize(stSizeVbDefault.u32Width, stSizeVbDefault.u32Height,
		PIXEL_FORMAT_RGB_888_PLANAR, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkRotSize = COMMON_GetPicBufferSize(stSizeVbDefault.u32Height, stSizeVbDefault.u32Width,
		PIXEL_FORMAT_RGB_888_PLANAR, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSize = MAX2(u32BlkSize, u32BlkRotSize);

	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 1;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	SAMPLE_PRT("common pool[0] BlkSize %d\n", u32BlkSize);

	u32BlkSize = COMMON_GetPicBufferSize(1280, 736,
		PIXEL_FORMAT_RGB_888_PLANAR, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkRotSize = COMMON_GetPicBufferSize(736, 1280,
		PIXEL_FORMAT_RGB_888_PLANAR, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSize = MAX2(u32BlkSize, u32BlkRotSize);

	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[1].u32BlkCnt	= 3;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	SAMPLE_PRT("common pool[1] BlkSize %d\n", u32BlkSize);
	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "system init failed with %#x\n", s32Ret);
		return s32Ret;
	}

	if (ops->yuv_2k_test) {
		if (ops->yuv_2k_test(fail_pause) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*---------Golden YUV 2k test failed---------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			s32Ret = CVI_FAILURE;
		} else {
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*---------Golden YUV 2k test PASS-----------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		}
	}

	if (ops->yuv_4k_test) {
		if (ops->yuv_4k_test(fail_pause) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*---------Golden YUV 4k test failed---------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			s32Ret = CVI_FAILURE;
		} else {
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*---------Golden YUV 4k test PASS-----------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		}
	}

	/************************************************
	 * Restart, exit to reallocate resource for next test
	 ************************************************/
	SAMPLE_COMM_SYS_Exit();

	s32Ret = SAMPLE_COMM_SYS_GetPicSize(PIC_1080P, &stSize);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_SYS_GetPicSize failed with %#x\n", s32Ret);
		return s32Ret;
	}

	_get_max_vb_size(&stSizeVbDefault, stSize);
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	stVbConf.u32MaxPoolCnt = 2;
	u32BlkSize = COMMON_GetPicBufferSize(stSizeVbDefault.u32Width, stSizeVbDefault.u32Height,
		PIXEL_FORMAT_RGB_888_PLANAR, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkRotSize = COMMON_GetPicBufferSize(stSizeVbDefault.u32Height, stSizeVbDefault.u32Width,
		PIXEL_FORMAT_RGB_888_PLANAR, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSize = MAX2(u32BlkSize, u32BlkRotSize);

	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 2;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	SAMPLE_PRT("common pool[0] BlkSize %d\n", u32BlkSize);

	u32BlkSize = COMMON_GetPicBufferSize(608, 608,
		PIXEL_FORMAT_RGB_888_PLANAR, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[1].u32BlkCnt	= 2;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	SAMPLE_PRT("common pool[1] BlkSize %d\n", u32BlkSize);
	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "system init failed with %#x\n", s32Ret);
		return s32Ret;
	}

	if (ops->rgb_test) {
		if (ops->rgb_test(fail_pause) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*----------Golden RGB 2k test failed--------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			s32Ret = CVI_FAILURE;
		} else {
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*----------Golden RGB 2k test PASS----------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		}
	}

	/************************************************
	 * Restart, exit to reallocate resource for next test
	 ************************************************/
	SAMPLE_COMM_SYS_Exit();

	s32Ret = SAMPLE_COMM_SYS_GetPicSize(PIC_1080P, &stSize);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_SYS_GetPicSize failed with %#x\n", s32Ret);
		return s32Ret;
	}

	_get_max_vb_size(&stSizeVbDefault, stSize);
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	stVbConf.u32MaxPoolCnt = 2;
	u32BlkSize = COMMON_GetPicBufferSize(stSizeVbDefault.u32Width, stSizeVbDefault.u32Height,
		PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkRotSize = COMMON_GetPicBufferSize(stSizeVbDefault.u32Height, stSizeVbDefault.u32Width,
		PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSize = MAX2(u32BlkSize, u32BlkRotSize);

	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 1;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	SAMPLE_PRT("common pool[0] BlkSize %d\n", u32BlkSize);
	u32BlkSize = COMMON_GetPicBufferSize(736, 1280,
		PIXEL_FORMAT_RGB_888_PLANAR, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkRotSize = COMMON_GetPicBufferSize(1280, 736,
		PIXEL_FORMAT_RGB_888_PLANAR, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSize = MAX2(u32BlkSize, u32BlkRotSize);

	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[1].u32BlkCnt	= 2;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	SAMPLE_PRT("common pool[1] BlkSize %d\n", u32BlkSize);

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "system init failed with %#x\n", s32Ret);
		return s32Ret;
	}

	/************************************************
	 * dual mode test
	 ************************************************/
	if (ops->dual_yuv_test) {
		if (ops->dual_yuv_test(fail_pause) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------Golden YUV 2k dual test failed------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			s32Ret = CVI_FAILURE;
		} else {
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------Golden YUV 2k dual test PASS--------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		}
	}

	if (ops->param_grp_test) {
		if (ops->param_grp_test(fail_pause) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*--------VPSS GRP PARAM test failed---------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			s32Ret = CVI_FAILURE;
		} else {
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*---------  VPSS GRP PARAM PASS   ----------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		}
	}

	if (ops->param_chn_test) {
		if (ops->param_chn_test(fail_pause) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*--------VPSS CHN PARAM test failed---------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			s32Ret = CVI_FAILURE;
		} else {
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*---------  VPSS CHN PARAM PASS   ----------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		}
	}

	/************************************************
	 * Restart, exit to reallocate resource for next test
	 ************************************************/
	SAMPLE_COMM_SYS_Exit();

	s32Ret = SAMPLE_COMM_SYS_GetPicSize(PIC_1080P, &stSize);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_SYS_GetPicSize failed with %#x\n", s32Ret);
		return s32Ret;
	}

	_get_max_vb_size(&stSizeVbDefault, stSize);
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

	stVbConf.u32MaxPoolCnt = 2;
	u32BlkSize = COMMON_GetPicBufferSize(stSizeVbDefault.u32Width, stSizeVbDefault.u32Height,
		PIXEL_FORMAT_RGB_888_PLANAR, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkRotSize = COMMON_GetPicBufferSize(stSizeVbDefault.u32Height, stSizeVbDefault.u32Width,
		PIXEL_FORMAT_RGB_888_PLANAR, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSize = MAX2(u32BlkSize, u32BlkRotSize);

	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 1;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	SAMPLE_PRT("common pool[0] BlkSize %d\n", u32BlkSize);
	u32BlkSize = COMMON_GetPicBufferSize(736, 1280,
		PIXEL_FORMAT_RGB_888_PLANAR, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkRotSize = COMMON_GetPicBufferSize(1280, 736,
		PIXEL_FORMAT_RGB_888_PLANAR, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSize = MAX2(u32BlkSize, u32BlkRotSize);

	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[1].u32BlkCnt	= 2;
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;
	SAMPLE_PRT("common pool[1] BlkSize %d\n", u32BlkSize);

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "system init failed with %#x\n", s32Ret);
		return s32Ret;
	}

	if (ops->dual_combo_test) {
		if (ops->dual_combo_test(fail_pause) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "*---------------------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------Golden YUV422/BGR packed 2k dual test failed------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*---------------------------------------------------------*\n");
			s32Ret = CVI_FAILURE;
		} else {
			CVI_TRACE_LOG(CVI_DBG_WARN, "*---------------------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------Golden YUV422/BGR packed 2k dual test PASS--------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*---------------------------------------------------------*\n");
		}
	}
	// test fd ops.
	CVI_S32 fd = CVI_VPSS_GetChnFd(9, 0);

	if (fd < 0) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnFd is fail\n");
		return CVI_FAILURE;
	}
#endif

	// stop all vpss.
	for (VpssGrp = 0; VpssGrp < VPSS_MAX_GRP_NUM; ++VpssGrp)
		SAMPLE_COMM_VPSS_Stop(VpssGrp, abChnEnable);
	return s32Ret;
}

static CVI_S32 _vpss_hw_stress_test(CVI_VOID)
{
	CVI_U32 u32BlkSize = 0;
	VB_CONFIG_S stVbConf;
	CVI_S32 s32Ret = CVI_SUCCESS;
	struct vpss_test_ops *ops;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = { CVI_TRUE, };

#if defined(ARCH_CV182X)
	ops = &vpss_test_ops_1822;
#elif defined(__CV181X__) || defined(__CV180X__) || defined(__CV186X__)
	ops = &vpss_test_ops_mars;
#else
	ops = &vpss_test_ops_1835;
#endif

	// Exit to reallocate resource for test
	SAMPLE_COMM_VI_DestroyIsp(&g_stViConfig);
	SAMPLE_COMM_VI_DestroyVi(&g_stViConfig);
	SAMPLE_COMM_SYS_Exit();

#if defined(ARCH_CV182X) || defined(__CV181X__) || defined(__CV180X__) || defined(__CV186X__)
	for (int pixFmt = 0; pixFmt < PIXEL_FORMAT_MAX; ++pixFmt) {
		CVI_U32 u32Size = COMMON_GetPicBufferSize(4000, 3000,
			pixFmt, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkSize = (u32BlkSize < u32Size) ? u32Size : u32BlkSize;
	}
#else
	for (int pixFmt = 0; pixFmt < PIXEL_FORMAT_MAX; ++pixFmt) {
		CVI_U32 u32Size = COMMON_GetPicBufferSize(VPSS_MAX_IMAGE_WIDTH, VPSS_MAX_IMAGE_HEIGHT,
			pixFmt, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
		u32BlkSize = (u32BlkSize < u32Size) ? u32Size : u32BlkSize;
	}
#endif

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt = 1;
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 2;
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	SAMPLE_PRT("common pool[0] BlkSize %d\n", u32BlkSize);

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "system init failed with %#x\n", s32Ret);
		return s32Ret;
	}

	if (ops->grp_attr_stress_test) {
		if (ops->grp_attr_stress_test() != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*------VPSS grp attr stress test failed-----*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			s32Ret = CVI_FAILURE;
		} else {
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*----  VPSS grp attr stress test PASS   ----*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		}
	}
	SAMPLE_COMM_VPSS_Stop(0, abChnEnable);

	if (ops->chn_attr_stress_test) {
		if (ops->chn_attr_stress_test() != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*------VPSS chn attr stress test failed-----*\n");
			CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
			s32Ret = CVI_FAILURE;
		} else {
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*----  VPSS chn attr stress test PASS   ----*\n");
			CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		}
	}

	SAMPLE_COMM_VPSS_Stop(0, abChnEnable);
	SAMPLE_COMM_SYS_Exit();
	return s32Ret;
}

static CVI_S32 _sys_test(CVI_VOID)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

#if 0
	if (CVI_SYS_FIFO_TEST() != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
		CVI_TRACE_LOG(CVI_DBG_ERR, "*--------    SYS FIFO test failed  ---------*\n");
		CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
		s32Ret = CVI_FAILURE;
	} else {
		CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		CVI_TRACE_LOG(CVI_DBG_WARN, "*---------      SYS FIFO PASS     ----------*\n");
		CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
	}
#endif

	if (CVI_SYS_FMT_TEST() != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
		CVI_TRACE_LOG(CVI_DBG_ERR, "*--------     SYS FMT test failed  ---------*\n");
		CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
		s32Ret = CVI_FAILURE;
	} else {
		CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		CVI_TRACE_LOG(CVI_DBG_WARN, "*---------       SYS FMT PASS     ----------*\n");
		CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
	}

	if (CVI_SYS_ION_TEST() != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
		CVI_TRACE_LOG(CVI_DBG_ERR, "*--------    SYS ION test failed   ---------*\n");
		CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
		s32Ret = CVI_FAILURE;
	} else {
		CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		CVI_TRACE_LOG(CVI_DBG_WARN, "*---------      SYS ION PASS      ----------*\n");
		CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
	}

	if (CVI_VB_TEST() != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
		CVI_TRACE_LOG(CVI_DBG_ERR, "*----------     VB test failed   -----------*\n");
		CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
		s32Ret = CVI_FAILURE;
	} else {
		CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		CVI_TRACE_LOG(CVI_DBG_WARN, "*----------        VB PASS       -----------*\n");
		CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
	}

	if (CVI_SYS_BIND_TEST() != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
		CVI_TRACE_LOG(CVI_DBG_ERR, "*---------- SYS BIND test failed -----------*\n");
		CVI_TRACE_LOG(CVI_DBG_ERR, "*-------------------------------------------*\n");
		s32Ret = CVI_FAILURE;
	} else {
		CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
		CVI_TRACE_LOG(CVI_DBG_WARN, "*----------     SYS BIND PASS   ------------*\n");
		CVI_TRACE_LOG(CVI_DBG_WARN, "*-------------------------------------------*\n");
	}

	return s32Ret;
}

static CVI_S32 _tuya_test(CVI_VOID)
{
	SIZE_S stSize = {.u32Width = 1080, .u32Height = 1920};
	VPSS_GRP_ATTR_S    stVpssGrpAttr;
	VPSS_GRP	   VpssGrp = 0;
	VPSS_CHN	   VpssChn = VPSS_CHN0;
	CVI_BOOL	   abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
	CVI_S32 s32Ret = CVI_SUCCESS;

	CVI_VI_SetChnRotation(0, 0, ROTATION_90);

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = PIXEL_FORMAT_YUV_PLANAR_420;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;

	VpssChn = 0;
	astVpssChnAttr[VpssChn].u32Width		    = 608;
	astVpssChnAttr[VpssChn].u32Height		    = 608;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_RGB_888_PLANAR;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 0;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	VpssChn = 1;
	astVpssChnAttr[VpssChn].u32Width		    = 480;
	astVpssChnAttr[VpssChn].u32Height		    = 640;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_BGR_888;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 0;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	VpssChn = 2;
	astVpssChnAttr[VpssChn].u32Width		    = 720;
	astVpssChnAttr[VpssChn].u32Height		    = 1280;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_YUV_PLANAR_420;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 0;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

#if defined(ARCH_CV183X) || defined(__CV181X__) || defined(__CV186X__)
	VpssChn = 3;
	astVpssChnAttr[VpssChn].u32Width		    = 320;
	astVpssChnAttr[VpssChn].u32Height		    = 320;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_RGB_888_PLANAR;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 0;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;
#endif

	/*start vpss*/
#if defined(ARCH_CV182X) || defined(__CV180X__)
	abChnEnable[0] = abChnEnable[1] = abChnEnable[2] = CVI_TRUE;
#elif defined(__CV181X__) || defined(__CV186X__)
	abChnEnable[0] = abChnEnable[1] = abChnEnable[2] = abChnEnable[3] = CVI_TRUE;
#else
	abChnEnable[0] = abChnEnable[1] = abChnEnable[2] = abChnEnable[3] = CVI_TRUE;
#endif
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VI_Bind_VPSS(0, 0, VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	VpssGrp = 1;
	VpssChn = 0;
	astVpssChnAttr[VpssChn].u32Width		    = 1080;
	astVpssChnAttr[VpssChn].u32Height		    = 1920;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_YUV_PLANAR_420;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 0;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = CVI_TRUE;
#if defined(ARCH_CV182X) || defined(__CV180X__)
	abChnEnable[1] = abChnEnable[2] = CVI_FALSE;
#elif defined(__CV181X__) || defined(__CV186X__)
	abChnEnable[1] = abChnEnable[2] = abChnEnable[3] = CVI_FALSE;
#else
	abChnEnable[1] = abChnEnable[2] = abChnEnable[3] = CVI_FALSE;
#endif
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VI_Bind_VPSS(0, 0, VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_PLAT_VO_INIT(1);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vo init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	SAMPLE_COMM_VPSS_Bind_VO(0, 2, 1, 0);

	return CVI_SUCCESS;
}

static CVI_S32 _vpss_dual_test(CVI_VOID)
{
	SIZE_S stSize = {.u32Width = 2560, .u32Height = 1440};
	VPSS_GRP_ATTR_S    stVpssGrpAttr;
	VPSS_GRP	   VpssGrp = 0;
	VPSS_CHN	   VpssChn = VPSS_CHN0;
	CVI_BOOL	   abChnEnable[VPSS_MAX_PHY_CHN_NUM] = { [0 ... VPSS_MAX_PHY_CHN_NUM - 1] = CVI_FALSE };
	VPSS_CHN_ATTR_S    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
	CVI_S32 s32Ret = CVI_SUCCESS;

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = PIXEL_FORMAT_YUV_PLANAR_422;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;

	VpssChn = 0;
	astVpssChnAttr[VpssChn].u32Width		    = 2560;
	astVpssChnAttr[VpssChn].u32Height		    = 1440;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_YUV_PLANAR_420;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 0;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	VpssChn = 1;
	astVpssChnAttr[VpssChn].u32Width		    = 1280;
	astVpssChnAttr[VpssChn].u32Height		    = 720;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_YUV_PLANAR_420;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 0;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	VpssChn = 2;
	astVpssChnAttr[VpssChn].u32Width		    = 2560;
	astVpssChnAttr[VpssChn].u32Height		    = 1440;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_YUV_PLANAR_420;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 0;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = abChnEnable[1] = abChnEnable[2] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VI_Bind_VPSS(0, 0, VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	VpssGrp = 1;
	VpssChn = 0;
	astVpssChnAttr[VpssChn].u32Width		    = 2560;
	astVpssChnAttr[VpssChn].u32Height		    = 1440;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_YUV_PLANAR_420;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 0;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = CVI_TRUE;
	abChnEnable[1] = abChnEnable[2] = CVI_FALSE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VI_Bind_VPSS(0, 0, VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 _gdc_test(CVI_VOID)
{
	SIZE_S stSize = {.u32Width = 1920, .u32Height = 1080};
	VPSS_GRP_ATTR_S    stVpssGrpAttr;
	VPSS_GRP	   VpssGrp = 0;
	VPSS_CHN	   VpssChn = VPSS_CHN0;
	CVI_BOOL	   abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
	CVI_S32 s32Ret = CVI_SUCCESS;
	VIDEO_FRAME_INFO_S stVideoFrame;
	VI_LDC_ATTR_S      stVILDCAttr = {
		.bEnable = CVI_TRUE,
		.stAttr = { .bAspect = CVI_TRUE, .s32XYRatio = 100,
			.s32CenterXOffset = 0, .s32CenterYOffset = 0, .s32DistortionRatio = -300 }
	};
	VPSS_LDC_ATTR_S    stVPSSLDCAttr = {
		.bEnable = CVI_TRUE,
		.stAttr = { .bAspect = CVI_TRUE, .s32XYRatio = 100,
			.s32CenterXOffset = 0, .s32CenterYOffset = 0, .s32DistortionRatio = -300 }
	};

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat		     = PIXEL_FORMAT_YUV_PLANAR_420;
	stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
	stVpssGrpAttr.u32MaxH			     = stSize.u32Height;

	VpssChn = 0;
	astVpssChnAttr[VpssChn].u32Width		    = 1920;
	astVpssChnAttr[VpssChn].u32Height		    = 1080;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_YUV_PLANAR_420;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 0;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	VpssChn = 1;
	astVpssChnAttr[VpssChn].u32Width		    = 1280;
	astVpssChnAttr[VpssChn].u32Height		    = 720;
	astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_YUV_PLANAR_420;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth		    = 0;
	astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
	astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
	astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = abChnEnable[1] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VI_Bind_VPSS(0, 0, VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
		return CVI_FAILURE;
	}

	usleep(3000 * 1000);

	// test vi rotation
	for (int j = 0; j < 100; ++j) {
		CVI_TRACE_LOG(CVI_DBG_INFO, "vi rotation stress test: %d\n", j);
		CVI_VI_SetChnRotation(0, 0, ROTATION_180);
		usleep(100 * 1000);
		if (CVI_VPSS_GetChnFrame(0, 0, &stVideoFrame, 1000) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vi rotation stress test NG on at %d\n", j);
			PAUSE();
		}
		CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrame);

		CVI_VI_SetChnRotation(0, 0, ROTATION_0);
		usleep(100 * 1000);
		if (CVI_VPSS_GetChnFrame(0, 0, &stVideoFrame, 1000) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vi rotation stress test NG off at %d\n", j);
			PAUSE();
		}
		CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrame);
	}

	// test vi ldc
	for (int j = 0; j < 100; ++j) {
		CVI_TRACE_LOG(CVI_DBG_INFO, "vi ldc stress test: %d\n", j);
		stVILDCAttr.bEnable = CVI_TRUE;
		CVI_VI_SetChnLDCAttr(0, 0, &stVILDCAttr);
		usleep(100 * 1000);
		if (CVI_VPSS_GetChnFrame(0, 0, &stVideoFrame, 1000) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vi rotation stress test NG on at %d\n", j);
			PAUSE();
		}
		CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrame);

		stVILDCAttr.bEnable = CVI_FALSE;
		CVI_VI_SetChnLDCAttr(0, 0, &stVILDCAttr);
		usleep(100 * 1000);
		if (CVI_VPSS_GetChnFrame(0, 0, &stVideoFrame, 1000) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vi rotation stress test NG off at %d\n", j);
			PAUSE();
		}
		CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrame);
	}

	// test vpss rotation
	for (int j = 0; j < 100; ++j) {
		CVI_TRACE_LOG(CVI_DBG_INFO, "vpss rotation stress test: %d\n", j);
		CVI_VPSS_SetChnRotation(0, 0, ROTATION_180);
		usleep(100 * 1000);
		if (CVI_VPSS_GetChnFrame(0, 0, &stVideoFrame, 1000) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vpss rotation stress test on NG at %d\n", j);
			PAUSE();
		}
		CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrame);

		CVI_VPSS_SetChnRotation(0, 0, ROTATION_0);
		usleep(100 * 1000);
		if (CVI_VPSS_GetChnFrame(0, 0, &stVideoFrame, 1000) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vpss rotation stress test off NG at %d\n", j);
			PAUSE();
		}
		CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrame);
	}

	// test vpss ldc
	for (int j = 0; j < 100; ++j) {
		CVI_TRACE_LOG(CVI_DBG_INFO, "vpss ldc stress test: %d\n", j);
		stVPSSLDCAttr.bEnable = CVI_TRUE;
		CVI_VPSS_SetChnLDCAttr(0, 0, &stVPSSLDCAttr);
		usleep(100 * 1000);
		if (CVI_VPSS_GetChnFrame(0, 0, &stVideoFrame, 1000) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vpss ldc stress test on NG at %d\n", j);
			PAUSE();
		}
		CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrame);

		stVPSSLDCAttr.bEnable = CVI_FALSE;
		CVI_VPSS_SetChnLDCAttr(0, 0, &stVPSSLDCAttr);
		usleep(100 * 1000);
		if (CVI_VPSS_GetChnFrame(0, 0, &stVideoFrame, 1000) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vpss ldc stress test off NG at %d\n", j);
			PAUSE();
		}
		CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrame);
	}

	stVILDCAttr.bEnable = CVI_TRUE;
	CVI_VI_SetChnLDCAttr(0, 0, &stVILDCAttr);
	for (int j = 0; j < 100; ++j) {
		CVI_TRACE_LOG(CVI_DBG_INFO, "vi ldc  + vpss rotation stress test: %d\n", j);
		CVI_VPSS_SetChnRotation(0, 0, ROTATION_180);

		usleep(100 * 1000);
		if (CVI_VPSS_GetChnFrame(0, 0, &stVideoFrame, 1000) != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vi rotation  + vpss ldc stress test NG at %d!\n", j);
			PAUSE();
		}
		CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrame);
		CVI_VPSS_SetChnRotation(0, 0, ROTATION_0);
		usleep(100 * 1000);
	}
	stVILDCAttr.bEnable = CVI_FALSE;
	CVI_VI_SetChnLDCAttr(0, 0, &stVILDCAttr);

	return CVI_SUCCESS;
}

static CVI_S32 _handle_vpss_op(CVI_S32 op)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U8 chns;
	VPSS_CROP_INFO_S pstCropInfo;
	int tmp;

	switch (op) {
	case 30: {
		break;
	}
	case 31: {
		VIDEO_FRAME_INFO_S stVideoFrame;

		printf("Which VPSS CHN: ");
		scanf("%d", &tmp);
		s32Ret = CVI_VPSS_GetChnFrame(0, tmp, &stVideoFrame, 1000);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnFrame failed with %#x\n", s32Ret);
			break;
		}

		SAMPLE_COMM_FRAME_SaveToFile("./dump.bin", &stVideoFrame);

		if (CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrame) != 0)
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_ReleaseChnFrame NG\n");

		break;
	}
	case 32:
		CVI_VPSS_GetGrpCrop(0, &pstCropInfo);
		printf("GrpCrop info:%d %d %d %d\n", pstCropInfo.stCropRect.s32X, pstCropInfo.stCropRect.s32Y
		       , pstCropInfo.stCropRect.u32Width, pstCropInfo.stCropRect.u32Height);
		break;
	case 33:
		CVI_TRACE_LOG(CVI_DBG_WARN, "Set GrpCrop. plz set input x:\n");
		scanf("%d", &pstCropInfo.stCropRect.s32X);
		CVI_TRACE_LOG(CVI_DBG_WARN, "input y:\n");
		scanf("%d", &pstCropInfo.stCropRect.s32Y);
		CVI_TRACE_LOG(CVI_DBG_WARN, "input width:\n");
		scanf("%d", &pstCropInfo.stCropRect.u32Width);
		CVI_TRACE_LOG(CVI_DBG_WARN, "input height:\n");
		scanf("%d", &pstCropInfo.stCropRect.u32Height);
		pstCropInfo.bEnable = CVI_TRUE;
		CVI_VPSS_SetGrpCrop(0, &pstCropInfo);
		CVI_TRACE_LOG(CVI_DBG_WARN, "GrpCrop info:%d %d %d %d\n",
				pstCropInfo.stCropRect.s32X, pstCropInfo.stCropRect.s32Y
				, pstCropInfo.stCropRect.u32Width, pstCropInfo.stCropRect.u32Height);
		break;
	case 34:
		CVI_VPSS_GetChnCrop(0, 0, &pstCropInfo);
		CVI_TRACE_LOG(CVI_DBG_WARN, "chnCrop info:%d %d %d %d\n",
				pstCropInfo.stCropRect.s32X, pstCropInfo.stCropRect.s32Y
				, pstCropInfo.stCropRect.u32Width, pstCropInfo.stCropRect.u32Height);
		break;
	case 35:
		CVI_TRACE_LOG(CVI_DBG_WARN, "Set Chn Crop. plz set input x:\n");
		scanf("%d", &pstCropInfo.stCropRect.s32X);
		CVI_TRACE_LOG(CVI_DBG_WARN, "input y:\n");
		scanf("%d", &pstCropInfo.stCropRect.s32Y);
		CVI_TRACE_LOG(CVI_DBG_WARN, "input width:\n");
		scanf("%d", &pstCropInfo.stCropRect.u32Width);
		CVI_TRACE_LOG(CVI_DBG_WARN, "input height:\n");
		scanf("%d", &pstCropInfo.stCropRect.u32Height);
		pstCropInfo.bEnable = CVI_TRUE;
		CVI_VPSS_SetChnCrop(0, 0, &pstCropInfo);
		break;
	case 36: {
		VPSS_GRP_ATTR_S    stVpssGrpAttr;
		VPSS_GRP	   VpssGrp = 0;
		VPSS_CHN	   VpssChn = VPSS_CHN0;
		CVI_BOOL	   abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
		VPSS_CHN_ATTR_S    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
		CVI_S32 s32Ret = CVI_SUCCESS;
		SIZE_S stSize;
		VPSS_CHN chns;

		CVI_TRACE_LOG(CVI_DBG_WARN, "--input size for vpss--\n");
		CVI_TRACE_LOG(CVI_DBG_WARN, "width: ");
		scanf("%d", &stSize.u32Width);
		CVI_TRACE_LOG(CVI_DBG_WARN, "height: ");
		scanf("%d", &stSize.u32Height);

		CVI_TRACE_LOG(CVI_DBG_WARN, "how many chns(1~4): ");
		scanf("%d", &chns);

		stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
		stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
		stVpssGrpAttr.enPixelFormat		     = PIXEL_FORMAT_YUV_PLANAR_420;
		stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
		stVpssGrpAttr.u32MaxH			     = stSize.u32Height;

		for (VpssChn = VPSS_CHN0; VpssChn < chns; ++VpssChn) {
			CVI_TRACE_LOG(CVI_DBG_WARN, "--output size for vpss chn(%d)--\n", VpssChn);
			CVI_TRACE_LOG(CVI_DBG_WARN, "width: ");
			scanf("%d", &stSize.u32Width);
			CVI_TRACE_LOG(CVI_DBG_WARN, "height: ");
			scanf("%d", &stSize.u32Height);

			astVpssChnAttr[VpssChn].u32Width		    = stSize.u32Width;
			astVpssChnAttr[VpssChn].u32Height		    = stSize.u32Height;

			astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
			astVpssChnAttr[VpssChn].enPixelFormat		    = PIXEL_FORMAT_YUV_PLANAR_420;
			astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
			astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
			astVpssChnAttr[VpssChn].u32Depth		    = 0;
			astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
			astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
			astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
			astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
			astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
			astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

			abChnEnable[VpssChn] = CVI_TRUE;
		}

		/*start vpss*/
		s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
			break;
		}

		s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
			break;
		}

		s32Ret = SAMPLE_COMM_VI_Bind_VPSS(0, 0, VpssGrp);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
			break;
		}
		break;
	}
	case 37: {
		VPSS_GRP_ATTR_S    stVpssGrpAttr;
		VPSS_GRP	   VpssGrp = 0;
		VPSS_CHN	   VpssChn = VPSS_CHN0;
		CVI_BOOL	   abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
		VPSS_CHN_ATTR_S    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
		CVI_S32 s32Ret = CVI_SUCCESS;
		SIZE_S stSize;
		VPSS_CHN chns;

		CVI_TRACE_LOG(CVI_DBG_WARN, "--input size for vpss--\n");
		CVI_TRACE_LOG(CVI_DBG_WARN, "width: ");
		scanf("%d", &stSize.u32Width);
		CVI_TRACE_LOG(CVI_DBG_WARN, "height: ");
		scanf("%d", &stSize.u32Height);

		CVI_TRACE_LOG(CVI_DBG_WARN, "how many chns(1~%d): ", VPSS_MAX_PHY_CHN_NUM - 1);
		scanf("%d", &chns);

		stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
		stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
		stVpssGrpAttr.enPixelFormat		     = SAMPLE_PIXEL_FORMAT;
		stVpssGrpAttr.u32MaxW			     = stSize.u32Width;
		stVpssGrpAttr.u32MaxH			     = stSize.u32Height;

		for (VpssChn = VPSS_CHN0; VpssChn < chns; ++VpssChn) {
			CVI_TRACE_LOG(CVI_DBG_WARN, "--output size for vpss chn(%d)--\n", VpssChn);
			CVI_TRACE_LOG(CVI_DBG_WARN, "width: ");
			scanf("%d", &stSize.u32Width);
			CVI_TRACE_LOG(CVI_DBG_WARN, "height: ");
			scanf("%d", &stSize.u32Height);

			astVpssChnAttr[VpssChn].u32Width		    = stSize.u32Width;
			astVpssChnAttr[VpssChn].u32Height		    = stSize.u32Height;

			astVpssChnAttr[VpssChn].enVideoFormat		    = VIDEO_FORMAT_LINEAR;
			astVpssChnAttr[VpssChn].enPixelFormat		    = SAMPLE_PIXEL_FORMAT;
			astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
			astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
			astVpssChnAttr[VpssChn].u32Depth		    = 0;
			astVpssChnAttr[VpssChn].bMirror			    = CVI_FALSE;
			astVpssChnAttr[VpssChn].bFlip			    = CVI_FALSE;
			astVpssChnAttr[VpssChn].stAspectRatio.enMode	    = ASPECT_RATIO_AUTO;
			astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor = CVI_TRUE;
			astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
			astVpssChnAttr[VpssChn].stNormalize.bEnable	    = CVI_FALSE;

			abChnEnable[VpssChn] = CVI_TRUE;
		}

		/*start vpss*/
		s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
			break;
		}

		s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
			break;
		}

		s32Ret = SAMPLE_COMM_VI_Bind_VPSS(0, 0, VpssGrp);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
			break;
		}
		break;
	}
	case 38: {
		SIZE_S stSize;

		CVI_TRACE_LOG(CVI_DBG_WARN, "--input size for vpss--\n");
		CVI_TRACE_LOG(CVI_DBG_WARN, "width: ");
		scanf("%d", &stSize.u32Width);
		CVI_TRACE_LOG(CVI_DBG_WARN, "height: ");
		scanf("%d", &stSize.u32Height);

		chns = 1;
		_vpss_start_bind_vi(0, stSize, chns);
		_vpss_start_bind_vi(1, stSize, chns);
		break;
	}
	case 39: {
		chns = 1;
		_vpss_stop_unbind_vi(0);
		_vpss_stop_unbind_vi(1);
		break;
	}
	case 40: {
		SIZE_S stSize;

		CVI_TRACE_LOG(CVI_DBG_WARN, "img width: ");
		scanf("%d", &stSize.u32Width);
		CVI_TRACE_LOG(CVI_DBG_WARN, "img height: ");
		scanf("%d", &stSize.u32Height);

		SAMPLE_COMM_VPSS_SendFrame(0, &stSize, SAMPLE_PIXEL_FORMAT, "./sample.yuv");
		break;
	}
	case 41: {
		int sel;

		SAMPLE_PRT("Rotation 0(0)/1(90)/2(180)/3(270): ");
		scanf("%d", &sel);
		CVI_VPSS_SetChnRotation(0, 0, sel);
		break;
	}
	case 42: {
		VPSS_GRP_ATTR_S stGrpAttr;
		VPSS_CHN_ATTR_S stChnAttr;
		ROTATION_E enRotation;

		s32Ret = CVI_VPSS_GetGrpAttr(0, &stGrpAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetGrpAttr is fail\n");
			return s32Ret;
		}

		s32Ret = CVI_VPSS_GetChnAttr(0, 0, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnAttr is fail\n");
			return s32Ret;
		}

		s32Ret = CVI_VPSS_GetChnRotation(0, 0, &enRotation);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnRotation is fail\n");
			return s32Ret;
		}

		CVI_TRACE_LOG(CVI_DBG_WARN, "-- VPSS Grp info --\n");
		CVI_TRACE_LOG(CVI_DBG_WARN, "Max width(%d) height(%d)\n", stGrpAttr.u32MaxW, stGrpAttr.u32MaxH);
		CVI_TRACE_LOG(CVI_DBG_WARN, "PxlFormat(%d)\n", stGrpAttr.enPixelFormat);
		CVI_TRACE_LOG(CVI_DBG_WARN, "frame-rate src(%d) dst(%d)\n"
			, stGrpAttr.stFrameRate.s32SrcFrameRate, stGrpAttr.stFrameRate.s32DstFrameRate);
		CVI_TRACE_LOG(CVI_DBG_WARN, "-- VPSS Chn info --\n");
		CVI_TRACE_LOG(CVI_DBG_WARN, "width(%d) height(%d)\n", stChnAttr.u32Width, stChnAttr.u32Height);
		CVI_TRACE_LOG(CVI_DBG_WARN, "VideoFormat(%d) PxlFormat(%d) Depth(%d)\n"
			, stChnAttr.enVideoFormat, stChnAttr.enPixelFormat, stChnAttr.u32Depth);
		CVI_TRACE_LOG(CVI_DBG_WARN, "Flip(%d) Mirror(%d)\n", stChnAttr.bFlip, stChnAttr.bMirror);
		CVI_TRACE_LOG(CVI_DBG_WARN, "frame-rate src(%d) dst(%d)\n"
			, stChnAttr.stFrameRate.s32SrcFrameRate, stChnAttr.stFrameRate.s32DstFrameRate);
		CVI_TRACE_LOG(CVI_DBG_WARN, "aspect ratio mode(%d) bgcolor(%#x)\n"
			, stChnAttr.stAspectRatio.enMode, stChnAttr.stAspectRatio.u32BgColor);
		if (stChnAttr.stAspectRatio.enMode == ASPECT_RATIO_MANUAL)
			CVI_TRACE_LOG(CVI_DBG_WARN, "manual rect(%d %d %d %d)\n"
				, stChnAttr.stAspectRatio.stVideoRect.s32X, stChnAttr.stAspectRatio.stVideoRect.s32Y
				, stChnAttr.stAspectRatio.stVideoRect.u32Width
				, stChnAttr.stAspectRatio.stVideoRect.u32Height);

		CVI_TRACE_LOG(CVI_DBG_WARN, "VPSS get info TEST-PASS\n");
		break;
	}
	case 44: {
		PROC_AMP_E type;
		PROC_AMP_CTRL_S ctrl;
		CVI_S32 cur;

		SAMPLE_PRT("Which ProcAmp to get 0(brightness)/1(contrast)/2(saturation)/3(hue): ");
		scanf("%d", &tmp);
		type = tmp;
		s32Ret = CVI_VPSS_GetGrpProcAmpCtrl(0, type, &ctrl);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetGrpProcAmpCtrl is fail\n");
			return s32Ret;
		}
		s32Ret = CVI_VPSS_GetGrpProcAmp(0, type, &cur);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetGrpProcAmp is fail\n");
			return s32Ret;
		}
		CVI_TRACE_LOG(CVI_DBG_WARN, "min(%d) max(%d) step(%d) default(%d) current(%d)\n"
			, ctrl.minimum, ctrl.maximum, ctrl.step, ctrl.default_value, cur);
		CVI_TRACE_LOG(CVI_DBG_WARN, "VPSS fd ctrl TEST-PASS\n");
		break;
	}
	case 45: {
		PROC_AMP_E type;
		CVI_S32 value;

		SAMPLE_PRT("Which ProcAmp to set 0(brightness)/1(contrast)/2(saturation)/3(hue): ");
		scanf("%d", &tmp);
		type = tmp;
		SAMPLE_PRT("new value: ");
		scanf("%d", &value);
		s32Ret = CVI_VPSS_SetGrpProcAmp(0, type, value);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_SetGrpProcAmp is fail\n");
			return s32Ret;
		}
		break;
	}
	case 46: {
		VPSS_CHN_ATTR_S stChnAttr;
		CVI_S32 value;

		s32Ret = CVI_VPSS_GetChnAttr(0, 0, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnAttr is fail\n");
			return s32Ret;
		}

		SAMPLE_PRT("Aspect Ratio 0(None)/1(AUTO)/2(MANUAL): ");
		scanf("%d", &value);
		stChnAttr.stAspectRatio.enMode = value;
		if ((value == ASPECT_RATIO_AUTO) || (value == ASPECT_RATIO_MANUAL)) {
			SAMPLE_PRT("BG Color 0(Disable)/1(Enable): ");
			scanf("%d", &value);
			stChnAttr.stAspectRatio.bEnableBgColor = value;
		}
		if (stChnAttr.stAspectRatio.enMode == ASPECT_RATIO_MANUAL) {
			SAMPLE_PRT("Manual acpect ratio. plz set input x:\n");
			scanf("%d", &stChnAttr.stAspectRatio.stVideoRect.s32X);
			SAMPLE_PRT("input y:\n");
			scanf("%d", &stChnAttr.stAspectRatio.stVideoRect.s32Y);
			SAMPLE_PRT("input width:\n");
			scanf("%d", &stChnAttr.stAspectRatio.stVideoRect.u32Width);
			SAMPLE_PRT("input height:\n");
			scanf("%d", &stChnAttr.stAspectRatio.stVideoRect.u32Height);
		}
		s32Ret = CVI_VPSS_SetChnAttr(0, 0, &stChnAttr);
		break;
	}
	case 47: {
		VPSS_CHN_ATTR_S stChnAttr;
		CVI_S32 value;

		s32Ret = CVI_VPSS_GetChnAttr(0, 0, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VPSS_GetChnAttr is fail\n");
			return s32Ret;
		}

		SAMPLE_PRT("Flip 0(Disable)/1(Enable): ");
		scanf("%d", &value);
		stChnAttr.bFlip = value;
		SAMPLE_PRT("Mirror 0(Disable)/1(Enable): ");
		scanf("%d", &value);
		stChnAttr.bMirror = value;
		s32Ret = CVI_VPSS_SetChnAttr(0, 0, &stChnAttr);
		break;
	}
	case 48: {
		CVI_FLOAT ratio;

		SAMPLE_PRT("Y Ratio: ");
		scanf("%f", &ratio);
		s32Ret = CVI_VPSS_SetChnYRatio(0, 0, ratio);
		break;
	}
	case 49: {
		VPSS_LDC_ATTR_S stLDCAttr;

		SAMPLE_PRT("Enable 1(Y)/0(N): ");
		scanf("%d", &tmp);
		stLDCAttr.bEnable = tmp;
		if (stLDCAttr.bEnable) {
			SAMPLE_PRT("Keep AspectRatio 1(Y)/0(N): ");
			scanf("%d", &tmp);
			stLDCAttr.stAttr.bAspect = tmp;
			if (stLDCAttr.stAttr.bAspect) {
				SAMPLE_PRT("Ratio (0 ~ 100): ");
				scanf("%d", &tmp);
				stLDCAttr.stAttr.s32XYRatio = tmp;
			} else {
				SAMPLE_PRT("XRatio (0 ~ 100): ");
				scanf("%d", &stLDCAttr.stAttr.s32XRatio);
				SAMPLE_PRT("YRatio (0 ~ 100): ");
				scanf("%d", &stLDCAttr.stAttr.s32YRatio);
			}
			SAMPLE_PRT("XOffset (-511 ~ 511): ");
			scanf("%d", &stLDCAttr.stAttr.s32CenterXOffset);
			SAMPLE_PRT("YOffset (-511 ~ 511): ");
			scanf("%d", &stLDCAttr.stAttr.s32CenterYOffset);
			SAMPLE_PRT("DistortionRatio (-300 ~ 500): ");
			scanf("%d", &stLDCAttr.stAttr.s32DistortionRatio);
		}
		s32Ret = CVI_VPSS_SetChnLDCAttr(0, 0, &stLDCAttr);
		break;
	}

	}
	return s32Ret;
}

static CVI_S32 _vo_sendframe(VO_LAYER VoLayer, SIZE_S *stSize, CVI_CHAR *filename)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	FILE *fp;

	if (SAMPLE_COMM_PrepareFrame(*stSize, SAMPLE_PIXEL_FORMAT, &stVideoFrame) != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_PrepareFrame failed\n");
		return CVI_FAILURE;
	}

	stVideoFrame.stVFrame.pu8VirAddr[0]
		= CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0], stVideoFrame.stVFrame.u32Length[0]);
	stVideoFrame.stVFrame.pu8VirAddr[1]
		= CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[1], stVideoFrame.stVFrame.u32Length[1]);
	stVideoFrame.stVFrame.pu8VirAddr[2]
		= CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[2], stVideoFrame.stVFrame.u32Length[2]);

	CVI_TRACE_LOG(CVI_DBG_WARN, "phy addr(%#"PRIx64", %#"PRIx64", %#"PRIx64"\n",
		stVideoFrame.stVFrame.u64PhyAddr[0], stVideoFrame.stVFrame.u64PhyAddr[1],
		stVideoFrame.stVFrame.u64PhyAddr[2]);
	CVI_TRACE_LOG(CVI_DBG_WARN, "vir addr(%p, %p, %p\n", stVideoFrame.stVFrame.pu8VirAddr[0]
		, stVideoFrame.stVFrame.pu8VirAddr[1], stVideoFrame.stVFrame.pu8VirAddr[2]);

	fp = fopen(filename, "r");
	for (int i = 0; i < 3; i++) {
		fread((void *)stVideoFrame.stVFrame.pu8VirAddr[i]
			, stVideoFrame.stVFrame.u32Length[i], 1, fp);
	}
	fclose(fp);

	CVI_VO_SendFrame(VoLayer, 0, &stVideoFrame, -1);
	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(stVideoFrame.stVFrame.u64PhyAddr[0]));
	return CVI_SUCCESS;
}

static long diff_in_us(struct timespec t1, struct timespec t2)
{
	struct timespec diff;

	if (t2.tv_nsec-t1.tv_nsec < 0) {
		diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
		diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
	} else {
		diff.tv_sec  = t2.tv_sec - t1.tv_sec;
		diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
	}
	return (diff.tv_sec * 1000000.0 + diff.tv_nsec / 1000.0);
}

static void ViConfigReInit(SAMPLE_VI_CONFIG_S *p_stViConfig, SAMPLE_INI_CFG_S *p_stIniCfg)
{
	int s32WorkSnsId = 0;

	for (s32WorkSnsId = 0; s32WorkSnsId < p_stIniCfg->devNum; s32WorkSnsId++) {
		p_stViConfig->s32WorkingViNum					= 1 + s32WorkSnsId;
		p_stViConfig->as32WorkingViId[s32WorkSnsId]			= s32WorkSnsId;
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.enSnsType	= p_stIniCfg->enSnsType[s32WorkSnsId];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.MipiDev		= p_stIniCfg->MipiDev[s32WorkSnsId];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.s32BusId	= p_stIniCfg->s32BusId[s32WorkSnsId];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[0]   =
			p_stIniCfg->as16LaneId[s32WorkSnsId][0];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[1]   =
			p_stIniCfg->as16LaneId[s32WorkSnsId][1];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[2]   =
			p_stIniCfg->as16LaneId[s32WorkSnsId][2];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[3]   =
			p_stIniCfg->as16LaneId[s32WorkSnsId][3];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as16LaneId[4]   =
			p_stIniCfg->as16LaneId[s32WorkSnsId][4];

		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[0]	= p_stIniCfg->as8PNSwap[s32WorkSnsId][0];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[1]	= p_stIniCfg->as8PNSwap[s32WorkSnsId][1];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[2]	= p_stIniCfg->as8PNSwap[s32WorkSnsId][2];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[3]	= p_stIniCfg->as8PNSwap[s32WorkSnsId][3];
		p_stViConfig->astViInfo[s32WorkSnsId].stSnsInfo.as8PNSwap[4]	= p_stIniCfg->as8PNSwap[s32WorkSnsId][4];
		p_stViConfig->astViInfo[s32WorkSnsId].stDevInfo.enWDRMode	= p_stIniCfg->enWDRMode[s32WorkSnsId];
	}
}

#ifndef FPGA_PORTING
static CVI_S32 _isp_test_init(CVI_VOID)
{
	CVI_U32 waitTime = 500, waitFrame = 100;
	ISP_EXPOSURE_ATTR_S tempAeAttr;
	ISP_WB_ATTR_S tmpAwbAttr;
	ISP_DCI_ATTR_S dciAttr;
	ISP_STATISTICS_CFG_S stsCfg;
	ISP_PUB_ATTR_S stPubAttr;
	CVI_S32 s32Ret = 0;
	CVI_U32 i;
	// set manual awb.
	CVI_ISP_GetWBAttr(0, &tmpAwbAttr);
	tmpAwbAttr.enOpType = OP_TYPE_MANUAL;
	tmpAwbAttr.stManual.u16Rgain = tmpAwbAttr.stManual.u16Grgain = tmpAwbAttr.stManual.u16Gbgain =
		tmpAwbAttr.stManual.u16Bgain = 1024;
	tmpAwbAttr.u8AWBRunInterval = 1;
	CVI_ISP_SetWBAttr(0, &tmpAwbAttr);
	// set manual ae.
	CVI_ISP_GetExposureAttr(0, &tempAeAttr);
	tempAeAttr.stManual.enExpTimeOpType = OP_TYPE_MANUAL;
	tempAeAttr.stManual.enAGainOpType = OP_TYPE_MANUAL;
	tempAeAttr.stManual.enDGainOpType = OP_TYPE_MANUAL;
	tempAeAttr.stManual.enISPDGainOpType = OP_TYPE_MANUAL;
	tempAeAttr.stManual.enISONumOpType = OP_TYPE_MANUAL;
	tempAeAttr.enOpType = OP_TYPE_MANUAL;
	tempAeAttr.stManual.u32ExpTime = 16384;
	tempAeAttr.stManual.u32AGain = 1024;
	tempAeAttr.stManual.u32AGain = tempAeAttr.stManual.u32ISPDGain = 1024;
	tempAeAttr.stManual.u32ISONum = 100;
	CVI_ISP_SetExposureAttr(0, &tempAeAttr);
	// Close DCI for IIR.
	CVI_ISP_GetDCIAttr(0, &dciAttr);
	dciAttr.Enable = 0;
	CVI_ISP_SetDCIAttr(0, &dciAttr);
	CVI_ISP_GetPubAttr(0, &stPubAttr);
	// set default statistics config.
	stsCfg.stAECfg.stCrop[0].bEnable = 0;
	stsCfg.stAECfg.stCrop[0].u16X = stsCfg.stAECfg.stCrop[0].u16Y = 0;
	stsCfg.stAECfg.stCrop[0].u16W = stPubAttr.stWndRect.u32Width;
	stsCfg.stAECfg.stCrop[0].u16H = stPubAttr.stWndRect.u32Height;
	#ifdef ARCH_CV183X
	stsCfg.stAECfg.stCrop[1].bEnable = 0;
	stsCfg.stAECfg.stCrop[1].u16X = stsCfg.stAECfg.stCrop[1].u16Y = 0;
	stsCfg.stAECfg.stCrop[1].u16W = stPubAttr.stWndRect.u32Width;
	stsCfg.stAECfg.stCrop[1].u16H = stPubAttr.stWndRect.u32Height;
	#endif
	stsCfg.stWBCfg.u16ZoneRow = AWB_ZONE_ORIG_ROW;
	stsCfg.stWBCfg.u16ZoneCol = AWB_ZONE_ORIG_COLUMN;
	stsCfg.stWBCfg.stCrop.bEnable = 0;
	stsCfg.stWBCfg.stCrop.u16X = stsCfg.stWBCfg.stCrop.u16Y = 0;
	stsCfg.stWBCfg.stCrop.u16W = stPubAttr.stWndRect.u32Width;
	stsCfg.stWBCfg.stCrop.u16H = stPubAttr.stWndRect.u32Height;
	stsCfg.stWBCfg.u16BlackLevel = 0;
	stsCfg.stWBCfg.u16WhiteLevel = 4095;
	stsCfg.stFocusCfg.stConfig.bEnable = 1;
	stsCfg.stFocusCfg.stConfig.u8HFltShift = 1;
	stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[0] = 1;
	stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[1] = 2;
	stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[2] = 3;
	stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[3] = 5;
	stsCfg.stFocusCfg.stConfig.s8HVFltLpCoeff[4] = 10;
	stsCfg.stFocusCfg.stConfig.stRawCfg.PreGammaEn = 0;
	stsCfg.stFocusCfg.stConfig.stPreFltCfg.PreFltEn = 1;
	stsCfg.stFocusCfg.stConfig.u16Hwnd = 17;
	stsCfg.stFocusCfg.stConfig.u16Vwnd = 15;
	stsCfg.stFocusCfg.stConfig.stCrop.bEnable = 0;
	// AF offset and size has some limitation.
	stsCfg.stFocusCfg.stConfig.stCrop.u16X = AF_XOFFSET_MIN;
	stsCfg.stFocusCfg.stConfig.stCrop.u16Y = AF_YOFFSET_MIN;
	stsCfg.stFocusCfg.stConfig.stCrop.u16W = stPubAttr.stWndRect.u32Width - AF_XOFFSET_MIN * 2;
	stsCfg.stFocusCfg.stConfig.stCrop.u16H = stPubAttr.stWndRect.u32Height - AF_YOFFSET_MIN * 2;
	stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[0] = 1;
	stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[1] = 4;
	stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[2] = 8;
	stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[3] = 16;
	stsCfg.stFocusCfg.stHParam_FIR0.s8HFltHpCoeff[4] = 0;
	stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[0] = 1;
	stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[1] = 2;
	stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[2] = 4;
	stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[3] = 8;
	stsCfg.stFocusCfg.stHParam_FIR1.s8HFltHpCoeff[4] = 0;
	stsCfg.stFocusCfg.stVParam_FIR.s8VFltHpCoeff[0] = 1;
	stsCfg.stFocusCfg.stVParam_FIR.s8VFltHpCoeff[1] = 16;
	stsCfg.stFocusCfg.stVParam_FIR.s8VFltHpCoeff[2] = 0;
	stsCfg.unKey.bit1FEAeGloStat = stsCfg.unKey.bit1FEAeLocStat =
		stsCfg.unKey.bit1AwbStat1 = stsCfg.unKey.bit1AwbStat2 = stsCfg.unKey.bit1FEAfStat = 1;
	s32Ret = CVI_ISP_SetStatisticsConfig(0, &stsCfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "ISP Set Statistic failed with %#x!\n", s32Ret);
		return s32Ret;
	}
	// First wait some frame for start not stable.
	for (i = 0; i < waitFrame; i++) {
		CVI_ISP_GetVDTimeOut(0, ISP_VD_FE_START, waitTime);
	}
	return s32Ret;
}

static CVI_S32 _isp_test(CVI_VOID)
{
#define ISP_TEST_ITEM(_testItem, failLog) \
	{if (_testItem) { \
		if (_testItem() != CVI_SUCCESS) { \
			SAMPLE_PRT("*-------------------------------------------*\n"); \
			SAMPLE_PRT("*---------  %s failed ---------*\n", failLog); \
			SAMPLE_PRT("*-------------------------------------------*\n"); \
			s32Ret = CVI_FAILURE; \
		} else { \
			SAMPLE_PRT("*-------------------------------------------*\n"); \
			SAMPLE_PRT("*---------  %s success   ---------*\n", failLog); \
			SAMPLE_PRT("*-------------------------------------------*\n"); \
		} } }

	struct isp_test_ops *ops;
	CVI_S32 s32Ret = CVI_SUCCESS;

	ops = &isp_api_test;
	_isp_test_init();

	ISP_TEST_ITEM(ops->aaa_bind_test, "CVI_ISP_BindAttr test");
	ISP_TEST_ITEM(ops->fw_state_test, "CVI_ISP_FMWState test");
	ISP_TEST_ITEM(ops->statistic_test, "3a statistic test");
	ISP_TEST_ITEM(ops->module_ctrl_test, "CVI_ISP_ModuleControl test");
	ISP_TEST_ITEM(ops->reg_set_test, "CVI_ISP_Register test");
	ISP_TEST_ITEM(ops->dcf_info_test, "CVI_ISP_DCFInfo test");

	return s32Ret;
}

static CVI_S32 _isp_raw_replay_test(CVI_VOID)
{
	struct isp_test_ops *ops;
	CVI_S32 s32Ret = CVI_SUCCESS;

	ops = &isp_api_test;

	ISP_TEST_ITEM(ops->raw_replay_test, "isp rew replay test");

	return s32Ret;
}
#endif

static CVI_S32 _handle_vb_op(CVI_S32 op)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	int tmp;

	switch (op) {
	case 90: {
		VB_POOL_CONFIG_S cfg;
		VB_POOL PoolID;

		printf("u32BlkSize =");
		scanf("%d", &tmp);
		cfg.u32BlkSize = tmp;
		printf("u32BlkCnt =");
		scanf("%d", &tmp);
		cfg.u32BlkCnt = tmp;
		PoolID = CVI_VB_CreatePool(&cfg);
		if (PoolID == VB_INVALID_POOLID) {
			printf("CVI_VB_CreatePool failed\n");
			s32Ret = CVI_FAILURE;
		} else {
			printf("Get PoolID(%d)\n", PoolID);
		}
		break;
	}
	case 91: {
		VB_POOL PoolID;

		printf("The Pool to destroy =");
		scanf("%d", &tmp);
		PoolID = tmp;
		s32Ret = CVI_VB_DestroyPool(PoolID);
		break;
	}
	case 92: {
		VB_POOL PoolID;
		VB_BLK blk;

		printf("The Pool to get block =");
		scanf("%d", &tmp);
		PoolID = tmp;
		printf("The size of block =");
		scanf("%d", &tmp);
		blk = CVI_VB_GetBlock(PoolID, tmp);
		if (blk == VB_INVALID_HANDLE) {
			printf("CVI_VB_GetBlock failed\n");
			s32Ret = CVI_FAILURE;
		} else {
			printf("Get VB_BLK(%zu)\n", blk);
		}
		break;
	}
	case 93: {
		printf("The Blk to release =");
		scanf("%d", &tmp);
		CVI_VB_ReleaseBlock(tmp);
		break;
	}
	}
	return s32Ret;
}

static CVI_S32 _handle_op(CVI_S32 op, SAMPLE_INI_CFG_S *pstIniCfg, SAMPLE_VI_CONFIG_S *pstViConfig)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	static VB_BLK vi_usr_blk;
	struct vi_test_ops *vi_ops = &viOps;
	int tmp;
	UNUSED(pstIniCfg);

	if (op >= 30 && op < 50)
		return _handle_vpss_op(op);
	if (op >= 90 && op < 100)
		return _handle_vb_op(op);

	switch (op) {
	case 0: {
		ViConfigReInit(&g_stViConfig, &g_stIniCfg);

		if (vi_ops->vi_open) {
			s32Ret = vi_ops->vi_open();
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "vi open failed. s32Ret: 0x%x !\n", s32Ret);
				return s32Ret;
			}
		}

		if (vi_ops->vi_init) {
			s32Ret = vi_ops->vi_init(&g_stViConfig);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "vi init failed. s32Ret: 0x%x !\n", s32Ret);
				return s32Ret;
			}
		}
		break;
	}
	case 1: {
		if (vi_ops->vi_close) {
			s32Ret = vi_ops->vi_close(&g_stViConfig);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "vi close failed. s32Ret: 0x%x !\n", s32Ret);
				return s32Ret;
			}
		}
		break;
	}
	case 2: {
		CVI_S32 loop = 0;
		CVI_U32 ok = 0, ng = 0;
		CVI_U8  chn = 0;
		struct timespec start, end;

		CVI_TRACE_LOG(CVI_DBG_WARN, "Get frm from which chn(0~1): ");
		scanf("%d", &tmp);
		chn = tmp;
		CVI_TRACE_LOG(CVI_DBG_WARN, "how many loops to do(11111 is infinite: ");
		scanf("%d", &loop);
		while (loop > 0) {
			clock_gettime(CLOCK_MONOTONIC, &start);
			if (_vi_get_chn_frame(chn) == CVI_SUCCESS) {
				++ok;
				clock_gettime(CLOCK_MONOTONIC, &end);
				CVI_TRACE_LOG(CVI_DBG_WARN, "ms consumed: %f\n",
							(CVI_FLOAT)diff_in_us(start, end)/1000);
			} else
				++ng;
			//sleep(1);
			if (loop != 11111)
				loop--;
		}
		CVI_TRACE_LOG(CVI_DBG_WARN, "VI GetChnFrame OK(%d) NG(%d)\n", ok, ng);

		CVI_TRACE_LOG(CVI_DBG_WARN, "Dump VI yuv TEST-PASS\n");
		break;
	}
	case 3: {
		CVI_U8 chnID;

		CVI_TRACE_LOG(CVI_DBG_WARN, "ChnID: ");
		scanf("%d", &tmp);
		chnID = tmp;

		SAMPLE_COMM_VI_UnBind_VPSS(0, (chnID ^ 1), 0);
		SAMPLE_COMM_VI_Bind_VPSS(0, chnID, 0);
		break;
	}
	case 5: {
		SIZE_S stSizeIn, stSizeOut;

		CVI_TRACE_LOG(CVI_DBG_WARN, "--input size for vpss--\n");
		CVI_TRACE_LOG(CVI_DBG_WARN, "width: ");
		scanf("%d", &stSizeIn.u32Width);
		CVI_TRACE_LOG(CVI_DBG_WARN, "height: ");
		scanf("%d", &stSizeIn.u32Height);
		CVI_TRACE_LOG(CVI_DBG_WARN, "--output size for vpss--\n");
		CVI_TRACE_LOG(CVI_DBG_WARN, "width: ");
		scanf("%d", &stSizeOut.u32Width);
		CVI_TRACE_LOG(CVI_DBG_WARN, "height: ");
		scanf("%d", &stSizeOut.u32Height);

		s32Ret = SAMPLE_PLAT_VPSS_INIT(0, stSizeIn, stSizeOut);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vpss init failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}

		s32Ret = SAMPLE_COMM_VI_Bind_VPSS(0, 0, 0);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
		break;
	}
	case 6: {
		_vpss_stop_unbind_vi(0);
		break;
	}

	case 7: {
		VO_DEV VoDev;
		VO_LAYER VoLayer;

		CVI_TRACE_LOG(CVI_DBG_WARN, "vo device:(0/1)\n");
		scanf("%d", &VoDev);
		VoDev %= 2;
		s32Ret = SAMPLE_PLAT_VO_INIT(VoDev);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vo init failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
		VoLayer = VoDev;

		CVI_VO_SetChnRotation(VoLayer, 0, ROTATION_90);

		SAMPLE_COMM_VPSS_Bind_VO(0, 0, VoLayer, 0);
		break;
	}
	case 8: {
		SAMPLE_VO_CONFIG_S stVoConfig;

		s32Ret = SAMPLE_COMM_VO_GetDefConfig(&stVoConfig);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VO_GetDefConfig failed with %#x\n", s32Ret);
			break;
		}

		SAMPLE_COMM_VPSS_UnBind_VO(0, 0, 0, 0);

		s32Ret = SAMPLE_COMM_VO_StopVO(&stVoConfig);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VO_StopVO failed with %#x\n", s32Ret);
			break;
		}
		break;
	}

	case 9: {
		VI_VPSS_MODE_S stVIVPSSMode;

		CVI_TRACE_LOG(CVI_DBG_WARN, "VI_OFFLINE_VPSS_OFFLINE(0)/VI_OFFLINE_VPSS_ONLINE(1) ");
		CVI_TRACE_LOG(CVI_DBG_WARN, "VI_ONLINE_VPSS_OFFLINE (2)/VI_ONLINE_VPSS_ONLINE (3): ");
		scanf("%d", &tmp);
		stVIVPSSMode.aenMode[0] = tmp;
		s32Ret = CVI_SYS_SetVIVPSSMode(&stVIVPSSMode);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_SetVIVPSSMode failed with %#x\n", s32Ret);
			break;
		}

		s32Ret = CVI_SYS_GetVIVPSSMode(&stVIVPSSMode);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_SYS_GetVIVPSSMode failed with %#x\n", s32Ret);
			break;
		}
		CVI_TRACE_LOG(CVI_DBG_WARN, "set vi/vpss mode TEST-PASS\n");
		break;
	}
	case 10: {
		int enable;
		VI_DEV_TIMING_ATTR_S stTimingAttr;

		CVI_TRACE_LOG(CVI_DBG_WARN, "enable(1)/disable(0): ");
		scanf("%d", &enable);
		stTimingAttr.bEnable = enable;
		if (enable) {
			CVI_TRACE_LOG(CVI_DBG_WARN, "update fps: ");
			scanf("%d", &stTimingAttr.s32FrmRate);
			s32Ret = CVI_VI_SetPipeFrameSource(0, VI_PIPE_FRAME_SOURCE_USER_FE);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_SetPipeFrameSource failed with %#x\n", s32Ret);
				return s32Ret;
			}
		} else {
			stTimingAttr.s32FrmRate = 0;
			CVI_VI_SetPipeFrameSource(0, VI_PIPE_FRAME_SOURCE_DEV);
		}

		s32Ret = CVI_VI_SetDevTimingAttr(0, &stTimingAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_SetDevTimingAttr failed with %#x\n", s32Ret);
			return s32Ret;
		}

		CVI_TRACE_LOG(CVI_DBG_WARN, "set usr-pic TEST-PASS\n");
		break;
	}
	case 11: {
		VIDEO_FRAME_INFO_S stVideoFrame;
		const VIDEO_FRAME_INFO_S *pstVideoFrame[1];
		VI_PIPE PipeId[] = {0};
		FILE *fp;
		CVI_CHAR filename[64];
		CVI_U32 u32len, height, is_hdr_on, value;

		CVI_TRACE_LOG(CVI_DBG_WARN, "filename: ");
		scanf("%s", filename);
		fp = fopen(filename, "r");
		if (fp == CVI_NULL) {
			CVI_TRACE_LOG(CVI_DBG_WARN, "open data file error\n");
			return CVI_FAILURE;
		}

		CVI_TRACE_LOG(CVI_DBG_WARN, "img width: ");
		scanf("%d", &stVideoFrame.stVFrame.u32Width);
		CVI_TRACE_LOG(CVI_DBG_WARN, "img height: ");
		scanf("%d", &stVideoFrame.stVFrame.u32Height);
		CVI_TRACE_LOG(CVI_DBG_WARN, "crop left offset: ");
		scanf("%d", &value);
		stVideoFrame.stVFrame.s16OffsetLeft = (CVI_S16)value;
		CVI_TRACE_LOG(CVI_DBG_WARN, "crop top offset: ");
		scanf("%d", &value);
		stVideoFrame.stVFrame.s16OffsetTop = (CVI_S16)value;
		CVI_TRACE_LOG(CVI_DBG_WARN, "crop right offset: ");
		scanf("%d", &value);
		stVideoFrame.stVFrame.s16OffsetRight = (CVI_S16)value;
		CVI_TRACE_LOG(CVI_DBG_WARN, "crop bottom offset: ");
		scanf("%d", &value);
		stVideoFrame.stVFrame.s16OffsetBottom = (CVI_S16)value;
		CVI_TRACE_LOG(CVI_DBG_WARN, "bayer format BG(0)/GB(1)/GR(2)/RG(3): ");
		scanf("%d", &tmp);
		stVideoFrame.stVFrame.enBayerFormat = tmp;
		CVI_TRACE_LOG(CVI_DBG_WARN, "is_hdr_input: ");
		scanf("%d", &is_hdr_on);

		if (is_hdr_on == 1) {
			stVideoFrame.stVFrame.enDynamicRange = DYNAMIC_RANGE_HDR10;
			height = stVideoFrame.stVFrame.u32Height * 2;
		} else
			height = stVideoFrame.stVFrame.u32Height;

		CVI_U32 u32BlkSize = VI_GetRawBufferSize(stVideoFrame.stVFrame.u32Width,
				     height, PIXEL_FORMAT_RGB_BAYER_12BPP,
				     COMPRESS_MODE_NONE, DEFAULT_ALIGN, 0);
		vi_usr_blk = CVI_VB_GetBlock(VB_INVALID_POOLID, u32BlkSize);
		CVI_U64 u64PhyAddr = CVI_VB_Handle2PhysAddr(vi_usr_blk);
		CVI_VOID *puVirAddr = CVI_SYS_Mmap(u64PhyAddr, u32BlkSize);

		u32len = fread(puVirAddr, u32BlkSize, 1, fp);
		if (u32len <= 0) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "fread raw error\n");
			fclose(fp);
			return CVI_FAILURE;
		}

		CVI_TRACE_LOG(CVI_DBG_WARN, "load img to bayer buffer: size(0x%x) paddr(%#"PRIx64") vaddr(%p)\n",
			   u32BlkSize, u64PhyAddr, puVirAddr);

		stVideoFrame.stVFrame.u64PhyAddr[0] = u64PhyAddr;
		pstVideoFrame[0] = &stVideoFrame;
		CVI_VI_SendPipeRaw(1, PipeId, pstVideoFrame, 80);

		CVI_TRACE_LOG(CVI_DBG_WARN, "Send usr-pic TEST-PASS\n");
		break;
	}
	case 12: {
		if (vi_usr_blk) {
			CVI_VB_ReleaseBlock(vi_usr_blk);
			vi_usr_blk = 0;
		}
		break;
	}
	case 13: {
		VI_PIPE ViPipe = 0;
		char name[64] = {0};
		FILE *fp = NULL;
		VI_DUMP_REGISTER_TABLE_S reg_tbl;

		snprintf(name, 64, "vi_dump_register.json");
		fp = fopen(name, "w");
		if (fp == NULL) {
			CVI_TRACE_LOG(CVI_DBG_WARN, "open %s fail!!!\n", name);
			return CVI_FAILURE;
		}

		s32Ret = CVI_VI_DumpHwRegisterToFile(ViPipe, fp, &reg_tbl);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_DumpHwRegisterToFile failed with %#x\n", s32Ret);
			fclose(fp);
			return s32Ret;
		}

		fclose(fp);
		CVI_TRACE_LOG(CVI_DBG_WARN, "Dump register pass\n");
		break;
	}
	case 14: {
		int flip;
		int mirror;
		int chnID;
		int pipeID;

		CVI_TRACE_LOG(CVI_DBG_WARN, "chn(0~1): ");
		scanf("%d", &chnID);
		CVI_TRACE_LOG(CVI_DBG_WARN, "Flip enable/disable(1/0): ");
		scanf("%d", &flip);
		CVI_TRACE_LOG(CVI_DBG_WARN, "Mirror enable/disable(1/0): ");
		scanf("%d", &mirror);
		pipeID = chnID;
		CVI_VI_SetChnFlipMirror(pipeID, chnID, flip, mirror);
		break;
	}
	// TODO move to an test application
	// copy this block to cvi_test.c to test MPI
	case 15: {
		VIDEO_FRAME_INFO_S stVideoFrame[2];
		VI_DUMP_ATTR_S attr;
		struct timeval tv1;
		int frm_num = 1, j = 0;
		CVI_U32 dev = 0, loop = 0;
		struct timespec start, end;

		memset(stVideoFrame, 0, sizeof(stVideoFrame));

		stVideoFrame[0].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;
		stVideoFrame[1].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;

		CVI_TRACE_LOG(CVI_DBG_WARN, "To get raw dump from dev(0~1): ");
		scanf("%d", &dev);

		attr.bEnable = 1;
		attr.u32Depth = 0;
		attr.enDumpType = VI_DUMP_TYPE_RAW;

		CVI_VI_SetPipeDumpAttr(dev, &attr);

		attr.bEnable = 0;
		attr.enDumpType = VI_DUMP_TYPE_IR;

		CVI_VI_GetPipeDumpAttr(dev, &attr);

		CVI_TRACE_LOG(CVI_DBG_WARN, "Enable(%d), DumpType(%d):\n", attr.bEnable, attr.enDumpType);
		CVI_TRACE_LOG(CVI_DBG_WARN, "how many loops to do (1~60)");
		scanf("%d", &loop);

		if (loop > 60)
			break;

		while (loop > 0) {
			clock_gettime(CLOCK_MONOTONIC, &start);
			frm_num = 1;

			CVI_VI_GetPipeFrame(dev, stVideoFrame, 1000);

			if (stVideoFrame[1].stVFrame.u64PhyAddr[0] != 0)
				frm_num = 2;

			gettimeofday(&tv1, NULL);

			for (j = 0; j < frm_num; j++) {
				size_t image_size = stVideoFrame[j].stVFrame.u32Length[0];
				unsigned char *ptr = calloc(1, image_size);
				FILE *output;
				char img_name[128] = {0,}, order_id[8] = {0,};

				if (attr.enDumpType == VI_DUMP_TYPE_RAW) {
					stVideoFrame[j].stVFrame.pu8VirAddr[0]
						= CVI_SYS_Mmap(stVideoFrame[j].stVFrame.u64PhyAddr[0]
						  , stVideoFrame[j].stVFrame.u32Length[0]);
					CVI_TRACE_LOG(CVI_DBG_WARN, "paddr(%#"PRIx64") vaddr(%p)\n",
								stVideoFrame[j].stVFrame.u64PhyAddr[0],
								stVideoFrame[j].stVFrame.pu8VirAddr[0]);

					memcpy(ptr, (const void *)stVideoFrame[j].stVFrame.pu8VirAddr[0],
						stVideoFrame[j].stVFrame.u32Length[0]);
					CVI_SYS_Munmap((void *)stVideoFrame[j].stVFrame.pu8VirAddr[0],
							stVideoFrame[j].stVFrame.u32Length[0]);

					switch (stVideoFrame[j].stVFrame.enBayerFormat) {
					default:
					case BAYER_FORMAT_BG:
						snprintf(order_id, sizeof(order_id), "BG");
						break;
					case BAYER_FORMAT_GB:
						snprintf(order_id, sizeof(order_id), "GB");
						break;
					case BAYER_FORMAT_GR:
						snprintf(order_id, sizeof(order_id), "GR");
						break;
					case BAYER_FORMAT_RG:
						snprintf(order_id, sizeof(order_id), "RG");
						break;
					}

					snprintf(img_name, sizeof(img_name),
							"./vi_%d_%s_%s_w_%d_h_%d_x_%d_y_%d_tv_%ld_%ld.raw",
							dev, (j == 0) ? "LE" : "SE", order_id,
							stVideoFrame[j].stVFrame.u32Width,
							stVideoFrame[j].stVFrame.u32Height,
							stVideoFrame[j].stVFrame.s16OffsetLeft,
							stVideoFrame[j].stVFrame.s16OffsetTop,
							tv1.tv_sec, tv1.tv_usec);

					CVI_TRACE_LOG(CVI_DBG_WARN, "dump image %s\n", img_name);

					output = fopen(img_name, "wb");

					fwrite(ptr, image_size, 1, output);
					fclose(output);
					free(ptr);
				}
			}

			CVI_VI_ReleasePipeFrame(dev, stVideoFrame);

			clock_gettime(CLOCK_MONOTONIC, &end);
			CVI_TRACE_LOG(CVI_DBG_WARN, "ms consumed: %f\n",
						(CVI_FLOAT)diff_in_us(start, end) / 1000);

			loop--;
		}

		CVI_TRACE_LOG(CVI_DBG_WARN, "Dump VI raw TEST-PASS\n");
		break;
	}
	case 16: {
		VI_CROP_INFO_S pstCropInfo;

		CVI_TRACE_LOG(CVI_DBG_WARN, "Set Chn Crop. plz set input x:\n");
		scanf("%d", &pstCropInfo.stCropRect.s32X);
		CVI_TRACE_LOG(CVI_DBG_WARN, "input y:\n");
		scanf("%d", &pstCropInfo.stCropRect.s32Y);
		CVI_TRACE_LOG(CVI_DBG_WARN, "input width:\n");
		scanf("%d", &pstCropInfo.stCropRect.u32Width);
		CVI_TRACE_LOG(CVI_DBG_WARN, "input height:\n");
		scanf("%d", &pstCropInfo.stCropRect.u32Height);

		pstCropInfo.bEnable = CVI_TRUE;
		CVI_VI_SetChnCrop(0, 0, &pstCropInfo);

		CVI_TRACE_LOG(CVI_DBG_WARN, "set chn crop TEST-PASS\n");
		break;
	}
	case 17: {
		VI_CROP_INFO_S pstCropInfo;

		CVI_VI_GetChnCrop(0, 0, &pstCropInfo);
		CVI_TRACE_LOG(CVI_DBG_WARN, "VI_GetChnCrop:x_%d_y_%d_w_%d_h_%d\n",
			pstCropInfo.stCropRect.s32X, pstCropInfo.stCropRect.s32Y,
			pstCropInfo.stCropRect.u32Width, pstCropInfo.stCropRect.u32Height);

		CVI_TRACE_LOG(CVI_DBG_WARN, "get chn crop TEST-PASS\n");
		break;
	}
	case 18: {
		s32Ret = CVI_VI_GetPipeFd(0);
		if (s32Ret < 0) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "GetPipeFd is fail\n");
			return s32Ret;
		}

		s32Ret = CVI_VI_CloseFd();
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CloseFd is fail\n");
			return s32Ret;
		}

		CVI_TRACE_LOG(CVI_DBG_WARN, "pipe fd ctrl TEST-PASS\n");
		break;
	}
	case 19: {
		int sel;

		SAMPLE_PRT("Rotation 0(0)/1(90)/2(180)/3(270): ");
		scanf("%d", &sel);
		CVI_VI_SetChnRotation(0, 0, sel);
		break;
	}

	case 20: {
		SIZE_S stSize;
		VO_LAYER VoLayer;

		CVI_TRACE_LOG(CVI_DBG_WARN, "VO take yuv420-planar fmt in this test.\n");

		CVI_TRACE_LOG(CVI_DBG_WARN, "vo Layer (0/1): \n");
		scanf("%d", &VoLayer);
		VoLayer %= 2;
		CVI_TRACE_LOG(CVI_DBG_WARN, "img width: ");
		scanf("%d", &stSize.u32Width);
		CVI_TRACE_LOG(CVI_DBG_WARN, "img height: ");
		scanf("%d", &stSize.u32Height);

		_vo_sendframe(VoLayer, &stSize, "vo.yuv");
		break;
	}
	case 21: {
		int sel;
		VO_LAYER VoLayer;

		SAMPLE_PRT("VO Layer (0/1): ");
		scanf("%d", &VoLayer);
		SAMPLE_PRT("VO Layer on(1)/off(0): ");
		scanf("%d", &sel);
		if (sel)
			CVI_VO_EnableVideoLayer(VoLayer);
		else
			CVI_VO_DisableVideoLayer(VoLayer);
		break;
	}
	case 22: {
		int sel;
		VO_LAYER VoLayer;

		SAMPLE_PRT("VO Layer (0/1): ");
		scanf("%d", &VoLayer);
		SAMPLE_PRT("Vo Chn Show(1)/Hide(0): ");
		scanf("%d", &sel);
		if (sel)
			CVI_VO_ShowChn(VoLayer, 0);
		else
			CVI_VO_HideChn(VoLayer, 0);
		break;
	}
	case 23: {
		s32Ret = CVI_VO_CloseFd();
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CloseFd is fail\n");
			return s32Ret;
		}

		CVI_TRACE_LOG(CVI_DBG_WARN, "VO ctrl TEST-PASS\n");
		break;
	}
	case 24: {
		ROTATION_E enRotation;
		VO_PUB_ATTR_S stPubAttr;
		VO_VIDEO_LAYER_ATTR_S stVideoAttr;
		CVI_U32 u32BufLen;
		VO_DEV VoDev;
		VO_LAYER VoLayer;

		SAMPLE_PRT("VO VoDev (0/1): ");
		scanf("%d", &VoDev);

		VoLayer = VoDev;
		s32Ret = CVI_VO_GetPubAttr(VoDev, &stPubAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VO_GetPubAttr is fail\n");
			return s32Ret;
		}

		s32Ret = CVI_VO_GetVideoLayerAttr(VoLayer, &stVideoAttr);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VO_GetVideoLayerAttr is fail\n");
			return s32Ret;
		}

		s32Ret = CVI_VO_GetDisplayBufLen(VoLayer, &u32BufLen);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VO_GetDisplayBufLen is fail\n");
			return s32Ret;
		}


		s32Ret = CVI_VO_GetChnRotation(VoLayer, 0, &enRotation);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VO_GetChnRotation is fail\n");
			return s32Ret;
		}

		CVI_TRACE_LOG(CVI_DBG_WARN, "-- VO Dev info --\n");
		CVI_TRACE_LOG(CVI_DBG_WARN, "Intf Type(%d)  Sync Type(%d)\n"
			, stPubAttr.enIntfType, stPubAttr.enIntfSync);
		if (stPubAttr.enIntfSync == VO_OUTPUT_USER) {
			CVI_TRACE_LOG(CVI_DBG_WARN, "Hor front-porch(%d) back-porch(%d) active(%d) sync(%d)\n"
				, stPubAttr.stSyncInfo.u16Hfb, stPubAttr.stSyncInfo.u16Hbb
				, stPubAttr.stSyncInfo.u16Hact, stPubAttr.stSyncInfo.u16Hpw);
			CVI_TRACE_LOG(CVI_DBG_WARN, "Ver front-porch(%d) back-porch(%d) active(%d) sync(%d)\n"
				, stPubAttr.stSyncInfo.u16Vfb, stPubAttr.stSyncInfo.u16Vbb
				, stPubAttr.stSyncInfo.u16Vact, stPubAttr.stSyncInfo.u16Vpw);
		}
		CVI_TRACE_LOG(CVI_DBG_WARN, "-- VO VideoLayer info --\n");
		CVI_TRACE_LOG(CVI_DBG_WARN, "width(%d)  height(%d)\n"
				, stVideoAttr.stImageSize.u32Width, stVideoAttr.stImageSize.u32Height);
		CVI_TRACE_LOG(CVI_DBG_WARN, "PixFormat(%d)  DispFrmRate(%d)  u32BufLen(%d)\n"
				, stVideoAttr.enPixFormat, stVideoAttr.u32DispFrmRt, u32BufLen);
		CVI_TRACE_LOG(CVI_DBG_WARN, "Chn Rotation(%d)\n", enRotation);

		CVI_TRACE_LOG(CVI_DBG_WARN, "VO info TEST-PASS\n");
		break;
	}

	case 25: {
		CROP_INFO_S pstCropInfo;

		CVI_TRACE_LOG(CVI_DBG_WARN, "Set Pipe Crop. plz set input x:\n");
		scanf("%d", &pstCropInfo.stRect.s32X);
		CVI_TRACE_LOG(CVI_DBG_WARN, "input y:\n");
		scanf("%d", &pstCropInfo.stRect.s32Y);
		CVI_TRACE_LOG(CVI_DBG_WARN, "input width:\n");
		scanf("%d", &pstCropInfo.stRect.u32Width);
		CVI_TRACE_LOG(CVI_DBG_WARN, "input height:\n");
		scanf("%d", &pstCropInfo.stRect.u32Height);

		pstCropInfo.bEnable = CVI_TRUE;
		CVI_VI_SetPipeCrop(0, &pstCropInfo);

		CVI_VI_GetPipeCrop(0, &pstCropInfo);
		CVI_TRACE_LOG(CVI_DBG_WARN, "VI_GetPipeCrop:x_%d_y_%d_w_%d_h_%d\n",
			pstCropInfo.stRect.s32X, pstCropInfo.stRect.s32Y,
			pstCropInfo.stRect.u32Width, pstCropInfo.stRect.u32Height);

		CVI_TRACE_LOG(CVI_DBG_WARN, "set/get pipe crop TEST-PASS\n");
		break;
	}

	case 26: {
		PROC_AMP_E type;
		PROC_AMP_CTRL_S ctrl;
		CVI_S32 cur;
		VO_LAYER VoLayer;

		SAMPLE_PRT("VO VoLayer (0/1): ");
		scanf("%d", &VoLayer);

		SAMPLE_PRT("Which ProcAmp to get 0(brightness)/1(contrast)/2(saturation)/3(hue): ");
		scanf("%d", &tmp);
		type = tmp;
		s32Ret = CVI_VO_GetLayerProcAmpCtrl(VoLayer, type, &ctrl);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VO_GetLayerProcAmpCtrl is fail\n");
			return s32Ret;
		}
		s32Ret = CVI_VO_GetLayerProcAmp(VoLayer, type, &cur);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VO_GetLayerProcAmp is fail\n");
			return s32Ret;
		}
		CVI_TRACE_LOG(CVI_DBG_WARN, "min(%d) max(%d) step(%d) default(%d) current(%d)\n"
			, ctrl.minimum, ctrl.maximum, ctrl.step, ctrl.default_value, cur);
		CVI_TRACE_LOG(CVI_DBG_WARN, "VO fd ctrl TEST-PASS\n");
		break;
	}
	case 27: {
		PROC_AMP_E type;
		CVI_S32 value;
		VO_LAYER VoLayer;

		SAMPLE_PRT("VO VoLayer (0/1): ");
		scanf("%d", &VoLayer);

		SAMPLE_PRT("Which ProcAmp to set 0(brightness)/1(contrast)/2(saturation)/3(hue): ");
		scanf("%d", &tmp);
		type = tmp;
		SAMPLE_PRT("new value: ");
		scanf("%d", &value);
		s32Ret = CVI_VO_SetLayerProcAmp(VoLayer, type, value);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VO_SetLayerProcAmp is fail\n");
			return s32Ret;
		}
		break;
	}
	case 28: {
		int pause;
		VO_LAYER VoLayer;

		SAMPLE_PRT("VO VoLayer (0/1): ");
		scanf("%d", &VoLayer);

		SAMPLE_PRT("1(pause)/0(resume): ");
		scanf("%d", &pause);

		if (pause)
			CVI_VO_PauseChn(VoLayer, 0);
		else
			CVI_VO_ResumeChn(VoLayer, 0);
		break;
	}

	case 29: {
		VI_LDC_ATTR_S stLDCAttr;

		SAMPLE_PRT("Enable 1(Y)/0(N): ");
		scanf("%d", &tmp);
		stLDCAttr.bEnable = tmp;
		if (stLDCAttr.bEnable) {
			SAMPLE_PRT("Keep AspectRatio 1(Y)/0(N): ");
			scanf("%d", &tmp);
			stLDCAttr.stAttr.bAspect = tmp;
			if (stLDCAttr.stAttr.bAspect) {
				SAMPLE_PRT("Ratio (0 ~ 100): ");
				scanf("%d", &tmp);
				stLDCAttr.stAttr.s32XYRatio = tmp;
			} else {
				SAMPLE_PRT("XRatio (0 ~ 100): ");
				scanf("%d", &stLDCAttr.stAttr.s32XRatio);
				SAMPLE_PRT("YRatio (0 ~ 100): ");
				scanf("%d", &stLDCAttr.stAttr.s32YRatio);
			}
			SAMPLE_PRT("XOffset (-511 ~ 511): ");
			scanf("%d", &stLDCAttr.stAttr.s32CenterXOffset);
			SAMPLE_PRT("YOffset (-511 ~ 511): ");
			scanf("%d", &stLDCAttr.stAttr.s32CenterYOffset);
			SAMPLE_PRT("DistortionRatio (-300 ~ 500): ");
			scanf("%d", &stLDCAttr.stAttr.s32DistortionRatio);
		}
		s32Ret = CVI_VI_SetChnLDCAttr(0, 0, &stLDCAttr);
		break;
	}
	case 50: {
		s32Ret = _vpss_test();
		if (s32Ret == CVI_SUCCESS)
			s32Ret = _vpss_test();
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "VPSS test fail\n");
			break;
		}

		s32Ret = CVI_VPSS_CloseFd();
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CloseFd is fail\n");
			break;
		}

		CVI_TRACE_LOG(CVI_DBG_WARN, "VPSS TEST-%s\n", (s32Ret == CVI_SUCCESS) ? "PASS" : "FAIL");
		break;
	}
	case 51: {
		s32Ret = _sys_test();
		CVI_TRACE_LOG(CVI_DBG_WARN, "SYS TEST-%s\n", (s32Ret == CVI_SUCCESS) ? "PASS" : "FAIL");
		break;
	}
	case 52: {
		_tuya_test();
		break;
	}
	case 53: {
		_vi_sdk_test(pstViConfig);
		break;
	}
	case 54: {
		SIZE_S stSize = { .u32Width = 2560, .u32Height = 1440 };
		VIDEO_FRAME_INFO_S stVideoFrame;

		_vpss_dual_test();

		if (SAMPLE_COMM_FRAME_LoadFromFile("./sample.yuv", &stVideoFrame, &stSize, PIXEL_FORMAT_YUV_PLANAR_422)
			!= CVI_SUCCESS)
			break;
		while (1) {
			if (CVI_VPSS_SendFrame(0, &stVideoFrame, -1) != CVI_SUCCESS) {
				PAUSE();
			}
			if (CVI_VPSS_SendFrame(1, &stVideoFrame, -1) != CVI_SUCCESS) {
				PAUSE();
			}
			usleep(1000 * 33);
		}
		break;
	}
	case 55: {
		_gdc_test();
		break;
	}
	case 56: {
		SAMPLE_PRT("vpss hw stress test\n");
		s32Ret = _vpss_hw_stress_test();
		break;
	}
	case 57: {
		ViConfigReInit(&g_stViConfig, &g_stIniCfg);

		if (vi_ops->vi_close) {
			s32Ret = vi_ops->vi_close(&g_stViConfig);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "vi close failed. s32Ret: 0x%x !\n", s32Ret);
				return s32Ret;
			}
		}

		if (vi_ops->vi_open) {
			s32Ret = vi_ops->vi_open();
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "vi open failed. s32Ret: 0x%x !\n", s32Ret);
				return s32Ret;
			}
		}

		if (vi_ops->vi_ofl_ol_vpss) {
			s32Ret = vi_ops->vi_ofl_ol_vpss(&g_stViConfig, &g_stIniCfg);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "vi offline/online vpss failed. s32Ret: 0x%x !\n", s32Ret);
				return s32Ret;
			}
		}

		break;
	}
	case 60: {
		SAMPLE_PRT("bind with VI\n");
		psvt->bind_mode = VENC_BIND_VI;
		psvt->numChn = 1;
		break;
	}
	case 61: {
		SAMPLE_PRT("bind with VPSS\n");
		psvt->bind_mode = VENC_BIND_VPSS;
		psvt->vpssGrp = 0;

		CVI_TRACE_LOG(CVI_DBG_WARN, "how many chns(1~3): ");
		scanf("%d", &psvt->numChn);

		for (int idx = 0; idx < psvt->numChn; idx++)
			psvt->vpssChn[idx] = idx;

		break;
	}
	case 62: {
		SAMPLE_PRT("Encode H.265\n");
		psvt->codec = PT_H265;
		break;
	}
	case 63: {
		SAMPLE_PRT("Encode H.264\n");
		psvt->codec = PT_H264;
		break;
	}
	case 64: {
		SAMPLE_PRT("Encode MJPEG\n");
		psvt->codec = PT_MJPEG;
		break;
	}
	case 65: {
		SAMPLE_PRT("Encode JPEG\n");
		psvt->codec = PT_JPEG;
		break;
	}
	case 66: {
		SAMPLE_PRT("start VENC\n");
		_start_venc(psvt);
		break;
	}
	case 67: {
		SAMPLE_PRT("stop VENC\n");
		_stop_venc(psvt);
		break;
	}
	case 68: {
		SAMPLE_PRT("get vpss info\n");
		_get_vpss_info();
		break;
	}
	case 80: {
		CVI_S32 HandleNum;
		RGN_TYPE_E enType;
		RGN_CHN_ATTR_S stChnAttr;
		RGN_ATTR_S stRegion;
		CVI_S32 s32X, s32Y;
		CVI_U32 u32W, u32H, u32Layer;
		MMF_CHN_S chn = {.enModId = CVI_ID_VPSS, .s32DevId = 0, .s32ChnId = 0};

		printf("Attach to VPSS Grp:\n");
		scanf(" %d", &chn.s32DevId);
		printf("Attach to VPSS Chn:\n");
		scanf(" %d", &chn.s32ChnId);

		printf("RGN Handle Id:\n");
		scanf(" %d", &HandleNum);
		printf("RGN Type 0(Overlay)/1(Cover):/2(CoverEx)/3:(OverlayEx)\n");
		scanf(" %d", &tmp);
		enType = tmp;
		printf("start x:\n");
		scanf(" %d", &s32X);
		printf("start y:\n");
		scanf(" %d", &s32Y);

		stChnAttr.bShow = CVI_TRUE;
		stRegion.enType = stChnAttr.enType = enType;
		switch (enType) {
		case OVERLAY_RGN:
			printf("overlay width:\n");
			scanf(" %d", &u32W);
			printf("overlay height:\n");
			scanf(" %d", &u32H);

			stRegion.unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
			stRegion.unAttr.stOverlay.stSize.u32Height = u32H;
			stRegion.unAttr.stOverlay.stSize.u32Width = u32W;
			stRegion.unAttr.stOverlay.u32BgColor = 0x00000000; // ARGB1555 transparent
			stRegion.unAttr.stOverlay.u32CanvasNum = 2;

			stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = s32X;
			stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = s32Y;
			stChnAttr.unChnAttr.stOverlayChn.u32Layer = 0;
			break;
		case OVERLAYEX_RGN:
			printf("overlayEx width:\n");
			scanf(" %d", &u32W);
			printf("overlayEx height:\n");
			scanf(" %d", &u32H);
			printf("overlayEx layer:\n");
			scanf(" %d", &u32Layer);

			stRegion.unAttr.stOverlayEx.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
			stRegion.unAttr.stOverlayEx.stSize.u32Height = u32H;
			stRegion.unAttr.stOverlayEx.stSize.u32Width = u32W;
			stRegion.unAttr.stOverlayEx.u32BgColor = 0x00000000; // ARGB1555 transparent
			stRegion.unAttr.stOverlayEx.u32CanvasNum = 2;

			stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = s32X;
			stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = s32Y;
			stChnAttr.unChnAttr.stOverlayExChn.u32Layer = u32Layer;
			break;
		case COVER_RGN:
			stChnAttr.unChnAttr.stCoverChn.enCoverType = AREA_RECT;

			stChnAttr.unChnAttr.stCoverChn.stRect.s32X = s32X;
			stChnAttr.unChnAttr.stCoverChn.stRect.s32Y = s32Y;
			stChnAttr.unChnAttr.stCoverChn.stRect.u32Height = 100;
			stChnAttr.unChnAttr.stCoverChn.stRect.u32Width = 100;

			stChnAttr.unChnAttr.stCoverChn.u32Color = 0x0000ffff;

			stChnAttr.unChnAttr.stCoverChn.enCoordinate = RGN_ABS_COOR;

			stChnAttr.unChnAttr.stCoverChn.u32Layer = 0;
			break;
		case COVEREX_RGN:
			printf("coverEx layer:\n");
			scanf(" %d", &u32Layer);

			stChnAttr.unChnAttr.stCoverExChn.enCoverType = AREA_RECT;

			stChnAttr.unChnAttr.stCoverExChn.stRect.s32X = s32X;
			stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y = s32Y;
			stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = 100;
			stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width = 100;

			stChnAttr.unChnAttr.stCoverExChn.u32Color = 0x0000ffff;

			stChnAttr.unChnAttr.stCoverExChn.u32Layer = u32Layer;
			break;
		default:
			return CVI_FAILURE;
		}

		s32Ret = CVI_RGN_Create(HandleNum, &stRegion);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_RGN_Create failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}

		s32Ret = CVI_RGN_AttachToChn(HandleNum, &chn, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_RGN_AttachToChn failed!\n");
			break;
		}

		if (enType == OVERLAY_RGN || enType == OVERLAYEX_RGN) {
			s32Ret = SAMPLE_COMM_REGION_GetUpCanvas(HandleNum, "tiger.bmp");
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("SAMPLE_COMM_REGION_GetUpCanvas failed!\n");
				break;
			}
		}
		break;
	}
	case 100: {
		CVI_S32 testId = 0;
		CVI_S32 random = 0;

		printf("input testId:\n");
		scanf(" %d", &testId);
		printf("testId %d\n", testId);

		printf("input random:\n");
		scanf(" %d", &random);
		printf("random %d\n", random);

#ifndef FPGA_PORTING
		isp_block_mpi_setest(0, testId, random);
#endif
		break;
	}

	case 101: {
		CVI_S32 again = 0;
		CVI_S32 dgain = 0;
		CVI_S32 shutterTime = 0;
		ISP_EXPOSURE_ATTR_S expAttr = {0};

		printf("input again:\n");
		scanf(" %d", &again);
		printf("input dgain:\n");
		scanf(" %d", &dgain);
		printf("input shuttertime(us):\n");
		scanf(" %d", &shutterTime);
		expAttr.stManual.enAGainOpType = expAttr.stManual.enDGainOpType =
			expAttr.stManual.enExpTimeOpType = 1;
		expAttr.stManual.u32AGain = again;
		expAttr.stManual.u32DGain = dgain;
		expAttr.stManual.u32ExpTime = shutterTime;
		CVI_ISP_SetExposureAttr(0, &expAttr);
		break;
	}
	case 102: {
	//	ISP_AE_STATISTICS_S aeSts;
	//	CVI_ISP_GetAEStatistics(0, &aeSts);
	//	isp_3aInfo_dump();
	//	CVI_ISP_GetWBStatistics(0, &aeSts);
		ISP_AF_STATISTICS_S afStat;

		CVI_ISP_GetFocusStatistics(0, &afStat);
		break;
	}
	case 103:
#ifndef FPGA_PORTING
		isp_test_set_debug_level();
#endif
		break;
	case 104: {
		CVI_U32 loop;

		CVI_TRACE_LOG(CVI_DBG_WARN, "how many loops to do(11111 is infinite: ");
		scanf("%d", &loop);

		_vi_get_multi_chn_frame(loop);
		break;
	}
	case 105: {
		CVI_S32 s32Ret = 0;
		ISP_PUB_ATTR_S getPubAttr = {0};
		ISP_PUB_ATTR_S orgPubAttr = {0};
		ISP_PUB_ATTR_S setPubAttr = {0};
		CVI_U32 timeOut = 500;

		s32Ret = CVI_ISP_GetPubAttr(0, &orgPubAttr);
		s32Ret = CVI_ISP_GetPubAttr(0, &setPubAttr);
		setPubAttr.u8SnsMode += 2;
		setPubAttr.enBayer = (setPubAttr.enBayer + 1) % BAYER_BUTT;
		s32Ret |= CVI_ISP_SetPubAttr(0, &setPubAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_ISP_GetPubAttr failed with %#x\n", s32Ret);
			return s32Ret;
		}
		for (CVI_U32 i = 0; i < 10; i++) {
			s32Ret = CVI_ISP_GetVDTimeOut(0, ISP_VD_FE_START, timeOut);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("wait Vd time out. s32Ret: %#x\n", s32Ret);
			}
		}
		s32Ret = CVI_ISP_GetPubAttr(0, &getPubAttr);
		// compare current sensor pub attr and set to mw pub attr.
		s32Ret = memcmp(&getPubAttr, &setPubAttr, sizeof(ISP_PUB_ATTR_S));
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_ISP_GetPubAttr failed with compare error %#x\n", s32Ret);
			return s32Ret;
		} else {
			SAMPLE_PRT("CVI_ISP_Set/GetPubAttr test success\n");
		}
		s32Ret = CVI_ISP_SetPubAttr(0, &orgPubAttr);
		SAMPLE_PRT("CVI_ISP_GetPubAttr print :\n");
		SAMPLE_PRT("ROI x : %d y : %d w : %d h : %d\n", getPubAttr.stWndRect.s32X,
			getPubAttr.stWndRect.s32Y, getPubAttr.stWndRect.u32Width, getPubAttr.stWndRect.u32Height);
		SAMPLE_PRT("sns Size w : %d h : %d\n", getPubAttr.stSnsSize.u32Width,
			getPubAttr.stSnsSize.u32Height);
		SAMPLE_PRT("sns fps : %d bayer : %d wdr : %d mode : %d\n", getPubAttr.u8SnsMode,
			getPubAttr.enBayer, getPubAttr.enWDRMode, getPubAttr.u8SnsMode);
		break;
	}

	case 106: {
		ISP_FMW_STATE_E setState = ISP_FMW_STATE_RUN;
		ISP_FMW_STATE_E getState = ISP_FMW_STATE_BUTT;

		printf("FW state freeze? (0 : run, 1 : freeze):\n");
		scanf("%d", &tmp);
		setState = tmp;
		s32Ret = CVI_ISP_SetFMWState(0, setState);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_ISP_SetFMWState failed with %#x\n", s32Ret);
			return s32Ret;
		}
		// compare current sensor pub attr and set to mw pub attr.
		s32Ret = CVI_ISP_GetFMWState(0, &getState);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_ISP_GetFMWState failed with %#x\n", s32Ret);
			return s32Ret;
		}
		if (setState != getState) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_ISP_GetFMWState compare failed\n");
			return s32Ret;
		}
		break;
	}
	case 107: {
#ifndef FPGA_PORTING
		isp_gpio_ctrl();
#endif
		break;
	}

	case 108: {
		CVI_S32 s32Ret = 0;
		ISP_CTRL_PARAM_S setParam;
		ISP_CTRL_PARAM_S getParam;

		memset(&setParam, 0, sizeof(ISP_CTRL_PARAM_S));
		memset(&getParam, 0, sizeof(ISP_CTRL_PARAM_S));

		setParam.u32ProcParam = 30;
		setParam.u32ProcLevel = 1;
		setParam.u32AEStatIntvl = 1;
		setParam.u32AWBStatIntvl = 6;
		setParam.u32AFStatIntvl = 1;
		setParam.u32UpdatePos = 0;
		setParam.u32IntTimeOut = 0;
		setParam.u32PwmNumber = 0;
		setParam.u32PortIntDelay = 0;

		s32Ret = CVI_ISP_SetCtrlParam(0, &setParam);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_ISP_SetCtrlParam failed with %#x\n", s32Ret);
			return s32Ret;
		}

		s32Ret = CVI_ISP_GetCtrlParam(0, &getParam);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_ISP_GetCtrlParam failed with %#x\n", s32Ret);
			return s32Ret;
		}

		if (memcmp(&setParam, &getParam, sizeof(ISP_CTRL_PARAM_S)) != 0) {
			CVI_TRACE_LOG(CVI_DBG_ERR,
				"CVI_ISP_GetCtrlParam value is not equal to CVI_ISP_SetCtrlParam value\n");
			return CVI_FAILURE;
		}
		break;
	}

	case 109:
#ifndef FPGA_PORTING
		sensor_ae_test();
#endif
		break;
	case 110:
#ifndef FPGA_PORTING
		AE_Debug();
#endif
		break;

	case 111:
#ifndef FPGA_PORTING
		AWB_Debug();
#endif
		break;

	case 112: {
		CVI_U8 wdrMode = 0;
		SAMPLE_COMM_VI_DestroyIsp(&g_stViConfig);
		// Stop VI.
		SAMPLE_COMM_VI_DestroyVi(&g_stViConfig);
		// Close ISP device.
		s32Ret = SAMPLE_COMM_VI_CLOSE();
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vi close failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
		// select which mode want to switch.
		printf("Please select sensor input mode (0:linear/1:wdr) :");
		scanf("%d", &tmp);
		wdrMode = tmp;
		if (wdrMode == 0) {
			// Reset main sensor initial config to linear setting.
			g_stIniCfg.enSnsType[0] = SONY_IMX327_2L_MIPI_2M_30FPS_12BIT;
			g_stIniCfg.enWDRMode[0] = WDR_MODE_NONE;
			// Reset slave sensor initial config to linear setting.
			g_stIniCfg.enSnsType[1] = SONY_IMX327_SLAVE_MIPI_2M_30FPS_12BIT;
			g_stIniCfg.enWDRMode[1] = WDR_MODE_NONE;
		} else {
			// Reset main sensor initial config to wdr setting.
			g_stIniCfg.enSnsType[0] = SONY_IMX327_2L_MIPI_2M_30FPS_12BIT_WDR2TO1;
			g_stIniCfg.enWDRMode[0] = WDR_MODE_2To1_LINE;
			// Reset slave sensor initial config to wdr setting.
			g_stIniCfg.enSnsType[1] = SONY_IMX327_SLAVE_MIPI_2M_30FPS_12BIT_WDR2TO1;
			g_stIniCfg.enWDRMode[1] = WDR_MODE_2To1_LINE;
		}
		// Reconfig VI setting for different mode Re-initial correctly.
		ViConfigReInit(&g_stViConfig, &g_stIniCfg);
		// open Isp device.
		s32Ret = SAMPLE_COMM_VI_OPEN();
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vi open failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
		// Initial VI & ISP.
		s32Ret = SAMPLE_PLAT_VI_INIT(&g_stViConfig);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vi init failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
		break;
	}

	case 113: {
		ISP_VD_TYPE_E vdType = ISP_VD_MAX;
		CVI_U32 timeOut = 500;
		CVI_S32 s32Ret = 0;

		for (vdType = ISP_VD_FE_START; vdType < ISP_VD_MAX; vdType++) {
			s32Ret = CVI_ISP_GetVDTimeOut(0, vdType, timeOut);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "wait Vd time out. type: %d s32Ret: %#x\n",
					vdType, s32Ret);
			}
		}
		break;
	}

	case 114: {
		CVI_S32 s32Ret = 0;

#ifndef FPGA_PORTING
		s32Ret = _isp_test();
#endif
		if (s32Ret == CVI_FAILURE) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "ISP auto test NG\n");
		}
		break;
	}

	case 116: {
		CVI_S32 s32Ret = 0;

#ifndef FPGA_PORTING
		s32Ret = _isp_raw_replay_test();
#endif
		if (s32Ret == CVI_FAILURE) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "ISP raw replay test NG\n");
		}
		break;
	}

	case 120:
#ifndef FPGA_PORTING
		CVI_AE_AutoTest(0);
#endif
		break;

	case 121:
#ifndef FPGA_PORTING
		CVI_AWB_AutoTest(0);
#endif
		break;

	case 130: {
		VO_DEV VoDev;
		VO_LAYER VoLayer;

		CVI_TRACE_LOG(CVI_DBG_WARN, "vo device:(0/1)\n");
		scanf("%d", &VoDev);
		VoDev %= 2;
		s32Ret = SAMPLE_PLAT_VO_INIT_BT656(VoDev);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vo init_bt656 failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
		VoLayer = VoDev;

		//CVI_VO_SetChnRotation(0, 0, ROTATION_90);
		SAMPLE_COMM_VPSS_Bind_VO(0, 0, VoLayer, 0);
		SAMPLE_COMM_VO_Init_BT656_MS7024("/dev/i2c-3", 0x76, 2); //MS7024_DVIN_576P50
		break;
	}

	case 131: {
		VO_DEV VoDev;
		VO_LAYER VoLayer;

		CVI_TRACE_LOG(CVI_DBG_WARN, "vo device:(0/1)\n");
		scanf("%d", &VoDev);
		VoDev %= 2;
		s32Ret = SAMPLE_PLAT_VO_INIT_BT656(VoDev);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vo init_bt656 failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
		VoLayer = VoDev;

		//CVI_VO_SetChnRotation(VoLayer, 0, ROTATION_90);
		SAMPLE_COMM_VPSS_Bind_VO(0, 0, VoLayer, 0);
		SAMPLE_COMM_VO_Init_BT656_MS7024("/dev/i2c-3", 0x76, 1); //MS7024_INTERNAL_PATTERN
		break;
	}

	case 132: {
		VO_DEV VoDev;
		VO_LAYER VoLayer;

		CVI_TRACE_LOG(CVI_DBG_WARN, "vo device:(0/1)\n");
		scanf("%d", &VoDev);
		VoDev %= 2;
		s32Ret = SAMPLE_PLAT_VO_INIT_BT656(VoDev);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vo init_bt656 failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
		VoLayer = VoDev;

		//CVI_VO_SetChnRotation(0VoLayer, 0, ROTATION_90);
		SAMPLE_COMM_VPSS_Bind_VO(0, 0, VoLayer, 0);
		SAMPLE_COMM_VO_Init_BT656_MS7024("/dev/i2c-3", 0x76, 3); //MS7024_DVIN_480P60
		break;
	}

	case 133: {
		//test ISP set/get
#ifdef ARCH_CV183X
		TEST_ISP_API(CVI_ISP_GetClutCoeff, CVI_ISP_SetClutCoeff, ISP_CLUT_LUT_S);
		TEST_ISP_API(CVI_ISP_GetDemosaicEEAttr, CVI_ISP_SetDemosaicEEAttr, ISP_DEMOSAIC_EE_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetHSVAttr, CVI_ISP_SetHSVAttr, ISP_HSV_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetDCFInfo, CVI_ISP_SetDCFInfo, ISP_DCF_INFO_S);
#else
		TEST_ISP_API(CVI_ISP_GetRGBCACAttr, CVI_ISP_SetRGBCACAttr, ISP_RGBCAC_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetCCMSaturationAttr, CVI_ISP_SetCCMSaturationAttr, ISP_CCM_SATURATION_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetClutHslAttr, CVI_ISP_SetClutHslAttr, ISP_CLUT_HSL_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetCAAttr, CVI_ISP_SetCAAttr, ISP_CA_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetPreSharpenAttr, CVI_ISP_SetPreSharpenAttr, ISP_PRESHARPEN_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetTNRMotionAdaptAttr, CVI_ISP_SetTNRMotionAdaptAttr, ISP_TNR_MOTION_ADAPT_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetCNRMotionNRAttr, CVI_ISP_SetCNRMotionNRAttr, ISP_CNR_MOTION_NR_ATTR_S);
#endif
		TEST_ISP_API(CVI_ISP_GetCrosstalkAttr, CVI_ISP_SetCrosstalkAttr, ISP_CROSSTALK_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetPubAttr, CVI_ISP_SetPubAttr, ISP_PUB_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetCtrlParam, CVI_ISP_SetCtrlParam, ISP_CTRL_PARAM_S);
		TEST_ISP_API(CVI_ISP_GetBindAttr, CVI_ISP_SetBindAttr, ISP_BIND_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetModuleControl, CVI_ISP_SetModuleControl, ISP_MODULE_CTRL_U);
		TEST_ISP_API(CVI_ISP_GetColorToneAttr, CVI_ISP_SetColorToneAttr, ISP_COLOR_TONE_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetDehazeAttr, CVI_ISP_SetDehazeAttr, ISP_DEHAZE_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetGammaAttr, CVI_ISP_SetGammaAttr, ISP_GAMMA_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetAutoGammaAttr, CVI_ISP_SetAutoGammaAttr, ISP_AUTO_GAMMA_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetStatisticsConfig, CVI_ISP_SetStatisticsConfig, ISP_STATISTICS_CFG_S);
		TEST_ISP_API(CVI_ISP_GetNRAttr, CVI_ISP_SetNRAttr, ISP_NR_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetNRFilterAttr, CVI_ISP_SetNRFilterAttr, ISP_NR_FILTER_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetYNRAttr, CVI_ISP_SetYNRAttr, ISP_YNR_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetYNRMotionNRAttr, CVI_ISP_SetYNRMotionNRAttr, ISP_YNR_MOTION_NR_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetYNRFilterAttr, CVI_ISP_SetYNRFilterAttr, ISP_YNR_FILTER_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetCNRAttr, CVI_ISP_SetCNRAttr, ISP_CNR_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetCACAttr, CVI_ISP_SetCACAttr, ISP_CAC_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetDCIAttr, CVI_ISP_SetDCIAttr, ISP_DCI_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetMeshShadingAttr, CVI_ISP_SetMeshShadingAttr, ISP_MESH_SHADING_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetMeshShadingGainLutAttr, CVI_ISP_SetMeshShadingGainLutAttr,
						ISP_MESH_SHADING_GAIN_LUT_ATTR_S);
#if defined(__CV181X__) || defined(__CV180X__) || defined(__CV186X__)
		//TEST_ISP_API(CVI_ISP_GetRadialShadingAttr, CVI_ISP_SetRadialShadingAttr, ISP_RADIAL_SHADING_ATTR_S);
#else
		TEST_ISP_API(CVI_ISP_GetRadialShadingAttr, CVI_ISP_SetRadialShadingAttr, ISP_RADIAL_SHADING_ATTR_S);
#endif
		TEST_ISP_API(CVI_ISP_GetTNRAttr, CVI_ISP_SetTNRAttr, ISP_TNR_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetTNRNoiseModelAttr, CVI_ISP_SetTNRNoiseModelAttr, ISP_TNR_NOISE_MODEL_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetTNRLumaMotionAttr, CVI_ISP_SetTNRLumaMotionAttr, ISP_TNR_LUMA_MOTION_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetTNRGhostAttr, CVI_ISP_SetTNRGhostAttr, ISP_TNR_GHOST_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetTNRMtPrtAttr, CVI_ISP_SetTNRMtPrtAttr, ISP_TNR_MT_PRT_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetClutAttr, CVI_ISP_SetClutAttr, ISP_CLUT_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetBlackLevelAttr, CVI_ISP_SetBlackLevelAttr, ISP_BLACK_LEVEL_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetDemosaicAttr, CVI_ISP_SetDemosaicAttr, ISP_DEMOSAIC_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetDemosaicDemoireAttr, CVI_ISP_SetDemosaicDemoireAttr,
						ISP_DEMOSAIC_DEMOIRE_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetSaturationAttr, CVI_ISP_SetSaturationAttr, ISP_SATURATION_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetCCMAttr, CVI_ISP_SetCCMAttr, ISP_CCM_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetDPDynamicAttr, CVI_ISP_SetDPDynamicAttr, ISP_DP_DYNAMIC_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetDPStaticAttr, CVI_ISP_SetDPStaticAttr, ISP_DP_STATIC_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetCrosstalkAttr, CVI_ISP_SetCrosstalkAttr, ISP_CROSSTALK_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetFSWDRAttr, CVI_ISP_SetFSWDRAttr, ISP_FSWDR_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetDRCAttr, CVI_ISP_SetDRCAttr, ISP_DRC_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetSharpenAttr, CVI_ISP_SetSharpenAttr, ISP_SHARPEN_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetNoiseProfileAttr, CVI_ISP_SetNoiseProfileAttr, ISP_CMOS_NOISE_CALIBRATION_S);
		TEST_ISP_API(CVI_ISP_GetYContrastAttr, CVI_ISP_SetYContrastAttr, ISP_YCONTRAST_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetMonoAttr, CVI_ISP_SetMonoAttr, ISP_MONO_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetDisAttr, CVI_ISP_SetDisAttr, ISP_DIS_ATTR_S);
		TEST_ISP_API(CVI_ISP_GetDisConfig, CVI_ISP_SetDisConfig, ISP_DIS_CONFIG_S);
		TEST_ISP_API(CVI_ISP_GetVCAttr, CVI_ISP_SetVCAttr, ISP_VC_ATTR_S);
		break;
	}

	case 200: {
		LOG_LEVEL_CONF_S log_conf;
		CVI_CHAR buffer[20];

		for (CVI_U8 i = 0; i < CVI_ID_BUTT; ++i) {
			if ((i % 3) == 0)
				printf("\n");
			log_conf.enModId = i;
			CVI_LOG_GetLevelConf(&log_conf);
			snprintf(buffer, sizeof(buffer), "%s(%d): %d", CVI_SYS_GetModName(i), i, log_conf.s32Level);
			printf("%-19s", buffer);
		}
		printf("\n");
		printf("ModID(111 for all):\n");
		scanf("%d", &tmp);
		log_conf.enModId = tmp;
		printf("Log level(0~7):\n");
		scanf("%d", &log_conf.s32Level);
		if (log_conf.enModId != 111)
			CVI_LOG_SetLevelConf(&log_conf);
		else {
			for (CVI_U8 i = 0; i < CVI_ID_BUTT; ++i) {
				log_conf.enModId = i;
				CVI_LOG_SetLevelConf(&log_conf);
			}
		}
		break;
	}
	case 201: {
		LOG_LEVEL_CONF_S log_conf;
		CVI_CHAR buffer[20];

		for (CVI_U8 i = 0; i < CVI_ID_BUTT; ++i) {
			if ((i % 3) == 0)
				printf("\n");
			log_conf.enModId = i;
			CVI_LOG_GetLevelConf(&log_conf);
			snprintf(buffer, sizeof(buffer), "%s(%d): %d", CVI_SYS_GetModName(i), i, log_conf.s32Level);
			printf("%-19s", buffer);
		}
		printf("\n");
		break;
	}
	case 203: {
		int devno = 0;
		VO_PM_OPS_S vo_ops = {
			.pfnPanelSuspend = NULL,
			.pfnPanelResume = SAMPLE_COMM_VO_Init_MIPI_HX8394,
		};

		s32Ret = CVI_VO_RegPmCallBack(0, &vo_ops, (void *)(long)devno);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VO_RegPmCallBack failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}

		s32Ret = CVI_VI_Suspend();
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("Vi suspend failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}

		// Fixme: A2 suspend isn't ready
		/*
		s32Ret = CVI_MISC_SysSuspend();
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("Sys suspend failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
		*/
		break;
	}
	case 204: {
		// Fixme: A2 resume isn't ready
		/*
		s32Ret = CVI_MISC_SysResume();
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("Sys resume failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
		*/

		s32Ret = CVI_VI_Resume();
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("Vi resume failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
		break;
	}
	case 205: {
		const int pfd = open("/sys/power/state", O_RDWR);
		char buf[32];
		int len;
		int devno = 0;

		VO_PM_OPS_S vo_ops = {
			.pfnPanelSuspend = NULL,
			.pfnPanelResume = SAMPLE_COMM_VO_Init_MIPI_HX8394,
		};

		s32Ret = CVI_VO_RegPmCallBack(0, &vo_ops, (void *)(long)devno);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VO_RegPmCallBack failed with %#x!\n", s32Ret);
			return CVI_FAILURE;
		}

		len = sprintf(buf, "mem");
		len = write(pfd, buf, len);
		close(pfd);
		break;
	}
	case 206: {
		CVI_S32 mod_select;

		do {
			SAMPLE_PRT("---Show Procfs Info------------------------------------\n");
			SAMPLE_PRT("0: vi  1: vi_dbg  2: vb    3: sys    4: vpss  5: rgn  6: gdc\n");
			SAMPLE_PRT("7: vo  8: vo_disp 9: venc  10: vdec  11: log  12: isp\n");
			SAMPLE_PRT("13: h264e  14: h265e  15: jpege  16: mipi_tx\n");
			SAMPLE_PRT("255: exit procfs test\n");
			scanf("%d", &mod_select);

			switch (mod_select) {
			case 0:
				system("cat /proc/soph/vi");
				break;
			case 1:
				system("cat /proc/soph/vi_dbg");
				break;
			case 2:
				system("cat /proc/soph/vb");
				break;
			case 3:
				system("cat /proc/soph/sys");
				break;
			case 4:
				system("cat /proc/soph/vpss");
				break;
			case 5:
				system("cat /proc/soph/rgn");
				break;
			case 6:
				system("cat /proc/soph/gdc");
				break;
			case 7:
				system("cat /proc/soph/vo");
				break;
			case 8:
				system("cat /proc/soph/vo_disp");
				break;
			case 9:
				system("cat /proc/soph/venc");
				break;
			case 10:
				system("cat /proc/soph/vdec");
				break;
			case 11:
				system("cat /proc/soph/log");
				break;
			case 12:
				system("cat /proc/soph/isp");
				break;
			case 13:
				system("cat /proc/soph/h264e");
				break;
			case 14:
				system("cat /proc/soph/h265e");
				break;
			case 15:
				system("cat /proc/soph/jpege");
				break;
			case 16:
				system("cat /proc/soph/mipi_tx");
				break;
			default:
				break;
			}
		} while (mod_select != 255);
		break;
	}
	case 207: {
		MESH_DUMP_ATTR_S meshDumpAttr = {0};

		SAMPLE_PRT("pls input file name: ");
		scanf("%s", meshDumpAttr.binFileName);
		SAMPLE_PRT("MOD:(6:VPSS, 14:VI) ");
		scanf("%d", &tmp);
		meshDumpAttr.enModId = (MOD_ID_E)tmp;
		switch (meshDumpAttr.enModId) {
		case CVI_ID_VI:
			SAMPLE_PRT("chn: (0~1) ");
			scanf("%d", &meshDumpAttr.viMeshAttr.chn);
			break;
		case CVI_ID_VPSS:
			SAMPLE_PRT("grp: (0~15) ");
			scanf("%d", &meshDumpAttr.vpssMeshAttr.grp);
			SAMPLE_PRT("chn: (0~3) ");
			scanf("%d", &meshDumpAttr.vpssMeshAttr.chn);
			break;
		default:
			SAMPLE_PRT("not supported mod:(%d)\n", meshDumpAttr.enModId);
			return CVI_FAILURE;
		}
		// CVI_GDC_DumpMesh(&meshDumpAttr);
		break;
	}
	case 208: {
		MESH_DUMP_ATTR_S meshDumpAttr = {0};

		SAMPLE_PRT("pls input file name: ");
		scanf("%s", meshDumpAttr.binFileName);
		SAMPLE_PRT("MOD:(6:VPSS, 14:VI) ");
		scanf("%d", &tmp);
		meshDumpAttr.enModId = (MOD_ID_E)tmp;
		switch (meshDumpAttr.enModId) {
		case CVI_ID_VI:
			SAMPLE_PRT("chn: (0~1) ");
			scanf("%d", &meshDumpAttr.viMeshAttr.chn);
			break;
		case CVI_ID_VPSS:
			SAMPLE_PRT("grp: (0~15) ");
			scanf("%d", &meshDumpAttr.vpssMeshAttr.grp);
			SAMPLE_PRT("chn: (0~3) ");
			scanf("%d", &meshDumpAttr.vpssMeshAttr.chn);
			break;
		default:
			SAMPLE_PRT("not supported mod:(%d)\n", meshDumpAttr.enModId);
			return CVI_FAILURE;
		}
		// CVI_GDC_LoadMesh(&meshDumpAttr, NULL);
		break;
	}
	default:
		break;
	}

	return s32Ret;
}

int runMw(CVI_S32 cmd)
{
	MMF_VERSION_S stVersion;
	SAMPLE_INI_CFG_S	   stIniCfg = {0};
	SAMPLE_VI_CONFIG_S stViConfig;

	PIC_SIZE_E enPicSize;
	SIZE_S stSize;
	CVI_S32 s32Ret = CVI_SUCCESS;
	LOG_LEVEL_CONF_S log_conf;
	CVI_S32 op = cmd;

	stIniCfg = (SAMPLE_INI_CFG_S) {
		.enSource  = VI_PIPE_FRAME_SOURCE_DEV,
		.devNum    = 1,
		.enSnsType[0] = SONY_IMX327_MIPI_2M_30FPS_12BIT,
		.enWDRMode[0] = WDR_MODE_NONE,
		.s32BusId[0]  = 3,
		.s32SnsI2cAddr[0] = -1,
		.MipiDev[0]   = 0xFF,
		.u8UseMultiSns = 0,
	};
#ifdef MTRACE
	printf("[MTRACE] Test Start\n");
	mtrace();
#endif
	CVI_SYS_GetVersion(&stVersion);
	SAMPLE_PRT("MMF Version:%s\n", stVersion.version);

	log_conf.enModId = CVI_ID_LOG;
	log_conf.s32Level = CVI_DBG_INFO;
	CVI_LOG_SetLevelConf(&log_conf);

	// Get config from ini if found.
	s32Ret = SAMPLE_COMM_VI_ParseIni(&stIniCfg);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Parse fail\n");
	} else {
		SAMPLE_PRT("Parse complete\n");
	}

	//Set sensor number
	CVI_VI_SetDevNum(stIniCfg.devNum);

	/************************************************
	 * step1:  Config VI
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_IniToViCfg(&stIniCfg, &stViConfig);
	if (s32Ret != CVI_SUCCESS)
		return s32Ret;

	memcpy(&g_stViConfig, &stViConfig, sizeof(SAMPLE_VI_CONFIG_S));
	memcpy(&g_stIniCfg, &stIniCfg, sizeof(SAMPLE_INI_CFG_S));

	/************************************************
	 * step2:  Get input size
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(stIniCfg.enSnsType[0], &enPicSize);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VI_GetSizeBySensor failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_SYS_GetPicSize failed with %#x\n", s32Ret);
		return s32Ret;
	}

	/************************************************
	 * step3:  Init modules
	 ************************************************/
	s32Ret = SAMPLE_PLAT_SYS_INIT(stSize);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "sys init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

#ifndef FPGA_PORTING
	if (stIniCfg.enSource == VI_PIPE_FRAME_SOURCE_DEV) {
		s32Ret = SAMPLE_PLAT_VI_INIT(&stViConfig);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "vi init failed. s32Ret: 0x%x !\n", s32Ret);
			return s32Ret;
		}
	}
#endif
	system("stty erase ^H");

	if (op != -1) {
		s32Ret = _handle_op(op, &stIniCfg, &stViConfig);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "op(%d) failed with %#x!\n", op, s32Ret);
			return s32Ret;
		}
	} else {
		do {

			SAMPLE_PRT("---Basic------------------------------------------------\n");
			SAMPLE_PRT("0: start vi                 1: stop vi\n");
			SAMPLE_PRT("2: dump vi yuv frame\n");
			SAMPLE_PRT("3: vi bind vpss with manul chn\n");
			SAMPLE_PRT("5: start vpss               6: stop vpss\n");
			SAMPLE_PRT("7: start vo                 8: stop vo\n");
			SAMPLE_PRT("9: Set VI/VPSS Mode\n");
			SAMPLE_PRT("---VI---------------------------------------------------\n");
			SAMPLE_PRT("10: set usr-pic		11. send usr-pic\n");
			SAMPLE_PRT("12. release usr-pic\n");
			SAMPLE_PRT("13: dump vi register\n");
			SAMPLE_PRT("14: set chn flip/mirror\n");
			SAMPLE_PRT("15: dump vi raw frame\n");
			SAMPLE_PRT("16: set chn crop            17: get chn crop\n");
			SAMPLE_PRT("18: pipefd ctrl\n");
			SAMPLE_PRT("19: set vi rotation         29: set up ldc\n");
			SAMPLE_PRT("25: set/get pipe crop\n");
			SAMPLE_PRT("---VO---------------------------------------------------\n");
			SAMPLE_PRT("20: send vo frame\n");
			SAMPLE_PRT("21: Layer on/off            22: Chn Show/Hide\n");
			SAMPLE_PRT("23: Close vo fd             24: Get vo info\n");
			SAMPLE_PRT("26: Get ProcAmp             27: Set ProcAmp\n");
			SAMPLE_PRT("28: VO Pause/Resume\n");
			SAMPLE_PRT("---VPSS-------------------------------------------------\n");
			SAMPLE_PRT("31: get vpss chn frame\n");
			SAMPLE_PRT("32: get group crop          33: set group crop\n");
			SAMPLE_PRT("34: get chn crop            35: set chn crop\n");
			SAMPLE_PRT("36: start vpss(multi-ch)    37: start vpss rgn_ex mode\n");
			SAMPLE_PRT("38: start vpss(2 grp)       39: stop vpss(2 grp)\n");
			SAMPLE_PRT("40: send vpss grp frame     41: set vpss rotation\n");
			SAMPLE_PRT("42: Get vpss info\n");
			SAMPLE_PRT("44: Get ProcAmp             45: Set ProcAmp\n");
			SAMPLE_PRT("46: Set AspectRatio         47: Set Flip/Mirror\n");
			SAMPLE_PRT("48: Set Y Ratio             49: Set LDC\n");
			SAMPLE_PRT("---Test code-------------------------------------------\n");
			SAMPLE_PRT("50: VPSS test.\n");
			SAMPLE_PRT("51: SYS test.\n");
			SAMPLE_PRT("52: tyua test.\n");
			SAMPLE_PRT("53: VI SDK test.\n");
			SAMPLE_PRT("54: vpss dual test.\n");
			SAMPLE_PRT("55: gdc test.\n");
			SAMPLE_PRT("56: VPSS hw stress test.\n");
			SAMPLE_PRT("57: VI VPSS off/online test.\n");
			SAMPLE_PRT("---VENC------------------------------------------------\n");
			SAMPLE_PRT("60: bind with VI            61: bind with VPSS\n");
			SAMPLE_PRT("62: encode H.265            63: encode H.264\n");
			SAMPLE_PRT("64: encode MJPEG            65: encode JPEG\n");
			SAMPLE_PRT("66: start venc              67: stop venc\n");
			SAMPLE_PRT("68: get vpss info\n");
			SAMPLE_PRT("---RGN-------------------------------------------------\n");
			SAMPLE_PRT("80: Setup RGN to VPSS\n");
			SAMPLE_PRT("---VB--------------------------------------------------\n");
			SAMPLE_PRT("90: Create VB POOL          91: Destroy VB POOL\n");
			SAMPLE_PRT("92: Get Block               93: Release Block\n");
			SAMPLE_PRT("---ISP-------------------------------------------------\n");
			SAMPLE_PRT("101:set ae gain / shutter value\n");
			SAMPLE_PRT("102:show part of ae sts\n");
			SAMPLE_PRT("107:GPIO test\n");
			SAMPLE_PRT("109:sensor AE test\n");
			SAMPLE_PRT("110:AE debug\n");
			SAMPLE_PRT("111:AWB debug\n");
			SAMPLE_PRT("120:AE AutoTest\n");
			SAMPLE_PRT("121:AWB AutoTest\n");
			SAMPLE_PRT("130:BT656_MS7024 dv_in 576P50\n");
			SAMPLE_PRT("131:BT656_MS7024 module's test pattern\n");
			SAMPLE_PRT("132:BT656_MS7024 dv_in 480P60\n");
			SAMPLE_PRT("---SYS-------------------------------------------------\n");
			SAMPLE_PRT("200:set log level		201: get log level\n");
			SAMPLE_PRT("202:log to file\n");
			SAMPLE_PRT("203:Suspend			204: Resume\n");
			SAMPLE_PRT("205:Suspend to Dram\n");
			SAMPLE_PRT("206:show procfs info\n");
			SAMPLE_PRT("255: exit\n");
			scanf("%d", &op);

			s32Ret = _handle_op(op, &stIniCfg, &stViConfig);
			if (s32Ret != CVI_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "op(%d) failed with %#x!\n", op, s32Ret);
				break;
			}
		} while (op != 255);
	}

	SAMPLE_COMM_VI_DestroyIsp(&stViConfig);

	SAMPLE_COMM_VI_DestroyVi(&stViConfig);

	SAMPLE_COMM_SYS_Exit();

#ifdef MTRACE
	printf("[MTRACE] Test End\n");
	muntrace();
#endif

	return s32Ret;
}

void *runMwThread(void *param)
{
	runMw(*(CVI_S32 *)param);
	return NULL;
}

int main(int argc, char **argv)
{
#define STACKSIZE "STACKSIZE"
	CVI_S32 op = -1;

	if (argc == 2) {
		op = atoi(argv[1]);
	}

	char *pszEnv = getenv(STACKSIZE);

	if (pszEnv == NULL) {
		printf("normal run\n");
		runMw(op);
	} else {
		struct sched_param param;
		pthread_attr_t attr;
		pthread_t id;
		int ret;
		size_t stacksize;

		stacksize = atoi(pszEnv) * 1024;

		param.sched_priority = 75;//low priority
		ret |= pthread_attr_init(&attr);
		ret |= pthread_attr_setschedpolicy(&attr, SCHED_RR);
		ret |= pthread_attr_setschedparam(&attr, &param);
		ret |= pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
		ret |= pthread_attr_setstacksize(&attr, stacksize);
		ret |= pthread_attr_getstacksize(&attr, &stacksize);
		printf("use pthread(stack:%zuB) run\n", stacksize);
		ret = pthread_create(&id, &attr, runMwThread, (void *)&op);
		if (ret != 0) {
			printf("ret = %d\n", ret);
			return CVI_FAILURE;
		}
		pthread_join(id, NULL);
	}
	return CVI_SUCCESS;
}


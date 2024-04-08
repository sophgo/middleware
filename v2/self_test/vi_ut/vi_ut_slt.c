#include "vi_ut.h"

extern int _vi_set_patgen(bool enable);

CVI_S32 CompareWithFile(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	FILE *fp;
	CVI_U32 u32len, plane_len, data_len;
	CVI_U32 u32LumaData, u32ChromaData = 0, data_height;
	CVI_S32 result = CVI_SUCCESS;
	VB_CAL_CONFIG_S stVbCalConfig;

	u32LumaData = pstVideoFrame->stVFrame.u32Width;
	data_height = pstVideoFrame->stVFrame.u32Height;

	COMMON_GetPicBufferConfig(pstVideoFrame->stVFrame.u32Width, pstVideoFrame->stVFrame.u32Height,
		pstVideoFrame->stVFrame.enPixelFormat, DATA_BITWIDTH_8,
		COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_RGB_888_PLANAR ||
	    pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_BGR_888_PLANAR ||
	    pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_444) {
		u32ChromaData = u32LumaData;
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_422) {
		u32ChromaData =  (pstVideoFrame->stVFrame.u32Width / 2);
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) {
		u32ChromaData =  (pstVideoFrame->stVFrame.u32Width / 2);
		data_height = pstVideoFrame->stVFrame.u32Height / 2;
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV12 ||
		   pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV21) {
		u32ChromaData = u32LumaData;
		data_height = pstVideoFrame->stVFrame.u32Height / 2;
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV16 ||
		   pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_NV61) {
		u32ChromaData = u32LumaData;
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUYV ||
		   pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_UYVY ||
		   pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YVYU ||
		   pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_VYUY) {
		u32LumaData *= 2;
		u32ChromaData = 0;
	} else if (pstVideoFrame->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_400) {
		u32ChromaData = 0;
	}

	VI_UT_PRT("u32LumaSize(%d): u32ChromaSize(%d)\n",
		stVbCalConfig.u32MainYSize, stVbCalConfig.u32MainCSize);
	VI_UT_PRT("u32LumaData(%d): u32ChromaData(%d)\n", u32LumaData, u32ChromaData);
	fp = fopen(filename, "r");
	if (fp == CVI_NULL) {
		VI_UT_PRT("open data file, %s, error\n", filename);
		return CVI_FAILURE;
	}

	CVI_U8 buffer[stVbCalConfig.u32MainYSize];
	CVI_U32 offset = 0;

	for (int i = 0; i < stVbCalConfig.plane_num; ++i) {
		plane_len = (i == 0) ? stVbCalConfig.u32MainYSize : stVbCalConfig.u32MainCSize;
		if (plane_len == 0)
			continue;
		data_len = (i == 0) ? u32LumaData : u32ChromaData;
		offset = 0;

		pstVideoFrame->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[i], pstVideoFrame->stVFrame.u32Length[i]);

		u32len = fread(buffer, plane_len, 1, fp);
		if (u32len <= 0) {
			VI_UT_PRT("fread data(%d) error\n", i);
			result = CVI_FAILURE;
			break;
		}
		// line by line check to avoid padding data mismatch problem.
		for (CVI_U32 line = 0; line < data_height; ++line) {
			if (memcmp(buffer + offset, pstVideoFrame->stVFrame.pu8VirAddr[i] + offset, data_len) != 0) {
				VI_UT_PRT("plane(%d) line(%d) offset(%d) data mismatch:\n",
					      i, line, offset);
				VI_UT_PRT(" paddr(%#"PRIx64") vaddr(%p) stride(%d)\n",
					      pstVideoFrame->stVFrame.u64PhyAddr[i],
					      pstVideoFrame->stVFrame.pu8VirAddr[i],
					      pstVideoFrame->stVFrame.u32Stride[i]);

				result = CVI_FAILURE;
				break;
			}
			offset += pstVideoFrame->stVFrame.u32Stride[i];
		}
		CVI_SYS_Munmap(pstVideoFrame->stVFrame.pu8VirAddr[i], pstVideoFrame->stVFrame.u32Length[i]);
	}

	fclose(fp);
	return result;
}

CVI_S32 vi_ut_slt(char *filename)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_PIPE ViPipe = 0;
	VI_PIPE_ATTR_S stPipeAttr;
	CVI_S32 i = 0, j = 0;
	CVI_S32 s32WorkSnsId = 0;

	s32Ret = vi_ut_plat_sys_init();
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("vi_ut_plat_sys_init failed. s32Ret: 0x%x !\n", s32Ret);
		goto error;
	}

	stPipeAttr.enCompressMode = vi_ut_ctx.enCompressMode;
	VI_UT_PRT("stPipeAttr.enCompressMode (%d)\n", stPipeAttr.enCompressMode);

	s32Ret = SAMPLE_COMM_VI_StartSensor(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("system start sensor failed with %#x\n", s32Ret);
		goto error;
	}

	for (i = 0; i < stViConfig.s32WorkingViNum; i++) {
		s32Ret = SAMPLE_COMM_VI_StartDev(&stViConfig.astViInfo[i]);
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("VI_StartDev failed with %#x!\n", s32Ret);
			goto error;
		}
	}

	s32Ret = SAMPLE_COMM_VI_StartMIPI(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("system start MIPI failed with %#x\n", s32Ret);
		goto error;
	}

	s32Ret = SAMPLE_COMM_VI_SensorProbe(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("system sensor probe failed with %#x\n", s32Ret);
		goto error;
	}

	for (i = 0; i < stViConfig.s32WorkingViNum; i++) {
		CVI_S32 s32DevNum;
		SAMPLE_VI_INFO_S *pstViInfo = NULL;

		s32DevNum = stViConfig.as32WorkingViId[i];
		pstViInfo = &stViConfig.astViInfo[s32DevNum];

		stPipeAttr.u32MaxW = vi_ut_ctx.stSize[i].u32Width;
		stPipeAttr.u32MaxH = vi_ut_ctx.stSize[i].u32Height;
		stPipeAttr.enPixFmt = PIXEL_FORMAT_RGB_BAYER_12BPP;
		stPipeAttr.enBitWidth = DATA_BITWIDTH_12;
		stPipeAttr.stFrameRate.s32SrcFrameRate = -1;
		stPipeAttr.stFrameRate.s32DstFrameRate = -1;
		stPipeAttr.bNrEn = CVI_TRUE;
		stPipeAttr.bYuvBypassPath = CVI_FALSE;

		for (j = 0; j < WDR_MAX_PIPE_NUM; j++) {
			if (pstViInfo->stPipeInfo.aPipe[j] >= 0 && pstViInfo->stPipeInfo.aPipe[j] < VI_MAX_PIPE_NUM) {
				ViPipe = pstViInfo->stPipeInfo.aPipe[j];
				s32Ret = CVI_VI_CreatePipe(ViPipe, &stPipeAttr);
				if (s32Ret != CVI_SUCCESS) {
					VI_UT_PRT("CVI_VI_CreatePipe failed with %#x!\n", s32Ret);
					goto error;
				}

				s32Ret = CVI_VI_StartPipe(ViPipe);
				if (s32Ret != CVI_SUCCESS) {
					VI_UT_PRT("CVI_VI_StartPipe failed with %#x!\n", s32Ret);
					goto error;
				}
			}
		}
	}

	s32Ret = SAMPLE_COMM_VI_CreateIsp(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("SAMPLE_COMM_VI_CreateIsp failed !\n");
		return CVI_FAILURE;
	}

	s32Ret = _vi_set_patgen(vi_ut_ctx.is_patgen_enable);
	if (s32Ret < 0) {
		VI_UT_PRT( "set patgen failed\n");
		return CVI_FAILURE;
	}

	// set i2c for sensor pattern
	CVI_S32 i2c_fd = -1;
	char buf[128] = {0};

	switch (stViConfig.astViInfo[s32WorkSnsId].stSnsInfo.enSnsType) {
	case SONY_IMX307_MIPI_2M_30FPS_12BIT:
	case SONY_IMX327_MIPI_1M_30FPS_10BIT:
	case SONY_IMX327_MIPI_2M_30FPS_12BIT:
		sprintf(buf, "/dev/i2c-%d", stViConfig.astViInfo[s32WorkSnsId].stSnsInfo.s32BusId);
		i2c_fd = SAMPLE_COMM_I2C_Open(buf);
		SAMPLE_COMM_I2C_Write(i2c_fd, 0x1a, 0x308c, 0x21, 2, 1);
		SAMPLE_COMM_I2C_Close(i2c_fd);
		break;
	case GCORE_GC4653_MIPI_4M_30FPS_10BIT:
		sprintf(buf, "/dev/i2c-%d", stViConfig.astViInfo[s32WorkSnsId].stSnsInfo.s32BusId);
		i2c_fd = SAMPLE_COMM_I2C_Open(buf);
		SAMPLE_COMM_I2C_Write(i2c_fd, 0x29, 0x008c, 0x11, 2, 1);
		SAMPLE_COMM_I2C_Close(i2c_fd);
		break;
	default:
		break;
	}

	s32Ret = vi_ut_plat_vi_startchn(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("VI_StartViChn failed with %#x!\n", s32Ret);
		goto error;
	}

	VI_UT_PRT("Wait for ISP to stabilize...\n");

	VI_CHN_STATUS_S stChnStatus = {0};
	struct timeval tv_0, tv_1;

	gettimeofday(&tv_0, NULL);
	do {
		usleep(10 * 1000);

		gettimeofday(&tv_1, NULL);
		s32Ret = CVI_VI_QueryChnStatus(0, 0, &stChnStatus);
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("CVI_VI_QueryChnStatus fail\n");
			break;
		}

		if (stChnStatus.u32RecvPic > 30) {
			VI_UT_PRT("vi has got stable data\n");
			s32Ret = CVI_SUCCESS;
			break;
		}

		if (tv_1.tv_sec - tv_0.tv_sec > 30) {
			VI_UT_PRT("vi chn has no data\n");
			s32Ret = CVI_FAILURE;
			break;
		}
	} while (1);

	if (s32Ret == CVI_SUCCESS) {
		VIDEO_FRAME_INFO_S stVideoFrame;

		memset(&stVideoFrame, 0, sizeof(stVideoFrame));
		if (CVI_VI_GetChnFrame(0, 0, &stVideoFrame, 3000) == 0) {

			s32Ret = vi_ut_save_frame2file(filename, &stVideoFrame);
			s32Ret = CompareWithFile("res/vi_slt_golden.yuv", &stVideoFrame);

			if (CVI_VI_ReleaseChnFrame(0, 0, &stVideoFrame) != 0)
				VI_UT_PRT("CVI_VI_ReleaseChnFrame NG\n");

			if (s32Ret != CVI_SUCCESS)
				return s32Ret;
		}
	}

error:
	return s32Ret;
}

#include "vi_ut.h"

//#define DUMP_YUV

static CVI_S32 _set_usr_pic(void)
{
	CVI_S32 s32Ret;
	int enable;
	VI_DEV_TIMING_ATTR_S stTimingAttr;

	// VI_UT_PRT("enable(1)/disable(0): ");
	// scanf("%d", &enable);
	enable = 1;
	stTimingAttr.bEnable = enable;
	if (enable) {
		// VI_UT_PRT("update fps: ");
		// scanf("%d", &stTimingAttr.s32FrmRate);
#ifdef FPGA_PORTING
		stTimingAttr.s32FrmRate = 1;
#else
		stTimingAttr.s32FrmRate = 25;
#endif

		s32Ret = CVI_VI_SetPipeFrameSource(0, VI_PIPE_FRAME_SOURCE_USER_FE);
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("CVI_VI_SetPipeFrameSource failed with %#x\n", s32Ret);
			return s32Ret;
		}
	} else {
		stTimingAttr.s32FrmRate = 0;
		CVI_VI_SetPipeFrameSource(0, VI_PIPE_FRAME_SOURCE_DEV);
	}

	s32Ret = CVI_VI_SetDevTimingAttr(0, &stTimingAttr);
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("CVI_VI_SetDevTimingAttr failed with %#x\n", s32Ret);
		return s32Ret;
	}

	VI_UT_PRT("set usr-pic TEST-PASS\n");

	return s32Ret;
}

static CVI_S32 _send_usr_pic(void)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	const VIDEO_FRAME_INFO_S *pstVideoFrame[1];
	VI_PIPE PipeId[] = {0};
	FILE *fp_le = NULL, *fp_se = NULL;
	CVI_CHAR filename_le[64] = "res/2k_ballon_bayer_12_GR.bin";
	CVI_CHAR filename_se[64] = "res/2k_ballon_bayer_12_GR.bin";
	CVI_U32 u32len = 0, is_hdr_on = 0, value = 0;
	CVI_U32 u32BlkSize = 0;
	CVI_U64 u64PhyAddr_le = 0, u64PhyAddr_se = 0;
	CVI_VOID *puVirAddr_le = 0, *puVirAddr_se = 0;

	static VB_BLK vi_usr_blk_le = VB_INVALID_HANDLE, vi_usr_blk_se = VB_INVALID_HANDLE;

	memset(&stVideoFrame, 0, sizeof(stVideoFrame));
	pstVideoFrame[0] = NULL;

	if (vi_ut_ctx.enCompressMode == COMPRESS_MODE_NONE)
		VI_UT_PRT("dpcm off, make sure to load non-dpcm raw\n");
	else
		VI_UT_PRT("dpcm on, make sure to load dpcm raw\n");

	if (!vi_ut_ctx.is_test_mode) {
		VI_UT_PRT("is_hdr_input? (0:no, 1:yes): ");
		scanf("%d", &is_hdr_on);

		VI_UT_PRT("LE filename: ");
		scanf("%s", filename_le);
		fp_le = fopen(filename_le, "r");
		if (fp_le == CVI_NULL) {
			VI_UT_PRT("open data file error\n");
			return CVI_FAILURE;
		}

		if (is_hdr_on == 1) {
			vi_ut_ctx.is_hdr_enable = 1;
			stVideoFrame.stVFrame.enDynamicRange = DYNAMIC_RANGE_HDR10;

			VI_UT_PRT("SE filename: ");
			scanf("%s", filename_se);
			fp_se = fopen(filename_se, "r");
			if (fp_se == CVI_NULL) {
				VI_UT_PRT("open data file error\n");
				return CVI_FAILURE;
			}
		}

		VI_UT_PRT("img width: ");
		scanf("%d", &stVideoFrame.stVFrame.u32Width);
		VI_UT_PRT("img height: ");
		scanf("%d", &stVideoFrame.stVFrame.u32Height);
		VI_UT_PRT("crop left offset: ");
		scanf("%hd", &stVideoFrame.stVFrame.s16OffsetLeft);
		VI_UT_PRT("crop top offset: ");
		scanf("%hd", &stVideoFrame.stVFrame.s16OffsetTop);
		VI_UT_PRT("crop right offset: ");
		scanf("%hd", &stVideoFrame.stVFrame.s16OffsetRight);
		VI_UT_PRT("crop bottom offset: ");
		scanf("%hd", &stVideoFrame.stVFrame.s16OffsetBottom);
		VI_UT_PRT("bayer format BG(0)/GB(1)/GR(2)/RG(3): ");
		scanf("%d", &value);
		stVideoFrame.stVFrame.enBayerFormat = value;
	} else {
		if (strlen(vi_ut_ctx.filepath_le) == 0) {
			memcpy(&vi_ut_ctx.filepath_le, filename_le, sizeof(filename_le));
		}
		VI_UT_PRT("open %s file\n", vi_ut_ctx.filepath_le);
		fp_le = fopen(vi_ut_ctx.filepath_le, "r");
		if (fp_le == CVI_NULL) {
			VI_UT_PRT("open data file error\n");
			return CVI_FAILURE;
		}

		if (vi_ut_ctx.is_hdr_on == 1) {
			is_hdr_on = 1;
			vi_ut_ctx.is_hdr_enable = 1;
			stVideoFrame.stVFrame.enDynamicRange = DYNAMIC_RANGE_HDR10;

			if (strlen(vi_ut_ctx.filepath_se) == 0) {
				memcpy(&vi_ut_ctx.filepath_se, filename_se, sizeof(filename_se));
			}
			VI_UT_PRT("SE filename: %s", vi_ut_ctx.filepath_se);
			fp_se = fopen(vi_ut_ctx.filepath_se, "r");
			if (fp_se == CVI_NULL) {
				VI_UT_PRT("open data file error\n");
				return CVI_FAILURE;
			}
		}

		stVideoFrame.stVFrame.u32Width = vi_ut_ctx.stSize[0].u32Width;
		stVideoFrame.stVFrame.u32Height = vi_ut_ctx.stSize[0].u32Height;
		stVideoFrame.stVFrame.s16OffsetLeft = (CVI_S16)value;
		stVideoFrame.stVFrame.s16OffsetTop = (CVI_S16)value;
		stVideoFrame.stVFrame.s16OffsetRight = (CVI_S16)value;
		stVideoFrame.stVFrame.s16OffsetBottom = (CVI_S16)value;
		stVideoFrame.stVFrame.enBayerFormat = 2;
	}

	u32BlkSize = VI_GetRawBufferSize(stVideoFrame.stVFrame.u32Width,
				 stVideoFrame.stVFrame.u32Height, PIXEL_FORMAT_RGB_BAYER_12BPP,
				 vi_ut_ctx.enCompressMode, DEFAULT_ALIGN, 0);

	if (vi_usr_blk_le == VB_INVALID_HANDLE) {
		vi_usr_blk_le = CVI_VB_GetBlock(VB_INVALID_POOLID, u32BlkSize);
		if (vi_usr_blk_le == VB_INVALID_HANDLE) {
			fclose(fp_le);
			return CVI_FAILURE;
		}
	}
	u64PhyAddr_le = CVI_VB_Handle2PhysAddr(vi_usr_blk_le);
	puVirAddr_le = CVI_SYS_Mmap(u64PhyAddr_le, u32BlkSize);
	u32len = fread(puVirAddr_le, u32BlkSize, 1, fp_le);
	if (u32len <= 0) {
		VI_UT_PRT("fread raw error\n");
		fclose(fp_le);
		return CVI_FAILURE;
	}
	VI_UT_PRT("load le img to bayer buffer: size(0x%x) paddr(0x%#"PRIx64") vaddr(%p)\n",
		   u32BlkSize, u64PhyAddr_le, puVirAddr_le);

	if (vi_ut_ctx.is_hdr_enable) {
		if (vi_usr_blk_se == VB_INVALID_HANDLE) {
			vi_usr_blk_se = CVI_VB_GetBlock(VB_INVALID_POOLID, u32BlkSize);
		}
		u64PhyAddr_se = CVI_VB_Handle2PhysAddr(vi_usr_blk_se);
		puVirAddr_se = CVI_SYS_Mmap(u64PhyAddr_se, u32BlkSize);
		u32len = fread(puVirAddr_se, u32BlkSize, 1, fp_se);
		if (u32len <= 0) {
			VI_UT_PRT("fread raw error\n");
			fclose(fp_se);
			return CVI_FAILURE;
		}
		VI_UT_PRT("load se img to bayer buffer: size(0x%x) paddr(0x%#"PRIx64") vaddr(%p)\n",
			u32BlkSize, u64PhyAddr_se, puVirAddr_se);
	}

	stVideoFrame.stVFrame.u64PhyAddr[0] = u64PhyAddr_le;
	stVideoFrame.stVFrame.u64PhyAddr[1] = u64PhyAddr_se;
	pstVideoFrame[0] = &stVideoFrame;
	CVI_VI_SendPipeRaw(1, PipeId, pstVideoFrame, 80);

	fclose(fp_le);
	if (vi_ut_ctx.is_hdr_enable) {
		fclose(fp_se);
	}
	VI_UT_PRT("Send usr-pic TEST-PASS\n");

	return CVI_SUCCESS;
}

static CVI_S32 vi_ut_raw_replay(void)
{
	CVI_S32 s32Ret;

	s32Ret = _set_usr_pic();
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("_set_usr_pic() failed with %#x!\n", s32Ret);
		goto error;
	}

	s32Ret = _send_usr_pic();
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("_send_usr_pic() failed with %#x!\n", s32Ret);
		goto error;
	}

	s32Ret = vi_ut_plat_vi_init();
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("vi_ut_plat_vi_init failed. s32Ret: 0x%x !\n", s32Ret);
		goto error;
	}

error:
	return s32Ret;
}

CVI_S32 vi_raw_replay_test(void)
{
	CVI_S32 s32Ret;

	s32Ret = vi_ut_plat_sys_init();
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("vi_ut_plat_sys_init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	vi_ut_raw_replay();

	return s32Ret;
}

CVI_S32 vi_raw_replay_manual_test(void)
{
	CVI_S32 s32Ret;
	CVI_S32 loop;

	vi_ut_ctx.is_get_dpcm_mode	= 1;
	vi_ut_ctx.is_enable_sensor	= 0;
	vi_ut_ctx.is_be_online		= 0;
	vi_ut_ctx.is_post_online	= 1;
	vi_ut_ctx.is_patgen_enable	= 0;
	vi_ut_ctx.is_hdr_enable		= 0;
	vi_ut_ctx.is_vpss_online	= 0;

	system("echo 1 > /sys/module/cv186x_vi/parameters/stop_stream_en");

	s32Ret = vi_ut_plat_sys_init();
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("vi_ut_plat_sys_init failed. s32Ret: 0x%x !\n", s32Ret);
		goto error;
	}

	s32Ret = _set_usr_pic();
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("_set_usr_pic() failed with %#x!\n", s32Ret);
		goto error;
	}

	s32Ret = _send_usr_pic();
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("_send_usr_pic() failed with %#x!\n", s32Ret);
		goto error;
	}

	s32Ret = vi_ut_plat_vi_init();
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("vi_ut_plat_vi_init failed. s32Ret: 0x%x !\n", s32Ret);
		goto error;
	}

	while (1) {
		VI_UT_PRT("input 1 will dump yuv: ");
		VI_UT_PRT("input 255 will exit: ");
		scanf("%d", &loop);

		if (loop == 255)
			break;
		if (loop == 1) {
			s32Ret = vi_ut_get_chn_frame(0);
			if (s32Ret != CVI_SUCCESS) {
				VI_UT_PRT("get_chn_frame() failed with %#x!\n", s32Ret);
				goto error;
			}
			continue;
		}
		s32Ret = _set_usr_pic();
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("_set_usr_pic() failed with %#x!\n", s32Ret);
			goto error;
		}
		s32Ret = _send_usr_pic();
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("_send_usr_pic() failed with %#x!\n", s32Ret);
			goto error;
		}
	}
error:
	system("echo 0 > /sys/module/cv186x_vi/parameters/stop_stream_en");
	return s32Ret;
}

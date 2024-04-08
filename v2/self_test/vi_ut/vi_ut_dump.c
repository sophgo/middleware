#include "vi_ut.h"
#include <inttypes.h>


CVI_S32 vi_ut_save_frame2file(const CVI_CHAR *filename, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	FILE *output = NULL;
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_CROP_INFO_S crop_info;
	size_t image_size = pstVideoFrame->stVFrame.u32Length[0] + pstVideoFrame->stVFrame.u32Length[1]
				+ pstVideoFrame->stVFrame.u32Length[2];
	CVI_VOID *vir_addr;
	CVI_U32 plane_offset, u32LumaSize, u32ChromaSize;

	VI_UT_PRT("name: %s\n", filename);
	VI_UT_PRT("width: %d, height: %d, total_buf_length: %zu\n",
			pstVideoFrame->stVFrame.u32Width,
			pstVideoFrame->stVFrame.u32Height, image_size);

	u32LumaSize =  pstVideoFrame->stVFrame.u32Stride[0] * pstVideoFrame->stVFrame.u32Height;
	u32ChromaSize =  pstVideoFrame->stVFrame.u32Stride[1] * pstVideoFrame->stVFrame.u32Height / 2;

	CVI_VI_GetChnCrop(0, 0, &crop_info);
	VI_UT_PRT("crop_en=%d, x_y_w_h=%d_%d_%d_%d\n", crop_info.bEnable,
					crop_info.stCropRect.s32X, crop_info.stCropRect.s32Y,
					crop_info.stCropRect.u32Width, crop_info.stCropRect.u32Height);

	output = fopen(filename, "wb");
	if (output == NULL) {
		VI_UT_PRT("fopen fail\n");
		return CVI_FAILURE;
	}

	if (crop_info.bEnable) {
		u32LumaSize = ALIGN((crop_info.stCropRect.u32Width * 8 + 7) >> 3, DEFAULT_ALIGN) *
			ALIGN(crop_info.stCropRect.u32Height, 2);
		u32ChromaSize = (ALIGN((crop_info.stCropRect.u32Width * 8 + 7) >> 3, DEFAULT_ALIGN) *
			ALIGN(crop_info.stCropRect.u32Height, 2)) >> 1;
	}

	vir_addr = CVI_SYS_Mmap(pstVideoFrame->stVFrame.u64PhyAddr[0], image_size);
	CVI_SYS_IonInvalidateCache(pstVideoFrame->stVFrame.u64PhyAddr[0], vir_addr, image_size);
	plane_offset = 0;

	for (int i = 0; i < 3; i++) {
		if (pstVideoFrame->stVFrame.u32Length[i] != 0) {
			pstVideoFrame->stVFrame.pu8VirAddr[i] = vir_addr + plane_offset;
			plane_offset += pstVideoFrame->stVFrame.u32Length[i];
			VI_UT_PRT("plane(%d): paddr(0x%#"PRIx64") vaddr(%p) stride(%d) length(%d)\n",
					i, pstVideoFrame->stVFrame.u64PhyAddr[i],
					pstVideoFrame->stVFrame.pu8VirAddr[i],
					pstVideoFrame->stVFrame.u32Stride[i],
					pstVideoFrame->stVFrame.u32Length[i]);
			fwrite((void *)pstVideoFrame->stVFrame.pu8VirAddr[i]
				, (i == 0) ? u32LumaSize : u32ChromaSize, 1, output);
		}
	}

	CVI_SYS_Munmap(vir_addr, image_size);

	fclose(output);

	return s32Ret;
}

CVI_S32 vi_ut_smooth_rawdump(CVI_U8 dev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	VI_PIPE ViPipe = 0;
	VB_POOL PoolID;
	VB_POOL_CONFIG_S cfg;
	VI_SMOOTH_RAW_DUMP_INFO_S stDumpInfo;
	VIDEO_FRAME_INFO_S stVideoFrame[2];
	VI_DEV_ATTR_S stDevAttr;
	VI_PIPE_ATTR_S stPipeAttr;
	VI_CHN_ATTR_S stChnAttr;
	CVI_U8 BlkCnt, TotalFrameCnt;
	CVI_U64 u64PhyAddr, *phy_addr_list = CVI_NULL;
	VB_BLK vb_blk;
	int frm_num = 1;
	CVI_U32 dev_frm_w, dev_frm_h, frm_w, frm_h;
	CVI_U32 crop_x = 0, crop_y = 0, crop_w = 0, crop_h = 0;

	ViPipe = dev;
	BlkCnt = 2;
	TotalFrameCnt = 10;

	CVI_VI_GetDevAttr((VI_DEV)ViPipe, &stDevAttr);
	CVI_VI_GetChnAttr(0, (VI_CHN)ViPipe, &stChnAttr);
	CVI_VI_GetPipeAttr(ViPipe, &stPipeAttr);
	stPipeAttr.enCompressMode = vi_ut_ctx.enCompressMode;
	CVI_VI_SetPipeAttr(ViPipe, &stPipeAttr);

	dev_frm_w = stDevAttr.stSize.u32Width;
	dev_frm_h = stDevAttr.stSize.u32Height;

	frm_w = dev_frm_w;
	frm_h = dev_frm_h;

	frm_num = (stDevAttr.stWDRAttr.enWDRMode == WDR_MODE_2To1_LINE) ? 2 : 1;
	cfg.u32BlkCnt = frm_num * BlkCnt;
	cfg.u32BlkSize = VI_GetRawBufferSize(frm_w, frm_h,
					PIXEL_FORMAT_RGB_BAYER_12BPP,
					stPipeAttr.enCompressMode,
					16,
					(stChnAttr.stSize.u32Width > 2304) ? CVI_TRUE : CVI_FALSE);

	VI_UT_PRT("Create VB pool cnt(%d) blksize(0x%x)\n",
			cfg.u32BlkCnt, cfg.u32BlkSize);

	PoolID = CVI_VB_CreatePool(&cfg);
	if (PoolID == VB_INVALID_POOLID) {
		VI_UT_PRT("create vb pool failed\n");
		s32Ret = CVI_FAILURE;
		return s32Ret;
	}

	phy_addr_list = malloc(sizeof(*phy_addr_list) * cfg.u32BlkCnt);
	if (phy_addr_list == CVI_NULL) {
		VI_UT_PRT("malloc phy_addr_list failed\n");
		s32Ret = CVI_FAILURE;
		return s32Ret;
	}

	for (CVI_U32 i = 0; i < cfg.u32BlkCnt; i++) {
		vb_blk = CVI_VB_GetBlock(PoolID, cfg.u32BlkSize);
		if (vb_blk == VB_INVALID_HANDLE) {
			VI_UT_PRT("get VB blk failed\n");
			s32Ret = CVI_FAILURE;
			return s32Ret;
		}
		u64PhyAddr = CVI_VB_Handle2PhysAddr(vb_blk);
		*(phy_addr_list + i) = u64PhyAddr;
		VI_UT_PRT("i=%d, vb_blk=%#"PRIx64", addr(%#"PRIx64"), phy_addr(%#"PRIx64")\n",
					i, (intmax_t)vb_blk, u64PhyAddr, *(phy_addr_list + i));
	}

	memset(&stDumpInfo, 0, sizeof(stDumpInfo));
	stDumpInfo.ViPipe = ViPipe;
	stDumpInfo.u8BlkCnt = BlkCnt;
	stDumpInfo.phy_addr_list = phy_addr_list;
	// set rawdump crop info in stDumpInfo
	stDumpInfo.stCropRect.s32X = crop_x;
	stDumpInfo.stCropRect.s32Y = crop_y;
	stDumpInfo.stCropRect.u32Width = crop_w;
	stDumpInfo.stCropRect.u32Height = crop_h;

	s32Ret = CVI_VI_StartSmoothRawDump(&stDumpInfo);
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("start failed\n");
		s32Ret = CVI_FAILURE;
		return s32Ret;
	}

	for (int i = 0; i < TotalFrameCnt; i++) {
		memset(stVideoFrame, 0, sizeof(stVideoFrame));
		s32Ret = CVI_VI_GetSmoothRawDump(ViPipe, stVideoFrame, 1000);
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("[%d] get frame failed\n", i);
			continue;
		}

		VI_UT_PRT("[%d] get roi frame addr(%#"PRIx64") length(%d), number(%d)\n", i,
				stVideoFrame[0].stVFrame.u64PhyAddr[0],
				stVideoFrame[0].stVFrame.u32Length[0],
				stVideoFrame[0].stVFrame.u32TimeRef);

		if (stVideoFrame[1].stVFrame.u64PhyAddr[0] != 0) {
			VI_UT_PRT("[%d] get roi frame addr(%#"PRIx64") length(%d) number(%d)\n", i,
					stVideoFrame[1].stVFrame.u64PhyAddr[0],
					stVideoFrame[1].stVFrame.u32Length[0],
					stVideoFrame[1].stVFrame.u32TimeRef);
		}

		s32Ret = CVI_VI_PutSmoothRawDump(ViPipe, stVideoFrame);
		if (s32Ret != CVI_SUCCESS) {
			VI_UT_PRT("[%d] release frame failed\n", i);
			continue;
		}
	}

	s32Ret = CVI_VI_StopSmoothRawDump(&stDumpInfo);
	if (s32Ret != CVI_SUCCESS) {
		VI_UT_PRT("stop failed\n");
		s32Ret = CVI_FAILURE;
		return s32Ret;
	}

	for (CVI_U32 i = 0; i < cfg.u32BlkCnt; i++) {
		u64PhyAddr = *(phy_addr_list + i);
		vb_blk = CVI_VB_PhysAddr2Handle(u64PhyAddr);
		if (vb_blk != VB_INVALID_HANDLE) {
			CVI_VB_ReleaseBlock(vb_blk);
		}
	}

	if (phy_addr_list != CVI_NULL) {
		free(phy_addr_list);
		phy_addr_list = CVI_NULL;
	}
	s32Ret = CVI_VB_DestroyPool(PoolID);

	return s32Ret;
}

CVI_S32 vi_ut_get_pipe_frame(CVI_U8 dev)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	VIDEO_FRAME_INFO_S stVideoFrame[2];
	VI_DUMP_ATTR_S attr;
	VI_PIPE_ATTR_S pipe_attr;

	struct timeval tv1;
	int frm_num = 1, j = 0;

	memset(stVideoFrame, 0, sizeof(stVideoFrame));

	stVideoFrame[0].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;
	stVideoFrame[1].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;

	// Config pipe_attr.enCompressMode
	CVI_VI_GetPipeAttr(0, &pipe_attr);
	pipe_attr.enCompressMode = vi_ut_ctx.enCompressMode;
	CVI_VI_SetPipeAttr(0, &pipe_attr);

	attr.bEnable = 1;
	attr.u32Depth = 0;
	attr.enDumpType = VI_DUMP_TYPE_RAW;

	CVI_VI_SetPipeDumpAttr(dev, &attr);

	attr.bEnable = 0;
	attr.enDumpType = VI_DUMP_TYPE_IR;

	CVI_VI_GetPipeDumpAttr(dev, &attr);

	VI_UT_PRT("Enable(%d), DumpType(%d):\n", attr.bEnable, attr.enDumpType);

	frm_num = 1;

	s32Ret = CVI_VI_GetPipeFrame(dev, stVideoFrame, 3000);
	if (s32Ret == CVI_SUCCESS) {
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
				VI_UT_PRT("paddr(%#"PRIx64") vaddr(%p)\n",
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

				VI_UT_PRT("dump image %s\n", img_name);

				output = fopen(img_name, "wb");

				fwrite(ptr, image_size, 1, output);
				fclose(output);
				free(ptr);
			}
		}

		CVI_VI_ReleasePipeFrame(dev, stVideoFrame);
	}

	return s32Ret;
}

CVI_S32 vi_ut_get_chn_frame(CVI_U8 chn)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VIDEO_FRAME_INFO_S stVideoFrame;

	memset(&stVideoFrame, 0, sizeof(stVideoFrame));

	if (CVI_VI_GetChnFrame(0, chn, &stVideoFrame, 3000) == 0) {
		CVI_CHAR img_name[128] = {0, };
		struct timeval tv;

		gettimeofday(&tv, NULL);
		snprintf(img_name, sizeof(img_name), "ut_chn%d_%u_%u_%ld_%ld.yuv",
				chn, stVideoFrame.stVFrame.u32Width, stVideoFrame.stVFrame.u32Height,
				tv.tv_sec, tv.tv_usec);

		s32Ret = vi_ut_save_frame2file(img_name, &stVideoFrame);

		if (CVI_VI_ReleaseChnFrame(0, chn, &stVideoFrame) != 0)
			VI_UT_PRT("CVI_VI_ReleaseChnFrame NG\n");

		return s32Ret;
	}

	VI_UT_PRT("CVI_VI_GetChnFrame NG\n");

	return s32Ret;
}

CVI_S32 vi_ut_get_vpss_chn_frame(CVI_U8 chn)
{
	VIDEO_FRAME_INFO_S stVideoFrame = {0};

	if (CVI_VPSS_GetChnFrame(0, chn, &stVideoFrame, 3000) == 0) {
		FILE *output;
		size_t image_size = stVideoFrame.stVFrame.u32Length[0]
				  + stVideoFrame.stVFrame.u32Length[1]
				  + stVideoFrame.stVFrame.u32Length[2];
		CVI_VOID *vir_addr;
		CVI_U32 plane_offset, u32LumaSize, u32ChromaSize;
		CVI_CHAR img_name[128] = {0, };
		struct timeval tv;

		gettimeofday(&tv, NULL);
		snprintf(img_name, sizeof(img_name), "ut_chn%d_%u_%u_%ld_%ld.yuv",
				chn, stVideoFrame.stVFrame.u32Width, stVideoFrame.stVFrame.u32Height,
				tv.tv_sec, tv.tv_usec);

		VI_UT_PRT("name: %s\n", img_name);
		VI_UT_PRT("width: %d, height: %d, total_buf_length: %zu\n",
			   stVideoFrame.stVFrame.u32Width,
			   stVideoFrame.stVFrame.u32Height, image_size);

		output = fopen(img_name, "wb");
		if (output == NULL) {
			CVI_VPSS_ReleaseChnFrame(0, chn, &stVideoFrame);
			VI_UT_PRT("fopen fail\n");
			return CVI_FAILURE;
		}

		u32LumaSize = stVideoFrame.stVFrame.u32Stride[0] * stVideoFrame.stVFrame.u32Height;
		u32ChromaSize = stVideoFrame.stVFrame.u32Stride[1] * stVideoFrame.stVFrame.u32Height / 2;
		vir_addr = CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0], image_size);
		CVI_SYS_IonInvalidateCache(stVideoFrame.stVFrame.u64PhyAddr[0], vir_addr, image_size);
		plane_offset = 0;
		for (int i = 0; i < 3; i++) {
			if (stVideoFrame.stVFrame.u32Length[i] != 0) {
				stVideoFrame.stVFrame.pu8VirAddr[i] = vir_addr + plane_offset;
				plane_offset += stVideoFrame.stVFrame.u32Length[i];
				VI_UT_PRT( "plane(%d): paddr(0x%#"PRIx64") vaddr(%p) stride(%d) length(%d)\n",
					   i, stVideoFrame.stVFrame.u64PhyAddr[i],
					   stVideoFrame.stVFrame.pu8VirAddr[i],
					   stVideoFrame.stVFrame.u32Stride[i],
					   stVideoFrame.stVFrame.u32Length[i]);
				fwrite((void *)stVideoFrame.stVFrame.pu8VirAddr[i],
					(i == 0) ? u32LumaSize : u32ChromaSize, 1, output);
			}
		}
		CVI_SYS_Munmap(vir_addr, image_size);

		if (CVI_VPSS_ReleaseChnFrame(0, chn, &stVideoFrame) != 0)
			VI_UT_PRT("CVI_VPSS_ReleaseChnFrame NG\n");

		fclose(output);
		return CVI_SUCCESS;
	}
	VI_UT_PRT("CVI_VPSS_GetChnFrame NG\n");
	return CVI_FAILURE;
}

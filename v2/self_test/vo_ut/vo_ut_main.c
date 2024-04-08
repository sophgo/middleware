#include "vo_ut.h"

#define VO_DEVNODE	"/dev/soph-vo"

#define COMMON_POOL0_BLK_SIZE (0x600000) // 6M
#define COMMON_POOL1_BLK_SIZE (0x300000) // 3M
#define COMMON_POOL0_BLK_CNT (8)
#define COMMON_POOL1_BLK_CNT (8)
#ifndef UNUSED
#define UNUSED(x) ((x) = (x))
#endif

#define NONE	"\033[m"
#define RED	"\033[0;32;31m"
#define GREEN	"\033[0;32;32m"

typedef struct _VO_UT_FILE {
	SIZE_S stSize;
	PIXEL_FORMAT_E enPixelFormat;
	char filename[30];
} VO_UT_FILE;

int vo_fd = -1;
int intf;
SAMPLE_VO_CONFIG_S stVoConfig;

VO_UT_FILE vo_ut_file1[] = {
	{.stSize.u32Width = 720, .stSize.u32Height = 1280,
	.filename = "res/vo/720x1280.nv21", .enPixelFormat = PIXEL_FORMAT_NV21},//0:sensor image 720x1280
	{.stSize.u32Width = 1280, .stSize.u32Height = 720,
	.filename = "res/vo/1280x720.nv21",  .enPixelFormat = PIXEL_FORMAT_NV21},//1:rocket 1280x720
	{.stSize.u32Width = 1080, .stSize.u32Height = 1920,
	.filename = "res/vo/golden_1080x1920.nv21", .enPixelFormat = PIXEL_FORMAT_NV21},
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.nv21", .enPixelFormat = PIXEL_FORMAT_NV21}
};

VO_UT_FILE vo_ut_file2[] = {
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.rgb", .enPixelFormat = PIXEL_FORMAT_RGB_888},//0:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.bgr", .enPixelFormat = PIXEL_FORMAT_BGR_888},//1:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.rgbm", .enPixelFormat = PIXEL_FORMAT_RGB_888_PLANAR},//2:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.bgrm", .enPixelFormat = PIXEL_FORMAT_BGR_888_PLANAR},//3:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.yuv420", .enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420},//4:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.yuv422", .enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_422},//5:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.y", .enPixelFormat = PIXEL_FORMAT_YUV_400},//6:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.nv12", .enPixelFormat = PIXEL_FORMAT_NV12},//7:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.nv21", .enPixelFormat = PIXEL_FORMAT_NV21},//8:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.nv16", .enPixelFormat = PIXEL_FORMAT_NV16},//9:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.nv61", .enPixelFormat = PIXEL_FORMAT_NV61},//10:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.yuyv", .enPixelFormat = PIXEL_FORMAT_YUYV},//11:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.uyvy", .enPixelFormat = PIXEL_FORMAT_UYVY},//12:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.yvyu", .enPixelFormat = PIXEL_FORMAT_YVYU},//13:
	{.stSize.u32Width = 1920, .stSize.u32Height = 1080,
	.filename = "res/vo/golden_1920x1080.vyuy", .enPixelFormat = PIXEL_FORMAT_VYUY},//14:
};

CVI_S32 VO_PrepareFrame(SIZE_S stSize, PIXEL_FORMAT_E enPixelFormat, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VB_BLK blk;
	VB_CAL_CONFIG_S stVbCalConfig;

	if (pstVideoFrame == CVI_NULL) {
		SAMPLE_PRT("Null pointer!\n");
		return CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(stSize.u32Width, stSize.u32Height, enPixelFormat, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	memset(pstVideoFrame, 0, sizeof(*pstVideoFrame));
	pstVideoFrame->stVFrame.enCompressMode = COMPRESS_MODE_NONE;
	pstVideoFrame->stVFrame.enPixelFormat = enPixelFormat;
	pstVideoFrame->stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
	pstVideoFrame->stVFrame.enColorGamut = COLOR_GAMUT_BT601;
	pstVideoFrame->stVFrame.u32Width = stSize.u32Width;
	pstVideoFrame->stVFrame.u32Height = stSize.u32Height;
	pstVideoFrame->stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
	pstVideoFrame->stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
	pstVideoFrame->stVFrame.u32TimeRef = 0;
	pstVideoFrame->stVFrame.u64PTS = 0;
	pstVideoFrame->stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;

	blk = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCalConfig.u32VBSize);
	if (blk == VB_INVALID_HANDLE) {
		SAMPLE_PRT("Can't acquire vb block\n");
		return CVI_FAILURE;
	}

	pstVideoFrame->u32PoolId = CVI_VB_Handle2PoolId(blk);
	pstVideoFrame->stVFrame.u32Length[0] = stVbCalConfig.u32MainYSize;
	pstVideoFrame->stVFrame.u32Length[1] = stVbCalConfig.u32MainCSize;
	pstVideoFrame->stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
	pstVideoFrame->stVFrame.u64PhyAddr[1] = pstVideoFrame->stVFrame.u64PhyAddr[0]
		+ ALIGN(stVbCalConfig.u32MainYSize, stVbCalConfig.u16AddrAlign);
	if (stVbCalConfig.plane_num == 3) {
		pstVideoFrame->stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
		pstVideoFrame->stVFrame.u32Length[2] = stVbCalConfig.u32MainCSize;
		pstVideoFrame->stVFrame.u64PhyAddr[2] = pstVideoFrame->stVFrame.u64PhyAddr[1]
			+ ALIGN(stVbCalConfig.u32MainCSize, stVbCalConfig.u16AddrAlign);
	}

	return CVI_SUCCESS;
}

CVI_S32 vo_ut_send_frame(VO_UT_FILE *fileptr, VO_DEV VoDev)
{
	CVI_S32 ret = 0;
	FILE *fp;
	SIZE_S stSize;
	struct vo_snd_frm_cfg cfg;
	VO_LAYER VoLayer = VoDev;
	VO_CHN VoChn = 0;

	stSize.u32Width = fileptr->stSize.u32Width;
	stSize.u32Height = fileptr->stSize.u32Height;

	SAMPLE_PRT("File[name, w, h, format]=[%s, %d,%d, %d]\n",
		fileptr->filename, fileptr->stSize.u32Width,
		fileptr->stSize.u32Height, fileptr->enPixelFormat);

	if (VO_PrepareFrame(stSize, fileptr->enPixelFormat, &cfg.stVideoFrame) != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_PrepareFrame failed\n");
		return CVI_FAILURE;
	}

	cfg.stVideoFrame.stVFrame.pu8VirAddr[0]
		= CVI_SYS_Mmap(cfg.stVideoFrame.stVFrame.u64PhyAddr[0], cfg.stVideoFrame.stVFrame.u32Length[0]);
	if (cfg.stVideoFrame.stVFrame.pu8VirAddr[0] == NULL) {
		SAMPLE_PRT("CVI_SYS_Mmap failed\n");
		return CVI_FAILURE;
	}
	cfg.stVideoFrame.stVFrame.pu8VirAddr[1]
		= CVI_SYS_Mmap(cfg.stVideoFrame.stVFrame.u64PhyAddr[1], cfg.stVideoFrame.stVFrame.u32Length[1]);
	if (cfg.stVideoFrame.stVFrame.pu8VirAddr[1] == NULL) {
		SAMPLE_PRT("CVI_SYS_Mmap failed\n");
		CVI_SYS_Munmap(cfg.stVideoFrame.stVFrame.pu8VirAddr[0], cfg.stVideoFrame.stVFrame.u32Length[0]);
		return CVI_FAILURE;
	}

	SAMPLE_PRT("phy addr(%#"PRIx64", %#"PRIx64")\n",
		cfg.stVideoFrame.stVFrame.u64PhyAddr[0], cfg.stVideoFrame.stVFrame.u64PhyAddr[1]);
	SAMPLE_PRT("vir addr(%p, %p)\n", cfg.stVideoFrame.stVFrame.pu8VirAddr[0]
		, cfg.stVideoFrame.stVFrame.pu8VirAddr[1]);

	fp = fopen(fileptr->filename, "r");

	if (fp == NULL) {
		SAMPLE_PRT("open file %s fail\n", fileptr->filename);
		return CVI_FAILURE;
	} else
		SAMPLE_PRT("open file %s success\n", fileptr->filename);

	for (int i = 0; i < 2; i++) {
		SAMPLE_PRT("vir addr(%p, %d)\n", cfg.stVideoFrame.stVFrame.pu8VirAddr[i],
						cfg.stVideoFrame.stVFrame.u32Length[i]);
		fread((void *)cfg.stVideoFrame.stVFrame.pu8VirAddr[i]
			, cfg.stVideoFrame.stVFrame.u32Length[i], 1, fp);
	}
	fclose(fp);
	CVI_SYS_Munmap(cfg.stVideoFrame.stVFrame.pu8VirAddr[0], cfg.stVideoFrame.stVFrame.u32Length[0]);
	CVI_SYS_Munmap(cfg.stVideoFrame.stVFrame.pu8VirAddr[1], cfg.stVideoFrame.stVFrame.u32Length[1]);

	CVI_VO_SendFrame(VoLayer, VoChn, &cfg.stVideoFrame, -1);

	CVI_VB_ReleaseBlock(CVI_VB_PhysAddr2Handle(cfg.stVideoFrame.stVFrame.u64PhyAddr[0]));
	return ret;
}

void _vb_ut_handleSig(int nSignal, siginfo_t *si, void *arg)
{
	CVI_S32 s32Ret;

	UNUSED(nSignal);
	UNUSED(si);
	UNUSED(arg);

	s32Ret = CVI_SYS_Exit();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Exit failed!\n");
		exit(1);
	}

	s32Ret = CVI_VB_Exit();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_Exit failed!\n");
		exit(1);
	}

	exit(1);
}

// TODO:
#ifdef VO_SUSPEND_RESUME_IMPLEMENT
CVI_S32 _vo_ut_suspend_function(void *pvData)
{
	UNUSED(pvData);

	return 0;
}
#endif

CVI_S32 vo_ut_vpss_init_by_fmt(CVI_S32 fmt, SIZE_S in_size, SIZE_S out_size)
{
	PIXEL_FORMAT_E enPixelFormat = fmt;
	PIXEL_FORMAT_E enPixelFormatOut = fmt;

	SIZE_S		   stSize = in_size;
	SIZE_S		   stOutSize = out_size;
	CVI_S32		s32Ret = CVI_SUCCESS;

	/************************************************
	 * step2:  Init VPSS
	 ************************************************/
	VPSS_GRP	   VpssGrp = 0;
	VPSS_GRP_ATTR_S    stVpssGrpAttr;
	VPSS_CHN	   VpssChn = VPSS_CHN0;
	CVI_BOOL	   abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM] = {0};

	// grp0 for left half
	stVpssGrpAttr.stFrameRate.s32SrcFrameRate	 = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate	 = -1;
	stVpssGrpAttr.enPixelFormat			= enPixelFormat;
	stVpssGrpAttr.u32MaxW				 = stSize.u32Width;
	stVpssGrpAttr.u32MaxH				 = stSize.u32Height;

	astVpssChnAttr[VpssChn].u32Width			= stOutSize.u32Width;
	astVpssChnAttr[VpssChn].u32Height			= stOutSize.u32Height;
	astVpssChnAttr[VpssChn].enVideoFormat			= VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn].enPixelFormat			= enPixelFormatOut;
	astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
	astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
	astVpssChnAttr[VpssChn].u32Depth			= 1;
	astVpssChnAttr[VpssChn].bMirror				= CVI_FALSE;
	astVpssChnAttr[VpssChn].bFlip				= CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.enMode		= ASPECT_RATIO_MANUAL;
	astVpssChnAttr[VpssChn].stAspectRatio.bEnableBgColor   = CVI_FALSE;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.s32X = 0;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.s32Y = 0;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.u32Width = stOutSize.u32Width;
	astVpssChnAttr[VpssChn].stAspectRatio.stVideoRect.u32Height = stOutSize.u32Height;
	astVpssChnAttr[VpssChn].stNormalize.bEnable				= CVI_FALSE;

	/*start vpss*/
	abChnEnable[0] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
	}

	return s32Ret;
}

CVI_S32 vo_ut_vpss_deinit(void)
{
	VPSS_GRP VpssGrp = 0;
	CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	CVI_S32 s32Ret = CVI_SUCCESS;

	abChnEnable[0] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Stop(VpssGrp, abChnEnable);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
	}

	return s32Ret;

}

CVI_S32 _vo_ut_resume_function(void *pvData)
{
	UNUSED(pvData);

	return 0;
}

CVI_S32 vo_ut_sys_init(void)
{
	CVI_S32 s32Ret;
	VB_CONFIG_S	stVbConf;
	struct sigaction sa = {};

	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = _vb_ut_handleSig;
	sa.sa_flags = SA_SIGINFO|SA_RESETHAND;	// Reset signal handler to system default after signal triggered
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	CVI_VB_Exit();

	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt = 2;
	stVbConf.astCommPool[0].u32BlkSize = COMMON_POOL0_BLK_SIZE;
	stVbConf.astCommPool[0].u32BlkCnt = COMMON_POOL0_BLK_CNT;
	SAMPLE_PRT("common pool[0] BlkSize(%d) BlkCnt(%d)\n",
		COMMON_POOL0_BLK_SIZE, COMMON_POOL0_BLK_CNT);
	stVbConf.astCommPool[0].enRemapMode	= VB_REMAP_MODE_CACHED;
	stVbConf.astCommPool[1].u32BlkSize = COMMON_POOL1_BLK_SIZE;
	stVbConf.astCommPool[1].u32BlkCnt = COMMON_POOL1_BLK_CNT;
	SAMPLE_PRT("common pool[1] BlkSize(%d) BlkCnt(%d)\n",
		COMMON_POOL1_BLK_SIZE, COMMON_POOL1_BLK_CNT);
	stVbConf.astCommPool[1].enRemapMode	= VB_REMAP_MODE_CACHED;

	s32Ret = CVI_VB_SetConfig(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_SetConf failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_Init failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_SYS_Init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Init failed!\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 vo_ut_sys_deinit(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = CVI_SYS_Exit();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_SYS_Exit failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_VB_Exit();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VB_Exit failed!\n");
		return s32Ret;
	}

	return s32Ret;
}

static CVI_S32 _vo_open_device(void)
{
	struct stat st;

	if (vo_fd != -1)
		return CVI_SUCCESS;

	vo_fd = open(VO_DEVNODE, O_RDWR, 0);
	if (-1 == vo_fd) {
		fprintf(stderr, "Cannot open '%s': (%d), %s\n", VO_DEVNODE, errno, strerror(errno));
		printf("Cannot open '%s'\n", VO_DEVNODE);
		return -1;
	}

	if (-1 == fstat(vo_fd, &st)) {
		close(vo_fd);
		fprintf(stderr, "Cannot identify '%s': %d, %s\n", VO_DEVNODE, errno, strerror(errno));
		printf("Cannot identify '%s'\n", VO_DEVNODE);
		return -1;
	}

	if (!S_ISCHR(st.st_mode)) {
		close(vo_fd);
		fprintf(stderr, "%s is no device\n", VO_DEVNODE);
		printf("'%s' is no device\n", VO_DEVNODE);
		return -ENODEV;
	}

	return CVI_SUCCESS;
}

static CVI_S32 _vo_close_device(void)
{
	if (vo_fd == -1)
		return CVI_SUCCESS;

	if (-1 == close(vo_fd)) {
		fprintf(stderr, "%s: fd(%d) failure\n", __func__, vo_fd);
		return -1;
	}

	vo_fd = -1;

	return CVI_SUCCESS;
}

CVI_S32 vo_ut_vo_init_by_fmt(CVI_S32 fmt, VO_DEV VoDev)
{
	RECT_S stDefDispRect;
	SIZE_S stDefImageSize;
	CVI_S32 s32Ret = CVI_SUCCESS;
	VO_LAYER VoLayer = VoDev;
	VO_CHN VoChn = 0;

	stDefDispRect.s32X = 0;
	stDefDispRect.s32Y = 0;
	stDefDispRect.u32Width = (intf == 0) ? 720 : 1080;
	stDefDispRect.u32Height = (intf == 0) ? 1280 : 1920;
	stDefImageSize.u32Width = stDefDispRect.u32Width;
	stDefImageSize.u32Height = stDefDispRect.u32Height;
	s32Ret = SAMPLE_COMM_VO_GetDefConfig(&stVoConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VO_GetDefConfig failed with %#x\n", s32Ret);
		return s32Ret;
	}

	stVoConfig.VoDev	 = VoDev;
	stVoConfig.stVoPubAttr.enIntfType  = VO_INTF_MIPI;
	stVoConfig.stVoPubAttr.enIntfSync  = (intf == 0) ? VO_OUTPUT_720x1280_60 : VO_OUTPUT_1080x1920_60;
	stVoConfig.stDispRect	 = stDefDispRect;
	stVoConfig.stImageSize	 = stDefImageSize;
	stVoConfig.enPixFormat	 = fmt;
	SAMPLE_PRT("SAMPLE_COMM_VO_StartVO fmt %x\n", fmt);
	stVoConfig.enVoMode	 = VO_MODE_1MUX;
	s32Ret = SAMPLE_COMM_VO_StartVO(&stVoConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VO_StartVO failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = CVI_VO_SetChnRotation(VoLayer, VoChn, ROTATION_0);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("CVI_VO_SetChnRotation is fail\n");
		return s32Ret;
	}

	return s32Ret;
}

CVI_S32 vo_ut_vo_deinit(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	s32Ret = SAMPLE_COMM_VO_StopVO(&stVoConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VO_StopVO failed with %#x\n", s32Ret);
	}
	return s32Ret;
}

static CVI_S32 _vo_ut_handle_op(CVI_S32 op, VO_DEV VoDev)
{
	CVI_S32 s32Ret = CVI_SUCCESS, s32DeinitRet = CVI_SUCCESS;
	VO_LAYER VoLayer = VoDev;
	VO_CHN VoChn = 0;

	s32Ret = vo_ut_vo_init_by_fmt(vo_ut_file1[0 + intf * 2].enPixelFormat, VoDev);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vo_ut_vo_init_by_fmt failed\n");
		return -1;
	}

	switch (op) {
	case 1: {
		s32Ret = vo_ut_send_frame(&vo_ut_file1[0 + intf * 2], VoDev);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vo_ut_send_frame failed\n");
			break;
		}

		break;
	}

	case 2: {
		s32Ret = CVI_VO_SetChnRotation(VoLayer, VoChn, ROTATION_90);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SetChnRotation is fail\n");
			break;
		}

		s32Ret = vo_ut_send_frame(&vo_ut_file1[1 + intf * 2], VoDev);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vo_ut_send_frame failed\n");
			break;
		}
		break;
	}

	case 3: {
		VO_PUB_ATTR_S stPubAttr;
		VO_VIDEO_LAYER_ATTR_S stVideoAttr;
		CVI_U32 u32BufLen;
		CVI_U32 u32Toleration;
		VO_CSC_S stVideoCSC;
		VO_CHN_ATTR_S stChnAttr;
		ROTATION_E enRotation;
		VO_CHN_ZOOM_ATTR_S stChnZoomAttr;
		VO_CHN_PARAM_S stChnParam;
		VO_CHN_BORDER_ATTR_S stChnBorder;
		CVI_U32 u32Threshold;
		CVI_S32 s32ChnFrmRate;
		CVI_U64 u64ChnPTS;
		VO_QUERY_STATUS_S stStatus;
		VO_CHN_MIRROR_TYPE enChnMirror;

		s32Ret = vo_ut_send_frame(&vo_ut_file1[0 + intf * 2], VoDev);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vo_ut_send_frame is fail\n");
			break;
		}

		s32Ret = CVI_VO_GetPubAttr(VoDev, &stPubAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetPubAttr is fail\n");
			break;
		}

		s32Ret = CVI_VO_GetVideoLayerAttr(VoLayer, &stVideoAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetVideoLayerAttr is fail\n");
			break;
		}

		s32Ret = CVI_VO_GetPlayToleration(VoLayer, &u32Toleration);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetPlayToleration is fail\n");
			break;
		}

		s32Ret = CVI_VO_GetVideoLayerCSC(VoLayer, &stVideoCSC);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetVideoLayerCSC is fail\n");
			break;
		}

		s32Ret = CVI_VO_GetDisplayBufLen(VoLayer, &u32BufLen);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetDisplayBufLen is fail\n");
			break;
		}

		s32Ret = CVI_VO_GetChnRotation(VoLayer, VoChn, &enRotation);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetChnRotation is fail\n");
			break;
		}

		s32Ret = CVI_VO_GetChnAttr(VoLayer, VoChn, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetChnAttr is fail\n");
			break;
		}

		s32Ret = CVI_VO_GetChnZoomInWindow(VoLayer, VoChn, &stChnZoomAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetChnParam is fail\n");
			break;
		}

		s32Ret = CVI_VO_GetChnParam(VoLayer, VoChn, &stChnParam);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetChnParam is fail\n");
			break;
		}

		s32Ret = CVI_VO_GetChnBorder(VoLayer, VoChn, &stChnBorder);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetChnParam is fail\n");
			break;
		}

		s32Ret = CVI_VO_GetChnRecvThreshold(VoLayer, VoChn, &u32Threshold);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetChnRecvThreshold is fail\n");
			break;
		}

		s32Ret = CVI_VO_GetChnFrameRate(VoLayer, VoChn, &s32ChnFrmRate);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetChnRecvThreshold is fail\n");
			break;
		}

		s32Ret = CVI_VO_GetChnPTS(VoLayer, VoChn, &u64ChnPTS);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetChnPTS is fail\n");
			break;
		}

		s32Ret = CVI_VO_QueryChnStatus(VoLayer, VoChn, &stStatus);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetChnPTS is fail\n");
			break;
		}

		s32Ret = CVI_VO_GetChnMirror(VoLayer, VoChn, &enChnMirror);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetChnPTS is fail\n");
			break;
		}

		SAMPLE_PRT("-- VO Dev info --\n");
		SAMPLE_PRT("Intf Type(%d)  Sync Type(%d)\n"
			, stPubAttr.enIntfType, stPubAttr.enIntfSync);
		if (stPubAttr.enIntfSync == VO_OUTPUT_USER) {
			SAMPLE_PRT("Hor front-porch(%d) back-porch(%d) active(%d) sync(%d)\n"
				, stPubAttr.stSyncInfo.u16Hfb, stPubAttr.stSyncInfo.u16Hbb
				, stPubAttr.stSyncInfo.u16Hact, stPubAttr.stSyncInfo.u16Hpw);
			SAMPLE_PRT("Ver front-porch(%d) back-porch(%d) active(%d) sync(%d)\n"
				, stPubAttr.stSyncInfo.u16Vfb, stPubAttr.stSyncInfo.u16Vbb
				, stPubAttr.stSyncInfo.u16Vact, stPubAttr.stSyncInfo.u16Vpw);
		}

		SAMPLE_PRT("-- VO VideoLayer info --\n");
		SAMPLE_PRT("width(%d)  height(%d)\n"
			, stVideoAttr.stImageSize.u32Width, stVideoAttr.stImageSize.u32Height);
		SAMPLE_PRT("PixFormat(%d)  DispFrmRate(%d)  u32BufLen(%d) u32Depth(%d)\n"
			, stVideoAttr.enPixFormat, stVideoAttr.u32DispFrmRt, u32BufLen, stVideoAttr.u32Depth);
		SAMPLE_PRT("Toleration(%d)  CscMatrix(%d)\n"
			, u32Toleration, stVideoCSC.enCscMatrix);

		SAMPLE_PRT("-- VO Chn info --\n");
		SAMPLE_PRT("Chn Rotation(%d) Mirror(%d)\n", enRotation, enChnMirror);
		SAMPLE_PRT("Chn Attr(x,y,w,h,priority,threshold,depth)=(%d, %d, %d, %d, %d, %d, %d)\n",
			stChnAttr.stRect.s32X, stChnAttr.stRect.s32Y,
			stChnAttr.stRect.u32Width, stChnAttr.stRect.u32Height,
			stChnAttr.u32Priority, u32Threshold, stChnAttr.u32Depth);
		SAMPLE_PRT("Chn Zoom(type,x,y,w,h,xratio,yration,wratio,hratio)="
			"(%d, %d, %d, %d, %d, %d, %d, %d, %d)\n",
			stChnZoomAttr.enZoomType, stChnZoomAttr.stRect.s32X,
			stChnZoomAttr.stRect.s32Y, stChnZoomAttr.stRect.u32Width,
			stChnZoomAttr.stRect.u32Height, stChnZoomAttr.stZoomRatio.u32Xratio,
			stChnZoomAttr.stZoomRatio.u32Yratio, stChnZoomAttr.stZoomRatio.u32WidthRatio,
			stChnZoomAttr.stZoomRatio.u32HeightRatio);
		SAMPLE_PRT("Chn AspectRatio(mode,bgcoloren,bgcolor,x,y,w,h)=(%d, %d, 0x%x, %d, %d, %d, %d)\n",
			stChnParam.stAspectRatio.enMode, stChnParam.stAspectRatio.bEnableBgColor,
			stChnParam.stAspectRatio.u32BgColor, stChnParam.stAspectRatio.stVideoRect.s32X,
			stChnParam.stAspectRatio.stVideoRect.s32Y, stChnParam.stAspectRatio.stVideoRect.u32Width,
			stChnParam.stAspectRatio.stVideoRect.u32Height);
		SAMPLE_PRT("Chn Border(enable,top,bottom,left,right,w,h)=(%d, %d, %d, %d, %d, 0x%x)\n",
			stChnBorder.enable, stChnBorder.stBorder.u32TopWidth,
			stChnBorder.stBorder.u32BottomWidth, stChnBorder.stBorder.u32LeftWidth,
			stChnBorder.stBorder.u32RightWidth, stChnBorder.stBorder.u32Color);
		SAMPLE_PRT("Chn ChnFrmRate(%d) PlayPts(%lu) BufferUsed(%d)\n",
			s32ChnFrmRate, u64ChnPTS, stStatus.u32ChnBufUsed);
		break;
	}

	case 4: {
		int enable;

		s32Ret = CVI_VO_DisableChn(VoLayer, VoChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_DisableChn is fail\n");
			break;
		}

		s32Ret = CVI_VO_DisableVideoLayer(VoLayer);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_DisableVideoLayer is fail\n");
			break;
		}

		s32Ret = CVI_VO_Disable(VoDev);
		enable = CVI_VO_IsEnabled(VoDev);
		if (enable) {
			SAMPLE_PRT("VO disable failed!\n");
			break;
		}

		s32Ret = CVI_VO_Enable(VoDev);
		enable = CVI_VO_IsEnabled(VoDev);
		if (!enable) {
			SAMPLE_PRT("VO enable failed!\n");
			break;
		}

		s32Ret = CVI_VO_EnableVideoLayer(VoLayer);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_DisableVideoLayer is fail\n");
			break;
		}

		s32Ret = CVI_VO_EnableChn(VoLayer, VoChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_EnableChn is fail\n");
			break;
		}

		break;
	}

	case 5: {
		s32Ret = vo_ut_send_frame(&vo_ut_file1[0 + intf * 2], VoDev);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vo_ut_send_frame failed\n");
			break;
		}

		s32Ret = CVI_VO_HideChn(VoLayer, VoChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vo hide chn failed\n");
			break;
		}
		usleep(30 * 1000);

		s32Ret = CVI_VO_ShowChn(VoLayer, VoChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vo show chn failed\n");
			break;
		}
		usleep(30 * 1000);

		s32Ret = CVI_VO_PauseChn(VoLayer, VoChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vo pause chn failed\n");
			break;
		}
		usleep(30 * 1000);

		s32Ret = CVI_VO_RefreshChn(VoLayer, VoChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vo refresh chn failed\n");
			break;
		}
		usleep(30 * 1000);

		s32Ret = CVI_VO_ResumeChn(VoLayer, VoChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vo resume chn failed\n");
			break;
		}
		usleep(30 * 1000);

		s32Ret = CVI_VO_StepChn(VoLayer, VoChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vo step chn failed\n");
			break;
		}
		usleep(30 * 1000);

		s32Ret = CVI_VO_ResumeChn(VoLayer, VoChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vo resume chn failed\n");
			break;
		}
		usleep(30 * 1000);

		s32Ret = CVI_VO_ClearChnBuf(VoLayer, VoChn, true);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_ClearChnBuf failed with %#x!\n", s32Ret);
			break;
		}
		usleep(30 * 1000);
		break;
	}

	case 6: {
		VO_BIN_INFO_S Get_vo_bin_info;
		int i = 0;
		static VO_BIN_INFO_S vo_bin_info = {
			.gamma_info = {
				.enable = CVI_FALSE,
				.osd_apply = CVI_FALSE,
				.value = {
					0,	 3,   7,   11,	15,  19,  23,  27,
					31,  35,  39,  43,	47,  51,  55,  59,
					63,  67,  71,  75,	79,  83,  87,  91,
					95,  99,  103, 107, 111, 115, 119, 123,
					127, 131, 135, 139, 143, 147, 151, 155,
					159, 163, 167, 171, 175, 179, 183, 187,
					191, 195, 199, 203, 207, 211, 215, 219,
					223, 227, 231, 235, 239, 243, 247, 251,
					255
				}
			},
			.guard_magic = 0x12345678
		};

		vo_bin_info.gamma_info.s32VoDev = VoDev;
		Get_vo_bin_info.gamma_info.s32VoDev = VoDev;

		s32Ret = CVI_VO_SetGammaInfo(&vo_bin_info.gamma_info);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SetGammaInfo failed with %#x!\n", s32Ret);
			break;
		}

		s32Ret = CVI_VO_GetGammaInfo(&Get_vo_bin_info.gamma_info);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetGammaInfo failed with %#x!\n", s32Ret);
			break;
		}

		for (i = 0; i < VO_GAMMA_NODENUM; ++i) {
			if (Get_vo_bin_info.gamma_info.value[i] != vo_bin_info.gamma_info.value[i]) {
				s32Ret = CVI_FAILURE;
				SAMPLE_PRT("Gamma info compare fail!\n");
				break;
			}
		}

		if ((Get_vo_bin_info.gamma_info.enable !=  vo_bin_info.gamma_info.enable) &&
			(Get_vo_bin_info.gamma_info.osd_apply !=  vo_bin_info.gamma_info.osd_apply) &&
			(Get_vo_bin_info.gamma_info.s32VoDev !=  vo_bin_info.gamma_info.s32VoDev)) {
			s32Ret = CVI_FAILURE;
			SAMPLE_PRT("Gamma info compare fail!\n");
		}

		break;
	}

	case 7: {
		PROC_AMP_E type;
		PROC_AMP_CTRL_S ctrl;
		CVI_S32 cur, value;

		value = 128;
		for (type = PROC_AMP_BRIGHTNESS; type < PROC_AMP_MAX; ++type) {
			s32Ret = CVI_VO_SetLayerProcAmp(VoLayer, type, value);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("CVI_VO_SetLayerProcAmp is fail\n");
				break;
			}

			s32Ret = CVI_VO_GetLayerProcAmpCtrl(VoLayer, type, &ctrl);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("CVI_VO_GetLayerProcAmpCtrl is fail\n");
				break;
			}
			s32Ret = CVI_VO_GetLayerProcAmp(VoLayer, type, &cur);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("CVI_VO_GetLayerProcAmp is fail\n");
				break;
			}
			SAMPLE_PRT("min(%d) max(%d) step(%d) default(%d) current(%d) value(%d)\n"
				, ctrl.minimum, ctrl.maximum, ctrl.step, ctrl.default_value, cur, value);
			value++;
		}

		break;
	}

	case 8: {
#ifdef VO_SUSPEND_RESUME_IMPLEMENT
		VO_PM_OPS_S vo_ops = {
			.pfnPanelSuspend = _vo_ut_suspend_function,
			.pfnPanelResume = _vo_ut_resume_function,
		};

		s32Ret = CVI_VO_RegPmCallBack(VoDev, &vo_ops, NULL);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_RegPmCallBack failed with %#x!\n", s32Ret);
			break;
		}

		s32Ret = CVI_VO_Suspend();
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vo suspend failed\n");
			break;
		}
		usleep(30 * 1000);

		s32Ret = CVI_VO_Resume();
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vo resume failed\n");
			break;
		}
#endif
		break;
	}

	case 9: {
		VIDEO_FRAME_INFO_S stVideoFrame;
		CVI_CHAR file_name[64] = "";
		VO_UT_FILE ut_file;
		VO_VIDEO_LAYER_ATTR_S stLayerAttr;
		VO_CHN_ATTR_S stChnAttr;
		CVI_U32 u32DisBufLen;

		s32Ret = CVI_VO_GetChnAttr(VoLayer, VoChn, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetChnAttr\n");
			break;
		}

		s32Ret = CVI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetVideoLayerAttr\n");
			break;
		}

		s32Ret = CVI_VO_DisableChn(VoLayer, VoChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_DisableChn is fail\n");
			break;
		}

		s32Ret = CVI_VO_DisableVideoLayer(VoLayer);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_DisableVideoLayer is fail\n");
			break;
		}

		stLayerAttr.u32Depth = 1;

		s32Ret = CVI_VO_SetVideoLayerAttr(VoLayer, &stLayerAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SetVideoLayerAttr\n");
			break;
		}

		s32Ret = CVI_VO_SetChnAttr(VoLayer, VoChn, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SetChnAttr\n");
			break;
		}

		s32Ret = CVI_VO_EnableVideoLayer(VoLayer);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_EnableVideoLayer is fail\n");
			break;
		}

		s32Ret = CVI_VO_EnableChn(VoLayer, VoChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_EnableChn is fail\n");
			break;
		}

		memcpy(&ut_file, &vo_ut_file1[0 + intf * 2], sizeof(VO_UT_FILE));

		s32Ret = CVI_VO_GetDisplayBufLen(VoLayer, &u32DisBufLen);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetDisplayBufLen failed\n");
			break;
		}

		for (CVI_U32 i = 0; i < u32DisBufLen + 1; i++) {
			s32Ret = vo_ut_send_frame(&ut_file, VoDev);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("vo_ut_send_frame failed\n");
				break;
			}
			usleep(100 * 1000);
		}

		s32Ret = CVI_VO_GetScreenFrame(VoLayer, &stVideoFrame, 1000);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetScreenFrame failed\n");
			break;
		}
		snprintf(file_name, sizeof(file_name), "vo_ut_get_screen_frame_nv21_%dx%d.bin",
				ut_file.stSize.u32Width, ut_file.stSize.u32Height);
		s32Ret = SAMPLE_COMM_FRAME_SaveToFile(file_name, &stVideoFrame);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_FRAME_SaveToFile failed with s32Ret: 0x%x !\n", s32Ret);
			break;
		}

		s32Ret = CVI_VO_ReleaseScreenFrame(VoLayer, &stVideoFrame);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_ReleaseScreenFrame failed\n");
			break;
		}

		break;
	}

	case 10: {
		VIDEO_FRAME_INFO_S stVideoFrame;
		CVI_CHAR file_name[64] = "";
		VO_UT_FILE ut_file;
		VO_CHN_ATTR_S stChnAttr;
		CVI_U32 u32Threshold;

		s32Ret = CVI_VO_GetChnAttr(VoLayer, VoChn, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetChnAttr\n");
			break;
		}

		s32Ret = CVI_VO_DisableChn(VoLayer, VoChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_DisableChn is fail\n");
			break;
		}

		stChnAttr.u32Depth = 1;

		s32Ret = CVI_VO_SetChnAttr(VoLayer, VoChn, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SetChnAttr\n");
			break;
		}

		s32Ret = CVI_VO_EnableChn(VoLayer, VoChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_EnableChn is fail\n");
			break;
		}

		memcpy(&ut_file, &vo_ut_file1[0 + intf * 2], sizeof(VO_UT_FILE));

		s32Ret = CVI_VO_GetChnRecvThreshold(VoLayer, VoChn, &u32Threshold);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetChnRecvThreshold is fail\n");
			break;
		}

		for (CVI_U32 i = 0; i < u32Threshold + 2; i++) {
			s32Ret = vo_ut_send_frame(&ut_file, VoDev);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("vo_ut_send_frame failed\n");
				break;
			}
			usleep(100 * 1000);
		}

		s32Ret = CVI_VO_GetChnFrame(VoLayer, VoChn, &stVideoFrame, 1000);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetChnFrame failed\n");
			break;
		}
		snprintf(file_name, sizeof(file_name), "vo_ut_get_chn_frame_nv21_%dx%d.bin",
				ut_file.stSize.u32Width, ut_file.stSize.u32Height);
		s32Ret = SAMPLE_COMM_FRAME_SaveToFile(file_name, &stVideoFrame);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_FRAME_SaveToFile failed with s32Ret: 0x%x !\n", s32Ret);
			break;
		}

		s32Ret = CVI_VO_ReleaseChnFrame(VoLayer, VoChn, &stVideoFrame);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_ReleaseChnFrame failed\n");
			break;
		}

		break;
	}

	case 11: {
		VIDEO_FRAME_INFO_S stVideoFrame;
		CVI_CHAR file_name[64] = "";
		VO_UT_FILE ut_file;
		VO_WBC VoWbc = 0;
		VO_WBC_SRC_S stWbcSrc;
		VO_WBC_ATTR_S stWbcAttr;
		CVI_U32 u32Depth;
		CVI_U32 u32DisBufLen;
		VO_VIDEO_LAYER_ATTR_S stLayerAttr;

		stWbcSrc.enSrcType = VO_WBC_SRC_DEV;
		stWbcSrc.u32SrcId = VoDev;
		s32Ret = CVI_VO_SetWbcSrc(VoWbc, &stWbcSrc);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SetWbcSrc failed\n");
			break;
		}
		s32Ret = CVI_VO_GetWbcSrc(VoWbc, &stWbcSrc);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetWbcSrc failed\n");
			break;
		}

		s32Ret = CVI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetVideoLayerAttr\n");
			break;
		}

		stWbcAttr.stTargetSize = stLayerAttr.stImageSize;
		stWbcAttr.enPixFormat = PIXEL_FORMAT_NV21;
		stWbcAttr.u32FrameRate = 60;
		stWbcAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
		stWbcAttr.enCompressMode = COMPRESS_MODE_NONE;
		s32Ret = CVI_VO_SetWbcAttr(VoWbc, &stWbcAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SetWbcAttr failed\n");
			break;
		}
		s32Ret = CVI_VO_GetWbcAttr(VoWbc, &stWbcAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetWbcAttr failed\n");
			break;
		}

		s32Ret = CVI_VO_SetWbcDepth(VoWbc, 1);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SetWbcAttr failed\n");
			break;
		}
		s32Ret = CVI_VO_GetWbcDepth(VoWbc, &u32Depth);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetWbcDepth failed\n");
			break;
		}

		SAMPLE_PRT("Wbc(srctype,srcid)=(%d,%d)\n",
			stWbcSrc.enSrcType, stWbcSrc.u32SrcId);
		SAMPLE_PRT("TargetSize %d %d\n", stWbcAttr.stTargetSize.u32Width,
			stWbcAttr.stTargetSize.u32Height);
		SAMPLE_PRT("PixFormat %d\n", stWbcAttr.enPixFormat);
		SAMPLE_PRT("FrameRate %d\n", stWbcAttr.u32FrameRate);
		SAMPLE_PRT("DynamicRange %d\n", stWbcAttr.enDynamicRange);
		SAMPLE_PRT("CompressMode %d\n", stWbcAttr.enCompressMode);
		SAMPLE_PRT("Depth %d\n", u32Depth);

		s32Ret = CVI_VO_EnableWbc(VoWbc);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_EnableWbc failed\n");
			break;
		}

		memcpy(&ut_file, &vo_ut_file1[0 + intf * 2], sizeof(VO_UT_FILE));

		s32Ret = CVI_VO_GetDisplayBufLen(VoLayer, &u32DisBufLen);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetDisplayBufLen failed\n");
			break;
		}

		for (CVI_U32 i = 0; i < u32DisBufLen + 3; i++) {
			s32Ret = vo_ut_send_frame(&ut_file, VoDev);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("vo_ut_send_frame failed\n");
				break;
			}
			usleep(100 * 1000);
		}

		s32Ret = CVI_VO_GetWbcFrame(VoWbc, &stVideoFrame, 1000);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetWbcFrame failed\n");
			break;
		}
		snprintf(file_name, sizeof(file_name), "vo_ut_get_wbc_dev_frame_nv21_%dx%d.bin",
				ut_file.stSize.u32Width, ut_file.stSize.u32Height);
		s32Ret = SAMPLE_COMM_FRAME_SaveToFile(file_name, &stVideoFrame);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_FRAME_SaveToFile failed with s32Ret: 0x%x !\n", s32Ret);
			break;
		}

		s32Ret = CVI_VO_ReleaseWbcFrame(VoWbc, &stVideoFrame);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_ReleaseWbcFrame failed\n");
			break;
		}

		s32Ret = CVI_VO_DisableWbc(VoWbc);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_DisableWbc failed\n");
			break;
		}

		break;
	}

	case 12: {
		VIDEO_FRAME_INFO_S stVideoFrame;
		CVI_CHAR file_name[64] = "";
		VO_UT_FILE ut_file;
		VO_WBC VoWbc = 0;
		VO_WBC_SRC_S stWbcSrc;
		VO_WBC_ATTR_S stWbcAttr;
		CVI_U32 u32Depth;
		CVI_U32 u32DisBufLen;
		VO_VIDEO_LAYER_ATTR_S stLayerAttr;
		VO_CHN_ATTR_S stChnAttr;

		s32Ret = CVI_VO_GetChnAttr(VoLayer, VoChn, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetChnAttr\n");
			break;
		}

		s32Ret = CVI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetVideoLayerAttr\n");
			break;
		}

		s32Ret = CVI_VO_DisableChn(VoLayer, VoChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_DisableChn is fail\n");
			break;
		}

		s32Ret = CVI_VO_DisableVideoLayer(VoLayer);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_DisableVideoLayer is fail\n");
			break;
		}

		stLayerAttr.u32Depth = 1;

		s32Ret = CVI_VO_SetVideoLayerAttr(VoLayer, &stLayerAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SetVideoLayerAttr\n");
			break;
		}

		s32Ret = CVI_VO_SetChnAttr(VoLayer, VoChn, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SetChnAttr\n");
			break;
		}

		s32Ret = CVI_VO_EnableVideoLayer(VoLayer);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_EnableVideoLayer is fail\n");
			break;
		}

		s32Ret = CVI_VO_EnableChn(VoLayer, VoChn);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_EnableChn is fail\n");
			break;
		}

		stWbcSrc.enSrcType = VO_WBC_SRC_VIDEO;
		stWbcSrc.u32SrcId = VoDev;
		s32Ret = CVI_VO_SetWbcSrc(VoWbc, &stWbcSrc);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SetWbcSrc failed\n");
			break;
		}
		s32Ret = CVI_VO_GetWbcSrc(VoWbc, &stWbcSrc);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetWbcSrc failed\n");
			break;
		}

		stWbcAttr.stTargetSize = stLayerAttr.stImageSize;
		stWbcAttr.enPixFormat = PIXEL_FORMAT_NV21;
		stWbcAttr.u32FrameRate = 60;
		stWbcAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
		stWbcAttr.enCompressMode = COMPRESS_MODE_NONE;
		s32Ret = CVI_VO_SetWbcAttr(VoWbc, &stWbcAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SetWbcAttr failed\n");
			break;
		}
		s32Ret = CVI_VO_GetWbcAttr(VoWbc, &stWbcAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetWbcAttr failed\n");
			break;
		}

		s32Ret = CVI_VO_GetWbcDepth(VoWbc, &u32Depth);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetWbcDepth failed\n");
			break;
		}

		SAMPLE_PRT("Wbc(srctype,srcid)=(%d,%d)\n",
			stWbcSrc.enSrcType, stWbcSrc.u32SrcId);
		SAMPLE_PRT("TargetSize %d %d\n", stWbcAttr.stTargetSize.u32Width,
			stWbcAttr.stTargetSize.u32Height);
		SAMPLE_PRT("PixFormat %d\n", stWbcAttr.enPixFormat);
		SAMPLE_PRT("FrameRate %d\n", stWbcAttr.u32FrameRate);
		SAMPLE_PRT("DynamicRange %d\n", stWbcAttr.enDynamicRange);
		SAMPLE_PRT("CompressMode %d\n", stWbcAttr.enCompressMode);
		SAMPLE_PRT("Depth %d\n", u32Depth);

		s32Ret = CVI_VO_EnableWbc(VoWbc);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_EnableWbc failed\n");
			break;
		}

		memcpy(&ut_file, &vo_ut_file1[0 + intf * 2], sizeof(VO_UT_FILE));

		s32Ret = CVI_VO_GetDisplayBufLen(VoLayer, &u32DisBufLen);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetDisplayBufLen failed\n");
			break;
		}

		for (CVI_U32 i = 0; i < u32DisBufLen + 1; i++) {
			s32Ret = vo_ut_send_frame(&ut_file, VoDev);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("vo_ut_send_frame failed\n");
				break;
			}
			usleep(100 * 1000);
		}

		s32Ret = CVI_VO_GetWbcFrame(VoWbc, &stVideoFrame, 1000);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetWbcFrame failed\n");
			break;
		}
		snprintf(file_name, sizeof(file_name), "vo_ut_get_wbc_video_frame_nv21_%dx%d.bin",
				ut_file.stSize.u32Width, ut_file.stSize.u32Height);
		s32Ret = SAMPLE_COMM_FRAME_SaveToFile(file_name, &stVideoFrame);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("SAMPLE_COMM_FRAME_SaveToFile failed with s32Ret: 0x%x !\n", s32Ret);
			break;
		}

		s32Ret = CVI_VO_ReleaseWbcFrame(VoWbc, &stVideoFrame);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_ReleaseWbcFrame failed\n");
			break;
		}

		s32Ret = CVI_VO_DisableWbc(VoWbc);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_DisableWbc failed\n");
			break;
		}

		break;
	}

	case 13: {
		VO_CHN_ZOOM_ATTR_S stChnZoomAttr;

		s32Ret = vo_ut_send_frame(&vo_ut_file1[0 + intf * 2], VoDev);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vo_ut_send_frame failed\n");
			break;
		}

		stChnZoomAttr.enZoomType = VO_CHN_ZOOM_IN_RECT;
		stChnZoomAttr.stRect.s32X = 0;
		stChnZoomAttr.stRect.s32Y = 0;
		stChnZoomAttr.stRect.u32Width = 400;
		stChnZoomAttr.stRect.u32Height = 400;
		s32Ret = CVI_VO_SetChnZoomInWindow(VoLayer, VoChn, &stChnZoomAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SetChnZoomInWindow failed\n");
			break;
		}

		s32Ret = CVI_VO_GetChnZoomInWindow(VoLayer, VoChn, &stChnZoomAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetChnZoomInWindow failed\n");
			break;
		}

		SAMPLE_PRT("ZoomType %d\n", stChnZoomAttr.enZoomType);
		SAMPLE_PRT("rect(%d %d %d %d)\n", stChnZoomAttr.stRect.s32X,
			stChnZoomAttr.stRect.s32Y, stChnZoomAttr.stRect.u32Width,
			stChnZoomAttr.stRect.u32Height);

		stChnZoomAttr.enZoomType = VO_CHN_ZOOM_IN_RATIO;
		stChnZoomAttr.stZoomRatio.u32Xratio = 200;
		stChnZoomAttr.stZoomRatio.u32Yratio = 200;
		stChnZoomAttr.stZoomRatio.u32WidthRatio = 400;
		stChnZoomAttr.stZoomRatio.u32HeightRatio = 400;
		s32Ret = CVI_VO_SetChnZoomInWindow(VoLayer, VoChn, &stChnZoomAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SetChnZoomInWindow failed\n");
			break;
		}

		s32Ret = CVI_VO_GetChnZoomInWindow(VoLayer, VoChn, &stChnZoomAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetChnZoomInWindow failed\n");
			break;
		}

		SAMPLE_PRT("ZoomType %d\n", stChnZoomAttr.enZoomType);
		SAMPLE_PRT("ratio(%d %d %d %d)\n", stChnZoomAttr.stZoomRatio.u32Xratio,
			stChnZoomAttr.stZoomRatio.u32Yratio, stChnZoomAttr.stZoomRatio.u32WidthRatio,
			stChnZoomAttr.stZoomRatio.u32HeightRatio);

		stChnZoomAttr.enZoomType = VO_CHN_ZOOM_IN_RECT;
		stChnZoomAttr.stZoomRatio.u32Xratio = 0;
		stChnZoomAttr.stZoomRatio.u32Yratio = 0;
		stChnZoomAttr.stZoomRatio.u32WidthRatio = 0;
		stChnZoomAttr.stZoomRatio.u32HeightRatio = 0;
		s32Ret = CVI_VO_SetChnZoomInWindow(VoLayer, VoChn, &stChnZoomAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SetChnZoomInWindow failed\n");
			break;
		}

		break;
	}

	case 14: {
		VO_CHN_PARAM_S stChnParam;
		VO_CHN_ATTR_S stChnAttr;

		s32Ret = vo_ut_send_frame(&vo_ut_file1[0 + intf * 2], VoDev);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vo_ut_send_frame failed\n");
			break;
		}

		stChnParam.stAspectRatio.enMode = ASPECT_RATIO_AUTO;
		stChnParam.stAspectRatio.bEnableBgColor = 1;
		stChnParam.stAspectRatio.u32BgColor = 0x0;
		s32Ret = CVI_VO_SetChnParam(VoLayer, VoChn, &stChnParam);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SetChnParam failed\n");
			break;
		}

		s32Ret = CVI_VO_GetChnParam(VoLayer, VoChn, &stChnParam);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetChnParam failed\n");
			break;
		}

		SAMPLE_PRT("Mode %d\n", stChnParam.stAspectRatio.enMode);
		SAMPLE_PRT("BgColorEn %d\n", stChnParam.stAspectRatio.bEnableBgColor);
		SAMPLE_PRT("BgColor %d\n", stChnParam.stAspectRatio.u32BgColor);
		SAMPLE_PRT("rect(%d %d %d %d)\n",
			stChnParam.stAspectRatio.stVideoRect.s32X,
			stChnParam.stAspectRatio.stVideoRect.s32Y,
			stChnParam.stAspectRatio.stVideoRect.u32Width,
			stChnParam.stAspectRatio.stVideoRect.u32Height);

		s32Ret = CVI_VO_GetChnAttr(VoLayer, VoChn, &stChnAttr);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetChnAttr failed\n");
			break;
		}
		stChnParam.stAspectRatio.enMode = ASPECT_RATIO_MANUAL;
		stChnParam.stAspectRatio.bEnableBgColor = 1;
		stChnParam.stAspectRatio.u32BgColor = 0x0;
		stChnParam.stAspectRatio.stVideoRect.s32X = 50;
		stChnParam.stAspectRatio.stVideoRect.s32Y = 50;
		stChnParam.stAspectRatio.stVideoRect.u32Width = stChnAttr.stRect.u32Width - 100;
		stChnParam.stAspectRatio.stVideoRect.u32Height = stChnAttr.stRect.u32Height - 100;
		s32Ret = CVI_VO_SetChnParam(VoLayer, VoChn, &stChnParam);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SetChnParam failed\n");
			break;
		}

		s32Ret = CVI_VO_GetChnParam(VoLayer, VoChn, &stChnParam);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetChnParam failed\n");
			break;
		}

		SAMPLE_PRT("Mode %d\n", stChnParam.stAspectRatio.enMode);
		SAMPLE_PRT("BgColorEn %d\n", stChnParam.stAspectRatio.bEnableBgColor);
		SAMPLE_PRT("BgColor %d\n", stChnParam.stAspectRatio.u32BgColor);
		SAMPLE_PRT("rect(%d %d %d %d)\n",
			stChnParam.stAspectRatio.stVideoRect.s32X,
			stChnParam.stAspectRatio.stVideoRect.s32Y,
			stChnParam.stAspectRatio.stVideoRect.u32Width,
			stChnParam.stAspectRatio.stVideoRect.u32Height);

		break;
	}

	case 15: {
		VO_CHN_BORDER_ATTR_S stChnBorder;

		s32Ret = vo_ut_send_frame(&vo_ut_file1[0 + intf * 2], VoDev);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vo_ut_send_frame failed\n");
			break;
		}

		stChnBorder.enable = 1;
		stChnBorder.stBorder.u32TopWidth = 14;
		stChnBorder.stBorder.u32BottomWidth = 14;
		stChnBorder.stBorder.u32LeftWidth = 14;
		stChnBorder.stBorder.u32RightWidth = 14;
		stChnBorder.stBorder.u32Color = 0x00FF0000;
		s32Ret = CVI_VO_SetChnBorder(VoLayer, VoChn, &stChnBorder);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SetChnBorder failed\n");
			break;
		}

		s32Ret = CVI_VO_GetChnBorder(VoLayer, VoChn, &stChnBorder);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_GetChnBorder failed\n");
			break;
		}
		SAMPLE_PRT("enable %d\n", stChnBorder.enable);
		SAMPLE_PRT("TopWidth %d\n", stChnBorder.stBorder.u32TopWidth);
		SAMPLE_PRT("BottomWidth %d\n", stChnBorder.stBorder.u32BottomWidth);
		SAMPLE_PRT("LeftWidth %d\n", stChnBorder.stBorder.u32LeftWidth);
		SAMPLE_PRT("RightWidth %d\n", stChnBorder.stBorder.u32RightWidth);

		stChnBorder.enable = 0;
		stChnBorder.stBorder.u32TopWidth = 0;
		stChnBorder.stBorder.u32BottomWidth = 0;
		stChnBorder.stBorder.u32LeftWidth = 0;
		stChnBorder.stBorder.u32RightWidth = 0;
		stChnBorder.stBorder.u32Color = 0x00000000;
		s32Ret = CVI_VO_SetChnBorder(VoLayer, VoChn, &stChnBorder);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SetChnBorder failed\n");
			break;
		}

		s32Ret = CVI_VO_SetChnMirror(VoLayer, VoChn, VO_CHN_MIRROR_HOR);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SetChnMirror failed\n");
			break;
		}

		s32Ret = CVI_VO_SetChnMirror(VoLayer, VoChn, VO_CHN_MIRROR_VER);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SetChnMirror failed\n");
			break;
		}

		s32Ret = CVI_VO_SetChnMirror(VoLayer, VoChn, VO_CHN_MIRROR_BOTH);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SetChnMirror failed\n");
			break;
		}

		s32Ret = CVI_VO_SetChnMirror(VoLayer, VoChn, VO_CHN_MIRROR_NONE);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("CVI_VO_SetChnMirror failed\n");
			break;
		}

		break;
	}

	case 30: {
		CVI_U32 index;
		SIZE_S stSize;
		SIZE_S stOutSize_720x1280 = {.u32Width = 720, .u32Height = 1280};
		SIZE_S stOutSize_1080x1920 = {.u32Width = 1080, .u32Height = 1920};

		for (index = 0; index < ARRAY_SIZE(vo_ut_file2); ++index) {
			s32Ret = vo_ut_vo_deinit();
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("vo_ut_vo_deinit failed\n");
				break;
			}
			s32Ret = vo_ut_vo_init_by_fmt(vo_ut_file2[index].enPixelFormat, VoDev);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("vo_ut_vo_init_by_fmt failed\n");
				break;
			}

			s32Ret = vo_ut_vpss_init_by_fmt(vo_ut_file2[index].enPixelFormat, vo_ut_file2[index].stSize
					, (intf == 0) ? stOutSize_720x1280 : stOutSize_1080x1920);
			if (s32Ret != CVI_SUCCESS) {
				vo_ut_vpss_deinit();
				SAMPLE_PRT("vo_ut_vpss_init_by_fmt failed\n");
				break;
			}

			s32Ret = SAMPLE_COMM_VPSS_Bind_VO(0, 0, VoLayer, VoChn);
			if (s32Ret != CVI_SUCCESS) {
				vo_ut_vpss_deinit();
				SAMPLE_PRT("SAMPLE_COMM_VPSS_Bind_VO failed\n");
				break;
			}

			stSize.u32Height = vo_ut_file2[index].stSize.u32Height;
			stSize.u32Width = vo_ut_file2[index].stSize.u32Width;

			s32Ret = SAMPLE_COMM_VPSS_SendFrame(0, &stSize,
						vo_ut_file2[index].enPixelFormat, vo_ut_file2[index].filename);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_COMM_VPSS_UnBind_VO(0, 0, VoLayer, VoChn);
				vo_ut_vpss_deinit();
				SAMPLE_PRT("s32Ret: 0x%x !\n", s32Ret);
				break;
			}

			usleep(30 * 1000);
			SAMPLE_COMM_VPSS_UnBind_VO(0, 0, VoLayer, VoChn);
			vo_ut_vpss_deinit();
		}

		break;
	}

	case 100: {
		s32Ret = vo_ioctl_test(vo_fd, VoDev);
		if (s32Ret != CVI_SUCCESS) {
			SAMPLE_PRT("vo_ioctl_test is fail\n");
		}

		break;
	}

	case 255: {
		break;
	}

	default:
		break;
	}

	if (s32Ret == CVI_SUCCESS)
		printf(GREEN"\n=== case(%d) pass ===\n"NONE"\n", op);
	else
		printf(RED"\n=== case(%d) fail===\n"NONE"\n", op);

	s32DeinitRet = vo_ut_vo_deinit();
	if (s32DeinitRet != CVI_SUCCESS) {
		SAMPLE_PRT("vo_ut_vo_deinit failed\n");
	}

	return s32Ret | s32DeinitRet;
}

int main(int argc, char *argv[])
{
	int op;
	VO_DEV VoDev = 0;
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (argc < 3) {
		SAMPLE_PRT("Usage: vo_ut vodev(0/1) intf(0:720x1280 1:1080x1920) [test_option]\n");
		return CVI_FAILURE;
	}

	s32Ret = vo_ut_sys_init();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vo_ut_sys_init failed\n");
		return CVI_FAILURE;
	}

	s32Ret = _vo_open_device();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("_vo_open_device failed\n");
		return CVI_FAILURE;
	}

	VoDev = (CVI_S32)atoi(argv[1]);
	intf = (int)atoi(argv[2]);

	if (argc >= 4) {
		op = (CVI_S32)atoi(argv[3]);
		s32Ret = _vo_ut_handle_op(op, VoDev);
		SAMPLE_PRT("vo ut dev(%d) op[%d] %s\n", VoDev, op, s32Ret == CVI_SUCCESS ? "pass" : "fail");
	} else {
		do {
			SAMPLE_PRT("1:   vo test Send one frame 720x1280(sensor image)\n");
			SAMPLE_PRT("2:   vo test Send one frame 1280x720(Rocket) with rotation 90\n");
			SAMPLE_PRT("3:   vo test Get VO info\n");
			SAMPLE_PRT("4:   vo test dev/videolayer/chn disable/enable\n");
			SAMPLE_PRT("5:   vo test Vo Chn Show/Hide & Step/Refresh & Pause/Resume & clrChnBuffer\n");
			SAMPLE_PRT("6:   vo test Set/Get gamma info\n");
			SAMPLE_PRT("7:   vo test ProcAmp ctrl\n");
			SAMPLE_PRT("8:   vo test Susepnd/Resume\n");
			SAMPLE_PRT("9:   vo test Get/Release screen frame\n");
			SAMPLE_PRT("10:  vo test Get/Release chn frame\n");
			SAMPLE_PRT("11:  vo test Get/Release wbc dev frame\n");
			SAMPLE_PRT("12:  vo test Get/Release wbc video frame\n");
			SAMPLE_PRT("13:  vo test Chn zoom\n");
			SAMPLE_PRT("14:  vo test Chn AspectRatio\n");
			SAMPLE_PRT("15:  vo test Chn Border & Mirror\n");
			SAMPLE_PRT("30:  vo test vo init by fmt & vpss bind vo\n");
			SAMPLE_PRT("100: vo test vo ioctl test\n");
			SAMPLE_PRT("255: exit\n");
			scanf("%d", &op);
			s32Ret = _vo_ut_handle_op(op, VoDev);
			if (s32Ret != CVI_SUCCESS) {
				SAMPLE_PRT("op(%d) failed with %#x!\n", op, s32Ret);
				break;
			}
		} while (op != 255);
	}

	s32Ret = vo_ut_sys_deinit();
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vo_ut_sys_deinit failed\n");
		return CVI_FAILURE;
	}

	_vo_close_device();

	return 0;
}


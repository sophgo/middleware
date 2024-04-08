#include "vi_ut.h"

int _isp_set_hdr(int fd)
{
	CVI_BOOL is_hdr_on = 1;

	S_CTRL_VALUE(fd, is_hdr_on, VI_IOCTL_HDR);
}

int _isp_set_3dnr(int fd)
{
	CVI_BOOL is_3dnr_on = 1;

	S_CTRL_VALUE(fd, is_3dnr_on, VI_IOCTL_3DNR);
}

int _isp_set_tile(int fd)
{
	CVI_BOOL is_tile_on = 1;

	S_CTRL_VALUE(fd, is_tile_on, VI_IOCTL_TILE);
}

int _isp_set_compress_en(int fd)
{
	CVI_BOOL is_compress_en = 1;

	S_CTRL_VALUE(fd, is_compress_en, VI_IOCTL_COMPRESS_EN);
}

int _isp_set_snr_info(int fd)
{
	struct cvi_isp_snr_info cfg;
	CVI_U8 i = 0;

	cfg.raw_num = 0;
	cfg.color_mode = BAYER_RGGB;
	cfg.snr_fmt.frm_num = 2;
	for (i = 0; i < cfg.snr_fmt.frm_num; i++) {
		cfg.snr_fmt.img_size[i].max_width = 1920;
		cfg.snr_fmt.img_size[i].max_height = 1080;
		cfg.snr_fmt.img_size[i].start_x = 0;
		cfg.snr_fmt.img_size[i].start_y = 0;
		cfg.snr_fmt.img_size[i].active_w = 1920;
		cfg.snr_fmt.img_size[i].active_h = 1080;
		cfg.snr_fmt.img_size[i].width = 1920;
		cfg.snr_fmt.img_size[i].height = 1080;
	}

	S_CTRL_PTR(fd, &cfg, VI_IOCTL_SET_SNR_INFO);
}

int _isp_set_pipe_dump(int fd)
{
	CVI_U32 dev_num = 1;

	S_CTRL_VALUE(fd, dev_num, VI_IOCTL_PUT_PIPE_DUMP);
}

int _isp_get_sts_mem(int fd)
{
	struct cvi_isp_sts_mem mem_info[2];

	mem_info[0].raw_num = 0;

	G_CTRL_PTR(fd, mem_info, VI_IOCTL_STS_MEM);

	VI_UT_PRT("ae_le.phy_addr=%llx, size=%d\n", mem_info[0].ae_le.phy_addr, mem_info[0].ae_le.size);
	VI_UT_PRT("af.phy_addr=%llx, size=%d\n", mem_info[0].af.phy_addr, mem_info[0].af.size);
	VI_UT_PRT("awb.phy_addr=%llx, size=%d\n", mem_info[0].awb.phy_addr, mem_info[0].awb.size);

	return 0;
}

int _isp_get_sts_get(int fd)
{
	CVI_U8 pre_sts_busy_idx = 0;

	G_CTRL_VALUE(fd, &pre_sts_busy_idx, VI_IOCTL_STS_GET);

	VI_UT_PRT("pre_sts_busy_idx=%d\n", pre_sts_busy_idx);

	return 0;
}

int _isp_set_sts_put(int fd)
{
	CVI_U8 raw_num = 0;

	S_CTRL_VALUE(fd, raw_num, VI_IOCTL_STS_PUT);
}

int _isp_set_usr_pic_onoff(int fd)
{
	CVI_BOOL enable = 1;

	S_CTRL_VALUE(fd, enable, VI_IOCTL_USR_PIC_ONOFF);
}

int _isp_set_usr_pic_cfg(int fd)
{
	struct cvi_isp_usr_pic_cfg cfg;

	cfg.fmt.width = 1920;
	cfg.fmt.height = 1080;
	cfg.fmt.code = BAYER_FORMAT_GR;
	cfg.crop.left = 0;
	cfg.crop.top = 0;
	cfg.crop.width = 1920;
	cfg.crop.height = 1080;

	S_CTRL_PTR(fd, &cfg, VI_IOCTL_USR_PIC_CFG);
}

int _isp_set_usr_pic_put(int fd)
{
	CVI_U64 phyAddr = 0x12345678;

	S_CTRL_VALUE64(fd, phyAddr, VI_IOCTL_USR_PIC_PUT);
}

int _isp_set_usr_pic_timing(int fd)
{
	CVI_U32 fps = 30;

	S_CTRL_VALUE(fd, fps, VI_IOCTL_USR_PIC_TIMING);
}

int _isp_get_lsc_phy_buf(int fd)
{
	struct cvi_vip_memblock mem_info;

	mem_info.raw_num = 0;

	G_CTRL_PTR(fd, &mem_info, VI_IOCTL_GET_LSC_PHY_BUF);

	VI_UT_PRT("lsc phy addr=0x%llx\n", mem_info.phy_addr);
	VI_UT_PRT("lsc size=0x%x\n", mem_info.size);

	return 0;
}

int _isp_get_post_sts_get(int fd)
{
	CVI_U8 post_sts_busy_idx = 0;

	G_CTRL_VALUE(fd, &post_sts_busy_idx, VI_IOCTL_POST_STS_GET);

	VI_UT_PRT("post_sts_busy_idx=%d\n", post_sts_busy_idx);

	return 0;
}

int _isp_set_post_sts_put(int fd)
{
	CVI_U8 raw_num = 0;

	S_CTRL_VALUE(fd, raw_num, VI_IOCTL_POST_STS_PUT);
}

int _isp_get_awb_sts_get(int fd)
{
	struct cvi_vip_isp_awb_sts awb_sts;

	awb_sts.raw_num = 0;

	G_CTRL_PTR(fd, &awb_sts, VI_IOCTL_AWB_STS_GET);

	VI_UT_PRT("awb_sts.is_se=%d\n", awb_sts.is_se);
	VI_UT_PRT("awb_sts.buf_idx=%d\n", awb_sts.buf_idx);

	return 0;
}

int _isp_set_awb_sts_put(int fd)
{
	CVI_U8 raw_num = 0;

	S_CTRL_VALUE(fd, raw_num, VI_IOCTL_AWB_STS_PUT);
}

int _isp_get_fswdr_phy_buf(int fd)
{
	struct cvi_vip_memblock mem_info;

	mem_info.raw_num = 0;

	G_CTRL_PTR(fd, &mem_info, VI_IOCTL_GET_FSWDR_PHY_BUF);

	VI_UT_PRT("fswdr vir addr=%p\n", mem_info.vir_addr);
	VI_UT_PRT("fswdr phy addr=0x%llx\n", mem_info.phy_addr);
	VI_UT_PRT("fswdr size=0x%x\n", mem_info.size);

	return 0;
}

int _isp_get_tun_addr(int fd)
{
	struct isp_tuning_cfg tun_buf_info;

	G_CTRL_PTR(fd, &tun_buf_info, VI_IOCTL_GET_TUN_ADDR);

	VI_UT_PRT("tun_buf_info.fe_addr=0x%"PRIx64"\n", tun_buf_info.fe_addr[0]);
	VI_UT_PRT("tun_buf_info.be_addr=0x%"PRIx64"\n", tun_buf_info.be_addr[0]);
	VI_UT_PRT("tun_buf_info.post_addr=0x%"PRIx64"\n", tun_buf_info.post_addr[0]);

	return 0;
}

int _isp_get_dma_size(int fd)
{
	CVI_U32 size = 0;

	G_CTRL_VALUE(fd, &size, VI_IOCTL_GET_BUF_SIZE);

	VI_UT_PRT("size=%d\n", size);

	return 0;
}

int _isp_set_be_online(int fd)
{
	CVI_BOOL online = 1;

	S_CTRL_VALUE(fd, online, VI_IOCTL_BE_ONLINE);
}

static int (*ioctl_test[VI_IOCTL_MAX])(int) = {
	[VI_IOCTL_HDR]			= _isp_set_hdr,
	[VI_IOCTL_3DNR]			= _isp_set_3dnr,
	[VI_IOCTL_TILE]			= _isp_set_tile,
	[VI_IOCTL_COMPRESS_EN]		= _isp_set_compress_en,
	[VI_IOCTL_BE_ONLINE]		= _isp_set_be_online,
	[VI_IOCTL_GET_BUF_SIZE]		= _isp_get_dma_size,
	[VI_IOCTL_STS_MEM]		= _isp_get_sts_mem,
	[VI_IOCTL_STS_GET]		= _isp_get_sts_get,
	[VI_IOCTL_STS_PUT]		= _isp_set_sts_put,
	[VI_IOCTL_USR_PIC_ONOFF]	= _isp_set_usr_pic_onoff,
	[VI_IOCTL_USR_PIC_CFG]		= _isp_set_usr_pic_cfg,
	[VI_IOCTL_USR_PIC_PUT]		= _isp_set_usr_pic_put,
	[VI_IOCTL_USR_PIC_TIMING]	= _isp_set_usr_pic_timing,
	[VI_IOCTL_GET_LSC_PHY_BUF]	= _isp_get_lsc_phy_buf,
	[VI_IOCTL_POST_STS_GET]		= _isp_get_post_sts_get,
	[VI_IOCTL_POST_STS_PUT]		= _isp_set_post_sts_put,
	[VI_IOCTL_AWB_STS_GET]		= _isp_get_awb_sts_get,
	[VI_IOCTL_AWB_STS_PUT]		= _isp_set_awb_sts_put,
	[VI_IOCTL_GET_FSWDR_PHY_BUF]	= _isp_get_fswdr_phy_buf,
	[VI_IOCTL_GET_TUN_ADDR]		= _isp_get_tun_addr,
	[VI_IOCTL_SET_SNR_INFO]		= _isp_set_snr_info,
	[VI_IOCTL_PUT_PIPE_DUMP]	= _isp_set_pipe_dump,
};

static int vi_check_ioctl_ret(enum VI_IOCTL ioctl, int ret)
{
	switch (ioctl) {
	case VI_IOCTL_HDR:
#ifdef __CV180X__
		if (ret != 0)
			return 0;
#endif
		break;
	default:
		break;
	}
	return ret;
}

CVI_S32 vi_ioctl_test(int fd)
{
	CVI_S32 ret = 0;
	CVI_U32 i = 0;

	for (i = 0; i < ARRAY_SIZE(ioctl_test); i++) {
		if (ioctl_test[i] != NULL) {
			ret = ioctl_test[i](fd);
			if (vi_check_ioctl_ret(i, ret) != 0)
				return ret;
		}
	}

	return ret;
}

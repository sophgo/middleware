#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <syslog.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#ifdef ARCH_CV182X
#include <linux/cvi_vip_snsr.h>
#include "cvi_comm_video.h"
#else

#include <cvi_comm_video.h>
#endif
#include "cvi_sns_ctrl.h"
#include "sc1330_cmos_ex.h"

static void sc1330_linear_960p30_init(VI_PIPE ViPipe);
static void sc1330_linear_960p30_slave_init(VI_PIPE ViPipe);
static void sc1330_linear_960p30_master_init(VI_PIPE ViPipe);

#define SC1330_CHIP_ID_HI_ADDR		0x3107
#define SC1330_CHIP_ID_LO_ADDR		0x3108
#define SC1330_CHIP_ID				0xca18

/*msg type  address:16-bit  data:8-bit  dev addr:7-bit*/
const CVI_U32 sc1330_addr_byte = 2;
const CVI_U32 sc1330_data_byte = 1;
static int g_fd[VI_MAX_PIPE_NUM] = {[0 ... (VI_MAX_PIPE_NUM - 1)] = -1};

int sc1330_i2c_init(VI_PIPE ViPipe)
{
	char acDevFile[16] = {0};
	CVI_U8 u8DevNum;

	if (g_fd[ViPipe] >= 0)
		return CVI_SUCCESS;
	int ret;

	u8DevNum = g_aunSC1330_BusInfo[ViPipe].s8I2cDev;
	// printf("iic u8DevNum = %d\n", u8DevNum);
	snprintf(acDevFile, sizeof(acDevFile),  "/dev/i2c-%u", u8DevNum);

	g_fd[ViPipe] = open(acDevFile, O_RDWR, 0600);

	if (g_fd[ViPipe] < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Open /dev/cvi_i2c_drv-%u error!\n", u8DevNum);
		return CVI_FAILURE;
	}

	ret = ioctl(g_fd[ViPipe], I2C_SLAVE_FORCE, g_aunSC1330_AddrInfo[ViPipe].s8I2cAddr);
	if (ret < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "I2C_SLAVE_FORCE error!\n");
		close(g_fd[ViPipe]);
		g_fd[ViPipe] = -1;
		return ret;
	}

	return CVI_SUCCESS;
}

int sc1330_i2c_exit(VI_PIPE ViPipe)
{
	if (g_fd[ViPipe] >= 0) {
		close(g_fd[ViPipe]);
		g_fd[ViPipe] = -1;
		return CVI_SUCCESS;
	}
	return CVI_FAILURE;
}

int sc1330_read_register(VI_PIPE ViPipe, int addr)
{
	int ret, data;
	CVI_U8 buf[8];
	CVI_U8 idx = 0;

	if (g_fd[ViPipe] < 0)
		return CVI_FAILURE;

	if (sc1330_addr_byte == 2)
		buf[idx++] = (addr >> 8) & 0xff;

	// add address byte 0
	buf[idx++] = addr & 0xff;

	ret = write(g_fd[ViPipe], buf, sc1330_addr_byte);
	if (ret < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "I2C_WRITE error!\n");
		return ret;
	}

	buf[0] = 0;
	buf[1] = 0;
	ret = read(g_fd[ViPipe], buf, sc1330_data_byte);
	if (ret < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "I2C_READ error!\n");
		return ret;
	}

	// pack read back data
	data = 0;
	if (sc1330_data_byte == 2) {
		data = buf[0] << 8;
		data += buf[1];
	} else {
		data = buf[0];
	}

	// printf("i2c r 0x%x = 0x%x\n", addr, data);
	return data;
}

int sc1330_write_register(VI_PIPE ViPipe, int addr, int data)
{
	int idx = 0;
	int ret;
	char buf[8];

	if (g_fd[ViPipe] < 0)
		return CVI_SUCCESS;

	if (sc1330_addr_byte == 2) {
		buf[idx] = (addr >> 8) & 0xff;
		idx++;
		buf[idx] = addr & 0xff;
		idx++;
	}

	if (sc1330_data_byte == 1) {
		buf[idx] = data & 0xff;
		idx++;
	}

	ret = write(g_fd[ViPipe], buf, sc1330_addr_byte + sc1330_data_byte);
	if (ret < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "I2C_WRITE error!\n");
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void sc1330_standby(VI_PIPE ViPipe)
{
	sc1330_write_register(ViPipe, 0x0100, 0x00);
}

void sc1330_restart(VI_PIPE ViPipe)
{
	sc1330_write_register(ViPipe, 0x0100, 0x00);
	delay_ms(20);
	sc1330_write_register(ViPipe, 0x0100, 0x01);
}

void sc1330_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastSC1330[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		sc1330_write_register(ViPipe,
			g_pastSC1330[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
			g_pastSC1330[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void sc1330_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 val = 0;

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		val |= 0x6;
		break;
	case ISP_SNS_FLIP:
		val |= 0xE0;
		break;
	case ISP_SNS_MIRROR_FLIP:
		val |= 0xE6;
		break;
	default:
		return;
	}

	sc1330_write_register(ViPipe, 0x3221, val);
}

int sc1330_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	usleep(4*1000);
	if (sc1330_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = sc1330_read_register(ViPipe, SC1330_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = sc1330_read_register(ViPipe, SC1330_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != SC1330_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}


void sc1330_init(VI_PIPE ViPipe)
{
	CVI_U8 u8ImgMode = g_pastSC1330[ViPipe]->u8ImgMode;

	sc1330_i2c_init(ViPipe);

	if (u8ImgMode == SC1330_MODE_960P30)
		sc1330_linear_960p30_init(ViPipe);
	else if (u8ImgMode == SC1330_MODE_960P30_MASTER)
		sc1330_linear_960p30_master_init(ViPipe);
	else if (u8ImgMode == SC1330_MODE_960P30_SLAVE)
		sc1330_linear_960p30_slave_init(ViPipe);

	g_pastSC1330[ViPipe]->bInit = CVI_TRUE;
}

/* 960P30 */
static void sc1330_linear_960p30_slave_init(VI_PIPE ViPipe)
{
	sc1330_write_register(ViPipe, 0x0103, 0x01);
	sc1330_write_register(ViPipe, 0x0100, 0x00);
	sc1330_write_register(ViPipe, 0x36e9, 0x80);
	sc1330_write_register(ViPipe, 0x36f9, 0x80);
	sc1330_write_register(ViPipe, 0x301f, 0x5f);
	sc1330_write_register(ViPipe, 0x3031, 0x0a);
	sc1330_write_register(ViPipe, 0x3037, 0x20);
	sc1330_write_register(ViPipe, 0x3211, 0x06);
	sc1330_write_register(ViPipe, 0x3251, 0x98);
	sc1330_write_register(ViPipe, 0x3253, 0x08);
	sc1330_write_register(ViPipe, 0x325f, 0x0a);
	sc1330_write_register(ViPipe, 0x3304, 0x40);
	sc1330_write_register(ViPipe, 0x3306, 0x70);
	sc1330_write_register(ViPipe, 0x3309, 0x70);
	sc1330_write_register(ViPipe, 0x330a, 0x01);
	sc1330_write_register(ViPipe, 0x330b, 0xf0);
	sc1330_write_register(ViPipe, 0x330d, 0x30);
	sc1330_write_register(ViPipe, 0x3310, 0x0e);
	sc1330_write_register(ViPipe, 0x3314, 0x92);
	sc1330_write_register(ViPipe, 0x331e, 0x31);
	sc1330_write_register(ViPipe, 0x331f, 0x61);
	sc1330_write_register(ViPipe, 0x335d, 0x60);
	sc1330_write_register(ViPipe, 0x3364, 0x5e);
	sc1330_write_register(ViPipe, 0x3396, 0x08);
	sc1330_write_register(ViPipe, 0x3397, 0x18);
	sc1330_write_register(ViPipe, 0x3398, 0x38);
	sc1330_write_register(ViPipe, 0x3399, 0x0c);
	sc1330_write_register(ViPipe, 0x339a, 0x10);
	sc1330_write_register(ViPipe, 0x339b, 0x1e);
	sc1330_write_register(ViPipe, 0x339c, 0x70);
	sc1330_write_register(ViPipe, 0x33af, 0x38);
	sc1330_write_register(ViPipe, 0x360f, 0x21);
	sc1330_write_register(ViPipe, 0x3621, 0xe8);
	sc1330_write_register(ViPipe, 0x3632, 0x68);
	sc1330_write_register(ViPipe, 0x3633, 0x33);
	sc1330_write_register(ViPipe, 0x3634, 0x23);
	sc1330_write_register(ViPipe, 0x3635, 0x20);
	sc1330_write_register(ViPipe, 0x3637, 0x19);
	sc1330_write_register(ViPipe, 0x3638, 0x08);
	sc1330_write_register(ViPipe, 0x363b, 0x04);
	sc1330_write_register(ViPipe, 0x363c, 0x06);
	sc1330_write_register(ViPipe, 0x3670, 0x42);
	sc1330_write_register(ViPipe, 0x3671, 0x05);
	sc1330_write_register(ViPipe, 0x3672, 0x15);
	sc1330_write_register(ViPipe, 0x3673, 0x15);
	sc1330_write_register(ViPipe, 0x3674, 0xc0);
	sc1330_write_register(ViPipe, 0x3675, 0x84);
	sc1330_write_register(ViPipe, 0x3676, 0x88);
	sc1330_write_register(ViPipe, 0x367a, 0x48);
	sc1330_write_register(ViPipe, 0x367b, 0x58);
	sc1330_write_register(ViPipe, 0x367c, 0x48);
	sc1330_write_register(ViPipe, 0x367d, 0x58);
	sc1330_write_register(ViPipe, 0x3699, 0x00);
	sc1330_write_register(ViPipe, 0x369a, 0x00);
	sc1330_write_register(ViPipe, 0x369b, 0x1f);
	sc1330_write_register(ViPipe, 0x36a2, 0x48);
	sc1330_write_register(ViPipe, 0x36a3, 0x58);
	sc1330_write_register(ViPipe, 0x36a6, 0x48);
	sc1330_write_register(ViPipe, 0x36a7, 0x58);
	sc1330_write_register(ViPipe, 0x36ab, 0xc0);
	sc1330_write_register(ViPipe, 0x36ac, 0x84);
	sc1330_write_register(ViPipe, 0x36ad, 0x88);
	sc1330_write_register(ViPipe, 0x36d0, 0x40);
	sc1330_write_register(ViPipe, 0x36db, 0x04);
	sc1330_write_register(ViPipe, 0x36dc, 0x14);
	sc1330_write_register(ViPipe, 0x36dd, 0x14);
	sc1330_write_register(ViPipe, 0x36de, 0x48);
	sc1330_write_register(ViPipe, 0x36df, 0x58);
	sc1330_write_register(ViPipe, 0x36ea, 0x38);
	sc1330_write_register(ViPipe, 0x36eb, 0x15);
	sc1330_write_register(ViPipe, 0x36ec, 0x15);
	sc1330_write_register(ViPipe, 0x36ed, 0x14);
	sc1330_write_register(ViPipe, 0x36fa, 0x34);
	sc1330_write_register(ViPipe, 0x36fb, 0x00);
	sc1330_write_register(ViPipe, 0x36fc, 0x10);
	sc1330_write_register(ViPipe, 0x36fd, 0x14);
	sc1330_write_register(ViPipe, 0x3e01, 0x3e);
	sc1330_write_register(ViPipe, 0x450a, 0x71);
	sc1330_write_register(ViPipe, 0x4819, 0x04);
	sc1330_write_register(ViPipe, 0x481b, 0x02);
	sc1330_write_register(ViPipe, 0x481d, 0x06);
	sc1330_write_register(ViPipe, 0x481f, 0x02);
	sc1330_write_register(ViPipe, 0x4821, 0x07);
	sc1330_write_register(ViPipe, 0x4823, 0x02);
	sc1330_write_register(ViPipe, 0x4825, 0x02);
	sc1330_write_register(ViPipe, 0x4827, 0x02);
	sc1330_write_register(ViPipe, 0x4829, 0x03);
	sc1330_write_register(ViPipe, 0x578a, 0x18);
	sc1330_write_register(ViPipe, 0x578b, 0x10);
	sc1330_write_register(ViPipe, 0x5793, 0x18);
	sc1330_write_register(ViPipe, 0x5794, 0x10);
	sc1330_write_register(ViPipe, 0x5799, 0x00);
	sc1330_write_register(ViPipe, 0x3222, 0x01);  // slave mode
	sc1330_write_register(ViPipe, 0x36e9, 0x24);
	sc1330_write_register(ViPipe, 0x36f9, 0x30);

	sc1330_default_reg_init(ViPipe);

	sc1330_write_register(ViPipe, 0x0100, 0x01);

	printf("ViPipe:%d,===SC1330 960P 30fps 10bit SLAVE LINE Init OK!===\n", ViPipe);
}

/* 960P60 */
static void sc1330_linear_960p30_master_init(VI_PIPE ViPipe)
{
	sc1330_write_register(ViPipe, 0x0103, 0x01);
	sc1330_write_register(ViPipe, 0x0100, 0x00);
	sc1330_write_register(ViPipe, 0x36e9, 0x80);
	sc1330_write_register(ViPipe, 0x36f9, 0x80);
	sc1330_write_register(ViPipe, 0x301f, 0x5f);
	sc1330_write_register(ViPipe, 0x3031, 0x0a);
	sc1330_write_register(ViPipe, 0x3037, 0x20);
	sc1330_write_register(ViPipe, 0x3211, 0x06);
	sc1330_write_register(ViPipe, 0x3251, 0x98);
	sc1330_write_register(ViPipe, 0x3253, 0x08);
	sc1330_write_register(ViPipe, 0x325f, 0x0a);
	sc1330_write_register(ViPipe, 0x3304, 0x40);
	sc1330_write_register(ViPipe, 0x3306, 0x70);
	sc1330_write_register(ViPipe, 0x3309, 0x70);
	sc1330_write_register(ViPipe, 0x330a, 0x01);
	sc1330_write_register(ViPipe, 0x330b, 0xf0);
	sc1330_write_register(ViPipe, 0x330d, 0x30);
	sc1330_write_register(ViPipe, 0x3310, 0x0e);
	sc1330_write_register(ViPipe, 0x3314, 0x92);
	sc1330_write_register(ViPipe, 0x331e, 0x31);
	sc1330_write_register(ViPipe, 0x331f, 0x61);
	sc1330_write_register(ViPipe, 0x335d, 0x60);
	sc1330_write_register(ViPipe, 0x3364, 0x5e);
	sc1330_write_register(ViPipe, 0x3396, 0x08);
	sc1330_write_register(ViPipe, 0x3397, 0x18);
	sc1330_write_register(ViPipe, 0x3398, 0x38);
	sc1330_write_register(ViPipe, 0x3399, 0x0c);
	sc1330_write_register(ViPipe, 0x339a, 0x10);
	sc1330_write_register(ViPipe, 0x339b, 0x1e);
	sc1330_write_register(ViPipe, 0x339c, 0x70);
	sc1330_write_register(ViPipe, 0x33af, 0x38);
	sc1330_write_register(ViPipe, 0x360f, 0x21);
	sc1330_write_register(ViPipe, 0x3621, 0xe8);
	sc1330_write_register(ViPipe, 0x3632, 0x68);
	sc1330_write_register(ViPipe, 0x3633, 0x33);
	sc1330_write_register(ViPipe, 0x3634, 0x23);
	sc1330_write_register(ViPipe, 0x3635, 0x20);
	sc1330_write_register(ViPipe, 0x3637, 0x19);
	sc1330_write_register(ViPipe, 0x3638, 0x08);
	sc1330_write_register(ViPipe, 0x363b, 0x04);
	sc1330_write_register(ViPipe, 0x363c, 0x06);
	sc1330_write_register(ViPipe, 0x3670, 0x42);
	sc1330_write_register(ViPipe, 0x3671, 0x05);
	sc1330_write_register(ViPipe, 0x3672, 0x15);
	sc1330_write_register(ViPipe, 0x3673, 0x15);
	sc1330_write_register(ViPipe, 0x3674, 0xc0);
	sc1330_write_register(ViPipe, 0x3675, 0x84);
	sc1330_write_register(ViPipe, 0x3676, 0x88);
	sc1330_write_register(ViPipe, 0x367a, 0x48);
	sc1330_write_register(ViPipe, 0x367b, 0x58);
	sc1330_write_register(ViPipe, 0x367c, 0x48);
	sc1330_write_register(ViPipe, 0x367d, 0x58);
	sc1330_write_register(ViPipe, 0x3699, 0x00);
	sc1330_write_register(ViPipe, 0x369a, 0x00);
	sc1330_write_register(ViPipe, 0x369b, 0x1f);
	sc1330_write_register(ViPipe, 0x36a2, 0x48);
	sc1330_write_register(ViPipe, 0x36a3, 0x58);
	sc1330_write_register(ViPipe, 0x36a6, 0x48);
	sc1330_write_register(ViPipe, 0x36a7, 0x58);
	sc1330_write_register(ViPipe, 0x36ab, 0xc0);
	sc1330_write_register(ViPipe, 0x36ac, 0x84);
	sc1330_write_register(ViPipe, 0x36ad, 0x88);
	sc1330_write_register(ViPipe, 0x36d0, 0x40);
	sc1330_write_register(ViPipe, 0x36db, 0x04);
	sc1330_write_register(ViPipe, 0x36dc, 0x14);
	sc1330_write_register(ViPipe, 0x36dd, 0x14);
	sc1330_write_register(ViPipe, 0x36de, 0x48);
	sc1330_write_register(ViPipe, 0x36df, 0x58);
	sc1330_write_register(ViPipe, 0x36ea, 0x38);
	sc1330_write_register(ViPipe, 0x36eb, 0x15);
	sc1330_write_register(ViPipe, 0x36ec, 0x15);
	sc1330_write_register(ViPipe, 0x36ed, 0x14);
	sc1330_write_register(ViPipe, 0x36fa, 0x34);
	sc1330_write_register(ViPipe, 0x36fb, 0x00);
	sc1330_write_register(ViPipe, 0x36fc, 0x10);
	sc1330_write_register(ViPipe, 0x36fd, 0x14);
	sc1330_write_register(ViPipe, 0x3e01, 0x3e);
	sc1330_write_register(ViPipe, 0x450a, 0x71);
	sc1330_write_register(ViPipe, 0x4819, 0x04);
	sc1330_write_register(ViPipe, 0x481b, 0x02);
	sc1330_write_register(ViPipe, 0x481d, 0x06);
	sc1330_write_register(ViPipe, 0x481f, 0x02);
	sc1330_write_register(ViPipe, 0x4821, 0x07);
	sc1330_write_register(ViPipe, 0x4823, 0x02);
	sc1330_write_register(ViPipe, 0x4825, 0x02);
	sc1330_write_register(ViPipe, 0x4827, 0x02);
	sc1330_write_register(ViPipe, 0x4829, 0x03);
	sc1330_write_register(ViPipe, 0x578a, 0x18);
	sc1330_write_register(ViPipe, 0x578b, 0x10);
	sc1330_write_register(ViPipe, 0x5793, 0x18);
	sc1330_write_register(ViPipe, 0x5794, 0x10);
	sc1330_write_register(ViPipe, 0x5799, 0x00);
	sc1330_write_register(ViPipe, 0x300a, 0x24); //master mode
	sc1330_write_register(ViPipe, 0x3032, 0xa0);
	sc1330_write_register(ViPipe, 0x36e9, 0x24);
	sc1330_write_register(ViPipe, 0x36f9, 0x30);

	sc1330_default_reg_init(ViPipe);

	sc1330_write_register(ViPipe, 0x0100, 0x01);

	printf("ViPipe:%d,===SC1330 960P 30fps 10bit MASTER LINE Init OK!===\n", ViPipe);
}

/* 960P60 */
static void sc1330_linear_960p30_init(VI_PIPE ViPipe)
{
	sc1330_write_register(ViPipe, 0x0103, 0x01);
	sc1330_write_register(ViPipe, 0x0100, 0x00);
	sc1330_write_register(ViPipe, 0x36e9, 0x80);
	sc1330_write_register(ViPipe, 0x36f9, 0x80);
	sc1330_write_register(ViPipe, 0x301f, 0x5f);
	sc1330_write_register(ViPipe, 0x3031, 0x0a);
	sc1330_write_register(ViPipe, 0x3037, 0x20);
	sc1330_write_register(ViPipe, 0x3211, 0x06);
	sc1330_write_register(ViPipe, 0x3251, 0x98);
	sc1330_write_register(ViPipe, 0x3253, 0x08);
	sc1330_write_register(ViPipe, 0x325f, 0x0a);
	sc1330_write_register(ViPipe, 0x3304, 0x40);
	sc1330_write_register(ViPipe, 0x3306, 0x70);
	sc1330_write_register(ViPipe, 0x3309, 0x70);
	sc1330_write_register(ViPipe, 0x330a, 0x01);
	sc1330_write_register(ViPipe, 0x330b, 0xf0);
	sc1330_write_register(ViPipe, 0x330d, 0x30);
	sc1330_write_register(ViPipe, 0x3310, 0x0e);
	sc1330_write_register(ViPipe, 0x3314, 0x92);
	sc1330_write_register(ViPipe, 0x331e, 0x31);
	sc1330_write_register(ViPipe, 0x331f, 0x61);
	sc1330_write_register(ViPipe, 0x335d, 0x60);
	sc1330_write_register(ViPipe, 0x3364, 0x5e);
	sc1330_write_register(ViPipe, 0x3396, 0x08);
	sc1330_write_register(ViPipe, 0x3397, 0x18);
	sc1330_write_register(ViPipe, 0x3398, 0x38);
	sc1330_write_register(ViPipe, 0x3399, 0x0c);
	sc1330_write_register(ViPipe, 0x339a, 0x10);
	sc1330_write_register(ViPipe, 0x339b, 0x1e);
	sc1330_write_register(ViPipe, 0x339c, 0x70);
	sc1330_write_register(ViPipe, 0x33af, 0x38);
	sc1330_write_register(ViPipe, 0x360f, 0x21);
	sc1330_write_register(ViPipe, 0x3621, 0xe8);
	sc1330_write_register(ViPipe, 0x3632, 0x68);
	sc1330_write_register(ViPipe, 0x3633, 0x33);
	sc1330_write_register(ViPipe, 0x3634, 0x23);
	sc1330_write_register(ViPipe, 0x3635, 0x20);
	sc1330_write_register(ViPipe, 0x3637, 0x19);
	sc1330_write_register(ViPipe, 0x3638, 0x08);
	sc1330_write_register(ViPipe, 0x363b, 0x04);
	sc1330_write_register(ViPipe, 0x363c, 0x06);
	sc1330_write_register(ViPipe, 0x3670, 0x42);
	sc1330_write_register(ViPipe, 0x3671, 0x05);
	sc1330_write_register(ViPipe, 0x3672, 0x15);
	sc1330_write_register(ViPipe, 0x3673, 0x15);
	sc1330_write_register(ViPipe, 0x3674, 0xc0);
	sc1330_write_register(ViPipe, 0x3675, 0x84);
	sc1330_write_register(ViPipe, 0x3676, 0x88);
	sc1330_write_register(ViPipe, 0x367a, 0x48);
	sc1330_write_register(ViPipe, 0x367b, 0x58);
	sc1330_write_register(ViPipe, 0x367c, 0x48);
	sc1330_write_register(ViPipe, 0x367d, 0x58);
	sc1330_write_register(ViPipe, 0x3699, 0x00);
	sc1330_write_register(ViPipe, 0x369a, 0x00);
	sc1330_write_register(ViPipe, 0x369b, 0x1f);
	sc1330_write_register(ViPipe, 0x36a2, 0x48);
	sc1330_write_register(ViPipe, 0x36a3, 0x58);
	sc1330_write_register(ViPipe, 0x36a6, 0x48);
	sc1330_write_register(ViPipe, 0x36a7, 0x58);
	sc1330_write_register(ViPipe, 0x36ab, 0xc0);
	sc1330_write_register(ViPipe, 0x36ac, 0x84);
	sc1330_write_register(ViPipe, 0x36ad, 0x88);
	sc1330_write_register(ViPipe, 0x36d0, 0x40);
	sc1330_write_register(ViPipe, 0x36db, 0x04);
	sc1330_write_register(ViPipe, 0x36dc, 0x14);
	sc1330_write_register(ViPipe, 0x36dd, 0x14);
	sc1330_write_register(ViPipe, 0x36de, 0x48);
	sc1330_write_register(ViPipe, 0x36df, 0x58);
	sc1330_write_register(ViPipe, 0x36ea, 0x38);
	sc1330_write_register(ViPipe, 0x36eb, 0x15);
	sc1330_write_register(ViPipe, 0x36ec, 0x15);
	sc1330_write_register(ViPipe, 0x36ed, 0x14);
	sc1330_write_register(ViPipe, 0x36fa, 0x34);
	sc1330_write_register(ViPipe, 0x36fb, 0x00);
	sc1330_write_register(ViPipe, 0x36fc, 0x10);
	sc1330_write_register(ViPipe, 0x36fd, 0x14);
	sc1330_write_register(ViPipe, 0x3e01, 0x3e);
	sc1330_write_register(ViPipe, 0x450a, 0x71);
	sc1330_write_register(ViPipe, 0x4819, 0x04);
	sc1330_write_register(ViPipe, 0x481b, 0x02);
	sc1330_write_register(ViPipe, 0x481d, 0x06);
	sc1330_write_register(ViPipe, 0x481f, 0x02);
	sc1330_write_register(ViPipe, 0x4821, 0x07);
	sc1330_write_register(ViPipe, 0x4823, 0x02);
	sc1330_write_register(ViPipe, 0x4825, 0x02);
	sc1330_write_register(ViPipe, 0x4827, 0x02);
	sc1330_write_register(ViPipe, 0x4829, 0x03);
	sc1330_write_register(ViPipe, 0x578a, 0x18);
	sc1330_write_register(ViPipe, 0x578b, 0x10);
	sc1330_write_register(ViPipe, 0x5793, 0x18);
	sc1330_write_register(ViPipe, 0x5794, 0x10);
	sc1330_write_register(ViPipe, 0x5799, 0x00);
	sc1330_write_register(ViPipe, 0x36e9, 0x24);
	sc1330_write_register(ViPipe, 0x36f9, 0x30);

	sc1330_default_reg_init(ViPipe);

	sc1330_write_register(ViPipe, 0x0100, 0x01);

	printf("ViPipe:%d,===SC1330 960P 30fps 10bit LINE Init OK!===\n", ViPipe);
}
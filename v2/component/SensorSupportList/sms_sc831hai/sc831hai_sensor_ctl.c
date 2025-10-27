#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "sc831hai_cmos_ex.h"
#include "sensor_i2c.h"

#define SC831HAI_PATTERN_ENABLE	0

static void sc831hai_linear_2160p30_master_init(VI_PIPE ViPipe);
static void sc831hai_linear_2160p30_slave_init(VI_PIPE ViPipe);

CVI_U8 sc831hai_i2c_addr = 0x30;        /* I2C Address of SC831HAI */
const CVI_U32 sc831hai_addr_byte = 2;
const CVI_U32 sc831hai_data_byte = 1;

int sc831hai_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunSC831HAI_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC831HAI_AddrInfo[ViPipe].s8I2cAddr);
}

int sc831hai_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunSC831HAI_BusInfo[ViPipe].s8I2cDev);
}

int sc831hai_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunSC831HAI_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC831HAI_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc831hai_addr_byte, sc831hai_data_byte);
}

int sc831hai_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunSC831HAI_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC831HAI_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc831hai_addr_byte, (CVI_U32)data, sc831hai_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void sc831hai_standby(VI_PIPE ViPipe)
{
	sc831hai_write_register(ViPipe, 0x0100, 0x00);
}

void sc831hai_restart(VI_PIPE ViPipe)
{
	sc831hai_write_register(ViPipe, 0x0100, 0x00);
	delay_ms(20);
	sc831hai_write_register(ViPipe, 0x0100, 0x01);
}

void sc831hai_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastSC831HAI[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		sc831hai_write_register(ViPipe,
				g_pastSC831HAI[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastSC831HAI[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

#define SC831HAI_CHIP_ID_HI_ADDR		0x3107
#define SC831HAI_CHIP_ID_LO_ADDR		0x3108
#define SC831HAI_CHIP_ID			0xc170

void sc831hai_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 val = 0;

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		val |= 0x6;
		break;
	case ISP_SNS_FLIP:
		val |= 0x60;
		break;
	case ISP_SNS_MIRROR_FLIP:
		val |= 0x66;
		break;
	default:
		return;
	}

	sc831hai_write_register(ViPipe, 0x3221, val);
}

int sc831hai_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	delay_ms(4);
	if (sc831hai_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = sc831hai_read_register(ViPipe, SC831HAI_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = sc831hai_read_register(ViPipe, SC831HAI_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != SC831HAI_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}


void sc831hai_init(VI_PIPE ViPipe)
{
	CVI_U8 u8ImgMode = g_pastSC831HAI[ViPipe]->u8ImgMode;

	sc831hai_i2c_init(ViPipe);

	if (u8ImgMode == SC831HAI_MODE_2160P30_4L_MASTER)
		sc831hai_linear_2160p30_master_init(ViPipe);
	else if (u8ImgMode == SC831HAI_MODE_2160P30_4L_SLAVE)
		sc831hai_linear_2160p30_slave_init(ViPipe);

	g_pastSC831HAI[ViPipe]->bInit = CVI_TRUE;
}

/* 2160P30 */
static void sc831hai_linear_2160p30_master_init(VI_PIPE ViPipe)
{
	sc831hai_write_register(ViPipe, 0x0103, 0x01);
	sc831hai_write_register(ViPipe, 0x36e9, 0x80);
	sc831hai_write_register(ViPipe, 0x37f9, 0x80);
	sc831hai_write_register(ViPipe, 0x301f, 0x02);
	sc831hai_write_register(ViPipe, 0x30b8, 0x44);
	sc831hai_write_register(ViPipe, 0x320c, 0x08);
	sc831hai_write_register(ViPipe, 0x320d, 0x34);
	sc831hai_write_register(ViPipe, 0x320e, 0x08);
	sc831hai_write_register(ViPipe, 0x320f, 0xca);

	//master mode
	sc831hai_write_register(ViPipe, 0x3221, 0x66);
	sc831hai_write_register(ViPipe, 0x3222, 0x00);
	sc831hai_write_register(ViPipe, 0x300a, 0x24);
	sc831hai_write_register(ViPipe, 0x3032, 0xa0);

	sc831hai_write_register(ViPipe, 0x3301, 0x0a);
	sc831hai_write_register(ViPipe, 0x3302, 0x10);
	sc831hai_write_register(ViPipe, 0x3303, 0x10);
	sc831hai_write_register(ViPipe, 0x3304, 0x68);
	sc831hai_write_register(ViPipe, 0x3305, 0x00);
	sc831hai_write_register(ViPipe, 0x3306, 0x70);
	sc831hai_write_register(ViPipe, 0x3307, 0x08);
	sc831hai_write_register(ViPipe, 0x3308, 0x14);
	sc831hai_write_register(ViPipe, 0x3309, 0x98);
	sc831hai_write_register(ViPipe, 0x330a, 0x00);
	sc831hai_write_register(ViPipe, 0x330b, 0xf8);
	sc831hai_write_register(ViPipe, 0x330c, 0x10);
	sc831hai_write_register(ViPipe, 0x330d, 0x08);
	sc831hai_write_register(ViPipe, 0x330e, 0x3c);
	sc831hai_write_register(ViPipe, 0x331e, 0x41);
	sc831hai_write_register(ViPipe, 0x331f, 0x71);
	sc831hai_write_register(ViPipe, 0x3333, 0x10);
	sc831hai_write_register(ViPipe, 0x3334, 0x40);
	sc831hai_write_register(ViPipe, 0x334c, 0x10);
	sc831hai_write_register(ViPipe, 0x335d, 0x60);
	sc831hai_write_register(ViPipe, 0x3364, 0x5e);
	sc831hai_write_register(ViPipe, 0x3367, 0x04);
	sc831hai_write_register(ViPipe, 0x338f, 0x80);
	sc831hai_write_register(ViPipe, 0x3390, 0x01);
	sc831hai_write_register(ViPipe, 0x3391, 0x03);
	sc831hai_write_register(ViPipe, 0x3392, 0x07);
	sc831hai_write_register(ViPipe, 0x3393, 0x28);
	sc831hai_write_register(ViPipe, 0x3394, 0x4c);
	sc831hai_write_register(ViPipe, 0x3395, 0x4c);
	sc831hai_write_register(ViPipe, 0x3396, 0x01);
	sc831hai_write_register(ViPipe, 0x3397, 0x03);
	sc831hai_write_register(ViPipe, 0x3398, 0x07);
	sc831hai_write_register(ViPipe, 0x3399, 0x0e);
	sc831hai_write_register(ViPipe, 0x339a, 0x12);
	sc831hai_write_register(ViPipe, 0x339b, 0x4c);
	sc831hai_write_register(ViPipe, 0x339c, 0x4c);
	sc831hai_write_register(ViPipe, 0x33ad, 0x24);
	sc831hai_write_register(ViPipe, 0x33ae, 0x58);
	sc831hai_write_register(ViPipe, 0x33af, 0x88);
	sc831hai_write_register(ViPipe, 0x33b2, 0x50);
	sc831hai_write_register(ViPipe, 0x33b3, 0x20);
	sc831hai_write_register(ViPipe, 0x33f8, 0x00);
	sc831hai_write_register(ViPipe, 0x33f9, 0x88);
	sc831hai_write_register(ViPipe, 0x33fa, 0x00);
	sc831hai_write_register(ViPipe, 0x33fb, 0xa0);
	sc831hai_write_register(ViPipe, 0x33fc, 0x43);
	sc831hai_write_register(ViPipe, 0x33fd, 0x47);
	sc831hai_write_register(ViPipe, 0x349f, 0x03);
	sc831hai_write_register(ViPipe, 0x34a6, 0x43);
	sc831hai_write_register(ViPipe, 0x34a7, 0x47);
	sc831hai_write_register(ViPipe, 0x34a8, 0x20);
	sc831hai_write_register(ViPipe, 0x34a9, 0x20);
	sc831hai_write_register(ViPipe, 0x34aa, 0x01);
	sc831hai_write_register(ViPipe, 0x34ab, 0x10);
	sc831hai_write_register(ViPipe, 0x34ac, 0x01);
	sc831hai_write_register(ViPipe, 0x34ad, 0x28);
	sc831hai_write_register(ViPipe, 0x34f8, 0x43);
	sc831hai_write_register(ViPipe, 0x34f9, 0x08);
	sc831hai_write_register(ViPipe, 0x3632, 0x64);
	sc831hai_write_register(ViPipe, 0x363b, 0x16);
	sc831hai_write_register(ViPipe, 0x363c, 0x0e);
	sc831hai_write_register(ViPipe, 0x363d, 0x8e);
	sc831hai_write_register(ViPipe, 0x363e, 0x6c);
	sc831hai_write_register(ViPipe, 0x3654, 0x00);
	sc831hai_write_register(ViPipe, 0x3674, 0x94);
	sc831hai_write_register(ViPipe, 0x3675, 0x84);
	sc831hai_write_register(ViPipe, 0x3676, 0x68);
	sc831hai_write_register(ViPipe, 0x367c, 0x41);
	sc831hai_write_register(ViPipe, 0x367d, 0x43);
	sc831hai_write_register(ViPipe, 0x3690, 0x35);
	sc831hai_write_register(ViPipe, 0x3691, 0x35);
	sc831hai_write_register(ViPipe, 0x3692, 0x45);
	sc831hai_write_register(ViPipe, 0x3693, 0x40);
	sc831hai_write_register(ViPipe, 0x3694, 0x41);
	sc831hai_write_register(ViPipe, 0x3696, 0x81);
	sc831hai_write_register(ViPipe, 0x3697, 0x80);
	sc831hai_write_register(ViPipe, 0x3698, 0x80);
	sc831hai_write_register(ViPipe, 0x3699, 0x83);
	sc831hai_write_register(ViPipe, 0x369a, 0x81);
	sc831hai_write_register(ViPipe, 0x369b, 0xff);
	sc831hai_write_register(ViPipe, 0x369c, 0xff);
	sc831hai_write_register(ViPipe, 0x369d, 0xff);
	sc831hai_write_register(ViPipe, 0x36a2, 0x40);
	sc831hai_write_register(ViPipe, 0x36a3, 0x41);
	sc831hai_write_register(ViPipe, 0x36a4, 0x43);
	sc831hai_write_register(ViPipe, 0x36a5, 0x47);
	sc831hai_write_register(ViPipe, 0x36a6, 0x4f);
	sc831hai_write_register(ViPipe, 0x36a7, 0x4f);
	sc831hai_write_register(ViPipe, 0x36a8, 0x4f);
	sc831hai_write_register(ViPipe, 0x36d0, 0x15);
	sc831hai_write_register(ViPipe, 0x36ea, 0x09);
	sc831hai_write_register(ViPipe, 0x36eb, 0x04);
	sc831hai_write_register(ViPipe, 0x36ec, 0x43);
	sc831hai_write_register(ViPipe, 0x36ed, 0x3a);
	sc831hai_write_register(ViPipe, 0x370f, 0x01);
	sc831hai_write_register(ViPipe, 0x3724, 0xe5);
	sc831hai_write_register(ViPipe, 0x3725, 0xa8);
	sc831hai_write_register(ViPipe, 0x3727, 0x14);
	sc831hai_write_register(ViPipe, 0x37b0, 0x17);
	sc831hai_write_register(ViPipe, 0x37b1, 0x9b);
	sc831hai_write_register(ViPipe, 0x37b2, 0xfb);
	sc831hai_write_register(ViPipe, 0x37b3, 0x41);
	sc831hai_write_register(ViPipe, 0x37b4, 0x43);
	sc831hai_write_register(ViPipe, 0x37fa, 0x0e);
	sc831hai_write_register(ViPipe, 0x37fb, 0x31);
	sc831hai_write_register(ViPipe, 0x37fc, 0x00);
	sc831hai_write_register(ViPipe, 0x37fd, 0x06);
	sc831hai_write_register(ViPipe, 0x3905, 0x0f);
	sc831hai_write_register(ViPipe, 0x391f, 0x41);
	sc831hai_write_register(ViPipe, 0x3933, 0x80);
	sc831hai_write_register(ViPipe, 0x3934, 0xd3);
	sc831hai_write_register(ViPipe, 0x3937, 0x70);
	sc831hai_write_register(ViPipe, 0x3939, 0x0f);
	sc831hai_write_register(ViPipe, 0x393a, 0xf8);
	sc831hai_write_register(ViPipe, 0x3e00, 0x01);
	sc831hai_write_register(ViPipe, 0x3e01, 0x17);
	sc831hai_write_register(ViPipe, 0x3e02, 0x00);
	sc831hai_write_register(ViPipe, 0x3e16, 0x00);
	sc831hai_write_register(ViPipe, 0x3e17, 0xbc);
	sc831hai_write_register(ViPipe, 0x3e18, 0x00);
	sc831hai_write_register(ViPipe, 0x3e19, 0xbc);
	sc831hai_write_register(ViPipe, 0x4424, 0x02);
	sc831hai_write_register(ViPipe, 0x4509, 0x1a);
	sc831hai_write_register(ViPipe, 0x450d, 0x0b);
	sc831hai_write_register(ViPipe, 0x4800, 0x24);
	sc831hai_write_register(ViPipe, 0x5000, 0x0e);
	sc831hai_write_register(ViPipe, 0x575c, 0x10);
	sc831hai_write_register(ViPipe, 0x575d, 0x08);
	sc831hai_write_register(ViPipe, 0x5780, 0x76);
	sc831hai_write_register(ViPipe, 0x5784, 0x10);
	sc831hai_write_register(ViPipe, 0x5785, 0x08);
	sc831hai_write_register(ViPipe, 0x5787, 0x0a);
	sc831hai_write_register(ViPipe, 0x5788, 0x0a);
	sc831hai_write_register(ViPipe, 0x5789, 0x08);
	sc831hai_write_register(ViPipe, 0x578a, 0x0a);
	sc831hai_write_register(ViPipe, 0x578b, 0x0a);
	sc831hai_write_register(ViPipe, 0x578c, 0x08);
	sc831hai_write_register(ViPipe, 0x578d, 0x40);
	sc831hai_write_register(ViPipe, 0x5790, 0x08);
	sc831hai_write_register(ViPipe, 0x5791, 0x04);
	sc831hai_write_register(ViPipe, 0x5792, 0x04);
	sc831hai_write_register(ViPipe, 0x5793, 0x08);
	sc831hai_write_register(ViPipe, 0x5794, 0x04);
	sc831hai_write_register(ViPipe, 0x5795, 0x04);
	sc831hai_write_register(ViPipe, 0x57a8, 0xd2);
	sc831hai_write_register(ViPipe, 0x57aa, 0x2a);
	sc831hai_write_register(ViPipe, 0x57ab, 0x7f);
	sc831hai_write_register(ViPipe, 0x57ac, 0x00);
	sc831hai_write_register(ViPipe, 0x57ad, 0x00);
	sc831hai_write_register(ViPipe, 0x36e9, 0x24);
	sc831hai_write_register(ViPipe, 0x37f9, 0x27);
#if SC831HAI_PATTERN_ENABLE
	sc831hai_write_register(ViPipe, 0x4501, 0xcc);
#endif
	sc831hai_default_reg_init(ViPipe);
	sc831hai_write_register(ViPipe, 0x0100, 0x01);

	printf("ViPipe:%d,===SC831HAI 2160P 30fps 10bit MASTER Init OK!===\n", ViPipe);
}

static void sc831hai_linear_2160p30_slave_init(VI_PIPE ViPipe)
{
	sc831hai_write_register(ViPipe, 0x0103, 0x01);
	sc831hai_write_register(ViPipe, 0x36e9, 0x80);
	sc831hai_write_register(ViPipe, 0x37f9, 0x80);
	sc831hai_write_register(ViPipe, 0x301f, 0x02);
	sc831hai_write_register(ViPipe, 0x30b8, 0x44);
	sc831hai_write_register(ViPipe, 0x320c, 0x08);
	sc831hai_write_register(ViPipe, 0x320d, 0x34);
	sc831hai_write_register(ViPipe, 0x320e, 0x08);
	sc831hai_write_register(ViPipe, 0x320f, 0xca);

	//slave efsync
	sc831hai_write_register(ViPipe, 0x3221, 0x66);
	sc831hai_write_register(ViPipe, 0x3222, 0x01);
	sc831hai_write_register(ViPipe, 0x3224, 0xc2);//fsync trig:0xd2
	sc831hai_write_register(ViPipe, 0x3230, 0x00);
	sc831hai_write_register(ViPipe, 0x3231, 0x04);

	sc831hai_write_register(ViPipe, 0x3301, 0x0a);
	sc831hai_write_register(ViPipe, 0x3302, 0x10);
	sc831hai_write_register(ViPipe, 0x3303, 0x10);
	sc831hai_write_register(ViPipe, 0x3304, 0x68);
	sc831hai_write_register(ViPipe, 0x3305, 0x00);
	sc831hai_write_register(ViPipe, 0x3306, 0x70);
	sc831hai_write_register(ViPipe, 0x3307, 0x08);
	sc831hai_write_register(ViPipe, 0x3308, 0x14);
	sc831hai_write_register(ViPipe, 0x3309, 0x98);
	sc831hai_write_register(ViPipe, 0x330a, 0x00);
	sc831hai_write_register(ViPipe, 0x330b, 0xf8);
	sc831hai_write_register(ViPipe, 0x330c, 0x10);
	sc831hai_write_register(ViPipe, 0x330d, 0x08);
	sc831hai_write_register(ViPipe, 0x330e, 0x3c);
	sc831hai_write_register(ViPipe, 0x331e, 0x41);
	sc831hai_write_register(ViPipe, 0x331f, 0x71);
	sc831hai_write_register(ViPipe, 0x3333, 0x10);
	sc831hai_write_register(ViPipe, 0x3334, 0x40);
	sc831hai_write_register(ViPipe, 0x334c, 0x10);
	sc831hai_write_register(ViPipe, 0x335d, 0x60);
	sc831hai_write_register(ViPipe, 0x3364, 0x5e);
	sc831hai_write_register(ViPipe, 0x3367, 0x04);
	sc831hai_write_register(ViPipe, 0x338f, 0x80);
	sc831hai_write_register(ViPipe, 0x3390, 0x01);
	sc831hai_write_register(ViPipe, 0x3391, 0x03);
	sc831hai_write_register(ViPipe, 0x3392, 0x07);
	sc831hai_write_register(ViPipe, 0x3393, 0x28);
	sc831hai_write_register(ViPipe, 0x3394, 0x4c);
	sc831hai_write_register(ViPipe, 0x3395, 0x4c);
	sc831hai_write_register(ViPipe, 0x3396, 0x01);
	sc831hai_write_register(ViPipe, 0x3397, 0x03);
	sc831hai_write_register(ViPipe, 0x3398, 0x07);
	sc831hai_write_register(ViPipe, 0x3399, 0x0e);
	sc831hai_write_register(ViPipe, 0x339a, 0x12);
	sc831hai_write_register(ViPipe, 0x339b, 0x4c);
	sc831hai_write_register(ViPipe, 0x339c, 0x4c);
	sc831hai_write_register(ViPipe, 0x33ad, 0x24);
	sc831hai_write_register(ViPipe, 0x33ae, 0x58);
	sc831hai_write_register(ViPipe, 0x33af, 0x88);
	sc831hai_write_register(ViPipe, 0x33b2, 0x50);
	sc831hai_write_register(ViPipe, 0x33b3, 0x20);
	sc831hai_write_register(ViPipe, 0x33f8, 0x00);
	sc831hai_write_register(ViPipe, 0x33f9, 0x88);
	sc831hai_write_register(ViPipe, 0x33fa, 0x00);
	sc831hai_write_register(ViPipe, 0x33fb, 0xa0);
	sc831hai_write_register(ViPipe, 0x33fc, 0x43);
	sc831hai_write_register(ViPipe, 0x33fd, 0x47);
	sc831hai_write_register(ViPipe, 0x349f, 0x03);
	sc831hai_write_register(ViPipe, 0x34a6, 0x43);
	sc831hai_write_register(ViPipe, 0x34a7, 0x47);
	sc831hai_write_register(ViPipe, 0x34a8, 0x20);
	sc831hai_write_register(ViPipe, 0x34a9, 0x20);
	sc831hai_write_register(ViPipe, 0x34aa, 0x01);
	sc831hai_write_register(ViPipe, 0x34ab, 0x10);
	sc831hai_write_register(ViPipe, 0x34ac, 0x01);
	sc831hai_write_register(ViPipe, 0x34ad, 0x28);
	sc831hai_write_register(ViPipe, 0x34f8, 0x43);
	sc831hai_write_register(ViPipe, 0x34f9, 0x08);
	sc831hai_write_register(ViPipe, 0x3632, 0x64);
	sc831hai_write_register(ViPipe, 0x363b, 0x16);
	sc831hai_write_register(ViPipe, 0x363c, 0x0e);
	sc831hai_write_register(ViPipe, 0x363d, 0x8e);
	sc831hai_write_register(ViPipe, 0x363e, 0x6c);
	sc831hai_write_register(ViPipe, 0x3654, 0x00);
	sc831hai_write_register(ViPipe, 0x3674, 0x94);
	sc831hai_write_register(ViPipe, 0x3675, 0x84);
	sc831hai_write_register(ViPipe, 0x3676, 0x68);
	sc831hai_write_register(ViPipe, 0x367c, 0x41);
	sc831hai_write_register(ViPipe, 0x367d, 0x43);
	sc831hai_write_register(ViPipe, 0x3690, 0x35);
	sc831hai_write_register(ViPipe, 0x3691, 0x35);
	sc831hai_write_register(ViPipe, 0x3692, 0x45);
	sc831hai_write_register(ViPipe, 0x3693, 0x40);
	sc831hai_write_register(ViPipe, 0x3694, 0x41);
	sc831hai_write_register(ViPipe, 0x3696, 0x81);
	sc831hai_write_register(ViPipe, 0x3697, 0x80);
	sc831hai_write_register(ViPipe, 0x3698, 0x80);
	sc831hai_write_register(ViPipe, 0x3699, 0x83);
	sc831hai_write_register(ViPipe, 0x369a, 0x81);
	sc831hai_write_register(ViPipe, 0x369b, 0xff);
	sc831hai_write_register(ViPipe, 0x369c, 0xff);
	sc831hai_write_register(ViPipe, 0x369d, 0xff);
	sc831hai_write_register(ViPipe, 0x36a2, 0x40);
	sc831hai_write_register(ViPipe, 0x36a3, 0x41);
	sc831hai_write_register(ViPipe, 0x36a4, 0x43);
	sc831hai_write_register(ViPipe, 0x36a5, 0x47);
	sc831hai_write_register(ViPipe, 0x36a6, 0x4f);
	sc831hai_write_register(ViPipe, 0x36a7, 0x4f);
	sc831hai_write_register(ViPipe, 0x36a8, 0x4f);
	sc831hai_write_register(ViPipe, 0x36d0, 0x15);
	sc831hai_write_register(ViPipe, 0x36ea, 0x09);
	sc831hai_write_register(ViPipe, 0x36eb, 0x04);
	sc831hai_write_register(ViPipe, 0x36ec, 0x43);
	sc831hai_write_register(ViPipe, 0x36ed, 0x3a);
	sc831hai_write_register(ViPipe, 0x370f, 0x01);
	sc831hai_write_register(ViPipe, 0x3724, 0xe5);
	sc831hai_write_register(ViPipe, 0x3725, 0xa8);
	sc831hai_write_register(ViPipe, 0x3727, 0x14);
	sc831hai_write_register(ViPipe, 0x37b0, 0x17);
	sc831hai_write_register(ViPipe, 0x37b1, 0x9b);
	sc831hai_write_register(ViPipe, 0x37b2, 0xfb);
	sc831hai_write_register(ViPipe, 0x37b3, 0x41);
	sc831hai_write_register(ViPipe, 0x37b4, 0x43);
	sc831hai_write_register(ViPipe, 0x37fa, 0x0e);
	sc831hai_write_register(ViPipe, 0x37fb, 0x31);
	sc831hai_write_register(ViPipe, 0x37fc, 0x00);
	sc831hai_write_register(ViPipe, 0x37fd, 0x06);
	sc831hai_write_register(ViPipe, 0x3905, 0x0f);
	sc831hai_write_register(ViPipe, 0x391f, 0x41);
	sc831hai_write_register(ViPipe, 0x3933, 0x80);
	sc831hai_write_register(ViPipe, 0x3934, 0xd3);
	sc831hai_write_register(ViPipe, 0x3937, 0x70);
	sc831hai_write_register(ViPipe, 0x3939, 0x0f);
	sc831hai_write_register(ViPipe, 0x393a, 0xf8);
	sc831hai_write_register(ViPipe, 0x3e00, 0x01);
	sc831hai_write_register(ViPipe, 0x3e01, 0x17);
	sc831hai_write_register(ViPipe, 0x3e02, 0x00);
	sc831hai_write_register(ViPipe, 0x3e16, 0x00);
	sc831hai_write_register(ViPipe, 0x3e17, 0xbc);
	sc831hai_write_register(ViPipe, 0x3e18, 0x00);
	sc831hai_write_register(ViPipe, 0x3e19, 0xbc);
	sc831hai_write_register(ViPipe, 0x4424, 0x02);
	sc831hai_write_register(ViPipe, 0x4509, 0x1a);
	sc831hai_write_register(ViPipe, 0x450d, 0x0b);
	sc831hai_write_register(ViPipe, 0x4800, 0x24);
	sc831hai_write_register(ViPipe, 0x5000, 0x0e);
	sc831hai_write_register(ViPipe, 0x575c, 0x10);
	sc831hai_write_register(ViPipe, 0x575d, 0x08);
	sc831hai_write_register(ViPipe, 0x5780, 0x76);
	sc831hai_write_register(ViPipe, 0x5784, 0x10);
	sc831hai_write_register(ViPipe, 0x5785, 0x08);
	sc831hai_write_register(ViPipe, 0x5787, 0x0a);
	sc831hai_write_register(ViPipe, 0x5788, 0x0a);
	sc831hai_write_register(ViPipe, 0x5789, 0x08);
	sc831hai_write_register(ViPipe, 0x578a, 0x0a);
	sc831hai_write_register(ViPipe, 0x578b, 0x0a);
	sc831hai_write_register(ViPipe, 0x578c, 0x08);
	sc831hai_write_register(ViPipe, 0x578d, 0x40);
	sc831hai_write_register(ViPipe, 0x5790, 0x08);
	sc831hai_write_register(ViPipe, 0x5791, 0x04);
	sc831hai_write_register(ViPipe, 0x5792, 0x04);
	sc831hai_write_register(ViPipe, 0x5793, 0x08);
	sc831hai_write_register(ViPipe, 0x5794, 0x04);
	sc831hai_write_register(ViPipe, 0x5795, 0x04);
	sc831hai_write_register(ViPipe, 0x57a8, 0xd2);
	sc831hai_write_register(ViPipe, 0x57aa, 0x2a);
	sc831hai_write_register(ViPipe, 0x57ab, 0x7f);
	sc831hai_write_register(ViPipe, 0x57ac, 0x00);
	sc831hai_write_register(ViPipe, 0x57ad, 0x00);
	sc831hai_write_register(ViPipe, 0x36e9, 0x24);
	sc831hai_write_register(ViPipe, 0x37f9, 0x27);
#if SC831HAI_PATTERN_ENABLE
	sc831hai_write_register(ViPipe, 0x4501, 0xcc);
#endif
	sc831hai_default_reg_init(ViPipe);
	sc831hai_write_register(ViPipe, 0x0100, 0x01);

	printf("ViPipe:%d,===SC831HAI 2160P 30fps 10bit SLAVE Init OK!===\n", ViPipe);
}

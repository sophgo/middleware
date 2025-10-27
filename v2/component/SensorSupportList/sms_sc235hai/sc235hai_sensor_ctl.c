#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "sc235hai_cmos_ex.h"
#include "sensor_i2c.h"

static void sc235hai_1l_linear_1080p15_init(VI_PIPE ViPipe);
static void sc235hai_1l_slave_linear_1080p15_init(VI_PIPE ViPipe);
static void sc235hai_2l_linear_1080p15_init(VI_PIPE ViPipe);
static void sc235hai_2l_slave_linear_1080p15_init(VI_PIPE ViPipe);

const CVI_U8 sc235hai_i2c_addr   = 0x32;        /* I2C Address of SC235HAI */
const CVI_U32 sc235hai_addr_byte = 2;
const CVI_U32 sc235hai_data_byte = 1;

int sc235hai_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunSC235HAI_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC235HAI_AddrInfo[ViPipe].s8I2cAddr);
}

int sc235hai_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunSC235HAI_BusInfo[ViPipe].s8I2cDev);
}

int sc235hai_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunSC235HAI_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC235HAI_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc235hai_addr_byte, sc235hai_data_byte);
}

int sc235hai_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunSC235HAI_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC235HAI_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc235hai_addr_byte, (CVI_U32)data, sc235hai_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void sc235hai_standby(VI_PIPE ViPipe)
{
	sc235hai_write_register(ViPipe, 0x0100, 0x00);
}

void sc235hai_restart(VI_PIPE ViPipe)
{
	sc235hai_write_register(ViPipe, 0x0100, 0x00);
	delay_ms(20);
	sc235hai_write_register(ViPipe, 0x0100, 0x01);
}

void sc235hai_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastSC235HAI[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		sc235hai_write_register(ViPipe,
				g_pastSC235HAI[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastSC235HAI[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

#define sc235hai_CHIP_ID_HI_ADDR		0x3107
#define sc235hai_CHIP_ID_LO_ADDR		0x3108
#define sc235hai_CHIP_ID				0xcb6a

void sc235hai_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
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

	sc235hai_write_register(ViPipe, 0x3221, val);
}

int sc235hai_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	delay_ms(4);
	if (sc235hai_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = sc235hai_read_register(ViPipe, sc235hai_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = sc235hai_read_register(ViPipe, sc235hai_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != sc235hai_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}


void sc235hai_init(VI_PIPE ViPipe)
{
	WDR_MODE_E       enWDRMode;
	CVI_U8            u8ImgMode;

	enWDRMode   = g_pastSC235HAI[ViPipe]->enWDRMode;
	u8ImgMode   = g_pastSC235HAI[ViPipe]->u8ImgMode;

	sc235hai_i2c_init(ViPipe);

	/* When sensor first init, config all registers */
		if (enWDRMode == WDR_MODE_NONE) {
			if (u8ImgMode == SC235HAI_MODE_1080P15_1L) {
				sc235hai_1l_linear_1080p15_init(ViPipe);

			} else if (u8ImgMode == SC235HAI_MODE_1080P15_2L) {
				sc235hai_2l_linear_1080p15_init(ViPipe);
			} else if (u8ImgMode == SC235HAI_MODE_1080P15_2L_SLAVE) {
				sc235hai_2l_slave_linear_1080p15_init(ViPipe);
			} else if (u8ImgMode == SC235HAI_MODE_1080P15_1L_SLAVE) {
				sc235hai_1l_slave_linear_1080p15_init(ViPipe);
			}
		}else if (enWDRMode == WDR_MODE_2To1_LINE) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "not support wdr mode\n");
		}
}

/* 1080P15*/
static void sc235hai_1l_linear_1080p15_init(VI_PIPE ViPipe)
{
	sc235hai_write_register(ViPipe, 0x0103, 0x01);
	sc235hai_write_register(ViPipe, 0x36e9, 0x80);
	sc235hai_write_register(ViPipe, 0x37f9, 0x80);
	sc235hai_write_register(ViPipe, 0x3018, 0x1a);
	sc235hai_write_register(ViPipe, 0x3019, 0x0e);
	sc235hai_write_register(ViPipe, 0x301f, 0x49);
	sc235hai_write_register(ViPipe, 0x3058, 0x21);
	sc235hai_write_register(ViPipe, 0x3059, 0x53);
	sc235hai_write_register(ViPipe, 0x305a, 0x40);
	sc235hai_write_register(ViPipe, 0x320c, 0x08);
	sc235hai_write_register(ViPipe, 0x320d, 0xc0);
	sc235hai_write_register(ViPipe, 0x3250, 0x00);
	sc235hai_write_register(ViPipe, 0x3301, 0x16);
	sc235hai_write_register(ViPipe, 0x3302, 0x20);
	sc235hai_write_register(ViPipe, 0x3304, 0x50);
	sc235hai_write_register(ViPipe, 0x3305, 0x00);
	sc235hai_write_register(ViPipe, 0x3306, 0x58);
	sc235hai_write_register(ViPipe, 0x3309, 0x90);
	sc235hai_write_register(ViPipe, 0x330b, 0xc8);
	sc235hai_write_register(ViPipe, 0x330d, 0x08);
	sc235hai_write_register(ViPipe, 0x331c, 0x04);
	sc235hai_write_register(ViPipe, 0x331e, 0x41);
	sc235hai_write_register(ViPipe, 0x331f, 0x81);
	sc235hai_write_register(ViPipe, 0x3323, 0x06);
	sc235hai_write_register(ViPipe, 0x3333, 0x10);
	sc235hai_write_register(ViPipe, 0x3334, 0x40);
	sc235hai_write_register(ViPipe, 0x3364, 0x5e);
	sc235hai_write_register(ViPipe, 0x336c, 0x8e);
	sc235hai_write_register(ViPipe, 0x337f, 0x13);
	sc235hai_write_register(ViPipe, 0x338f, 0x80);
	sc235hai_write_register(ViPipe, 0x3390, 0x08);
	sc235hai_write_register(ViPipe, 0x3391, 0x18);
	sc235hai_write_register(ViPipe, 0x3392, 0xb8);
	sc235hai_write_register(ViPipe, 0x3393, 0x16);
	sc235hai_write_register(ViPipe, 0x3394, 0x14);
	sc235hai_write_register(ViPipe, 0x3395, 0x10);
	sc235hai_write_register(ViPipe, 0x3396, 0x88);
	sc235hai_write_register(ViPipe, 0x3397, 0x98);
	sc235hai_write_register(ViPipe, 0x3398, 0xf8);
	sc235hai_write_register(ViPipe, 0x3399, 0x16);
	sc235hai_write_register(ViPipe, 0x339a, 0x16);
	sc235hai_write_register(ViPipe, 0x339b, 0x16);
	sc235hai_write_register(ViPipe, 0x339c, 0x16);
	sc235hai_write_register(ViPipe, 0x33ae, 0x40);
	sc235hai_write_register(ViPipe, 0x33af, 0x80);
	sc235hai_write_register(ViPipe, 0x33b2, 0x50);
	sc235hai_write_register(ViPipe, 0x33b3, 0x14);
	sc235hai_write_register(ViPipe, 0x33f8, 0x00);
	sc235hai_write_register(ViPipe, 0x33f9, 0x58);
	sc235hai_write_register(ViPipe, 0x33fa, 0x00);
	sc235hai_write_register(ViPipe, 0x33fb, 0x58);
	sc235hai_write_register(ViPipe, 0x33fc, 0x48);
	sc235hai_write_register(ViPipe, 0x33fd, 0x78);
	sc235hai_write_register(ViPipe, 0x349f, 0x03);
	sc235hai_write_register(ViPipe, 0x34a6, 0x40);
	sc235hai_write_register(ViPipe, 0x34a7, 0x58);
	sc235hai_write_register(ViPipe, 0x34a8, 0x10);
	sc235hai_write_register(ViPipe, 0x34a9, 0x10);
	sc235hai_write_register(ViPipe, 0x34f8, 0x78);
	sc235hai_write_register(ViPipe, 0x34f9, 0x10);
	sc235hai_write_register(ViPipe, 0x3619, 0x20);
	sc235hai_write_register(ViPipe, 0x361a, 0x90);
	sc235hai_write_register(ViPipe, 0x3633, 0x44);
	sc235hai_write_register(ViPipe, 0x3637, 0x5c);
	sc235hai_write_register(ViPipe, 0x363c, 0xc0);
	sc235hai_write_register(ViPipe, 0x363d, 0x02);
	sc235hai_write_register(ViPipe, 0x3660, 0x80);
	sc235hai_write_register(ViPipe, 0x3661, 0x81);
	sc235hai_write_register(ViPipe, 0x3662, 0x8f);
	sc235hai_write_register(ViPipe, 0x3663, 0x81);
	sc235hai_write_register(ViPipe, 0x3664, 0x81);
	sc235hai_write_register(ViPipe, 0x3665, 0x82);
	sc235hai_write_register(ViPipe, 0x3666, 0x8f);
	sc235hai_write_register(ViPipe, 0x3667, 0x08);
	sc235hai_write_register(ViPipe, 0x3668, 0x80);
	sc235hai_write_register(ViPipe, 0x3669, 0x88);
	sc235hai_write_register(ViPipe, 0x366a, 0x98);
	sc235hai_write_register(ViPipe, 0x366b, 0xb8);
	sc235hai_write_register(ViPipe, 0x366c, 0xf8);
	sc235hai_write_register(ViPipe, 0x3670, 0xb2);
	sc235hai_write_register(ViPipe, 0x3671, 0xa2);
	sc235hai_write_register(ViPipe, 0x3672, 0x88);
	sc235hai_write_register(ViPipe, 0x3680, 0x33);
	sc235hai_write_register(ViPipe, 0x3681, 0x33);
	sc235hai_write_register(ViPipe, 0x3682, 0x43);
	sc235hai_write_register(ViPipe, 0x36c0, 0x80);
	sc235hai_write_register(ViPipe, 0x36c1, 0x88);
	sc235hai_write_register(ViPipe, 0x36c8, 0x88);
	sc235hai_write_register(ViPipe, 0x36c9, 0xb8);
	sc235hai_write_register(ViPipe, 0x36ea, 0x0e);
	sc235hai_write_register(ViPipe, 0x36eb, 0x1c);
	sc235hai_write_register(ViPipe, 0x36ec, 0x4c);
	sc235hai_write_register(ViPipe, 0x36ed, 0x14);
	sc235hai_write_register(ViPipe, 0x3718, 0x04);
	sc235hai_write_register(ViPipe, 0x3722, 0x8b);
	sc235hai_write_register(ViPipe, 0x3724, 0xd1);
	sc235hai_write_register(ViPipe, 0x3741, 0x08);
	sc235hai_write_register(ViPipe, 0x3770, 0x17);
	sc235hai_write_register(ViPipe, 0x3771, 0x9b);
	sc235hai_write_register(ViPipe, 0x3772, 0x9b);
	sc235hai_write_register(ViPipe, 0x37c0, 0x88);
	sc235hai_write_register(ViPipe, 0x37c1, 0xb8);
	sc235hai_write_register(ViPipe, 0x37fa, 0x0e);
	sc235hai_write_register(ViPipe, 0x37fc, 0x11);
	sc235hai_write_register(ViPipe, 0x37fd, 0x14);
	sc235hai_write_register(ViPipe, 0x3902, 0xc0);
	sc235hai_write_register(ViPipe, 0x3903, 0x40);
	sc235hai_write_register(ViPipe, 0x3909, 0x00);
	sc235hai_write_register(ViPipe, 0x391f, 0x41);
	sc235hai_write_register(ViPipe, 0x3926, 0xe0);
	sc235hai_write_register(ViPipe, 0x3933, 0x80);
	sc235hai_write_register(ViPipe, 0x3934, 0x02);
	sc235hai_write_register(ViPipe, 0x3937, 0x6f);
	sc235hai_write_register(ViPipe, 0x3e00, 0x00);
	sc235hai_write_register(ViPipe, 0x3e01, 0x45);
	sc235hai_write_register(ViPipe, 0x3e02, 0xa0);
	sc235hai_write_register(ViPipe, 0x3e08, 0x00);
	sc235hai_write_register(ViPipe, 0x4509, 0x20);
	sc235hai_write_register(ViPipe, 0x450d, 0x07);
	sc235hai_write_register(ViPipe, 0x4837, 0x34);
	sc235hai_write_register(ViPipe, 0x5780, 0x76);
	sc235hai_write_register(ViPipe, 0x5784, 0x10);
	sc235hai_write_register(ViPipe, 0x5787, 0x0a);
	sc235hai_write_register(ViPipe, 0x5788, 0x0a);
	sc235hai_write_register(ViPipe, 0x5789, 0x08);
	sc235hai_write_register(ViPipe, 0x578a, 0x0a);
	sc235hai_write_register(ViPipe, 0x578b, 0x0a);
	sc235hai_write_register(ViPipe, 0x578c, 0x08);
	sc235hai_write_register(ViPipe, 0x578d, 0x40);
	sc235hai_write_register(ViPipe, 0x5792, 0x04);
	sc235hai_write_register(ViPipe, 0x5795, 0x04);
	sc235hai_write_register(ViPipe, 0x57ac, 0x00);
	sc235hai_write_register(ViPipe, 0x57ad, 0x00);
	sc235hai_write_register(ViPipe, 0x36e9, 0x24);
	sc235hai_write_register(ViPipe, 0x37f9, 0x24);

	sc235hai_default_reg_init(ViPipe);
	sc235hai_write_register(ViPipe, 0x0100, 0x01);
	delay_ms(50);

	printf("ViPipe:%d,===sc235hai 1080P 15fps 10bit 1LINE Init OK!===\n", ViPipe);
}

/* 1080P15*/
static void sc235hai_1l_slave_linear_1080p15_init(VI_PIPE ViPipe)
{
	sc235hai_write_register(ViPipe, 0x0103, 0x01);
	sc235hai_write_register(ViPipe, 0x36e9, 0x80);
	sc235hai_write_register(ViPipe, 0x37f9, 0x80);
	sc235hai_write_register(ViPipe, 0x3018, 0x1a);
	sc235hai_write_register(ViPipe, 0x3019, 0x0e);
	sc235hai_write_register(ViPipe, 0x301f, 0x49);
	sc235hai_write_register(ViPipe, 0x3058, 0x21);
	sc235hai_write_register(ViPipe, 0x3059, 0x53);
	sc235hai_write_register(ViPipe, 0x305a, 0x40);
	sc235hai_write_register(ViPipe, 0x320c, 0x08);
	sc235hai_write_register(ViPipe, 0x320d, 0xc0);
	sc235hai_write_register(ViPipe, 0x3222, 0x01); //slave mode sel (master:0 slave:1)
	sc235hai_write_register(ViPipe, 0x3224, 0xc2); //slave mode trigger sel (bit[4] FSYNC:1 EFSYNC:0)
	sc235hai_write_register(ViPipe, 0x3250, 0x00);
	sc235hai_write_register(ViPipe, 0x3301, 0x16);
	sc235hai_write_register(ViPipe, 0x3302, 0x20);
	sc235hai_write_register(ViPipe, 0x3304, 0x50);
	sc235hai_write_register(ViPipe, 0x3305, 0x00);
	sc235hai_write_register(ViPipe, 0x3306, 0x58);
	sc235hai_write_register(ViPipe, 0x3309, 0x90);
	sc235hai_write_register(ViPipe, 0x330b, 0xc8);
	sc235hai_write_register(ViPipe, 0x330d, 0x08);
	sc235hai_write_register(ViPipe, 0x331c, 0x04);
	sc235hai_write_register(ViPipe, 0x331e, 0x41);
	sc235hai_write_register(ViPipe, 0x331f, 0x81);
	sc235hai_write_register(ViPipe, 0x3323, 0x06);
	sc235hai_write_register(ViPipe, 0x3333, 0x10);
	sc235hai_write_register(ViPipe, 0x3334, 0x40);
	sc235hai_write_register(ViPipe, 0x3364, 0x5e);
	sc235hai_write_register(ViPipe, 0x336c, 0x8e);
	sc235hai_write_register(ViPipe, 0x337f, 0x13);
	sc235hai_write_register(ViPipe, 0x338f, 0x80);
	sc235hai_write_register(ViPipe, 0x3390, 0x08);
	sc235hai_write_register(ViPipe, 0x3391, 0x18);
	sc235hai_write_register(ViPipe, 0x3392, 0xb8);
	sc235hai_write_register(ViPipe, 0x3393, 0x16);
	sc235hai_write_register(ViPipe, 0x3394, 0x14);
	sc235hai_write_register(ViPipe, 0x3395, 0x10);
	sc235hai_write_register(ViPipe, 0x3396, 0x88);
	sc235hai_write_register(ViPipe, 0x3397, 0x98);
	sc235hai_write_register(ViPipe, 0x3398, 0xf8);
	sc235hai_write_register(ViPipe, 0x3399, 0x16);
	sc235hai_write_register(ViPipe, 0x339a, 0x16);
	sc235hai_write_register(ViPipe, 0x339b, 0x16);
	sc235hai_write_register(ViPipe, 0x339c, 0x16);
	sc235hai_write_register(ViPipe, 0x33ae, 0x40);
	sc235hai_write_register(ViPipe, 0x33af, 0x80);
	sc235hai_write_register(ViPipe, 0x33b2, 0x50);
	sc235hai_write_register(ViPipe, 0x33b3, 0x14);
	sc235hai_write_register(ViPipe, 0x33f8, 0x00);
	sc235hai_write_register(ViPipe, 0x33f9, 0x58);
	sc235hai_write_register(ViPipe, 0x33fa, 0x00);
	sc235hai_write_register(ViPipe, 0x33fb, 0x58);
	sc235hai_write_register(ViPipe, 0x33fc, 0x48);
	sc235hai_write_register(ViPipe, 0x33fd, 0x78);
	sc235hai_write_register(ViPipe, 0x349f, 0x03);
	sc235hai_write_register(ViPipe, 0x34a6, 0x40);
	sc235hai_write_register(ViPipe, 0x34a7, 0x58);
	sc235hai_write_register(ViPipe, 0x34a8, 0x10);
	sc235hai_write_register(ViPipe, 0x34a9, 0x10);
	sc235hai_write_register(ViPipe, 0x34f8, 0x78);
	sc235hai_write_register(ViPipe, 0x34f9, 0x10);
	sc235hai_write_register(ViPipe, 0x3619, 0x20);
	sc235hai_write_register(ViPipe, 0x361a, 0x90);
	sc235hai_write_register(ViPipe, 0x3633, 0x44);
	sc235hai_write_register(ViPipe, 0x3637, 0x5c);
	sc235hai_write_register(ViPipe, 0x363c, 0xc0);
	sc235hai_write_register(ViPipe, 0x363d, 0x02);
	sc235hai_write_register(ViPipe, 0x3660, 0x80);
	sc235hai_write_register(ViPipe, 0x3661, 0x81);
	sc235hai_write_register(ViPipe, 0x3662, 0x8f);
	sc235hai_write_register(ViPipe, 0x3663, 0x81);
	sc235hai_write_register(ViPipe, 0x3664, 0x81);
	sc235hai_write_register(ViPipe, 0x3665, 0x82);
	sc235hai_write_register(ViPipe, 0x3666, 0x8f);
	sc235hai_write_register(ViPipe, 0x3667, 0x08);
	sc235hai_write_register(ViPipe, 0x3668, 0x80);
	sc235hai_write_register(ViPipe, 0x3669, 0x88);
	sc235hai_write_register(ViPipe, 0x366a, 0x98);
	sc235hai_write_register(ViPipe, 0x366b, 0xb8);
	sc235hai_write_register(ViPipe, 0x366c, 0xf8);
	sc235hai_write_register(ViPipe, 0x3670, 0xb2);
	sc235hai_write_register(ViPipe, 0x3671, 0xa2);
	sc235hai_write_register(ViPipe, 0x3672, 0x88);
	sc235hai_write_register(ViPipe, 0x3680, 0x33);
	sc235hai_write_register(ViPipe, 0x3681, 0x33);
	sc235hai_write_register(ViPipe, 0x3682, 0x43);
	sc235hai_write_register(ViPipe, 0x36c0, 0x80);
	sc235hai_write_register(ViPipe, 0x36c1, 0x88);
	sc235hai_write_register(ViPipe, 0x36c8, 0x88);
	sc235hai_write_register(ViPipe, 0x36c9, 0xb8);
	sc235hai_write_register(ViPipe, 0x36ea, 0x0e);
	sc235hai_write_register(ViPipe, 0x36eb, 0x1c);
	sc235hai_write_register(ViPipe, 0x36ec, 0x4c);
	sc235hai_write_register(ViPipe, 0x36ed, 0x14);
	sc235hai_write_register(ViPipe, 0x3718, 0x04);
	sc235hai_write_register(ViPipe, 0x3722, 0x8b);
	sc235hai_write_register(ViPipe, 0x3724, 0xd1);
	sc235hai_write_register(ViPipe, 0x3741, 0x08);
	sc235hai_write_register(ViPipe, 0x3770, 0x17);
	sc235hai_write_register(ViPipe, 0x3771, 0x9b);
	sc235hai_write_register(ViPipe, 0x3772, 0x9b);
	sc235hai_write_register(ViPipe, 0x37c0, 0x88);
	sc235hai_write_register(ViPipe, 0x37c1, 0xb8);
	sc235hai_write_register(ViPipe, 0x37fa, 0x0e);
	sc235hai_write_register(ViPipe, 0x37fc, 0x11);
	sc235hai_write_register(ViPipe, 0x37fd, 0x14);
	sc235hai_write_register(ViPipe, 0x3902, 0xc0);
	sc235hai_write_register(ViPipe, 0x3903, 0x40);
	sc235hai_write_register(ViPipe, 0x3909, 0x00);
	sc235hai_write_register(ViPipe, 0x391f, 0x41);
	sc235hai_write_register(ViPipe, 0x3926, 0xe0);
	sc235hai_write_register(ViPipe, 0x3933, 0x80);
	sc235hai_write_register(ViPipe, 0x3934, 0x02);
	sc235hai_write_register(ViPipe, 0x3937, 0x6f);
	sc235hai_write_register(ViPipe, 0x3e00, 0x00);
	sc235hai_write_register(ViPipe, 0x3e01, 0x45);
	sc235hai_write_register(ViPipe, 0x3e02, 0xa0);
	sc235hai_write_register(ViPipe, 0x3e08, 0x00);
	sc235hai_write_register(ViPipe, 0x4509, 0x20);
	sc235hai_write_register(ViPipe, 0x450d, 0x07);
	sc235hai_write_register(ViPipe, 0x4837, 0x34);
	sc235hai_write_register(ViPipe, 0x5780, 0x76);
	sc235hai_write_register(ViPipe, 0x5784, 0x10);
	sc235hai_write_register(ViPipe, 0x5787, 0x0a);
	sc235hai_write_register(ViPipe, 0x5788, 0x0a);
	sc235hai_write_register(ViPipe, 0x5789, 0x08);
	sc235hai_write_register(ViPipe, 0x578a, 0x0a);
	sc235hai_write_register(ViPipe, 0x578b, 0x0a);
	sc235hai_write_register(ViPipe, 0x578c, 0x08);
	sc235hai_write_register(ViPipe, 0x578d, 0x40);
	sc235hai_write_register(ViPipe, 0x5792, 0x04);
	sc235hai_write_register(ViPipe, 0x5795, 0x04);
	sc235hai_write_register(ViPipe, 0x57ac, 0x00);
	sc235hai_write_register(ViPipe, 0x57ad, 0x00);
	sc235hai_write_register(ViPipe, 0x36e9, 0x24);
	sc235hai_write_register(ViPipe, 0x37f9, 0x24);


	sc235hai_default_reg_init(ViPipe);
	sc235hai_write_register(ViPipe, 0x0100, 0x01);
	delay_ms(50);

	printf("ViPipe:%d,===sc235hai 1080P 15fps 10bit 1LINE SLAVE Init OK!===\n", ViPipe);
}

/* 1080P15*/
static void sc235hai_2l_linear_1080p15_init(VI_PIPE ViPipe)
{

	sc235hai_write_register(ViPipe, 0x0103, 0x01);
	sc235hai_write_register(ViPipe, 0x36e9, 0x80);
	sc235hai_write_register(ViPipe, 0x37f9, 0x80);
	sc235hai_write_register(ViPipe, 0x301f, 0x05);
	sc235hai_write_register(ViPipe, 0x3058, 0x21);
	sc235hai_write_register(ViPipe, 0x3059, 0x53);
	sc235hai_write_register(ViPipe, 0x305a, 0x40);
	sc235hai_write_register(ViPipe, 0x320c, 0x08);//HTS=2200
	sc235hai_write_register(ViPipe, 0x320d, 0x98);
	//sc235hai_write_register(ViPipe, 0x320e, 0x04);//VTS=1125  30FPS
	//sc235hai_write_register(ViPipe, 0x320f, 0x65);
	sc235hai_write_register(ViPipe, 0x320e, 0x08);//VTS=2250  15FPS
	sc235hai_write_register(ViPipe, 0x320f, 0xBA);
	sc235hai_write_register(ViPipe, 0x3250, 0x00);
	sc235hai_write_register(ViPipe, 0x3301, 0x0a);
	sc235hai_write_register(ViPipe, 0x3302, 0x20);
	sc235hai_write_register(ViPipe, 0x3304, 0x90);
	sc235hai_write_register(ViPipe, 0x3305, 0x00);
	sc235hai_write_register(ViPipe, 0x3306, 0x68);
	sc235hai_write_register(ViPipe, 0x3309, 0xd0);
	sc235hai_write_register(ViPipe, 0x330b, 0xd8);
	sc235hai_write_register(ViPipe, 0x330d, 0x08);
	sc235hai_write_register(ViPipe, 0x331c, 0x04);
	sc235hai_write_register(ViPipe, 0x331e, 0x81);
	sc235hai_write_register(ViPipe, 0x331f, 0xc1);
	sc235hai_write_register(ViPipe, 0x3323, 0x06);
	sc235hai_write_register(ViPipe, 0x3333, 0x10);
	sc235hai_write_register(ViPipe, 0x3334, 0x40);
	sc235hai_write_register(ViPipe, 0x3364, 0x5e);
	sc235hai_write_register(ViPipe, 0x336c, 0x8e);
	sc235hai_write_register(ViPipe, 0x337f, 0x13);
	sc235hai_write_register(ViPipe, 0x338f, 0x80);
	sc235hai_write_register(ViPipe, 0x3390, 0x08);
	sc235hai_write_register(ViPipe, 0x3391, 0x18);
	sc235hai_write_register(ViPipe, 0x3392, 0xb8);
	sc235hai_write_register(ViPipe, 0x3393, 0x0e);
	sc235hai_write_register(ViPipe, 0x3394, 0x14);
	sc235hai_write_register(ViPipe, 0x3395, 0x10);
	sc235hai_write_register(ViPipe, 0x3396, 0x88);
	sc235hai_write_register(ViPipe, 0x3397, 0x98);
	sc235hai_write_register(ViPipe, 0x3398, 0xf8);
	sc235hai_write_register(ViPipe, 0x3399, 0x0a);
	sc235hai_write_register(ViPipe, 0x339a, 0x0e);
	sc235hai_write_register(ViPipe, 0x339b, 0x10);
	sc235hai_write_register(ViPipe, 0x339c, 0x16);
	sc235hai_write_register(ViPipe, 0x33ae, 0x80);
	sc235hai_write_register(ViPipe, 0x33af, 0xc0);
	sc235hai_write_register(ViPipe, 0x33b2, 0x50);
	sc235hai_write_register(ViPipe, 0x33b3, 0x14);
	sc235hai_write_register(ViPipe, 0x33f8, 0x00);
	sc235hai_write_register(ViPipe, 0x33f9, 0x68);
	sc235hai_write_register(ViPipe, 0x33fa, 0x00);
	sc235hai_write_register(ViPipe, 0x33fb, 0x68);
	sc235hai_write_register(ViPipe, 0x33fc, 0x48);
	sc235hai_write_register(ViPipe, 0x33fd, 0x78);
	sc235hai_write_register(ViPipe, 0x349f, 0x03);
	sc235hai_write_register(ViPipe, 0x34a6, 0x40);
	sc235hai_write_register(ViPipe, 0x34a7, 0x58);
	sc235hai_write_register(ViPipe, 0x34a8, 0x10);
	sc235hai_write_register(ViPipe, 0x34a9, 0x10);
	sc235hai_write_register(ViPipe, 0x34f8, 0x78);
	sc235hai_write_register(ViPipe, 0x34f9, 0x10);
	sc235hai_write_register(ViPipe, 0x3619, 0x20);
	sc235hai_write_register(ViPipe, 0x361a, 0x90);
	sc235hai_write_register(ViPipe, 0x3633, 0x44);
	sc235hai_write_register(ViPipe, 0x3637, 0x5c);
	sc235hai_write_register(ViPipe, 0x363c, 0xc0);
	sc235hai_write_register(ViPipe, 0x363d, 0x02);
	sc235hai_write_register(ViPipe, 0x3660, 0x80);
	sc235hai_write_register(ViPipe, 0x3661, 0x81);
	sc235hai_write_register(ViPipe, 0x3662, 0x8f);
	sc235hai_write_register(ViPipe, 0x3663, 0x81);
	sc235hai_write_register(ViPipe, 0x3664, 0x81);
	sc235hai_write_register(ViPipe, 0x3665, 0x82);
	sc235hai_write_register(ViPipe, 0x3666, 0x8f);
	sc235hai_write_register(ViPipe, 0x3667, 0x08);
	sc235hai_write_register(ViPipe, 0x3668, 0x80);
	sc235hai_write_register(ViPipe, 0x3669, 0x88);
	sc235hai_write_register(ViPipe, 0x366a, 0x98);
	sc235hai_write_register(ViPipe, 0x366b, 0xb8);
	sc235hai_write_register(ViPipe, 0x366c, 0xf8);
	sc235hai_write_register(ViPipe, 0x3670, 0xb2);
	sc235hai_write_register(ViPipe, 0x3671, 0xa2);
	sc235hai_write_register(ViPipe, 0x3672, 0x88);
	sc235hai_write_register(ViPipe, 0x3680, 0x33);
	sc235hai_write_register(ViPipe, 0x3681, 0x33);
	sc235hai_write_register(ViPipe, 0x3682, 0x43);
	sc235hai_write_register(ViPipe, 0x36c0, 0x80);
	sc235hai_write_register(ViPipe, 0x36c1, 0x88);
	sc235hai_write_register(ViPipe, 0x36c8, 0x88);
	sc235hai_write_register(ViPipe, 0x36c9, 0xb8);
	sc235hai_write_register(ViPipe, 0x36ea, 0x0b);
	sc235hai_write_register(ViPipe, 0x36eb, 0x0c);
	sc235hai_write_register(ViPipe, 0x36ec, 0x5c);
	sc235hai_write_register(ViPipe, 0x36ed, 0x24);
	sc235hai_write_register(ViPipe, 0x3718, 0x04);
	sc235hai_write_register(ViPipe, 0x3722, 0x8b);
	sc235hai_write_register(ViPipe, 0x3724, 0xd1);
	sc235hai_write_register(ViPipe, 0x3741, 0x08);
	sc235hai_write_register(ViPipe, 0x3770, 0x17);
	sc235hai_write_register(ViPipe, 0x3771, 0x9b);
	sc235hai_write_register(ViPipe, 0x3772, 0x9b);
	sc235hai_write_register(ViPipe, 0x37c0, 0x88);
	sc235hai_write_register(ViPipe, 0x37c1, 0xb8);
	sc235hai_write_register(ViPipe, 0x37fa, 0x0b);
	sc235hai_write_register(ViPipe, 0x37fc, 0x10);
	sc235hai_write_register(ViPipe, 0x37fd, 0x24);
	sc235hai_write_register(ViPipe, 0x3902, 0xc0);
	sc235hai_write_register(ViPipe, 0x3903, 0x40);
	sc235hai_write_register(ViPipe, 0x3909, 0x00);
	sc235hai_write_register(ViPipe, 0x391f, 0x41);
	sc235hai_write_register(ViPipe, 0x3926, 0xe0);
	sc235hai_write_register(ViPipe, 0x3933, 0x80);
	sc235hai_write_register(ViPipe, 0x3934, 0x02);
	sc235hai_write_register(ViPipe, 0x3937, 0x6f);
	sc235hai_write_register(ViPipe, 0x3e00, 0x00);
	sc235hai_write_register(ViPipe, 0x3e01, 0x8b);
	sc235hai_write_register(ViPipe, 0x3e02, 0xf0);
	sc235hai_write_register(ViPipe, 0x3e08, 0x00);
	sc235hai_write_register(ViPipe, 0x4509, 0x20);
	sc235hai_write_register(ViPipe, 0x450d, 0x07);
	sc235hai_write_register(ViPipe, 0x4837, 0x36);
	sc235hai_write_register(ViPipe, 0x5780, 0x76);
	sc235hai_write_register(ViPipe, 0x5784, 0x10);
	sc235hai_write_register(ViPipe, 0x5787, 0x0a);
	sc235hai_write_register(ViPipe, 0x5788, 0x0a);
	sc235hai_write_register(ViPipe, 0x5789, 0x08);
	sc235hai_write_register(ViPipe, 0x578a, 0x0a);
	sc235hai_write_register(ViPipe, 0x578b, 0x0a);
	sc235hai_write_register(ViPipe, 0x578c, 0x08);
	sc235hai_write_register(ViPipe, 0x578d, 0x40);
	sc235hai_write_register(ViPipe, 0x5792, 0x04);
	sc235hai_write_register(ViPipe, 0x5795, 0x04);
	sc235hai_write_register(ViPipe, 0x57ac, 0x00);
	sc235hai_write_register(ViPipe, 0x57ad, 0x00);
	sc235hai_write_register(ViPipe, 0x36e9, 0x20);
	sc235hai_write_register(ViPipe, 0x37f9, 0x20);

	sc235hai_default_reg_init(ViPipe);
	sc235hai_write_register(ViPipe, 0x0100, 0x01);
	delay_ms(50);
	printf("ViPipe:%d,===sc235hai 1080P 15fps 10bit 2LINE Init OK!===\n", ViPipe);

}

/* 1080P15 SLAVE*/
static void sc235hai_2l_slave_linear_1080p15_init(VI_PIPE ViPipe)
{

	sc235hai_write_register(ViPipe, 0x0103, 0x01);
	sc235hai_write_register(ViPipe, 0x36e9, 0x80);
	sc235hai_write_register(ViPipe, 0x37f9, 0x80);
	sc235hai_write_register(ViPipe, 0x301f, 0x05);
	sc235hai_write_register(ViPipe, 0x3058, 0x21);
	sc235hai_write_register(ViPipe, 0x3059, 0x53);
	sc235hai_write_register(ViPipe, 0x305a, 0x40);
	sc235hai_write_register(ViPipe, 0x320c, 0x08);//HTS=2200
	sc235hai_write_register(ViPipe, 0x320d, 0x98);
	//sc235hai_write_register(ViPipe, 0x320e, 0x04);//VTS=1125  30FPS
	//sc235hai_write_register(ViPipe, 0x320f, 0x65);
	sc235hai_write_register(ViPipe, 0x320e, 0x08);//VTS=2250  15FPS
	sc235hai_write_register(ViPipe, 0x320f, 0xBA);
	sc235hai_write_register(ViPipe, 0x3222, 0x01); //slave mode sel (master:0 slave:1)
	sc235hai_write_register(ViPipe, 0x3224, 0xc2); //slave mode trigger sel (bit[4] FSYNC:1 EFSYNC:0)
	sc235hai_write_register(ViPipe, 0x3250, 0x00);
	sc235hai_write_register(ViPipe, 0x3301, 0x0a);
	sc235hai_write_register(ViPipe, 0x3302, 0x20);
	sc235hai_write_register(ViPipe, 0x3304, 0x90);
	sc235hai_write_register(ViPipe, 0x3305, 0x00);
	sc235hai_write_register(ViPipe, 0x3306, 0x68);
	sc235hai_write_register(ViPipe, 0x3309, 0xd0);
	sc235hai_write_register(ViPipe, 0x330b, 0xd8);
	sc235hai_write_register(ViPipe, 0x330d, 0x08);
	sc235hai_write_register(ViPipe, 0x331c, 0x04);
	sc235hai_write_register(ViPipe, 0x331e, 0x81);
	sc235hai_write_register(ViPipe, 0x331f, 0xc1);
	sc235hai_write_register(ViPipe, 0x3323, 0x06);
	sc235hai_write_register(ViPipe, 0x3333, 0x10);
	sc235hai_write_register(ViPipe, 0x3334, 0x40);
	sc235hai_write_register(ViPipe, 0x3364, 0x5e);
	sc235hai_write_register(ViPipe, 0x336c, 0x8e);
	sc235hai_write_register(ViPipe, 0x337f, 0x13);
	sc235hai_write_register(ViPipe, 0x338f, 0x80);
	sc235hai_write_register(ViPipe, 0x3390, 0x08);
	sc235hai_write_register(ViPipe, 0x3391, 0x18);
	sc235hai_write_register(ViPipe, 0x3392, 0xb8);
	sc235hai_write_register(ViPipe, 0x3393, 0x0e);
	sc235hai_write_register(ViPipe, 0x3394, 0x14);
	sc235hai_write_register(ViPipe, 0x3395, 0x10);
	sc235hai_write_register(ViPipe, 0x3396, 0x88);
	sc235hai_write_register(ViPipe, 0x3397, 0x98);
	sc235hai_write_register(ViPipe, 0x3398, 0xf8);
	sc235hai_write_register(ViPipe, 0x3399, 0x0a);
	sc235hai_write_register(ViPipe, 0x339a, 0x0e);
	sc235hai_write_register(ViPipe, 0x339b, 0x10);
	sc235hai_write_register(ViPipe, 0x339c, 0x16);
	sc235hai_write_register(ViPipe, 0x33ae, 0x80);
	sc235hai_write_register(ViPipe, 0x33af, 0xc0);
	sc235hai_write_register(ViPipe, 0x33b2, 0x50);
	sc235hai_write_register(ViPipe, 0x33b3, 0x14);
	sc235hai_write_register(ViPipe, 0x33f8, 0x00);
	sc235hai_write_register(ViPipe, 0x33f9, 0x68);
	sc235hai_write_register(ViPipe, 0x33fa, 0x00);
	sc235hai_write_register(ViPipe, 0x33fb, 0x68);
	sc235hai_write_register(ViPipe, 0x33fc, 0x48);
	sc235hai_write_register(ViPipe, 0x33fd, 0x78);
	sc235hai_write_register(ViPipe, 0x349f, 0x03);
	sc235hai_write_register(ViPipe, 0x34a6, 0x40);
	sc235hai_write_register(ViPipe, 0x34a7, 0x58);
	sc235hai_write_register(ViPipe, 0x34a8, 0x10);
	sc235hai_write_register(ViPipe, 0x34a9, 0x10);
	sc235hai_write_register(ViPipe, 0x34f8, 0x78);
	sc235hai_write_register(ViPipe, 0x34f9, 0x10);
	sc235hai_write_register(ViPipe, 0x3619, 0x20);
	sc235hai_write_register(ViPipe, 0x361a, 0x90);
	sc235hai_write_register(ViPipe, 0x3633, 0x44);
	sc235hai_write_register(ViPipe, 0x3637, 0x5c);
	sc235hai_write_register(ViPipe, 0x363c, 0xc0);
	sc235hai_write_register(ViPipe, 0x363d, 0x02);
	sc235hai_write_register(ViPipe, 0x3660, 0x80);
	sc235hai_write_register(ViPipe, 0x3661, 0x81);
	sc235hai_write_register(ViPipe, 0x3662, 0x8f);
	sc235hai_write_register(ViPipe, 0x3663, 0x81);
	sc235hai_write_register(ViPipe, 0x3664, 0x81);
	sc235hai_write_register(ViPipe, 0x3665, 0x82);
	sc235hai_write_register(ViPipe, 0x3666, 0x8f);
	sc235hai_write_register(ViPipe, 0x3667, 0x08);
	sc235hai_write_register(ViPipe, 0x3668, 0x80);
	sc235hai_write_register(ViPipe, 0x3669, 0x88);
	sc235hai_write_register(ViPipe, 0x366a, 0x98);
	sc235hai_write_register(ViPipe, 0x366b, 0xb8);
	sc235hai_write_register(ViPipe, 0x366c, 0xf8);
	sc235hai_write_register(ViPipe, 0x3670, 0xb2);
	sc235hai_write_register(ViPipe, 0x3671, 0xa2);
	sc235hai_write_register(ViPipe, 0x3672, 0x88);
	sc235hai_write_register(ViPipe, 0x3680, 0x33);
	sc235hai_write_register(ViPipe, 0x3681, 0x33);
	sc235hai_write_register(ViPipe, 0x3682, 0x43);
	sc235hai_write_register(ViPipe, 0x36c0, 0x80);
	sc235hai_write_register(ViPipe, 0x36c1, 0x88);
	sc235hai_write_register(ViPipe, 0x36c8, 0x88);
	sc235hai_write_register(ViPipe, 0x36c9, 0xb8);
	sc235hai_write_register(ViPipe, 0x36ea, 0x0b);
	sc235hai_write_register(ViPipe, 0x36eb, 0x0c);
	sc235hai_write_register(ViPipe, 0x36ec, 0x5c);
	sc235hai_write_register(ViPipe, 0x36ed, 0x24);
	sc235hai_write_register(ViPipe, 0x3718, 0x04);
	sc235hai_write_register(ViPipe, 0x3722, 0x8b);
	sc235hai_write_register(ViPipe, 0x3724, 0xd1);
	sc235hai_write_register(ViPipe, 0x3741, 0x08);
	sc235hai_write_register(ViPipe, 0x3770, 0x17);
	sc235hai_write_register(ViPipe, 0x3771, 0x9b);
	sc235hai_write_register(ViPipe, 0x3772, 0x9b);
	sc235hai_write_register(ViPipe, 0x37c0, 0x88);
	sc235hai_write_register(ViPipe, 0x37c1, 0xb8);
	sc235hai_write_register(ViPipe, 0x37fa, 0x0b);
	sc235hai_write_register(ViPipe, 0x37fc, 0x10);
	sc235hai_write_register(ViPipe, 0x37fd, 0x24);
	sc235hai_write_register(ViPipe, 0x3902, 0xc0);
	sc235hai_write_register(ViPipe, 0x3903, 0x40);
	sc235hai_write_register(ViPipe, 0x3909, 0x00);
	sc235hai_write_register(ViPipe, 0x391f, 0x41);
	sc235hai_write_register(ViPipe, 0x3926, 0xe0);
	sc235hai_write_register(ViPipe, 0x3933, 0x80);
	sc235hai_write_register(ViPipe, 0x3934, 0x02);
	sc235hai_write_register(ViPipe, 0x3937, 0x6f);
	sc235hai_write_register(ViPipe, 0x3e00, 0x00);
	sc235hai_write_register(ViPipe, 0x3e01, 0x8b);
	sc235hai_write_register(ViPipe, 0x3e02, 0xf0);
	sc235hai_write_register(ViPipe, 0x3e08, 0x00);
	sc235hai_write_register(ViPipe, 0x4509, 0x20);
	sc235hai_write_register(ViPipe, 0x450d, 0x07);
	sc235hai_write_register(ViPipe, 0x4837, 0x36);
	sc235hai_write_register(ViPipe, 0x5780, 0x76);
	sc235hai_write_register(ViPipe, 0x5784, 0x10);
	sc235hai_write_register(ViPipe, 0x5787, 0x0a);
	sc235hai_write_register(ViPipe, 0x5788, 0x0a);
	sc235hai_write_register(ViPipe, 0x5789, 0x08);
	sc235hai_write_register(ViPipe, 0x578a, 0x0a);
	sc235hai_write_register(ViPipe, 0x578b, 0x0a);
	sc235hai_write_register(ViPipe, 0x578c, 0x08);
	sc235hai_write_register(ViPipe, 0x578d, 0x40);
	sc235hai_write_register(ViPipe, 0x5792, 0x04);
	sc235hai_write_register(ViPipe, 0x5795, 0x04);
	sc235hai_write_register(ViPipe, 0x57ac, 0x00);
	sc235hai_write_register(ViPipe, 0x57ad, 0x00);
	sc235hai_write_register(ViPipe, 0x36e9, 0x20);
	sc235hai_write_register(ViPipe, 0x37f9, 0x20);

	sc235hai_default_reg_init(ViPipe);
	sc235hai_write_register(ViPipe, 0x0100, 0x01);
	delay_ms(50);
	printf("ViPipe:%d,===sc235hai 1080P 15fps 10bit 2LINE SLAVE Init OK!===\n", ViPipe);

}




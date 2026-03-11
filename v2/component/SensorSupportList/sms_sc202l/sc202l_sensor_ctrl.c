#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "sc202l_cmos_ex.h"
#include "sensor_i2c.h"

static void sc202l_linear_1080P30_master_init(VI_PIPE ViPipe);
static void sc202l_linear_1080P30_slave_init(VI_PIPE ViPipe);

const CVI_U8 sc202l_i2c_addr = 0x30;        /* I2C Address of SC202L */
const CVI_U32 sc202l_addr_byte = 2;
const CVI_U32 sc202l_data_byte = 1;

int sc202l_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunSC202L_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC202L_AddrInfo[ViPipe].s8I2cAddr);
}

int sc202l_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunSC202L_BusInfo[ViPipe].s8I2cDev);
}

int sc202l_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunSC202L_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC202L_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc202l_addr_byte, sc202l_data_byte);
}

int sc202l_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunSC202L_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunSC202L_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							sc202l_addr_byte, (CVI_U32)data, sc202l_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void sc202l_standby(VI_PIPE ViPipe)
{
    CVI_U8 val = sc202l_read_register(ViPipe, 0x0100);
    val &= ~0x01;
	sc202l_write_register(ViPipe, 0x0100, val);
}

void sc202l_restart(VI_PIPE ViPipe)
{
    CVI_U8 val = sc202l_read_register(ViPipe, 0x0103);
    val |= 0x01;
	sc202l_write_register(ViPipe, 0x0103, val);
	delay_ms(20);
    val &= ~0x01;
	sc202l_write_register(ViPipe, 0x0103, val);
}

void sc202l_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 val = sc202l_read_register(ViPipe, 0x3221) & ~0x66;

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

	sc202l_write_register(ViPipe, 0x3221, val);
}

#define sc202l_CHIP_ID_HI_ADDR		0x3107
#define sc202l_CHIP_ID_LO_ADDR		0x3108
#define sc202l_CHIP_ID				0xcb3e

int sc202l_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	delay_ms(4);
	if (sc202l_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = sc202l_read_register(ViPipe, sc202l_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = sc202l_read_register(ViPipe, sc202l_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != sc202l_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}


void sc202l_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastSC202L[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		if (g_pastSC202L[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].bUpdate == CVI_TRUE) {
			sc202l_write_register(ViPipe,
				g_pastSC202L[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastSC202L[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
		}
	}
}

void sc202l_init(VI_PIPE ViPipe)
{
    CVI_U8 u8ImgMode = g_pastSC202L[ViPipe]->u8ImgMode;

	sc202l_i2c_init(ViPipe);

	//linear mode only
    if (u8ImgMode == SC202L_MODE_1920X1080P30_MASTER) {
        sc202l_linear_1080P30_master_init(ViPipe);
    } else if (u8ImgMode == SC202L_MODE_1920X1080P30_SLAVE) {
        sc202l_linear_1080P30_slave_init(ViPipe);
    }

	g_pastSC202L[ViPipe]->bInit = CVI_TRUE;
}

/* 1080P30 */
static void sc202l_linear_1080P30_master_init(VI_PIPE ViPipe)
{
    sc202l_write_register(ViPipe, 0x0103, 0x01);
    sc202l_write_register(ViPipe, 0x0100, 0x00);
    sc202l_write_register(ViPipe, 0x36e9, 0x80);
    sc202l_write_register(ViPipe, 0x37f9, 0x80);
    sc202l_write_register(ViPipe, 0x300a, 0x20);
    sc202l_write_register(ViPipe, 0x301f, 0x0b);
    sc202l_write_register(ViPipe, 0x3032, 0x22);
    sc202l_write_register(ViPipe, 0x30b8, 0x44);
    sc202l_write_register(ViPipe, 0x320c, 0x08);
    sc202l_write_register(ViPipe, 0x320d, 0x55);
    sc202l_write_register(ViPipe, 0x3222, 0x00); //master mode
    sc202l_write_register(ViPipe, 0x3253, 0x0c);
    sc202l_write_register(ViPipe, 0x3281, 0x80);
    sc202l_write_register(ViPipe, 0x3301, 0x06);
    sc202l_write_register(ViPipe, 0x3302, 0x12);
    sc202l_write_register(ViPipe, 0x3306, 0x80);
    sc202l_write_register(ViPipe, 0x3309, 0xc0);
    sc202l_write_register(ViPipe, 0x330a, 0x00);
    sc202l_write_register(ViPipe, 0x330b, 0xe0);
    sc202l_write_register(ViPipe, 0x330d, 0x20);
    sc202l_write_register(ViPipe, 0x3314, 0x15);
    sc202l_write_register(ViPipe, 0x331e, 0x41);
    sc202l_write_register(ViPipe, 0x331f, 0xb1);
    sc202l_write_register(ViPipe, 0x3320, 0x0a);
    sc202l_write_register(ViPipe, 0x3326, 0x0e);
    sc202l_write_register(ViPipe, 0x3333, 0x10);
    sc202l_write_register(ViPipe, 0x3334, 0x40);
    sc202l_write_register(ViPipe, 0x335d, 0x60);
    sc202l_write_register(ViPipe, 0x335e, 0x06);
    sc202l_write_register(ViPipe, 0x335f, 0x08);
    sc202l_write_register(ViPipe, 0x3364, 0x56);
    sc202l_write_register(ViPipe, 0x337a, 0x06);
    sc202l_write_register(ViPipe, 0x337b, 0x0e);
    sc202l_write_register(ViPipe, 0x337c, 0x02);
    sc202l_write_register(ViPipe, 0x337d, 0x0a);
    sc202l_write_register(ViPipe, 0x3390, 0x03);
    sc202l_write_register(ViPipe, 0x3391, 0x0f);
    sc202l_write_register(ViPipe, 0x3392, 0x1f);
    sc202l_write_register(ViPipe, 0x3393, 0x06);
    sc202l_write_register(ViPipe, 0x3394, 0x06);
    sc202l_write_register(ViPipe, 0x3395, 0x06);
    sc202l_write_register(ViPipe, 0x3396, 0x48);
    sc202l_write_register(ViPipe, 0x3397, 0x4b);
    sc202l_write_register(ViPipe, 0x3398, 0x5f);
    sc202l_write_register(ViPipe, 0x3399, 0x06);
    sc202l_write_register(ViPipe, 0x339a, 0x06);
    sc202l_write_register(ViPipe, 0x339b, 0x52);
    sc202l_write_register(ViPipe, 0x339c, 0x52);
    sc202l_write_register(ViPipe, 0x33a2, 0x04);
    sc202l_write_register(ViPipe, 0x33a3, 0x0a);
    sc202l_write_register(ViPipe, 0x33ac, 0x08);
    sc202l_write_register(ViPipe, 0x33ad, 0x1c);
    sc202l_write_register(ViPipe, 0x33ae, 0x40);
    sc202l_write_register(ViPipe, 0x33af, 0xb0);
    sc202l_write_register(ViPipe, 0x33b1, 0x80);
    sc202l_write_register(ViPipe, 0x33b3, 0x20);
    sc202l_write_register(ViPipe, 0x349f, 0x02);
    sc202l_write_register(ViPipe, 0x34a6, 0x48);
    sc202l_write_register(ViPipe, 0x34a7, 0x4b);
    sc202l_write_register(ViPipe, 0x34a8, 0x20);
    sc202l_write_register(ViPipe, 0x34a9, 0x20);
    sc202l_write_register(ViPipe, 0x34f8, 0x5f);
    sc202l_write_register(ViPipe, 0x34f9, 0x10);
    sc202l_write_register(ViPipe, 0x3616, 0xac);
    sc202l_write_register(ViPipe, 0x3630, 0xc0);
    sc202l_write_register(ViPipe, 0x3631, 0x86);
    sc202l_write_register(ViPipe, 0x3632, 0x24);
    sc202l_write_register(ViPipe, 0x3633, 0x32);
    sc202l_write_register(ViPipe, 0x3637, 0x26);
    sc202l_write_register(ViPipe, 0x363a, 0x84);
    sc202l_write_register(ViPipe, 0x363b, 0x04);
    sc202l_write_register(ViPipe, 0x363c, 0x88);
    sc202l_write_register(ViPipe, 0x3641, 0x22);
    sc202l_write_register(ViPipe, 0x364f, 0x36);
    sc202l_write_register(ViPipe, 0x3670, 0xce);
    sc202l_write_register(ViPipe, 0x3674, 0xc0);
    sc202l_write_register(ViPipe, 0x3675, 0xc0);
    sc202l_write_register(ViPipe, 0x3676, 0xc0);
    sc202l_write_register(ViPipe, 0x3677, 0x86);
    sc202l_write_register(ViPipe, 0x3678, 0x8a);
    sc202l_write_register(ViPipe, 0x3679, 0x8a);
    sc202l_write_register(ViPipe, 0x367c, 0x4b);
    sc202l_write_register(ViPipe, 0x367d, 0x5f);
    sc202l_write_register(ViPipe, 0x367e, 0x4b);
    sc202l_write_register(ViPipe, 0x367f, 0x5f);
    sc202l_write_register(ViPipe, 0x3690, 0x22);
    sc202l_write_register(ViPipe, 0x3691, 0x22);
    sc202l_write_register(ViPipe, 0x3692, 0x32);
    sc202l_write_register(ViPipe, 0x3699, 0x86);
    sc202l_write_register(ViPipe, 0x369a, 0x95);
    sc202l_write_register(ViPipe, 0x369b, 0xab);
    sc202l_write_register(ViPipe, 0x369c, 0x48);
    sc202l_write_register(ViPipe, 0x369d, 0x4b);
    sc202l_write_register(ViPipe, 0x36a2, 0x4b);
    sc202l_write_register(ViPipe, 0x36a3, 0x4f);
    sc202l_write_register(ViPipe, 0x36ea, 0x0a);
    sc202l_write_register(ViPipe, 0x36eb, 0x0c);
    sc202l_write_register(ViPipe, 0x36ec, 0x1c);
    sc202l_write_register(ViPipe, 0x36ed, 0x08);
    sc202l_write_register(ViPipe, 0x370f, 0x01);
    sc202l_write_register(ViPipe, 0x3721, 0x6c);
    sc202l_write_register(ViPipe, 0x3722, 0x09);
    sc202l_write_register(ViPipe, 0x3724, 0x41);
    sc202l_write_register(ViPipe, 0x3725, 0xc4);
    sc202l_write_register(ViPipe, 0x37b0, 0x09);
    sc202l_write_register(ViPipe, 0x37b1, 0x09);
    sc202l_write_register(ViPipe, 0x37b2, 0x05);
    sc202l_write_register(ViPipe, 0x37b3, 0x48);
    sc202l_write_register(ViPipe, 0x37b4, 0x5f);
    sc202l_write_register(ViPipe, 0x37fa, 0x0c);
    sc202l_write_register(ViPipe, 0x37fb, 0x32);
    sc202l_write_register(ViPipe, 0x37fc, 0x10);
    sc202l_write_register(ViPipe, 0x37fd, 0x07);
    sc202l_write_register(ViPipe, 0x3900, 0x19);
    sc202l_write_register(ViPipe, 0x3901, 0x02);
    sc202l_write_register(ViPipe, 0x3905, 0xb8);
    sc202l_write_register(ViPipe, 0x391b, 0x80);
    sc202l_write_register(ViPipe, 0x391c, 0x04);
    sc202l_write_register(ViPipe, 0x391d, 0x81);
    sc202l_write_register(ViPipe, 0x391f, 0x04);
    sc202l_write_register(ViPipe, 0x3933, 0x81);
    sc202l_write_register(ViPipe, 0x3934, 0x4c);
    sc202l_write_register(ViPipe, 0x393f, 0xff);
    sc202l_write_register(ViPipe, 0x3940, 0x75);
    sc202l_write_register(ViPipe, 0x3942, 0x01);
    sc202l_write_register(ViPipe, 0x3943, 0x4d);
    sc202l_write_register(ViPipe, 0x3946, 0x20);
    sc202l_write_register(ViPipe, 0x3957, 0x86);
    sc202l_write_register(ViPipe, 0x3e01, 0x8c);
    sc202l_write_register(ViPipe, 0x3e28, 0xc4);
    sc202l_write_register(ViPipe, 0x4501, 0xc0);
    sc202l_write_register(ViPipe, 0x4509, 0x14);
    sc202l_write_register(ViPipe, 0x450d, 0x11);
    sc202l_write_register(ViPipe, 0x4518, 0x00);
    sc202l_write_register(ViPipe, 0x451b, 0x0a);
    sc202l_write_register(ViPipe, 0x501c, 0x00);
    sc202l_write_register(ViPipe, 0x501d, 0x80);
    sc202l_write_register(ViPipe, 0x501e, 0x00);
    sc202l_write_register(ViPipe, 0x501f, 0x40);
    sc202l_write_register(ViPipe, 0x5784, 0x10);
    sc202l_write_register(ViPipe, 0x5787, 0x0a);
    sc202l_write_register(ViPipe, 0x5788, 0x0a);
    sc202l_write_register(ViPipe, 0x5789, 0x08);
    sc202l_write_register(ViPipe, 0x578a, 0x0a);
    sc202l_write_register(ViPipe, 0x578b, 0x0a);
    sc202l_write_register(ViPipe, 0x578c, 0x08);
    sc202l_write_register(ViPipe, 0x578d, 0x40);
    sc202l_write_register(ViPipe, 0x5790, 0x08);
    sc202l_write_register(ViPipe, 0x5791, 0x04);
    sc202l_write_register(ViPipe, 0x5792, 0x04);
    sc202l_write_register(ViPipe, 0x5793, 0x08);
    sc202l_write_register(ViPipe, 0x5794, 0x04);
    sc202l_write_register(ViPipe, 0x5795, 0x04);
    sc202l_write_register(ViPipe, 0x5799, 0x46);
    sc202l_write_register(ViPipe, 0x57aa, 0x2a);
    sc202l_write_register(ViPipe, 0x57ab, 0x7f);
    sc202l_write_register(ViPipe, 0x57ac, 0x00);
    sc202l_write_register(ViPipe, 0x57ad, 0x00);
    sc202l_write_register(ViPipe, 0x5ae0, 0xfe);
    sc202l_write_register(ViPipe, 0x5ae1, 0x40);
    sc202l_write_register(ViPipe, 0x5ae2, 0x38);
    sc202l_write_register(ViPipe, 0x5ae3, 0x30);
    sc202l_write_register(ViPipe, 0x5ae4, 0x28);
    sc202l_write_register(ViPipe, 0x5ae5, 0x38);
    sc202l_write_register(ViPipe, 0x5ae6, 0x30);
    sc202l_write_register(ViPipe, 0x5ae7, 0x28);
    sc202l_write_register(ViPipe, 0x5ae8, 0x3f);
    sc202l_write_register(ViPipe, 0x5ae9, 0x34);
    sc202l_write_register(ViPipe, 0x5aea, 0x2c);
    sc202l_write_register(ViPipe, 0x5aeb, 0x3f);
    sc202l_write_register(ViPipe, 0x5aec, 0x34);
    sc202l_write_register(ViPipe, 0x5aed, 0x2c);
    sc202l_write_register(ViPipe, 0x5aee, 0xfe);
    sc202l_write_register(ViPipe, 0x5aef, 0x40);
    sc202l_write_register(ViPipe, 0x5af4, 0x38);
    sc202l_write_register(ViPipe, 0x5af5, 0x30);
    sc202l_write_register(ViPipe, 0x5af6, 0x28);
    sc202l_write_register(ViPipe, 0x5af7, 0x38);
    sc202l_write_register(ViPipe, 0x5af8, 0x30);
    sc202l_write_register(ViPipe, 0x5af9, 0x28);
    sc202l_write_register(ViPipe, 0x5afa, 0x3f);
    sc202l_write_register(ViPipe, 0x5afb, 0x34);
    sc202l_write_register(ViPipe, 0x5afc, 0x2c);
    sc202l_write_register(ViPipe, 0x5afd, 0x3f);
    sc202l_write_register(ViPipe, 0x5afe, 0x34);
    sc202l_write_register(ViPipe, 0x5aff, 0x2c);
    sc202l_write_register(ViPipe, 0x36e9, 0x03);
    sc202l_write_register(ViPipe, 0x37f9, 0x03);
    sc202l_write_register(ViPipe, 0x440d, 0x10);
    sc202l_write_register(ViPipe, 0x440e, 0x01);
    sc202l_write_register(ViPipe, 0x0100, 0x01);

    sc202l_default_reg_init(ViPipe);
    delay_ms(50);
    sc202l_write_register(ViPipe, 0x0100, 0x01);

    printf("ViPipe:%d,===SC202L 1080P 30fps 10bit LINE MASTER Init OK!===\n", ViPipe);
}

static void sc202l_linear_1080P30_slave_init(VI_PIPE ViPipe)
{
    sc202l_write_register(ViPipe, 0x0103, 0x01);
    sc202l_write_register(ViPipe, 0x0100, 0x00);
    sc202l_write_register(ViPipe, 0x36e9, 0x80);
    sc202l_write_register(ViPipe, 0x37f9, 0x80);
    sc202l_write_register(ViPipe, 0x300a, 0x20);
    sc202l_write_register(ViPipe, 0x301f, 0x0b);
    sc202l_write_register(ViPipe, 0x3032, 0x22);
    sc202l_write_register(ViPipe, 0x30b8, 0x44);
    sc202l_write_register(ViPipe, 0x320c, 0x08);
    sc202l_write_register(ViPipe, 0x320d, 0x55);
    sc202l_write_register(ViPipe, 0x3222, 0x01); //slave mode
    sc202l_write_register(ViPipe, 0x3224, 0x82); //sel EFSYNC
    sc202l_write_register(ViPipe, 0x3253, 0x0c);
    sc202l_write_register(ViPipe, 0x3281, 0x80);
    sc202l_write_register(ViPipe, 0x3301, 0x06);
    sc202l_write_register(ViPipe, 0x3302, 0x12);
    sc202l_write_register(ViPipe, 0x3306, 0x80);
    sc202l_write_register(ViPipe, 0x3309, 0xc0);
    sc202l_write_register(ViPipe, 0x330a, 0x00);
    sc202l_write_register(ViPipe, 0x330b, 0xe0);
    sc202l_write_register(ViPipe, 0x330d, 0x20);
    sc202l_write_register(ViPipe, 0x3314, 0x15);
    sc202l_write_register(ViPipe, 0x331e, 0x41);
    sc202l_write_register(ViPipe, 0x331f, 0xb1);
    sc202l_write_register(ViPipe, 0x3320, 0x0a);
    sc202l_write_register(ViPipe, 0x3326, 0x0e);
    sc202l_write_register(ViPipe, 0x3333, 0x10);
    sc202l_write_register(ViPipe, 0x3334, 0x40);
    sc202l_write_register(ViPipe, 0x335d, 0x60);
    sc202l_write_register(ViPipe, 0x335e, 0x06);
    sc202l_write_register(ViPipe, 0x335f, 0x08);
    sc202l_write_register(ViPipe, 0x3364, 0x56);
    sc202l_write_register(ViPipe, 0x337a, 0x06);
    sc202l_write_register(ViPipe, 0x337b, 0x0e);
    sc202l_write_register(ViPipe, 0x337c, 0x02);
    sc202l_write_register(ViPipe, 0x337d, 0x0a);
    sc202l_write_register(ViPipe, 0x3390, 0x03);
    sc202l_write_register(ViPipe, 0x3391, 0x0f);
    sc202l_write_register(ViPipe, 0x3392, 0x1f);
    sc202l_write_register(ViPipe, 0x3393, 0x06);
    sc202l_write_register(ViPipe, 0x3394, 0x06);
    sc202l_write_register(ViPipe, 0x3395, 0x06);
    sc202l_write_register(ViPipe, 0x3396, 0x48);
    sc202l_write_register(ViPipe, 0x3397, 0x4b);
    sc202l_write_register(ViPipe, 0x3398, 0x5f);
    sc202l_write_register(ViPipe, 0x3399, 0x06);
    sc202l_write_register(ViPipe, 0x339a, 0x06);
    sc202l_write_register(ViPipe, 0x339b, 0x52);
    sc202l_write_register(ViPipe, 0x339c, 0x52);
    sc202l_write_register(ViPipe, 0x33a2, 0x04);
    sc202l_write_register(ViPipe, 0x33a3, 0x0a);
    sc202l_write_register(ViPipe, 0x33ac, 0x08);
    sc202l_write_register(ViPipe, 0x33ad, 0x1c);
    sc202l_write_register(ViPipe, 0x33ae, 0x40);
    sc202l_write_register(ViPipe, 0x33af, 0xb0);
    sc202l_write_register(ViPipe, 0x33b1, 0x80);
    sc202l_write_register(ViPipe, 0x33b3, 0x20);
    sc202l_write_register(ViPipe, 0x349f, 0x02);
    sc202l_write_register(ViPipe, 0x34a6, 0x48);
    sc202l_write_register(ViPipe, 0x34a7, 0x4b);
    sc202l_write_register(ViPipe, 0x34a8, 0x20);
    sc202l_write_register(ViPipe, 0x34a9, 0x20);
    sc202l_write_register(ViPipe, 0x34f8, 0x5f);
    sc202l_write_register(ViPipe, 0x34f9, 0x10);
    sc202l_write_register(ViPipe, 0x3616, 0xac);
    sc202l_write_register(ViPipe, 0x3630, 0xc0);
    sc202l_write_register(ViPipe, 0x3631, 0x86);
    sc202l_write_register(ViPipe, 0x3632, 0x24);
    sc202l_write_register(ViPipe, 0x3633, 0x32);
    sc202l_write_register(ViPipe, 0x3637, 0x26);
    sc202l_write_register(ViPipe, 0x363a, 0x84);
    sc202l_write_register(ViPipe, 0x363b, 0x04);
    sc202l_write_register(ViPipe, 0x363c, 0x88);
    sc202l_write_register(ViPipe, 0x3641, 0x22);
    sc202l_write_register(ViPipe, 0x364f, 0x36);
    sc202l_write_register(ViPipe, 0x3670, 0xce);
    sc202l_write_register(ViPipe, 0x3674, 0xc0);
    sc202l_write_register(ViPipe, 0x3675, 0xc0);
    sc202l_write_register(ViPipe, 0x3676, 0xc0);
    sc202l_write_register(ViPipe, 0x3677, 0x86);
    sc202l_write_register(ViPipe, 0x3678, 0x8a);
    sc202l_write_register(ViPipe, 0x3679, 0x8a);
    sc202l_write_register(ViPipe, 0x367c, 0x4b);
    sc202l_write_register(ViPipe, 0x367d, 0x5f);
    sc202l_write_register(ViPipe, 0x367e, 0x4b);
    sc202l_write_register(ViPipe, 0x367f, 0x5f);
    sc202l_write_register(ViPipe, 0x3690, 0x22);
    sc202l_write_register(ViPipe, 0x3691, 0x22);
    sc202l_write_register(ViPipe, 0x3692, 0x32);
    sc202l_write_register(ViPipe, 0x3699, 0x86);
    sc202l_write_register(ViPipe, 0x369a, 0x95);
    sc202l_write_register(ViPipe, 0x369b, 0xab);
    sc202l_write_register(ViPipe, 0x369c, 0x48);
    sc202l_write_register(ViPipe, 0x369d, 0x4b);
    sc202l_write_register(ViPipe, 0x36a2, 0x4b);
    sc202l_write_register(ViPipe, 0x36a3, 0x4f);
    sc202l_write_register(ViPipe, 0x36ea, 0x0a);
    sc202l_write_register(ViPipe, 0x36eb, 0x0c);
    sc202l_write_register(ViPipe, 0x36ec, 0x1c);
    sc202l_write_register(ViPipe, 0x36ed, 0x08);
    sc202l_write_register(ViPipe, 0x370f, 0x01);
    sc202l_write_register(ViPipe, 0x3721, 0x6c);
    sc202l_write_register(ViPipe, 0x3722, 0x09);
    sc202l_write_register(ViPipe, 0x3724, 0x41);
    sc202l_write_register(ViPipe, 0x3725, 0xc4);
    sc202l_write_register(ViPipe, 0x37b0, 0x09);
    sc202l_write_register(ViPipe, 0x37b1, 0x09);
    sc202l_write_register(ViPipe, 0x37b2, 0x05);
    sc202l_write_register(ViPipe, 0x37b3, 0x48);
    sc202l_write_register(ViPipe, 0x37b4, 0x5f);
    sc202l_write_register(ViPipe, 0x37fa, 0x0c);
    sc202l_write_register(ViPipe, 0x37fb, 0x32);
    sc202l_write_register(ViPipe, 0x37fc, 0x10);
    sc202l_write_register(ViPipe, 0x37fd, 0x07);
    sc202l_write_register(ViPipe, 0x3900, 0x19);
    sc202l_write_register(ViPipe, 0x3901, 0x02);
    sc202l_write_register(ViPipe, 0x3905, 0xb8);
    sc202l_write_register(ViPipe, 0x391b, 0x80);
    sc202l_write_register(ViPipe, 0x391c, 0x04);
    sc202l_write_register(ViPipe, 0x391d, 0x81);
    sc202l_write_register(ViPipe, 0x391f, 0x04);
    sc202l_write_register(ViPipe, 0x3933, 0x81);
    sc202l_write_register(ViPipe, 0x3934, 0x4c);
    sc202l_write_register(ViPipe, 0x393f, 0xff);
    sc202l_write_register(ViPipe, 0x3940, 0x75);
    sc202l_write_register(ViPipe, 0x3942, 0x01);
    sc202l_write_register(ViPipe, 0x3943, 0x4d);
    sc202l_write_register(ViPipe, 0x3946, 0x20);
    sc202l_write_register(ViPipe, 0x3957, 0x86);
    sc202l_write_register(ViPipe, 0x3e01, 0x8c);
    sc202l_write_register(ViPipe, 0x3e28, 0xc4);
    sc202l_write_register(ViPipe, 0x4501, 0xc0);
    sc202l_write_register(ViPipe, 0x4509, 0x14);
    sc202l_write_register(ViPipe, 0x450d, 0x11);
    sc202l_write_register(ViPipe, 0x4518, 0x00);
    sc202l_write_register(ViPipe, 0x451b, 0x0a);
    sc202l_write_register(ViPipe, 0x501c, 0x00);
    sc202l_write_register(ViPipe, 0x501d, 0x80);
    sc202l_write_register(ViPipe, 0x501e, 0x00);
    sc202l_write_register(ViPipe, 0x501f, 0x40);
    sc202l_write_register(ViPipe, 0x5784, 0x10);
    sc202l_write_register(ViPipe, 0x5787, 0x0a);
    sc202l_write_register(ViPipe, 0x5788, 0x0a);
    sc202l_write_register(ViPipe, 0x5789, 0x08);
    sc202l_write_register(ViPipe, 0x578a, 0x0a);
    sc202l_write_register(ViPipe, 0x578b, 0x0a);
    sc202l_write_register(ViPipe, 0x578c, 0x08);
    sc202l_write_register(ViPipe, 0x578d, 0x40);
    sc202l_write_register(ViPipe, 0x5790, 0x08);
    sc202l_write_register(ViPipe, 0x5791, 0x04);
    sc202l_write_register(ViPipe, 0x5792, 0x04);
    sc202l_write_register(ViPipe, 0x5793, 0x08);
    sc202l_write_register(ViPipe, 0x5794, 0x04);
    sc202l_write_register(ViPipe, 0x5795, 0x04);
    sc202l_write_register(ViPipe, 0x5799, 0x46);
    sc202l_write_register(ViPipe, 0x57aa, 0x2a);
    sc202l_write_register(ViPipe, 0x57ab, 0x7f);
    sc202l_write_register(ViPipe, 0x57ac, 0x00);
    sc202l_write_register(ViPipe, 0x57ad, 0x00);
    sc202l_write_register(ViPipe, 0x5ae0, 0xfe);
    sc202l_write_register(ViPipe, 0x5ae1, 0x40);
    sc202l_write_register(ViPipe, 0x5ae2, 0x38);
    sc202l_write_register(ViPipe, 0x5ae3, 0x30);
    sc202l_write_register(ViPipe, 0x5ae4, 0x28);
    sc202l_write_register(ViPipe, 0x5ae5, 0x38);
    sc202l_write_register(ViPipe, 0x5ae6, 0x30);
    sc202l_write_register(ViPipe, 0x5ae7, 0x28);
    sc202l_write_register(ViPipe, 0x5ae8, 0x3f);
    sc202l_write_register(ViPipe, 0x5ae9, 0x34);
    sc202l_write_register(ViPipe, 0x5aea, 0x2c);
    sc202l_write_register(ViPipe, 0x5aeb, 0x3f);
    sc202l_write_register(ViPipe, 0x5aec, 0x34);
    sc202l_write_register(ViPipe, 0x5aed, 0x2c);
    sc202l_write_register(ViPipe, 0x5aee, 0xfe);
    sc202l_write_register(ViPipe, 0x5aef, 0x40);
    sc202l_write_register(ViPipe, 0x5af4, 0x38);
    sc202l_write_register(ViPipe, 0x5af5, 0x30);
    sc202l_write_register(ViPipe, 0x5af6, 0x28);
    sc202l_write_register(ViPipe, 0x5af7, 0x38);
    sc202l_write_register(ViPipe, 0x5af8, 0x30);
    sc202l_write_register(ViPipe, 0x5af9, 0x28);
    sc202l_write_register(ViPipe, 0x5afa, 0x3f);
    sc202l_write_register(ViPipe, 0x5afb, 0x34);
    sc202l_write_register(ViPipe, 0x5afc, 0x2c);
    sc202l_write_register(ViPipe, 0x5afd, 0x3f);
    sc202l_write_register(ViPipe, 0x5afe, 0x34);
    sc202l_write_register(ViPipe, 0x5aff, 0x2c);
    sc202l_write_register(ViPipe, 0x36e9, 0x03);
    sc202l_write_register(ViPipe, 0x37f9, 0x03);
    sc202l_write_register(ViPipe, 0x440d, 0x10);
    sc202l_write_register(ViPipe, 0x440e, 0x01);
    sc202l_write_register(ViPipe, 0x0100, 0x01);

    sc202l_default_reg_init(ViPipe);
    delay_ms(50);
    sc202l_write_register(ViPipe, 0x0100, 0x01);

    printf("ViPipe:%d,===SC202L 1080P 30fps 10bit LINE SLAVE Init OK!===\n", ViPipe);
}

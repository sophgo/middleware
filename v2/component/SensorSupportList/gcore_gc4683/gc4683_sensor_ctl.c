#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "gc4683_cmos_ex.h"
#include "sensor_i2c.h"

#define GC4683_CHIP_ID_ADDR_H	0x03f0
#define GC4683_CHIP_ID_ADDR_L	0x03f1
#define GC4683_CHIP_ID		0x4683

static void gc4683_linear_1440p60_init(VI_PIPE ViPipe);
static void gc4683_linear_720p120_init(VI_PIPE ViPipe);

const CVI_U32 gc4683_addr_byte = 2;
const CVI_U32 gc4683_data_byte = 1;

int gc4683_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunGc4683_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunGc4683_AddrInfo[ViPipe].s8I2cAddr);
}

int gc4683_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunGc4683_BusInfo[ViPipe].s8I2cDev);
}

int gc4683_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunGc4683_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunGc4683_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							gc4683_addr_byte, gc4683_data_byte);
}

int gc4683_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunGc4683_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunGc4683_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							gc4683_addr_byte, (CVI_U32)data, gc4683_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void gc4683_standby(VI_PIPE ViPipe)
{
	gc4683_write_register(ViPipe, 0x0100, 0x00);
	gc4683_write_register(ViPipe, 0x031c, 0xc7);
	gc4683_write_register(ViPipe, 0x0317, 0x01);

	printf("gc4683 standby\n");
}

void gc4683_restart(VI_PIPE ViPipe)
{
	gc4683_write_register(ViPipe, 0x0317, 0x00);
	gc4683_write_register(ViPipe, 0x031c, 0xc6);
	gc4683_write_register(ViPipe, 0x0100, 0x09);

	printf("gc4683 restart\n");
}

void gc4683_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastGc4683[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		gc4683_write_register(ViPipe,
				g_pastGc4683[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastGc4683[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

int gc4683_probe(VI_PIPE ViPipe)
{
	int nVal;
	int nVal2;

	usleep(50);
	if (gc4683_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = gc4683_read_register(ViPipe, GC4683_CHIP_ID_ADDR_H);
	nVal2 = gc4683_read_register(ViPipe, GC4683_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != GC4683_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void gc4683_init(VI_PIPE ViPipe)
{

	CVI_U8 u8ImgMode;

	u8ImgMode = g_pastGc4683[ViPipe]->u8ImgMode;

	gc4683_i2c_init(ViPipe);
	if (u8ImgMode == GC4683_MODE_2560X1440P60) {
		gc4683_linear_1440p60_init(ViPipe);
	} else if (u8ImgMode == GC4683_MODE_1280X720P60) {
		gc4683_linear_720p120_init(ViPipe);
	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ImageMode[%d] invalid!\n", u8ImgMode);
	}

	g_pastGc4683[ViPipe]->bInit = CVI_TRUE;
}

static void gc4683_linear_1440p60_init(VI_PIPE ViPipe)
{
	gc4683_write_register(ViPipe, 0x03fe, 0xf0);
	gc4683_write_register(ViPipe, 0x03fe, 0x00);
	gc4683_write_register(ViPipe, 0x03fe, 0x10);
	gc4683_write_register(ViPipe, 0x0a38, 0x00);
	gc4683_write_register(ViPipe, 0x0a38, 0x05);
	gc4683_write_register(ViPipe, 0x0331, 0x07);
	gc4683_write_register(ViPipe, 0x0320, 0xf2);
	gc4683_write_register(ViPipe, 0x0a22, 0x04);
	gc4683_write_register(ViPipe, 0x0a27, 0x02);
	gc4683_write_register(ViPipe, 0x0a20, 0x1a);
	gc4683_write_register(ViPipe, 0x0a21, 0x1a);
	gc4683_write_register(ViPipe, 0x032b, 0x54);
	gc4683_write_register(ViPipe, 0x032a, 0x55);
	gc4683_write_register(ViPipe, 0x0a22, 0x17);
	gc4683_write_register(ViPipe, 0x0a23, 0x20);
	gc4683_write_register(ViPipe, 0x0a24, 0x0c);
	gc4683_write_register(ViPipe, 0x0a25, 0x50);
	gc4683_write_register(ViPipe, 0x0a34, 0x00);
	gc4683_write_register(ViPipe, 0x0a35, 0x60);
	gc4683_write_register(ViPipe, 0x0a36, 0x0c);
	gc4683_write_register(ViPipe, 0x0a37, 0x64);
	gc4683_write_register(ViPipe, 0x0a27, 0x0b);
	gc4683_write_register(ViPipe, 0x0a28, 0x24);
	gc4683_write_register(ViPipe, 0x0a29, 0x14);
	gc4683_write_register(ViPipe, 0x0a2a, 0xb0);
	gc4683_write_register(ViPipe, 0x031c, 0x46);
	gc4683_write_register(ViPipe, 0x0213, 0x1c);
	gc4683_write_register(ViPipe, 0x0219, 0xc7);
	gc4683_write_register(ViPipe, 0x0261, 0x98);
	gc4683_write_register(ViPipe, 0x0259, 0x06);
	gc4683_write_register(ViPipe, 0x025a, 0x18);
	gc4683_write_register(ViPipe, 0x0340, 0x06);
	gc4683_write_register(ViPipe, 0x0341, 0x40);
	gc4683_write_register(ViPipe, 0x0342, 0x02);
	gc4683_write_register(ViPipe, 0x0343, 0x26);
	gc4683_write_register(ViPipe, 0x0346, 0x00);
	gc4683_write_register(ViPipe, 0x0347, 0x30);
	gc4683_write_register(ViPipe, 0x0348, 0x0a);
	gc4683_write_register(ViPipe, 0x0349, 0x08);
	gc4683_write_register(ViPipe, 0x034a, 0x05);
	gc4683_write_register(ViPipe, 0x034b, 0xa8);
	gc4683_write_register(ViPipe, 0x034e, 0x0a);
	gc4683_write_register(ViPipe, 0x034f, 0xb0);
	gc4683_write_register(ViPipe, 0x0094, 0x0a);
	gc4683_write_register(ViPipe, 0x0095, 0x00);
	gc4683_write_register(ViPipe, 0x0096, 0x05);
	gc4683_write_register(ViPipe, 0x0097, 0xa0);
	gc4683_write_register(ViPipe, 0x0099, 0x04);
	gc4683_write_register(ViPipe, 0x009b, 0x04);
	gc4683_write_register(ViPipe, 0x070c, 0x00);
	gc4683_write_register(ViPipe, 0x070d, 0x0a);
	gc4683_write_register(ViPipe, 0x070e, 0x05);
	gc4683_write_register(ViPipe, 0x070f, 0x0e);
	gc4683_write_register(ViPipe, 0x0902, 0x0b);
	gc4683_write_register(ViPipe, 0x0903, 0x03);
	gc4683_write_register(ViPipe, 0x0904, 0x0a);
	gc4683_write_register(ViPipe, 0x0907, 0x35);
	gc4683_write_register(ViPipe, 0x0909, 0x07);
	gc4683_write_register(ViPipe, 0x090d, 0x0c);
	gc4683_write_register(ViPipe, 0x0276, 0x07);
	gc4683_write_register(ViPipe, 0x0277, 0xa4);
	gc4683_write_register(ViPipe, 0x0278, 0x3d);
	gc4683_write_register(ViPipe, 0x0279, 0x57);
	gc4683_write_register(ViPipe, 0x027b, 0x16);
	gc4683_write_register(ViPipe, 0x072a, 0x38);
	gc4683_write_register(ViPipe, 0x0724, 0x03);
	gc4683_write_register(ViPipe, 0x0727, 0x03);
	gc4683_write_register(ViPipe, 0x072a, 0x38);
	gc4683_write_register(ViPipe, 0x072b, 0x19);
	gc4683_write_register(ViPipe, 0x072f, 0x02);
	gc4683_write_register(ViPipe, 0x0002, 0x80);
	gc4683_write_register(ViPipe, 0x0004, 0x1f);
	gc4683_write_register(ViPipe, 0x0060, 0x40);
	gc4683_write_register(ViPipe, 0x0038, 0x40);
	gc4683_write_register(ViPipe, 0x0039, 0x40);
	gc4683_write_register(ViPipe, 0x003a, 0x40);
	gc4683_write_register(ViPipe, 0x003b, 0x40);
	gc4683_write_register(ViPipe, 0x02ac, 0x00);
	gc4683_write_register(ViPipe, 0x0274, 0x0a);
	gc4683_write_register(ViPipe, 0x02ad, 0x04);
	gc4683_write_register(ViPipe, 0x02ae, 0x01);
	gc4683_write_register(ViPipe, 0x0247, 0x00);
	gc4683_write_register(ViPipe, 0x0248, 0x00);
	gc4683_write_register(ViPipe, 0x1466, 0x20);
	gc4683_write_register(ViPipe, 0x1467, 0x24);
	gc4683_write_register(ViPipe, 0x1468, 0x24);
	gc4683_write_register(ViPipe, 0x1469, 0x02);
	gc4683_write_register(ViPipe, 0x146a, 0x40);
	gc4683_write_register(ViPipe, 0x146b, 0x00);
	gc4683_write_register(ViPipe, 0x0707, 0x08);
	gc4683_write_register(ViPipe, 0x0704, 0x00);
	gc4683_write_register(ViPipe, 0x0719, 0x00);
	gc4683_write_register(ViPipe, 0x071a, 0x40);
	gc4683_write_register(ViPipe, 0x021b, 0xb0);
	gc4683_write_register(ViPipe, 0x0006, 0x00);
	gc4683_write_register(ViPipe, 0x0216, 0x01);
	gc4683_write_register(ViPipe, 0x027c, 0x0f);
	gc4683_write_register(ViPipe, 0x1430, 0x00);
	gc4683_write_register(ViPipe, 0x1409, 0x03);
	gc4683_write_register(ViPipe, 0x143a, 0x03);
	gc4683_write_register(ViPipe, 0x1433, 0x80);
	gc4683_write_register(ViPipe, 0x140f, 0x21);
	gc4683_write_register(ViPipe, 0x1461, 0x20);
	gc4683_write_register(ViPipe, 0x1462, 0x20);
	gc4683_write_register(ViPipe, 0x146e, 0x40);
	gc4683_write_register(ViPipe, 0x146f, 0x02);
	gc4683_write_register(ViPipe, 0x1470, 0x3e);
	gc4683_write_register(ViPipe, 0x1471, 0x02);
	gc4683_write_register(ViPipe, 0x1474, 0x40);
	gc4683_write_register(ViPipe, 0x1479, 0x12);
	gc4683_write_register(ViPipe, 0x1485, 0x06);
	gc4683_write_register(ViPipe, 0x1475, 0x12);
	gc4683_write_register(ViPipe, 0x1476, 0xe0);
	gc4683_write_register(ViPipe, 0x14a1, 0x0d);
	gc4683_write_register(ViPipe, 0x14a8, 0x70);
	gc4683_write_register(ViPipe, 0x14a6, 0x30);
	gc4683_write_register(ViPipe, 0x1420, 0x14);
	gc4683_write_register(ViPipe, 0x1464, 0x15);
	gc4683_write_register(ViPipe, 0x146c, 0x08);
	gc4683_write_register(ViPipe, 0x146d, 0x08);
	gc4683_write_register(ViPipe, 0x1423, 0x08);
	gc4683_write_register(ViPipe, 0x1428, 0x40);
	gc4683_write_register(ViPipe, 0x0245, 0xd9);
	gc4683_write_register(ViPipe, 0x023a, 0x08);
	gc4683_write_register(ViPipe, 0x02cd, 0x42);
	gc4683_write_register(ViPipe, 0x0243, 0x04);
	gc4683_write_register(ViPipe, 0x029e, 0x3f);
	gc4683_write_register(ViPipe, 0x029d, 0x3c);
	gc4683_write_register(ViPipe, 0x0089, 0x03);
	gc4683_write_register(ViPipe, 0x0040, 0xa3);
	gc4683_write_register(ViPipe, 0x0075, 0x60);
	gc4683_write_register(ViPipe, 0x0004, 0x1f);
	gc4683_write_register(ViPipe, 0x0002, 0x82);
	gc4683_write_register(ViPipe, 0x0053, 0x00);
	gc4683_write_register(ViPipe, 0x0205, 0x0c);
	gc4683_write_register(ViPipe, 0x0317, 0x01);
	gc4683_write_register(ViPipe, 0x021a, 0x10);
	gc4683_write_register(ViPipe, 0x0076, 0x01);
	gc4683_write_register(ViPipe, 0x0054, 0x98);
	gc4683_write_register(ViPipe, 0x0042, 0x60);
	gc4683_write_register(ViPipe, 0x0052, 0x02);
	gc4683_write_register(ViPipe, 0x0046, 0x60);
	gc4683_write_register(ViPipe, 0x0448, 0x07);
	gc4683_write_register(ViPipe, 0x0449, 0x07);
	gc4683_write_register(ViPipe, 0x044a, 0x07);
	gc4683_write_register(ViPipe, 0x044b, 0x07);
	gc4683_write_register(ViPipe, 0x044c, 0x79);
	gc4683_write_register(ViPipe, 0x044d, 0x79);
	gc4683_write_register(ViPipe, 0x044e, 0x79);
	gc4683_write_register(ViPipe, 0x044f, 0x79);
	gc4683_write_register(ViPipe, 0x0468, 0x00);
	gc4683_write_register(ViPipe, 0x0010, 0x08);
	gc4683_write_register(ViPipe, 0x04b0, 0x30);
	gc4683_write_register(ViPipe, 0x04b1, 0x10);
	gc4683_write_register(ViPipe, 0x04c0, 0x20);
	gc4683_write_register(ViPipe, 0x04c1, 0x20);
	gc4683_write_register(ViPipe, 0x04d0, 0x20);
	gc4683_write_register(ViPipe, 0x04d1, 0x5f);
	gc4683_write_register(ViPipe, 0x04b2, 0x30);
	gc4683_write_register(ViPipe, 0x04b3, 0x10);
	gc4683_write_register(ViPipe, 0x04c2, 0x20);
	gc4683_write_register(ViPipe, 0x04c3, 0x20);
	gc4683_write_register(ViPipe, 0x04d2, 0x20);
	gc4683_write_register(ViPipe, 0x04d3, 0x5f);
	gc4683_write_register(ViPipe, 0x04b4, 0x30);
	gc4683_write_register(ViPipe, 0x04b5, 0x10);
	gc4683_write_register(ViPipe, 0x04c4, 0x20);
	gc4683_write_register(ViPipe, 0x04c5, 0x20);
	gc4683_write_register(ViPipe, 0x04d4, 0x20);
	gc4683_write_register(ViPipe, 0x04d5, 0x5f);
	gc4683_write_register(ViPipe, 0x04b6, 0x40);
	gc4683_write_register(ViPipe, 0x04b7, 0x10);
	gc4683_write_register(ViPipe, 0x04c6, 0x20);
	gc4683_write_register(ViPipe, 0x04c7, 0x20);
	gc4683_write_register(ViPipe, 0x04d6, 0x20);
	gc4683_write_register(ViPipe, 0x04d7, 0x5f);
	gc4683_write_register(ViPipe, 0x04b8, 0x40);
	gc4683_write_register(ViPipe, 0x04b9, 0x10);
	gc4683_write_register(ViPipe, 0x04c8, 0x20);
	gc4683_write_register(ViPipe, 0x04c9, 0x20);
	gc4683_write_register(ViPipe, 0x04d8, 0x20);
	gc4683_write_register(ViPipe, 0x04d9, 0x5f);
	gc4683_write_register(ViPipe, 0x04ba, 0x40);
	gc4683_write_register(ViPipe, 0x04bb, 0x10);
	gc4683_write_register(ViPipe, 0x04ca, 0x20);
	gc4683_write_register(ViPipe, 0x04cb, 0x20);
	gc4683_write_register(ViPipe, 0x04da, 0x20);
	gc4683_write_register(ViPipe, 0x04db, 0x5f);
	gc4683_write_register(ViPipe, 0x04bc, 0x40);
	gc4683_write_register(ViPipe, 0x04bd, 0x10);
	gc4683_write_register(ViPipe, 0x04cc, 0x20);
	gc4683_write_register(ViPipe, 0x04cd, 0x20);
	gc4683_write_register(ViPipe, 0x04dc, 0x20);
	gc4683_write_register(ViPipe, 0x04dd, 0x5f);
	gc4683_write_register(ViPipe, 0x04be, 0x50);
	gc4683_write_register(ViPipe, 0x04bf, 0x10);
	gc4683_write_register(ViPipe, 0x04ce, 0x20);
	gc4683_write_register(ViPipe, 0x04cf, 0x20);
	gc4683_write_register(ViPipe, 0x04de, 0x20);
	gc4683_write_register(ViPipe, 0x04df, 0x5f);
	gc4683_write_register(ViPipe, 0x0704, 0x07);
	gc4683_write_register(ViPipe, 0x0715, 0x04);
	gc4683_write_register(ViPipe, 0x0716, 0xb0);
	gc4683_write_register(ViPipe, 0x0718, 0xd0);
	gc4683_write_register(ViPipe, 0x071b, 0x00);
	gc4683_write_register(ViPipe, 0x071c, 0x40);
	gc4683_write_register(ViPipe, 0x071d, 0x00);
	gc4683_write_register(ViPipe, 0x071e, 0x40);
	gc4683_write_register(ViPipe, 0x031f, 0x02);
	gc4683_write_register(ViPipe, 0x031f, 0x00);
	gc4683_write_register(ViPipe, 0x0a67, 0x80);
	gc4683_write_register(ViPipe, 0x0a51, 0x41);
	gc4683_write_register(ViPipe, 0x0a52, 0x41);
	gc4683_write_register(ViPipe, 0x0a4e, 0x0c);
	gc4683_write_register(ViPipe, 0x0a4f, 0x0c);
	gc4683_write_register(ViPipe, 0x0a54, 0x36);
	gc4683_write_register(ViPipe, 0x0a55, 0x36);
	gc4683_write_register(ViPipe, 0x0a9f, 0x17);
	gc4683_write_register(ViPipe, 0x0a9e, 0x10);
	gc4683_write_register(ViPipe, 0x0aa1, 0x10);
	gc4683_write_register(ViPipe, 0x0a53, 0x00);
	gc4683_write_register(ViPipe, 0x05be, 0x00);
	gc4683_write_register(ViPipe, 0x05a9, 0x01);
	gc4683_write_register(ViPipe, 0x0028, 0x0a);
	gc4683_write_register(ViPipe, 0x0029, 0x08);
	gc4683_write_register(ViPipe, 0x002a, 0x05);
	gc4683_write_register(ViPipe, 0x002b, 0xa8);
	gc4683_write_register(ViPipe, 0x0022, 0x00);
	gc4683_write_register(ViPipe, 0x0023, 0x00);
	gc4683_write_register(ViPipe, 0x0024, 0x00);
	gc4683_write_register(ViPipe, 0x0025, 0x00);
	gc4683_write_register(ViPipe, 0x0a70, 0x03);
	gc4683_write_register(ViPipe, 0x0a71, 0x02);
	gc4683_write_register(ViPipe, 0x0a73, 0x60);
	gc4683_write_register(ViPipe, 0x0a82, 0x01);
	gc4683_write_register(ViPipe, 0x0a83, 0x10);
	gc4683_write_register(ViPipe, 0x0a5a, 0x80);
	gc4683_write_register(ViPipe, 0x0313, 0x80);
	delay_ms(20);
	gc4683_write_register(ViPipe, 0x05be, 0x01);
	gc4683_write_register(ViPipe, 0x0080, 0x02);
	gc4683_write_register(ViPipe, 0x0021, 0x40);
	gc4683_write_register(ViPipe, 0x0020, 0x8c);
	gc4683_write_register(ViPipe, 0x0202, 0x01);
	gc4683_write_register(ViPipe, 0x0203, 0x00);
	gc4683_write_register(ViPipe, 0x02aa, 0x00);
	gc4683_write_register(ViPipe, 0x02ab, 0x00);
	gc4683_write_register(ViPipe, 0x0181, 0xf0);
	gc4683_write_register(ViPipe, 0x0185, 0x08);
	gc4683_write_register(ViPipe, 0x0111, 0x2b);
	gc4683_write_register(ViPipe, 0x0180, 0x46);
	gc4683_write_register(ViPipe, 0x02ce, 0x45);
	gc4683_write_register(ViPipe, 0x0100, 0x09);
	gc4683_write_register(ViPipe, 0x0106, 0x38);
	gc4683_write_register(ViPipe, 0x010d, 0x0c);
	gc4683_write_register(ViPipe, 0x010e, 0x80);
	gc4683_write_register(ViPipe, 0x0112, 0x01);
	gc4683_write_register(ViPipe, 0x0114, 0x03);
	gc4683_write_register(ViPipe, 0x0115, 0x10);
	gc4683_write_register(ViPipe, 0x0125, 0x20);
	gc4683_write_register(ViPipe, 0x0124, 0x02);
	gc4683_write_register(ViPipe, 0x0122, 0x06);
	gc4683_write_register(ViPipe, 0x0123, 0x20);
	gc4683_write_register(ViPipe, 0x0126, 0x08);
	gc4683_write_register(ViPipe, 0x0121, 0x10);
	gc4683_write_register(ViPipe, 0x0129, 0x08);
	gc4683_write_register(ViPipe, 0x012a, 0x0a);
	gc4683_write_register(ViPipe, 0x012b, 0x08);
	gc4683_write_register(ViPipe, 0x03fe, 0x00);

	gc4683_default_reg_init(ViPipe);
	delay_ms(10);

	printf("ViPipe:%d,===GC4683 1440P 60fps 10bit LINEAR Init OK!===\n", ViPipe);
}
static void gc4683_linear_720p120_init(iPipe)
{
	gc4683_write_register(ViPipe, 0x03fe, 0xf0);
	gc4683_write_register(ViPipe, 0x03fe, 0x00);
	gc4683_write_register(ViPipe, 0x03fe, 0x10);
	gc4683_write_register(ViPipe, 0x0a38, 0x00);
	gc4683_write_register(ViPipe, 0x0a38, 0x05);
	gc4683_write_register(ViPipe, 0x0331, 0x07);
	gc4683_write_register(ViPipe, 0x0320, 0xf2);
	gc4683_write_register(ViPipe, 0x0a22, 0x04);
	gc4683_write_register(ViPipe, 0x0a27, 0x02);
	gc4683_write_register(ViPipe, 0x0a20, 0x1a);
	gc4683_write_register(ViPipe, 0x0a21, 0x1a);
	gc4683_write_register(ViPipe, 0x032b, 0x54);
	gc4683_write_register(ViPipe, 0x032a, 0x55);
	gc4683_write_register(ViPipe, 0x0a22, 0x17);
	gc4683_write_register(ViPipe, 0x0a23, 0x20);
	gc4683_write_register(ViPipe, 0x0a24, 0x0c);
	gc4683_write_register(ViPipe, 0x0a25, 0x50);
	gc4683_write_register(ViPipe, 0x0a34, 0x00);
	gc4683_write_register(ViPipe, 0x0a35, 0x61);
	gc4683_write_register(ViPipe, 0x0a36, 0x0c);
	gc4683_write_register(ViPipe, 0x0a37, 0x64);
	gc4683_write_register(ViPipe, 0x0a27, 0x0b);
	gc4683_write_register(ViPipe, 0x0a28, 0x24);
	gc4683_write_register(ViPipe, 0x0a29, 0x14);
	gc4683_write_register(ViPipe, 0x0a2a, 0xb0);
	gc4683_write_register(ViPipe, 0x031c, 0x46);
	gc4683_write_register(ViPipe, 0x0213, 0x1c);
	gc4683_write_register(ViPipe, 0x0219, 0xc7);
	gc4683_write_register(ViPipe, 0x0261, 0x98);
	gc4683_write_register(ViPipe, 0x0259, 0x03);
	gc4683_write_register(ViPipe, 0x025a, 0x00);
	gc4683_write_register(ViPipe, 0x0340, 0x03);
	gc4683_write_register(ViPipe, 0x0341, 0x20);
	gc4683_write_register(ViPipe, 0x0342, 0x02);
	gc4683_write_register(ViPipe, 0x0343, 0x26);
	gc4683_write_register(ViPipe, 0x0346, 0x00);
	gc4683_write_register(ViPipe, 0x0347, 0x30);
	gc4683_write_register(ViPipe, 0x0348, 0x0a);
	gc4683_write_register(ViPipe, 0x0349, 0x08);
	gc4683_write_register(ViPipe, 0x034a, 0x05);
	gc4683_write_register(ViPipe, 0x034b, 0xa8);
	gc4683_write_register(ViPipe, 0x034e, 0x0a);
	gc4683_write_register(ViPipe, 0x034f, 0xb0);
	gc4683_write_register(ViPipe, 0x0094, 0x05);
	gc4683_write_register(ViPipe, 0x0095, 0x00);
	gc4683_write_register(ViPipe, 0x0096, 0x02);
	gc4683_write_register(ViPipe, 0x0097, 0xd0);
	gc4683_write_register(ViPipe, 0x0218, 0x14);
	gc4683_write_register(ViPipe, 0x0273, 0x03);
	gc4683_write_register(ViPipe, 0x0077, 0x08);
	gc4683_write_register(ViPipe, 0x0099, 0x02);
	gc4683_write_register(ViPipe, 0x009b, 0x02);
	gc4683_write_register(ViPipe, 0x070c, 0x00);
	gc4683_write_register(ViPipe, 0x070d, 0x0a);
	gc4683_write_register(ViPipe, 0x070e, 0x05);
	gc4683_write_register(ViPipe, 0x070f, 0x0e);
	gc4683_write_register(ViPipe, 0x0902, 0x0b);
	gc4683_write_register(ViPipe, 0x0903, 0x03);
	gc4683_write_register(ViPipe, 0x0904, 0x0a);
	gc4683_write_register(ViPipe, 0x0907, 0x35);
	gc4683_write_register(ViPipe, 0x0909, 0x07);
	gc4683_write_register(ViPipe, 0x090d, 0x0c);
	gc4683_write_register(ViPipe, 0x0276, 0x07);
	gc4683_write_register(ViPipe, 0x0277, 0xa4);
	gc4683_write_register(ViPipe, 0x0278, 0x3d);
	gc4683_write_register(ViPipe, 0x0279, 0x57);
	gc4683_write_register(ViPipe, 0x027b, 0x16);
	gc4683_write_register(ViPipe, 0x072a, 0x38);
	gc4683_write_register(ViPipe, 0x0724, 0x03);
	gc4683_write_register(ViPipe, 0x0727, 0x03);
	gc4683_write_register(ViPipe, 0x072a, 0x38);
	gc4683_write_register(ViPipe, 0x072b, 0x19);
	gc4683_write_register(ViPipe, 0x072f, 0x02);
	gc4683_write_register(ViPipe, 0x0002, 0x80);
	gc4683_write_register(ViPipe, 0x0004, 0x1f);
	gc4683_write_register(ViPipe, 0x0060, 0x40);
	gc4683_write_register(ViPipe, 0x0038, 0x40);
	gc4683_write_register(ViPipe, 0x0039, 0x40);
	gc4683_write_register(ViPipe, 0x003a, 0x40);
	gc4683_write_register(ViPipe, 0x003b, 0x40);
	gc4683_write_register(ViPipe, 0x02ac, 0x00);
	gc4683_write_register(ViPipe, 0x0274, 0x0a);
	gc4683_write_register(ViPipe, 0x02ad, 0x04);
	gc4683_write_register(ViPipe, 0x02ae, 0x01);
	gc4683_write_register(ViPipe, 0x0247, 0x00);
	gc4683_write_register(ViPipe, 0x0248, 0x00);
	gc4683_write_register(ViPipe, 0x1466, 0x20);
	gc4683_write_register(ViPipe, 0x1467, 0x24);
	gc4683_write_register(ViPipe, 0x1468, 0x24);
	gc4683_write_register(ViPipe, 0x1469, 0x02);
	gc4683_write_register(ViPipe, 0x146a, 0x40);
	gc4683_write_register(ViPipe, 0x146b, 0x00);
	gc4683_write_register(ViPipe, 0x0707, 0x08);
	gc4683_write_register(ViPipe, 0x0704, 0x00);
	gc4683_write_register(ViPipe, 0x0719, 0x00);
	gc4683_write_register(ViPipe, 0x071a, 0x40);
	gc4683_write_register(ViPipe, 0x021b, 0xb0);
	gc4683_write_register(ViPipe, 0x0006, 0x00);
	gc4683_write_register(ViPipe, 0x0216, 0x01);
	gc4683_write_register(ViPipe, 0x027c, 0x0f);
	gc4683_write_register(ViPipe, 0x1430, 0x00);
	gc4683_write_register(ViPipe, 0x1409, 0x03);
	gc4683_write_register(ViPipe, 0x143a, 0x03);
	gc4683_write_register(ViPipe, 0x1433, 0x80);
	gc4683_write_register(ViPipe, 0x140f, 0x21);
	gc4683_write_register(ViPipe, 0x1461, 0x20);
	gc4683_write_register(ViPipe, 0x1462, 0x20);
	gc4683_write_register(ViPipe, 0x146e, 0x40);
	gc4683_write_register(ViPipe, 0x146f, 0x02);
	gc4683_write_register(ViPipe, 0x1470, 0x3e);
	gc4683_write_register(ViPipe, 0x1471, 0x02);
	gc4683_write_register(ViPipe, 0x1474, 0x40);
	gc4683_write_register(ViPipe, 0x1479, 0x12);
	gc4683_write_register(ViPipe, 0x1485, 0x06);
	gc4683_write_register(ViPipe, 0x1475, 0x12);
	gc4683_write_register(ViPipe, 0x1476, 0xe0);
	gc4683_write_register(ViPipe, 0x14a1, 0x0d);
	gc4683_write_register(ViPipe, 0x14a8, 0x70);
	gc4683_write_register(ViPipe, 0x14a6, 0x30);
	gc4683_write_register(ViPipe, 0x1420, 0x14);
	gc4683_write_register(ViPipe, 0x1464, 0x15);
	gc4683_write_register(ViPipe, 0x146c, 0x08);
	gc4683_write_register(ViPipe, 0x146d, 0x08);
	gc4683_write_register(ViPipe, 0x1423, 0x08);
	gc4683_write_register(ViPipe, 0x1428, 0x40);
	gc4683_write_register(ViPipe, 0x0245, 0xd9);
	gc4683_write_register(ViPipe, 0x023a, 0x04);
	gc4683_write_register(ViPipe, 0x02cd, 0x42);
	gc4683_write_register(ViPipe, 0x0243, 0x04);
	gc4683_write_register(ViPipe, 0x029e, 0x3f);
	gc4683_write_register(ViPipe, 0x029d, 0x3c);
	gc4683_write_register(ViPipe, 0x0089, 0x03);
	gc4683_write_register(ViPipe, 0x0040, 0xa3);
	gc4683_write_register(ViPipe, 0x0075, 0x60);
	gc4683_write_register(ViPipe, 0x0004, 0x1f);
	gc4683_write_register(ViPipe, 0x0002, 0x82);
	gc4683_write_register(ViPipe, 0x0053, 0x00);
	gc4683_write_register(ViPipe, 0x0205, 0x0c);
	gc4683_write_register(ViPipe, 0x0317, 0x0f);
	gc4683_write_register(ViPipe, 0x021a, 0x10);
	gc4683_write_register(ViPipe, 0x0076, 0x01);
	gc4683_write_register(ViPipe, 0x0054, 0x98);
	gc4683_write_register(ViPipe, 0x0042, 0x60);
	gc4683_write_register(ViPipe, 0x0052, 0x02);
	gc4683_write_register(ViPipe, 0x0046, 0x60);
	gc4683_write_register(ViPipe, 0x0448, 0x09);
	gc4683_write_register(ViPipe, 0x0449, 0x09);
	gc4683_write_register(ViPipe, 0x044a, 0x09);
	gc4683_write_register(ViPipe, 0x044b, 0x09);
	gc4683_write_register(ViPipe, 0x044c, 0x77);
	gc4683_write_register(ViPipe, 0x044d, 0x77);
	gc4683_write_register(ViPipe, 0x044e, 0x77);
	gc4683_write_register(ViPipe, 0x044f, 0x77);
	gc4683_write_register(ViPipe, 0x0010, 0x08);
	gc4683_write_register(ViPipe, 0x04b0, 0x30);
	gc4683_write_register(ViPipe, 0x04b1, 0x10);
	gc4683_write_register(ViPipe, 0x04c0, 0x20);
	gc4683_write_register(ViPipe, 0x04c1, 0x20);
	gc4683_write_register(ViPipe, 0x04d0, 0x20);
	gc4683_write_register(ViPipe, 0x04d1, 0x3f);
	gc4683_write_register(ViPipe, 0x04b2, 0x30);
	gc4683_write_register(ViPipe, 0x04b3, 0x10);
	gc4683_write_register(ViPipe, 0x04c2, 0x20);
	gc4683_write_register(ViPipe, 0x04c3, 0x20);
	gc4683_write_register(ViPipe, 0x04d2, 0x20);
	gc4683_write_register(ViPipe, 0x04d3, 0x3f);
	gc4683_write_register(ViPipe, 0x04b4, 0x30);
	gc4683_write_register(ViPipe, 0x04b5, 0x10);
	gc4683_write_register(ViPipe, 0x04c4, 0x20);
	gc4683_write_register(ViPipe, 0x04c5, 0x20);
	gc4683_write_register(ViPipe, 0x04d4, 0x20);
	gc4683_write_register(ViPipe, 0x04d5, 0x3f);
	gc4683_write_register(ViPipe, 0x04b6, 0x40);
	gc4683_write_register(ViPipe, 0x04b7, 0x10);
	gc4683_write_register(ViPipe, 0x04c6, 0x20);
	gc4683_write_register(ViPipe, 0x04c7, 0x20);
	gc4683_write_register(ViPipe, 0x04d6, 0x20);
	gc4683_write_register(ViPipe, 0x04d7, 0x08);
	gc4683_write_register(ViPipe, 0x04b8, 0x40);
	gc4683_write_register(ViPipe, 0x04b9, 0x10);
	gc4683_write_register(ViPipe, 0x04c8, 0x20);
	gc4683_write_register(ViPipe, 0x04c9, 0x20);
	gc4683_write_register(ViPipe, 0x04d8, 0x20);
	gc4683_write_register(ViPipe, 0x04d9, 0x08);
	gc4683_write_register(ViPipe, 0x04ba, 0x40);
	gc4683_write_register(ViPipe, 0x04bb, 0x10);
	gc4683_write_register(ViPipe, 0x04ca, 0x20);
	gc4683_write_register(ViPipe, 0x04cb, 0x20);
	gc4683_write_register(ViPipe, 0x04da, 0x20);
	gc4683_write_register(ViPipe, 0x04db, 0x08);
	gc4683_write_register(ViPipe, 0x04bc, 0x40);
	gc4683_write_register(ViPipe, 0x04bd, 0x10);
	gc4683_write_register(ViPipe, 0x04cc, 0x20);
	gc4683_write_register(ViPipe, 0x04cd, 0x20);
	gc4683_write_register(ViPipe, 0x04dc, 0x20);
	gc4683_write_register(ViPipe, 0x04dd, 0x08);
	gc4683_write_register(ViPipe, 0x04be, 0x50);
	gc4683_write_register(ViPipe, 0x04bf, 0x10);
	gc4683_write_register(ViPipe, 0x04ce, 0x20);
	gc4683_write_register(ViPipe, 0x04cf, 0x20);
	gc4683_write_register(ViPipe, 0x04de, 0x20);
	gc4683_write_register(ViPipe, 0x04df, 0x08);
	gc4683_write_register(ViPipe, 0x0704, 0x07);
	gc4683_write_register(ViPipe, 0x0715, 0x04);
	gc4683_write_register(ViPipe, 0x0716, 0xb0);
	gc4683_write_register(ViPipe, 0x0718, 0xd0);
	gc4683_write_register(ViPipe, 0x071b, 0x00);
	gc4683_write_register(ViPipe, 0x071c, 0x40);
	gc4683_write_register(ViPipe, 0x071d, 0x00);
	gc4683_write_register(ViPipe, 0x071e, 0x40);
	gc4683_write_register(ViPipe, 0x031f, 0x02);
	gc4683_write_register(ViPipe, 0x031f, 0x00);
	gc4683_write_register(ViPipe, 0x0a67, 0x80);
	gc4683_write_register(ViPipe, 0x0a51, 0x41);
	gc4683_write_register(ViPipe, 0x0a52, 0x41);
	gc4683_write_register(ViPipe, 0x0a4e, 0x0c);
	gc4683_write_register(ViPipe, 0x0a4f, 0x0c);
	gc4683_write_register(ViPipe, 0x0a54, 0x36);
	gc4683_write_register(ViPipe, 0x0a55, 0x36);
	gc4683_write_register(ViPipe, 0x0a9f, 0x17);
	gc4683_write_register(ViPipe, 0x0a9e, 0x10);
	gc4683_write_register(ViPipe, 0x0aa1, 0x10);
	gc4683_write_register(ViPipe, 0x0a53, 0x00);
	gc4683_write_register(ViPipe, 0x05be, 0x00);
	gc4683_write_register(ViPipe, 0x05a9, 0x01);
	gc4683_write_register(ViPipe, 0x0028, 0x0a);
	gc4683_write_register(ViPipe, 0x0029, 0x08);
	gc4683_write_register(ViPipe, 0x002a, 0x05);
	gc4683_write_register(ViPipe, 0x002b, 0xa8);
	gc4683_write_register(ViPipe, 0x0022, 0x00);
	gc4683_write_register(ViPipe, 0x0023, 0x00);
	gc4683_write_register(ViPipe, 0x0024, 0x00);
	gc4683_write_register(ViPipe, 0x0025, 0x00);
	gc4683_write_register(ViPipe, 0x0a70, 0x03);
	gc4683_write_register(ViPipe, 0x0a71, 0x02);
	gc4683_write_register(ViPipe, 0x0a73, 0x60);
	gc4683_write_register(ViPipe, 0x0a82, 0x01);
	gc4683_write_register(ViPipe, 0x0a83, 0x10);
	gc4683_write_register(ViPipe, 0x0a5a, 0x80);
	gc4683_write_register(ViPipe, 0x0313, 0x80);
	delay_ms(20);
	gc4683_write_register(ViPipe, 0x05be, 0x01);
	gc4683_write_register(ViPipe, 0x0080, 0x02);
	gc4683_write_register(ViPipe, 0x0021, 0x40);
	gc4683_write_register(ViPipe, 0x0020, 0x8c);
	gc4683_write_register(ViPipe, 0x0202, 0x01);
	gc4683_write_register(ViPipe, 0x0203, 0x00);
	gc4683_write_register(ViPipe, 0x02aa, 0x00);
	gc4683_write_register(ViPipe, 0x02ab, 0x00);
	gc4683_write_register(ViPipe, 0x0181, 0xf0);
	gc4683_write_register(ViPipe, 0x0185, 0x08);
	gc4683_write_register(ViPipe, 0x0111, 0x2b);
	gc4683_write_register(ViPipe, 0x0180, 0x46);
	gc4683_write_register(ViPipe, 0x02ce, 0x42);
	gc4683_write_register(ViPipe, 0x0100, 0x09);
	gc4683_write_register(ViPipe, 0x0106, 0x38);
	gc4683_write_register(ViPipe, 0x010d, 0x06);
	gc4683_write_register(ViPipe, 0x010e, 0x40);
	gc4683_write_register(ViPipe, 0x0112, 0x01);
	gc4683_write_register(ViPipe, 0x0114, 0x03);
	gc4683_write_register(ViPipe, 0x0115, 0x10);
	gc4683_write_register(ViPipe, 0x0125, 0x20);
	gc4683_write_register(ViPipe, 0x0124, 0x02);
	gc4683_write_register(ViPipe, 0x0122, 0x02);
	gc4683_write_register(ViPipe, 0x0123, 0x10);
	gc4683_write_register(ViPipe, 0x0126, 0x04);
	gc4683_write_register(ViPipe, 0x0121, 0x10);
	gc4683_write_register(ViPipe, 0x0129, 0x03);
	gc4683_write_register(ViPipe, 0x012a, 0x0a);
	gc4683_write_register(ViPipe, 0x012b, 0x04);
	gc4683_write_register(ViPipe, 0x03fe, 0x00);

	gc4683_default_reg_init(ViPipe);
	delay_ms(10);

	printf("ViPipe:%d,===GC4683 720P 120fps 10bit LINEAR Init OK!===\n", ViPipe);
}

#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "imx135_cmos_ex.h"
#include "sensor_i2c.h"

static void imx135_linear_8M25_init(VI_PIPE ViPipe);


CVI_U8 imx135_i2c_addr = 0x1a;
const CVI_U32 imx135_addr_byte = 2;
const CVI_U32 imx135_data_byte = 1;

int imx135_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunImx135_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx135_AddrInfo[ViPipe].s8I2cAddr);
}

int imx135_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunImx135_BusInfo[ViPipe].s8I2cDev);
}

int imx135_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunImx135_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx135_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							imx135_addr_byte, imx135_data_byte);
}

int imx135_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunImx135_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx135_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							imx135_addr_byte, (CVI_U32)data, imx135_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void imx135_standby(VI_PIPE ViPipe)
{
	imx135_write_register(ViPipe, 0x3000, 0x01); /* STANDBY */
	imx135_write_register(ViPipe, 0x3002, 0x01); /* XTMSTA */
}

void imx135_restart(VI_PIPE ViPipe)
{
	imx135_write_register(ViPipe, 0x3000, 0x00); /* standby */
	delay_ms(20);
	imx135_write_register(ViPipe, 0x3002, 0x00); /* master mode start */
}

void imx135_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastImx135[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		imx135_write_register(ViPipe,
				g_pastImx135[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastImx135[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void imx135_init(VI_PIPE ViPipe)
{
	WDR_MODE_E        enWDRMode;
	CVI_U8            u8ImgMode;

	enWDRMode   = g_pastImx135[ViPipe]->enWDRMode;
	u8ImgMode   = g_pastImx135[ViPipe]->u8ImgMode;

	imx135_i2c_init(ViPipe);
	if (enWDRMode == WDR_MODE_2To1_LINE) {

	} else {
		if (u8ImgMode == IMX135_MODE_8M25)
			imx135_linear_8M25_init(ViPipe);
		else {
		}
	}
	g_pastImx135[ViPipe]->bInit = CVI_TRUE;
}
#define IMX135_CHIP_ID_HI_ADDR		0x0016
#define IMX135_CHIP_ID_LO_ADDR		0x0017
#define IMX135_CHIP_ID			0x135

int imx135_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	if (imx135_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	delay_ms(5);

	nVal = imx135_read_register(ViPipe, IMX135_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = imx135_read_register(ViPipe, IMX135_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != IMX135_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static void imx135_linear_8M25_init(VI_PIPE ViPipe)
{
	// MCLK:24Mhz, MIPI DATA RATE:728Mbps, HTS:4572, VTS:3056, PIXEL CLK:176Mhz X 2CH
	imx135_write_register(ViPipe, 0x0100, 0x00);
	imx135_write_register(ViPipe, 0x0101, 0x00);
	imx135_write_register(ViPipe, 0x0105, 0x01);
	imx135_write_register(ViPipe, 0x0110, 0x00);
	imx135_write_register(ViPipe, 0x0220, 0x01);
	imx135_write_register(ViPipe, 0x3302, 0x11);
	imx135_write_register(ViPipe, 0x3833, 0x20);
	imx135_write_register(ViPipe, 0x3893, 0x00);
	imx135_write_register(ViPipe, 0x3906, 0x08);
	imx135_write_register(ViPipe, 0x3907, 0x01);
	imx135_write_register(ViPipe, 0x391B, 0x01);
	imx135_write_register(ViPipe, 0x3C09, 0x01);
	imx135_write_register(ViPipe, 0x600A, 0x00);
	imx135_write_register(ViPipe, 0x3008, 0xB0);
	imx135_write_register(ViPipe, 0x320A, 0x01);
	imx135_write_register(ViPipe, 0x320D, 0x10);
	imx135_write_register(ViPipe, 0x3216, 0x2E);
	imx135_write_register(ViPipe, 0x322C, 0x02);
	imx135_write_register(ViPipe, 0x3409, 0x0C);
	imx135_write_register(ViPipe, 0x340C, 0x2D);
	imx135_write_register(ViPipe, 0x3411, 0x39);
	imx135_write_register(ViPipe, 0x3414, 0x1E);
	imx135_write_register(ViPipe, 0x3427, 0x04);
	imx135_write_register(ViPipe, 0x3480, 0x1E);
	imx135_write_register(ViPipe, 0x3484, 0x1E);
	imx135_write_register(ViPipe, 0x3488, 0x1E);
	imx135_write_register(ViPipe, 0x348C, 0x1E);
	imx135_write_register(ViPipe, 0x3490, 0x1E);
	imx135_write_register(ViPipe, 0x3494, 0x1E);
	imx135_write_register(ViPipe, 0x3511, 0x8F);
	imx135_write_register(ViPipe, 0x364F, 0x2D);
	imx135_write_register(ViPipe, 0x011E, 0x18);
	imx135_write_register(ViPipe, 0x011F, 0x00);
	imx135_write_register(ViPipe, 0x0301, 0x05);
	imx135_write_register(ViPipe, 0x0303, 0x01);
	imx135_write_register(ViPipe, 0x0304, 0x01);
	imx135_write_register(ViPipe, 0x0305, 0x0C);
	imx135_write_register(ViPipe, 0x0307, 0x4F);
	imx135_write_register(ViPipe, 0x0309, 0x05);
	imx135_write_register(ViPipe, 0x030B, 0x01);
	imx135_write_register(ViPipe, 0x030C, 0x01);
	imx135_write_register(ViPipe, 0x030D, 0xB8);
	imx135_write_register(ViPipe, 0x030E, 0x01);
	imx135_write_register(ViPipe, 0x3A06, 0x11);
	imx135_write_register(ViPipe, 0x0108, 0x03);
	imx135_write_register(ViPipe, 0x0112, 0x0A);
	imx135_write_register(ViPipe, 0x0113, 0x0A);
	imx135_write_register(ViPipe, 0x0381, 0x01);
	imx135_write_register(ViPipe, 0x0383, 0x01);
	imx135_write_register(ViPipe, 0x0385, 0x01);
	imx135_write_register(ViPipe, 0x0387, 0x01);
	imx135_write_register(ViPipe, 0x0390, 0x00);
	imx135_write_register(ViPipe, 0x0391, 0x11);
	imx135_write_register(ViPipe, 0x0392, 0x00);
	imx135_write_register(ViPipe, 0x0401, 0x00);
	imx135_write_register(ViPipe, 0x0404, 0x00);
	imx135_write_register(ViPipe, 0x0405, 0x10);
	imx135_write_register(ViPipe, 0x4082, 0x01);
	imx135_write_register(ViPipe, 0x4083, 0x01);
	imx135_write_register(ViPipe, 0x7006, 0x04);
	imx135_write_register(ViPipe, 0x0700, 0x00);
	imx135_write_register(ViPipe, 0x3A63, 0x00);
	imx135_write_register(ViPipe, 0x4100, 0xF8);
	imx135_write_register(ViPipe, 0x4203, 0xFF);
	imx135_write_register(ViPipe, 0x4344, 0x00);
	imx135_write_register(ViPipe, 0x441C, 0x01);
	imx135_write_register(ViPipe, 0x0340, 0x0C);
	imx135_write_register(ViPipe, 0x0341, 0x06);
	imx135_write_register(ViPipe, 0x0342, 0x11);
	imx135_write_register(ViPipe, 0x0343, 0xDC);
	imx135_write_register(ViPipe, 0x0344, 0x00);
	imx135_write_register(ViPipe, 0x0345, 0xB4);
	imx135_write_register(ViPipe, 0x0346, 0x01);
	imx135_write_register(ViPipe, 0x0347, 0xDC);
	imx135_write_register(ViPipe, 0x0348, 0x0F);
	imx135_write_register(ViPipe, 0x0349, 0xBB);
	imx135_write_register(ViPipe, 0x034A, 0x0A);
	imx135_write_register(ViPipe, 0x034B, 0x53);
	imx135_write_register(ViPipe, 0x034C, 0x0F);
	imx135_write_register(ViPipe, 0x034D, 0x08);
	imx135_write_register(ViPipe, 0x034E, 0x08);
	imx135_write_register(ViPipe, 0x034F, 0x78);
	imx135_write_register(ViPipe, 0x0350, 0x00);
	imx135_write_register(ViPipe, 0x0351, 0x00);
	imx135_write_register(ViPipe, 0x0352, 0x00);
	imx135_write_register(ViPipe, 0x0353, 0x00);
	imx135_write_register(ViPipe, 0x0354, 0x0F);
	imx135_write_register(ViPipe, 0x0355, 0x08);
	imx135_write_register(ViPipe, 0x0356, 0x08);
	imx135_write_register(ViPipe, 0x0357, 0x78);
	imx135_write_register(ViPipe, 0x301D, 0x30);
	imx135_write_register(ViPipe, 0x3310, 0x10);
	imx135_write_register(ViPipe, 0x3311, 0x70);
	imx135_write_register(ViPipe, 0x3312, 0x0C);
	imx135_write_register(ViPipe, 0x3313, 0x30);
	imx135_write_register(ViPipe, 0x3314, 0x00);
	imx135_write_register(ViPipe, 0x331C, 0x01);
	imx135_write_register(ViPipe, 0x331D, 0x68);
	imx135_write_register(ViPipe, 0x4084, 0x00);
	imx135_write_register(ViPipe, 0x4085, 0x00);
	imx135_write_register(ViPipe, 0x4086, 0x00);
	imx135_write_register(ViPipe, 0x4087, 0x00);
	imx135_write_register(ViPipe, 0x4400, 0x00);
	imx135_write_register(ViPipe, 0x0830, 0x87);
	imx135_write_register(ViPipe, 0x0831, 0x3F);
	imx135_write_register(ViPipe, 0x0832, 0x67);
	imx135_write_register(ViPipe, 0x0833, 0x3F);
	imx135_write_register(ViPipe, 0x0834, 0x3F);
	imx135_write_register(ViPipe, 0x0835, 0x4F);
	imx135_write_register(ViPipe, 0x0836, 0xDF);
	imx135_write_register(ViPipe, 0x0837, 0x47);
	imx135_write_register(ViPipe, 0x0839, 0x1F);
	imx135_write_register(ViPipe, 0x083A, 0x17);
	imx135_write_register(ViPipe, 0x083B, 0x02);
	imx135_write_register(ViPipe, 0x0202, 0x09);
	imx135_write_register(ViPipe, 0x0203, 0xC0);
	imx135_write_register(ViPipe, 0x0205, 0x00);
	imx135_write_register(ViPipe, 0x020E, 0x01);
	imx135_write_register(ViPipe, 0x020F, 0x00);
	imx135_write_register(ViPipe, 0x0210, 0x01);
	imx135_write_register(ViPipe, 0x0211, 0x00);
	imx135_write_register(ViPipe, 0x0212, 0x01);
	imx135_write_register(ViPipe, 0x0213, 0x00);
	imx135_write_register(ViPipe, 0x0214, 0x01);
	imx135_write_register(ViPipe, 0x0215, 0x00);
	imx135_write_register(ViPipe, 0x0230, 0x00);
	imx135_write_register(ViPipe, 0x0231, 0x00);
	imx135_write_register(ViPipe, 0x0233, 0x00);
	imx135_write_register(ViPipe, 0x0234, 0x00);
	imx135_write_register(ViPipe, 0x0235, 0x40);
	imx135_write_register(ViPipe, 0x0238, 0x01);
	imx135_write_register(ViPipe, 0x0239, 0x04);
	imx135_write_register(ViPipe, 0x023B, 0x00);
	imx135_write_register(ViPipe, 0x023C, 0x01);
	imx135_write_register(ViPipe, 0x33B0, 0x04);
	imx135_write_register(ViPipe, 0x33B1, 0x00);
	imx135_write_register(ViPipe, 0x33B3, 0x00);
	imx135_write_register(ViPipe, 0x33B4, 0x01);
	imx135_write_register(ViPipe, 0x3800, 0x00);
	imx135_write_register(ViPipe, 0x3A43, 0x01);
	imx135_write_register(ViPipe, 0x380A, 0x01);
	imx135_write_register(ViPipe, 0x380B, 0x01);
	imx135_write_register(ViPipe, 0x4103, 0x01);
	delay_ms(4);
	imx135_write_register(ViPipe, 0x0100, 0x01);

	imx135_default_reg_init(ViPipe);

	printf("ViPipe:%d,===IMX135 8M 25fps 10bit LINEAR Init OK!===\n", ViPipe);
}

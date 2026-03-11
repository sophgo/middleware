#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "imx376_cmos_ex.h"
#include "sensor_i2c.h"

static void imx376_linear_5M30_init(VI_PIPE ViPipe);


CVI_U8 imx376_i2c_addr = 0x1a;
const CVI_U32 imx376_addr_byte = 2;
const CVI_U32 imx376_data_byte = 1;

int imx376_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunImx376_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx376_AddrInfo[ViPipe].s8I2cAddr);
}

int imx376_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunImx376_BusInfo[ViPipe].s8I2cDev);
}

int imx376_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunImx376_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx376_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							imx376_addr_byte, imx376_data_byte);
}

int imx376_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunImx376_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx376_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							imx376_addr_byte, (CVI_U32)data, imx376_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void imx376_standby(VI_PIPE ViPipe)
{
	imx376_write_register(ViPipe, 0x3000, 0x01); /* STANDBY */
	imx376_write_register(ViPipe, 0x3002, 0x01); /* XTMSTA */
}

void imx376_restart(VI_PIPE ViPipe)
{
	imx376_write_register(ViPipe, 0x3000, 0x00); /* standby */
	delay_ms(20);
	imx376_write_register(ViPipe, 0x3002, 0x00); /* master mode start */
}

void imx376_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastImx376[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		imx376_write_register(ViPipe,
				g_pastImx376[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastImx376[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void imx376_init(VI_PIPE ViPipe)
{
	WDR_MODE_E        enWDRMode;
	CVI_U8            u8ImgMode;

	enWDRMode   = g_pastImx376[ViPipe]->enWDRMode;
	u8ImgMode   = g_pastImx376[ViPipe]->u8ImgMode;

	imx376_i2c_init(ViPipe);
	if (enWDRMode == WDR_MODE_2To1_LINE) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport image mode[%d]\n", enWDRMode);
		return;
	} else {
		if (u8ImgMode == IMX376_MODE_5M25)
			imx376_linear_5M30_init(ViPipe);
		else {
		}
	}
	g_pastImx376[ViPipe]->bInit = CVI_TRUE;
}
#define IMX376_CHIP_ID_HI_ADDR		0x0016
#define IMX376_CHIP_ID_LO_ADDR		0x0017
#define IMX376_CHIP_ID			0x376

int imx376_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	if (imx376_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	delay_ms(5);

	nVal = imx376_read_register(ViPipe, IMX376_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = imx376_read_register(ViPipe, IMX376_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != IMX376_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static void imx376_linear_5M30_init(VI_PIPE ViPipe)
{
	//MCLK: 24MHz
	//Output Size: 2560*1920 RAW10
	//Lane Num: 2 Lane Mipi
	//Data Rate: 798Mbps
	//Frame Rate: 25fps
	//Freame Length: 2384
	//Line Length: 6656
	//PCLK: 397.2M
	imx376_write_register(ViPipe, 0x0136,  0x18);
	imx376_write_register(ViPipe, 0x0137,  0x00);
	imx376_write_register(ViPipe, 0x3C7D,  0x28);
	imx376_write_register(ViPipe, 0x3C7E,  0x01);
	imx376_write_register(ViPipe, 0x3C7F,  0x08);
	imx376_write_register(ViPipe, 0x3F02,  0x02);
	imx376_write_register(ViPipe, 0x3F22,  0x01);
	imx376_write_register(ViPipe, 0x3F7F,  0x01);
	imx376_write_register(ViPipe, 0x4421,  0x04);
	imx376_write_register(ViPipe, 0x4430,  0x05);
	imx376_write_register(ViPipe, 0x4431,  0xDC);
	imx376_write_register(ViPipe, 0x5222,  0x02);
	imx376_write_register(ViPipe, 0x56B7,  0x74);
	imx376_write_register(ViPipe, 0x6204,  0xC6);
	imx376_write_register(ViPipe, 0x620E,  0x27);
	imx376_write_register(ViPipe, 0x6210,  0x69);
	imx376_write_register(ViPipe, 0x6211,  0xD6);
	imx376_write_register(ViPipe, 0x6213,  0x01);
	imx376_write_register(ViPipe, 0x6215,  0x5A);
	imx376_write_register(ViPipe, 0x6216,  0x75);
	imx376_write_register(ViPipe, 0x6218,  0x5A);
	imx376_write_register(ViPipe, 0x6219,  0x75);
	imx376_write_register(ViPipe, 0x6220,  0x06);
	imx376_write_register(ViPipe, 0x6222,  0x0C);
	imx376_write_register(ViPipe, 0x6225,  0x19);
	imx376_write_register(ViPipe, 0x6228,  0x32);
	imx376_write_register(ViPipe, 0x6229,  0x70);
	imx376_write_register(ViPipe, 0x622B,  0x64);
	imx376_write_register(ViPipe, 0x622E,  0xB0);
	imx376_write_register(ViPipe, 0x6231,  0x71);
	imx376_write_register(ViPipe, 0x6234,  0x06);
	imx376_write_register(ViPipe, 0x6236,  0x46);
	imx376_write_register(ViPipe, 0x6237,  0x46);
	imx376_write_register(ViPipe, 0x6239,  0x0C);
	imx376_write_register(ViPipe, 0x623C,  0x19);
	imx376_write_register(ViPipe, 0x623F,  0x32);
	imx376_write_register(ViPipe, 0x6240,  0x71);
	imx376_write_register(ViPipe, 0x6242,  0x64);
	imx376_write_register(ViPipe, 0x6243,  0x44);
	imx376_write_register(ViPipe, 0x6245,  0xB0);
	imx376_write_register(ViPipe, 0x6246,  0xA8);
	imx376_write_register(ViPipe, 0x6248,  0x71);
	imx376_write_register(ViPipe, 0x624B,  0x06);
	imx376_write_register(ViPipe, 0x624D,  0x46);
	imx376_write_register(ViPipe, 0x625C,  0xC9);
	imx376_write_register(ViPipe, 0x625F,  0x92);
	imx376_write_register(ViPipe, 0x6262,  0x26);
	imx376_write_register(ViPipe, 0x6264,  0x46);
	imx376_write_register(ViPipe, 0x6265,  0x46);
	imx376_write_register(ViPipe, 0x6267,  0x0C);
	imx376_write_register(ViPipe, 0x626A,  0x19);
	imx376_write_register(ViPipe, 0x626D,  0x32);
	imx376_write_register(ViPipe, 0x626E,  0x72);
	imx376_write_register(ViPipe, 0x6270,  0x64);
	imx376_write_register(ViPipe, 0x6271,  0x68);
	imx376_write_register(ViPipe, 0x6273,  0xC8);
	imx376_write_register(ViPipe, 0x6276,  0x91);
	imx376_write_register(ViPipe, 0x6279,  0x27);
	imx376_write_register(ViPipe, 0x627B,  0x46);
	imx376_write_register(ViPipe, 0x627C,  0x55);
	imx376_write_register(ViPipe, 0x627F,  0x95);
	imx376_write_register(ViPipe, 0x6282,  0x84);
	imx376_write_register(ViPipe, 0x6283,  0x40);
	imx376_write_register(ViPipe, 0x6284,  0x00);
	imx376_write_register(ViPipe, 0x6285,  0x00);
	imx376_write_register(ViPipe, 0x6286,  0x08);
	imx376_write_register(ViPipe, 0x6287,  0xC0);
	imx376_write_register(ViPipe, 0x6288,  0x00);
	imx376_write_register(ViPipe, 0x6289,  0x00);
	imx376_write_register(ViPipe, 0x628A,  0x1B);
	imx376_write_register(ViPipe, 0x628B,  0x80);
	imx376_write_register(ViPipe, 0x628C,  0x20);
	imx376_write_register(ViPipe, 0x628E,  0x35);
	imx376_write_register(ViPipe, 0x628F,  0x00);
	imx376_write_register(ViPipe, 0x6290,  0x50);
	imx376_write_register(ViPipe, 0x6291,  0x00);
	imx376_write_register(ViPipe, 0x6292,  0x14);
	imx376_write_register(ViPipe, 0x6293,  0x00);
	imx376_write_register(ViPipe, 0x6294,  0x00);
	imx376_write_register(ViPipe, 0x6296,  0x54);
	imx376_write_register(ViPipe, 0x6297,  0x00);
	imx376_write_register(ViPipe, 0x6298,  0x00);
	imx376_write_register(ViPipe, 0x6299,  0x01);
	imx376_write_register(ViPipe, 0x629A,  0x10);
	imx376_write_register(ViPipe, 0x629B,  0x01);
	imx376_write_register(ViPipe, 0x629C,  0x00);
	imx376_write_register(ViPipe, 0x629D,  0x03);
	imx376_write_register(ViPipe, 0x629E,  0x50);
	imx376_write_register(ViPipe, 0x629F,  0x05);
	imx376_write_register(ViPipe, 0x62A0,  0x00);
	imx376_write_register(ViPipe, 0x62B1,  0x00);
	imx376_write_register(ViPipe, 0x62B2,  0x00);
	imx376_write_register(ViPipe, 0x62B3,  0x00);
	imx376_write_register(ViPipe, 0x62B5,  0x00);
	imx376_write_register(ViPipe, 0x62B6,  0x00);
	imx376_write_register(ViPipe, 0x62B7,  0x00);
	imx376_write_register(ViPipe, 0x62B8,  0x00);
	imx376_write_register(ViPipe, 0x62B9,  0x00);
	imx376_write_register(ViPipe, 0x62BA,  0x00);
	imx376_write_register(ViPipe, 0x62BB,  0x00);
	imx376_write_register(ViPipe, 0x62BC,  0x00);
	imx376_write_register(ViPipe, 0x62BD,  0x00);
	imx376_write_register(ViPipe, 0x62BE,  0x00);
	imx376_write_register(ViPipe, 0x62BF,  0x00);
	imx376_write_register(ViPipe, 0x62D0,  0x0C);
	imx376_write_register(ViPipe, 0x62D1,  0x00);
	imx376_write_register(ViPipe, 0x62D2,  0x00);
	imx376_write_register(ViPipe, 0x62D4,  0x40);
	imx376_write_register(ViPipe, 0x62D5,  0x00);
	imx376_write_register(ViPipe, 0x62D6,  0x00);
	imx376_write_register(ViPipe, 0x62D7,  0x00);
	imx376_write_register(ViPipe, 0x62D8,  0xD8);
	imx376_write_register(ViPipe, 0x62D9,  0x00);
	imx376_write_register(ViPipe, 0x62DA,  0x00);
	imx376_write_register(ViPipe, 0x62DB,  0x02);
	imx376_write_register(ViPipe, 0x62DC,  0xB0);
	imx376_write_register(ViPipe, 0x62DD,  0x03);
	imx376_write_register(ViPipe, 0x62DE,  0x00);
	imx376_write_register(ViPipe, 0x62EF,  0x14);
	imx376_write_register(ViPipe, 0x62F0,  0x00);
	imx376_write_register(ViPipe, 0x62F1,  0x00);
	imx376_write_register(ViPipe, 0x62F3,  0x58);
	imx376_write_register(ViPipe, 0x62F4,  0x00);
	imx376_write_register(ViPipe, 0x62F5,  0x00);
	imx376_write_register(ViPipe, 0x62F6,  0x01);
	imx376_write_register(ViPipe, 0x62F7,  0x20);
	imx376_write_register(ViPipe, 0x62F8,  0x00);
	imx376_write_register(ViPipe, 0x62F9,  0x00);
	imx376_write_register(ViPipe, 0x62FA,  0x03);
	imx376_write_register(ViPipe, 0x62FB,  0x80);
	imx376_write_register(ViPipe, 0x62FC,  0x00);
	imx376_write_register(ViPipe, 0x62FD,  0x00);
	imx376_write_register(ViPipe, 0x62FE,  0x04);
	imx376_write_register(ViPipe, 0x62FF,  0x60);
	imx376_write_register(ViPipe, 0x6300,  0x04);
	imx376_write_register(ViPipe, 0x6301,  0x00);
	imx376_write_register(ViPipe, 0x6302,  0x09);
	imx376_write_register(ViPipe, 0x6303,  0x00);
	imx376_write_register(ViPipe, 0x6304,  0x0C);
	imx376_write_register(ViPipe, 0x6305,  0x00);
	imx376_write_register(ViPipe, 0x6306,  0x1B);
	imx376_write_register(ViPipe, 0x6307,  0x80);
	imx376_write_register(ViPipe, 0x6308,  0x30);
	imx376_write_register(ViPipe, 0x630A,  0x38);
	imx376_write_register(ViPipe, 0x630B,  0x00);
	imx376_write_register(ViPipe, 0x630C,  0x60);
	imx376_write_register(ViPipe, 0x630E,  0x14);
	imx376_write_register(ViPipe, 0x630F,  0x00);
	imx376_write_register(ViPipe, 0x6310,  0x00);
	imx376_write_register(ViPipe, 0x6312,  0x58);
	imx376_write_register(ViPipe, 0x6313,  0x00);
	imx376_write_register(ViPipe, 0x6314,  0x00);
	imx376_write_register(ViPipe, 0x6315,  0x01);
	imx376_write_register(ViPipe, 0x6316,  0x18);
	imx376_write_register(ViPipe, 0x6317,  0x01);
	imx376_write_register(ViPipe, 0x6318,  0x80);
	imx376_write_register(ViPipe, 0x6319,  0x03);
	imx376_write_register(ViPipe, 0x631A,  0x60);
	imx376_write_register(ViPipe, 0x631B,  0x06);
	imx376_write_register(ViPipe, 0x631C,  0x00);
	imx376_write_register(ViPipe, 0x632D,  0x0E);
	imx376_write_register(ViPipe, 0x632E,  0x00);
	imx376_write_register(ViPipe, 0x632F,  0x00);
	imx376_write_register(ViPipe, 0x6331,  0x44);
	imx376_write_register(ViPipe, 0x6332,  0x00);
	imx376_write_register(ViPipe, 0x6333,  0x00);
	imx376_write_register(ViPipe, 0x6334,  0x00);
	imx376_write_register(ViPipe, 0x6335,  0xE8);
	imx376_write_register(ViPipe, 0x6336,  0x00);
	imx376_write_register(ViPipe, 0x6337,  0x00);
	imx376_write_register(ViPipe, 0x6338,  0x02);
	imx376_write_register(ViPipe, 0x6339,  0xF0);
	imx376_write_register(ViPipe, 0x633A,  0x00);
	imx376_write_register(ViPipe, 0x633B,  0x00);
	imx376_write_register(ViPipe, 0x634C,  0x0C);
	imx376_write_register(ViPipe, 0x634D,  0x00);
	imx376_write_register(ViPipe, 0x634E,  0x00);
	imx376_write_register(ViPipe, 0x6350,  0x40);
	imx376_write_register(ViPipe, 0x6351,  0x00);
	imx376_write_register(ViPipe, 0x6352,  0x00);
	imx376_write_register(ViPipe, 0x6353,  0x00);
	imx376_write_register(ViPipe, 0x6354,  0xD8);
	imx376_write_register(ViPipe, 0x6355,  0x00);
	imx376_write_register(ViPipe, 0x6356,  0x00);
	imx376_write_register(ViPipe, 0x6357,  0x02);
	imx376_write_register(ViPipe, 0x6358,  0xB0);
	imx376_write_register(ViPipe, 0x6359,  0x04);
	imx376_write_register(ViPipe, 0x635A,  0x00);
	imx376_write_register(ViPipe, 0x636B,  0x00);
	imx376_write_register(ViPipe, 0x636C,  0x00);
	imx376_write_register(ViPipe, 0x636D,  0x00);
	imx376_write_register(ViPipe, 0x636F,  0x00);
	imx376_write_register(ViPipe, 0x6370,  0x00);
	imx376_write_register(ViPipe, 0x6371,  0x00);
	imx376_write_register(ViPipe, 0x6372,  0x00);
	imx376_write_register(ViPipe, 0x6373,  0x00);
	imx376_write_register(ViPipe, 0x6374,  0x00);
	imx376_write_register(ViPipe, 0x6375,  0x00);
	imx376_write_register(ViPipe, 0x6376,  0x00);
	imx376_write_register(ViPipe, 0x6377,  0x00);
	imx376_write_register(ViPipe, 0x6378,  0x00);
	imx376_write_register(ViPipe, 0x6379,  0x00);
	imx376_write_register(ViPipe, 0x637A,  0x13);
	imx376_write_register(ViPipe, 0x637B,  0xD4);
	imx376_write_register(ViPipe, 0x6388,  0x22);
	imx376_write_register(ViPipe, 0x6389,  0x82);
	imx376_write_register(ViPipe, 0x638A,  0xC8);
	imx376_write_register(ViPipe, 0x639D,  0x20);
	imx376_write_register(ViPipe, 0x7BA0,  0x01);
	imx376_write_register(ViPipe, 0x7BA9,  0x00);
	imx376_write_register(ViPipe, 0x7BAA,  0x01);
	imx376_write_register(ViPipe, 0x7BAD,  0x00);
	imx376_write_register(ViPipe, 0x9002,  0x00);
	imx376_write_register(ViPipe, 0x9003,  0x00);
	imx376_write_register(ViPipe, 0x9004,  0x0D);
	imx376_write_register(ViPipe, 0x9006,  0x01);
	imx376_write_register(ViPipe, 0x9200,  0x93);
	imx376_write_register(ViPipe, 0x9201,  0x85);
	imx376_write_register(ViPipe, 0x9202,  0x93);
	imx376_write_register(ViPipe, 0x9203,  0x87);
	imx376_write_register(ViPipe, 0x9204,  0x93);
	imx376_write_register(ViPipe, 0x9205,  0x8D);
	imx376_write_register(ViPipe, 0x9206,  0x93);
	imx376_write_register(ViPipe, 0x9207,  0x8F);
	imx376_write_register(ViPipe, 0x9208,  0x62);
	imx376_write_register(ViPipe, 0x9209,  0x2C);
	imx376_write_register(ViPipe, 0x920A,  0x62);
	imx376_write_register(ViPipe, 0x920B,  0x2F);
	imx376_write_register(ViPipe, 0x920C,  0x6A);
	imx376_write_register(ViPipe, 0x920D,  0x23);
	imx376_write_register(ViPipe, 0x920E,  0x71);
	imx376_write_register(ViPipe, 0x920F,  0x08);
	imx376_write_register(ViPipe, 0x9210,  0x71);
	imx376_write_register(ViPipe, 0x9211,  0x09);
	imx376_write_register(ViPipe, 0x9212,  0x71);
	imx376_write_register(ViPipe, 0x9213,  0x0B);
	imx376_write_register(ViPipe, 0x9214,  0x6A);
	imx376_write_register(ViPipe, 0x9215,  0x0F);
	imx376_write_register(ViPipe, 0x9216,  0x71);
	imx376_write_register(ViPipe, 0x9217,  0x07);
	imx376_write_register(ViPipe, 0x9218,  0x71);
	imx376_write_register(ViPipe, 0x9219,  0x03);
	imx376_write_register(ViPipe, 0x935D,  0x01);
	imx376_write_register(ViPipe, 0x9389,  0x05);
	imx376_write_register(ViPipe, 0x938B,  0x05);
	imx376_write_register(ViPipe, 0x9391,  0x05);
	imx376_write_register(ViPipe, 0x9393,  0x05);
	imx376_write_register(ViPipe, 0x9395,  0x65);
	imx376_write_register(ViPipe, 0x9397,  0x5A);
	imx376_write_register(ViPipe, 0x9399,  0x05);
	imx376_write_register(ViPipe, 0x939B,  0x05);
	imx376_write_register(ViPipe, 0x939D,  0x05);
	imx376_write_register(ViPipe, 0x939F,  0x05);
	imx376_write_register(ViPipe, 0x93A1,  0x05);
	imx376_write_register(ViPipe, 0x93A3,  0x05);
	imx376_write_register(ViPipe, 0xB3F1,  0x80);
	imx376_write_register(ViPipe, 0xB3F2,  0x0E);
	imx376_write_register(ViPipe, 0xBC40,  0x03);
	imx376_write_register(ViPipe, 0xBC82,  0x07);
	imx376_write_register(ViPipe, 0xBC83,  0xB0);
	imx376_write_register(ViPipe, 0xBC84,  0x0D);
	imx376_write_register(ViPipe, 0xBC85,  0x08);
	imx376_write_register(ViPipe, 0xE0A6,  0x0A);
	imx376_write_register(ViPipe, 0x0100,  0x00);
	delay_ms(50);
	imx376_write_register(ViPipe, 0x0112,  0x0A);
	imx376_write_register(ViPipe, 0x0113,  0x0A);
	imx376_write_register(ViPipe, 0x0114,  0x01);
	imx376_write_register(ViPipe, 0x0342,  0x1A);
	imx376_write_register(ViPipe, 0x0343,  0x00);
	imx376_write_register(ViPipe, 0x0340,  0x09);
	imx376_write_register(ViPipe, 0x0341,  0x50);
	imx376_write_register(ViPipe, 0x0344,  0x00);
	imx376_write_register(ViPipe, 0x0345,  0x00);
	imx376_write_register(ViPipe, 0x0346,  0x00);
	imx376_write_register(ViPipe, 0x0347,  0x00);
	imx376_write_register(ViPipe, 0x0348,  0x14);
	imx376_write_register(ViPipe, 0x0349,  0x3F);
	imx376_write_register(ViPipe, 0x034A,  0x0F);
	imx376_write_register(ViPipe, 0x034B,  0x27);
	imx376_write_register(ViPipe, 0x0381,  0x01);
	imx376_write_register(ViPipe, 0x0383,  0x01);
	imx376_write_register(ViPipe, 0x0385,  0x01);
	imx376_write_register(ViPipe, 0x0387,  0x01);
	imx376_write_register(ViPipe, 0x0900,  0x01);
	imx376_write_register(ViPipe, 0x0901,  0x22);
	imx376_write_register(ViPipe, 0x0902,  0x08);
	imx376_write_register(ViPipe, 0x3F4D,  0x81);
	imx376_write_register(ViPipe, 0x3F4C,  0x81);
	imx376_write_register(ViPipe, 0x4254,  0x7F);
	imx376_write_register(ViPipe, 0x0401,  0x00);
	imx376_write_register(ViPipe, 0x0404,  0x00);
	imx376_write_register(ViPipe, 0x0405,  0x10);
	imx376_write_register(ViPipe, 0x0408,  0x00);
	imx376_write_register(ViPipe, 0x0409,  0x00);
	imx376_write_register(ViPipe, 0x040A,  0x00);
	imx376_write_register(ViPipe, 0x040B,  0x00);
	imx376_write_register(ViPipe, 0x040C,  0x0A);
	imx376_write_register(ViPipe, 0x040D,  0x00);
	imx376_write_register(ViPipe, 0x040E,  0x07);
	imx376_write_register(ViPipe, 0x040F,  0x80);
	imx376_write_register(ViPipe, 0x034C,  0x0A);
	imx376_write_register(ViPipe, 0x034D,  0x04);
	imx376_write_register(ViPipe, 0x034E,  0x07);
	imx376_write_register(ViPipe, 0x034F,  0x84);
	imx376_write_register(ViPipe, 0x0301,  0x05);
	imx376_write_register(ViPipe, 0x0303,  0x04);
	imx376_write_register(ViPipe, 0x0305,  0x04);
	imx376_write_register(ViPipe, 0x0306,  0x01);
	imx376_write_register(ViPipe, 0x0307,  0x4B);
	imx376_write_register(ViPipe, 0x030B,  0x02);
	imx376_write_register(ViPipe, 0x030D,  0x04);
	imx376_write_register(ViPipe, 0x030E,  0x01);
	imx376_write_register(ViPipe, 0x030F,  0x0A);
	imx376_write_register(ViPipe, 0x0310,  0x01);
	imx376_write_register(ViPipe, 0x0820,  0x0C);
	imx376_write_register(ViPipe, 0x0821,  0x78);
	imx376_write_register(ViPipe, 0x0822,  0x00);
	imx376_write_register(ViPipe, 0x0823,  0x00);
	imx376_write_register(ViPipe, 0xBC41,  0x01);
	imx376_write_register(ViPipe, 0x0106,  0x00);
	imx376_write_register(ViPipe, 0x0B00,  0x00);
	imx376_write_register(ViPipe, 0x0B05,  0x01);
	imx376_write_register(ViPipe, 0x0B06,  0x01);
	imx376_write_register(ViPipe, 0x3230,  0x00);
	imx376_write_register(ViPipe, 0x3602,  0x01);
	imx376_write_register(ViPipe, 0x3C00,  0x74);
	imx376_write_register(ViPipe, 0x3C01,  0x5F);
	imx376_write_register(ViPipe, 0x3C02,  0x73);
	imx376_write_register(ViPipe, 0x3C03,  0x64);
	imx376_write_register(ViPipe, 0x3C04,  0x54);
	imx376_write_register(ViPipe, 0x3C05,  0xA8);
	imx376_write_register(ViPipe, 0x3C06,  0xBC);
	imx376_write_register(ViPipe, 0x3C07,  0x00);
	imx376_write_register(ViPipe, 0x3C08,  0x00);
	imx376_write_register(ViPipe, 0x3C09,  0x01);
	imx376_write_register(ViPipe, 0x3C0A,  0x14);
	imx376_write_register(ViPipe, 0x3C0B,  0x01);
	imx376_write_register(ViPipe, 0x3C0C,  0x00);
	imx376_write_register(ViPipe, 0x3F14,  0x00);
	imx376_write_register(ViPipe, 0x3F17,  0x00);
	imx376_write_register(ViPipe, 0x3F3C,  0x00);
	imx376_write_register(ViPipe, 0x3F78,  0x03);
	imx376_write_register(ViPipe, 0x3F79,  0x84);
	imx376_write_register(ViPipe, 0x3F7A,  0x02);
	imx376_write_register(ViPipe, 0x3F7B,  0xFC);
	imx376_write_register(ViPipe, 0x562B,  0x0A);
	imx376_write_register(ViPipe, 0x562D,  0x0C);
	imx376_write_register(ViPipe, 0x5617,  0x0A);
	imx376_write_register(ViPipe, 0x9104,  0x04);
	imx376_write_register(ViPipe, 0x0202,  0x09);
	imx376_write_register(ViPipe, 0x0203,  0x48);
	imx376_write_register(ViPipe, 0x0204,  0x00);
	imx376_write_register(ViPipe, 0x0205,  0x00);
	imx376_write_register(ViPipe, 0x020E,  0x01);
	imx376_write_register(ViPipe, 0x020F,  0x00);
	imx376_write_register(ViPipe, 0x3614,  0x00);
	imx376_write_register(ViPipe, 0x3616,  0x0D);
	imx376_write_register(ViPipe, 0x3617,  0x56);
	imx376_write_register(ViPipe, 0xB612,  0x2C);
	imx376_write_register(ViPipe, 0xB613,  0x2C);
	imx376_write_register(ViPipe, 0xB614,  0x1C);
	imx376_write_register(ViPipe, 0xB615,  0x1C);
	imx376_write_register(ViPipe, 0xB616,  0x06);
	imx376_write_register(ViPipe, 0xB617,  0x06);
	imx376_write_register(ViPipe, 0xB618,  0x20);
	imx376_write_register(ViPipe, 0xB619,  0x20);
	imx376_write_register(ViPipe, 0xB61A,  0x0C);
	imx376_write_register(ViPipe, 0xB61B,  0x0C);
	imx376_write_register(ViPipe, 0xB61C,  0x06);
	imx376_write_register(ViPipe, 0xB61D,  0x06);
	imx376_write_register(ViPipe, 0xB666,  0x39);
	imx376_write_register(ViPipe, 0xB667,  0x39);
	imx376_write_register(ViPipe, 0xB668,  0x39);
	imx376_write_register(ViPipe, 0xB669,  0x39);
	imx376_write_register(ViPipe, 0xB66A,  0x13);
	imx376_write_register(ViPipe, 0xB66B,  0x13);
	imx376_write_register(ViPipe, 0xB66C,  0x20);
	imx376_write_register(ViPipe, 0xB66D,  0x20);
	imx376_write_register(ViPipe, 0xB66E,  0x20);
	imx376_write_register(ViPipe, 0xB66F,  0x20);
	imx376_write_register(ViPipe, 0xB670,  0x10);
	imx376_write_register(ViPipe, 0xB671,  0x10);
	imx376_write_register(ViPipe, 0x3900,  0x00);
	imx376_write_register(ViPipe, 0x3901,  0x00);
	imx376_write_register(ViPipe, 0x3237,  0x00);
	imx376_write_register(ViPipe, 0x30AC,  0x00);
	delay_ms(4);
	imx376_write_register(ViPipe, 0x0100, 0x01);

	imx376_default_reg_init(ViPipe);

	printf("ViPipe:%d,===IMX376 5M 30fps 10bit LINEAR Init OK!===\n", ViPipe);
}

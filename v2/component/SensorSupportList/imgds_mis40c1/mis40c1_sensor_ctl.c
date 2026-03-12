#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "mis40c1_cmos_ex.h"
#include "sensor_i2c.h"
static void mis40c1_linear_1440p20_2l_init(VI_PIPE ViPipe);
static void mis40c1_linear_1440p20_1l_init(VI_PIPE ViPipe);

CVI_U8 mis40c1_i2c_addr = 0x30;        /* I2C Address of MIS40C1 */
const CVI_U32 mis40c1_addr_byte = 2;
const CVI_U32 mis40c1_data_byte = 1;

int mis40c1_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunMIS40C1_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunMIS40C1_AddrInfo[ViPipe].s8I2cAddr);
}

int mis40c1_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunMIS40C1_BusInfo[ViPipe].s8I2cDev);
}

int mis40c1_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunMIS40C1_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunMIS40C1_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							mis40c1_addr_byte, mis40c1_data_byte);
}

int mis40c1_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunMIS40C1_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunMIS40C1_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							mis40c1_addr_byte, (CVI_U32)data, mis40c1_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void mis40c1_standby(VI_PIPE ViPipe)
{
	mis40c1_write_register(ViPipe, 0x3006, 0x02);
}

void mis40c1_restart(VI_PIPE ViPipe)
{
	mis40c1_write_register(ViPipe, 0x3006, 0x01);
}

void mis40c1_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastMIS40C1[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		mis40c1_write_register(ViPipe,
				g_pastMIS40C1[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastMIS40C1[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
		// printf("RegAddr = 0x%x, RegData = 0x%x.\n",
		// 		g_pastMIS40C1[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
		// 		g_pastMIS40C1[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);

	}
}

#define MIS40C1_CHIP_ID_HI_ADDR		0x3000
#define MIS40C1_CHIP_ID_LO_ADDR		0x3001
#define MIS40C1_CHIP_ID			0x2008

int mis40c1_probe(VI_PIPE ViPipe)
{
	int nVal;
	CVI_U16 chip_id;

	if (mis40c1_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	delay_ms(5);

	nVal = mis40c1_read_register(ViPipe, MIS40C1_CHIP_ID_HI_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id = (nVal & 0xFF) << 8;
	nVal = mis40c1_read_register(ViPipe, MIS40C1_CHIP_ID_LO_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	chip_id |= (nVal & 0xFF);

	if (chip_id != MIS40C1_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		// return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}


void mis40c1_init(VI_PIPE ViPipe)
{
	CVI_U8 u8ImgMode;

	mis40c1_i2c_init(ViPipe);

	u8ImgMode = g_pastMIS40C1[ViPipe]->u8ImgMode;

	if (u8ImgMode == MIS40C1_MODE_1440P20_1L) {
		mis40c1_linear_1440p20_1l_init(ViPipe);
	} else if (u8ImgMode == MIS40C1_MODE_1440P20_2L) {
		mis40c1_linear_1440p20_2l_init(ViPipe);
	}

	g_pastMIS40C1[ViPipe]->bInit = CVI_TRUE;
}

void mis40c1_exit(VI_PIPE ViPipe)
{
	mis40c1_i2c_exit(ViPipe);
}

/* 1440p20 */
static void mis40c1_linear_1440p20_1l_init(VI_PIPE ViPipe)
{
	//Sensor revision:Mis40c1
	//Input clock frequency:24M
	//Image output size:2560x1440
	//Frame timing and frame rate:Linear 20Fps
	//System clock frequency:84M
	//Output interface and data rate:MIPI 1Lane RAW10 840Mbps
	//HTS = 3107/3108 =0xaf0
	//VTS = 3105/3106 =0x5DC
	//Tline = 33.3us

	mis40c1_write_register(ViPipe, 0x3006,0x01);
	mis40c1_write_register(ViPipe, 0x302d,0x01);
	mis40c1_write_register(ViPipe, 0xffff,0x50); //延时一帧
	mis40c1_write_register(ViPipe, 0x3006,0x02);
	mis40c1_write_register(ViPipe, 0x3014,0x00);
	mis40c1_write_register(ViPipe, 0x301f,0x01);
	mis40c1_write_register(ViPipe, 0x3c1d,0x0a);
	mis40c1_write_register(ViPipe, 0x3c1e,0x00);
	mis40c1_write_register(ViPipe, 0x3c1f,0x05);
	mis40c1_write_register(ViPipe, 0x3c20,0xa0);
	mis40c1_write_register(ViPipe, 0x3106,0xdc);
	mis40c1_write_register(ViPipe, 0x3105,0x05);
	mis40c1_write_register(ViPipe, 0x3108,0xe4);
	mis40c1_write_register(ViPipe, 0x3107,0x0c);
	mis40c1_write_register(ViPipe, 0x310a,0x04);
	mis40c1_write_register(ViPipe, 0x3109,0x00);
	mis40c1_write_register(ViPipe, 0x310c,0xa3);
	mis40c1_write_register(ViPipe, 0x310b,0x05);
	mis40c1_write_register(ViPipe, 0x310e,0x04);
	mis40c1_write_register(ViPipe, 0x310d,0x00);
	mis40c1_write_register(ViPipe, 0x3110,0x03);
	mis40c1_write_register(ViPipe, 0x310f,0x0a);
	mis40c1_write_register(ViPipe, 0x3112,0x0c);
	mis40c1_write_register(ViPipe, 0x61a8,0x00);
	mis40c1_write_register(ViPipe, 0x4201,0x01);
	mis40c1_write_register(ViPipe, 0x4200,0x6e);
	mis40c1_write_register(ViPipe, 0x4203,0x04);
	mis40c1_write_register(ViPipe, 0x4210,0x00);
	mis40c1_write_register(ViPipe, 0x420a,0x01);
	mis40c1_write_register(ViPipe, 0x4202,0x01);//1lane
	mis40c1_write_register(ViPipe, 0x4208,0x01);
	mis40c1_write_register(ViPipe, 0x4204,0x01);
	mis40c1_write_register(ViPipe, 0x6101,0x3c);
	mis40c1_write_register(ViPipe, 0x6100,0x00);
	mis40c1_write_register(ViPipe, 0x6105,0xff);
	mis40c1_write_register(ViPipe, 0x6104,0x1f);
	mis40c1_write_register(ViPipe, 0x6103,0xc3);
	mis40c1_write_register(ViPipe, 0x6102,0x0c);
	mis40c1_write_register(ViPipe, 0x6107,0xff);
	mis40c1_write_register(ViPipe, 0x6106,0x1f);
	mis40c1_write_register(ViPipe, 0x6109,0x7c);
	mis40c1_write_register(ViPipe, 0x6108,0x04);
	mis40c1_write_register(ViPipe, 0x610d,0xff);
	mis40c1_write_register(ViPipe, 0x610c,0x1f);
	mis40c1_write_register(ViPipe, 0x610b,0xa5);
	mis40c1_write_register(ViPipe, 0x610a,0x05);
	mis40c1_write_register(ViPipe, 0x610f,0xff);
	mis40c1_write_register(ViPipe, 0x610e,0x1f);
	mis40c1_write_register(ViPipe, 0x6111,0x00);
	mis40c1_write_register(ViPipe, 0x6110,0x00);
	mis40c1_write_register(ViPipe, 0x6113,0x35);
	mis40c1_write_register(ViPipe, 0x6112,0x02);
	mis40c1_write_register(ViPipe, 0x6115,0x6d);
	mis40c1_write_register(ViPipe, 0x6114,0x04);
	mis40c1_write_register(ViPipe, 0x6117,0xa8);
	mis40c1_write_register(ViPipe, 0x6116,0x04);
	mis40c1_write_register(ViPipe, 0x6119,0x7c);
	mis40c1_write_register(ViPipe, 0x6118,0x04);
	mis40c1_write_register(ViPipe, 0x611b,0xb7);
	mis40c1_write_register(ViPipe, 0x611a,0x04);
	mis40c1_write_register(ViPipe, 0x611d,0x1e);
	mis40c1_write_register(ViPipe, 0x611c,0x00);
	mis40c1_write_register(ViPipe, 0x611f,0x99);
	mis40c1_write_register(ViPipe, 0x611e,0x04);
	mis40c1_write_register(ViPipe, 0x6121,0x00);
	mis40c1_write_register(ViPipe, 0x6120,0x00);
	mis40c1_write_register(ViPipe, 0x6123,0xd2);
	mis40c1_write_register(ViPipe, 0x6122,0x0c);
	mis40c1_write_register(ViPipe, 0x6125,0x00);
	mis40c1_write_register(ViPipe, 0x6124,0x00);
	mis40c1_write_register(ViPipe, 0x6127,0xd2);
	mis40c1_write_register(ViPipe, 0x6126,0x0c);
	mis40c1_write_register(ViPipe, 0x6129,0x1e);
	mis40c1_write_register(ViPipe, 0x6128,0x00);
	mis40c1_write_register(ViPipe, 0x612b,0xab);
	mis40c1_write_register(ViPipe, 0x612a,0x0c);
	mis40c1_write_register(ViPipe, 0x612d,0x00);
	mis40c1_write_register(ViPipe, 0x612c,0x00);
	mis40c1_write_register(ViPipe, 0x612f,0x84);
	mis40c1_write_register(ViPipe, 0x612e,0x04);
	mis40c1_write_register(ViPipe, 0x6131,0x3c);
	mis40c1_write_register(ViPipe, 0x6130,0x00);
	mis40c1_write_register(ViPipe, 0x6133,0xbe);
	mis40c1_write_register(ViPipe, 0x6132,0x01);
	mis40c1_write_register(ViPipe, 0x6135,0x3c);
	mis40c1_write_register(ViPipe, 0x6134,0x00);
	mis40c1_write_register(ViPipe, 0x6137,0xaf);
	mis40c1_write_register(ViPipe, 0x6136,0x01);
	mis40c1_write_register(ViPipe, 0x61ad,0x17);
	mis40c1_write_register(ViPipe, 0x61ac,0x02);
	mis40c1_write_register(ViPipe, 0x61b1,0x5e);
	mis40c1_write_register(ViPipe, 0x61b0,0x04);
	mis40c1_write_register(ViPipe, 0x61af,0x35);
	mis40c1_write_register(ViPipe, 0x61ae,0x02);
	mis40c1_write_register(ViPipe, 0x61b3,0x39);
	mis40c1_write_register(ViPipe, 0x61b2,0x06);
	mis40c1_write_register(ViPipe, 0x6139,0x70);
	mis40c1_write_register(ViPipe, 0x6138,0x02);
	mis40c1_write_register(ViPipe, 0x613d,0x74);
	mis40c1_write_register(ViPipe, 0x613c,0x06);
	mis40c1_write_register(ViPipe, 0x613b,0x40);
	mis40c1_write_register(ViPipe, 0x613a,0x04);
	mis40c1_write_register(ViPipe, 0x613f,0xb4);
	mis40c1_write_register(ViPipe, 0x613e,0x0c);
	mis40c1_write_register(ViPipe, 0x6141,0x61);
	mis40c1_write_register(ViPipe, 0x6140,0x02);
	mis40c1_write_register(ViPipe, 0x6143,0x4f);
	mis40c1_write_register(ViPipe, 0x6142,0x04);
	mis40c1_write_register(ViPipe, 0x6145,0x66);
	mis40c1_write_register(ViPipe, 0x6144,0x06);
	mis40c1_write_register(ViPipe, 0x6147,0xc3);
	mis40c1_write_register(ViPipe, 0x6146,0x0c);
	mis40c1_write_register(ViPipe, 0x6149,0x52);
	mis40c1_write_register(ViPipe, 0x6148,0x02);
	mis40c1_write_register(ViPipe, 0x614d,0x57);
	mis40c1_write_register(ViPipe, 0x614c,0x06);
	mis40c1_write_register(ViPipe, 0x614b,0x3c);
	mis40c1_write_register(ViPipe, 0x614a,0x04);
	mis40c1_write_register(ViPipe, 0x614f,0xb0);
	mis40c1_write_register(ViPipe, 0x614e,0x0c);
	mis40c1_write_register(ViPipe, 0x6151,0x00);
	mis40c1_write_register(ViPipe, 0x6150,0x00);
	mis40c1_write_register(ViPipe, 0x6155,0x52);
	mis40c1_write_register(ViPipe, 0x6154,0x02);
	mis40c1_write_register(ViPipe, 0x6159,0x57);
	mis40c1_write_register(ViPipe, 0x6158,0x06);
	mis40c1_write_register(ViPipe, 0x6153,0x77);
	mis40c1_write_register(ViPipe, 0x6152,0x00);
	mis40c1_write_register(ViPipe, 0x6157,0x49);
	mis40c1_write_register(ViPipe, 0x6156,0x04);
	mis40c1_write_register(ViPipe, 0x615b,0xbd);
	mis40c1_write_register(ViPipe, 0x615a,0x0c);
	mis40c1_write_register(ViPipe, 0x615d,0x38);
	mis40c1_write_register(ViPipe, 0x615c,0x03);
	mis40c1_write_register(ViPipe, 0x6161,0x3c);
	mis40c1_write_register(ViPipe, 0x6160,0x07);
	mis40c1_write_register(ViPipe, 0x615f,0x40);
	mis40c1_write_register(ViPipe, 0x615e,0x04);
	mis40c1_write_register(ViPipe, 0x6163,0xb4);
	mis40c1_write_register(ViPipe, 0x6162,0x0c);
	mis40c1_write_register(ViPipe, 0x6165,0x00);
	mis40c1_write_register(ViPipe, 0x6164,0x00);
	mis40c1_write_register(ViPipe, 0x6169,0x7c);
	mis40c1_write_register(ViPipe, 0x6168,0x04);
	mis40c1_write_register(ViPipe, 0x6167,0x70);
	mis40c1_write_register(ViPipe, 0x6166,0x02);
	mis40c1_write_register(ViPipe, 0x616b,0x74);
	mis40c1_write_register(ViPipe, 0x616a,0x06);
	mis40c1_write_register(ViPipe, 0x616d,0xf9);
	mis40c1_write_register(ViPipe, 0x616c,0x01);
	mis40c1_write_register(ViPipe, 0x6171,0x5e);
	mis40c1_write_register(ViPipe, 0x6170,0x04);
	mis40c1_write_register(ViPipe, 0x616f,0x49);
	mis40c1_write_register(ViPipe, 0x616e,0x04);
	mis40c1_write_register(ViPipe, 0x6173,0xbd);
	mis40c1_write_register(ViPipe, 0x6172,0x0c);
	mis40c1_write_register(ViPipe, 0x6175,0x00);
	mis40c1_write_register(ViPipe, 0x6174,0x00);
	mis40c1_write_register(ViPipe, 0x6177,0x0f);
	mis40c1_write_register(ViPipe, 0x6176,0x00);
	mis40c1_write_register(ViPipe, 0x6179,0x01);
	mis40c1_write_register(ViPipe, 0x6178,0x00);
	mis40c1_write_register(ViPipe, 0x617b,0xbd);
	mis40c1_write_register(ViPipe, 0x617a,0x0c);
	mis40c1_write_register(ViPipe, 0x617d,0x00);
	mis40c1_write_register(ViPipe, 0x617c,0x00);
	mis40c1_write_register(ViPipe, 0x617f,0x29);
	mis40c1_write_register(ViPipe, 0x617e,0x01);
	mis40c1_write_register(ViPipe, 0x6181,0x00);
	mis40c1_write_register(ViPipe, 0x6180,0x00);
	mis40c1_write_register(ViPipe, 0x6183,0x29);
	mis40c1_write_register(ViPipe, 0x6182,0x01);
	mis40c1_write_register(ViPipe, 0x6185,0x00);
	mis40c1_write_register(ViPipe, 0x6184,0x00);
	mis40c1_write_register(ViPipe, 0x6187,0x29);
	mis40c1_write_register(ViPipe, 0x6186,0x01);
	mis40c1_write_register(ViPipe, 0x61b5,0x5e);
	mis40c1_write_register(ViPipe, 0x61b4,0x04);
	mis40c1_write_register(ViPipe, 0x61b7,0x6d);
	mis40c1_write_register(ViPipe, 0x61b6,0x04);
	mis40c1_write_register(ViPipe, 0x6189,0xd2);
	mis40c1_write_register(ViPipe, 0x6188,0x0c);
	mis40c1_write_register(ViPipe, 0x618b,0xe1);
	mis40c1_write_register(ViPipe, 0x618a,0x0c);
	mis40c1_write_register(ViPipe, 0x618d,0xd0);
	mis40c1_write_register(ViPipe, 0x618c,0x00);
	mis40c1_write_register(ViPipe, 0x6191,0x7c);
	mis40c1_write_register(ViPipe, 0x6190,0x04);
	mis40c1_write_register(ViPipe, 0x618f,0xee);
	mis40c1_write_register(ViPipe, 0x618e,0x00);
	mis40c1_write_register(ViPipe, 0x6193,0x99);
	mis40c1_write_register(ViPipe, 0x6192,0x04);
	mis40c1_write_register(ViPipe, 0x6195,0x0d);
	mis40c1_write_register(ViPipe, 0x6194,0x0d);
	mis40c1_write_register(ViPipe, 0x61a3,0xd0);
	mis40c1_write_register(ViPipe, 0x61a2,0x01);
	mis40c1_write_register(ViPipe, 0x61a5,0xcc);
	mis40c1_write_register(ViPipe, 0x61a4,0x05);
	//BLC
	mis40c1_write_register(ViPipe, 0x5400,0x2B);
	mis40c1_write_register(ViPipe, 0x5403,0x08);
	mis40c1_write_register(ViPipe, 0x5406,0x00);
	mis40c1_write_register(ViPipe, 0x5408,0x3e);
	//DPC ON 20230821
	mis40c1_write_register(ViPipe, 0x3902,0x02);
	//Added in 2022/10/31 by chenzheng
	mis40c1_write_register(ViPipe, 0x3a04,0x10);
	mis40c1_write_register(ViPipe, 0x3a17,0x00);
	mis40c1_write_register(ViPipe, 0x3a18,0x00);
	mis40c1_write_register(ViPipe, 0x3048,0x01);
	mis40c1_write_register(ViPipe, 0x6207,0x00);
	mis40c1_write_register(ViPipe, 0x6208,0x00);
	mis40c1_write_register(ViPipe, 0x3103,0x03);
	mis40c1_write_register(ViPipe, 0x3104,0xe0);
	mis40c1_write_register(ViPipe, 0x3118,0x01);
	mis40c1_write_register(ViPipe, 0x3700,0x01);
	//Offset 0x40
	mis40c1_write_register(ViPipe, 0x3701,0x00);
	mis40c1_write_register(ViPipe, 0x3702,0x40);
	mis40c1_write_register(ViPipe, 0x3703,0x00);
	mis40c1_write_register(ViPipe, 0x3704,0x40);
	mis40c1_write_register(ViPipe, 0x3705,0x00);
	mis40c1_write_register(ViPipe, 0x3706,0x40);
	mis40c1_write_register(ViPipe, 0x3707,0x00);
	mis40c1_write_register(ViPipe, 0x3708,0x40);
	mis40c1_write_register(ViPipe, 0x300c,0x01);
	mis40c1_write_register(ViPipe, 0x610b,0x40);
	mis40c1_write_register(ViPipe, 0x613d,0x70);
	//Added in 20230717 by huangyuanxi
	mis40c1_write_register(ViPipe, 0x61a1,0x90); //128x竖纹
	mis40c1_write_register(ViPipe, 0x6202,0x0c); //去高倍太阳黑子
	mis40c1_write_register(ViPipe, 0x3a03,0x39);
	mis40c1_write_register(ViPipe, 0x3a02,0xfc); //0xfd功耗最小基础上PAC OFF 20230810
	mis40c1_write_register(ViPipe, 0x3a08,0x0c);
	mis40c1_write_register(ViPipe, 0x3048,0x01);
	//PAC对称
	mis40c1_write_register(ViPipe, 0x61ae,0x02);
	mis40c1_write_register(ViPipe, 0x61af,0x50);
	mis40c1_write_register(ViPipe, 0x61b2,0x06);
	mis40c1_write_register(ViPipe, 0x61b3,0x50);
	mis40c1_write_register(ViPipe, 0x3040,0x01);
	mis40c1_write_register(ViPipe, 0x6200,0x09); //CM电流次小 优化横带
	mis40c1_write_register(ViPipe, 0x6201,0x09); //RCS电流次小 优化横带
	//优化灯管横带   20230821
	mis40c1_write_register(ViPipe, 0x3a02,0x2d);
	mis40c1_write_register(ViPipe, 0x3a03,0x19);
	mis40c1_write_register(ViPipe, 0x6200,0x00);
	mis40c1_write_register(ViPipe, 0x6201,0x09);
	mis40c1_write_register(ViPipe, 0x6124,0x1f);
	mis40c1_write_register(ViPipe, 0x6125,0xff);
	mis40c1_write_register(ViPipe, 0x6126,0x00);
	mis40c1_write_register(ViPipe, 0x6127,0x00);
	//Eclp电流减小弱化亮点问题
	mis40c1_write_register(ViPipe, 0x3a13,0x39);
	mis40c1_write_register(ViPipe, 0x3a15,0x08);
	//高温闪烁/BLC不准/画面异常   20230824
	mis40c1_write_register(ViPipe, 0x3a02,0x2d);
	mis40c1_write_register(ViPipe, 0x5400,0x23);
	mis40c1_write_register(ViPipe, 0x3a0c,0x07);
	mis40c1_write_register(ViPipe, 0x3040,0x01);
	//上电竖纹问题优化   20230828
	mis40c1_write_register(ViPipe, 0x6199,0x90);
	mis40c1_write_register(ViPipe, 0x619b,0x90);
	mis40c1_write_register(ViPipe, 0x619d,0x90);
	mis40c1_write_register(ViPipe, 0x619f,0x90);
	mis40c1_write_register(ViPipe, 0x61a1,0x90);
	//IIC片内切换并降低时钟频率   20230829
	mis40c1_write_register(ViPipe, 0x420c,0x00);
	mis40c1_write_register(ViPipe, 0x420d,0x0a);
	//1x 部分芯片竖线问题   20230829
	mis40c1_write_register(ViPipe, 0x3a01,0x01);
	//优化灯管横带问题   20230831//20231218降低功耗
	mis40c1_write_register(ViPipe, 0x6209,0x12);
	mis40c1_write_register(ViPipe, 0x3a02,0xad);
	mis40c1_write_register(ViPipe, 0x610b,0x08);//君正竖纹改善20231218
	mis40c1_write_register(ViPipe, 0x3a14,0x0c);//君正竖纹改善20231220
	//改善CG不线性,3a0e=60会使得CG不线性和CG偏大 20240119
	mis40c1_write_register(ViPipe, 0x3a0e,0xe0);
	//优化BLC闪烁问题
	mis40c1_write_register(ViPipe, 0x3a05,0x6a);
	mis40c1_write_register(ViPipe, 0x3048,0x01);
	mis40c1_write_register(ViPipe, 0x3a0d,0x32);
	mis40c1_write_register(ViPipe, 0x3040,0x01);
	mis40c1_write_register(ViPipe, 0x5407,0x02);
	mis40c1_write_register(ViPipe, 0x540d,0x01);
	mis40c1_write_register(ViPipe, 0x540d,0x03);
	mis40c1_write_register(ViPipe, 0x540e,0xff);
	//优化偏粉问题
	mis40c1_write_register(ViPipe, 0x5403,0x0f);
	mis40c1_write_register(ViPipe, 0x5404,0xff);
	//mipi 连续与非连续
	//0x3c35,0x8c//comtinue 8d
	//0x3047,0xf5//continue be
	mis40c1_write_register(ViPipe, 0x4200,0x8c);//1lane
	mis40c1_write_register(ViPipe, 0x4201,0x01);
	mis40c1_write_register(ViPipe, 0x4202,0x01);
	mis40c1_write_register(ViPipe, 0x4203,0x03);
	mis40c1_write_register(ViPipe, 0x4204,0x02);
	mis40c1_write_register(ViPipe, 0x4208,0x01);
	mis40c1_write_register(ViPipe, 0x3c37,0x01);
	mis40c1_write_register(ViPipe, 0x3c0c,0x0a);
	mis40c1_write_register(ViPipe, 0x3c0e,0x0f);
	mis40c1_write_register(ViPipe, 0x3c14,0x08);
	mis40c1_write_register(ViPipe, 0x3046,0x00);
	mis40c1_write_register(ViPipe, 0x3047,0xf0);
	mis40c1_write_register(ViPipe, 0x3107,0x0a);
	mis40c1_write_register(ViPipe, 0x3108,0xf0);
	mis40c1_write_register(ViPipe, 0x3006,0x00);
	mis40c1_write_register(ViPipe, 0x302d,0x00);

	// 2564x1444
	mis40c1_write_register(ViPipe, 0x3109,0x00);
	mis40c1_write_register(ViPipe, 0x310a,0x00);
	mis40c1_write_register(ViPipe, 0x310b,0x05);
	mis40c1_write_register(ViPipe, 0x310c,0xa3);
	mis40c1_write_register(ViPipe, 0x310d,0x00);
	mis40c1_write_register(ViPipe, 0x310e,0x00);
	mis40c1_write_register(ViPipe, 0x310f,0x0a);
	mis40c1_write_register(ViPipe, 0x3110,0x03);
	mis40c1_write_register(ViPipe, 0x3c1d,0x0a);
	mis40c1_write_register(ViPipe, 0x3c1e,0x04);
	mis40c1_write_register(ViPipe, 0x3c1f,0x05);
	mis40c1_write_register(ViPipe, 0x3c20,0xa4);

	mis40c1_default_reg_init(ViPipe);


	delay_ms(100);

	printf("ViPipe:%d,===MIS40C1 1440P 20fps 1lane 10bit LINE Init OK!===\n", ViPipe);
}

static void mis40c1_linear_1440p20_2l_init(VI_PIPE ViPipe)
{

	//Sensor revision:Mis40c1
	//Input clock frequency:27M
	//Image output size:2560x1440
	//Frame timing and frame rate:Linear 20Fps
	//System clock frequency:132M
	//Output interface and data rate:MIPI 2Lane RAW10 660Mbps
	//HTS = 3107/3108 =0xCE4
	//VTS = 3105/3106 =0x7d0
	//Tline = 25us

	mis40c1_write_register(ViPipe, 0x3006,0x01);
	mis40c1_write_register(ViPipe, 0x302d,0x01);
	mis40c1_write_register(ViPipe, 0xffff,0x50); //延时一帧
	mis40c1_write_register(ViPipe, 0x3006,0x02);
	mis40c1_write_register(ViPipe, 0x3014,0x00);

	mis40c1_write_register(ViPipe, 0x301f,0x01);
	mis40c1_write_register(ViPipe, 0x3c1d,0x0a);
	mis40c1_write_register(ViPipe, 0x3c1e,0x00);
	mis40c1_write_register(ViPipe, 0x3c1f,0x05);
	mis40c1_write_register(ViPipe, 0x3c20,0xa0);

	mis40c1_write_register(ViPipe, 0x3106,0xd0);
	mis40c1_write_register(ViPipe, 0x3105,0x07);
	mis40c1_write_register(ViPipe, 0x3108,0xe4);
	mis40c1_write_register(ViPipe, 0x3107,0x0c);
	mis40c1_write_register(ViPipe, 0x310a,0x04);
	mis40c1_write_register(ViPipe, 0x3109,0x00);
	mis40c1_write_register(ViPipe, 0x310c,0xa3);
	mis40c1_write_register(ViPipe, 0x310b,0x05);
	mis40c1_write_register(ViPipe, 0x310e,0x04);
	mis40c1_write_register(ViPipe, 0x310d,0x00);
	mis40c1_write_register(ViPipe, 0x3110,0x03);
	mis40c1_write_register(ViPipe, 0x310f,0x0a);
	mis40c1_write_register(ViPipe, 0x3112,0x0c);
	mis40c1_write_register(ViPipe, 0x61a8,0x00);
	mis40c1_write_register(ViPipe, 0x4201,0x01);
	mis40c1_write_register(ViPipe, 0x4200,0x6e);
	mis40c1_write_register(ViPipe, 0x4203,0x04);
	mis40c1_write_register(ViPipe, 0x4210,0x00);
	mis40c1_write_register(ViPipe, 0x420a,0x01);
	mis40c1_write_register(ViPipe, 0x4202,0x01);
	mis40c1_write_register(ViPipe, 0x4208,0x01);
	mis40c1_write_register(ViPipe, 0x4204,0x01);
	mis40c1_write_register(ViPipe, 0x6101,0x3c);
	mis40c1_write_register(ViPipe, 0x6100,0x00);
	mis40c1_write_register(ViPipe, 0x6105,0xff);
	mis40c1_write_register(ViPipe, 0x6104,0x1f);
	mis40c1_write_register(ViPipe, 0x6103,0xc3);
	mis40c1_write_register(ViPipe, 0x6102,0x0c);
	mis40c1_write_register(ViPipe, 0x6107,0xff);
	mis40c1_write_register(ViPipe, 0x6106,0x1f);
	mis40c1_write_register(ViPipe, 0x6109,0x7c);
	mis40c1_write_register(ViPipe, 0x6108,0x04);
	mis40c1_write_register(ViPipe, 0x610d,0xff);
	mis40c1_write_register(ViPipe, 0x610c,0x1f);
	mis40c1_write_register(ViPipe, 0x610b,0xa5);
	mis40c1_write_register(ViPipe, 0x610a,0x05);
	mis40c1_write_register(ViPipe, 0x610f,0xff);
	mis40c1_write_register(ViPipe, 0x610e,0x1f);
	mis40c1_write_register(ViPipe, 0x6111,0x00);
	mis40c1_write_register(ViPipe, 0x6110,0x00);
	mis40c1_write_register(ViPipe, 0x6113,0x35);
	mis40c1_write_register(ViPipe, 0x6112,0x02);
	mis40c1_write_register(ViPipe, 0x6115,0x6d);
	mis40c1_write_register(ViPipe, 0x6114,0x04);
	mis40c1_write_register(ViPipe, 0x6117,0xa8);
	mis40c1_write_register(ViPipe, 0x6116,0x04);
	mis40c1_write_register(ViPipe, 0x6119,0x7c);
	mis40c1_write_register(ViPipe, 0x6118,0x04);
	mis40c1_write_register(ViPipe, 0x611b,0xb7);
	mis40c1_write_register(ViPipe, 0x611a,0x04);
	mis40c1_write_register(ViPipe, 0x611d,0x1e);
	mis40c1_write_register(ViPipe, 0x611c,0x00);
	mis40c1_write_register(ViPipe, 0x611f,0x99);
	mis40c1_write_register(ViPipe, 0x611e,0x04);
	mis40c1_write_register(ViPipe, 0x6121,0x00);
	mis40c1_write_register(ViPipe, 0x6120,0x00);
	mis40c1_write_register(ViPipe, 0x6123,0xd2);
	mis40c1_write_register(ViPipe, 0x6122,0x0c);
	mis40c1_write_register(ViPipe, 0x6125,0x00);
	mis40c1_write_register(ViPipe, 0x6124,0x00);
	mis40c1_write_register(ViPipe, 0x6127,0xd2);
	mis40c1_write_register(ViPipe, 0x6126,0x0c);
	mis40c1_write_register(ViPipe, 0x6129,0x1e);
	mis40c1_write_register(ViPipe, 0x6128,0x00);
	mis40c1_write_register(ViPipe, 0x612b,0xab);
	mis40c1_write_register(ViPipe, 0x612a,0x0c);
	mis40c1_write_register(ViPipe, 0x612d,0x00);
	mis40c1_write_register(ViPipe, 0x612c,0x00);
	mis40c1_write_register(ViPipe, 0x612f,0x84);
	mis40c1_write_register(ViPipe, 0x612e,0x04);
	mis40c1_write_register(ViPipe, 0x6131,0x3c);
	mis40c1_write_register(ViPipe, 0x6130,0x00);
	mis40c1_write_register(ViPipe, 0x6133,0xbe);
	mis40c1_write_register(ViPipe, 0x6132,0x01);
	mis40c1_write_register(ViPipe, 0x6135,0x3c);
	mis40c1_write_register(ViPipe, 0x6134,0x00);
	mis40c1_write_register(ViPipe, 0x6137,0xaf);
	mis40c1_write_register(ViPipe, 0x6136,0x01);
	mis40c1_write_register(ViPipe, 0x61ad,0x17);
	mis40c1_write_register(ViPipe, 0x61ac,0x02);
	mis40c1_write_register(ViPipe, 0x61b1,0x5e);
	mis40c1_write_register(ViPipe, 0x61b0,0x04);
	mis40c1_write_register(ViPipe, 0x61af,0x35);
	mis40c1_write_register(ViPipe, 0x61ae,0x02);
	mis40c1_write_register(ViPipe, 0x61b3,0x39);
	mis40c1_write_register(ViPipe, 0x61b2,0x06);
	mis40c1_write_register(ViPipe, 0x6139,0x70);
	mis40c1_write_register(ViPipe, 0x6138,0x02);
	mis40c1_write_register(ViPipe, 0x613d,0x74);
	mis40c1_write_register(ViPipe, 0x613c,0x06);
	mis40c1_write_register(ViPipe, 0x613b,0x40);
	mis40c1_write_register(ViPipe, 0x613a,0x04);
	mis40c1_write_register(ViPipe, 0x613f,0xb4);
	mis40c1_write_register(ViPipe, 0x613e,0x0c);
	mis40c1_write_register(ViPipe, 0x6141,0x61);
	mis40c1_write_register(ViPipe, 0x6140,0x02);
	mis40c1_write_register(ViPipe, 0x6143,0x4f);
	mis40c1_write_register(ViPipe, 0x6142,0x04);
	mis40c1_write_register(ViPipe, 0x6145,0x66);
	mis40c1_write_register(ViPipe, 0x6144,0x06);
	mis40c1_write_register(ViPipe, 0x6147,0xc3);
	mis40c1_write_register(ViPipe, 0x6146,0x0c);
	mis40c1_write_register(ViPipe, 0x6149,0x52);
	mis40c1_write_register(ViPipe, 0x6148,0x02);
	mis40c1_write_register(ViPipe, 0x614d,0x57);
	mis40c1_write_register(ViPipe, 0x614c,0x06);
	mis40c1_write_register(ViPipe, 0x614b,0x3c);
	mis40c1_write_register(ViPipe, 0x614a,0x04);
	mis40c1_write_register(ViPipe, 0x614f,0xb0);
	mis40c1_write_register(ViPipe, 0x614e,0x0c);
	mis40c1_write_register(ViPipe, 0x6151,0x00);
	mis40c1_write_register(ViPipe, 0x6150,0x00);
	mis40c1_write_register(ViPipe, 0x6155,0x52);
	mis40c1_write_register(ViPipe, 0x6154,0x02);
	mis40c1_write_register(ViPipe, 0x6159,0x57);
	mis40c1_write_register(ViPipe, 0x6158,0x06);
	mis40c1_write_register(ViPipe, 0x6153,0x77);
	mis40c1_write_register(ViPipe, 0x6152,0x00);
	mis40c1_write_register(ViPipe, 0x6157,0x49);
	mis40c1_write_register(ViPipe, 0x6156,0x04);
	mis40c1_write_register(ViPipe, 0x615b,0xbd);
	mis40c1_write_register(ViPipe, 0x615a,0x0c);
	mis40c1_write_register(ViPipe, 0x615d,0x38);
	mis40c1_write_register(ViPipe, 0x615c,0x03);
	mis40c1_write_register(ViPipe, 0x6161,0x3c);
	mis40c1_write_register(ViPipe, 0x6160,0x07);
	mis40c1_write_register(ViPipe, 0x615f,0x40);
	mis40c1_write_register(ViPipe, 0x615e,0x04);
	mis40c1_write_register(ViPipe, 0x6163,0xb4);
	mis40c1_write_register(ViPipe, 0x6162,0x0c);
	mis40c1_write_register(ViPipe, 0x6165,0x00);
	mis40c1_write_register(ViPipe, 0x6164,0x00);
	mis40c1_write_register(ViPipe, 0x6169,0x7c);
	mis40c1_write_register(ViPipe, 0x6168,0x04);
	mis40c1_write_register(ViPipe, 0x6167,0x70);
	mis40c1_write_register(ViPipe, 0x6166,0x02);
	mis40c1_write_register(ViPipe, 0x616b,0x74);
	mis40c1_write_register(ViPipe, 0x616a,0x06);
	mis40c1_write_register(ViPipe, 0x616d,0xf9);
	mis40c1_write_register(ViPipe, 0x616c,0x01);
	mis40c1_write_register(ViPipe, 0x6171,0x5e);
	mis40c1_write_register(ViPipe, 0x6170,0x04);
	mis40c1_write_register(ViPipe, 0x616f,0x49);
	mis40c1_write_register(ViPipe, 0x616e,0x04);
	mis40c1_write_register(ViPipe, 0x6173,0xbd);
	mis40c1_write_register(ViPipe, 0x6172,0x0c);
	mis40c1_write_register(ViPipe, 0x6175,0x00);
	mis40c1_write_register(ViPipe, 0x6174,0x00);
	mis40c1_write_register(ViPipe, 0x6177,0x0f);
	mis40c1_write_register(ViPipe, 0x6176,0x00);
	mis40c1_write_register(ViPipe, 0x6179,0x01);
	mis40c1_write_register(ViPipe, 0x6178,0x00);
	mis40c1_write_register(ViPipe, 0x617b,0xbd);
	mis40c1_write_register(ViPipe, 0x617a,0x0c);
	mis40c1_write_register(ViPipe, 0x617d,0x00);
	mis40c1_write_register(ViPipe, 0x617c,0x00);
	mis40c1_write_register(ViPipe, 0x617f,0x29);
	mis40c1_write_register(ViPipe, 0x617e,0x01);
	mis40c1_write_register(ViPipe, 0x6181,0x00);
	mis40c1_write_register(ViPipe, 0x6180,0x00);
	mis40c1_write_register(ViPipe, 0x6183,0x29);
	mis40c1_write_register(ViPipe, 0x6182,0x01);
	mis40c1_write_register(ViPipe, 0x6185,0x00);
	mis40c1_write_register(ViPipe, 0x6184,0x00);
	mis40c1_write_register(ViPipe, 0x6187,0x29);
	mis40c1_write_register(ViPipe, 0x6186,0x01);
	mis40c1_write_register(ViPipe, 0x61b5,0x5e);
	mis40c1_write_register(ViPipe, 0x61b4,0x04);
	mis40c1_write_register(ViPipe, 0x61b7,0x6d);
	mis40c1_write_register(ViPipe, 0x61b6,0x04);
	mis40c1_write_register(ViPipe, 0x6189,0xd2);
	mis40c1_write_register(ViPipe, 0x6188,0x0c);
	mis40c1_write_register(ViPipe, 0x618b,0xe1);
	mis40c1_write_register(ViPipe, 0x618a,0x0c);
	mis40c1_write_register(ViPipe, 0x618d,0xd0);
	mis40c1_write_register(ViPipe, 0x618c,0x00);
	mis40c1_write_register(ViPipe, 0x6191,0x7c);
	mis40c1_write_register(ViPipe, 0x6190,0x04);
	mis40c1_write_register(ViPipe, 0x618f,0xee);
	mis40c1_write_register(ViPipe, 0x618e,0x00);
	mis40c1_write_register(ViPipe, 0x6193,0x99);
	mis40c1_write_register(ViPipe, 0x6192,0x04);
	mis40c1_write_register(ViPipe, 0x6195,0x0d);
	mis40c1_write_register(ViPipe, 0x6194,0x0d);
	mis40c1_write_register(ViPipe, 0x61a3,0xd0);
	mis40c1_write_register(ViPipe, 0x61a2,0x01);
	mis40c1_write_register(ViPipe, 0x61a5,0xcc);
	mis40c1_write_register(ViPipe, 0x61a4,0x05);

	//BLC
	mis40c1_write_register(ViPipe, 0x5400,0x2B);
	mis40c1_write_register(ViPipe, 0x5403,0x08);
	mis40c1_write_register(ViPipe, 0x5406,0x00);
	mis40c1_write_register(ViPipe, 0x5408,0x3e);
	//DPC ON 20230821
	mis40c1_write_register(ViPipe, 0x3902,0x02);
	//Added in 2022/10/31 by chenzheng
	mis40c1_write_register(ViPipe, 0x3a04,0x10);
	mis40c1_write_register(ViPipe, 0x3a17,0x00);
	mis40c1_write_register(ViPipe, 0x3a18,0x00);
	mis40c1_write_register(ViPipe, 0x3048,0x01);
	mis40c1_write_register(ViPipe, 0x6207,0x00);
	mis40c1_write_register(ViPipe, 0x6208,0x00);
	mis40c1_write_register(ViPipe, 0x3103,0x03);
	mis40c1_write_register(ViPipe, 0x3104,0xe0);
	mis40c1_write_register(ViPipe, 0x3118,0x01);
	mis40c1_write_register(ViPipe, 0x3700,0x01);
	//Offset 0x40
	mis40c1_write_register(ViPipe, 0x3701,0x00);
	mis40c1_write_register(ViPipe, 0x3702,0x40);
	mis40c1_write_register(ViPipe, 0x3703,0x00);
	mis40c1_write_register(ViPipe, 0x3704,0x40);
	mis40c1_write_register(ViPipe, 0x3705,0x00);
	mis40c1_write_register(ViPipe, 0x3706,0x40);
	mis40c1_write_register(ViPipe, 0x3707,0x00);
	mis40c1_write_register(ViPipe, 0x3708,0x40);
	mis40c1_write_register(ViPipe, 0x300c,0x01);
	mis40c1_write_register(ViPipe, 0x610b,0x40);
	mis40c1_write_register(ViPipe, 0x613d,0x70);
	//Added in 20230717 by huangyuanxi);
	mis40c1_write_register(ViPipe, 0x61a1,0x90); //128x竖纹
	mis40c1_write_register(ViPipe, 0x6202,0x0c); //去高倍太阳黑子
	mis40c1_write_register(ViPipe, 0x3a03,0x39);
	mis40c1_write_register(ViPipe, 0x3a02,0xfc); //0xfd功耗最小基础上PAC OFF 20230810
	mis40c1_write_register(ViPipe, 0x3a08,0x0c);
	mis40c1_write_register(ViPipe, 0x3048,0x01);
	//PAC对称
	mis40c1_write_register(ViPipe, 0x61ae,0x02);
	mis40c1_write_register(ViPipe, 0x61af,0x50);
	mis40c1_write_register(ViPipe, 0x61b2,0x06);
	mis40c1_write_register(ViPipe, 0x61b3,0x50);
	mis40c1_write_register(ViPipe, 0x3040,0x01);
	mis40c1_write_register(ViPipe, 0x6200,0x09); //CM电流次小 优化横带
	mis40c1_write_register(ViPipe, 0x6201,0x09); //RCS电流次小 优化横带

	//优化灯管横带   20230821
	mis40c1_write_register(ViPipe, 0x3a02,0x2d);
	mis40c1_write_register(ViPipe, 0x3a03,0x19);
	mis40c1_write_register(ViPipe, 0x6200,0x00);
	mis40c1_write_register(ViPipe, 0x6201,0x09);
	mis40c1_write_register(ViPipe, 0x6124,0x1f);
	mis40c1_write_register(ViPipe, 0x6125,0xff);
	mis40c1_write_register(ViPipe, 0x6126,0x00);
	mis40c1_write_register(ViPipe, 0x6127,0x00);

	//Eclp电流减小弱化亮点问题
	mis40c1_write_register(ViPipe, 0x3a13,0x39);
	mis40c1_write_register(ViPipe, 0x3a15,0x08);

	//高温闪烁/BLC不准/画面异常   20230824
	mis40c1_write_register(ViPipe, 0x3a02,0x2d);
	mis40c1_write_register(ViPipe, 0x5400,0x23);
	mis40c1_write_register(ViPipe, 0x3a0c,0x07);
	mis40c1_write_register(ViPipe, 0x3040,0x01);

	//上电竖纹问题优化   20230828);
	mis40c1_write_register(ViPipe, 0x6199,0x90);
	mis40c1_write_register(ViPipe, 0x619b,0x90);
	mis40c1_write_register(ViPipe, 0x619d,0x90);
	mis40c1_write_register(ViPipe, 0x619f,0x90);
	mis40c1_write_register(ViPipe, 0x61a1,0x90);
	//IIC片内切换并降低时钟频率   20230829);
	mis40c1_write_register(ViPipe, 0x420c,0x00);
	mis40c1_write_register(ViPipe, 0x420d,0x0a);
	//1x 部分芯片竖线问题   20230829);
	mis40c1_write_register(ViPipe, 0x3a01,0x01);
	//优化灯管横带问题   20230831//20231218降);低功耗
	mis40c1_write_register(ViPipe, 0x6209,0x12);
	mis40c1_write_register(ViPipe, 0x3a02,0xad);
	mis40c1_write_register(ViPipe, 0x610b,0x08);//君正竖纹改善20231218
	mis40c1_write_register(ViPipe, 0x3a14,0x0c);//君正竖纹改善20231220

	//改善CG不线性,3a0e=60会使得CG不线性和CG偏);大 20240119
	mis40c1_write_register(ViPipe, 0x3a0e,0xe0);

	//优化BLC闪烁问题);
	mis40c1_write_register(ViPipe, 0x3a05,0x6a);
	mis40c1_write_register(ViPipe, 0x3048,0x01);
	mis40c1_write_register(ViPipe, 0x3a0d,0x32);
	mis40c1_write_register(ViPipe, 0x3040,0x01);
	mis40c1_write_register(ViPipe, 0x5407,0x02);
	mis40c1_write_register(ViPipe, 0x540d,0x01);
	mis40c1_write_register(ViPipe, 0x540d,0x03);
	mis40c1_write_register(ViPipe, 0x540e,0xff);

	//优化偏粉问题);
	mis40c1_write_register(ViPipe, 0x5403,0x0f);
	mis40c1_write_register(ViPipe, 0x5404,0xff);

	//mipi 连续与非连续);
	//0x3c35,0x8c//comtinue 8d);
	//0x3047,0xf5//continue be);

	mis40c1_write_register(ViPipe, 0x3006,0x00);
	mis40c1_write_register(ViPipe, 0x302d,0x00);

	// 2564x1444
	mis40c1_write_register(ViPipe, 0x3109,0x00);
	mis40c1_write_register(ViPipe, 0x310a,0x00);
	mis40c1_write_register(ViPipe, 0x310b,0x05);
	mis40c1_write_register(ViPipe, 0x310c,0xa3);
	mis40c1_write_register(ViPipe, 0x310d,0x00);
	mis40c1_write_register(ViPipe, 0x310e,0x00);
	mis40c1_write_register(ViPipe, 0x310f,0x0a);
	mis40c1_write_register(ViPipe, 0x3110,0x03);
	mis40c1_write_register(ViPipe, 0x3c1d,0x0a);
	mis40c1_write_register(ViPipe, 0x3c1e,0x04);
	mis40c1_write_register(ViPipe, 0x3c1f,0x05);
	mis40c1_write_register(ViPipe, 0x3c20,0xa4);

	mis40c1_default_reg_init(ViPipe);


	delay_ms(100);

	printf("ViPipe:%d,===MIS40C1 1440P 20fps 2lane 10bit LINE Init OK!===\n", ViPipe);
}
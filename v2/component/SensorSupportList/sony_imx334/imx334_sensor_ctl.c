#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "imx334_cmos_ex.h"
#include "sensor_i2c.h"

static void imx334_linear_8M30_init(VI_PIPE ViPipe);
static void imx334_linear_8M60_init(VI_PIPE ViPipe);
static void imx334_wdr_8M30_2to1_init(VI_PIPE ViPipe);

const CVI_U8 imx334_i2c_addr = 0x1A;

const CVI_U32 imx334_addr_byte = 2;
const CVI_U32 imx334_data_byte = 1;

int imx334_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunImx334_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx334_AddrInfo[ViPipe].s8I2cAddr);
}

int imx334_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunImx334_BusInfo[ViPipe].s8I2cDev);
}

int imx334_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunImx334_BusInfo[ViPipe].s8I2cDev,
			imx334_i2c_addr, (CVI_U32)addr, imx334_addr_byte, imx334_data_byte);
}

int imx334_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunImx334_BusInfo[ViPipe].s8I2cDev,
			imx334_i2c_addr, (CVI_U32)addr, imx334_addr_byte,
			(CVI_U32)data, imx334_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void imx334_standby(VI_PIPE ViPipe)
{
	imx334_write_register(ViPipe, 0x3000, 0x01); /* STANDBY */
	imx334_write_register(ViPipe, 0x3002, 0x01); /* XTMSTA */
}

void imx334_restart(VI_PIPE ViPipe)
{
	imx334_write_register(ViPipe, 0x3000, 0x00); /* standby */
	delay_ms(20);
	imx334_write_register(ViPipe, 0x3002, 0x00); /* master mode start */
}

void imx334_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastImx334[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		imx334_write_register(ViPipe,
				g_pastImx334[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastImx334[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void imx334_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 u8Filp = 0;
	CVI_U8 u8Mirror = 0;

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		u8Mirror = 1;
		break;
	case ISP_SNS_FLIP:
		u8Filp = 1;
		break;
	case ISP_SNS_MIRROR_FLIP:
		u8Filp = 1;
		u8Mirror = 1;
		break;
	default:
		return;
	}

	imx334_write_register(ViPipe, 0x304e, u8Mirror);
	imx334_write_register(ViPipe, 0x304f, u8Filp);
	if (u8Filp != 0) {
		imx334_write_register(ViPipe, 0x3074, 0xc0);
		imx334_write_register(ViPipe, 0x3075, 0x11);
		imx334_write_register(ViPipe, 0x308e, 0xc1);
		imx334_write_register(ViPipe, 0x308f, 0x11);
		imx334_write_register(ViPipe, 0x3080, 0xfe);
		imx334_write_register(ViPipe, 0x309b, 0xfe);
		imx334_write_register(ViPipe, 0x30b6, 0xfa);
		imx334_write_register(ViPipe, 0x30b7, 0x01);
		imx334_write_register(ViPipe, 0x3116, 0x02);
		imx334_write_register(ViPipe, 0x3117, 0x00);
	} else {
		imx334_write_register(ViPipe, 0x3074, 0xb0);
		imx334_write_register(ViPipe, 0x3075, 0x00);
		imx334_write_register(ViPipe, 0x308e, 0xb1);
		imx334_write_register(ViPipe, 0x308f, 0x00);
		imx334_write_register(ViPipe, 0x3080, 0x02);
		imx334_write_register(ViPipe, 0x309b, 0x02);
		imx334_write_register(ViPipe, 0x30b6, 0x00);
		imx334_write_register(ViPipe, 0x30b7, 0x00);
		imx334_write_register(ViPipe, 0x3116, 0x08);
		imx334_write_register(ViPipe, 0x3117, 0x00);
	}
}

void imx334_init(VI_PIPE ViPipe)
{
	WDR_MODE_E        enWDRMode;
	CVI_U8            u8ImgMode;

	enWDRMode   = g_pastImx334[ViPipe]->enWDRMode;
	u8ImgMode   = g_pastImx334[ViPipe]->u8ImgMode;

	imx334_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		if (u8ImgMode == IMX334_MODE_8M30_WDR) {
			imx334_wdr_8M30_2to1_init(ViPipe);
		}
	} else {
		if (u8ImgMode == IMX334_MODE_8M30)
			imx334_linear_8M30_init(ViPipe);
		else if (u8ImgMode == IMX334_MODE_8M60)
			imx334_linear_8M60_init(ViPipe);
		else {
		}
	}
	g_pastImx334[ViPipe]->bInit = CVI_TRUE;
}

// void imx334_exit(VI_PIPE ViPipe)
// {
// 	imx334_i2c_exit(ViPipe);
// }
static void imx334_linear_8M30_init(VI_PIPE ViPipe)
{
	delay_ms(4);
	imx334_write_register(ViPipe, 0x3000, 0x01); // STANDBY
	imx334_write_register(ViPipe, 0x3002, 0x01); // XTMSTA

	imx334_write_register(ViPipe, 0x300C, 0x5B);// BCWAIT_TIME[7:0]
	imx334_write_register(ViPipe, 0x300D, 0x40);// CPWAIT_TIME[7:0]
	imx334_write_register(ViPipe, 0x3034, 0x4C);// HMAX[15:0]
	imx334_write_register(ViPipe, 0x3035, 0x04);//
	imx334_write_register(ViPipe, 0x314C, 0x80);// INCKSEL 1[8:0]
	imx334_write_register(ViPipe, 0x315A, 0x02);// INCKSEL2[1:0]
	imx334_write_register(ViPipe, 0x316A, 0x7E);// INCKSEL4[1:0]
	imx334_write_register(ViPipe, 0x319E, 0x01);// SYS_MODE
	imx334_write_register(ViPipe, 0x31A1, 0x00);// XVS_DRV[1:0]
	imx334_write_register(ViPipe, 0x3288, 0x21);// -
	imx334_write_register(ViPipe, 0x328A, 0x02);// -
	imx334_write_register(ViPipe, 0x3414, 0x05);// -
	imx334_write_register(ViPipe, 0x3416, 0x18);// -
	imx334_write_register(ViPipe, 0x35AC, 0x0E);// -
	imx334_write_register(ViPipe, 0x3648, 0x01);// -
	imx334_write_register(ViPipe, 0x364A, 0x04);// -
	imx334_write_register(ViPipe, 0x364C, 0x04);// -
	imx334_write_register(ViPipe, 0x3678, 0x01);// -
	imx334_write_register(ViPipe, 0x367C, 0x31);// -
	imx334_write_register(ViPipe, 0x367E, 0x31);// -
	imx334_write_register(ViPipe, 0x3708, 0x02);// -
	imx334_write_register(ViPipe, 0x3714, 0x01);// -
	imx334_write_register(ViPipe, 0x3715, 0x02);// -
	imx334_write_register(ViPipe, 0x3716, 0x02);// -
	imx334_write_register(ViPipe, 0x3717, 0x02);// -
	imx334_write_register(ViPipe, 0x371C, 0x3D);// -
	imx334_write_register(ViPipe, 0x371D, 0x3F);// -
	imx334_write_register(ViPipe, 0x372C, 0x00);// -
	imx334_write_register(ViPipe, 0x372D, 0x00);// -
	imx334_write_register(ViPipe, 0x372E, 0x46);// -
	imx334_write_register(ViPipe, 0x372F, 0x00);// -
	imx334_write_register(ViPipe, 0x3730, 0x89);// -
	imx334_write_register(ViPipe, 0x3731, 0x00);// -
	imx334_write_register(ViPipe, 0x3732, 0x08);// -
	imx334_write_register(ViPipe, 0x3733, 0x01);// -
	imx334_write_register(ViPipe, 0x3734, 0xFE);// -
	imx334_write_register(ViPipe, 0x3735, 0x05);// -
	imx334_write_register(ViPipe, 0x375D, 0x00);// -
	imx334_write_register(ViPipe, 0x375E, 0x00);// -
	imx334_write_register(ViPipe, 0x375F, 0x61);// -
	imx334_write_register(ViPipe, 0x3760, 0x06);// -
	imx334_write_register(ViPipe, 0x3768, 0x1B);// -
	imx334_write_register(ViPipe, 0x3769, 0x1B);// -
	imx334_write_register(ViPipe, 0x376A, 0x1A);// -
	imx334_write_register(ViPipe, 0x376B, 0x19);// -
	imx334_write_register(ViPipe, 0x376C, 0x18);// -
	imx334_write_register(ViPipe, 0x376D, 0x14);// -
	imx334_write_register(ViPipe, 0x376E, 0x0F);// -
	imx334_write_register(ViPipe, 0x3776, 0x00);// -
	imx334_write_register(ViPipe, 0x3777, 0x00);// -
	imx334_write_register(ViPipe, 0x3778, 0x46);// -
	imx334_write_register(ViPipe, 0x3779, 0x00);// -
	imx334_write_register(ViPipe, 0x377A, 0x08);// -
	imx334_write_register(ViPipe, 0x377B, 0x01);// -
	imx334_write_register(ViPipe, 0x377C, 0x45);// -
	imx334_write_register(ViPipe, 0x377D, 0x01);// -
	imx334_write_register(ViPipe, 0x377E, 0x23);// -
	imx334_write_register(ViPipe, 0x377F, 0x02);// -
	imx334_write_register(ViPipe, 0x3780, 0xD9);// -
	imx334_write_register(ViPipe, 0x3781, 0x03);// -
	imx334_write_register(ViPipe, 0x3782, 0xF5);// -
	imx334_write_register(ViPipe, 0x3783, 0x06);// -
	imx334_write_register(ViPipe, 0x3784, 0xA5);// -
	imx334_write_register(ViPipe, 0x3788, 0x0F);// -
	imx334_write_register(ViPipe, 0x378A, 0xD9);// -
	imx334_write_register(ViPipe, 0x378B, 0x03);// -
	imx334_write_register(ViPipe, 0x378C, 0xEB);// -
	imx334_write_register(ViPipe, 0x378D, 0x05);// -
	imx334_write_register(ViPipe, 0x378E, 0x87);// -
	imx334_write_register(ViPipe, 0x378F, 0x06);// -
	imx334_write_register(ViPipe, 0x3790, 0xF5);// -
	imx334_write_register(ViPipe, 0x3792, 0x43);// -
	imx334_write_register(ViPipe, 0x3794, 0x7A);// -
	imx334_write_register(ViPipe, 0x3796, 0xA1);// -
	imx334_write_register(ViPipe, 0x3A18, 0x8F);// TCLKPOST[15:0]
	imx334_write_register(ViPipe, 0x3A1A, 0x4F);// TCLKPREPARE[15:0]
	imx334_write_register(ViPipe, 0x3A1C, 0x47);// TCLKTRAIL[15:0]
	imx334_write_register(ViPipe, 0x3A1E, 0x37);// TCLKZERO[15:0]
	imx334_write_register(ViPipe, 0x3A20, 0x4F);// THSPREPARE[15:0]
	imx334_write_register(ViPipe, 0x3A22, 0x87);// THSZERO[15:0]
	imx334_write_register(ViPipe, 0x3A24, 0x4F);// THSTRAIL[15:0]
	imx334_write_register(ViPipe, 0x3A26, 0x7F);// THSEXIT[15:0]
	imx334_write_register(ViPipe, 0x3A28, 0x3F);// TLPX[15:0]
	imx334_write_register(ViPipe, 0x3E04, 0x0E);// -
	imx334_default_reg_init(ViPipe);

	imx334_write_register(ViPipe, 0x3000, 0x00); /* standby */
	delay_ms(20);
	imx334_write_register(ViPipe, 0x3002, 0x00); /* master mode start */
	printf("ViPipe:%d,===IMX334 8M 30fps 12bit LINEAR Init OK!===\n", ViPipe);
}

static void imx334_linear_8M60_init(VI_PIPE ViPipe)
{
	delay_ms(4);
	imx334_write_register(ViPipe, 0x3000, 0x01); // STANDBY
	imx334_write_register(ViPipe, 0x3002, 0x01); // XTMSTA

	imx334_write_register(ViPipe, 0x300C, 0x5B);// BCWAIT_TIME[7:0]
	imx334_write_register(ViPipe, 0x300D, 0x40);// CPWAIT_TIME[7:0]
	imx334_write_register(ViPipe, 0x3058, 0x84);// SHR0[19:0]
	imx334_write_register(ViPipe, 0x3059, 0x03);//
	imx334_write_register(ViPipe, 0x30E8, 0x14);// GAIN[10:0]
	imx334_write_register(ViPipe, 0x315A, 0x02);// INCKSEL2[1:0]
	imx334_write_register(ViPipe, 0x316A, 0x7E);// INCKSEL4[1:0]
	imx334_write_register(ViPipe, 0x31A1, 0x00);// XVS_DRV[1:0]
	imx334_write_register(ViPipe, 0x3288, 0x21);// -
	imx334_write_register(ViPipe, 0x328A, 0x02);// -
	imx334_write_register(ViPipe, 0x3414, 0x05);// -
	imx334_write_register(ViPipe, 0x3416, 0x18);// -
	imx334_write_register(ViPipe, 0x35AC, 0x0E);// -
	imx334_write_register(ViPipe, 0x3648, 0x01);// -
	imx334_write_register(ViPipe, 0x364A, 0x04);// -
	imx334_write_register(ViPipe, 0x364C, 0x04);// -
	imx334_write_register(ViPipe, 0x3678, 0x01);// -
	imx334_write_register(ViPipe, 0x367C, 0x31);// -
	imx334_write_register(ViPipe, 0x367E, 0x31);// -
	imx334_write_register(ViPipe, 0x3708, 0x02);// -
	imx334_write_register(ViPipe, 0x3714, 0x01);// -
	imx334_write_register(ViPipe, 0x3715, 0x02);// -
	imx334_write_register(ViPipe, 0x3716, 0x02);// -
	imx334_write_register(ViPipe, 0x3717, 0x02);// -
	imx334_write_register(ViPipe, 0x371C, 0x3D);// -
	imx334_write_register(ViPipe, 0x371D, 0x3F);// -
	imx334_write_register(ViPipe, 0x372C, 0x00);// -
	imx334_write_register(ViPipe, 0x372D, 0x00);// -
	imx334_write_register(ViPipe, 0x372E, 0x46);// -
	imx334_write_register(ViPipe, 0x372F, 0x00);// -
	imx334_write_register(ViPipe, 0x3730, 0x89);// -
	imx334_write_register(ViPipe, 0x3731, 0x00);// -
	imx334_write_register(ViPipe, 0x3732, 0x08);// -
	imx334_write_register(ViPipe, 0x3733, 0x01);// -
	imx334_write_register(ViPipe, 0x3734, 0xFE);// -
	imx334_write_register(ViPipe, 0x3735, 0x05);// -
	imx334_write_register(ViPipe, 0x375D, 0x00);// -
	imx334_write_register(ViPipe, 0x375E, 0x00);// -
	imx334_write_register(ViPipe, 0x375F, 0x61);// -
	imx334_write_register(ViPipe, 0x3760, 0x06);// -
	imx334_write_register(ViPipe, 0x3768, 0x1B);// -
	imx334_write_register(ViPipe, 0x3769, 0x1B);// -
	imx334_write_register(ViPipe, 0x376A, 0x1A);// -
	imx334_write_register(ViPipe, 0x376B, 0x19);// -
	imx334_write_register(ViPipe, 0x376C, 0x18);// -
	imx334_write_register(ViPipe, 0x376D, 0x14);// -
	imx334_write_register(ViPipe, 0x376E, 0x0F);// -
	imx334_write_register(ViPipe, 0x3776, 0x00);// -
	imx334_write_register(ViPipe, 0x3777, 0x00);// -
	imx334_write_register(ViPipe, 0x3778, 0x46);// -
	imx334_write_register(ViPipe, 0x3779, 0x00);// -
	imx334_write_register(ViPipe, 0x377A, 0x08);// -
	imx334_write_register(ViPipe, 0x377B, 0x01);// -
	imx334_write_register(ViPipe, 0x377C, 0x45);// -
	imx334_write_register(ViPipe, 0x377D, 0x01);// -
	imx334_write_register(ViPipe, 0x377E, 0x23);// -
	imx334_write_register(ViPipe, 0x377F, 0x02);// -
	imx334_write_register(ViPipe, 0x3780, 0xD9);// -
	imx334_write_register(ViPipe, 0x3781, 0x03);// -
	imx334_write_register(ViPipe, 0x3782, 0xF5);// -
	imx334_write_register(ViPipe, 0x3783, 0x06);// -
	imx334_write_register(ViPipe, 0x3784, 0xA5);// -
	imx334_write_register(ViPipe, 0x3788, 0x0F);// -
	imx334_write_register(ViPipe, 0x378A, 0xD9);// -
	imx334_write_register(ViPipe, 0x378B, 0x03);// -
	imx334_write_register(ViPipe, 0x378C, 0xEB);// -
	imx334_write_register(ViPipe, 0x378D, 0x05);// -
	imx334_write_register(ViPipe, 0x378E, 0x87);// -
	imx334_write_register(ViPipe, 0x378F, 0x06);// -
	imx334_write_register(ViPipe, 0x3790, 0xF5);// -
	imx334_write_register(ViPipe, 0x3792, 0x43);// -
	imx334_write_register(ViPipe, 0x3794, 0x7A);// -
	imx334_write_register(ViPipe, 0x3796, 0xA1);// -
	imx334_write_register(ViPipe, 0x3E04, 0x0E);// -
	imx334_default_reg_init(ViPipe);

	imx334_write_register(ViPipe, 0x3000, 0x00); /* standby */
	delay_ms(300);
	imx334_write_register(ViPipe, 0x3002, 0x00); /* master mode start */
	printf("ViPipe:%d,===IMX334 8M 60fps 12bit LINEAR Init OK!===\n", ViPipe);
}

static void imx334_wdr_8M30_2to1_init(VI_PIPE ViPipe)
{
	delay_ms(4);
	imx334_write_register(ViPipe, 0x3000, 0x01); /* STANDBY */
	imx334_write_register(ViPipe, 0x3002, 0x01); /* XTMSTA */

	imx334_write_register(ViPipe, 0x300C, 0x5B);// BCWAIT_TIME[7:0]
	imx334_write_register(ViPipe, 0x300D, 0x40);// CPWAIT_TIME[7:0]
	imx334_write_register(ViPipe, 0x3048, 0x01);// WDMODE[0]
	imx334_write_register(ViPipe, 0x3049, 0x01);// WDSEL[1:0]
	imx334_write_register(ViPipe, 0x304A, 0x01);// WD_SET1[2:0]
	imx334_write_register(ViPipe, 0x304B, 0x02);// WD_SET2[3:0]
	imx334_write_register(ViPipe, 0x304C, 0x13);// OPB_SIZE_V[5:0]
	imx334_write_register(ViPipe, 0x3058, 0xB4);// SHR0[19:0]
	imx334_write_register(ViPipe, 0x3068, 0x25);// RHS1[19:0]
	imx334_write_register(ViPipe, 0x315A, 0x02);// INCKSEL2[1:0]
	imx334_write_register(ViPipe, 0x316A, 0x7E);// INCKSEL4[1:0]
	imx334_write_register(ViPipe, 0x31A1, 0x00);// XVS_DRV[1:0]
	imx334_write_register(ViPipe, 0x31D7, 0x01);// XVSMSKCNT_INT[1:0]
	imx334_write_register(ViPipe, 0x3200, 0x11);// FGAINEN[0] each frame gain adjustment disable
	imx334_write_register(ViPipe, 0x3288, 0x21);// -
	imx334_write_register(ViPipe, 0x328A, 0x02);// -
	imx334_write_register(ViPipe, 0x3414, 0x05);// -
	imx334_write_register(ViPipe, 0x3416, 0x18);// -
	imx334_write_register(ViPipe, 0x35AC, 0x0E);// -
	imx334_write_register(ViPipe, 0x3648, 0x01);// -
	imx334_write_register(ViPipe, 0x364A, 0x04);// -
	imx334_write_register(ViPipe, 0x364C, 0x04);// -
	imx334_write_register(ViPipe, 0x3678, 0x01);// -
	imx334_write_register(ViPipe, 0x367C, 0x31);// -
	imx334_write_register(ViPipe, 0x367E, 0x31);// -
	imx334_write_register(ViPipe, 0x3708, 0x02);// -
	imx334_write_register(ViPipe, 0x3714, 0x01);// -
	imx334_write_register(ViPipe, 0x3715, 0x02);// -
	imx334_write_register(ViPipe, 0x3716, 0x02);// -
	imx334_write_register(ViPipe, 0x3717, 0x02);// -
	imx334_write_register(ViPipe, 0x371C, 0x3D);// -
	imx334_write_register(ViPipe, 0x371D, 0x3F);// -
	imx334_write_register(ViPipe, 0x372C, 0x00);// -
	imx334_write_register(ViPipe, 0x372D, 0x00);// -
	imx334_write_register(ViPipe, 0x372E, 0x46);// -
	imx334_write_register(ViPipe, 0x372F, 0x00);// -
	imx334_write_register(ViPipe, 0x3730, 0x89);// -
	imx334_write_register(ViPipe, 0x3731, 0x00);// -
	imx334_write_register(ViPipe, 0x3732, 0x08);// -
	imx334_write_register(ViPipe, 0x3733, 0x01);// -
	imx334_write_register(ViPipe, 0x3734, 0xFE);// -
	imx334_write_register(ViPipe, 0x3735, 0x05);// -
	imx334_write_register(ViPipe, 0x375D, 0x00);// -
	imx334_write_register(ViPipe, 0x375E, 0x00);// -
	imx334_write_register(ViPipe, 0x375F, 0x61);// -
	imx334_write_register(ViPipe, 0x3760, 0x06);// -
	imx334_write_register(ViPipe, 0x3768, 0x1B);// -
	imx334_write_register(ViPipe, 0x3769, 0x1B);// -
	imx334_write_register(ViPipe, 0x376A, 0x1A);// -
	imx334_write_register(ViPipe, 0x376B, 0x19);// -
	imx334_write_register(ViPipe, 0x376C, 0x18);// -
	imx334_write_register(ViPipe, 0x376D, 0x14);// -
	imx334_write_register(ViPipe, 0x376E, 0x0F);// -
	imx334_write_register(ViPipe, 0x3776, 0x00);// -
	imx334_write_register(ViPipe, 0x3777, 0x00);// -
	imx334_write_register(ViPipe, 0x3778, 0x46);// -
	imx334_write_register(ViPipe, 0x3779, 0x00);// -
	imx334_write_register(ViPipe, 0x377A, 0x08);// -
	imx334_write_register(ViPipe, 0x377B, 0x01);// -
	imx334_write_register(ViPipe, 0x377C, 0x45);// -
	imx334_write_register(ViPipe, 0x377D, 0x01);// -
	imx334_write_register(ViPipe, 0x377E, 0x23);// -
	imx334_write_register(ViPipe, 0x377F, 0x02);// -
	imx334_write_register(ViPipe, 0x3780, 0xD9);// -
	imx334_write_register(ViPipe, 0x3781, 0x03);// -
	imx334_write_register(ViPipe, 0x3782, 0xF5);// -
	imx334_write_register(ViPipe, 0x3783, 0x06);// -
	imx334_write_register(ViPipe, 0x3784, 0xA5);// -
	imx334_write_register(ViPipe, 0x3788, 0x0F);// -
	imx334_write_register(ViPipe, 0x378A, 0xD9);// -
	imx334_write_register(ViPipe, 0x378B, 0x03);// -
	imx334_write_register(ViPipe, 0x378C, 0xEB);// -
	imx334_write_register(ViPipe, 0x378D, 0x05);// -
	imx334_write_register(ViPipe, 0x378E, 0x87);// -
	imx334_write_register(ViPipe, 0x378F, 0x06);// -
	imx334_write_register(ViPipe, 0x3790, 0xF5);// -
	imx334_write_register(ViPipe, 0x3792, 0x43);// -
	imx334_write_register(ViPipe, 0x3794, 0x7A);// -
	imx334_write_register(ViPipe, 0x3796, 0xA1);// -
	imx334_write_register(ViPipe, 0x3E04, 0x0E);// -
	imx334_default_reg_init(ViPipe);

	imx334_write_register(ViPipe, 0x3000, 0x00); /* standby */
	delay_ms(20);
	imx334_write_register(ViPipe, 0x3002, 0x00); /* master mode start */

	printf("===Imx334 sensor 8M30fps 12bit 2to1 WDR(60fps->30fps) init success!=====\n");
}

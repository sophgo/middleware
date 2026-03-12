#include <unistd.h>
#include "cvi_sns_ctrl.h"
#include <cvi_comm_video.h>
#include "imx662_cmos_ex.h"
#include "sensor_i2c.h"

static void imx662_wdr_1080p30_2to1_init(VI_PIPE ViPipe);
static void imx662_linear_1080p30_init(VI_PIPE ViPipe);

const CVI_U8 imx662_i2c_addr = 0x1A;
const CVI_U32 imx662_addr_byte = 2;
const CVI_U32 imx662_data_byte = 1;
//static int g_fd[VI_MAX_PIPE_NUM] = {[0 ... (VI_MAX_PIPE_NUM - 1)] = -1};

int imx662_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunImx662_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx662_AddrInfo[ViPipe].s8I2cAddr);
}

int imx662_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunImx662_BusInfo[ViPipe].s8I2cDev);
}

int imx662_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunImx662_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx662_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							imx662_addr_byte, imx662_data_byte);
}

int imx662_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunImx662_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx662_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							imx662_addr_byte, (CVI_U32)data, imx662_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void imx662_standby(VI_PIPE ViPipe)
{
	imx662_write_register(ViPipe, 0x3000, 0x01); /* STANDBY */
	imx662_write_register(ViPipe, 0x3002, 0x01); /* XTMSTA */

	printf("imx662 standby\n");
}

void imx662_restart(VI_PIPE ViPipe)
{
	imx662_write_register(ViPipe, 0x3000, 0x00); /* standby */
	delay_ms(20);
	imx662_write_register(ViPipe, 0x3002, 0x00); /* master mode start */
	imx662_write_register(ViPipe, 0x304b, 0x0a);

    printf("imx662 restart\n");
}

void imx662_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastImx662[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		imx662_write_register(ViPipe,
				g_pastImx662[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastImx662[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

#define IMX662_CHIP_ID_ADDR	0x31dc
#define IMX662_CHIP_ID		0x4
#define IMX662_CHIP_ID_MASK	0x6

void imx662_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 h_val = imx662_read_register(ViPipe, 0x3020) & ~0x1;
	CVI_U8 v_val = imx662_read_register(ViPipe, 0x3021) & ~0x1;
	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		h_val |= 0x1;
		break;
	case ISP_SNS_FLIP:
		v_val |= 0x1;
		break;
	case ISP_SNS_MIRROR_FLIP:
		v_val |= 0x1;
		h_val |= 0x1;
		break;
	default:
		return;
	}

	imx662_write_register(ViPipe, 0x3020, h_val);
	imx662_write_register(ViPipe, 0x3021, v_val);
}

int imx662_probe(VI_PIPE ViPipe)
{
	int nVal;

	delay_ms(1); //waitting i2c stable
	if (imx662_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal = imx662_read_register(ViPipe, IMX662_CHIP_ID_ADDR);
	if (nVal < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((nVal & IMX662_CHIP_ID_MASK) != IMX662_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID %d Mismatch! Use the wrong sensor??\n", nVal);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void imx662_init(VI_PIPE ViPipe)
{
	WDR_MODE_E        enWDRMode;
	CVI_U8            u8ImgMode;

	enWDRMode   = g_pastImx662[ViPipe]->enWDRMode;
	u8ImgMode   = g_pastImx662[ViPipe]->u8ImgMode;

	imx662_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		if (u8ImgMode == IMX662_MODE_1080P30_WDR) {
			imx662_wdr_1080p30_2to1_init(ViPipe);
		}
	} else {
		imx662_linear_1080p30_init(ViPipe);
	}
	g_pastImx662[ViPipe]->bInit = CVI_TRUE;
}

/* 1080P30 and 1080P25 */
static void imx662_linear_1080p30_init(VI_PIPE ViPipe)
{
	imx662_write_register(ViPipe, 0x3002, 0x01); /* ADBIT*/
	delay_ms(1);
	imx662_write_register(ViPipe, 0x3000, 0x01); /* STANDBY */
	delay_ms(1);
	imx662_write_register(ViPipe, 0x3002, 0x01);
	imx662_write_register(ViPipe, 0x3014, 0x03);  // INCK_SEL [3:0]
	imx662_write_register(ViPipe, 0x3015, 0x04);  // DATARATE_SEL [3:0]
	imx662_write_register(ViPipe, 0x3018, 0x00);  // WINMODE [3:0]
	imx662_write_register(ViPipe, 0x3019, 0x00);  // CFMODE
	imx662_write_register(ViPipe, 0x301a, 0x00);  // WDMODE
	imx662_write_register(ViPipe, 0x301b, 0x00);  // ADDMODE[1:0]
	imx662_write_register(ViPipe, 0x301c, 0x00);  // THIN_V_EN
	imx662_write_register(ViPipe, 0x301e, 0x01);  // VCMODE
	imx662_write_register(ViPipe, 0x3020, 0x00);  // HREVERSE
	imx662_write_register(ViPipe, 0x3021, 0x00);  // VREVERSE
	imx662_write_register(ViPipe, 0x3022, 0x00);  // ADBIT
	imx662_write_register(ViPipe, 0x3023, 0x00);  // MDBIT [1:0]
	imx662_write_register(ViPipe, 0x3028, 0xc4);  // VMAX [19:0]
	imx662_write_register(ViPipe, 0x3029, 0x09);  //
	imx662_write_register(ViPipe, 0x302a, 0x00);  //
	imx662_write_register(ViPipe, 0x302C, 0xbc);  // HMAX [15:0]
	imx662_write_register(ViPipe, 0x302D, 0x07);  //
	imx662_write_register(ViPipe, 0x3030, 0x00);  //FDG_SEL0[1:0]
	imx662_write_register(ViPipe, 0x3031, 0x00);  //FDG_SEL1[1:0]
	imx662_write_register(ViPipe, 0x3032, 0x00);  //FDG_SEL2[1:0]
	imx662_write_register(ViPipe, 0x303c, 0x00);  //PIX_HST[12:0]
	imx662_write_register(ViPipe, 0x303d, 0x00);  //PIX_HST[12:0]
	imx662_write_register(ViPipe, 0x303e, 0x90);  //PIX_HWIDTH[12:0]
	imx662_write_register(ViPipe, 0x303f, 0x07);  //PIX_HWIDTH[12:0]
	imx662_write_register(ViPipe, 0x3040, 0x01);  //PIX_HWIDTH[12:0]
	imx662_write_register(ViPipe, 0x3044, 0x00);  //PIX_VST[12:0]
	imx662_write_register(ViPipe, 0x3045, 0x00);  //PIX_VST[12:0]
	imx662_write_register(ViPipe, 0x3046, 0x4c);  //PIX_VWIDTH[11:0]
	imx662_write_register(ViPipe, 0x3047, 0x04);  //PIX_VWIDTH[11:0]
	imx662_write_register(ViPipe, 0x3050, 0x04);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3051, 0x00);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3052, 0x00);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3054, 0x0e);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3055, 0x00);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3056, 0x00);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3058, 0x8a);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3059, 0x01);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x305a, 0x00);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3060, 0x16);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3061, 0x01);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3062, 0x00);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3064, 0xc4);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3065, 0x0c);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3066, 0x00);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3069, 0x00);  // CHDR_GAIN_EN
	imx662_write_register(ViPipe, 0x3070, 0x00);  // GAIN [19:0]
	imx662_write_register(ViPipe, 0x3071, 0x00);  // GAIN [19:0]
	imx662_write_register(ViPipe, 0x3072, 0x00);  // GAIN_1 [19:0]
	imx662_write_register(ViPipe, 0x3073, 0x00);  // GAIN_1 [19:0]
	imx662_write_register(ViPipe, 0x3074, 0x00);  // GAIN_2 [19:0]
	imx662_write_register(ViPipe, 0x3075, 0x00);  // GAIN_2 [19:0]
	imx662_write_register(ViPipe, 0x3081, 0x00);  // EXP_GAIN [19:0]
	imx662_write_register(ViPipe, 0x308c, 0x00);  // CHDR_DGAIN0_HG [19:0]
	imx662_write_register(ViPipe, 0x308d, 0x01);  // CHDR_DGAIN0_HG [19:0]
	imx662_write_register(ViPipe, 0x3094, 0x00);  // CHDR_AGAIN0_LG [19:0]
	imx662_write_register(ViPipe, 0x3095, 0x00);  // CHDR_AGAIN0_LG [19:0]
	imx662_write_register(ViPipe, 0x3096, 0x00);  // CHDR_AGAIN1 [19:0]
	imx662_write_register(ViPipe, 0x3097, 0x00);  // CHDR_AGAIN1 [19:0]
	imx662_write_register(ViPipe, 0x309c, 0x00);  // CHDR_AGAIN0_HG [19:0]
	imx662_write_register(ViPipe, 0x309d, 0x00);  // CHDR_AGAIN0_HG [19:0]
	imx662_write_register(ViPipe, 0x30A4, 0xaa);  // XVSOUTSEL [1:0]
	imx662_write_register(ViPipe, 0x30A6, 0x00);  // XVS_DRV [1:0]
	imx662_write_register(ViPipe, 0x30cc, 0x00);  // XVS_DRV [1:0]
	imx662_write_register(ViPipe, 0x30cd, 0x00);  // XVS_DRV [1:0]
	imx662_write_register(ViPipe, 0x30dc, 0x32);  // XVS_DRV [1:0]
	imx662_write_register(ViPipe, 0x30dd, 0x40);  // XVS_DRV [1:0]
	imx662_write_register(ViPipe, 0x3400, 0x01);  // XVS_DRV [1:0]

	imx662_write_register(ViPipe, 0x3444, 0xAC);  // -
	imx662_write_register(ViPipe, 0x3460, 0x21);  // -
	imx662_write_register(ViPipe, 0x3492, 0x08);  // -
	imx662_write_register(ViPipe, 0x3A50, 0x62);  // -
	imx662_write_register(ViPipe, 0x3A51, 0x01);  // -
	imx662_write_register(ViPipe, 0x3A52, 0x19);  // -
	imx662_write_register(ViPipe, 0x3B00, 0x39);  // -
	imx662_write_register(ViPipe, 0x3B23, 0x2D);  // -
	imx662_write_register(ViPipe, 0x3B45, 0x04);  // -
	imx662_write_register(ViPipe, 0x3C0A, 0x1F);  // -
	imx662_write_register(ViPipe, 0x3C0B, 0x1E);  // -
	imx662_write_register(ViPipe, 0x3C38, 0x21);  // -
	imx662_write_register(ViPipe, 0x3C40, 0x06);  // -
	imx662_write_register(ViPipe, 0x3C44, 0x00);  // -
	imx662_write_register(ViPipe, 0x3CB6, 0xD8);  // -
	imx662_write_register(ViPipe, 0x3CC4, 0xDA);  // -
	imx662_write_register(ViPipe, 0x3E24, 0x79);  // -
	imx662_write_register(ViPipe, 0x3E2C, 0x15);  // -
	imx662_write_register(ViPipe, 0x3EDC, 0x2D);  // -
	imx662_write_register(ViPipe, 0x4498, 0x05);  // -
	imx662_write_register(ViPipe, 0x449C, 0x19);  // -
	imx662_write_register(ViPipe, 0x449D, 0x00);  // -
	imx662_write_register(ViPipe, 0x449E, 0x32);  // -
	imx662_write_register(ViPipe, 0x449F, 0x01);  // -
	imx662_write_register(ViPipe, 0x44A0, 0x92);  // -
	imx662_write_register(ViPipe, 0x44A2, 0x91);  // -
	imx662_write_register(ViPipe, 0x44A4, 0x8C);  // -
	imx662_write_register(ViPipe, 0x44A6, 0x87);  // -
	imx662_write_register(ViPipe, 0x44A8, 0x82);  // -
	imx662_write_register(ViPipe, 0x44AA, 0x78);  // -
	imx662_write_register(ViPipe, 0x44AC, 0x6E);  // -
	imx662_write_register(ViPipe, 0x44AE, 0x69);  // -
	imx662_write_register(ViPipe, 0x44B0, 0x92);  // -
	imx662_write_register(ViPipe, 0x44B2, 0x91);  // -
	imx662_write_register(ViPipe, 0x44B4, 0x8C);  // -
	imx662_write_register(ViPipe, 0x44B6, 0x87);  // -
	imx662_write_register(ViPipe, 0x44B8, 0x82);  // -
	imx662_write_register(ViPipe, 0x44BA, 0x78);  // -
	imx662_write_register(ViPipe, 0x44BC, 0x6E);  // -
	imx662_write_register(ViPipe, 0x44BE, 0x69);  // -
	imx662_write_register(ViPipe, 0x44C0, 0x7F);  // -
	imx662_write_register(ViPipe, 0x44C1, 0x01);  // -
	imx662_write_register(ViPipe, 0x44C2, 0x7F);  // -
	imx662_write_register(ViPipe, 0x44C3, 0x01);  // -
	imx662_write_register(ViPipe, 0x44C4, 0x7A);  // -
	imx662_write_register(ViPipe, 0x44C5, 0x01);  // -
	imx662_write_register(ViPipe, 0x44C6, 0x7A);  // -
	imx662_write_register(ViPipe, 0x44C7, 0x01);  // -
	imx662_write_register(ViPipe, 0x44C8, 0x70);  // -
	imx662_write_register(ViPipe, 0x44C9, 0x01);  // -
	imx662_write_register(ViPipe, 0x44CA, 0x6B);  // -
	imx662_write_register(ViPipe, 0x44CB, 0x01);  // -
	imx662_write_register(ViPipe, 0x44CC, 0x6B);  // -
	imx662_write_register(ViPipe, 0x44CD, 0x01);  // -
	imx662_write_register(ViPipe, 0x44CE, 0x5C);  // -
	imx662_write_register(ViPipe, 0x44CF, 0x01);  // -
	imx662_write_register(ViPipe, 0x44D0, 0x7F);  // -
	imx662_write_register(ViPipe, 0x44D1, 0x01);  // -
	imx662_write_register(ViPipe, 0x44D2, 0x7F);  // -
	imx662_write_register(ViPipe, 0x44D3, 0x01);  // -
	imx662_write_register(ViPipe, 0x44D4, 0x7A);  // -
	imx662_write_register(ViPipe, 0x44D5, 0x01);  // -
	imx662_write_register(ViPipe, 0x44D6, 0x7A);  // -
	imx662_write_register(ViPipe, 0x44D7, 0x01);  // -
	imx662_write_register(ViPipe, 0x44D8, 0x70);  // -
	imx662_write_register(ViPipe, 0x44D9, 0x01);  // -
	imx662_write_register(ViPipe, 0x44DA, 0x6B);  // -
	imx662_write_register(ViPipe, 0x44DB, 0x01);  // -
	imx662_write_register(ViPipe, 0x44DC, 0x6B);  // -
	imx662_write_register(ViPipe, 0x44DD, 0x01);  // -
	imx662_write_register(ViPipe, 0x44DE, 0x5C);  // -
	imx662_write_register(ViPipe, 0x44DF, 0x01);  // -
	imx662_write_register(ViPipe, 0x4534, 0x1C);  // -
	imx662_write_register(ViPipe, 0x4535, 0x03);  // -
	imx662_write_register(ViPipe, 0x4538, 0x1C);  // -
	imx662_write_register(ViPipe, 0x4539, 0x1C);  // -
	imx662_write_register(ViPipe, 0x453A, 0x1C);  // -
	imx662_write_register(ViPipe, 0x453B, 0x1C);  // -
	imx662_write_register(ViPipe, 0x453C, 0x1C);  // -
	imx662_write_register(ViPipe, 0x453D, 0x1C);  // -
	imx662_write_register(ViPipe, 0x453E, 0x1C);  // -
	imx662_write_register(ViPipe, 0x453F, 0x1C);  // -
	imx662_write_register(ViPipe, 0x4540, 0x1C);  // -
	imx662_write_register(ViPipe, 0x4541, 0x03);  // -
	imx662_write_register(ViPipe, 0x4542, 0x03);  // -
	imx662_write_register(ViPipe, 0x4543, 0x03);  // -
	imx662_write_register(ViPipe, 0x4544, 0x03);  // -
	imx662_write_register(ViPipe, 0x4545, 0x03);  // -
	imx662_write_register(ViPipe, 0x4546, 0x03);  // -
	imx662_write_register(ViPipe, 0x4547, 0x03);  // -
	imx662_write_register(ViPipe, 0x4548, 0x03);  // -
	imx662_write_register(ViPipe, 0x4549, 0x03);  // -

	imx662_default_reg_init(ViPipe);

	imx662_write_register(ViPipe, 0x3000, 0x00); /* standby */
	delay_ms(20);
	imx662_write_register(ViPipe, 0x3002, 0x00); /* master mode start */
	printf("ViPipe:%d,===IMX662 1080P 30fps 10bit LINE Init OK!===\n", ViPipe);
}

static void imx662_wdr_1080p30_2to1_init(VI_PIPE ViPipe)
{
	imx662_write_register(ViPipe, 0x3002, 0x01); /* ADBIT*/
	delay_ms(1);
	imx662_write_register(ViPipe, 0x3000, 0x01); /* STANDBY */
	delay_ms(1);
	imx662_write_register(ViPipe, 0x3002, 0x01);
	imx662_write_register(ViPipe, 0x3014, 0x03);  // INCK_SEL [3:0]
	imx662_write_register(ViPipe, 0x3015, 0x04);  // DATARATE_SEL [3:0]
	imx662_write_register(ViPipe, 0x3018, 0x00);  // WINMODE [3:0]
	imx662_write_register(ViPipe, 0x3019, 0x00);  // CFMODE
	imx662_write_register(ViPipe, 0x301a, 0x01);  // WDMODE
	imx662_write_register(ViPipe, 0x301b, 0x00);  // ADDMODE[1:0]
	imx662_write_register(ViPipe, 0x301c, 0x01);  // THIN_V_EN
	imx662_write_register(ViPipe, 0x301e, 0x01);  // VCMODE
	imx662_write_register(ViPipe, 0x3020, 0x00);  // HREVERSE
	imx662_write_register(ViPipe, 0x3021, 0x00);  // VREVERSE
	imx662_write_register(ViPipe, 0x3022, 0x00);  // ADBIT
	imx662_write_register(ViPipe, 0x3023, 0x00);  // MDBIT [1:0]
	imx662_write_register(ViPipe, 0x3028, 0xe2);  // VMAX [19:0]
	imx662_write_register(ViPipe, 0x3029, 0x04);  //
	imx662_write_register(ViPipe, 0x302a, 0x00);  //
	imx662_write_register(ViPipe, 0x302C, 0xde);  // HMAX [15:0]
	imx662_write_register(ViPipe, 0x302D, 0x03);  //
	imx662_write_register(ViPipe, 0x3030, 0x00);  //FDG_SEL0[1:0]
	imx662_write_register(ViPipe, 0x3031, 0x00);  //FDG_SEL1[1:0]
	imx662_write_register(ViPipe, 0x3032, 0x00);  //FDG_SEL2[1:0]
	imx662_write_register(ViPipe, 0x303c, 0x00);  //PIX_HST[12:0]
	imx662_write_register(ViPipe, 0x303d, 0x00);  //PIX_HST[12:0]
	imx662_write_register(ViPipe, 0x303e, 0x90);  //PIX_HWIDTH[12:0]
	imx662_write_register(ViPipe, 0x303f, 0x07);  //PIX_HWIDTH[12:0]
	imx662_write_register(ViPipe, 0x3040, 0x01);  //PIX_HWIDTH[12:0]
	imx662_write_register(ViPipe, 0x3044, 0x00);  //PIX_VST[12:0]
	imx662_write_register(ViPipe, 0x3045, 0x00);  //PIX_VST[12:0]
	imx662_write_register(ViPipe, 0x3046, 0x4c);  //PIX_VWIDTH[11:0]
	imx662_write_register(ViPipe, 0x3047, 0x04);  //PIX_VWIDTH[11:0]
	imx662_write_register(ViPipe, 0x3050, 0x14);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3051, 0x05);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3052, 0x00);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3054, 0x05);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3055, 0x00);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3056, 0x00);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3058, 0x8a);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3059, 0x01);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x305a, 0x00);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3060, 0x51);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3061, 0x00);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3062, 0x00);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3064, 0xc4);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3065, 0x0c);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3066, 0x00);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3069, 0x00);  // SHR0 [19:0]
	imx662_write_register(ViPipe, 0x3070, 0x00);  // GAIN [19:0]
	imx662_write_register(ViPipe, 0x3071, 0x00);  // GAIN [19:0]
	imx662_write_register(ViPipe, 0x3072, 0x00);  // GAIN_1 [19:0]
	imx662_write_register(ViPipe, 0x3073, 0x00);  // GAIN_1 [19:0]
	imx662_write_register(ViPipe, 0x3074, 0x00);  // GAIN_2 [19:0]
	imx662_write_register(ViPipe, 0x3075, 0x00);  // GAIN_2 [19:0]
	imx662_write_register(ViPipe, 0x3081, 0x00);  // EXP_GAIN [19:0]
	imx662_write_register(ViPipe, 0x308c, 0x00);  // CHDR_DGAIN0_HG [19:0]
	imx662_write_register(ViPipe, 0x308d, 0x01);  // CHDR_DGAIN0_HG [19:0]
	imx662_write_register(ViPipe, 0x3094, 0x00);  // CHDR_AGAIN0_LG [19:0]
	imx662_write_register(ViPipe, 0x3095, 0x00);  // CHDR_AGAIN0_LG [19:0]
	imx662_write_register(ViPipe, 0x3096, 0x00);  // CHDR_AGAIN1 [19:0]
	imx662_write_register(ViPipe, 0x3097, 0x00);  // CHDR_AGAIN1 [19:0]
	imx662_write_register(ViPipe, 0x309c, 0x00);  // CHDR_AGAIN0_HG [19:0]
	imx662_write_register(ViPipe, 0x309d, 0x00);  // CHDR_AGAIN0_HG [19:0]
	imx662_write_register(ViPipe, 0x30A4, 0xaa);  // XVSOUTSEL [1:0]
	imx662_write_register(ViPipe, 0x30A6, 0x00);  // XVS_DRV [1:0]
	imx662_write_register(ViPipe, 0x30cc, 0x00);  // XVS_DRV [1:0]
	imx662_write_register(ViPipe, 0x30cd, 0x00);  // XVS_DRV [1:0]
	imx662_write_register(ViPipe, 0x30dc, 0x32);  // XVS_DRV [1:0]
	imx662_write_register(ViPipe, 0x30dd, 0x40);  // XVS_DRV [1:0]
	imx662_write_register(ViPipe, 0x3400, 0x00);  // XVS_DRV [1:0]

	imx662_write_register(ViPipe, 0x3444, 0xAC);  // -
	imx662_write_register(ViPipe, 0x3460, 0x21);  // -
	imx662_write_register(ViPipe, 0x3492, 0x08);  // -
	imx662_write_register(ViPipe, 0x3A50, 0x62);  // -
	imx662_write_register(ViPipe, 0x3A51, 0x01);  // -
	imx662_write_register(ViPipe, 0x3A52, 0x19);  // -
	imx662_write_register(ViPipe, 0x3B00, 0x39);  // -
	imx662_write_register(ViPipe, 0x3B23, 0x2D);  // -
	imx662_write_register(ViPipe, 0x3B45, 0x04);  // -
	imx662_write_register(ViPipe, 0x3C0A, 0x1F);  // -
	imx662_write_register(ViPipe, 0x3C0B, 0x1E);  // -
	imx662_write_register(ViPipe, 0x3C38, 0x21);  // -
	imx662_write_register(ViPipe, 0x3C40, 0x06);  // -
	imx662_write_register(ViPipe, 0x3C44, 0x00);  // -
	imx662_write_register(ViPipe, 0x3CB6, 0xD8);  // -
	imx662_write_register(ViPipe, 0x3CC4, 0xDA);  // -
	imx662_write_register(ViPipe, 0x3E24, 0x79);  // -
	imx662_write_register(ViPipe, 0x3E2C, 0x15);  // -
	imx662_write_register(ViPipe, 0x3EDC, 0x2D);  // -
	imx662_write_register(ViPipe, 0x4498, 0x05);  // -
	imx662_write_register(ViPipe, 0x449C, 0x19);  // -
	imx662_write_register(ViPipe, 0x449D, 0x00);  // -
	imx662_write_register(ViPipe, 0x449E, 0x32);  // -
	imx662_write_register(ViPipe, 0x449F, 0x01);  // -
	imx662_write_register(ViPipe, 0x44A0, 0x92);  // -
	imx662_write_register(ViPipe, 0x44A2, 0x91);  // -
	imx662_write_register(ViPipe, 0x44A4, 0x8C);  // -
	imx662_write_register(ViPipe, 0x44A6, 0x87);  // -
	imx662_write_register(ViPipe, 0x44A8, 0x82);  // -
	imx662_write_register(ViPipe, 0x44AA, 0x78);  // -
	imx662_write_register(ViPipe, 0x44AC, 0x6E);  // -
	imx662_write_register(ViPipe, 0x44AE, 0x69);  // -
	imx662_write_register(ViPipe, 0x44B0, 0x92);  // -
	imx662_write_register(ViPipe, 0x44B2, 0x91);  // -
	imx662_write_register(ViPipe, 0x44B4, 0x8C);  // -
	imx662_write_register(ViPipe, 0x44B6, 0x87);  // -
	imx662_write_register(ViPipe, 0x44B8, 0x82);  // -
	imx662_write_register(ViPipe, 0x44BA, 0x78);  // -
	imx662_write_register(ViPipe, 0x44BC, 0x6E);  // -
	imx662_write_register(ViPipe, 0x44BE, 0x69);  // -
	imx662_write_register(ViPipe, 0x44C0, 0x7F);  // -
	imx662_write_register(ViPipe, 0x44C1, 0x01);  // -
	imx662_write_register(ViPipe, 0x44C2, 0x7F);  // -
	imx662_write_register(ViPipe, 0x44C3, 0x01);  // -
	imx662_write_register(ViPipe, 0x44C4, 0x7A);  // -
	imx662_write_register(ViPipe, 0x44C5, 0x01);  // -
	imx662_write_register(ViPipe, 0x44C6, 0x7A);  // -
	imx662_write_register(ViPipe, 0x44C7, 0x01);  // -
	imx662_write_register(ViPipe, 0x44C8, 0x70);  // -
	imx662_write_register(ViPipe, 0x44C9, 0x01);  // -
	imx662_write_register(ViPipe, 0x44CA, 0x6B);  // -
	imx662_write_register(ViPipe, 0x44CB, 0x01);  // -
	imx662_write_register(ViPipe, 0x44CC, 0x6B);  // -
	imx662_write_register(ViPipe, 0x44CD, 0x01);  // -
	imx662_write_register(ViPipe, 0x44CE, 0x5C);  // -
	imx662_write_register(ViPipe, 0x44CF, 0x01);  // -
	imx662_write_register(ViPipe, 0x44D0, 0x7F);  // -
	imx662_write_register(ViPipe, 0x44D1, 0x01);  // -
	imx662_write_register(ViPipe, 0x44D2, 0x7F);  // -
	imx662_write_register(ViPipe, 0x44D3, 0x01);  // -
	imx662_write_register(ViPipe, 0x44D4, 0x7A);  // -
	imx662_write_register(ViPipe, 0x44D5, 0x01);  // -
	imx662_write_register(ViPipe, 0x44D6, 0x7A);  // -
	imx662_write_register(ViPipe, 0x44D7, 0x01);  // -
	imx662_write_register(ViPipe, 0x44D8, 0x70);  // -
	imx662_write_register(ViPipe, 0x44D9, 0x01);  // -
	imx662_write_register(ViPipe, 0x44DA, 0x6B);  // -
	imx662_write_register(ViPipe, 0x44DB, 0x01);  // -
	imx662_write_register(ViPipe, 0x44DC, 0x6B);  // -
	imx662_write_register(ViPipe, 0x44DD, 0x01);  // -
	imx662_write_register(ViPipe, 0x44DE, 0x5C);  // -
	imx662_write_register(ViPipe, 0x44DF, 0x01);  // -
	imx662_write_register(ViPipe, 0x4534, 0x1C);  // -
	imx662_write_register(ViPipe, 0x4535, 0x03);  // -
	imx662_write_register(ViPipe, 0x4538, 0x1C);  // -
	imx662_write_register(ViPipe, 0x4539, 0x1C);  // -
	imx662_write_register(ViPipe, 0x453A, 0x1C);  // -
	imx662_write_register(ViPipe, 0x453B, 0x1C);  // -
	imx662_write_register(ViPipe, 0x453C, 0x1C);  // -
	imx662_write_register(ViPipe, 0x453D, 0x1C);  // -
	imx662_write_register(ViPipe, 0x453E, 0x1C);  // -
	imx662_write_register(ViPipe, 0x453F, 0x1C);  // -
	imx662_write_register(ViPipe, 0x4540, 0x1C);  // -
	imx662_write_register(ViPipe, 0x4541, 0x03);  // -
	imx662_write_register(ViPipe, 0x4542, 0x03);  // -
	imx662_write_register(ViPipe, 0x4543, 0x03);  // -
	imx662_write_register(ViPipe, 0x4544, 0x03);  // -
	imx662_write_register(ViPipe, 0x4545, 0x03);  // -
	imx662_write_register(ViPipe, 0x4546, 0x03);  // -
	imx662_write_register(ViPipe, 0x4547, 0x03);  // -
	imx662_write_register(ViPipe, 0x4548, 0x03);  // -
	imx662_write_register(ViPipe, 0x4549, 0x03);  // -

	imx662_default_reg_init(ViPipe);

	imx662_write_register(ViPipe, 0x3000, 0x00); /* standby */
	delay_ms(20);
	imx662_write_register(ViPipe, 0x3002, 0x00); /* master mode start */
	printf("===Imx662 sensor 1080P30fps 10bit 2to1 WDR(60fps->30fps) init success!=====\n");
}

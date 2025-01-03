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
#include "imx415_cmos_ex.h"

static void imx415_linear_5m25_init(VI_PIPE ViPipe);
static void imx415_linear_8m25_init(VI_PIPE ViPipe);
static void imx415_linear_4m25_init(VI_PIPE ViPipe);
static void imx415_wdr_4m25_2to1_init(VI_PIPE ViPipe);

const CVI_U32 imx415_addr_byte = 2;
const CVI_U32 imx415_data_byte = 1;
static int g_fd[VI_MAX_PIPE_NUM] = {[0 ... (VI_MAX_PIPE_NUM - 1)] = -1};

int imx415_i2c_init(VI_PIPE ViPipe)
{
	char acDevFile[16] = {0};
	CVI_U8 u8DevNum;

	if (g_fd[ViPipe] >= 0)
		return CVI_SUCCESS;
	int ret;

	u8DevNum = g_aunImx415_BusInfo[ViPipe].s8I2cDev;
	snprintf(acDevFile, sizeof(acDevFile),  "/dev/i2c-%u", u8DevNum);

	g_fd[ViPipe] = open(acDevFile, O_RDWR, 0600);

	if (g_fd[ViPipe] < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Open /dev/i2c-%u error!\n", u8DevNum);
		return CVI_FAILURE;
	}

	ret = ioctl(g_fd[ViPipe], I2C_SLAVE_FORCE, g_aunImx415_AddrInfo[ViPipe].s8I2cAddr);
	if (ret < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "I2C_SLAVE_FORCE error!\n");
		close(g_fd[ViPipe]);
		g_fd[ViPipe] = -1;
		return ret;
	}

	return CVI_SUCCESS;
}

int imx415_i2c_exit(VI_PIPE ViPipe)
{
	if (g_fd[ViPipe] >= 0) {
		close(g_fd[ViPipe]);
		g_fd[ViPipe] = -1;
		return CVI_SUCCESS;
	}
	return CVI_FAILURE;
}

int imx415_read_register(VI_PIPE ViPipe, int addr)
{
	int ret, data;
	CVI_U8 buf[8];
	CVI_U8 idx = 0;

	if (g_fd[ViPipe] < 0)
		return CVI_FAILURE;

	if (imx415_addr_byte == 2)
		buf[idx++] = (addr >> 8) & 0xff;

	// add address byte 0
	buf[idx++] = addr & 0xff;

	ret = write(g_fd[ViPipe], buf, imx415_addr_byte);
	if (ret < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "I2C_WRITE error!\n");
		return ret;
	}

	buf[0] = 0;
	buf[1] = 0;
	ret = read(g_fd[ViPipe], buf, imx415_data_byte);
	if (ret < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "I2C_READ error!\n");
		return ret;
	}

	// pack read back data
	data = 0;
	if (imx415_data_byte == 2) {
		data = buf[0] << 8;
		data += buf[1];
	} else {
		data = buf[0];
	}

	syslog(LOG_DEBUG, "i2c r 0x%x = 0x%x\n", addr, data);
	return data;
}


int imx415_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 idx = 0;
	int ret;
	CVI_U8 buf[8];

	if (g_fd[ViPipe] < 0)
		return CVI_SUCCESS;

	if (imx415_addr_byte == 2) {
		buf[idx] = (addr >> 8) & 0xff;
		idx++;
		buf[idx] = addr & 0xff;
		idx++;
	}

	if (imx415_data_byte == 1) {
		buf[idx] = data & 0xff;
		idx++;
	}

	ret = write(g_fd[ViPipe], buf, imx415_addr_byte + imx415_data_byte);
	if (ret < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "I2C_WRITE error!\n");
		return CVI_FAILURE;
	}
	syslog(LOG_DEBUG, "i2c w 0x%x 0x%x\n", addr, data);
	return CVI_SUCCESS;
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void imx415_standby(VI_PIPE ViPipe)
{
	(void)(ViPipe);
}

void imx415_restart(VI_PIPE ViPipe)
{
	(void)(ViPipe);
	delay_ms(20);
}

void imx415_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastImx415[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		imx415_write_register(ViPipe,
				g_pastImx415[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastImx415[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void imx415_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
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

	(void)(u8Filp);
	(void)(u8Mirror);
	(void)(ViPipe);

	if (u8Filp != 0) {

	} else {

	}
}

int imx415_probe(VI_PIPE ViPipe)
{
	// int nVal;

	usleep(100);
	if (imx415_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	// nVal = imx415_read_register(ViPipe, 0x3000);
	// if (nVal < 0) {
	// 	CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
	// 	return nVal;
	// }

	return CVI_SUCCESS;
}

void imx415_init(VI_PIPE ViPipe)
{
	WDR_MODE_E        enWDRMode;
	CVI_U8            u8ImgMode;

	enWDRMode   = g_pastImx415[ViPipe]->enWDRMode;
	u8ImgMode   = g_pastImx415[ViPipe]->u8ImgMode;

	imx415_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		if (u8ImgMode == IMX415_MODE_4M25_WDR)
			imx415_wdr_4m25_2to1_init(ViPipe);
		else{
		}
	} else {
		if (u8ImgMode == IMX415_MODE_4M25)
			imx415_linear_4m25_init(ViPipe);
		else if (u8ImgMode == IMX415_MODE_8M25)
			imx415_linear_8m25_init(ViPipe);
		else if (u8ImgMode == IMX415_MODE_5M25)
			imx415_linear_5m25_init(ViPipe);
		else {
		}
	}
	g_pastImx415[ViPipe]->bInit = CVI_TRUE;
}

// void imx415_exit(VI_PIPE ViPipe)
// {
// 	imx415_i2c_exit(ViPipe);
// }

static void imx415_linear_4m25_init(VI_PIPE ViPipe)
{
	// 37.125M 2568x1440@25
	delay_ms(4);
	imx415_write_register(ViPipe, 0x3008, 0x7F);
	imx415_write_register(ViPipe, 0x300A, 0x5B);
	imx415_write_register(ViPipe, 0x3024, 0x8C);
	imx415_write_register(ViPipe, 0x3025, 0x0A);
	imx415_write_register(ViPipe, 0x3028, 0x4C);
	imx415_write_register(ViPipe, 0x3029, 0x04);
	imx415_write_register(ViPipe, 0x3030, 0x01);
	imx415_write_register(ViPipe, 0x3033, 0x06);
	imx415_write_register(ViPipe, 0x3050, 0x08);
	imx415_write_register(ViPipe, 0x30C1, 0x00);
	imx415_write_register(ViPipe, 0x3116, 0x24);
	imx415_write_register(ViPipe, 0x3118, 0x80);
	imx415_write_register(ViPipe, 0x311E, 0x24);
	imx415_write_register(ViPipe, 0x32D4, 0x21);
	imx415_write_register(ViPipe, 0x32EC, 0xA1);
	imx415_write_register(ViPipe, 0x344C, 0x2B);
	imx415_write_register(ViPipe, 0x344D, 0x01);
	imx415_write_register(ViPipe, 0x344E, 0xED);
	imx415_write_register(ViPipe, 0x344F, 0x01);
	imx415_write_register(ViPipe, 0x3450, 0xF6);
	imx415_write_register(ViPipe, 0x3451, 0x02);
	imx415_write_register(ViPipe, 0x3452, 0x7F);
	imx415_write_register(ViPipe, 0x3453, 0x03);
	imx415_write_register(ViPipe, 0x358A, 0x04);
	imx415_write_register(ViPipe, 0x35A1, 0x02);
	imx415_write_register(ViPipe, 0x35EC, 0x27);
	imx415_write_register(ViPipe, 0x35EE, 0x8D);
	imx415_write_register(ViPipe, 0x35F0, 0x8D);
	imx415_write_register(ViPipe, 0x35F2, 0x29);
	imx415_write_register(ViPipe, 0x36BC, 0x0C);
	imx415_write_register(ViPipe, 0x36CC, 0x53);
	imx415_write_register(ViPipe, 0x36CD, 0x00);
	imx415_write_register(ViPipe, 0x36CE, 0x3C);
	imx415_write_register(ViPipe, 0x36D0, 0x8C);
	imx415_write_register(ViPipe, 0x36D1, 0x00);
	imx415_write_register(ViPipe, 0x36D2, 0x71);
	imx415_write_register(ViPipe, 0x36D4, 0x3C);
	imx415_write_register(ViPipe, 0x36D6, 0x53);
	imx415_write_register(ViPipe, 0x36D7, 0x00);
	imx415_write_register(ViPipe, 0x36D8, 0x71);
	imx415_write_register(ViPipe, 0x36DA, 0x8C);
	imx415_write_register(ViPipe, 0x36DB, 0x00);
	imx415_write_register(ViPipe, 0x3720, 0x00);
	imx415_write_register(ViPipe, 0x3724, 0x02);
	imx415_write_register(ViPipe, 0x3726, 0x02);
	imx415_write_register(ViPipe, 0x3732, 0x02);
	imx415_write_register(ViPipe, 0x3734, 0x03);
	imx415_write_register(ViPipe, 0x3736, 0x03);
	imx415_write_register(ViPipe, 0x3742, 0x03);
	imx415_write_register(ViPipe, 0x3862, 0xE0);
	imx415_write_register(ViPipe, 0x38CC, 0x30);
	imx415_write_register(ViPipe, 0x38CD, 0x2F);
	imx415_write_register(ViPipe, 0x395C, 0x0C);
	imx415_write_register(ViPipe, 0x39A4, 0x07);
	imx415_write_register(ViPipe, 0x39A8, 0x32);
	imx415_write_register(ViPipe, 0x39AA, 0x32);
	imx415_write_register(ViPipe, 0x39AC, 0x32);
	imx415_write_register(ViPipe, 0x39AE, 0x32);
	imx415_write_register(ViPipe, 0x39B0, 0x32);
	imx415_write_register(ViPipe, 0x39B2, 0x2F);
	imx415_write_register(ViPipe, 0x39B4, 0x2D);
	imx415_write_register(ViPipe, 0x39B6, 0x28);
	imx415_write_register(ViPipe, 0x39B8, 0x30);
	imx415_write_register(ViPipe, 0x39BA, 0x30);
	imx415_write_register(ViPipe, 0x39BC, 0x30);
	imx415_write_register(ViPipe, 0x39BE, 0x30);
	imx415_write_register(ViPipe, 0x39C0, 0x30);
	imx415_write_register(ViPipe, 0x39C2, 0x2E);
	imx415_write_register(ViPipe, 0x39C4, 0x2B);
	imx415_write_register(ViPipe, 0x39C6, 0x25);
	imx415_write_register(ViPipe, 0x3A42, 0xD1);
	imx415_write_register(ViPipe, 0x3A4C, 0x77);
	imx415_write_register(ViPipe, 0x3AE0, 0x02);
	imx415_write_register(ViPipe, 0x3AEC, 0x0C);
	imx415_write_register(ViPipe, 0x3B00, 0x2E);
	imx415_write_register(ViPipe, 0x3B06, 0x29);
	imx415_write_register(ViPipe, 0x3B98, 0x25);
	imx415_write_register(ViPipe, 0x3B99, 0x21);
	imx415_write_register(ViPipe, 0x3B9B, 0x13);
	imx415_write_register(ViPipe, 0x3B9C, 0x13);
	imx415_write_register(ViPipe, 0x3B9D, 0x13);
	imx415_write_register(ViPipe, 0x3B9E, 0x13);
	imx415_write_register(ViPipe, 0x3BA1, 0x00);
	imx415_write_register(ViPipe, 0x3BA2, 0x06);
	imx415_write_register(ViPipe, 0x3BA3, 0x0B);
	imx415_write_register(ViPipe, 0x3BA4, 0x10);
	imx415_write_register(ViPipe, 0x3BA5, 0x14);
	imx415_write_register(ViPipe, 0x3BA6, 0x18);
	imx415_write_register(ViPipe, 0x3BA7, 0x1A);
	imx415_write_register(ViPipe, 0x3BA8, 0x1A);
	imx415_write_register(ViPipe, 0x3BA9, 0x1A);
	imx415_write_register(ViPipe, 0x3BAC, 0xED);
	imx415_write_register(ViPipe, 0x3BAD, 0x01);
	imx415_write_register(ViPipe, 0x3BAE, 0xF6);
	imx415_write_register(ViPipe, 0x3BAF, 0x02);
	imx415_write_register(ViPipe, 0x3BB0, 0xA2);
	imx415_write_register(ViPipe, 0x3BB1, 0x03);
	imx415_write_register(ViPipe, 0x3BB2, 0xE0);
	imx415_write_register(ViPipe, 0x3BB3, 0x03);
	imx415_write_register(ViPipe, 0x3BB4, 0xE0);
	imx415_write_register(ViPipe, 0x3BB5, 0x03);
	imx415_write_register(ViPipe, 0x3BB6, 0xE0);
	imx415_write_register(ViPipe, 0x3BB7, 0x03);
	imx415_write_register(ViPipe, 0x3BB8, 0xE0);
	imx415_write_register(ViPipe, 0x3BBA, 0xE0);
	imx415_write_register(ViPipe, 0x3BBC, 0xDA);
	imx415_write_register(ViPipe, 0x3BBE, 0x88);
	imx415_write_register(ViPipe, 0x3BC0, 0x44);
	imx415_write_register(ViPipe, 0x3BC2, 0x7B);
	imx415_write_register(ViPipe, 0x3BC4, 0xA2);
	imx415_write_register(ViPipe, 0x3BC8, 0xBD);
	imx415_write_register(ViPipe, 0x3BCA, 0xBD);
	imx415_write_register(ViPipe, 0x4004, 0x48);
	imx415_write_register(ViPipe, 0x4005, 0x09);
	imx415_write_register(ViPipe, 0x400C, 0x00);
	imx415_write_register(ViPipe, 0x4018, 0x8F);
	imx415_write_register(ViPipe, 0x401A, 0x4F);
	imx415_write_register(ViPipe, 0x401C, 0x47);
	imx415_write_register(ViPipe, 0x401E, 0x37);
	imx415_write_register(ViPipe, 0x4020, 0x4F);
	imx415_write_register(ViPipe, 0x4022, 0x87);
	imx415_write_register(ViPipe, 0x4024, 0x4F);
	imx415_write_register(ViPipe, 0x4026, 0x7F);
	imx415_write_register(ViPipe, 0x4028, 0x3F);
	imx415_write_register(ViPipe, 0x3000, 0x00);
	imx415_write_register(ViPipe, 0x3002, 0x00);
	imx415_default_reg_init(ViPipe);

	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe:%d,===IMX415 4M25 Init success!===\n", ViPipe);
}

static void imx415_wdr_4m25_2to1_init(VI_PIPE ViPipe)
{
	delay_ms(4);

	imx415_default_reg_init(ViPipe);

	CVI_TRACE_SNS(CVI_DBG_INFO, "===Imx415 sensor 4M25 2to1 WDR init success!===\n");
}

static void imx415_linear_8m25_init(VI_PIPE ViPipe)
{
	delay_ms(4);
CVI_TRACE_SNS(CVI_DBG_ERR, "ViPipe:%d,===IMX415 3864x2192@25 Init success!===\n", ViPipe);
	// // 37.125M 3864x2192@25
	// imx415_write_register(ViPipe, 0x3000, 0x01);
	// imx415_write_register(ViPipe, 0x3002, 0x01);
	// imx415_write_register(ViPipe, 0x3008, 0x7F);
	// imx415_write_register(ViPipe, 0x300A, 0x5B);
	// imx415_write_register(ViPipe, 0x3024, 0x8C);
	// imx415_write_register(ViPipe, 0x3025, 0x0A);
	// imx415_write_register(ViPipe, 0x3028, 0x4C);
	// imx415_write_register(ViPipe, 0x3029, 0x04);
	// imx415_write_register(ViPipe, 0x3030, 0x01);
	// imx415_write_register(ViPipe, 0x3033, 0x06);
	// imx415_write_register(ViPipe, 0x3050, 0x08);
	// imx415_write_register(ViPipe, 0x30C1, 0x00);
	// imx415_write_register(ViPipe, 0x3116, 0x24);
	// imx415_write_register(ViPipe, 0x3118, 0x80);
	// imx415_write_register(ViPipe, 0x311E, 0x24);
	// imx415_write_register(ViPipe, 0x32D4, 0x21);
	// imx415_write_register(ViPipe, 0x32EC, 0xA1);
	// imx415_write_register(ViPipe, 0x344C, 0x2B);
	// imx415_write_register(ViPipe, 0x344D, 0x01);
	// imx415_write_register(ViPipe, 0x344E, 0xED);
	// imx415_write_register(ViPipe, 0x344F, 0x01);
	// imx415_write_register(ViPipe, 0x3450, 0xF6);
	// imx415_write_register(ViPipe, 0x3451, 0x02);
	// imx415_write_register(ViPipe, 0x3452, 0x7F);
	// imx415_write_register(ViPipe, 0x3453, 0x03);
	// imx415_write_register(ViPipe, 0x358A, 0x04);
	// imx415_write_register(ViPipe, 0x35A1, 0x02);
	// imx415_write_register(ViPipe, 0x35EC, 0x27);
	// imx415_write_register(ViPipe, 0x35EE, 0x8D);
	// imx415_write_register(ViPipe, 0x35F0, 0x8D);
	// imx415_write_register(ViPipe, 0x35F2, 0x29);
	// imx415_write_register(ViPipe, 0x36BC, 0x0C);
	// imx415_write_register(ViPipe, 0x36CC, 0x53);
	// imx415_write_register(ViPipe, 0x36CD, 0x00);
	// imx415_write_register(ViPipe, 0x36CE, 0x3C);
	// imx415_write_register(ViPipe, 0x36D0, 0x8C);
	// imx415_write_register(ViPipe, 0x36D1, 0x00);
	// imx415_write_register(ViPipe, 0x36D2, 0x71);
	// imx415_write_register(ViPipe, 0x36D4, 0x3C);
	// imx415_write_register(ViPipe, 0x36D6, 0x53);
	// imx415_write_register(ViPipe, 0x36D7, 0x00);
	// imx415_write_register(ViPipe, 0x36D8, 0x71);
	// imx415_write_register(ViPipe, 0x36DA, 0x8C);
	// imx415_write_register(ViPipe, 0x36DB, 0x00);
	// imx415_write_register(ViPipe, 0x3720, 0x00);
	// imx415_write_register(ViPipe, 0x3724, 0x02);
	// imx415_write_register(ViPipe, 0x3726, 0x02);
	// imx415_write_register(ViPipe, 0x3732, 0x02);
	// imx415_write_register(ViPipe, 0x3734, 0x03);
	// imx415_write_register(ViPipe, 0x3736, 0x03);
	// imx415_write_register(ViPipe, 0x3742, 0x03);
	// imx415_write_register(ViPipe, 0x3862, 0xE0);
	// imx415_write_register(ViPipe, 0x38CC, 0x30);
	// imx415_write_register(ViPipe, 0x38CD, 0x2F);
	// imx415_write_register(ViPipe, 0x395C, 0x0C);
	// imx415_write_register(ViPipe, 0x39A4, 0x07);
	// imx415_write_register(ViPipe, 0x39A8, 0x32);
	// imx415_write_register(ViPipe, 0x39AA, 0x32);
	// imx415_write_register(ViPipe, 0x39AC, 0x32);
	// imx415_write_register(ViPipe, 0x39AE, 0x32);
	// imx415_write_register(ViPipe, 0x39B0, 0x32);
	// imx415_write_register(ViPipe, 0x39B2, 0x2F);
	// imx415_write_register(ViPipe, 0x39B4, 0x2D);
	// imx415_write_register(ViPipe, 0x39B6, 0x28);
	// imx415_write_register(ViPipe, 0x39B8, 0x30);
	// imx415_write_register(ViPipe, 0x39BA, 0x30);
	// imx415_write_register(ViPipe, 0x39BC, 0x30);
	// imx415_write_register(ViPipe, 0x39BE, 0x30);
	// imx415_write_register(ViPipe, 0x39C0, 0x30);
	// imx415_write_register(ViPipe, 0x39C2, 0x2E);
	// imx415_write_register(ViPipe, 0x39C4, 0x2B);
	// imx415_write_register(ViPipe, 0x39C6, 0x25);
	// imx415_write_register(ViPipe, 0x3A42, 0xD1);
	// imx415_write_register(ViPipe, 0x3A4C, 0x77);
	// imx415_write_register(ViPipe, 0x3AE0, 0x02);
	// imx415_write_register(ViPipe, 0x3AEC, 0x0C);
	// imx415_write_register(ViPipe, 0x3B00, 0x2E);
	// imx415_write_register(ViPipe, 0x3B06, 0x29);
	// imx415_write_register(ViPipe, 0x3B98, 0x25);
	// imx415_write_register(ViPipe, 0x3B99, 0x21);
	// imx415_write_register(ViPipe, 0x3B9B, 0x13);
	// imx415_write_register(ViPipe, 0x3B9C, 0x13);
	// imx415_write_register(ViPipe, 0x3B9D, 0x13);
	// imx415_write_register(ViPipe, 0x3B9E, 0x13);
	// imx415_write_register(ViPipe, 0x3BA1, 0x00);
	// imx415_write_register(ViPipe, 0x3BA2, 0x06);
	// imx415_write_register(ViPipe, 0x3BA3, 0x0B);
	// imx415_write_register(ViPipe, 0x3BA4, 0x10);
	// imx415_write_register(ViPipe, 0x3BA5, 0x14);
	// imx415_write_register(ViPipe, 0x3BA6, 0x18);
	// imx415_write_register(ViPipe, 0x3BA7, 0x1A);
	// imx415_write_register(ViPipe, 0x3BA8, 0x1A);
	// imx415_write_register(ViPipe, 0x3BA9, 0x1A);
	// imx415_write_register(ViPipe, 0x3BAC, 0xED);
	// imx415_write_register(ViPipe, 0x3BAD, 0x01);
	// imx415_write_register(ViPipe, 0x3BAE, 0xF6);
	// imx415_write_register(ViPipe, 0x3BAF, 0x02);
	// imx415_write_register(ViPipe, 0x3BB0, 0xA2);
	// imx415_write_register(ViPipe, 0x3BB1, 0x03);
	// imx415_write_register(ViPipe, 0x3BB2, 0xE0);
	// imx415_write_register(ViPipe, 0x3BB3, 0x03);
	// imx415_write_register(ViPipe, 0x3BB4, 0xE0);
	// imx415_write_register(ViPipe, 0x3BB5, 0x03);
	// imx415_write_register(ViPipe, 0x3BB6, 0xE0);
	// imx415_write_register(ViPipe, 0x3BB7, 0x03);
	// imx415_write_register(ViPipe, 0x3BB8, 0xE0);
	// imx415_write_register(ViPipe, 0x3BBA, 0xE0);
	// imx415_write_register(ViPipe, 0x3BBC, 0xDA);
	// imx415_write_register(ViPipe, 0x3BBE, 0x88);
	// imx415_write_register(ViPipe, 0x3BC0, 0x44);
	// imx415_write_register(ViPipe, 0x3BC2, 0x7B);
	// imx415_write_register(ViPipe, 0x3BC4, 0xA2);
	// imx415_write_register(ViPipe, 0x3BC8, 0xBD);
	// imx415_write_register(ViPipe, 0x3BCA, 0xBD);
	// imx415_write_register(ViPipe, 0x4004, 0x48);
	// imx415_write_register(ViPipe, 0x4005, 0x09);
	// imx415_write_register(ViPipe, 0x400C, 0x00);
	// imx415_write_register(ViPipe, 0x4018, 0x8F);
	// imx415_write_register(ViPipe, 0x401A, 0x4F);
	// imx415_write_register(ViPipe, 0x401C, 0x47);
	// imx415_write_register(ViPipe, 0x401E, 0x37);
	// imx415_write_register(ViPipe, 0x4020, 0x4F);
	// imx415_write_register(ViPipe, 0x4022, 0x87);
	// imx415_write_register(ViPipe, 0x4024, 0x4F);
	// imx415_write_register(ViPipe, 0x4026, 0x7F);
	// imx415_write_register(ViPipe, 0x4028, 0x3F);
	// imx415_write_register(ViPipe, 0x3000, 0x00);
	// imx415_write_register(ViPipe, 0x3002, 0x00);


	// 24M
	imx415_write_register(ViPipe, 0x3000, 0x01);
	imx415_write_register(ViPipe, 0x3001, 0x00);
	imx415_write_register(ViPipe, 0x3002, 0x01);
	imx415_write_register(ViPipe, 0x3003, 0x00);
	imx415_write_register(ViPipe, 0x3008, 0x54);
	imx415_write_register(ViPipe, 0x3009, 0x00);
	imx415_write_register(ViPipe, 0x300A, 0x3B);
	imx415_write_register(ViPipe, 0x300B, 0xA0);
	imx415_write_register(ViPipe, 0x300C, 0x03);
	imx415_write_register(ViPipe, 0x301C, 0x00);
	imx415_write_register(ViPipe, 0x301D, 0x08);
	imx415_write_register(ViPipe, 0x3020, 0x00);
	imx415_write_register(ViPipe, 0x3021, 0x00);
	imx415_write_register(ViPipe, 0x3022, 0x00);
	imx415_write_register(ViPipe, 0x3023, 0x01);
	imx415_write_register(ViPipe, 0x3024, 0x8E); // 25fps
	imx415_write_register(ViPipe, 0x3025, 0x0A);
	imx415_write_register(ViPipe, 0x3026, 0x00);
	imx415_write_register(ViPipe, 0x3028, 0x2A);
	imx415_write_register(ViPipe, 0x3029, 0x04);
	imx415_write_register(ViPipe, 0x302C, 0x00);
	imx415_write_register(ViPipe, 0x302D, 0x00);
	imx415_write_register(ViPipe, 0x3030, 0x00);
	imx415_write_register(ViPipe, 0x3031, 0x01);
	imx415_write_register(ViPipe, 0x3032, 0x01);
	imx415_write_register(ViPipe, 0x3033, 0x08);
	imx415_write_register(ViPipe, 0x3040, 0x00);
	imx415_write_register(ViPipe, 0x3041, 0x00);
	imx415_write_register(ViPipe, 0x3042, 0x18);
	imx415_write_register(ViPipe, 0x3043, 0x0F);
	imx415_write_register(ViPipe, 0x3044, 0x00);
	imx415_write_register(ViPipe, 0x3045, 0x00);
	imx415_write_register(ViPipe, 0x3046, 0x20);
	imx415_write_register(ViPipe, 0x3047, 0x11);
	imx415_write_register(ViPipe, 0x3050, 0x08);
	imx415_write_register(ViPipe, 0x3051, 0x00);
	imx415_write_register(ViPipe, 0x3052, 0x00);
	imx415_write_register(ViPipe, 0x3054, 0x19);
	imx415_write_register(ViPipe, 0x3055, 0x00);
	imx415_write_register(ViPipe, 0x3056, 0x00);
	imx415_write_register(ViPipe, 0x3058, 0x3E);
	imx415_write_register(ViPipe, 0x3059, 0x00);
	imx415_write_register(ViPipe, 0x305A, 0x00);
	imx415_write_register(ViPipe, 0x305C, 0x66);
	imx415_write_register(ViPipe, 0x305D, 0x00);
	imx415_write_register(ViPipe, 0x305E, 0x00);
	imx415_write_register(ViPipe, 0x3060, 0x25);
	imx415_write_register(ViPipe, 0x3061, 0x00);
	imx415_write_register(ViPipe, 0x3062, 0x00);
	imx415_write_register(ViPipe, 0x3064, 0x4A);
	imx415_write_register(ViPipe, 0x3065, 0x00);
	imx415_write_register(ViPipe, 0x3066, 0x00);
	imx415_write_register(ViPipe, 0x3090, 0x00);
	imx415_write_register(ViPipe, 0x3091, 0x00);
	imx415_write_register(ViPipe, 0x3092, 0x00);
	imx415_write_register(ViPipe, 0x3093, 0x00);
	imx415_write_register(ViPipe, 0x3094, 0x00);
	imx415_write_register(ViPipe, 0x3095, 0x00);
	imx415_write_register(ViPipe, 0x3096, 0x00);
	imx415_write_register(ViPipe, 0x3097, 0x00);
	imx415_write_register(ViPipe, 0x30C0, 0x2A);
	imx415_write_register(ViPipe, 0x30C1, 0x00);
	imx415_write_register(ViPipe, 0x30CC, 0x00);
	imx415_write_register(ViPipe, 0x30CD, 0x00);
	imx415_write_register(ViPipe, 0x30CF, 0x00);
	imx415_write_register(ViPipe, 0x30D9, 0x06);
	imx415_write_register(ViPipe, 0x30DA, 0x02);
	imx415_write_register(ViPipe, 0x30E2, 0x32);
	imx415_write_register(ViPipe, 0x30E3, 0x00);
	imx415_write_register(ViPipe, 0x3115, 0x00);
	imx415_write_register(ViPipe, 0x3116, 0x23);
	imx415_write_register(ViPipe, 0x3118, 0xB4);
	imx415_write_register(ViPipe, 0x3119, 0x00);
	imx415_write_register(ViPipe, 0x311A, 0xFC);
	imx415_write_register(ViPipe, 0x311B, 0x00);
	imx415_write_register(ViPipe, 0x311E, 0x23);
	imx415_write_register(ViPipe, 0x3260, 0x01);
	imx415_write_register(ViPipe, 0x32C8, 0x01);
	imx415_write_register(ViPipe, 0x32D4, 0x21);
	imx415_write_register(ViPipe, 0x32EC, 0xA1);
	imx415_write_register(ViPipe, 0x344C, 0x2B);
	imx415_write_register(ViPipe, 0x344D, 0x01);
	imx415_write_register(ViPipe, 0x344E, 0xED);
	imx415_write_register(ViPipe, 0x344F, 0x01);
	imx415_write_register(ViPipe, 0x3450, 0xF6);
	imx415_write_register(ViPipe, 0x3451, 0x02);
	imx415_write_register(ViPipe, 0x3452, 0x7F);
	imx415_write_register(ViPipe, 0x3453, 0x03);
	imx415_write_register(ViPipe, 0x358A, 0x04);
	imx415_write_register(ViPipe, 0x35A1, 0x02);
	imx415_write_register(ViPipe, 0x35EC, 0x27);
	imx415_write_register(ViPipe, 0x35EE, 0x8D);
	imx415_write_register(ViPipe, 0x35F0, 0x8D);
	imx415_write_register(ViPipe, 0x35F2, 0x29);
	imx415_write_register(ViPipe, 0x36BC, 0x0C);
	imx415_write_register(ViPipe, 0x36CC, 0x53);
	imx415_write_register(ViPipe, 0x36CD, 0x00);
	imx415_write_register(ViPipe, 0x36CE, 0x3C);
	imx415_write_register(ViPipe, 0x36D0, 0x8C);
	imx415_write_register(ViPipe, 0x36D1, 0x00);
	imx415_write_register(ViPipe, 0x36D2, 0x71);
	imx415_write_register(ViPipe, 0x36D4, 0x3C);
	imx415_write_register(ViPipe, 0x36D6, 0x53);
	imx415_write_register(ViPipe, 0x36D7, 0x00);
	imx415_write_register(ViPipe, 0x36D8, 0x71);
	imx415_write_register(ViPipe, 0x36DA, 0x8C);
	imx415_write_register(ViPipe, 0x36DB, 0x00);
	imx415_write_register(ViPipe, 0x3701, 0x03);
	imx415_write_register(ViPipe, 0x3720, 0x00);
	imx415_write_register(ViPipe, 0x3724, 0x02);
	imx415_write_register(ViPipe, 0x3726, 0x02);
	imx415_write_register(ViPipe, 0x3732, 0x02);
	imx415_write_register(ViPipe, 0x3734, 0x03);
	imx415_write_register(ViPipe, 0x3736, 0x03);
	imx415_write_register(ViPipe, 0x3742, 0x03);
	imx415_write_register(ViPipe, 0x3862, 0xE0);
	imx415_write_register(ViPipe, 0x38CC, 0x30);
	imx415_write_register(ViPipe, 0x38CD, 0x2F);
	imx415_write_register(ViPipe, 0x395C, 0x0C);
	imx415_write_register(ViPipe, 0x39A4, 0x07);
	imx415_write_register(ViPipe, 0x39A8, 0x32);
	imx415_write_register(ViPipe, 0x39AA, 0x32);
	imx415_write_register(ViPipe, 0x39AC, 0x32);
	imx415_write_register(ViPipe, 0x39AE, 0x32);
	imx415_write_register(ViPipe, 0x39B0, 0x32);
	imx415_write_register(ViPipe, 0x39B2, 0x2F);
	imx415_write_register(ViPipe, 0x39B4, 0x2D);
	imx415_write_register(ViPipe, 0x39B6, 0x28);
	imx415_write_register(ViPipe, 0x39B8, 0x30);
	imx415_write_register(ViPipe, 0x39BA, 0x30);
	imx415_write_register(ViPipe, 0x39BC, 0x30);
	imx415_write_register(ViPipe, 0x39BE, 0x30);
	imx415_write_register(ViPipe, 0x39C0, 0x30);
	imx415_write_register(ViPipe, 0x39C2, 0x2E);
	imx415_write_register(ViPipe, 0x39C4, 0x2B);
	imx415_write_register(ViPipe, 0x39C6, 0x25);
	imx415_write_register(ViPipe, 0x3A42, 0xD1);
	imx415_write_register(ViPipe, 0x3A4C, 0x77);
	imx415_write_register(ViPipe, 0x3AE0, 0x02);
	imx415_write_register(ViPipe, 0x3AEC, 0x0C);
	imx415_write_register(ViPipe, 0x3B00, 0x2E);
	imx415_write_register(ViPipe, 0x3B06, 0x29);
	imx415_write_register(ViPipe, 0x3B98, 0x25);
	imx415_write_register(ViPipe, 0x3B99, 0x21);
	imx415_write_register(ViPipe, 0x3B9B, 0x13);
	imx415_write_register(ViPipe, 0x3B9C, 0x13);
	imx415_write_register(ViPipe, 0x3B9D, 0x13);
	imx415_write_register(ViPipe, 0x3B9E, 0x13);
	imx415_write_register(ViPipe, 0x3BA1, 0x00);
	imx415_write_register(ViPipe, 0x3BA2, 0x06);
	imx415_write_register(ViPipe, 0x3BA3, 0x0B);
	imx415_write_register(ViPipe, 0x3BA4, 0x10);
	imx415_write_register(ViPipe, 0x3BA5, 0x14);
	imx415_write_register(ViPipe, 0x3BA6, 0x18);
	imx415_write_register(ViPipe, 0x3BA7, 0x1A);
	imx415_write_register(ViPipe, 0x3BA8, 0x1A);
	imx415_write_register(ViPipe, 0x3BA9, 0x1A);
	imx415_write_register(ViPipe, 0x3BAC, 0xED);
	imx415_write_register(ViPipe, 0x3BAD, 0x01);
	imx415_write_register(ViPipe, 0x3BAE, 0xF6);
	imx415_write_register(ViPipe, 0x3BAF, 0x02);
	imx415_write_register(ViPipe, 0x3BB0, 0xA2);
	imx415_write_register(ViPipe, 0x3BB1, 0x03);
	imx415_write_register(ViPipe, 0x3BB2, 0xE0);
	imx415_write_register(ViPipe, 0x3BB3, 0x03);
	imx415_write_register(ViPipe, 0x3BB4, 0xE0);
	imx415_write_register(ViPipe, 0x3BB5, 0x03);
	imx415_write_register(ViPipe, 0x3BB6, 0xE0);
	imx415_write_register(ViPipe, 0x3BB7, 0x03);
	imx415_write_register(ViPipe, 0x3BB8, 0xE0);
	imx415_write_register(ViPipe, 0x3BBA, 0xE0);
	imx415_write_register(ViPipe, 0x3BBC, 0xDA);
	imx415_write_register(ViPipe, 0x3BBE, 0x88);
	imx415_write_register(ViPipe, 0x3BC0, 0x44);
	imx415_write_register(ViPipe, 0x3BC2, 0x7B);
	imx415_write_register(ViPipe, 0x3BC4, 0xA2);
	imx415_write_register(ViPipe, 0x3BC8, 0xBD);
	imx415_write_register(ViPipe, 0x3BCA, 0xBD);
	imx415_write_register(ViPipe, 0x4000, 0x10);
	imx415_write_register(ViPipe, 0x4001, 0x03);
	imx415_write_register(ViPipe, 0x4004, 0x00);
	imx415_write_register(ViPipe, 0x4005, 0x06);
	imx415_write_register(ViPipe, 0x400C, 0x01);
	imx415_write_register(ViPipe, 0x4018, 0x9F);
	imx415_write_register(ViPipe, 0x4019, 0x00);
	imx415_write_register(ViPipe, 0x401A, 0x57);
	imx415_write_register(ViPipe, 0x401B, 0x00);
	imx415_write_register(ViPipe, 0x401C, 0x57);
	imx415_write_register(ViPipe, 0x401D, 0x00);
	imx415_write_register(ViPipe, 0x401E, 0x87);
	imx415_write_register(ViPipe, 0x401F, 0x01);
	imx415_write_register(ViPipe, 0x4020, 0x5F);
	imx415_write_register(ViPipe, 0x4021, 0x00);
	imx415_write_register(ViPipe, 0x4022, 0xA7);
	imx415_write_register(ViPipe, 0x4023, 0x00);
	imx415_write_register(ViPipe, 0x4024, 0x5F);
	imx415_write_register(ViPipe, 0x4025, 0x00);
	imx415_write_register(ViPipe, 0x4026, 0x97);
	imx415_write_register(ViPipe, 0x4027, 0x00);
	imx415_write_register(ViPipe, 0x4028, 0x4F);
	imx415_write_register(ViPipe, 0x4029, 0x00);
	imx415_write_register(ViPipe, 0x4074, 0x00);
	imx415_write_register(ViPipe, 0x3000, 0x00);
	imx415_write_register(ViPipe, 0x3002, 0x00);
	imx415_default_reg_init(ViPipe);

	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe:%d,===IMX415 3864x2192@25 Init success!===\n", ViPipe);
}


static void imx415_linear_5m25_init(VI_PIPE ViPipe)
{
	delay_ms(4);

	// 24M 2568x2160@25
	imx415_write_register(ViPipe, 0x3000, 0x01);
	imx415_write_register(ViPipe, 0x3001, 0x00);
	imx415_write_register(ViPipe, 0x3002, 0x01);
	imx415_write_register(ViPipe, 0x3003, 0x00);
	imx415_write_register(ViPipe, 0x3008, 0x54);
	imx415_write_register(ViPipe, 0x3009, 0x00);
	imx415_write_register(ViPipe, 0x300A, 0x3B);
	imx415_write_register(ViPipe, 0x300B, 0xA0);
	imx415_write_register(ViPipe, 0x301C, 0x04);
	imx415_write_register(ViPipe, 0x301D, 0x08);
	imx415_write_register(ViPipe, 0x3020, 0x00);
	imx415_write_register(ViPipe, 0x3021, 0x00);
	imx415_write_register(ViPipe, 0x3022, 0x00);
	imx415_write_register(ViPipe, 0x3023, 0x01);
	imx415_write_register(ViPipe, 0x3024, 0x8C);
	imx415_write_register(ViPipe, 0x3025, 0x0A);
	imx415_write_register(ViPipe, 0x3026, 0x00);
	imx415_write_register(ViPipe, 0x3028, 0x4C);
	imx415_write_register(ViPipe, 0x3029, 0x04);
	imx415_write_register(ViPipe, 0x302C, 0x00);
	imx415_write_register(ViPipe, 0x302D, 0x00);
	imx415_write_register(ViPipe, 0x3030, 0x01);
	imx415_write_register(ViPipe, 0x3031, 0x01);
	imx415_write_register(ViPipe, 0x3032, 0x01);
	imx415_write_register(ViPipe, 0x3033, 0x06);
	imx415_write_register(ViPipe, 0x3040, 0x88);
	imx415_write_register(ViPipe, 0x3041, 0x02);
	imx415_write_register(ViPipe, 0x3042, 0x08);
	imx415_write_register(ViPipe, 0x3043, 0x0A);
	imx415_write_register(ViPipe, 0x3044, 0x20);
	imx415_write_register(ViPipe, 0x3045, 0x00);
	imx415_write_register(ViPipe, 0x3046, 0xE0);
	imx415_write_register(ViPipe, 0x3047, 0x10);
	imx415_write_register(ViPipe, 0x3050, 0x08);
	imx415_write_register(ViPipe, 0x3051, 0x00);
	imx415_write_register(ViPipe, 0x3052, 0x00);
	imx415_write_register(ViPipe, 0x3054, 0x19);
	imx415_write_register(ViPipe, 0x3055, 0x00);
	imx415_write_register(ViPipe, 0x3056, 0x00);
	imx415_write_register(ViPipe, 0x3058, 0x3E);
	imx415_write_register(ViPipe, 0x3059, 0x00);
	imx415_write_register(ViPipe, 0x305A, 0x00);
	imx415_write_register(ViPipe, 0x305C, 0x66);
	imx415_write_register(ViPipe, 0x305D, 0x00);
	imx415_write_register(ViPipe, 0x305E, 0x00);
	imx415_write_register(ViPipe, 0x3060, 0x25);
	imx415_write_register(ViPipe, 0x3061, 0x00);
	imx415_write_register(ViPipe, 0x3062, 0x00);
	imx415_write_register(ViPipe, 0x3064, 0x4A);
	imx415_write_register(ViPipe, 0x3065, 0x00);
	imx415_write_register(ViPipe, 0x3066, 0x00);
	imx415_write_register(ViPipe, 0x3090, 0x00);
	imx415_write_register(ViPipe, 0x3091, 0x00);
	imx415_write_register(ViPipe, 0x3092, 0x00);
	imx415_write_register(ViPipe, 0x3093, 0x00);
	imx415_write_register(ViPipe, 0x3094, 0x00);
	imx415_write_register(ViPipe, 0x3095, 0x00);
	imx415_write_register(ViPipe, 0x3096, 0x00);
	imx415_write_register(ViPipe, 0x3097, 0x00);
	imx415_write_register(ViPipe, 0x30C0, 0x2A);
	imx415_write_register(ViPipe, 0x30C1, 0x00);
	imx415_write_register(ViPipe, 0x30CC, 0x00);
	imx415_write_register(ViPipe, 0x30CD, 0x00);
	imx415_write_register(ViPipe, 0x30CF, 0x00);
	imx415_write_register(ViPipe, 0x30D9, 0x06);
	imx415_write_register(ViPipe, 0x30DA, 0x02);
	imx415_write_register(ViPipe, 0x30E2, 0x32);
	imx415_write_register(ViPipe, 0x30E3, 0x00);
	imx415_write_register(ViPipe, 0x3115, 0x00);
	imx415_write_register(ViPipe, 0x3116, 0x24);
	imx415_write_register(ViPipe, 0x3118, 0xC6);
	imx415_write_register(ViPipe, 0x3119, 0x00);
	imx415_write_register(ViPipe, 0x311A, 0x5A);
	imx415_write_register(ViPipe, 0x311B, 0x01);
	imx415_write_register(ViPipe, 0x311E, 0x24);
	imx415_write_register(ViPipe, 0x3260, 0x01);
	imx415_write_register(ViPipe, 0x32C8, 0x01);
	imx415_write_register(ViPipe, 0x32D4, 0x21);
	imx415_write_register(ViPipe, 0x32EC, 0xA1);
	imx415_write_register(ViPipe, 0x344C, 0x2B);
	imx415_write_register(ViPipe, 0x344D, 0x01);
	imx415_write_register(ViPipe, 0x344E, 0xED);
	imx415_write_register(ViPipe, 0x344F, 0x01);
	imx415_write_register(ViPipe, 0x3450, 0xF6);
	imx415_write_register(ViPipe, 0x3451, 0x02);
	imx415_write_register(ViPipe, 0x3452, 0x7F);
	imx415_write_register(ViPipe, 0x3453, 0x03);
	imx415_write_register(ViPipe, 0x358A, 0x04);
	imx415_write_register(ViPipe, 0x35A1, 0x02);
	imx415_write_register(ViPipe, 0x35EC, 0x27);
	imx415_write_register(ViPipe, 0x35EE, 0x8D);
	imx415_write_register(ViPipe, 0x35F0, 0x8D);
	imx415_write_register(ViPipe, 0x35F2, 0x29);
	imx415_write_register(ViPipe, 0x36BC, 0x0C);
	imx415_write_register(ViPipe, 0x36CC, 0x53);
	imx415_write_register(ViPipe, 0x36CD, 0x00);
	imx415_write_register(ViPipe, 0x36CE, 0x3C);
	imx415_write_register(ViPipe, 0x36D0, 0x8C);
	imx415_write_register(ViPipe, 0x36D1, 0x00);
	imx415_write_register(ViPipe, 0x36D2, 0x71);
	imx415_write_register(ViPipe, 0x36D4, 0x3C);
	imx415_write_register(ViPipe, 0x36D6, 0x53);
	imx415_write_register(ViPipe, 0x36D7, 0x00);
	imx415_write_register(ViPipe, 0x36D8, 0x71);
	imx415_write_register(ViPipe, 0x36DA, 0x8C);
	imx415_write_register(ViPipe, 0x36DB, 0x00);
	imx415_write_register(ViPipe, 0x3701, 0x03);
	imx415_write_register(ViPipe, 0x3720, 0x00);
	imx415_write_register(ViPipe, 0x3724, 0x02);
	imx415_write_register(ViPipe, 0x3726, 0x02);
	imx415_write_register(ViPipe, 0x3732, 0x02);
	imx415_write_register(ViPipe, 0x3734, 0x03);
	imx415_write_register(ViPipe, 0x3736, 0x03);
	imx415_write_register(ViPipe, 0x3742, 0x03);
	imx415_write_register(ViPipe, 0x3862, 0xE0);
	imx415_write_register(ViPipe, 0x38CC, 0x30);
	imx415_write_register(ViPipe, 0x38CD, 0x2F);
	imx415_write_register(ViPipe, 0x395C, 0x0C);
	imx415_write_register(ViPipe, 0x39A4, 0x07);
	imx415_write_register(ViPipe, 0x39A8, 0x32);
	imx415_write_register(ViPipe, 0x39AA, 0x32);
	imx415_write_register(ViPipe, 0x39AC, 0x32);
	imx415_write_register(ViPipe, 0x39AE, 0x32);
	imx415_write_register(ViPipe, 0x39B0, 0x32);
	imx415_write_register(ViPipe, 0x39B2, 0x2F);
	imx415_write_register(ViPipe, 0x39B4, 0x2D);
	imx415_write_register(ViPipe, 0x39B6, 0x28);
	imx415_write_register(ViPipe, 0x39B8, 0x30);
	imx415_write_register(ViPipe, 0x39BA, 0x30);
	imx415_write_register(ViPipe, 0x39BC, 0x30);
	imx415_write_register(ViPipe, 0x39BE, 0x30);
	imx415_write_register(ViPipe, 0x39C0, 0x30);
	imx415_write_register(ViPipe, 0x39C2, 0x2E);
	imx415_write_register(ViPipe, 0x39C4, 0x2B);
	imx415_write_register(ViPipe, 0x39C6, 0x25);
	imx415_write_register(ViPipe, 0x3A42, 0xD1);
	imx415_write_register(ViPipe, 0x3A4C, 0x77);
	imx415_write_register(ViPipe, 0x3AE0, 0x02);
	imx415_write_register(ViPipe, 0x3AEC, 0x0C);
	imx415_write_register(ViPipe, 0x3B00, 0x2E);
	imx415_write_register(ViPipe, 0x3B06, 0x29);
	imx415_write_register(ViPipe, 0x3B98, 0x25);
	imx415_write_register(ViPipe, 0x3B99, 0x21);
	imx415_write_register(ViPipe, 0x3B9B, 0x13);
	imx415_write_register(ViPipe, 0x3B9C, 0x13);
	imx415_write_register(ViPipe, 0x3B9D, 0x13);
	imx415_write_register(ViPipe, 0x3B9E, 0x13);
	imx415_write_register(ViPipe, 0x3BA1, 0x00);
	imx415_write_register(ViPipe, 0x3BA2, 0x06);
	imx415_write_register(ViPipe, 0x3BA3, 0x0B);
	imx415_write_register(ViPipe, 0x3BA4, 0x10);
	imx415_write_register(ViPipe, 0x3BA5, 0x14);
	imx415_write_register(ViPipe, 0x3BA6, 0x18);
	imx415_write_register(ViPipe, 0x3BA7, 0x1A);
	imx415_write_register(ViPipe, 0x3BA8, 0x1A);
	imx415_write_register(ViPipe, 0x3BA9, 0x1A);
	imx415_write_register(ViPipe, 0x3BAC, 0xED);
	imx415_write_register(ViPipe, 0x3BAD, 0x01);
	imx415_write_register(ViPipe, 0x3BAE, 0xF6);
	imx415_write_register(ViPipe, 0x3BAF, 0x02);
	imx415_write_register(ViPipe, 0x3BB0, 0xA2);
	imx415_write_register(ViPipe, 0x3BB1, 0x03);
	imx415_write_register(ViPipe, 0x3BB2, 0xE0);
	imx415_write_register(ViPipe, 0x3BB3, 0x03);
	imx415_write_register(ViPipe, 0x3BB4, 0xE0);
	imx415_write_register(ViPipe, 0x3BB5, 0x03);
	imx415_write_register(ViPipe, 0x3BB6, 0xE0);
	imx415_write_register(ViPipe, 0x3BB7, 0x03);
	imx415_write_register(ViPipe, 0x3BB8, 0xE0);
	imx415_write_register(ViPipe, 0x3BBA, 0xE0);
	imx415_write_register(ViPipe, 0x3BBC, 0xDA);
	imx415_write_register(ViPipe, 0x3BBE, 0x88);
	imx415_write_register(ViPipe, 0x3BC0, 0x44);
	imx415_write_register(ViPipe, 0x3BC2, 0x7B);
	imx415_write_register(ViPipe, 0x3BC4, 0xA2);
	imx415_write_register(ViPipe, 0x3BC8, 0xBD);
	imx415_write_register(ViPipe, 0x3BCA, 0xBD);
	imx415_write_register(ViPipe, 0x4000, 0x10);
	imx415_write_register(ViPipe, 0x4001, 0x03);
	imx415_write_register(ViPipe, 0x4004, 0x00);
	imx415_write_register(ViPipe, 0x4005, 0x06);
	imx415_write_register(ViPipe, 0x400C, 0x00);
	imx415_write_register(ViPipe, 0x4018, 0x8F);
	imx415_write_register(ViPipe, 0x4019, 0x00);
	imx415_write_register(ViPipe, 0x401A, 0x4F);
	imx415_write_register(ViPipe, 0x401B, 0x00);
	imx415_write_register(ViPipe, 0x401C, 0x47);
	imx415_write_register(ViPipe, 0x401D, 0x00);
	imx415_write_register(ViPipe, 0x401E, 0x37);
	imx415_write_register(ViPipe, 0x401F, 0x01);
	imx415_write_register(ViPipe, 0x4020, 0x4F);
	imx415_write_register(ViPipe, 0x4021, 0x00);
	imx415_write_register(ViPipe, 0x4022, 0x87);
	imx415_write_register(ViPipe, 0x4023, 0x00);
	imx415_write_register(ViPipe, 0x4024, 0x4F);
	imx415_write_register(ViPipe, 0x4025, 0x00);
	imx415_write_register(ViPipe, 0x4026, 0x7F);
	imx415_write_register(ViPipe, 0x4027, 0x00);
	imx415_write_register(ViPipe, 0x4028, 0x3F);
	imx415_write_register(ViPipe, 0x4029, 0x00);
	imx415_write_register(ViPipe, 0x4074, 0x00);
	imx415_write_register(ViPipe, 0x3000, 0x00);
	imx415_write_register(ViPipe, 0x3002, 0x00);
	imx415_default_reg_init(ViPipe);

	CVI_TRACE_SNS(CVI_DBG_INFO, "ViPipe:%d,===IMX415 2568x2160@25 Init success!===\n", ViPipe);
}


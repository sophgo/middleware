/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef RESET_MNG_H_
#define RESET_MNG_H_

#include "includes.h"
#include "hdmi_tx_app.h"


#define HDMITX_RESET_NEG		0x0
#define HDMITX_MAIN_BIT			0
#define HDMITX_DMAH_BIT			4

#define HDCP22EXT_RESET_NEG		0x4
#define HDCP22EXT_RSTN_BIT		0

#define HT3XFACE_RESET_NEG		0x8
#define HT3_SPDIF_BIT			6
#define HT3_I2C_BIT			5
#define HT3_CEA_BIT			4
#define HT3_DLY_CFG_BIT			1
#define HT3_DLYCTRL_BIT 		0

#define TXPHYXFACE_RESET_NEG		0xC
#define TXPHY_TMDS_BIT			2
#define TXPHY_DLY_CFG_BIT 		1
#define TXPHY_DLYCTRL_BIT		0

#define TXPHYXFACE_RESET_POS		0x10
#define TXPHY_ZCAL_BIT			0

#define VIDEOGEN_RESET_NEG		0x14
#define VIDEOGEN_BIT			0

#define AUDIOGEN_RESET_NEG		0x18
#define AUDIOGEN_BIT			0

#define MMCM_RESET_POS			0x20
#define MMCM_AUDIOCLK_BIT		1
#define MMCM_PIXELCLK_BIT		0

#define MMCM_LOCKSTS			0x24
#define MMCM_SYSTEMCLK_BIT		2
#define MMCM_AUDIOCLK_BIT		1
#define MMCM_PIXELCLK_BIT		0

#define CEC_CLKGEN			0x28
#define CEC_SFR_CLKDIV_BIT		0

#define AUDIO_CLKGEN_DIV		0x2c
#define AUDCLK_DIV_BIT			0

#define PIXELCLK_SRC_SEL		0x30
#define PIXELCLK_SRC_BIT		0

#define AUDIOCLK_SRC_SEL		0x34
#define AUDIOCLK_SRX_BIT		0



void rst_mng_write(struct hdmi_tx_app *app, uint32_t data, uint32_t reg);

uint32_t rst_mng_read(struct hdmi_tx_app *app, uint32_t reg);

void zcal_reset(uint32_t value);

void hdmi_tx_core_reset(uint32_t value);

void mmcm_pixel_clk_reset(int enable);
void mmcm_audio_clk_reset(int enable);
uint32_t video_mmcm_lock(void);
uint32_t audio_mmcm_lock(void);

int mmcm_audio_clk_div(int div);

void clean_all_reset(void);

void hdmitx_clean_reset(void);
void hdcp22_clean_reset(void);

#endif /* RESET_MNG_H_ */

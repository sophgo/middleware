// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "reset_mng.h"
#include "hdmi_tx_system_parameters.h"
#include "platform.h"
#include "system.h"

void rst_mng_write(struct hdmi_tx_app *app, uint32_t reg, uint32_t data){
	int ret = 0;
	fb_ioctl_data write_data;
	write_data.address = reg + PROTO_HDMI_TX_APB_CRM_RSTMGR_ADDRESS_START;
	write_data.value = data;
	return;
	if(app->hdmi_tx_driver < 0)
		return;

	LOGGER(SNPS_TRACE, "%s:addr 0x%x - value 0x%x", __func__,
			write_data.address, write_data.value);

	ret = ioctl(app->hdmi_tx_driver, FB_HDMI_CORE_WRITE, &write_data);
	if(ret < 0){
		if(app->verbose)
			LOGGER(SNPS_ERROR, "%s:IOCTL error [%d]\n", __func__, ret);
	}
}

uint32_t rst_mng_read(struct hdmi_tx_app *app, uint32_t reg){
	int ret = 0;
	fb_ioctl_data read_data;
	read_data.address = reg + PROTO_HDMI_TX_APB_CRM_RSTMGR_ADDRESS_START;
	return 0;
	if(app->hdmi_tx_driver < 0)
		return 0;

	ret = ioctl(app->hdmi_tx_driver, FB_HDMI_CORE_READ, &read_data);
	if(ret < 0){
		if(app->verbose)
			LOGGER(SNPS_ERROR, "%s:IOCTL error [%d]\n", __func__, ret);
	}

	LOGGER(SNPS_TRACE, "%s:addr 0x%x - value 0x%x", __func__,
			read_data.address, read_data.value);

	return read_data.value;
}

void zcal_reset(uint32_t value)
{
	struct hdmi_tx_app *p_app = get_platform();
	uint32_t reg = 0;

	LOG_TRACE();

	reg = rst_mng_read(p_app, TXPHYXFACE_RESET_POS);
	if (value) {
		rst_mng_write(p_app, TXPHYXFACE_RESET_POS, reg | (1 << TXPHY_ZCAL_BIT));
	}
	else {
		reg &= ~(1 << TXPHY_ZCAL_BIT);
		rst_mng_write(p_app, TXPHYXFACE_RESET_POS, reg);
	}
}

void hdmi_tx_core_reset(uint32_t value)
{
	struct hdmi_tx_app *p_app = get_platform();
	uint32_t reg = 0;

	LOG_TRACE();

	reg = rst_mng_read(p_app, HDMITX_RESET_NEG);
	if (value) {
		rst_mng_write(p_app, HDMITX_RESET_NEG, reg & ~(1 << HDMITX_MAIN_BIT));
	}
	else {
		reg |= (1 << HDMITX_MAIN_BIT);
		rst_mng_write(p_app, HDMITX_RESET_NEG, reg);
	}
}

void mmcm_pixel_clk_reset(int enable)
{
	struct hdmi_tx_app *p_app = get_platform();
	uint32_t value = 0;

	value = rst_mng_read(p_app, MMCM_RESET_POS);

	value &= ~(1 << MMCM_PIXELCLK_BIT);
	value |= (enable << MMCM_PIXELCLK_BIT);

	rst_mng_write(p_app, MMCM_RESET_POS, value);
}

void mmcm_audio_clk_reset(int enable)
{
	struct hdmi_tx_app *p_app = get_platform();
	uint32_t value = 0;

	value = rst_mng_read(p_app, MMCM_RESET_POS);

	value &= ~(1 << MMCM_AUDIOCLK_BIT);
	value |= (enable << MMCM_AUDIOCLK_BIT);

	rst_mng_write(p_app, MMCM_RESET_POS, value);
}

int mmcm_audio_clk_div(int div)
{
	struct hdmi_tx_app *p_app = get_platform();
	uint32_t setting = 0;
	switch(div) {
		case 1: setting = 0; break;
		case 2: setting = 1; break;
		case 4: setting = 2; break;
		default:
			return FALSE;
	}

	rst_mng_write(p_app, AUDIO_CLKGEN_DIV, setting);
	return TRUE;
}

void mmcm_clean_reset(void){
	struct hdmi_tx_app *p_app = get_platform();
	rst_mng_write(p_app, MMCM_RESET_POS, 0x0);
}

void hdmitx_clean_reset(void){
	struct hdmi_tx_app *p_app = get_platform();
	rst_mng_write(p_app, HDMITX_RESET_NEG, 0xff);
}

void hdcp22_clean_reset(void){
	struct hdmi_tx_app *p_app = get_platform();
	rst_mng_write(p_app, HDCP22EXT_RESET_NEG, 0xff);
}

void ht3xface_clean_reset(void){
	struct hdmi_tx_app *p_app = get_platform();
	rst_mng_write(p_app, HT3XFACE_RESET_NEG, 0xff);
}

void txphyxface_clean_reset(void){
	struct hdmi_tx_app *p_app = get_platform();
	//rst_mng_write(p_app, TXPHYXFACE_RESET_NEG, 0xff);
	rst_mng_write(p_app, TXPHYXFACE_RESET_NEG, 0x07);
	rst_mng_write(p_app, TXPHYXFACE_RESET_POS, 0x00);
}

void videogen_clean_reset(void){
	struct hdmi_tx_app *p_app = get_platform();
	rst_mng_write(p_app, VIDEOGEN_RESET_NEG, 0xff);
}

void audiogen_clean_reset(void){
	struct hdmi_tx_app *p_app = get_platform();
	rst_mng_write(p_app, AUDIOGEN_RESET_NEG, 0xff);
}

void clean_all_reset(void){
	hdmitx_clean_reset();
	hdcp22_clean_reset();
	mmcm_clean_reset();
	ht3xface_clean_reset();
	txphyxface_clean_reset();
	videogen_clean_reset();
	audiogen_clean_reset();
}

uint32_t video_mmcm_lock(void){
	struct hdmi_tx_app *p_app = get_platform();
	if(rst_mng_read(p_app, MMCM_LOCKSTS) & 0x1)
		return TRUE;

	return FALSE;
}

uint32_t audio_mmcm_lock(void){
	struct hdmi_tx_app *p_app = get_platform();
	if(rst_mng_read(p_app, MMCM_LOCKSTS) & 0x2)
		return TRUE;

	return FALSE;
}

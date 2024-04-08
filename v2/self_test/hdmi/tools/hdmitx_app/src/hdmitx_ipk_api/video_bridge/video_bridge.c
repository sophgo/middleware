// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "video_bridge.h"
#include "vg_patterns.h"
#include "hdmi_tx_system_parameters.h"
#include "platform.h"
#include "system.h"


/**
 * Prototypes
 */
void _vg_sw_reset(struct hdmi_tx_app *app, int status);
void _vg_ycc_enable(struct hdmi_tx_app *app, int bit);
void _vg_ycc422_mapping(struct hdmi_tx_app *app, int bit);
void _vg_v_blank_osc(struct hdmi_tx_app *app, int bit);
void _vg_color_increment(struct hdmi_tx_app *app, int bit);
void _vg_interlaced(struct hdmi_tx_app *app, int bit);
void _vg_v_sync_polarity(struct hdmi_tx_app *app, int bit);
void _vg_h_sync_polarity(struct hdmi_tx_app *app, int bit);
void _vg_data_enable_polarity(struct hdmi_tx_app *app, int bit);
void _vg_color_resolution(struct hdmi_tx_app *app, uint32_t color_res);
void _vg_pixel_repetition_input(struct hdmi_tx_app *app, uint32_t data);
void _vg_h_active(struct hdmi_tx_app *app, uint32_t data);
void _vg_h_blank(struct hdmi_tx_app *app, uint32_t data);
void _vg_h_sync_edge_delay(struct hdmi_tx_app *app, uint32_t data);
void _vg_h_sync_pulse_width(struct hdmi_tx_app *app, uint32_t data);
void _vg_v_active(struct hdmi_tx_app *app, uint32_t data);
void _vg_v_blank(struct hdmi_tx_app *app, uint32_t data);
void _vg_v_sync_edge_delay(struct hdmi_tx_app *app, uint32_t data);
void _vg_v_sync_pulse_width(struct hdmi_tx_app *app, uint32_t data);
void _vg_3D_enable(struct hdmi_tx_app *app, int bit);
void _vg_3D_structure(struct hdmi_tx_app *app, uint32_t data);
void _vg_write_start(struct hdmi_tx_app *app, uint32_t data);
void _vg_write_data(struct hdmi_tx_app *app, uint32_t data);
uint32_t _vg_get_write_current_addr(struct hdmi_tx_app *app);


void video_bridge_write(struct hdmi_tx_app *app, uint32_t reg, uint32_t data){
	int ret = 0;
	fb_ioctl_data write_data;
	return;
	write_data.address = reg + PROTO_HDMI_TX_APB_VIDEOBRIDGE_ADDRESS_START;
	write_data.value = data;

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

uint32_t video_bridge_read(struct hdmi_tx_app *app, uint32_t reg){
	int ret = 0;
	fb_ioctl_data read_data;
	return 0;
	read_data.address = reg + PROTO_HDMI_TX_APB_VIDEOBRIDGE_ADDRESS_START;

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

void video_chess_board_config(struct hdmi_tx_app *app)
{
	u16 width = app->mode.pVideo.mDtd.mHActive / 4;
	u16 height = app->mode.pVideo.mDtd.mVActive / 4;
	u8 color_inc = 0xff / 12;

	video_bridge_write(app, VG_CBWIDTH0, width & 0xFF);
	video_bridge_write(app, VG_CBWIDTH1, ((width >> 8) & 0x3));

	video_bridge_write(app, VG_CBHEIGHT0, height & 0xFF);
	video_bridge_write(app, VG_CBHEIGHT1, ((height >> 8) & 0x3));

	// Set pattern color
	video_bridge_write(app, VG_CBCOLORA0, color_inc * 0);
	video_bridge_write(app, VG_CBCOLORA1, color_inc * 1);
	video_bridge_write(app, VG_CBCOLORA2, color_inc * 2);
	video_bridge_write(app, VG_CBCOLORA3, color_inc * 3);
	video_bridge_write(app, VG_CBCOLORA4, color_inc * 4);
	video_bridge_write(app, VG_CBCOLORA5, color_inc * 5);

	video_bridge_write(app, VG_CBCOLORB0, color_inc * 6);
	video_bridge_write(app, VG_CBCOLORB1, color_inc * 7);
	video_bridge_write(app, VG_CBCOLORB2, color_inc * 8);
	video_bridge_write(app, VG_CBCOLORB3, color_inc * 9);
	video_bridge_write(app, VG_CBCOLORB4, color_inc * 10);
	video_bridge_write(app, VG_CBCOLORB5, color_inc * 11);
}

void video_generator_pattern_mode(struct hdmi_tx_app *app, u8 value)
{
	video_bridge_write(app, VG_PATTERNMODE, value & 0x3);
}

int video_generator_config(struct hdmi_tx_app *app){
	unsigned resolution = 0;
	uint8_t color_res = 0;
	encoding_t encoding = RGB;
	dtd_t *dtd;
	uint8_t dtd_code = 0;
	uint8_t video_format = 0;

	LOG_TRACE();

	dtd = &app->mode.pVideo.mDtd;
	color_res = app->mode.pVideo.mColorResolution;
	encoding = app->mode.pVideo.mEncodingIn;
	dtd_code = dtd->mCode;

	if ((color_res == 8) || (color_res == 0))
		resolution = 0;
	else if (color_res == 10)
		resolution = 1;
	else if (color_res == 12)
		resolution = 2;
	else if ((color_res == 16) && (encoding != YCC422))
		resolution = 3;
	else {
//		error_set(ERR_COLOR_DEPTH_NOT_SUPPORTED);
		LOGGER(SNPS_ERROR,"%s:Invalid color depth: %d", __func__, color_res);
		return FALSE;
	}

	_vg_ycc_enable(app, (encoding != RGB) ? 1 : 0);
	/*TODO: this is a quick fix for pattern in Ycc422 */
/*	_vg_ycc422_mapping(app, (encoding == YCC422) ? 1 : 0); */
	_vg_ycc422_mapping(app, 0);
	if(dtd_code == 39){
		_vg_v_blank_osc(app, 0);
	}
	else{
		_vg_v_blank_osc(app, dtd->mInterlaced ? 1 : 0);
	}
	_vg_color_increment(app, 0);
	_vg_interlaced(app, dtd->mInterlaced);
	_vg_v_sync_polarity(app, dtd->mVSyncPolarity);
	_vg_h_sync_polarity(app, dtd->mHSyncPolarity);
	_vg_data_enable_polarity(app, app->data_enable_polarity);
	_vg_color_resolution(app, resolution);
	_vg_pixel_repetition_input(app,	dtd->mPixelRepetitionInput);

	if (encoding == YCC420) {
		LOGGER(SNPS_WARN, "Encoding configured to YCC 420");
		_vg_h_active(app, dtd->mHActive/2);
		_vg_h_blank(app, dtd->mHBlanking/2);
		_vg_h_sync_edge_delay(app, dtd->mHSyncOffset/2);
		_vg_h_sync_pulse_width(app, dtd->mHSyncPulseWidth/2);
	} else {
		_vg_h_active(app, dtd->mHActive);
		_vg_h_blank(app, dtd->mHBlanking);
		_vg_h_sync_edge_delay(app, dtd->mHSyncOffset);
		_vg_h_sync_pulse_width(app, dtd->mHSyncPulseWidth);
	}

	_vg_v_active(app, dtd->mVActive);
	_vg_v_blank(app, dtd->mVBlanking);
	_vg_v_sync_edge_delay(app, dtd->mVSyncOffset);
	_vg_v_sync_pulse_width(app, dtd->mVSyncPulseWidth);

	video_format = app->mode.pVideo.mHdmiVideoFormat;
	_vg_3D_enable(app, (video_format == 2) ? 1 : 0);
	_vg_3D_structure(app, app->mode.pVideo.m3dStructure);
	_vg_sw_reset(app, TRUE);
	_vg_sw_reset(app, FALSE);
	return TRUE;
}


void _vg_sw_reset(struct hdmi_tx_app *app, int status){
	LOG_TRACE1(status);
	/* active low */
	video_bridge_write(app, VG_SWRSTZ, status ? 0 : 1);
}

void _vg_ycc_enable(struct hdmi_tx_app *app, int bit){
	uint32_t value = 0;
	LOG_TRACE1(bit);
	value = video_bridge_read(app, VG_CONF);

	if(bit)
		value |= 1 << 7;
	else
		value &= ~(1 << 7);

	video_bridge_write(app, VG_CONF, value);
}

void _vg_ycc422_mapping(struct hdmi_tx_app *app, int bit){
	uint32_t value = 0;
	LOG_TRACE1(bit);

	value = video_bridge_read(app, VG_CONF);

	if(bit)
		value |= 1 << 6;
	else
		value &= ~(1 << 6);

	video_bridge_write(app, VG_CONF, value);

}

void _vg_v_blank_osc(struct hdmi_tx_app *app, int bit) {
	uint32_t value = 0;
	LOG_TRACE1(bit);

	value = video_bridge_read(app, VG_CONF);

	if(bit)
		value |= 1 << 5;
	else
		value &= ~(1 << 5);

	video_bridge_write(app, VG_CONF, value);
}

void _vg_color_increment(struct hdmi_tx_app *app, int bit){
	uint32_t value = 0;
	LOG_TRACE1(bit);

	value = video_bridge_read(app, VG_CONF);

	if(bit)
		value |= 1 << 4;
	else
		value &= ~(1 << 4);

	video_bridge_write(app, VG_CONF, value);
}

void _vg_interlaced(struct hdmi_tx_app *app, int bit){
	uint32_t value = 0;
	LOG_TRACE1(bit);

	value = video_bridge_read(app, VG_CONF);

	if(bit)
		value |= 1 << 3;
	else
		value &= ~(1 << 3);

	video_bridge_write(app, VG_CONF, value);
}

void _vg_v_sync_polarity(struct hdmi_tx_app *app, int bit){
	uint32_t value = 0;
	LOG_TRACE1(bit);

	value = video_bridge_read(app, VG_CONF);

	if(bit)
		value |= 1 << 2;
	else
		value &= ~(1 << 2);

	video_bridge_write(app, VG_CONF, value);
}

void _vg_h_sync_polarity(struct hdmi_tx_app *app, int bit){
	uint32_t value = 0;
	LOG_TRACE1(bit);

	value = video_bridge_read(app, VG_CONF);

	if(bit)
		value |= 1 << 1;
	else
		value &= ~(1 << 1);

	video_bridge_write(app, VG_CONF, value);
}

void _vg_data_enable_polarity(struct hdmi_tx_app *app, int bit){
	uint32_t value = 0;
	LOG_TRACE1(bit);

	value = video_bridge_read(app, VG_CONF);

	if(bit)
		value |= 1;
	else
		value &= ~(1);

	video_bridge_write(app, VG_CONF, value);
}

void _vg_color_resolution(struct hdmi_tx_app *app, uint32_t color_res){
	uint32_t value = 0x0;
	uint32_t mask = BIT(2) - 1;
	int shift = 4;

	LOG_TRACE1(color_res);

	value = video_bridge_read(app, VG_PREP);

	value &= ~(mask << shift);
	value |= (color_res & mask) << shift;

	video_bridge_write(app, VG_PREP, value);
}

void _vg_pixel_repetition_input(struct hdmi_tx_app *app, uint32_t data){
	uint32_t value = 0x0;
	uint32_t mask = BIT(4) - 1;
	int shift = 0;

	LOG_TRACE1(data);

	value = video_bridge_read(app, VG_PREP);

	value &= ~(mask << shift);
	value |= (data & mask) << shift;

	video_bridge_write(app, VG_PREP, value);
}

void _vg_h_active(struct hdmi_tx_app *app, uint32_t data){
	LOG_TRACE1(data);

	video_bridge_write(app, VG_HACTIVE0, data & 0xff);
	video_bridge_write(app, VG_HACTIVE1, (data >> 8) & 0x1f);
}

void _vg_h_blank(struct hdmi_tx_app *app, uint32_t data){
	LOG_TRACE1(data);

	video_bridge_write(app, VG_HBLANK0, data & 0xff);
	video_bridge_write(app, VG_HBLANK1, (data >> 8) & 0x1f);
}

void _vg_h_sync_edge_delay(struct hdmi_tx_app *app, uint32_t data){
	LOG_TRACE1(data);

	video_bridge_write(app, VG_HDELAY0, data & 0xff);
	video_bridge_write(app, VG_HDELAY1, (data >> 8) & 0xf);
}

void _vg_h_sync_pulse_width(struct hdmi_tx_app *app, uint32_t data){
	LOG_TRACE1(data);

	video_bridge_write(app, VG_HWIDTH0, data & 0xff);
	video_bridge_write(app, VG_HWIDTH1, (data >> 8) & 0x1);
}

void _vg_v_active(struct hdmi_tx_app *app, uint32_t data){
	LOG_TRACE1(data);

	video_bridge_write(app, VG_VACTIVE0, data & 0xff);
	video_bridge_write(app, VG_VACTIVE1, (data >> 8) & 0xf);
}

void _vg_v_blank(struct hdmi_tx_app *app, uint32_t data){
	LOG_TRACE1(data);

	video_bridge_write(app, VG_VBLANK0, data & 0xff);
}

void _vg_v_sync_edge_delay(struct hdmi_tx_app *app, uint32_t data){
	LOG_TRACE1(data);

	video_bridge_write(app, VG_VDELAY0, data & 0xff);
}

void _vg_v_sync_pulse_width(struct hdmi_tx_app *app, uint32_t data){
	LOG_TRACE1(data);

	video_bridge_write(app, VG_VWIDTH0, data & 0x3f);
}

void _vg_3D_enable(struct hdmi_tx_app *app, int bit){
	uint32_t value = 0;
	LOG_TRACE1(bit);

	value = video_bridge_read(app, VG_3DSTRUCT);

	if(bit)
		value |= 1 << 4;
	else
		value &= ~(1 << 4);

	video_bridge_write(app, VG_3DSTRUCT, value);
}

void _vg_3D_structure(struct hdmi_tx_app *app, uint32_t data){
	uint32_t value = 0;
	LOG_TRACE1(data);

	value = video_bridge_read(app, VG_3DSTRUCT);

	value &= 0xf0;
	value |= data & 0x0f;

	video_bridge_write(app, VG_3DSTRUCT, value);
}

void _vg_write_start(struct hdmi_tx_app *app, uint32_t data){
	uint32_t value = 0;
	LOG_TRACE1(data);

	video_bridge_write(app, VG_RAM_ADDR0, data & 0xff);
	video_bridge_write(app, VG_RAM_ADDR1, (data >> 8) & 0x1f);

	// start RAM write
	value = video_bridge_read(app, VG_WRT_RAM_CTRL);
	value |= 0x1;
	video_bridge_write(app, VG_WRT_RAM_CTRL, value);
}

void _vg_write_data(struct hdmi_tx_app *app, uint32_t data){
	LOG_TRACE1(data);

	video_bridge_write(app, VG_WRT_RAM_DATA, data & 0xff);
}

uint32_t _vg_get_write_current_addr(struct hdmi_tx_app *app){
	uint32_t value = 0;

	value = (video_bridge_read(app, VG_WRT_RAM_STOP_ADDR0)) & 0xff;
	value |= (video_bridge_read(app, VG_WRT_RAM_STOP_ADDR1) & 0x1f) << 8;

	return value;
}


/**
 * Video Generator fill pattern
 */
void _vg_write_pattern(uint16_t offset, encoding_t encoding)
{
	struct hdmi_tx_app * p_app = get_platform();

	// Set address ADDR0
	video_bridge_write(p_app, VG_RAM_ADDR0, (offset & 0x00ff));

	// Set address ADDR1
	video_bridge_write(p_app, VG_RAM_ADDR1, ((offset & 0x1f00) >> 8));

	// Toggle CTRL
	video_bridge_write(p_app, VG_WRT_RAM_CTRL, 1);
	video_bridge_write(p_app, VG_WRT_RAM_CTRL, 0);

	switch (encoding) {
	case RGB:
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_rgb[offset][0]);
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_rgb[offset][1]);
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_rgb[offset][2]);
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_rgb[offset][3]);
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_rgb[offset][4]);
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_rgb[offset][5]);
		break;
	case YCC444:
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_ycc444[offset][0]);
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_ycc444[offset][1]);
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_ycc444[offset][2]);
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_ycc444[offset][3]);
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_ycc444[offset][4]);
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_ycc444[offset][5]);
		break;
	case YCC422:
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_ycc422[offset][0]);
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_ycc422[offset][1]);
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_ycc422[offset][2]);
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_ycc422[offset][3]);
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_ycc422[offset][4]);
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_ycc422[offset][5]);
		break;
	case YCC420:
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_ycc420[offset][0]);
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_ycc420[offset][1]);
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_ycc420[offset][2]);
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_ycc420[offset][3]);
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_ycc420[offset][4]);
		video_bridge_write(p_app, VG_WRT_RAM_DATA, pattern_ycc420[offset][5]);
		break;
	case ENC_UNDEFINED:
	default:
		hdmitx_logger(SNPS_ERROR, "%s:Invalid encoding received - video bridge not configured", __func__);
	}
}


void video_generator_set_pattern(encoding_t encoding)
{
	uint32_t counter = 0;

	hdmitx_logger(SNPS_DEBUG, "%s:Writting new pattern to VG", __func__);

	while(counter < 4800){
		_vg_write_pattern(counter, encoding);
		counter = counter + 1;
	}

	hdmitx_logger(SNPS_DEBUG, "%s:Done", __func__);
}

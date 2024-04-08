// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "bsp/board.h"
#include "bsp/access.h"
#include "util/types.h"
#include "core/audio/audio_sample_reg.h"
#include "core/audio/audio_sample_spdif_reg.h"
#include "cec/cec_reg.h"
#include "bsp/eddc_reg.h"
#include "hdcp/hdcp_reg.h"
#include "core/frame_composer/frame_composer_reg.h"
#include "phy/phy_reg.h"
#include "core/video/video_packetizer_reg.h"
#include "util/log.h"

static struct board_wrappers *board_wrapper;

void register_board_wrappers(struct board_wrappers *board_wrp){
	board_wrapper = board_wrp;
}

/** Initialize board
 * @param baseAddr base address of controller
 * @param pixelClock pixel clock [10KHz]
 * @param cd color depth (8, 10, 12 or 16)
 * @return TRUE if successful
 */
uint16_t board_Initialize(hdmi_tx_dev_t *dev)
{
	unsigned i = 0;
	struct
	{
		uint32_t addr;
		uint8_t data;
	} cfg[] =
	{
		/* by default mask all interrupts */
		{ VP_MASK, 0xFF }, /* VP */
		{ FC_MASK0, 0xFF }, /* Packets */
		{ FC_MASK1, 0xFF }, /* Packets */
		{ FC_MASK2, 0xFF }, /* Packets */
		{ PHY_MASK0, 0xF1 }, /* PHY - leave HPD */
		{ AUD_INT, 0xFF }, /* AS - I2S */
		{ AUD_SPDIFINT, 0xFF }, /* AS - SPDIF */
		{ A_APIINTMSK, 0xFF }, /* HDCP */
		{ CEC_MASK, 0xFF }, /* CEC */
	};

	for (i = 0; i < (sizeof(cfg) / sizeof(cfg[0])); i++){
		dev_write(dev,cfg[i].addr, cfg[i].data);
	}

	return TRUE;
}

/** Set up oscillator for audio
 * @param baseAddr base address of controller
 * @param value: audio clock [Hz]
 * @return TRUE if successful
 */
uint16_t configure_audio_pll(hdmi_tx_dev_t *dev, double freq, uint16_t over_sampling_factor){
	if(board_wrapper->configure_audio_pll)
		return board_wrapper->configure_audio_pll(freq, over_sampling_factor);
	return 0;
}

/** Set up oscillator for video
 * @param baseAddr base address of controller
 * @param value: pixel clock [10KHz]
 * @param cd color depth
 * @return TRUE if successful
 */
uint16_t board_PixelClock(hdmi_tx_dev_t *dev)
{
	if(board_wrapper->configure_pixel_clock){
		LOGGER(SNPS_DEBUG, "Configure pixel clock with %fMHz", dev->snps_hdmi_ctrl.pixel_clock);
		return board_wrapper->configure_pixel_clock(dev->snps_hdmi_ctrl.pixel_clock);
	}
	return 0;
}

int  phy_interface_odelay(hdmi_tx_dev_t *dev, uint32_t lines, uint16_t delay)
{
	if(board_wrapper->data_output_delay){
		LOGGER(SNPS_INFO, "Configure Phy Interface Output data delay to %d", delay);
		return board_wrapper->data_output_delay(lines, delay);
	}
	return -1;
}

void board_ZcalReset(uint32_t value){
	if(board_wrapper->zcal_reset)
		board_wrapper->zcal_reset(value);
}

/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef BOARD_H_
#define BOARD_H_

#include <stdint.h>

#include "../hdmitx_dev.h"

struct board_wrappers {
	char name[30];

	uint16_t (*initialize) (uint16_t pixelClock, uint8_t cd);
	uint16_t (*configure_audio_pll) (double freq, uint16_t over_sampling_factor);
	uint16_t (*configure_pixel_clock) (double freq);
	int (*data_output_delay) (uint32_t lines,  uint32_t delay);
	void (*zcal_reset) (uint32_t value);
};

void register_board_wrappers(struct board_wrappers *board_wrp);

/*******************************
 * HDMI Tx Support functions   *
 *******************************/

/** Initialize board
 * @param baseAddr base address of controller
 * @param pixelClock pixel clock [10KHz]
 * @param cd color depth (8, 10, 12 or 16)
 * @return TRUE if successful
 */
uint16_t board_Initialize(hdmi_tx_dev_t *dev);

/** Set up oscillator for audio
 * @param baseAddr base address of controller
 * @param value: audio clock [Hz]
 * @return TRUE if successful
 */
uint16_t configure_audio_pll(hdmi_tx_dev_t *dev, double freq, uint16_t over_sampling_factor);

/** Set up oscillator for video
 * @param baseAddr base address of controller
 * @param value: pixel clock [10KHz]
 * @param cd color depth
 * @return TRUE if successful
 */
uint16_t board_PixelClock(hdmi_tx_dev_t *dev);

int  phy_interface_odelay(hdmi_tx_dev_t *dev, uint32_t lines, uint16_t delay);

/** Reset Zcal
 * @param value reset value
 * @return void
 */
void board_ZcalReset(uint32_t value);

#endif /* BOARD_H_ */

/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef PLATFORM_H_
#define PLATFORM_H_

#include "includes.h"
#include "hdmi_tx_app.h"


void set_platform(struct hdmi_tx_app *app);
struct hdmi_tx_app * get_platform();

/*******************************
 * HDMI Tx Support functions   *
 *******************************/

/** Set up oscillator for audio
 * @param baseAddr base address of controller
 * @param value: audio clock [Hz]
 * @return TRUE if successful
 */
uint16_t audio_clock(double value, uint16_t over_sampling_factor);

/** Set up oscillator for video
 * @param baseAddr base address of controller
 * @param value: pixel clock [10KHz]
 * @param cd color depth
 * @return TRUE if successful
 */
uint16_t pixel_clock(double freq);

struct hdmi_mode * app_get_user_config(struct hdmi_tx_app *app);

bool parse_ulong(const char *str,unsigned long *val);

void app_init_video(struct hdmi_mode * cfg);
void app_init_audio(struct hdmi_mode * cfg);

#endif /* PLATFORM_H_ */

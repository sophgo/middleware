/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef CLOCK_MNG_H_
#define CLOCK_MNG_H_

#include "includes.h"
#include "hdmi_tx_app.h"

void clk_mng_write(struct hdmi_tx_app *app, uint32_t data, uint32_t reg);

uint32_t clk_mng_read(struct hdmi_tx_app *app, uint32_t reg);

struct mmcm *get_video_mmcm_configs(double frequency);

struct mmcm *get_audio_mmcm_configs(double audio_clock_freq, uint16_t oversample_factor);

int configure_video_mmcm(struct hdmi_tx_app *app, struct mmcm *cfg);

int configure_audio_mmcm(struct hdmi_tx_app *app, struct mmcm * cfg);

#endif /* CLOCK_MNG_H_ */

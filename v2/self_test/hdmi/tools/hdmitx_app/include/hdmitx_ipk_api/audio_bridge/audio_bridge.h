/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef AUDIO_BRIDGE_H_
#define AUDIO_BRIDGE_H_

#include "includes.h"
#include "hdmi_tx_app.h"

#define AG_SWRSTZ 				0x00
#define AG_SWRSTZ_SHIFT 			0

void audio_bridge_write(struct hdmi_tx_app *app, uint32_t reg, uint32_t data);

uint32_t audio_bridge_read(struct hdmi_tx_app *app, uint32_t reg);

void audio_generator_config(struct hdmi_tx_app *app);


#endif /* AUDIO_BRIDGE_H_ */

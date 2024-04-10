/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef HALAUDIOI2S_H_
#define HALAUDIOI2S_H_

#include "../hdmitx_dev.h"
#include "core/audio.h"
#include "util/types.h"

void audio_i2s_configure(hdmi_tx_dev_t * dev, audioParams_t * audio);

void audio_i2s_select(hdmi_tx_dev_t *dev, u8 bit);

void audio_i2s_interrupt_mask(hdmi_tx_dev_t *dev, u8 value);

#endif	/* HALAUDIOI2S_H_ */

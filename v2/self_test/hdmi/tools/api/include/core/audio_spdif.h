/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef HALAUDIOSPDIF_H_
#define HALAUDIOSPDIF_H_

#include "../hdmitx_dev.h"
#include "core/audio.h"
#include "util/types.h"

void audio_spdif_config(hdmi_tx_dev_t *dev, audioParams_t *audio);

void audio_spdif_interrupt_mask(hdmi_tx_dev_t *dev, u8 value);

#endif	/* HALAUDIOSPDIF_H_ */

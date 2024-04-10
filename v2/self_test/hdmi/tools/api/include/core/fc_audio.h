/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef HALFRAMECOMPOSERAUDIO_H_
#define HALFRAMECOMPOSERAUDIO_H_

#include "../hdmitx_dev.h"
#include "util/types.h"
#include "core/audio_params.h"

void fc_audio_config(hdmi_tx_dev_t *dev, audioParams_t * audio);
void fc_audio_mute(hdmi_tx_dev_t *dev);
void fc_audio_unmute(hdmi_tx_dev_t *dev);

#endif	/* HALFRAMECOMPOSERAUDIO_H_ */

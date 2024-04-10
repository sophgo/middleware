/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef HALFRAMECOMPOSERAUDIOINFO_H_
#define HALFRAMECOMPOSERAUDIOINFO_H_

#include "../hdmitx_dev.h"
#include "core/audio_params.h"
#include "util/types.h"

void fc_audio_info_config(hdmi_tx_dev_t *dev, audioParams_t * audio);

#endif	/* HALFRAMECOMPOSERAUDIOINFO_H_ */

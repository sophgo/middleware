/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef HALVIDEOSAMPLER_H_
#define HALVIDEOSAMPLER_H_

#include "../hdmitx_dev.h"
#include "util/types.h"

void video_sampler_config(hdmi_tx_dev_t *dev, u8 map_code);

#endif	/* HALVIDEOSAMPLER_H_ */

/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef HALFRAMECOMPOSERAVI_H_
#define HALFRAMECOMPOSERAVI_H_

#include "../hdmitx_dev.h"
#include "core/video_params.h"
#include "util/types.h"

void fc_avi_config(hdmi_tx_dev_t *dev, videoParams_t * videoParams);

#endif	/* HALFRAMECOMPOSERAVI_H_ */

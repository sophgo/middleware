/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef HALFRAMECOMPOSERVIDEO_H_
#define HALFRAMECOMPOSERVIDEO_H_

#include "../hdmitx_dev.h"
#include "core/video_params.h"
#include "util/types.h"

int fc_video_config(hdmi_tx_dev_t *dev, videoParams_t *video);

void fc_video_hdcp_keepout(hdmi_tx_dev_t *dev, u8 bit);

#endif	/* HALFRAMECOMPOSERVIDEO_H_ */

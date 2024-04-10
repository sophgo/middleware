/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef HALFRAMECOMPOSERGAMUT_H_
#define HALFRAMECOMPOSERGAMUT_H_

#include "../hdmitx_dev.h"
#include "util/types.h"

void fc_gamut_enable_tx(hdmi_tx_dev_t *dev, u8 enable);

void fc_gamut_config(hdmi_tx_dev_t *dev);

void fc_gamut_packet_config(hdmi_tx_dev_t *dev, const u8 * gbdContent, u8 length);

#endif	/* HALFRAMECOMPOSERGAMUT_H_ */

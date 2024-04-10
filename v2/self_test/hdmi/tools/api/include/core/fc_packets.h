/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef HALFRAMECOMPOSERPACKETS_H_
#define HALFRAMECOMPOSERPACKETS_H_

#include "../hdmitx_dev.h"
#include "util/types.h"

#define ACP_TX  	0
#define ISRC1_TX 	1
#define ISRC2_TX 	2
#define SPD_TX 		4
#define VSD_TX 		3

void fc_packets_metadata_config(hdmi_tx_dev_t *dev);

void fc_packets_AutoSend(hdmi_tx_dev_t *dev, u8 enable, u8 mask);

void fc_packets_ManualSend(hdmi_tx_dev_t *dev, u8 mask);

void fc_packets_disable_all(hdmi_tx_dev_t *dev);

#endif	/* HALFRAMECOMPOSERPACKETS_H_ */

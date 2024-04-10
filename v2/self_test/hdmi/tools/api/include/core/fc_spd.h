/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef HALFRAMECOMPOSERSPD_H_
#define HALFRAMECOMPOSERSPD_H_

#include "../hdmitx_dev.h"
#include "util/types.h"

typedef struct fc_spd_info {
	const u8 * vName;
	u8 vLength;
	const u8 * pName;
	u8 pLength;
	u8 code;
	u8 autoSend;
}fc_spd_info_t;

int fc_spd_config(hdmi_tx_dev_t *dev, fc_spd_info_t *spd_data);

#endif	/* HALFRAMECOMPOSERSPD_H_ */

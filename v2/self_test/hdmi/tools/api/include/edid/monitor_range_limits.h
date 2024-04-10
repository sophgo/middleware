/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef MONITORRANGELIMITS_H_
#define MONITORRANGELIMITS_H_

#include "../hdmitx_dev.h"
#include "util/types.h"

/**
 * @file
 * Second Monitor Descriptor
 * Parse and hold Monitor Range Limits information read from EDID
 */
typedef struct {
	u8 mMinVerticalRate;

	u8 mMaxVerticalRate;

	u8 mMinHorizontalRate;

	u8 mMaxHorizontalRate;

	u8 mMaxPixelClock;

	int mValid;
} monitorRangeLimits_t;

void monitor_range_limits_reset(hdmi_tx_dev_t *dev, monitorRangeLimits_t * mrl);

#endif	/* MONITORRANGELIMITS_H_ */

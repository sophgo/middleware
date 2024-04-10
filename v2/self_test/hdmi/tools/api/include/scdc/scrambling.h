/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef SCRAMBLING_H_
#define SCRAMBLING_H_

#include "util/types.h"
#include "../../src/core/frame_composer/frame_composer_reg.h"
#include "../hdmitx_dev.h"


void scrambling(hdmi_tx_dev_t *dev, u8 enable);

void scrambling_Enable(hdmi_tx_dev_t *dev, u8 bit);

#endif	/* SCRAMBLING_H_ */

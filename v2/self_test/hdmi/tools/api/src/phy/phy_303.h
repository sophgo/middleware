/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef SRC_PHY_PHY_303_H_
#define SRC_PHY_PHY_303_H_

#include "../../include/hdmitx_dev.h"
#include "util/types.h"
#include "core/video.h"
#include "phy.h"
#include "phy/phy_i2c.h"

#include "core/main_controller.h"
#include "util/log.h"
#include "util/error.h"
#include "bsp/board.h"

#include "phy_reg.h"
#include "bsp/access.h"

#include "interrupt/interrupt_reg.h"
#include "system.h"



int phy303_configure_supported(hdmi_tx_dev_t *dev, double pClk, color_depth_t color, pixel_repetition_t pixel);
int phy303_configure(hdmi_tx_dev_t *dev, double pClk, color_depth_t color, pixel_repetition_t pixel);



#endif /* SRC_PHY_PHY_303_H_ */

// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "scdc/scdc.h"
#include "scdc/scrambling.h"
#include "util/types.h"
#include "util/log.h"
#include "core/fc_video.h"
#include "core/main_controller.h"
#include "bsp/access.h"

void scrambling(hdmi_tx_dev_t *dev, u8 enable){
	if (enable == 1) {
		scdc_scrambling_enable_flag(dev, 1);
		snps_sleep(100);

		/* Start/stop HDCP keep-out window generation not needed because it's always on */
		/* TMDS software reset request */
		mc_tmds_clock_reset(dev, TRUE);

		/* Enable/Disable Scrambling */
		scrambling_Enable(dev, TRUE);
	} else {
		/* Enable/Disable Scrambling */
		scrambling_Enable(dev, FALSE);
		scdc_scrambling_enable_flag(dev, 0);

		/* TMDS software reset request */
		mc_tmds_clock_reset(dev, FALSE);
	}
}

void scrambling_Enable(hdmi_tx_dev_t *dev, u8 bit)
{
	dev_write_mask(dev, FC_SCRAMBLER_CTRL, FC_SCRAMBLER_CTRL_SCRAMBLER_ON_MASK, bit);
}

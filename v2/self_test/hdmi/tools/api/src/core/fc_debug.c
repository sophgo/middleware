// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "frame_composer/frame_composer_reg.h"
#include "core/fc_debug.h"
#include "bsp/access.h"
#include "util/log.h"


void fc_force_audio(hdmi_tx_dev_t *dev, u8 bit)
{
	LOG_TRACE1(bit);
	dev_write_mask(dev, FC_DBGFORCE, FC_DBGFORCE_FORCEAUDIO_MASK, bit);
}

void fc_force_video(hdmi_tx_dev_t *dev, u8 bit)
{
	LOG_TRACE1(bit);

	/* avoid glitches */
	if (bit != 0) {
		dev_write(dev, FC_DBGTMDS2, 0x00);	/* R */
		dev_write(dev, FC_DBGTMDS1, 0x00);	/* G */
		dev_write(dev, FC_DBGTMDS0, 0xFF);	/* B */
		dev_write_mask(dev, FC_DBGFORCE, FC_DBGFORCE_FORCEVIDEO_MASK, 1);
	} else {
		dev_write_mask(dev, FC_DBGFORCE, FC_DBGFORCE_FORCEVIDEO_MASK, 0);
		dev_write(dev, FC_DBGTMDS2, 0x00);	/* R */
		dev_write(dev, FC_DBGTMDS1, 0x00);	/* G */
		dev_write(dev, FC_DBGTMDS0, 0x00);	/* B */
	}
}

void fc_force_output(hdmi_tx_dev_t *dev, int enable)
{
	fc_force_audio(dev, 0);
	fc_force_video(dev, (u8)enable);
}

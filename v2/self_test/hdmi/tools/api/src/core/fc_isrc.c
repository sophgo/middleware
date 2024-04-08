// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "frame_composer/frame_composer_reg.h"
#include "core/fc_isrc.h"
#include "bsp/access.h"
#include "util/log.h"

void fc_isrc_status(hdmi_tx_dev_t *dev, u8 code)
{
	LOG_TRACE1(code);
	dev_write_mask(dev, FC_ISCR1_0, FC_ISCR1_0_ISRC_STATUS_MASK, code);
}

void fc_isrc_valid(hdmi_tx_dev_t *dev, u8 validity)
{
	LOG_TRACE1(validity);
	dev_write_mask(dev, FC_ISCR1_0, FC_ISCR1_0_ISRC_VALID_MASK, (validity ? 1 : 0));
}

void fc_isrc_cont(hdmi_tx_dev_t *dev, u8 isContinued)
{
	LOG_TRACE1(isContinued);
	dev_write_mask(dev, FC_ISCR1_0, FC_ISCR1_0_ISRC_CONT_MASK, (isContinued ? 1 : 0));
}

void fc_isrc_isrc1_codes(hdmi_tx_dev_t *dev, u8 * codes, u8 length)
{
	u8 c = 0;
	LOG_TRACE1(codes[0]);
	if (length > (FC_ISCR1_1 - FC_ISCR1_16 + 1)) {
		length = (FC_ISCR1_1 - FC_ISCR1_16 + 1);
		LOGGER(SNPS_WARN,"ISCR1 Codes Truncated");
	}

	for (c = 0; c < length; c++)
		dev_write(dev, FC_ISCR1_1 - c, codes[c]);
}

void fc_isrc_isrc2_codes(hdmi_tx_dev_t *dev, u8 * codes, u8 length)
{
	u8 c = 0;
	LOG_TRACE1(codes[0]);
	if (length > (FC_ISCR2_0 - FC_ISCR2_15 + 1)) {
		length = (FC_ISCR2_0 - FC_ISCR2_15 + 1);
		LOGGER(SNPS_WARN,"ISCR2 Codes Truncated");
	}

	for (c = 0; c < length; c++)
		dev_write(dev, FC_ISCR2_0 - c, codes[c]);
}

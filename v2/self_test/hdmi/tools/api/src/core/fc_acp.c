// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "frame_composer/frame_composer_reg.h"
#include "core/fc_acp.h"
#include "bsp/access.h"
#include "util/log.h"


void fc_acp_type(hdmi_tx_dev_t *dev, u8 type)
{
	LOG_TRACE1(type);
	dev_write(dev, FC_ACP0, type);
}

void fc_acp_type_dependent_fields(hdmi_tx_dev_t *dev, u8 * fields, u8 fieldsLength)
{
	u8 c = 0;
	LOG_TRACE1(fields[0]);
	if (fieldsLength > (FC_ACP1 - FC_ACP16 + 1)) {
		fieldsLength = (FC_ACP1 - FC_ACP16 + 1);
		LOGGER(SNPS_WARN,"ACP Fields Truncated");
	}

	for (c = 0; c < fieldsLength; c++)
		dev_write(dev, FC_ACP1 - c, fields[c]);
}

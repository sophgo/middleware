// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "frame_composer/frame_composer_reg.h"
#include "core/fc_vsd.h"
#include "bsp/access.h"
#include "util/log.h"

void fc_vsd_vendor_OUI(hdmi_tx_dev_t *dev, u32 id)
{
	LOG_TRACE1(id);
	dev_write(dev, (FC_VSDIEEEID0), id);
	dev_write(dev, (FC_VSDIEEEID1), id >> 8);
	dev_write(dev, (FC_VSDIEEEID2), id >> 16);
}

u8 fc_vsd_vendor_payload(hdmi_tx_dev_t *dev, const u8 * data, unsigned short length)
{
	const unsigned short size = 24;
	unsigned i = 0;
	LOG_TRACE();
	if (data == 0) {
		LOGGER(SNPS_WARN,"invalid parameter");
		return 1;
	}
	if (length > size) {
		length = size;
		LOGGER(SNPS_WARN,"vendor payload truncated");
	}
	for (i = 0; i < length; i++) {
		dev_write(dev, (FC_VSDPAYLOAD0 + (i*4)), data[i]);
	}
	return 0;
}

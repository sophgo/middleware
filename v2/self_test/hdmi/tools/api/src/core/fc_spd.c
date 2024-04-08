// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "frame_composer/frame_composer_reg.h"
#include "core/fc_spd.h"
#include "core/fc_packets.h"
#include "bsp/access.h"
#include "util/log.h"
#include "util/error.h"

// Source Product Descriptor

void fc_spd_VendorName(hdmi_tx_dev_t *dev, const u8 * data, unsigned short length)
{
	unsigned short i = 0;
	LOG_TRACE();
	for (i = 0; i < length; i++) {
		dev_write(dev, FC_SPDVENDORNAME0 + (i*4), data[i]);
	}
}

void fc_spd_ProductName(hdmi_tx_dev_t *dev, const u8 * data, unsigned short length)
{
	unsigned short i = 0;
	LOG_TRACE();
	for (i = 0; i < length; i++) {
		dev_write(dev, FC_SPDPRODUCTNAME0 + (i*4), data[i]);
	}
}

void fc_spd_SourceDeviceInfo(hdmi_tx_dev_t *dev, u8 code)
{
	LOG_TRACE1(code);
	dev_write(dev, FC_SPDDEVICEINF, code);
}

int fc_spd_config(hdmi_tx_dev_t *dev, fc_spd_info_t *spd_data)
{
	const unsigned short pSize = 8;
	const unsigned short vSize = 16;

	LOG_TRACE();

	if(spd_data == NULL){
		LOGGER(SNPS_ERROR, "Improper argument: spd_data");
		return FALSE;
	}

	fc_packets_AutoSend(dev, 0, SPD_TX);	/* prevent sending half the info. */

	if (spd_data->vName == 0) {
		error_set(ERR_INVALID_PARAM_VENDOR_NAME);
		LOGGER(SNPS_ERROR,"invalid parameter");
		return FALSE;
	}
	if (spd_data->vLength > vSize) {
		spd_data->vLength = vSize;
		LOGGER(SNPS_WARN,"vendor name truncated");
	}
	if (spd_data->pName == 0) {
		error_set(ERR_INVALID_PARAM_PRODUCT_NAME);
		LOGGER(SNPS_ERROR,"invalid parameter");
		return FALSE;
	}
	if (spd_data->pLength > pSize) {
		spd_data->pLength = pSize;
		LOGGER(SNPS_WARN,"product name truncated");
	}

	fc_spd_VendorName(dev, spd_data->vName, spd_data->vLength);
	fc_spd_ProductName(dev, spd_data->pName, spd_data->pLength);

	fc_spd_SourceDeviceInfo(dev, spd_data->code);

	if (spd_data->autoSend) {
		fc_packets_AutoSend(dev, spd_data->autoSend, SPD_TX);
	} else {
		fc_packets_ManualSend(dev, SPD_TX);
	}

	return TRUE;
}

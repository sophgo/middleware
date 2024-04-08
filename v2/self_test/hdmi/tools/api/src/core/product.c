// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "core/product.h"
#include "util/log.h"

void product_reset(hdmi_tx_dev_t *dev, productParams_t * product)
{
	unsigned i = 0;
	product->mVendorNameLength = 0;
	product->mProductNameLength = 0;
	product->mSourceType = (u8) (-1);
	product->mOUI = (u8) (-1);
	product->mVendorPayloadLength = 0;
	for (i = 0; i < sizeof(product->mVendorName); i++) {
		product->mVendorName[i] = 0;
	}
	for (i = 0; i < sizeof(product->mProductName); i++) {
		product->mProductName[i] = 0;
	}
	for (i = 0; i < sizeof(product->mVendorPayload); i++) {
		product->mVendorPayload[i] = 0;
	}
}

u8 product_SetProductName(hdmi_tx_dev_t *dev, productParams_t * product, const u8 * data, u8 length)
{
	u16 i = 0;
	if ((data == 0) || length > sizeof(product->mProductName)) {
		LOGGER(SNPS_ERROR,"invalid parameter");
		return 1;
	}
	product->mProductNameLength = 0;
	for (i = 0; i < sizeof(product->mProductName); i++) {
		product->mProductName[i] = (i < length) ? data[i] : 0;
	}
	product->mProductNameLength = length;
	return 0;
}

u8 product_SetVendorName(hdmi_tx_dev_t *dev, productParams_t * product, const u8 * data, u8 length)
{
	u16 i = 0;
	if (data == 0 || length > sizeof(product->mVendorName)) {
		LOGGER(SNPS_ERROR,"invalid parameter");
		return 1;
	}
	product->mVendorNameLength = 0;
	for (i = 0; i < sizeof(product->mVendorName); i++) {
		product->mVendorName[i] = (i < length) ? data[i] : 0;
	}
	product->mVendorNameLength = length;
	return 0;
}

u8 product_SetVendorPayload(hdmi_tx_dev_t *dev, productParams_t * product, const u8 * data, u8 length)
{
	u16 i = 0;
	if (data == 0 || length > sizeof(product->mVendorPayload)) {
		LOGGER(SNPS_ERROR,"invalid parameter");
		return 1;
	}
	product->mVendorPayloadLength = 0;
	for (i = 0; i < sizeof(product->mVendorPayload); i++) {
		product->mVendorPayload[i] = (i < length) ? data[i] : 0;
	}
	product->mVendorPayloadLength = length;
	return 0;
}

u8 product_IsSourceProductValid(hdmi_tx_dev_t *dev, productParams_t * product)
{
	return (product->mSourceType != (u8)(-1)) && (product->mVendorNameLength != 0) && (product->mProductNameLength != 0);
}

u8 product_IsVendorSpecificValid(hdmi_tx_dev_t *dev, productParams_t * product)
{
	return (product->mOUI != (u32)(-1)) && (product->mVendorPayloadLength != 0);
}

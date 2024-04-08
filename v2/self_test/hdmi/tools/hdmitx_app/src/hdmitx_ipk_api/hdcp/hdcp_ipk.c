// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "hdcp_ipk.h"
#include "hdmi_tx_system_parameters.h"


void hdcp_write(struct hdmi_tx_app *app, uint32_t data, uint32_t reg){
	int ret = 0;
	fb_ioctl_data write_data;
	write_data.address = reg + PROTO_HDMI_TX_APB_HDCP_ADDRESS_START;
	write_data.value = data;

	if(app->hdmi_tx_driver < 0)
		return;

	LOGGER(SNPS_TRACE, "%s:addr 0x%x - value 0x%x", __func__,
			write_data.address, write_data.value);

	ret = ioctl(app->hdmi_tx_driver, FB_HDMI_CORE_WRITE, &write_data);
	if(ret < 0){
		if(app->verbose)
			LOGGER(SNPS_ERROR, "%s:IOCTL error [%d]\n", __func__, ret);
	}
}

uint32_t hdcp_read(struct hdmi_tx_app *app, uint32_t reg){
	int ret = 0;
	fb_ioctl_data read_data;
	read_data.address = reg + PROTO_HDMI_TX_APB_HDCP_ADDRESS_START;

	if(app->hdmi_tx_driver < 0)
		return 0;

	ret = ioctl(app->hdmi_tx_driver, FB_HDMI_CORE_READ, &read_data);
	if(ret < 0){
		if(app->verbose)
			LOGGER(SNPS_ERROR, "%s:IOCTL error [%d]\n", __func__, ret);
	}

	LOGGER(SNPS_TRACE, "%s:addr 0x%x - value 0x%x", __func__,
			read_data.address, read_data.value);

	return read_data.value;
}

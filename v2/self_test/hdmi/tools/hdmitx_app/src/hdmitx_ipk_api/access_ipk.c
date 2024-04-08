// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "hdmitx_ipk_api/access_ipk.h"
#include "platform.h"

/**
 * Access Functions
 */
int hdmitx_initialize(void){
	struct hdmi_tx_app *p_app = get_platform();

	if(p_app->verbose){
		printf("Opening device: %s", p_app->hdmi_tx_name);
	}

	return TRUE;
}

int hdmitx_disable(void){

	return 0;
}
void hdmitx_write(u32 addr, u32 data){
	struct hdmi_tx_app *p_app = get_platform();
	int ret = 0;
	fb_ioctl_data write_data;
	write_data.address = addr;
	write_data.value = data;

	if(p_app->hdmi_tx_driver < 0)
		return;

	ret = ioctl(p_app->hdmi_tx_driver, FB_HDMI_CORE_WRITE, &write_data);
	if(ret < 0){
		if(p_app->verbose)
			LOGGER(SNPS_ERROR, "%s:IOCTL error [%d]\n", __func__, ret);
	}
}

u32 hdmitx_read(u32 addr){
	struct hdmi_tx_app *p_app = get_platform();
	int ret = 0;
	fb_ioctl_data read_data;
	read_data.address = addr;

	if(p_app->hdmi_tx_driver < 0)
		return 0;

	ret = ioctl(p_app->hdmi_tx_driver, FB_HDMI_CORE_READ, &read_data);
	if(ret < 0){
		if(p_app->verbose)
			LOGGER(SNPS_ERROR, "%s:IOCTL error [%d]\n", __func__, ret);
	}

	return read_data.value;
}

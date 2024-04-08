// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "phy_interface.h"
#include "phy_interface_regs.h"
#include "hdmi_tx_system_parameters.h"
#include "platform.h"
#include "reset_mng.h"


void phy_if_write(struct hdmi_tx_app *app, uint32_t reg, uint32_t data){
	int ret = 0;
	fb_ioctl_data write_data;
	write_data.address = reg + PROTO_HDMI_TX_APB_TXPHY_IF_ADDRESS_START;
	write_data.value = data;

	if(app->hdmi_tx_driver < 0)
		return;

	LOGGER(SNPS_TRACE, "%s:addr 0x%x - value 0x%x", __func__,
			write_data.address, write_data.value);

	ret = ioctl(app->hdmi_tx_driver, FB_HDMI_CORE_WRITE, &write_data);
	if(ret < 0){
		if(app->verbose)
			LOGGER(SNPS_ERROR, "%s:IOCTL error [%d]", __func__, ret);
	}
}

uint32_t phy_if_read(struct hdmi_tx_app *app, uint32_t reg){
	int ret = 0;
	fb_ioctl_data read_data;
	read_data.address = reg + PROTO_HDMI_TX_APB_TXPHY_IF_ADDRESS_START;

	if(app->hdmi_tx_driver < 0)
		return 0;

	ret = ioctl(app->hdmi_tx_driver, FB_HDMI_CORE_READ, &read_data);
	if(ret < 0){
		if(app->verbose)
			LOGGER(SNPS_ERROR, "%s:IOCTL error [%d]", __func__, ret);
	}

	LOGGER(SNPS_TRACE, "%s:addr 0x%x - value 0x%x", __func__,
			read_data.address, read_data.value);

	return read_data.value;
}

int phy_data_output_delay(uint32_t lines,  uint32_t delay)
{
	uint32_t value = 0;
	struct hdmi_tx_app * p_app = get_platform();

	phy_if_write(p_app, ODLYCFG, 0); //ensure dly_value_set is disable for data lines

	phy_if_write(p_app, ODLYSEL, lines); //select all data line odelays to be configured
	phy_if_write(p_app, ODLYCNTINVAL, delay); //delay value to be configured
	phy_if_write(p_app, ODLYCFG, 1); //apply delay value to data lines
	phy_if_write(p_app, ODLYCFG, 0); //ensure dly_value_set is disable for data lines
	value = phy_if_read(p_app, ODLYCNTOUTVAL); //check if $value was correcly set

	if(value != delay){
		LOGGER(SNPS_ERROR, "%s:Data output delay configuration failed, configured %d returned %d", __func__, delay, value);
		return -1;
	}

	value = rst_mng_read(p_app, 0x0C); //TXPHYXFACE_RESET_NEG
	value &= ~(1 << 0);//TXPHY_DLYCTRL_BIT
	rst_mng_write(p_app, 0x0C, value);

	value |= (1 << 0);//TXPHY_DLYCTRL_BIT
	rst_mng_write(p_app, 0x0C, value);


	return 0;
}

uint32_t phy_data_output_delay_get()
{
	uint32_t value = 0;
	struct hdmi_tx_app * p_app = get_platform();

	//TODO: This value is depending of the last configuration and in case of different configuration per line the returned value can be incorrect
	//	Align with the HW the best way of doing this. (Could be not possible with the current HW implementation)
	value = phy_if_read(p_app, ODLYCNTOUTVAL); //check if $value was correcly set

	return value;
}


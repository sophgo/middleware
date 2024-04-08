// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "bsp/access.h"

#include "../../include/hdmitx_dev.h"
#include "util/log.h"
#include "util/error.h"
#include "util/util.h"

static struct device_access * device_bsp;

void register_bsp_functions(struct device_access * device)
{
	LOGGER(SNPS_INFO,"Device registered %s", device->name);
	device_bsp = device;
}


int dev_initialize(hdmi_tx_dev_t * dev)
{
	if(device_bsp)
		return device_bsp->initialize();

	LOGGER(SNPS_ERROR,"BSP functions not registered");
	return -1;
}

int dev_standby(hdmi_tx_dev_t * dev)
{
	if(device_bsp)
		return device_bsp->disable();

	LOGGER(SNPS_ERROR,"BSP functions not registered");
	return -1;
}

u32 dev_read(hdmi_tx_dev_t * dev, u32 addr)
{
	if(dev && device_bsp)
		return device_bsp->read(addr);
	LOGGER(SNPS_ERROR,"BSP functions not registered");
	return 0;
}


u32 dev_read_mask(hdmi_tx_dev_t * dev, u32 addr, u32 mask)
{
	u32 shift = first_bit_set(mask);
	return ((dev_read(dev, addr) & mask) >> shift);
}

void dev_write(hdmi_tx_dev_t * dev, u32 addr, u32 data)
{
	if(dev && device_bsp)
		return device_bsp->write(addr, data);
	
	LOGGER(SNPS_ERROR,"BSP functions not registered");
}

void dev_write_mask(hdmi_tx_dev_t * dev, u32 addr, u32 mask, u32 data)
{
	u32 temp = 0;
	u32 shift = first_bit_set(mask);

	temp = dev_read(dev, addr);
	temp &= ~(mask);
	temp |= (mask & data << shift);
	dev_write(dev, addr, temp);
}


int access_Initialize(hdmi_tx_dev_t * dev)
{
	return dev_initialize(dev);
}

int access_Standby(hdmi_tx_dev_t * dev)
{
	return dev_standby(dev);
}


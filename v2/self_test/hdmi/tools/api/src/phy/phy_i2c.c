// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "phy/phy_reg.h"
#include "phy/phy_i2c.h"
#include "bsp/access.h"
#include "util/log.h"
#include "interrupt/interrupt_reg.h"
#include "phy/phy.h"

#define I2C_TIMEOUT				100

void phy_i2c_fast_mode(hdmi_tx_dev_t *dev, u8 bit)
{
	dev_write_mask(dev, PHY_I2CM_DIV, PHY_I2CM_DIV_FAST_STD_MODE_MASK, bit);
}

void phy_i2c_master_reset(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	dev_write_mask(dev, PHY_I2CM_SOFTRSTZ, PHY_I2CM_SOFTRSTZ_I2C_SOFTRSTZ_MASK, 1);
}

void phy_i2c_mask_interrupts(hdmi_tx_dev_t *dev, int mask)
{
	LOG_TRACE1(mask);
	dev_write_mask(dev, PHY_I2CM_INT, PHY_I2CM_INT_DONE_MASK_MASK, mask ? 1 : 0);
	dev_write_mask(dev, PHY_I2CM_CTLINT, PHY_I2CM_CTLINT_ARBITRATION_MASK_MASK, mask ? 1 : 0);
	dev_write_mask(dev, PHY_I2CM_CTLINT, PHY_I2CM_CTLINT_NACK_MASK_MASK, mask ? 1 : 0);
}

void phy_i2c_slave_address(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	dev_write_mask(dev, PHY_I2CM_SLAVE, PHY_I2CM_SLAVE_SLAVEADDR_MASK, value);
}

int phy_i2c_write(hdmi_tx_dev_t *dev, u8 addr, u16 data)
{
	int timeout = PHY_TIMEOUT;
	u32 status  = 0;

	LOG_TRACE2(data, addr);

	//Set address
	dev_write(dev, PHY_I2CM_ADDRESS, addr);

	//Set value
	dev_write(dev,PHY_I2CM_DATAO_1, (u8) ((data >> 8) & 0xFF));
	dev_write(dev,PHY_I2CM_DATAO_0, (u8) (data & 0xFF));

	dev_write(dev, PHY_I2CM_OPERATION, PHY_I2CM_OPERATION_WR_MASK);

	do {
		snps_sleep(10);
		status = dev_read_mask(dev, IH_I2CMPHY_STAT0, IH_I2CMPHY_STAT0_I2CMPHYERROR_MASK |
							     IH_I2CMPHY_STAT0_I2CMPHYDONE_MASK);
	} while (status == 0 && (timeout--));

	dev_write(dev, IH_I2CMPHY_STAT0, status); //clear read status

	if(status & IH_I2CMPHY_STAT0_I2CMPHYERROR_MASK){
		LOGGER(SNPS_INFO, "%s: I2C PHY write failed",__func__);
		return -1;
	}

	if(status & IH_I2CMPHY_STAT0_I2CMPHYDONE_MASK){
		return 0;
	}

	LOGGER(SNPS_ERROR, "%s: ASSERT I2C Write timeout - check PHY - exiting",__func__);
	exit(-1);
}

int phy_i2c_read(hdmi_tx_dev_t *dev, u8 addr, u16 * value)
{
	int timeout = PHY_TIMEOUT;
	u32 status  = 0;

	//Set address
	dev_write(dev, PHY_I2CM_ADDRESS, addr);

	dev_write(dev, PHY_I2CM_OPERATION, PHY_I2CM_OPERATION_RD_MASK);

	do {
		snps_sleep(10);
		status = dev_read_mask(dev, IH_I2CMPHY_STAT0, IH_I2CMPHY_STAT0_I2CMPHYERROR_MASK |
													  IH_I2CMPHY_STAT0_I2CMPHYDONE_MASK);
	} while (status == 0 && (timeout--));

	dev_write(dev, IH_I2CMPHY_STAT0, status); //clear read status

	if(status & IH_I2CMPHY_STAT0_I2CMPHYERROR_MASK){
		LOGGER(SNPS_INFO, "%s: I2C Read failed",__func__);
		return -1;
	}

	if(status & IH_I2CMPHY_STAT0_I2CMPHYDONE_MASK){

		*value = ((u16) (dev_read(dev, (PHY_I2CM_DATAI_1)) << 8)
				| dev_read(dev, (PHY_I2CM_DATAI_0)));
		return 0;
	}

	LOGGER(SNPS_ERROR, "%s: ASSERT I2C Read timeout - check PHY - exiting",__func__);
	exit(-1);
}

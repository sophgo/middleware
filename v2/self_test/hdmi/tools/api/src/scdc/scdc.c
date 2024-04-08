// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "bsp/eddc_reg.h"
#include "util/util.h"
#include "util/log.h"
#include "util/error.h"
#include "scdc/scdc.h"
#include "bsp/i2cm.h"
#include "bsp/access.h"
#include "core/irq.h"

int scdc_read(hdmi_tx_dev_t *dev, u8 address, u8 size, u8 * data)
{
	if(ddc_read(dev, SCDC_SLAVE_ADDRESS, 0,0 , address, size, data)){
		LOGGER(SNPS_ERROR, "%s: SCDC addr 0x%x read failed ",__func__, address);
		return -1;
	}
	return 0;
}

int scdc_write(hdmi_tx_dev_t *dev, u8 address, u8 size, u8 * data)
{
	if(ddc_write(dev, SCDC_SLAVE_ADDRESS, address, size, data)){
		LOGGER(SNPS_ERROR, "%s: SCDC addr 0x%x write failed ",__func__, address);
		return -1;
	}
	return 0;
}

void scdc_enable_rr(hdmi_tx_dev_t *dev, u8 enable)
{

	if (enable == 1) {
		/* Enable Readrequest from the Tx controller */
		dev_write_mask(dev, I2CM_SCDC_READ_UPDATE, I2CM_SCDC_READ_UPDATE_READ_REQUEST_EN_MASK, 1);
		scdc_set_rr_flag(dev, 0x01);
		irq_scdc_read_request(dev, TRUE);
	}
	if (enable == 0) {
		/* Disable ReadRequest on Tx controller */
		irq_scdc_read_request(dev, FALSE);
		scdc_set_rr_flag(dev, 0x00);
		dev_write_mask(dev, I2CM_SCDC_READ_UPDATE, I2CM_SCDC_READ_UPDATE_READ_REQUEST_EN_MASK, 0);
	}
}

int scdc_scrambling_status(hdmi_tx_dev_t *dev)
{
	u8 read_value = 0;
	if(scdc_read(dev, SCDC_SCRAMBLER_STAT, 1, &read_value)){
		LOGGER(SNPS_ERROR, "%s: SCDC addr 0x%x read failed ",__func__, SCDC_SINK_VER);
		return 0;
	}
	return (read_value & 0x01);
}

int scdc_scrambling_enable_flag(hdmi_tx_dev_t *dev, u8 enable)
{
	u8 read_value = 0;
	if(scdc_read(dev, SCDC_TMDS_CONFIG, 1 , &read_value)){
		LOGGER(SNPS_ERROR, "%s: SCDC addr 0x%x read failed ",__func__, SCDC_TMDS_CONFIG);
		return -1;
	}
	read_value = set(read_value, 0x1, enable ? 0x1 : 0x0);
	if(scdc_write(dev, SCDC_TMDS_CONFIG, 1, &read_value)){
		LOGGER(SNPS_ERROR, "%s: SCDC addr 0x%x write failed ",__func__, SCDC_TMDS_CONFIG);
		return -1;
	}
	return 0;
}

void scdc_set_rr_flag(hdmi_tx_dev_t *dev, u8 enable)
{

	if(ddc_write(dev, SCDC_SLAVE_ADDRESS, SCDC_CONFIG_0, 1, &enable)){
		LOGGER(SNPS_ERROR, "%s: SCDC addr 0x%x - 0x%x write failed ",__func__, SCDC_CONFIG_0, enable);
	}
}

int scdc_get_rr_flag(hdmi_tx_dev_t *dev, u8 * flag)
{
	if(ddc_read(dev, SCDC_SLAVE_ADDRESS, 0, 0 , SCDC_CONFIG_0, 1, flag)){
		LOGGER(SNPS_ERROR, "%s: SCDC addr 0x%x read failed ",__func__, SCDC_CONFIG_0);
		return -1;
	}
	return 0;
}

void scdc_test_rr(hdmi_tx_dev_t *dev, u8 test_rr_delay)
{
	scdc_enable_rr(dev, 0x01);
	test_rr_delay = set(test_rr_delay, 0x80, 1);
	scdc_write(dev, SCDC_TEST_CFG_0, 1, &test_rr_delay);
}

int scdc_test_rr_update_flag(hdmi_tx_dev_t *dev)
{
	u8 read_value = 0;
	if(scdc_read(dev, SCDC_UPDATE_0, 1, &read_value)){
		LOGGER(SNPS_ERROR, "%s: SCDC addr 0x%x read failed ",__func__, SCDC_UPDATE_0);
		return 0;
	}
	return read_value;
}

int scdc_read_cnt(hdmi_tx_dev_t *dev, u8 *data)
{
	if(ddc_read(dev, SCDC_SLAVE_ADDRESS, 0, 0, SCDC_ERR_DET_0_L, 8, data)){
		LOGGER(SNPS_ERROR, "%s: SCDC addr 0x%x read failed ",__func__, SCDC_ERR_DET_0_L);
		return -1;
	}

	return 0;
}


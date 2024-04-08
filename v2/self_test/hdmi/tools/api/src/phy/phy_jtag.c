// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "bsp/i2cm.h"
#include "util/log.h"
#include "util/util.h"
#include "bsp/access.h"
#include "phy/phy_reg.h"
#include "phy_jtag.h"

void _send_data_pulse(hdmi_tx_dev_t * dev, u8 tms, u8 tdi)
{
	u8 in_value = 0;
	in_value = set(in_value, JTAG_PHY_TAP_IN_JTAG_TMS_MASK, tms);
	in_value = set(in_value, JTAG_PHY_TAP_IN_JTAG_TDI_MASK, tdi);

	dev_write(dev, JTAG_PHY_TAP_TCK, (u8) 0);
	snps_sleep(100);
	dev_write(dev, JTAG_PHY_TAP_IN, in_value);
	snps_sleep(100);
	dev_write(dev, JTAG_PHY_TAP_TCK, (u8) 1);
	snps_sleep(100);
}

void _tap_soft_reset(hdmi_tx_dev_t * dev)
{
	int i;
	for(i = 0; i < 5; i++){
		_send_data_pulse(dev, 1, 0);
	}
	_send_data_pulse(dev, 0, 0);
}

void _tap_goto_shift_dr(hdmi_tx_dev_t * dev)
{
	//RTI -> Select-DR
	_send_data_pulse(dev, 1, 0);
	// Select-DR-Scan -> Capture-DR -> Shift-DR
	_send_data_pulse(dev, 0, 0);
	_send_data_pulse(dev, 0, 0);
}

void _tap_goto_shift_ir(hdmi_tx_dev_t * dev)
{
	// RTI->Sel IR_Scan
	_send_data_pulse(dev, 1, 0);
	_send_data_pulse(dev, 1, 0);
	// Select-IR-Scan -> Shift_IR
	_send_data_pulse(dev, 0, 0);
	_send_data_pulse(dev, 0, 0);
}


void _tap_goto_run_test_idle(hdmi_tx_dev_t * dev)
{
	// Exit1_DR -> Update_DR
	_send_data_pulse(dev, 1, 0);

	// Update_DR -> Run_Test_Idle
	_send_data_pulse(dev, 0, 0);
}

void _send_value_shift_ir(hdmi_tx_dev_t * dev, u8 jtag_addr)
{
	int i;
	for(i = 0; i < 7; i++){
		_send_data_pulse(dev, 0, jtag_addr & 0x01);
		jtag_addr = jtag_addr >> 1;
	}
	//Shift_IR -> Exit_IR w/ last MSB bit
	_send_data_pulse(dev, 1, jtag_addr & 0x01);
}

u16 _send_value_shift_dr(hdmi_tx_dev_t * dev, u8 cmd, u16 data_in)
{
	int i;
	u32 aux_in = (cmd << 16) | data_in;
	u16 data_out = 0;
	// Shift_DR
	for(i = 0; i < 16; i++){
		_send_data_pulse(dev, 0, aux_in);
		data_out |= (dev_read(dev, JTAG_PHY_TAP_OUT) & 0x01) << i;
		aux_in = aux_in >> 1;
	}
	// Shift_DR, TAP command bit
	_send_data_pulse(dev, 0, aux_in);
	aux_in = aux_in >> 1;

	// Shift_DR -> Exit_DR w/ MSB TAP command bit
	i++;
	_send_data_pulse(dev, 1, aux_in);
	data_out |= (dev_read(dev, JTAG_PHY_TAP_OUT) & 0x01) << i;

	return data_out;
}


int phy_jtag_init(hdmi_tx_dev_t * dev, u8 jtag_addr)
{
	dev_write(dev, JTAG_PHY_ADDR, jtag_addr);
	phy_jtag_reset(dev);
	_tap_soft_reset(dev);
	phy_jtag_slave_address(dev, jtag_addr);

	return 1;
}

void phy_jtag_slave_address(hdmi_tx_dev_t * dev, u8 jtag_addr)
{
	_tap_goto_shift_ir(dev);

	// Shift-IR - write jtag slave address
	_send_value_shift_ir(dev, jtag_addr);

	_tap_goto_run_test_idle(dev);

}

int phy_jtag_read(hdmi_tx_dev_t * dev, u16 addr,  u16 * pvalue)
{
	_tap_goto_shift_dr(dev);

	// Shift-DR (shift 16 times) and -> Exit1 -DR
	_send_value_shift_dr(dev, JTAG_TAP_ADDR_CMD, addr << 8);

	_tap_goto_run_test_idle(dev);
	_tap_goto_shift_dr(dev);

	*pvalue = _send_value_shift_dr(dev, JTAG_TAP_READ_CMD, 0xFFFF);

	_tap_goto_run_test_idle(dev);

	return 0;
}

int phy_jtag_write(hdmi_tx_dev_t * dev, u16 addr,  u16 value)
{
	_tap_goto_shift_dr(dev);

	// Shift-DR (shift 16 times) and -> Exit1 -DR
	_send_value_shift_dr(dev, JTAG_TAP_ADDR_CMD, addr << 8);

	_tap_goto_run_test_idle(dev);
	_tap_goto_shift_dr(dev);

	_send_value_shift_dr(dev, JTAG_TAP_WRITE_CMD, value);
	_tap_goto_run_test_idle(dev);

	return 0;
}

void phy_jtag_reset(hdmi_tx_dev_t * dev)
{
	dev_write(dev, JTAG_PHY_TAP_IN, 0x10);
	snps_sleep(100);
	dev_write(dev, JTAG_PHY_CONFIG, 0);
	snps_sleep(100);
	dev_write(dev, JTAG_PHY_CONFIG, 1); //enable interface to JTAG
	_send_data_pulse(dev, 0, 0);
}


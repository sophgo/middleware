// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "phy/phy.h"
#include "phy/phy_i2c.h"

#include "core/main_controller.h"
#include "util/log.h"
#include "util/error.h"
#include "bsp/board.h"

#include "phy_reg.h"
#include "bsp/access.h"
#include "util/general_ops.h"

#include "interrupt/interrupt_reg.h"
#include "system.h"
#include "phy_jtag.h"
#include "phy_301.h"
#include "phy_302.h"
#include "phy_303.h"
#include "phy_305.h"
#include "phy_308.h"
#include "phy_311.h"
#include "phy_312.h"
#include "phy_316.h"

#ifdef PHY_THIRD_PARTY
#include "phy/phy_ack.h"
#endif

/*************************************************************
 * Internal functions
 *
 */

void _power_down(hdmi_tx_dev_t *dev, u8 bit)
{
	//TODO: Correct register mask - extract the information from IP-XACT
	LOG_TRACE1(bit);
	dev_write_mask(dev, PHY_CONF0, PHY_CONF0_SPARES_2_MASK, (bit ? 1 : 0));
}

void _enable_tmds(hdmi_tx_dev_t *dev, u8 bit)
{
	//TODO: Correct register mask - extract the information from IP-XACT
	LOG_TRACE1(bit);
	dev_write_mask(dev, PHY_CONF0, PHY_CONF0_SPARES_1_MASK, (bit ? 1 : 0));
}

void _set_pddq(hdmi_tx_dev_t *dev, u8 bit)
{
	LOG_TRACE1(bit);
	dev_write_mask(dev, PHY_CONF0, PHY_CONF0_PDDQ_MASK, (bit ? 1 : 0));
}

void _tx_power_on(hdmi_tx_dev_t *dev, u8 bit)
{
	LOG_TRACE1(bit);
	dev_write_mask(dev, PHY_CONF0, PHY_CONF0_TXPWRON_MASK, (bit ? 1 : 0));
}

void phy_enable_hpd_sense(hdmi_tx_dev_t *dev, u8 bit)
{
	LOG_TRACE1(bit);
	dev_write_mask(dev, PHY_CONF0, PHY_CONF0_ENHPDRXSENSE_MASK, (bit ? 1 : 0));
}

void _data_enable_polarity(hdmi_tx_dev_t *dev, u8 bit)
{
	LOG_TRACE1(bit);
	dev_write_mask(dev, PHY_CONF0, PHY_CONF0_SELDATAENPOL_MASK, (bit ? 1 : 0));
}

void _interface_control(hdmi_tx_dev_t *dev, u8 bit)
{
	LOG_TRACE1(bit);
	dev_write_mask(dev, PHY_CONF0, PHY_CONF0_SELDIPIF_MASK, (bit ? 1 : 0));
}

void _test_clear(hdmi_tx_dev_t *dev, u8 bit)
{
	//TODO: removed register?
	LOG_TRACE1(bit);
	dev_write_mask(dev, PHY_TST0, PHY_TST0_SPARE_4_MASK, (bit ? 1 : 0));
}

void _test_enable(hdmi_tx_dev_t *dev, u8 bit)
{
	//TODO: removed register?
	LOG_TRACE1(bit);
	dev_write_mask(dev, PHY_TST0, PHY_TST0_SPARE_3_MASK, (bit ? 1 : 0));
}

void _test_clock(hdmi_tx_dev_t *dev, u8 bit)
{
	//TODO: removed register?
	LOG_TRACE1(bit);
	dev_write_mask(dev, PHY_TST0, PHY_TST0_SPARE_0_MASK, (bit ? 1 : 0));
}

void _test_data_in(hdmi_tx_dev_t *dev, u8 data)
{
	LOG_TRACE1(data);
	dev_write(dev, (PHY_TST1), data);
}

u8 _test_data_out(hdmi_tx_dev_t *dev, u32 baseAddr)
{
	LOG_TRACE();
	return dev_read(dev, PHY_TST2);
}

u8 _interrupt_state(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	return dev_read(dev,PHY_INT0);
}

u8 _interrupt_mask_status(hdmi_tx_dev_t *dev, u8 mask)
{
	LOG_TRACE1(mask);
	return dev_read(dev, PHY_MASK0) & mask;
}

void _interrupt_polarity(hdmi_tx_dev_t *dev, u8 bitShift, u8 value)
{
	LOG_TRACE2(bitShift, value);
	dev_write_mask(dev, PHY_POL0, (1 << bitShift), value);
}

u8 _interrupt_polarity_status(hdmi_tx_dev_t *dev, u8 mask)
{
	LOG_TRACE1(mask);
	return dev_read(dev, PHY_POL0) & mask;
}


/*************************************************************
 * External functions
 *************************************************************/

int phy_write(hdmi_tx_dev_t *dev, u16 addr, u32 data)
{
	switch(dev->snps_hdmi_ctrl.phy_access){
		case PHY_JTAG:
			return phy_jtag_write(dev, addr, (u16) data);
		case PHY_I2C:
			return phy_i2c_write(dev, addr, (u16) data);
#ifdef PHY_THIRD_PARTY
		case PHY_EXTERN:
			phyack_write(dev, addr, data);
			return 0;
#endif
		default:
			LOGGER(SNPS_ERROR, "%s:PHY interface not defined", __func__);
	}
	return -1;
}

int phy_read(hdmi_tx_dev_t *dev, u16 addr, u32 * value)
{
	switch(dev->snps_hdmi_ctrl.phy_access){
		case PHY_JTAG:
			return phy_jtag_read(dev, addr, (u16 *)value);
		case PHY_I2C:
			return phy_i2c_read(dev, addr, (u16 *)value);
#ifdef PHY_THIRD_PARTY
		case PHY_EXTERN:
			*value = phyack_read(dev, addr);
			return 0;
#endif
		default:
			LOGGER(SNPS_ERROR,"%s:PHY interface not defined", __func__);
	}
	return -1;
}

char * _get_interface(phy_access_t interface)
{
	switch (interface) {
		case PHY_JTAG: return "JTAG";
		case PHY_I2C:  return "I2C";
		case PHY_EXTERN:return "External";
		default: ;
	}
	return "Undefined";
}

int phy_set_interface(hdmi_tx_dev_t *dev, phy_access_t interface)
{
	//TODO: should be implemented a lock where to prevent PHY access during this configuration
	if(dev->snps_hdmi_ctrl.phy_access == interface){
		LOGGER(SNPS_INFO, "Phy interface already set to %s", _get_interface(interface));
		return 0;
	}

	switch (interface) {
		case PHY_JTAG:
			dev->snps_hdmi_ctrl.phy_access = interface;
			phy_jtag_init(dev,0xD4);
			break;
		case PHY_I2C:
			dev->snps_hdmi_ctrl.phy_access = interface;
			dev_write(dev, JTAG_PHY_CONFIG, JTAG_PHY_CONFIG_I2C_JTAGZ_MASK);
			phy_slave_address(dev,PHY_I2C_SLAVE_ADDR);
			break;

		case PHY_EXTERN:
#ifdef PHY_THIRD_PARTY
			if (dev->ack_phy.status == 0){
				LOGGER(SNPS_ERROR,"%s:PHY interface only supported for external PHYs", __func__);
				return -1;
			}
#else
			LOGGER(SNPS_INFO,"Third Party PHY support not available");

#endif
			break;
		default:
			LOGGER(SNPS_ERROR,"%s:PHY interface not defined", __func__);
			return -1;
	}
	//dev->snps_hdmi_ctrl.phy_access = interface;
	LOGGER(SNPS_INFO, "PHY interface set to %s",_get_interface(interface));
	return 0;
}

int phy_reconfigure_interface(hdmi_tx_dev_t *dev)
{
	switch (dev->snps_hdmi_ctrl.phy_access) {
		case PHY_JTAG:
			phy_jtag_init(dev,0xD4);
			break;
		case PHY_I2C:
			dev_write(dev, JTAG_PHY_CONFIG, JTAG_PHY_CONFIG_I2C_JTAGZ_MASK);
			phy_slave_address(dev,PHY_I2C_SLAVE_ADDR);
			break;
		default:
			LOGGER(SNPS_ERROR,"%s:PHY interface not defined", __func__);
			return -1;
	}
	LOGGER(SNPS_WARN, "PHY interface reconfiguration, set to %s", dev->snps_hdmi_ctrl.phy_access == PHY_I2C ? "I2C": "JTAG");
	return 0;

}

phy_access_t phy_get_interface(hdmi_tx_dev_t *dev)
{
	return dev->snps_hdmi_ctrl.phy_access;
}

int phy_slave_address(hdmi_tx_dev_t *dev, u8 value)
{
	switch(dev->snps_hdmi_ctrl.phy_access){
		case PHY_JTAG:
			phy_jtag_slave_address(dev, 0xD4);
			return 0;
		case PHY_I2C:
			phy_i2c_slave_address(dev, value);
			return 0;
		default:
			LOGGER(SNPS_ERROR,"%s:PHY interface not defined", __func__);
	}
	return -1;
}

int phy_preparation(hdmi_tx_dev_t *dev, u16 phy_model)
{
#ifdef PHY_THIRD_PARTY
	phy_set_interface(dev, PHY_EXTERN);
	return phyack_preparation(dev);
#endif
	return 0;
}


int phy_powerup(hdmi_tx_dev_t *dev, u16 phy_model)
{
#ifdef PHY_THIRD_PARTY
	int error = 0;

	LOGGER(SNPS_INFO,"Power Up ThirdParty ACK PHY");

	error = phyack_powerup(dev, dev->ack_phy.sus_clock, dev->ack_phy.pwclock);
	if(error) {
		LOGGER(SNPS_INFO,"************** Power Up ThirdParty ACK PHY failed ******************");
		return error;
	}

	//	error = phyack_swing_prog(dev, dev->ack_phy.voltage, dev->ack_phy.gain);
	//	if(error) {
	//		LOGGER(SNPS_INFO,"************** Swing program failed ******************");
	//		return error;
	//	}
#endif
	return 0;
}

char * phy_identification(hdmi_tx_dev_t *dev, u16 phy_model)
{
	switch (phy_model){
			case PHY_MODEL_108: return "GEN 2 GF 28SLP 1.8V build - E108";
			case PHY_MODEL_301: return "HDMI MHL TSMC 28HPM 1.8V build - E301";
			case PHY_MODEL_302: return "HDMI GLOBAL FOUNDRIES 28nm SLP 1.8V build - E302";
			case PHY_MODEL_303: return "HDMI MHL TSMC 28HPM 1.8V build - E303";
			case PHY_MODEL_305: return "HDMI MHL TSMC 28HPM 1.8V build - E305";
			case PHY_MODEL_308: return "HDMI SMIC 28nm PS 1.8V build - E308";
			case PHY_MODEL_311: return "HDMI TSMC 28nm HPCP 1.8V build - E311";
			case PHY_MODEL_312: return "HDMI TSMC 16nm FCC 1.8V build - E312";
			case PHY_MODEL_316: return "HDMI TSMC 12nm FCC 1.8V build - E316";
#ifdef PHY_THIRD_PARTY
			case PHY_MODEL_THIRD_PARTY_ACK: return "ThirdParty AntCreek";
			case PHY_MODEL_THIRD_PARTY_ANGELCREEK: return "ThirdParty AngelCreek";
#endif
	}
	return "Unknown";
}

int phy_initialize(hdmi_tx_dev_t *dev, u16 phy_model)
{
	LOG_TRACE1(dev->snps_hdmi_ctrl.data_enable_polarity);

	switch (phy_model) {
		case PHY_MODEL_108:
		case PHY_MODEL_301:
		case PHY_MODEL_302:
		case PHY_MODEL_303:
		case PHY_MODEL_305:
		case PHY_MODEL_308:
		case PHY_MODEL_311:
		case PHY_MODEL_312:
		case PHY_MODEL_316:
			board_ZcalReset(1);
			board_ZcalReset(0);

			if (dev->snps_hdmi_ctrl.phy_access != PHY_JTAG)
			{
				if(dev->snps_hdmi_ctrl.phy_access != PHY_I2C)
						if(phy_set_interface(dev, PHY_I2C) < 0) {
							return FALSE;
							break;
							}
			}


#ifdef PHY_THIRD_PARTY
		case PHY_MODEL_THIRD_PARTY_ACK:
		case PHY_MODEL_THIRD_PARTY_ANGELCREEK:

			if(phy_set_interface(dev, PHY_EXTERN) < 0) {
				return FALSE;
			}
			LOGGER(SNPS_INFO,"Third Party ThirdParty PHY");
#else
			LOGGER(SNPS_INFO,"Third Party PHY support not available");

#endif
			break;
		default:
			LOGGER(SNPS_WARN,"Unknown PHY");
	}


	_tx_power_on(dev, 0);
	_set_pddq(dev, 1);

	phy_interrupt_mask(dev, PHY_MASK0_TX_PHY_LOCK_MASK |
				PHY_MASK0_HPD_MASK |
				PHY_MASK0_RX_SENSE_0_MASK |
				PHY_MASK0_RX_SENSE_1_MASK |
				PHY_MASK0_RX_SENSE_2_MASK |
				PHY_MASK0_RX_SENSE_3_MASK);
	_data_enable_polarity(dev, dev->snps_hdmi_ctrl.data_enable_polarity);
	_interface_control(dev, 0);
	_enable_tmds(dev, 0);
	_power_down(dev, 0);	/* disable PHY */
	phy_i2c_mask_interrupts(dev, 0);

	// Clean IH_I2CMPHY_STAT0
	dev_write_mask(dev, IH_I2CMPHY_STAT0, IH_I2CMPHY_STAT0_I2CMPHYERROR_MASK | IH_I2CMPHY_STAT0_I2CMPHYDONE_MASK, 0);

	return TRUE;
}

int phy_configure(hdmi_tx_dev_t *dev, u16 phy_model)
{
	switch (phy_model) {
		case PHY_MODEL_301:
			return phy301_configure(dev, dev->snps_hdmi_ctrl.pixel_clock,
									 dev->snps_hdmi_ctrl.color_resolution,
									 dev->snps_hdmi_ctrl.pixel_repetition);
		case PHY_MODEL_302:
			return phy302_configure(dev, dev->snps_hdmi_ctrl.pixel_clock,
									 dev->snps_hdmi_ctrl.color_resolution,
									 dev->snps_hdmi_ctrl.pixel_repetition);
		case PHY_MODEL_303:
			return phy303_configure(dev, dev->snps_hdmi_ctrl.pixel_clock,
								 dev->snps_hdmi_ctrl.color_resolution,
								 dev->snps_hdmi_ctrl.pixel_repetition);
		case PHY_MODEL_305:
			return phy305_configure(dev, dev->snps_hdmi_ctrl.pixel_clock,
								 dev->snps_hdmi_ctrl.color_resolution,
								 dev->snps_hdmi_ctrl.pixel_repetition);
		case PHY_MODEL_308:
			return phy308_configure(dev, dev->snps_hdmi_ctrl.pixel_clock,
									 dev->snps_hdmi_ctrl.color_resolution,
									 dev->snps_hdmi_ctrl.pixel_repetition);
		case PHY_MODEL_311:
			return phy311_configure(dev, dev->snps_hdmi_ctrl.pixel_clock,
									 dev->snps_hdmi_ctrl.color_resolution,
									 dev->snps_hdmi_ctrl.pixel_repetition);
		case PHY_MODEL_312:
			return phy312_configure(dev, dev->snps_hdmi_ctrl.pixel_clock,
									 dev->snps_hdmi_ctrl.color_resolution,
									 dev->snps_hdmi_ctrl.pixel_repetition);
		case PHY_MODEL_316:
			return phy316_configure(dev, dev->snps_hdmi_ctrl.pixel_clock,
									 dev->snps_hdmi_ctrl.color_resolution,
									 dev->snps_hdmi_ctrl.pixel_repetition);
#ifdef PHY_THIRD_PARTY
		case PHY_MODEL_THIRD_PARTY_ACK:
		case PHY_MODEL_THIRD_PARTY_ANGELCREEK:
			return phyack_configure(dev, dev->ack_phy.refclock, dev->snps_hdmi_ctrl.pixel_clock);
#endif
		default:
		LOGGER(SNPS_ERROR,"****** PHY not supported %d *******\n", phy_model);
		return FALSE;
	}
}

int phy_standby(hdmi_tx_dev_t *dev)
{
	phy_interrupt_mask(dev, PHY_MASK0_TX_PHY_LOCK_MASK |
				PHY_MASK0_RX_SENSE_0_MASK |
				PHY_MASK0_RX_SENSE_1_MASK |
				PHY_MASK0_RX_SENSE_2_MASK |
				PHY_MASK0_RX_SENSE_3_MASK);	/* mask phy interrupts - leave HPD */
	_enable_tmds(dev, 0);
	_power_down(dev, 0);	/*  disable PHY */
	_tx_power_on(dev, 0);
	_set_pddq(dev, 1);

	return TRUE;
}


int phy_interrupt_enable(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	dev_write(dev, PHY_MASK0, value);
	return TRUE;
}

int phy_phase_lock_loop_state(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	return dev_read_mask(dev, (PHY_STAT0), PHY_STAT0_TX_PHY_LOCK_MASK);
}

void phy_interrupt_mask(hdmi_tx_dev_t *dev, u8 mask)
{
	LOG_TRACE1(mask);
	// Mask will determine which bits will be enabled
	dev_write_mask(dev, PHY_MASK0, mask, 0xff);
}

void phy_interrupt_unmask(hdmi_tx_dev_t *dev, u8 mask)
{
	LOG_TRACE1(mask);
	// Mask will determine which bits will be enabled
	dev_write_mask(dev, PHY_MASK0, mask, 0x0);
}

u8 phy_rx_s0_state(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	return dev_read_mask(dev, (PHY_STAT0), PHY_STAT0_RX_SENSE_0_MASK);
}

u8 phy_rx_s1_state(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	return dev_read_mask(dev, (PHY_STAT0), PHY_STAT0_RX_SENSE_1_MASK);
}
u8 phy_rx_s2_state(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	return dev_read_mask(dev, (PHY_STAT0), PHY_STAT0_RX_SENSE_2_MASK);
}

u8 phy_rx_s3_state(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	return dev_read_mask(dev, (PHY_STAT0), PHY_STAT0_RX_SENSE_3_MASK);
}

u8 phy_rx_sense_state(hdmi_tx_dev_t *dev)
{
	u8 state;

	LOG_TRACE();
	state = phy_rx_s0_state(dev);
	state |= phy_rx_s1_state(dev);
	state |= phy_rx_s2_state(dev);
	state |= phy_rx_s3_state(dev);
	return state;
}

u8 phy_hot_plug_state(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	// return dev_read_mask(dev, (PHY_STAT0), PHY_STAT0_HPD_MASK);
	return 0x3;
}

int phy_hpd_sense(hdmi_tx_dev_t *dev, int enable)
{
// #ifndef PHY_THIRD_PARTY
// 	phy_enable_hpd_sense(dev, (enable ? 1 : 0));
// #endif
	return TRUE;
}

int phy_hot_plug_detected(hdmi_tx_dev_t *dev)
{
	/* MASK         STATUS          POLARITY        INTERRUPT        HPD
	 *   0             0                 0               1             0
	 *   0             1                 0               0             1
	 *   0             0                 1               0             0
	 *   0             1                 1               1             1
	 *   1             x                 x               0             x
	 */

	int hpd_polarity = dev_read_mask(dev, PHY_POL0, PHY_POL0_HPD_MASK);
	int hpd = dev_read_mask(dev, PHY_STAT0, PHY_STAT0_HPD_MASK);

	// Mask interrupt
	phy_interrupt_mask(dev, PHY_MASK0_HPD_MASK);

	if (hpd_polarity == hpd) {
		dev_write_mask(dev, PHY_POL0, PHY_POL0_HPD_MASK, !hpd_polarity);

		// Un-mask interrupts
		phy_interrupt_unmask(dev, PHY_MASK0_HPD_MASK);

		return hpd_polarity;
	}

	// Un-mask interrupts
	phy_interrupt_unmask(dev, PHY_MASK0_HPD_MASK);

	return !hpd_polarity;
}

int phy_rx_s0_detected(hdmi_tx_dev_t *dev)
{
	/* MASK         STATUS          POLARITY        INTERRUPT        RxS
	 *   0             0                 0               1             0
	 *   0             1                 0               0             1
	 *   0             0                 1               0             0
	 *   0             1                 1               1             1
	 *   1             x                 x               0             x
	 */

	int RxS_polarity = dev_read_mask(dev, PHY_POL0, PHY_POL0_RX_SENSE_0_MASK);
	int RxS = dev_read_mask(dev, PHY_STAT0, PHY_STAT0_RX_SENSE_0_MASK);

	// Mask interrupt
	phy_interrupt_mask(dev, PHY_MASK0_RX_SENSE_0_MASK);

	if (RxS_polarity == RxS) {
		dev_write_mask(dev, PHY_POL0, PHY_POL0_RX_SENSE_0_MASK, !RxS_polarity);

		// Un-mask interrupts
		phy_interrupt_unmask(dev, PHY_MASK0_RX_SENSE_0_MASK);

		return RxS_polarity;
	}

	// Un-mask interrupts
	phy_interrupt_unmask(dev, PHY_MASK0_RX_SENSE_0_MASK);

	return !RxS_polarity;
}

int phy_rx_s1_detected(hdmi_tx_dev_t *dev)
{
	/* MASK         STATUS          POLARITY        INTERRUPT        RxS
	 *   0             0                 0               1             0
	 *   0             1                 0               0             1
	 *   0             0                 1               0             0
	 *   0             1                 1               1             1
	 *   1             x                 x               0             x
	 */

	int RxS_polarity = dev_read_mask(dev, PHY_POL0, PHY_POL0_RX_SENSE_1_MASK);
	int RxS = dev_read_mask(dev, PHY_STAT0, PHY_STAT0_RX_SENSE_1_MASK);

	// Mask interrupt
	phy_interrupt_mask(dev, PHY_MASK0_RX_SENSE_1_MASK);

	if (RxS_polarity == RxS) {
		dev_write_mask(dev, PHY_POL0, PHY_POL0_RX_SENSE_1_MASK, !RxS_polarity);

		// Un-mask interrupts
		phy_interrupt_unmask(dev, PHY_MASK0_RX_SENSE_1_MASK);

		return RxS_polarity;
	}

	// Un-mask interrupts
	phy_interrupt_unmask(dev, PHY_MASK0_RX_SENSE_1_MASK);

	return !RxS_polarity;
}

int phy_rx_s2_detected(hdmi_tx_dev_t *dev)
{
	/* MASK         STATUS          POLARITY        INTERRUPT        RxS
	 *   0             0                 0               1             0
	 *   0             1                 0               0             1
	 *   0             0                 1               0             0
	 *   0             1                 1               1             1
	 *   1             x                 x               0             x
	 */

	int RxS_polarity = dev_read_mask(dev, PHY_POL0, PHY_POL0_RX_SENSE_2_MASK);
	int RxS = dev_read_mask(dev, PHY_STAT0, PHY_STAT0_RX_SENSE_2_MASK);

	// Mask interrupt
	phy_interrupt_mask(dev, PHY_MASK0_RX_SENSE_2_MASK);

	if (RxS_polarity == RxS) {
		dev_write_mask(dev, PHY_POL0, PHY_POL0_RX_SENSE_2_MASK, !RxS_polarity);

		// Un-mask interrupts
		phy_interrupt_unmask(dev, PHY_MASK0_RX_SENSE_2_MASK);

		return RxS_polarity;
	}

	// Un-mask interrupts
	phy_interrupt_unmask(dev, PHY_MASK0_RX_SENSE_2_MASK);

	return !RxS_polarity;
}

int phy_rx_s3_detected(hdmi_tx_dev_t *dev)
{
	/* MASK         STATUS          POLARITY        INTERRUPT        RxS
	 *   0             0                 0               1             0
	 *   0             1                 0               0             1
	 *   0             0                 1               0             0
	 *   0             1                 1               1             1
	 *   1             x                 x               0             x
	 */

	int RxS_polarity = dev_read_mask(dev, PHY_POL0, PHY_POL0_RX_SENSE_3_MASK);
	int RxS = dev_read_mask(dev, PHY_STAT0, PHY_STAT0_RX_SENSE_3_MASK);

	// Mask interrupt
	phy_interrupt_mask(dev, PHY_MASK0_RX_SENSE_3_MASK);

	if (RxS_polarity == RxS) {
		dev_write_mask(dev, PHY_POL0, PHY_POL0_RX_SENSE_3_MASK, !RxS_polarity);

		// Un-mask interrupts
		phy_interrupt_unmask(dev, PHY_MASK0_RX_SENSE_3_MASK);

		return RxS_polarity;
	}

	// Un-mask interrupts
	phy_interrupt_unmask(dev, PHY_MASK0_RX_SENSE_3_MASK);

	return !RxS_polarity;
}

double phy_get_freq(double pClk)
{
	if(((pClk >= 25.175) && (pClk <= 25.180)) || ((pClk >= 25.195) && (pClk <= 25.205)))
		return 25.175;
	else if (((pClk >= 26.995) && (pClk <= 27.005)) || ((pClk >= 27.022) && (pClk <= 27.032)))
		return 27.000;
	else if (double_is_equal(pClk, 31.500))
		return 31.500;
	else if (double_is_equal(pClk,  33.750))
		return 33.750;
	else if (double_is_equal(pClk,  35.500))
		return 35.500;
	else if (((pClk >= 35.995) && (pClk <= 36.005)) || ((pClk >= 36.031) && (pClk <= 36.041)))
		return 36.000;
	else if (double_is_equal(pClk,  40.000))
		return 40.000;
	else if (double_is_equal(pClk,  44.900))
		return 44.900;
	else if (double_is_equal(pClk,  49.500))
		return 49.500;
	else if (double_is_equal(pClk,  50.000))
		return 50.000;
	else if (((pClk >= 50.345) && (pClk <= 50.355)) || ((pClk >= 50.395) && (pClk <= 50.405)))
		return 50.350;
	else if (((pClk >= 53.995) && (pClk <= 54.005)) || ((pClk >= 50.049) && (pClk <= 54.059)))
		return 54.000;
	else if (double_is_equal(pClk,  56.250))
		return 56.250;
	else if (((pClk >= 59.336) && (pClk <= 59.346)) || ((pClk >= 59.395) && (pClk <= 59.405)))
		return 59.400;
	else if (double_is_equal(pClk,  65.000))
		return 65.000;
	else if (double_is_equal(pClk,  68.250))
		return 68.250;
	else if (double_is_equal(pClk,  71.000))
		return 71.000;
	else if (double_is_equal(pClk,  72.000))
		return 72.000;
	else if (double_is_equal(pClk,  73.250))
		return 73.250;
	else if (((pClk >= 74.171) && (pClk <= 74.181)) || ((pClk >= 74.245) && (pClk <= 74.255)))
		return 74.250;
	else if (double_is_equal(pClk,  75.000))
		return 75.000;
	else if (double_is_equal(pClk,  78.750))
		return 78.750;
	else if (double_is_equal(pClk,  79.500))
		return 79.500;
	else if (((pClk >= 82.143) && (pClk <= 82.153)) || ((pClk >= 82.495) && (pClk <= 82.505)))
		return 82.500;
	else if (double_is_equal(pClk,  83.500))
		return 83.500;
	else if (double_is_equal(pClk,  85.500))
		return 85.500;
	else if (double_is_equal(pClk,  88.750))
		return 88.750;
	else if (double_is_equal(pClk,  90.000))
		return 90.000;
	else if (double_is_equal(pClk,  94.500))
		return 94.500;
	else if (((pClk >= 98.896) && (pClk <= 98.906)) || ((pClk >= 98.995) && (pClk <= 99.005)))
		return 99.000;
	else if (((pClk >= 100.695) && (pClk <= 100.705)) || ((pClk >= 100.795) && (pClk <= 100.805)))
		return 100.700;
	else if (double_is_equal(pClk, 101.000))
		return 101.000;
	else if (double_is_equal(pClk, 102.250))
		return 102.250;
	else if (double_is_equal(pClk, 106.500))
		return 106.500;
	else if (((pClk >= 107.995) && (pClk <= 108.005)) || ((pClk >= 108.103) && (pClk <= 108.113)))
		return 108.000;
	else if (double_is_equal(pClk, 115.500))
		return 115.500;
	else if (double_is_equal(pClk, 117.500))
		return 117.500;
	else if (((pClk >= 118.795) && (pClk <= 118.805)) || ((pClk >= 118.677) && (pClk <= 118.687)))
		return 118.800;
	else if (double_is_equal(pClk, 119.000))
		return 119.000;
	else if (double_is_equal(pClk, 121.750))
		return 122.500;
	else if (double_is_equal(pClk, 122.500))
		return 121.750;
	else if (double_is_equal(pClk, 135.000))
		return 135.000;
	else if (double_is_equal(pClk, 136.750))
		return 136.750;
	else if (double_is_equal(pClk, 140.250))
		return 140.250;
	else if (double_is_equal(pClk, 144.000))
		return 144.000;
	else if (double_is_equal(pClk, 146.250))
		return 146.250;
	else if (double_is_equal(pClk, 148.250))
		return 148.250;
	else if (((pClk >= 148.347) && (pClk <= 148.357)) || ((pClk >= 148.495) && (pClk <= 148.505)))
		return 148.500;
	else if (double_is_equal(pClk, 154.000))
		return 154.000;
	else if (double_is_equal(pClk, 156.000))
		return 156.000;
	else if (double_is_equal(pClk, 157.000))
		return 157.000;
	else if (double_is_equal(pClk, 157.500))
		return 157.500;
	else if (double_is_equal(pClk, 162.000))
		return 162.000;
	else if (((pClk >= 164.830) && (pClk <= 164.840)) || ((pClk >= 164.995) && (pClk <= 165.005)))
		return 165.000;
	else if (double_is_equal(pClk, 175.500))
		return 175.500;
	else if (double_is_equal(pClk, 179.500))
		return 179.500;
	else if (double_is_equal(pClk, 180.000))
		return 180.000;
	else if (double_is_equal(pClk, 182.750))
		return 182.750;
	else if (((pClk >= 185.435) && (pClk <= 185.445)) || ((pClk >= 185.620) && (pClk <= 185.630)))
		return 185.625;
	else if (double_is_equal(pClk, 187.000))
		return 187.000;
	else if (double_is_equal(pClk, 187.250))
		return 187.250;
	else if (double_is_equal(pClk, 189.000))
		return 189.000;
	else if (double_is_equal(pClk, 193.250))
		return 193.250;
	else if (((pClk >= 197.797) && (pClk <= 197.807)) || ((pClk >= 197.995) && (pClk <= 198.005)))
		return 198.000;
	else if (double_is_equal(pClk, 202.500))
		return 202.500;
	else if (double_is_equal(pClk, 204.750))
		return 204.750;
	else if (double_is_equal(pClk, 208.000))
		return 208.000;
	else if (double_is_equal(pClk, 214.750))
		return 214.750;
	else if (((pClk >= 216.211) && (pClk <= 216.221)) || ((pClk >= 215.995) && (pClk <= 216.005)))
		return 216.000;
	else if (double_is_equal(pClk, 218.250))
		return 218.250;
	else if (double_is_equal(pClk, 229.500))
		return 229.500;
	else if (double_is_equal(pClk, 234.000))
		return 234.000;
	else if (((pClk >= 237.359) && (pClk <= 237.369)) || ((pClk >= 237.595) && (pClk <= 237.605)))
		return 237.600;
	else if (double_is_equal(pClk, 245.250))
		return 245.250;
	else if (double_is_equal(pClk, 245.500))
		return 245.500;
	else if (double_is_equal(pClk, 261.000))
		return 261.000;
	else if (double_is_equal(pClk, 268.250))
		return 268.250;
	else if (double_is_equal(pClk, 268.500))
		return 268.500;
	else if (double_is_equal(pClk, 281.250))
		return 281.250;
	else if (double_is_equal(pClk, 288.000))
		return 288.000;
	else if (((pClk >= 296.698) && (pClk <= 296.708)) || ((pClk >= 296.995) && (pClk <= 297.005)))
		return 297.000;
	else if (double_is_equal(pClk, 317.000))
		return 317.000;
	else if (double_is_equal(pClk, 330.000))
		return 330.000;
	else if (double_is_equal(pClk, 333.250))
		return 333.250;
	else if (((pClk >= 339.655) && (pClk <= 339.665)) || ((pClk >= 339.995) && (pClk <= 340.005)))
		return 340.000;
	else if (double_is_equal(pClk, 348.500))
		return 348.500;
	else if (double_is_equal(pClk, 356.500))
		return 356.500;
	else if (double_is_equal(pClk, 360.000))
		return 360.000;
	else if (((pClk >= 370.874) && (pClk <= 370.884)) || ((pClk >= 371.245) && (pClk <= 371.255)))
		return 371.250;
	else if (double_is_equal(pClk, 380.500))
		return 380.500;
	else if (((pClk >= 395.599) && (pClk <= 395.609)) || ((pClk >= 395.995) && (pClk <= 396.005)))
		return 396.000;
	else if (((pClk >= 431.952) && (pClk <= 431.967)) || ((pClk >= 431.995) && (pClk <= 432.005)) || ((pClk >= 432.427) && (pClk <= 432.437)))
		return 432.000;
	else if (double_is_equal(pClk, 443.250))
		return 443.250;
	else if (((pClk >= 475.148) && (pClk <= 475.158)) || ((pClk >= 475.195) && (pClk <= 475.205)) || ((pClk >= 474.723) && (pClk <= 474.733)))
		return 475.200;
	else if (((pClk >= 494.500) && (pClk <= 494.510)) || ((pClk >= 494.995) && (pClk <= 495.005)))
		return 495.000;
	else if (double_is_equal(pClk, 505.250))
		return 505.250;
	else if (double_is_equal(pClk, 552.750))
		return 552.750;
	else if (((pClk >= 593.995) && (pClk <= 594.005)) || ((pClk >= 593.403) && (pClk <= 593.413)))
		return 594.000;
	else{
		LOGGER(SNPS_ERROR, "%s:Unable to map input pixel clock frequency %0.3fMHz", __func__, pClk);
	}
	return 1.000;
}

/**** Unused functions */

int phy_test_control(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	_test_data_in(dev, value);
	_test_enable(dev, 1);
	_test_clock(dev, 1);
	_test_clock(dev, 0);
	_test_enable(dev, 0);
	return TRUE;
}

int phy_test_data(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	_test_data_in(dev, value);
	_test_enable(dev, 0);
	_test_clock(dev, 1);
	_test_clock(dev, 0);
	return TRUE;
}

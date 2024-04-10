/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

/**
 * @file
 * Physical line interface configuration
 */
#ifndef PHY_H_
#define PHY_H_

#include "../hdmitx_dev.h"
#include "video.h"
#include "util/types.h"

#define PHY_TIMEOUT 		100
#define PHY_I2C_SLAVE_ADDR   	0x54

#define PHY_MODEL_108		108
#define PHY_MODEL_301		301
#define PHY_MODEL_302		302
#define PHY_MODEL_303		303
#define PHY_MODEL_305		305
#define PHY_MODEL_308		308
#define PHY_MODEL_311		311
#define PHY_MODEL_312		312
#define PHY_MODEL_316		316
#ifdef PHY_THIRD_PARTY
#define PHY_MODEL_THIRD_PARTY_ACK 		10
#define PHY_MODEL_THIRD_PARTY_ANGELCREEK	11
#endif

int phy_powerup(hdmi_tx_dev_t *dev, u16 phy_model);

int phy_preparation(hdmi_tx_dev_t *dev, u16 phy_model);

struct phy_config{
	double 			clock;
	pixel_repetition_t 	pixel;
	color_depth_t      	color;
	operation_mode_t 	opmode;
	u16		 	oppllcfg;
	u16			pllcurrctrl;
	u16			pllgmpctrl;
	u16                 	txterm;
	u16                 	vlevctrl;
	u16                 	cksymtxctrl;
};

/**
 * Initialize PHY and put into a known state
 * @param dev device structure
 * @param phy_model
 * return always TRUE
 */
int phy_initialize(hdmi_tx_dev_t *dev, u16 phy_model);

char * phy_identification(hdmi_tx_dev_t *dev, u16 phy_model);

/**
 * Bring up PHY and start sending media for a specified pixel clock, pixel
 * repetition and color resolution (to calculate TMDS) - this fields must
 * be configured in the dev->snps_hdmi_ctrl variables
 * @param dev device structure
 * return TRUE if success, FALSE if not success and -1 if PHY configurations
 * are not supported.
 */
int phy_configure(hdmi_tx_dev_t *dev, u16 phy_model);

/**
 * Set PHY to standby mode - turn off all interrupts
 * @param dev device structure
 */
int phy_standby(hdmi_tx_dev_t *dev);

/**
 * Enable HPD sensing circuitry
 * @param dev device structure
 */
void phy_enable_hpd_sense(hdmi_tx_dev_t *dev, u8 bit);

/**
 * Disable HPD sensing circuitry
 * @param dev device structure
 */
int phy_disable_hpd_sense(hdmi_tx_dev_t *dev);

/**
 * Detects the signal on the HPD line and 
 * upon change, it inverts polarity of the interrupt
 * bit so that interrupt raises only on change
 * @param dev device structure
 * @return TRUE the HPD line is asserted
 */
int phy_hot_plug_detected(hdmi_tx_dev_t *dev);


int phy_rx_s0_detected(hdmi_tx_dev_t *dev);
int phy_rx_s1_detected(hdmi_tx_dev_t *dev);
int phy_rx_s2_detected(hdmi_tx_dev_t *dev);
int phy_rx_s3_detected(hdmi_tx_dev_t *dev);
/**
 * @param dev device structure
 * @param value of mask of interrupt register
 */
int phy_interrupt_enable(hdmi_tx_dev_t *dev, u8 value);

/**
 * @param baseAddr of controller
 * @param value
 */
int phy_test_control(hdmi_tx_dev_t *dev, u8 value);

/**
 * @param dev device structure
 * @param value
 */
int phy_test_data(hdmi_tx_dev_t *dev, u8 value);

int phy_hpd_sense(hdmi_tx_dev_t *dev, int enable);

int phy_phase_lock_loop_state(hdmi_tx_dev_t *dev);

void phy_interrupt_mask(hdmi_tx_dev_t *dev, u8 mask);

void phy_interrupt_unmask(hdmi_tx_dev_t *dev, u8 mask);

u8 phy_rx_s0_state(hdmi_tx_dev_t *dev);
u8 phy_rx_s1_state(hdmi_tx_dev_t *dev);
u8 phy_rx_s2_state(hdmi_tx_dev_t *dev);
u8 phy_rx_s3_state(hdmi_tx_dev_t *dev);
u8 phy_rx_sense_state(hdmi_tx_dev_t *dev);

u8 phy_hot_plug_state(hdmi_tx_dev_t *dev);

int phy_write(hdmi_tx_dev_t *dev, u16 addr, u32 data);

int phy_read(hdmi_tx_dev_t *dev, u16 addr, u32 * value);

int phy_slave_address(hdmi_tx_dev_t *dev, u8 value);

int phy_set_interface(hdmi_tx_dev_t *dev, phy_access_t interface);

int phy_reconfigure_interface(hdmi_tx_dev_t *dev);
phy_access_t phy_get_interface(hdmi_tx_dev_t *dev);

double phy_get_freq(double pClk);

#endif	/* PHY_H_ */

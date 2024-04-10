/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

/**
 * @file 
 *      Access methods mask the the register access
 *      it accesses register using 16-bit addresses
 *      and 8-bit data
 *      @note: at least access_Write and access_Read should be re-implemented
 *      @note functions in this module are critical sections and should not be
 *      	interrupted
 *      @note: this implementation protects critical sections using muteces.
 *     		make sure critical section are protected from race conditions,
 *  	   	particularly when interrupts are enabled.
 */

#ifndef ACCESS_H_
#define ACCESS_H_

#include <stdint.h>

#include "../hdmitx_dev.h"
#include "util/types.h"

#define ADDR_JUMP 4

struct device_access {
	char name[30];

	int (*initialize) (void);
	int (*disable) (void);

	void (*write) (u32 addr, u32 data);
	u32  (*read)  (u32 addr);
};

void register_bsp_functions(struct device_access * device);

/**
 *Initialize communications with development board
 *@param baseAddr pointer to the address of the core on the bus
 *@return TRUE  if successful.
 */
int dev_initialize();

/**
 *Close communications with development board and free resources
 *@return TRUE  if successful.
 */
int dev_standby();

/**
 *Read the contents of a register
 *@param addr of the register
 *@return 8bit byte containing the contents
 */
u32 dev_read(hdmi_tx_dev_t * dev, u32 addr);

/**
 *Read several bits from a register
 *@param addr of the register
 *@param shift of the bit from the beginning
 *@param width or number of bits to read
 *@return the contents of the specified bits
 */
u32 dev_read_mask(hdmi_tx_dev_t * dev, u32 addr, u32 mask);

/**
 *Write a byte to a register
 *@param data to be written to the register
 *@param addr of the register
 */
void dev_write(hdmi_tx_dev_t * dev, u32 addr, u32 data);

/**
 *Write to several bits in a register
 *
 *@param data to be written to the required part
 *@param addr of the register
 *@param shift of the bits from the beginning
 *@param width or number of bits to written to
 */
void dev_write_mask(hdmi_tx_dev_t * dev, u32 addr, u32 mask, u32 data);

/**
 *Initialize communications with development board
 *
 *@param baseAddr pointer to the address of the core on the bus
 *@return TRUE  if successful.
 */
int access_Initialize(hdmi_tx_dev_t * dev);

/**
 *Close communications with development board and free resources
 *
 *@return TRUE  if successful.
 */
int access_Standby(hdmi_tx_dev_t * dev);

///**
// *Read several bits from a register
// *
// *@param addr of the register
// *@param shift of the bit from the beginning
// *@param width or number of bits to read
// *@return the contents of the specified bits
// */
//u32 access_CoreRead(hdmi_tx_dev_t * dev, u32 addr, u8 shift, u8 width);
//
///**
// *Write to several bits in a register
// *
// *@param data to be written to the required part
// *@param addr of the register
// *@param shift of the bits from the beginning
// *@param width or number of bits to written to
// */
//void access_CoreWrite(hdmi_tx_dev_t *dev, u8 data, u32 addr, u8 shift, u8 width);

#endif				/* ACCESS_H_ */

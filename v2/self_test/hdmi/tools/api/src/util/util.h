/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef SNPS_API_UTIL_H_
#define SNPS_API_UTIL_H_

#include "bsp/board.h"

/* *************************************************************************
 * Data Manipulation and Access
 * *************************************************************************/
/**
 * Find first (least significant) bit set
 * @param[in] data word to search
 * @return bit position or 32 if none is set
 */
static inline uint32_t first_bit_set(uint32_t data)
{
	uint32_t n = 0;

	if (data != 0) {
		for (n = 0; (data & 1) == 0; n++) {
			data >>= 1;
		}
	}
	return n;
}

/**
 * Get bit field
 * @param[in] data raw data
 * @param[in] mask bit field mask
 * @return bit field value
 */
static inline uint32_t get(uint32_t data, uint32_t mask)
{
	return ((data & mask) >> first_bit_set(mask));
}

/**
 * Set bit field
 * @param[in] data raw data
 * @param[in] mask bit field mask
 * @param[in] value new value
 * @return new raw data
 */
static inline uint32_t set(uint32_t data, uint32_t mask, uint32_t value)
{
	return (((value << first_bit_set(mask)) & mask) | (data & ~mask));
}

/**
 * Get bit field
 * @param[in] data raw data
 * @param[in] mask bit field mask
 * @return bit field value
 */
static inline uint8_t get8(uint8_t data, uint8_t mask)
{
	return ((data & mask) >> first_bit_set(mask));
}

/**
 * Set bit field
 * @param[in] data raw data
 * @param[in] mask bit field mask
 * @param[in] value new value
 * @return new raw data
 */
static inline uint8_t set8(uint8_t data, uint8_t mask, uint8_t value)
{
	return (((value << first_bit_set(mask)) & mask) | (data & ~mask));
}

#endif

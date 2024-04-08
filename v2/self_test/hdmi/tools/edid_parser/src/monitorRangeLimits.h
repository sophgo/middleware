/*
 * monitor_range_limits.h
 *
 *  Created on: Jul 22, 2010
 * 
 * Synopsys Inc.
 * SG DWC PT02
 */

#ifndef MONITORRANGELIMITS_H_
#define MONITORRANGELIMITS_H_

#include "includes.h"

/**
 * @file
 * Second Monitor Descriptor
 * Parse and hold Monitor Range Limits information read from EDID
 */

typedef struct {
	u8 mMinVerticalRate;

	u8 mMaxVerticalRate;

	u8 mMinHorizontalRate;

	u8 mMaxHorizontalRate;

	u8 mMaxPixelClock;

	int mValid;
} monitorRangeLimits_t;

void monitor_range_limits_reset(hdmi_tx_dev_t *dev, monitorRangeLimits_t * mrl);

int monitorRangeLimits_Parse(hdmi_tx_dev_t *dev, monitorRangeLimits_t * mrl, u8 * data);

/**
 * @return the maximum parameter for horizontal frequencies
 */
u8 monitorRangeLimits_GetMaxHorizontalRate(hdmi_tx_dev_t *dev, monitorRangeLimits_t * mrl);
/**
 * @return the maximum parameter for pixel clock rate
 */
u8 monitorRangeLimits_GetMaxPixelClock(hdmi_tx_dev_t *dev, monitorRangeLimits_t * mrl);
/**
 * @return the maximum parameter for vertical frequencies
 */
u8 monitorRangeLimits_GetMaxVerticalRate(hdmi_tx_dev_t *dev, monitorRangeLimits_t * mrl);
/**
 * @return the minimum parameter for horizontal frequencies
 */
u8 monitorRangeLimits_GetMinHorizontalRate(hdmi_tx_dev_t *dev, monitorRangeLimits_t * mrl);
/**
 * @return the minimum parameter for vertical frequencies
 */
u8 monitorRangeLimits_GetMinVerticalRate(hdmi_tx_dev_t *dev, monitorRangeLimits_t * mrl);

#endif				/* MONITORRANGELIMITS_H_ */

/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef DTD_H_
#define DTD_H_

#include "../hdmitx_dev.h"
#include "util/types.h"

/**
 * @file
 * For detailed handling of this structure, refer to documentation of the functions
 */
typedef struct {
	/** VIC code */
	u32 mCode;

	/** Identifies modes that ONLY can be displayed in YCC 4:2:0 */
	u8 mLimitedToYcc420;

	/** Identifies modes that can also be displayed in YCC 4:2:0 */
	u8 mYcc420;

	u16 mPixelRepetitionInput;

	/** In units of 1MHz */
	double mPixelClock;

	/** 1 for interlaced, 0 progressive */
	u8 mInterlaced;

	u16 mHActive;

	u16 mHBlanking;

	u16 mHBorder;

	u16 mHImageSize;

	u16 mHSyncOffset;

	u16 mHSyncPulseWidth;

	/** 0 for Active low, 1 active high */
	u8 mHSyncPolarity;

	u16 mVActive;

	u16 mVBlanking;

	u16 mVBorder;

	u16 mVImageSize;

	u16 mVSyncOffset;

	u16 mVSyncPulseWidth;

	/** 0 for Active low, 1 active high */
	u8 mVSyncPolarity;

} dtd_t;

/**
 * Parses the Detailed Timing Descriptor.
 * Encapsulating the parsing process
 * @param dtd pointer to dtd_t strucutute for the information to be save in
 * @param data a pointer to the 18-byte structure to be parsed.
 * @return TRUE if success
 */
int dtd_parse(hdmi_tx_dev_t *dev, dtd_t * dtd, u8 data[18]);

/**
 * @param dtd pointer to dtd_t strucutute for the information to be save in
 * @param code the CEA 861-D video code.
 * @param refreshRate the specified vertical refresh rate.
 * @return TRUE if success
 */
int dtd_fill(hdmi_tx_dev_t *dev, dtd_t * dtd, u8 code, double refreshRate);

/**
 * @param dtd Pointer to the current DTD parameters
 * @return The refresh rate if DTD parameters are correct and 0 if not
 */
double dtd_get_refresh_rate(dtd_t *dtd);

/**
 * @param dtd Pointer to the current DTD parameters
 * @param tempDtd Pointer to the temp DTD parameters
 * @return The refresh rate if DTD parameters are correct and 0 if not
 */
void dtd_change_horiz_for_ycc420(hdmi_tx_dev_t *dev, dtd_t * tempDtd);

#endif	/* DTD_H_ */

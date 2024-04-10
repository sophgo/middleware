/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef SHORTAUDIODESC_H_
#define SHORTAUDIODESC_H_

#include "../hdmitx_dev.h"
#include "util/types.h"
/**
 * @file
 * Short Audio Descriptor.
 * Found in Audio Data Block (shortAudioDesc_t *sad, CEA Data Block Tage Code 1).
 * Parse and hold information from EDID data structure
 */
/** For detailed handling of this structure, refer to documentation of the functions */
typedef struct {
	u8 mFormat;

	u8 mMaxChannels;

	u8 mSampleRates;

	u8 mByte3;
} shortAudioDesc_t;

void sad_reset(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad);

/**
 * Parse Short Audio Descriptor
 */
int sad_parse(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad, u8 * data);

/**
 *@return the sample rates byte, where bit 7 is always 0 and rates are sorted respectively starting with bit 6:
 * 192 kHz  176.4 kHz  96 kHz  88.2 kHz  48 kHz  44.1 kHz  32 kHz
 */
//u8 sad_GetSampleRates(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad);

int sad_support32k(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad);

int sad_support44k1(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad);

int sad_support48k(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad);

int sad_support88k2(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad);

int sad_support96k(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad);

int sad_support176k4(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad);

int sad_support192k(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad);

int sad_support16bit(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad);

int sad_support20bit(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad);

int sad_support24bit(hdmi_tx_dev_t *dev, shortAudioDesc_t * sad);

#endif	/* SHORTAUDIODESC_H_ */

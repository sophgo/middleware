/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef VIDEO_H_
#define VIDEO_H_
/**
 * @file
 * Video controller layer.
 * Configure and set up the video transmitted to sink.
 * This includes colour-space conversion as well as pixel packetising.
 */

#include "../hdmitx_dev.h"
#include "util/types.h"
#include "video_params.h"

/**
 * Initializes and configures the video blocks to transmit a blue screen
 * @param baseAddr Base Address of module
 * @param params VideoParams
 * @param dataEnablePolarity data enable polarity (1 = enable, 0 not)
 * @return TRUE if successful
 */
int video_Initialize(hdmi_tx_dev_t *dev, videoParams_t * params,
		     u8 dataEnablePolarity);

/**
 * Configures the video blocks to do any video processing and to
 * transmit the video set up required by the user, allowing to
 * force video pixels (from the DEBUG pixels) to be transmitted
 * rather than the video stream being received.
 * @param baseAddr Base Address of module
 * @param params VideoParams
 * @return TRUE if successful
 */
int video_Configure(hdmi_tx_dev_t *dev, videoParams_t * params);

/**
 * Set up color space converter to video requirements
 * (if there is any encoding type conversion or csc coefficients)
 * @param baseAddr Base Address of module 
 * @param params VideoParams
 * @return TRUE if successful
 */
int video_ColorSpaceConverter(hdmi_tx_dev_t *dev, videoParams_t * params);

/**
 * Set up video packetizer which "packetizes" pixel transmission
 * (in deep colour mode, YCC422 mapping and pixel repetition)
 * @param baseAddr Base Address of module
 * @param params VideoParams
 * @return TRUE if successful
 */
int video_VideoPacketizer(hdmi_tx_dev_t *dev, videoParams_t * params);

/**
 * Set up video mapping and stuffing
 * @param baseAddr Base Address of module
 * @param params VideoParams
 * @return TRUE if successful
 */
int video_VideoSampler(hdmi_tx_dev_t *dev, videoParams_t * params);

/**
 * A test only method that is used for a test module
 * @param baseAddr Base Address of module
 * @param params VideoParams
 * @param dataEnablePolarity
 * @return TRUE if successful
 */
int video_VideoGenerator(hdmi_tx_dev_t *dev, videoParams_t * params,
			 u8 dataEnablePolarity);

#endif	/* VIDEO_H_ */

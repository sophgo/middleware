/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

/** 
 * @file
 * This is the upper layer of the HDMI TX API.
 * It is the entry point and interface of the software package
 * It crosses information from the sink and information passed on from
 * the user to check if it violates the HDMI protocol or not.
 * It coordinates between the various modules of the API to program the 
 * controller successfully in the correct steps and in a compliant 
 * configuration.
 * Errors are written to the error buffer. use error_Get in util/error.h to read
 * last error.
 */

#ifndef API_H_
#define API_H_
#include "util/types.h"
#include "util/error.h"
#include "core/product.h"
#include "core/video_params.h"
#include "core/audio_params.h"
#include "hdcp/hdcp_params.h"
#include "edid/dtd.h"
#include "edid/short_video_desc.h"
#include "edid/short_audio_desc.h"
#include "edid/hdmivsdb.h"
#include "edid/hdmiforumvsdb.h"
#include "edid/monitor_range_limits.h"
#include "edid/video_cap_data_block.h"
#include "edid/colorimetry_data_block.h"
#include "edid/speaker_alloc_data_block.h"
#include "core/fc_debug.h"

#include "bsp/access.h"
#include "bsp/system.h"
#include "phy/phy.h"
#include <stdio.h>
#include <string.h>
#include "../hdmitx_dev.h"


/** @addtogroup init_api_grp Initialization API Routines
 *
 * These routines handle initialization of the CIL and PCD driver components
 * and the DWC_usb3 controller.
 */
/** @{ */

void hdmitx_api_init(hdmi_tx_dev_t * dev, char * name);

/**
 *
 */
void api_set_hdmi_ctrl(hdmi_tx_dev_t *dev, videoParams_t * video, hdcpParams_t * hdcp);

/**
 * Configure API.
 * Configure the modules of the API according to the parameters given by
 * the user. If EDID at sink is read, it does parameter checking using the 
 * Check methods against the sink's E-EDID. Violations are outputted to the 
 * buffer.
 * Shall only be called after an Init call or configure.
 * @param video parameters pointer
 * @param audio parameters pointer
 * @param product parameters pointer
 * @param hdcp parameters pointer
 * @return TRUE when successful
 * @note during this function, all controller's interrupts are disabled
 * @note this function needs to have the HW initialized before the first call
 */
int api_Configure(hdmi_tx_dev_t *dev, videoParams_t * video, audioParams_t * audio,
	      productParams_t * product, hdcpParams_t * hdcp, u16 phy_model);

/**
 * Prepare API modules and local variables to standby mode (hdmi_tx_dev_t *dev, and not respond
 * to interrupts) and frees all resources
 * @return TRUE when successful
 * @note must be called to free up resources and before another Init.
 */
int api_Standby(hdmi_tx_dev_t *dev);

/**
 * AV Mute in the General Control Packet
 * @param enable TRUE set the AVMute in the general control packet
 */
void api_avmute(hdmi_tx_dev_t *dev, int enable);

/** @} */

#endif	/* API_H_ */


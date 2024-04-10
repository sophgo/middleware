/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef API_HDCP_H_
#define API_HDCP_H_

#include "../hdmitx_dev.h"
#include "util/types.h"
#include "hdcp_params.h"
#include "video_params.h"

typedef enum {
	HDCP_IDLE = 0,
	HDCP_KSV_LIST_READY,
	HDCP_ERR_KSV_LIST_NOT_VALID,
	HDCP_KSV_LIST_ERR_DEPTH_EXCEEDED,
	HDCP_KSV_LIST_ERR_MEM_ACCESS,
	HDCP_ENGAGED,
	HDCP_FAILED
} hdcp_status_t;

/**
 * @param dev Device structure
 * @param dataEnablePolarity
 * @return TRUE if successful
 */
int hdcp_initialize(hdmi_tx_dev_t *dev);

/**
 * HDCP configuration - HDMI initialization Step G (Controller User Guide)
 * @param dev Device structure
 * @param params HDCP parameters
 * @param mode HDMI or DVI
 * @param hsPol HSYNC polarity
 * @param vsPol VSYNC polarity
 * @return TRUE if successful
 */
int hdcp_configure(hdmi_tx_dev_t *dev, hdcpParams_t * hdcp, videoParams_t *video);

/**
 * The method handles DONE and ERROR events.
 * A DONE event will trigger the retrieving the read byte, and sending a request to read the following byte. The EDID is read until the block is done and then the reading moves to the next block.
 *  When the block is successfully read, it is sent to be parsed.
 * @param dev Device structure
 * @param hpd on or off
 * @param state of the HDCP engine interrupts
 * @param param to be returned to application:
 * 		no of KSVs in KSV LIST if KSV_LIST_EVENT
 * 		1 (engaged) 0 (fail) if HDCP_EGNAGED_EVENT
 * @return the state of which the event was handled (FALSE for fail)
 */
/* @param ksvHandler Handler to call when KSV list is ready*/
u8 hdcp_event_handler(hdmi_tx_dev_t *dev, int *param);

/**
 * Enable/disable HDCP 1.4
 * @param dev Device structure
 * @param enable
 */
void hdcp_rxdetect(hdmi_tx_dev_t *dev, u8 enable);

/**
 * Enter or exit AV mute mode
 * @param dev Device structure
 * @param enable the HDCP AV mute
 */
void hdcp_av_mute(hdmi_tx_dev_t *dev, int enable);

/**
 * Bypass data encryption stage
 * @param dev Device structure
 * @param bypass the HDCP AV mute
 */
//void hdcp_BypassEncryption(hdmi_tx_dev_t *dev, int bypass);

void hdcp_sw_reset(hdmi_tx_dev_t *dev);


/**
 *@param dev Device structure
 * @param disable the HDCP encrption
 */
void hdcp_disable_encryption(hdmi_tx_dev_t *dev, int disable);

/**
 * @param baseAddr base address of HDCP module registers
 * @return HDCP interrupts state  
 */
u8 hdcp_interrupt_status(hdmi_tx_dev_t *dev);

/** 
 * Clear HDCP interrupts
 * @param dev Device structure
 * @param value mask of interrupts to clear
 * @return TRUE if successful
 */
int hdcp_interrupt_clear(hdmi_tx_dev_t *dev, u8 value);


#endif	/* API_HDCP_H_ */

/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef PACKETS_H_
#define PACKETS_H_

#include "../hdmitx_dev.h"
#include "util/types.h"
#include "product.h"
#include "video_params.h"
#include "core/audio_params.h"

#define PACKET_HDMIVIDEOFORMAT_MASK 0xE0
#define PACKET_3D_STRUCTURE_MASK    0xF0
#define PACKET_3D_EXT_DATA_MASK     0xF0
#define PACKET_HDMI_VIC_MASK   	    0xFF

/**
 * Initialize the packets package. Reset local variables.
 * @param dev Device structure
 * @return TRUE when successful
 */
int packets_Initialize(hdmi_tx_dev_t *dev);

/**
 * Configure Source Product Description, Vendor Specific and Auxiliary
 * Video InfoFrames.
 * @param dev Device structure
 * @param video  Video Parameters to set up AVI InfoFrame (and all
 * other video parameters)
 * @param prod Description of Vendor and Product to set up Vendor
 * Specific InfoFrame and Source Product Description InfoFrame
 * @return TRUE when successful
 */
int packets_Configure(hdmi_tx_dev_t *dev, videoParams_t * video,
		      productParams_t * prod);

/**
 * Configure Audio Content Protection packets.
 * @param type Content protection type (see HDMI1.3a Section 9.3)
 * @param fields  ACP Type Dependent Fields
 * @param length of the ACP fields
 * @param autoSend Send Packets Automatically
 */
void packets_AudioContentProtection(hdmi_tx_dev_t *dev, u8 type, const u8 * fields,
				    u8 length, u8 autoSend);

/**
 * Configure ISRC 1 & 2 Packets
 * @param dev Device structure
 * @param initStatus Initial status which the packets are sent with (usually starting position)
 * @param codes ISRC codes array
 * @param length of the ISRC codes array
 * @param autoSend Send ISRC Automatically
 * @note Automatic sending does not change status automatically, it does the insertion of the packets in the data
 * islands.
 */
void packets_IsrcPackets(hdmi_tx_dev_t *dev, u8 initStatus, const u8 * codes,
			 u8 length, u8 autoSend);

/**
 * Send/stop sending AV Mute in the General Control Packet
 * @param dev Device structure
 * @param enable (TRUE) /disable (FALSE) the AV Mute
 */
void packets_AvMute(hdmi_tx_dev_t *dev, u8 enable);

/**
 * Set ISRC status that is changing during play back depending on position (see HDMI 1.3a Section 8.8)
 * @param dev Device structure
 * @param status the ISRC status code according to position of track
 */
void packets_IsrcStatus(hdmi_tx_dev_t *dev, u8 status);


/**
 * Stop sending ACP packets when in auto send mode
 * @param dev Device structure
 */
void packets_StopSendAcp(hdmi_tx_dev_t *dev);

/**
 * Stop sending ISRC 1 & 2 packets when in auto send mode (ISRC 2 packets cannot be send without ISRC 1)
 * @param dev Device structure
 */
void packets_StopSendIsrc1(hdmi_tx_dev_t *dev);

/**
 * Stop sending ISRC 2 packets when in auto send mode
 * @param dev Device structure
 */
void packets_StopSendIsrc2(hdmi_tx_dev_t *dev);

/**
 * Stop sending Source Product Description InfoFrame packets when in auto send mode
 * @param dev Device structure
 */
void packets_StopSendSpd(hdmi_tx_dev_t *dev);

/**
 * Stop sending Vendor Specific InfoFrame packets when in auto send mode
 * @param dev Device structure
 */
void packets_StopSendVsd(hdmi_tx_dev_t *dev);

/**
 * Disable all metadata packets from being sent automatically. (ISRC 1& 2, ACP, VSD and SPD)
 * @param dev Device structure
 */
void packets_DisableAllPackets(hdmi_tx_dev_t *dev);

/**
 * Configure Vendor Specific InfoFrames.
 * @param dev Device structure
 * @param oui Vendor Organisational Unique Identifier 24 bit IEEE
 * Registration Identifier
 * @param payload Vendor Specific Info Payload
 * @param length of the payload array
 * @param autoSend Start send Vendor Specific InfoFrame automatically
 */
int packets_VendorSpecificInfoFrame(hdmi_tx_dev_t *dev, u32 oui, const u8 * payload, u8 length, u8 autoSend);

/**
 * Configure Colorimetry packets
 * @param dev Device structure
 * @param video Video information structure
 */
void packets_colorimetry_config(hdmi_tx_dev_t *dev, videoParams_t * video);


#endif	/* PACKETS_H_ */

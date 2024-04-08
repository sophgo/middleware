// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "core/packets.h"
#include "core/fc_packets.h"
#include "core/fc_gamut.h"
#include "core/fc_acp.h"
#include "core/fc_audio_info.h"
#include "core/fc_avi.h"
#include "core/fc_isrc.h"
#include "core/fc_spd.h"
#include "core/fc_vsd.h"
#include "frame_composer/frame_composer_reg.h"
#include "core/audio_multistream.h"
#include "util/error.h"
#include "util/log.h"
#include "util/util.h"
#include "edid/edid.h"
#include "general_ops.h"


#define ACP_PACKET_SIZE 	16
#define ISRC_PACKET_SIZE 	16


int packets_Initialize(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	packets_DisableAllPackets(dev);
	return TRUE;
}

int packets_Configure(hdmi_tx_dev_t *dev, videoParams_t * video, productParams_t * prod)
{
	//TODO: check implementation
	//u8 send3d = FALSE;
	LOG_TRACE();

	if(dev->snps_hdmi_ctrl.hdmi_on == 0){
		LOGGER(SNPS_WARN, "DVI mode selected: packets not configured");
		return TRUE;
	}

	if (video->mHdmiVideoFormat == HDMI_3D_FORMAT) {
		LOGGER(SNPS_DEBUG,"%s:3D packet configuration", __func__);
		fc_packets_AutoSend(dev,  0, VSD_TX);	/* prevent sending half the info. */
		// frame packing || tab || sbs
		if ((video->m3dStructure == FRAME_PACKING_3D)  ||
		    (video->m3dStructure == TOP_AND_BOTTOM_3D) ||
		    (video->m3dStructure == SIDE_BY_SIDE_3D)) {
			u8 packet_data[3] = {0,0,0}; //PB4-PB6
			packet_data[0] = set8(0, PACKET_HDMIVIDEOFORMAT_MASK, video->mHdmiVideoFormat);
			packet_data[1] = set8(0, PACKET_3D_STRUCTURE_MASK, video->m3dStructure);
			packet_data[2] = set8(0, PACKET_3D_EXT_DATA_MASK,  video->m3dExtData);


			packets_VendorSpecificInfoFrame(dev, HDMI_LICENSING_LLC_OUI, packet_data, sizeof(packet_data), 1);
			//send3d = TRUE;
		}
		else {
			error_set(ERR_3D_STRUCT_NOT_SUPPORTED);
			LOGGER(SNPS_ERROR,"%s:3D structure not supported %d", __func__, video->m3dStructure);
			return FALSE;
		}
		fc_packets_AutoSend(dev, 1, VSD_TX);
	} else if (video->mHdmiVideoFormat == HDMI_EXT_RES_FORMAT ) {
		u8 packet_data[3] = {0,0,0}; //PB4-PB6
		LOGGER(SNPS_DEBUG,"%s:4k packet configuration", __func__);
		fc_packets_AutoSend(dev,  0, VSD_TX);	/* prevent sending half the info. */

		packet_data[0] = set8(0, PACKET_HDMIVIDEOFORMAT_MASK, video->mHdmiVideoFormat);
		//TODO: correct this - use the hdmivic information instead
		packet_data[1] = set8(0, PACKET_HDMI_VIC_MASK, videoParams_GetHdmiVicCode(video->mDtd.mCode));
		packet_data[2] = 0;

		packets_VendorSpecificInfoFrame(dev, HDMI_LICENSING_LLC_OUI, packet_data, sizeof(packet_data), 1);

		fc_packets_AutoSend(dev, 1, VSD_TX);
	} else {
		fc_packets_AutoSend(dev,  0, VSD_TX);	//stop VSD packets
	}

	/*
	if (prod != 0) {
		fc_spd_info_t spd_data;
		spd_data.vName    = prod->mVendorName;
		spd_data.vLength  = prod->mVendorNameLength;
		spd_data.pName    = prod->mProductName;
		spd_data.pLength  = prod->mProductNameLength;
		spd_data.code     = prod->mSourceType;
		spd_data.autoSend = 1;

		u32 oui = prod->mOUI;
		u8 *vendor_payload = prod->mVendorPayload;
		u8 payload_length  = prod->mVendorPayloadLength;

		fc_spd_config(dev, &spd_data);

		packets_VendorSpecificInfoFrame(dev, oui, vendor_payload, payload_length, 1);
	}
	else {
		LOGGER(SNPS_WARN,"No product info provided: not configured");
	}*/

	fc_packets_metadata_config(dev);

	// default phase 1 = true
	dev_write_mask(dev, FC_GCP, FC_GCP_DEFAULT_PHASE_MASK, ((video->mPixelPackingDefaultPhase == 1) ? 1 : 0));

	fc_gamut_config(dev);

	fc_avi_config(dev, video);

	/** Colorimetry */
	packets_colorimetry_config(dev, video);

	return TRUE;
}

void packets_AudioContentProtection(hdmi_tx_dev_t *dev, u8 type, const u8 * fields, u8 length, u8 autoSend)
{
	u8 newFields[ACP_PACKET_SIZE];
	u16 i = 0;
	LOG_TRACE();
	fc_packets_AutoSend(dev, 0, ACP_TX);

	fc_acp_type(dev, type);

	for (i = 0; i < length; i++) {
		newFields[i] = fields[i];
	}
	if (length < ACP_PACKET_SIZE) {
		for (i = length; i < ACP_PACKET_SIZE; i++) {
			newFields[i] = 0;	/* Padding */
		}
		length = ACP_PACKET_SIZE;
	}
	fc_acp_type_dependent_fields(dev, newFields, length);
	if (!autoSend) {
		fc_packets_ManualSend(dev, ACP_TX);
	} else {
		fc_packets_AutoSend(dev, autoSend, ACP_TX);
	}

}

void packets_IsrcPackets(hdmi_tx_dev_t *dev, u8 initStatus, const u8 * codes, u8 length, u8 autoSend)
{
	u16 i = 0;
	u8 newCodes[ISRC_PACKET_SIZE * 2];
	LOG_TRACE();

	fc_packets_AutoSend(dev, 0, ISRC1_TX);
	fc_packets_AutoSend(dev, 0, ISRC2_TX);

	fc_isrc_status(dev, initStatus);

	for (i = 0; i < length; i++) {
		newCodes[i] = codes[i];
	}

	if (length > ISRC_PACKET_SIZE) {
		for (i = length; i < (ISRC_PACKET_SIZE * 2); i++) {
			newCodes[i] = 0;	/* Padding */
		}
		length = (ISRC_PACKET_SIZE * 2);

		fc_isrc_isrc2_codes(dev, newCodes + (ISRC_PACKET_SIZE * sizeof(u8)), length - ISRC_PACKET_SIZE);
		fc_isrc_cont(dev, 1);

		fc_packets_AutoSend(dev, autoSend, ISRC2_TX);

		if (!autoSend) {
			fc_packets_ManualSend(dev, ISRC2_TX);
		}
	}
	if (length < ISRC_PACKET_SIZE) {
		for (i = length; i < ISRC_PACKET_SIZE; i++) {
			newCodes[i] = 0;	/* Padding */
		}

		length = ISRC_PACKET_SIZE;

		fc_isrc_cont(dev, 0);
	}

	fc_isrc_isrc1_codes(dev, newCodes, length);	/* first part only */
	fc_isrc_valid(dev, 1);

	fc_packets_AutoSend(dev, autoSend, ISRC1_TX);

	if (!autoSend) {
		fc_packets_ManualSend(dev, ISRC1_TX);
	}
}

void packets_AvMute(hdmi_tx_dev_t *dev, u8 enable)
{
	LOG_TRACE1(enable);
	dev_write_mask(dev, FC_GCP, FC_GCP_SET_AVMUTE_MASK, (enable ? 1 : 0));
	dev_write_mask(dev, FC_GCP, FC_GCP_CLEAR_AVMUTE_MASK, (enable ? 0 : 1));
}

void packets_IsrcStatus(hdmi_tx_dev_t *dev, u8 status)
{
	LOG_TRACE();
	fc_isrc_status(dev, status);
}

void packets_StopSendAcp(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	fc_packets_AutoSend(dev, 0, ACP_TX);
}

void packets_StopSendIsrc1(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	fc_packets_AutoSend(dev, 0, ISRC1_TX);
	fc_packets_AutoSend(dev, 0, ISRC2_TX);
}

void packets_StopSendIsrc2(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	fc_isrc_cont(dev, 0);
	fc_packets_AutoSend(dev, 0, ISRC2_TX);
}

void packets_StopSendSpd(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	fc_packets_AutoSend(dev, 0, SPD_TX);
}

void packets_StopSendVsd(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	fc_packets_AutoSend(dev, 0, VSD_TX);
}

void packets_DisableAllPackets(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	fc_packets_disable_all(dev);
}

int packets_VendorSpecificInfoFrame(hdmi_tx_dev_t *dev, u32 oui, const u8 * payload, u8 length, u8 autoSend)
{
	LOG_TRACE();
	fc_packets_AutoSend(dev,  0, VSD_TX);	/* prevent sending half the info. */
	fc_vsd_vendor_OUI(dev, oui);
	if (fc_vsd_vendor_payload(dev, payload, length)) {
		return FALSE;	/* DEFINE ERROR */
	}
	if (autoSend) {
		fc_packets_AutoSend(dev, autoSend, VSD_TX);
	} else {
		fc_packets_ManualSend(dev, VSD_TX);
	}
	return TRUE;
}

u8 packets_AudioMetaDataPacket(hdmi_tx_dev_t *dev, audioMetaDataPacket_t * audioMetaDataPckt)
{
	halAudioMultistream_MetaDataPacket_Header(dev, audioMetaDataPckt);
	halAudioMultistream_MetaDataPacketBody(dev, audioMetaDataPckt);
	return TRUE;
}

void packets_colorimetry_config(hdmi_tx_dev_t *dev, videoParams_t * video)
{
	u8 gamut_metadata[28] = {0};
	int gdb_color_space = 0;

	fc_gamut_enable_tx(dev, 0);

	if(video->mColorimetry == EXTENDED_COLORIMETRY){
		if(video->mExtColorimetry == XV_YCC601){
			gdb_color_space = 1;
		}
		else if(video->mExtColorimetry == XV_YCC709){
			gdb_color_space = 2;
			LOGGER(SNPS_WARN, "xv ycc709");
		}
		else if(video->mExtColorimetry == S_YCC601){
			gdb_color_space = 3;
		}
		else if(video->mExtColorimetry == ADOBE_YCC601){
			gdb_color_space = 3;
		}
		else if(video->mExtColorimetry == ADOBE_RGB){
			gdb_color_space = 3;
		}

		if(video->mColorimetryDataBlock == TRUE){
			gamut_metadata[0] = (1 << 7) | gdb_color_space;
			fc_gamut_packet_config(dev, gamut_metadata, (sizeof(gamut_metadata) / sizeof(u8)));
		}
	}
}

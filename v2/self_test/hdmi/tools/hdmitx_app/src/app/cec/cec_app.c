// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "includes.h"
#include "hdmi_tx_app.h"
#include "app/edid/edid.h"
#include "app/video/video.h"
#include "app/hdcp/hdcp.h"
#include "app/edid/edid_parser.h"
#include "app/cec/cec_app.h"
#include "core/interrupt/interrupt_reg.h"


int cec_set_physical_address(struct hdmi_tx_app *app, sink_edid_t * sink)
{
	LOGGER(SNPS_TRACE,"");

	if(sink->edid_mHdmivsdb.mValid)
	{
		app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.physical_address = sink->edid_mHdmivsdb.mPhysicalAddress;
		LOGGER(SNPS_NOTICE, "CEC Physical Address: %x", app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.physical_address);
		return 1;
	}else
	{
		LOGGER(SNPS_ERROR, "CEC Physical Address not assigned by the Sink, check EDID HDMI VSDB");
		return -1;
	}
}

int cec_set_device_params(hdmi_tx_dev_t *hdmi_tx, cec_device_category_t device_category)
{
	switch (device_category)
	{
	case TV_DEVICE:
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.device_category = TV;
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.number_of_supported_addresses = 1;
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.supported_addresses[0] = TV;
		break;
	case RECORDING_DEVICE:
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.device_category = RECORDING_DEVICE;
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.number_of_supported_addresses = 3;
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.supported_addresses[0] = RECORDING_DEVICE_1;
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.supported_addresses[1] = RECORDING_DEVICE_2;
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.supported_addresses[2] = RECORDING_DEVICE_3;
		break;
	case TUNER_DEVICE:
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.device_category = TUNER_DEVICE;
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.number_of_supported_addresses = 4;
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.supported_addresses[0] = TUNER_1;
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.supported_addresses[1] = TUNER_2;
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.supported_addresses[2] = TUNER_3;
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.supported_addresses[2] = TUNER_4;
		break;
	case AUDIO_SUBSYSTEM_DEVICE:
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.device_category = AUDIO_SUBSYSTEM_DEVICE;
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.number_of_supported_addresses = 1;
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.supported_addresses[0] = AUDIO_SUBSYSTEM;
		break;
	case PLAYBACK_DEVICE:
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.device_category = PLAYBACK_DEVICE;
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.number_of_supported_addresses = 3;
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.supported_addresses[0] = PLAYBACK_DEVICE_1;
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.supported_addresses[1] = PLAYBACK_DEVICE_2;
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.supported_addresses[2] = PLAYBACK_DEVICE_3;
		break;
	case SPECIFIC_USE_DEVICE:
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.device_category = SPECIFIC_USE;
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.number_of_supported_addresses = 1;
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.supported_addresses[0] = SPECIFIC_USE;
		break;
	default:
		return -1;
		break;
	}
	hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.cec_state = CEC_NO_PHYSICAL_ADDR;
	//hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.physical_address = 0;
	hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address = 0;

	return 1;
}

int cec_check_opcodes_answer(hdmi_tx_dev_t *hdmi_tx, char *buffer)
{
	if(buffer == NULL)
		return -1;
	switch(buffer[0])
	{
	case CEC_OPCODE_ACTIVE_SOURCE:
		return 0;
		break;
	default:
		return 1;
	}
}

int cec_update_menu_language(hdmi_tx_dev_t *hdmi_tx, char *buffer)
{
	char *language = NULL;
	int retval = 1;

	language = &buffer[2];
	if(strcmp(language, "por") == 0)
		strcpy(hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.language, language);
	else
		if(strcmp(language, "eng") == 0)
			strcpy(hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.language, language);
		else
			retval = 0;

	if(retval)
		LOGGER(SNPS_NOTICE, "CEC language set to %s", hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.language);
	else
		LOGGER(SNPS_NOTICE, "CEC language not supported or incorrect");
	return retval;
}

int cec_check_logical_address(hdmi_tx_dev_t *hdmi_tx, char *buffer)
{
	if(buffer == NULL)
		return -1;
	if((buffer[0] & 0xf) == hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address)
		return 1;
	else
		return 0;
}

int cec_check_broadcast_logical_address(hdmi_tx_dev_t *hdmi_tx, char *buffer)
{
	if(buffer == NULL)
		return -1;
	if((buffer[0] & 0xf) == 0xf)
		return 1;
	else
		return 0;
}

int cec_check_physical_address(hdmi_tx_dev_t *hdmi_tx, char *buffer)
{
	if(((buffer[2]) == (hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.physical_address >> 8)) &&
			(buffer[3] == (hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.physical_address & 0xff)))
		return 1;
	else
		return 0;
}

int cec_check_message_size(unsigned char opcode, int size)
{
	int retval = 0;
	/* acording to the CEC HDMI 1.4b spec, the DUT */
	/* should interpret a message if it haves the */
	/* the correct number of parameters of higher */
	/* (check CTS for CEC 9.4-2) */
	switch (opcode) {

	case CEC_OPCODE_GET_CEC_VERSION:
	case CEC_OPCODE_REQUEST_ACTIVE_SOURCE:
	case CEC_OPCODE_INACTIVE_SOURCE:
	case CEC_OPCODE_GIVE_OSD_NAME:
	case CEC_OPCODE_GIVE_DEVICE_POWER_STATUS:
	case CEC_OPCODE_GIVE_PHYSICAL_ADDRESS:
	case CEC_OPCODE_STANDBY:
	case CEC_OPCODE_ABORT:
		if (size >= 2)
			retval = 1;
		break;
	case CEC_OPCODE_PLAY:
	case CEC_OPCODE_DECK_CONTROL:
	case CEC_OPCODE_ROUTING_INFORMATION:
	case CEC_OPCODE_REPORT_POWER_STATUS:
	case CEC_OPCODE_GIVE_DECK_STATUS:
		if (size >= 3)
			retval = 1;
		break;
	case CEC_OPCODE_SET_STREAM_PATH:
	case CEC_OPCODE_ACTIVE_SOURCE:
	case CEC_OPCODE_FEATURE_ABORT:
		if (size >= 4)
			retval = 1;
		break;
	case CEC_OPCODE_ROUTING_CHANGE:
	case CEC_OPCODE_SET_MENU_LANGUAGE:
		if (size >= 5)
			retval = 1;
		break;
	default:
		LOGGER(SNPS_ERROR, "CEC: Opcode not supported");
		retval = -1;
		break;
	}

	return retval;
}

int cec_update_transmition_buffer(hdmi_tx_dev_t *hdmi_tx, char *buffer, int lenght, cec_logical_address_t destination_addr)
{
	int i = 0;
	while(i < lenght)
	{
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.transmit_buffer[i] = buffer[i];
		i++;
	}
	hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.destination_addr = destination_addr;
	hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.transmit_size = lenght;
	return 1;
}

int cec_validate_deck_message_parameters(hdmi_tx_dev_t *hdmi_tx, char *buffer)
{
	switch(buffer[2])
	{
	/* validation of parameters for <Deck Control> Message */
	case DECK_CTRL_SKIP_FW_WIND:
	case DECK_CTRL_SKIP_REVERSE_REWIND:
	case DECK_CTRL_STOP:
	case DECK_CTRL_EJECT:
		/* validation of parameters for <Play> Message */
	case PLAY_FOWARD:
	case PLAY_REVERSE:
	case PLAY_STILL:
	case FAST_FOWARD_MIN_SPEED:
	case FAST_FOWARD_MEDIUM_SPEED:
	case FAST_FOWARD_MAX_SPEED:
	case FAST_REVERSE_MIN_SPEED:
	case FAST_REVERSE_MEDIUM_SPEED:
	case FAST_REVERSE_MAX_SPEED:
	case SLOW_FOWARD_MIN_SPEED:
	case SLOW_FOWARD_MEDIUM_SPEED:
	case SLOW_FOWARD_MAX_SPEED:
	case SLOW_REVERSE_MIN_SPEED:
	case SLOW_REVERSE_MEDIUM_SPEED:
	case SLOW_REVERSE_MAX_SPEED:
		return 1;
		break;
	default:
		return 0;
		break;
	}
}

char *cec_deck_status_to_string(cec_deck_status_t deck_status)
{
	switch (deck_status)
	{
	case DECK_INFO_PLAY              : return "PLAY              "; break;
	case DECK_INFO_RECORD            : return "RECORD            "; break;
	case DECK_INFO_PLAY_REVERSE      : return "PLAY_REVERSE      "; break;
	case DECK_INFO_STILL             : return "STILL             "; break;
	case DECK_INFO_SLOW              : return "SLOW              "; break;
	case DECK_INFO_SLOW_REVERSE      : return "SLOW_REVERSE      "; break;
	case DECK_INFO_FAST_FORWARD      : return "FAST_FORWARD      "; break;
	case DECK_INFO_FAST_REVERSE      : return "FAST_REVERSE      "; break;
	case DECK_INFO_NO_MEDIA          : return "NO_MEDIA          "; break;
	case DECK_INFO_STOP              : return "STOP              "; break;
	case DECK_INFO_SKIP_FORWARD_WIND : return "SKIP_FORWARD_WIND "; break;
	case DECK_INFO_SKIP_REVERSE_RWIND: return "SKIP_REVERSE_RWIND"; break;
	case DECK_INFO_INDEX_SEARCH_FW   : return "INDEX_SEARCH_FW   "; break;
	case DECK_INFO_INDEX_SEARCH_RV   : return "INDEX_SEARCH_RV   "; break;
	case DECK_INFO_OTHER_STATUS      : return "OTHER_STATUS      "; break;
	default				     : return "UNDEFINED		 "; break;
	}
}


int cec_deck_state_manager(hdmi_tx_dev_t *hdmi_tx, char *buffer)
{
	cec_deck_status_t status = 0;
	switch(buffer[2])
	{
	/* parameters for <Deck Control> Message */
	case DECK_CTRL_SKIP_FW_WIND:
		status = DECK_INFO_SKIP_FORWARD_WIND;
		break;
	case DECK_CTRL_SKIP_REVERSE_REWIND:
		status = DECK_INFO_SKIP_REVERSE_RWIND;
		break;
	case DECK_CTRL_STOP:
		status = DECK_INFO_STOP;
		break;
	case DECK_CTRL_EJECT:
		status = DECK_INFO_NO_MEDIA;
		break;
		/* parameters for <Play> Message */
	case PLAY_FOWARD:
		status = DECK_INFO_PLAY;
		break;
	case PLAY_REVERSE:
		status = DECK_INFO_SLOW_REVERSE;
		break;
	case PLAY_STILL:
		status = DECK_INFO_STILL;
		break;
	case FAST_FOWARD_MIN_SPEED:
	case FAST_FOWARD_MEDIUM_SPEED:
	case FAST_FOWARD_MAX_SPEED:
		status = DECK_INFO_FAST_FORWARD;
		break;

	case FAST_REVERSE_MIN_SPEED:
	case FAST_REVERSE_MEDIUM_SPEED:
	case FAST_REVERSE_MAX_SPEED:
		status = DECK_INFO_FAST_REVERSE;
		break;
	case SLOW_FOWARD_MIN_SPEED:
	case SLOW_FOWARD_MEDIUM_SPEED:
	case SLOW_FOWARD_MAX_SPEED:
		status = DECK_INFO_SLOW;
		break;
	case SLOW_REVERSE_MIN_SPEED:
	case SLOW_REVERSE_MEDIUM_SPEED:
	case SLOW_REVERSE_MAX_SPEED:
		status = DECK_INFO_SLOW_REVERSE;
		break;
	default:
		LOGGER(SNPS_ERROR, "CEC: received incorrect deck status change: 0x%x", buffer[2]);
		return 0;
		break;
	}
	if(hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.deck_status != status)
	{
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.deck_status = status;
		LOGGER(SNPS_NOTICE, "CEC: Deck status changed to: 0x%x %s", status, cec_deck_status_to_string(status));
		return 1;
	}
	else
	{
		LOGGER(SNPS_NOTICE, "CEC: Deck status already set to: 0x%x %s", status, cec_deck_status_to_string(status));
		return 1;
	}
}

int cec_message_handler(hdmi_tx_dev_t *hdmi_tx)
{
	int destination_addr = 0;
	char *received_buffer = NULL;
	char *tx_buffer = NULL;
	int lenght = 0;

	tx_buffer = malloc(sizeof(char)*16);
	memset(tx_buffer,0,16);

	received_buffer = &hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.received_buffer[0];
	lenght = hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.received_size;

	/* check for message completion */
	if(cec_check_message_size(received_buffer[1], lenght) == 0)
	{
		LOGGER(SNPS_ERROR, "CEC: Wrong number of parameters for message with opcode 0x%x.", received_buffer[1]);
		goto cleanup_exit;
	}

	LOGGER(SNPS_NOTICE,"CEC: received OPCODE 0x%x\n", received_buffer[1]);
	switch(received_buffer[1]){

	case CEC_OPCODE_GIVE_PHYSICAL_ADDRESS:
		if(cec_check_logical_address(hdmi_tx, received_buffer) == 0)
			goto cleanup_exit;
		/* send broadcast message <Report Physical Address> */
		tx_buffer[0]=CEC_OPCODE_REPORT_PHYSICAL_ADDRESS;
		tx_buffer[1]=hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.physical_address >> 8;
		tx_buffer[2]=hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.physical_address;
		tx_buffer[3]=hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address;
		cec_update_transmition_buffer(hdmi_tx, tx_buffer, 4, UNREGISTERED_BROADCAST);
		if(!cec_ctrlSendFrame(hdmi_tx, tx_buffer, 4,
				hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address, UNREGISTERED_BROADCAST))
			LOGGER(SNPS_ERROR, "CEC: Error sending <Report Physical Address>.");
		break;

	case CEC_OPCODE_STANDBY:
		if(hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.power_state != STANDBY)
		{
			if(hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.active_source)
			{
				/* send broadcast message <Inactive Source> */
				tx_buffer[0]=CEC_OPCODE_INACTIVE_SOURCE;
				tx_buffer[1] = hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.physical_address >> 8;
				tx_buffer[2] = hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.physical_address;
				cec_update_transmition_buffer(hdmi_tx, tx_buffer, 3, TV);
				if(!cec_ctrlSendFrame(hdmi_tx, tx_buffer, 3,
						hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address, TV))
					LOGGER(SNPS_ERROR, "CEC: Error sending <Standby>.");
			}
			/* send broadcast message <Standby> */
			tx_buffer[0]=CEC_OPCODE_STANDBY;
			cec_update_transmition_buffer(hdmi_tx, tx_buffer, 1, UNREGISTERED_BROADCAST);
			if(!cec_ctrlSendFrame(hdmi_tx, tx_buffer, 1,
					hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address, UNREGISTERED_BROADCAST))
				LOGGER(SNPS_ERROR, "CEC: Error sending <Standby>.");

			hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.power_state = STANDBY;
			hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.active_source = 0;
			LOGGER(SNPS_NOTICE,"CEC: Device Set to standby mode");
		}
		else
		{
			LOGGER(SNPS_NOTICE, "CEC: Device already set to standby mode");
		}
		break;


	case CEC_OPCODE_ABORT:
		if(cec_check_logical_address(hdmi_tx, received_buffer) == 0)
			goto cleanup_exit;
		destination_addr = received_buffer[0] >> 4;
		tx_buffer[0]=CEC_OPCODE_FEATURE_ABORT;
		tx_buffer[1]=0xff; // opcode 0
		tx_buffer[2]=0x00;
		cec_update_transmition_buffer(hdmi_tx, tx_buffer, 3, destination_addr);
		if(cec_ctrlSendFrame(hdmi_tx, tx_buffer, 3,
				hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address, destination_addr) < 0)
			LOGGER(SNPS_ERROR, "CEC: Error sending <Feature Abort>.");
		cec_ctrlSendFrame(hdmi_tx, tx_buffer, 3,
				hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address, destination_addr);
		break;

	case CEC_OPCODE_REQUEST_ACTIVE_SOURCE:
		if(cec_check_broadcast_logical_address(hdmi_tx, received_buffer) == 0)
			goto cleanup_exit;
		if(hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.active_source)
		{
			tx_buffer[0]=CEC_OPCODE_ACTIVE_SOURCE;
			tx_buffer[1]=hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.physical_address >> 8;
			tx_buffer[2]=hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.physical_address;
			cec_update_transmition_buffer(hdmi_tx, tx_buffer, 3, UNREGISTERED_BROADCAST);
			if(!cec_ctrlSendFrame(hdmi_tx, tx_buffer, 3,
					hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address, UNREGISTERED_BROADCAST))
				LOGGER(SNPS_ERROR, "CEC: Error sending <Active Source>.");
		}
		break;

	case CEC_OPCODE_GIVE_DEVICE_POWER_STATUS:
		if(cec_check_logical_address(hdmi_tx, received_buffer) == 0)
			goto cleanup_exit;
		destination_addr = tx_buffer[0] >> 4;
		tx_buffer[0]=CEC_OPCODE_REPORT_POWER_STATUS;
		tx_buffer[1]=hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.power_state;
		cec_update_transmition_buffer(hdmi_tx, tx_buffer, 2, destination_addr);
		if(!cec_ctrlSendFrame(hdmi_tx, tx_buffer, 2,
				hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address, destination_addr))
			LOGGER(SNPS_ERROR, "CEC: Error sending <Report Power status>.");
		break;

	case CEC_OPCODE_SET_STREAM_PATH:
		if(cec_check_broadcast_logical_address(hdmi_tx, received_buffer) == 0)
			goto cleanup_exit;
		if(cec_check_physical_address(hdmi_tx, received_buffer) == 0)
			goto cleanup_exit;
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.active_source = 1;
		LOGGER(SNPS_NOTICE, "CEC: SNPS TX defined as active source.");
		tx_buffer[0]=CEC_OPCODE_ACTIVE_SOURCE;
		tx_buffer[1]=hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.physical_address >> 8;
		tx_buffer[2]=hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.physical_address;
		cec_update_transmition_buffer(hdmi_tx, tx_buffer, 3, UNREGISTERED_BROADCAST);
		if(!cec_ctrlSendFrame(hdmi_tx, tx_buffer, 3,
				hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address, UNREGISTERED_BROADCAST))
			LOGGER(SNPS_ERROR, "CEC: Error sending <Set Stream Path>.");
		break;

	case CEC_OPCODE_GET_CEC_VERSION:
		if(cec_check_logical_address(hdmi_tx, received_buffer) == 0)
			goto cleanup_exit;
		destination_addr = received_buffer[0] >> 4;
		tx_buffer[0]=CEC_OPCODE_CEC_VERSION;
		tx_buffer[1]=0x05; // version 1.4b
		cec_update_transmition_buffer(hdmi_tx, tx_buffer, 2, destination_addr);
		if(!cec_ctrlSendFrame(hdmi_tx, tx_buffer, 2,
				hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address, destination_addr))
			LOGGER(SNPS_ERROR, "CEC: Error sending <CEC Version>.");
		break;

	case CEC_OPCODE_GIVE_OSD_NAME:
		if(cec_check_logical_address(hdmi_tx, received_buffer) == 0)
			goto cleanup_exit;
		destination_addr = received_buffer[0] >> 4;
		/* do not accept a request from a non-registered address */
		if(destination_addr == 0xf)
			goto cleanup_exit;
		tx_buffer[0]=CEC_OPCODE_SET_OSD_NAME;
		strcat(tx_buffer,"SNPS_BluRay");
		cec_update_transmition_buffer(hdmi_tx, tx_buffer, 12, destination_addr);
		if(!cec_ctrlSendFrame(hdmi_tx, tx_buffer, 12,
				hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address, destination_addr))
			LOGGER(SNPS_ERROR, "CEC: Error sending <Set OSD Name>.");
		break;

	case CEC_OPCODE_SET_MENU_LANGUAGE:
		if(cec_check_broadcast_logical_address(hdmi_tx, received_buffer) == 0)
			goto cleanup_exit;
		destination_addr = received_buffer[0] >> 4;
		/* do not accept a request from a non-registered address */
		if(destination_addr == 0xf)
			goto cleanup_exit;
		if(!cec_update_menu_language(hdmi_tx, received_buffer));
		goto cleanup_exit;

		break;

	case CEC_OPCODE_GIVE_DECK_STATUS:
		if(cec_check_logical_address(hdmi_tx, received_buffer) == 0)
			goto cleanup_exit;
		destination_addr = tx_buffer[0] >> 4;
		/* do not accept a request from a non-registered address */
		if(destination_addr == 0xf)
			goto cleanup_exit;
		/* extracting Give Status information */
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.deck_status =
				(received_buffer[2] == GIVE_STATUS_ON ? GIVE_STATUS_ON :
						received_buffer[2] == GIVE_STATUS_ONCE ? GIVE_STATUS_ONCE : GIVE_STATUS_OFF);
		/* preparing answer */
		tx_buffer[0]=CEC_OPCODE_DECK_STATUS;
		tx_buffer[1]=hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.deck_status;
		strcpy(hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.transmit_buffer, tx_buffer);
		cec_update_transmition_buffer(hdmi_tx, tx_buffer, 2, destination_addr);
		if(!cec_ctrlSendFrame(hdmi_tx, tx_buffer, 2,
				hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address, destination_addr))
			LOGGER(SNPS_ERROR, "CEC: Error sending <Deck Status>.");
		break;

	case CEC_OPCODE_DECK_CONTROL:
	case CEC_OPCODE_PLAY:
		if(cec_check_logical_address(hdmi_tx, received_buffer) == 0)
			goto cleanup_exit;
		destination_addr = received_buffer[0] >> 4;
		/* do not accept a request from a non-registered address */
		if(destination_addr == 0xf)
			goto cleanup_exit;
		if(cec_validate_deck_message_parameters(hdmi_tx, received_buffer))
		{
			/* if state changed */
			if(cec_deck_state_manager(hdmi_tx, received_buffer))
			{
				if(hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.give_deck_status == GIVE_STATUS_OFF)
					goto cleanup_exit;
				if(received_buffer[1] == CEC_OPCODE_PLAY)
				{
					tx_buffer[0] = CEC_OPCODE_IMAGE_VIEW_ON;
					cec_update_transmition_buffer(hdmi_tx, tx_buffer, 1, TV);
					if(!cec_ctrlSendFrame(hdmi_tx, tx_buffer, 1,
							hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address, TV))
						LOGGER(SNPS_ERROR, "CEC: Error sending <ImageView On>.\n");
					hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.power_state = ON;
				}
				tx_buffer[0]=CEC_OPCODE_DECK_STATUS;
				tx_buffer[1]=hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.deck_status;
				cec_update_transmition_buffer(hdmi_tx, tx_buffer, 2, destination_addr);
				if(!cec_ctrlSendFrame(hdmi_tx, tx_buffer, 2,
						hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address, destination_addr))
					LOGGER(SNPS_ERROR, "CEC: Error sending <Deck Status>.");
			}
		}
		else
		{
			/* send feature abort */
			destination_addr = received_buffer[0] >> 4;
			tx_buffer[0]=CEC_OPCODE_FEATURE_ABORT;
			tx_buffer[1]=CEC_OPCODE_DECK_CONTROL; // opcode
			tx_buffer[2]=0x04; // refused
			cec_update_transmition_buffer(hdmi_tx, tx_buffer, 3, destination_addr);;
			if(!cec_ctrlSendFrame(hdmi_tx, tx_buffer, 3,
					hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address, destination_addr))
				LOGGER(SNPS_ERROR, "CEC: Error sending <Feature Abort>.");
		}
		break;

	default:
		/* if unrecognized opcode issue an feature abort */
		if(cec_check_logical_address(hdmi_tx, received_buffer) == 0)
			goto cleanup_exit;
		if(cec_check_opcodes_answer(hdmi_tx, received_buffer) == 0)
		{
			if(hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.active_source)
			{
				destination_addr = received_buffer[0] >> 4;
				tx_buffer[0]=CEC_OPCODE_FEATURE_ABORT;
				received_buffer[1] == 0 ? tx_buffer[1]=0xff : 0;
				tx_buffer[2]=0x00;
				cec_update_transmition_buffer(hdmi_tx, tx_buffer, 3, destination_addr);
				if(!cec_ctrlSendFrame(hdmi_tx, tx_buffer, 3,
						hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address, destination_addr))
					LOGGER(SNPS_ERROR, "CEC: Error sending <Feature Abort>.");
			}
		}
		break;
	}
	cleanup_exit:
	free(tx_buffer);
	return 0;
}

int cec_state_manager(hdmi_tx_dev_t *hdmi_tx, int decode_cec_interrupt)
{
	u8 run_state_manager = 0;
	static u8 address_counter = 0;
	static u16 cec_address_candidate = 0;
	char *buffer = NULL;
	int cnt = 0;

	buffer = malloc(sizeof(char)*16);
	memset(buffer,0,16);

	LOGGER(SNPS_DEBUG, "%s cec state %d\n", __func__, hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.cec_state);

	switch (hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.cec_state){
	case CEC_NOT_INIT:
		/* reset data structures to support CEC device as Playback Device */
		cec_set_device_params(hdmi_tx, PLAYBACK_DEVICE);
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.cec_state = CEC_NO_PHYSICAL_ADDR;
		address_counter = 0;
		cec_address_candidate = 0;
		run_state_manager = 1;
		break;
	case CEC_NO_PHYSICAL_ADDR:
		if(hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.physical_address != 0){
			hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.cec_state = CEC_POLL_LOGICAL_ADDR;
			/* set logical address for Unregistered - 15*/
			cec_CfgLogicAddr(hdmi_tx, UNREGISTERED_BROADCAST, 1);
			run_state_manager = 1;
		}
		else
		{

			LOGGER(SNPS_NOTICE,"%s: CEC Physical Address not assigned by the Sink, check EDID HDMI VSDB", __func__);
			run_state_manager = 0;
		}
		break;
	case CEC_POLL_LOGICAL_ADDR:
		if(address_counter < hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.number_of_supported_addresses)
		{
			/* send poll frame with the logical address to be defined */
			cec_address_candidate = hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.supported_addresses[address_counter];
			if(!cec_ctrlSendFrame(hdmi_tx, buffer, 0,
					cec_address_candidate, cec_address_candidate))
				LOGGER(SNPS_ERROR, "CEC: Error sending Poll frame.");
			/* set state to wait for an addr ack */
			hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.cec_state = CEC_AWAITING_ADDR_ACK;
			LOGGER(SNPS_DEBUG,"set state to CEC_AWAITING_ADDR_ACK");
			address_counter ++;
		}
		else
		{
			//LOGGER(SNPS_ERROR, "CEC: No available CEC Logical addresses within this category. Setting address to unregistered.");
			LOGGER(SNPS_ERROR,"CEC: No available CEC Logical addresses within this category. Setting address to unregistered.");
			address_counter = 0;
			hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address = UNREGISTERED_BROADCAST;
			cec_CfgLogicAddr(hdmi_tx, UNREGISTERED_BROADCAST, 1);
			hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.cec_state = CEC_UNREGISTERED;
		}
		run_state_manager = 0;
		break;
	case CEC_AWAITING_ADDR_ACK:
		/* if no one ACKs this address, then registers as its own address */
		if((decode_cec_interrupt & IH_CEC_STAT0_NACK_MASK) != 0)
		{
			LOGGER(SNPS_NOTICE, "CEC: Logical address 0x%x available.", cec_address_candidate);
			hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address = cec_address_candidate;
			cec_CfgLogicAddr(hdmi_tx, cec_address_candidate, 1);
			/* send broadcast message <Report Physical Address> */
			buffer[0]=0x84;
			buffer[1]=hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.physical_address >> 8;
			buffer[2]=hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.physical_address;
			buffer[3]=hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address;
			if(!cec_ctrlSendFrame(hdmi_tx, buffer, 4,
					hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address, UNREGISTERED_BROADCAST))
				LOGGER(SNPS_ERROR, "CEC: Error sending <Report Physical Address>.");
			/* set state to registered address */
			hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.cec_state = CEC_REGISTERED;
			run_state_manager = 1;
		}
		else
		{
			if((decode_cec_interrupt & IH_CEC_STAT0_DONE_MASK))
			{
				hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.cec_state = CEC_POLL_LOGICAL_ADDR;
				run_state_manager = 1;
			}
		}
		break;
	case CEC_REGISTERED:
		LOGGER(SNPS_NOTICE, "CEC Tx Registered, Logical Address: 0x%x", hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address);
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.active_source = 0;
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.deck_status = DECK_CTRL_STOP;
		//cec_ctrlSendFrame(hdmi_tx, cec_address_candidate, 1);
		run_state_manager = 1;
		/* set state to Cec follower */
		hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.cec_state = CEC_FOLLOWER;
		break;
	case CEC_FOLLOWER:
		LOGGER(SNPS_TRACE, "CEC state: follower.");
		run_state_manager = 0;
		/* if there is not an follower error */
		if(((decode_cec_interrupt & IH_CEC_STAT0_ERROR_FOLLOW_MASK) == 0) || ((decode_cec_interrupt & IH_CEC_STAT0_ARB_LOST_MASK) == 0))
		{
			usleep(10);
			/* if failed last message sending, resend */
			if((decode_cec_interrupt & IH_CEC_STAT0_ERROR_INITIATOR_MASK) != 0)
				if(!cec_ctrlSendFrame(hdmi_tx, hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.transmit_buffer,
						hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.transmit_size,
						hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.logical_address, hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.destination_addr))
					LOGGER(SNPS_ERROR, "CEC: Error re-sending last message.");

			if((decode_cec_interrupt & (IH_CEC_STAT0_EOM_MASK)) != 0)
			{
				cnt = cec_msgRx(hdmi_tx, hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.received_buffer, 16, 1);
				hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.received_size = cnt;
				LOGGER(SNPS_NOTICE,"CEC: received %d bytes", cnt);

				int i = 0;
				printf("CEC RX Buffer:");
				for(i=0; i < cnt; i++)
					printf("%02x ", hdmi_tx->snps_hdmi_ctrl.cec_tx_dev.received_buffer[i]);
				printf("\n");

				run_state_manager = cec_message_handler(hdmi_tx);
			}
		}
		break;
	case CEC_INITIATOR:
		LOGGER(SNPS_NOTICE, "CEC state: initiator.\n");
		run_state_manager = 0;
		break;
	case CEC_UNREGISTERED:
		LOGGER(SNPS_NOTICE, "CEC: state unregistered.\n");
		run_state_manager = 0;
		break;
	default:
		run_state_manager = 0;
		break;
	}
	free(buffer);
	return run_state_manager;
}

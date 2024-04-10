/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef HDMI_TX_MAIN_TOOLS_HDMITX_APP_INCLUDE_APP_CEC_APP_H_
#define HDMI_TX_MAIN_TOOLS_HDMITX_APP_INCLUDE_APP_CEC_APP_H_

#include "includes.h"
#include "hdmi_tx_app.h"

/*
 * cec_app.c
 *
 *  Created on: Jun 1, 2016
 *      Author: synopsys
 */


int cec_set_physical_address(struct hdmi_tx_app *app, sink_edid_t * sink);

int cec_set_device_params(hdmi_tx_dev_t *hdmi_tx, cec_device_category_t device_category);

int cec_check_opcodes_answer(hdmi_tx_dev_t *hdmi_tx, char *buffer);

int cec_check_logical_address(hdmi_tx_dev_t *hdmi_tx, char *buffer);

int cec_check_broadcast_logical_address(hdmi_tx_dev_t *hdmi_tx, char *buffer);

int cec_check_physical_address(hdmi_tx_dev_t *hdmi_tx, char *buffer);

int cec_check_message_size(unsigned char opcode, int size);

int cec_update_transmition_buffer(hdmi_tx_dev_t *hdmi_tx, char *buffer, int lenght, cec_logical_address_t destination_addr);

int cec_validate_deck_message_parameters(hdmi_tx_dev_t *hdmi_tx, char *buffer);

int cec_deck_state_manager(hdmi_tx_dev_t *hdmi_tx, char *buffer);

int cec_message_handler(hdmi_tx_dev_t *hdmi_tx);

int cec_update_menu_language(hdmi_tx_dev_t *hdmi_tx, char *buffer);

int cec_state_manager(hdmi_tx_dev_t *hdmi_tx, int decode_cec_interrupt);


#endif /* HDMI_TX_MAIN_TOOLS_HDMITX_APP_INCLUDE_APP_CEC_APP_H_ */

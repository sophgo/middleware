// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "app/hdcp/hdcp.h"

#include <hdmitx_dev.h>

#include "includes.h"
#include "hdmitx_ipk_api/hdmitx_ipk_api.h"
#include "hdmi_tx_app.h"
#include "platform.h"
#include <limits.h>
#include "hdcp/hdcp.h"
#include "app/hdmitx.h"

#include "app/cmd_line/cmd_interface.h"


void reset_hdcp_params(struct hdmi_tx_app *app)
{
	hdcp_params_reset(&app->hdmi_tx, &app->mode.pHdcp);
	app->hdmi_tx.hdcp.maxDevices = app->mode.ksv_devices;
	app->mode.pHdcp.mKsvListBuffer = NULL; //app->mode.ksv_list_buffer;
	app->mode.pHdcp.mAksv = app->mode.dpk_aksv;
	app->mode.pHdcp.mSwEncKey = app->mode.sw_enc_key;
	app->mode.pHdcp.mKeys = app->mode.dpk_keys;

	// Check if the HDCP was previously enabled by the user
	if(app->hdmi_tx.snps_hdmi_ctrl.hdcp_on == 1)
		app->mode.pHdcp.bypass = FALSE;
	else
		app->mode.pHdcp.bypass = TRUE;
}


void update_hdcp_cfg(hdcpParams_t * user, hdcpParams_t * cfg)
{
	if(user->bypass != -1)
		cfg->bypass = user->bypass;

	if(user->mEnable11Feature != -1)
		cfg->mEnable11Feature = user->mEnable11Feature;

	if(user->mRiCheck != -1)
		cfg->mRiCheck = user->mRiCheck;

	if(user->mI2cFastMode != -1)
		cfg->mI2cFastMode = user->mI2cFastMode;

	if(user->mEnhancedLinkVerification != -1)
		cfg->mEnhancedLinkVerification = user->mEnhancedLinkVerification;

	if(user->maxDevices != 0)
		cfg->maxDevices = user->maxDevices;


	//Free the struct in the current configuration and apply the new
	if(user->mKsvListBuffer != NULL){
		if(cfg->mKsvListBuffer != NULL)
			free(cfg->mKsvListBuffer);
		cfg->mKsvListBuffer = user->mKsvListBuffer;
		user->mKsvListBuffer = NULL;
	}

	if(user->mAksv != NULL){
		if(cfg->mAksv != NULL)
			free(cfg->mAksv);
		cfg->mAksv = user->mAksv;
		user->mAksv = NULL;
	}
	if(user->mKeys != NULL){
		if(cfg->mKeys != NULL)
			free(cfg->mKeys);
		cfg->mKeys = user->mKeys;
		user->mKeys = NULL;
	}
	if(user->mSwEncKey != NULL){
		if(cfg->mSwEncKey != NULL)
			free(cfg->mSwEncKey);
		cfg->mSwEncKey = user->mSwEncKey;
		user->mSwEncKey = NULL;
	}
}

/**
 * Keys
 */
void hdcp_init_keys(struct hdmi_tx_app *app)
{

}


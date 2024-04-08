// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "hdcp/hdcp_params.h"

void hdcp_params_reset(hdmi_tx_dev_t *dev, hdcpParams_t * params)
{
	params->bypass = TRUE;
	params->mEnable11Feature = 0;
	params->mRiCheck = 1;
	params->mI2cFastMode = 0;
	params->mEnhancedLinkVerification = 0;
	params->maxDevices = 0;

	if(params->mKsvListBuffer != NULL)
		free(params->mKsvListBuffer);
	params->mKsvListBuffer = NULL;
}

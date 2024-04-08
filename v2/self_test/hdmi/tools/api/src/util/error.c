// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "util/error.h"
#include "util/log.h"

static errorType_t errorCode = NO_ERROR;

void error_set(errorType_t err)
{
	if ((err > NO_ERROR) && (err < ERR_UNDEFINED_HTX)) {
		errorCode = err;
		LOGGER(SNPS_ERROR, "Setting error %d", err);
	}
}

errorType_t error_Get()
{
	errorType_t tmpErr = errorCode;
	errorCode = NO_ERROR;
	return tmpErr;
}

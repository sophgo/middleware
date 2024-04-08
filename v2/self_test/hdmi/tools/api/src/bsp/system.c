// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp/system.h"
#include "util/log.h"
#include "util/error.h"

struct system_functions * snps_functions = 0;

void register_system_functions(struct system_functions * functions)
{
	LOGGER(SNPS_INFO,"System functions %s", functions->name);
	snps_functions = functions;
}


void snps_sleep(unsigned ms)
{
	snps_functions->sleep(ms);
}




#ifdef __cplusplus
}
#endif

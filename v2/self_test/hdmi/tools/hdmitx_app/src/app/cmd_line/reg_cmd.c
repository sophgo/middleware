// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */
#include "app/cmd_line/reg_cmd.h"
#include "../../../../api/include/reg_scan/reg_scan.h"


void reg_scan_cmd(struct hdmi_tx_app *app, cmd_list_t *cmd){

	if(app == NULL){
		printf("reg scan cmd params error!");
		return;
	}
	reg_scan(app->hdmi_tx);

	return;
}

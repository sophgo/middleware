// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "ipc_msg.h"

int main()
{
	int err;
	enum ipc_status status; 

	printf("Informing hdcp of kill status request...\n");
	err = send_hdmi_status(STATUS_HDMI_KILL_REQUEST);
	if (err<0){
		printf("Failed sending status with error: %s\n", strerror(-err));
		exit(1);
	}
	
	printf("Getting hdcp status...\n");
	err = get_hdcp_status();
	if (err<0){
		printf("Failed getting status with error: %s\n", strerror(-err));
		exit(1);
	}
	status = err;
	switch (status){
		case STATUS_HDCP_FIRMWARE_KILLED:
			printf("HDCP Firmware killed!\n");
			break;
		case STATUS_HDCP_READY:
			printf("FAIL: HDCP firmware was not killed.\n");
			exit(1);
			break;
		default:
			printf("FAIL: Received invalid status %d\n", status);
			exit(1);
			break;
	}

	// TODO reset HDMI TX
	printf("Resetting the HDMI TX...\n");
	sleep(2);
	printf("Reset done.\n");

	printf("Informing hdcp of esm being ready...\n");
	err = send_hdmi_status(STATUS_HDMI_LOAD_REQUEST);
	if (err<0){
		printf("Failed sending status with error: %s\n", strerror(-err));
		exit(1);
	}

	printf("done!\n");

	return 0;
}

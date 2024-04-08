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

	printf("Getting hdmi status...\n");
	err = get_hdmi_status();
	if (err<0){
		printf("Failed getting status with error: %s\n", strerror(-err));
		exit(1);
	}
	status = err;
	switch (status){
		case STATUS_HDMI_KILL_REQUEST:
			printf("HDCP Firmware kill request!\n");
			break;
		case STATUS_HDMI_LOAD_REQUEST:
			printf("Everything is already fine.\n");
			exit(0);
			break;
		default:
			printf("FAIL: Received invalid status %d\n", status);
			exit(1);
			break;
	}

	// TODO reset HDMI TX
	printf("Killing the firmware...\n");
	sleep(2);
	printf("Firmware was killed.\n");


	printf("Informing hdmi that the firmware was killed...\n");
	err = send_hdcp_status(STATUS_HDCP_FIRMWARE_KILLED);
	if (err<0){
		printf("Failed sending status with error: %s\n", strerror(-err));
		exit(1);
	}
	
	printf("Closing the hostlib modules...\n");

	printf("Getting hdmi status...\n");
	err = get_hdmi_status();
	if (err<0){
		printf("Failed getting status with error: %s\n", strerror(-err));
		exit(1);
	}
	status = err;
	switch (status){
		case STATUS_HDMI_LOAD_REQUEST:
			printf("Hostlib can be reinitialized.\n");
			break;
		case STATUS_HDMI_KILL_REQUEST:
			printf("FAIL: HDCP Firmware kill request!\n");
			exit(1);
			break;
		default:
			printf("FAIL: Received invalid status %d\n", status);
			exit(1);
			break;
	}

	printf("Initializing the hostlib...\n");

	printf("done!\n");
	return 0;
}

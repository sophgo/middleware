// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "hdmitx_dev.h"

#include "includes.h"
#include "hdmi_tx_app.h"
#include "platform.h"
#include "hdmitx_ipk_api/hdmitx_ipk_api.h"
#include "core/irq.h"

#include "app/cmd_line/cmd_interface.h"
#include "app/hdmitx.h"
#include "app/signal_handler.h"


#define IIC_TIMOUT	1000000

extern unsigned repeater_enabled;
unsigned repeater_enabled = 0;


int main(int argc, char **argv){
	char command[CMD_SIZE + 1];
	char device[CMD_SIZE + 1] = {0,0,};
	cmd_list_t * command_list = NULL;
	int verbose = 0;
	int phy_debug = 0;
	//char * framebuffer = NULL;
	int ret = 1;
	int phy_model = 0;
	char edid_tx_cap[30];
	struct hdmi_tx_app app;
	static struct option long_options[] = {
		{"verbose", no_argument, 0, 'v'},
		{NULL, 0, NULL, 0}
	};
	int option = 0;
	int option_index = 0;
	strcpy(app.edid_tx_capabilites_file,"");

	while((option = getopt_long(argc, argv, "v:p:c:h", long_options, &option_index)) != -1) {
		switch(option){
		case 'v':
			if(optarg == NULL)
				verbose = 1;
			else
				verbose =  strtol(optarg, NULL, 10);
			break;
		case 'p':
			if(optarg != NULL){
#ifndef PHY_THIRD_PARTY
				if(strcasecmp(optarg, "E301") == 0)
					phy_model = PHY_MODEL_301;
				if(strcasecmp(optarg, "E302") == 0)
					phy_model = PHY_MODEL_302;
				if(strcasecmp(optarg, "E303") == 0)
					phy_model = PHY_MODEL_303;
				if(strcasecmp(optarg, "E305") == 0)
					phy_model = PHY_MODEL_305;
				if(strcasecmp(optarg, "E308") == 0)
					phy_model = PHY_MODEL_308;
				if(strcasecmp(optarg, "E311") == 0)
					phy_model = PHY_MODEL_311;
				if(strcasecmp(optarg, "E312") == 0)
					phy_model = PHY_MODEL_312;
				if(strcasecmp(optarg, "E316") == 0)
					phy_model = PHY_MODEL_316;
#else 			//if(strcasecmp(optarg, "ack") == 0)
					//phy_model = PHY_MODEL_THIRD_PARTY_ACK;
				if(strcasecmp(optarg, "angel") == 0)
					phy_model = PHY_MODEL_THIRD_PARTY_ANGELCREEK;
#endif
			}
			break;
		case 'c':
			if(optarg != NULL)
				strncpy(edid_tx_cap, optarg,30 - 1);
			break;
		case 'h':
			printf("HDMITX App\n");
			printf("Usage: HDMITX -h -v -p <phy_version> -c <edid_cap_file>\n");
			printf("	-h - shows this help menu\n");
			printf("	-v - enables verbosity\n");
#ifndef PHY_THIRD_PARTY
			printf("	-p <phy_model> - choose the phy model (E301,E302,E303,E305,E308,E311,E312,E316)\n");
#else
			printf("	-p <phy_model> - choose the phy model (angel)\n");
#endif
			printf("	-c <edid_cap_file> - load EDID binary with HDMI Tx capabilities\n");
			exit(1);
			break;
		/*case 'f':
			if(optarg == NULL)
				break; //use default

			framebuffer = optarg;
			printf("Open framebuffer %s\n",optarg);

			break;*/
		}
	}

	// Device name
	memset(&command, 0, CMD_SIZE);

	// For stub mode comment this
	strcpy(device, "/dev/fb1");

	if(hdmitx_init_app(&app, phy_model, device, edid_tx_cap, 0, verbose, phy_debug)){
		printf("Application init failed\n");
		exit(-1);
	}
	printf("Starting Application\n");

	// sleep(1);
	irq_hpd_sense_enable(&app.hdmi_tx);
	printf("after irq_hpd_sense_enable,line:%d\n",__LINE__);
	printf("after irq_hpd_sense_enable,line:%d\n",__LINE__);
	printf("after irq_hpd_sense_enable,line:%d\n",__LINE__);
	printf("after irq_hpd_sense_enable,line:%d\n",__LINE__);
	printf("after irq_hpd_sense_enable,line:%d\n",__LINE__);
	printf("wait\n");
	printf("wait\n");
	printf("wait\n");
	printf("wait\n");
	printf("wait\n");
	printf("wait\n");
	// Wait before jumping to the visual interface
	// sleep(1);

	/************************************************************
	 * Get command
	 */
	ret = 1;
	while(ret){
		ret = get_cmd_line(&app, command);
		if(ret == 0)
			continue;

		/************************************************************
		 * Parse command and return the list of commands to execute
		 */
		command_list = parse_cmd_line(&app, command);

		/************************************************************
		 * Execute command list
		 */
		execute_cmd_list(&app, command_list);
		free_cmd_list(command_list);
	}

	ret = 0;
	write(app.hdmi_tx_signal, &ret, sizeof(uint32_t));

	close(app.hdmi_tx_signal);
	close(app.hdmi_tx_driver);
	close(app.hdmi_tx_log_file);

	return 0;
}


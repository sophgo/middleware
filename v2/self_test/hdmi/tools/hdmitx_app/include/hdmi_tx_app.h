/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef HDMI_TX_APP_H_
#define HDMI_TX_APP_H_

#include "includes.h"
#include "app/edid/edid_parser.h"
#include "bsp/board.h"
#include "bsp/access.h"
#include "bsp/system.h"

#define STRING_ARRAY_SIZE 150

#define IS_EQUAL(buffer, text) \
	((strcmp(buffer, text) == 0) ? 1 : 0)

#define GET(buf, argc) \
	strtoul(buf[argc], NULL, 0)

#define SIZE_ARRAY(array, type) \
	(sizeof(array) / sizeof(type))

typedef enum {
	COMPLIANCE = 0,
	EXPERT
}app_mode_t;



/**
 * Expert mode structure
 * This structure contains the important information regarding the expert
 * mode: variables and structures
 */
struct hdmi_expert_mode {
	int 		menu_option;
	int		audio_mute;
};

/**
 * Compliance mode structure
 * This structure contains the important information regarding the expert
 * mode: variables and structures
 */
struct hdmi_compliance_mode {
	int 		current_mode;
	int 		re_configure;
	int 		ycc420_support;
};

/**
 * Demo mode structure
 * This structure contains the important information regarding the expert
 * mode: variables and structures
 */
struct hdmi_demo_mode {
	uint32_t	current_mode;
	uint32_t 	cea;
	uint32_t 	dcm;
	int 		current_3D;
	uint32_t	sfr_clk;
	int 		re_configure;
	int 		ycc420_support;
	int 		menu_option;
};

struct hdmi_mode {
	videoParams_t 		pVideo;
	audioParams_t 		pAudio;
	hdcpParams_t 		pHdcp;
	productParams_t 	pProduct;
	productParams_t 	mProduct;
	hdmivsdb_t 		vsdb;
	hdmiforumvsdb_t 	forumvsdb;

	uint8_t			ksv_list_buffer[670];
	uint8_t			ksv_devices;
	uint8_t			dpk_aksv[7];
	uint8_t			sw_enc_key[2];
	uint8_t			dpk_keys[560];

	int 			hpd;
	int 			edid_done;

	struct edid 	      edid;
	u8 		      edid_ext[3][128];
	sink_edid_t  	      * sink_cap;

	struct hdmi_expert_mode		expert;
	struct hdmi_compliance_mode 	compliance;
	struct hdmi_demo_mode		demo;
};

struct menu_s;
/**
 * Main structure
 */
struct hdmi_tx_app {
	/** Verbose */
	int 				verbose;
	char				edid_tx_capabilites_file[30];
	u32 				flags; //flags to control menu
	char				log_file[20];

	/** PHY version */
	int 				hdmi_tx_phy;

	/** HDMI TX Driver */
	char 				hdmi_tx_name[STRING_ARRAY_SIZE];
	/** File descriptor */
	int 				hdmi_tx_driver;
	uint32_t			base_address;

	/** GUI options */
	int 				gui_interface;
	int 				gui_enabled;
	app_mode_t			app_mode;

	/** Log file */
	char				hdmi_tx_log_name[STRING_ARRAY_SIZE];
	int 				hdmi_tx_log_file;  /** File descriptor */

	/** Report file */
	char				hdmi_tx_report_name[STRING_ARRAY_SIZE];
	int 				hdmi_tx_report;  /** File descriptor */

	/** SW Version*/
	int 				sw_version_major;
	int 					sw_version_minor;
	char 					sw_version_letter[2];
	
	/** signal bind */
	/** File descriptor */
	int 				hdmi_tx_signal;

	/* Reserved for API internal use only */
	/** HDMI TX API Internals */
	struct device_access 		dev_access;
	struct system_functions 	sys_functions;
	struct board_wrappers		board_wrp;
	hdmi_tx_dev_t 			hdmi_tx;

	/** Application mode configurations */
	struct hdmi_mode		mode;
	/** Configurations to apply */
	struct hdmi_mode		* user_cfg;
	int 				data_enable_polarity;

	struct menu_s * 		current_menu;

	/** Mutex to synchronize calls */
	pthread_mutex_t  		mutex;

	struct edid 	        	* tx_edid;
	u8 		        	* tx_edid_ext;
	sink_edid_t  	        	* tx_sink_cap;

	int 				edid_tx_checks;
	/** Enable/disable load of pattern upon HPD */
	int 				pattern_load_enable;
	/** Enable/disable datapath configuration upon HPD */
	int 				datapath_config_enable;
	
};



/**
 * Keys
 */
void init_keys(struct hdmi_tx_app *app);


/**
 * Initialization
 */
void system_initialize(struct hdmi_tx_app *app);
#endif /* HDMI_TX_APP_H_ */

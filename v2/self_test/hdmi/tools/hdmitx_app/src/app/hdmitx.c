// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "app/hdmitx.h"
#include "app/system.h"
#include "app/signal_handler.h"
#include "commands.h"
#include "app/hdcp/hdcp.h"

#include "identification/identification.h"
#include "core/control.h"
#include "core/audio.h"
#include "core/packets.h"
#include "core/fc_video.h"
#include "core/fc_debug.h"
#include "core/irq.h"
#include "core/main_controller.h"

#include "hdmitx_ipk_api/access_ipk.h"
#include "hdmitx_ipk_api/phy_interface/phy_interface.h"
#include "hdcp.h"
#include "app/edid/edid.h"
#include "../../src/phy/phy_reg.h"
#include "app/video/video.h"


extern menu_t main_menu[];

int _open_device(struct hdmi_tx_app *app, char * device)
{
	int ret = 0;
	// file descriptors

	if(strlen(device) == 0){
		strcpy(app->hdmi_tx_name, "Stub Device");
		printf("#################################################################\n");
		printf("#   Warning: System working in stub mode                        #\n");
		printf("#################################################################\n");
		return 0;
	}

	strcpy(app->hdmi_tx_name, device);

	// open the file for reading and writing
	LOGGER(SNPS_INFO,"Opening %s file.\n", app->hdmi_tx_name);
	app->hdmi_tx_driver = open(app->hdmi_tx_name, O_RDWR);
	if(app->hdmi_tx_driver < 0){
		printf("Could not open %s device. Please verify if hdmi_tx_module.ko is installed.\n", app->hdmi_tx_name);
		return app->hdmi_tx_driver;
	}

	usleep(1000);

	// Get base address
	LOGGER(SNPS_INFO,"Get driver base address.\n");
	ret = ioctl(app->hdmi_tx_driver, FB_HDMI_BASE_ADDR, &(app->base_address));
	if(ret < 0){
		printf("IOCTL error [%d], could not read base address\n", ret);
		return -EINVAL;
	}

	usleep(1000);

	LOGGER(SNPS_INFO,"Base address 0x%x\n", app->base_address);

	return 0;
}

int _open_log(struct hdmi_tx_app *app, char * log)
{
	mode_t file_mode = S_IRUSR |S_IWUSR|S_IRGRP|S_IWGRP; //user and group read/write permissions
	strncpy(app->hdmi_tx_log_name, log,STRING_ARRAY_SIZE - 1);

	LOGGER(SNPS_INFO,"Opening %s file.\n", app->hdmi_tx_log_name);

	app->hdmi_tx_log_file = open(app->hdmi_tx_log_name,  O_WRONLY | O_CREAT | O_APPEND, file_mode );
	if(app->hdmi_tx_log_file < 0){
		printf("Could not open %s file\n", app->hdmi_tx_log_name);
		return app->hdmi_tx_log_file;
	}

	return 0;
}

int _open_report(struct hdmi_tx_app *app, char * report)
{
	mode_t file_mode = S_IRUSR |S_IWUSR|S_IRGRP|S_IWGRP; //user and group read permissions
	strncpy(app->hdmi_tx_report_name, report,STRING_ARRAY_SIZE - 1);

	LOGGER(SNPS_INFO,"Opening %s file.\n", app->hdmi_tx_report_name);

	app->hdmi_tx_report = open(app->hdmi_tx_report_name,  O_RDWR | O_CREAT | O_APPEND, file_mode);
	if(app->hdmi_tx_report < 0){
		printf("Could not open %s file\n", app->hdmi_tx_report_name);
		return -1;
	}

	return 0;
}


int _init_interrupts(struct hdmi_tx_app *app)
{
	printf("***********_init_interrupts*****\n");
	register_signal_handler(app);

	// Reset HDMI TX Controller
	// hdmi_tx_core_reset(1);

	// Bind with the driver using the proc interface
	app->hdmi_tx_signal = open("/proc/hdmi_tx/bind", O_RDWR);
	if(app->hdmi_tx_signal <= 0){
		printf("Could not bind with the driver. "
			"No interrupts will be signaled!\n");
	}
	else{
		int pid = getpid();
		write(app->hdmi_tx_signal, &pid, sizeof(uint32_t));
	}

	// Enable HDMI TX Controller
	// hdmi_tx_core_reset(0);

	// Disable all interrupts
	irq_mask_all(&app->hdmi_tx);

	return 0;
}

int _api_init(struct hdmi_tx_app *app)
{
	hdmitx_api_init(&(app->hdmi_tx), "HDMI Tx App");

	// Register functions
	register_system_functions(&(app->sys_functions));
	register_bsp_functions(&(app->dev_access));

	register_board_wrappers(&(app->board_wrp));

	// board_ZcalReset(1);
	// snps_sleep(10);
	// board_ZcalReset(0);

	return 0;
}

int hdmitx_init_app(struct hdmi_tx_app *app, int phy, char * device, char * edid_tx_cap_file, int interface, int verbose, int phy_debug)
{
	int ret = 0;
	// Clear app structure
	memset(app, 0, sizeof(struct hdmi_tx_app));

	pthread_mutex_init(&(app->mutex), NULL);

	app->verbose = verbose;

	app->app_mode = EXPERT;
	app->current_menu = main_menu;

	// Set internal configurations
	app->data_enable_polarity = 1;
	app->hdmi_tx_driver = -1;

	// Copy HDCP static keys
	hdcp_init_keys(app);

	// set device access functions
	strcpy(app->dev_access.name, "HDMI TX Device Access");
	app->dev_access.initialize = hdmitx_initialize;
	app->dev_access.disable = hdmitx_disable;
	app->dev_access.read = hdmitx_read;
	app->dev_access.write = hdmitx_write;

	// set system abstraction functions
	strcpy(app->sys_functions.name, "HDMI TX System Functions");
	app->sys_functions.logger = hdmitx_logger;
	app->sys_functions.sleep = hdmitx_sleep;

	// Set global variable
	set_platform(app);

	app->edid_tx_checks = 1; // Enable EDID TX reading
	if(edid_read_hdmitx_cap(edid_tx_cap_file) < 0)
		return 1;
	// Set enable pattern loading upon HPD
	app->pattern_load_enable = 1;

	// set board specific abstraction functions
	strcpy(app->board_wrp.name, "HDMI TX Board wrappers");
	app->board_wrp.configure_audio_pll = audio_clock;
	#ifndef PHY_THIRD_PARTY
	app->board_wrp.configure_pixel_clock = pixel_clock;
	#endif
	// app->board_wrp.data_output_delay = phy_data_output_delay;
	// app->board_wrp.zcal_reset = zcal_reset;

	// Print and Update SW version
	app->sw_version_major = SW_VERSION_MAJOR;
	app->sw_version_minor = SW_VERSION_MINOR;
	strcpy(app->sw_version_letter,(char*) SW_VERSION_LETTER);
	printf("HDMI Application SW version = %d.%d%c\n", app->sw_version_major,app->sw_version_minor,app->sw_version_letter[0]);

	ret = _open_device(app, device);
	if(ret){
		printf("_open_device failed.\n");
		return ret;
	}

	_open_log(app, "hdmi_tx.log");
	_open_report(app, "hdmi_tx_report.log");

	_api_init(app);

	_init_interrupts(app);

	clean_all_reset();

	// if(hdmitx_set_phy(app, phy, phy_debug)){
	// 	printf("HDMI phy set failed\n");
	// 	return -1;
	// }

	app->mode.pHdcp.bypass = TRUE;

	// Print Core version
	printf("HDMI CORE version = %x\n", (id_design(&app->hdmi_tx) << 8) | id_revision(&app->hdmi_tx));

	return 0;
}

/**
 * @short HDMI TX Initialization Step B
 * Configure VGA (640x480p) DVI Video format
 * @param dev
 * @param data_polarity
 * @param phy_model
 * @return TRUE if successful or FALSE if not.
 */
int hdmitx_vga_init(struct hdmi_tx_app *app)
{
	dtd_t dtd;
	videoParams_t  video;
	struct hdmi_tx_dev * dev = NULL;

	if(app == NULL){
		LOGGER(SNPS_ERROR, "Invalid app argument");
		return FALSE;
	}

	//video = &app->mode.pVideo;
	dev = &app->hdmi_tx;

/*	if((video == NULL) || (dev == NULL)){
		LOGGER(SNPS_ERROR, "Invalid pointers: video=%x; dev=%x", video, dev);
		return FALSE;
	}*/

	LOGGER(SNPS_INFO,"HDMI TX Initialization Step B - Configure VGA (640x480p) DVI Video format");

	//Always due to HDCP and Scrambler
	// fc_video_hdcp_keepout(dev, TRUE);

	// reset error
	error_set(NO_ERROR);

	LOGGER(SNPS_TRACE, "Disable interrupts");
	irq_mute(dev); /* disable interrupts */

	dtd_fill(dev, &dtd, 1, 59.940);	/* VGA */



	video_params_reset(dev, &video);
	video.mEncodingIn = RGB;
	video.mEncodingOut = RGB;
	//video->mHdmi = FALSE;	/* DVI */
	//memcpy(&video->mDtd, &dtd, sizeof(dtd_t));

	//print_videoinfo(&video);

	dev->snps_hdmi_ctrl.hdmi_on = 0;
	dev->snps_hdmi_ctrl.pixel_clock = videoParams_GetPixelClock(dev, &video);
	dev->snps_hdmi_ctrl.color_resolution = COLOR_DEPTH_8;
	dev->snps_hdmi_ctrl.pixel_repetition = 0;
	dev->snps_hdmi_ctrl.csc_on = 1;
	dev->snps_hdmi_ctrl.audio_on = 0;


	LOGGER(SNPS_DEBUG, "hdmi=%d; pixel_clock=%f; color_res=%d; pixel_rep=%d; csc=%d; audio=%d; hdcp=%d",
			dev->snps_hdmi_ctrl.hdmi_on, dev->snps_hdmi_ctrl.pixel_clock, dev->snps_hdmi_ctrl.color_resolution,
			dev->snps_hdmi_ctrl.pixel_repetition, dev->snps_hdmi_ctrl.csc_on, dev->snps_hdmi_ctrl.audio_on,
			dev->snps_hdmi_ctrl.hdcp_on);

	// video_generator_config(app);

	// if (board_Initialize(dev) != TRUE) {
	// 	LOGGER(SNPS_ERROR, "%s:board_Initialize error", __func__);
	// 	return FALSE;
	// }

	LOGGER(SNPS_INFO,"HDMI TX Controller Design %x", id_design(dev));
	LOGGER(SNPS_INFO,"HDMI TX Controller Revision %x",id_revision(dev));

	if (id_hdcp_support(dev) == TRUE) {
		LOGGER(SNPS_INFO,"HDMI TX controller supports HDCP");
	}

	if (video_Initialize(dev, &video,  1) != TRUE) {
		LOGGER(SNPS_ERROR, "%s:video_Initialize error", __func__);
		return FALSE;
	}

	if (audio_Initialize(dev) != TRUE) {
		LOGGER(SNPS_ERROR, "%s:audio_Initialize error", __func__);
		return FALSE;
	}

	if (packets_Initialize(dev) != TRUE) {
		LOGGER(SNPS_ERROR, "%s:packets_Initialize error", __func__);
		return FALSE;
	}

	if (hdcp_initialize(dev) != TRUE) {
		LOGGER(SNPS_ERROR, "%s:hdcp_initialize error", __func__);
		return FALSE;
	}

	if (control_Initialize(dev) != TRUE) {
		LOGGER(SNPS_ERROR, "%s:control_Initialize error", __func__);
		return FALSE;
	}

	// if (phy_initialize(dev, app->hdmi_tx_phy) != TRUE) {
	// 	LOGGER(SNPS_ERROR, "%s:phy_initialize error", __func__);
	// 	return FALSE;
	// }

	control_InterruptClearAll(dev);
#ifdef PHY_THIRD_PARTY
	// send AVMUTE SET (optional)
	// if (phy_configure(dev, app->hdmi_tx_phy) !=  TRUE) {
	// 	LOGGER(SNPS_ERROR, "%s:phy_configure error", __func__);
	// 	return FALSE;
	// }
#else
	// if (board_PixelClock(dev) != TRUE){
	// 	LOGGER(SNPS_ERROR, "%s:board_PixelClock error", __func__);
	// 	return FALSE;
	// }
#endif

	if (video_Configure(dev, &video) != TRUE) {
		LOGGER(SNPS_ERROR, "%s:video_Configure error", __func__);
		return FALSE;
	}
	/*
	 * By default no pixel repetition and color resolution is 8
	 */
	mc_enable_all_clocks(dev);

#ifndef PHY_THIRD_PARTY
	// send AVMUTE SET (optional)
	// if (phy_configure(dev, app->hdmi_tx_phy) !=  TRUE) {
	// 	LOGGER(SNPS_ERROR, "%s:phy_configure error", __func__);
	// 	return FALSE;
	// }
#endif

	// disable blue screen transmission after turning on all necessary blocks (e.g. HDCP)
	fc_force_output(dev, TRUE);

	// enable interrupts
	irq_unmute(dev);
//	phy_interrupt_mask(dev, 0xF1); //ENABLE HPD
	// phy_interrupt_mask(dev, PHY_MASK0_TX_PHY_LOCK_MASK);
	// phy_interrupt_unmask(dev, PHY_STAT0_HPD_MASK |
	// 			  PHY_MASK0_RX_SENSE_0_MASK |
	// 			  PHY_MASK0_RX_SENSE_1_MASK |
	// 			  PHY_MASK0_RX_SENSE_2_MASK |
	// 			  PHY_MASK0_RX_SENSE_3_MASK);

	return TRUE;
}

/**
 * @short Set PHY number
 * @param[in] app Main structure
 * @param[in] phy PHY number
 * @return 0 if successful or -1 if failure
 */
int hdmitx_set_phy(struct hdmi_tx_app *app, int phy, int debug_mode)
{
	//LOGGER(SNPS_INFO, "PHY Type: %s", id_phy_string(&(app->hdmi_tx)));

	LOGGER(SNPS_INFO, "Setup PHY: %s - %s", phy_identification(&(app->hdmi_tx), phy), id_phy_string(&(app->hdmi_tx)));
	hdmitx_sleep(10000); //wait time to allow stop the initialization
#ifdef PHY_THIRD_PARTY

	if(phy != PHY_MODEL_THIRD_PARTY_ANGELCREEK) {
		printf("PHY value outside of range: %d\n", phy);
		return -1;
	}
	struct hdmi_ack_tx_phy * ack_phy = &(app->hdmi_tx.ack_phy);

	//enable external phy commands
	app->flags = CMD_EXTERN;

	ack_phy->status = 1;
	ack_phy->version = 1;
	ack_phy->generation = 1;
	ack_phy->voltage = 400;
	ack_phy->gain = 0;
	ack_phy->sus_clock = 25;
	ack_phy->pwclock = 25; //Mhz
	ack_phy->refclock = 100;
	ack_phy->datarate = 25;
	ack_phy->debug_mode = debug_mode;

	LOGGER(SNPS_INFO, "%s: PHY THIRD_PARTY ACK init", __func__);

	phy_preparation(&(app->hdmi_tx), phy);

	if(phy_powerup(&(app->hdmi_tx), phy)){
		LOGGER(SNPS_WARN, "%s: PHY THIRD_PARTY ACK powerup failed ", __func__);
		return -1;
	}
#else
	if((phy > 500) || (phy < 100)){
		printf("PHY value outside of range: %d\n", phy);
		return -1;
	}
#endif
	app->hdmi_tx_phy = phy;
	return 0;
}

/**
 * @short Configure VGA
 * @param[in] app Main structure
 * @return 0
 */
int hdmitx_configure_vga(struct hdmi_tx_app *app)
{
	printf("*****hdmitx_configure_vga******\n");
	audio_reset(&app->hdmi_tx, &app->mode.pAudio);
	product_reset(&app->hdmi_tx, &app->mode.pProduct);
	video_params_reset(&app->hdmi_tx, &app->mode.pVideo);

	reset_hdcp_params(app);

	return hdmitx_vga_init(app);
}

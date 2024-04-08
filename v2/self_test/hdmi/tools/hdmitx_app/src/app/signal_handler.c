// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "includes.h"
#include "app/signal_handler.h"
#include "app/edid/edid.h"
#include "app/video/video.h"
#include "app/hdcp/hdcp.h"
#include "app/edid/edid_parser.h"
#include "app/cec/cec_app.h"

#include "reset_mng.h"
#include "video_bridge.h"
#include "platform.h"
#include "app/hdmitx.h"
#include "edid/edid_type.h"

#include "hdcp/hdcp.h"
#include "core/irq.h"
#include "core/packets.h"
#include "core/interrupt/interrupt_reg.h"
#include "scdc/scrambling.h"
#include "ipc/ipc_msg.h"

#include "hdmitx_ipk_api/access_ipk.h"
#include "util.h"

#include <sys/time.h>

#define HPD_VALUE_MASK		0x00000001
#define HPD_COUNTER_MASK	0xFFFFFFFE

void hpd_handler();
static int start_state = 0;
static int hpd_counter = 0;

/**
 * Signal handlers
 */
void dwc_hdmi_tx_handler(int signum){
	uint32_t decode = 0;
	uint8_t phy_decode = 0;
	uint32_t hdcp_irq = 0;
	int execute = 0;
	struct hdmi_tx_app *p_app = get_platform();
	printf("***********dwc_hdmi_tx_handler*****\n");
	pthread_mutex_lock(&(p_app->mutex));

	decode = read_interrupt_decode(&p_app->hdmi_tx);
	hdcp_irq = hdcp_interrupt_status(&p_app->hdmi_tx);

	if(hdcp_irq != 0){
		hdcp_handler(0);
	}

	if(decode_is_fc_stat0(decode)){
		irq_clear_source(&p_app->hdmi_tx, AUDIO_PACKETS);
	}

	if(decode_is_fc_stat1(decode)){
		irq_clear_source(&p_app->hdmi_tx, OTHER_PACKETS);
	}

	if(decode_is_fc_stat2_vp(decode)){
		// TODO: mask this for now...
		irq_mute_source(&p_app->hdmi_tx, PACKETS_OVERFLOW);
		irq_mute_source(&p_app->hdmi_tx, VIDEO_PACKETIZER);
	}
	if(decode_is_as_stat0(decode)){
		irq_clear_source(&p_app->hdmi_tx, AUDIO_SAMPLER);
	}
	if(decode_is_phy(decode)){
		LOGGER(SNPS_TRACE, "%s:PHY interrupt 0x%08x", __func__, decode);

		irq_read_stat(&p_app->hdmi_tx, PHY, &phy_decode);

		if(decode_is_phy_lock(phy_decode)){
			irq_clear_bit(&p_app->hdmi_tx, PHY, IH_PHY_STAT0_TX_PHY_LOCK_MASK);
		}

		// if(decode_is_phy_rx_s0(phy_decode)) {
		// 	LOGGER(SNPS_WARN, "Rx sense 0 received");
		// 	phy_rx_s0_detected(&p_app->hdmi_tx);
		// 	irq_clear_bit(&p_app->hdmi_tx, PHY, IH_PHY_STAT0_RX_SENSE_0_MASK);
		// 	execute = 1;
		// }

		// if(decode_is_phy_rx_s1(phy_decode)) {
		// 	LOGGER(SNPS_WARN, "Rx sense 1 received");
		// 	phy_rx_s1_detected(&p_app->hdmi_tx);
		// 	irq_clear_bit(&p_app->hdmi_tx, PHY, IH_PHY_STAT0_RX_SENSE_1_MASK);
		// 	execute = 1;
		// }

		// if(decode_is_phy_rx_s2(phy_decode)) {
		// 	LOGGER(SNPS_WARN, "Rx sense 2 received");
		// 	phy_rx_s2_detected(&p_app->hdmi_tx);
		// 	irq_clear_bit(&p_app->hdmi_tx, PHY, IH_PHY_STAT0_RX_SENSE_2_MASK);
		// 	execute = 1;
		// }

		// if(decode_is_phy_rx_s3(phy_decode)) {
		// 	LOGGER(SNPS_WARN, "Rx sense 3 received");
		// 	phy_rx_s3_detected(&p_app->hdmi_tx);
		// 	irq_clear_bit(&p_app->hdmi_tx, PHY, IH_PHY_STAT0_RX_SENSE_3_MASK);
		// 	execute = 1;
		// }

		if(decode_is_phy_hpd(phy_decode)) {
			LOGGER(SNPS_WARN, "HPD received");
			phy_hot_plug_detected(&p_app->hdmi_tx);
			irq_clear_bit(&p_app->hdmi_tx, PHY, IH_PHY_STAT0_HPD_MASK);
			execute = 1;
		}

		// if(execute)
		// 	hpd_handler();

	}

	hpd_handler();

	if(decode_is_i2c_stat0(decode)){
		uint8_t state = 0;

		irq_read_stat(&p_app->hdmi_tx, I2C_DDC, &state);

		// I2Cmastererror - I2Cmasterdone
		if(state & 0x3){
			irq_clear_bit(&p_app->hdmi_tx, I2C_DDC, IH_I2CM_STAT0_I2CMASTERDONE_MASK);
			//The I2C communication interrupts must be masked - they will be handled by polling in the eDDC block
			LOGGER(SNPS_INFO, "%s:I2C DDC interrupt received 0x%x - mask interrupt", __func__, state);
		}
		// SCDC_READREQ
		else if(state & 0x4){
			irq_clear_bit(&p_app->hdmi_tx, I2C_DDC, IH_I2CM_STAT0_SCDC_READREQ_MASK);
		}
	}
	if(decode_is_cec_stat0(decode))
		cec_handler();

	irq_unmute(&p_app->hdmi_tx);
	pthread_mutex_unlock(&(p_app->mutex));
}

void video_brigde_handler(int signum){
	hdmitx_logger(SNPS_DEBUG, "%s:Caught signal %d\n", __func__, signum);
}

void audio_bridge_handler(int signum){
	hdmitx_logger(SNPS_DEBUG, "%s:Caught signal %d\n", __func__, signum);
}

void tx_phy_if_handler(int signum){
	hdmitx_logger(SNPS_DEBUG, "%s:Caught signal %d\n", __func__, signum);
}

void hdcp_handler(int signum){
	struct hdmi_tx_app *p_app = get_platform();
	int temp = 0;
	hdmitx_logger(SNPS_DEBUG, "%s:Caught signal %d\n", __func__, signum);

	hdcp_event_handler(&(p_app->hdmi_tx), &temp);
}

void dwc_hdmi_tx_cec_handler(int signum){
	hdmitx_logger(SNPS_DEBUG, "%s:Caught signal %d\n", __func__, signum);
}

int register_signal_handler(struct hdmi_tx_app *app){
	// DWC HDMI TX
	printf("***********register_signal_handler*****\n");
	signal(SIG_DWC_HDMI_TX, dwc_hdmi_tx_handler);

	// Video Bridge
	signal(SIG_VID_BRIDGE, video_brigde_handler);

	// DWC HDMI TX
	signal(SIG_AUD_BRIDGE, audio_bridge_handler);

	// DWC HDMI TX
	signal(SIG_TXPHY_IF, tx_phy_if_handler);

	// DWC HDMI TX
	signal(SIG_HDCP, hdcp_handler);

	// DWC HDMI TX
	signal(SIG_CEC_DETECT, dwc_hdmi_tx_cec_handler);

	return 0;
}

int test_signal_handlers(struct hdmi_tx_app *app){
	int ret = 0;

	printf("Test signal handler SIG_DWC_HDMI_TX...\n");
	ret = ioctl(app->hdmi_tx_driver, SIG_DWC_HDMI_TX, NULL);
	if(ret < 0){
		printf("%s:IOCTL error [%d]\n", __func__, ret);
	}

	printf("Test signal handler SIG_VID_BRIDGE...\n");
	ret = ioctl(app->hdmi_tx_driver, SIG_VID_BRIDGE, NULL);
	if(ret < 0){
		printf("%s:IOCTL error [%d]\n", __func__, ret);
	}

	printf("Test signal handler SIG_AUD_BRIDGE...\n");
	ret = ioctl(app->hdmi_tx_driver, SIG_AUD_BRIDGE, NULL);
	if(ret < 0){
		printf("%s:IOCTL error [%d]\n", __func__, ret);
	}

	printf("Test signal handler SIG_TXPHY_IF...\n");
	ret = ioctl(app->hdmi_tx_driver, SIG_TXPHY_IF, NULL);
	if(ret < 0){
		printf("%s:IOCTL error [%d]\n", __func__, ret);
	}

	printf("Test signal handler SIG_HDCP...\n");
	ret = ioctl(app->hdmi_tx_driver, SIG_HDCP, NULL);
	if(ret < 0){
		printf("%s:IOCTL error [%d]\n", __func__, ret);
	}

	printf("Test signal handler SIG_CEC_DETECT...\n");
	ret = ioctl(app->hdmi_tx_driver, SIG_CEC_DETECT, NULL);
	if(ret < 0){
		printf("%s:IOCTL error [%d]\n", __func__, ret);
	}
	return 0;
}

int do_reset(){
	struct hdmi_tx_app *p_app = get_platform();
	
	int err;
	enum ipc_status status; 
	struct timeval end_time, start_time;

	gettimeofday(&start_time, NULL);
	LOGGER(SNPS_INFO, "Informing hdcp of kill status request...");
	err = send_hdmi_status(STATUS_HDMI_KILL_REQUEST);
	if (err<0){
		LOGGER(SNPS_ERROR, "Failed sending status with error: %s", strerror(-err));
		return -1;
	}
	
	LOGGER(SNPS_INFO, "Waiting for hdcp status...");
	err = get_hdcp_status();
	if (err<0){
		LOGGER(SNPS_ERROR, "Failed getting status with error: %s", strerror(-err));
		return -1;
	}
	status = err;
	switch (status){
		case STATUS_HDCP_FIRMWARE_KILLED:
			LOGGER(SNPS_INFO, "HDCP Firmware killed!");
			break;
		case STATUS_HDCP_READY:
			LOGGER(SNPS_ERROR, "FAIL: HDCP firmware was not killed.");
			return -1;
		default:
			LOGGER(SNPS_ERROR, "FAIL: Received invalid status %d", status);
			return -1;
	}
	
	LOGGER(SNPS_INFO, "Running reset procedure...");

	// board_ZcalReset(1);
	// // snps_sleep(10);
	// board_ZcalReset(0);
	// Reset HDMI TX Controller
	// hdmi_tx_core_reset(1);
	// Enable HDMI TX Controller
	// hdmi_tx_core_reset(0);

	irq_mask_all(&p_app->hdmi_tx);

	clean_all_reset();
	// if(hdmitx_set_phy(p_app, p_app->hdmi_tx_phy, 0)){
	// 	LOGGER(SNPS_ERROR, "HDMI phy set failed");
	// }

	// irq_hpd_sense_enable(&p_app->hdmi_tx);

	// snps_sleep(10);
	LOGGER(SNPS_INFO, "Reset done.");

	LOGGER(SNPS_INFO, "Informing hdcp that reset is done...");
	err = send_hdmi_status(STATUS_HDMI_LOAD_REQUEST);
	if (err<0){
		LOGGER(SNPS_INFO, "FAIL: Sending status with error: %s", strerror(-err));
		return -1;
	}
	gettimeofday(&end_time, NULL);
	LOGGER(SNPS_INFO, "Done reseting in %dms!", (end_time.tv_sec-start_time.tv_sec)*1000+(end_time.tv_usec-start_time.tv_usec)/1000);
	return 0;
}

int stop_handler(struct hdmi_tx_app *p_app){
	LOGGER(SNPS_WARN, "Cable disconnected");
	p_app->mode.hpd = 0;
	p_app->hdmi_tx.snps_hdmi_ctrl.hpd = 0;
	hpd_counter = (((hpd_counter >> 1)) << 1) | p_app->mode.hpd;
	if(ioctl(p_app->hdmi_tx_driver, FB_HDMI_SET_HPD, &hpd_counter) >= 0)
		LOGGER(SNPS_INFO, "Writing HPD[%d] to driver", p_app->mode.hpd);

	if(p_app->mode.sink_cap != NULL)
		free(p_app->mode.sink_cap);
	p_app->mode.sink_cap = NULL;

	//if(p_app->mode.edid != NULL)
		//free(p_app->mode.edid);
	//p_app->mode.edid = NULL;

	//if(p_app->mode.edid_ext != NULL)
		//free(p_app->mode.edid_ext);
	//p_app->mode.edid_ext = NULL;

	hdcp_rxdetect(&p_app->hdmi_tx, 0);
	hdcp_sw_reset(&p_app->hdmi_tx);

	// Stand by
	api_Standby(&p_app->hdmi_tx);
	memset(&p_app->mode, 0, sizeof(struct hdmi_mode));
	p_app->mode.edid_done = 0;

	p_app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.cec_state = CEC_NOT_INIT;
	p_app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.power_state = STANDBY;

	do_reset();

	return 0;
}

int start_handler(struct hdmi_tx_app *p_app){
	sink_edid_t * sink = NULL;

	p_app->mode.hpd = 0;
	hpd_counter = (((hpd_counter >> 1)) << 1) | p_app->mode.hpd;
	if(ioctl(p_app->hdmi_tx_driver, FB_HDMI_SET_HPD, &hpd_counter) >= 0)
		LOGGER(SNPS_INFO, "Writing HPD[%d] to driver", p_app->mode.hpd);

	// video_bridge_write(p_app, VG_SWRSTZ, 0); //Stop video generator to reduce the interrupts for the ESM
	// audio_bridge_write(p_app, AG_SWRSTZ, 0); //Stop audio generator to reduce the interrupts for the ESM
	printf("*****start_handler******\n");
	LOGGER(SNPS_INFO, "Cable connected");

	memset(&p_app->mode, 0, sizeof(struct hdmi_mode));

	p_app->data_enable_polarity = 1;
	p_app->hdmi_tx.snps_hdmi_ctrl.data_enable_polarity = 1;

	// Initial VGA configuration
	if(hdmitx_configure_vga(p_app) != TRUE){
		LOGGER(SNPS_ERROR, "hdmitx_configure_vga failed");
	}

	// Read sink's EDID
	// edid_read_cap(); // sink is updated in this function
	// if (!phy_hot_plug_state(&p_app->hdmi_tx) || !phy_rx_sense_state(&p_app->hdmi_tx)) {
	// 	LOGGER(SNPS_WARN, "I2C comunication aborted!");
	// 	return stop_handler(p_app);
	// }
	sink = p_app->mode.sink_cap;
	if(sink && sink->edid_mHdmiForumvsdb.mRR_Capable){
		LOGGER(SNPS_INFO, "Enabling Read Request.");
		scdc_enable_rr(&p_app->hdmi_tx, TRUE);
	}

	if(sink){
		LOGGER(SNPS_INFO,"Sink is HDMI 2.0: %d\n", sink->edid_m20Sink);
		LOGGER(SNPS_INFO,"HDMI-Forum VSDB valid: %d\n", sink->edid_mHdmiForumvsdb.mValid);
		LOGGER(SNPS_INFO,"Sink supports SCDC: %d\n", sink->edid_mHdmiForumvsdb.mSCDC_Present);
		LOGGER(SNPS_INFO,"Sink supports RR: %d\n", sink->edid_mHdmiForumvsdb.mRR_Capable);
		LOGGER(SNPS_INFO,"Sink supports LTE_340Mcs Scrambling: %d\n", sink->edid_mHdmiForumvsdb.mLTS_340Mcs_scramble);
		LOGGER(SNPS_INFO,"Number of SVDs parsed %d\n", sink->edid_mSvdIndex);
		LOGGER(SNPS_INFO,"First SVDs parsed VIC %d\n", sink->edid_mSvd[0].mCode);
		LOGGER(SNPS_INFO,"First SVDs Limited Ycc420 %d\n", sink->edid_mSvd[0].mLimitedToYcc420);
		LOGGER(SNPS_INFO,"First SVDs supports Ycc420 %d\n", sink->edid_mSvd[0].mYcc420);
		LOGGER(SNPS_INFO,"Second SVDs parsed VIC %d\n", sink->edid_mSvd[1].mCode);
		LOGGER(SNPS_INFO,"Third SVDs parsed VIC %d\n", sink->edid_mSvd[2].mCode);
	}

	p_app->mode.hpd = 1;
	p_app->hdmi_tx.snps_hdmi_ctrl.hpd = 1;

	// Reset video, audio and packet structures
	audio_reset(&p_app->hdmi_tx, &p_app->mode.pAudio);
	product_reset(&p_app->hdmi_tx, &p_app->mode.pProduct);
	video_params_reset(&p_app->hdmi_tx, &p_app->mode.pVideo);
	reset_hdcp_params(p_app);

	if(sink)
		p_app->mode.pVideo.mHdmi = (sink->edid_mHdmivsdb.mValid == TRUE ? HDMI: DVI);
	else
		p_app->mode.pVideo.mHdmi = DVI;

	if(p_app->mode.pVideo.mHdmi == HDMI)
		cec_set_physical_address(p_app, sink);

	// Set video and audio mode
	if(p_app->mode.edid_done != 1){
		// Read EDID failed - Set VGA DVI
		LOGGER(SNPS_WARN, "Set to VGA-DVI mode");
		video_params_reset(&p_app->hdmi_tx, &p_app->mode.pVideo);
		p_app->mode.pVideo.mHdmi = DVI;
		p_app->mode.pVideo.mEncodingOut = RGB;
		p_app->mode.pVideo.mEncodingIn = RGB;
		dtd_fill(&p_app->hdmi_tx, &p_app->mode.pVideo.mDtd, 1, 60.0);
	} else{
		edid_set_video_prefered(&p_app->hdmi_tx, p_app->mode.sink_cap, &p_app->mode.pVideo);
		edid_set_audio_prefered(&p_app->hdmi_tx, p_app->mode.sink_cap, &p_app->mode.pAudio);
	}

	// if(p_app->pattern_load_enable == 1)
	// 	video_generator_set_pattern(p_app->mode.pVideo.mEncodingIn);

	printf("Changing video mode to:\n");
	print_videoinfo(&(p_app->mode.pVideo));
	if(sink != NULL) {
		if (sink->edid_mHdmivsdb.mSupportsAi == 0){
			packets_StopSendIsrc1(&p_app->hdmi_tx);
			packets_StopSendIsrc2(&p_app->hdmi_tx);
			packets_StopSendAcp(&p_app->hdmi_tx);
		} else {
			packets_AudioContentProtection(&p_app->hdmi_tx,0,NULL,0,1);//Type 0 doesn't need any dependent fields 9.3.3 HDMI 1.4 spec
			packets_IsrcPackets(&p_app->hdmi_tx,0x01,NULL,0,1);//Configuring without packet info in ISRC
		}
	}

	/* automatic scrambling enable/disable - MOI HF1-13
	 * FIXME: This should be done not in HPD context,
	 * but on video mode change context */
	if((sink != NULL) && (sink->edid_mHdmiForumvsdb.mValid) &&
			(sink->edid_mHdmiForumvsdb.mSCDC_Present) &&
			(sink->edid_m20Sink)) {
		if ((sink->edid_mHdmiForumvsdb.mLTS_340Mcs_scramble) &&
				(p_app->mode.pVideo.mDtd.mPixelClock <= 297)) {
			scrambling(&p_app->hdmi_tx, 1);
			LOGGER(SNPS_INFO, "Enabling Scrambling.");
		} else {
			scrambling(&p_app->hdmi_tx, 0);
		}
	}

	// video_generator_config(p_app);
	// audio_generator_config(p_app);
	printf("*****before api configure******\n");
	api_Configure(&p_app->hdmi_tx, &p_app->mode.pVideo, &p_app->mode.pAudio,
			&p_app->mode.pProduct, &p_app->mode.pHdcp, p_app->hdmi_tx_phy);

	hpd_counter = (((hpd_counter >> 1) + 1) << 1) | p_app->mode.hpd;
	if(ioctl(p_app->hdmi_tx_driver, FB_HDMI_SET_HPD, &hpd_counter) >= 0)
		LOGGER(SNPS_INFO, "Writing HPD[%d] to driver", p_app->mode.hpd);

	if(p_app->hdmi_tx.snps_hdmi_ctrl.cec_on == 1){
		p_app->hdmi_tx.snps_hdmi_ctrl.cec_tx_dev.power_state = ON;
		cec_Init(&p_app->hdmi_tx);
		cec_handler();
	}
	printf("*****end start handler******\n");
	return 1;
}

void hpd_handler(){
	struct hdmi_tx_app *p_app = get_platform();
	int new_state = 1;
	printf("*****hpd_handler******\n");
	LOGGER(SNPS_WARN, "%s:%d Hot Plug => %s, Rx Sense => %s, start_state = %s", __FUNCTION__, __LINE__, phy_hot_plug_state(&p_app->hdmi_tx) ? "ON" : "OFF", phy_rx_sense_state(&p_app->hdmi_tx) ? "ON" : "OFF", start_state ? "ON" : "OFF");
	// if (phy_hot_plug_state(&p_app->hdmi_tx) && !start_state) {
	// 	new_state = 1;
	// } else if ((!phy_hot_plug_state(&p_app->hdmi_tx) || !phy_rx_sense_state(&p_app->hdmi_tx)) && start_state) {
	// 	new_state = 2;
	// }

	switch (new_state) {
		case 1:
			LOGGER(SNPS_WARN, "%s:%d START", __FUNCTION__, __LINE__);
			start_state = start_handler(p_app);
			break;
		case 2:
			LOGGER(SNPS_WARN, "%s:%d STOP", __FUNCTION__, __LINE__);
			start_state = stop_handler(p_app);
			break;
		default:
			break;
	}
}

void cec_handler(){
	struct hdmi_tx_app *p_app = get_platform();
	u32 decode_cec_stat0 = 0;

	decode_cec_stat0 = dev_read(&p_app->hdmi_tx, IH_CEC_STAT0);
	dev_write(&p_app->hdmi_tx, IH_CEC_STAT0, decode_cec_stat0);

	if((decode_cec_stat0 & IH_CEC_STAT0_DONE_MASK) != 0)
		LOGGER(SNPS_INFO, "%s IH_CEC_STAT0_DONE_MASK\n", __func__);
	if((decode_cec_stat0 & IH_CEC_STAT0_EOM_MASK)  != 0)
		LOGGER(SNPS_INFO, "%s IH_CEC_STAT0_EOM_MASK\n", __func__);
	if((decode_cec_stat0 & IH_CEC_STAT0_NACK_MASK) != 0)
		LOGGER(SNPS_INFO, "%s IH_CEC_STAT0_NACK_MASK\n", __func__);
	if((decode_cec_stat0 & IH_CEC_STAT0_ARB_LOST_MASK) != 0)
		LOGGER(SNPS_INFO, "%s IH_CEC_STAT0_ARB_LOST_MASK\n", __func__);
	if((decode_cec_stat0 & IH_CEC_STAT0_ERROR_INITIATOR_MASK) != 0)
		LOGGER(SNPS_INFO, "%s IH_CEC_STAT0_ERROR_INITIATOR_MASK\n", __func__);
	if((decode_cec_stat0 & IH_CEC_STAT0_ERROR_FOLLOW_MASK) != 0)
		LOGGER(SNPS_INFO, "%s IH_CEC_STAT0_ERROR_FOLLOW_MASK\n", __func__);
	if((decode_cec_stat0 & IH_CEC_STAT0_WAKEUP_MASK) != 0)
		LOGGER(SNPS_INFO, "%s IH_CEC_STAT0_WAKEUP_MASK\n", __func__);

	irq_mute_source(&p_app->hdmi_tx, CEC);
	while(cec_state_manager(&p_app->hdmi_tx, decode_cec_stat0));
	irq_unmute_source(&p_app->hdmi_tx, CEC);
}

void edid_event_handler(void *param)
{

}

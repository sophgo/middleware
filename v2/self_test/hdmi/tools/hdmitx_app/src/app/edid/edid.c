// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "app/edid/edid.h"
#include "includes.h"
#include "platform.h"
#include "core/fc_gamut.h"
#include <string.h>
#include <unistd.h>


void edid_init_sink_cap(sink_edid_t * sink)
{
	memset(sink, 0, sizeof(sink_edid_t));
}

void edid_read_cap(void)
{
	struct hdmi_tx_app *p_app = get_platform();
	sink_edid_t * sink = NULL;
	int edid_ok = 0;
	int edid_tries = 3;

	if(p_app == NULL){
		LOGGER(SNPS_ERROR, "%s:Could not get app structure", __func__);
		return;
	}

	// Data allocation
	sink = (sink_edid_t *) malloc(sizeof(sink_edid_t));
	memset(sink, 0, sizeof(sink_edid_t));

	edid_init_sink_cap(sink);
	edid_parser_CeaExtReset(&p_app->hdmi_tx, sink);

	p_app->mode.sink_cap = NULL;

	do{
		// if (!phy_hot_plug_state(&p_app->hdmi_tx)) {
		// 	LOGGER(SNPS_WARN, "%s:%d Hot Plug = %s", __FUNCTION__, __LINE__, phy_hot_plug_state(&p_app->hdmi_tx) ? "ON" : "OFF");
		// 	break;
		// }
		edid_ok = edid_read(&p_app->hdmi_tx, &p_app->mode.edid);

		if(edid_ok) //error case
			continue;

		if(edid_parser(&p_app->hdmi_tx, (u8 *) &p_app->mode.edid, sink, 128) == FALSE){
			LOGGER(SNPS_ERROR, "Could not parse EDID");
			p_app->mode.edid_done = 0;
			free(sink);
			return;
		}
		break;
	}while(edid_tries--);


	if(edid_tries <= 0){
		LOGGER(SNPS_ERROR, "Could not read EDID");
		p_app->mode.edid_done = 0;
		free(sink);
		return;
	}

	if(p_app->mode.edid.extensions == 0){
		p_app->mode.edid_done = 1;
	}
	else {
		int edid_ext_cnt = 1;
		while(edid_ext_cnt <= p_app->mode.edid.extensions){
			LOGGER(SNPS_INFO,"EDID Extension %d", edid_ext_cnt);
			edid_tries = 3;
			do{
				// if (!phy_hot_plug_state(&p_app->hdmi_tx)) {
				// 	LOGGER(SNPS_WARN, "%s:%d Hot Plug = %s", __FUNCTION__, __LINE__, phy_hot_plug_state(&p_app->hdmi_tx) ? "ON" : "OFF");
				// 	break;
				// }
				edid_ok = edid_extension_read(&p_app->hdmi_tx, edid_ext_cnt, &p_app->mode.edid_ext[edid_ext_cnt-1][0]);
				if(edid_ok) //error case
					continue;

				p_app->mode.edid_done = 1;
				if(edid_ext_cnt < 2){ // TODO: add support for EDID parsing w/ Ext blocks > 1
					if(edid_parser(&p_app->hdmi_tx, &p_app->mode.edid_ext[edid_ext_cnt-1][0], sink, 128) == FALSE){
						LOGGER(SNPS_ERROR, "Could not parse EDID EXTENSIONS");
						p_app->mode.edid_done = 0;
						//free(sink);
						//return;
					}
					p_app->mode.sink_cap = sink;

				}
				break;
			}while(edid_tries--);
			edid_ext_cnt++;
		}
	}
}

int edid_read_hdmitx_cap(char * edid_tx_cap_file){
	int error = 0;
	const u8 header[] = {0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00};
	int edid_fp = 0;
	const int data_size = 128;
	struct hdmi_tx_app *p_app = get_platform();

	if(p_app->edid_tx_checks == 0){
		LOGGER(SNPS_DEBUG, "EDID TX checks disabled");
		return 0;
	}

	p_app->tx_edid = (struct edid *) malloc(sizeof(struct edid));
	memset(p_app->tx_edid, 0, sizeof(struct edid));

	if(edid_tx_cap_file != NULL && edid_tx_cap_file[0]!='\0'){
		strncpy(p_app->edid_tx_capabilites_file, edid_tx_cap_file, 30 - 1);
		printf("Opening %s (user defined)...", p_app->edid_tx_capabilites_file);
		edid_fp = open(p_app->edid_tx_capabilites_file,  O_RDONLY , S_IRUSR);
		if(edid_fp < 0){
			printf("Could not open %s file.\n", p_app->edid_tx_capabilites_file);
		}
		else
		{
			goto file_parsing;
		}
	}
	strcpy(p_app->edid_tx_capabilites_file, "DWC_IPK_HDMITX_EDID.bin");
	printf("Opening %s (default file)...", p_app->edid_tx_capabilites_file);
	edid_fp = open(p_app->edid_tx_capabilites_file,  O_RDONLY , S_IRUSR);

	if(edid_fp < 0){
		printf("\nCould not open default EDID Tx Capabilities file.\n");
		free(p_app->tx_edid);
		p_app->tx_edid = NULL;
		p_app->edid_tx_checks = 0;
		return -1;
	}
	file_parsing:
	//Update file name on global variables
	set_platform(p_app);
	printf("Ok\n");

	error = read(edid_fp, p_app->tx_edid, data_size);
	if(error < data_size){
		LOGGER(SNPS_ERROR, "HDMI TX EDID read failed");
		close(edid_fp);
		return 0;
	}

	error = memcmp((u8 *)p_app->tx_edid, (u8 *) header, sizeof(header));
	if(error){
		LOGGER(SNPS_ERROR, "EDID header check failed");
		close(edid_fp);
		return 0;
	}

	if(p_app->tx_edid->extensions > 0){
		p_app->tx_sink_cap = (sink_edid_t *) malloc(sizeof(sink_edid_t));
		edid_init_sink_cap(p_app->tx_sink_cap);

		p_app->tx_edid_ext = (u8 *) malloc(data_size);
		memset(p_app->tx_edid_ext, 0, sizeof(data_size));

		error = read(edid_fp, p_app->tx_edid_ext, data_size);
		if(error < data_size){
			LOGGER(SNPS_ERROR, "HDMI TX EDID EXT read failed");
			close(edid_fp);
			return 0;
		}

		edid_parser(&p_app->hdmi_tx, p_app->tx_edid_ext, p_app->tx_sink_cap, 128);
	}
	else{
		LOGGER(SNPS_WARN, "HDMI TX EDID: no extensions");
	}
	close(edid_fp);
	return 1;
}

int edid_tx_supports_cea_code(u32 cea_code){
	int i = 0;
	struct hdmi_tx_app *p_app = get_platform();

	if(p_app == NULL){
		return FALSE;
	}

	if(p_app->edid_tx_checks == 0){
		return TRUE;
	}

	if(p_app->tx_sink_cap == NULL){
		return FALSE;
	}

	for(i = 0; (i < EDID_SVD_ARRAY_SIZE) && (p_app->tx_sink_cap->edid_mSvd[i].mCode != 0); i++){
		if(p_app->tx_sink_cap->edid_mSvd[i].mCode == cea_code){
			return TRUE;
		}
	}

	for(i = 0; (i < 4) && (p_app->tx_sink_cap->edid_mHdmivsdb.mHdmiVic[i] != 0); i++){
		if(p_app->tx_sink_cap->edid_mHdmivsdb.mHdmiVic[i] == videoParams_GetHdmiVicCode(cea_code)){
			return TRUE;
		}
	}
	return FALSE;
}

void edid_set_video_prefered(hdmi_tx_dev_t *dev, sink_edid_t * sink_cap, videoParams_t * pVideo)
{
	/* Configure using Native SVD or HDMI_VIC */
	int found_native = 0;
	int i = 0;
	int cea_vic = 0;
	unsigned int vic_index = 0;
	int found_ycc420 = 0;
	int latest_below_300_vic_index = -1;
	encoding_t set_encoding = RGB;
	dtd_t dtd;

	if((dev == NULL) || (sink_cap == NULL) || (pVideo == NULL)){
		return;
	}

	pVideo->mHdmiVideoFormat = HDMI_NORMAL_FORMAT;  //set normal video mode. It will be overwrited if need

	/* Sink is HDMI 1.4 and there are HDMI_VICs on Sink's EDID */
	if((sink_cap->edid_m20Sink == 0) && (sink_cap->edid_mHdmivsdb.mHdmiVicCount != 0))
	{
		cea_vic = videoParams_GetCeaVicCode(sink_cap->edid_mHdmivsdb.mHdmiVic[0]);
		LOGGER(SNPS_INFO, "Configuring system to HDMI 1.4 HDMI_VIC %d\n", cea_vic);
	}else{
		pVideo->mHdmi20 = sink_cap->edid_m20Sink;
		/* if sink is hdmi 2.0 looks for YCC420 only VICs */
		if(sink_cap->edid_m20Sink)
		{
			for(i = 0; (i < EDID_SVD_ARRAY_SIZE) && (sink_cap->edid_mSvd[i].mCode != 0); i++){
				if(edid_tx_supports_cea_code(sink_cap->edid_mSvd[i].mCode) == FALSE)
					continue;
				if(sink_cap->edid_mSvd[i].mLimitedToYcc420){
					cea_vic = sink_cap->edid_mSvd[i].mCode;
					vic_index = i;
					found_ycc420 = 1;
					LOGGER(SNPS_INFO, "Found HDMI 2.0 VIC limited to YCC420 %d\n", cea_vic);
				}
				if(sink_cap->edid_mSvd[i].mYcc420){
					cea_vic = sink_cap->edid_mSvd[i].mCode;
					vic_index = i;
					found_ycc420 = 1;
					LOGGER(SNPS_INFO, "Found HDMI 2.0 VIC supports YCC420 %d\n", cea_vic);
				}
				if(found_ycc420)
					goto native_config;
			}
		}
		native_config:
		/* sink does not support any ycc420 modes, looking for native VICs */
		if(found_ycc420 == 0)
		{
			for(i = 0; (i < EDID_SVD_ARRAY_SIZE) && (sink_cap->edid_mSvd[i].mCode != 0); i++)
			{
				/*assuming that the sink does not support ycc420 and we are configuring for a 4k VIC, we want to avoid 6G configs (non-Ycc420)*/
				if(sink_cap->edid_mSvd[i].mCode == 96 		||	sink_cap->edid_mSvd[i].mCode == 97 	||
						sink_cap->edid_mSvd[i].mCode == 106 ||	sink_cap->edid_mSvd[i].mCode == 107	||
						sink_cap->edid_mSvd[i].mCode == 101 ||	sink_cap->edid_mSvd[i].mCode == 102)
					continue;
				else
					latest_below_300_vic_index = latest_below_300_vic_index >= 0 ? latest_below_300_vic_index : i;

				if(sink_cap->edid_mSvd[i].mNative){
					found_native = 1;
					cea_vic = sink_cap->edid_mSvd[i].mCode;
					vic_index = i;
					LOGGER(SNPS_INFO, "Found Sink Native VIC %d\n", cea_vic);
				}
			}
		}
		/* if not ycc420 and not native, then choose the highest positioned VIC from the list below 297*/
		if(found_native == 0 && found_ycc420 == 0)
			vic_index = latest_below_300_vic_index >= 0 ? latest_below_300_vic_index : 0;
	}

	cea_vic = sink_cap->edid_mSvd[vic_index].mCode;

	/* last check to see if the video mode selected is supported */
	if(edid_tx_supports_cea_code(cea_vic) == FALSE){
		LOGGER(SNPS_WARN, "Set to VGA-DVI mode");
		video_params_reset(dev, pVideo);
		pVideo->mHdmi = DVI;
		pVideo->mEncodingIn = RGB;
		pVideo->mEncodingOut = RGB;
		dtd_fill(dev, &pVideo->mDtd, 1, 60.00);
		return;
	}


	if(pVideo->mHdmi == DVI)
	{
		pVideo->mEncodingIn = RGB;
		pVideo->mEncodingOut = RGB;
	}
	else
	{
	/* color encoding configuration */
	if((sink_cap->edid_m20Sink == 1) &&
	(sink_cap->edid_mSvd[vic_index].mLimitedToYcc420 == 1)){
		/* if current Svd is Limited to YCC420 (EDID) */
		set_encoding = YCC420;
		LOGGER(SNPS_INFO,"Setting encoding to Ycc420");

	}else{
		if((sink_cap->edid_m20Sink == 1) && (sink_cap->edid_mSvd[vic_index].mYcc420 == 1)){
			set_encoding = YCC420;
		}else{
			//set_encoding = RGB;
			if(sink_cap->edid_mYcc444Support == 1){
				set_encoding = YCC444;
			}
			else if(sink_cap->edid_mYcc422Support == 1){
				set_encoding = YCC422;
			}
			else{
				set_encoding = RGB;
			}
		}
	}
	}

	/** Colorimetry */
	if(sink_cap->edid_mYcc444Support == 1){
		pVideo->mColorimetry = EXTENDED_COLORIMETRY;

		if(supports_xv_ycc601(dev, &sink_cap->edid_mColorimetryDataBlock) == TRUE){
			pVideo->mExtColorimetry = XV_YCC601;
		}
		else if(supports_xv_ycc709(dev, &sink_cap->edid_mColorimetryDataBlock) == TRUE){
			pVideo->mExtColorimetry = XV_YCC709;
		}
		else if(supports_s_ycc601(dev, &sink_cap->edid_mColorimetryDataBlock) == TRUE){
			pVideo->mExtColorimetry = S_YCC601;
		}
		else if(supports_adobe_ycc601(dev, &sink_cap->edid_mColorimetryDataBlock) == TRUE){
			pVideo->mExtColorimetry = ADOBE_YCC601;
		}
		else if(supports_adobe_rgb(dev, &sink_cap->edid_mColorimetryDataBlock) == TRUE){
			pVideo->mExtColorimetry = ADOBE_RGB;
		}
		else{
			pVideo->mExtColorimetry = (u8)(-1);
		}

		if(supports_metadata0(dev, &sink_cap->edid_mColorimetryDataBlock) == TRUE){
			pVideo->mColorimetryDataBlock = TRUE;
		}
	}
	else{
		pVideo->mColorimetry = 0;
		pVideo->mExtColorimetry = (u8)(-1);
	}

	pVideo->mEncodingOut = set_encoding;
	pVideo->mEncodingIn = set_encoding;

	if(dtd_fill(dev, &dtd, cea_vic, 0) == FALSE){
		LOGGER(SNPS_ERROR, "Could not fill DTD with CEA VIC %d", cea_vic);
		return;
	}
	//workaround for supporting
	if((cea_vic == 93) || (cea_vic == 94) || (cea_vic == 95) || (cea_vic == 98) ){
		pVideo->mHdmiVideoFormat = HDMI_EXT_RES_FORMAT;
	}

	videoParams_SetYcc420Support(dev, &dtd, &sink_cap->edid_mSvd[vic_index]);

	/* parse configs */
	memcpy(&pVideo->mDtd, &dtd, sizeof(dtd_t));

	/* Limited to TMDS Clock 297 Mhz and no Deep Color support in YCC420 */
	if(pVideo->mDtd.mPixelClock >= 297.00 || set_encoding == YCC420){
		pVideo->mColorResolution = COLOR_DEPTH_8;
	}else{
		if (sink_cap->edid_mHdmivsdb.mDeepColor30)
			pVideo->mColorResolution = COLOR_DEPTH_10;
		else if (sink_cap->edid_mHdmivsdb.mDeepColor36)
			pVideo->mColorResolution = COLOR_DEPTH_12;
		else if (sink_cap->edid_mHdmivsdb.mDeepColor48)
			pVideo->mColorResolution = COLOR_DEPTH_16;
		else
			pVideo->mColorResolution = COLOR_DEPTH_8;
	}

	if(found_native){
		printf("Configuring for Native video mode: %d\n", cea_vic);
	}
	else{
		printf("Configuring for video mode %d, DCM %d\n", cea_vic, pVideo->mColorResolution);
	}
}

void edid_set_audio_prefered(hdmi_tx_dev_t *dev, sink_edid_t * sink_cap, audioParams_t * pAudio)
{


	speakerAllocationDataBlock_t *data =
		&sink_cap->edid_mSpeakerAllocationDataBlock;


	pAudio->mChannelAllocation = get_channell_alloc_code(dev, data);

	pAudio->mChannelAllocation = (pAudio->mChannelAllocation == -1) ? 0 :
		pAudio->mChannelAllocation;

	if (sink_cap->edid_mSadIndex > 0) {	/* if there is at least one short audio descriptor */
		pAudio->mInterfaceType = I2S;
		pAudio->mCodingType = sink_cap->edid_mSad[0].mFormat;
		pAudio->mPacketType = AUDIO_SAMPLE;

		if (sad_support192k(dev, &sink_cap->edid_mSad[0])) {
			pAudio->mSamplingFrequency = 192.00;
		}
		else if (sad_support176k4(dev, &sink_cap->edid_mSad[0])) {
			pAudio->mSamplingFrequency = 176.40;
		}
		else if (sad_support96k(dev, &sink_cap->edid_mSad[0])) {
			pAudio->mSamplingFrequency = 96.00;
		}
		else if (sad_support88k2(dev, &sink_cap->edid_mSad[0])) {
			pAudio->mSamplingFrequency = 88.20;
		}
		else if (sad_support48k(dev, &sink_cap->edid_mSad[0])) {
			pAudio->mSamplingFrequency = 48.00;
		}
		else if (sad_support44k1(dev, &sink_cap->edid_mSad[0])) {
			pAudio->mSamplingFrequency = 44.10;
		}
		else if (sad_support32k(dev, &sink_cap->edid_mSad[0])) {
			pAudio->mSamplingFrequency = 32.00;
		} else {
			LOGGER(SNPS_WARN, "Audio Sampling Frequency not supported, configuring for 32k.");
			pAudio->mSamplingFrequency = 32.00;
		}
	}

	/* choose highest sample size supported by sink */
	if (sink_cap->edid_mSad[0].mFormat == 1) {
		if (sad_support24bit(dev, &sink_cap->edid_mSad[0])) {
			pAudio->mSampleSize = 24;
		}
		else if (sad_support20bit(dev, &sink_cap->edid_mSad[0])) {
			pAudio->mSampleSize = 20;
		}
		else if (sad_support16bit(dev, &sink_cap->edid_mSad[0])) {
			pAudio->mSampleSize = 16;
		}
	} else {
		pAudio->mSampleSize = 16;
	}

	pAudio->mLevelShiftValue = 0;
	pAudio->mDownMixInhibitFlag = 0;
	pAudio->mLevelShiftValue = 128;

#if 0
	printf("Audio Sampling frequency %f\n", pAudio->mSamplingFrequency);
	printf("Audio Sample size %d\n", pAudio->mSampleSize);
	printf("Audio Fs Factor %d\n", pAudio->mClockFsFactor);
	printf("Audio Interface type %s\n", audio_interface_type_string(pAudio));
	printf("Audio Coding type %s\n", audio_coding_type_string(pAudio));
#endif
}

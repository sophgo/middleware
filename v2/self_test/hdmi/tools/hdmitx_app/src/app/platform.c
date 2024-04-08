// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "hdmi_tx_app.h"
#include "platform.h"
#include "hdmitx_ipk_api/hdmitx_ipk_api.h"
#include "hdmitx_ipk_api/clock_mng/clock_mng.h"
#include <limits.h>
#include "app/cmd_line/cmd_interface.h"
#include "hdmitx_dev.h"

static struct hdmi_tx_app *p_app = NULL;


/*******************************
 * HDMI TX Support functions   *
 ******************************/
void set_platform(struct hdmi_tx_app *app){
	p_app = app;
}

struct hdmi_tx_app * get_platform(){
	return p_app;
}

uint16_t audio_clock(double value, uint16_t over_sampling_factor)
{
	struct mmcm *configurations = NULL;

	LOGGER(SNPS_TRACE, "%s TBD", __func__);

	if(p_app == NULL)
		return -EINVAL;

	configurations = get_audio_mmcm_configs(value, over_sampling_factor);

	if(configurations == NULL)
		return -EINVAL;

	// configure_audio_mmcm(p_app, configurations);

	return TRUE;
}

uint16_t pixel_clock(double freq)
{
	struct mmcm *configurations = NULL;

	LOGGER(SNPS_TRACE, "%s", __func__);

	if(p_app == NULL)
		return -EINVAL;

	LOGGER(SNPS_DEBUG, "%s:Frequency %03fMHz", __func__, freq);

	configurations = get_video_mmcm_configs(freq);

	if(configurations == NULL)
		return -EINVAL;

	// configure_video_mmcm(p_app, configurations);

	return TRUE;
}

void app_init_hdcp(struct hdmi_mode * cfg)
{
	hdcpParams_t * pHdcp = &(cfg->pHdcp);
	pHdcp->bypass = -1;
	pHdcp->mEnable11Feature = -1;
	pHdcp->mRiCheck = -1;
	pHdcp->mI2cFastMode = -1;
	pHdcp->mEnhancedLinkVerification = -1;
	pHdcp->maxDevices = 0;
	pHdcp->mKsvListBuffer = NULL;
	pHdcp->mAksv = NULL;
	pHdcp->mKeys = NULL;
	pHdcp->mSwEncKey = NULL;
}

void app_init_video(struct hdmi_mode * cfg)
{
	videoParams_t * video = &(cfg->pVideo);

	memset(video, 0, sizeof(videoParams_t));

	video->mHdmi = MODE_UNDEFINED;
	video->mEncodingOut = ENC_UNDEFINED;
	video->mEncodingIn = ENC_UNDEFINED;
	video->mColorResolution = COLOR_DEPTH_INVALID;
	video->mColorimetry = UNDEFINED_COLORIMETRY;
	video->mExtColorimetry = UNDEFINED_EXTCOLOR;
	video->mEndTopBar = (u16)(~0);
	video->mStartBottomBar = (u16)(~0);
	video->mStartRightBar = (u16)(~0);

	video->mHdmiVideoFormat = UNDEFINED_FORMAT;
	video->m3dStructure = UNDEFINED_3D;
	video->m3dExtData = UNDEFINED_EXTDATA_3D;
	video->mHdmiVic = UNDEFINED_HDMI_VIC;
}

void app_init_audio(struct hdmi_mode * cfg)
{
	audioParams_t * audio = &(cfg->pAudio);

	memset(audio, 0, sizeof(audioParams_t));

	audio->mInterfaceType = INTERFACE_NOT_DEFINED;
	audio->mCodingType = CODING_NOT_DEFINED;
	audio->mPacketType = PACKET_NOT_DEFINED;
	audio->mDmaBeatIncrement = DMA_NOT_DEFINED;
}

struct hdmi_mode * app_get_user_config(struct hdmi_tx_app *app)
{
	double curr_refresh_rate;

	if(app->user_cfg == NULL){
		app->user_cfg = (struct hdmi_mode *) calloc(sizeof(struct hdmi_mode), 1);
		if(app->user_cfg == NULL){
			LOGGER(SNPS_ERROR, "%s Memory allocation for user configuration failed: Exiting", __func__);
			exit(-1);
		}
		app_init_hdcp(app->user_cfg);
		app_init_video(app->user_cfg);
		app_init_audio(app->user_cfg);

		// Load current Dtd settings
		curr_refresh_rate = dtd_get_refresh_rate(&app->mode.pVideo.mDtd);
		dtd_fill(&app->hdmi_tx, &app->user_cfg->pVideo.mDtd,
			 app->mode.pVideo.mDtd.mCode, curr_refresh_rate);
	}

	return app->user_cfg;
}

bool parse_ulong(const char *str,unsigned long *val)
{
	char *temp;
	bool rc = true;
	errno = 0;
	*val = strtol(str, &temp, 0);

	if (temp == str || *temp != '\0' || ((*val == LONG_MIN || *val == LONG_MAX) && (errno == ERANGE)))
		rc = false;

	return rc;
}



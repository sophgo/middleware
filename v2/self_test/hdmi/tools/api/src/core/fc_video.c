// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */
#include "stdio.h"
#include "stdlib.h"
#include "frame_composer/frame_composer_reg.h"
#include "core/fc_video.h"
#include "bsp/access.h"
#include "util/log.h"




void fc_video_hdcp_keepout(hdmi_tx_dev_t *dev, u8 bit)
{
	LOG_TRACE1(bit);
	printf("value:%x, func:%s,line:%d\n",bit, __func__, __LINE__);
	dev_write_mask(dev, FC_INVIDCONF, FC_INVIDCONF_HDCP_KEEPOUT_MASK, 0);
}

void fc_video_VSyncPolarity(hdmi_tx_dev_t *dev, u8 bit)
{
	LOG_TRACE1(bit);
	printf("value:%x, func:%s,line:%d\n",bit, __func__, __LINE__);
	dev_write_mask(dev, FC_INVIDCONF, FC_INVIDCONF_VSYNC_IN_POLARITY_MASK, bit);
}

void fc_video_HSyncPolarity(hdmi_tx_dev_t *dev, u8 bit)
{
	LOG_TRACE1(bit);
	printf("value:%x, func:%s,line:%d\n",bit, __func__, __LINE__);
	dev_write_mask(dev, FC_INVIDCONF, FC_INVIDCONF_HSYNC_IN_POLARITY_MASK, bit);
}

void fc_video_DataEnablePolarity(hdmi_tx_dev_t *dev, u8 bit)
{
	LOG_TRACE1(bit);
	printf("value:%x, func:%s,line:%d\n",bit, __func__, __LINE__);
	dev_write_mask(dev, FC_INVIDCONF, FC_INVIDCONF_DE_IN_POLARITY_MASK, bit);
}

void fc_video_DviOrHdmi(hdmi_tx_dev_t *dev, u8 bit)
{
	LOG_TRACE1(bit);
	/* 1: HDMI; 0: DVI */
	printf("bit:%d, func:%s,line:%d\n",bit, __func__, __LINE__);
	dev_write_mask(dev, FC_INVIDCONF, FC_INVIDCONF_DVI_MODEZ_MASK, bit);
}

void fc_video_VBlankOsc(hdmi_tx_dev_t *dev, u8 bit)
{
	LOG_TRACE1(bit);
	printf("value:%x, func:%s,line:%d\n",bit, __func__, __LINE__);
	dev_write_mask(dev, FC_INVIDCONF, FC_INVIDCONF_R_V_BLANK_IN_OSC_MASK, bit);
}

void fc_video_Interlaced(hdmi_tx_dev_t *dev, u8 bit)
{
	LOG_TRACE1(bit);
	printf("value:%x, func:%s,line:%d\n",bit, __func__, __LINE__);
	dev_write_mask(dev, FC_INVIDCONF, FC_INVIDCONF_IN_I_P_MASK, bit);
}

void fc_video_HActive(hdmi_tx_dev_t *dev, u16 value)
{
	LOG_TRACE1(value);
	/* 12-bit width */
	printf("value:%x, func:%s,line:%d\n",value, __func__, __LINE__);
	dev_write(dev, (FC_INHACTIV0), (u8) (value));
	dev_write_mask(dev, FC_INHACTIV1, FC_INHACTIV1_H_IN_ACTIV_MASK |
									  FC_INHACTIV1_H_IN_ACTIV_12_MASK, (u8)(value >> 8));
}

void fc_video_HBlank(hdmi_tx_dev_t *dev, u16 value)
{
	LOG_TRACE1(value);
	/* 10-bit width */
	printf("value:%x, func:%s,line:%d\n",value, __func__, __LINE__);
	dev_write(dev, (FC_INHBLANK0), (u8) (value));
	dev_write_mask(dev, FC_INHBLANK1, FC_INHBLANK1_H_IN_BLANK_MASK |
									  FC_INHBLANK1_H_IN_BLANK_12_MASK, (u8)(value >> 8));
}

void fc_video_VActive(hdmi_tx_dev_t *dev, u16 value)
{
	LOG_TRACE1(value);
	/* 11-bit width */
	printf("value:%x, func:%s,line:%d\n",value, __func__, __LINE__);
	dev_write(dev, (FC_INVACTIV0), (u8) (value));
	dev_write_mask(dev, FC_INVACTIV1, FC_INVACTIV1_V_IN_ACTIV_MASK |
									  FC_INVACTIV1_V_IN_ACTIV_12_11_MASK, (u8)(value >> 8));
}

void fc_video_VBlank(hdmi_tx_dev_t *dev, u16 value)
{
	LOG_TRACE1(value);
	/* 8-bit width */
	printf("value:%x, func:%s,line:%d\n",value, __func__, __LINE__);
	dev_write(dev, (FC_INVBLANK), (u8) (value));
}

void fc_video_HSyncEdgeDelay(hdmi_tx_dev_t *dev, u16 value)
{
	LOG_TRACE1(value);
	/* 11-bit width */
	printf("value:%x, func:%s,line:%d\n",value, __func__, __LINE__);
	dev_write(dev, (FC_HSYNCINDELAY0), (u8) (value));
	dev_write_mask(dev, FC_HSYNCINDELAY1, FC_HSYNCINDELAY1_H_IN_DELAY_MASK |
										  FC_HSYNCINDELAY1_H_IN_DELAY_12_MASK, (u8)(value >> 8));
}

void fc_video_HSyncPulseWidth(hdmi_tx_dev_t *dev, u16 value)
{
	LOG_TRACE1(value);
	/* 9-bit width */
	printf("value:%x, func:%s,line:%d\n",value, __func__, __LINE__);
	dev_write(dev, (FC_HSYNCINWIDTH0), (u8) (value));
	dev_write_mask(dev, FC_HSYNCINWIDTH1, FC_HSYNCINWIDTH1_H_IN_WIDTH_MASK, (u8)(value >> 8));
}

void fc_video_VSyncEdgeDelay(hdmi_tx_dev_t *dev, u16 value)
{
	LOG_TRACE1(value);
	/* 8-bit width */
	printf("value:%x, func:%s,line:%d\n",value, __func__, __LINE__);
	dev_write(dev, (FC_VSYNCINDELAY), (u8) (value));
}

void fc_video_VSyncPulseWidth(hdmi_tx_dev_t *dev, u16 value)
{
	LOG_TRACE1(value);
	printf("value:%x, func:%s,line:%d\n",value, __func__, __LINE__);
	dev_write_mask(dev, FC_VSYNCINWIDTH, FC_VSYNCINWIDTH_V_IN_WIDTH_MASK, (u8)(value));
}

void fc_video_RefreshRate(hdmi_tx_dev_t *dev, u32 value)
{
	LOG_TRACE1(value);
	/* 20-bit width */
	printf("value:%x, func:%s,line:%d\n",value, __func__, __LINE__);
	dev_write(dev, (FC_INFREQ0), (u8) (value >> 0));
	dev_write(dev, (FC_INFREQ1), (u8) (value >> 8));
	dev_write_mask(dev, FC_INFREQ2, FC_INFREQ2_INFREQ_MASK, (u8)(value >> 16));
}

void fc_video_ControlPeriodMinDuration(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	printf("value:%x, func:%s,line:%d\n",value, __func__, __LINE__);
	dev_write(dev, (FC_CTRLDUR), value);
}

void fc_video_ExtendedControlPeriodMinDuration(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	printf("value:%x, func:%s,line:%d\n",value, __func__, __LINE__);
	dev_write(dev, (FC_EXCTRLDUR), value);
}

void fc_video_ExtendedControlPeriodMaxSpacing(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	printf("value:%x, func:%s,line:%d\n",value, __func__, __LINE__);
	dev_write(dev, (FC_EXCTRLSPAC), value);
}

void fc_video_PreambleFilter(hdmi_tx_dev_t *dev, u8 value, unsigned channel)
{
	LOG_TRACE1(value);
	printf("value:%x, func:%s,line:%d, channel:%d \n",value, __func__, __LINE__,channel);
	if (channel == 0)
		dev_write(dev, (FC_CH0PREAM), value);
	else if (channel == 1)
		dev_write_mask(dev, FC_CH1PREAM, FC_CH1PREAM_CH1_PREAMBLE_FILTER_MASK, value);
	else if (channel == 2)
		dev_write_mask(dev, FC_CH2PREAM, FC_CH2PREAM_CH2_PREAMBLE_FILTER_MASK, value);
	else
		LOGGER(SNPS_ERROR,"invalid channel number: %d", channel);
}

void fc_video_PixelRepetitionInput(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE1(value);
	printf("value:%x, func:%s,line:%d\n",value, __func__, __LINE__);
	dev_write_mask(dev, FC_PRCONF, FC_PRCONF_INCOMING_PR_FACTOR_MASK, value);
}

int fc_video_config(hdmi_tx_dev_t *dev, videoParams_t *video)
{
	const dtd_t *dtd = &video->mDtd;
	u16 i = 0;

	LOG_TRACE();

	if((dev == NULL) || (video == NULL)){
		LOGGER(SNPS_ERROR, "Invalid arguments: dev=%x; video=%x", dev, video);
		return FALSE;
	}

	dtd = &video->mDtd;

	fc_video_VSyncPolarity(dev, dtd->mVSyncPolarity);
	fc_video_HSyncPolarity(dev, dtd->mHSyncPolarity);
	fc_video_DataEnablePolarity(dev, dev->snps_hdmi_ctrl.data_enable_polarity);
	printf("video->mHdmi:%d\n",video->mHdmi);
	fc_video_DviOrHdmi(dev, 1);

	if (video->mHdmiVideoFormat == HDMI_3D_FORMAT) {
		if (video->m3dStructure == FRAME_PACKING_3D) {/* 3d data frame packing is transmitted as a progressive format */
			fc_video_VBlankOsc(dev, 0);
			fc_video_Interlaced(dev, 0);

			if (dtd->mInterlaced) {
				fc_video_VActive(dev, (dtd->mVActive << 2) + 3 * dtd->mVBlanking + 2);
			}
			else {
				fc_video_VActive(dev, (dtd->mVActive << 1) + dtd->mVBlanking);
			}
		}
		else {
			fc_video_VBlankOsc(dev, dtd->mInterlaced);
			fc_video_Interlaced(dev, dtd->mInterlaced);
			fc_video_VActive(dev, dtd->mVActive);
		}
	}
	else {
		fc_video_VBlankOsc(dev, dtd->mInterlaced);
		fc_video_Interlaced(dev, dtd->mInterlaced);
		fc_video_VActive(dev, dtd->mVActive);
	}

	if(video->mEncodingOut == YCC420){
		fc_video_HActive(dev, dtd->mHActive/2);
		fc_video_HBlank(dev, dtd->mHBlanking/2);
		fc_video_HSyncPulseWidth(dev, dtd->mHSyncPulseWidth/2);
		fc_video_HSyncEdgeDelay(dev, dtd->mHSyncOffset/2);
	} else {
		fc_video_HActive(dev, dtd->mHActive);
		fc_video_HBlank(dev, dtd->mHBlanking);
		fc_video_HSyncPulseWidth(dev, dtd->mHSyncPulseWidth);
		fc_video_HSyncEdgeDelay(dev, dtd->mHSyncOffset);
	}

	fc_video_VBlank(dev, dtd->mVBlanking);
	fc_video_VSyncEdgeDelay(dev, dtd->mVSyncOffset);
	fc_video_VSyncPulseWidth(dev, dtd->mVSyncPulseWidth);
	fc_video_ControlPeriodMinDuration(dev, 12);
	fc_video_ExtendedControlPeriodMinDuration(dev, 32);

	/* spacing < 256^2 * config / tmdsClock, spacing <= 50ms
	 * worst case: tmdsClock == 25MHz => config <= 19
	 */
	fc_video_ExtendedControlPeriodMaxSpacing(dev, 1);

	for (i = 0; i < 3; i++)
		fc_video_PreambleFilter(dev, (i + 1) * 11, i);

	fc_video_PixelRepetitionInput(dev, dtd->mPixelRepetitionInput + 1);

	return TRUE;
}

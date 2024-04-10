/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef HALMAINCONTROLLER_H_
#define HALMAINCONTROLLER_H_

#include "../hdmitx_dev.h"
#include "util/types.h"

//void mc_SfrClockDivision(hdmi_tx_dev_t *dev, u8 value);
void mc_disable_all_clocks(hdmi_tx_dev_t *dev);

void mc_enable_all_clocks(hdmi_tx_dev_t *dev);

//void mc_hdcp_clock_enable(hdmi_tx_dev_t *dev, u8 bit);
//
//void mc_cec_clock_enable(hdmi_tx_dev_t *dev, u8 bit);
//
//void mc_colorspace_converter_clock_enable(hdmi_tx_dev_t *dev, u8 bit);
//
void mc_audio_sampler_clock_enable(hdmi_tx_dev_t *dev, u8 bit);
//
//void mc_pixel_repetition_clock_enable(hdmi_tx_dev_t *dev, u8 bit);
//
//void mc_tmds_clock_enable(hdmi_tx_dev_t *dev, u8 bit);
//
//void mc_pixel_clock_enable(hdmi_tx_dev_t *dev, u8 bit);
//
//void mc_cec_clock_reset(hdmi_tx_dev_t *dev, u8 bit);
//
//void mc_audio_gpa_reset(hdmi_tx_dev_t *dev, u8 bit);
//
//void mc_audio_hbr_reset(hdmi_tx_dev_t *dev, u8 bit);
//
//void mc_audio_spdif_reset(hdmi_tx_dev_t *dev, u8 bit);
//
//void mc_audio_i2s_reset(hdmi_tx_dev_t *dev, u8 bit);
//
//void mc_pixel_repetition_clock_reset(hdmi_tx_dev_t *dev, u8 bit);
//
void mc_tmds_clock_reset(hdmi_tx_dev_t *dev, u8 bit);
//
//void mc_pixel_clock_reset(hdmi_tx_dev_t *dev, u8 bit);
//
//void mc_video_feed_through_off(hdmi_tx_dev_t *dev, u8 bit);
//
////void mc_PhyReset(hdmi_tx_dev_t *dev, u8 bit);
//
void mc_phy_reset(hdmi_tx_dev_t *dev, u8 bit);
//
//void mc_heac_phy_reset(hdmi_tx_dev_t *dev, u8 bit);
//
//u8 mc_lock_on_clock_status(hdmi_tx_dev_t *dev, u8 clockDomain);
//
//void mc_lock_on_clock_clear(hdmi_tx_dev_t *dev, u8 clockDomain);

#endif	/* HALMAINCONTROLLER_H_ */

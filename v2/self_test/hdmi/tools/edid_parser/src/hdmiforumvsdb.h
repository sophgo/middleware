/*
 * hdmiforumvsdb.h
 *
 *  Created on: Jun 4, 2014
 *      Author: 
 */

#ifndef HDMIFORUMVSDB_H_
#define HDMIFORUMVSDB_H_

#include "includes.h"

/* HDMI 2.0 HF_VSDB */
typedef struct {
	u32 mIeee_Oui;

	u8 mValid;

	u8 mVersion;

	u8 mMaxTmdsCharRate;

	u8 m3D_OSD_Disparity;

	u8 mDualView;

	u8 mIndependentView;

	u8 mLTS_340Mcs_scramble;

	u8 mRR_Capable;

	u8 mSCDC_Present;

	u8 mDC_30bit_420;

	u8 mDC_36bit_420;

	u8 mDC_48bit_420;

} hdmiforumvsdb_t;

void hdmiforumvsdb_reset(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb);

int hdmiforumvsdb_parse(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb, u8 * data);

int hdmiforumvsdb_GetDeepColor30_420(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb);

int hdmiforumvsdb_GetDeepColor36_420(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb);

int hdmivsdb_GetDeepColor48_420(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb);

int hdmivsdb_GetSCDC_Present(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb);

int hdmivsdb_GetDual_View(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb);

u8 hdmivsdb_GetMaxTmdsCharRate(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb);

u16 hdmivsdb_Get3D_OSD_Disparity(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb);

u16 hdmivsdb_GetVersion(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb);

u16 hdmivsdb_GetIndependent_View(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb);

u16 hdmivsdb_GetRR_Capable(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb);

u16 hdmivsdb_GetLTS_340Msc_scramble(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb);
#endif				/* HDMIFORUMVSDB_H_ */

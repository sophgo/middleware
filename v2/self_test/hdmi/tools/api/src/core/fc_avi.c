// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "frame_composer/frame_composer_reg.h"
#include "core/fc_avi.h"
#include "bsp/access.h"
#include "util/log.h"

void fc_RgbYcc(hdmi_tx_dev_t *dev, u8 type)
{
	LOG_TRACE1(type);
	dev_write_mask(dev, FC_AVICONF0, FC_AVICONF0_RGC_YCC_INDICATION_MASK, type);
}

void fc_ScanInfo(hdmi_tx_dev_t *dev, u8 left)
{
	LOG_TRACE1(left);
	dev_write_mask(dev, FC_AVICONF0, FC_AVICONF0_SCAN_INFORMATION_MASK, left);
}

void fc_Colorimetry(hdmi_tx_dev_t *dev, unsigned cscITU)
{
	LOG_TRACE1(cscITU);
	dev_write_mask(dev, FC_AVICONF1, FC_AVICONF1_COLORIMETRY_MASK, cscITU);
}

void fc_PicAspectRatio(hdmi_tx_dev_t *dev, u8 ar)
{
	LOG_TRACE1(ar);
	dev_write_mask(dev, FC_AVICONF1, FC_AVICONF1_PICTURE_ASPECT_RATIO_MASK, ar);
}

void fc_ActiveAspectRatioValid(hdmi_tx_dev_t *dev, u8 valid)
{
	LOG_TRACE1(valid);
	dev_write_mask(dev, FC_AVICONF0, FC_AVICONF0_ACTIVE_FORMAT_PRESENT_MASK, valid);
}

void fc_ActiveFormatAspectRatio(hdmi_tx_dev_t *dev, u8 left)
{
	LOG_TRACE1(left);
	dev_write_mask(dev, FC_AVICONF1, FC_AVICONF1_ACTIVE_ASPECT_RATIO_MASK, left);
}

void fc_IsItContent(hdmi_tx_dev_t *dev, u8 it)
{
	LOG_TRACE1(it);
	dev_write_mask(dev, FC_AVICONF2, FC_AVICONF2_IT_CONTENT_MASK, (it ? 1 : 0));
}

void fc_ExtendedColorimetry(hdmi_tx_dev_t *dev, u8 extColor)
{
	LOG_TRACE1(extColor);
	dev_write_mask(dev, FC_AVICONF2, FC_AVICONF2_EXTENDED_COLORIMETRY_MASK, extColor);
	dev_write_mask(dev, FC_AVICONF1, FC_AVICONF1_COLORIMETRY_MASK, 0x3);
}

void fc_QuantizationRange(hdmi_tx_dev_t *dev, u8 range)
{
	LOG_TRACE1(range);
	dev_write_mask(dev, FC_AVICONF2, FC_AVICONF2_QUANTIZATION_RANGE_MASK, range);
}

void fc_NonUniformPicScaling(hdmi_tx_dev_t *dev, u8 scale)
{
	LOG_TRACE1(scale);
	dev_write_mask(dev, FC_AVICONF2, FC_AVICONF2_NON_UNIFORM_PICTURE_SCALING_MASK, scale);
}

void fc_VideoCode(hdmi_tx_dev_t *dev, u8 code)
{
	LOG_TRACE1(code);
	dev_write(dev, FC_AVIVID, code);
}

void fc_HorizontalBarsValid(hdmi_tx_dev_t *dev, u8 validity)
{
	dev_write_mask(dev, FC_AVICONF0, FC_AVICONF0_BAR_INFORMATION_MASK & 0x8, (validity ? 1 : 0));
}

void fc_HorizontalBars(hdmi_tx_dev_t *dev, u16 endTop, u16 startBottom)
{
	LOG_TRACE2(endTop, startBottom);
	dev_write(dev, FC_AVIETB0, (u8) (endTop));
	dev_write(dev, FC_AVIETB1, (u8) (endTop >> 8));
	dev_write(dev, FC_AVISBB0, (u8) (startBottom));
	dev_write(dev, FC_AVISBB1, (u8) (startBottom >> 8));
}

void fc_VerticalBarsValid(hdmi_tx_dev_t *dev, u8 validity)
{
	dev_write_mask(dev, FC_AVICONF0, FC_AVICONF0_BAR_INFORMATION_MASK & 0x4, (validity ? 1 : 0));
}

void fc_VerticalBars(hdmi_tx_dev_t *dev, u16 endLeft, u16 startRight)
{
	LOG_TRACE2(endLeft, startRight);
	dev_write(dev, FC_AVIELB0, (u8) (endLeft));
	dev_write(dev, FC_AVIELB1, (u8) (endLeft >> 8));
	dev_write(dev, FC_AVISRB0, (u8) (startRight));
	dev_write(dev, FC_AVISRB1, (u8) (startRight >> 8));
}

void fc_OutPixelRepetition(hdmi_tx_dev_t *dev, u8 pr)
{
	LOG_TRACE1(pr);
	dev_write_mask(dev, FC_PRCONF, FC_PRCONF_OUTPUT_PR_FACTOR_MASK, pr);
}

u32 fc_GetInfoFrameSatus(hdmi_tx_dev_t *dev)
{
	return dev_read(dev, FC_AVICONF0);
}

void fc_avi_config(hdmi_tx_dev_t *dev, videoParams_t *videoParams)
{
	u16 endTop = 0;
	u16 startBottom = 0;
	u16 endLeft = 0;
	u16 startRight = 0;
	dtd_t *dtd = &videoParams->mDtd;

	LOG_TRACE();

	if (videoParams->mEncodingOut == RGB) {
		LOGGER(SNPS_INFO,"%s:rgb", __func__);
		fc_RgbYcc(dev, 0);
	}
	else if (videoParams->mEncodingOut == YCC422) {
		LOGGER(SNPS_INFO,"%s:ycc422", __func__);
		fc_RgbYcc(dev, 1);
	}
	else if (videoParams->mEncodingOut == YCC444) {
		LOGGER(SNPS_INFO,"%s:ycc444", __func__);
		fc_RgbYcc(dev, 2);
	}
	else if (videoParams->mEncodingOut == YCC420) {
		LOGGER(SNPS_INFO,"%s:ycc420", __func__);
		fc_RgbYcc(dev, 3);
	}

	fc_ActiveFormatAspectRatio(dev, 0x8);

	LOGGER(SNPS_INFO, "%s:infoframe status %x", __func__,
			fc_GetInfoFrameSatus(dev));

	fc_ScanInfo(dev, videoParams->mScanInfo);

	if (dtd->mHImageSize != 0 || dtd->mVImageSize != 0) {
		u8 pic = (dtd->mHImageSize * 10) % dtd->mVImageSize;
		// 16:9 or 4:3
		fc_PicAspectRatio(dev, (pic > 5) ? 2 : 1);
	}
	else {
		// No Data
		fc_PicAspectRatio(dev, 0);
	}

	fc_IsItContent(dev, videoParams->mItContent);

	fc_QuantizationRange(dev, videoParams->mRgbQuantizationRange);
	fc_NonUniformPicScaling(dev, videoParams->mNonUniformScaling);
	if (dtd->mCode != (u8) (-1)) {
		if (videoParams->mHdmi20 == 1) {
			fc_VideoCode(dev, dtd->mCode);
		} else {
			if (dtd->mCode < 90) {
				fc_VideoCode(dev, dtd->mCode);
			} else {
				fc_VideoCode(dev, 0);
			}
		}
	} else {
		fc_VideoCode(dev, 0);
	}
	if (videoParams->mColorimetry == EXTENDED_COLORIMETRY) { /* ext colorimetry valid */
		if (videoParams->mExtColorimetry != (u8) (-1)) {
			fc_ExtendedColorimetry(dev, videoParams->mExtColorimetry);
			fc_Colorimetry(dev, videoParams->mColorimetry);	/* EXT-3 */
		} else {
			fc_Colorimetry(dev, 0);	/* No Data */
		}
	} else {
		fc_Colorimetry(dev, videoParams->mColorimetry);	/* NODATA-0/ 601-1/ 709-2/ EXT-3 */
	}
	if (videoParams->mActiveFormatAspectRatio != 0) {
		fc_ActiveFormatAspectRatio(dev, videoParams->mActiveFormatAspectRatio);
		fc_ActiveAspectRatioValid(dev, 1);
	} else {
		fc_ActiveAspectRatioValid(dev, 0);
	}
	if (videoParams->mEndTopBar != (u16) (-1) || videoParams->mStartBottomBar != (u16) (-1)) {
		if (videoParams->mEndTopBar != (u16) (-1)) {
			endTop = videoParams->mEndTopBar;
		}
		if (videoParams->mStartBottomBar != (u16) (-1)) {
			startBottom = videoParams->mStartBottomBar;
		}
		fc_HorizontalBars(dev, endTop, startBottom);
		fc_HorizontalBarsValid(dev, 1);
	} else {
		fc_HorizontalBarsValid(dev, 0);
	}
	if (videoParams->mEndLeftBar != (u16) (-1) || videoParams->mStartRightBar != (u16) (-1)) {
		if (videoParams->mEndLeftBar != (u16) (-1)) {
			endLeft = videoParams->mEndLeftBar;
		}
		if (videoParams->mStartRightBar != (u16) (-1)) {
			startRight = videoParams->mStartRightBar;
		}
		fc_VerticalBars(dev, endLeft, startRight);
		fc_VerticalBarsValid(dev, 1);
	} else {
		fc_VerticalBarsValid(dev, 0);
	}
	fc_OutPixelRepetition(dev, (dtd->mPixelRepetitionInput + 1) * (videoParams->mPixelRepetitionFactor + 1) - 1);
}

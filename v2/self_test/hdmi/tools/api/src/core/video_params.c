// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "core/video_params.h"
#include "util/log.h"

int videoParams_GetCeaVicCode(int hdmi_vic_code)
{
	switch(hdmi_vic_code)
	{
	case 1:
		return 95;
		break;
	case 2:
		return 94;
		break;
	case 3:
		return 93;
		break;
	case 4:
		return 98;
		break;
	default:
		return -1;
		break;
	}
	return -1;
}

int videoParams_GetHdmiVicCode(int cea_code)
{
	switch(cea_code)
	{
	case 95:
		return 1;
		break;
	case 94:
		return 2;
		break;
	case 93:
		return 3;
		break;
	case 98:
		return 4;
		break;
	default:
		return -1;
		break;
	}
	return -1;
}

void video_params_reset(hdmi_tx_dev_t *dev, videoParams_t * params)
{
	dtd_t dtd;
	dtd_fill(dev, &dtd, 1, 60.000);

	params->mHdmi = MODE_UNDEFINED;
	params->mEncodingOut = RGB;
	params->mEncodingIn = RGB;
	params->mColorResolution = COLOR_DEPTH_8;
	params->mPixelRepetitionFactor = 0;
	params->mRgbQuantizationRange = 0;
	params->mPixelPackingDefaultPhase = 0;
	params->mColorimetry = 0;
	params->mScanInfo = 0;
	params->mActiveFormatAspectRatio = 8;
	params->mNonUniformScaling = 0;
	params->mExtColorimetry = ~0;
	params->mItContent = 0;
	params->mEndTopBar = ~0;
	params->mStartBottomBar = ~0;
	params->mEndLeftBar = ~0;
	params->mStartRightBar = ~0;
	params->mCscFilter = 0;
	params->mHdmiVideoFormat = UNDEFINED_FORMAT;
	params->m3dStructure = UNDEFINED_3D;
	params->m3dExtData = UNDEFINED_EXTDATA_3D;
	params->mHdmiVic = 0;
	params->mHdmi20 = 0;

	memcpy(&params->mDtd, &dtd, sizeof(dtd_t));

//	params->mDtd.mCode = 0;
//	params->mDtd.mLimitedToYcc420 = 0xFF;
//	params->mDtd.mYcc420 = 0xFF;
//	params->mDtd.mPixelRepetitionInput = 0xFF;
//	params->mDtd.mPixelClock = 0.0;
//	params->mDtd.mInterlaced = 0xFF;
//	params->mDtd.mHActive = 0;
//	params->mDtd.mHBlanking = 0;
//	params->mDtd.mHBorder = 0xFFFF;
//	params->mDtd.mHImageSize = 0;
//	params->mDtd.mHSyncOffset = 0;
//	params->mDtd.mHSyncPulseWidth = 0;
//	params->mDtd.mHSyncPolarity = 0xFF;
//	params->mDtd.mVActive = 0;
//	params->mDtd.mVBlanking = 0;
//	params->mDtd.mVBorder = 0xFFFF;
//	params->mDtd.mVImageSize = 0;
//	params->mDtd.mVSyncOffset = 0;
//	params->mDtd.mVSyncPulseWidth = 0;
//	params->mDtd.mVSyncPolarity = 0xFF;
}

u16 *videoParams_GetCscA(hdmi_tx_dev_t *dev, videoParams_t * params)
{
	videoParams_UpdateCscCoefficients(dev, params);
	return params->mCscA;
}

void videoParams_SetCscA(hdmi_tx_dev_t *dev, videoParams_t * params, u16 value[4])
{
	u16 i = 0;
	for (i = 0; i < sizeof(params->mCscA) / sizeof(params->mCscA[0]); i++) {
		params->mCscA[i] = value[i];
	}
}

u16 *videoParams_GetCscB(hdmi_tx_dev_t *dev, videoParams_t * params)
{
	videoParams_UpdateCscCoefficients(dev, params);
	return params->mCscB;
}

void videoParams_SetCscB(hdmi_tx_dev_t *dev, videoParams_t * params, u16 value[4])
{
	u16 i = 0;
	for (i = 0; i < sizeof(params->mCscB) / sizeof(params->mCscB[0]); i++) {
		params->mCscB[i] = value[i];
	}
}

u16 *videoParams_GetCscC(hdmi_tx_dev_t *dev, videoParams_t * params)
{
	videoParams_UpdateCscCoefficients(dev, params);
	return params->mCscC;
}

void videoParams_SetCscC(hdmi_tx_dev_t *dev, videoParams_t * params, u16 value[4])
{
	u16 i = 0;
	for (i = 0; i < sizeof(params->mCscC) / sizeof(params->mCscC[0]); i++) {
		params->mCscC[i] = value[i];
	}
}

void videoParams_SetCscScale(hdmi_tx_dev_t *dev, videoParams_t * params, u16 value)
{
	params->mCscScale = value;
}

/* [0.01 MHz] */
double videoParams_GetPixelClock(hdmi_tx_dev_t *dev, videoParams_t * params)
{
	double pixelClock = 0;

	pixelClock = params->mDtd.mPixelClock;

	if ((params->mHdmiVideoFormat == HDMI_3D_FORMAT) && (params->m3dStructure == FRAME_PACKING_3D)) {
			pixelClock =  2 * pixelClock;
	}
#ifdef PHY_THIRD_PARTY
	//TODO: THIS SHOULD ONLY BE APPLIED FOR THIRD_PARTY ACK PHY
	pixelClock = pixelClock * videoParams_GetRatioClock(dev, params);
#endif
	LOGGER(SNPS_INFO, "Pixel clock %.3fMHz");

	return pixelClock;
}

/* 0.01 */
double videoParams_GetRatioClock(hdmi_tx_dev_t *dev, videoParams_t * params)
{
	double ratio = 1.000;

	if (params->mEncodingOut != YCC422) {
		if (params->mColorResolution == COLOR_DEPTH_8) {
			ratio = 1.000;
		} else if (params->mColorResolution == COLOR_DEPTH_10) {
			ratio = 1.250;
		} else if (params->mColorResolution == COLOR_DEPTH_12) {
			ratio = 1.500;
		} else if (params->mColorResolution == COLOR_DEPTH_16) {
			ratio = 2.000;
		}
	}
	return ratio * (params->mPixelRepetitionFactor + 1);
}

int videoParams_IsColorSpaceConversion(hdmi_tx_dev_t *dev, videoParams_t * params)
{
	return params->mEncodingIn != params->mEncodingOut;
}

int videoParams_IsColorSpaceDecimation(hdmi_tx_dev_t *dev, videoParams_t * params)
{
	return params->mEncodingOut == YCC422 && (params->mEncodingIn == RGB
			|| params->mEncodingIn ==
					YCC444);
}

int videoParams_IsColorSpaceInterpolation(hdmi_tx_dev_t *dev, videoParams_t * params)
{
	return params->mEncodingIn == YCC422 && (params->mEncodingOut == RGB
			|| params->mEncodingOut ==
					YCC444);
}

int videoParams_IsPixelRepetition(hdmi_tx_dev_t *dev, videoParams_t * params)
{
	return (params->mPixelRepetitionFactor > 0) || (params->mDtd.mPixelRepetitionInput > 0);
}

void videoParams_UpdateCscCoefficients(hdmi_tx_dev_t *dev, videoParams_t * params)
{
	u16 i = 0;
	if (!videoParams_IsColorSpaceConversion(dev, params)) {
		for (i = 0; i < 4; i++) {
			params->mCscA[i] = 0;
			params->mCscB[i] = 0;
			params->mCscC[i] = 0;
		}
		params->mCscA[0] = 0x2000;
		params->mCscB[1] = 0x2000;
		params->mCscC[2] = 0x2000;
		params->mCscScale = 1;
	} else if (videoParams_IsColorSpaceConversion(dev, params) && params->mEncodingOut == RGB) {
		if (params->mColorimetry == ITU601) {
			params->mCscA[0] = 0x2000;
			params->mCscA[1] = 0x6926;
			params->mCscA[2] = 0x74fd;
			params->mCscA[3] = 0x010e;

			params->mCscB[0] = 0x2000;
			params->mCscB[1] = 0x2cdd;
			params->mCscB[2] = 0x0000;
			params->mCscB[3] = 0x7e9a;

			params->mCscC[0] = 0x2000;
			params->mCscC[1] = 0x0000;
			params->mCscC[2] = 0x38b4;
			params->mCscC[3] = 0x7e3b;

			params->mCscScale = 1;
		} else if (params->mColorimetry == ITU709) {
			params->mCscA[0] = 0x2000;
			params->mCscA[1] = 0x7106;
			params->mCscA[2] = 0x7a02;
			params->mCscA[3] = 0x00a7;

			params->mCscB[0] = 0x2000;
			params->mCscB[1] = 0x3264;
			params->mCscB[2] = 0x0000;
			params->mCscB[3] = 0x7e6d;

			params->mCscC[0] = 0x2000;
			params->mCscC[1] = 0x0000;
			params->mCscC[2] = 0x3b61;
			params->mCscC[3] = 0x7e25;

			params->mCscScale = 1;
		}
	} else if (videoParams_IsColorSpaceConversion(dev, params) && params->mEncodingIn == RGB) {
		if (params->mColorimetry == ITU601) {
			params->mCscA[0] = 0x2591;
			params->mCscA[1] = 0x1322;
			params->mCscA[2] = 0x074b;
			params->mCscA[3] = 0x0000;

			params->mCscB[0] = 0x6535;
			params->mCscB[1] = 0x2000;
			params->mCscB[2] = 0x7acc;
			params->mCscB[3] = 0x0200;

			params->mCscC[0] = 0x6acd;
			params->mCscC[1] = 0x7534;
			params->mCscC[2] = 0x2000;
			params->mCscC[3] = 0x0200;

			params->mCscScale = 0;
		} else if (params->mColorimetry == ITU709) {
			params->mCscA[0] = 0x2dc5;
			params->mCscA[1] = 0x0d9b;
			params->mCscA[2] = 0x049e;
			params->mCscA[3] = 0x0000;

			params->mCscB[0] = 0x62f0;
			params->mCscB[1] = 0x2000;
			params->mCscB[2] = 0x7d11;
			params->mCscB[3] = 0x0200;

			params->mCscC[0] = 0x6756;
			params->mCscC[1] = 0x78ab;
			params->mCscC[2] = 0x2000;
			params->mCscC[3] = 0x0200;

			params->mCscScale = 0;
		}
	}
	/* else use user coefficients */
}

void videoParams_SetYcc420Support(hdmi_tx_dev_t *dev, dtd_t * paramsDtd, shortVideoDesc_t * paramsSvd)
{
	paramsDtd->mLimitedToYcc420 = paramsSvd->mLimitedToYcc420;
	paramsDtd->mYcc420 = paramsSvd->mYcc420;
	LOGGER(SNPS_DEBUG,"set ParamsDtd->limite %d", paramsDtd->mLimitedToYcc420);
	LOGGER(SNPS_DEBUG,"set ParamsDtd->supports %d", paramsDtd->mYcc420);
	LOGGER(SNPS_DEBUG,"set ParamsSvd->limite %d", paramsSvd->mLimitedToYcc420);
	LOGGER(SNPS_DEBUG,"set ParamsSvd->supports %d", paramsSvd->mYcc420);
}


char * getEncodingString(encoding_t encoding)
{
	switch (encoding){
		case 	RGB: return "RGB";
		case	YCC444: return "YCbCr-444";
		case	YCC422: return "YCbCr-422";
		case	YCC420: return "YCbCr-420";
		default :break;
	}
	return "Undefined";
}

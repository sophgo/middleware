// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "app/video/video.h"

#include <hdmitx_dev.h>

#include "includes.h"
#include "hdmitx_ipk_api/hdmitx_ipk_api.h"
#include "hdmi_tx_app.h"
#include "platform.h"
#include "util/general_ops.h"
#include "app/edid/edid.h"
#include "app/hdmitx.h"


void update_dtd_cfg(dtd_t * user, dtd_t * cfg)
{
	if(user->mCode != 0)
		cfg->mCode = user->mCode;

	if(user->mLimitedToYcc420 != 0xFF)
		cfg->mLimitedToYcc420 = user->mLimitedToYcc420;

	if(user->mYcc420 != 0xFF)
		cfg->mYcc420 = user->mYcc420;

	if(user->mPixelRepetitionInput != 0xFF)
		cfg->mPixelRepetitionInput = user->mPixelRepetitionInput;

	if(!double_is_equal(user->mPixelClock, 0.0))
		cfg->mPixelClock = user->mPixelClock;

	if(user->mInterlaced != 0xFF)
		cfg->mInterlaced = user->mInterlaced;

	if(user->mHActive != 0)
		cfg->mHActive = user->mHActive;

	if(user->mHBlanking != 0)
		cfg->mHBlanking = user->mHBlanking;

	if(user->mHBorder != 0xFFFF)
		cfg->mHBorder = user->mHBorder;

	if(user->mHImageSize != 0)
		cfg->mHImageSize = user->mHImageSize;

	if(user->mHSyncOffset != 0)
		cfg->mHSyncOffset = user->mHSyncOffset;

	if(user->mHSyncPulseWidth != 0)
		cfg->mHSyncPulseWidth = user->mHSyncPulseWidth;

	if(user->mHSyncPolarity != 0xFF)
		cfg->mHSyncPolarity = user->mHSyncPolarity;

	if(user->mVActive != 0)
		cfg->mVActive = user->mVActive;

	if(user->mVBlanking != 0)
		cfg->mVBlanking = user->mVBlanking;

	if(user->mVBorder != 0xFFFF)
		cfg->mVBorder = user->mVBorder;

	if(user->mVImageSize != 0)
		cfg->mVImageSize = user->mVImageSize;

	if(user->mVSyncOffset != 0)
		cfg->mVSyncOffset = user->mVSyncOffset;

	if(user->mVSyncPulseWidth != 0)
		cfg->mVSyncPulseWidth = user->mVSyncPulseWidth;

	if(user->mVSyncPolarity != 0xFF)
		cfg->mVSyncPolarity = user->mVSyncPolarity;
}


void update_video_cfg(videoParams_t * user, videoParams_t * cfg)
{
	if(user->mHdmi != MODE_UNDEFINED)
		cfg->mHdmi = user->mHdmi;

	if(user->mEncodingOut != ENC_UNDEFINED)
		cfg->mEncodingOut = user->mEncodingOut;

	if(user->mEncodingIn != ENC_UNDEFINED)
		cfg->mEncodingIn = user->mEncodingIn;

	if(user->mColorResolution != COLOR_DEPTH_INVALID)
		cfg->mColorResolution = user->mColorResolution;

	if(user->mPixelRepetitionFactor != 0)
		cfg->mPixelRepetitionFactor = user->mPixelRepetitionFactor;

	if(user->mRgbQuantizationRange != 0)
		cfg->mRgbQuantizationRange = user->mRgbQuantizationRange;

	if(user->mPixelPackingDefaultPhase != 0)
		cfg->mPixelPackingDefaultPhase = user->mPixelPackingDefaultPhase;

	if(user->mColorimetry != UNDEFINED_COLORIMETRY)
		cfg->mColorimetry = user->mColorimetry;

	if(user->mScanInfo != 0)
		cfg->mScanInfo = user->mScanInfo;

	if(user->mActiveFormatAspectRatio != 0)
		cfg->mActiveFormatAspectRatio = user->mActiveFormatAspectRatio;

	if(user->mNonUniformScaling != 0)
		cfg->mNonUniformScaling = user->mNonUniformScaling;

	if(user->mExtColorimetry != 0)
		cfg->mExtColorimetry = user->mExtColorimetry;

	if(user->mItContent != 0)
		cfg->mItContent = user->mItContent;

	if(user->mEndTopBar != (u16)(~0))
		cfg->mEndTopBar = user->mEndTopBar;

	if(user->mStartBottomBar != (u16)(~0))
		cfg->mStartBottomBar = user->mStartBottomBar;

	if(user->mEndLeftBar != 0)
		cfg->mEndLeftBar = user->mEndLeftBar;

	if(user->mStartRightBar != (u16)(~0))
		cfg->mStartRightBar = user->mStartRightBar;

	if(user->mCscFilter != 0)
		cfg->mCscFilter = user->mCscFilter;

	if((user->mCscA[0] != 0) || (user->mCscA[1] != 0) || (user->mCscA[2] != 0) || (user->mCscA[3] != 0))
		memcpy(cfg->mCscA, user->mCscA, sizeof(cfg->mCscA[0])*4);

	if((user->mCscB[0] != 0) || (user->mCscB[1] != 0) || (user->mCscB[2] != 0) || (user->mCscB[3] != 0))
		memcpy(cfg->mCscB, user->mCscB, sizeof(cfg->mCscB[0])*4);

	if((user->mCscC[0] != 0) || (user->mCscC[1] != 0) || (user->mCscC[2] != 0) || (user->mCscC[3] != 0))
		memcpy(cfg->mCscC, user->mCscC, sizeof(cfg->mCscC[0])*4);

	if(user->mCscScale != 0)
		cfg->mCscScale = user->mCscScale;

	if(user->mHdmiVic != UNDEFINED_HDMI_VIC)
		cfg->mHdmiVic = user->mHdmiVic;

	if (user->mHdmiVideoFormat != UNDEFINED_FORMAT)
		cfg->mHdmiVideoFormat = user->mHdmiVideoFormat;

	/* if the user has not configured or commuted back to non-3d format */
	if(user->m3dStructure != UNDEFINED_3D && user->mHdmiVideoFormat != HDMI_NORMAL_FORMAT)
		cfg->m3dStructure = user->m3dStructure;

	if(user->m3dExtData != UNDEFINED_EXTDATA_3D)
		cfg->m3dExtData = user->m3dExtData;

	update_dtd_cfg(&(user->mDtd), &(cfg->mDtd));
}

char * get_3d_structure_name(u8 m3dStructure)
{
	switch (m3dStructure){
	case FRAME_PACKING_3D: return "Frame Packing";
	case TOP_AND_BOTTOM_3D: return "Top and Bottom";
	case SIDE_BY_SIDE_3D: return "Side By Side (Half)";
	case UNDEFINED_3D: return "Disabled";
	}
	return "Unknown";
}

char * get_videoinfo(videoParams_t *pVideo)
{
	char * video_info;
	double refresh_rate = dtd_get_refresh_rate(&pVideo->mDtd);
	int length = 0;

	video_info = (char *) calloc(150, sizeof(char));
	length += sprintf(video_info, "CEA VIC=%3d:", pVideo->mDtd.mCode);

	if(pVideo->mDtd.mInterlaced)
		length += sprintf(video_info+ length," %4dx%di", 
				pVideo->mEncodingIn == YCC420 ? pVideo->mDtd.mHActive*2 : pVideo->mDtd.mHActive,
						pVideo->mDtd.mVActive*2);
	else
		length += sprintf(video_info+ length," %4dx%dp",
				pVideo->mEncodingIn == YCC420 ? pVideo->mDtd.mHActive*2 : pVideo->mDtd.mHActive,
						pVideo->mDtd.mVActive);
	length += sprintf(video_info+ length," @");
	length += sprintf(video_info+ length," %3.3fHz", refresh_rate);
	length += sprintf(video_info + length," %d:%d,", pVideo->mDtd.mHImageSize, pVideo->mDtd.mVImageSize);
	length += sprintf(video_info + length," %2d-bits", pVideo->mColorResolution * 3);
	length += sprintf(video_info + length," %s,", getEncodingString(pVideo->mEncodingIn));
	if(pVideo->mHdmiVideoFormat == HDMI_3D_FORMAT)
		length += sprintf(video_info + length, " %s (3D)", get_3d_structure_name(pVideo->m3dStructure));

	length += sprintf(video_info + length," %s", pVideo->mHdmi == HDMI ? "HDMI" : "DVI");
	return video_info;
}

//void print_videoinfo(videoParams_t *pVideo)
//{
//char * video_info = get_videoinfo(pVideo);
//printf("%s\n", video_info);
//free(video_info);
//}


void print_videoinfo(videoParams_t *pVideo)
{
	double refresh_rate = dtd_get_refresh_rate(&pVideo->mDtd);
	printf("CEA VIC=%d: ", pVideo->mDtd.mCode);
	if(pVideo->mDtd.mInterlaced)
		printf("%dx%di",pVideo->mDtd.mHActive, pVideo->mDtd.mVActive*2);
	else
		printf("%dx%dp", pVideo->mDtd.mHActive,	pVideo->mDtd.mVActive);
	printf("@ ");
	printf("%3.3f Hz ", refresh_rate);
	printf("%d:%d, ", pVideo->mDtd.mHImageSize, pVideo->mDtd.mVImageSize);
	printf("%d-bpp ", pVideo->mColorResolution);
	printf("%s, ", getEncodingString(pVideo->mEncodingIn));
	printf("%s\n", pVideo->mHdmi == HDMI ? "HDMI" : "DVI");
	printf("3D Format: %s\n", (pVideo->mHdmiVideoFormat != HDMI_3D_FORMAT)? "None" : get_3d_structure_name(pVideo->m3dStructure));
}


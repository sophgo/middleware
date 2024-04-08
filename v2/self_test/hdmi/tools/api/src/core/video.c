// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */
#include "stdio.h"
#include "core/video.h"
#include "util/log.h"
#include "core/csc.h"
#include "core/fc_video.h"
#include "core/main_controller.h"
#include "core/video_packetizer.h"
#include "core/video_sampler.h"
#include "core/fc_debug.h"
#include "edid/dtd.h"
#include "util/error.h"


int video_Initialize(hdmi_tx_dev_t *dev, videoParams_t * video, u8 dataEnablePolarity)
{
	LOG_TRACE1(dataEnablePolarity);
	return TRUE;
}

int video_Configure(hdmi_tx_dev_t *dev, videoParams_t * video)
{
	LOG_TRACE();

	/* DVI mode does not support pixel repetition */
	if ((video->mHdmi == DVI) && videoParams_IsPixelRepetition(dev, video)) {
		error_set(ERR_DVI_MODE_WITH_PIXEL_REPETITION);
		LOGGER(SNPS_ERROR,"DVI mode with pixel repetition: video not transmitted");
		return FALSE;
	}

	fc_force_output(dev, 1);

	if (fc_video_config(dev, video) == FALSE)
		return FALSE;
	if (video_VideoPacketizer(dev, video) == FALSE)
		return FALSE;
	if (video_ColorSpaceConverter(dev, video) == FALSE)
		return FALSE;
	if (video_VideoSampler(dev, video) == FALSE)
		return FALSE;


	printf("*******video_Configure******\n");
	return TRUE;
}

int video_ColorSpaceConverter(hdmi_tx_dev_t *dev, videoParams_t * video)
{
	unsigned interpolation = 0;
	unsigned decimation = 0;
	unsigned color_depth = 0;

	LOG_TRACE();
	printf("*******CSC******\n");
	if (videoParams_IsColorSpaceInterpolation(dev, video)) {
		if (video->mCscFilter > 1) {
			error_set(ERR_CHROMA_INTERPOLATION_FILTER_INVALID);
			LOGGER(SNPS_ERROR,"invalid chroma interpolation filter: %d", video->mCscFilter);
			return FALSE;
		}
		interpolation = 1 + video->mCscFilter;
	}
	else if (videoParams_IsColorSpaceDecimation(dev, video)) {
		if (video->mCscFilter > 2) {
			error_set(ERR_CHROMA_DECIMATION_FILTER_INVALID);
			LOGGER(SNPS_ERROR,"invalid chroma decimation filter: %d", video->mCscFilter);
			return FALSE;
		}
		decimation = 1 + video->mCscFilter;
	}

	if ((video->mColorResolution == COLOR_DEPTH_8) || (video->mColorResolution == 0))
		color_depth = 4;
	else if (video->mColorResolution == COLOR_DEPTH_10)
		color_depth = 5;
	else if (video->mColorResolution == COLOR_DEPTH_12)
		color_depth = 6;
	else if (video->mColorResolution == COLOR_DEPTH_16)
		color_depth = 7;
	else {
		error_set(ERR_COLOR_DEPTH_NOT_SUPPORTED);
		LOGGER(SNPS_ERROR,"invalid color depth: %d", video->mColorResolution);
		return FALSE;
	}

	csc_config(dev, video, interpolation, decimation, color_depth);

	return TRUE;
}

int video_VideoPacketizer(hdmi_tx_dev_t *dev, videoParams_t * video)
{
	unsigned color_depth = 0;
	unsigned remap_size = 0;
	unsigned output_select = 0;

	LOG_TRACE();
	if ((video->mEncodingOut == RGB) || (video->mEncodingOut == YCC444) || (video->mEncodingOut == YCC420)) {
		if (video->mColorResolution == 0)
			output_select = 3;
		else if (video->mColorResolution == COLOR_DEPTH_8) {
			color_depth = 0;
			output_select = 3;
		} else if (video->mColorResolution == COLOR_DEPTH_10)
			color_depth = 5;
		else if (video->mColorResolution == COLOR_DEPTH_12)
			color_depth = 6;
		else if (video->mColorResolution == COLOR_DEPTH_16)
			color_depth = 7;
		else {
			error_set(ERR_COLOR_DEPTH_NOT_SUPPORTED);
			LOGGER(SNPS_ERROR,"invalid color depth: %d", video->mColorResolution);
			return FALSE;
		}
	}
	else if (video->mEncodingOut == YCC422) {
		if ((video->mColorResolution == COLOR_DEPTH_8) || (video->mColorResolution == 0))
			remap_size = 0;
		else if (video->mColorResolution == COLOR_DEPTH_10)
			remap_size = 1;
		else if (video->mColorResolution == COLOR_DEPTH_12)
			remap_size = 2;
		else {
			error_set(ERR_COLOR_REMAP_SIZE_INVALID);
			LOGGER(SNPS_ERROR,"invalid color remap size: %d", video->mColorResolution);
			return FALSE;
		}
		output_select = 1;
	}
	else {
		error_set(ERR_OUTPUT_ENCODING_TYPE_INVALID);
		LOGGER(SNPS_ERROR,"invalid output encoding type: %d", video->mEncodingOut);
		return FALSE;
	}

	vp_PixelRepetitionFactor(dev, video->mPixelRepetitionFactor);
	vp_ColorDepth(dev, color_depth);
	vp_PixelPackingDefaultPhase(dev, video->mPixelPackingDefaultPhase);
	vp_Ycc422RemapSize(dev, remap_size);
	vp_OutputSelector(dev, output_select);
	return TRUE;
}

int video_VideoSampler(hdmi_tx_dev_t *dev, videoParams_t * video)
{
	unsigned map_code = 0;

	LOG_TRACE();

	if (video->mEncodingIn == RGB || video->mEncodingIn == YCC444 || video->mEncodingIn == YCC420) {
		if ((video->mColorResolution == COLOR_DEPTH_8) || (video->mColorResolution == 0))
			map_code = 1;
		else if (video->mColorResolution == COLOR_DEPTH_10)
			map_code = 3;
		else if (video->mColorResolution == COLOR_DEPTH_12)
			map_code = 5;
		else if (video->mColorResolution == COLOR_DEPTH_16)
			map_code = 7;
		else {
			error_set(ERR_COLOR_DEPTH_NOT_SUPPORTED);
			LOGGER(SNPS_ERROR,"invalid color depth: %d", video->mColorResolution);
			return FALSE;
		}
		map_code += (video->mEncodingIn != RGB) ? 8 : 0;
	} else if (video->mEncodingIn == YCC422) {
		/* YCC422 mapping is discontinued - only map 1 is supported */
		if (video->mColorResolution == COLOR_DEPTH_12)
			map_code = 18;
		else if (video->mColorResolution == COLOR_DEPTH_10)
			map_code = 20;
		else if ((video->mColorResolution == COLOR_DEPTH_8) || (video->mColorResolution == 0))
			map_code = 22;
		else {
			error_set(ERR_COLOR_REMAP_SIZE_INVALID);
			LOGGER(SNPS_ERROR,"invalid color remap size: %d", video->mColorResolution);
			return FALSE;
		}
	} else {
		error_set(ERR_INPUT_ENCODING_TYPE_INVALID);
		LOGGER(SNPS_ERROR,"invalid input encoding type: %d", video->mEncodingIn);
		return FALSE;
	}

	video_sampler_config(dev, map_code);

	return TRUE;
}

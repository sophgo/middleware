// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "core/audio_params.h"
#include "util/general_ops.h"
#include "util/log.h"

typedef union iec {
	double frequency;
	u8 sample_size;
}iec_t;

typedef struct iec_sampling_freq {
	iec_t iec;
	u8 value;
}iec_params_t;

typedef struct channel_count {
	unsigned char channel_allocation;
	unsigned char channel_count;
}channel_count_t;


iec_params_t iec_original_sampling_freq_values[] = {
		{{.frequency = 44.100}, 0xF},
		{{.frequency = 88.200}, 0x7},
		{{.frequency = 22.050}, 0xB},
		{{.frequency = 176.40}, 0x3},
		{{.frequency = 48.000}, 0xD},
		{{.frequency = 96.000}, 0x5},
		{{.frequency = 24.000}, 0x9},
		{{.frequency = 192.00}, 0x1},
		{{.frequency =  8.000}, 0x6},
		{{.frequency = 11.025}, 0xA},
		{{.frequency = 12.000}, 0x2},
		{{.frequency = 32.000}, 0xC},
		{{.frequency = 16.000}, 0x8},
		{{.frequency = 0},      0x0}
};

iec_params_t iec_sampling_freq_values[] = {
		{{.frequency = 22.050}, 0x4},
		{{.frequency = 44.100}, 0x0},
		{{.frequency = 88.200}, 0x8},
		{{.frequency = 176.40}, 0xC},
		{{.frequency = 24.000}, 0x6},
		{{.frequency = 48.000}, 0x2},
		{{.frequency = 96.000}, 0xA},
		{{.frequency = 192.00}, 0xE},
		{{.frequency = 32.000}, 0x3},
		{{.frequency = 768.00}, 0x9},
		{{.frequency = 0},      0x0}
};

iec_params_t iec_word_length[] = {
		{{.sample_size = 16}, 0x2},
		{{.sample_size = 17}, 0xC},
		{{.sample_size = 18}, 0x4},
		{{.sample_size = 19}, 0x8},
		{{.sample_size = 20}, 0x3},
		{{.sample_size = 21}, 0xD},
		{{.sample_size = 22}, 0x5},
		{{.sample_size = 23}, 0x9},
		{{.sample_size = 24}, 0xB},
		{{.sample_size = 0},  0x0}
};

static channel_count_t channel_cnt[] = {
		{0x00, 1},
		{0x01, 2},
		{0x02, 2},
		{0x04, 2},
		{0x03, 3},
		{0x05, 3},
		{0x06, 3},
		{0x08, 3},
		{0x14, 3},
		{0x07, 4},
		{0x09, 4},
		{0x0A, 4},
		{0x0C, 4},
		{0x15, 4},
		{0x16, 4},
		{0x18, 4},
		{0x0B, 5},
		{0x0D, 5},
		{0x0E, 5},
		{0x10, 5},
		{0x17, 5},
		{0x19, 5},
		{0x1A, 5},
		{0x1C, 5},
		{0x20, 5},
		{0x22, 5},
		{0x24, 5},
		{0x26, 5},
		{0x0F, 6},
		{0x11, 6},
		{0x12, 6},
		{0x1B, 6},
		{0x1D, 6},
		{0x1E, 6},
		{0x21, 6},
		{0x23, 6},
		{0x25, 6},
		{0x27, 6},
		{0x28, 6},
		{0x2A, 6},
		{0x2C, 6},
		{0x2E, 6},
		{0x30, 6},
		{0x13, 7},
		{0x1F, 7},
		{0x29, 7},
		{0x2B, 7},
		{0x2D, 7},
		{0x2F, 7},
		{0x31, 7},
		{0, 0},
};


void audio_reset(hdmi_tx_dev_t *dev, audioParams_t * params)
{
	params->mInterfaceType = I2S;
	params->mCodingType = PCM;
	params->mChannelAllocation = 0;
	params->mSampleSize = 16;
	params->mSamplingFrequency = 32.000;
	params->mLevelShiftValue = 0;
	params->mDownMixInhibitFlag = 0;
	params->mIecCopyright = 1;
	params->mIecCgmsA = 3;
	params->mIecPcmMode = 0;
	params->mIecCategoryCode = 0;
	params->mIecSourceNumber = 1;
	params->mIecClockAccuracy = 0;
	params->mPacketType = AUDIO_SAMPLE;
	params->mClockFsFactor = 128;
	params->mDmaBeatIncrement = DMA_UNSPECIFIED_INCREMENT;
	params->mDmaThreshold = 0;
	params->mDmaHlock = 0;
	params->mGpaInsertPucv = 0;
}

u8 audio_channel_count(hdmi_tx_dev_t *dev, audioParams_t * params)
{
	int i = 0;

	for(i = 0; channel_cnt[i].channel_count != 0; i++){
		if(channel_cnt[i].channel_allocation == params->mChannelAllocation){
			return channel_cnt[i].channel_count;
		}
	}

	return 0;
}

u8 audio_iec_original_sampling_freq(hdmi_tx_dev_t *dev, audioParams_t * params)
{
	int i = 0;

	for(i = 0; iec_original_sampling_freq_values[i].iec.frequency != 0; i++){
		if(double_is_equal(params->mSamplingFrequency, iec_original_sampling_freq_values[i].iec.frequency)){
			u8 value = iec_original_sampling_freq_values[i].value;
			return value;
		}
	}

	// Not indicated
	return 0x0;
}

u8 audio_iec_sampling_freq(hdmi_tx_dev_t *dev, audioParams_t * params)
{
	int i = 0;

	for(i = 0; iec_sampling_freq_values[i].iec.frequency != 0; i++){
		if(double_is_equal(params->mSamplingFrequency, iec_sampling_freq_values[i].iec.frequency)){
			u8 value = iec_sampling_freq_values[i].value;
			return value;
		}
	}

	// Not indicated
	return 0x1;
}

u8 audio_iec_word_length(hdmi_tx_dev_t *dev, audioParams_t * params)
{
	int i = 0;

	for(i = 0; iec_word_length[i].iec.sample_size != 0; i++){
		if(params->mSampleSize == iec_word_length[i].iec.sample_size){
			return iec_word_length[i].value;
		}
	}

	// Not indicated
	return 0x0;
}

u8 audio_is_channel_en(hdmi_tx_dev_t *dev, audioParams_t * params, u8 channel)
{
	switch (channel) {
	case 0:
	case 1:
		return 1;
	case 2:
		return params->mChannelAllocation & BIT(0);
	case 3:
		return (params->mChannelAllocation & BIT(1)) >> 1;
	case 4:
		if (((params->mChannelAllocation > 0x03) && (params->mChannelAllocation < 0x14)) || ((params->mChannelAllocation > 0x17) && (params->mChannelAllocation < 0x20)))
			return 1;
		else
			return 0;
	case 5:
		if (((params->mChannelAllocation > 0x07) && (params->mChannelAllocation < 0x14)) || ((params->mChannelAllocation > 0x1C) && (params->mChannelAllocation < 0x20)))
			return 1;
		else
			return 0;
	case 6:
		if ((params->mChannelAllocation > 0x0B) && (params->mChannelAllocation < 0x20))
			return 1;
		else
			return 0;
	case 7:
		return (params->mChannelAllocation & BIT(4)) >> 4;
	default:
		return 0;
	}
}

char * audio_interface_type_string(audioParams_t *pAudio)
{
	switch(pAudio->mInterfaceType){
	case I2S:
			return "I2S";
	case SPDIF:
		return "SPDIF";
	case HBR:
			return "HBR";
	case GPA:
			return "GPA";
	case DMA:
			return "DMA";
	default:
		break;
	}
	return "undefined";
}

char * audio_coding_type_string(audioParams_t *pAudio)
{
	switch(pAudio->mCodingType){
	case PCM:
		return "PCM";
	case AC3:
		return "AC3";
	case MPEG1:
		return "MPEG1";
	case MP3:
		return "MP3";
	case MPEG2:
		return "MPEG2";
	case AAC:
		return "AAC";
	case ATRAC:
		return "ATRAC";
	case ONE_BIT_AUDIO:
		return "ONE_BIT_AUDIO";
	case DOLBY_DIGITAL_PLUS:
		return "DOLBY_DIGITAL_PLUS";
	case DTS_HD:
		return "DTS_HD";
	case MAT:
		return "MAT";
	case DST:
		return "DST";
	case WMAPRO:
		return "WMAPRO";
	default:
		break;
	}
	return "undefined";
}

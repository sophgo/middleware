/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

/**
 * @file
 * Define audio parameters structure and functions to manipulate the
 * information held by the structure.
 * Audio interfaces, packet types and coding types are also defined here
 */
#ifndef AUDIOPARAMS_H_
#define AUDIOPARAMS_H_

#include "../hdmitx_dev.h"
#include "util/types.h"

typedef enum {
	INTERFACE_NOT_DEFINED = -1, I2S = 0, SPDIF, HBR, GPA, DMA
} interfaceType_t;

typedef enum {
	PACKET_NOT_DEFINED = -1, AUDIO_SAMPLE = 1, HBR_STREAM
} packet_t;

typedef enum {
	CODING_NOT_DEFINED = -1,
	PCM = 1,
	AC3,
	MPEG1,
	MP3,
	MPEG2,
	AAC,
	DTS,
	ATRAC,
	ONE_BIT_AUDIO,
	DOLBY_DIGITAL_PLUS,
	DTS_HD,
	MAT,
	DST,
	WMAPRO
} codingType_t;

typedef enum {
	DMA_NOT_DEFINED = -1,
	DMA_4_BEAT_INCREMENT = 0,
	DMA_8_BEAT_INCREMENT,
	DMA_16_BEAT_INCREMENT,
	DMA_UNUSED_BEAT_INCREMENT,
	DMA_UNSPECIFIED_INCREMENT
} dmaIncrement_t;

/* Supplementary Audio type, table 8-14 HDMI 2.0 Spec. pg 79 */
typedef enum {
	RESERVED = 0,
	AUDIO_FOR_VIS_IMP_NARR,
	AUDIO_FOR_VIS_IMP_SPOKEN,
	AUDIO_FOR_HEAR_IMPAIRED,
	ADDITIONAL_AUDIO
} suppl_A_Type_t;

/* Audio Metadata Packet Header, table 8-4 HDMI 2.0 Spec. pg 71 */
typedef struct {
	u8 m3dAudio;
	u8 mNumViews;
	u8 mNumAudioStreams;
} audioMetaDataHeader_t;

/* Audio Metadata Descriptor, table 8-13 HDMI 2.0 Spec. pg 78 */
typedef struct {
	u8 mMultiviewRightLeft;
	u8 mLC_Valid;
	u8 mSuppl_A_Valid;
	u8 mSuppl_A_Mixed;
	suppl_A_Type_t mSuppl_A_Type;
	u8 mLanguage_Code[3];	/*ISO 639.2 alpha-3 code, examples: eng,fre,spa,por,jpn,chi */

} audioMetaDataDescriptor_t;

typedef struct {
	audioMetaDataHeader_t mAudioMetaDataHeader;
	audioMetaDataDescriptor_t mAudioMetaDataDescriptor[4];
} audioMetaDataPacket_t;

/**
 * For detailed handling of this structure,
 * refer to documentation of the functions
 */
typedef struct {
	interfaceType_t mInterfaceType;

	codingType_t mCodingType; /** (audioParams_t *params, see InfoFrame) */

	int mChannelAllocation; /** channel allocation (audioParams_t *params, 
						   see InfoFrame) */

	u8 mSampleSize;	/**  sample size (audioParams_t *params, 16 to 24) */

	double mSamplingFrequency;	/** sampling frequency (audioParams_t *params, kHz) */

	u8 mLevelShiftValue; /** level shift value (audioParams_t *params, 
						 see InfoFrame) */

	u8 mDownMixInhibitFlag;	/** down-mix inhibit flag (audioParams_t *params, 
							see InfoFrame) */

	u8 mIecCopyright; /** IEC copyright */

	u8 mIecCgmsA; /** IEC CGMS-A */

	u8 mIecPcmMode;	/** IEC PCM mode */

	u8 mIecCategoryCode; /** IEC category code */

	u8 mIecSourceNumber; /** IEC source number */

	u8 mIecClockAccuracy; /** IEC clock accuracy */

	packet_t mPacketType; /** packet type. currently only Audio Sample (AUDS) 
						  and High Bit Rate (HBR) are supported */

	u16 mClockFsFactor; /** Input audio clock Fs factor used at the audio
						packetizer to calculate the CTS value and ACR packet
						insertion rate */

	dmaIncrement_t mDmaBeatIncrement; /** Incremental burst modes: unspecified
									lengths (upper limit is 1kB boundary) and
									INCR4, INCR8, and INCR16 fixed-beat burst */

	u8 mDmaThreshold; /** When the number of samples in the Audio FIFO is lower
						than the threshold, the DMA engine requests a new burst
						request to the AHB master interface */

	u8 mDmaHlock; /** Master burst lock mechanism */

	u8 mGpaInsertPucv;	/* discard incoming (Parity, Channel status, User bit,
				   Valid and B bit) data and insert data configured in 
				   controller instead */
	audioMetaDataPacket_t mAudioMetaDataPacket; /** Audio Multistream variables, to be written to the Audio Metadata Packet */
} audioParams_t;

/**
 * This method reset the parameters structure to a known state
 * SPDIF 16bits 32Khz Linear PCM
 * It is recommended to call this method before setting any parameters
 * to start from a stable and known condition avoid overflows.
 * @param params pointer to the audio parameters structure
 */
void audio_reset(hdmi_tx_dev_t *dev, audioParams_t * params);

/**
 * @param params pointer to the audio parameters structure
 * @return number of audio channels transmitted -1
 */
u8 audio_channel_count(hdmi_tx_dev_t *dev, audioParams_t * params);

/**
 * @param params pointer to the audio parameters structure
 */
u8 audio_iec_original_sampling_freq(hdmi_tx_dev_t *dev, audioParams_t * params);

/**
 * @param params pointer to the audio parameters structure
 */
u8 audio_iec_sampling_freq(hdmi_tx_dev_t *dev, audioParams_t * params);

/**
 * @param params pointer to the audio parameters structure
 */
u8 audio_iec_word_length(hdmi_tx_dev_t *dev, audioParams_t * params);

/**
 * return if channel is enabled or not using the user's channel allocation
 * code
 * @param params pointer to the audio parameters structure
 * @param channel in question -1
 * @return 1 if channel is to be enabled, 0 otherwise
 */
u8 audio_is_channel_en(hdmi_tx_dev_t *dev, audioParams_t * params, u8 channel);

/**
 * Get the string that correspond to the interface type.
 * @param pAudio pointer to the audio parameters structure
 * @return interface type string
 */
char * audio_interface_type_string(audioParams_t *pAudio);

/**
 * Get the string that correspond to the coding type.
 * @param pAudio pointer to the audio parameters structure
 * @return coding type string
 */
char * audio_coding_type_string(audioParams_t *pAudio);

#endif	/* AUDIOPARAMS_H_ */

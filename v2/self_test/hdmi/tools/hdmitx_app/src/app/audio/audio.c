// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "audio.h"
#include "platform.h"
#include "general_ops.h"
#include "app/cmd_line/cmd_interface.h"
#include "video_bridge.h"
#include "audio_bridge.h"
#include "app/hdmitx.h"

void update_audio_cfg(audioParams_t * user, audioParams_t * cfg)
{
	if(user->mInterfaceType != INTERFACE_NOT_DEFINED)
		cfg->mInterfaceType = user->mInterfaceType;

	if(user->mCodingType != CODING_NOT_DEFINED)
		cfg->mCodingType = user->mCodingType;

	if(user->mChannelAllocation != 0)
		cfg->mChannelAllocation = user->mChannelAllocation;

	if(user->mSampleSize != 0)
		cfg->mSampleSize = user->mSampleSize;

	if(!double_is_equal(user->mSamplingFrequency, 0.0))
		cfg->mSamplingFrequency = user->mSamplingFrequency;

	if(user->mLevelShiftValue != 0)
		cfg->mLevelShiftValue = user->mLevelShiftValue;

	if(user->mDownMixInhibitFlag != 0)
		cfg->mDownMixInhibitFlag = user->mDownMixInhibitFlag;

	if(user->mIecCopyright != 0)
		cfg->mIecCopyright = user->mIecCopyright;

	if(user->mIecCgmsA != 0)
		cfg->mIecCgmsA = user->mIecCgmsA;

	if(user->mIecPcmMode != 0)
		cfg->mIecPcmMode = user->mIecPcmMode;

	if(user->mIecCategoryCode != 0)
		cfg->mIecCategoryCode = user->mIecCategoryCode;

	if(user->mIecSourceNumber != 0)
		cfg->mIecSourceNumber = user->mIecSourceNumber;

	if(user->mIecClockAccuracy != 0)
		cfg->mIecClockAccuracy = user->mIecClockAccuracy;

	if(user->mPacketType != PACKET_NOT_DEFINED)
		cfg->mPacketType = user->mPacketType;

	if(user->mClockFsFactor != 0)
		cfg->mClockFsFactor = user->mClockFsFactor;

	if(user->mDmaBeatIncrement != DMA_NOT_DEFINED)
		cfg->mDmaBeatIncrement = user->mDmaBeatIncrement;

	if(user->mDmaThreshold != 0)
		cfg->mDmaThreshold = user->mDmaThreshold;

	if(user->mDmaHlock != 0)
		cfg->mDmaHlock = user->mDmaHlock;

	if(user->mGpaInsertPucv != 0)
		cfg->mGpaInsertPucv = user->mGpaInsertPucv;

	if(user->mAudioMetaDataPacket.mAudioMetaDataHeader.m3dAudio != 0)
		cfg->mAudioMetaDataPacket.mAudioMetaDataHeader.m3dAudio = user->mAudioMetaDataPacket.mAudioMetaDataHeader.m3dAudio;

	if(user->mAudioMetaDataPacket.mAudioMetaDataHeader.mNumAudioStreams != 0)
		cfg->mAudioMetaDataPacket.mAudioMetaDataHeader.mNumAudioStreams = user->mAudioMetaDataPacket.mAudioMetaDataHeader.mNumAudioStreams;

	if(user->mAudioMetaDataPacket.mAudioMetaDataHeader.mNumViews != 0)
		cfg->mAudioMetaDataPacket.mAudioMetaDataHeader.mNumViews = user->mAudioMetaDataPacket.mAudioMetaDataHeader.mNumViews;

	if(user->mAudioMetaDataPacket.mAudioMetaDataDescriptor->mLC_Valid != 0)
		cfg->mAudioMetaDataPacket.mAudioMetaDataDescriptor->mLC_Valid = user->mAudioMetaDataPacket.mAudioMetaDataDescriptor->mLC_Valid;

	if(user->mAudioMetaDataPacket.mAudioMetaDataDescriptor->mMultiviewRightLeft != 0)
		cfg->mAudioMetaDataPacket.mAudioMetaDataDescriptor->mMultiviewRightLeft = user->mAudioMetaDataPacket.mAudioMetaDataDescriptor->mMultiviewRightLeft;

	if(user->mAudioMetaDataPacket.mAudioMetaDataDescriptor->mSuppl_A_Mixed != 0)
		cfg->mAudioMetaDataPacket.mAudioMetaDataDescriptor->mSuppl_A_Mixed = user->mAudioMetaDataPacket.mAudioMetaDataDescriptor->mSuppl_A_Mixed;

	if(user->mAudioMetaDataPacket.mAudioMetaDataDescriptor->mSuppl_A_Valid != 0)
		cfg->mAudioMetaDataPacket.mAudioMetaDataDescriptor->mSuppl_A_Valid = user->mAudioMetaDataPacket.mAudioMetaDataDescriptor->mSuppl_A_Valid;

	if(user->mAudioMetaDataPacket.mAudioMetaDataDescriptor->mSuppl_A_Type != RESERVED)
		cfg->mAudioMetaDataPacket.mAudioMetaDataDescriptor->mSuppl_A_Type = user->mAudioMetaDataPacket.mAudioMetaDataDescriptor->mSuppl_A_Type;
}

void print_audioinfo(audioParams_t *pAudio)
{
	printf("Coding %s @ ", pAudio->mCodingType == PCM ? "PCM" :
			               pAudio->mCodingType == AC3 ? "AC3" :
			               pAudio->mCodingType == MPEG1 ? "MPEG1" :
			               pAudio->mCodingType == MP3 ? "MP3" :
			               pAudio->mCodingType == MPEG2 ? "MPEG2" :
			               pAudio->mCodingType == AAC ? "AAC" :
			               pAudio->mCodingType == DTS ? "DTS" :
			               pAudio->mCodingType == ATRAC ? "ATRAC" :
			               pAudio->mCodingType == ONE_BIT_AUDIO ? "ONE BIT AUDIO" :
			               pAudio->mCodingType == DOLBY_DIGITAL_PLUS ? "DOLBY DIGITAL +" :
			               pAudio->mCodingType == DTS_HD ? "DTS HD" : "----");
	printf("%.3fkHz | ", pAudio->mSamplingFrequency);
	printf("Sample size = %d | ", pAudio->mSampleSize);
	printf("FS factor = %d\n", pAudio->mClockFsFactor);
}


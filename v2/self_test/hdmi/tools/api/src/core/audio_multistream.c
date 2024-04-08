// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "frame_composer/frame_composer_reg.h"
#include "core/audio_params.h"
#include "core/audio_multistream.h"
#include "bsp/access.h"

#include "util/log.h"

void halAudioMultistream_MetaDataPacket_Descriptor_X(hdmi_tx_dev_t *dev, u8 descrNr, audioMetaDataDescriptor_t *mAudioMetaDataDescriptor)
{
	u8 pb = 0x00;
	dev_write(dev, FC_AMP_PB, (u8)(mAudioMetaDataDescriptor->mMultiviewRightLeft & 0x03));
	pb = ((mAudioMetaDataDescriptor->mLC_Valid & 0x01) << 7);
	pb = pb | ((mAudioMetaDataDescriptor->mSuppl_A_Valid & 0x01) << 4);
	pb = pb | ((mAudioMetaDataDescriptor->mSuppl_A_Mixed & 0x01) << 3);
	pb = pb | ((mAudioMetaDataDescriptor->mSuppl_A_Type & 0x03) << 0);
	LOGGER(SNPS_INFO," pb = 0x%x", pb);
	dev_write(dev, (FC_AMP_PB + (((0x05 * descrNr) + 0x01)*4)), (u8)(pb));
	if (mAudioMetaDataDescriptor->mLC_Valid == 1) {
		dev_write(dev, FC_AMP_PB + (((0x05 * descrNr) + 0x02)*4), ((u8)(mAudioMetaDataDescriptor->mLanguage_Code[0])) & 0xff);
		dev_write(dev, FC_AMP_PB + (((0x05 * descrNr) + 0x03)*4), ((u8)(mAudioMetaDataDescriptor->mLanguage_Code[1])) & 0xff);
		dev_write(dev, FC_AMP_PB + (((0x05 * descrNr) + 0x04)*4), ((u8)(mAudioMetaDataDescriptor->mLanguage_Code[2])) & 0xff);
	}
	LOGGER(SNPS_INFO," body AMP descriptor 0 0x%x", FC_AMP_PB + (((0x05 * descrNr) + 0x00)*4));
	LOGGER(SNPS_INFO," body AMP descriptor 0 0x%x", FC_AMP_PB + (((0x05 * descrNr) + 0x01)*4));
	LOGGER(SNPS_INFO," body AMP descriptor 0 0x%x", FC_AMP_PB + (((0x05 * descrNr) + 0x02)*4));
	LOGGER(SNPS_INFO," body AMP descriptor 0 0x%x", FC_AMP_PB + (((0x05 * descrNr) + 0x03)*4));
	LOGGER(SNPS_INFO," body AMP descriptor 0 0x%x", FC_AMP_PB + (((0x05 * descrNr) + 0x04)*4));
}

void halAudioMultistream_MetaDataPacket_Header(hdmi_tx_dev_t *dev, audioMetaDataPacket_t *mAudioMetaDataPckt)
{
	dev_write(dev, FC_AMP_HB1, (u8)(mAudioMetaDataPckt->mAudioMetaDataHeader.m3dAudio));
	dev_write(dev, FC_AMP_HB2, (u8)(((mAudioMetaDataPckt->mAudioMetaDataHeader.mNumViews) | (mAudioMetaDataPckt->mAudioMetaDataHeader.mNumAudioStreams) << 2)));
	LOGGER(SNPS_INFO," AMP HB1 0x%x", FC_AMP_HB1);
	LOGGER(SNPS_INFO," AMP HB1 data 0x%x", (u8)(mAudioMetaDataPckt->mAudioMetaDataHeader.m3dAudio));
	LOGGER(SNPS_INFO," AMP HB2 0x%x", FC_AMP_HB2);
	LOGGER(SNPS_INFO," AMP HB2 data 0x%x", (u8)(((mAudioMetaDataPckt->mAudioMetaDataHeader.mNumViews) |  (mAudioMetaDataPckt->mAudioMetaDataHeader.mNumAudioStreams) << 2)));
}

void halAudioMultistream_MetaDataPacketBody(hdmi_tx_dev_t *dev, audioMetaDataPacket_t *mAudioMetaDataPacket)
{
	u8 cnt = 0;
	while (cnt <= (mAudioMetaDataPacket->mAudioMetaDataHeader.mNumAudioStreams + 1)){
		LOGGER(SNPS_INFO,"(mAudioMetaDataPacket->mAudioMetaDataHeader.mNumAudioStreams + 1) = %d", (mAudioMetaDataPacket->mAudioMetaDataHeader.mNumAudioStreams + 1));
		LOGGER(SNPS_INFO," audiometadata packet descriptor %d", cnt);
		halAudioMultistream_MetaDataPacket_Descriptor_X(dev, 0, &(mAudioMetaDataPacket->mAudioMetaDataDescriptor[cnt]));
		cnt++;
	}
}

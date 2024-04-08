// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "audio_bridge.h"
#include "hdmi_tx_system_parameters.h"
#include "includes.h"


//#define AG_SWRSTZ 				0x00
//#define AG_SWRSTZ_SHIFT 			0

#define AG_MODE 				0x04
#define I2S_WORDWIDTH_SHIFT                    	6
#define AUDIOSOURCE_CLOCKMULTIPLIERCODED_SHIFT 	3
#define I2S_MODE_SHIFT                         	0

#define AG_INCLEFT0 				0x08
#define INCLEFT0_SHIFT 				0

#define AG_INCLEFT1 				0x0c
#define INCLEFT1_SHIFT 				0

#define AG_INCRIGHT0 				0x10
#define INCRIGHT0_SHIFT 			0

#define AG_INCRIGHT1 				0x14
#define INCRIGHT1_SHIFT 			0

#define AG_AUDSOURCE 				0x18
#define SPDIF_INTERNAL_EXTERNAL_SHIFT 		1
#define I2S_INTERNAL_EXTERNAL_SHIFT   		0

#define AG_WIDTH 				0x1c
#define OINSERTI2SPCUV_SHIFT 					7
#define OHBRONI2S_SHIFT                     	6
#define ONLPCMONI2S_SHIFT                   	5
#define AUDIOWIDTH_EXTENDEDMODEENABLE_SHIFT 	4
#define AUD_WIDTH_SHIFT                     	0

#define AG_SPDIF_AUDSCHNLS0 			0x40
#define IEC_NLPCM_SHIFT     			3
#define IEC_CGMSA_SHIFT     			1
#define IEC_COPYRIGHT_SHIFT 			0

#define AG_SPDIF_AUDSCHNLS1 			0x44
#define IEC_CATEGORYCODE_SHIFT 			0

#define AG_SPDIF_AUDSCHNLS2 			0x48
#define IEC_PCM_AUDIO_MODE_SHIFT 		4
#define IEC_SOURCENUMBER_SHIFT  		0

#define AG_SPDIF_AUDSCHNLS3 			0x4c
#define IEC_CHANNELNUMCR0_SHIFT 		4
#define IEC_CHANNELNUMCL0_SHIFT 		0

#define AG_SPDIF_AUDSCHNLS4 			0x50
#define IEC_SAMPFREQ5_SHIFT   			7
#define IEC_SAMPFREQ4_SHIFT   			6
#define IEC_CLKACCURACY_SHIFT 			4
#define IEC_SAMPFREQ3_SHIFT   			3
#define IEC_SAMPFREQ2_SHIFT   			2
#define IEC_SAMPFREQ1_SHIFT   			1
#define IEC_SAMPFREQ0_SHIFT   			0

#define AG_SPDIF_AUDSCHNLS5 			0x54
#define IEC_ORIGSAMPFREQ_SHIFT 			4
#define IEC_WORDLENGTH_SHIFT   			0

#define AG_SPDIF_CONF 				0x58
#define ONOFSOVERSAMP_SHIFT 			4
#define USERDATA_CR0_SHIFT  			3
#define USERDATA_CL0_SHIFT  			2
#define SPDIFTXDATA_SHIFT   			1
#define SPDIFTXEN_SHIFT     			0

#define AG_GPA_CONF0 				0x60
#define GPA_HOLD_OUTPUT_SHIFT    		6
#define OAUDIO_USE_LUT_SHIFT     		5
#define OAUDIO_USE_COUNTER_SHIFT 		4
#define OGPAREPLY_LATENCY_SHIFT  		0

#define AG_GPA_CONF1 				0x64
#define OGPACHANNEL_SELECT_SHIFT 		0

#define AG_GPA_CONF2 				0x68
#define OAUDIO_COUNTER_OFFSET_SHIFT 		0

#define AG_GPA_SAMPLEVALID 			0x6c
#define OSAMPLE_VALID_SHIFT			0

#define AG_GPA_CHNUM1 				0x70
#define OIEC_CHANNELNUMCR1_SHIFT 		4
#define OIEC_CHANNELNUMCL1_SHIFT 		0

#define AG_GPA_CHNUM2 				0x74
#define OIEC_CHANNELNUMCR2_SHIFT		4
#define OIEC_CHANNELNUMCL2_SHIFT 		0

#define AG_GPA_CHNUM3 				0x78
#define OIEC_CHANNELNUMCR3_SHIFT 		4
#define OIEC_CHANNELNUMCL3_SHIFT 		0

#define AG_GPA_USERBIT 				0x7c
#define OUSERDATA_CR3_SHIFT 			5
#define OUSERDATA_CL3_SHIFT 			4
#define OUSERDATA_CR2_SHIFT 			3
#define OUSERDATA_CL2_SHIFT 			2
#define OUSERDATA_CR1_SHIFT 			1
#define OUSERDATA_CL1_SHIFT 			0

#define AG_GPA_SAMPLE_LSB 			0x80
#define IGPASAMPLERCTN_GPA0_SHIFT 		4
#define IGPASAMPLERCTN_FS0_SHIFT  		0

#define AG_GPA_SAMPLE_MSB 			0x84
#define IGPASAMPLERCTN_GPA1_SHIFT 		4
#define IGPASAMPLERCTN_FS1_SHIFT  		0

#define AG_GPA_SAMPLE_DIFF 			0x88
#define IGPASAMPLERCTN_DIFF_SHIFT 		0

#define AG_GPA_INT 				0x8c
#define IGPAFIFO_EMPTY_STICKY_SHIFT 		5
#define IGPAFIFO_FULL_STICKY_SHIFT  		4
#define IGPAFIFO_EMPTY_SHIFT        		1
#define IGPAFIFO_FULL_SHIFT         		0


/**
 * Prototypes - Internal functions
 */
void _ag_sw_reset(struct hdmi_tx_app *app, bool bit);
void _ag_i2s_mode(struct hdmi_tx_app *app, u8 data);
void _ag_i2s_multiplier(struct hdmi_tx_app *app, u32 value);
void _ag_freq_increment_left(struct hdmi_tx_app *app, u16 data);
void _ag_freq_increment_right(struct hdmi_tx_app *app, u16 data);
void _ag_iec_cgms_a(struct hdmi_tx_app *app, u8 data);
void _ag_iec_copyright(struct hdmi_tx_app *app, bool bit);
void _ag_iec_category_code(struct hdmi_tx_app *app, u8 data);
void _ag_iec_pcm_mode(struct hdmi_tx_app *app, u8 data);
void _ag_iec_source(struct hdmi_tx_app *app, u8 data);
void _ag_iec_channel_right(struct hdmi_tx_app *app, u8 data, u8 channelNo);
void _ag_iec_channel_left(struct hdmi_tx_app *app, u8 data, u8 channelNo);
void _ag_iec_clock_accuracy(struct hdmi_tx_app *app, u8 data);
void _ag_iec_sampling_freq(struct hdmi_tx_app *app, u8 data);
void _ag_iec_original_sampling_freq(struct hdmi_tx_app *app, u8 data);
void _ag_iec_word_length(struct hdmi_tx_app *app, u8 data);
void _ag_user_right(struct hdmi_tx_app *app, u8 bit, u8 channelNo);
void _ag_user_left(struct hdmi_tx_app *app, u8 bit, u8 channelNo);
void _ag_spdif_tx_data(struct hdmi_tx_app *app, bool bit);
void _ag_hbr_enable(struct hdmi_tx_app *app, bool bit);
void _ag_hbr_ddr_enable(struct hdmi_tx_app *app, bool bit);
void _ag_hbr_ddr_channel(struct hdmi_tx_app *app, bool bit);
void _ag_hbr_burst_length(struct hdmi_tx_app *app, bool value);
void _ag_hbr_clock_divider(struct hdmi_tx_app *app, u16 value);
void _ag_use_look_up_table(struct hdmi_tx_app *app, bool bit);
void _ag_gpa_reply_latency(struct hdmi_tx_app *app, u8 data);
void _ag_channel_select(struct hdmi_tx_app *app, bool enable, u8 channel);
void _ag_gpa_sample_valid(struct hdmi_tx_app *app, bool enable, u8 channelNo);


void audio_bridge_write(struct hdmi_tx_app *app, uint32_t reg, uint32_t data){
	int ret = 0;
	fb_ioctl_data write_data;
	write_data.address = reg + PROTO_HDMI_TX_APB_AUDIOBRIDGE_ADDRESS_START;
	write_data.value = data;

	if(app->hdmi_tx_driver < 0)
		return;

	LOGGER(SNPS_TRACE, "%s:addr 0x%x - value 0x%x", __func__,
			write_data.address, write_data.value);

	ret = ioctl(app->hdmi_tx_driver, FB_HDMI_CORE_WRITE, &write_data);
	if(ret < 0){
		if(app->verbose)
			LOGGER(SNPS_ERROR, "Audio bridge write IOCTL error [%d]\n", ret);
	}
}

uint32_t audio_bridge_read(struct hdmi_tx_app *app, uint32_t reg){
	int ret = 0;
	fb_ioctl_data read_data;
	read_data.address = reg + PROTO_HDMI_TX_APB_AUDIOBRIDGE_ADDRESS_START;

	if(app->hdmi_tx_driver < 0)
		return 0;

	ret = ioctl(app->hdmi_tx_driver, FB_HDMI_CORE_READ, &read_data);
	if(ret < 0){
		if(app->verbose)
			LOGGER(SNPS_ERROR, "Audio bridge read IOCTL error [%d]\n", ret);
	}

	LOGGER(SNPS_TRACE, "%s:addr 0x%x - value 0x%x", __func__,
			read_data.address, read_data.value);

	return read_data.value;
}

void audio_generator_config(struct hdmi_tx_app *app){
	uint32_t tmp;
	uint8_t data = 0;
	audioParams_t *audio = &app->mode.pAudio;

	uint32_t sampling_freq = 0;
	sampling_freq = (uint32_t)(audio->mSamplingFrequency * 1000);

	if(sampling_freq == 0){
		LOGGER(SNPS_ERROR, "%s: Audio Sampling Frequency not defined", __func__);
		return;
	}
	
	LOG_TRACE();

	if(audio->mInterfaceType == I2S){
		_ag_i2s_mode(app, 1);
		_ag_i2s_multiplier(app, audio->mClockFsFactor);
		
		//I2S workaround
		tmp = audio_bridge_read(app, AG_WIDTH);		
		tmp |=  (0x1 << OINSERTI2SPCUV_SHIFT);
		audio_bridge_write(app, AG_WIDTH, tmp);
	}
	else if(audio->mInterfaceType == SPDIF){
		_ag_i2s_mode(app, 0);
		audio->mClockFsFactor = 512; //NOTE: AG SPDIF only supports 512 FS
		_ag_i2s_multiplier(app, audio->mClockFsFactor);

		_ag_iec_cgms_a(app, audio->mIecCgmsA);
		_ag_iec_copyright(app, audio->mIecCopyright ? 0 : 1);
		_ag_iec_category_code(app, audio->mIecCategoryCode);
		_ag_iec_pcm_mode(app, audio->mIecPcmMode);
		_ag_iec_source(app, audio->mIecSourceNumber);

		_ag_iec_clock_accuracy(app, audio->mIecClockAccuracy);

		LOGGER(SNPS_WARN, "mIecCgmsA %d; mIecCopyright %d; mIecCategoryCode %d; mIecPcmMode %d; mIecSourceNumber %d; mIecClockAccuracy %d",
				audio->mIecCgmsA, audio->mIecCopyright, audio->mIecCategoryCode, audio->mIecPcmMode, audio->mIecSourceNumber, audio->mIecClockAccuracy);

		data = audio_iec_sampling_freq(&app->hdmi_tx, audio);
		_ag_iec_sampling_freq(app, data);
		LOGGER(SNPS_WARN, "iec_sample_freq %x", data);

		data = audio_iec_original_sampling_freq(&app->hdmi_tx, audio);
		_ag_iec_original_sampling_freq(app, data);
		LOGGER(SNPS_WARN, "iec_original_freq %x", data);

		data = audio_iec_word_length(&app->hdmi_tx, audio);
		_ag_iec_word_length(app, data);
		LOGGER(SNPS_WARN, "iec_word_lenght %x", data);

		_ag_spdif_tx_data(app, true);

		// Configurations following test case log
//		audio_bridge_write(app, AG_GPA_INT, 0xff);
//
//		audio_bridge_write(app, AG_SPDIF_AUDSCHNLS0, 0xf);
//
//		audio_bridge_write(app, AG_SPDIF_AUDSCHNLS1, 0x55);
//
//		audio_bridge_write(app, AG_SPDIF_AUDSCHNLS2, 0x8);
//
//		audio_bridge_write(app, AG_SPDIF_AUDSCHNLS3, 0x12);
//
//		audio_bridge_write(app, AG_SPDIF_AUDSCHNLS4, 0x2e);
//
//		audio_bridge_write(app, AG_SPDIF_AUDSCHNLS5, 0xab);
//
//		audio_bridge_write(app, AG_SPDIF_CONF, 0xf);
//
//		audio_bridge_write(app, AG_GPA_SAMPLEVALID, 0x0);
//
//		audio_bridge_write(app, AG_GPA_CHNUM1, 0x21);
//
//		audio_bridge_write(app, AG_GPA_CHNUM2, 0x10);
//
//		audio_bridge_write(app, AG_GPA_CHNUM3, 0x11);
//
//		audio_bridge_write(app, AG_GPA_USERBIT, 0x1d);
//
//		audio_bridge_write(app, AG_GPA_USERBIT, 0x1d);
//
//		audio_bridge_write(app, AG_MODE, 0x50);
//
//		audio_bridge_write(app, AG_WIDTH, 0x95);
//
//		audio_bridge_write(app, AG_INCLEFT0, 0x40);
//
//		audio_bridge_write(app, AG_INCLEFT1, 0x0);
//
//		audio_bridge_write(app, AG_INCRIGHT0, 0x80);
//
//		audio_bridge_write(app, AG_INCRIGHT1, 0x0);
//
//		audio_bridge_write(app, AG_GPA_CONF0, 0x13);
//
//		audio_bridge_write(app, AG_GPA_CONF1, 0xff);
//
//		audio_bridge_write(app, AG_GPA_CONF0, 0x13);
	}
	else {
		LOGGER(SNPS_ERROR, "%s:Audio Interface Type not supported [%d]", __func__, audio->mInterfaceType);
		return;
	}


	tmp = (500 * 65535) / sampling_freq;	/* 500Hz */

	_ag_freq_increment_left(app, (u16) (tmp & 0xffff));
	_ag_freq_increment_right(app, (u16) (tmp & 0xffff));
	for (tmp = 0; tmp < 4; tmp++) {

		// 0 -> iec spec 60958-3 means "do not take into account"
		_ag_iec_channel_left(app, 0, tmp);
		_ag_iec_channel_right(app, 0, tmp);


		// USER_BIT 0 default by spec
		_ag_user_left(app, 0, tmp);
		_ag_user_right(app, 0, tmp);
	}

	_ag_sw_reset(app, 1);
	_ag_sw_reset(app, 0);
}

void _ag_sw_reset(struct hdmi_tx_app *app, bool bit){
	LOG_TRACE1(bit);

	audio_bridge_write(app, AG_SWRSTZ, bit ? 0 : 1);
}

void _ag_i2s_mode(struct hdmi_tx_app *app, u8 data){
	uint32_t value = 0;
	uint32_t mask = BIT(3) - 1;

	LOG_TRACE1(data);

	value = audio_bridge_read(app, AG_MODE);

	value &= ~(mask << I2S_MODE_SHIFT);
	value |= (data & mask) << I2S_MODE_SHIFT;

	audio_bridge_write(app, AG_MODE, value);
}

void _ag_i2s_multiplier(struct hdmi_tx_app *app, u32 data){
	uint32_t value = 0;
	uint32_t mask = BIT(3) - 1;

	LOG_TRACE1(data);

	value = audio_bridge_read(app, AG_MODE);

	value &= ~(mask << AUDIOSOURCE_CLOCKMULTIPLIERCODED_SHIFT);

	switch(data){
	case 64:
		value |= (0x4 & mask) << AUDIOSOURCE_CLOCKMULTIPLIERCODED_SHIFT;
		break;
	case 128:
		value |= (0x0 & mask) << AUDIOSOURCE_CLOCKMULTIPLIERCODED_SHIFT;
		break;
	case 256:
		value |= (0x1 & mask) << AUDIOSOURCE_CLOCKMULTIPLIERCODED_SHIFT;
		break;
	case 512:
		value |= (0x2 & mask) << AUDIOSOURCE_CLOCKMULTIPLIERCODED_SHIFT;
		break;
	default:
		value |= (0x0 & mask) << AUDIOSOURCE_CLOCKMULTIPLIERCODED_SHIFT;
		break;
	}

	audio_bridge_write(app, AG_MODE, value);
}

void _ag_freq_increment_left(struct hdmi_tx_app *app, u16 data){
	LOG_TRACE1(data);
	audio_bridge_write(app, AG_INCLEFT0, (u8)(data));
	audio_bridge_write(app, AG_INCLEFT1, (u8)(data >> 8));
}

void _ag_freq_increment_right(struct hdmi_tx_app *app, u16 data){
	LOG_TRACE1(data);
	audio_bridge_write(app, AG_INCRIGHT0, (u8)(data));
	audio_bridge_write(app, AG_INCRIGHT1, (u8)(data >> 8));
}

void _ag_iec_cgms_a(struct hdmi_tx_app *app, u8 data){
	uint32_t value = 0;
	uint32_t mask = BIT(2) - 1;

	LOG_TRACE1(value);

	value = audio_bridge_read(app, AG_SPDIF_AUDSCHNLS0);

	value &= ~(mask << IEC_CGMSA_SHIFT);
	value |= (data & mask) << IEC_CGMSA_SHIFT;

	audio_bridge_write(app, AG_SPDIF_AUDSCHNLS0, value);
}

void _ag_iec_copyright(struct hdmi_tx_app *app, bool bit){
	uint32_t value = 0;
	uint32_t mask = BIT(1) - 1;

	LOG_TRACE1(bit);

	value = audio_bridge_read(app, AG_SPDIF_AUDSCHNLS0);

	value &= ~(mask << IEC_COPYRIGHT_SHIFT);
	value |= (bit & mask) << IEC_COPYRIGHT_SHIFT;

	audio_bridge_write(app, AG_SPDIF_AUDSCHNLS0, value);
}

void _ag_iec_category_code(struct hdmi_tx_app *app, u8 data){
	LOG_TRACE1(data);
	audio_bridge_write(app, AG_SPDIF_AUDSCHNLS1, data);
}

void _ag_iec_pcm_mode(struct hdmi_tx_app *app, u8 data){
	uint32_t value = 0;
	uint32_t mask = BIT(3) - 1;

	LOG_TRACE1(data);

	value = audio_bridge_read(app, AG_SPDIF_AUDSCHNLS2);

	value &= ~(mask << IEC_PCM_AUDIO_MODE_SHIFT);
	value |= (data & mask) << IEC_PCM_AUDIO_MODE_SHIFT;

	audio_bridge_write(app, AG_SPDIF_AUDSCHNLS2, value);
}

void _ag_iec_source(struct hdmi_tx_app *app, u8 data){
	uint32_t value = 0;
	uint32_t mask = BIT(4) - 1;

	LOG_TRACE1(data);

	value = audio_bridge_read(app, AG_SPDIF_AUDSCHNLS2);

	value &= ~(mask << IEC_SOURCENUMBER_SHIFT);
	value |= (data & mask) << IEC_SOURCENUMBER_SHIFT;

	audio_bridge_write(app, AG_SPDIF_AUDSCHNLS2, value);
}

void _ag_iec_channel_right(struct hdmi_tx_app *app, u8 data, u8 channelNo){
	uint32_t value = 0;
	uint32_t mask = BIT(4) - 1;

	LOG_TRACE1(data);

	switch (channelNo) {
	case 0:
		value = audio_bridge_read(app, AG_SPDIF_AUDSCHNLS3);

		value &= ~(mask << IEC_CHANNELNUMCR0_SHIFT);
		value |= (data & mask) << IEC_CHANNELNUMCR0_SHIFT;

		audio_bridge_write(app, AG_SPDIF_AUDSCHNLS3, value);
		break;
	case 1:
		value = audio_bridge_read(app, AG_GPA_CHNUM1);

		value &= ~(mask << OIEC_CHANNELNUMCR1_SHIFT);
		value |= (data & mask) << OIEC_CHANNELNUMCR1_SHIFT;

		audio_bridge_write(app, AG_GPA_CHNUM1, value);
		break;
	case 2:
		value = audio_bridge_read(app, AG_GPA_CHNUM2);

		value &= ~(mask << OIEC_CHANNELNUMCR2_SHIFT);
		value |= (data & mask) << OIEC_CHANNELNUMCR2_SHIFT;

		audio_bridge_write(app, AG_GPA_CHNUM2, value);
		break;
	case 3:
		value = audio_bridge_read(app, AG_GPA_CHNUM3);

		value &= ~(mask << OIEC_CHANNELNUMCR3_SHIFT);
		value |= (data & mask) << OIEC_CHANNELNUMCR3_SHIFT;

		audio_bridge_write(app, AG_GPA_CHNUM3, value);
		break;
	default:
		LOGGER(SNPS_ERROR,"%s:Wrong channel number", __func__);
		break;
	}

}

void _ag_iec_channel_left(struct hdmi_tx_app *app, u8 data, u8 channelNo){
	uint32_t value = 0;
	uint32_t mask = BIT(4) - 1;

	LOG_TRACE1(data);

	switch (channelNo) {
	case 0:
		value = audio_bridge_read(app, AG_SPDIF_AUDSCHNLS3);

		value &= ~(mask << OIEC_CHANNELNUMCL1_SHIFT);
		value |= (data & mask) << OIEC_CHANNELNUMCL1_SHIFT;

		audio_bridge_write(app, AG_SPDIF_AUDSCHNLS3, value);
		break;
	case 1:
		value = audio_bridge_read(app, AG_GPA_CHNUM1);

		value &= ~(mask << OIEC_CHANNELNUMCL1_SHIFT);
		value |= (data & mask) << OIEC_CHANNELNUMCL1_SHIFT;

		audio_bridge_write(app, AG_GPA_CHNUM1, value);
		break;
	case 2:
		value = audio_bridge_read(app, AG_GPA_CHNUM2);

		value &= ~(mask << OIEC_CHANNELNUMCL2_SHIFT);
		value |= (data & mask) << OIEC_CHANNELNUMCL2_SHIFT;

		audio_bridge_write(app, AG_GPA_CHNUM2, value);
		break;
	case 3:
		value = audio_bridge_read(app, AG_GPA_CHNUM3);

		value &= ~(mask << OIEC_CHANNELNUMCL3_SHIFT);
		value |= (data & mask) << OIEC_CHANNELNUMCL3_SHIFT;

		audio_bridge_write(app, AG_GPA_CHNUM3, value);
		break;
	default:
		LOGGER(SNPS_ERROR,"%s:Wrong channel number", __func__);
		break;
	}
}

void _ag_iec_clock_accuracy(struct hdmi_tx_app *app, u8 data){
	uint32_t value = 0;
	uint32_t mask = BIT(2) - 1;

	LOG_TRACE1(data);

	value = audio_bridge_read(app, AG_SPDIF_AUDSCHNLS4);

	value &= ~(mask << IEC_CLKACCURACY_SHIFT);
	value |= (data & mask) << IEC_CLKACCURACY_SHIFT;

	audio_bridge_write(app, AG_SPDIF_AUDSCHNLS4, value);
}

void _ag_iec_sampling_freq(struct hdmi_tx_app *app, u8 data){
	uint32_t value = 0;
	uint32_t mask = BIT(4) - 1;

	LOG_TRACE1(data);

	value = audio_bridge_read(app, AG_SPDIF_AUDSCHNLS4);

	value &= ~(mask << IEC_SAMPFREQ0_SHIFT);
	value |= (data & mask) << IEC_SAMPFREQ0_SHIFT;
	value &= 0x3f;

	audio_bridge_write(app, AG_SPDIF_AUDSCHNLS4, value);
}

void _ag_iec_original_sampling_freq(struct hdmi_tx_app *app, u8 data){
	uint32_t value = 0;
	uint32_t mask = BIT(4) - 1;

	LOG_TRACE1(data);

	value = audio_bridge_read(app, AG_SPDIF_AUDSCHNLS5);

	value &= ~(mask << IEC_ORIGSAMPFREQ_SHIFT);
	value |= (data & mask) << IEC_ORIGSAMPFREQ_SHIFT;

	audio_bridge_write(app, AG_SPDIF_AUDSCHNLS5, value);
}

void _ag_iec_word_length(struct hdmi_tx_app *app, u8 data){
	uint32_t value = 0;
	uint32_t mask = BIT(4) - 1;

	LOG_TRACE1(data);

	value = audio_bridge_read(app, AG_SPDIF_AUDSCHNLS5);

	value &= ~(mask << IEC_WORDLENGTH_SHIFT);
	value |= (data & mask) << IEC_WORDLENGTH_SHIFT;

	audio_bridge_write(app, AG_SPDIF_AUDSCHNLS5, value);
}

void _ag_user_right(struct hdmi_tx_app *app, u8 bit, u8 channelNo){
	uint32_t value = 0;
	uint32_t mask = BIT(1) - 1;

	LOG_TRACE1(bit);

	switch (channelNo) {
	case 0:
		value = audio_bridge_read(app, AG_SPDIF_CONF);

		value &= ~(mask << USERDATA_CR0_SHIFT);
		value |= (bit & mask) << USERDATA_CR0_SHIFT;

		audio_bridge_write(app, AG_SPDIF_CONF, value);
		break;
	case 1:
		value = audio_bridge_read(app, AG_GPA_USERBIT);

		value &= ~(mask << OUSERDATA_CR1_SHIFT);
		value |= (bit & mask) << OUSERDATA_CR1_SHIFT;

		audio_bridge_write(app, AG_GPA_USERBIT, value);
		break;
	case 2:
		value = audio_bridge_read(app, AG_GPA_USERBIT);

		value &= ~(mask << OUSERDATA_CR2_SHIFT);
		value |= (bit & mask) << OUSERDATA_CR2_SHIFT;

		audio_bridge_write(app, AG_GPA_USERBIT, value);
		break;
	case 3:
		value = audio_bridge_read(app, AG_GPA_USERBIT);

		value &= ~(mask << OUSERDATA_CR3_SHIFT);
		value |= (bit & mask) << OUSERDATA_CR3_SHIFT;

		audio_bridge_write(app, AG_GPA_USERBIT, value);
		break;
	default:
		LOGGER(SNPS_ERROR,"%s:Wrong channel number", __func__);
		break;
	}
}

void _ag_user_left(struct hdmi_tx_app *app, u8 bit, u8 channelNo){
	uint32_t value = 0;
	uint32_t mask = BIT(1) - 1;

	LOG_TRACE1(bit);

	switch (channelNo) {
	case 0:
		value = audio_bridge_read(app, AG_SPDIF_CONF);

		value &= ~(mask << USERDATA_CL0_SHIFT);
		value |= (bit & mask) << USERDATA_CL0_SHIFT;

		audio_bridge_write(app, AG_SPDIF_CONF, value);
		break;
	case 1:
		value = audio_bridge_read(app, AG_GPA_USERBIT);

		value &= ~(mask << OUSERDATA_CL1_SHIFT);
		value |= (bit & mask) << OUSERDATA_CL1_SHIFT;

		audio_bridge_write(app, AG_GPA_USERBIT, value);
		break;
	case 2:
		value = audio_bridge_read(app, AG_GPA_USERBIT);

		value &= ~(mask << OUSERDATA_CL2_SHIFT);
		value |= (bit & mask) << OUSERDATA_CL2_SHIFT;

		audio_bridge_write(app, AG_GPA_USERBIT, value);
		break;
	case 3:
		value = audio_bridge_read(app, AG_GPA_USERBIT);

		value &= ~(mask << OUSERDATA_CL3_SHIFT);
		value |= (bit & mask) << OUSERDATA_CL3_SHIFT;

		audio_bridge_write(app, AG_GPA_USERBIT, value);
		break;
	default:
		LOGGER(SNPS_ERROR,"%s:Wrong channel number", __func__);
		break;
	}
}

void _ag_spdif_tx_data(struct hdmi_tx_app *app, bool bit){
	uint32_t value = 0;
	uint32_t mask = BIT(1) - 1;

	LOG_TRACE1(bit);

	value = audio_bridge_read(app, AG_SPDIF_CONF);

	value &= ~(mask << SPDIFTXDATA_SHIFT);
	value |= (bit & mask) << SPDIFTXDATA_SHIFT;

	audio_bridge_write(app, AG_SPDIF_CONF, value);
}


void _ag_hbr_enable(struct hdmi_tx_app *app, bool bit)
{
#ifdef AUDIO_HBR_BLOCK
	LOG_TRACE1(bit);
	access_CoreWrite(dev, bit, AG_HBR_CONF, 0, 1);
#endif
}

void _ag_hbr_ddr_enable(struct hdmi_tx_app *app, bool bit)
{
#ifdef AUDIO_HBR_BLOCK
	LOG_TRACE1(bit);
	access_CoreWrite(dev, bit, AG_HBR_CONF, 1, 1);
#endif
}

void _ag_hbr_ddr_channel(struct hdmi_tx_app *app, bool bit)
{
#ifdef AUDIO_HBR_BLOCK
	LOG_TRACE1(bit);
	access_CoreWrite(dev, bit, AG_HBR_CONF, 2, 1);
#endif
}

void _ag_hbr_burst_length(struct hdmi_tx_app *app, bool value)
{
#ifdef AUDIO_HBR_BLOCK
	LOG_TRACE1(value);
	access_CoreWrite(dev, value, AG_HBR_CONF, 3, 4);
#endif
}

void _ag_hbr_clock_divider(struct hdmi_tx_app *app, u16 value)
{
#ifdef AUDIO_HBR_BLOCK
	LOG_TRACE1(value);
	/* 9-bit width */
	dev_write(dev, (u8) (value), AG_HBR_CLKDIV0);
	access_CoreWrite(dev, value >> 8, AG_HBR_CLKDIV1, 0, 1);
#endif
}


void _ag_use_look_up_table(struct hdmi_tx_app *app, bool bit){
	uint32_t value = 0;
	uint32_t mask = BIT(1) - 1;

	LOG_TRACE1(bit);

	value = audio_bridge_read(app, AG_GPA_CONF0);

	value &= ~(mask << OAUDIO_USE_LUT_SHIFT);
	value |= (bit & mask) << OAUDIO_USE_LUT_SHIFT;

	audio_bridge_write(app, AG_GPA_CONF0, value);
}

void _ag_gpa_reply_latency(struct hdmi_tx_app *app, u8 data){
	uint32_t value = 0;
	uint32_t mask = BIT(2) - 1;

	LOG_TRACE1(data);

	value = audio_bridge_read(app, AG_GPA_CONF0);

	value &= ~(mask << OGPAREPLY_LATENCY_SHIFT);
	value |= (data & mask) << OGPAREPLY_LATENCY_SHIFT;

	audio_bridge_write(app, AG_GPA_CONF0, value);
}

void _ag_channel_select(struct hdmi_tx_app *app, bool enable, u8 channel){
	uint32_t value = 0;
	uint32_t mask = BIT(1) - 1;

	LOGGER(SNPS_INFO, "%s:channel %d - enable %d", __func__, channel,
			enable);

	value = audio_bridge_read(app, AG_GPA_CONF1);

	value &= ~(mask << channel);
	value |= (enable & mask) << channel;

	audio_bridge_write(app, AG_GPA_CONF1, value);
}

void _ag_gpa_sample_valid(struct hdmi_tx_app *app, bool enable, u8 channelNo){
	uint32_t value = 0;
	uint32_t mask = BIT(1) - 1;

	LOGGER(SNPS_INFO, "%s:channelNo %d - enable %d", __func__, enable,
			channelNo);

	value = audio_bridge_read(app, AG_GPA_SAMPLEVALID);

	value &= ~(mask << channelNo);
	value |= (enable & mask) << channelNo;

	audio_bridge_write(app, AG_GPA_SAMPLEVALID, value);
}

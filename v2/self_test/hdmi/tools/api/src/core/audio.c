// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "core/audio.h"
#include "core/fc_audio.h"
#include "core/fc_audio_info.h"
#include "core/audio_i2s.h"
#include "core/audio_spdif.h"
#include "core/main_controller.h"
#include "util/log.h"
#include "general_ops.h"
#include "bsp/board.h"
#include "core/packets.h"

//#include "core/halAudioClock.h"
#include "audio/audio_packetizer_reg.h"
#include "bsp/access.h"
#include "util/log.h"
#include "../../src/core/audio/audio_packetizer_reg.h"
#include "../frame_composer/frame_composer_reg.h"
#include "../../include/core/interrupt/interrupt_reg.h"
typedef struct audio_n_computation {
	double pixel_clock;
	u32 n;
}audio_n_computation_t;

audio_n_computation_t n_values_32[] = {
	{0,      4096},
	{25.175, 4576},
	{25.200, 4096},
	{27.000, 4096},
	{27.027, 4096},
	{54.000, 4096},
	{54.054, 4096},
	{74.176,11648},
	{74.250, 4096},
	{148.352,11648},
	{148.50, 4096},
	{296.703,5824},
	{297.00, 3072},
	{0, 0}
};

audio_n_computation_t n_values_44p1[] = {
	{0,      6272},
	{25.175, 7007},
	{25.200, 6272},
	{27.000, 6272},
	{27.027, 6272},
	{54.000, 6272},
	{54.054, 6272},
	{74.176,17836},
	{74.250, 6272},
	{148.352,8918},
	{148.50, 6272},
	{296.703,4459},
	{297.00, 4704},
	{0, 0}
};

audio_n_computation_t n_values_48[] = {
	{0,      6144},
	{25.175, 6864},
	{25.200, 6144},
	{27.000, 6144},
	{27.027, 6144},
	{54.000, 6144},
	{54.054, 6144},
	{74.176,11648},
	{74.250, 6144},
	{148.352,5824},
	{148.50, 6144},
	{296.703,5824},
	{297.00, 5120},
	{0, 0}
};


/**********************************************
 * Internal functions
 */
void _audio_clock_n(hdmi_tx_dev_t *dev, u32 value)
{
	LOG_TRACE();
	/* 19-bit width */
	dev_write_mask(dev, AUD_N3, AUD_N3_AUDN_MASK, (u8)(value >> 16));
	dev_write(dev, AUD_N2, (u8)(value >> 8));
	dev_write(dev, AUD_N1, (u8)(value >> 0));
	
	
	/* no shift */
	dev_write_mask(dev, AUD_CTS3, AUD_CTS3_N_SHIFT_MASK, 0);
}

void _audio_clock_cts(hdmi_tx_dev_t *dev, u32 value)
{
	LOG_TRACE1(value);
	if(value > 0){
		/* 19-bit width */		
		dev_write_mask(dev, AUD_CTS3, AUD_CTS3_AUDCTS_MASK, (u8)(value >> 16));
		dev_write_mask(dev, AUD_CTS3, AUD_CTS3_CTS_MANUAL_MASK, 1);
		dev_write(dev, AUD_CTS2, (u8)(value >> 8));
		dev_write(dev, AUD_CTS1, (u8)(value >> 0));
	}
	else{
		/* Set to automatic generation of CTS values */
		dev_write_mask(dev, AUD_CTS3, AUD_CTS3_CTS_MANUAL_MASK, 0);
	}
}

void _audio_clock_atomic(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE();
	dev_write_mask(dev, AUD_N3, AUD_N3_NCTS_ATOMIC_WRITE_MASK, value);
}


void _audio_clock_f(hdmi_tx_dev_t *dev, u8 value)
{
	LOG_TRACE();
	dev_write_mask(dev, AUD_INPUTCLKFS, AUD_INPUTCLKFS_IFSFACTOR_MASK, value);
}

int configure_i2s(hdmi_tx_dev_t *dev, audioParams_t * audio)
{
	audio_i2s_configure(dev, audio);

	switch (audio->mClockFsFactor) {
	case 64:
		_audio_clock_f(dev, 4);
		break;
	case 128:
		_audio_clock_f(dev, 0);
		break;
	case 256:
		_audio_clock_f(dev, 1);
		break;
	case 512:
		_audio_clock_f(dev, 2);
		break;
	default:
		_audio_clock_f(dev, 0);
		LOGGER(SNPS_ERROR, "%s:Fs Factor [%d] not supported", __func__, audio->mClockFsFactor);
		return FALSE;
		break;
	}

	return TRUE;
}

int configure_spdif(hdmi_tx_dev_t *dev, audioParams_t * audio)
{
	audio_spdif_config(dev, audio);

	switch (audio->mClockFsFactor) {
	case 64:
		_audio_clock_f(dev, 0);
		break;
	case 128:
		_audio_clock_f(dev, 0);
		break;
	case 256:
		_audio_clock_f(dev, 0);
		break;
	case 512:
		_audio_clock_f(dev, 2);
		break;
	default:
		_audio_clock_f(dev, 0);
		LOGGER(SNPS_ERROR, "%s:Fs Factor [%d] not supported", __func__, audio->mClockFsFactor);
		return FALSE;
		break;
	}

	return TRUE;
}

#define ENABLE_HLOCK_MASK        0x8
#define SELECT_BURST_TYPE_MASK   0x6
#define BURST_EN_MASK            0x1
#define FIFO_THRESHOLD_MASK      0xff
#define INITIAL_ADDR_MASK        0xff
#define FINAL_ADDR_MASK          0xff
#define AHB_DMA_START_MASK       0x1
#define MAS_AUDIO_SAMPLE_MASK    0x1
#define HBR_AUDIO_SAMPLE_MASK    0x10
#define AUTO_START_AND_EN_MASK   0x3
#define MULTI_STREAM_MASK        0x1
#define INSERT_PCUV				 0x40

void audio_ahbdma(hdmi_tx_dev_t *dev)
{

	dev_write(dev, IH_AHBDMAAUD_STAT0, 0x0);

	dev_write_mask(dev, AHB_DMA_CONF0, ENABLE_HLOCK_MASK, 0x1);           //enable_hlock
	dev_write_mask(dev, AHB_DMA_CONF0, INSERT_PCUV, 0x1);				//insert PCUV
	dev_write_mask(dev, AHB_DMA_CONF0, SELECT_BURST_TYPE_MASK, 0x0);    //Select the desired burst type
	dev_write_mask(dev, AHB_DMA_CONF0, BURST_EN_MASK, 0x1);   // select burst_en  burst mode
	dev_write(dev, AHB_DMA_CONF1, 0x3);                          // select 2 channels
	
	dev_write_mask(dev, AHB_DMA_THRSLD, FIFO_THRESHOLD_MASK, 0x0f);    //Select the FIFO threshold
	dev_write_mask(dev, AHB_DMA_STRADDR_SET0 + 0x8 , INITIAL_ADDR_MASK ,0x10 );
	dev_write_mask(dev, AHB_DMA_STRADDR_SET0 + 0xC , INITIAL_ADDR_MASK ,0x7 );    //initial_addr[31:0] bit fields.

    dev_write_mask(dev, AHB_DMA_STPADDR_SET0, FINAL_ADDR_MASK , 0X30);       //final_addr[31:0] bit fields
	dev_write_mask(dev, AHB_DMA_STPADDR_SET0 + 0x4, FINAL_ADDR_MASK , 0x0);
	dev_write_mask(dev, AHB_DMA_STPADDR_SET0 + 0x8, FINAL_ADDR_MASK , 0x10);
	dev_write_mask(dev, AHB_DMA_STPADDR_SET0 + 0xC, FINAL_ADDR_MASK , 0x7);

	dev_write_mask(dev, AHB_DMA_STRADDR_SET1, INITIAL_ADDR_MASK ,0x31 );
	dev_write_mask(dev, AHB_DMA_STRADDR_SET1 + 0x4 , INITIAL_ADDR_MASK ,0x0 );
	dev_write_mask(dev, AHB_DMA_STRADDR_SET1 + 0x8 , INITIAL_ADDR_MASK ,0x10 );
	dev_write_mask(dev, AHB_DMA_STRADDR_SET1 + 0xC , INITIAL_ADDR_MASK ,0x7 );    //initial_addr1[31:0] bit fields.

	dev_write_mask(dev, AHB_DMA_STPADDR_SET1, FINAL_ADDR_MASK , 0X70);       //final_addr1[31:0] bit fields
	dev_write_mask(dev, AHB_DMA_STPADDR_SET1 + 0x4, FINAL_ADDR_MASK , 0x0);
	dev_write_mask(dev, AHB_DMA_STPADDR_SET1 + 0x8, FINAL_ADDR_MASK , 0x10);
	dev_write_mask(dev, AHB_DMA_STPADDR_SET1 + 0xC, FINAL_ADDR_MASK , 0x7);

	dev_write(dev, AHB_DMA_CONF2, 0x1);
	dev_write(dev, AHB_DMA_CONF2, 0x3);
	
	dev_write(dev, FC_AUDICONF1, 0x32);
	dev_write(dev, FC_AUDSCHNL3, 0x44);
	dev_write(dev, FC_AUDSCHNL4, 0x44);
	dev_write(dev, FC_AUDSCHNL5, 0x88);
	dev_write(dev, FC_AUDSCHNL6, 0x88);
	//PCUV config
	// dev_write(dev,FC_AUDSV, 0xff);    //validity bit
	// dev_write(dev,FC_AUDSU

	dev_write_mask(dev, AHB_DMA_START, AHB_DMA_START_MASK, 0x1);                //Order the AHB DMA to start reading
	
	// dev_write_mask(dev, AHB_DMA_CONF0, HBR_AUDIO_SAMPLE_MASK, 0x0 );				//Define audio samples transport method
	// dev_write(dev, FC_MULTISTREAM_CTRL, 0x1);

}
/**********************************************
 * External functions
 */
int audio_Initialize(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();

	// Mask all interrupts
	// audio_i2s_interrupt_mask(dev, 0x3);
	// audio_spdif_interrupt_mask(dev, 0x3);

	return audio_mute(dev,  1);
}

int audio_Configure(hdmi_tx_dev_t *dev, audioParams_t * audio)
{
	u32 n = 0;
	int ret = 0;
//	static int counter = 0;
//	uint32_t value = 0;

	LOG_TRACE();

	if((dev == NULL) || (audio == NULL)){
		LOGGER(SNPS_ERROR, "Improper function arguments");
		return FALSE;
	}


	if(dev->snps_hdmi_ctrl.audio_on == 0){
		LOGGER(SNPS_WARN, "DVI mode selected: audio not configured");
		return TRUE;
	}

	ret = configure_audio_pll(dev, audio->mSamplingFrequency, audio->mClockFsFactor);
	if (ret == FALSE) {
		LOGGER(SNPS_ERROR, "Could not configure audio MMCM");
		return FALSE;
	}

	LOGGER(SNPS_DEBUG, "Audio interface type = %s", audio->mInterfaceType == I2S ? "I2S" :
													audio->mInterfaceType == SPDIF ? "SPDIF" :
													audio->mInterfaceType == HBR ? "HBR" :
													audio->mInterfaceType == GPA ? "GPA" :
													audio->mInterfaceType == DMA ? "DMA" : "---");

	LOGGER(SNPS_DEBUG, "Audio coding = %s", audio->mCodingType == PCM ? "PCM" :
								   audio->mCodingType == AC3 ? "AC3" :
								   audio->mCodingType == MPEG1 ? "MPEG1" :
								   audio->mCodingType == MP3 ? "MP3" :
								   audio->mCodingType == MPEG2 ? "MPEG2" :
								   audio->mCodingType == AAC ? "AAC" :
								   audio->mCodingType == DTS ? "DTS" :
								   audio->mCodingType == ATRAC ? "ATRAC" :
								   audio->mCodingType == ONE_BIT_AUDIO ? "ONE BIT AUDIO" :
								   audio->mCodingType == DOLBY_DIGITAL_PLUS ? "DOLBY DIGITAL +" :
								   audio->mCodingType == DTS_HD ? "DTS HD" : "----");
	LOGGER(SNPS_DEBUG, "Audio frequency = %.3fkHz", audio->mSamplingFrequency);
	LOGGER(SNPS_DEBUG, "Audio sample size = %d", audio->mSampleSize);
	LOGGER(SNPS_DEBUG, "Audio FS factor = %d\n", audio->mClockFsFactor);

	// Set PCUV info from external source
	audio->mGpaInsertPucv = 1;

	audio_mute(dev, 1);

	// Configure Frame Composer audio parameters
	fc_audio_config(dev, audio);

	if(audio->mInterfaceType == I2S){
		if(configure_i2s(dev, audio) == FALSE){
			LOGGER(SNPS_ERROR, "I2S audio configuration failed");
			return FALSE;
		}
	}
	else if(audio->mInterfaceType == SPDIF){
		if(configure_spdif(dev, audio) == FALSE){
			LOGGER(SNPS_ERROR, "SPDIF audio configuration failed");
			return FALSE;
		}
	}
	else if (audio->mInterfaceType == DMA) {
		audio_ahbdma(dev);
		printf("ahbdma is selected\n");
	}
	else if(audio->mInterfaceType == HBR){
		LOGGER(SNPS_ERROR, "HBR is not supported");
		return FALSE;
	}
	else if (audio->mInterfaceType == GPA) {
		LOGGER(SNPS_ERROR, "GPA is not supported");
		return FALSE;
	}

	else if (audio->mInterfaceType == DMA) {
		LOGGER(SNPS_ERROR, "DMA is not supported");
		return FALSE;
	}

	n = audio_ComputeN(dev, audio->mSamplingFrequency, dev->snps_hdmi_ctrl.pixel_clock);
	printf("audio:n=%d\n",n);
	mc_audio_sampler_clock_enable(dev, 0);
	
	snps_sleep(1000);

	_audio_clock_atomic(dev, 1);
	
	_audio_clock_cts(dev, 165000);
	
	_audio_clock_n(dev, n);		

	audio_mute(dev, 0);

	// Configure audio info frame packets
	fc_audio_info_config(dev, audio);

	return TRUE;
}

int audio_mute(hdmi_tx_dev_t *dev, u8 state)
{
	/* audio mute priority: AVMUTE, sample flat, validity */
	/* AVMUTE also mutes video */
	// TODO: Check the audio mute process
	if(state){
		fc_audio_mute(dev);
	}
	else{
		fc_audio_unmute(dev);
	}
	return TRUE;
}

u32 audio_ComputeN(hdmi_tx_dev_t *dev, double freq, double pixelClk)
{
	int i = 0;
	u32 n = 0;
	audio_n_computation_t *n_values = NULL;
	int multiplier_factor = 1;

	if(double_is_equal(freq, 64) || double_is_equal(freq, 88.2) || double_is_equal(freq, 96)){
		multiplier_factor = 2;
	}
	else if(double_is_equal(freq, 128) || double_is_equal(freq, 176.4) || double_is_equal(freq, 192)){
		multiplier_factor = 4;
	}
	else if(double_is_equal(freq, 256) || double_is_equal(freq, 352.8) || double_is_equal(freq, 384)){
		multiplier_factor = 8;
	}

	if(double_is_equal(32.000, freq/multiplier_factor)){
		n_values = n_values_32;
	}
	else if(double_is_equal(44.100, freq/multiplier_factor)){
		n_values = n_values_44p1;
	}
	else if(double_is_equal(48.000, freq/multiplier_factor)){
		n_values = n_values_48;
	}
	else {
		LOGGER(SNPS_ERROR, "%s:Could not compute N values", __func__);
		LOGGER(SNPS_ERROR, "%s:Audio Frequency %f", __func__, freq);
		LOGGER(SNPS_ERROR, "%s:Pixel Clock %f", __func__, pixelClk);
		LOGGER(SNPS_ERROR, "%s:Multiplier factor %d", __func__, multiplier_factor);
		return FALSE;
	}

	for(i = 0; n_values[i].n != 0; i++){
		if(double_is_equal(pixelClk, n_values[i].pixel_clock)){
			n = n_values[i].n * multiplier_factor;
			LOGGER(SNPS_DEBUG, "Audio N value = %d", n);
			return n;
		}
	}

	n = n_values[0].n * multiplier_factor;

	LOGGER(SNPS_DEBUG, "Audio N value default = %d", n);

	return n;
}

u32 audio_ComputeCts(hdmi_tx_dev_t *dev, double freq, double pixelClk, u16 ratioClk)
{
	u32 cts = 0;

	uint32_t temp_freq = (uint32_t)(freq * 1000);
	uint32_t temp_pixel_clk = (uint32_t)(pixelClk * 1000);

	cts = temp_pixel_clk * audio_ComputeN(dev, freq, pixelClk);
	cts = cts / 128;
	cts = (cts * 100) / (temp_freq / 100);
	cts = cts * ratioClk;
	cts = cts / 100;
	if ((temp_pixel_clk * (ratioClk / 100)) > 34000) {
		cts = cts * 4;
	}
	LOGGER(SNPS_DEBUG, "audio_ComputeCtsEquation CTS = %d", cts);
	return cts;
}

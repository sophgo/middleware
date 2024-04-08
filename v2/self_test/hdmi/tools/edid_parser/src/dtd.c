/*
 * @file:dtd.c
 *
 *
 * Synopsys Inc.
 * SG DWC PT02
 */

#include "dtd.h"
#include "bit_operation.h"


int dtd_parse(hdmi_tx_dev_t *dev, dtd_t * dtd, u8 data[18])
{
	

	dtd->mCode = -1;
	dtd->mPixelRepetitionInput = 0;

	dtd->mPixelClock = byte_to_word(data[1], data[0]);	/*  [10000Hz] */
	if (dtd->mPixelClock < 0x01) {	/* 0x0000 is defined as reserved */
		return FALSE;
	}

	dtd->mHActive = concat_bits(data[4], 4, 4, data[2], 0, 8);
	dtd->mHBlanking = concat_bits(data[4], 0, 4, data[3], 0, 8);
	dtd->mHSyncOffset = concat_bits(data[11], 6, 2, data[8], 0, 8);
	dtd->mHSyncPulseWidth = concat_bits(data[11], 4, 2, data[9], 0, 8);
	dtd->mHImageSize = concat_bits(data[14], 4, 4, data[12], 0, 8);
	dtd->mHBorder = data[15];

	dtd->mVActive = concat_bits(data[7], 4, 4, data[5], 0, 8);
	dtd->mVBlanking = concat_bits(data[7], 0, 4, data[6], 0, 8);
	dtd->mVSyncOffset = concat_bits(data[11], 2, 2, data[10], 4, 4);
	dtd->mVSyncPulseWidth = concat_bits(data[11], 0, 2, data[10], 0, 4);
	dtd->mVImageSize = concat_bits(data[14], 0, 4, data[13], 0, 8);
	dtd->mVBorder = data[16];

	if (bit_field(data[17], 4, 1) != 1) {	/* if not DIGITAL SYNC SIGNAL DEF */
		return FALSE;
	}
	if (bit_field(data[17], 3, 1) != 1) {	/* if not DIGITAL SEPATATE SYNC */
		return FALSE;
	}
	/* no stereo viewing support in HDMI */
	dtd->mInterlaced = bit_field(data[17], 7, 1) == 1;
	dtd->mVSyncPolarity = bit_field(data[17], 2, 1) == 1;
	dtd->mHSyncPolarity = bit_field(data[17], 1, 1) == 1;
	return TRUE;
}

int dtd_fill(hdmi_tx_dev_t *dev, dtd_t * dtd, u8 code, double refreshRate)
{
	
	dtd->mCode = code;
	dtd->mHBorder = 0;
	dtd->mVBorder = 0;
	dtd->mPixelRepetitionInput = 0;
	dtd->mHImageSize = 16;
	dtd->mVImageSize = 9;
	dtd->mYcc420 = 0;
	dtd->mLimitedToYcc420 = 0;

	switch (code) {
	case 1:		/* 640x480p @ 59.94/60Hz 4:3 */
		dtd->mHImageSize = 4;
		dtd->mVImageSize = 3;
		dtd->mHActive = 640;
		dtd->mVActive = 480;
		dtd->mHBlanking = 160;
		dtd->mVBlanking = 45;
		dtd->mHSyncOffset = 16;
		dtd->mVSyncOffset = 10;
		dtd->mHSyncPulseWidth = 96;
		dtd->mVSyncPulseWidth = 2;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 0;	/* Active low */
		dtd->mInterlaced = 0;	/* not(progressive_nI) */
		dtd->mPixelClock = (refreshRate == 59.940) ? 25.175 : 25.200;	/* pixel clock depends on refresh rate */
		break;
	case 2:		/* 720x480p @ 59.94/60Hz 4:3 */
		dtd->mHImageSize = 4;
		dtd->mVImageSize = 3;
	case 3:		/* 720x480p @ 59.94/60Hz 16:9 */
		dtd->mHActive = 720;
		dtd->mVActive = 480;
		dtd->mHBlanking = 138;
		dtd->mVBlanking = 45;
		dtd->mHSyncOffset = 16;
		dtd->mVSyncOffset = 9;
		dtd->mHSyncPulseWidth = 62;
		dtd->mVSyncPulseWidth = 6;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 0;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = (refreshRate == 59.940) ? 27.000 : 27.027;
		break;
	case 4:		/* 1280x720p @ 59.94/60Hz 16:9 */
		dtd->mHActive = 1280;
		dtd->mVActive = 720;
		dtd->mHBlanking = 370;
		dtd->mVBlanking = 30;
		dtd->mHSyncOffset = 110;
		dtd->mVSyncOffset = 5;
		dtd->mHSyncPulseWidth = 40;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = (refreshRate == 59.940) ? 74.176 : 74.250;
		break;
	case 5:		/* 1920x1080i @ 59.94/60Hz 16:9 */
		dtd->mHActive = 1920;
		dtd->mVActive = 540;
		dtd->mHBlanking = 280;
		dtd->mVBlanking = 22;
		dtd->mHSyncOffset = 88;
		dtd->mVSyncOffset = 2;
		dtd->mHSyncPulseWidth = 44;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 1;
		dtd->mPixelClock = (refreshRate == 59.940) ? 74.176 : 74.250;
		break;
	case 6:		/* 720(1440)x480i @ 59.94/60Hz 4:3 */
		dtd->mHImageSize = 4;
		dtd->mVImageSize = 3;
	case 7:		/* 720(1440)x480i @ 59.94/60Hz 16:9 */
		dtd->mHActive = 1440;
		dtd->mVActive = 240;
		dtd->mHBlanking = 276;
		dtd->mVBlanking = 22;
		dtd->mHSyncOffset = 38;
		dtd->mVSyncOffset = 4;
		dtd->mHSyncPulseWidth = 124;
		dtd->mVSyncPulseWidth = 3;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 0;
		dtd->mInterlaced = 1;
		dtd->mPixelClock = (refreshRate == 59.940) ? 27.000 : 27.027;
		dtd->mPixelRepetitionInput = 1;
		break;
	case 8:		/* 720(1440)x240p @ 59.826/60.054/59.886/60.115Hz 4:3 */
		dtd->mHImageSize = 4;
		dtd->mVImageSize = 3;
	case 9:		/* 720(1440)x240p @ 59.826/60.054/59.886/60.115Hz 16:9 */
		dtd->mHActive = 1440;
		dtd->mVActive = 240;
		dtd->mHBlanking = 276;
		dtd->mVBlanking = (refreshRate > 60.000) ? 22 : 23;
		dtd->mHSyncOffset = 38;
		dtd->mVSyncOffset = (refreshRate > 60.000) ? 4 : 5;
		dtd->mHSyncPulseWidth = 124;
		dtd->mVSyncPulseWidth = 3;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 0;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = ((refreshRate == 60.054) || refreshRate == 59.826) ? 27.000 : 27.027;	/*  else 60.115/59.886 Hz */
		dtd->mPixelRepetitionInput = 1;
		break;
	case 10:		/* 2880x480i @ 59.94/60Hz 4:3 */
		dtd->mHImageSize = 4;
		dtd->mVImageSize = 3;
	case 11:		/* 2880x480i @ 59.94/60Hz 16:9 */
		dtd->mHActive = 2880;
		dtd->mVActive = 240;
		dtd->mHBlanking = 552;
		dtd->mVBlanking = 22;
		dtd->mHSyncOffset = 76;
		dtd->mVSyncOffset = 4;
		dtd->mHSyncPulseWidth = 248;
		dtd->mVSyncPulseWidth = 3;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 0;
		dtd->mInterlaced = 1;
		dtd->mPixelClock = (refreshRate == 59.940) ? 54.000 : 54.054;
		break;
	case 12:		/* 2880x240p @ 59.826/60.054/59.886/60.115Hz 4:3 */
		dtd->mHImageSize = 4;
		dtd->mVImageSize = 3;
	case 13:		/* 2880x240p @ 59.826/60.054/59.886/60.115Hz 16:9 */
		dtd->mHActive = 2880;
		dtd->mVActive = 240;
		dtd->mHBlanking = 552;
		dtd->mVBlanking = (refreshRate > 60.000) ? 22 : 23;
		dtd->mHSyncOffset = 76;
		dtd->mVSyncOffset = (refreshRate > 60.000) ? 4 : 5;
		dtd->mHSyncPulseWidth = 248;
		dtd->mVSyncPulseWidth = 3;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 0;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = ((refreshRate == 60.054) || refreshRate == 59.826) ? 54.000 : 54.054;	/*  else 60.115/59.886 Hz */
		break;
	case 14:		/* 1440x480p @ 59.94/60Hz 4:3 */
		dtd->mHImageSize = 4;
		dtd->mVImageSize = 3;
	case 15:		/* 1440x480p @ 59.94/60Hz 16:9 */
		dtd->mHActive = 1440;
		dtd->mVActive = 480;
		dtd->mHBlanking = 276;
		dtd->mVBlanking = 45;
		dtd->mHSyncOffset = 32;
		dtd->mVSyncOffset = 9;
		dtd->mHSyncPulseWidth = 124;
		dtd->mVSyncPulseWidth = 6;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 0;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = (refreshRate == 59.940) ? 54.000 : 54.054;
		break;
	case 16:		/* 1920x1080p @ 59.94/60Hz 16:9 */
		dtd->mHActive = 1920;
		dtd->mVActive = 1080;
		dtd->mHBlanking = 280;
		dtd->mVBlanking = 45;
		dtd->mHSyncOffset = 88;
		dtd->mVSyncOffset = 4;
		dtd->mHSyncPulseWidth = 44;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = (refreshRate == 59.940) ? 148.352 : 148.50;
		break;
	case 17:		/* 720x576p @ 50Hz 4:3 */
		dtd->mHImageSize = 4;
		dtd->mVImageSize = 3;
	case 18:		/* 720x576p @ 50Hz 16:9 */
		dtd->mHActive = 720;
		dtd->mVActive = 576;
		dtd->mHBlanking = 144;
		dtd->mVBlanking = 49;
		dtd->mHSyncOffset = 12;
		dtd->mVSyncOffset = 5;
		dtd->mHSyncPulseWidth = 64;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 0;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 27.00;
		break;
	case 19:		/* 1280x720p @ 50Hz 16:9 */
		dtd->mHActive = 1280;
		dtd->mVActive = 720;
		dtd->mHBlanking = 700;
		dtd->mVBlanking = 30;
		dtd->mHSyncOffset = 440;
		dtd->mVSyncOffset = 5;
		dtd->mHSyncPulseWidth = 40;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 74.25;
		break;
	case 20:		/* 1920x1080i @ 50Hz 16:9 */
		dtd->mHActive = 1920;
		dtd->mVActive = 540;
		dtd->mHBlanking = 720;
		dtd->mVBlanking = 22;
		dtd->mHSyncOffset = 528;
		dtd->mVSyncOffset = 2;
		dtd->mHSyncPulseWidth = 44;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 1;
		dtd->mPixelClock = 74.250;
		break;
	case 21:		/* 720(1440)x576i @ 50Hz 4:3 */
		dtd->mHImageSize = 4;
		dtd->mVImageSize = 3;
	case 22:		/* 720(1440)x576i @ 50Hz 16:9 */
		dtd->mHActive = 1440;
		dtd->mVActive = 288;
		dtd->mHBlanking = 288;
		dtd->mVBlanking = 24;
		dtd->mHSyncOffset = 24;
		dtd->mVSyncOffset = 2;
		dtd->mHSyncPulseWidth = 126;
		dtd->mVSyncPulseWidth = 3;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 0;
		dtd->mInterlaced = 1;
		dtd->mPixelClock = 27.00;
		dtd->mPixelRepetitionInput = 1;
		break;
	case 23:		/* 720(1440)x288p @ 50Hz 4:3 */
		dtd->mHImageSize = 4;
		dtd->mVImageSize = 3;
	case 24:		/* 720(1440)x288p @ 50Hz 16:9 */
		dtd->mHActive = 1440;
		dtd->mVActive = 288;
		dtd->mHBlanking = 288;
		dtd->mVBlanking = (refreshRate == 50.000) ? 24
		    : ((refreshRate == 49.920) ? 25 : 26);
		dtd->mHSyncOffset = 24;
		dtd->mVSyncOffset = (refreshRate == 50.000) ? 2
		    : ((refreshRate == 49.920) ? 3 : 4);
		dtd->mHSyncPulseWidth = 126;
		dtd->mVSyncPulseWidth = 3;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 0;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 27.00;
		dtd->mPixelRepetitionInput = 1;
		break;
	case 25:		/* 2880x576i @ 50Hz 4:3 */
		dtd->mHImageSize = 4;
		dtd->mVImageSize = 3;
	case 26:		/* 2880x576i @ 50Hz 16:9 */
		dtd->mHActive = 2880;
		dtd->mVActive = 288;
		dtd->mHBlanking = 576;
		dtd->mVBlanking = 24;
		dtd->mHSyncOffset = 48;
		dtd->mVSyncOffset = 2;
		dtd->mHSyncPulseWidth = 252;
		dtd->mVSyncPulseWidth = 3;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 0;
		dtd->mInterlaced = 1;
		dtd->mPixelClock = 54.00;
		break;
	case 27:		/* 2880x288p @ 50Hz 4:3 */
		dtd->mHImageSize = 4;
		dtd->mVImageSize = 3;
	case 28:		/* 2880x288p @ 50Hz 16:9 */
		dtd->mHActive = 2880;
		dtd->mVActive = 288;
		dtd->mHBlanking = 576;
		dtd->mVBlanking = (refreshRate == 50.000) ? 24
		    : ((refreshRate == 49.920) ? 25 : 26);
		dtd->mHSyncOffset = 48;
		dtd->mVSyncOffset = (refreshRate == 50.000) ? 2
		    : ((refreshRate == 49.920) ? 3 : 4);
		dtd->mHSyncPulseWidth = 252;
		dtd->mVSyncPulseWidth = 3;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 0;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 54.000;
		break;
	case 29:		/* 1440x576p @ 50Hz 4:3 */
		dtd->mHImageSize = 4;
		dtd->mVImageSize = 3;
	case 30:		/* 1440x576p @ 50Hz 16:9 */
		dtd->mHActive = 1440;
		dtd->mVActive = 576;
		dtd->mHBlanking = 288;
		dtd->mVBlanking = 49;
		dtd->mHSyncOffset = 24;
		dtd->mVSyncOffset = 5;
		dtd->mHSyncPulseWidth = 128;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 0;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 54.000;
		break;
	case 31:		/* 1920x1080p @ 50Hz 16:9 */
		dtd->mHActive = 1920;
		dtd->mVActive = 1080;
		dtd->mHBlanking = 720;
		dtd->mVBlanking = 45;
		dtd->mHSyncOffset = 528;
		dtd->mVSyncOffset = 4;
		dtd->mHSyncPulseWidth = 44;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 148.50;
		break;
	case 32:		/* 1920x1080p @ 23.976/24Hz 16:9 */
		dtd->mHActive = 1920;
		dtd->mVActive = 1080;
		dtd->mHBlanking = 830;
		dtd->mVBlanking = 45;
		dtd->mHSyncOffset = 638;
		dtd->mVSyncOffset = 4;
		dtd->mHSyncPulseWidth = 44;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = (refreshRate == 23.976) ? 74.176 : 74.250;
		break;
	case 33:		/* 1920x1080p @ 25Hz 16:9 */
		dtd->mHActive = 1920;
		dtd->mVActive = 1080;
		dtd->mHBlanking = 720;
		dtd->mVBlanking = 45;
		dtd->mHSyncOffset = 528;
		dtd->mVSyncOffset = 4;
		dtd->mHSyncPulseWidth = 44;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 74.25;
		break;
	case 34:		/* 1920x1080p @ 29.97/30Hz 16:9 */
		dtd->mHActive = 1920;
		dtd->mVActive = 1080;
		dtd->mHBlanking = 280;
		dtd->mVBlanking = 45;
		dtd->mHSyncOffset = 88;
		dtd->mVSyncOffset = 4;
		dtd->mHSyncPulseWidth = 44;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = (refreshRate == 29.970) ? 74.176 : 74.250;
		break;
	case 35:		/* 2880x480p @ 60Hz 4:3 */
		dtd->mHImageSize = 4;
		dtd->mVImageSize = 3;
	case 36:		/* 2880x480p @ 60Hz 16:9 */
		dtd->mHActive = 2880;
		dtd->mVActive = 480;
		dtd->mHBlanking = 552;
		dtd->mVBlanking = 45;
		dtd->mHSyncOffset = 64;
		dtd->mVSyncOffset = 9;
		dtd->mHSyncPulseWidth = 248;
		dtd->mVSyncPulseWidth = 6;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 0;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = (refreshRate == 59.940) ? 108.00 : 108.108;
		break;
	case 37:		/* 2880x576p @ 50Hz 4:3 */
		dtd->mHImageSize = 4;
		dtd->mVImageSize = 3;
	case 38:		/* 2880x576p @ 50Hz 16:9 */
		dtd->mHActive = 2880;
		dtd->mVActive = 576;
		dtd->mHBlanking = 576;
		dtd->mVBlanking = 49;
		dtd->mHSyncOffset = 48;
		dtd->mVSyncOffset = 5;
		dtd->mHSyncPulseWidth = 256;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 0;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 108.00;
		break;
	case 39:		/* 1920x1080i (1250 total) @ 50Hz 16:9 */
		dtd->mHActive = 1920;
		dtd->mVActive = 540;
		dtd->mHBlanking = 384;
		dtd->mVBlanking = 85;
		dtd->mHSyncOffset = 32;
		dtd->mVSyncOffset = 23;
		dtd->mHSyncPulseWidth = 168;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = 1;
		dtd->mVSyncPolarity = 0;
		dtd->mInterlaced = 1;
		dtd->mPixelClock = 72.00;
		break;
	case 40:		/* 1920x1080i @ 100Hz 16:9 */
		dtd->mHActive = 1920;
		dtd->mVActive = 540;
		dtd->mHBlanking = 720;
		dtd->mVBlanking = 22;
		dtd->mHSyncOffset = 528;
		dtd->mVSyncOffset = 2;
		dtd->mHSyncPulseWidth = 44;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 1;
		dtd->mPixelClock = 148.50;
		break;
	case 41:		/* 1280x720p @ 100Hz 16:9 */
		dtd->mHActive = 1280;
		dtd->mVActive = 720;
		dtd->mHBlanking = 700;
		dtd->mVBlanking = 30;
		dtd->mHSyncOffset = 440;
		dtd->mVSyncOffset = 5;
		dtd->mHSyncPulseWidth = 40;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 148.50;
		break;
	case 42:		/* 720x576p @ 100Hz 4:3 */
		dtd->mHImageSize = 4;
		dtd->mVImageSize = 3;
	case 43:		/* 720x576p @ 100Hz 16:9 */
		dtd->mHActive = 720;
		dtd->mVActive = 576;
		dtd->mHBlanking = 144;
		dtd->mVBlanking = 49;
		dtd->mHSyncOffset = 12;
		dtd->mVSyncOffset = 5;
		dtd->mHSyncPulseWidth = 64;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 0;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 54.00;
		break;
	case 44:		/* 720(1440)x576i @ 100Hz 4:3 */
		dtd->mHImageSize = 4;
		dtd->mVImageSize = 3;
	case 45:		/* 720(1440)x576i @ 100Hz 16:9 */
		dtd->mHActive = 1440;
		dtd->mVActive = 288;
		dtd->mHBlanking = 288;
		dtd->mVBlanking = 24;
		dtd->mHSyncOffset = 24;
		dtd->mVSyncOffset = 2;
		dtd->mHSyncPulseWidth = 126;
		dtd->mVSyncPulseWidth = 3;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 0;
		dtd->mInterlaced = 1;
		dtd->mPixelClock = 54.00;
		dtd->mPixelRepetitionInput = 1;
		break;
	case 46:		/* 1920x1080i @ 119.88/120Hz 16:9 */
		dtd->mHActive = 1920;
		dtd->mVActive = 540;
		dtd->mHBlanking = 280;
		dtd->mVBlanking = 22;
		dtd->mHSyncOffset = 88;
		dtd->mVSyncOffset = 2;
		dtd->mHSyncPulseWidth = 44;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 1;
		dtd->mPixelClock = (refreshRate == 119.880) ? 148.352 : 148.50;
		break;
	case 47:		/* 1280x720p @ 119.88/120Hz 16:9 */
		dtd->mHActive = 1280;
		dtd->mVActive = 720;
		dtd->mHBlanking = 370;
		dtd->mVBlanking = 30;
		dtd->mHSyncOffset = 110;
		dtd->mVSyncOffset = 5;
		dtd->mHSyncPulseWidth = 40;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = (refreshRate == 119.880) ? 148.352 : 148.50;
		break;
	case 48:		/* 720x480p @ 119.88/120Hz 4:3 */
		dtd->mHImageSize = 4;
		dtd->mVImageSize = 3;
	case 49:		/* 720x480p @ 119.88/120Hz 16:9 */
		dtd->mHActive = 720;
		dtd->mVActive = 480;
		dtd->mHBlanking = 138;
		dtd->mVBlanking = 45;
		dtd->mHSyncOffset = 16;
		dtd->mVSyncOffset = 9;
		dtd->mHSyncPulseWidth = 62;
		dtd->mVSyncPulseWidth = 6;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 0;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = (refreshRate == 119.880) ? 54.00 : 54.054;
		break;
	case 50:		/* 720(1440)x480i @ 119.88/120Hz 4:3 */
		dtd->mHImageSize = 4;
		dtd->mVImageSize = 3;
	case 51:		/* 720(1440)x480i @ 119.88/120Hz 16:9 */
		dtd->mHActive = 1440;
		dtd->mVActive = 240;
		dtd->mHBlanking = 276;
		dtd->mVBlanking = 22;
		dtd->mHSyncOffset = 38;
		dtd->mVSyncOffset = 4;
		dtd->mHSyncPulseWidth = 124;
		dtd->mVSyncPulseWidth = 3;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 0;
		dtd->mInterlaced = 1;
		dtd->mPixelClock = (refreshRate == 119.880) ? 54.00 : 54.054;
		dtd->mPixelRepetitionInput = 1;
		break;
	case 52:		/* 720X576p @ 200Hz 4:3 */
		dtd->mHImageSize = 4;
		dtd->mVImageSize = 3;
	case 53:		/* 720X576p @ 200Hz 16:9 */
		dtd->mHActive = 720;
		dtd->mVActive = 576;
		dtd->mHBlanking = 144;
		dtd->mVBlanking = 49;
		dtd->mHSyncOffset = 12;
		dtd->mVSyncOffset = 5;
		dtd->mHSyncPulseWidth = 64;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 0;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 108.00;
		break;
	case 54:		/* 720(1440)x576i @ 200Hz 4:3 */
		dtd->mHImageSize = 4;
		dtd->mVImageSize = 3;
	case 55:		/* 720(1440)x576i @ 200Hz 16:9 */
		dtd->mHActive = 1440;
		dtd->mVActive = 288;
		dtd->mHBlanking = 288;
		dtd->mVBlanking = 24;
		dtd->mHSyncOffset = 24;
		dtd->mVSyncOffset = 2;
		dtd->mHSyncPulseWidth = 126;
		dtd->mVSyncPulseWidth = 3;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 0;
		dtd->mInterlaced = 1;
		dtd->mPixelClock = 108.00;
		dtd->mPixelRepetitionInput = 1;
		break;
	case 56:		/* 720x480p @ 239.76/240Hz 4:3 */
		dtd->mHImageSize = 4;
		dtd->mVImageSize = 3;
	case 57:		/* 720x480p @ 239.76/240Hz 16:9 */
		dtd->mHActive = 720;
		dtd->mVActive = 480;
		dtd->mHBlanking = 138;
		dtd->mVBlanking = 45;
		dtd->mHSyncOffset = 16;
		dtd->mVSyncOffset = 9;
		dtd->mHSyncPulseWidth = 62;
		dtd->mVSyncPulseWidth = 6;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 0;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = (refreshRate == 239.760) ? 108.00 : 108.108;
		break;
	case 58:		/* 720(1440)x480i @ 239.76/240Hz 4:3 */
		dtd->mHImageSize = 4;
		dtd->mVImageSize = 3;
	case 59:		/* 720(1440)x480i @ 239.76/240Hz 16:9 */
		dtd->mHActive = 1440;
		dtd->mVActive = 240;
		dtd->mHBlanking = 276;
		dtd->mVBlanking = 22;
		dtd->mHSyncOffset = 38;
		dtd->mVSyncOffset = 4;
		dtd->mHSyncPulseWidth = 124;
		dtd->mVSyncPulseWidth = 3;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 0;
		dtd->mInterlaced = 1;
		dtd->mPixelClock = (refreshRate == 239.760) ? 108.00 : 108.108;
		dtd->mPixelRepetitionInput = 1;
		break;
	case 60:		/* 1280x720p @ 23.97/24Hz 16:9 */
		dtd->mHActive = 1280;
		dtd->mVActive = 720;
		dtd->mHBlanking = 2020;
		dtd->mVBlanking = 30;
		dtd->mHSyncOffset = 1760;
		dtd->mVSyncOffset = 5;
		dtd->mHSyncPulseWidth = 40;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = (refreshRate == 23.970) ? 59.341 : 59.40;
		break;
	case 61:		/* 1280x720p @ 25Hz 16:9 */
		dtd->mHActive = 1280;
		dtd->mVActive = 720;
		dtd->mHBlanking = 2680;
		dtd->mVBlanking = 30;
		dtd->mHSyncOffset = 2420;
		dtd->mVSyncOffset = 5;
		dtd->mHSyncPulseWidth = 40;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 74.25;
		break;
	case 62:		/* 1280x720p @ 29.97/30Hz  16:9 */
		dtd->mHActive = 1280;
		dtd->mVActive = 720;
		dtd->mHBlanking = 2020;
		dtd->mVBlanking = 30;
		dtd->mHSyncOffset = 1760;
		dtd->mVSyncOffset = 5;
		dtd->mHSyncPulseWidth = 40;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = (refreshRate == 29.970) ? 74.176 : 74.25;
		break;
	case 63:		/* 1920x1080p @ 119.88/120Hz 16:9 */
		dtd->mHActive = 1920;
		dtd->mVActive = 1080;
		dtd->mHBlanking = 280;
		dtd->mVBlanking = 45;
		dtd->mHSyncOffset = 88;
		dtd->mVSyncOffset = 4;
		dtd->mHSyncPulseWidth = 44;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = (refreshRate == 119.880) ? 296.703 : 297.00;
		break;
	case 64:		/* 1920x1080p @ 100Hz 16:9 */
		dtd->mHActive = 1920;
		dtd->mVActive = 1080;
		dtd->mHBlanking = 720;
		dtd->mVBlanking = 45;
		dtd->mHSyncOffset = 528;
		dtd->mVSyncOffset = 4;
		dtd->mHSyncPulseWidth = 44;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 297.00;
		break;
	case 68:
		dtd->mHActive = 1280;
		dtd->mVActive = 720;
		dtd->mHBlanking = 700;
		dtd->mVBlanking = 30;
		dtd->mHSyncOffset = 440;
		dtd->mVSyncOffset = 5;
		dtd->mHSyncPulseWidth = 40;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 74.25;
		break;
	case 69:		/* 1280x720p @ 60Hz 21:9 */
		dtd->mHActive = 1280;
		dtd->mVActive = 720;
		dtd->mHBlanking = 370;
		dtd->mVBlanking = 30;
		dtd->mHSyncOffset = 110;
		dtd->mVSyncOffset = 5;
		dtd->mHSyncPulseWidth = 40;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 74.25;
		break;
	case 75:		//1080p
		dtd->mHActive = 1920;
		dtd->mVActive = 1080;
		dtd->mHBlanking = 720;
		dtd->mVBlanking = 45;
		dtd->mHSyncOffset = 528;
		dtd->mVSyncOffset = 4;
		dtd->mHSyncPulseWidth = 44;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 148.50;
		break;
	case 76:		//1080p
		dtd->mHActive = 1920;
		dtd->mVActive = 1080;
		dtd->mHBlanking = 280;
		dtd->mVBlanking = 45;
		dtd->mHSyncOffset = 88;
		dtd->mVSyncOffset = 4;
		dtd->mHSyncPulseWidth = 44;
		dtd->mVSyncPulseWidth = 5;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 148.50;
		break;
	case 93:		/* 4k x 2k, 30Hz, HDMI_VIC 3 */
		dtd->mCode = 93;
		dtd->mHActive = 3840;
		dtd->mVActive = 2160;
		dtd->mHBlanking = 1660;
		dtd->mVBlanking = 90;
		dtd->mHSyncOffset = 1276;
		dtd->mVSyncOffset = 8;
		dtd->mHSyncPulseWidth = 88;
		dtd->mVSyncPulseWidth = 10;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 297.00;	//(refreshRate == 59940) ? 14835 : 14850;
		break;
	case 94:		/* 4k x 2k, 30Hz, HDMI_VIC 2 */
		dtd->mCode = 94;
		dtd->mHActive = 3840;
		dtd->mVActive = 2160;
		dtd->mHBlanking = 1440;
		dtd->mVBlanking = 90;
		dtd->mHSyncOffset = 1056;
		dtd->mVSyncOffset = 8;
		dtd->mHSyncPulseWidth = 88;
		dtd->mVSyncPulseWidth = 10;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 297.00;	//(refreshRate == 59940) ? 14835 : 14850;
		break;

	case 95:		/* 4k x 2k, HDMI_VIC 1 */
		dtd->mCode = 95;
		dtd->mHActive = 3840;
		dtd->mVActive = 2160;
		dtd->mHBlanking = 560;
		dtd->mVBlanking = 90;
		dtd->mHSyncOffset = 176;
		dtd->mVSyncOffset = 8;
		dtd->mHSyncPulseWidth = 88;
		dtd->mVSyncPulseWidth = 10;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 297.00;	//(refreshRate == 59940) ? 14835 : 14850;
		break;
	case 96:		/* 4k x 2k, HDMI_VIC 2 */
		dtd->mCode = 96;
		dtd->mHActive = 3840;
		dtd->mVActive = 2160;
		dtd->mHBlanking = 1440;
		dtd->mVBlanking = 90;
		dtd->mHSyncOffset = 1056;
		dtd->mVSyncOffset = 8;
		dtd->mHSyncPulseWidth = 88;
		dtd->mVSyncPulseWidth = 10;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 297.00;	//(refreshRate == 59940) ? 14835 : 14850;
		break;
	case 97:		/* 4k x 2k, HDMI_VIC 1 */
		dtd->mCode = 97;
		dtd->mHActive = 3840;
		dtd->mVActive = 2160;
		dtd->mHBlanking = 560;
		dtd->mVBlanking = 90;
		dtd->mHSyncOffset = 176;
		dtd->mVSyncOffset = 8;
		dtd->mHSyncPulseWidth = 88;
		dtd->mVSyncPulseWidth = 10;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 297.00;	//(refreshRate == 59940) ? 14835 : 14850;
		break;
	case 98:		/* 4k x 2k, HDMI_VIC 1 */
		dtd->mCode = 98;
		dtd->mHActive = 4096;
		dtd->mVActive = 2160;
		dtd->mHBlanking = 1404;
		dtd->mVBlanking = 90;
		dtd->mHSyncOffset = 1020;
		dtd->mVSyncOffset = 8;
		dtd->mHSyncPulseWidth = 88;
		dtd->mVSyncPulseWidth = 10;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 297.00;	//(refreshRate == 59940) ? 14835 : 14850;
		break;
	case 102:                 //4096x2160  60hz
		dtd->mCode = 102;
		dtd->mHActive = 4096;
		dtd->mVActive = 2160;
		dtd->mHBlanking = 304;
		dtd->mVBlanking = 90;
		dtd->mHSyncOffset = 88;
		dtd->mVSyncOffset = 8;
		dtd->mHSyncPulseWidth = 88;
		dtd->mVSyncPulseWidth = 10;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 594.00;	
		break;
	case 103:
		dtd->mCode = 103;
		dtd->mHActive = 3840;
		dtd->mVActive = 2160;
		dtd->mHBlanking = 1660;
		dtd->mVBlanking = 90;
		dtd->mHSyncOffset = 1276;
		dtd->mVSyncOffset = 8;
		dtd->mHSyncPulseWidth = 88;
		dtd->mVSyncPulseWidth = 10;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 297.00;	//(refreshRate == 59940) ? 14835 : 14850;
		break;
	case 104:
		dtd->mCode = 104;
		dtd->mHActive = 3840;
		dtd->mVActive = 2160;
		dtd->mHBlanking = 1440;
		dtd->mVBlanking = 90;
		dtd->mHSyncOffset = 1056;
		dtd->mVSyncOffset = 8;
		dtd->mHSyncPulseWidth = 88;
		dtd->mVSyncPulseWidth = 10;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 297.00;	//(refreshRate == 59940) ? 14835 : 14850;
		break;
	case 105:
		dtd->mCode = 105;
		dtd->mHActive = 3840;
		dtd->mVActive = 2160;
		dtd->mHBlanking = 560;
		dtd->mVBlanking = 90;
		dtd->mHSyncOffset = 176;
		dtd->mVSyncOffset = 8;
		dtd->mHSyncPulseWidth = 88;
		dtd->mVSyncPulseWidth = 10;
		dtd->mHSyncPolarity = dtd->mVSyncPolarity = 1;
		dtd->mInterlaced = 0;
		dtd->mPixelClock = 297.00;	//(refreshRate == 59940) ? 14835 : 14850;
		break;
	default:
		dtd->mCode = -1;
		LOGGER(SNPS_ERROR,"invalid code");
		return FALSE;
	}
	return TRUE;
}

u8 dtd_GetCode(hdmi_tx_dev_t *dev, const dtd_t * dtd)
{
	return dtd->mCode;
}

u16 dtd_GetPixelRepetitionInput(hdmi_tx_dev_t *dev, const dtd_t * dtd)
{
	return dtd->mPixelRepetitionInput;
}

double dtd_GetPixelClock(hdmi_tx_dev_t *dev, const dtd_t * dtd)
{
	return dtd->mPixelClock;
}

u8 dtd_GetInterlaced(hdmi_tx_dev_t *dev, const dtd_t * dtd)
{
	return dtd->mInterlaced;
}

u16 dtd_GetHActive(hdmi_tx_dev_t *dev, const dtd_t * dtd)
{
	return dtd->mHActive;
}

u16 dtd_GetHBlanking(hdmi_tx_dev_t *dev, const dtd_t * dtd)
{
	return dtd->mHBlanking;
}

u16 dtd_GetHBorder(hdmi_tx_dev_t *dev, const dtd_t * dtd)
{
	return dtd->mHBorder;
}

u16 dtd_GetHImageSize(hdmi_tx_dev_t *dev, const dtd_t * dtd)
{
	return dtd->mHImageSize;
}

u16 dtd_GetHSyncOffset(hdmi_tx_dev_t *dev, const dtd_t * dtd)
{
	return dtd->mHSyncOffset;
}

u8 dtd_GetHSyncPolarity(hdmi_tx_dev_t *dev, const dtd_t * dtd)
{
	return dtd->mHSyncPolarity;
}

u16 dtd_GetHSyncPulseWidth(hdmi_tx_dev_t *dev, const dtd_t * dtd)
{
	return dtd->mHSyncPulseWidth;
}

u16 dtd_GetVActive(hdmi_tx_dev_t *dev, const dtd_t * dtd)
{
	return dtd->mVActive;
}

u16 dtd_GetVBlanking(hdmi_tx_dev_t *dev, const dtd_t * dtd)
{
	return dtd->mVBlanking;
}

u16 dtd_GetVBorder(hdmi_tx_dev_t *dev, const dtd_t * dtd)
{
	return dtd->mVBorder;
}

u16 dtd_GetVImageSize(hdmi_tx_dev_t *dev, const dtd_t * dtd)
{
	return dtd->mVImageSize;
}

u16 dtd_GetVSyncOffset(hdmi_tx_dev_t *dev, const dtd_t * dtd)
{
	return dtd->mVSyncOffset;
}

u8 dtd_GetVSyncPolarity(hdmi_tx_dev_t *dev, const dtd_t * dtd)
{
	return dtd->mVSyncPolarity;
}

u16 dtd_GetVSyncPulseWidth(hdmi_tx_dev_t *dev, const dtd_t * dtd)
{
	return dtd->mVSyncPulseWidth;
}

int dtd_IsEqual(hdmi_tx_dev_t *dev, const dtd_t * dtd1, const dtd_t * dtd2)
{
	return (dtd1->mInterlaced == dtd2->mInterlaced && dtd1->mHActive
		== dtd2->mHActive && dtd1->mHBlanking == dtd2->mHBlanking
		&& dtd1->mHBorder == dtd2->mHBorder && dtd1->mHSyncOffset
		== dtd2->mHSyncOffset && dtd1->mHSyncPolarity
		== dtd2->mHSyncPolarity && dtd1->mHSyncPulseWidth
		== dtd2->mHSyncPulseWidth && dtd1->mVActive == dtd2->mVActive
		&& dtd1->mVBlanking == dtd2->mVBlanking && dtd1->mVBorder
		== dtd2->mVBorder && dtd1->mVSyncOffset == dtd2->mVSyncOffset
		&& dtd1->mVSyncPolarity == dtd2->mVSyncPolarity
		&& dtd1->mVSyncPulseWidth == dtd2->mVSyncPulseWidth
		&& ((dtd1->mHImageSize * 10) / dtd1->mVImageSize)
		== ((dtd2->mHImageSize * 10) / dtd2->mVImageSize));
}

int dtd_SetPixelRepetitionInput(hdmi_tx_dev_t *dev, dtd_t * dtd, u16 value)
{
	

	switch (dtd->mCode) {
	case (u8) - 1:
	case 10:
	case 11:
	case 12:
	case 13:
	case 25:
	case 26:
	case 27:
	case 28:
		if (value < 10) {
			dtd->mPixelRepetitionInput = value;
			return TRUE;
		}
		break;
	case 14:
	case 15:
	case 29:
	case 30:
		if (value < 2) {
			dtd->mPixelRepetitionInput = value;
			return TRUE;
		}
		break;
	case 35:
	case 36:
	case 37:
	case 38:
		if (value < 2 || value == 3) {
			dtd->mPixelRepetitionInput = value;
			return TRUE;
		}
		break;
	default:
		if (value == dtd->mPixelRepetitionInput) {
			return TRUE;
		}
		break;
	}
	LOGGER(SNPS_ERROR,"Invalid pixel repetition input for video mode %d 0x%x", value,
		   dtd->mCode);
	return FALSE;
}

int dtd_IsLimitedToYcc420(hdmi_tx_dev_t *dev, dtd_t * dtd)
{
	return dtd->mLimitedToYcc420;

}

void dtd_change_horiz_for_ycc420(hdmi_tx_dev_t *dev, dtd_t * tempDtd)
{
	tempDtd->mHActive = tempDtd->mHActive / 2;
	tempDtd->mHBlanking = tempDtd->mHBlanking / 2;
	tempDtd->mHSyncOffset = tempDtd->mHSyncOffset / 2;
	tempDtd->mHSyncPulseWidth = tempDtd->mHSyncPulseWidth / 2;
}

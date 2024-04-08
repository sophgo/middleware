/*
 * dtd.h
 *
 *  Created on: Jul 5, 2010
 *
 *  Synopsys Inc.
 *  SG DWC PT02 
 */

#ifndef DTD_H_
#define DTD_H_

#include "includes.h"
/**
 * @file
 * For detailed handling of this structure, refer to documentation of the functions
 */
typedef struct {
	/** VIC code */
	u32 mCode;
	/** Identifies modes that ONLY can be displayed in YCC 4:2:0 */
	u8 mLimitedToYcc420;
	/** Identifies modes that can also be displayed in YCC 4:2:0 */
	u8 mYcc420;

	u16 mPixelRepetitionInput;
	/** in units of 10KHz */
	double mPixelClock;
	/** 1 for interlaced, 0 progressive */
	u8 mInterlaced;

	u16 mHActive;

	u16 mHBlanking;

	u16 mHBorder;

	u16 mHImageSize;

	u16 mHSyncOffset;

	u16 mHSyncPulseWidth;
	/** 0 for Active low, 1 active high */
	u8 mHSyncPolarity;

	u16 mVActive;

	u16 mVBlanking;

	u16 mVBorder;

	u16 mVImageSize;

	u16 mVSyncOffset;

	u16 mVSyncPulseWidth;
	/** 0 for Active low, 1 active high */
	u8 mVSyncPolarity;

} dtd_t;
/**
 * Parses the Detailed Timing Descriptor.
 * Encapsulating the parsing process
 * @param dtd pointer to dtd_t strucutute for the information to be save in
 * @param data a pointer to the 18-byte structure to be parsed.
 * @return TRUE if success
 */
int dtd_parse(hdmi_tx_dev_t *dev, dtd_t * dtd, u8 data[18]);
/**
 * @param dtd pointer to dtd_t strucutute for the information to be save in
 * @param code the CEA 861-D video code.
 * @param refreshRate the specified vertical refresh rate.
 * @return TRUE if success
 */
int dtd_fill(hdmi_tx_dev_t *dev, dtd_t * dtd, u8 code, double refreshRate);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the CEA861-D code if the DTD is listed in the spec
 * @return < 0 when the code has an undefined DTD
 */
u8 dtd_GetCode(hdmi_tx_dev_t *dev, const dtd_t * dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the input pixel reptition
 */
u16 dtd_GetPixelRepetitionInput(hdmi_tx_dev_t *dev, const dtd_t * dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the pixel clock rate of the DTD
 */
double dtd_GetPixelClock(hdmi_tx_dev_t *dev, const dtd_t * dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return 1 if the DTD is of an interlaced viedo format
 */
u8 dtd_GetInterlaced(hdmi_tx_dev_t *dev, const dtd_t * dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the horizontal active pixels (addressable video)
 */
u16 dtd_GetHActive(hdmi_tx_dev_t *dev, const dtd_t * dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the horizontal blanking pixels
 */
u16 dtd_GetHBlanking(hdmi_tx_dev_t *dev, const dtd_t * dtd);
/**
 * @param dtd p( inter to dtd_t strucutute where the information is held
 * @return the horizontal border in pixels
 */
u16 dtd_GetHBorder(hdmi_tx_dev_t *dev, const dtd_t * dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the horizontal image size in mm
 */
u16 dtd_GetHImageSize(hdmi_tx_dev_t *dev, const dtd_t * dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the horizontal sync offset (front porch) from blanking start to start of sync in pixels
 */
u16 dtd_GetHSyncOffset(hdmi_tx_dev_t *dev, const dtd_t * dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the horizontal sync polarity
 */
u8 dtd_GetHSyncPolarity(hdmi_tx_dev_t *dev, const dtd_t * dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the horizontal sync pulse width in pixels
 */
u16 dtd_GetHSyncPulseWidth(hdmi_tx_dev_t *dev, const dtd_t * dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the vertical active pixels (addressable video)
 */
u16 dtd_GetVActive(hdmi_tx_dev_t *dev, const dtd_t * dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the vertical border in pixels
 */
u16 dtd_GetVBlanking(hdmi_tx_dev_t *dev, const dtd_t * dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the horizontal image size
 */
u16 dtd_GetVBorder(hdmi_tx_dev_t *dev, const dtd_t * dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the vertical image size in mm
 */
u16 dtd_GetVImageSize(hdmi_tx_dev_t *dev, const dtd_t * dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the vertical sync offset (front porch) from blanking start to start
 *  of sync in lines
 */
u16 dtd_GetVSyncOffset(hdmi_tx_dev_t *dev, const dtd_t * dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the vertical sync polarity
 */
u8 dtd_GetVSyncPolarity(hdmi_tx_dev_t *dev, const dtd_t * dtd);
/**
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @return the vertical sync pulse width in line
 */
u16 dtd_GetVSyncPulseWidth(hdmi_tx_dev_t *dev, const dtd_t * dtd);
/**
 * @param dtd1 pointer to dtd_t structure to be compared
 * @param dtd2 pointer to dtd_t structure to be compared to
 * @return TRUE if the two DTDs are identical
 * @note: although DTDs may have different refresh rates, and hence
 * pixel clocks, they can still be identical
 */
int dtd_IsEqual(hdmi_tx_dev_t *dev, const dtd_t * dtd1, const dtd_t * dtd2);
/**
 * Set the desired pixel repetition
 * @param dtd pointer to dtd_t strucutute where the information is held
 * @param value of pixel repetitions
 * @return TRUE if successful
 * @note for CEA video modes, the value has to fall within the
 * defined range, otherwise the method will fail.
 */
int dtd_SetPixelRepetitionInput(hdmi_tx_dev_t *dev, dtd_t * dtd, u16 value);

int dtd_IsLimitedToYcc420(hdmi_tx_dev_t *dev, dtd_t * dtd);

void dtd_change_horiz_for_ycc420(hdmi_tx_dev_t *dev, dtd_t * tempDtd);

#endif				/* DTD_H_ */

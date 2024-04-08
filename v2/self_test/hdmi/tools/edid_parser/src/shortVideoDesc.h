/*
 * short_video_desc.h
 * 
 *  Synopsys Inc.
 *  SG DWC PT02
 */

#ifndef SHORTVIDEODESC_H_
#define SHORTVIDEODESC_H_

#include "includes.h"
#include "bit_operation.h"
/**
 * @file
 * Short Video Descriptor.
 * Parse and hold Short Video Descriptors found in Video Data Block in EDID.
 */
/** For detailed handling of this structure, refer to documentation of the functions */
typedef struct {
	int mNative;

	unsigned mCode;

	unsigned mLimitedToYcc420;

	unsigned mYcc420;

} shortVideoDesc_t;

void shortVideoDesc_Reset(hdmi_tx_dev_t *dev, shortVideoDesc_t * svd);

int svd_parse(hdmi_tx_dev_t *dev, shortVideoDesc_t * svd, u8 data);
/**
 * @return the video code (VIC) defined in CEA-861-(D or later versions)
 */
unsigned shortVideoDesc_GetCode(hdmi_tx_dev_t *dev, shortVideoDesc_t * svd);

/**
 * @return TRUE if video mode is native to sink
 */
int shortVideoDesc_GetNative(hdmi_tx_dev_t *dev, shortVideoDesc_t * svd);

#endif				/* SHORTVIDEODESC_H_ */

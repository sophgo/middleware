/*
 * edid.h
 *
 *  Created on: Jul 23, 2010
 *
 * Synopsys Inc.
 * SG DWC PT02
 */
/**
 * @file
 * E-EDID reader and parser
 * Initiates and handles I2C communications to read E-EDID then
 * parses read blocks and retains the information in corresponding
 * data structures
 */

#ifndef EDID_H_
#define EDID_H_

#include "dtd.h"
#include "short_video_desc.h"
#include "short_audio_desc.h"
#include "hdmivsdb.h"
#include "hdmiforumvsdb.h"
#include "monitor_range_limits.h"
#include "video_cap_data_block.h"
#include "colorimetry_data_block.h"
#include "speaker_alloc_data_block.h"
#include "includes.h"

typedef struct {

	/**
	 * Array to hold all the parsed Detailed Timing Descriptors.
	 */
	dtd_t edid_mDtd[32];

	unsigned int edid_mDtdIndex;
	/**
	 * array to hold all the parsed Short Video Descriptors.
	 */
	shortVideoDesc_t edid_mSvd[128];  //TODO: too big for stack

	shortVideoDesc_t tmpSvd;

	unsigned int edid_mSvdIndex;
	/**
	 * array to hold all the parsed Short Audio Descriptors.
	 */
	shortAudioDesc_t edid_mSad[128]; //TODO: too big for stack

	unsigned int edid_mSadIndex;
#if 1
	/**
	 * A string to hold the Monitor Name parsed from EDID.
	 */
	char edid_mMonitorName[13];

	int edid_mYcc444Support;

	int edid_mYcc422Support;

	int edid_mYcc420Support;

	int edid_mBasicAudioSupport;

	int edid_mUnderscanSupport;
#endif
	/**
	 *  If Sink is HDMI 2.0
	 */
	int edid_m20Sink;

	hdmivsdb_t edid_mHdmivsdb;

	hdmiforumvsdb_t edid_mHdmiForumvsdb;

	monitorRangeLimits_t edid_mMonitorRangeLimits;

	videoCapabilityDataBlock_t edid_mVideoCapabilityDataBlock;

	colorimetryDataBlock_t edid_mColorimetryDataBlock;

	speakerAllocationDataBlock_t edid_mSpeakerAllocationDataBlock;
} edidCeaExt_t;


/**
 * Initialise the E-EDID reader
 * reset all internal variables
 * reset reader state
 * prepare I2C master
 * commence reading E-EDID memory from sink
 * @param baseAddr base address of controller
 * @param sfrClock external clock supplied to controller
 * @note: this version only works with 25MHz
 */
int edid_Initialize(hdmi_tx_dev_t *dev, u16 sfrClock);
/**
 * Reset local variables and clear information from memory to avoid confusion with any newly read EDID structure
 * @param baseAddr base address of controller
 */
#if 0
int edid_Standby(hdmi_tx_dev_t *dev);

/**
 * The method handles DONE and ERROR events.
 * A DONE event will trigger the retrieving the read byte, and sending a request to read the following byte. 
 * The EDID is read until the block is done and then the reading moves to the next block.
 *  When the block is successfully read, it is sent to be parsed.
 * @param baseAddr base address of controller
 * @param hpd state of hot plug (whether virtual or physical)
 * @param state of the EDID reader interrupts
 * @return edid_status_t status of the reading statemachine 
 */
u8 edid_EventHandler(hdmi_tx_dev_t *dev, int hpd, u8 state);

// TODO: comment this correctly
int is_edid_done(void);

/**
 * Get the number of DTDs read and parsed from EDID structure
 * @param baseAddr base address of controller
 * @return the number of DTDs read and parsed from EDID structure
 */
unsigned int edid_GetDtdCount(hdmi_tx_dev_t *dev);
/**
 * Get a Detailed Timing Descriptors data type read and parsed from EDID
 * structure.
 * @param baseAddr base address of controller
 * @param n index of the DTD in the sinks EDID
 * @param dtd pointer to a dtd_t structure to hold the information
 * @return TRUE if a DTD is found at specified index
 */
int edid_GetDtd(hdmi_tx_dev_t *dev, unsigned int n, dtd_t * dtd);
/**
 * Get a HDMI VSDB data type read and parsed from EDID structure.
 * @param baseAddr base address of controller
 * @param vsdb pointer to a hdmivsdb_t structure to hold the information
 * @return TRUE if an HDMI VSDB is found (ie. if sink is HDMI)
 */
int edid_GetHdmivsdb(hdmi_tx_dev_t *dev, hdmivsdb_t * vsdb);

int edid_GetHdmiForumvsdb(hdmi_tx_dev_t *dev, hdmiforumvsdb_t * forumvsdb);
/**
 * Get the monitor name parsed from the E-EDID strucutre at sink
 * @param baseAddr base address of controller
 * @param name pointer to the allocated memory of char type to hold the name
 * @param length of the memory (no of charachters the memory can hold)
 * @return number of copied characters
 */
int edid_GetMonitorName(hdmi_tx_dev_t *dev, char *name, unsigned int length);
/**
 * Get the monitor range limits parsed from the E-EDID strucutre at sink
 * @param baseAddr base address of controller
 * @param limits pointer to structure of type monitorRangeLimits_t to hold 
 * information 
 * @return TRUE if a monitor range limit is found
 */
int edid_GetMonitorRangeLimits(hdmi_tx_dev_t *dev, monitorRangeLimits_t * limits);

unsigned int edid_GetHdmiVicCount(hdmi_tx_dev_t *dev, hdmivsdb_t * vsdb);
/**
 * @param baseAddr base address of controller
 * @return the number of all Short Video Descriptors read and parsed from EDID structure
 */
unsigned int edid_GetSvdCount(hdmi_tx_dev_t *dev);
/**
 * Get a Short Video Descriptor data type read and parsed from EDID structure.
 * @param baseAddr base address of controller
 * @param n index of the SVD in the sinks EDID
 * @param svd pointer to a Short Video Descriptor structure to hold the information
 * @return TRUE if a Short Video Descriptor is found at specified index
 */
int edid_GetSvd(hdmi_tx_dev_t *dev, unsigned int n, shortVideoDesc_t * svd);
/**
 * @param baseAddr base address of controller
 * @return the number of all Short Audio Descriptors read and parsed from EDID structure
 */
unsigned int edid_GetSadCount(hdmi_tx_dev_t *dev);
/**
 * Get a Short Audio Descriptor data type read and parsed from EDID structure.
 * @param baseAddr base address of controller
 * @param n index of the SAD in the sinks EDID
 * @param sad pointer to a Short Audio Descriptor structure to hold the information
 * @return TRUE if a Short Audio Descriptor is found at specified index
 */
int edid_GetSad(hdmi_tx_dev_t *dev, unsigned int n, shortAudioDesc_t * sad);
/**
 * Get the video capability data block parsed from the E-EDID strucutre at sink
 * @param baseAddr base address of controller
 * @param capability pointer to structure of type videoCapabilityDataBlock_t to 
 * hold information 
 * @return TRUE if a video capability data block is found
 */
int
edid_GetVideoCapabilityDataBlock(hdmi_tx_dev_t *dev,
				 videoCapabilityDataBlock_t * capability);
/**
 * Get the speaker allocation data block parsed from the E-EDID strucutre at 
 * sink
 * @param baseAddr base address of controller
 * @param allocation pointer to structure of type speakerAllocationDataBlock_t
 *  to hold information 
 * @return TRUE if a speaker allocation data block is found
 */
int
edid_GetSpeakerAllocationDataBlock(hdmi_tx_dev_t *dev,
				   speakerAllocationDataBlock_t * allocation);
/**
 * Get the colorimetry data block parsed from the E-EDID strucutre at 
 * sink
 * @param baseAddr base address of controller
 * @param colorimetry pointer to structure of type colorimetryDataBlock_t
 *  to hold information 
 * @return TRUE if a colorimetry data block is found
 */
int
edid_GetColorimetryDataBlock(hdmi_tx_dev_t *dev,
			     colorimetryDataBlock_t * colorimetry);
/**
 * @param baseAddr base address of controller
 * @return TRUE if sink supports basic audio
 */
int edid_SupportsBasicAudio(hdmi_tx_dev_t *dev);
/**
 * @param baseAddr base address of controller
 * @return TRUE if sink supports underscan
 */
int edid_SupportsUnderscan(hdmi_tx_dev_t *dev);
/**
 * @param baseAddr base address of controller
 * @return TRUE if sink supports YCC:4:2:2
 */
int edid_SupportsYcc422(hdmi_tx_dev_t *dev);
/**
 * @param baseAddr base address of controller
 * @return TRUE if sink supports YCC:4:4:4
 */
int edid_SupportsYcc444(hdmi_tx_dev_t *dev);

u16 edid_GetVicSupported3dStructs(hdmi_tx_dev_t *dev, hdmivsdb_t * vsdb, u8 vic);

int
edid_Get3dStructVics(hdmi_tx_dev_t *dev, hdmivsdb_t * vsdb, u8 struct3d, u8 * vics,
		     unsigned vicsLength);

int
edid_VicSupports3dStruct(hdmi_tx_dev_t *dev, hdmivsdb_t * vsdb, u8 vic, u8 struct3d);

u8 edid_Get3dStructDetail(hdmi_tx_dev_t *dev, hdmivsdb_t * vsdb, u8 vic, u8 struct3d);

int
edid_3dStructHasDetail(hdmi_tx_dev_t *dev, hdmivsdb_t * vsdb, u8 vic, u8 struct3d);

int edid_CheckHdmiVic(hdmi_tx_dev_t *dev, int n, hdmivsdb_t * vsdb);
#endif
/**
 * @param baseAddr base address of controller
 * @return TRUE if successful
 */
int edid_CeaExtReset(hdmi_tx_dev_t *dev, edidCeaExt_t *edidExt);

/**
 * Parses an EDID block of 128 bytes.
 * Each known group (based on location or CEA tag) is parsed using the constructor of its own datatype (class), called in this function accordingly, this function then adds the new valid data structures into the EDID library.
 * @param baseAddr base address of controller
 * @param buffer a pointer to buffer (of 128 bytes)
 * @return TRUE if successful
 */
int edid_ParseBlock(hdmi_tx_dev_t *dev, u8 * buffer);
/**
 * Parse Data Block data structures listed in the CEA Data Block Collection.
 * It identifies the block type and calls the appropriate class constructor to parse it.
 * It adds the new parsed block (object) to the respective vector arrays in the edid object.
 * @param baseAddr base address of controller
 * @param data a buffer array of bytes (pointer to the start of the block).
 * @return the length of the parsed Data Block in the Collection
 */
int edid_ParseDataBlock(hdmi_tx_dev_t *dev, u8 * data, edidCeaExt_t *edidExt);

/* void edid_GetRawEdid(u8 * nrBlocks, u8 * rawEdid); */

int edid_GetHdmiVicCode(hdmi_tx_dev_t *dev, int n, hdmivsdb_t * vsdb);

int edid_IsSink20(hdmi_tx_dev_t *dev);

void edid_printEdid(hdmi_tx_dev_t *dev);

int edid_isSvdLimitedToYcc420(hdmi_tx_dev_t *dev, int edid_svdCode);

int edid_SvdSupportsYcc420(hdmi_tx_dev_t *dev, int edid_svdCode);

#endif				/* EDID_H_ */

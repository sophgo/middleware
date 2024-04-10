/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#ifndef INCLUDE_HDMITX_DEV_H_
#define INCLUDE_HDMITX_DEV_H_

#include "util/types.h"

#include <string.h>

// HDMI Licensing, LLC
#define HDMI_LICENSING_LLC_OUI 0x000C03

/** event_t events to register a callback for in the API
 */
typedef enum {
	MODE_UNDEFINED = -1,
	DVI = 0,
	HDMI
} video_mode_t;

typedef enum {
	PHY_ACCESS_UNDEFINED = 0,
	PHY_I2C = 1,
	PHY_JTAG,
	PHY_EXTERN
} phy_access_t;

typedef enum {
	SNPS_REPORT 	= -6,
	SNPS_EMERG 	= -5,
	SNPS_ALERT 	= -4,
	SNPS_CRITICAL 	= -3,
	SNPS_ERROR 	= -2,
	SNPS_WARN 	= -1,
	SNPS_NOTICE 	=  0,
	SNPS_INFO,
	SNPS_DEBUG,
	SNPS_TRACE,
} log_t;

typedef enum{
	TV = 0,
	RECORDING_DEVICE_1,
	RECORDING_DEVICE_2,
	TUNER_1,
	PLAYBACK_DEVICE_1,
	AUDIO_SUBSYSTEM,
	TUNER_2,
	TUNER_3,
	PLAYBACK_DEVICE_2,
	RECORDING_DEVICE_3,
	TUNER_4,
	PLAYBACK_DEVICE_3,
	RESERVED_1,
	RESERVED_2,
	SPECIFIC_USE,
	UNREGISTERED_BROADCAST

} cec_logical_address_t;

typedef enum{
	TV_DEVICE = 0,
	RECORDING_DEVICE,
	TUNER_DEVICE,
	AUDIO_SUBSYSTEM_DEVICE,
	PLAYBACK_DEVICE,
	SPECIFIC_USE_DEVICE
} cec_device_category_t;

typedef enum{
	CEC_OPCODE_ACTIVE_SOURCE                 = 0x82,
	CEC_OPCODE_IMAGE_VIEW_ON                 = 0x04,
	CEC_OPCODE_TEXT_VIEW_ON                  = 0x0D,
	CEC_OPCODE_INACTIVE_SOURCE               = 0x9D,
	CEC_OPCODE_REQUEST_ACTIVE_SOURCE         = 0x85,
	CEC_OPCODE_ROUTING_CHANGE                = 0x80,
	CEC_OPCODE_ROUTING_INFORMATION           = 0x81,
	CEC_OPCODE_SET_STREAM_PATH               = 0x86,
	CEC_OPCODE_STANDBY                       = 0x36,
	CEC_OPCODE_RECORD_OFF                    = 0x0B,
	CEC_OPCODE_RECORD_ON                     = 0x09,
	CEC_OPCODE_RECORD_STATUS                 = 0x0A,
	CEC_OPCODE_RECORD_TV_SCREEN              = 0x0F,
	CEC_OPCODE_CLEAR_ANALOGUE_TIMER          = 0x33,
	CEC_OPCODE_CLEAR_DIGITAL_TIMER           = 0x99,
	CEC_OPCODE_CLEAR_EXTERNAL_TIMER          = 0xA1,
	CEC_OPCODE_SET_ANALOGUE_TIMER            = 0x34,
	CEC_OPCODE_SET_DIGITAL_TIMER             = 0x97,
	CEC_OPCODE_SET_EXTERNAL_TIMER            = 0xA2,
	CEC_OPCODE_SET_TIMER_PROGRAM_TITLE       = 0x67,
	CEC_OPCODE_TIMER_CLEARED_STATUS          = 0x43,
	CEC_OPCODE_TIMER_STATUS                  = 0x35,
	CEC_OPCODE_CEC_VERSION                   = 0x9E,
	CEC_OPCODE_GET_CEC_VERSION               = 0x9F,
	CEC_OPCODE_GIVE_PHYSICAL_ADDRESS         = 0x83,
	CEC_OPCODE_GET_MENU_LANGUAGE             = 0x91,
	CEC_OPCODE_REPORT_PHYSICAL_ADDRESS       = 0x84,
	CEC_OPCODE_SET_MENU_LANGUAGE             = 0x32,
	CEC_OPCODE_DECK_CONTROL                  = 0x42,
	CEC_OPCODE_DECK_STATUS                   = 0x1B,
	CEC_OPCODE_GIVE_DECK_STATUS              = 0x1A,
	CEC_OPCODE_PLAY                          = 0x41,
	CEC_OPCODE_GIVE_TUNER_DEVICE_STATUS      = 0x08,
	CEC_OPCODE_SELECT_ANALOGUE_SERVICE       = 0x92,
	CEC_OPCODE_SELECT_DIGITAL_SERVICE        = 0x93,
	CEC_OPCODE_TUNER_DEVICE_STATUS           = 0x07,
	CEC_OPCODE_TUNER_STEP_DECREMENT          = 0x06,
	CEC_OPCODE_TUNER_STEP_INCREMENT          = 0x05,
	CEC_OPCODE_DEVICE_VENDOR_ID              = 0x87,
	CEC_OPCODE_GIVE_DEVICE_VENDOR_ID         = 0x8C,
	CEC_OPCODE_VENDOR_COMMAND                = 0x89,
	CEC_OPCODE_VENDOR_COMMAND_WITH_ID        = 0xA0,
	CEC_OPCODE_VENDOR_REMOTE_BUTTON_DOWN     = 0x8A,
	CEC_OPCODE_VENDOR_REMOTE_BUTTON_UP       = 0x8B,
	CEC_OPCODE_SET_OSD_STRING                = 0x64,
	CEC_OPCODE_GIVE_OSD_NAME                 = 0x46,
	CEC_OPCODE_SET_OSD_NAME                  = 0x47,
	CEC_OPCODE_MENU_REQUEST                  = 0x8D,
	CEC_OPCODE_MENU_STATUS                   = 0x8E,
	CEC_OPCODE_USER_CONTROL_PRESSED          = 0x44,
	CEC_OPCODE_USER_CONTROL_RELEASE          = 0x45,
	CEC_OPCODE_GIVE_DEVICE_POWER_STATUS      = 0x8F,
	CEC_OPCODE_REPORT_POWER_STATUS           = 0x90,
	CEC_OPCODE_FEATURE_ABORT                 = 0x00,
	CEC_OPCODE_ABORT                         = 0xFF,
	CEC_OPCODE_GIVE_AUDIO_STATUS             = 0x71,
	CEC_OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS = 0x7D,
	CEC_OPCODE_REPORT_AUDIO_STATUS           = 0x7A,
	CEC_OPCODE_SET_SYSTEM_AUDIO_MODE         = 0x72,
	CEC_OPCODE_SYSTEM_AUDIO_MODE_REQUEST     = 0x70,
	CEC_OPCODE_SYSTEM_AUDIO_MODE_STATUS      = 0x7E,
	CEC_OPCODE_SET_AUDIO_RATE                = 0x9A,

} cec_opcode_t;

typedef enum{
	CEC_NOT_INIT = 0,
	CEC_NO_PHYSICAL_ADDR,
	CEC_POLL_LOGICAL_ADDR,
	CEC_AWAITING_ADDR_ACK,
	CEC_REGISTERED,
	CEC_UNREGISTERED,
	CEC_FOLLOWER,
	CEC_INITIATOR,
} cec_state_t;

typedef enum{
	DECK_INFO_PLAY = 0X11,
	DECK_INFO_RECORD,
	DECK_INFO_PLAY_REVERSE,
	DECK_INFO_STILL,
	DECK_INFO_SLOW,
	DECK_INFO_SLOW_REVERSE,
	DECK_INFO_FAST_FORWARD,
	DECK_INFO_FAST_REVERSE,
	DECK_INFO_NO_MEDIA,
	DECK_INFO_STOP,
	DECK_INFO_SKIP_FORWARD_WIND,
	DECK_INFO_SKIP_REVERSE_RWIND,
	DECK_INFO_INDEX_SEARCH_FW,
	DECK_INFO_INDEX_SEARCH_RV,
	DECK_INFO_OTHER_STATUS
} cec_deck_status_t;

typedef enum{
	DECK_CTRL_SKIP_FW_WIND = 1,
	DECK_CTRL_SKIP_REVERSE_REWIND,
	DECK_CTRL_STOP,
	DECK_CTRL_EJECT,
} cec_deck_ctrl_mode;

typedef enum{
	PLAY_FOWARD = 0x24,
	PLAY_REVERSE = 0x20,
	PLAY_STILL = 0x25,
	FAST_FOWARD_MIN_SPEED = 0x05,
	FAST_FOWARD_MEDIUM_SPEED = 0x06,
	FAST_FOWARD_MAX_SPEED = 0x07,
	FAST_REVERSE_MIN_SPEED = 0x09,
	FAST_REVERSE_MEDIUM_SPEED = 0x0A,
	FAST_REVERSE_MAX_SPEED = 0xB,
	SLOW_FOWARD_MIN_SPEED = 0x15,
	SLOW_FOWARD_MEDIUM_SPEED = 0x16,
	SLOW_FOWARD_MAX_SPEED = 0x17,
	SLOW_REVERSE_MIN_SPEED = 0x19,
	SLOW_REVERSE_MEDIUM_SPEED = 0x1A,
	SLOW_REVERSE_MAX_SPEED = 0x1B
} cec_play_mode_t;


typedef enum{
	GIVE_STATUS_ON = 1,
	GIVE_STATUS_OFF,
	GIVE_STATUS_ONCE
} cec_give_deck_status_t;

typedef struct {
	int physical_address;
	cec_logical_address_t logical_address;
	cec_device_category_t device_category;
	cec_logical_address_t supported_addresses[4];
	int number_of_supported_addresses;
	cec_state_t cec_state;
	int power_state;
	int active_source;
	char language[3];
	char received_buffer[16];
	char transmit_buffer[16];
	int received_size;
	int transmit_size;
	cec_logical_address_t destination_addr;
	cec_deck_status_t deck_status;
	cec_give_deck_status_t give_deck_status;
} cec_device_t;

typedef enum{
	ON = 0,
	STANDBY,
	TRANSITION_TO_POWER_ON,
	TRANSITION_TO_STANDBY,
} cec_power_status;

/**
 * @short HDMI TX controller status information
 *
 * Initialize @b user fields (set status to zero).
 * After opening this data is for internal use only.
 */
struct hdmi_tx_ctrl {
	/** (@b user) Context status: closed (0), opened (<0) and
	 *  configured (>0) */
	int status;

	u8 data_enable_polarity;

	/** This is used to check if a cable is connected and if so the
	 * assume green light to configure the the core */
	int hpd;

	double pixel_clock;
	u8 pixel_repetition;
	u8 color_resolution;
	u8 encoding;
	u8 csc_on;
	u8 audio_on;
	u8 cec_on;
	u8 hdcp_on;
	u8 hdmi_on;
	phy_access_t phy_access;
	cec_device_t cec_tx_dev;
	u16 cec_phy_address;
};

/**
 * @short Main structures to instantiate the driver
 */
struct hdmi_tx_phy {
	int version;

	int generation;
	/** (@b user) Context status: closed (0), opened (<0) and configured
	 *  (>0) */
	int status;
};

#ifdef PHY_THIRD_PARTY

#define ACK_PHY_SIMULATION   99

struct register_table {
	int status;
	u32 value;
};

/**
 * @short Main structures to instantiate the driver
 */
struct hdmi_ack_tx_phy {
	int version;

	int generation;
	/** (@b user) Context status: closed (0), opened (<0) and configured
	 *  (>0) */
	int status;

	int debug_mode; //0 - disable; 1 - 4 verbose levels; 99 simulation

	int voltage;

	double sus_clock; //Mhz

	double gain;

	double pwclock; //Mhz

	double refclock;

	double datarate; //Mhz

	struct register_table * phy_sim;  //Register table to read/write

	struct register_table * phy_write; //Store Register configuration sent
};
#endif

typedef struct {
	/** Bypass encryption */
	int bypass;

	/** Enable Feature 1.1 */
	int mEnable11Feature;

	/** Check Ri every 128th frame */
	int mRiCheck;

	/** I2C fast mode */
	int mI2cFastMode;

	/** Enhanced link verification */
	int mEnhancedLinkVerification;

	/** Number of supported devices
	 * (depending on instantiated KSV MEM RAM – Revocation Memory to support
	 * HDCP repeaters)
	 */
	u8 maxDevices;

	/** KSV List buffer
	 * Shall be dimensioned to accommodate 5[bytes] x No. of supported devices
	 * (depending on instantiated KSV MEM RAM – Revocation Memory to support
	 * HDCP repeaters)
	 * plus 8 bytes (64-bit) M0 secret value
	 * plus 2 bytes Bstatus
	 * Plus 20 bytes to calculate the SHA-1 (VH0-VH4)
	 * Total is (30[bytes] + 5[bytes] x Number of supported devices)
	 */
	u8 *mKsvListBuffer;

	/** aksv total of 14 chars**/
	u8 *mAksv;

	/** Keys list
	 * 40 Keys of 14 characters each
	 * stored in segments of 8 bits (2 chars)
	 * **/
	u8 *mKeys;

	u8 *mSwEncKey;
} hdcpParams_t;

/**
 * @short Main structures to instantiate the driver
 */
typedef struct hdmi_tx_dev{
	char 			device_name[20];

#ifdef KERNEL
	/** Device node */
	struct device		*dev;
#endif

	/** Verbose */
	int			verbose;

	/** SYNOPSYS DATA */
	struct hdmi_tx_ctrl	snps_hdmi_ctrl;

	hdcpParams_t 		hdcp;
#ifdef PHY_THIRD_PARTY
	struct hdmi_ack_tx_phy  ack_phy;
#endif
} hdmi_tx_dev_t;


#endif /* INCLUDE_HDMITX_DEV_H_ */

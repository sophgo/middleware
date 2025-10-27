/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_common_mipi_tx.h
 * Description:
 */

#ifndef __CVI_COMM_MIPI_TX_H__
#define __CVI_COMM_MIPI_TX_H__

#define MIPI_TX0_NAME "/dev/soph-mipi-tx0"	/* Device path for MIPI TX interface 0 */
#define MIPI_TX1_NAME "/dev/soph-mipi-tx1"	/* Device path for MIPI TX interface 1 */

#define CMD_MAX_NUM    128		/* Maximum number of commands */
#define RX_MAX_NUM     4		/* Maximum number of RX channels */
#define LANE_MAX_NUM   5		/* Maximum number of lanes */

/* type of output mode
 *
 * OUTPUT_MODE_CSI: CSI mode
 * OUTPUT_MODE_DSI_VIDEO: DSI video mode
 * OUTPUT_MODE_DSI_CMD: DSI command mode
 */
enum output_mode_e {
	OUTPUT_MODE_CSI            = 0x0,
	OUTPUT_MODE_DSI_VIDEO      = 0x1,
	OUTPUT_MODE_DSI_CMD        = 0x2,
	OUTPUT_MODE_BUTT
};

/* type of video mode
 *
 * BURST_MODE: Burst mode
 * NON_BURST_MODE_SYNC_PULSES: Non-burst mode with sync pulses
 * NON_BURST_MODE_SYNC_EVENTS: Non-burst mode with sync events
 */
enum video_mode_e {
	BURST_MODE                      = 0x0,
	NON_BURST_MODE_SYNC_PULSES      = 0x1,
	NON_BURST_MODE_SYNC_EVENTS      = 0x2,
};

/* type of output format
 *
 * OUT_FORMAT_RGB_16_BIT: RGB 16-bit format
 * OUT_FORMAT_RGB_18_BIT: RGB 18-bit format
 * OUT_FORMAT_RGB_24_BIT: RGB 24-bit format
 * OUT_FORMAT_RGB_30_BIT: RGB 30-bit format
 * OUT_FORMAT_YUV420_8_BIT_NORMAL: YUV420 8-bit normal format
 * OUT_FORMAT_YUV420_8_BIT_LEGACY: YUV420 8-bit legacy format
 * OUT_FORMAT_YUV422_8_BIT: YUV422 8-bit format
 */
enum output_format_e {
	OUT_FORMAT_RGB_16_BIT          = 0x0,
	OUT_FORMAT_RGB_18_BIT          = 0x1,
	OUT_FORMAT_RGB_24_BIT          = 0x2,
	OUT_FORMAT_RGB_30_BIT          = 0x3,
	OUT_FORMAT_YUV420_8_BIT_NORMAL = 0x4,
	OUT_FORMAT_YUV420_8_BIT_LEGACY = 0x5,
	OUT_FORMAT_YUV422_8_BIT        = 0x6,
	OUT_FORMAT_BUTT
};

/* MIPI TX lane identification
 *
 * MIPI_TX_LANE_CLK: Clock lane
 * MIPI_TX_LANE_0: Data lane 0
 * MIPI_TX_LANE_1: Data lane 1
 * MIPI_TX_LANE_2: Data lane 2
 * MIPI_TX_LANE_3: Data lane 3
 */
enum mipi_tx_lane_id {
	MIPI_TX_LANE_CLK = 0,
	MIPI_TX_LANE_0,
	MIPI_TX_LANE_1,
	MIPI_TX_LANE_2,
	MIPI_TX_LANE_3,
	MIPI_TX_LANE_MAX,
};

/* Synchronization information structure
 *
 * vid_hsa_pixels: Horizontal sync active pixels
 * vid_hbp_pixels: Horizontal back porch pixels
 * vid_hfp_pixels: Horizontal front porch pixels
 * vid_hline_pixels: Total horizontal line pixels
 * vid_vsa_lines: Vertical sync active lines
 * vid_vbp_lines: Vertical back porch lines
 * vid_vfp_lines: Vertical front porch lines
 * vid_active_lines: Active display lines
 * edpi_cmd_size: EDPI command size
 * vid_vsa_pos_polarity: Vertical sync polarity
 * vid_hsa_pos_polarity: Horizontal sync polarity
 */
struct sync_info_s {
	unsigned short vid_hsa_pixels;
	unsigned short vid_hbp_pixels;
	unsigned short vid_hfp_pixels;
	unsigned short vid_hline_pixels;
	unsigned short vid_vsa_lines;
	unsigned short vid_vbp_lines;
	unsigned short vid_vfp_lines;
	unsigned short vid_active_lines;
	unsigned short edpi_cmd_size;
	bool vid_vsa_pos_polarity;
	bool vid_hsa_pos_polarity;
};

/* MIPI device configuration structure
 *
 * devno: Device number
 * lane_id: Lane ID array (-1 for disable)
 * output_mode: Output mode (CSI/DSI_VIDEO/DSI_CMD)
 * video_mode: Video mode
 * output_format: Output format
 * sync_info: Synchronization information
 * phy_data_rate: Physical layer data rate (Mbps)
 * pixel_clk: Pixel clock (KHz)
 * lane_pn_swap: Lane PN swap flags
 */
struct combo_dev_cfg_s {
	unsigned int devno;
	enum mipi_tx_lane_id lane_id[LANE_MAX_NUM];
	enum output_mode_e output_mode;
	enum video_mode_e video_mode;
	enum output_format_e output_format;
	struct sync_info_s sync_info;
	unsigned int phy_data_rate;
	unsigned int pixel_clk;
	bool lane_pn_swap[LANE_MAX_NUM];
};

/* Command information structure
 *
 * devno: Device number
 * data_type: Data type
 * cmd_size: Command size
 * cmd: Command data pointer
 */
struct cmd_info_s {
	unsigned int devno;
	unsigned short data_type;
	unsigned short cmd_size;
	unsigned char *cmd;
};

/* Get command information structure
 *
 * devno: Device number
 * data_param: data param,low 8 bit:first param.high 8 bit:second param, set 0 if not use
 * get_data_size: read data size
 * get_data: read data memery address, should  malloc by user
 */
struct get_cmd_info_s {
	unsigned int devno;
	unsigned short data_type;
	unsigned short data_param;
	unsigned short get_data_size;
	unsigned char *get_data;
};

/* High-speed timing configuration structure
 *
 * prepare: Prepare time
 * zero: Zero time
 * trail: Trail time
 */
struct hs_settle_s {
	unsigned char prepare;
	unsigned char zero;
	unsigned char trail;
};

/* DSC instruction structure
 *
 * delay: Delay time
 * data_type: Data type
 * size: Data size
 * data: Data pointer
 */
struct dsc_instr {
	unsigned char delay;
	unsigned char data_type;
	unsigned char size;
	unsigned char *data;
};

#define MIPI_TX_IOC_MAGIC   't'		/* Magic number for MIPI TX IOCTL commands */

/* IOCTL commands for MIPI TX */
#define MIPI_TX_SET_DEV_CFG              _IOW(MIPI_TX_IOC_MAGIC, 0x01, struct combo_dev_cfg_s) /* Set device configuration */
#define MIPI_TX_GET_DEV_CFG              _IOWR(MIPI_TX_IOC_MAGIC, 0x01, struct combo_dev_cfg_s) /* Get device configuration */
#define MIPI_TX_SET_CMD                  _IOW(MIPI_TX_IOC_MAGIC, 0x02, struct cmd_info_s) /* Set command */
#define MIPI_TX_ENABLE                   _IO(MIPI_TX_IOC_MAGIC, 0x03) /* Enable MIPI TX */
#define MIPI_TX_GET_CMD                  _IOWR(MIPI_TX_IOC_MAGIC, 0x04, struct get_cmd_info_s) /* Get command */
#define MIPI_TX_DISABLE                  _IO(MIPI_TX_IOC_MAGIC, 0x05) /* Disable MIPI TX */
#define MIPI_TX_SET_HS_SETTLE            _IOW(MIPI_TX_IOC_MAGIC, 0x06, struct hs_settle_s) /* Set high-speed timing parameters */
#define MIPI_TX_GET_HS_SETTLE            _IOWR(MIPI_TX_IOC_MAGIC, 0x06, struct hs_settle_s) /* Get high-speed timing parameters */
#define MIPI_TX_SUSPEND                  _IO(MIPI_TX_IOC_MAGIC, 0x07) /* Suspend MIPI TX */
#define MIPI_TX_RESUME                   _IO(MIPI_TX_IOC_MAGIC, 0x08) /* Resume MIPI TX */

#endif // __CVI_COMM_MIPI_TX_H__


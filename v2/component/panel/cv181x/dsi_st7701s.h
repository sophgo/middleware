#ifndef _MIPI_TX_PARAM_ST_7701S_H_
#define _MIPI_TX_PARAM_ST_7701S_H_

#include <cvi_comm_mipi_tx.h>

#define PANEL_NAME "NETEASE-2"

#define ST7701S_NETEASE_VACT	640
#define ST7701S_NETEASE_VSA		10
#define ST7701S_NETEASE_VBP		20
#define ST7701S_NETEASE_VFP		20

#define ST7701S_NETEASE_HACT	480
#define ST7701S_NETEASE_HSA		10
#define ST7701S_NETEASE_HBP		60
#define ST7701S_NETEASE_HFP		60

#define PIXEL_CLK(x) ((x##_VACT + x##_VSA + x##_VBP + x##_VFP) \
	* (x##_HACT + x##_HSA + x##_HBP + x##_HFP) * 60 / 1000)

struct combo_dev_cfg_s dev_cfg_st7701s_480x640 = {
	.devno = 0,
	.lane_id = {MIPI_TX_LANE_0, MIPI_TX_LANE_1, -1, -1, MIPI_TX_LANE_CLK},
	.lane_pn_swap = {false, false, false, false, false},
	.output_mode = OUTPUT_MODE_DSI_VIDEO,
	.video_mode = BURST_MODE,
	.output_format = OUT_FORMAT_RGB_24_BIT,
	.sync_info = {
		.vid_hsa_pixels = ST7701S_NETEASE_HSA,
		.vid_hbp_pixels = ST7701S_NETEASE_HBP,
		.vid_hfp_pixels = ST7701S_NETEASE_HFP,
		.vid_hline_pixels = ST7701S_NETEASE_HACT,
		.vid_vsa_lines = ST7701S_NETEASE_VSA,
		.vid_vbp_lines = ST7701S_NETEASE_VBP,
		.vid_vfp_lines = ST7701S_NETEASE_VFP,
		.vid_active_lines = ST7701S_NETEASE_VACT,
		.vid_vsa_pos_polarity = true,
		.vid_hsa_pos_polarity = false,
	},
	.pixel_clk = PIXEL_CLK(ST7701S_NETEASE),
};

const struct hs_settle_s hs_timing_cfg_st7701s_480x640 = { .prepare = 6, .zero = 32, .trail = 1 };

static CVI_U8 data_st7701s_0[] = { 0xff, 0x77, 0x01, 0x00, 0x00, 0x13 };
static CVI_U8 data_st7701s_1[] = { 0xef, 0x08 };
static CVI_U8 data_st7701s_2[] = { 0xff, 0x77, 0x01, 0x00, 0x00, 0x10 };
static CVI_U8 data_st7701s_3[] = { 0xc0, 0x4f, 0x00 };
static CVI_U8 data_st7701s_4[] = { 0xc1, 0x07, 0x02 };
static CVI_U8 data_st7701s_5[] = { 0xc2, 0x31, 0x07 };
static CVI_U8 data_st7701s_6[] = { 0xcc, 0x10 };
static CVI_U8 data_st7701s_7[] = {
	0xb0, 0x00, 0x0a, 0x11, 0x0c, 0x10, 0x05, 0x00, 0x08, 0x08,
	0x1f, 0x07, 0x13, 0x10, 0xa9, 0x30, 0x18
};

static CVI_U8 data_st7701s_8[] = {
	0xb1, 0x00, 0x0b, 0x11, 0x0d, 0x0f, 0x05, 0x02, 0x07, 0x06,
	0x20, 0x05, 0x15, 0x13, 0xa9, 0x30, 0x18
};

static CVI_U8 data_st7701s_9[] = { 0xff, 0x77, 0x01, 0x00, 0x00, 0x11 };
static CVI_U8 data_st7701s_10[] = { 0xb0, 0x53 };
static CVI_U8 data_st7701s_11[] = { 0xb1, 0x60 };
static CVI_U8 data_st7701s_12[] = { 0xb2, 0x87 };
static CVI_U8 data_st7701s_13[] = { 0xb3, 0x80 };
static CVI_U8 data_st7701s_14[] = { 0xb5, 0x49 };
static CVI_U8 data_st7701s_15[] = { 0xb7, 0x85 };
static CVI_U8 data_st7701s_16[] = { 0xb8, 0x21 };
static CVI_U8 data_st7701s_17[] = { 0xbb, 0x33 };
static CVI_U8 data_st7701s_18[] = { 0xbc, 0x33 };
static CVI_U8 data_st7701s_19[] = { 0xc1, 0x78 };
static CVI_U8 data_st7701s_20[] = { 0xc2, 0x78 };
static CVI_U8 data_st7701s_21[] = { 0xd0, 0x88 };
static CVI_U8 data_st7701s_22[] = { 0xe0, 0x00, 0x00, 0x02 };
static CVI_U8 data_st7701s_23[] = {
	0xe1, 0x03, 0xa0, 0x00, 0x00, 0x02, 0xa0, 0x00, 0x00, 0x00,
	0x33, 0x33
};

static CVI_U8 data_st7701s_24[] = {
	0xe2, 0x22, 0x22, 0x33, 0x33, 0x88, 0xa0, 0x00, 0x00, 0x87,
	0xa0, 0x00, 0x00
};

static CVI_U8 data_st7701s_25[] = { 0xe3, 0x00, 0x00, 0x22, 0x22 };
static CVI_U8 data_st7701s_26[] = { 0xe4, 0x44, 0x44 };
static CVI_U8 data_st7701s_27[] = {
	0xe5, 0x04, 0x84, 0xa0, 0xa0, 0x06, 0x86, 0xa0, 0xa0, 0x08,
	0x88, 0xa0, 0xa0, 0x0a, 0x8a, 0xa0, 0xa0
};

static CVI_U8 data_st7701s_28[] = { 0xe6, 0x00, 0x00, 0x22, 0x22 };
static CVI_U8 data_st7701s_29[] = { 0xe7, 0x44, 0x44 };
static CVI_U8 data_st7701s_30[] = {
	0xe8, 0x03, 0x83, 0xa0, 0xa0, 0x05, 0x85, 0xa0, 0xa0, 0x07,
	0x87, 0xa0, 0xa0, 0x09, 0x89, 0xa0, 0xa0
};

static CVI_U8 data_st7701s_31[] = { 0xeb, 0x00, 0x01, 0xe4, 0xe4, 0x88, 0x00, 0x40 };
static CVI_U8 data_st7701s_32[] = { 0xec, 0x3c, 0x01 };
static CVI_U8 data_st7701s_33[] = {
	0xed, 0xab, 0x89, 0x76, 0x54, 0x02, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0x20, 0x45, 0x67, 0x98, 0xba
};

static CVI_U8 data_st7701s_34[] = { 0xef, 0x10, 0x0d, 0x04, 0x08, 0x3f, 0x1f };
static CVI_U8 data_st7701s_35[] = { 0xff, 0x77, 0x01, 0x00, 0x00, 0x13 };
static CVI_U8 data_st7701s_36[] = { 0xe6, 0x16, 0x7c };
static CVI_U8 data_st7701s_37[] = { 0xe8, 0x00, 0x0e };
static CVI_U8 data_st7701s_38[] = { 0xff, 0x77, 0x01, 0x00, 0x00, 0x00 };
static CVI_U8 data_st7701s_39[] = { 0x11 };
static CVI_U8 data_st7701s_40[] = { 0xff, 0x77, 0x01, 0x00, 0x00, 0x13 };
static CVI_U8 data_st7701s_41[] = { 0xe8, 0x00, 0x0c };
static CVI_U8 data_st7701s_42[] = { 0xe8, 0x00, 0x00 };
static CVI_U8 data_st7701s_43[] = { 0xff, 0x77, 0x01, 0x00, 0x00, 0x00 };
static CVI_U8 data_st7701s_44[] = { 0x29 };


const struct dsc_instr dsi_init_cmds_st7701s_480x640[] = {
	{.delay = 0, .data_type = 0x39, .size = 6, .data = data_st7701s_0 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701s_1 },
	{.delay = 0, .data_type = 0x39, .size = 6, .data = data_st7701s_2 },
	{.delay = 0, .data_type = 0x39, .size = 3, .data = data_st7701s_3 },
	{.delay = 0, .data_type = 0x39, .size = 3, .data = data_st7701s_4 },
	{.delay = 0, .data_type = 0x39, .size = 3, .data = data_st7701s_5 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701s_6 },
	{.delay = 0, .data_type = 0x39, .size = 17, .data = data_st7701s_7 },
	{.delay = 0, .data_type = 0x39, .size = 17, .data = data_st7701s_8 },
	{.delay = 0, .data_type = 0x39, .size = 6, .data = data_st7701s_9 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701s_10 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701s_11 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701s_12 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701s_13 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701s_14 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701s_15 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701s_16 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701s_17 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701s_18 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701s_19 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701s_20 },
	{.delay = 10, .data_type = 0x15, .size = 2, .data = data_st7701s_21 },
	{.delay = 0, .data_type = 0x39, .size = 4, .data = data_st7701s_22 },
	{.delay = 0, .data_type = 0x39, .size = 12, .data = data_st7701s_23 },
	{.delay = 0, .data_type = 0x39, .size = 13, .data = data_st7701s_24 },
	{.delay = 0, .data_type = 0x39, .size = 5, .data = data_st7701s_25 },
	{.delay = 0, .data_type = 0x39, .size = 3, .data = data_st7701s_26 },
	{.delay = 0, .data_type = 0x39, .size = 17, .data = data_st7701s_27 },
	{.delay = 0, .data_type = 0x39, .size = 5, .data = data_st7701s_28 },
	{.delay = 0, .data_type = 0x39, .size = 3, .data = data_st7701s_29 },
	{.delay = 0, .data_type = 0x39, .size = 17, .data = data_st7701s_30 },
	{.delay = 0, .data_type = 0x39, .size = 8, .data = data_st7701s_31 },
	{.delay = 0, .data_type = 0x39, .size = 3, .data = data_st7701s_32 },
	{.delay = 0, .data_type = 0x39, .size = 17, .data = data_st7701s_33 },
	{.delay = 0, .data_type = 0x39, .size = 7, .data = data_st7701s_34 },
	{.delay = 0, .data_type = 0x39, .size = 6, .data = data_st7701s_35 },
	{.delay = 0, .data_type = 0x39, .size = 3, .data = data_st7701s_36 },
	{.delay = 0, .data_type = 0x39, .size = 3, .data = data_st7701s_37 },
	{.delay = 0, .data_type = 0x39, .size = 6, .data = data_st7701s_38 },
	{.delay = 120, .data_type = 0x05, .size = 1, .data = data_st7701s_39 },
	{.delay = 0, .data_type = 0x39, .size = 6, .data = data_st7701s_40 },
	{.delay = 10, .data_type = 0x39, .size = 3, .data = data_st7701s_41 },
	{.delay = 0, .data_type = 0x39, .size = 3, .data = data_st7701s_42 },
	{.delay = 0, .data_type = 0x39, .size = 6, .data = data_st7701s_43 },
	{.delay = 0, .data_type = 0x39, .size = 1, .data = data_st7701s_44 },
};

#else
#error "_MIPI_TX_PARAM_ST_7701S_H_ multi-delcaration!!"
#endif // _MIPI_TX_PARAM_ST_7701S_H_

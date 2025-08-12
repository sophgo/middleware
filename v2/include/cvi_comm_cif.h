/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_comm_cif.h
 * Description:
 *   Common cif definitions.
 */

#ifndef __CVI_COMM_CIF_H__
#define __CVI_COMM_CIF_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* Maximum number of MIPI lanes supported */
#define MIPI_LANE_NUM	8
/* Number of virtual channels for Wide Dynamic Range (WDR) mode */
#define WDR_VC_NUM	2
/* Number of synchronization codes */
#define SYNC_CODE_NUM	4
/* Number of BT (BT.656/BT.1120) demultiplexers */
#define BT_DEMUX_NUM	4
/* Number of MIPI demultiplexers */
#define MIPI_DEMUX_NUM	4

/* Basic image size structure */
struct img_size_s {
	unsigned int	width;		/* Image width */
	unsigned int	height; 	/* Image height */
	unsigned int	start_x;	/* Image start x */
	unsigned int	start_y;	/* Image start y */
	unsigned int	active_w;	/* Active width */
	unsigned int	active_h;	/* Active height */
	unsigned int	max_width;	/* Maximum width */
	unsigned int	max_height;	/* Maximum height */
};

/* RX MAC clock frequency enumeration */
enum rx_mac_clk_e {
	RX_MAC_CLK_150M = 0, 	/* 150MHz clock */
	RX_MAC_CLK_200M, 		/* 200MHz clock */
	RX_MAC_CLK_300M, 		/* 300MHz clock */
	RX_MAC_CLK_400M, 		/* 400MHz clock */
	RX_MAC_CLK_500M, 		/* 500MHz clock */
	RX_MAC_CLK_600M, 		/* 600MHz clock */
	RX_MAC_CLK_900M, 		/* 900MHz clock */
	RX_MAC_CLK_BUTT, 		/* Invalid value */
};

/* Camera PLL frequency enumeration */
enum cam_pll_freq_e {
	CAMPLL_FREQ_NONE = 0,	/* No frequency */
	CAMPLL_FREQ_37P125M,	/* 37.125MHz */
	CAMPLL_FREQ_25M,		/* 25MHz */
	CAMPLL_FREQ_27M,		/* 27MHz */
	CAMPLL_FREQ_24M,		/* 24MHz */
	CAMPLL_FREQ_26M,		/* 26MHz */
	CAMPLL_FREQ_NUM,		/* Number of frequencies */
};

/* MCLK and PLL configuration structure */
struct mclk_pll_s {
	unsigned int			cam; 		/* Camera index */
	enum cam_pll_freq_e 	freq;		/* PLL frequency */
};

/* DPHY configuration structure */
struct dphy_s {
	unsigned char 	enable;			/* DPHY enable flag */
	unsigned char 	hs_settle;		/* High speed settle time */
};

/* Input interface mode enumeration */
enum input_mode_e {
	INPUT_MODE_MIPI = 0,		/* MIPI CSI-2 interface */
	INPUT_MODE_SUBLVDS,			/* Sub-LVDS interface */
	INPUT_MODE_HISPI,			/* HiSPi interface */
	INPUT_MODE_CMOS, 			/* CMOS/parallel interface */
	INPUT_MODE_BT1120,			/* BT.1120 interface */
	INPUT_MODE_BT601,			/* BT.601 interface */
	INPUT_MODE_BT656_9B,		/* BT.656 9-bit interface */
	INPUT_MODE_BT656_9B_DDR,	/* BT.656 9-bit DDR interface */
	INPUT_MODE_CUSTOM_0,		/* Custom interface 0 */
	INPUT_MODE_BT_DEMUX,		/* BT demux interface */
	INPUT_MODE_BUTT				/* Invalid mode */
};

/* Raw data type enumeration */
enum raw_data_type_e {
	RAW_DATA_8BIT = 0,		/* 8-bit raw data */
	RAW_DATA_10BIT,			/* 10-bit raw data */
	RAW_DATA_12BIT,			/* 12-bit raw data */
	RAW_DATA_16BIT,			/* 16-bit raw data */
	YUV422_8BIT,			/* 8-bit YUV422 format, MIPI-CSI only */
	YUV422_10BIT,			/* 10-bit YUV422 format, MIPI-CSI only */
	RAW_DATA_BUTT			/* Invalid type */
};

/* MIPI WDR mode enumeration */
enum mipi_wdr_mode_e {
	CVI_MIPI_WDR_MODE_NONE = 0,		/* No WDR mode */
	CVI_MIPI_WDR_MODE_VC,			/* Virtual Channel WDR mode */
	CVI_MIPI_WDR_MODE_DT,			/* Data Type WDR mode */
	CVI_MIPI_WDR_MODE_DOL,			/* DOL WDR mode */
	CVI_MIPI_WDR_MODE_MANUAL,		/* Manual WDR mode (SOI case) */
	CVI_MIPI_WDR_MODE_BUTT			/* Invalid mode */
};

/* WDR mode enumeration */
enum wdr_mode_e {
	CVI_WDR_MODE_NONE = 0,			/* No WDR */
	CVI_WDR_MODE_2F,				/* 2-frame WDR */
	CVI_WDR_MODE_3F,				/* 3-frame WDR */
	CVI_WDR_MODE_DOL_2F,			/* 2-frame DOL WDR */
	CVI_WDR_MODE_DOL_3F,			/* 3-frame DOL WDR */
	CVI_WDR_MODE_DOL_BUTT,			/* Invalid mode */
};

/* LVDS sync mode enumeration */
enum lvds_sync_mode_e {
	LVDS_SYNC_MODE_SOF = 0,			/* Start of Frame sync mode */
	LVDS_SYNC_MODE_SAV,				/* Start Active Video sync mode */
	LVDS_SYNC_MODE_BUTT				/* Invalid mode */
};

/* LVDS bit endian enumeration */
enum lvds_bit_endian {
	LVDS_ENDIAN_LITTLE = 0,        /* Little endian */
	LVDS_ENDIAN_BIG,               /* Big endian */
	LVDS_ENDIAN_BUTT               /* Invalid endian */
};

/* LVDS vsync type enumeration */
enum lvds_vsync_type_e {
	LVDS_VSYNC_NORMAL = 0,			/* Normal vsync */
	LVDS_VSYNC_SHARE,				/* Shared vsync */
	LVDS_VSYNC_HCONNECT,			/* H-connected vsync */
	LVDS_VSYNC_BUTT					/* Invalid type */
};

/* LVDS field ID type enumeration */
enum lvds_fid_type_e {
	LVDS_FID_NONE = 0,			/* No field ID */
	LVDS_FID_IN_SAV,			/* Field ID in SAV */
	LVDS_FID_BUTT 				/* Invalid type */
};

/* LVDS field ID type structure */
struct lvds_fid_type_s {
	enum lvds_fid_type_e	fid;		/* Field ID type */
};

/* LVDS vsync type structure */
struct lvds_vsync_type_s {
	enum lvds_vsync_type_e	sync_type;			/* Vsync type */
	unsigned short			hblank1;			/* First horizontal blanking period */
	unsigned short			hblank2;			/* Second horizontal blanking period */
};

/* LVDS device attributes structure */
struct lvds_dev_attr_s {
	enum wdr_mode_e			wdr_mode;												/* WDR mode */
	enum lvds_sync_mode_e	sync_mode;												/* Sync mode */
	enum raw_data_type_e	raw_data_type;											/* Raw data type */
	enum lvds_bit_endian	data_endian;											/* Data endian */
	enum lvds_bit_endian	sync_code_endian;										/* Sync code endian */
	short 					lane_id[MIPI_LANE_NUM+1];								/* Lane ID mapping */
	short 					sync_code[MIPI_LANE_NUM][WDR_VC_NUM+1][SYNC_CODE_NUM];	/* Sync codes */
/*
 * sublvds:
 * sync_code[x][0][0] sync_code[x][0][1] sync_code[x][0][2] sync_code[x][0][3]
 *	n0_lef_sav	   n0_lef_eav	      n1_lef_sav	 n1_lef_eav
 * sync_code[x][1][0] sync_code[x][1][1] sync_code[x][1][2] sync_code[x][1][3]
 *	n0_sef_sav	   n0_sef_eav	      n1_sef_sav	 n1_sef_eav
 * sync_code[x][2][0] sync_code[x][2][1] sync_code[x][2][2] sync_code[x][2][3]
 *	n0_lsef_sav	   n0_lsef_eav	      n1_lsef_sav	 n1_lsef_eav
 *
 * hispi:
 * sync_code[x][0][0] sync_code[x][0][1] sync_code[x][0][2] sync_code[x][0][3]
 *	t1_sol		   tl_eol	      t1_sof		 t1_eof
 * sync_code[x][1][0] sync_code[x][1][1] sync_code[x][1][2] sync_code[x][1][3]
 *	t2_sol		   t2_eol	      t2_sof		 t2_eof
 */
	struct lvds_vsync_type_s	vsync_type;					/* Vsync type configuration */
	struct lvds_fid_type_s		fid_type;					/* Field ID type configuration */
	char 						pn_swap[MIPI_LANE_NUM+1];	/* PN swap configuration */
};

/* MIPI demux information structure */
struct mipi_demux_info_s {
	unsigned int	demux_en;						/* Demux enable flag */
	unsigned char	vc_mapping[MIPI_DEMUX_NUM]; 	/* Virtual channel mapping */
};

/* MIPI device attributes structure */
struct mipi_dev_attr_s {
	enum raw_data_type_e		raw_data_type;					/* Raw data type */
	short						lane_id[MIPI_LANE_NUM+1];		/* Lane ID mapping */
	enum mipi_wdr_mode_e		wdr_mode;						/* WDR mode */
	short						data_type[WDR_VC_NUM];			/* Data type for each virtual channel */
	char						pn_swap[MIPI_LANE_NUM+1];		/* PN swap configuration */
	struct dphy_s				dphy;							/* DPHY configuration */
	struct mipi_demux_info_s	demux;							/* Demux configuration */
};

/* Manual WDR attributes structure */
struct manual_wdr_attr_s {
	unsigned int		manual_en;					/* Manual mode enable */
	unsigned short		l2s_distance;				/* Long to short frame distance */
	unsigned short		lsef_length; 				/* Long/Short exposure frame length */
	unsigned int		discard_padding_lines;		/* Number of padding lines to discard */
	unsigned int		update;						/* Update flag */
};

/* TTL pin function enumeration */
enum ttl_pin_func_e {
	TTL_PIN_FUNC_VS,		/* Vertical Sync signal */
	TTL_PIN_FUNC_HS,		/* Horizontal Sync signal */
	TTL_PIN_FUNC_VDE,		/* Vertical Data Enable signal */
	TTL_PIN_FUNC_HDE,		/* Horizontal Data Enable signal */
	TTL_PIN_FUNC_D0,		/* Data bit 0 */
	TTL_PIN_FUNC_D1,		/* Data bit 1 */
	TTL_PIN_FUNC_D2,		/* Data bit 2 */
	TTL_PIN_FUNC_D3,		/* Data bit 3 */
	TTL_PIN_FUNC_D4,		/* Data bit 4 */
	TTL_PIN_FUNC_D5,		/* Data bit 5 */
	TTL_PIN_FUNC_D6,		/* Data bit 6 */
	TTL_PIN_FUNC_D7,		/* Data bit 7 */
	TTL_PIN_FUNC_D8,		/* Data bit 8 */
	TTL_PIN_FUNC_D9,		/* Data bit 9 */
	TTL_PIN_FUNC_D10,		/* Data bit 10 */
	TTL_PIN_FUNC_D11,		/* Data bit 11 */
	TTL_PIN_FUNC_D12,		/* Data bit 12 */
	TTL_PIN_FUNC_D13,		/* Data bit 13 */
	TTL_PIN_FUNC_D14,		/* Data bit 14 */
	TTL_PIN_FUNC_D15,		/* Data bit 15 */
	TTL_PIN_FUNC_NUM,		/* Total number of TTL pin functions */
};

/* TTL VI0 clock selection enumeration */
enum ttl_vi0clk_e {
	TTL_VI0_CLK0 = 0,		/* VI0 clock 0 */
	TTL_VI0_CLK1,			/* VI0 clock 1 */
	TTL_VI0_CLK_MAX			/* Maximum clock index */
};

/* TTL source enumeration */
enum ttl_src_e {
	TTL_VI_SRC_VI0 = 0,		/* VI source 0 */
	TTL_VI_SRC_VI1,			/* VI source 1 (BT demux & BT) */
	TTL_VI_SRC_VI2,			/* VI source 2 (BT demux & BT) */
	TTL_VI_SRC_NUM			/* Number of VI sources */
};

/* TTL format enumeration */
enum ttl_fmt_e {
	TTL_SYNC_PAT = 0,	/* Sync pattern format */
	TTL_VHS_11B,		/* VHS 11-bit format */
	TTL_VHS_19B,		/* VHS 19-bit format */
	TTL_VDE_11B,		/* VDE 11-bit format */
	TTL_VDE_19B,		/* VDE 19-bit format */
	TTL_VSDE_11B,		/* VSDE 11-bit format */
	TTL_VSDE_19B, 		/* VSDE 19-bit format */
};

/* BT demux mode enumeration */
enum bt_demux_mode_e {
	BT_DEMUX_DISABLE = 0,	/* Demux disabled */
	BT_DEMUX_2,				/* 2-channel demux */
	BT_DEMUX_3,				/* 3-channel demux */
	BT_DEMUX_4,				/* 4-channel demux */
};

/* BT demux sync structure */
struct bt_demux_sync_s {
	unsigned char		sav_vld;		/* SAV valid flag */
	unsigned char		sav_blk;		/* SAV blanking flag */
	unsigned char		eav_vld;		/* EAV valid flag */
	unsigned char		eav_blk;		/* EAV blanking flag */
};

/* BT demux attributes structure */
struct bt_demux_attr_s {
	enum ttl_src_e			vi;								/* select switch vi data from, necessary */
	signed char				func[TTL_PIN_FUNC_NUM];			/* Pin function mapping */
	unsigned short			v_fp;							/* Vertical front porch */
	unsigned short			h_fp;							/* Horizontal front porch */
	unsigned short			v_bp;							/* Vertical back porch */
	unsigned short			h_bp;							/* Horizontal back porch */
	enum bt_demux_mode_e	mode;							/* Demux mode */
	unsigned char			sync_code_part_A[3];			/* Sync codes 0-2 */
	struct bt_demux_sync_s	sync_code_part_B[BT_DEMUX_NUM];	/* Sync codes 3 */
	char					yc_exchg;						/* Y/C exchange flag */
};

/* TTL device attributes structure */
struct ttl_dev_attr_s {
	enum ttl_src_e			vi;							/* select switch vi data from, necessary */
	enum ttl_fmt_e			ttl_fmt;					/* TTL format */
	enum raw_data_type_e	raw_data_type;				/* Raw data type */
	signed char				func[TTL_PIN_FUNC_NUM];		/* Pin function mapping */
	enum ttl_vi0clk_e		vi0_clk;					/* VI0 clock selection */
	unsigned short			v_bp;						/* Vertical back porch */
	unsigned short			h_bp;						/* Horizontal back porch */
};

/* Combined device attributes structure */
struct combo_dev_attr_s {
	enum input_mode_e			input_mode;			/* Input interface mode */
	enum rx_mac_clk_e			mac_clk;			/* MAC clock frequency */
	struct mclk_pll_s			mclk;				/* MCLK and PLL configuration */
	union {
		struct mipi_dev_attr_s	mipi_attr;			/* MIPI device attributes */
		struct lvds_dev_attr_s	lvds_attr;			/* LVDS device attributes */
		struct ttl_dev_attr_s	ttl_attr;			/* TTL device attributes */
		struct bt_demux_attr_s	bt_demux_attr;		/* BT demux attributes */
	};
	unsigned int				devno;				/* Device number */
	unsigned int				cif_mode;			/* CIF mode */
	struct img_size_s			img_size;			/* Image size configuration */
	struct manual_wdr_attr_s	wdr_manu;			/* Manual WDR configuration */
};

/* Clock edge enumeration */
enum clk_edge_e {
	CLK_UP_EDGE = 0,		/* Rising edge */
	CLK_DOWN_EDGE,			/* Falling edge */
	CLK_EDGE_BUTT			/* Invalid edge */
};

/* Clock edge configuration structure */
struct clk_edge_s {
	unsigned int devno;			/* Device number */
	enum clk_edge_e edge;		/* Clock edge selection */
};

enum output_msb_e {
	OUTPUT_NORM_MSB = 0,
	OUTPUT_REVERSE_MSB,
	OUTPUT_MSB_BUTT
};

struct msb_s {
	unsigned int			devno;
	enum output_msb_e		msb;
};

/* Crop top configuration structure */
struct crop_top_s {
	unsigned int	devno;			/* Device number */
	unsigned int	crop_top;		/* Crop top lines */
	unsigned int	update;		/* Update flag */
};

/* Manual WDR configuration structure */
struct manual_wdr_s {
	unsigned int				devno;			/* Device number */
	struct manual_wdr_attr_s	attr;			/* Manual WDR attributes */
};

/* Vsync generation configuration structure */
struct vsync_gen_s {
	unsigned int	devno;				/* Device number */
	unsigned int	distance_fp;		/* Front porch distance */
};

/* BT format output enumeration */
enum bt_fmt_out_e {
	BT_FMT_OUT_CBYCRY,				/* CBYCRY format */
	BT_FMT_OUT_CRYCBY,				/* CRYCBY format */
	BT_FMT_OUT_YCBYCR,				/* YCBYCR format */
	BT_FMT_OUT_YCRYCB,				/* YCRYCB format */
};

/* BT format output configuration structure */
struct bt_fmt_out_s {
	unsigned int		devno;				/* Device number */
	enum bt_fmt_out_e	fmt_out;			/* Output format */
};

/* CIF crop window configuration structure */
struct cif_crop_win_s {
	unsigned int		devno;				/* Device number */
	unsigned int		enable;				/* Enable flag */
	unsigned int		x;					/* X coordinate */
	unsigned int		y;					/* Y coordinate */
	unsigned int		w;					/* Width */
	unsigned int		h;					/* Height */
};

/* CIF YUV swap configuration structure */
struct cif_yuv_swap_s {
	unsigned int		devno;					/* Device number */
	unsigned int		uv_swap;				/* U/V swap flag */
	unsigned int		yc_swap;				/* Y/C swap flag */
};

/* Sensor reset active level enumeration */
enum sns_rst_active_e {
	RST_ACTIVE_LOW,					/* Active low reset */
	RST_ACTIVE_HIGH,				/* Active high reset */
	RST_ACTIVE_BUFF,				/* Buffered reset */
};

/* Sensor reset configuration structure */
typedef struct sns_rst_config {
	unsigned int			devno;					/* Device number */
	unsigned int			gpio_pin;				/* GPIO pin number */
	enum sns_rst_active_e	gpio_active;			/* GPIO active level */
} SNS_RST_CONFIG;

/* mipi_rx ioctl commands related definition */
#define CVI_MIPI_IOC_MAGIC		'm'

/* Support commands */
#define CVI_MIPI_SET_DEV_ATTR		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x01, struct combo_dev_attr_s)
#define CVI_MIPI_SET_OUTPUT_CLK_EDGE	_IOW(CVI_MIPI_IOC_MAGIC, \
						0x02, struct clk_edge_s)
#define CVI_MIPI_RESET_SENSOR		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x05, struct sns_rst_config)
#define CVI_MIPI_UNRESET_SENSOR		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x06, struct sns_rst_config)
#define CVI_MIPI_RESET_MIPI		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x07, unsigned int)
#define CVI_MIPI_ENABLE_SENSOR_CLOCK	_IOW(CVI_MIPI_IOC_MAGIC, \
						0x10, unsigned int)
#define CVI_MIPI_DISABLE_SENSOR_CLOCK	_IOW(CVI_MIPI_IOC_MAGIC, \
						0x11, unsigned int)
#define CVI_MIPI_SET_CROP_TOP		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x20, struct crop_top_s)
#define CVI_MIPI_SET_WDR_MANUAL		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x21, struct manual_wdr_s)
#define CVI_MIPI_SET_LVDS_FP_VS		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x22, struct vsync_gen_s)
#define CVI_MIPI_RESET_LVDS		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x23, unsigned int)
#define CVI_MIPI_SET_BT_FMT_OUT		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x24, struct bt_fmt_out_s)
#define CVI_MIPI_GET_CIF_ATTR		_IOWR(CVI_MIPI_IOC_MAGIC, \
						0x25, struct cif_attr_s)
#define CVI_MIPI_SET_SENSOR_CLOCK	_IOW(CVI_MIPI_IOC_MAGIC, \
						0x26, struct mclk_pll_s)
#define CVI_MIPI_SET_MAX_MAC_CLOCK	_IOW(CVI_MIPI_IOC_MAGIC, \
						0x27, unsigned int)
#define CVI_MIPI_SET_CROP_WINDOW	_IOW(CVI_MIPI_IOC_MAGIC, \
						0x28, struct cif_crop_win_s)
#define CVI_MIPI_SET_YUV_SWAP		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x29, struct cif_yuv_swap_s)
/* Unsupport commands */
#define CVI_MIPI_SET_PHY_CMVMODE	_IOW(CVI_MIPI_IOC_MAGIC, \
						0x04, unsigned int)
#define CVI_MIPI_UNRESET_MIPI		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x08, unsigned int)
#define CVI_MIPI_RESET_SLVS		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x09, unsigned int)
#define CVI_MIPI_UNRESET_SLVS		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x0A, unsigned int)
#define CVI_MIPI_SET_HS_MODE		_IOW(CVI_MIPI_IOC_MAGIC, \
						0x0B, unsigned int)
#define CVI_MIPI_ENABLE_MIPI_CLOCK	_IOW(CVI_MIPI_IOC_MAGIC, \
						0x0C, unsigned int)
#define CVI_MIPI_DISABLE_MIPI_CLOCK	_IOW(CVI_MIPI_IOC_MAGIC, \
						0x0D, unsigned int)
#define CVI_MIPI_ENABLE_SLVS_CLOCK	_IOW(CVI_MIPI_IOC_MAGIC, \
						0x0E, unsigned int)
#define CVI_MIPI_DISABLE_SLVS_CLOCK	_IOW(CVI_MIPI_IOC_MAGIC, \
						0x0F, unsigned int)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* End of #ifndef__CVI_COMM_CIF_H__ */

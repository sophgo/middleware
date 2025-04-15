#ifndef __CVI_SNS_CTRL_H__
#define __CVI_SNS_CTRL_H__

#include <cvi_comm_cif.h>
#include <cvi_errno.h>
#include <cvi_type.h>
#include "cvi_debug.h"
#include "cvi_comm_3a.h"
#include "cvi_comm_isp.h"
#include "cvi_ae_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct combo_dev_attr_s SNS_COMBO_DEV_ATTR_S;
typedef struct mclk_pll_s SNS_MCLK_ATTR_S;

/* Basic sensor attribute structure for 16-bit parameters */
typedef struct _SNS_ATTR_S {
	CVI_U16	u16Min;			/* Minimum value */
	CVI_U16 u16Max;			/* Maximum value */
	CVI_U16 u16Def;			/* Default value */
	CVI_U16 u16Step;		/* Step value for adjustment */
} SNS_ATTR_S;

/* Extended sensor attribute structure for 32-bit parameters */
typedef struct _SNS_ATTR_LARGE_S {
	CVI_U32	u32Min;			/* Minimum value */
	CVI_U32 u32Max;			/* Maximum value */
	CVI_U32 u32Def;			/* Default value */
	CVI_U32 u32Step;		/* Step value for adjustment */
} SNS_ATTR_LARGE_S;

/* ISP sensor state structure - Tracks current sensor status */
typedef struct _ISP_SNS_STATE_S {
	CVI_BOOL	bInit;					/* Sensor initialization status */
	CVI_BOOL	bSyncInit;				/* Sync register initialization status */
	CVI_U8		u8ImgMode;				/* Image mode */
	CVI_U8		u8Hdr;					/* HDR mode status */
	WDR_MODE_E	enWDRMode;				/* Wide Dynamic Range mode */

	ISP_SNS_SYNC_INFO_S astSyncInfo[2];	/* [0]: Sensor reg info of cur-frame; [1]: Sensor reg info of pre-frame */

	CVI_U32      au32FL[2];				/* [0]: FullLines of cur-frame; [1]: Pre FullLines of pre-frame */
	CVI_U32      u32FLStd; 				/* Standard frame lines */
	CVI_U32      au32WDRIntTime[4];		/* WDR integration time */
} ISP_SNS_STATE_S;

/* Sensor mirror/flip mode enumeration */
typedef enum _ISP_SNS_MIRRORFLIP_TYPE_E {
	ISP_SNS_NORMAL      = 0,			/* Normal mode */
	ISP_SNS_MIRROR      = 1,			/* Mirror mode */
	ISP_SNS_FLIP        = 2,			/* Flip mode */
	ISP_SNS_MIRROR_FLIP = 3,			/* Both mirror and flip */
	ISP_SNS_BUTT
} ISP_SNS_MIRRORFLIP_TYPE_E;

// Long to Short exposure mode enumeration
typedef enum _ISP_SNS_L2S_MODE_E {
	SNS_L2S_MODE_AUTO = 0,	/* Auto mode: sensor L2S distance varies by SEF integration time */
	SNS_L2S_MODE_FIX,		/* Fixed mode: sensor L2S distance is constant */
} ISP_SNS_INTTIME_MODE_E;

/* MCLK (Master Clock) attributes */
typedef struct _MCLK_ATTR_S {
	CVI_U8 u8Mclk;			/* MCLK frequency */
	CVI_BOOL bMclkEn;		/* MCLK enable flag */
} MCLK_ATTR_S;

/* Receiver initialization attributes for MIPI/TTL interface */
typedef struct _RX_INIT_ATTR_S {
	CVI_U32 MipiDev;							/* MIPI device ID */
	CVI_U32 MipiMode;							/* MIPI mode */
	CVI_S16 as16LaneId[MIPI_LANE_NUM + 1];		/* MIPI lane IDs */
	CVI_S16 as16FuncId[TTL_PIN_FUNC_NUM];		/* TTL pin function IDs */
	CVI_S8  as8PNSwap[MIPI_LANE_NUM + 1];		/* P/N swap configuration */
	MCLK_ATTR_S stMclkAttr;						/* MCLK attributes */
	CVI_BOOL hsettlen;							/* MIPI HS settle enable */
	CVI_U8 hsettle;								/* MIPI HS settle time */
} RX_INIT_ATTR_S;

/* Sensor bridge multiplexer mode enumeration */
typedef enum _SNS_BDG_MUX_MODE_E {
	SNS_BDG_MUX_NONE = 0,	/* sensor bridge mux is disabled */
	SNS_BDG_MUX_2,			/* sensor bridge mux 2 input */
	SNS_BDG_MUX_3,			/* sensor bridge mux 3 input */
	SNS_BDG_MUX_4,			/* sensor bridge mux 4 input */
} SNS_BDG_MUX_MODE_E;

/* ISP initialization attributes */
typedef struct _ISP_INIT_ATTR_S {
	CVI_U32 u32ExpTime;					/* Exposure time */
	CVI_U32 u32AGain;					/* Analog gain */
	CVI_U32 u32DGain;					/* Digital gain */
	CVI_U32 u32ISPDGain;				/* ISP digital gain */
	CVI_U32 u32Exposure;				/* Overall exposure value */
	CVI_U32 u32LinesPer500ms;			/* Lines per 500ms */
	CVI_U32 u32PirisFNO;				/* P-iris F-number */
	CVI_U16 u16WBRgain;					/* White balance red gain */
	CVI_U16 u16WBGgain;					/* White balance green gain */
	CVI_U16 u16WBBgain;					/* White balance blue gain */
	CVI_U16 u16SampleRgain;				/* Sample red gain */
	CVI_U16 u16SampleBgain;				/* Sample blue gain */
	CVI_U16 u16UseHwSync;				/* Hardware sync usage flag */
	ISP_SNS_GAIN_MODE_E enGainMode;		/* Gain mode */
	ISP_SNS_INTTIME_MODE_E enL2SMode;	/* L2S mode */
	SNS_BDG_MUX_MODE_E enSnsBdgMuxMode;	/* Sensor bridge mux mode */
} ISP_INIT_ATTR_S;

/* ISP sensor object structure - Contains function pointers for sensor operations */
typedef struct _ISP_SNS_OBJ_S {
	// Callback registration functions
	CVI_S32 (*pfnRegisterCallback)(VI_PIPE ViPipe, ALG_LIB_S *, ALG_LIB_S *);				// Register sensor callback functions
	CVI_S32 (*pfnUnRegisterCallback)(VI_PIPE ViPipe, ALG_LIB_S *, ALG_LIB_S *);				// Unregister sensor callback functions

	// Sensor control functions
	CVI_S32 (*pfnSetBusInfo)(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo);			// Set sensor bus information
	CVI_VOID (*pfnStandby)(VI_PIPE ViPipe);													// Set sensor to standby mode
	CVI_VOID (*pfnRestart)(VI_PIPE ViPipe);													// Restart sensor
	CVI_VOID (*pfnMirrorFlip)(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);	// Set mirror/flip mode
	CVI_S32 (*pfnWriteReg)(VI_PIPE ViPipe, CVI_S32 s32Addr, CVI_S32 s32Data);				// Write sensor register
	CVI_S32 (*pfnReadReg)(VI_PIPE ViPipe, CVI_S32 s32Addr);									// Read sensor register
	CVI_S32 (*pfnSetInit)(VI_PIPE ViPipe, ISP_INIT_ATTR_S *);								// Initialize sensor settings
	CVI_S32 (*pfnPatchRxAttr)(VI_PIPE ViPipe, RX_INIT_ATTR_S *);							// Patch receiver attributes
	CVI_VOID (*pfnPatchI2cAddr)(VI_PIPE ViPipe, CVI_S32 s32I2cAddr);						// Update I2C address
	CVI_S32 (*pfnGetRxAttr)(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *);						// Get receiver attributes
	CVI_S32 (*pfnExpSensorCb)(ISP_SENSOR_EXP_FUNC_S *);										// Exposure sensor callback
	CVI_S32 (*pfnExpAeCb)(AE_SENSOR_EXP_FUNC_S *);											// Auto exposure callback
	CVI_S32 (*pfnSnsProbe)(VI_PIPE ViPipe); 												// Probe sensor presence
} ISP_SNS_OBJ_S;

// External declarations for various sensor objects, grouped by manufacturer

// BG Series
extern ISP_SNS_OBJ_S stSnsBG0808_Obj;			// BG0808 sensor object

// GalaxyCore (GC) Series
extern ISP_SNS_OBJ_S stSnsGc02m1_Obj;			// GC02M1 sensor object
extern ISP_SNS_OBJ_S stSnsGc1054_Obj;			// GC1054 sensor object
extern ISP_SNS_OBJ_S stSnsGc2053_Obj;			// GC2053 sensor object
extern ISP_SNS_OBJ_S stSnsGc2053_Slave_Obj;		// GC2053 slave mode sensor object
extern ISP_SNS_OBJ_S stSnsGc2053_1l_Obj;		// GC2053 1-lane mode sensor object
extern ISP_SNS_OBJ_S stSnsGc2093_Obj;			// GC2093 sensor object
extern ISP_SNS_OBJ_S stSnsGc2093_Slave_Obj;		// GC2093 slave mode sensor object
extern ISP_SNS_OBJ_S stSnsGc2145_Obj;			// GC2145 sensor object
extern ISP_SNS_OBJ_S stSnsGc4023_Obj;			// GC4023 sensor object
extern ISP_SNS_OBJ_S stSnsGc4653_Obj;			// GC4653 sensor object
extern ISP_SNS_OBJ_S stSnsGc4653_Slave_Obj;		// GC4653 slave mode sensor object
extern ISP_SNS_OBJ_S stSnsGc8613_Obj;			// GC8613 sensor object

// N Series and NC Series
extern ISP_SNS_OBJ_S stSnsN5_Obj;				// N5 sensor object
extern ISP_SNS_OBJ_S stSnsN6_Obj;				// N6 sensor object
extern ISP_SNS_OBJ_S stSnsNC021_Obj;			// NC021 sensor object

// AR Series
extern ISP_SNS_OBJ_S stSnsAR2020_Obj;			// AR2020 sensor object

// OnSemi (OS) Series
extern ISP_SNS_OBJ_S stSnsOs02d10_Obj;			// OS02D10 sensor object
extern ISP_SNS_OBJ_S stSnsOs02d10_Slave_Obj;	// OS02D10 slave mode sensor object
extern ISP_SNS_OBJ_S stSnsOs02k10_Slave_Obj;	// OS02K10 slave mode sensor object
extern ISP_SNS_OBJ_S stSnsOs04a10_Obj;			// OS04A10 sensor object
extern ISP_SNS_OBJ_S stSnsOs04c10_Obj;			// OS04C10 sensor object
extern ISP_SNS_OBJ_S stSnsOs04c10_Slave_Obj;	// OS04C10 slave mode sensor object
extern ISP_SNS_OBJ_S stSnsOs04e10_Obj;			// OS04E10 sensor object
extern ISP_SNS_OBJ_S stSnsOs05a20_Obj;			// OS05A20 sensor object
extern ISP_SNS_OBJ_S stSnsOs08a20_Obj;			// OS08A20 sensor object
extern ISP_SNS_OBJ_S stSnsOs08a20_Slave_Obj;	// OS08A20 slave mode sensor object
extern ISP_SNS_OBJ_S stSnsOs08b10_Obj;			// OS08B10 sensor object

// OmniVision (OV) Series
extern ISP_SNS_OBJ_S stSnsOv2736_Obj;			// OV2736 sensor object
extern ISP_SNS_OBJ_S stSnsOv4689_Obj;			// OV4689 sensor object
extern ISP_SNS_OBJ_S stSnsOv6211_Obj;			// OV6211 sensor object
extern ISP_SNS_OBJ_S stSnsOv7251_Obj;			// OV7251 sensor object
extern ISP_SNS_OBJ_S stSnsOv9282_Obj;			// OV9282 sensor object

// PICO Series
extern ISP_SNS_OBJ_S stSnsPICO384_Obj;			// PICO384 sensor object
extern ISP_SNS_OBJ_S stSnsPICO640_Obj;			// PICO640 sensor object

// PR Series
extern ISP_SNS_OBJ_S stSnsPR2020_Obj;			// PR2020 sensor object
extern ISP_SNS_OBJ_S stSnsPR2100_Obj;			// PR2100 sensor object

// SmartSens (SC) Series
extern ISP_SNS_OBJ_S stSnsSC020HGS_Obj;			// SC020HGS sensor object
extern ISP_SNS_OBJ_S stSnsSC035GS_Obj;			// SC035GS sensor object
extern ISP_SNS_OBJ_S stSnsSC035GS_1L_Obj;		// SC035GS 1-lane mode sensor object
extern ISP_SNS_OBJ_S stSnsSC035HGS_Obj;			// SC035HGS sensor object
extern ISP_SNS_OBJ_S stSnsSC200AI_Obj;			// SC200AI sensor object
extern ISP_SNS_OBJ_S stSnsSC233HGS_Obj;			// SC233HGS sensor object
extern ISP_SNS_OBJ_S stSnsSC301IOT_Obj;			// SC301IOT sensor object
extern ISP_SNS_OBJ_S stSnsSC401AI_Obj;			// SC401AI sensor object
extern ISP_SNS_OBJ_S stSnsSC438AI_Obj;			// SC438AI sensor object
extern ISP_SNS_OBJ_S stSnsSC500AI_Obj;			// SC500AI sensor object
extern ISP_SNS_OBJ_S stSnsSC501AI_2L_Obj;		// SC501AI 2-lane mode sensor object
extern ISP_SNS_OBJ_S stSnsSC531AI_2L_Obj;		// SC531AI 2-lane mode sensor object
extern ISP_SNS_OBJ_S stSnsSC850SL_Obj;			// SC850SL sensor object
extern ISP_SNS_OBJ_S stSnsSC1330_Obj;			// SC1330 sensor object
extern ISP_SNS_OBJ_S stSnsSC3332_Obj;			// SC3332 sensor object
extern ISP_SNS_OBJ_S stSnsSC3335_Obj;			// SC3335 sensor object
extern ISP_SNS_OBJ_S stSnsSC3335_Slave_Obj;		// SC3335 slave mode sensor object
extern ISP_SNS_OBJ_S stSnsSC3336_Obj;			// SC3336 sensor object
extern ISP_SNS_OBJ_S stSnsSC2335_Obj;			// SC2335 sensor object
extern ISP_SNS_OBJ_S stSnsSC4210_Obj;			// SC4210 sensor object
extern ISP_SNS_OBJ_S stSnsSC4336_Obj;			// SC4336 sensor object
extern ISP_SNS_OBJ_S stSnsSC4336P_Obj;			// SC4336P sensor object
extern ISP_SNS_OBJ_S stSnsSC4336P_SLAVE_Obj;	// SC4336P slave mode sensor object
extern ISP_SNS_OBJ_S stSnsSC8238_Obj;			// SC8238 sensor object

// F Series
extern ISP_SNS_OBJ_S stSnsF23_Obj;				// F23 sensor object
extern ISP_SNS_OBJ_S stSnsF35_Obj;				// F35 sensor object
extern ISP_SNS_OBJ_S stSnsF35_Slave_Obj;		// F35 slave mode sensor object
extern ISP_SNS_OBJ_S stSnsF37P_Obj;				// F37P sensor object

// H, K, Q Series
extern ISP_SNS_OBJ_S stSnsH65_Obj;				// H65 sensor object
extern ISP_SNS_OBJ_S stSnsK06_Obj;				// K06 sensor object
extern ISP_SNS_OBJ_S stSnsQ03_Obj;				// Q03 sensor object

// Sony IMX Series
extern ISP_SNS_OBJ_S stSnsImx290_2l_Obj;		// IMX290 2-lane mode sensor object
extern ISP_SNS_OBJ_S stSnsImx307_Obj;			// IMX307 sensor object
extern ISP_SNS_OBJ_S stSnsImx307_Slave_Obj;		// IMX307 slave mode sensor object
extern ISP_SNS_OBJ_S stSnsImx307_2l_Obj;		// IMX307 2-lane mode sensor object
extern ISP_SNS_OBJ_S stSnsImx307_Sublvds_Obj;	// IMX307 Sub-LVDS mode sensor object
extern ISP_SNS_OBJ_S stSnsImx327_Obj;			// IMX327 sensor object
extern ISP_SNS_OBJ_S stSnsImx327_Slave_Obj;		// IMX327 slave mode sensor object
extern ISP_SNS_OBJ_S stSnsImx327_2l_Obj;		// IMX327 2-lane mode sensor object
extern ISP_SNS_OBJ_S stSnsImx327_fpga_Obj;		// IMX327 FPGA mode sensor object
extern ISP_SNS_OBJ_S stSnsImx327_Sublvds_Obj;	// IMX327 Sub-LVDS mode sensor object
extern ISP_SNS_OBJ_S stSnsImx334_Obj;			// IMX334 sensor object
extern ISP_SNS_OBJ_S stSnsImx335_Obj;			// IMX335 sensor object
extern ISP_SNS_OBJ_S stSnsImx347_Obj;			// IMX347 sensor object
extern ISP_SNS_OBJ_S stSnsImx385_Obj;			// IMX385 sensor object
extern ISP_SNS_OBJ_S stSnsImx412_Obj;			// IMX412 sensor object
extern ISP_SNS_OBJ_S stSnsImx415_Obj;			// IMX415 sensor object
extern ISP_SNS_OBJ_S stSnsImx585_Obj;			// IMX585 sensor object
extern ISP_SNS_OBJ_S stSnsImx900_Obj;			// IMX900 sensor object

// Other Manufacturers
extern ISP_SNS_OBJ_S stSnsTP2850_Obj;			// TP2850 sensor object
extern ISP_SNS_OBJ_S stSnsTP2860_Obj;			// TP2860 sensor object
extern ISP_SNS_OBJ_S stSnsMCS369_Obj;			// MCS369 sensor object
extern ISP_SNS_OBJ_S stSnsMCS369Q_Obj;			// MCS369Q sensor object
extern ISP_SNS_OBJ_S stSnsMM308M2_Obj;			// MM308M2 sensor object
extern ISP_SNS_OBJ_S stSnsLT6911_Obj;			// LT6911 sensor object

/* Utility macros for pointer validation and memory management */
#define CMOS_CHECK_POINTER(ptr)\
	do {\
		if (ptr == CVI_NULL) {\
			syslog(LOG_ERR, "Null Pointer! : %s\n", #ptr);\
			return CVI_ERR_VI_INVALID_NULL_PTR;\
		} \
	} while (0)	/* Checks for null pointer and returns error if found */

#define CMOS_CHECK_POINTER_VOID(ptr)\
	do {\
		if (ptr == CVI_NULL) {\
			syslog(LOG_ERR, "Null Pointer! : %s\n", #ptr);\
			return;\
		} \
	} while (0)	/* Checks for null pointer in void functions */

#define SENSOR_FREE(ptr)\
	do {\
		if (ptr != CVI_NULL) {\
			free(ptr);\
			ptr = CVI_NULL;\
		} \
	} while (0)	/* Safely frees memory and nullifies the pointer */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __CVI_SNS_CTRL_H__ */

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

typedef enum _ISP_SNS_SYNCMODE_TYPE_E {	/* Master and salve function of sensor */
	ISP_SNS_SLAVE_BY_FSYNC			= 0,
	ISP_SNS_MASTER_BY_FSYNC			= 1,
	ISP_SNS_NORMAL_MODE				= 2,
	ISP_SNS_SLAVE_BY_HIGH_ACTIVE	= 3,
	ISP_SNS_SLAVE_BY_LOW_ACTIVE		= 4,
} ISP_SNS_SYNCMODE_TYPE_E;

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

// Structure representing the AHD (Analog High Definition) object.
typedef struct _SNS_AHD_OBJ_S {
	CVI_S32 (*pfnAhdInit)(VI_PIPE ViPipe, bool isFirstInit); // Initialize AHD sensor.
	CVI_S32 (*pfnAhdDeinit)(VI_PIPE ViPipe); // Deinitialize AHD sensor.
	CVI_S32 (*pfnGetAhdMode)(VI_PIPE ViPipe); // Get current AHD mode.
	CVI_S32 (*pfnSetAhdMode)(VI_PIPE ViPipe, CVI_S32 astAhdMode); // Set AHD mode.
	CVI_S32 (*pfnSetAhdBusInfo)(VI_PIPE ViPipe, CVI_S32 astI2cDev); // Set AHD communication bus information.
	CVI_S32 (*pfnDetectAhdStatus)(VI_PIPE ViPipe, CVI_S32 ahdOldType, CVI_S32 *ahdType); // Detect AHD status.
} SNS_AHD_OBJ_S;

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
	CVI_S32 (*pfnAHDCb)(SNS_AHD_OBJ_S *); 													// AHD callback
	CVI_S32 (*pfnSnsProbe)(VI_PIPE ViPipe); 												// Probe sensor presence
} ISP_SNS_OBJ_S;

// External declarations for various sensor objects, grouped by manufacturer

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

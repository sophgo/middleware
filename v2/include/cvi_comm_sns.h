/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_common_sns.h
 * Description: Header file defining sensor-related data structures and interfaces
 */

#ifndef _CVI_COMM_SNS_H_
#define _CVI_COMM_SNS_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_type.h>
#include <cvi_defines.h>
#include "cvi_debug.h"
#include "cvi_comm_isp.h"

// ++++++++ If you want to change these interfaces, please contact the isp team. ++++++++

// Noise profile related parameter definitions
#define NOISE_PROFILE_CHANNEL_NUM 4		/*Number of noise profile channels*/
#define NOISE_PROFILE_LEVEL_NUM 2		/*Number of noise profile levels*/
#define NOISE_PROFILE_ISO_NUM 16		/*Number of ISO settings for noise profile*/
#define USE_USER_SEN_DRIVER 1			/*Use user sensor driver*/

/*Sensor image mode configuration structure*/
typedef struct _ISP_CMOS_SENSOR_IMAGE_MODE_S {
	CVI_U16 u16Width;			/*Image width*/
	CVI_U16 u16Height;			/*Image height*/
	CVI_FLOAT f32Fps;			/*Frame rate*/
	CVI_U8 u8SnsMode;			/*Sensor working mode*/
	CVI_U8 u8LaneNum;			/*Number of MIPI lanes*/
	CVI_U8 u8EnableMaster;		/*Master/Slave enable flag*/
} ISP_CMOS_SENSOR_IMAGE_MODE_S;

/*Black level configuration structure*/
typedef struct _ISP_CMOS_BLACK_LEVEL_S {
	CVI_BOOL bUpdate;					/*Update flag*/
	ISP_BLACK_LEVEL_ATTR_S blcAttr;		/*Black level attributes*/
} ISP_CMOS_BLACK_LEVEL_S;

/*Sensor attribute information structure*/
typedef struct _ISP_SNS_ATTR_INFO_S {
	CVI_U32 eSensorId;					/*Sensor ID*/
} ISP_SNS_ATTR_INFO_S;

/*Noise calibration configuration structure*/
typedef struct cviISP_CMOS_NOISE_CALIBRATION_S {
	CVI_FLOAT CalibrationCoef[NOISE_PROFILE_ISO_NUM][NOISE_PROFILE_CHANNEL_NUM][NOISE_PROFILE_LEVEL_NUM];	/*Calibration coefficients*/
} ISP_CMOS_NOISE_CALIBRATION_S;

/*ISP default configuration structure*/
typedef struct _ISP_CMOS_DEFAULT_S {
	ISP_CMOS_NOISE_CALIBRATION_S stNoiseCalibration;	/*Noise calibration configuration*/
} ISP_CMOS_DEFAULT_S;

/*Sensor operation function structure*/
typedef struct _ISP_SENSOR_EXP_FUNC_S {
	CVI_VOID (*pfn_cmos_sensor_init)(VI_PIPE ViPipe);															/*Sensor initialization function*/
	CVI_VOID (*pfn_cmos_sensor_exit)(VI_PIPE ViPipe);															/*Sensor exit function*/
	CVI_VOID (*pfn_cmos_sensor_global_init)(VI_PIPE ViPipe);													/*Sensor global initialization function*/
	CVI_S32 (*pfn_cmos_set_image_mode)(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode);		/*Set image mode*/
	CVI_S32 (*pfn_cmos_set_wdr_mode)(VI_PIPE ViPipe, CVI_U8 u8Mode);											/*Set wide dynamic range mode*/

	/* the algs get data which is associated with sensor, except 3a */
	CVI_S32 (*pfn_cmos_get_isp_default)(VI_PIPE ViPipe, ISP_CMOS_DEFAULT_S *pstDef);							/*Get ISP default configuration*/
	CVI_S32 (*pfn_cmos_get_isp_black_level)(VI_PIPE ViPipe, ISP_CMOS_BLACK_LEVEL_S *pstBlackLevel);				/*Get black level configuration*/
	CVI_S32 (*pfn_cmos_get_sns_reg_info)(VI_PIPE ViPipe, ISP_SNS_SYNC_INFO_S *pstSnsRegsInfo);					/*Get register information*/

	/* the function of sensor set pixel detect */
	//CVI_VOID (*pfn_cmos_set_pixel_detect)(VI_PIPE ViPipe, bool bEnable);
} ISP_SENSOR_EXP_FUNC_S;

/*MCLK frequency enumeration*/
typedef enum _MCLK_FREQ_E {
	MCLK_FREQ_NONE = 0,        /*No MCLK frequency*/
	MCLK_FREQ_37P125M,         /*37.125MHz MCLK frequency*/
	MCLK_FREQ_25M,             /*25MHz MCLK frequency*/
	MCLK_FREQ_27M,             /*27MHz MCLK frequency*/
	MCLK_FREQ_NUM              /*Number of MCLK frequency options*/
} MCLK_FREQ_E;

/*Sensor MCLK configuration structure*/
typedef struct _SNS_MCLK_S {
	CVI_U32     u8Cam;         /*Camera ID*/
	MCLK_FREQ_E enFreq;        /*MCLK frequency selection*/
} SNS_MCLK_S;

/*Sensor registration structure*/
typedef struct bmISP_SENSOR_REGISTER_S {
	ISP_SENSOR_EXP_FUNC_S stSnsExp;		/*Sensor operation functions*/
} ISP_SENSOR_REGISTER_S;


// -------- If you want to change these interfaces, please contact the isp team. --------

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* _CVI_COMM_SNS_H_ */
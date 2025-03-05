/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_comm_aio.h
 * Description: basic audio in out definition
 */

#ifndef __CVI_COMM_AIO_H__
#define __CVI_COMM_AIO_H__

#include <cvi_common.h>
#include <cvi_errno.h>


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#define CVI_MAX_AUDIO_FRAME_NUM    300       /* max count of audio frame in Buffer */
#define CVI_AUD_MAX_VOICE_POINT_NUM    1280      /* max sample per frame for voice encode */
#define CVI_AUD_MAX_AUDIO_POINT_NUM    2048     /* max sample per frame for all encoder */
#define CVI_AUD_MAX_CHANNEL_NUM      8         /* Maximum number of audio channels supported */
#define CVI_MAX_AUDIO_STREAM_LEN   (4 * 4096)  /* Maximum length of an audio stream in bytes */



#define MAX_AUDIO_FILE_PATH_LEN	256           /* Maximum length of the file path for an audio file */
#define MAX_AUDIO_FILE_NAME_LEN	256           /* Maximum length of the file name for an audio file */
#define MAX_AUDIO_VQE_CUSTOMIZE_NAME	64      /* Maximum length of the name for a custom VQE setting */

/*The VQE EQ Band num.*/
#define VQE_EQ_BAND_NUM  10

#define AI_RECORDVQE_MASK_HPF     0x1  /* High Pass Filter (HPF) Enable Mask */
#define AI_RECORDVQE_MASK_RNR     0x2  /* Noise Reduction (RNR) Enable Mask */
#define AI_RECORDVQE_MASK_HDR     0x4  /* High Dynamic Range (HDR) Enable Mask */
#define AI_RECORDVQE_MASK_DRC     0x8  /* Dynamic Range Compression (DRC) Enable Mask */
#define AI_RECORDVQE_MASK_EQ      0x10 /* Equalizer (EQ) Enable Mask */
#define AI_RECORDVQE_MASK_AGC     0x20 /* Automatic Gain Control (AGC) Enable Mask */


//#define AI_TALKVQE_MASK_HPF		0x1//not support
#define AI_TALKVQE_MASK_AEC		0x3 /* Acoustic Echo Cancellation (AEC) Enable Mask */
#define AI_TALKVQE_MASK_ANR		0x4 /* Acoustic Noise Reduction (ANR) Enable Mask */
#define AI_TALKVQE_MASK_AGC		0x8 /* Automatic Gain Control (AGC) Enable Mask */
#define AI_TALKVQE_MASK_NOTCH_FILTER	0x30 /* Notch Filter Enable Mask */
//#define AI_TALKVQE_MASK_EQ		0x10//not supoort
#define NEXT_SSP_ALGO 1


/*  LP AEC Control */
#define  LP_AEC_ENABLE 0x1  /* bit 0 */
/*  NLP AES Control */
#define  NLP_AES_ENABLE 0x2  /* bit 1 */
/*  NR Control */
#define  NR_ENABLE 0x4  /* bit 2 */
/*  AGC Control */
#define AGC_ENABLE 0x8  /* bit 3 */
/*  Notch Filter Control */
#define NOTCH_ENABLE 0x10  /* bit 4 */
/*  DC Filter Control */
#define DCREMOVER_ENABLE 0x20  /* bit 5 */
/*  DG Control */
#define DG_ENABLE 0x40  /* bit 6 */
/*  Delay Control */
#define DELAY_ENABLE 0x80  /* bit 7 */

//control for AO_VQE
/*  AGC Control in SPK Path */
#define SPK_AGC_ENABLE 0x1  /* bit 0 */
#define SPK_EQ_ENABLE 0x2  /* bit 1 */


/* DNVQE Control Flags */
#define DNVQE_HPFILTER				0x1  /* bit 0: High Pass Filter */
#define DNVQE_EQ					0x2  /* bit 1: Equalizer */
#define DNVQE_DRC_EXPANDER_COMPRESS 0x4  /* bit 2: DRC Expander/Compressor */
#define DNVQE_DRC_LIMITER			0x8  /* bit 3: DRC Limiter */

#define CVI_MAX_AI_DEVICE_ID_NUM 5   /* Maximum number of AI device ID */
#define CVI_MAX_AI_CARD_ID_NUM 5   /* Maximum number of AI card ID */
#define CVI_MAX_AO_DEVICE_ID_NUM 5   /* Maximum number of AO device ID */
#define CVI_MAX_AO_CARD_ID_NUM 5   /* Maximum number of AO card ID */

#define CHECK_AI_DEVID_VALID(x) \
	((((x) > (CVI_MAX_AI_DEVICE_ID_NUM-1))) ? 1:0)
/* Check if the AI card ID is valid */
#define CHECK_AI_CARD_VALID(x) \
	((((x) > (CVI_MAX_AI_CARD_ID_NUM-1))) ? 1:0)
/* Check if the AO device ID is valid */
#define CHECK_AO_DEVID_VALID(x) \
	((((x) > (CVI_MAX_AO_DEVICE_ID_NUM-1))) ? 1:0)
/* Check if the AO card ID is valid */
#define CHECK_AO_CARD_VALID(x) \
	((((x) > (CVI_MAX_AO_CARD_ID_NUM-1))) ? 1:0)
/* Check if the AENC device ID is valid */
#define CHECK_AENC_DEVID_VALID(x) \
	((((x) > (AENC_MAX_CHN_NUM-1))) ? 1:0)
/* Check if the ADEC device ID is valid */
#define CHECK_ADEC_DEVID_VALID(x) \
	((((x) > (ADEC_MAX_CHN_NUM-1))) ? 1:0)


typedef enum _AUDIO_SAMPLE_RATE_E {
	AUDIO_SAMPLE_RATE_8000   = 8000,    /* 8K samplerate*/
	/* 12K samplerate(not support in this version)*/
	AUDIO_SAMPLE_RATE_11025  = 11025,   /* 11.025K samplerate*/
	AUDIO_SAMPLE_RATE_16000  = 16000,   /* 16K samplerate*/
	AUDIO_SAMPLE_RATE_22050  = 22050,   /* 22.050K samplerate*/
	AUDIO_SAMPLE_RATE_24000  = 24000,   /* 24K samplerate*/
	AUDIO_SAMPLE_RATE_32000  = 32000,   /* 32K samplerate*/
	AUDIO_SAMPLE_RATE_44100  = 44100,   /* 44.1K samplerate*/
	AUDIO_SAMPLE_RATE_48000  = 48000,   /* 48K samplerate*/
	AUDIO_SAMPLE_RATE_64000  = 64000,   /* 64K samplerate*/
	/* 96K samplerate is not support in cv183x series*/
	AUDIO_SAMPLE_RATE_BUTT,		     	/* Used for boundary checking */
} AUDIO_SAMPLE_RATE_E;

typedef enum _AUDIO_BIT_WIDTH_E {
	AUDIO_BIT_WIDTH_8   = 0,   /* 8bit width */
	AUDIO_BIT_WIDTH_16  = 1,   /* 16bit width*/
	AUDIO_BIT_WIDTH_24  = 2,   /* 24bit width*/
	AUDIO_BIT_WIDTH_32  = 3,   /* 32bit width*/
	AUDIO_BIT_WIDTH_BUTT, /* boundary check */
} AUDIO_BIT_WIDTH_E;

typedef enum _AIO_MODE_E {
	AIO_MODE_I2S_MASTER  = 0,   /* AIO I2S master mode */
	AIO_MODE_I2S_SLAVE,         /* AIO I2S slave mode */
	AIO_MODE_PCM_SLAVE_STD,     /* AIO PCM slave standard mode */
	AIO_MODE_PCM_SLAVE_NSTD,    /* AIO PCM slave non-standard mode */
	AIO_MODE_PCM_MASTER_STD,    /* AIO PCM master standard mode */
	AIO_MODE_PCM_MASTER_NSTD,   /* AIO PCM master non-standard mode */
	AIO_MODE_BUTT			    /* boundary check */
} AIO_MODE_E;

typedef enum {
	AIO_I2STYPE_INNERCODEC = 0, /* AIO I2S connect inner audio CODEC */
	AIO_I2STYPE_INNERHDMI,      /* AIO I2S connect Inner HDMI */
	AIO_I2STYPE_EXTERN,         /* AIO I2S connect extern hardware */
} AIO_I2STYPE_E;

typedef enum _AIO_SOUND_MODE_E {
	AUDIO_SOUND_MODE_MONO   = 0, /*mono*/
	AUDIO_SOUND_MODE_STEREO = 1, /*stereo only support interlace mode*/
	AUDIO_SOUND_MODE_BUTT        /*boundary check*/
} AUDIO_SOUND_MODE_E;

/*An example of the packing scheme for G726-32 codewords is as */
/*shown, and bit A3 is the least significant */
/*bit of the first codeword:*/
/*RTP G726-32:*/
/*0                   1*/
/*0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5*/
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
/*|B B B B|A A A A|D D D D|C C C C| ...*/
/*|0 1 2 3|0 1 2 3|0 1 2 3|0 1 2 3|*/
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
/*MEDIA G726-32:*/
/*0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5*/
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
/*|A A A A|B B B B|C C C C|D D D D| ...*/
/*|3 2 1 0|3 2 1 0|3 2 1 0|3 2 1 0|*/
/*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
typedef enum _G726_BPS_E {
	G726_16K = 0,       /* G726 16kbps, see RFC3551.txt  4.5.4 G726-16 */
	G726_24K,           /* G726 24kbps, see RFC3551.txt  4.5.4 G726-24 */
	G726_32K,           /* G726 32kbps, see RFC3551.txt  4.5.4 G726-32 */
	G726_40K,           /* G726 40kbps, see RFC3551.txt  4.5.4 G726-40 */
	MEDIA_G726_16K,     /* G726 16kbps for ASF ... */
	MEDIA_G726_24K,     /* G726 24kbps for ASF ... */
	MEDIA_G726_32K,     /* G726 32kbps for ASF ... */
	MEDIA_G726_40K,     /* G726 40kbps for ASF ... */
	G726_BUTT,          /* Used for boundary checking */
} G726_BPS_E;

typedef enum _ADPCM_TYPE_E {
	/* see DVI4 diiffers in three respects from the IMA ADPCM at RFC3551.txt 4.5.1 DVI4 */

	ADPCM_TYPE_DVI4 = 0, /* 32kbps ADPCM(DVI4) for RTP */
	ADPCM_TYPE_IMA, /* 32kbps ADPCM(IMA),NOTICE:point num must be 161/241/321/481 */
	ADPCM_TYPE_ORG_DVI4, /* Original DVI4 ADPCM type */
	ADPCM_TYPE_BUTT, /* Used for boundary checking */
} ADPCM_TYPE_E;


typedef struct _AIO_ATTR_S {
	AUDIO_SAMPLE_RATE_E enSamplerate;	/* sample rate */
	AUDIO_BIT_WIDTH_E   enBitwidth;		/* bitwidth */
	AIO_MODE_E          enWorkmode;	/* master or slave mode */
	AUDIO_SOUND_MODE_E  enSoundmode;	/* momo or steror */
	CVI_U32  u32EXFlag;
	/* expand 8bit to 16bit,use AI_EXPAND(only valid for AI 8bit),*/
	/*use AI_CUT(only valid for extern Codec for 24bit) */
	CVI_U32 u32FrmNum;
	/* frame num in buf[2,CVI_MAX_AUDIO_FRAME_NUM] */
	CVI_U32 u32PtNumPerFrm;
	/* point num per frame (80/160/240/320/480/1024/2048) */
	/*(ADPCM IMA should add 1 point, AMR only support 160) */
	CVI_U32 u32ChnCnt;	/* channel number on FS, valid value:1/2/4/8 */
	CVI_U32 u32ClkSel;	/* 0: AI and AO clock is separate*/
	/* 1: AI and AO clock is inseparate, AI use AO's clock*/
	AIO_I2STYPE_E enI2sType;	/* i2s type */
} AIO_ATTR_S;

typedef struct _AI_CHN_PARAM_S {
	CVI_U32 u32UsrFrmDepth; /* user frame depth */
} AI_CHN_PARAM_S;

typedef struct _AUDIO_FRAME_S {
	AUDIO_BIT_WIDTH_E   enBitwidth;/*audio frame bitwidth*/
	AUDIO_SOUND_MODE_E  enSoundmode;/*audio frame momo or stereo mode*/
	CVI_U8  * u64VirAddr[2];			  /*audio frame vir addr*/
	CVI_U64  u64PhyAddr[2];               /*audio frame phy addr*/
	CVI_U64  u64TimeStamp;                /*audio frame timestamp*/
	CVI_U32  u32Seq;                      /*audio frame seq*/
	CVI_U32  u32Len;                      /*data length per channel in frame*/
	CVI_U32  u32PoolId[2];                /*audio frame pool id*/
} AUDIO_FRAME_S;

typedef struct _AEC_FRAME_S {
	AUDIO_FRAME_S   stRefFrame;    /* AEC reference audio frame */
	CVI_BOOL         bValid;        /* whether frame is valid */
	CVI_BOOL         bSysBind;       /* whether is sysbind */
} AEC_FRAME_S;


typedef struct _AUDIO_FRAME_INFO_S {
	AUDIO_FRAME_S *pstFrame;/*frame ptr*/
	CVI_U32         u32Id;   /*frame id*/
} AUDIO_FRAME_INFO_S;

typedef struct _AUDIO_STREAM_S {
	/* CVI_U8 ATTRIBUTE *pStream;the virtual address of stream */
	/* CVI_U64 ATTRIBUTE u64PhyAddr; the physics address of stream */
	CVI_U8  *pStream;         /* the virtual address of stream */
	CVI_U64  u64PhyAddr;      /* the physics address of stream */
	CVI_U32 u32Len;          /* stream length, by bytes */
	CVI_U64 u64TimeStamp;    /* frame time stamp*/
	CVI_U32 u32Seq;          /* frame seq,if stream is not a valid frame,u32Seq is 0*/
} AUDIO_STREAM_S;


typedef struct _AO_CHN_STATE_S {
	CVI_U32                  u32ChnTotalNum;    /* total number of channel buffer */
	CVI_U32                  u32ChnFreeNum;     /* free number of channel buffer */
	CVI_U32                  u32ChnBusyNum;     /* busy number of channel buffer */
} AO_CHN_STATE_S;

typedef enum _AUDIO_TRACK_MODE_E {
    AUDIO_TRACK_NORMAL      = 0,  /* Normal audio track */
    AUDIO_TRACK_BOTH_LEFT   = 1,  /* Both channels play left audio */
    AUDIO_TRACK_BOTH_RIGHT  = 2,  /* Both channels play right audio */
    AUDIO_TRACK_EXCHANGE    = 3,  /* Exchange left and right audio channels */
    AUDIO_TRACK_MIX         = 4,  /* Mix both left and right audio channels */
    AUDIO_TRACK_LEFT_MUTE   = 5,  /* Mute left audio channel */
    AUDIO_TRACK_RIGHT_MUTE  = 6,  /* Mute right audio channel */
    AUDIO_TRACK_BOTH_MUTE   = 7,  /* Mute both audio channels */

    AUDIO_TRACK_BUTT        /* End of audio track modes */
} AUDIO_TRACK_MODE_E;


typedef enum _AUDIO_FADE_RATE_E {
    AUDIO_FADE_RATE_NONE    = 0,   /* No fade */
    AUDIO_FADE_RATE_10      = 10,  /* Fade rate 10 */
    AUDIO_FADE_RATE_20      = 20,  /* Fade rate 20 */
    AUDIO_FADE_RATE_30      = 30,  /* Fade rate 30 */
    AUDIO_FADE_RATE_50      = 50,  /* Fade rate 50 */
    AUDIO_FADE_RATE_100     = 100, /* Fade rate 100 */
    AUDIO_FADE_RATE_200     = 200, /* Fade rate 200 */
    AUDIO_FADE_RATE_BUTT    = -1   /* Invalid fade rate */
} AUDIO_FADE_RATE_E;

typedef struct _AUDIO_FADE_S {
    CVI_BOOL         bFade;          /* Enable or disable fade */
    AUDIO_FADE_RATE_E enFadeInRate;  /* Fade-in rate */
    AUDIO_FADE_RATE_E enFadeOutRate; /* Fade-out rate */
} AUDIO_FADE_S;

/**Defines the configure parameters of AEC.*/
typedef struct _AI_AEC_CONFIG_S {
	CVI_U16 para_aec_filter_len;           /* the filter length of AEC, [1, 13] */
	CVI_U16 para_aes_std_thrd;                 /* the threshold of STD/DTD, [0, 39] */
	CVI_U16 para_aes_supp_coeff;           /* the residual echo suppression level in AES, [0, 100] */
} AI_AEC_CONFIG_S;

/**Defines the configure parameters of UPVQE work state.*/
typedef enum _VQE_WORKSTATE_E {
	VQE_WORKSTATE_COMMON  = 0,
	/* common environment, Applicable to the family of voice calls. */
	VQE_WORKSTATE_MUSIC   = 1,
	/* music environment , Applicable to the family of music environment. */
	VQE_WORKSTATE_NOISY   = 2,
	/* noisy environment , Applicable to the noisy voice calls.  */
} VQE_WORKSTATE_E;

/**Defines record type*/
typedef enum _VQE_RECORD_TYPE {
	VQE_RECORD_NORMAL        = 0,
	/*<double micphone recording. */
	VQE_RECORD_BUTT,  /* Used for boundary checking */
} VQE_RECORD_TYPE;


/* HDR Set CODEC GAIN Function Handle type */
typedef CVI_S32(*pFuncGainCallBack)(CVI_S32 s32SetGain);




//#define CVIAUDIO_ALGO_SSP	0x11//defalut algorithm
//#define CVIAUDIO_ALGO_SSP_NOTCH	0x12//customize algorithm with notch filter
#define CVIAUDIO_ALGO_ONLINE_PARAM  0xC0  //customize parameter from /mnt/data/audvqe.cfg
typedef struct _AUDIO_DELAY_CONFIG_S {
	/* the initial filter length of linear AEC to support up for echo tail, [1, 13] */
	CVI_U16 para_aec_init_filter_len;
	/* the digital gain target, [1, 12] */
	CVI_U16 para_dg_target;
	/* the delay sample for ref signal, [1, 3000] */
	CVI_U16 para_delay_sample;
} AUDIO_DELAY_CONFIG_S;
typedef struct _AUDIO_AGC_CONFIG_S {
	/* the max boost gain for AGC release processing, [0, 3] */
	/* para_obj.para_agc_max_gain = 1; */
	CVI_S8 para_agc_max_gain;
	/* the gain level of target high of AGC, [0, 36] */
	/* para_obj.para_agc_target_high = 2; */
	CVI_S8 para_agc_target_high;
	/* the gain  level of target low of AGC, [0, 36] */
	/* para_obj.para_agc_target_low = 6; */
	CVI_S8 para_agc_target_low;
	/* speech-activated AGC functionality, [0, 1] */
	/* para_obj.para_agc_vad_enable = 1; */
	CVI_BOOL para_agc_vad_ena;
} AUDIO_AGC_CONFIG_S;


typedef struct _AUDIO_SPK_AGC_CONFIG_S {
	/* the max boost gain for AGC release processing, [0, 3] */
	/* para_obj.para_agc_max_gain = 1; */
	CVI_S8 para_agc_max_gain;
	/* the gain level of target high of AGC, [0, 36] */
	/* para_obj.para_agc_target_high = 2; */
	CVI_S8 para_agc_target_high;
	/* the gain  level of target low of AGC, [0, 36] */
	/* para_obj.para_agc_target_low = 6; */
	CVI_S8 para_agc_target_low;
} AUDIO_SPK_AGC_CONFIG_S;

typedef struct _AUDIO_SPK_EQ_CONFIG_S {
    CVI_U16 para_spk_eq_nband; /* Number of EQ bands */
    CVI_U16 para_spk_eq_freq[5]; /* EQ band frequencies */
    CVI_U16 para_spk_eq_gain[5]; /* EQ band gains */
    CVI_U16 para_spk_eq_qfactor[5]; /* EQ band Q factors */
} AUDIO_SPK_EQ_CONFIG_S;

typedef struct _AUDIO_ANR_CONFIG_S {
	/* the coefficient of NR priori SNR tracking, [0, 20] */
	/* para_obj.para_nr_snr_coeff = 15; */
	CVI_U16 para_nr_snr_coeff;
	/* the coefficient of NR noise tracking, [0, 14] */
	/* para_obj.para_nr_noise_coeff = 2; */
	//CVI_S8 para_nr_noise_coeff;
	CVI_U16 para_nr_init_sile_time;
} AUDIO_ANR_CONFIG_S;

typedef struct _AI_TALKVQE_CONFIG_S {
    CVI_U16 para_client_config;              /* Client-specific configuration parameter */
    CVI_U32 u32OpenMask;                     /* VQE feature enable mask */
    CVI_S32 s32WorkSampleRate;               /* Sample Rate: 8KHz/16KHz. Default: 8KHz */
    // MIC IN VQE settings
    AI_AEC_CONFIG_S     stAecCfg;             /* Acoustic Echo Cancellation configuration */
    AUDIO_ANR_CONFIG_S  stAnrCfg;             /* Automatic Noise Reduction configuration */
    AUDIO_AGC_CONFIG_S  stAgcCfg;             /* Automatic Gain Control configuration */
    AUDIO_DELAY_CONFIG_S stAecDelayCfg;       /* AEC delay configuration */
	CVI_S32 s32RevMask;                       /* Reverse mask */
    CVI_S32 para_notch_freq;                  /* User can ignore this flag */
    CVI_CHAR customize[MAX_AUDIO_VQE_CUSTOMIZE_NAME]; /* Customization name */
} AI_TALKVQE_CONFIG_S;

typedef enum {
    E_FILTER_LPF, /* Low-pass filter */
    E_FILTER_HPF, /* High-pass filter */
    E_FILTER_LSF, /* Low-shelf filter */
    E_FILTER_HSF, /* High-shelf filter */
    E_FILTER_PEF, /* Peak filter */
    E_FILTER_MAX, /* Maximum filter type */
} HPF_FILTER_TYPE;

typedef struct _CVI_HPF_CONFIG_S {
	int type; /* HPF filter type */
	float f0; /* cut-off frequency */
	float Q; /* Q factor */
	float gainDb; /* gain in dB */
} CVI_HPF_CONFIG_S;

typedef struct _CVI_EQ_CONFIG_S {
    int bandIdx;      /* Index of the EQ band */
    CVI_U32 freq;    /* Frequency in Hz */
    float QValue;     /* Quality factor of the EQ band */
    float gainDb;     /* Gain in decibels for the EQ band */
} CVI_EQ_CONFIG_S;


typedef struct _CVI_DRC_COMPRESSOR_PARAM {
    CVI_U32 attackTimeMs; /* Attack time in milliseconds */
    CVI_U32 releaseTimeMs; /* Release time in milliseconds */
    CVI_U16 ratio; /* Compression ratio */
    float thresholdDb; /* Threshold level in decibels */
} CVI_DRC_COMPRESSOR_PARAM;

typedef struct _CVI_DRC_LIMITER_PARAM {
    CVI_U32 attackTimeMs;    /* Attack time in milliseconds */
    CVI_U32 releaseTimeMs;   /* Release time in milliseconds */
    float thresholdDb;        /* Threshold in decibels */
    float postGain;           /* Post-gain in decibels */
} CVI_DRC_LIMITER_PARAM;

typedef struct _CVI_DRC_EXPANDER_PARAM {
    CVI_U32 attackTimeMs; /* Attack time in milliseconds */
    CVI_U32 releaseTimeMs; /* Release time in milliseconds */
    CVI_U32 holdTimeMs; /* Hold time in milliseconds */
    CVI_U16 ratio; /* Expansion ratio */
    float thresholdDb; /* Threshold level in decibels */
    float minDb; /* Minimum level in decibels */
} CVI_DRC_EXPANDER_PARAM;

typedef struct _AO_VQE_CONFIG_S {
    CVI_U32 u32OpenMask; /* Open mask for VQE modules */
    CVI_S32 s32WorkSampleRate; /* Working sample rate */
    CVI_S32 s32channels; /* Number of channels */
	/* Sample Rate: 8KHz/16KHz default: 8KHz*/
    AUDIO_SPK_AGC_CONFIG_S stAgcCfg; /* AGC configuration */
    AUDIO_SPK_EQ_CONFIG_S stEqCfg; /* EQ configuration */
    CVI_HPF_CONFIG_S stHpfParam; /* HPF configuration */
    CVI_EQ_CONFIG_S stEqParam; /* EQ configuration */
    CVI_DRC_COMPRESSOR_PARAM stDrcCompressor; /* DRC compressor configuration */
    CVI_DRC_LIMITER_PARAM stDrcLimiter; /* DRC limiter configuration */
    CVI_DRC_EXPANDER_PARAM stDrcExpander; /* DRC expander configuration */
} AO_VQE_CONFIG_S;

/**Defines the configure parameters of Record VQE.*/
typedef struct _AI_RECORDVQE_CONFIG_S {
	CVI_U32             u32OpenMask;       /* Bitmask for enabling/disabling features */
    CVI_S32             s32WorkSampleRate; /* Sample Rate: 16KHz/48KHz */
	/* Sample Rate:16KHz/48KHz*/
    CVI_S32             s32FrameSample;    /* Number of samples per frame */
    CVI_S32             s32BytesPerSample; /* Number of bytes per sample */

	/* VQE frame length:80-4096 */
	VQE_WORKSTATE_E     enWorkstate;       /* Current work state of VQE */
    CVI_S32             s32InChNum;        /* Number of input channels */
    CVI_S32             s32OutChNum;       /* Number of output channels */
    VQE_RECORD_TYPE     enRecordType;      /* Type of recording */
    AUDIO_AGC_CONFIG_S  stAgcCfg;          /* Configuration for Automatic Gain Control (AGC) */
} AI_RECORDVQE_CONFIG_S;
/* Defines the module register configure of VQE. */
typedef struct _VQE_MODULE_CONFIG_S {
	CVI_VOID *pHandle; /* Handle of the VQE module */
} VQE_MODULE_CONFIG_S;

typedef struct _AUDIO_VQE_REGISTER_S {
    VQE_MODULE_CONFIG_S stResModCfg;     /* Configuration for the Resample module */
    VQE_MODULE_CONFIG_S stHpfModCfg;     /* Configuration for the High Pass Filter module */
    VQE_MODULE_CONFIG_S stHdrModCfg;     /* Configuration for the HDR module */
    VQE_MODULE_CONFIG_S stGainModCfg;    /* Configuration for the Gain module */

	// Record VQE
    VQE_MODULE_CONFIG_S stRecordModCfg;  /* Configuration for the Record VQE module */

	// Talk VQE
    VQE_MODULE_CONFIG_S stAecModCfg;     /* Configuration for the Acoustic Echo Cancellation module */
    VQE_MODULE_CONFIG_S stAnrModCfg;     /* Configuration for the Automatic Noise Reduction module */
    VQE_MODULE_CONFIG_S stAgcModCfg;     /* Configuration for the Automatic Gain Control module */
    VQE_MODULE_CONFIG_S stEqModCfg;      /* Configuration for the Equalizer module */

	// CviFi VQE
    VQE_MODULE_CONFIG_S stRnrModCfg;     /* Configuration for the Residual Noise Reduction module */
    VQE_MODULE_CONFIG_S stDrcModCfg;     /* Configuration for the Dynamic Range Compression module */
    VQE_MODULE_CONFIG_S stPeqModCfg;     /* Configuration for the Parametric Equalizer module */
} AUDIO_VQE_REGISTER_S;

/*Defines the configure parameters of AI saving file.*/
typedef struct _AUDIO_SAVE_FILE_INFO_S {
    CVI_BOOL    bCfg;                  /* Configuration flag (TRUE/FALSE) */
    CVI_CHAR    aFilePath[MAX_AUDIO_FILE_PATH_LEN]; /* File path where the audio is saved */
    CVI_CHAR    aFileName[MAX_AUDIO_FILE_NAME_LEN]; /* Name of the saved audio file */
    CVI_U32     u32FileSize;          /* Size of the file in KB */
} AUDIO_SAVE_FILE_INFO_S;

/*Defines whether the file is saving or not .*/
typedef struct _AUDIO_FILE_STATUS_S {
	CVI_BOOL     bSaving; /* Whether the file is saving or not */
} AUDIO_FILE_STATUS_S;

/**Defines audio clksel type*/
typedef enum _AUDIO_CLKSEL_E {
	AUDIO_CLKSEL_BASE       = 0,  /*<Audio base clk. */
	AUDIO_CLKSEL_SPARE,           /*<Audio spare clk. */

	AUDIO_CLKSEL_BUTT,            /*<The end of the enumeration. */
} AUDIO_CLKSEL_E;

/*Defines audio mode parameter.*/
typedef struct _AUDIO_MOD_PARAM_S {
	AUDIO_CLKSEL_E enClkSel; /* Audio clock select */
} AUDIO_MOD_PARAM_S;


typedef struct _cvi_wavHEADER {
	/* RIFF string */
	CVI_U8 riff[4];
	// overall size of file in bytes
	CVI_U32 overall_size;
	// WAVE string
	CVI_U8 wave[4];
	// fmt string with trailing null char
	CVI_U8 fmt_chunk_marker[4];
	// length of the format data
	CVI_U32 length_of_fmt;
	// format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
	CVI_U16 format_type;
	// no.of channels
	CVI_U16 channels;
	// sampling rate (blocks per second)
	CVI_U32 sample_rate;
	// SampleRate * NumChannels * BitsPerSample/8
	CVI_U32 byterate;
	// NumChannels * BitsPerSample/8
	CVI_U16 block_align;
	// bits per sample, 8- 8bits, 16- 16 bits etc
	CVI_U16 bits_per_sample;
	// DATA string or FLLR string
	CVI_U8 data_chunk_header[4];
	// NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
	CVI_U32 data_size;
} ST_CVI_WAV_HEADER;


typedef struct _AudioUnitTestCfg {
    CVI_S32 channels;          /* Number of audio channels */
    CVI_S32 Time_in_second;    /* Duration of the test in seconds */
    CVI_S32 sample_rate;       /* Sampling rate in Hz */
    CVI_CHAR format[64];       /* Audio format */
    CVI_S32 period_size;       /* Size of the audio period */
    CVI_S32 bitdepth;          /* Bit depth of the audio */
    CVI_S32 unit_test;         /* Flag to indicate if unit test is enabled */
    CVI_S32 s32TestMode;       /* Test mode */
    CVI_CHAR filename[256];    /* File name for the input/output audio file */
    CVI_BOOL bOptCfg;          /* Boolean flag for optional configuration */
} ST_AudioUnitTestCfg;


typedef struct _ST_CVIAUDIO_SPEEDPLAY_USR_CONFIG {
    int sampleRate;    /* Sampling rate in Hz */
    int channels;      /* Number of audio channels */
    float speed;       /* Playback speed multiplier */
    float pitch;       /* Pitch adjustment factor */
    float rate;        /* Playback rate adjustment */
    float volume;      /* Volume adjustment multiplier */
} ST_CVIAO_SPEEDPLAY_CONFIG;

extern ST_AudioUnitTestCfg  stAudTestCfg;
/* at least one parameter is illegal ,eg, an illegal enumeration value  */
#define CVI_ERR_AIO_ILLEGAL_PARAM     0xAA000001
/* using a NULL point */
#define CVI_ERR_AIO_NULL_PTR          0xAA000002
/* operation is not supported by NOW */
#define CVI_ERR_AIO_NOT_PERM          0xAA000003
/* vqe  err */
#define CVI_ERR_AIO_REGISTER_ERR      0xAA000004

/* invalid device ID */
#define CVI_ERR_AI_INVALID_DEVID     0xA0000005
/* invalid channel ID */
#define CVI_ERR_AI_INVALID_CHNID     0xA0000006
/* at least one parameter is illegal ,eg, an illegal enumeration value  */
#define CVI_ERR_AI_ILLEGAL_PARAM     0xA0000001
/* using a NULL point */
#define CVI_ERR_AI_NULL_PTR          0xA0000002
/* try to enable or initialize system,device or channel, before configing attribute */
#define CVI_ERR_AI_NOT_CONFIG        0xA0000007
/* operation is not supported by NOW */
#define CVI_ERR_AI_NOT_SUPPORT       0xA0000008
/* operation is not permitted ,eg, try to change stati attribute */
#define CVI_ERR_AI_NOT_PERM         0xA0000003
/* the devide is not enabled  */
#define CVI_ERR_AI_NOT_ENABLED       0xA0000009
/* failure caused by malloc memory */
#define CVI_ERR_AI_NOMEM             0xA000000A
/* failure caused by malloc buffer */
#define CVI_ERR_AI_NOBUF             0xA000000B
/* no data in buffer */
#define CVI_ERR_AI_BUF_EMPTY         0xA000000C
/* no buffer for new data */
#define CVI_ERR_AI_BUF_FULL          0xA000000D
/* system is not ready,had not initialized or loaded*/
#define CVI_ERR_AI_SYS_NOTREADY      0xA000000E

#define CVI_ERR_AI_BUSY              0xA000000F
/* vqe  err */
#define CVI_ERR_AI_VQE_ERR       0xA0000010
#define CVI_ERR_AI_VQE_BUF_FULL       0xA0000011
#define CVI_ERR_AI_VQE_FILE_UNEXIST       0xA0000012
/*invalid card ID*/
#define CVI_ERR_AI_INVALID_CARDID    0xA100013
/* invalid device ID */
#define CVI_ERR_AO_INVALID_DEVID     0xA1000001
/* invalid channel ID */
#define CVI_ERR_AO_INVALID_CHNID     0xA1000002
/* at least one parameter is illegal ,eg, an illegal enumeration value  */
#define CVI_ERR_AO_ILLEGAL_PARAM     0xA1000003
/* using a NULL point */
#define CVI_ERR_AO_NULL_PTR          0xA1000004
/* try to enable or initialize system,device or channel, before configing attribute */
#define CVI_ERR_AO_NOT_CONFIG        0xA1000005
/* operation is not supported by NOW */
#define CVI_ERR_AO_NOT_SUPPORT       0xA1000006
/* operation is not permitted ,eg, try to change stati attribute */
#define CVI_ERR_AO_NOT_PERM          0xA1000007
/* the devide is not enabled  */
#define CVI_ERR_AO_NOT_ENABLED       0xA1000008
/* failure caused by malloc memory */
#define CVI_ERR_AO_NOMEM             0xA1000009
/* failure caused by malloc buffer */
#define CVI_ERR_AO_NOBUF             0xA100000A
/* no data in buffer */
#define CVI_ERR_AO_BUF_EMPTY         0xA100000B
/* no buffer for new data */
#define CVI_ERR_AO_BUF_FULL          0xA100000C
/* system is not ready,had not initialized or loaded*/
#define CVI_ERR_AO_SYS_NOTREADY      0xA100000D

#define CVI_ERR_AO_BUSY              0xA100000E
/* vqe  err */
#define CVI_ERR_AO_VQE_ERR       0xA100000F
/*invalid card ID*/
#define CVI_ERR_AO_INVALID_CARDID    0xA100010


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __CVI_COMM_AI_H__ */


#ifndef _CVI_COMM_HDMI_H_
#define _CVI_COMM_HDMI_H_

#include <cvi_type.h>
#include <cvi_common.h>
#include <cvi_defines.h>
#include <cvi_comm_video.h>

/* Maximum length of HDMI vendor user data */
#define CVI_HDMI_VENDOR_USER_DATA_MAX_LEN   22
/* Maximum number of audio capabilities */
#define CVI_HDMI_MAX_AUDIO_CAPBILITY_CNT    16
/* Maximum number of sample rates supported */
#define CVI_HDMI_MAX_SAMPLE_RATE_NUM        8
/* Maximum number of bit depths supported */
#define CVI_HDMI_MAX_BIT_DEPTH_NUM          6
/* Maximum number of detailed timing entries */
#define CVI_HDMI_DETAIL_TIMING_MAX          10
/* Length of EDID raw data */
#define CVI_HDMI_EDID_RAW_DATA_LEN          512
/* Number of hardware parameters */
#define CVI_HDMI_HW_PARAM_NUM               4
/* Length of manufacturer name */
#define CVI_HDMI_MANUFACTURE_NAME_LEN        4
/* Number of audio speaker configurations */
#define CVI_HDMI_AUDIO_SPEAKER_BUTT         1

/* Enumeration for HDMI video format standards */
typedef enum _CVI_HDMI_VIDEO_FORMAT {
	CVI_HDMI_VIDEO_FORMAT_CEA861_640x480p60		= 1,    /* 640x480p60 format */
	CVI_HDMI_VIDEO_FORMAT_CEA861_720x480p60		= 2,    /* 720x480p60 format */
	CVI_HDMI_VIDEO_FORMAT_CEA861_1280x720p60	= 4,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1920x1080i60	= 5,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1440x480i60	= 6,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1440x240p60	= 8,
	CVI_HDMI_VIDEO_FORMAT_CEA861_2880x480i60	= 10,
	CVI_HDMI_VIDEO_FORMAT_CEA861_2880x240p60	= 12,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1440x480p60	= 14,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1920x1080p60	= 16,
	CVI_HDMI_VIDEO_FORMAT_CEA861_720x576p50		= 17,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1280x720p50	= 19,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1920x1080i50	= 20,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1440x576i50	= 21,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1440x288p50	= 23,
	CVI_HDMI_VIDEO_FORMAT_CEA861_2880x576i50	= 25,
	CVI_HDMI_VIDEO_FORMAT_CEA861_2880x288p50	= 27,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1440x576p50	= 29,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1920x1080p50	= 31,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1920x1080p24	= 32,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1920x1080p25	= 33,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1920x1080p30	= 34,
	CVI_HDMI_VIDEO_FORMAT_CEA861_2880x480p60	= 35,
	CVI_HDMI_VIDEO_FORMAT_CEA861_2880x576p50	= 37,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1920x1080i100	= 40,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1280x720p100	= 41,
	CVI_HDMI_VIDEO_FORMAT_CEA861_720x576p100	= 42,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1440x576i100	= 44,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1920x1080i120	= 46,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1280x720p120	= 47,
	CVI_HDMI_VIDEO_FORMAT_CEA861_720x480p120	= 48,
	CVI_HDMI_VIDEO_FORMAT_CEA861_720x480i120	= 50,
	CVI_HDMI_VIDEO_FORMAT_CEA861_720x576p200	= 52,
	CVI_HDMI_VIDEO_FORMAT_CEA861_720x576i200	= 54,
	CVI_HDMI_VIDEO_FORMAT_CEA861_720x480p240	= 56,
	CVI_HDMI_VIDEO_FORMAT_CEA861_720x480i240	= 58,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1280x720p24	= 60,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1280x720p25	= 61,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1280x720p30	= 62,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1920x1080p120	= 63,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1920x1080p100	= 64,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1680x720p24	= 79,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1680x720p25	= 80,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1680x720p30	= 81,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1680x720p50	= 82,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1680x720p60	= 83,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1680x720p100	= 84,
	CVI_HDMI_VIDEO_FORMAT_CEA861_1680x720p120	= 85,
	CVI_HDMI_VIDEO_FORMAT_CEA861_2560x1080p24	= 86,
	CVI_HDMI_VIDEO_FORMAT_CEA861_2560x1080p25	= 87,
	CVI_HDMI_VIDEO_FORMAT_CEA861_2560x1080p30	= 88,
	CVI_HDMI_VIDEO_FORMAT_CEA861_2560x1080p50	= 89,
	CVI_HDMI_VIDEO_FORMAT_CEA861_2560x1080p60	= 90,
	CVI_HDMI_VIDEO_FORMAT_CEA861_2560x1080p100	= 91,
	CVI_HDMI_VIDEO_FORMAT_CEA861_2560x1080p120	= 92,
	CVI_HDMI_VIDEO_FORMAT_CEA861_3840x2160p24	= 93,
	CVI_HDMI_VIDEO_FORMAT_CEA861_3840x2160p25	= 94,
	CVI_HDMI_VIDEO_FORMAT_CEA861_3840x2160p30	= 95,
	CVI_HDMI_VIDEO_FORMAT_CEA861_3840x2160p50	= 96,
	CVI_HDMI_VIDEO_FORMAT_CEA861_3840x2160p60	= 97,
	CVI_HDMI_VIDEO_FORMAT_CEA861_4096x2160p24	= 98,
	CVI_HDMI_VIDEO_FORMAT_CEA861_4096x2160p25	= 99,
	CVI_HDMI_VIDEO_FORMAT_CEA861_4096x2160p30	= 100,
	CVI_HDMI_VIDEO_FORMAT_CEA861_4096x2160p50	= 101,
	CVI_HDMI_VIDEO_FORMAT_CEA861_4096x2160p60	= 102,
	CVI_HDMI_VIDEO_FORMAT_CVT_RB_2560X1440p60	= 108,
	CVI_HDMI_VIDEO_FORMAT_CUSTOMER_DEFINE,
	CVI_HDMI_VIDEO_FORMAT_BUTT,
} CVI_HDMI_VIDEO_FORMAT;

/* Enumeration for HDMI error codes */
typedef enum _CVI_HDMI_ERRORCODE {
	CVI_ERR_HDMI_WRITE_SCDC_FAILED = -100,  /* Failed to write SCDC (Status and Control Data Channel) */
	CVI_ERR_HDMI_READ_SCDC_FAILED,          /* Failed to read SCDC data */
	CVI_ERR_HDMI_SET_INFOFRAME_FAILED,      /* Failed to set HDMI infoframe */
	CVI_ERR_HDMI_API_CONFIG_FAILED,         /* Failed to configure HDMI API */
	CVI_ERR_HDMI_HPD_FAILED,               /* Hot Plug Detection (HPD) operation failed */
	CVI_ERR_HDMI_NOT_INIT,                 /* HDMI module not initialized */
	CVI_ERR_HDMI_INVALID_PARA,             /* Invalid parameter provided */
	CVI_ERR_HDMI_NULL_PTR,                 /* Null pointer error */
	CVI_ERR_HDMI_DEV_NOT_OPEN,             /* HDMI device not opened */
	CVI_ERR_HDMI_DEV_NOT_CONNECT,          /* HDMI device not connected */
	CVI_ERR_HDMI_READ_SINK_FAILED,         /* Failed to read sink device information */
	CVI_ERR_HDMI_INIT_ALREADY,             /* HDMI already initialized */
	CVI_ERR_HDMI_CALLBACK_ALREADY,         /* Callback function already registered */
	CVI_ERR_HDMI_INVALID_CALLBACK,         /* Invalid callback function */
	CVI_ERR_HDMI_FEATURE_NO_SUPPORT,       /* Feature not supported */
	CVI_ERR_HDMI_READ_EVENT_FAILED,        /* Failed to read HDMI event */
	CVI_ERR_HDMI_NOT_START,                /* HDMI not started */
	CVI_ERR_HDMI_READ_EDID_FAILED,         /* Failed to read EDID data */
	CVI_ERR_HDMI_INIT_FAILED,              /* HDMI initialization failed */
	CVI_ERR_HDMI_CREATE_TESK_FAILED,       /* Failed to create HDMI task */
	CVI_ERR_HDMI_MALLOC_FAILED,            /* Memory allocation failed */
	CVI_ERR_HDMI_FREE_FAILED,              /* Memory free operation failed */
	CVI_ERR_HDMI_PTHREAD_CREATE_FAILED,    /* Failed to create pthread */
	CVI_ERR_HDMI_PTHREAD_JOIN_FAILED,      /* Failed to join pthread */
	CVI_ERR_HDMI_STRATEGY_FAILED,          /* HDMI strategy operation failed */
	CVI_ERR_HDMI_SET_ATTR_FAILED,          /* Failed to set HDMI attributes */
	CVI_ERR_HDMI_CALLBACK_NOT_REGISTER,    /* Callback function not registered */
	CVI_ERR_HDMI_UNKNOWN_COMMAND,          /* Unknown HDMI command */
	CVI_ERR_HDMI_MUTEX_LOCK_FAILED,        /* Failed to lock mutex */
} CVI_HDMI_ERRORCODE;

/* Enumeration for HDMI device IDs */
typedef enum _CVI_HDMI_ID {
	CVI_HDMI_ID_0 = 0,     /* First HDMI device */
	CVI_HDMI_ID_BUTT,      /* Invalid HDMI device ID */
} CVI_HDMI_ID;

/* Enumeration for HDMI event types */
typedef enum _CVI_HDMI_EVENT_TYPE {
	CVI_HDMI_EVENT_HOTPLUG = 1,    /* HDMI hot plug event */
	CVI_HDMI_EVENT_NO_PLUG = 2,    /* HDMI unplug event */
	CVI_HDMI_EVENT_EDID_FAIL = 3,  /* EDID read failure event */
	CVI_HDMI_EVENT_BUTT,           /* Invalid event type */
} CVI_HDMI_EVENT_TYPE;

/* Enumeration for HDMI video modes */
typedef enum _CVI_HDMI_VIDEO_MODE {
	CVI_HDMI_VIDEO_MODE_RGB888 = 0,    /* RGB 888 mode */
	CVI_HDMI_VIDEO_MODE_YCBCR444,      /* YCbCr 4:4:4 mode */
	CVI_HDMI_VIDEO_MODE_YCBCR422,      /* YCbCr 4:2:2 mode */
	CVI_HDMI_VIDEO_MODE_YCBCR420,      /* YCbCr 4:2:0 mode */
	CVI_HDMI_VIDEO_MODE_BUTT,          /* Invalid video mode */
} CVI_HDMI_VIDEO_MODE;

/* Enumeration for HDMI deep color modes */
typedef enum _CVI_HDMI_DEEP_COLOR {
	CVI_HDMI_DEEP_COLOR_24BIT = 24,    /* 24-bit color depth */
	CVI_HDMI_DEEP_COLOR_30BIT = 30,    /* 30-bit color depth */
	CVI_HDMI_DEEP_COLOR_36BIT = 36,    /* 36-bit color depth */
	CVI_HDMI_DEEP_COLOR_BUTT,          /* Invalid color depth */
} CVI_HDMI_DEEP_COLOR;

/* Function pointer type for HDMI event callback */
typedef void (* CVI_HDMI_CALLBACK) (CVI_HDMI_EVENT_TYPE event, CVI_VOID *private_data);

/* Enumeration for HDMI sample rates */
typedef enum _CVI_HDMI_SAMPLE_RATE {
	CVI_HDMI_SAMPLE_RATE_UNKNOWN,              /* Unknown sample rate */
	CVI_HDMI_SAMPLE_RATE_32K = 32000,         /* 32KHz sample rate */
	CVI_HDMI_SAMPLE_RATE_44K = 44100,         /* 44.1KHz sample rate */
	CVI_HDMI_SAMPLE_RATE_48K = 48000,         /* 48KHz sample rate */
	CVI_HDMI_SAMPLE_RATE_88K = 88000,         /* 88KHz sample rate */
	CVI_HDMI_SAMPLE_RATE_96K = 96000,         /* 96KHz sample rate */
	CVI_HDMI_SAMPLE_RATE_176K = 176400,       /* 176.4KHz sample rate */
	CVI_HDMI_SAMPLE_RATE_192K = 192000,       /* 192KHz sample rate */
	CVI_HDMI_SAMPLE_RATE_BUTT,                /* Invalid sample rate */
} CVI_HDMI_SAMPLE_RATE;

/* Enumeration for HDMI bit depth */
typedef enum _CVI_HDMI_BIT_DEPTH {
	CVI_HDMI_BIT_DEPTH_UNKNOWN,               /* Unknown bit depth */
	CVI_HDMI_BIT_DEPTH_16 = 16,              /* 16-bit audio */
	CVI_HDMI_BIT_DEPTH_24 = 24,              /* 24-bit audio */
	CVI_HDMI_BIT_DEPTH_BUTT,                 /* Invalid bit depth */
} CVI_HDMI_BIT_DEPTH;

/* Enumeration for HDMI audio format codes */
typedef enum _CVI_HDMI_AUDIO_FORMAT_CODE {
	CVI_HDMI_AUDIO_FORMAT_CODE_RESERVED,      /* Reserved format */
	CVI_HDMI_AUDIO_FORMAT_CODE_PCM,          /* PCM format */
	CVI_HDMI_AUDIO_FORMAT_CODE_AC3,          /* AC3 format */
	CVI_HDMI_AUDIO_FORMAT_CODE_MPEG1,        /* MPEG1 format */
	CVI_HDMI_AUDIO_FORMAT_CODE_MP3,          /* MP3 format */
	CVI_HDMI_AUDIO_FORMAT_CODE_MPEG2,        /* MPEG2 format */
	CVI_HDMI_AUDIO_FORMAT_CODE_AAC,          /* AAC format */
	CVI_HDMI_AUDIO_FORMAT_CODE_DTS,          /* DTS format */
	CVI_HDMI_AUDIO_FORMAT_CODE_ATRAC,        /* ATRAC format */
	CVI_HDMI_AUDIO_FORMAT_CODE_ONE_BIT,      /* One bit audio */
	CVI_HDMI_AUDIO_FORMAT_CODE_DDP,          /* DDP format */
	CVI_HDMI_AUDIO_FORMAT_CODE_DTS_HD,       /* DTS HD format */
	CVI_HDMI_AUDIO_FORMAT_CODE_MAT,          /* MAT format */
	CVI_HDMI_AUDIO_FORMAT_CODE_DST,          /* DST format */
	CVI_HDMI_AUDIO_FORMAT_CODE_WMA_PRO,      /* WMA Pro format */
	CVI_HDMI_AUDIO_FORMAT_CODE_BUTT,         /* Invalid format */
} CVI_HDMI_AUDIO_FORMAT_CODE;

/* Enumeration for HDMI color space */
typedef enum _CVI_HDMI_COLOR_SPACE {
	CVI_HDMI_COLOR_SPACE_RGB888,             /* RGB 888 color space */
	CVI_HDMI_COLOR_SPACE_YCBCR422,          /* YCbCr 4:2:2 color space */
	CVI_HDMI_COLOR_SPACE_YCBCR444,          /* YCbCr 4:4:4 color space */
	CVI_HDMI_COLOR_SPACE_YCBCR420,          /* YCbCr 4:2:0 color space */
	CVI_HDMI_COLOR_SPACE_BUTT,              /* Invalid color space */
} CVI_HDMI_COLOR_SPACE;

/* Enumeration for HDMI bar information */
typedef enum _CVI_HDMI_BAR_INFO {
	CVI_HDMI_BAR_INFO_NCVI_VALID,           /* Bar info not valid */
	CVI_HDMI_BAR_INFO_V,                    /* Vertical bar info */
	CVI_HDMI_BAR_INFO_H,                    /* Horizontal bar info */
	CVI_HDMI_BAR_INFO_VH,                   /* Both vertical and horizontal */
	CVI_HDMI_BAR_INFO_BUTT,                 /* Invalid bar info */
} CVI_HDMI_BAR_INFO;

/* Enumeration for HDMI scan information */
typedef enum _CVI_HDMI_SCAN_INFO {
	CVI_HDMI_SCAN_INFO_NO_DATA,             /* No scan info */
	CVI_HDMI_SCAN_INFO_OVERSCANNED,         /* Overscanned */
	CVI_HDMI_SCAN_INFO_UNDERSCANNED,        /* Underscanned */
	CVI_HDMI_SCAN_INFO_BUTT,                /* Invalid scan info */
} CVI_HDMI_SCAN_INFO;

/* Enumeration for HDMI colorimetry standards */
typedef enum _CVI_HDMI_COLORIMETRY {
	CVI_HDMI_COMMON_COLORIMETRY_NO_DATA,        /* No colorimetry data */
	CVI_HDMI_COMMON_COLORIMETRY_ITU601,         /* ITU-R BT.601 colorimetry */
	CVI_HDMI_COMMON_COLORIMETRY_ITU709,         /* ITU-R BT.709 colorimetry */
	CVI_HDMI_COMMON_COLORIMETRY_BUTT,           /* Invalid colorimetry */
} CVI_HDMI_COLORIMETRY;

/* Enumeration for HDMI extended colorimetry standards */
typedef enum _CVI_HDMI_EX_COLORIMETRY {
	CVI_HDMI_COMMON_COLORIMETRY_XVYCC_601,      /* xvYCC 601 colorimetry */
	CVI_HDMI_COMMON_COLORIMETRY_XVYCC_709,      /* xvYCC 709 colorimetry */
	CVI_HDMI_COMMON_COLORIMETRY_S_YCC_601,      /* sYCC 601 colorimetry */
	CVI_HDMI_COMMON_COLORIMETRY_ADOBE_YCC_601,  /* Adobe YCC 601 colorimetry */
	CVI_HDMI_COMMON_COLORIMETRY_ADOBE_RGB,      /* Adobe RGB colorimetry */
	CVI_HDMI_COMMON_COLORIMETRY_2020_CONST_LUMINOUS,    /* BT.2020 constant luminance */
	CVI_HDMI_COMMON_COLORIMETRY_2020_NON_CONST_LUMINOUS, /* BT.2020 non-constant luminance */
	CVI_HDMI_COMMON_COLORIMETRY_EX_BUTT,        /* Invalid extended colorimetry */
} CVI_HDMI_EX_COLORIMETRY;

/* Enumeration for picture aspect ratios */
typedef enum _CVI_PIC_ASPECT_RATIO {
	CVI_HDMI_PIC_ASPECT_RATIO_NO_DATA,          /* No aspect ratio data */
	CVI_HDMI_PIC_ASPECT_RATIO_4TO3,             /* 4:3 aspect ratio */
	CVI_HDMI_PIC_ASPECT_RATIO_16TO9,            /* 16:9 aspect ratio */
	CVI_HDMI_PIC_ASPECT_RATIO_64TO27,           /* 64:27 aspect ratio */
	CVI_HDMI_PIC_ASPECT_RATIO_256TO135,         /* 256:135 aspect ratio */
	CVI_HDMI_PIC_ASPECT_BUTT,                   /* Invalid aspect ratio */
} CVI_PIC_ASPECT_RATIO;

/* Enumeration for active aspect ratios */
typedef enum _CVI_HDMI_ACTIVE_ASPECT_RATIO {
	CVI_HDMI_ACTIVE_ASPECT_RATIO_16TO9_TOP = 2,     /* 16:9 top letterbox */
	CVI_HDMI_ACTIVE_ASPECT_RATIO_14TO9_TOP,         /* 14:9 top letterbox */
	CVI_HDMI_ACTIVE_ASPECT_RATIO_16TO9_BOX_CENTER,  /* 16:9 center letterbox */
	CVI_HDMI_ACTIVE_ASPECT_RATIO_SAME_PIC = 8,      /* Same as picture aspect ratio */
	CVI_HDMI_ACTIVE_ASPECT_RATIO_4TO3_CENTER,       /* 4:3 center */
	CVI_HDMI_ACTIVE_ASPECT_RATIO_16TO9_CENTER,      /* 16:9 center */
	CVI_HDMI_ACTIVE_ASPECT_RATIO_14TO9_CENTER,      /* 14:9 center */
	CVI_HDMI_ACTIVE_ASPECT_RATIO_4TO3_14_9 = 13,    /* 4:3 with 14:9 center */
	CVI_HDMI_ACTIVE_ASPECT_RATIO_16TO9_14_9,        /* 16:9 with 14:9 center */
	CVI_HDMI_ACTIVE_ASPECT_RATIO_16TO9_4_3,         /* 16:9 with 4:3 center */
	CVI_HDMI_ACTIVE_ASPECT_RATIO_BUTT,              /* Invalid active aspect ratio */
} CVI_HDMI_ACTIVE_ASPECT_RATIO;

/* Enumeration for picture scaling modes */
typedef enum _CVI_HDMI_PIC_SCALINE {
	CVI_HDMI_PICTURE_NON_UNIFORM_SCALING,       /* Non-uniform scaling */
	CVI_HDMI_PICTURE_SCALING_H,                 /* Horizontal scaling only */
	CVI_HDMI_PICTURE_SCALING_V,                 /* Vertical scaling only */
	CVI_HDMI_PICTURE_SCALING_HV,                /* Both horizontal and vertical scaling */
	CVI_HDMI_PICTURE_SCALING_BUTT,             /* Invalid scaling mode */
} CVI_HDMI_PIC_SCALINE;

/* Enumeration for RGB quantization range */
typedef enum _CVI_HDMI_RGB_QUANT_RANGE {
	CVI_HDMI_RGB_QUANT_DEFAULT_RANGE,          /* Default RGB range */
	CVI_HDMI_RGB_QUANT_LIMITED_RANGE,          /* Limited RGB range (16-235) */
	CVI_HDMI_RGB_QUANT_FULL_RANGE,            /* Full RGB range (0-255) */
	CVI_HDMI_RGB_QUANT_FULL_BUTT,             /* Invalid RGB range */
} CVI_HDMI_RGB_QUANT_RANGE;

/* Enumeration for pixel repetition */
typedef enum _CVI_HDMI_PIXEL_REPETITION {
	CVI_HDMI_PIXEL_REPET_NO,                   /* No pixel repetition */
	CVI_HDMI_PIXEL_REPET_2_TIMES,             /* 2x pixel repetition */
	CVI_HDMI_PIXEL_REPET_3_TIMES,             /* 3x pixel repetition */
	CVI_HDMI_PIXEL_REPET_4_TIMES,             /* 4x pixel repetition */
	CVI_HDMI_PIXEL_REPET_5_TIMES,             /* 5x pixel repetition */
	CVI_HDMI_PIXEL_REPET_6_TIMES,             /* 6x pixel repetition */
	CVI_HDMI_PIXEL_REPET_7_TIMES,             /* 7x pixel repetition */
	CVI_HDMI_PIXEL_REPET_8_TIMES,             /* 8x pixel repetition */
	CVI_HDMI_PIXEL_REPET_9_TIMES,             /* 9x pixel repetition */
	CVI_HDMI_PIXEL_REPET_10_TIMES,            /* 10x pixel repetition */
	CVI_HDMI_PIXEL_REPET_BUTT,                /* Invalid pixel repetition */
} CVI_HDMI_PIXEL_REPETITION;

/* Enumeration for content types */
typedef enum _CVI_HDMI_CONTENT_TYPE {
	CVI_HDMI_CONTNET_GRAPHIC,                  /* Graphics content */
	CVI_HDMI_CONTNET_PHOTO,                    /* Photo content */
	CVI_HDMI_CONTNET_CINEMA,                   /* Cinema content */
	CVI_HDMI_CONTNET_GAME,                     /* Game content */
	CVI_HDMI_CONTNET_BUTT,                    /* Invalid content type */
} CVI_HDMI_CONTENT_TYPE;

/* Enumeration for YCC quantization range */
typedef enum _CVI_HDMI_YCC_QUANT_RANGE {
	CVI_HDMI_YCC_QUANT_LIMITED_RANGE,          /* Limited YCC range (16-235) */
	CVI_HDMI_YCC_QUANT_FULL_RANGE,            /* Full YCC range (0-255) */
	CVI_HDMI_YCC_QUANT_BUTT,                  /* Invalid YCC range */
} CVI_HDMI_YCC_QUANT_RANGE;

/* Enumeration for audio channel count */
typedef enum _CVI_HDMI_AUDIO_CHN_CNT {
	CVI_HDMI_AUDIO_CHANEL_CNT_STREAM,          /* Channel count from stream */
	CVI_HDMI_AUDIO_CHANEL_CNT_2,               /* 2 channels */
	CVI_HDMI_AUDIO_CHANEL_CNT_3,               /* 3 channels */
	CVI_HDMI_AUDIO_CHANEL_CNT_4,               /* 4 channels */
	CVI_HDMI_AUDIO_CHANEL_CNT_5,               /* 5 channels */
	CVI_HDMI_AUDIO_CHANEL_CNT_6,               /* 6 channels */
	CVI_HDMI_AUDIO_CHANEL_CNT_7,               /* 7 channels */
	CVI_HDMI_AUDIO_CHANEL_CNT_8,               /* 8 channels */
	CVI_HDMI_AUDIO_CHANEL_BUTT,                /* Invalid channel count */
} CVI_HDMI_AUDIO_CHN_CNT;

/* Enumeration for audio coding types */
typedef enum _CVI_HDMI_CODING_TYPE {
	CVI_HDMI_AUDIO_CODING_REFER_STREAM_HEAD,   /* Refer to stream header */
	CVI_HDMI_AUDIO_CODING_PCM,                 /* PCM coding */
	CVI_HDMI_AUDIO_CODING_AC3,                 /* AC3 coding */
	CVI_HDMI_AUDIO_CODING_MPEG1,               /* MPEG1 coding */
	CVI_HDMI_AUDIO_CODING_MP3,                 /* MP3 coding */
	CVI_HDMI_AUDIO_CODING_MPEG2,               /* MPEG2 coding */
	CVI_HDMI_AUDIO_CODING_AACLC,               /* AAC-LC coding */
	CVI_HDMI_AUDIO_CODING_DTS,                 /* DTS coding */
	CVI_HDMI_AUDIO_CODING_ATRAC,               /* ATRAC coding */
	CVI_HDMI_AUDIO_CODIND_ONE_BIT_AUDIO,       /* One bit audio coding */
	CVI_HDMI_AUDIO_CODING_ENAHNCED_AC3,        /* Enhanced AC3 coding */
	CVI_HDMI_AUDIO_CODING_DTS_HD,              /* DTS-HD coding */
	CVI_HDMI_AUDIO_CODING_MAT,                 /* MAT coding */
	CVI_HDMI_AUDIO_CODING_DST,                 /* DST coding */
	CVI_HDMI_AUDIO_CODING_WMA_PRO,             /* WMA Pro coding */
	CVI_HDMI_AUDIO_CODING_BUTT,                /* Invalid coding type */
} CVI_HDMI_CODING_TYPE;

/* Enumeration for HDMI audio sample size */
typedef enum _CVI_HDMI_AUDIO_SAMPLE_SIZE {
	CVI_HDMI_AUDIO_SAMPLE_SIZE_STREAM,         /* Sample size from stream */
	CVI_HDMI_AUDIO_SAMPLE_SIZE_16 = 16,       /* 16-bit sample size */
	CVI_HDMI_AUDIO_SAMPLE_SIZE_20 = 20,       /* 20-bit sample size */
	CVI_HDMI_AUDIO_SAMPLE_SIZE_24 = 24,       /* 24-bit sample size */
	CVI_HDMI_AUDIO_SAMPLE_SIZE_BUTT,          /* Invalid sample size */
} CVI_HDMI_AUDIO_SAMPLE_SIZE;

/* Enumeration for HDMI audio sample frequency */
typedef enum _CVI_HDMI_AUDIO_SAMPLE_FREQ {
	CVI_HDMI_AUDIO_SAMPLE_FREQ_STREAM,         /* Sample frequency from stream */
	CVI_HDMI_AUDIO_SAMPLE_FREQ_32000 = 32000, /* 32KHz sample frequency */
	CVI_HDMI_AUDIO_SAMPLE_FREQ_44100 = 44100, /* 44.1KHz sample frequency */
	CVI_HDMI_AUDIO_SAMPLE_FREQ_48000 = 48000, /* 48KHz sample frequency */
	CVI_HDMI_AUDIO_SAMPLE_FREQ_88200 = 88200, /* 88.2KHz sample frequency */
	CVI_HDMI_AUDIO_SAMPLE_FREQ_96000 = 96000, /* 96KHz sample frequency */
	CVI_HDMI_AUDIO_SAMPLE_FREQ_176400 = 176400, /* 176.4KHz sample frequency */
	CVI_HDMI_AUDIO_SAMPLE_FREQ_192000 = 192000, /* 192KHz sample frequency */
	CVI_HDMI_AUDIO_SAMPLE_FREQ_BUTT,          /* Invalid sample frequency */
} CVI_HDMI_AUDIO_SAMPLE_FREQ;

/* Enumeration for HDMI level shift values */
typedef enum _CVI_HDMI_LEVEL_SHIFT_VAL {
	CVI_HDMI_LEVEL_SHIFT_VALUE_0_DB,          /* 0 dB level shift */
	CVI_HDMI_LEVEL_SHIFT_VALUE_1_DB,          /* 1 dB level shift */
	CVI_HDMI_LEVEL_SHIFT_VALUE_2_DB,          /* 2 dB level shift */
	CVI_HDMI_LEVEL_SHIFT_VALUE_3_DB,          /* 3 dB level shift */
	CVI_HDMI_LEVEL_SHIFT_VALUE_4_DB,          /* 4 dB level shift */
	CVI_HDMI_LEVEL_SHIFT_VALUE_5_DB,          /* 5 dB level shift */
	CVI_HDMI_LEVEL_SHIFT_VALUE_6_DB,          /* 6 dB level shift */
	CVI_HDMI_LEVEL_SHIFT_VALUE_7_DB,          /* 7 dB level shift */
	CVI_HDMI_LEVEL_SHIFT_VALUE_8_DB,          /* 8 dB level shift */
	CVI_HDMI_LEVEL_SHIFT_VALUE_9_DB,          /* 9 dB level shift */
	CVI_HDMI_LEVEL_SHIFT_VALUE_10_DB,         /* 10 dB level shift */
	CVI_HDMI_LEVEL_SHIFT_VALUE_11_DB,         /* 11 dB level shift */
	CVI_HDMI_LEVEL_SHIFT_VALUE_12_DB,         /* 12 dB level shift */
	CVI_HDMI_LEVEL_SHIFT_VALUE_13_DB,         /* 13 dB level shift */
	CVI_HDMI_LEVEL_SHIFT_VALUE_14_DB,         /* 14 dB level shift */
	CVI_HDMI_LEVEL_SHIFT_VALUE_15_DB,         /* 15 dB level shift */
	CVI_HDMI_LEVEL_SHIFT_VALUE_BUTT,          /* Invalid level shift value */
} CVI_HDMI_LEVEL_SHIFT_VAL;

/* Enumeration for HDMI LFE playback level */
typedef enum _CVI_HDMI_LFE_PLAYBACK_LEVEL {
	CVI_HDMI_LFE_PLAYBACK_NO,                 /* No LFE playback */
	CVI_HDMI_LFE_PLAYBACK_0_DB,              /* 0 dB LFE playback level */
	CVI_HDMI_LFE_PLAYBACK_10_DB,             /* +10 dB LFE playback level */
	CVI_HDMI_LFE_PLAYBACK_BUTT,              /* Invalid LFE playback level */
} CVI_HDMI_LFE_PLAYBACK_LEVEL;

/* Enumeration for HDMI infoframe types */
typedef enum _CVI_HDMI_INFOFRAME_TYPE {
	CVI_INFOFRAME_TYPE_AVI,                   /* AVI infoframe type */
	CVI_INFOFRAME_TYPE_AUDIO,                 /* Audio infoframe type */
	CVI_INFOFRAME_TYPE_VENDORSPEC,            /* Vendor specific infoframe type */
	CVI_INFOFRAME_TYPE_BUTT,                  /* Invalid infoframe type */
} CVI_HDMI_INFOFRAME_TYPE;

/* Enumeration for HDMI force action */
typedef enum _CVI_HDMI_FORCE_ACTION {
	CVI_HDMI_FORCE_NULL,                      /* No force action */
	CVI_HDMI_FORCE_HDMI,                      /* Force HDMI mode */
} CVI_HDMI_FORCE_ACTION;

/* Structure for HDMI callback function */
typedef struct _CVI_HDMI_CALLBACK_FUNC {
	CVI_HDMI_CALLBACK hdmi_event_callback;   /* Event callback function */
	CVI_VOID *private_data;                  /* Private data for callback */
} CVI_HDMI_CALLBACK_FUNC;

/* Structure for HDMI attributes configuration */
typedef struct _CVI_HDMI_ATTR {
	CVI_BOOL hdmi_en;                      /* HDMI enable flag */
	CVI_HDMI_VIDEO_FORMAT video_format;    /* Video format */
	CVI_HDMI_DEEP_COLOR deep_color_mode;   /* Deep color mode */
	CVI_BOOL audio_en;                     /* Audio enable flag */
	CVI_BOOL hdcp14_en;                    /* HDCP 1.4 enable flag */
	CVI_HDMI_FORCE_ACTION hdmi_force_output; /* Force HDMI output mode */
	CVI_HDMI_VIDEO_MODE hdmi_video_input;    /* Video input mode */
	CVI_HDMI_VIDEO_MODE hdmi_video_output;   /* Video output mode */
	CVI_HDMI_SAMPLE_RATE sample_rate;        /* Audio sample rate */
	CVI_HDMI_BIT_DEPTH bit_depth;            /* Audio bit depth */
	CVI_BOOL auth_mode_en;                   /* Authentication mode enable */
	CVI_U64 audio_start_paddr;               /* Audio start physical address */
	CVI_U64 audio_stop_paddr;                /* Audio stop physical address */
	CVI_BOOL deep_color_adapt_en;            /* Deep color adaptation enable */
	CVI_U32 pix_clk;                        /* Pixel clock */
} CVI_HDMI_ATTR;

/* Structure for HDMI audio capability information */
typedef struct _CVI_HDMI_AUDIO_INFO {
	CVI_HDMI_AUDIO_FORMAT_CODE audio_format_code;    /* Audio format code (PCM, AC3, etc.) */
	CVI_HDMI_SAMPLE_RATE support_sample_rate[CVI_HDMI_MAX_SAMPLE_RATE_NUM];  /* Array of supported sample rates */
	CVI_U32 support_sample_rate_num;                 /* Number of supported sample rates */
	CVI_U8 audio_chn;                               /* Number of audio channels */
	CVI_HDMI_BIT_DEPTH support_bit_depth[CVI_HDMI_MAX_BIT_DEPTH_NUM];  /* Array of supported bit depths */
	CVI_U32 support_bit_depth_num;                  /* Number of supported bit depths */
	CVI_U32 max_bit_rate;                          /* Maximum supported bit rate */
} CVI_HDMI_AUDIO_INFO;

/* Structure for HDMI timing parameters */
typedef struct _CVI_HDMI_TIMING_INFO {
	CVI_U32 vfb;           /* Vertical front blanking lines */
	CVI_U32 vbb;           /* Vertical back blanking lines */
	CVI_U32 vact;          /* Vertical active lines */
	CVI_U32 hfb;           /* Horizontal front blanking pixels */
	CVI_U32 hbb;           /* Horizontal back blanking pixels */
	CVI_U32 hact;          /* Horizontal active pixels */
	CVI_U32 vpw;           /* Vertical sync pulse width */
	CVI_U32 hpw;           /* Horizontal sync pulse width */
	CVI_BOOL idv;          /* Inverse data valid signal */
	CVI_BOOL ihs;          /* Inverse horizontal sync signal */
	CVI_BOOL ivs;          /* Inverse vertical sync signal */
	CVI_U32 img_width;     /* Image width in pixels */
	CVI_U32 img_height;    /* Image height in pixels */
	CVI_U32 aspect_ratio_w;/* Aspect ratio width */
	CVI_U32 aspect_ratio_h;/* Aspect ratio height */
	CVI_BOOL interlace;    /* Interlace mode flag */
	CVI_U32 pixel_clk;     /* Pixel clock frequency in Hz */
} CVI_HDMI_TIMING_INFO;

/* Structure for HDMI detailed timing information */
typedef struct _CVI_HDMI_DETAIL_TIMING {
	CVI_U32 detail_timing_num;                           /* Number of detailed timing entries */
	CVI_HDMI_TIMING_INFO detail_timing[CVI_HDMI_DETAIL_TIMING_MAX]; /* Array of detailed timing information */
} CVI_HDMI_DETAIL_TIMING;

/* Structure for HDMI video information */
typedef struct _CVI_HDMI_VIDEO_INFO {
	CVI_HDMI_VIDEO_FORMAT mcode;      /* Video format code */
	CVI_HDMI_TIMING_INFO timing_info; /* Detailed timing parameters */
	CVI_U32 fresh_rate;               /* Refresh rate in Hz */
} CVI_HDMI_VIDEO_INFO;

/* Structure for HDMI sink device capabilities */
typedef struct _CVI_HDMI_SINK_CAPABILITY {
	CVI_BOOL is_connected;         /* True if sink device is connected */
	CVI_BOOL support_hdmi;        /* True if sink supports HDMI */
	CVI_BOOL is_sink_power_on;    /* True if sink device is powered on */
	CVI_HDMI_VIDEO_FORMAT native_video_format;  /* Native video format of sink */
	CVI_HDMI_VIDEO_INFO support_video_format[CVI_HDMI_VIDEO_FORMAT_BUTT];  /* Supported video formats */
	CVI_BOOL support_ycbcr;       /* True if YCbCr color space is supported */
	CVI_BOOL support_xvycc601;    /* True if xvYCC601 is supported */
	CVI_BOOL support_xvycc709;    /* True if xvYCC709 is supported */
	CVI_U8 md_bit;                /* Metadata bit information */
	CVI_U32 audio_info_num;       /* Number of audio information entries */
	CVI_BOOL hdcp14_en;           /* True if HDCP 1.4 is enabled */
	CVI_HDMI_VIDEO_MODE hdmi_video_input;   /* HDMI video input mode */
	CVI_HDMI_VIDEO_MODE hdmi_video_output;  /* HDMI video output mode */
	CVI_HDMI_AUDIO_INFO audio_info[CVI_HDMI_MAX_AUDIO_CAPBILITY_CNT];  /* Audio capability information */
	CVI_BOOL speaker[CVI_HDMI_AUDIO_SPEAKER_BUTT];  /* Speaker configuration */
	CVI_U8 manufacture_name[CVI_HDMI_MANUFACTURE_NAME_LEN];  /* Manufacturer name */
	CVI_U32 pdt_code;             /* Product code */
	CVI_U32 serial_num;           /* Serial number */
	CVI_U32 week_of_manufacture;  /* Week of manufacture */
	CVI_U32 year_of_manufacture;  /* Year of manufacture */
	CVI_U8 version;               /* Version number */
	CVI_U8 revision;              /* Revision number */
	CVI_U8 edid_ex_blk_num;      /* Number of EDID extension blocks */
	CVI_BOOL is_phy_addr_valid;   /* True if physical address is valid */
	CVI_U8 phys_addr_a;          /* Physical address part A */
	CVI_U8 phys_addr_b;          /* Physical address part B */
	CVI_U8 phys_addr_c;          /* Physical address part C */
	CVI_U8 phys_addr_d;          /* Physical address part D */
	CVI_BOOL support_dvi_dual;    /* True if DVI dual-link is supported */
	CVI_BOOL support_deepcolor_ycbcr444;  /* True if deep color YCbCr 4:4:4 is supported */
	CVI_BOOL support_deep_color_30bit;    /* True if 30-bit deep color is supported */
	CVI_BOOL support_deep_color_36bit;    /* True if 36-bit deep color is supported */
	CVI_BOOL support_deep_color_48bit;    /* True if 48-bit deep color is supported */
	CVI_BOOL support_ai;          /* True if AI (Audio Information) is supported */
	CVI_U32 max_tmds_clk;        /* Maximum TMDS clock frequency */
	CVI_BOOL i_latency_fields_present;    /* True if interlaced latency fields are present */
	CVI_BOOL latency_fields_present;      /* True if latency fields are present */
	CVI_BOOL hdmi_video_present;  /* True if HDMI video is present */
	CVI_U8 video_latency;        /* Video latency */
	CVI_U8 audio_latency;        /* Audio latency */
	CVI_U8 interlaced_video_latency;  /* Interlaced video latency */
	CVI_U8 interlaced_audio_latency;  /* Interlaced audio latency */
	CVI_BOOL support_y420_dc_30bit;   /* True if YCbCr 4:2:0 30-bit deep color is supported */
	CVI_BOOL support_y420_dc_36bit;   /* True if YCbCr 4:2:0 36-bit deep color is supported */
	CVI_BOOL support_y420_dc_48bit;   /* True if YCbCr 4:2:0 48-bit deep color is supported */
	CVI_BOOL support_hdmi_2_0;        /* True if HDMI 2.0 is supported */
	CVI_BOOL support_y420_format[CVI_HDMI_VIDEO_FORMAT_BUTT];  /* YCbCr 4:2:0 format support for each video format */
	CVI_BOOL only_support_y420_format[CVI_HDMI_VIDEO_FORMAT_BUTT];  /* True if only YCbCr 4:2:0 is supported for format */
	CVI_BOOL ycc_quant_selectable;    /* True if YCC quantization range is selectable */
	CVI_BOOL rgb_quant_selectable;    /* True if RGB quantization range is selectable */
	CVI_HDMI_DETAIL_TIMING detailed_timing;  /* Detailed timing information */
} CVI_HDMI_SINK_CAPABILITY;

/* Structure for HDMI EDID information */
typedef struct _CVI_HDMI_EDID {
	CVI_BOOL edid_valid;          /* EDID validity flag */
	CVI_U32 edid_len;            /* EDID data length */
	CVI_U8 edid[256];            /* EDID data buffer */
} CVI_HDMI_EDID;

/* Structure for HDMI AVI InfoFrame */
typedef struct _CVI_HDMI_AVI_INFOFRAME {
	CVI_HDMI_VIDEO_FORMAT timing_mode;     /* Video timing mode */
	CVI_HDMI_COLOR_SPACE color_space;      /* Color space */
	CVI_BOOL active_info_present;          /* True if active format information is present */
	CVI_HDMI_BAR_INFO bar_info;           /* Bar information */
	CVI_HDMI_SCAN_INFO scan_info;         /* Scan information */
	CVI_HDMI_COLORIMETRY colorimetry;     /* Basic colorimetry */
	CVI_HDMI_EX_COLORIMETRY ex_colorimetry; /* Extended colorimetry */
	CVI_PIC_ASPECT_RATIO aspect_ratio;    /* Picture aspect ratio */
	CVI_HDMI_ACTIVE_ASPECT_RATIO active_aspect_ratio;  /* Active format aspect ratio */
	CVI_HDMI_PIC_SCALINE pic_scaling;     /* Picture scaling information */
	CVI_HDMI_RGB_QUANT_RANGE rgb_quant;   /* RGB quantization range */
	CVI_BOOL is_it_content;               /* True if IT content */
	CVI_HDMI_PIXEL_REPETITION pixel_repetition;  /* Pixel repetition factor */
	CVI_HDMI_CONTENT_TYPE content_type;    /* Content type */
	CVI_HDMI_YCC_QUANT_RANGE ycc_quant;   /* YCC quantization range */
	CVI_U16 line_n_end_of_top_bar;        /* Line number at end of top bar */
	CVI_U16 line_n_start_of_bcvi_bar;     /* Line number at start of bottom bar */
	CVI_U16 pixel_n_end_of_left_bar;      /* Pixel number at end of left bar */
	CVI_U16 pixel_n_start_of_right_bar;   /* Pixel number at start of right bar */
} CVI_HDMI_AVI_INFOFRAME;

/* Structure for HDMI audio infoframe */
typedef struct _CVI_HDMI_AUDIO_INFOFRAME {
	CVI_HDMI_AUDIO_CHN_CNT chn_cnt;         /* Channel count */
	CVI_HDMI_CODING_TYPE coding_type;       /* Audio coding type */
	CVI_HDMI_AUDIO_SAMPLE_SIZE sample_size; /* Sample size */
	CVI_HDMI_AUDIO_SAMPLE_FREQ sampling_freq; /* Sampling frequency */
	CVI_U8 chn_alloc;                       /* Channel allocation */
	CVI_HDMI_LEVEL_SHIFT_VAL level_shift;   /* Level shift value */
	CVI_HDMI_LFE_PLAYBACK_LEVEL lfe_playback_level; /* LFE playback level */
	CVI_BOOL down_mix_inhibit;              /* Down-mix inhibit flag */
} CVI_HDMI_AUDIO_INFOFRAME;

/* Structure for HDMI vendor-specific infoframe */
typedef struct _CVI_HDMI_VENDORSPEC_INFOFRAME {
	CVI_U8 data_len;                        /* Data length */
	CVI_U8 user_data[CVI_HDMI_VENDOR_USER_DATA_MAX_LEN]; /* User data */
} CVI_HDMI_VENDORSPEC_INFOFRAME;

/* Structure for HDMI infoframe unit */
typedef struct _CVI_HDMI_INFOFRAME_UNIT {
	CVI_HDMI_AVI_INFOFRAME avi_infoframe;           /* AVI infoframe */
	CVI_HDMI_AUDIO_INFOFRAME audio_infoframe;       /* Audio infoframe */
	CVI_HDMI_VENDORSPEC_INFOFRAME vendor_spec_infoframe; /* Vendor specific infoframe */
} CVI_HDMI_INFOFRAME_UNIT;

/* Structure for HDMI infoframe */
typedef struct _CVI_HDMI_INFOFRAME {
	CVI_HDMI_INFOFRAME_TYPE infoframe_type; /* Infoframe type */
	CVI_HDMI_INFOFRAME_UNIT infoframe_unit; /* Infoframe unit */
} CVI_HDMI_INFOFRAME;

/* Structure for HDMI hardware parameters */
typedef struct _CVI_HDMI_HW_PARAM {
	CVI_U32 i_de_main_clk;                  /* DE main clock */
	CVI_U32 i_de_main_data;                 /* DE main data */
	CVI_U32 i_main_clk;                     /* Main clock */
	CVI_U32 i_main_data;                    /* Main data */
	CVI_U32 ft_cap_clk;                     /* FT capture clock */
	CVI_U32 ft_cap_data;                    /* FT capture data */
} CVI_HDMI_HW_PARAM;

/* Structure for HDMI hardware specifications */
typedef struct _CVI_HDMI_HW_SPEC {
	CVI_HDMI_HW_PARAM hw_param[CVI_HDMI_HW_PARAM_NUM]; /* Hardware parameters */
} CVI_HDMI_HW_SPEC;

#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <getopt.h>
#include <inttypes.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <errno.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "cvi_vb.h"
#include "cvi_sys.h"
#include "cvi_vdec.h"
#include "cvi_venc.h"
#include "cvi_buffer.h"



#define MAX_STRING_LEN      255
#define MAX_FILENAME_LEN    64
#define NUM_OF_USER_DATA_BUF 4


#define MAX_OPTIONS 128

typedef struct _commonInputCfg_ {
    CVI_U32 testMode;
    CVI_S32 numChn;
    CVI_S32 ifInitVb;
    CVI_U32 bindmode;
    CVI_U32 u32ViWidth;     // frame width of VI input or VPSS input
    CVI_U32 u32ViHeight;    // frame height of VI input or VPSS input
    CVI_U32 u32VpssWidth;   // frame width of VPSS output
    CVI_U32 u32VpssHeight;  // frame height of VPSS output
    CVI_CHAR yuvFolder[MAX_STRING_LEN];
    CVI_S32 vbMode;
    CVI_S32 bSingleEsBuf_jpege;
    CVI_S32 bSingleEsBuf_h264e;
    CVI_S32 bSingleEsBuf_h265e;
    CVI_S32 singleEsBufSize_jpege;
    CVI_S32 singleEsBufSize_h264e;
    CVI_S32 singleEsBufSize_h265e;
    CVI_S32 h265RefreshType;
    CVI_S32 jpegMarkerOrder;
    CVI_BOOL bThreadDisable;
} commonInputCfg;

typedef struct _chnInputCfg_ {
    char codec[64];
    unsigned int width;
    unsigned int height;
    char input_path[MAX_STRING_LEN];
    char vpssSrcPath[MAX_STRING_LEN];
    char output_path[MAX_STRING_LEN];
    char outputFileName[MAX_STRING_LEN];
    char roiCfgFile[MAX_STRING_LEN];
    char qpMapCfgFile[MAX_STRING_LEN];
    char user_data[NUM_OF_USER_DATA_BUF][MAX_STRING_LEN];
    CVI_S32 num_frames;
    CVI_S32 bsMode;
    CVI_U32 u32Profile;
    CVI_S32 rcMode;
    CVI_S32 iqp;
    CVI_S32 pqp;
    CVI_S32 gop;
    CVI_U32 gopMode;
    CVI_S32 bitrate;
    CVI_S32 minIprop;
    CVI_S32 maxIprop;
    CVI_U32 u32RowQpDelta;
    CVI_S32 firstFrmstartQp;
    CVI_S32 minIqp;
    CVI_S32 maxIqp;
    CVI_S32 minQp;
    CVI_S32 maxQp;
    CVI_S32 framerate;
    CVI_S32 quality;
    CVI_S32 maxbitrate;
    CVI_S32 s32ChangePos;
    CVI_S32 s32MinStillPercent;
    CVI_U32 u32MaxStillQP;
    CVI_U32 u32MotionSensitivity;
    CVI_S32 s32AvbrFrmLostOpen;
    CVI_S32 s32AvbrFrmGap;
    CVI_S32 s32AvbrPureStillThr;
    CVI_S32 statTime;
    CVI_S32 bind_mode;
    CVI_S32 pixel_format;
    CVI_S32 posX;
    CVI_S32 posY;
    CVI_S32 inWidth;
    CVI_S32 inHeight;
    CVI_S32 srcFramerate;
    CVI_U32 bitstreamBufSize;
    CVI_S32 single_LumaBuf;
    CVI_S32 single_core;
    CVI_S32 vpssGrp;
    CVI_S32 vpssChn;
    CVI_S32 forceIdr;
    CVI_S32 chgNum;
    CVI_S32 chgBitrate;
    CVI_S32 chgFramerate;
    CVI_S32 tempLayer;
    CVI_S32 testRoi;
    CVI_S32 bgInterval;
    CVI_S32 frameLost;
    CVI_U32 frameLostGap;
    CVI_U32 frameLostBspThr;
    CVI_S32 MCUPerECS;
    CVI_S32 bCreateChn;
    CVI_S32 getstream_timeout;
    CVI_S32 sendframe_timeout;
    CVI_S32 s32IPQpDelta;
    CVI_S32 s32BgQpDelta;
    CVI_S32 s32ViQpDelta;
    CVI_S32 bVariFpsEn;
    CVI_S32 initialDelay;
    CVI_U32 u32IntraCost;
    CVI_U32 u32ThrdLv;
    CVI_BOOL bBgEnhanceEn;
    CVI_S32 s32BgDeltaQp;
    CVI_U32 h264EntropyMode;
    CVI_S32 h264ChromaQpOffset;
    CVI_S32 h265CbQpOffset;
    CVI_S32 h265CrQpOffset;
    CVI_U32 enSuperFrmMode;
    CVI_U32 u32SuperIFrmBitsThr;
    CVI_U32 u32SuperPFrmBitsThr;
    CVI_S32 s32MaxReEncodeTimes;

    CVI_U8 aspectRatioInfoPresentFlag;
    CVI_U8 aspectRatioIdc;
    CVI_U8 overscanInfoPresentFlag;
    CVI_U8 overscanAppropriateFlag;
    CVI_U16 sarWidth;
    CVI_U16 sarHeight;

    CVI_U8 timingInfoPresentFlag;
    CVI_U8 fixedFrameRateFlag;
    CVI_U32 numUnitsInTick;
    CVI_U32 timeScale;

    CVI_U8 videoSignalTypePresentFlag;
    CVI_U8 videoFormat;
    CVI_U8 videoFullRangeFlag;
    CVI_U8 colourDescriptionPresentFlag;
    CVI_U8 colourPrimaries;
    CVI_U8 transferCharacteristics;
    CVI_U8 matrixCoefficients;

    CVI_U32 u32FrameQp;
    CVI_BOOL bTestUbrEn;

    CVI_BOOL bEsBufQueueEn;
    CVI_BOOL bIsoSendFrmEn;
    CVI_BOOL bSensorEn;

    CVI_U32 u32SliceCnt;
    CVI_U8 bDisableDeblk;
    CVI_S32 betaOffset;
    CVI_S32 alphaOffset;
    CVI_BOOL bIntraPred;
    CVI_U32 u32Rotation;
    CVI_S32 s32Chn;
} chnInputCfg;

typedef enum _SAMPLE_RC_E {
    SAMPLE_RC_CBR = 0,
    SAMPLE_RC_VBR,
    SAMPLE_RC_AVBR,
    SAMPLE_RC_QVBR,
    SAMPLE_RC_FIXQP,
    SAMPLE_RC_QPMAP,
    SAMPLE_RC_UBR,
    SAMPLE_RC_MAX
} SAMPLE_RC_E;


typedef enum _CHN_STATE_ {
    CHN_STAT_NONE = 0,
    CHN_STAT_START,
    CHN_STAT_STOP,
} CHN_STATE;

typedef enum _BS_MODE_ {
    BS_MODE_QUERY_STAT = 0,
    BS_MODE_SELECT,
} BS_MODE;

typedef struct _SAMPLE_COMM_VENC_ROI_ATTR_ {
    VENC_ROI_ATTR_S stVencRoiAttr;
    CVI_U32 u32FrameStart;
    CVI_U32 u32FrameEnd;
} SAMPLE_COMM_VENC_ROI;

#define MAX_NUM_ROI 8

typedef struct _vencChnCtx_ {
    VENC_CHN VencChn;
    SIZE_S stSize;
    VIDEO_FRAME_INFO_S *pstFrameInfo;
    VIDEO_FRAME_S *pstVFrame;
    CVI_U32 u32LumaSize;
    CVI_U32 u32ChrmSize;
    CVI_U32 u32FrameSize;
    CVI_U32 num_frames;
    CVI_S32 s32ChnNum;
    CVI_U32 s32FbCnt;
    CVI_U32 u32Profile;
    PAYLOAD_TYPE_E enPayLoad;
    VENC_GOP_MODE_E enGopMode;
    VENC_GOP_ATTR_S stGopAttr;
    SAMPLE_RC_E enRcMode;
    FILE *fpSrc;
    long file_size;
    FILE *pFile;
    chnInputCfg chnIc;
    PIXEL_FORMAT_E enPixelFormat;
    CHN_STATE chnStat;
    CHN_STATE nextChnStat;
    SAMPLE_COMM_VENC_ROI vencRoi[MAX_NUM_ROI];
    CVI_U8 *pu8QpMap;
    CVI_BOOL bQpMapValid;
    CVI_S32 s32VencFd;
} vencChnCtx;


typedef struct _sampleVenc_ {
    commonInputCfg commonIc;
    vencChnCtx chnCtx[VENC_MAX_CHN_NUM];
} sampleVenc;

// args options
typedef enum _ARG_TYPE_ {
    ARG_INT = 0,
    ARG_UINT,
    ARG_STRING,
} ARG_TYPE;

typedef struct _optionExt_ {
    struct option opt;
    int type;
    int64_t min;
    int64_t max;
    const char *help;
} optionExt;

typedef union {
    int32_t ival;
    uint32_t uval;
} SAMPLE_ARG;


static optionExt vc_long_option_ext[] = {
    {{"codec",     optional_argument, NULL, 'c'}, ARG_STRING, 0,   0,
        "265 = h.265, jpg = jpeg, mjp = motion jpeg" },
    {{"width",     optional_argument, NULL, 'w'}, ARG_UINT,    352, 3840,
        "width"},
    {{"height",    optional_argument, NULL, 'h'}, ARG_UINT,    352, 3840,
        "height"},
    {{"input",     optional_argument, NULL, 'i'}, ARG_STRING, 0,   0,
        "source yuv file"},
    {{"output",    optional_argument, NULL, 'o'}, ARG_STRING, 0,   0,
        "output bitstream"},
    {{"frame_num", optional_argument, NULL, 'n'}, ARG_UINT,    0,   1000000000,
        "number of frame to be encode"},
    {{"getBsMode", optional_argument, NULL, 0},   ARG_UINT,    0,   1,
        "get-bitstream mode, 0 = query status, 1 = select"},
    {{"profile",   optional_argument, NULL, 0},   ARG_INT,
        CVI_H264_PROFILE_MIN,   CVI_H264_PROFILE_MAX,
        "profile, 0 = h264 baseline, 1 = h264 main, 2 = h264 high, Default = 2"},
    {{"rcMode",    optional_argument, NULL, 0},   ARG_INT,    0,   6,
        "rate control mode, 0 = CBR, 1 = VBR, 2 = AVBR, 4 = FIXQP, 5 = QPMAP, 6 = UBR (User BR), default = 4"},
    {{"iqp",       optional_argument, NULL, 0},   ARG_INT,    0,   51,
        "I frame QP"},
    {{"pqp",       optional_argument, NULL, 0},   ARG_INT,    0,   51,
        "P frame QP"},
    {{"ipQpDelta",       optional_argument, NULL, 0},   ARG_INT,
        CVI_H26X_NORMALP_IP_QP_DELTA_MIN,   CVI_H26X_NORMALP_IP_QP_DELTA_MAX,
        "QP Delta between P frame and I frame"},
    {{"bgQpDelta", optional_argument, NULL, 0}, ARG_INT,
        CVI_H26X_SMARTP_BG_QP_DELTA_MIN, CVI_H26X_SMARTP_BG_QP_DELTA_MAX,
        "Smart-P QP delta between P frame and BG (background) frame. [-10, 30], default = 0"},
    {{"viQpDelta", optional_argument, NULL, 0}, ARG_INT,
        CVI_H26X_SMARTP_VI_QP_DELTA_MIN, CVI_H26X_SMARTP_VI_QP_DELTA_MAX,
        "Smart-P QP delta between P frame and VI (virtual I) frame. [-10, 30], default = 0"},
    {{"gop",       optional_argument, NULL, 0},   ARG_INT,
        CVI_H26X_GOP_MIN, CVI_H26X_GOP_MAX,
        "The period of one gop"},
    {{"gopMode",   optional_argument, NULL, 0},   ARG_UINT,
        CVI_H26X_GOP_MODE_MIN, CVI_H26X_GOP_MODE_MAX,
        "GOP mode. 0: Normal P, 2: Smart P, Default: 0"},
    {{"bitrate",   optional_argument, NULL, 0},   ARG_INT,    1,   1000000,
        "The average target bitrate (kbits)"},
    {{"initQp",    optional_argument, NULL, 0},   ARG_INT,    0,   100,
        "The Start Qp of 1st frame, 63 = default"},
    {{"minQp",     optional_argument, NULL, 0},   ARG_INT,    0,   51,
        "Minimum Qp for one frame"},
    {{"maxQp",     optional_argument, NULL, 0},   ARG_INT,    0,   51,
        "Maximum Qp for one frame"},
    {{"minIqp",    optional_argument, NULL, 0},   ARG_INT,    0,   51,
        "Minimum Qp for I frame"},
    {{"maxIqp",    optional_argument, NULL, 0},   ARG_INT,    0,   51,
        "Maximum Qp for I frame"},
    {{"srcFramerate", optional_argument, NULL, 0},   ARG_INT,     0,   240,
        "source frame rate"},
    {{"framerate", optional_argument, NULL, 0},   ARG_INT,    0,   3000000,
        "destination frame rate"},
    {{"vfps", required_argument, NULL, 0},     ARG_INT,   0,   1,
        "enable variable FPS"},
    {{"quality", required_argument, NULL, 0},     ARG_INT,    0,   99,
        "jpeg encode quality"},
    {{"maxbitrate", optional_argument, NULL, 0},  ARG_INT,    0,   1000000,
        "Maximum output bit rate (kbits)"},
    {{"changePos", optional_argument, NULL, 0},  ARG_INT,     50,   100,
        "Ratio to change Qp"},
    {{"minStillPercent", optional_argument, NULL, 0},  ARG_INT,   5,   100,
        "Percentage of target bitrate in low motion"},
    {{"maxStillQp", optional_argument, NULL, 0},  ARG_UINT,   0,   51,
        "Maximum Qp in low motion"},
    {{"motionSense", optional_argument, NULL, 0},  ARG_UINT,      0,   100,
        "Motion sensitivity"},
    {{"avbrFrmLostOpen", optional_argument, NULL, 0},  ARG_INT,   0,   100,
        "avbrFrmLostOpen"},
    {{"avbrFrmGap", optional_argument, NULL, 0},  ARG_INT,    0,   100,
        "avbrFrmGap"},
    {{"avbrPureStillThr", optional_argument, NULL, 0},  ARG_INT,      0,   100,
        "avbrPureStillThr"},
    {{"bgEnhanceEn", optional_argument, NULL, 0},  ARG_INT,
        CVI_H26X_BG_ENHANCE_EN_MIN,   CVI_H26X_BG_ENHANCE_EN_MAX,
        "Enable background enhancement"},
    {{"bgDeltaQp", optional_argument, NULL, 0},  ARG_INT,
        CVI_H26X_BG_DELTA_QP_MIN,   CVI_H26X_BG_DELTA_QP_MAX,
        "background delta qp"},
    {{"statTime", optional_argument, NULL, 0},    ARG_INT,    0,   240,
        "statistics time in seconds"},
    {{"testMode", optional_argument, NULL, 0},    ARG_UINT,   0,   (6 - 1),
        "samele_venc test mode"},
    {{"getstream-timeout", optional_argument, NULL, 0},    ARG_INT,   -1,   100000,
        "samele_venc getstream-timeout   -1:block mode, 0:try_once, >0 timeout in ms"},
    {{"sendframe-timeout", optional_argument, NULL, 0},    ARG_INT,   -1,   100000,
        "samele_venc sendframe-timeout   -1:block mode, 0:try_once, >0 timeout in ms"},
    {{"ifInitVb", optional_argument, NULL, 0},    ARG_INT,    0,   1,
        "if enable VB pool or not"},
    {{"vbMode", optional_argument, NULL, 0},    ARG_INT,      0,   3,
        "if enable VB pool mode. 0 = common, 1 = module, 2 = private, 3 = user"},
    {{"yuvFolder", optional_argument, NULL, 0},   ARG_STRING, 0,   256,
        "yuv files folder"},
    {{"bindmode", optional_argument, NULL, 0},    ARG_UINT,   0,   2,
        "bind mode"},
    {{"pixel_format", optional_argument, NULL, 0}, ARG_INT,   0,   12,
        "0: 420 planar, 1: 422 planar, 2: NV12, 3: NV21"},
    {{"posX", optional_argument, NULL, 0},        ARG_INT,    0, 3840,
        "x axis of start position, need to be multiple of 16 (used for crop)"},
    {{"posY", optional_argument, NULL, 0},        ARG_INT,    0, 3840,
        "y axis of start position, need to be multiple of 16 (used for crop)"},
    {{"inWidth", optional_argument, NULL, 0},     ARG_INT,    352, 3840,
        "width of input frame (used for crop)"},
    {{"inHeight", optional_argument, NULL, 0},    ARG_INT,    288, 3840,
        "height of input frame (used for crop)"},
    {{"bufSize", optional_argument, NULL, 0}, ARG_UINT,    0,   1000000000,
        "bitstream Buffer size"},
    {{"single_LumaBuf", optional_argument, NULL, 0}, ARG_INT,     0,   1,
        "0: disable, 1: use single luma buffer for H264"},
    {{"single_core", optional_argument, NULL, 0}, ARG_INT,    0,   1,
        "0: disable, 1: use single core(h264 or h265 only)"},
    {{"forceIdr", optional_argument, NULL, 0}, ARG_INT,   0,   1000000000,
        "0: disable, > 0: set force idr at number of frame"},
    {{"chgNum", optional_argument, NULL, 0},      ARG_INT,    0, 1000000,
        "frame num to change attr"},
    {{"chgBitrate", optional_argument, NULL, 0},  ARG_INT,    1, 1000000,
        "change bitrate  (kbits)"},
    {{"chgFramerate", optional_argument, NULL, 0},  ARG_INT,      0, 240,
        "change dstframerate"},
    {{"tempLayer", optional_argument, NULL, 0}, ARG_INT,      0,   3,
        "tempLayer"},
    {{"roiCfgFile", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
        "ROI configuration file"},
    {{"qpMapCfgFile", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
        "Roi-based qpMap file"},
    {{"bgInterval", optional_argument, NULL, 0}, ARG_INT,
        CVI_H26X_SMARTP_BG_INTERVAL_MIN, CVI_H26X_SMARTP_BG_INTERVAL_MAX,
        "bgInterval"},
    {{"frame_lost", optional_argument, NULL, 0}, ARG_INT,     0,   1,
        "0: disable, 1: use frame lost(h264 or h265 only)"},
    {{"frame_lost_gap", optional_argument, NULL, 0}, ARG_UINT,    0,   65536,
        "The gap between 2 frame_lost frames(h264 or h265 only)"},
    {{"frame_lost_thr", optional_argument, NULL, 0}, ARG_UINT,    0,   1200000000,
        "frame_lost bsp threshold(h264 or h265 only)"},
    {{"MCUPerECS", required_argument, NULL, 0},   ARG_INT,    0,   1000000,
        "jpeg encode MCUPerECS"},
    {{"single_EsBuf", optional_argument, NULL, 0}, ARG_INT,   0,   1,
        "0: disable, 1: use single stream buffer (jpege)"},
    {{"single_EsBuf_264", optional_argument, NULL, 0}, ARG_INT, 0, 1,
        "0: disable, 1: use single stream buffer (h264e)"},
    {{"single_EsBuf_265", optional_argument, NULL, 0}, ARG_INT, 0, 1,
        "0: disable, 1: use single stream buffer (h265e)"},
    {{"single_EsBufSize", optional_argument, NULL, 0}, ARG_INT, 0, 1000000000,
        "single stream buffer size (jpege)"},
    {{"single_EsBufSize_264", optional_argument, NULL, 0}, ARG_INT, 0, 1000000000,
        "single stream buffer size (h264e)"},
    {{"single_EsBufSize_265", optional_argument, NULL, 0}, ARG_INT, 0, 1000000000,
        "single stream buffer size (h265e)"},
    {{"numChn",    optional_argument, NULL, 0},   ARG_INT,    1,   VENC_MAX_CHN_NUM,
        "number of channels to encode"},
    {{"chn",       optional_argument, NULL, 0},   ARG_UINT,    0,   VENC_MAX_CHN_NUM - 1,
        "set channel-id to configure the following parameters"},
    {{"viWidth",       optional_argument, NULL, 0},   ARG_UINT,    0,   4096,
        "for VI input width"},
    {{"viHeight",       optional_argument, NULL, 0},   ARG_UINT,    0,   2304,
        "for VI input height"},
    {{"vpssWidth",       optional_argument, NULL, 0},   ARG_UINT,    0,   4096,
        "for Vpss output width"},
    {{"vpssHeight",       optional_argument, NULL, 0},   ARG_UINT,    0,   2304,
        "for VPss output height"},
    {{"vpssSrcPath", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
        "source file path for vpss"},
    {{"user_data1", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
        "user data binary file 1"},
    {{"user_data2", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
        "user data binary file 2"},
    {{"user_data3", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
        "user data binary file 3"},
    {{"user_data4", optional_argument, NULL, 0}, ARG_STRING, 0, 0,
        "user data binary file 4"},
    {{"h265RefreshType", optional_argument, NULL, 0}, ARG_INT, 0, 1,
        "0: IDR, 1: CRA, default = 0"},
    {{"initialDelay", optional_argument, NULL, 0}, ARG_INT,
        CVI_INITIAL_DELAY_MIN, CVI_INITIAL_DELAY_MAX,
        "rc initial delay in ms, default = 1000"},
    {{"jpegMarkerOrder", optional_argument, NULL, 0}, ARG_INT, 0, 2,
        "0: Cvitek, 1: SOI-JFIF-DQT_MERGE-SOF0-DHT_MERGE-DRI, 2: Cvitek w/ JFIF, default = 0"},
    {{"intraCost", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H26X_INTRACOST_MIN, CVI_H26X_INTRACOST_MAX,
        "intraCost, the extra cost of intra mode"},
    {{"thrdLv", optional_argument, NULL, 0},
        ARG_UINT, CVI_H26X_THRDLV_MIN, CVI_H26X_THRDLV_MAX,
        "thrdLv, threhold to control block qp"},
    {{"h264EntropyMode", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H264_ENTROPY_MIN, CVI_H264_ENTROPY_MAX,
        "0: CAVLC, 1: CABAC, default = 1"},
    {{"h264ChromaQpOffset", optional_argument, NULL, 0}, ARG_INT, -12, 12,
        "H264 Chroma QP offset [-12, 12], default = 0"},
    {{"h265CbQpOffset", optional_argument, NULL, 0}, ARG_INT, -12, 12,
        "H265 Cb QP offset [-12, 12], default = 0"},
    {{"h265CrQpOffset", optional_argument, NULL, 0}, ARG_INT, -12, 12,
        "H265 Cr QP offset [-12, 12], default = 0"},
    {{"maxIprop", optional_argument, NULL, 0}, ARG_INT, 1, 100,
        "max I frame bitrate ratio to P frame, default = 100"},
    {{"rowQpDelta", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H26X_ROW_QP_DELTA_MIN, CVI_H26X_ROW_QP_DELTA_MAX,
        "rowQpDelta [0, 10], default = 1"},
    {{"superFrmMode", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H26X_SUPER_FRM_MODE_MIN, CVI_H26X_SUPER_FRM_MODE_MAX,
        "superFrmMode, 0 = disable, 3 = encode to IDR, default = 0"},
    {{"superIBitsThr", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H26X_SUPER_I_BITS_THR_MIN, CVI_H26X_SUPER_I_BITS_THR_MAX,
        "superIBitsThr [1000, 33554432], default = 4000000"},
    {{"superPBitsThr", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H26X_SUPER_P_BITS_THR_MIN, CVI_H26X_SUPER_P_BITS_THR_MAX,
        "superPBitsThr [1000, 33554432], default = 4000000"},
    {{"maxReEnc", optional_argument, NULL, 0}, ARG_INT,
        CVI_H26X_MAX_RE_ENCODE_MIN, CVI_H26X_MAX_RE_ENCODE_MAX,
        "maxReEnc [0, 3], default = 0"},
    {{"aspectRatioInfoPresentFlag", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H26X_ASPECT_RATIO_INFO_PRESENT_FLAG_MIN, CVI_H26X_ASPECT_RATIO_INFO_PRESENT_FLAG_MAX,
        "aspect ratio info present flag [0, 1], default = 0"},
    {{"aspectRatioIdc", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H26X_ASPECT_RATIO_IDC_MIN, CVI_H26X_ASPECT_RATIO_IDC_MAX,
        "aspect ratio idc [0, 255], default = 1"},
    {{"overscanInfoPresentFlag", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H26X_OVERSCAN_INFO_PRESENT_FLAG_MIN, CVI_H26X_OVERSCAN_INFO_PRESENT_FLAG_MAX,
        "overscan info present flag [0, 1], default = 0"},
    {{"overscanAppropriateFlag", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H26X_OVERSCAN_APPROPRIATE_FLAG_MIN, CVI_H26X_OVERSCAN_APPROPRIATE_FLAG_MAX,
        "overscan appropriate flag [0, 1], default = 0"},
    {{"sarWidth", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H26X_SAR_WIDTH_MIN, CVI_H26X_SAR_WIDTH_MAX,
        "sar width [0, 65535], default = 1"},
    {{"sarHeight", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H26X_SAR_HEIGHT_MIN, CVI_H26X_SAR_HEIGHT_MAX,
        "sar height [0, 65535], default = 1"},
    {{"timingInfoPresentFlag", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H26X_TIMING_INFO_PRESENT_FLAG_MIN, CVI_H26X_TIMING_INFO_PRESENT_FLAG_MAX,
        "timing info present flag [0, 1], default = 0"},
    {{"fixedFrameRateFlag", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H264_FIXED_FRAME_RATE_FLAG_MIN, CVI_H264_FIXED_FRAME_RATE_FLAG_MAX,
        "fixed frame rate flag [0, 1], default = 0"},
    {{"numUnitsInTick", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H26X_NUM_UNITS_IN_TICK_MIN, CVI_H26X_NUM_UNITS_IN_TICK_MAX,
        "num units in tick [0, 4294967295], default = 1"},
    {{"timeScale", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H26X_TIME_SCALE_MIN, CVI_H26X_TIME_SCALE_MAX,
        "time scale [0, 4294967295], default = 60"},
    {{"videoSignalTypePresentFlag", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H26X_VIDEO_SIGNAL_TYPE_PRESENT_FLAG_MIN, CVI_H26X_VIDEO_SIGNAL_TYPE_PRESENT_FLAG_MAX,
        "video signal type present flag [0, 1], default = 0"},
    {{"videoFormat", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H26X_VIDEO_FORMAT_MIN, CVI_H264_VIDEO_FORMAT_MAX,
        "video format [0, 7], default = 5"},
    {{"videoFullRangeFlag", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H26X_VIDEO_FULL_RANGE_FLAG_MIN, CVI_H26X_VIDEO_FULL_RANGE_FLAG_MAX,
        "video full range flag [0, 1], default = 0"},
    {{"colourDescriptionPresentFlag", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H26X_COLOUR_DESCRIPTION_PRESENT_FLAG_MIN, CVI_H26X_COLOUR_DESCRIPTION_PRESENT_FLAG_MAX,
        "colour description present flag [0, 1], default = 0"},
    {{"colourPrimaries", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H26X_COLOUR_PRIMARIES_MIN, CVI_H26X_COLOUR_PRIMARIES_MAX,
        "colour primaries [0, 255], default = 2"},
    {{"transferCharacteristics", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H26X_TRANSFER_CHARACTERISTICS_MIN, CVI_H26X_TRANSFER_CHARACTERISTICS_MAX,
        "transfer characteristics [0, 255], default = 2"},
    {{"matrixCoefficients", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H26X_MATRIX_COEFFICIENTS_MIN, CVI_H26X_MATRIX_COEFFICIENTS_MAX,
        "matrix coefficients [0, 255], default = 2"},
    {{"testUbrEn", optional_argument, NULL, 0}, ARG_INT,
        CVI_H26X_TEST_UBR_EN_MIN, CVI_H26X_TEST_UBR_EN_MAX,
        "enable to test ubr [0, 1], default = 0"},
    {{"frameQp", optional_argument, NULL, 0}, ARG_UINT,
        CVI_H26X_FRAME_QP_MIN, CVI_H26X_FRAME_QP_MAX,
        "frameQp [0, 51], default = 38"},
    {{"esBufQueueEn", optional_argument, NULL, 0}, ARG_INT,
        CVI_H26X_ES_BUFFER_QUEUE_MIN, CVI_H26X_ES_BUFFER_QUEUE_MAX,
        "esBufQueueEn [0, 1], default = 1"},
    {{"isoSendFrmEn", optional_argument, NULL, 0}, ARG_INT,
        CVI_H26X_ISO_SEND_FRAME_MIN, CVI_H26X_ISO_SEND_FRAME_MAX,
        "isoSendFrmEn [0, 1], default = 1"},
    {{"sensorEn", optional_argument, NULL, 0}, ARG_INT,
        CVI_H26X_SENSOR_EN_MIN, CVI_H26X_SENSOR_EN_MAX,
        "sensorEn [0, 1], default = 0"},
    {{"sliceSplitCnt", optional_argument, NULL, 0}, ARG_INT, 0, 5,
        "sliceSplitCnt [1, 5], default = 1"},
    {{"disabledblk", optional_argument, NULL, 0}, ARG_INT, 0, 1,
        "disabledblk [0, 1], default = 0"},
    {{"betaOffset", optional_argument, NULL, 0}, ARG_INT, -6, 6,
        "betaOffset [-6, 6], default = 0"},
    {{"alphaoffset", optional_argument, NULL, 0}, ARG_INT, -6, 6,
        "alphaoffset [-6, 6], default = 0"},
    {{"intraPred", optional_argument, NULL, 0}, ARG_INT, 0, 1,
        "intraPred [0, 1], default = 0"},
    {{"rotation", optional_argument, NULL, 0}, ARG_INT, 0, 3,
        "rotation [0, 3], default = 0"},
    {{NULL, 0, NULL, 0}, ARG_INT, 0, 0, ""}

};

void print_help(char * const *argv)
{
    uint32_t idx;

    printf("// ------------------------------------------------\n");
    printf("// %s -c codec -w width -h height -i src.yuv -o enc\n", argv[0]);
    printf("EX.\n");
    printf("sample_venc -c 265 -w 1920 -h 1080 -i ReadySteadyGo_1920x1080_600.yuv -o enc\n");
    printf("// ------------------------------------------------\n");

    for (idx = 0; idx < sizeof(vc_long_option_ext) / sizeof(optionExt); idx++) {
        if (vc_long_option_ext[idx].opt.name == NULL)
            break;

        printf("--%s\n", vc_long_option_ext[idx].opt.name);
        printf("    %s\n", vc_long_option_ext[idx].help);
    }
}

static int32_t checkArg(int32_t entryIdx, SAMPLE_ARG *pArg)
{
    //printf("entryIdx = %d\n", entryIdx);

    if (vc_long_option_ext[entryIdx].type == ARG_INT) {
        pArg->ival = strtoimax(optarg, NULL, 10);
        if ((int64_t)(pArg->ival) < vc_long_option_ext[entryIdx].min ||
            (int64_t)(pArg->ival) > vc_long_option_ext[entryIdx].max) {
            printf("%s = %d, min = %" PRId64 ", max = %" PRId64 "\n",
                    vc_long_option_ext[entryIdx].opt.name,
                    pArg->ival,
                    vc_long_option_ext[entryIdx].min,
                    vc_long_option_ext[entryIdx].max);
            return -1;
        }
    } else if (vc_long_option_ext[entryIdx].type == ARG_UINT) {
        pArg->uval = strtoumax(optarg, NULL, 10);
        if ((int64_t)(pArg->uval) < vc_long_option_ext[entryIdx].min ||
            (int64_t)(pArg->uval) > vc_long_option_ext[entryIdx].max) {
            printf("%s = %u, min = %" PRId64 ", max = %" PRId64 "\n",
                    vc_long_option_ext[entryIdx].opt.name,
                    pArg->uval,
                    vc_long_option_ext[entryIdx].min,
                    vc_long_option_ext[entryIdx].max);
            return -1;
        }
    } else if (vc_long_option_ext[entryIdx].type == ARG_STRING) {
        if (optarg == NULL) {
            printf("%s = NULL\n", vc_long_option_ext[entryIdx].opt.name);
            return -1;
        }
    }

    return 0;
}


CVI_S32 parseEncArgv(sampleVenc *psv, chnInputCfg *pIc, CVI_S32 argc, char **argv)
{
    CVI_S32 ch, idx, ret;
    SAMPLE_ARG arg = { 0 };
    struct option long_options[MAX_OPTIONS + 1];

    memset((void *)long_options, 0, sizeof(long_options));

    printf("\n");

    for (idx = 0; idx < MAX_OPTIONS; idx++) {
        if (vc_long_option_ext[idx].opt.name == NULL)
            break;

        if (idx >= MAX_OPTIONS) {
            printf("too many options\n");
            return -1;
        }

        memcpy(&long_options[idx], &vc_long_option_ext[idx].opt, sizeof(struct option));
    }

    while ((ch = getopt_long(argc, argv, "c:w:h:i:o:n:", long_options, &idx)) != -1) {
        //printf("ch = %c, idx = %d, optind = %d\n", ch, idx, optind);

        switch (ch) {
        case 'c':
            strcpy(pIc->codec, optarg);
            break;
        case 'w':
            pIc->width = atoi(optarg);
            break;
        case 'h':
            pIc->height = atoi(optarg);
            break;
        case 'i':
            strcpy(pIc->input_path, optarg);
            break;
        case 'o':
            strcpy(pIc->output_path, optarg);
            break;
        case 'n':
            pIc->num_frames = atoi(optarg);
            break;
        case 0:
            ret = checkArg(idx, &arg);
            if (ret != CVI_SUCCESS) {
                printf("checkArg, %d\n", ret);
                print_help(argv);
                return ret;
            }
            //printf("idx %d name:%s\n", idx, long_options[idx].name);
            if (!strcmp(long_options[idx].name, "getBsMode")) {
                pIc->bsMode = arg.ival;
                //printf("bsMode = %d\n", pIc->bsMode);
            } else if (!strcmp(long_options[idx].name, "profile")) {
                pIc->u32Profile = arg.uval;
            } else if (!strcmp(long_options[idx].name, "rcMode")) {
                pIc->rcMode = arg.ival;
            } else if (!strcmp(long_options[idx].name, "iqp")) {
                pIc->iqp = arg.ival;
            } else if (!strcmp(long_options[idx].name, "pqp")) {
                pIc->pqp = arg.ival;
            } else if (!strcmp(long_options[idx].name, "ipQpDelta")) {
                pIc->s32IPQpDelta = arg.ival;
            } else if (!strcmp(long_options[idx].name, "bgQpDelta")) {
                pIc->s32BgQpDelta = arg.ival;
            } else if (!strcmp(long_options[idx].name, "viQpDelta")) {
                pIc->s32ViQpDelta = arg.ival;
            } else if (!strcmp(long_options[idx].name, "gop")) {
                pIc->gop = arg.ival;
            } else if (!strcmp(long_options[idx].name, "gopMode")) {
                pIc->gopMode = arg.uval;
            } else if (!strcmp(long_options[idx].name, "bitrate")) {
                pIc->bitrate = arg.ival;
                //printf("bitrate = %d\n", pIc->bitrate);
            } else if (!strcmp(long_options[idx].name, "initQp")) {
                pIc->firstFrmstartQp = arg.ival;
            } else if (!strcmp(long_options[idx].name, "minIqp")) {
                pIc->minIqp = arg.ival;
            } else if (!strcmp(long_options[idx].name, "maxIqp")) {
                pIc->maxIqp = arg.ival;
            } else if (!strcmp(long_options[idx].name, "minQp")) {
                pIc->minQp = arg.ival;
            } else if (!strcmp(long_options[idx].name, "maxQp")) {
                pIc->maxQp = arg.ival;
            } else if (!strcmp(long_options[idx].name, "srcFramerate")) {
                pIc->srcFramerate = arg.ival;
            } else if (!strcmp(long_options[idx].name, "framerate")) {
                pIc->framerate = arg.ival;
            } else if (!strcmp(long_options[idx].name, "vfps")) {
                pIc->bVariFpsEn = arg.ival;
            } else if (!strcmp(long_options[idx].name, "quality")) {
                pIc->quality = arg.ival;
            } else if (!strcmp(long_options[idx].name, "maxbitrate")) {
                pIc->maxbitrate = arg.ival;
            } else if (!strcmp(long_options[idx].name, "changePos")) {
                pIc->s32ChangePos = arg.ival;
            } else if (!strcmp(long_options[idx].name, "minStillPercent")) {
                pIc->s32MinStillPercent = arg.ival;
            } else if (!strcmp(long_options[idx].name, "maxStillQp")) {
                pIc->u32MaxStillQP = arg.uval;
            } else if (!strcmp(long_options[idx].name, "motionSense")) {
                pIc->u32MotionSensitivity = arg.uval;
            } else if (!strcmp(long_options[idx].name, "avbrFrmLostOpen")) {
                pIc->s32AvbrFrmLostOpen = arg.ival;
            } else if (!strcmp(long_options[idx].name, "avbrFrmGap")) {
                pIc->s32AvbrFrmGap = arg.ival;
            } else if (!strcmp(long_options[idx].name, "avbrPureStillThr")) {
                pIc->s32AvbrPureStillThr = arg.ival;
            } else if (!strcmp(long_options[idx].name, "statTime")) {
                pIc->statTime = arg.ival;
            } else if (!strcmp(long_options[idx].name, "testMode")) {
                psv->commonIc.testMode = arg.uval;
            } else if (!strcmp(long_options[idx].name, "getstream-timeout")) {
                pIc->getstream_timeout = arg.ival;
            } else if (!strcmp(long_options[idx].name, "sendframe-timeout")) {
                pIc->sendframe_timeout = arg.ival;
            } else if (!strcmp(long_options[idx].name, "ifInitVb")) {
                psv->commonIc.ifInitVb = arg.ival;
            } else if (!strcmp(long_options[idx].name, "vbMode")) {
                psv->commonIc.vbMode = arg.ival;
            } else if (!strcmp(long_options[idx].name, "yuvFolder")) {
                strcpy(psv->commonIc.yuvFolder, optarg);
            } else if (!strcmp(long_options[idx].name, "bindmode")) {
                psv->commonIc.bindmode = arg.uval;
            } else if (!strcmp(long_options[idx].name, "pixel_format")) {
                pIc->pixel_format = arg.ival;
            } else if (!strcmp(long_options[idx].name, "posX")) {
                pIc->posX = arg.ival;
            } else if (!strcmp(long_options[idx].name, "posY")) {
                pIc->posY = arg.ival;
            } else if (!strcmp(long_options[idx].name, "inWidth")) {
                pIc->inWidth = arg.ival;
            } else if (!strcmp(long_options[idx].name, "inHeight")) {
                pIc->inHeight = arg.ival;
            } else if (!strcmp(long_options[idx].name, "bufSize")) {
                pIc->bitstreamBufSize = arg.uval;
            } else if (!strcmp(long_options[idx].name, "single_core")) {
                pIc->single_core = arg.ival;
            } else if (!strcmp(long_options[idx].name, "single_LumaBuf")) {
                pIc->single_LumaBuf = arg.ival;
            } else if (!strcmp(long_options[idx].name, "forceIdr")) {
                pIc->forceIdr = arg.ival;
            } else if (!strcmp(long_options[idx].name, "chgNum")) {
                pIc->chgNum = arg.ival;
            } else if (!strcmp(long_options[idx].name, "chgBitrate")) {
                pIc->chgBitrate = arg.ival;
            } else if (!strcmp(long_options[idx].name, "chgFramerate")) {
                pIc->chgFramerate = arg.ival;
            } else if (!strcmp(long_options[idx].name, "tempLayer")) {
                pIc->tempLayer = arg.ival;
            } else if (!strcmp(long_options[idx].name, "roiCfgFile")) {
                strcpy(pIc->roiCfgFile, optarg);
            } else if (!strcmp(long_options[idx].name, "qpMapCfgFile")) {
                strcpy(pIc->qpMapCfgFile, optarg);
            } else if (!strcmp(long_options[idx].name, "bgInterval")) {
                pIc->bgInterval = arg.ival;
            } else if (!strcmp(long_options[idx].name, "frame_lost")) {
                pIc->frameLost = arg.ival;
            } else if (!strcmp(long_options[idx].name, "frame_lost_gap")) {
                pIc->frameLostGap = arg.uval;
            } else if (!strcmp(long_options[idx].name, "frame_lost_thr")) {
                pIc->frameLostBspThr = arg.uval;
            } else if (!strcmp(long_options[idx].name, "MCUPerECS")) {
                pIc->MCUPerECS = arg.ival;
            } else if (!strcmp(long_options[idx].name, "single_EsBuf")) {
                psv->commonIc.bSingleEsBuf_jpege = arg.ival;
            } else if (!strcmp(long_options[idx].name, "single_EsBuf_264")) {
                psv->commonIc.bSingleEsBuf_h264e = arg.ival;
            } else if (!strcmp(long_options[idx].name, "single_EsBuf_265")) {
                psv->commonIc.bSingleEsBuf_h265e = arg.ival;
            } else if (!strcmp(long_options[idx].name, "single_EsBufSize")) {
                psv->commonIc.singleEsBufSize_jpege = arg.ival;
            } else if (!strcmp(long_options[idx].name, "single_EsBufSize_264")) {
                psv->commonIc.singleEsBufSize_h264e = arg.ival;
            } else if (!strcmp(long_options[idx].name, "single_EsBufSize_265")) {
                psv->commonIc.singleEsBufSize_h265e = arg.ival;
            } else if (!strcmp(long_options[idx].name, "numChn")) {
                psv->commonIc.numChn = arg.ival;
            } else if (!strcmp(long_options[idx].name, "chn")) {
                pIc = &psv->chnCtx[arg.uval].chnIc;
                pIc->s32Chn = arg.uval;
            } else if (!strcmp(long_options[idx].name, "viWidth")) {
                psv->commonIc.u32ViWidth = arg.uval;
            } else if (!strcmp(long_options[idx].name, "viHeight")) {
                psv->commonIc.u32ViHeight = arg.uval;
            } else if (!strcmp(long_options[idx].name, "vpssWidth")) {
                psv->commonIc.u32VpssWidth = arg.uval;
            } else if (!strcmp(long_options[idx].name, "vpssHeight")) {
                psv->commonIc.u32VpssHeight = arg.uval;
            } else if (!strcmp(long_options[idx].name, "vpssSrcPath")) {
                strcpy(pIc->vpssSrcPath, optarg);
            } else if (!strcmp(long_options[idx].name, "user_data1")) {
                strcpy(pIc->user_data[0], optarg);
            } else if (!strcmp(long_options[idx].name, "user_data2")) {
                strcpy(pIc->user_data[1], optarg);
            } else if (!strcmp(long_options[idx].name, "user_data3")) {
                strcpy(pIc->user_data[2], optarg);
            } else if (!strcmp(long_options[idx].name, "user_data4")) {
                strcpy(pIc->user_data[3], optarg);
            } else if (!strcmp(long_options[idx].name, "h265RefreshType")) {
                psv->commonIc.h265RefreshType = arg.ival;
            } else if (!strcmp(long_options[idx].name, "initialDelay")) {
                pIc->initialDelay = arg.ival;
            } else if (!strcmp(long_options[idx].name, "jpegMarkerOrder")) {
                psv->commonIc.jpegMarkerOrder = arg.ival;
            } else if (!strcmp(long_options[idx].name, "intraCost")) {
                pIc->u32IntraCost = arg.uval;
            } else if (!strcmp(long_options[idx].name, "thrdLv")) {
                pIc->u32ThrdLv = arg.uval;
            } else if (!strcmp(long_options[idx].name, "bgEnhanceEn")) {
                pIc->bBgEnhanceEn = arg.ival;
            } else if (!strcmp(long_options[idx].name, "bgDeltaQp")) {
                pIc->s32BgDeltaQp = arg.ival;
            } else if (!strcmp(long_options[idx].name, "h264EntropyMode")) {
                pIc->h264EntropyMode = arg.uval;
            } else if (!strcmp(long_options[idx].name, "h264ChromaQpOffset")) {
                pIc->h264ChromaQpOffset = arg.ival;
            } else if (!strcmp(long_options[idx].name, "h265CbQpOffset")) {
                pIc->h265CbQpOffset = arg.ival;
            } else if (!strcmp(long_options[idx].name, "h265CrQpOffset")) {
                pIc->h265CrQpOffset = arg.ival;
            } else if (!strcmp(long_options[idx].name, "maxIprop")) {
                pIc->maxIprop = arg.ival;
            } else if (!strcmp(long_options[idx].name, "rowQpDelta")) {
                pIc->u32RowQpDelta = arg.uval;
            } else if (!strcmp(long_options[idx].name, "superFrmMode")) {
                pIc->enSuperFrmMode = arg.uval;
            } else if (!strcmp(long_options[idx].name, "superIBitsThr")) {
                pIc->u32SuperIFrmBitsThr = arg.uval;
            } else if (!strcmp(long_options[idx].name, "superPBitsThr")) {
                pIc->u32SuperPFrmBitsThr = arg.uval;
            } else if (!strcmp(long_options[idx].name, "maxReEnc")) {
                pIc->s32MaxReEncodeTimes = arg.ival;
            } else if (!strcmp(long_options[idx].name, "aspectRatioInfoPresentFlag")) {
                pIc->aspectRatioInfoPresentFlag = arg.uval;
            } else if (!strcmp(long_options[idx].name, "aspectRatioIdc")) {
                pIc->aspectRatioIdc = arg.uval;
            } else if (!strcmp(long_options[idx].name, "overscanInfoPresentFlag")) {
                pIc->overscanInfoPresentFlag = arg.uval;
            } else if (!strcmp(long_options[idx].name, "overscanAppropriateFlag")) {
                pIc->overscanAppropriateFlag = arg.uval;
            } else if (!strcmp(long_options[idx].name, "sarWidth")) {
                pIc->sarWidth = arg.uval;
            } else if (!strcmp(long_options[idx].name, "sarHeight")) {
                pIc->sarHeight = arg.uval;
            } else if (!strcmp(long_options[idx].name, "timingInfoPresentFlag")) {
                pIc->timingInfoPresentFlag = arg.uval;
            } else if (!strcmp(long_options[idx].name, "fixedFrameRateFlag")) {
                pIc->fixedFrameRateFlag = arg.uval;
            } else if (!strcmp(long_options[idx].name, "numUnitsInTick")) {
                pIc->numUnitsInTick = arg.uval;
            } else if (!strcmp(long_options[idx].name, "timeScale")) {
                pIc->timeScale = arg.uval;
            } else if (!strcmp(long_options[idx].name, "videoSignalTypePresentFlag")) {
                pIc->videoSignalTypePresentFlag = arg.uval;
            } else if (!strcmp(long_options[idx].name, "videoFormat")) {
                pIc->videoFormat = arg.uval;
            } else if (!strcmp(long_options[idx].name, "videoFullRangeFlag")) {
                pIc->videoFullRangeFlag = arg.uval;
            } else if (!strcmp(long_options[idx].name, "colourDescriptionPresentFlag")) {
                pIc->colourDescriptionPresentFlag = arg.uval;
            } else if (!strcmp(long_options[idx].name, "colourPrimaries")) {
                pIc->colourPrimaries = arg.uval;
            } else if (!strcmp(long_options[idx].name, "transferCharacteristics")) {
                pIc->transferCharacteristics = arg.uval;
            } else if (!strcmp(long_options[idx].name, "matrixCoefficients")) {
                pIc->matrixCoefficients = arg.uval;
            } else if (!strcmp(long_options[idx].name, "testUbrEn")) {
                pIc->bTestUbrEn = arg.ival;
            } else if (!strcmp(long_options[idx].name, "frameQp")) {
                pIc->u32FrameQp = arg.uval;
            } else if (!strcmp(long_options[idx].name, "esBufQueueEn")) {
                pIc->bEsBufQueueEn = arg.uval;
            } else if (!strcmp(long_options[idx].name, "isoSendFrmEn")) {
                pIc->bIsoSendFrmEn = arg.uval;
            } else if (!strcmp(long_options[idx].name, "sensorEn")) {
                pIc->bSensorEn = arg.uval;
            } else if (!strcmp(long_options[idx].name, "sliceSplitCnt")) {
                pIc->u32SliceCnt = arg.uval;
            } else if (!strcmp(long_options[idx].name, "disabledblk")) {
                pIc->bDisableDeblk = arg.uval;
            } else if (!strcmp(long_options[idx].name, "betaOffset")) {
                pIc->betaOffset = arg.ival;
            } else if (!strcmp(long_options[idx].name, "alphaoffset")) {
                pIc->alphaOffset = arg.ival;
            } else if (!strcmp(long_options[idx].name, "intraPred")) {
                pIc->bIntraPred = arg.uval;
            } else if (!strcmp(long_options[idx].name, "rotation")) {
                pIc->u32Rotation = arg.uval;
            } else {
                printf("not exist name = %s\n", long_options[idx].name);
                print_help(argv);
                return -1;
            }
            break;
        default:
            printf("ch = %c\n", ch);
            print_help(argv);
            break;
        }

    }

    if (optind < argc) {
        printf("optind %d argc %d\n", optind, argc);
        print_help(argv);
    }

    return 0;
}

// Map command line input pixel format to PIXEL_FORMAT_E.
static PIXEL_FORMAT_E vencMapPixelFormat(CVI_S32 pixel_format)
{
    PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;

    switch (pixel_format) {
    case 0:
        enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
        break;
    case 1:
        enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_422;
        break;
    case 2:
        enPixelFormat = PIXEL_FORMAT_NV12;
        break;
    case 3:
        enPixelFormat = PIXEL_FORMAT_NV21;
        break;
    case 4:
        enPixelFormat = PIXEL_FORMAT_NV16;
        break;
    case 5:
        enPixelFormat = PIXEL_FORMAT_NV61;
        break;
    case 6:
        enPixelFormat = PIXEL_FORMAT_YUYV;
        break;
    case 7:
        enPixelFormat = PIXEL_FORMAT_UYVY;
        break;
    case 8:
        enPixelFormat = PIXEL_FORMAT_YVYU;
        break;
    case 9:
        enPixelFormat = PIXEL_FORMAT_VYUY;
        break;
    case 10:
        enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_444;
        break;
    case 11:
        enPixelFormat = PIXEL_FORMAT_YUV_400;
        break;
    default:
        printf("Unknown input pixel format. Assume YUV420P.\n");
        enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
        break;
    }

    return enPixelFormat;
}




void InitCommonInputCfg(commonInputCfg *pCic)
{
    if (!pCic) {
        printf("pCic = NULL\n");
        return;
    }

    memset(pCic, 0, sizeof(commonInputCfg));
    pCic->ifInitVb = 1;
    pCic->vbMode = VB_SOURCE_COMMON;
    pCic->bSingleEsBuf_jpege = 0;
    pCic->bSingleEsBuf_h264e = 0;
    pCic->bSingleEsBuf_h265e = 0;
    pCic->singleEsBufSize_jpege = 0;
    pCic->singleEsBufSize_h264e = 0;
    pCic->singleEsBufSize_h265e = 0;
    pCic->h265RefreshType = 0;
    pCic->jpegMarkerOrder = 0;
    pCic->bThreadDisable = 0;
}

void InitChnInputCfg(chnInputCfg *pIc)
{
    if (!pIc) {
        printf("pIc = NULL\n");
        return;
    }

    memset(pIc, 0, sizeof(chnInputCfg));
    pIc->u32Profile = CVI_H264_PROFILE_DEFAULT;
    pIc->rcMode = -1;
    pIc->iqp = -1;
    pIc->pqp = -1;
    pIc->gop = CVI_H26X_GOP_DEFAULT;
    pIc->gopMode = CVI_H26X_GOP_MODE_DEFAULT;
    pIc->bitrate = -1;
    pIc->firstFrmstartQp = -1;
    pIc->num_frames = 1;
    pIc->framerate = 30;
    pIc->bVariFpsEn = 0;
    pIc->maxIprop = CVI_H26X_MAX_I_PROP_DEFAULT;
    pIc->minIprop = CVI_H26X_MIN_I_PROP_DEFAULT;
    pIc->maxQp = -1;
    pIc->minQp = -1;
    pIc->maxIqp = -1;
    pIc->minIqp = -1;
    pIc->quality = -1;
    pIc->maxbitrate = -1;
    pIc->statTime = -1;
    pIc->bind_mode = VENC_BIND_DISABLE;
    pIc->pixel_format = 0;
    pIc->bitstreamBufSize = 0;
    pIc->single_LumaBuf = 0;
    pIc->single_core = 0;
    pIc->forceIdr = -1;
    pIc->chgNum = -1;
    pIc->tempLayer = 0;
    pIc->bgInterval = CVI_H26X_SMARTP_BG_INTERVAL_DEFAULT;
    pIc->frameLost = -1;
    pIc->frameLostBspThr = -1;
    pIc->frameLostGap = -1;
    pIc->MCUPerECS = 0;
    pIc->sendframe_timeout = 20000;
    pIc->getstream_timeout = -1;
    pIc->s32IPQpDelta = CVI_H26X_NORMALP_IP_QP_DELTA_DEFAULT;
    pIc->s32BgQpDelta = CVI_H26X_SMARTP_BG_QP_DELTA_DEFAULT;
    pIc->s32ViQpDelta = CVI_H26X_SMARTP_VI_QP_DELTA_DEFAULT;
    pIc->initialDelay = CVI_INITIAL_DELAY_DEFAULT;
    pIc->h264EntropyMode = H264E_ENTROPY_CABAC;
    pIc->h264ChromaQpOffset = 0;
    pIc->h265CbQpOffset = 0;
    pIc->h265CrQpOffset = 0;
    pIc->u32RowQpDelta = CVI_H26X_ROW_QP_DELTA_DEFAULT;
    pIc->enSuperFrmMode = CVI_H26X_SUPER_FRM_MODE_DEFAULT;
    pIc->u32SuperIFrmBitsThr = CVI_H26X_SUPER_I_BITS_THR_DEFAULT;
    pIc->u32SuperPFrmBitsThr = CVI_H26X_SUPER_P_BITS_THR_DEFAULT;
    pIc->s32MaxReEncodeTimes = CVI_H26X_MAX_RE_ENCODE_DEFAULT;

    pIc->aspectRatioInfoPresentFlag = CVI_H26X_ASPECT_RATIO_INFO_PRESENT_FLAG_DEFAULT;
    pIc->aspectRatioIdc = CVI_H26X_ASPECT_RATIO_IDC_DEFAULT;
    pIc->overscanInfoPresentFlag = CVI_H26X_OVERSCAN_INFO_PRESENT_FLAG_DEFAULT;
    pIc->overscanAppropriateFlag = CVI_H26X_OVERSCAN_APPROPRIATE_FLAG_DEFAULT;
    pIc->sarWidth = CVI_H26X_SAR_WIDTH_DEFAULT;
    pIc->sarHeight = CVI_H26X_SAR_HEIGHT_DEFAULT;

    pIc->timingInfoPresentFlag = CVI_H26X_TIMING_INFO_PRESENT_FLAG_DEFAULT;
    pIc->fixedFrameRateFlag = CVI_H264_FIXED_FRAME_RATE_FLAG_DEFAULT;
    pIc->numUnitsInTick = CVI_H26X_NUM_UNITS_IN_TICK_DEFAULT;
    pIc->timeScale = CVI_H26X_TIME_SCALE_DEFAULT;

    pIc->videoSignalTypePresentFlag = CVI_H26X_VIDEO_SIGNAL_TYPE_PRESENT_FLAG_DEFAULT;
    pIc->videoFormat = CVI_H26X_VIDEO_FORMAT_DEFAULT;
    pIc->videoFullRangeFlag = CVI_H26X_VIDEO_FULL_RANGE_FLAG_DEFAULT;
    pIc->colourDescriptionPresentFlag = CVI_H26X_COLOUR_DESCRIPTION_PRESENT_FLAG_DEFAULT;
    pIc->colourPrimaries = CVI_H26X_COLOUR_PRIMARIES_DEFAULT;
    pIc->transferCharacteristics = CVI_H26X_TRANSFER_CHARACTERISTICS_DEFAULT;
    pIc->matrixCoefficients = CVI_H26X_MATRIX_COEFFICIENTS_DEFAULT;

    pIc->u32FrameQp = CVI_H26X_FRAME_QP_DEFAULT;
    pIc->bTestUbrEn = CVI_H26X_TEST_UBR_EN_DEFAULT;
    pIc->bEsBufQueueEn = CVI_H26X_ES_BUFFER_QUEUE_DEFAULT;
    pIc->bIsoSendFrmEn = CVI_H26X_ISO_SEND_FRAME_DEFAUL;
    pIc->bSensorEn = CVI_H26X_SENSOR_EN_DEFAULT;

    pIc->u32SliceCnt = 1;
    pIc->bIntraPred = 0;
    pIc->u32Rotation = 0;
    pIc->s32Chn = 0;
}


CVI_VOID initInputCfg(commonInputCfg *pcic, chnInputCfg *pIc)
{
    InitCommonInputCfg(pcic);
    InitChnInputCfg(pIc);
}



CVI_S32 SAMPLE_VENC_INIT_CFG(sampleVenc *psv, int argc, char **argv)
{
    chnInputCfg *pIc;
    commonInputCfg *pcic = &psv->commonIc;

    memset(psv, 0, sizeof(*psv));

    for (CVI_S32 s32ChnIdx = 0; s32ChnIdx < VENC_MAX_CHN_NUM; s32ChnIdx++) {
        pIc = &psv->chnCtx[s32ChnIdx].chnIc;
        initInputCfg(pcic, pIc);
    }

    pIc = &psv->chnCtx[0].chnIc;
    if (parseEncArgv(psv, pIc, argc, argv) < 0) {
        printf("parseEncArgv\n");
        return -1;
    }
#if 0
    if (pcic->numChn == 0) {
        if (pcic->testMode == VCODEC_SINGLE_STREAM_MODE ||
            pcic->testMode == JPEG_CONTI_ENCODE_MODE)
            pcic->numChn = 1;
    }
#endif
    if (pcic->numChn < 0) {
        printf("vcodec config failed\n");
        return -1;
    }

    return 0;
}


static CVI_S32 cviReadSrcFrame(VIDEO_FRAME_S *pstVFrame, FILE *fp)
{
    size_t read_byte;
    CVI_U8 *frm_ptr;
    CVI_U32 j;
    CVI_U32 u32CbCrReadSrcHeight;
    CVI_U32 bCbWidthShift, bCrWidthShift;
    CVI_U32 u32LumaRead = 1;
    CVI_U32 to_read; // bytes

    switch (pstVFrame->enPixelFormat) {
    case PIXEL_FORMAT_YUV_PLANAR_422:
        u32CbCrReadSrcHeight = pstVFrame->u32Height;
        bCbWidthShift = 1;
        bCrWidthShift = 1;
        break;
    case PIXEL_FORMAT_NV12:
    case PIXEL_FORMAT_NV21:
        u32CbCrReadSrcHeight = pstVFrame->u32Height >> 1;
        bCbWidthShift = 0;
        bCrWidthShift = 31;
        break;
    case PIXEL_FORMAT_NV16:
    case PIXEL_FORMAT_NV61:
        u32CbCrReadSrcHeight = pstVFrame->u32Height;
        bCbWidthShift = 0;
        bCrWidthShift = 31;
        break;
    case PIXEL_FORMAT_YUYV:
    case PIXEL_FORMAT_UYVY:
    case PIXEL_FORMAT_YVYU:
    case PIXEL_FORMAT_VYUY:
        u32LumaRead = 2;
        u32CbCrReadSrcHeight = pstVFrame->u32Height;
        bCbWidthShift = 31;
        bCrWidthShift = 31;
        break;
    case PIXEL_FORMAT_YUV_400:
        u32CbCrReadSrcHeight = pstVFrame->u32Height;
        bCbWidthShift = 31;
        bCrWidthShift = 31;
        break;
    case PIXEL_FORMAT_YUV_PLANAR_444:
        u32CbCrReadSrcHeight = pstVFrame->u32Height;
        bCbWidthShift = 0;
        bCrWidthShift = 0;
        break;
    case PIXEL_FORMAT_YUV_PLANAR_420:
    default:
        u32CbCrReadSrcHeight = pstVFrame->u32Height >> 1;
        bCbWidthShift = 1;
        bCrWidthShift = 1;
        break;
    }

    if (pstVFrame->u32Width == pstVFrame->u32Stride[0]) {
        // Luma
        frm_ptr = pstVFrame->pu8VirAddr[0];
        to_read = pstVFrame->u32Width * pstVFrame->u32Height * u32LumaRead;
        read_byte = fread((void *)frm_ptr, 1, to_read, fp);
        if (read_byte != to_read) {
            printf("Luma, fread %zu %d failed\n", read_byte, to_read);
            return CVI_FAILURE;
        }
        CVI_SYS_IonFlushCache(pstVFrame->u64PhyAddr[0], pstVFrame->pu8VirAddr[0], read_byte);

        // Cb
        frm_ptr = pstVFrame->pu8VirAddr[1];
        to_read = (pstVFrame->u32Width * u32CbCrReadSrcHeight) >> bCbWidthShift;
        read_byte = fread((void *)frm_ptr, 1, to_read, fp);
        if (read_byte != to_read) {
            printf("Cb, fread %zu %d failed\n", read_byte, to_read);
            return CVI_FAILURE;
        }
        CVI_SYS_IonFlushCache(pstVFrame->u64PhyAddr[1], pstVFrame->pu8VirAddr[1], read_byte);

        // Cr
        frm_ptr = pstVFrame->pu8VirAddr[2];
        to_read = (pstVFrame->u32Width * u32CbCrReadSrcHeight) >> bCrWidthShift;
        read_byte = fread((void *)frm_ptr, 1, to_read, fp);
        if (read_byte != to_read) {
            printf("Cr, fread %zu %d failed\n", read_byte, to_read);
            return CVI_FAILURE;
        }
        CVI_SYS_IonFlushCache(pstVFrame->u64PhyAddr[2], pstVFrame->pu8VirAddr[2], read_byte);
        } else {
            // Luma
            for (j = 0; j < pstVFrame->u32Height; j++) {
                frm_ptr = pstVFrame->pu8VirAddr[0] + j * pstVFrame->u32Stride[0];
                to_read = pstVFrame->u32Width * u32LumaRead;
                read_byte = fread((void *)frm_ptr, 1, to_read, fp);
                if (read_byte != to_read) {
                    printf("Luma, (row %d) fread %zu %d failed\n",
                        j, read_byte, to_read);
                    return CVI_FAILURE;
                }
            }

            // Cb
            for (j = 0; j < u32CbCrReadSrcHeight; j++) {
                frm_ptr = pstVFrame->pu8VirAddr[1] + j * (pstVFrame->u32Stride[0] >> bCbWidthShift);
                to_read = (pstVFrame->u32Width%2)? ((pstVFrame->u32Width+1)>> bCbWidthShift): ((pstVFrame->u32Width+1)>> bCbWidthShift);
                read_byte = fread((void *)frm_ptr, 1, to_read, fp);
                if (read_byte != to_read) {
                    printf("Cb, (row %d) fread %zu %d failed\n",
                        j, read_byte, to_read);
                    return CVI_FAILURE;
                }
            }

            // Cr
            for (j = 0; j < u32CbCrReadSrcHeight; j++) {
                frm_ptr = pstVFrame->pu8VirAddr[2] + j * (pstVFrame->u32Stride[0] >> bCrWidthShift);
                to_read = (pstVFrame->u32Width%2)? ((pstVFrame->u32Width+1)>> bCrWidthShift): ((pstVFrame->u32Width)>> bCrWidthShift);
                read_byte = fread((void *)frm_ptr, 1, to_read, fp);
                if (read_byte != to_read) {
                    printf("Cr, (row %d) fread %zu %d failed\n",
                        j, read_byte, to_read);
                    return CVI_FAILURE;
                }
            }
      }

    return CVI_SUCCESS;
}



int SubProcessChannel(chnInputCfg *pIc)
{
    VENC_CHN VeChn = pIc->s32Chn;
    VENC_CHN_ATTR_S stAttr = {0};
    VENC_RECV_PIC_PARAM_S stRecvParam = {0};
    VENC_STREAM_S stStream = {0};
    VIDEO_FRAME_INFO_S stFrame = {0};
    VB_POOL_CONFIG_S cfg = {0};
    VB_POOL pool = VB_INVALID_POOLID;
    VB_BLK blk = VB_INVALID_HANDLE;
    FILE *fpInput = NULL;
    CVI_S32 s32MilliSec = -1;
    FILE *fpOutput = NULL;
    CVI_U32 ySize = 0;
    VB_CAL_CONFIG_S stVbCfg;
    CVI_U32 uvSize = 0;
    CVI_S32 s32ReadLen;
    CVI_S32 ret = 0;
    CVI_S32 s32NumFrames = pIc->num_frames;
    CVI_U32 i;
    PIXEL_FORMAT_E enPixelFormat;

    CVI_S32 s32Ret;
    VB_CONFIG_S stVbConf = {0};

    printf("sub %d process: pid:%d chn:%d width:%d height:%d codec:%s input:%s output:%s num:%d\n",
        __LINE__, getpid(),pIc->s32Chn, pIc->width, pIc->height, pIc->codec, pIc->input_path, pIc->output_path, s32NumFrames);

    CVI_VB_Exit();

    fpInput = fopen(pIc->input_path, "rb");
    if (fpInput == NULL) {
        printf("can't open file %s\n", pIc->input_path);
        CVI_VB_Exit();
        return -1;
    }

    fpOutput = fopen(pIc->output_path, "wb");
    if (fpOutput == NULL) {
        printf("can't open file %s\n", pIc->output_path);
        return -1;
    }

    stStream.pstPack = malloc(8 * sizeof(VENC_PACK_S));
    if (stStream.pstPack == NULL) {
        printf("CVI_VENC_CreateChn FAIL: 0x%x\n", ret);
        return -1;
    }

    if (strncmp(pIc->codec, "264", 3) == 0)
        stAttr.stVencAttr.enType = PT_H264;
    else if (strncmp(pIc->codec, "265", 3) == 0)
        stAttr.stVencAttr.enType = PT_H265;
    else if (strncmp(pIc->codec, "mjp", 3) == 0
        || strncmp(pIc->codec, "jpg", 3) == 0)
        stAttr.stVencAttr.enType = PT_JPEG;
    else {
        printf("unsupport type: %s\n", pIc->codec);
        CVI_VB_Exit();
        return -1;
    }
    enPixelFormat = vencMapPixelFormat(pIc->pixel_format);
    if(enPixelFormat != PIXEL_FORMAT_YUV_PLANAR_420
        && enPixelFormat != PIXEL_FORMAT_NV12
        && enPixelFormat != PIXEL_FORMAT_NV12 && stAttr.stVencAttr.enType != PT_JPEG
        && stAttr.stVencAttr.enType != PT_MJPEG) {
        printf("encode type:%s not support format:%d.\n", pIc->codec, enPixelFormat);
        return -2;
    }

    stAttr.stVencAttr.u32MaxPicWidth = pIc->width;
     stAttr.stVencAttr.u32MaxPicHeight = pIc->height;
     stAttr.stVencAttr.u32BufSize = pIc->width * pIc->height;
     stAttr.stVencAttr.u32PicWidth = pIc->width;
     stAttr.stVencAttr.u32PicHeight = pIc->height;
     stAttr.stVencAttr.enPixelFormat = enPixelFormat;
     stAttr.stVencAttr.enRotation = (ROTATION_E)pIc->u32Rotation;

     if (stAttr.stVencAttr.enType == PT_H264)
         stAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
     else if (stAttr.stVencAttr.enType == PT_H265)
         stAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
     stAttr.stRcAttr.stH264Cbr.bVariFpsEn = 0;
     stAttr.stRcAttr.stH264Cbr.fr32DstFrameRate = 30;
     stAttr.stRcAttr.stH264Cbr.u32BitRate = 4000;
     stAttr.stRcAttr.stH264Cbr.u32Gop = 60;
     stAttr.stRcAttr.stH264Cbr.u32SrcFrameRate = 30;
     stAttr.stRcAttr.stH264Cbr.u32StatTime = 2;
     stAttr.stGopAttr.enGopMode = VENC_GOPMODE_NORMALP;
     stAttr.stGopAttr.stNormalP.s32IPQpDelta = 2;
     stAttr.stGopExAttr.u32GopPreset = GOP_PRESET_IDX_IPPPP;

     ret = CVI_VENC_CreateChn(VeChn, &stAttr);
     if (ret != CVI_SUCCESS) {
         printf("CVI_VENC_CreateChn FAIL: 0x%x\n", ret);
         return -1;
     }

     ret = CVI_VENC_StartRecvFrame(VeChn, &stRecvParam);
     if (ret != CVI_SUCCESS) {
         printf("CVI_VENC_StartRecvFrame FAIL: 0x%x\n", ret);
         return -1;
     }

     memset(&stVbCfg, 0, sizeof(stVbCfg));
     COMMON_GetPicBufferConfig(pIc->width, pIc->height, enPixelFormat,
         DATA_BITWIDTH_8, COMPRESS_MODE_NONE, 8, &stVbCfg);
     stVbConf.u32MaxPoolCnt = 1;
     stVbConf.astCommPool[0].u32BlkSize = stVbCfg.u32VBSize;
     stVbConf.astCommPool[0].u32BlkCnt = 1;
     stVbConf.astCommPool[0].enRemapMode = VB_REMAP_MODE_CACHED;
     s32Ret = CVI_VB_SetConfig(&stVbConf);
     if (s32Ret != CVI_SUCCESS) {
         printf("CVI_VB_SetConf failed!\n");
         return s32Ret;
     }

     s32Ret = CVI_VB_Init();
     if (s32Ret != CVI_SUCCESS) {
         printf("CVI_VB_Init failed!\n");
         return s32Ret;
     }

     cfg.u32BlkSize = stVbCfg.u32VBSize;
     cfg.u32BlkCnt = 1;
     cfg.enRemapMode = VB_REMAP_MODE_NOCACHE;
     pool = CVI_VB_CreatePool(&cfg);
     if (pool == VB_INVALID_POOLID) {
         printf("CVI_VB_CreatePool NG.\n");
         return -1;
     }

     ret = CVI_VB_MmapPool(pool);
     if (ret != CVI_SUCCESS) {
         printf("CVI_VB_MmapPool NG\n");
         return -1;
     }

     blk = CVI_VB_GetBlock(pool, stVbCfg.u32VBSize);
     if (blk == VB_INVALID_HANDLE) {
         printf("Can't acquire vb block\n");
         CVI_VB_Exit();
         return -2;
     }
     printf("blk x size:%u Ystride:%u vStride:%u Ysize:%u Csize:%u\n",
         stVbCfg.u32VBSize, stVbCfg.u32MainStride, stVbCfg.u32CStride, stVbCfg.u32MainSize, stVbCfg.u32MainCSize);

     stFrame.stVFrame.u32Width = stAttr.stVencAttr.u32PicWidth;
     stFrame.stVFrame.u32Height = stAttr.stVencAttr.u32PicHeight;
     stFrame.stVFrame.enPixelFormat = enPixelFormat;
     stFrame.stVFrame.u32Stride[0] = stVbCfg.u32MainStride;
     stFrame.stVFrame.u32Stride[1] = stVbCfg.u32CStride;
     stFrame.stVFrame.u32Stride[2] = stVbCfg.u32CStride;
     stFrame.stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
     stFrame.stVFrame.u64PhyAddr[1] = stFrame.stVFrame.u64PhyAddr[0] + stVbCfg.u32MainYSize;
     stFrame.stVFrame.u64PhyAddr[2] = stFrame.stVFrame.u64PhyAddr[1] + stVbCfg.u32MainCSize;
     stFrame.u32PoolId = CVI_VB_Handle2PoolId(blk);
     stFrame.stVFrame.enPixelFormat = enPixelFormat;

     ret = CVI_VB_GetBlockVirAddr(pool, blk, (void **)&(stFrame.stVFrame.pu8VirAddr[0]));
     if (ret != CVI_SUCCESS) {
         printf("CVI_VB_GetBlockVirAddr NG, ret:%d\n", ret);
         return -1;
     }

     stFrame.stVFrame.pu8VirAddr[1] = stFrame.stVFrame.pu8VirAddr[0] + stVbCfg.u32MainYSize;
     stFrame.stVFrame.pu8VirAddr[2] = stFrame.stVFrame.pu8VirAddr[1] + stVbCfg.u32MainCSize;

     fseek(fpInput, 0, SEEK_SET);
    ret = cviReadSrcFrame(&stFrame.stVFrame, fpInput);
    if (ret != CVI_SUCCESS) {
        fseek(fpInput, 0, SEEK_SET);
    }

    CVI_SYS_IonFlushCache(stFrame.stVFrame.u64PhyAddr[0], stFrame.stVFrame.pu8VirAddr[0], stVbCfg.u32MainSize+stVbCfg.u32MainCSize*2);

    while(s32NumFrames)
    {
        #if 0
        fseek(fpInput, 0, SEEK_SET);
        ret = cviReadSrcFrame(&stFrame.stVFrame, fpInput);
        if (ret != CVI_SUCCESS) {
            fseek(fpInput, 0, SEEK_SET);
            continue;
        }

        CVI_SYS_IonFlushCache(stFrame.stVFrame.u64PhyAddr[0], stFrame.stVFrame.pu8VirAddr[0], stVbCfg.u32MainSize+stVbCfg.u32MainCSize*2);
        #endif

        ret = CVI_VENC_SendFrame(VeChn, &stFrame, s32MilliSec);
        if (ret != CVI_SUCCESS) {
            printf("CVI_VENC_SendFrame FAIL: 0x%x\n", ret);
            return -1;
        }
        s32NumFrames--;

        ret = CVI_VENC_GetStream(VeChn, &stStream, 1000);
        if (ret != CVI_SUCCESS) {
            printf("CVI_VENC_GetStream FAIL: 0x%x\n", ret);
            if(ret == CVI_ERR_VENC_BUSY
                || ret == CVI_ERR_VENC_UNEXIST) {
                usleep(100*1000);
                continue;
            }
            return -1;
        }

        #if 0
        for (i=0; i<stStream.u32PackCount; i++) {
                fwrite(stStream.pstPack[i].pu8Addr , 1, stStream.pstPack[i].u32Len, fpOutput);
        }
        #endif
        ret = CVI_VENC_ReleaseStream(VeChn, &stStream);
        if (ret != CVI_SUCCESS) {
            printf("CVI_VENC_ReleaseStream FAIL: 0x%x\n", ret);
            return -1;
        }
    }

    fclose(fpInput);
    fclose(fpOutput);
    ret = CVI_VENC_StopRecvFrame(VeChn);
    if (ret != CVI_SUCCESS) {
        printf("CVI_VENC_StopRecvFrame FAIL: 0x%x\n", ret);
        return -1;
    }
    ret = CVI_VENC_DestroyChn(VeChn);
    if (ret != CVI_SUCCESS) {
        printf("CVI_VENC_DestroyChn FAIL: 0x%x\n", ret);
        return -1;
    }

    CVI_VB_ReleaseBlock(blk);

    ret = CVI_VB_DestroyPool(pool);
    if (ret != CVI_SUCCESS) {
        printf("CVI_VB_DestroyPool NG\n");
        CVI_VB_Exit();
        return -1;
    }
    CVI_VB_Exit();
    printf("[%d] process: pid:%d exit\n", __LINE__, getpid());

    return 0;
}

int main(int argc, char *argv[])
{
    sampleVenc sv, *psv = &sv;
    commonInputCfg *pcic = &psv->commonIc;
    CVI_S32 s32Ret = CVI_SUCCESS;
    int i;
    pid_t child_pids[32];

    s32Ret = SAMPLE_VENC_INIT_CFG(psv, argc, argv);
    if (s32Ret < 0) {
        printf("SAMPLE_VENC_INIT_CFG\n");
        return s32Ret;
    }

    for (i = 0; i < pcic->numChn; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            // 错误处理
            fprintf(stderr, "create sub process failed\n");
            exit(1);
        } else if (pid == 0) {
            chnInputCfg *pIc = &psv->chnCtx[i].chnIc;
            SubProcessChannel(pIc);
            exit(0);
        } else {
            child_pids[i] = pid;
        }
    }

    int num_completed = 0;
    while (num_completed < pcic->numChn) {
        for (i = 0; i < pcic->numChn; i++) {
            int status;
            pid_t result = waitpid(child_pids[i], &status, WNOHANG);

            if (result <= 0) {
                usleep(5000);
                continue;
            } else {
                if (WIFEXITED(status)) {
                    printf("sub process %d finished, exit status:%d\n", result, WEXITSTATUS(status));
                } else if (WIFSIGNALED(status)) {
                    printf("sub process %d finished, signal:%d\n", result, WTERMSIG(status));
                }
                num_completed++;
            }
        }
    }
    printf("exit suc.\n");

    return 0;
}

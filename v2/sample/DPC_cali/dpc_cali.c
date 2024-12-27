
#include <dirent.h>
#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/time.h>
#include <unistd.h>

#include "isp_algo_dpc.h"
#include "cvi_ae.h"
#include "dpcm_api.h"
#include "sample_comm.h"

#define DELAY_MS(x) (usleep(x * 1000))
#define DELAY_S(x) (usleep(x * 1000 * 1000))
#define BAD_RATIO (0.9)
#define TIME_LIMIT (25)
#define COMPRESS_MODE (1)       // 0: NONE / 1: TILE
#define SAVE_RAW_ONLINE (0)     // 0: FALSE / 1: TRUE
#define VI_INPUT_PIPE (0)

typedef struct {
    CVI_U32 pixel;
    CVI_U32 cnt;
} PIXEL_CNT_S;

static VI_PIPE ViPipe;

static SAMPLE_VI_CONFIG_S g_stViConfig;
static SAMPLE_INI_CFG_S g_stIniCfg;

static int sys_vi_init(void);
static void sys_vi_deinit(void);
static void print_usage(char *sPrgNm);

static int dpc_cali_offline(int argc, char **argv);
static int dpc_cali_online(int argc, char **argv);

static CVI_S32 isp_getrawbuffer(VI_PIPE ViPipe, CVI_U8 *ImgBuffer, BAYER_FORMAT_E *bayerFormat);
static CVI_S32 dpc_dump_raw(VI_PIPE ViPipe, CVI_U16 *ImgBuffer,
                    CVI_U16 width, CVI_U16 height, BAYER_FORMAT_E bayerFormat);


int main(int argc, char **argv)
{
    CVI_S32 ret = CVI_SUCCESS;

    // args check
    if (argc < 3) {
        printf("%d\n", argc);
        print_usage(argv[0]);
        return CVI_FAILURE;
    }

    int mode = atoi(argv[1]);

    if (mode == 0) {
        ret = dpc_cali_offline(argc, argv);
    } else if (mode == 1) {
        ret = dpc_cali_online(argc, argv);
    } else {
        print_usage(argv[0]);
        return CVI_FAILURE;
    }

    return ret;
}

static int sys_vi_init(void)
{
    MMF_VERSION_S stVersion;
    SAMPLE_INI_CFG_S stIniCfg;
    SAMPLE_VI_CONFIG_S stViConfig;

    PIC_SIZE_E enPicSize;
    SIZE_S stSize;
    CVI_S32 s32Ret = CVI_SUCCESS;
    LOG_LEVEL_CONF_S log_conf;

    CVI_SYS_GetVersion(&stVersion);
    SAMPLE_PRT("MMF Version:%s\n", stVersion.version);

    log_conf.enModId = CVI_ID_LOG;
    log_conf.s32Level = CVI_DBG_INFO;
    CVI_LOG_SetLevelConf(&log_conf);

    // Get config from ini if found.
    CVI_CHAR iniPath[256] = {0};

    snprintf(iniPath, sizeof(iniPath), "sensor_cfg.ini");
    SAMPLE_COMM_VI_SetIniPath(iniPath);
    // parse config file
    if (SAMPLE_COMM_VI_ParseIni(&stIniCfg)) {
        SAMPLE_PRT("Parse complete\n");
    }

    // Set sensor number
    CVI_VI_SetDevNum(stIniCfg.devNum);

    /************************************************
     * step1:  Config VI
     ************************************************/
    s32Ret = SAMPLE_COMM_VI_IniToViCfg(&stIniCfg, &stViConfig);
    if (s32Ret != CVI_SUCCESS)
        return s32Ret;

    if (COMPRESS_MODE == 1)
        stViConfig.astViInfo[0].stChnInfo.enCompressMode = COMPRESS_MODE_TILE;
    else
        stViConfig.astViInfo[0].stChnInfo.enCompressMode = COMPRESS_MODE_NONE;

    memcpy(&g_stViConfig, &stViConfig, sizeof(SAMPLE_VI_CONFIG_S));
    memcpy(&g_stIniCfg, &stIniCfg, sizeof(SAMPLE_INI_CFG_S));

    /************************************************
     * step2:  Get input size
     ************************************************/
    s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(stIniCfg.enSnsType[0], &enPicSize);
    if (s32Ret != CVI_SUCCESS) {
        CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VI_GetSizeBySensor failed with %#x\n", s32Ret);
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);
    if (s32Ret != CVI_SUCCESS) {
        CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_SYS_GetPicSize failed with %#x\n", s32Ret);
        return s32Ret;
    }

    /************************************************
     * step3:  Init modules
     ************************************************/
    s32Ret = SAMPLE_PLAT_SYS_INIT(stSize);
    if (s32Ret != CVI_SUCCESS) {
        CVI_TRACE_LOG(CVI_DBG_ERR, "sys init failed. s32Ret: 0x%x !\n", s32Ret);
        return s32Ret;
    }
    s32Ret = SAMPLE_PLAT_VI_INIT(&stViConfig);
    if (s32Ret != CVI_SUCCESS) {
        CVI_TRACE_LOG(CVI_DBG_ERR, "vi init failed. s32Ret: 0x%x !\n", s32Ret);
        return s32Ret;
    }

    return CVI_SUCCESS;
}

static void sys_vi_deinit(void)
{
    SAMPLE_COMM_VI_DestroyIsp(&g_stViConfig);

    SAMPLE_COMM_VI_DestroyVi(&g_stViConfig);

    SAMPLE_COMM_SYS_Exit();
}

static void print_usage(char *sPrgNm)
{
    printf("Usage :\n");
    printf("offline mode: dump raws by user\n");
    printf("\t:%s <mode> <threshold> <decttype> <input_raw_path> <output_table_path> <nbit> <width> <height>\n",
           sPrgNm);
    printf("online mode: dump raws by ISP\n");
    printf("\t:%s <mode> <threshold> <decttype> <bright_pixel_txt> <dark_pixel_txt>\n", sPrgNm);
    printf("dptype:\n");
    printf("mode:\n");
    printf("\t0: offline mode\n");
    printf("\t1: online mode\n");
    printf("threshold:\n\tthe absolute threshold value\n");
    printf("decttype:\n");
    printf("\t0: detect bright pixel in dark raws\n");
    printf("\t1: detect dark pixel in birght raws\n");
    printf(
        "input_raw_path/bright_pixel_txt:\n\t the raw dir path in offline mode\n\t the brigth table output path in online mode\n");
    printf(
        "output_table_path/dark_pixel_txt:\n\tthe output table path in offline mode\n\t the dark table output path in online mode\n");
    printf("nbit:\n\tthe positive pixel bit number in offline mode\n\tonly support 12 or 16 bits\n");
    printf("width:\n\tthe positive width of raw file in offline mode\n");
    printf("height:\n\tthe positive height of raw file in offline mode\n");
}

static int dpc_cali_offline(int argc, char **argv)
{
    // check args
    CVI_S32 ret = CVI_SUCCESS;

    printf("offline mode\n");
    if (argc != 9) {
        printf("%d\n", argc);
        print_usage(argv[0]);
        return CVI_FAILURE;
    }
    int threshold = atoi(argv[2]);
    int dptype = atoi(argv[3]);
    char *input_raw_path = argv[4];
    char *output_table_path = argv[5];
    int nbit = atoi(argv[6]);
    int width = atoi(argv[7]);
    int height = atoi(argv[8]);
    int stride = width;

    printf("threshold=%d, dptype=%d, nbit=%d, width=%d, height=%d\n", threshold, dptype, nbit, width, height);
    if (threshold <= 0 || (dptype != 0 && dptype != 1) || (nbit != 16 && nbit != 12) || width <= 0 || height <= 0) {
        printf("%d, %d, %d, %d, %d\n", threshold, dptype, nbit, width, height);
        print_usage(argv[0]);
        return CVI_FAILURE;
    }

    // init calibration resource
    char filename[256];
    FILE *fp = NULL;
    CVI_U32 raw_img_cnt = 0;
    uint16_t *image = NULL;
    uint32_t *table = NULL;
    PIXEL_CNT_S *pStBadPixel = NULL;
    uint8_t *tmpImage = NULL;
    float ratio = 1.5;
    int bad_cnt = 0;

    ssize_t size = stride * height * sizeof(uint16_t);

    image = (uint16_t *)calloc(1, size);

    if (!image) {
        printf("malloc image failed\n");
        goto exit;
    }
    table = (uint32_t *)calloc(1, sizeof(uint32_t) * STATIC_DP_COUNT_MAX);
    if (!table) {
        printf("malloc table failed\n");
        goto exit;
    }
    pStBadPixel = (PIXEL_CNT_S *)calloc(1, sizeof(PIXEL_CNT_S) * STATIC_DP_COUNT_MAX);
    if (!pStBadPixel) {
        printf("malloc pBadPixelSt failed\n");
        goto exit;
    }
    tmpImage = calloc(1, stride * height * ratio);
    if (!tmpImage) {
        printf("can't alloc tmpImage with size = %zu\n", size);
        goto exit;
    }

    DPC_Input input = {
        .image = image,
        .width = width,
        .height = height,
        .stride = stride,
        .abs_thresh = threshold << 2,
        .dpType = dptype,
        .table_max_size = STATIC_DP_COUNT_MAX,
        .verbose = 0,
        .debug = 0,
    };
    DPC_Output output = {
        .table = table,
        .table_size = STATIC_DP_COUNT_MAX,
    };

    // read raw dir
    struct dirent *raw_file;
    DIR *raw_dir = opendir(input_raw_path);

    if (!raw_dir) {
        printf("open input raw dir failed\n");
        goto exit;
    }

    // do calibration for raws in dir
    while ((raw_file = readdir(raw_dir)) != NULL) {
        if (strcmp(raw_file->d_name, ".") == 0 || strcmp(raw_file->d_name, "..") == 0) {
            continue;
        }

        raw_img_cnt++;
        memset(filename, 0, sizeof(filename));
        memset(image, 0, size);
        memset(table, 0, sizeof(uint32_t) * STATIC_DP_COUNT_MAX);
        snprintf(filename, sizeof(filename) + 1, "%s%s", input_raw_path, raw_file->d_name);
        printf("read %s\n", filename);
        fp = fopen(filename, "rb");
        if (!fp) {
            printf("open %s failed\n", filename);
            goto exit;
        }
        if (nbit == 16) {
            ssize_t readSize = fread(image, 1, size, fp);

            if (readSize != size) {
                printf("readSize=%zu, size=%zu\n", readSize, size);
                goto exit;
            }
        } else if (nbit == 12) {
            printf("bits = 12\n");
            memset(tmpImage, 0, stride * height * ratio);
            ssize_t readSize = fread(tmpImage, 1, stride * height * ratio, fp);

            if (readSize != (stride * height * ratio)) {
                printf("readSize=%zu, size=%f\n", readSize, (stride * height * ratio));
                goto exit;
            }

            Bayer_12bit_2_16bit(tmpImage, image, width, height, stride);
        }

        DPC_Calib(&input, &output);

        for (CVI_U32 i = 0; i < output.table_size; ++i) {
            for (CVI_U32 j = 0; j < STATIC_DP_COUNT_MAX; ++j) {
                if ((pStBadPixel + j)->pixel == 0) {
                    (pStBadPixel + j)->pixel = table[i];
                    (pStBadPixel + j)->cnt += 1;
                    break;
                } else if ((pStBadPixel + j)->pixel == table[i]) {
                    (pStBadPixel + j)->cnt += 1;
                    break;
                }
            }
        }
        fclose(fp);
        fp = NULL;
    }

    memset(table, 0, sizeof(uint32_t) * STATIC_DP_COUNT_MAX);
    for (CVI_U32 j = 0; j < STATIC_DP_COUNT_MAX; ++j) {
        if ((pStBadPixel + j)->pixel == 0) {
            break;
        } else if ((pStBadPixel + j)->cnt > raw_img_cnt * BAD_RATIO) {
            table[bad_cnt] = (pStBadPixel + j)->pixel;
            bad_cnt++;
        }
    }

    // save table to as txt file
    printf("bad_cnt=%d\n", bad_cnt);
    fp = fopen(output_table_path, "w");
    if (!fp) {
        printf("open %s failed\n", output_table_path);
        goto exit;
    }
    for (CVI_U32 i = 0; i < STATIC_DP_COUNT_MAX; ++i) {
        fprintf(fp, "%d", table[i]);
        if (i != STATIC_DP_COUNT_MAX - 1) {
            fprintf(fp, ",");
        }
    }
    fclose(fp);
    fp = NULL;

    // release resource
    if (image)
        free(image);
    if (table)
        free(table);
    if (pStBadPixel)
        free(pStBadPixel);
    if (tmpImage)
        free(tmpImage);
    return ret;

exit:
    if (image)
        free(image);
    if (table)
        free(table);
    if (pStBadPixel)
        free(pStBadPixel);
    if (tmpImage)
        free(tmpImage);
    if (fp)
        fclose(fp);
    return CVI_FAILURE;
};

static int dpc_cali_online(int argc, char **argv)
{
    // check args
    CVI_S32 ret = CVI_SUCCESS;
    if (argc != 6) {
        print_usage(argv[0]);
        return CVI_FAILURE;
    }
    int threshold = atoi(argv[2]);
    int dptype = atoi(argv[3]);
    char *bright_raw_path = argv[4];
    char *dark_table_path = argv[5];
    if (threshold <= 0 || (dptype != 0 && dptype != 1)) {
        print_usage(argv[0]);
        return CVI_FAILURE;
    }

    // init calibration source
    ViPipe = (VI_PIPE)VI_INPUT_PIPE;
    FILE *fp = NULL;
    char *table_path = NULL;
    // call api
    uint32_t *table = NULL;
    PIXEL_CNT_S *pStBadPixel = NULL;
    CVI_U16 *ImgBuffer16 = NULL;
    CVI_BOOL saveFileEn = CVI_FALSE;
    CVI_U32 raw_image_cnt = 0;
    CVI_U16 bad_cnt = 0;
    BAYER_FORMAT_E bayerFormat;

    if ((ViPipe < 0) || (ViPipe >= VI_MAX_PIPE_NUM)) {
        printf("ViPipe %d value error\n", ViPipe);
        return CVI_FAILURE;
    }

	// calibrate bright/dark
    if (dptype == 0) {
        table_path = bright_raw_path;
    } else {
        table_path = dark_table_path;
    }
    if (SAVE_RAW_ONLINE)
        saveFileEn = CVI_TRUE;

    ret = sys_vi_init();
    if (ret != CVI_SUCCESS) {
        printf("sys_vi_init failed\n");
        return ret;
    }

    PIC_SIZE_E enPicSize;
    SIZE_S stSize;
    SAMPLE_COMM_VI_GetSizeBySensor(g_stIniCfg.enSnsType[0], &enPicSize);
    SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);
    CVI_U16 width = (CVI_U16)stSize.u32Width;
    CVI_U16 height = (CVI_U16)stSize.u32Height;
    CVI_U16 stride = width; //unit is pixel

    table = (uint32_t *)calloc(1, sizeof(uint32_t) * STATIC_DP_COUNT_MAX);
    if (!table) {
        printf("malloc table failed\n");
        goto ERROR;
    }

    pStBadPixel = (PIXEL_CNT_S *)calloc(1, sizeof(PIXEL_CNT_S) * STATIC_DP_COUNT_MAX);
    if (!pStBadPixel) {
        printf("malloc pBadPixelSt failed\n");
        goto ERROR;
    }

    ImgBuffer16 = calloc(1, width * height * sizeof(CVI_U16));    //12bit data save in 16bit
    if (ImgBuffer16 == NULL) {
        printf("Calloc failed, ImgBuffer16(%p)\n", (void *)ImgBuffer16);
        goto ERROR;
    }

	//Set and Record Pub/AE Settings
    ISP_PUB_ATTR_S stPubAttr = {}, stPubAttrOrig = {};
    ISP_EXPOSURE_ATTR_S stExpAttr = {}, stExpAttrOrig = {};
    if (CVI_ISP_GetPubAttr(ViPipe, &stPubAttrOrig) != CVI_SUCCESS) {
        printf("CVI_ISP_GetPubAttr Pipe: %d fail\n", ViPipe);
    }
    stPubAttr = stPubAttrOrig;
    stPubAttr.f32FrameRate = 5;
    if (CVI_ISP_SetPubAttr(ViPipe, &stPubAttr) != CVI_SUCCESS) {
        printf("CVI_ISP_SetPubAttr Pipe: %d for DPC fail\n", ViPipe);
    }
    if (CVI_ISP_GetExposureAttr(ViPipe, &stExpAttrOrig) != CVI_SUCCESS) {
        printf("CVI_ISP_GetExposureAttr Pipe: %d fail\n", ViPipe);
    }
    stExpAttr = stExpAttrOrig;
    stExpAttr.enOpType = OP_TYPE_MANUAL;
    stExpAttr.stManual.enExpTimeOpType = OP_TYPE_MANUAL;
    stExpAttr.stManual.enGainType = AE_TYPE_GAIN;
    stExpAttr.stManual.enAGainOpType = OP_TYPE_MANUAL;
    stExpAttr.stManual.enDGainOpType = OP_TYPE_MANUAL;
    stExpAttr.stManual.enISPDGainOpType = OP_TYPE_MANUAL;
    stExpAttr.stManual.u32ExpTime = 200000;     // 200ms
    stExpAttr.stManual.u32AGain = 0x400;        // 1x
    stExpAttr.stManual.u32DGain = 0x400;        // 1x
    stExpAttr.stManual.u32ISPDGain = 0x400;     // 1x

    if (CVI_ISP_SetExposureAttr(ViPipe, &stExpAttr) != CVI_SUCCESS) {
        printf("CVI_ISP_SetExposureAttr Pipe: %d for DPC fail\n", ViPipe);
    }
    usleep(500000);     // 500ms

    DPC_Input input = {
        .image = ImgBuffer16,
        .width = width,
        .height = height,
        .stride = stride,
        .abs_thresh = (threshold << 2),
        .dpType = dptype,
        .table_max_size = STATIC_DP_COUNT_MAX,
        .verbose = 0,
        .debug = 0,
    };
    
    int CurTimes = 0;
    int EndTimes = TIME_LIMIT;
    do {
        raw_image_cnt++;
        memset(table, 0, sizeof(uint32_t) * STATIC_DP_COUNT_MAX);
        if (isp_getrawbuffer(ViPipe, (CVI_U8 *)ImgBuffer16, &bayerFormat) != CVI_SUCCESS) {
            printf("isp_getrawbuffer timeout\n");
            continue;
        }
        
        if (saveFileEn)
            dpc_dump_raw(ViPipe, ImgBuffer16, width, height, bayerFormat);

        DPC_Output output = {
            .table = table,
            .table_size = 0,
        };

        DPC_Calib(&input, &output);

        for (CVI_U32 i = 0; i < output.table_size; ++i) {
            for (CVI_U32 j = 0; j < STATIC_DP_COUNT_MAX; ++j) {
                if ((pStBadPixel + j)->pixel == 0) {
                    (pStBadPixel + j)->pixel = table[i];
                    (pStBadPixel + j)->cnt += 1;
                    break;
                } else if ((pStBadPixel + j)->pixel == table[i]) {
                    (pStBadPixel + j)->cnt += 1;
                    break;
                }
            }
        }
        CurTimes++;
    } while (CurTimes < EndTimes);

	memset(table, 0, sizeof(uint32_t) * STATIC_DP_COUNT_MAX);
    for (CVI_U32 j = 0; j < STATIC_DP_COUNT_MAX; ++j) {
        if ((pStBadPixel + j)->pixel == 0) {
            break;
        } else if ((pStBadPixel + j)->cnt > raw_image_cnt * BAD_RATIO) {
            table[bad_cnt] = (pStBadPixel + j)->pixel;
            bad_cnt += 1;
        }
    }
	//Reset Pub/AE Settings
	if (CVI_ISP_SetPubAttr(ViPipe, &stPubAttrOrig) != CVI_SUCCESS) {
        printf("CVI_ISP_SetPubAttr Pipe: %d fail\n", ViPipe);
    }
    if (CVI_ISP_SetExposureAttr(ViPipe, &stExpAttrOrig) != CVI_SUCCESS) {
        printf("CVI_ISP_SetExposureAttr Pipe: %d fail\n", ViPipe);
    }

    if (raw_image_cnt == 0) {
        printf("Bad pixel detect timeout\n");
        goto ERROR;
    }

    printf("DPC calibrate success\n");

    // save table as txt file
	printf("bad_cnt=%d\n", bad_cnt);
    fp = fopen(table_path, "w");
    if (!fp) {
        printf("open output table file failed\n");
        goto ERROR;
    }
    for (CVI_U32 i = 0; i < STATIC_DP_COUNT_MAX; ++i) {
        fprintf(fp, "%d", table[i]);
        if (i != STATIC_DP_COUNT_MAX - 1) {
            fprintf(fp, ",");
        }
    }
    fclose(fp);
    fp = NULL;
    // release source
    sys_vi_deinit();
    if (ImgBuffer16 != NULL)
        free(ImgBuffer16);
    if (pStBadPixel != NULL)
        free(pStBadPixel);
    if (table)
        free(table);
    return CVI_SUCCESS;

ERROR:
    if (ImgBuffer16 != NULL)
        free(ImgBuffer16);
    if (pStBadPixel != NULL)
        free(pStBadPixel);
    if (table)
        free(table);
    if (fp)
        fclose(fp);
    sys_vi_deinit();
    return CVI_FAILURE;
};

static CVI_S32 isp_getrawbuffer(VI_PIPE ViPipe, CVI_U8 *ImgBuffer, BAYER_FORMAT_E *bayerFormat)
{
    VIDEO_FRAME_INFO_S stVideoFrame;
    VI_DUMP_ATTR_S attr;
    memset(&stVideoFrame, 0, sizeof(stVideoFrame));

    stVideoFrame.stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;

    // Set Attr
    attr.bEnable = 1;
    attr.u32Depth = 0;
    attr.enDumpType = VI_DUMP_TYPE_RAW;
    CVI_VI_SetPipeDumpAttr(ViPipe, &attr);
    // Check Attr effective
    attr.bEnable = 0;
    attr.enDumpType = VI_DUMP_TYPE_IR;
    CVI_VI_GetPipeDumpAttr(ViPipe, &attr);
    if ((attr.bEnable != 1) || (attr.enDumpType != VI_DUMP_TYPE_RAW)) {
        printf("Enable(%d), DumpType(%d)\n", attr.bEnable, attr.enDumpType);
    }

    if (CVI_VI_GetPipeFrame(ViPipe, &stVideoFrame, 1000) != CVI_SUCCESS) {
        printf("CVI_VI_GetPipeFrame (Pipe: %d) timeout\n", ViPipe);
        return CVI_FAILURE;
    }

    size_t image_size = stVideoFrame.stVFrame.u32Length[0];
    CVI_U32 stride = (image_size/stVideoFrame.stVFrame.u32Height);

    *bayerFormat = stVideoFrame.stVFrame.enBayerFormat;
    if (attr.enDumpType == VI_DUMP_TYPE_RAW) {
        CVI_U32 width = stVideoFrame.stVFrame.u32Width;
        CVI_U32 height = stVideoFrame.stVFrame.u32Height;
        unsigned char *ptr = malloc(image_size);
        if (ptr == NULL) {
            printf("malloc size:%zu fail\n", image_size);
            CVI_VI_ReleasePipeFrame(ViPipe, &stVideoFrame);
            return CVI_FAILURE;
        }

        stVideoFrame.stVFrame.pu8VirAddr[0] = CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0], image_size);
        memcpy(ptr, (const void *)stVideoFrame.stVFrame.pu8VirAddr[0], image_size);

        if (stVideoFrame.stVFrame.enCompressMode == COMPRESS_MODE_NONE) {
            Bayer_12bit_2_16bit(ptr, (CVI_U16 *)ImgBuffer, width, height, width);
        } else if (stVideoFrame.stVFrame.enCompressMode == COMPRESS_MODE_TILE) {
            RAW_INFO rawInfo = {0};
            rawInfo.width = width;
            rawInfo.height = height;
            rawInfo.stride = stride;
            rawInfo.buffer = ptr;
            CVI_U8 *pDecImg = malloc(rawInfo.width * rawInfo.height * 2);
            if (pDecImg == NULL) {
                printf("malloc size:%u fail\n", rawInfo.width * rawInfo.height * 2);
                CVI_VI_ReleasePipeFrame(ViPipe, &stVideoFrame);
                free(ptr);
                return CVI_FAILURE;
            }

            if (decoderRaw(rawInfo, pDecImg) != CVI_SUCCESS) {
                printf("decoderRaw err\n");
                CVI_VI_ReleasePipeFrame(ViPipe, &stVideoFrame);
                free(ptr);
                return CVI_FAILURE;
            }
            memcpy(ImgBuffer, pDecImg, rawInfo.width * rawInfo.height * 2);
            free(pDecImg); pDecImg = NULL;
        } else {
            printf("not support [%d]compress mode\n", stVideoFrame.stVFrame.enCompressMode);
            CVI_VI_ReleasePipeFrame(ViPipe, &stVideoFrame);
            free(ptr);
            return CVI_FAILURE;
        }
        CVI_SYS_Munmap((void *)stVideoFrame.stVFrame.pu8VirAddr[0], image_size);
        free(ptr); ptr = NULL;
    } else {
        CVI_VI_ReleasePipeFrame(ViPipe, &stVideoFrame);
        return CVI_FAILURE;
    }
    CVI_VI_ReleasePipeFrame(ViPipe, &stVideoFrame);
    return CVI_SUCCESS;
}

static CVI_S32 dpc_dump_raw(VI_PIPE ViPipe, CVI_U16 *ImgBuffer,	CVI_U16 width, CVI_U16 height, BAYER_FORMAT_E bayerFormat)
{
    char img_name[128] = {0,}, order_id[8] = {0,};
    FILE *output;
    struct timeval tv1;

    switch (bayerFormat) {
        case BAYER_FORMAT_BG:
            snprintf(order_id, sizeof(order_id), "BG");
            break;
        case BAYER_FORMAT_GB:
            snprintf(order_id, sizeof(order_id), "GB");
            break;
        case BAYER_FORMAT_GR:
            snprintf(order_id, sizeof(order_id), "GR");
            break;
        case BAYER_FORMAT_RG:
        default:
            snprintf(order_id, sizeof(order_id), "RG");
            break;
    }

    gettimeofday(&tv1, NULL);
    snprintf(img_name, sizeof(img_name), "./vi_%d_byrid_%s_w_%d_h_%d_bit_%d_tv_%ld_%ld.raw",
            ViPipe, order_id,
            width,
            height,
            16, tv1.tv_sec, tv1.tv_usec
            );

    output = fopen(img_name, "wb");
    if (output) {
        printf("dump raw to %s\n", img_name);
        size_t ImgBufferSize = width
                            * height
                            * 2;
        fwrite(ImgBuffer,
            ImgBufferSize,
            1, output);
        fclose(output);
    } else {
        printf("Open dump destnation file %s fail\n", img_name);
    }
    return 0;
}

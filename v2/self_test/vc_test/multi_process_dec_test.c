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
#include <sys/time.h>
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



#define MAX_STRING_LEN      255
#define MAX_FILENAME_LEN    64

#define MAX_OPTIONS 128


typedef struct _VDEC_PARAM_S {
    CVI_S32 s32ChnId;
    PAYLOAD_TYPE_E enType;
    CVI_CHAR cFilePath[128];
    CVI_CHAR cFileName[128];
    CVI_S32 s32StreamMode;
    CVI_S32 s32MilliSec;
    CVI_S32 s32MinBufSize;
    CVI_S32 s32IntervalTime;
    CVI_U64 u64PtsInit;
    CVI_U64 u64PtsIncrease;
    CVI_BOOL bFileEnd;
    CVI_BOOL bDumpYUV;
    CVI_BOOL bStop;
    CVI_S32 s32NumFrames;
} VDEC_PARAM_S;


typedef struct _chnInputCfg_ {
    char codec[64];
    unsigned int width;
    unsigned int height;
    int chn;
    int num_frames;
    char input_path[MAX_STRING_LEN];
    char vpssSrcPath[MAX_STRING_LEN];
    char output_path[MAX_STRING_LEN];
}chnInputCfg;


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
    {{"input",     optional_argument, NULL, 'i'}, ARG_STRING, 0,   0,
        "source yuv file"},
    {{"output",    optional_argument, NULL, 'o'}, ARG_STRING, 0,   0,
        "output bitstream"},
    {{"frame_num", optional_argument, NULL, 'n'}, ARG_UINT,    0,   1000000000,
        "number of frame to be encode"},
    {{"numChn",    optional_argument, NULL, 0},   ARG_INT,    1,   32,
        "number of channels to encode"},
    {{"chn",       optional_argument, NULL, 0},   ARG_UINT,    0,   32 - 1,
        "set channel-id to configure the following parameters"},
    {{NULL, 0, NULL, 0}, ARG_INT, 0, 0, ""}
};

void print_help(char * const *argv)
{
    uint32_t idx;

    printf("// ------------------------------------------------\n");
    printf("// %s -c codec -i bitstream.codec -o decout.yuv\n", argv[0]);
    printf("EX.\n");
    printf("sample_venc -c 265 -i bitstream.265 -o dec_out.yuv\n");
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
    printf("entryIdx = %d\n", entryIdx);

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


int32_t parseArgv(chnInputCfg *pIc, int argc, char **argv)
{
    int32_t ch, idx, ret;
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
            if (ret != 0) {
                printf("checkArg, %d\n", ret);
                print_help(argv);
                return ret;
            }

            if (!strcmp(long_options[idx].name, "chn")) {
                pIc->chn = arg.uval;
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

    if (optind < argc)
        print_help(argv);

    return 0;
}

static void get_chroma_size_shift_factor(PIXEL_FORMAT_E enPixelFormat, CVI_S32 *w_shift, CVI_S32 *h_shift, CVI_S32 *s32Luma_Read)
{
	switch (enPixelFormat) {
	case PIXEL_FORMAT_YUV_PLANAR_420:
		*w_shift = 1;
		*h_shift = 1;
        *s32Luma_Read = 1;
		break;
	case PIXEL_FORMAT_YUV_PLANAR_422:
		*w_shift = 1;
		*h_shift = 0;
        *s32Luma_Read = 1;
		break;
	case PIXEL_FORMAT_YUV_PLANAR_444:
		*w_shift = 0;
		*h_shift = 0;
        *s32Luma_Read = 1;
		break;
	case PIXEL_FORMAT_NV12:
	case PIXEL_FORMAT_NV21:
		*w_shift = 0;
		*h_shift = 31;
        *s32Luma_Read = 1;
		break;
    case PIXEL_FORMAT_NV16:
    case PIXEL_FORMAT_NV61:
        *w_shift = 0;
        *h_shift = 31;
        *s32Luma_Read = 1;
        break;
    case PIXEL_FORMAT_YUYV:
    case PIXEL_FORMAT_UYVY:
    case PIXEL_FORMAT_YVYU:
    case PIXEL_FORMAT_VYUY:
        *s32Luma_Read = 2;
        *w_shift = 31;
        *h_shift = 31;
        break;
	case PIXEL_FORMAT_YUV_400: // no chroma
	default:
        *s32Luma_Read = 1;
		*w_shift = 31;
		*h_shift = 31;
		break;
	}
}

static int write_yuv(FILE *out_f, VIDEO_FRAME_S stVFrame)
{
	CVI_S32 c_w_shift, c_h_shift, s32LumaRead; // chroma width/height shift
	CVI_U8 *w_ptr;

	printf("u32Width = %d, u32Height = %d\n",
			stVFrame.u32Width, stVFrame.u32Height);
	printf("u32Stride[0] = %d, u32Stride[1] = %d, u32Stride[2] = %d\n",
			stVFrame.u32Stride[0], stVFrame.u32Stride[1], stVFrame.u32Stride[2]);
	printf("u32Length[0] = %d, u32Length[1] = %d, u32Length[2] = %d\n",
			stVFrame.u32Length[0], stVFrame.u32Length[1], stVFrame.u32Length[2]);

	get_chroma_size_shift_factor(stVFrame.enPixelFormat, &c_w_shift, &c_h_shift, &s32LumaRead);

	w_ptr = stVFrame.pu8VirAddr[0];
	for (CVI_U32 i = 0; i < stVFrame.u32Height; i++) {
		fwrite(w_ptr + i * stVFrame.u32Stride[0], 1, stVFrame.u32Width * s32LumaRead, out_f);
	}

	if (stVFrame.pu8VirAddr[1] && (stVFrame.u32Height >> c_h_shift) > 0) {
		w_ptr = stVFrame.pu8VirAddr[1];
		for (CVI_U32 i = 0; i < (stVFrame.u32Height >> c_h_shift); i++) {
			fwrite(w_ptr + i * stVFrame.u32Stride[1], 1, stVFrame.u32Width >> c_w_shift, out_f);
		}
	}

	if (stVFrame.pu8VirAddr[2] && (stVFrame.u32Height >> c_h_shift) > 0) {
		w_ptr = stVFrame.pu8VirAddr[2];
		for (CVI_U32 i = 0; i < (stVFrame.u32Height >> c_h_shift); i++) {
			fwrite(w_ptr + i * stVFrame.u32Stride[2], 1, stVFrame.u32Width >> c_w_shift, out_f);
		}
	}

	return 0;
}

int process_send_bitstream(VDEC_PARAM_S *pArgs)
{
    VDEC_PARAM_S *pstVdecParam = (VDEC_PARAM_S *)pArgs;
    CVI_BOOL bEndOfStream = CVI_FALSE;
    CVI_S32 s32UsedBytes = 0, s32ReadLen = 0;
    FILE *fpStrm = NULL;
    CVI_U8 *pu8Buf = NULL;
    VDEC_STREAM_S stStream;
    CVI_BOOL bFindStart, bFindEnd;
    CVI_U64 u64PTS = 0;
    CVI_U32 u32Len, u32Start;
    CVI_S32 s32Ret, i;

    fpStrm = fopen(pstVdecParam->cFileName, "rb");
    if (fpStrm == NULL) {
        printf("can't open file %s in send stream thread!\n", pstVdecParam->cFileName);
        pstVdecParam->bFileEnd = CVI_TRUE;
        return (CVI_FAILURE);
    }

    pu8Buf = malloc(pstVdecParam->s32MinBufSize);
    if (pu8Buf == NULL) {
        printf("can't alloc %d in send stream thread!\n", pstVdecParam->s32MinBufSize);
        fclose(fpStrm);
        pstVdecParam->bFileEnd = CVI_TRUE;
        return (CVI_FAILURE);
    }
    u64PTS = pstVdecParam->u64PtsInit;
    while (pstVdecParam->s32NumFrames > 0) {
        bEndOfStream = CVI_FALSE;
        bFindStart = CVI_FALSE;
        bFindEnd = CVI_FALSE;
        u32Start = 0;
        s32Ret = fseek(fpStrm, s32UsedBytes, SEEK_SET);
        memset(pu8Buf, 0 , pstVdecParam->s32MinBufSize);
        s32ReadLen = fread(pu8Buf, 1, pstVdecParam->s32MinBufSize, fpStrm);
        if (s32ReadLen == 0 && pstVdecParam->s32NumFrames > 0) {
            s32UsedBytes = 0;
            continue;
        }

        if (pstVdecParam->s32StreamMode == VIDEO_MODE_FRAME &&
                pstVdecParam->enType == PT_H264) {
            for (i = 0; i < s32ReadLen - 8; i++) {
                int tmp = pu8Buf[i + 3] & 0x1F;

                if (pu8Buf[i] == 0 && pu8Buf[i + 1] == 0 && pu8Buf[i + 2] == 1 &&
                    (((tmp == 0x5 || tmp == 0x1) && ((pu8Buf[i + 4] & 0x80) == 0x80)) ||
                     (tmp == 20 && (pu8Buf[i + 7] & 0x80) == 0x80))) {
                    bFindStart = CVI_TRUE;
                    i += 8;
                    break;
                }
            }

            for (; i < s32ReadLen - 8; i++) {
                int tmp = pu8Buf[i + 3] & 0x1F;

                if (pu8Buf[i] == 0 && pu8Buf[i + 1] == 0 && pu8Buf[i + 2] == 1 &&
                    (tmp == 15 || tmp == 7 || tmp == 8 || tmp == 6 ||
                     ((tmp == 5 || tmp == 1) && ((pu8Buf[i + 4] & 0x80) == 0x80)) ||
                     (tmp == 20 && (pu8Buf[i + 7] & 0x80) == 0x80))) {
                    bFindEnd = CVI_TRUE;
                    break;
                }
            }

            if (i > 0)
                s32ReadLen = i;
            if (bFindStart == CVI_FALSE) {
                printf("chn %d can not find H264 start code!s32ReadLen %d, s32UsedBytes %d.!\n",
                       pstVdecParam->s32ChnId, s32ReadLen, s32UsedBytes);
            }
            if (bFindEnd == CVI_FALSE) {
                s32ReadLen = i + 8;
            }

        } else if (pstVdecParam->s32StreamMode == VIDEO_MODE_FRAME &&
                pstVdecParam->enType == PT_H265) {
            CVI_BOOL bNewPic = CVI_FALSE;

            for (i = 0; i < s32ReadLen - 6; i++) {
                CVI_U32 tmp = (pu8Buf[i + 3] & 0x7E) >> 1;

                bNewPic = (pu8Buf[i + 0] == 0 && pu8Buf[i + 1] == 0 && pu8Buf[i + 2] == 1 &&
                       (tmp <= 21) && ((pu8Buf[i + 5] & 0x80) == 0x80));

                if (bNewPic) {
                    bFindStart = CVI_TRUE;
                    i += 6;
                    break;
                }
            }

            for (; i < s32ReadLen - 6; i++) {
                CVI_U32 tmp = (pu8Buf[i + 3] & 0x7E) >> 1;

                bNewPic = (pu8Buf[i + 0] == 0 && pu8Buf[i + 1] == 0 && pu8Buf[i + 2] == 1 &&
                       (tmp == 32 || tmp == 33 || tmp == 34 || tmp == 39 || tmp == 40 ||
                        ((tmp <= 21) && (pu8Buf[i + 5] & 0x80) == 0x80)));

                if (bNewPic) {
                    bFindEnd = CVI_TRUE;
                    break;
                }
            }
            if (i > 0)
                s32ReadLen = i;

            if (bFindEnd == CVI_FALSE) {
                s32ReadLen = i + 6;
            }

        } else if (pstVdecParam->enType == PT_MJPEG || pstVdecParam->enType == PT_JPEG) {
            for (i = 0; i < s32ReadLen - 1; i++) {
                if (pu8Buf[i] == 0xFF && pu8Buf[i + 1] == 0xD8) {
                    u32Start = i;
                    bFindStart = CVI_TRUE;
                    i = i + 2;
                    break;
                }
            }

            for (; i < s32ReadLen - 3; i++) {
                if ((pu8Buf[i] == 0xFF) && (pu8Buf[i + 1] & 0xF0) == 0xE0) {
                    u32Len = (pu8Buf[i + 2] << 8) + pu8Buf[i + 3];
                    i += 1 + u32Len;
                } else {
                    break;
                }
            }

            for (; i < s32ReadLen - 1; i++) {
                if (pu8Buf[i] == 0xFF && pu8Buf[i + 1] == 0xD9) {
                    bFindEnd = CVI_TRUE;
                    break;
                }
            }
            s32ReadLen = i + 2;
        } else {
            if ((s32ReadLen != 0) && (s32ReadLen < pstVdecParam->s32MinBufSize)) {
                bEndOfStream = CVI_TRUE;
            }
        }

        stStream.u64PTS = u64PTS;
        stStream.pu8Addr = pu8Buf + u32Start;
        stStream.u32Len = s32ReadLen;
        stStream.bEndOfFrame = (pstVdecParam->s32StreamMode == VIDEO_MODE_FRAME) ? CVI_TRUE : CVI_FALSE;
        if(pstVdecParam->enType == PT_MJPEG || pstVdecParam->enType == PT_JPEG)
            stStream.bEndOfFrame = CVI_FALSE;
        stStream.bEndOfStream = bEndOfStream;
        stStream.bDisplay = 1;

SendAgain:
        s32Ret = CVI_VDEC_SendStream(pstVdecParam->s32ChnId,
                &stStream, pstVdecParam->s32MilliSec);
        if (s32Ret != CVI_SUCCESS) {
            usleep(pstVdecParam->s32IntervalTime);

            if (s32Ret == CVI_ERR_VDEC_BUSY)
                printf("timeout in vdec sendstream\n");

            goto SendAgain;
        } else {
            bEndOfStream = CVI_FALSE;
            s32UsedBytes = s32UsedBytes + s32ReadLen + u32Start;
            u64PTS += pstVdecParam->u64PtsIncrease;
            pstVdecParam->s32NumFrames--;
        }
        usleep(pstVdecParam->s32IntervalTime);
    }

    fflush(stdout);
    if (pu8Buf != CVI_NULL) {
        free(pu8Buf);
    }

    fclose(fpStrm);

    //end of stream
    memset(&stStream, 0 ,sizeof(VDEC_STREAM_S));
    CVI_VDEC_SendStream(pstVdecParam->s32ChnId, &stStream, pstVdecParam->s32MilliSec);

    return CVI_SUCCESS;
}

int process_get_decode_data(VDEC_PARAM_S *pArgs)
{
    VDEC_PARAM_S *pstVdecParam = (VDEC_PARAM_S *)pArgs;
    FILE *fp = CVI_NULL;
    fd_set read_fds;
    int fd;
    CVI_S32 s32Ret, s32Cnt = 0;
    //VDEC_CHN_ATTR_S stAttr;
    VIDEO_FRAME_INFO_S stVFrame;
    struct timeval TimeoutVal;
    //VIDEO_FRAME_S *pstVFrame = &stVFrame.stVFrame;
    CVI_CHAR cSaveFile[256];
    int retry_cnt = 20;

#if 0
    s32Ret = CVI_VDEC_GetChnAttr(pstVdecParam->s32ChnId, &stAttr);
    if (s32Ret != CVI_SUCCESS) {
        printf("get chn attr fail for %#x!\n", s32Ret);
        return (CVI_FAILURE);
    }

    if (stAttr.enType != PT_JPEG &&
        stAttr.enType != PT_H264 &&
        stAttr.enType != PT_H265 &&
        stAttr.enType != PT_MJPEG) {
        printf("enType %d do not support save file!\n", stAttr.enType);
        return (CVI_FAILURE);
    }
#endif
    fd = CVI_VDEC_GetFd(pstVdecParam->s32ChnId);
    while (pstVdecParam->s32NumFrames > 0 && retry_cnt > 0) {
RETRY_GET_FRAME:
        FD_ZERO(&read_fds);
        FD_SET(fd, &read_fds);
        TimeoutVal.tv_sec  = 100;
        TimeoutVal.tv_usec = 0;
        s32Ret = select(fd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0) {
            printf("select failed!\n");
            return s32Ret;
        } else if (s32Ret == 0) {
            printf("select time out,retry %d\n", retry_cnt--);
            continue;
        } else if (!FD_ISSET(fd, &read_fds)) {
            printf("select return not set??\n");
        }
        s32Ret = CVI_VDEC_GetFrame(
                pstVdecParam->s32ChnId,
                &stVFrame,
                pstVdecParam->s32MilliSec);
        if (s32Ret == CVI_SUCCESS) {
            if (pstVdecParam->bDumpYUV == 1) {
                if (s32Cnt == 0) {
                    fp = fopen(pstVdecParam->cFileName, "wb");
                    if (fp == NULL) {
                        printf("can't open file %s\n", cSaveFile);
                        return (CVI_FAILURE);
                    }
                }

                write_yuv(fp, stVFrame.stVFrame);
            }

            s32Cnt++;
            retry_cnt = 3;
            pstVdecParam->s32NumFrames--;

            s32Ret = CVI_VDEC_ReleaseFrame(pstVdecParam->s32ChnId, &stVFrame);
            if (s32Ret != CVI_SUCCESS) {
                printf("CVI_MPI_VDEC_ReleaseFrame fail for s32Ret=0x%x!\n", s32Ret);
            }
        } else {
            if (s32Ret == CVI_ERR_VDEC_BUSY) {
                printf("vdec getframe timeout...retry\n");
                goto RETRY_GET_FRAME;
            }
            usleep(pstVdecParam->s32IntervalTime);
        }
    }

    if (pstVdecParam->bDumpYUV == 1) {
        if (fp != CVI_NULL)
            fclose(fp);
    }

    return CVI_SUCCESS;
}



int ChildProcessGetDecodedFrame(chnInputCfg *pIc)
{
    //VDEC_CHN VdChn = 0;
    VDEC_CHN_ATTR_S stAttr = {0};
    //VDEC_STREAM_S stStream = {0};
    //VIDEO_FRAME_INFO_S stFrameInfo = {0};
    VDEC_PARAM_S getParam = {0};
    //int ret;

    if (strncmp(pIc->codec, "264", 3) == 0)
        stAttr.enType = PT_H264;
    else if (strncmp(pIc->codec, "265", 3) == 0)
        stAttr.enType = PT_H265;
    else if (strncmp(pIc->codec, "mjp", 3) == 0
        || strncmp(pIc->codec, "jpg", 3) == 0)
        stAttr.enType = PT_JPEG;
    else {
        printf("unsupport type: %s\n", pIc->codec);
        return -1;
    }

    getParam.s32ChnId = 0;
    getParam.enType = stAttr.enType;
    memset(getParam.cFileName, 0, 128);
    memcpy(getParam.cFileName, pIc->output_path, strlen(pIc->output_path));
    getParam.bDumpYUV = 1;
    getParam.s32NumFrames = pIc->num_frames;
    process_get_decode_data(&getParam);

    printf("Child %d process: pid:%d exit\n", __LINE__, getpid());
    return 0;
}



int ParentProcessSendDecodeStream(chnInputCfg *pIc)
{
    VDEC_CHN VdChn = 0;
    VDEC_CHN_ATTR_S stAttr = {0};
    //VDEC_STREAM_S stStream = {0};
    VDEC_PARAM_S sendParam = {0};
    //VDEC_PARAM_S getThreadParam = {0};
    int ret;
    int status;

    if (strncmp(pIc->codec, "264", 3) == 0)
        stAttr.enType = PT_H264;
    else if (strncmp(pIc->codec, "265", 3) == 0)
        stAttr.enType = PT_H265;
    else if (strncmp(pIc->codec, "mjp", 3) == 0
        || strncmp(pIc->codec, "jpg", 3) == 0)
        stAttr.enType = PT_JPEG;
    else {
        printf("unsupport type: %s\n", pIc->codec);
        return -1;
    }

    stAttr.u32StreamBufSize = pIc->width * pIc->height *3;
    stAttr.u32FrameBufCnt = 6;
    ret = CVI_VDEC_CreateChn(VdChn, &stAttr);
    if (ret != CVI_SUCCESS) {
        printf("CVI_VDEC_CreateChn failed %d\n", ret);
        return -1;
    }

    ret = CVI_VDEC_StartRecvStream(VdChn);
    if (ret != CVI_SUCCESS) {
        printf("CVI_VDEC_StartRecvStream failed %d\n", ret);
        return -1;
    }

    sendParam.s32ChnId = 0;
    sendParam.enType = stAttr.enType;
    memset(sendParam.cFileName, 0, 128);
    memcpy(sendParam.cFileName, pIc->input_path, strlen(pIc->input_path));
    sendParam.s32StreamMode = VIDEO_MODE_FRAME;
    sendParam.s32MilliSec = -1;
    sendParam.s32MinBufSize = (pIc->width * pIc->height * 3) >> 1;;
    sendParam.s32IntervalTime = 10000;
    sendParam.u64PtsInit = 0;
    sendParam.u64PtsIncrease = 30;
    sendParam.bDumpYUV = 1;
    sendParam.s32NumFrames = pIc->num_frames;
    process_send_bitstream(&sendParam);

    wait(&status);
    printf("ChildProcess exit status:%d\n",WEXITSTATUS(status));

    ret = CVI_VDEC_StopRecvStream(VdChn);
    if (ret != CVI_SUCCESS) {
        printf("CVI_VDEC_StopRecvStream failed %d\n", ret);
        return -1;
    }

    ret = CVI_VDEC_DestroyChn(VdChn);
    if (ret != CVI_SUCCESS) {
        printf("CVI_VDEC_DestroyChn failed %d\n", ret);
        return -1;
    }

    printf("Parent %d process: pid:%d exit\n", __LINE__, getpid());

    return 0;
}


void init_cfg(chnInputCfg *pIc)
{
    strcpy(pIc->codec, "264");
    pIc->chn = 0;
    pIc->width = 4096;
    pIc->height = 4096;
    pIc->num_frames = 1;
}


int main(int argc, char *argv[])
{
    chnInputCfg stChnCfg;
    int ret = 0;

    init_cfg(&stChnCfg);
    ret = parseArgv(&stChnCfg, argc, argv);
    if(ret < 0 || argc < 2) {
        print_help(argv);
        return -1;
    }

    printf("parent process: codec:%s input:%s output file:%s\n",
        stChnCfg.codec, stChnCfg.input_path, stChnCfg.output_path);

    pid_t pid = fork();
    if (pid == 0) {
        ChildProcessGetDecodedFrame(&stChnCfg);
    } else if (pid > 0) {
        ParentProcessSendDecodeStream(&stChnCfg);
        //wait(NULL);
    } else {
        perror("fork");
        exit(1);
    }

    return 0;
}




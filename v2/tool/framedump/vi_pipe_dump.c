#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <pthread.h>
#include <errno.h>

#include "cvi_math.h"
#include "cvi_sys.h"
#include "cvi_vi.h"

static CVI_U32 u32SignalFlag = 0;
static VIDEO_FRAME_INFO_S stFrameInfo[2];
static VI_DUMP_ATTR_S attr;
static VI_PIPE ViPipe = 0;

void VI_Pipe_Dump_HandleSig(CVI_S32 signo)
{
	if (u32SignalFlag) {
		exit(-1);
	}

	if (SIGINT == signo || SIGTERM == signo) {
		u32SignalFlag++;
		if (stFrameInfo[0].stVFrame.u64PhyAddr[0])
			CVI_VI_ReleasePipeFrame(ViPipe, stFrameInfo);
		u32SignalFlag--;
		printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
	}

	exit(-1);
}

CVI_VOID SAMPLE_MISC_ViPipeDump(VI_PIPE Pipe, CVI_U32 u32FrameCnt)
{
	CVI_U32 u32Cnt = u32FrameCnt;
    int frm_num = 1, j = 0;

    memset(stFrameInfo, 0, sizeof(stFrameInfo));

    stFrameInfo[0].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;
    stFrameInfo[1].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;

    attr.bEnable = 1;
    attr.u32Depth = 0;
    attr.enDumpType = VI_DUMP_TYPE_RAW;

    CVI_VI_SetPipeDumpAttr(Pipe, &attr);

    attr.bEnable = 0;
    attr.enDumpType = VI_DUMP_TYPE_IR;

    CVI_VI_GetPipeDumpAttr(Pipe, &attr);

	/* get frame  */
	while (u32Cnt--) {
        frm_num = 1;

        CVI_VI_GetPipeFrame(Pipe, stFrameInfo, 1000);

        if (stFrameInfo[1].stVFrame.u64PhyAddr[0] != 0)
            frm_num = 2;

        for (j = 0; j < frm_num; j++) {
            size_t image_size = stFrameInfo[j].stVFrame.u32Length[0];
            unsigned char *ptr = calloc(1, image_size);
            FILE *output;
            char img_name[128] = {0,}, order_id[8] = {0,};

            if (attr.enDumpType == VI_DUMP_TYPE_RAW) {
                stFrameInfo[j].stVFrame.pu8VirAddr[0]
                    = CVI_SYS_Mmap(stFrameInfo[j].stVFrame.u64PhyAddr[0]
                        , stFrameInfo[j].stVFrame.u32Length[0]);
                printf("paddr(%#"PRIx64") vaddr(%p)\n",
                            stFrameInfo[j].stVFrame.u64PhyAddr[0],
                            stFrameInfo[j].stVFrame.pu8VirAddr[0]);

                memcpy(ptr, (const void *)stFrameInfo[j].stVFrame.pu8VirAddr[0],
                    stFrameInfo[j].stVFrame.u32Length[0]);
                CVI_SYS_Munmap((void *)stFrameInfo[j].stVFrame.pu8VirAddr[0],
                        stFrameInfo[j].stVFrame.u32Length[0]);

                switch (stFrameInfo[j].stVFrame.enBayerFormat) {
                default:
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
                    snprintf(order_id, sizeof(order_id), "RG");
                    break;
                }

                snprintf(img_name, sizeof(img_name),
                        "./vi_%d_%s_%s_w_%d_h_%d_x_%d_y_%d.raw",
                        Pipe, (j == 0) ? "LE" : "SE", order_id,
                        stFrameInfo[j].stVFrame.u32Width,
                        stFrameInfo[j].stVFrame.u32Height,
                        stFrameInfo[j].stVFrame.s16OffsetLeft,
                        stFrameInfo[j].stVFrame.s16OffsetTop);

                printf("dump image %s\n", img_name);

                output = fopen(img_name, "wb");

                fwrite(ptr, image_size, 1, output);
                fclose(output);
                free(ptr);
            }
        }
        CVI_VI_ReleasePipeFrame(Pipe, stFrameInfo);
    }
    printf("Dump VI raw TEST-PASS\n");
}

static void usage(void)
{
	printf(
		"\n"
		"*************************************************\n"
		"Usage: ./vi_pipe_dump [ViPipe] [FrmCnt]\n"
		"1)ViPipe: \n"
		"	Vi pipe id\n"
		"2)FrmCnt: \n"
		"	the count of frame to be dump\n"
		"*)Example:\n"
		"	e.g : ./vi_pipe_dump 0 1\n"
		"	e.g : ./vi_pipe_dump 0 3\n"
		"*************************************************\n"
		"\n");
}

int main(int argc, char **argv)
{
	CVI_U32 u32FrmCnt = 1;

	printf("\nNOTICE: This tool only can be used for TESTING !!!\n");
	printf("\tTo see more usage, please enter: ./vi_pipe_dump -h\n\n");

	if (argc > 1) {
		if (!strncmp(argv[1], "-h", 2)) {
			usage();
			exit(CVI_SUCCESS);
		}
	}

	if (argc < 3) {
		usage();
		exit(CVI_SUCCESS);
	}

	ViPipe = atoi(argv[1]);

	if (!VALUE_BETWEEN(ViPipe, 0, VI_MAX_PIPE_NUM - 1)) {
		printf("vi pipe id must be [0,%d]!!!!\n\n", VI_MAX_PIPE_NUM - 1);
		return -1;
	}

	u32SignalFlag = 0;
	signal(SIGINT, VI_Pipe_Dump_HandleSig);
	signal(SIGTERM, VI_Pipe_Dump_HandleSig);

	u32FrmCnt = atoi(argv[2]);/* frame count*/
	SAMPLE_MISC_ViPipeDump(ViPipe, u32FrmCnt);

	return CVI_SUCCESS;
}


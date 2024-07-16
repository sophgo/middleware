#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <linux/videodev2.h>
#include <vi_v4l2_uapi.h>
#include <cvi_buffer.h>

#include "devmem.h"
#include "cvi_isp_v4l2.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#define VIDEO_DEV_NUM 6
#define REQ_BUFFER_NUM 6
#define THREAD_LOOP_CNT 200

static int v4l2_fd[VIDEO_DEV_NUM];
static int test_dev_num;
static int is_wdr_mode = 0;
static int is_dump_yuv = 1;
static int is_run_isp_mw = 1;
static int is_ispmw_init[VIDEO_DEV_NUM];
static pthread_t g_video_thid[VIDEO_DEV_NUM];

extern int test_sensor_ctrl(int dev);
typedef struct VideoBuffer {
    void *start;
    size_t  length;
} VideoBuffer;

VideoBuffer framebuf[VIDEO_DEV_NUM][REQ_BUFFER_NUM];

static int set_wdr_on(int fd, int on)
{
	struct v4l2_ext_controls val;
	struct v4l2_ext_control control;
	memset(&val, 0, sizeof(struct v4l2_ext_controls));

	if(fd > 0) {
		// test set ext ctrl
		control.id = VI_IOCTL_HDR;
		control.value = on;
		val.count = 1;
		val.controls = &control;
		if(ioctl(fd, VIDIOC_S_EXT_CTRLS, &val) < 0) {
			printf("set hdr fail !\n");
			return -1;
		} else {
			printf("set hdr ctrls id:%d success\n", control.id);
		}
	}

	return 0;
}

static int set_bypass_frm(int fd, int bypass_num)
{
	struct v4l2_ext_controls val;
	struct v4l2_ext_control control;
	memset(&val, 0, sizeof(struct v4l2_ext_controls));

	if(fd > 0) {
		control.id = VI_IOCTL_SET_BYPASS_FRM;
		control.value = bypass_num;
		val.count = 1;
		val.controls = &control;
		if(ioctl(fd, VIDIOC_S_EXT_CTRLS, &val) < 0) {
			printf("set bypass frm fail !\n");
			return -1;
		} else {
			printf("set bypass frm:%d success\n", bypass_num);
		}
	}

	return 0;
}

static int test_get_ext_ctrl(int fd)
{
	struct v4l2_ext_controls val;
	struct v4l2_ext_control control;
	VI_PIPE_ATTR_S pipe_attr;
	memset(&val, 0, sizeof(struct v4l2_ext_controls));

	if(fd > 0) {
		// test get ext ctrl
		control.id = VI_IOCTL_GET_PIPE_ATTR;
		control.ptr = &pipe_attr;
		val.count = 1;
		val.controls = &control;
		if(ioctl(fd, VIDIOC_G_EXT_CTRLS, &val) < 0) {
			printf("get ext ctrls id:%d fail !\n", control.id);
			return -1;
		} else {
			printf("test get ext ctrls id:%d success\n", control.id);
		}
	}

	return 0;
}

static int test_get_raw_dump(int fd, int dev)
{
	struct v4l2_ext_controls val;
	struct v4l2_ext_control control;
	VIDEO_FRAME_INFO_S stVideoFrame[2];
	VI_DUMP_ATTR_S attr;
	int mem_fd = devm_open();
	int loop = 1;
	int frm_num = 1;
	int j;

	memset(&stVideoFrame[0], 0, sizeof(VIDEO_FRAME_INFO_S));
	memset(&val, 0, sizeof(struct v4l2_ext_controls));

	stVideoFrame[0].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;
	attr.bEnable = 1;
	attr.u32Depth = 0;
	attr.enDumpType = VI_DUMP_TYPE_RAW;

	control.id = VI_IOCTL_SET_PIPE_DUMP_ATTR;
	control.ptr = &attr;
	val.count = 1;
	val.controls = &control;
	if(ioctl(fd, VIDIOC_S_EXT_CTRLS, &val) < 0) {
		printf("set raw dump attr fail !\n");
		return -1;
	} else {
		printf("set raw dump attr id:%d success\n", control.id);
	}

	control.id = VI_IOCTL_GET_PIPE_FRAME;
	control.ptr = &stVideoFrame[0];
	val.count = 1;
	val.controls = &control;
	if(ioctl(fd, VIDIOC_G_EXT_CTRLS, &val) < 0) {
		printf("get raw dump fail !\n");
		return -1;
	} else {
		printf("test get raw dump id:%d success\n", control.id);
	}

	//write raw dump file
	while (loop--) {
		for (j = 0; j < frm_num; j++) {
			size_t image_size = stVideoFrame[j].stVFrame.u32Length[0];
			unsigned char *ptr = calloc(1, image_size);
			FILE *output;
			char img_name[128] = {0,}, order_id[8] = {0,};

			if (attr.enDumpType == VI_DUMP_TYPE_RAW) {
				stVideoFrame[j].stVFrame.pu8VirAddr[0] = devm_map(mem_fd,
					stVideoFrame[j].stVFrame.u64PhyAddr[0], stVideoFrame[j].stVFrame.u32Length[0]);

				printf("paddr(%lx) vaddr(%p)\n",
							stVideoFrame[j].stVFrame.u64PhyAddr[0],
							stVideoFrame[j].stVFrame.pu8VirAddr[0]);

				memcpy(ptr, (const void *)stVideoFrame[j].stVFrame.pu8VirAddr[0],
					stVideoFrame[j].stVFrame.u32Length[0]);
				devm_unmap((void *)stVideoFrame[j].stVFrame.pu8VirAddr[0],
						stVideoFrame[j].stVFrame.u32Length[0]);

				switch (stVideoFrame[j].stVFrame.enBayerFormat) {
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
						"./vi_%d_%s_%s_w_%d_h_%d_x_%d_y_%d_%d.raw",
						dev, (j == 0) ? "LE" : "SE", order_id,
						stVideoFrame[j].stVFrame.u32Width,
						stVideoFrame[j].stVFrame.u32Height,
						stVideoFrame[j].stVFrame.s16OffsetLeft,
						stVideoFrame[j].stVFrame.s16OffsetTop,
						loop);

				printf("dump image %s\n", img_name);

				output = fopen(img_name, "wb");

				fwrite(ptr, image_size, 1, output);
				fclose(output);
				free(ptr);
			}
		}
	}

	control.id = VI_IOCTL_RELEASE_PIPE_FRAME;
	control.ptr = &stVideoFrame[0];
	val.count = 1;
	val.controls = &control;
	if(ioctl(fd, VIDIOC_S_EXT_CTRLS, &val) < 0) {
		printf("release raw fail !\n");
		return -1;
	} else {
		printf("release raw id:%d success\n", control.id);
	}

	return 0;
}

static int test_get_yuv_dump(int fd, int dev)
{
	struct v4l2_ext_controls val;
	struct v4l2_ext_control control;
	VIDEO_FRAME_INFO_S stVideoFrame;
	VI_DUMP_ATTR_S attr;
	int ret;
	int mem_fd = devm_open();
	int loop = 5;

	memset(&stVideoFrame, 0, sizeof(VIDEO_FRAME_INFO_S));
	memset(&val, 0, sizeof(struct v4l2_ext_controls));

	attr.bEnable = 1;
	attr.u32Depth = 0;
	attr.enDumpType = VI_DUMP_TYPE_YUV;

	//write yuv dump file
	while (loop--) {
		control.id = VI_IOCTL_GET_CHN_FRAME;
		control.ptr = &stVideoFrame;
		val.count = 1;
		val.controls = &control;
		ret = ioctl(fd, VIDIOC_G_EXT_CTRLS, &val);
				printf("get yuv dump ret:%d !\n", ret);

		if(ret < 0) {
			printf("get yuv dump fail !\n");
			return -1;
		} else {
			printf("test get yuv dump id:%d success\n", control.id);
		}

		// dump file
		if (1) {
			size_t image_size =
				stVideoFrame.stVFrame.u32Length[0] + stVideoFrame.stVFrame.u32Length[1];
			unsigned char *ptr = calloc(1, image_size);
			FILE *output;
			char img_name[128] = {0,};

			if (attr.enDumpType == VI_DUMP_TYPE_YUV) {
				stVideoFrame.stVFrame.pu8VirAddr[0] = devm_map(mem_fd,
					stVideoFrame.stVFrame.u64PhyAddr[0], image_size);

				printf("paddr(%lx) vaddr(%p)\n",
							stVideoFrame.stVFrame.u64PhyAddr[0],
							stVideoFrame.stVFrame.pu8VirAddr[0]);

				memcpy(ptr, (const void *)stVideoFrame.stVFrame.pu8VirAddr[0], image_size);
				devm_unmap((void *)stVideoFrame.stVFrame.pu8VirAddr[0], image_size);

				snprintf(img_name, sizeof(img_name),
						"./vi_%d_w_%d_h_%d_%d.yuv",
						dev,
						stVideoFrame.stVFrame.u32Width,
						stVideoFrame.stVFrame.u32Height,
						loop);

				printf("dump image %s\n", img_name);

				output = fopen(img_name, "wb");

				fwrite(ptr, image_size, 1, output);
				fclose(output);
				free(ptr);
			}
		}

		control.id = VI_IOCTL_RELEASE_CHN_FRAME;
		control.ptr = &stVideoFrame;
		val.count = 1;
		val.controls = &control;
		if(ioctl(fd, VIDIOC_S_EXT_CTRLS, &val) < 0) {
			printf("release yuv fail !\n");
			return -1;
		} else {
			printf("release yuv id:%d success\n", control.id);
		}
	}

	return 0;
}

static int request_buffer(int fd, int dev)
{
	struct v4l2_buffer buf;
	struct v4l2_requestbuffers reqbuf;
	int i;
	int n = dev;
	int ret = 0;

	if(fd > 0) {
		reqbuf.count = REQ_BUFFER_NUM;
		reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		reqbuf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(fd, VIDIOC_REQBUFS, &reqbuf);
		if(ret < 0) {
			printf("request buffer fail, ret:%d!\n", ret);
			return ret;
		}

		for(i = 0; i < REQ_BUFFER_NUM; i++) {
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index= i;
			ret = ioctl (fd, VIDIOC_QUERYBUF, &buf);
			if (ret < 0) {
				printf("query buffer fail, ret:%d\n", ret);
				return ret;
			}
			framebuf[n][i].length = buf.length;
			framebuf[n][i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE,
									MAP_SHARED, fd, buf.m.offset);
			if(MAP_FAILED == framebuf[n][i].start) {
				printf("mmap fail !\n");
				return -1;
			}
			// printf("buf[%d][%d]: addr=%p, len=%ld, offset=%d\n", n, buf.index, framebuf[n][i].start,
			// 		framebuf[n][i].length, buf.m.offset);
		}
	}
	return ret;
}

static void free_buffer(int fd, int dev)
{
	int i;
	struct v4l2_requestbuffers reqbuf;

	for(i = 0; i < REQ_BUFFER_NUM; i++)
		munmap(framebuf[dev][i].start, framebuf[dev][i].length);

	reqbuf.count = 0;
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	if(ioctl(fd, VIDIOC_REQBUFS, &reqbuf) < 0)
		printf("free buffer fail!\n");
}

static void set_patgen(int is_patgen)
{
	char cmdstr[128];
	char param[16];
	int i;

	for (i = 0; i < 6; i++) {
		if(is_patgen) {
			if (i == 0)
				sprintf(param, "1");
			else
				sprintf(param, "%s,1", param);
		}
		else {
			if (i == 0)
				sprintf(param, "0");
			else
				sprintf(param, "%s,0", param);
		}
	}
	sprintf(cmdstr, "echo %s > /sys/module/soph_ispv4l2/parameters/csi_patgen_en", param);

	system(cmdstr);
}

static void *streamimg_thread(void *arg)
{
	int dev = *(int *)arg;
	int fd = v4l2_fd[dev];
	struct v4l2_buffer buf;
	enum v4l2_buf_type type;
	int loop_cnt = 30;
	int i = 0;

	if (!is_dump_yuv) {
		loop_cnt = THREAD_LOOP_CNT; //for stream test
	}

	if (!is_run_isp_mw) {
		set_bypass_frm(fd, 20);
	}

	if(!framebuf[dev][0].start) {
		if (request_buffer(fd, dev) < 0) {
			printf("request buffer fail\n");
			return NULL;
		}
	}

	// queue all requested buffer to driver
	for(i = 0; i < REQ_BUFFER_NUM; i++) {
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index= i;

		if (ioctl (fd, VIDIOC_QUERYBUF, &buf) < 0) {
			printf("query buffer fail\n");
			return NULL;
		}
		if(ioctl(fd, VIDIOC_QBUF, &buf) < 0) {
			printf("dev0 fd(%d) qbuf_%d fail !\n", fd, i);
			return NULL;
		}
	}

	// stream on
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(fd, VIDIOC_STREAMON, &type) < 0) {
		printf("fd(%d) stream on fail !\n", fd);
		return NULL;
	}

	for (i = 0; i < loop_cnt; i++) {
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		// Get frame
		if (ioctl(fd, VIDIOC_DQBUF, &buf) < 0) {
			printf("VIDIOC_DQBUF failed at frame(%d) !\n", i);
			return NULL;
		}

		if (is_dump_yuv && i > 20) {
			char filename[128];
			int index = i % 20;
			sprintf(filename, "./v4l2_video%d_%d.yuv", dev, index);
			FILE *fp = fopen(filename, "wb");
			if (fp == NULL) {
				memset(filename, 0x0, sizeof(filename));
				snprintf(filename, sizeof(filename), "/mnt/data/v4l2_video%d_%d.yuv", dev, index);
				fp = fopen(filename, "wb");
				if (fp == NULL) {
					printf("open frame data file failed\n");
					return NULL;
				}
			}
			fwrite(framebuf[dev][buf.index].start, framebuf[dev][buf.index].length, 1, fp);
			fclose(fp);
			printf("size[%ld],saved in %s\n", framebuf[dev][buf.index].length, filename);
		}

		// requeue buffer
		if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) {
			printf("VIDIOC_QBUF failed at frame(%d)\n", i);
			return NULL;
		}
	}

	if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0) {
		printf("stream off fd(%d) fail !\n", fd);
	}

	free_buffer(fd, dev);

	return arg;
}


#if 0
static void *poll_thread(void *arg)
{
	int fd = *(int *)arg;
	struct v4l2_ext_controls val;
	struct v4l2_ext_control control;
	struct vi_event ev;
	struct timeval tv;
	fd_set efds;
	int loop = 100;
	int ret;

	memset(&val, 0, sizeof(struct v4l2_ext_controls));
	control.id = VI_IOCTL_DQEVENT;
	control.ptr = (void *)&ev;
	val.count = 1;
	val.controls = &control;

	while (loop--) {
		FD_ZERO(&efds);
		FD_SET(fd, &efds);

		tv.tv_sec = 0;
		tv.tv_usec = 500 * 1000;
		ret = select(fd + 1, NULL, NULL, &efds, &tv);
		if (ret == -1) {
			continue;
			printf("select error\n");
			return NULL;
		}

		if (ret == 0) {
			printf("select timeout\n");
			return NULL;
		}

		if(ioctl(fd, VIDIOC_G_EXT_CTRLS, &val) < 0) {
			printf("get ext ctrls fail !\n");
			return NULL;
		} else {
			printf("event type:%d\n", ev.type);
		}
	}

	return NULL;
}
#endif

static int _v4l2_ut_handle_op(int fd, int dev, int op)
{
	int ret = 0;
	struct v4l2_format s_format;
	struct v4l2_format g_format;

	switch (op)
	{
	case 0:  // set format
		s_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		s_format.fmt.pix.width = 1920;
		s_format.fmt.pix.height = 1080;
		s_format.fmt.pix.pixelformat = V4L2_PIX_FMT_NV21;
		if(ioctl(fd, VIDIOC_S_FMT, &s_format) < 0) {
			printf("set format size fail !\n");
			ret = -1;
			goto exit;
		}

		g_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if(ioctl(fd, VIDIOC_G_FMT, &g_format) < 0) {
			printf("get format size fail !\n");
			ret = -1;
			goto exit;
		}
		if(g_format.fmt.pix.width == s_format.fmt.pix.width &&
			g_format.fmt.pix.height == s_format.fmt.pix.height) {
			printf("set fmt size[%d*%d] success\n", s_format.fmt.pix.width, s_format.fmt.pix.height);
		}
		break;
	case 5:// 1 output
	case 6:// 2 output
	case 7:// 4 output
	case 8:// 6 output
	{
		int *arg = malloc(sizeof(int));
		*arg = dev;
		pthread_create(&g_video_thid[dev], NULL, streamimg_thread, arg);

		ret = 1;
		break;
	}
	case 9:
	{
		ret = test_get_ext_ctrl(fd);
		if(ret != 0)
			goto exit;
		ret = test_sensor_ctrl(dev);
		if(ret != 0)
			goto exit;
		break;
	}
	case 10:
	{
		ret = test_get_raw_dump(fd, dev);
		ret = test_get_yuv_dump(fd, dev);
		break;
	}
	default:
		break;
	}

exit:
	return ret;
}

static int handle_op(int op)
{
	int i;
	int ret = 0;
	char devicename[64];
	int first_dev = 0;

	switch (op)
	{
	case 1:
		is_wdr_mode = 1; //enable wdr
		return ret;
	case 2:
		set_patgen(1); //enable pattern
		return ret;
	case 3:
		is_run_isp_mw = 0; //diable isp mw
		return ret;
	case 4:
		is_dump_yuv = 0; //diable dump
		break;
	case 5:
		test_dev_num = 1;
		break;
	case 6:
		test_dev_num = 2;
		break;
	case 7:
		test_dev_num = 4;
		break;
	case 8:
		test_dev_num = 6;
		break;
	default:
		return ret;
	}

	for (i = first_dev; i < test_dev_num; i++) {
		sprintf(devicename, "/dev/video%d", i);
		if (v4l2_fd[i] > 0) {
			printf("%s had been opened!\n", devicename);
			continue;
		}

		v4l2_fd[i] = open(devicename, O_RDWR | O_NONBLOCK);
		if (v4l2_fd[i] < 0) {
			printf("open %s fail! skip error dev%d\n", devicename, i);
			continue;
		}
	}

	//set hdr on or off,only supprot first dev now
	for (i = first_dev; i < first_dev + 1; i++) {
		if (v4l2_fd[i] > 0)	{
			if (is_wdr_mode) {
				set_wdr_on(v4l2_fd[i], 1);
			} else {
				set_wdr_on(v4l2_fd[i], 0);
			}
		}
	}

	// init isp mw
	if (is_run_isp_mw) {
		for (i = first_dev; i < test_dev_num; i++) {
			if (v4l2_fd[i] <= 0 || is_ispmw_init[i])
				continue;
			CVI_ISP_V4L2_Init(i, v4l2_fd[i]);
			is_ispmw_init[i] = 1;
		}
	}

	// execute operation one by one
	for (i = first_dev; i < test_dev_num; i++) {
		if (v4l2_fd[i] <= 0)
			continue;

		ret = _v4l2_ut_handle_op(v4l2_fd[i], i, op);
		if (!ret) {
			return ret;
		}
	}

	for (i = first_dev; i < test_dev_num; i++) {
		void *thd_ret = NULL;
		pthread_join(g_video_thid[i], &thd_ret);
		if (!thd_ret) {
			ret = -1;
		} else {
			ret = 0;
		}

		printf("ispv4l2 ut dev[%d] op[%d] %s\n", i, op, ret == 0 ? "pass" : "fail");
	}

	return ret;
}

int main(int argc, char **argv)
{
	int op = 0;
	int n, i;

	printf("start ispv4l2 ut -->>\n");

	if (argc >= 2) {
		if (argc == 2) {
			op = atoi(argv[1]);
			handle_op(op);
		}
	} else {
		do {
			printf("0 : change output size\n");
			printf("1 : enable wdr\n");
			printf("2 : enable pattern\n");
			printf("3 : disable isp middleware\n");
			printf("4 : disable dump yuv\n");
			printf("5 : test 1 video output\n");
			printf("6 : test 2 video output\n");
			printf("7 : test 4 video output\n");
			printf("8 : test 6 video output\n");
			printf("9 : test ioctl\n");
			printf("255: exit\n");
			scanf("%d", &op);
			handle_op(op);
		} while (op != 255);
	}

	for(n = 0; n < VIDEO_DEV_NUM; n++) {
		if (v4l2_fd[n] > 0) {
			CVI_ISP_V4L2_Exit(n);
			is_ispmw_init[n] = 0;
		}

		for(i = 0; i < REQ_BUFFER_NUM; i++) {
			if(!framebuf[n][i].start) {
				munmap(framebuf[n][i].start, framebuf[n][i].length);
				framebuf[n][i].start = NULL;
			}
		}

		if (v4l2_fd[n] > 0) {
			close(v4l2_fd[n]);
			v4l2_fd[n] =  0;
			printf("release dev_%d success\n", n);
		}
	}

	is_run_isp_mw = 1;
	is_wdr_mode = 0;

	return 0;
}

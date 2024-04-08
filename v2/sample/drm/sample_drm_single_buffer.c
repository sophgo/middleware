#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>
#include "buffers.h"

//64 beytes ALIGN PIC for test
#define PIC_RGB888 "pic_rgb888.bmp"
#define PIC_BGR888 "pic_bgr888.bmp"
#define PIC_YUV420 "pic_yuv420.yuv"
#define PIC_YUV422 "pic_yuv422.yuv"
#define PIC_NV12 "pic_nv12.yuv"
#define PIC_NV21 "pic_nv21.yuv"
#define PIC_YUYV "pic_yuyv.yuv"
#define PIC_YVYU "pic_yvyu.yuv"
#define PIC_UYVY "pic_uyvy.yuv"
#define PIC_VYUY "pic_vyuy.yuv"

struct cvitek_format {
	uint32_t pixel_format;
	char filename[32];
};

struct cvitek_format disp_formats[] = {
	{ DRM_FORMAT_NV12, PIC_NV12 },
	{ DRM_FORMAT_NV21, PIC_NV21 },
	{ DRM_FORMAT_YUYV, PIC_YUYV },
	{ DRM_FORMAT_YVYU, PIC_YVYU },
	{ DRM_FORMAT_UYVY, PIC_UYVY },
	{ DRM_FORMAT_VYUY, PIC_VYUY },
	{ DRM_FORMAT_YUV420, PIC_YUV420 },
	{ DRM_FORMAT_YUV422, PIC_YUV422 },
	{ DRM_FORMAT_RGB888, PIC_RGB888 },
	{ DRM_FORMAT_BGR888, PIC_BGR888 },
};

struct bo *bo_obj;

void print_usage(char *arg)
{
	printf("usage:%s <crtc_id> <connector_id>\n", arg);
}

int main(int argc, char *argv[])
{
	int fd;
	drmModeConnector *conn;
	drmModeRes *res;
	uint32_t crtc_id;
	uint32_t conn_id;
	if (argc < 3) {
		print_usage(argv[0]);
		return 0;
	} else {
		sscanf(argv[1], "%u", &crtc_id);
		sscanf(argv[2], "%u", &conn_id);
	}

	fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);

	res = drmModeGetResources(fd);
	for (int i = 0; i < res->count_crtcs; i++) {
		if(crtc_id == res->crtcs[i])
			break;
		if(i == res->count_crtcs -1) {
			printf("not find crtc-%u\n", crtc_id);
			close(fd);
			return 0;
		}
	}

	for (int i = 0; i < res->count_connectors; i++) {
		if(conn_id == res->connectors[i])
			break;
		if(i == res->count_connectors -1) {
			printf("not find connector-%u\n", conn_id);
			close(fd);
			return 0;
		}
	}

	conn = drmModeGetConnector(fd, conn_id);

	for (uint32_t i = 0; i < sizeof(disp_formats) / sizeof(struct cvitek_format); i++) {
		bo_obj = bo_create(fd, disp_formats[i].pixel_format,
			conn->modes[0].hdisplay, conn->modes[0].vdisplay,
			disp_formats[i].filename);

		drmModeSetCrtc(fd, crtc_id, bo_obj->fb_id,
				0, 0, &conn_id, 1, &conn->modes[0]);

		printf("drmModeSetCrtc\n");
		getchar();

		bo_destroy(bo_obj);
	}

	drmModeFreeConnector(conn);
	drmModeFreeResources(res);

	close(fd);

	return 0;
}

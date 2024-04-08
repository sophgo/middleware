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

//NV21 64 beytes ALIGN PIC for test
#define PIC1_NV21 "pic1_nv21.yuv"
#define PIC2_NV21 "pic2_nv21.yuv"

struct bo *bo_obj[2];

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

	bo_obj[0] = bo_create(fd, DRM_FORMAT_NV21,
		conn->modes[0].hdisplay, conn->modes[0].vdisplay,
		PIC1_NV21);
	bo_obj[1] = bo_create(fd, DRM_FORMAT_NV21,
		conn->modes[0].hdisplay, conn->modes[0].vdisplay,
		PIC2_NV21);

	drmModeSetCrtc(fd, crtc_id, bo_obj[0]->fb_id,
			0, 0, &conn_id, 1, &conn->modes[0]);

	printf("drmModeSetCrtc\n");
	getchar();

	drmModeSetCrtc(fd, crtc_id, bo_obj[1]->fb_id,
			0, 0, &conn_id, 1, &conn->modes[0]);

	printf("drmModeSetCrtc\n");
	getchar();

	bo_destroy(bo_obj[0]);
	bo_destroy(bo_obj[1]);

	drmModeFreeConnector(conn);
	drmModeFreeResources(res);

	close(fd);

	return 0;
}

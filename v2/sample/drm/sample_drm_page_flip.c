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
#include <signal.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>
#include "buffers.h"

//NV21 64 beytes ALIGN PIC for test
#define PIC1_NV21 "pic1_nv21.yuv"
#define PIC2_NV21 "pic2_nv21.yuv"

struct bo *bo_obj[2];
static int terminate;

void print_usage(char *arg)
{
	printf("usage:%s <crtc_id> <connector_id>\n", arg);
}

static void modeset_page_flip_handler(int fd, uint32_t frame,
				    uint32_t sec, uint32_t usec,
				    void *data)
{
	(void)frame;
	(void)sec;
	(void)usec;

	static int i = 0;
	uint32_t crtc_id = *(uint32_t *)data;

	i ^= 1;

	drmModePageFlip(fd, crtc_id, bo_obj[i]->fb_id,
			DRM_MODE_PAGE_FLIP_EVENT, data);

	usleep(500000);
}

static void sigint_handler(int arg)
{
	(void)arg;
	terminate = 1;
}

int main(int argc, char *argv[])
{
	int fd;
	drmEventContext ev = {};
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

	/* register CTRL+C terminate interrupt */
	signal(SIGINT, sigint_handler);

	ev.version = DRM_EVENT_CONTEXT_VERSION;
	ev.page_flip_handler = modeset_page_flip_handler;

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

	drmModePageFlip(fd, crtc_id, bo_obj[0]->fb_id,
			DRM_MODE_PAGE_FLIP_EVENT, &crtc_id);

	while (!terminate) {
		drmHandleEvent(fd, &ev);
	}

	bo_destroy(bo_obj[0]);
	bo_destroy(bo_obj[1]);

	drmModeFreeConnector(conn);
	drmModeFreeResources(res);

	close(fd);

	return 0;
}

#ifndef __BUFFERS_H__
#define __BUFFERS_H__

struct bo
{
	int fd;
	void *ptr;
	size_t size;
	size_t offset;
	size_t pitch;
	unsigned handle;
	uint32_t fb_id;
};

struct bo *bo_create(int fd, unsigned int format,
		   unsigned int width, unsigned int height,
		   char *filename);
void bo_destroy_dumb(struct bo *bo);
void bo_destroy(struct bo *bo);

#endif

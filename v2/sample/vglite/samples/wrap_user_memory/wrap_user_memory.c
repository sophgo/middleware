/*
 Resolution: 10 x 10
 Format: VG_LITE_RGBA8888
 Transformation: None
 Alpha Blending: None
 Related APIs: vg_lite_map
 Description: Allocate a block of memory for a buffer with malloc, then clear the buffer with red.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#define DEFAULT_SIZE   256.0f;
#define bpp            4
#define memory_align   64
#define __func__ __FUNCTION__
#define IS_ERROR(status)         (status > 0)
#define CHECK_ERROR(Function) \
    error = Function; \
    if (IS_ERROR(error)) \
    { \
        printf("[%s: %d] failed.error type is %s\n", __func__, __LINE__,error_type[error]);\
        goto ErrorHandler; \
    }
#define VG_ALIGN(number, alignment)    \
        (((number) + ((alignment) - 1)) & ~((alignment) - 1))

char* error_type[] =
{
    "VG_LITE_SUCCESS",
    "VG_LITE_INVALID_ARGUMENT",
    "VG_LITE_OUT_OF_MEMORY",
    "VG_LITE_NO_CONTEXT",
    "VG_LITE_TIMEOUT",
    "VG_LITE_OUT_OF_RESOURCES",
    "VG_LITE_GENERIC_IO",
    "VG_LITE_NOT_SUPPORT",
};

static int   fb_width = 10, fb_height = 10;
static vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.
static void* memory = NULL;

void cleanup(void)
{
    if (buffer.handle != NULL) {
        vg_lite_unmap(&buffer);
    }
    free(memory);
    vg_lite_close();
}

int main(int argc, const char* argv[])
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));
    buffer.width = fb_width;
    buffer.height = fb_width;
    buffer.format = VG_LITE_RGBA8888;
    buffer.stride = buffer.width * bpp;
    memory = malloc(buffer.stride * buffer.height + memory_align);
    buffer.memory = (vg_lite_pointer)VG_ALIGN((uint64_t)memory, memory_align);

    vg_lite_map_flag_t flag = VG_LITE_MAP_USER_MEMORY;
    int32_t fd = -1;
    /* when using VG_LITE_HAL_MAP_USER_MEMORY mode, fd is invalid. */
    CHECK_ERROR(vg_lite_map(&buffer, flag, fd));
    vg_lite_clear(&buffer, NULL, 0xFF0000FF);
    vg_lite_finish();
    CHECK_ERROR(vg_lite_flush_mapped_buffer(&buffer));
    vg_lite_save_png("wrap_user_memory.png", &buffer);

ErrorHandler:
    cleanup();
    return 0;
}

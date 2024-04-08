#include "GL/gl.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/queue.h>
#include "vglite/vg_lite.h"
#include "vglite/vg_lite_util.h"


// #define GL_DBG printf
#ifdef GL_DEBUG
#define GL_DBG(FMT, ...) printf("[DBG][%s:%d]" FMT, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define GL_DBG(FMT, ...)
#endif

#define GL_INFO(FMT, ...) printf("[INFO][%s:%d]" FMT, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define GL_ERR(FMT, ...) printf("[ERR][%s:%d]" FMT, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define DEFAULT_TESS_WIDTH   256
#define DEFAULT_TESS_HEIGHT  256

#define GLMAX(a, b) ((a) > (b) ? (a) : (b))
#define GLMIN(a, b) ((a) < (b) ? (a) : (b))

char *error_type[] =
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

#define IS_ERROR(status)         (status > 0)

#define CHECK_ERROR(Function) \
    error = Function; \
    if (IS_ERROR(error)) \
    { \
        printf("[%s: %d] failed.error type is %s\n", __func__, __LINE__,error_type[error]);\
        goto ErrorHandler; \
    }

#define MAKE_ABGRB8888(A, B, G, R) ((A <<24) | (B << 16) | (G << 8) | R)
#define GL_UNUSED(X)   ((void)X)

struct vertex {
    int32_t x;
    int32_t y;
    STAILQ_ENTRY(vertex) vertices_queue;        /* Singly linked tail queue */
};

STAILQ_HEAD(stailhead, vertex);

static void glDrawLines(vg_lite_buffer_t *fb, struct stailhead *vq);
static void glDrawLineLoop(vg_lite_buffer_t *fb, struct stailhead *vq);
static void glDrawLineStrip(vg_lite_buffer_t *fb, struct stailhead *vq);
static void glDrawQuads(vg_lite_buffer_t *fb, struct stailhead *vq);
static void glDrawTriangles(vg_lite_buffer_t *fb, struct stailhead *vq);
static void glDrawPolygon(vg_lite_buffer_t *fb, struct stailhead *vq);


static vg_lite_buffer_t color_buffer;
// the followings are not used in 2D GL API
static vg_lite_buffer_t depth_buffer;
static vg_lite_buffer_t accum_buffer;
static vg_lite_buffer_t stencil_buffer;

/* colors */
static vg_lite_color_t clear_color;
static vg_lite_color_t current_color;

/* matrix */
static vg_lite_matrix_t current_matrix;

// capabilities
static bool scissor_enable;
static bool blend_enable;

// modes
static GLenum begin_mode;
static GLenum g_blendequation;
static GLenum g_sfactor;
static GLenum g_dfactor;

// verticse queue
struct stailhead vertex_queue = STAILQ_HEAD_INITIALIZER(vertex_queue);

void __attribute__((constructor)) GL_init(void)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    // vg_lite_rectangle_t rect;

    printf("[%s:%d]\n", __FUNCTION__, __LINE__);

    CHECK_ERROR(vg_lite_init(DEFAULT_TESS_WIDTH, DEFAULT_TESS_HEIGHT));

    color_buffer.width = DEFAULT_TESS_WIDTH;
    color_buffer.height = DEFAULT_TESS_HEIGHT;
    color_buffer.format = VG_LITE_RGBA8888;
    CHECK_ERROR(vg_lite_allocate(&color_buffer));

    vg_lite_identity(&current_matrix);

    // init capabilities
    scissor_enable = false;
    blend_enable = false;
    g_blendequation = GL_FUNC_ADD;
    // rect.x = 0;
    // rect.y = 0;
    // rect.width = color_buffer.width;
    // rect.width = color_buffer.height;
    // CHECK_ERROR(vg_lite_scissor_rects(1, &rect));

ErrorHandler:
    return ;
}

void __attribute__((destructor)) GL_fini(void)
{
    vg_lite_buffer_t *fb;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    fb = &color_buffer;
    if (fb->handle != NULL) {
        // #ifdef GL_DEBUG
        vg_lite_save_png("gl.png", fb);
        // #endif
        CHECK_ERROR(vg_lite_free(fb));
    }

    CHECK_ERROR(vg_lite_close(DEFAULT_TESS_WIDTH, DEFAULT_TESS_HEIGHT));

ErrorHandler:
    return ;
}

static int32_t sqtailq_get_count(struct stailhead *vq)
{
    int32_t count = 0;
    struct vertex *np;

    if (vq == NULL) {
        return count;
    }

    STAILQ_FOREACH(np, vq, vertices_queue) {
        count++;
    }

    return count;
}

static void sqtailq_free_all(struct stailhead *vq)
{
    struct vertex *v;

    if (vq == NULL) {
        return;
    }

    v = STAILQ_FIRST(vq);
    while(v != NULL) {
        STAILQ_REMOVE(vq, v, vertex, vertices_queue); /* Deletion */
        free(v);
        v = STAILQ_FIRST(vq);
    }
}

void glBegin (GLenum mode)
{
    begin_mode = mode;
}

void glBlendEquation(GLenum mode)
{

}

void glBlendFunc (GLenum sfactor, GLenum dfactor)
{
    g_sfactor = sfactor;
    g_dfactor = dfactor;

    printf("[%s:%d]\n", __FUNCTION__, __LINE__);

    switch (sfactor) {
    case GL_ZERO:
        vg_lite_source_global_alpha(VG_LITE_GLOBAL, 0);
        break;
    case GL_ONE:
        vg_lite_source_global_alpha(VG_LITE_GLOBAL, 255);
        break;
    default:
        break;
    }

    switch (dfactor) {
    case GL_ZERO:
        vg_lite_dest_global_alpha(VG_LITE_GLOBAL, 0);
        break;
    case GL_ONE:
        vg_lite_dest_global_alpha(VG_LITE_GLOBAL, 255);
        break;
    default:
        break;
    }
}

void glClear (GLbitfield mask)
{
    vg_lite_buffer_t *fb;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    if (mask & GL_COLOR_BUFFER_BIT) {
        fb = &color_buffer;
        CHECK_ERROR(vg_lite_clear(fb, NULL, clear_color));
    }

ErrorHandler:
    return ;
}

void glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    uint32_t u32_alpha = alpha * 255.0f;
    uint32_t u32_blue = blue * 255.0f;
    uint32_t u32_green = green * 255.0f;
    uint32_t u32_red = red * 255.0f;
    clear_color = MAKE_ABGRB8888(u32_alpha, u32_blue, u32_green, u32_red);
}

void glColor3f (GLfloat red, GLfloat green, GLfloat blue)
{
    GLfloat alpha = 1.0f;
    uint32_t u32_alpha = alpha * 255.0f;
    uint32_t u32_blue = blue * 255.0f;
    uint32_t u32_green = green * 255.0f;
    uint32_t u32_red = red * 255.0f;

    current_color = MAKE_ABGRB8888(u32_alpha, u32_blue, u32_green, u32_red);
}

void glDisable (GLenum cap)
{
    switch (cap) {
    case GL_BLEND:
        blend_enable = false;
        break;
    case GL_SCISSOR_TEST:
        scissor_enable = false;
        vg_lite_disable_scissor();
        break;

    default:
        break;
    }
}

void glEnable (GLenum cap)
{
    switch (cap) {
    case GL_BLEND:
        blend_enable = true;
        break;

    case GL_SCISSOR_TEST:
        scissor_enable = true;
        vg_lite_enable_scissor();
        break;

    default:
        break;
    }
}

void glEnd (void)
{
    vg_lite_buffer_t *fb;
    struct stailhead *vq;

    fb = &color_buffer;
    vq = &vertex_queue;

    switch (begin_mode) {
    case GL_LINES: {
        glDrawLines(fb, vq);
        break;
    }

    case GL_LINE_LOOP: {
        glDrawLineLoop(fb, vq);
        break;
    }

    case GL_LINE_STRIP: {
        glDrawLineStrip(fb, vq);
        break;
    }

    case GL_TRIANGLES: {
        glDrawTriangles(fb, vq);
        break;
    }

    case  GL_QUADS: {
        glDrawQuads(fb, vq);
        break;
    }

    case GL_POLYGON: {
        glDrawPolygon(fb, vq);
        break;
    }

    default:
        break;
    }

    sqtailq_free_all(vq);
}

void glFinish (void)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;

    CHECK_ERROR(vg_lite_finish());

ErrorHandler:
    return ;
}

void glFlush (void)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;

    CHECK_ERROR(vg_lite_flush());

ErrorHandler:
    return ;
}

GLboolean glIsEnabled (GLenum cap)
{
    GLboolean is_enabled = false;

    switch (cap) {
    case GL_BLEND:
        is_enabled = true;
        break;

    default:
        break;
    }

    return is_enabled;
}

void glLoadIdentity(void)
{
    vg_lite_identity(&current_matrix);
}

void glMultMatrixf(const GLfloat* m)
{

}

void glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{

}


void glDrawRect(vg_lite_buffer_t *fb, GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    vg_lite_matrix_t *pmatrix;
    uint32_t data_size;
    vg_lite_path_t path;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_blend_t blend = VG_LITE_BLEND_NONE;

    static uint8_t sides_cmd[] = {
        VLC_OP_MOVE,
        VLC_OP_LINE,
        VLC_OP_LINE,
        VLC_OP_LINE,

        VLC_OP_END
    };
    static float sides_data_left[] = {
        0, 0,
        50, 0,
        50, 50,
        0, 50,
    };

    if (blend_enable) {
        // only support ADDITIVE in opengl v1.1
        blend = VG_LITE_BLEND_ADDITIVE;
    }

    pmatrix = &current_matrix;

    sides_data_left[0] = x1;
    sides_data_left[1] = y1;
    sides_data_left[2] = x1;
    sides_data_left[3] = y2;
    sides_data_left[4] = x2;
    sides_data_left[5] = y2;
    sides_data_left[6] = x2;
    sides_data_left[7] = y1;

    data_size = vg_lite_get_path_length(sides_cmd, sizeof(sides_cmd), VG_LITE_FP32);

    CHECK_ERROR(vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0));
    path.path = malloc(data_size);
    CHECK_ERROR(vg_lite_append_path(&path, sides_cmd, sides_data_left, sizeof(sides_cmd)));

    CHECK_ERROR(vg_lite_draw(fb, &path, VG_LITE_FILL_NON_ZERO, pmatrix, blend, current_color));

    CHECK_ERROR(vg_lite_clear_path(&path));

ErrorHandler:
    return ;

}

void glRectd (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
    int32_t i32_x1, i32_y1, i32_x2, i32_y2;
    int32_t width, height;
    vg_lite_rectangle_t rect;
    vg_lite_buffer_t *fb;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    fb = &color_buffer;
    i32_x1 = (int32_t)((fb->width / 2) * (1 + x1)) - 1;
    i32_y1 = (int32_t)((fb->height / 2) * (1 - y1)) - 1;
    i32_x2 = (int32_t)((fb->width / 2) * (1 + x2)) - 1;
    i32_y2 = (int32_t)((fb->height / 2) * (1 - y2)) - 1;

    glDrawRect(fb, i32_x1, i32_y1, i32_x2, i32_y2);

    return ;
}

void glRectdv (const GLdouble *v1, const GLdouble *v2)
{
    double x1, y1, x2, y2;
    int32_t i32_x1, i32_y1, i32_x2, i32_y2;
    int32_t width, height;
    vg_lite_rectangle_t rect;
    vg_lite_buffer_t *fb;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    fb = &color_buffer;
    x1 = v1[0];
    y1 = v1[1];
    x2 = v2[0];
    y2 = v2[1];
    i32_x1 = (int32_t)((fb->width / 2) * (1 + x1)) - 1;
    i32_y1 = (int32_t)((fb->height / 2) * (1 - y1)) - 1;
    i32_x2 = (int32_t)((fb->width / 2) * (1 + x2)) - 1;
    i32_y2 = (int32_t)((fb->height / 2) * (1 - y2)) - 1;

    glDrawRect(fb, i32_x1, i32_y1, i32_x2, i32_y2);

    return ;
}

void glRectf (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    int32_t i32_x1, i32_y1, i32_x2, i32_y2;
    int32_t width, height;
    vg_lite_rectangle_t rect;
    vg_lite_buffer_t *fb;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    fb = &color_buffer;
    i32_x1 = (int32_t)((fb->width / 2) * (1 + x1)) - 1;
    i32_y1 = (int32_t)((fb->height / 2) * (1 - y1)) - 1;
    i32_x2 = (int32_t)((fb->width / 2) * (1 + x2)) - 1;
    i32_y2 = (int32_t)((fb->height / 2) * (1 - y2)) - 1;

    glDrawRect(fb, i32_x1, i32_y1, i32_x2, i32_y2);

    return ;
}

void glRectfv (const GLfloat *v1, const GLfloat *v2)
{
    double x1, y1, x2, y2;
    int32_t i32_x1, i32_y1, i32_x2, i32_y2;
    int32_t width, height;
    vg_lite_rectangle_t rect;
    vg_lite_buffer_t *fb;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    fb = &color_buffer;
    x1 = v1[0];
    y1 = v1[1];
    x2 = v2[0];
    y2 = v2[1];

    i32_x1 = (int32_t)((fb->width / 2) * (1 + x1)) - 1;
    i32_y1 = (int32_t)((fb->height / 2) * (1 - y1)) - 1;
    i32_x2 = (int32_t)((fb->width / 2) * (1 + x2)) - 1;
    i32_y2 = (int32_t)((fb->height / 2) * (1 - y2)) - 1;

    glDrawRect(fb, i32_x1, i32_y1, i32_x2, i32_y2);

    return ;
}
void glRecti (GLint x1, GLint y1, GLint x2, GLint y2)
{
    int32_t i32_x1, i32_y1, i32_x2, i32_y2;
    int32_t width, height;
    vg_lite_rectangle_t rect;
    vg_lite_buffer_t *fb;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    fb = &color_buffer;
    i32_x1 = (int32_t)((fb->width / 2) * (1 + x1)) - 1;
    i32_y1 = (int32_t)((fb->height / 2) * (1 - y1)) - 1;
    i32_x2 = (int32_t)((fb->width / 2) * (1 + x2)) - 1;
    i32_y2 = (int32_t)((fb->height / 2) * (1 - y2)) - 1;

    glDrawRect(fb, i32_x1, i32_y1, i32_x2, i32_y2);

    return ;
}

void glRectiv (const GLint *v1, const GLint *v2)
{
    double x1, y1, x2, y2;
    int32_t i32_x1, i32_y1, i32_x2, i32_y2;
    int32_t width, height;
    vg_lite_rectangle_t rect;
    vg_lite_buffer_t *fb;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    fb = &color_buffer;
    x1 = v1[0];
    y1 = v1[1];
    x2 = v2[0];
    y2 = v2[1];
    i32_x1 = (int32_t)((fb->width / 2) * (1 + x1));
    i32_y1 = (int32_t)((fb->height / 2) * (1 - y1));
    i32_x2 = (int32_t)((fb->width / 2) * (1 + x2));
    i32_y2 = (int32_t)((fb->height / 2) * (1 - y2));

    glDrawRect(fb, i32_x1, i32_y1, i32_x2, i32_y2);

    return ;
}

void glRects (GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
    int32_t i32_x1, i32_y1, i32_x2, i32_y2;
    int32_t width, height;
    vg_lite_rectangle_t rect;
    vg_lite_buffer_t *fb;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    fb = &color_buffer;
    i32_x1 = (int32_t)((fb->width / 2) * (1 + x1));
    i32_y1 = (int32_t)((fb->height / 2) * (1 - y1));
    i32_x2 = (int32_t)((fb->width / 2) * (1 + x2));
    i32_y2 = (int32_t)((fb->height / 2) * (1 - y2));

    glDrawRect(fb, i32_x1, i32_y1, i32_x2, i32_y2);

    return ;
}

void glRectsv (const GLshort *v1, const GLshort *v2)
{
    double x1, y1, x2, y2;
    int32_t i32_x1, i32_y1, i32_x2, i32_y2;
    int32_t width, height;
    vg_lite_rectangle_t rect;
    vg_lite_buffer_t *fb;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    fb = &color_buffer;
    x1 = v1[0];
    y1 = v1[1];
    x2 = v2[0];
    y2 = v2[1];
    i32_x1 = (int32_t)((fb->width / 2) * (1 + x1));
    i32_y1 = (int32_t)((fb->height / 2) * (1 - y1));
    i32_x2 = (int32_t)((fb->width / 2) * (1 + x2));
    i32_y2 = (int32_t)((fb->height / 2) * (1 - y2));

    glDrawRect(fb, i32_x1, i32_y1, i32_x2, i32_y2);

    return ;
}


void glRotated (GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    // ignore (x,y,z) vector
    GL_UNUSED(x);
    GL_UNUSED(y);
    GL_UNUSED(z);

    vg_lite_rotate(angle, &current_matrix);
}

void glRotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    // ignore (x,y,z) vector
    GL_UNUSED(x);
    GL_UNUSED(y);
    GL_UNUSED(z);

    vg_lite_rotate(angle, &current_matrix);
}

void glScaled (GLdouble x, GLdouble y, GLdouble z)
{
    // ignore z
    GL_UNUSED(z);

    vg_lite_scale(x, y, &current_matrix);
}

void glScalef (GLfloat x, GLfloat y, GLfloat z)
{
    // ignore z
    GL_UNUSED(z);
    vg_lite_scale(x, y, &current_matrix);
}

void glScissor (GLint x, GLint y, GLsizei width, GLsizei height)
{
    vg_lite_rectangle_t rect;
    vg_lite_buffer_t *fb;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    fb = &color_buffer;

    rect.x = x;
    rect.y = fb->height - y - 1 - height;
    rect.width = width;
    rect.height = height;

    printf("rect:%d, %d, %d, %d\n", rect.x, rect.y, rect.width, rect.height);

    CHECK_ERROR(vg_lite_scissor_rects(1, &rect));

ErrorHandler:
    return ;

}

void glTranslated (GLdouble x, GLdouble y, GLdouble z)
{
    vg_lite_buffer_t *fb;
    vg_lite_float_t fx, fy;
    // ignore z
    GL_UNUSED(z);

    fb = &color_buffer;
    fx = (int32_t)(((fb->width) / 2) * x);
    fy = (int32_t)(((fb->height) / 2) * y);

    vg_lite_translate(fx, fy, &current_matrix);
}

void glTranslatef (GLfloat x, GLfloat y, GLfloat z)
{
    vg_lite_buffer_t *fb;
    vg_lite_float_t fx, fy;
    // ignore z
    GL_UNUSED(z);

    fb = &color_buffer;
    fx = (int32_t)(((fb->width) / 2) * x);
    fy = (int32_t)(((fb->height) / 2) * y);

    vg_lite_translate(fx, fy, &current_matrix);
}

void glVertex2f (GLfloat x, GLfloat y)
{
    struct vertex *vertex;
    vg_lite_buffer_t *fb;

    fb = &color_buffer;

    vertex = (struct vertex *)malloc(sizeof(struct vertex));
    vertex->x = (int32_t)(((fb->width) / 2) * (1 + x));
    vertex->y = (int32_t)(((fb->height) / 2) * (1 - y));

    STAILQ_INSERT_TAIL(&vertex_queue, vertex, vertices_queue);
}

void glVertex2fv (const GLfloat *v)
{
    struct vertex *vertex;
    vg_lite_buffer_t *fb;

    fb = &color_buffer;

    vertex = (struct vertex *)malloc(sizeof(struct vertex));
    vertex->x = (int32_t)((fb->width / 2) * (1 + v[0]));
    vertex->y = (int32_t)((fb->height / 2) * (1 - v[1]));

    STAILQ_INSERT_TAIL(&vertex_queue, vertex, vertices_queue);
}

void glVertex2i (GLint x, GLint y)
{
    struct vertex *vertex;
    vg_lite_buffer_t *fb;

    fb = &color_buffer;

    vertex = (struct vertex *)malloc(sizeof(struct vertex));
    vertex->x = (int32_t)((fb->width / 2) * (1 + x));
    vertex->y = (int32_t)((fb->height / 2) * (1 - y));

    STAILQ_INSERT_TAIL(&vertex_queue, vertex, vertices_queue);
}

void glVertex2iv (const GLint *v)
{
    struct vertex *vertex;
    vg_lite_buffer_t *fb;

    fb = &color_buffer;

    vertex = (struct vertex *)malloc(sizeof(struct vertex));
    vertex->x = (int32_t)((fb->width / 2) * (1 + v[0]));
    vertex->y = (int32_t)((fb->height / 2) * (1 - v[1]));

    STAILQ_INSERT_TAIL(&vertex_queue, vertex, vertices_queue);
}

void glVertex2s (GLshort x, GLshort y)
{
    struct vertex *vertex;
    vg_lite_buffer_t *fb;

    fb = &color_buffer;

    vertex = (struct vertex *)malloc(sizeof(struct vertex));
    vertex->x = (int32_t)((fb->width / 2) * (1 + x));
    vertex->y = (int32_t)((fb->height / 2) * (1 - y));

    STAILQ_INSERT_TAIL(&vertex_queue, vertex, vertices_queue);
}

void glVertex2sv (const GLshort *v)
{
    struct vertex *vertex;
    vg_lite_buffer_t *fb;

    fb = &color_buffer;

    vertex = (struct vertex *)malloc(sizeof(struct vertex));
    vertex->x = (int32_t)((fb->width / 2) * (1 + v[0]));
    vertex->y = (int32_t)((fb->height / 2) * (1 - v[1]));

    STAILQ_INSERT_TAIL(&vertex_queue, vertex, vertices_queue);
}

void glDrawSingleLine(vg_lite_buffer_t *fb, struct vertex *from, struct vertex *to)
{
    vg_lite_matrix_t *pmatrix;
    uint32_t data_size;
    vg_lite_path_t path;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_blend_t blend = VG_LITE_BLEND_NONE;

    static uint8_t sides_cmd[] = {
        VLC_OP_MOVE,
        VLC_OP_LINE,
        VLC_OP_LINE,
        VLC_OP_LINE,

        VLC_OP_END
    };
    static float sides_data_left[] = {
        0, 0,
        50, 0,
        50, 50,
        0, 50,
    };

    if (!from || !to) {
        return ;
    }

    if (blend_enable) {
        // only support ADDITIVE in opengl v1.1
        blend = VG_LITE_BLEND_ADDITIVE;
    }

    pmatrix = &current_matrix;

    sides_data_left[0] = from->x;
    sides_data_left[1] = from->y;
    sides_data_left[2] = to->x;
    sides_data_left[3] = to->y;

    if (from->y == to->y) {
        sides_data_left[4] = to->x;
        sides_data_left[5] = to->y + 1;
        sides_data_left[6] = from->x;
        sides_data_left[7] = from->y + 1;
    } else {
        sides_data_left[4] = to->x + 1;
        sides_data_left[5] = to->y + 1;
        sides_data_left[6] = from->x + 1;
        sides_data_left[7] = from->y + 1;
    }

    data_size = vg_lite_get_path_length(sides_cmd, sizeof(sides_cmd), VG_LITE_FP32);

    CHECK_ERROR(vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0));
    path.path = malloc(data_size);
    CHECK_ERROR(vg_lite_append_path(&path, sides_cmd, sides_data_left, sizeof(sides_cmd)));

    CHECK_ERROR(vg_lite_draw(fb, &path, VG_LITE_FILL_NON_ZERO, pmatrix, blend, current_color));

    CHECK_ERROR(vg_lite_clear_path(&path));

ErrorHandler:
    return ;
}

void glDrawLines(vg_lite_buffer_t *fb, struct stailhead *vq)
{
    int32_t remain;
    struct vertex *v;
    struct vertex *from, *to;
    const int32_t vertex_number = 2;
    if (vq == NULL) {
        return;
    }

    remain = sqtailq_get_count(vq);

    if (remain < vertex_number) {
        return;
    }

    for (int32_t i = 0; i < remain / vertex_number; i++) {
        from = STAILQ_FIRST(vq);
        if (from == NULL) {
            break;
        }
        STAILQ_REMOVE(vq, from, vertex, vertices_queue); /* Deletion */

        to = STAILQ_FIRST(vq);
        if (to == NULL) {
            break;
        }
        STAILQ_REMOVE(vq, to, vertex, vertices_queue); /* Deletion */

        // draw one single line
        glDrawSingleLine(fb, from, to);

        free(from);
        free(to);
    }
}

static void glDrawLineLoop(vg_lite_buffer_t *fb, struct stailhead *vq)
{
    int32_t remain;
    struct vertex *v;
    struct vertex *first, *current, *from, *to;
    const int32_t vertex_number = 2;
    if (vq == NULL) {
        return;
    }

    remain = sqtailq_get_count(vq);

    if (remain < vertex_number) {
        return;
    }

    first = STAILQ_FIRST(vq);
    current = first;
    for (int32_t i = 0; i < remain; i++) {
        from = current;
        if (from == NULL) {
            break;
        }

        to = STAILQ_NEXT(current, vertices_queue);
        if (i == (remain - 1)) {
            to = first;
        }
        if (to == NULL) {
            break;
        }

        // draw one single line
        glDrawSingleLine(fb, from, to);

        current = to;
    }
}

static void glDrawLineStrip(vg_lite_buffer_t *fb, struct stailhead *vq)
{
    int32_t remain;
    struct vertex *v;
    struct vertex *first, *current, *from, *to;
    const int32_t vertex_number = 2;
    if (vq == NULL) {
        return;
    }

    remain = sqtailq_get_count(vq);

    if (remain < vertex_number) {
        return;
    }

    first = STAILQ_FIRST(vq);
    current = first;
    for (int32_t i = 0; i < (remain - 1); i++) {
        from = current;
        if (from == NULL) {
            break;
        }

        to = STAILQ_NEXT(current, vertices_queue);
        if (to == NULL) {
            break;
        }

        // draw one single line
        glDrawSingleLine(fb, from, to);

        current = to;
    }
}

void glDrawTriangles(vg_lite_buffer_t *fb, struct stailhead *vq)
{
    int32_t remain;
    struct vertex *v;
    const int32_t vertex_number = 3;
    if (vq == NULL) {
        return;
    }

    remain = sqtailq_get_count(vq);

    if (remain < vertex_number) {
        return;
    }

    vg_lite_matrix_t *pmatrix;
    uint32_t data_size;
    vg_lite_path_t path;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_blend_t blend = VG_LITE_BLEND_NONE;

    static uint8_t sides_cmd[] = {
        VLC_OP_MOVE,
        VLC_OP_LINE,
        VLC_OP_LINE,
        VLC_OP_LINE,

        VLC_OP_END
    };
    static float sides_data_left[] = {
        0, 0,
        50, 0,
        50, 50,
        0, 0,
    };

    if (blend_enable) {
        // only support ADDITIVE in opengl v1.1
        blend = VG_LITE_BLEND_ADDITIVE;
    }

    pmatrix = &current_matrix;

    for (int32_t i = 0; i < remain / vertex_number; i++) {
        for (int32_t j = 0; j < vertex_number; j++) {
            v = STAILQ_FIRST(vq);
            if (v == NULL) {
                break;
            }

            sides_data_left[2 * j] = v->x;
            sides_data_left[2 * j + 1] = v->y;

            if (j == 0) {
                sides_data_left[vertex_number * 2] = v->x;
                sides_data_left[vertex_number * 2 + 1] = v->y;
            }

            STAILQ_REMOVE(vq, v, vertex, vertices_queue); /* Deletion */
            free(v);
        }

        data_size = vg_lite_get_path_length(sides_cmd, sizeof(sides_cmd), VG_LITE_FP32);

        CHECK_ERROR(vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0));
        path.path = malloc(data_size);
        CHECK_ERROR(vg_lite_append_path(&path, sides_cmd, sides_data_left, sizeof(sides_cmd)));

        CHECK_ERROR(vg_lite_draw(fb, &path, VG_LITE_FILL_NON_ZERO, pmatrix, blend, current_color));

        CHECK_ERROR(vg_lite_clear_path(&path));
    }

ErrorHandler:
    return ;
}


void glDrawQuads(vg_lite_buffer_t *fb, struct stailhead *vq)
{
    int32_t remain;
    struct vertex *v;
    const int32_t vertex_number = 4;
    if (vq == NULL) {
        return;
    }

    remain = sqtailq_get_count(vq);

    if (remain < vertex_number) {
        return;
    }

    vg_lite_matrix_t *pmatrix;
    uint32_t data_size;
    vg_lite_path_t path;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_blend_t blend = VG_LITE_BLEND_NONE;

    static uint8_t sides_cmd[] = {
        VLC_OP_MOVE,
        VLC_OP_LINE,
        VLC_OP_LINE,
        VLC_OP_LINE,
        VLC_OP_LINE,

        VLC_OP_END
    };
    static float sides_data_left[] = {
        0, 0,
        50, 0,
        50, 50,
        0, 50,
        0, 0,
    };

    if (blend_enable) {
        // only support ADDITIVE in opengl v1.1
        blend = VG_LITE_BLEND_ADDITIVE;
    }

    pmatrix = &current_matrix;

    for (int32_t i = 0; i < remain / vertex_number; i++) {
        for (int32_t j = 0; j < vertex_number; j++) {
            v = STAILQ_FIRST(vq);
            if (v == NULL) {
                break;
            }

            sides_data_left[2 * j] = v->x;
            sides_data_left[2 * j + 1] = v->y;

            if (j == 0) {
                sides_data_left[vertex_number * 2] = v->x;
                sides_data_left[vertex_number * 2 + 1] = v->y;
            }

            STAILQ_REMOVE(vq, v, vertex, vertices_queue); /* Deletion */
            free(v);
        }

        data_size = vg_lite_get_path_length(sides_cmd, sizeof(sides_cmd), VG_LITE_FP32);

        CHECK_ERROR(vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0));
        path.path = malloc(data_size);
        CHECK_ERROR(vg_lite_append_path(&path, sides_cmd, sides_data_left, sizeof(sides_cmd)));

        CHECK_ERROR(vg_lite_draw(fb, &path, VG_LITE_FILL_NON_ZERO, pmatrix, blend, current_color));

        CHECK_ERROR(vg_lite_clear_path(&path));
    }

ErrorHandler:
    return ;
}

static void glDrawPolygon(vg_lite_buffer_t *fb, struct stailhead *vq)
{
    int32_t remain;
    struct vertex *v;
    uint8_t *sides_cmd;
    float *sides_data_left;
    int32_t size_sides_cmd;
    // vg_lite_matrix_t matrix;
    vg_lite_matrix_t *pmatrix;
    uint32_t data_size;
    vg_lite_path_t path;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_blend_t blend = VG_LITE_BLEND_NONE;

    if (fb == NULL
        || vq == NULL) {
        return ;
    }

    if (blend_enable) {
        // only support ADDITIVE in opengl v1.1
        blend = VG_LITE_BLEND_ADDITIVE;
    }


    remain = sqtailq_get_count(vq);

    if (remain <= 0) {
        return;
    }

    size_sides_cmd = (remain + 2);
    sides_cmd = malloc(size_sides_cmd * sizeof(*sides_cmd));
    if (sides_cmd == NULL) {
        GL_ERR("out of memory\n");
        return ;
    }

    sides_data_left = malloc(sizeof(*sides_data_left) * (remain) * 2);
    if (sides_data_left == NULL) {
        GL_ERR("out of memory\n");
        return ;
    }

    sides_cmd[0] = VLC_OP_MOVE;
    sides_cmd[remain] = VLC_OP_CLOSE;
    sides_cmd[remain + 1] = VLC_OP_END;
    for (int32_t i = 1; i < (remain); i++) {
        sides_cmd[i] = VLC_OP_LINE;
    }

    // get all vertices
    for (int32_t i = 0; i < remain; i++) {
        v = STAILQ_FIRST(vq);
        if (v == NULL) {
            break;
        }

        sides_data_left[2 * i] = v->x;
        sides_data_left[2 * i + 1] = v->y;

        STAILQ_REMOVE(vq, v, vertex, vertices_queue); /* Deletion */
        free(v);
    }

    // vg_lite_identity(&matrix);
    pmatrix = &current_matrix;

    data_size = vg_lite_get_path_length(sides_cmd, size_sides_cmd, VG_LITE_FP32);

    CHECK_ERROR(vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0));
    path.path = malloc(data_size);
    CHECK_ERROR(vg_lite_append_path(&path, sides_cmd, sides_data_left, size_sides_cmd));

    CHECK_ERROR(vg_lite_draw(fb, &path, VG_LITE_FILL_EVEN_ODD, pmatrix, blend, current_color));

    CHECK_ERROR(vg_lite_clear_path(&path));

ErrorHandler:
    return ;
}
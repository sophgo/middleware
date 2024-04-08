#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>

#define GL_DEBUG
#ifdef GL_DEBUG
#define GL_DBG(FMT, ...) printf("[DBG][%s:%d]" FMT, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define GL_DBG(FMT, ...)
#endif

static void tde_ut_lines(void);
static void tde_ut_line_loop(void);
static void tde_ut_line_strip(void);
static void tde_ut_triangle(void);
static void tde_ut_square(void);
static void tde_ut_polygon(void);

int main(int argc, char *argv[]) {
    int which = 0;

    if (argc > 1) {
        which = atoi(argv[1]);
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
    glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer (background)

    switch (which) {
    case 0:
        tde_ut_lines();
        break;

    case 1:
        tde_ut_line_loop();
        break;

    case 2:
        tde_ut_line_strip();
        break;

    case 3:
        tde_ut_triangle();
        break;

    case 4:
        tde_ut_square();
        break;

    case 5:
        tde_ut_polygon();
        break;

    case 6:
    default:
        tde_ut_lines();
        tde_ut_line_loop();
        tde_ut_line_strip();
        tde_ut_triangle();
        tde_ut_square();
        tde_ut_polygon();
        break;
    }

    glFlush();  // Render now
}

void tde_ut_lines(void)
{
    // Draw a Red Line
    glBegin(GL_LINES);              // Each set of 3 vertices form a triangle
        glColor3f(1.0f, 0.0f, 0.0f); // Red
        glVertex2f(0.0f, -0.6f); // x, y
        glVertex2f(1.0f, -0.6f);
    glEnd();


    // Draw a Yellow Line
    glBegin(GL_LINES);              // Each set of 3 vertices form a triangle
        glColor3f(1.0f, 1.0f, 0.0f); // Red
        glVertex2f(0.0f, -0.8f); // x, y
        glVertex2f(0.9f, -0.8f);
    glEnd();

    // Draw a Yellow Line
    glBegin(GL_LINES);              // Each set of 3 vertices form a triangle
        glColor3f(1.0f, 1.0f, 0.0f); // Red
        glVertex2f(0.0f, -0.65f); // x, y
        glVertex2f(1.0f, -1.0f);
    glEnd();
}

void tde_ut_line_loop(void)
{
    // Draw a Yellow Line Loop
    glBegin(GL_LINE_LOOP);              // Each set of 3 vertices form a triangle
        glColor3f(1.0f, 1.0f, 0.0f); // Red
        glVertex2f(0.0f, 0.0f); // x, y
        glVertex2f(-0.3f, -0.3f);
        glVertex2f(-0.4f, 0.0f);
    glEnd();
}

void tde_ut_line_strip(void)
{
    // Draw a Yellow Line Strip
    glBegin(GL_LINE_STRIP);              // Each set of 3 vertices form a triangle
        glColor3f(1.0f, 1.0f, 0.0f); // Red
        glVertex2f(0.5f, 0.0f); // x, y
        glVertex2f(1.0f, 0.0f);
        glVertex2f(0.75f, 0.4f);
    glEnd();
}

void tde_ut_triangle(void)
{
    glLoadIdentity();
    glScalef(0.5f, 0.5f, 0.0f);

    // Draw a Red Triangle
    glBegin(GL_TRIANGLES);              // Each set of 3 vertices form a triangle
        glColor3f(1.0f, 0.0f, 0.0f); // Red
        glVertex2f(-1.0f, 1.0f); // x, y
        glVertex2f(0.0f, 1.0f);
        glVertex2f(-1.0f, 0.0f);
        glVertex2f(0.0f, 0.0f); // not used
    glEnd();
}

void tde_ut_square(void)
{
    glLoadIdentity();

    // Draw a Green Square
    glBegin(GL_QUADS);              // Each set of 4 vertices form a quad
        glColor3f(0.0f, 1.0f, 0.0f); // Green
        glVertex2f(0.0f, 0.0f);    // x, y
        glVertex2f(0.5f, 0.0f);
        glVertex2f(0.5f, 0.5f);
        glVertex2f(0.0f, 0.5f);
    glEnd();

    // Draw a Blue Square
    glColor3f(0.0f, 0.0f, 1.0f); // Blue
    GLfloat v1[] = {0.0f, 0.0f};
    GLfloat v2[] = {0.5f, -0.5f};
    glLoadIdentity();
    glTranslatef(-0.5f, 0.0f, 0.0f);

    glRectfv(v1, v2);
    glRectf(0.5f, 0.5f, 1.0f, 1.0f);
}

void tde_ut_polygon(void)
{
    // Draw a Yellow Polygon
    glLoadIdentity();
    glTranslatef(0.2f, 0.0f, 0.0f);
    glRotatef(-20.0f, 0.0f, 0.0f, 1.0f);

    glBegin(GL_POLYGON);              // Each set of 4 vertices form a quad
        glColor3f(1.0f, 1.0f, 0.0f); // Yellow
        glVertex2f(-0.9f, -0.9f);    // x, y
        glVertex2f(-0.9f, -0.7f);
        glVertex2f(-0.7f, -0.6f);
        glVertex2f(-0.5f, -0.4f);
        glVertex2f(-0.4f, -0.6f);
        glVertex2f(-0.7f, -1.0f);
    glEnd();
}


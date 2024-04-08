#include <stdio.h>
#include <GL/gl.h>

#define GL_DEBUG
#ifdef GL_DEBUG
#define GL_DBG(FMT, ...) printf("[DBG][%s:%d]" FMT, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define GL_DBG(FMT, ...)
#endif

#define pi 3.141592653584372


int main(int argc, char *argv[]) {
	int n = 50;
	float R = 0.5f;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
	glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer (background)

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	// draw a red circle
	glBegin(GL_POLYGON);
	glColor3f(1.0f, 0.0f, 0.0f);	// red
	for (int i = 0; i < n; i++)
	{
		GLfloat x, y;
		x = R * sin(2 * pi / n * i) - 0.3;
		y = R * cos(2 * pi / n * i) - 0.3;
		glVertex2f(x, y);
	}
	glEnd();

	// // draw a green circle
	glBegin(GL_POLYGON);
	glColor3f(0.0, 1.0, 0.0);	// Green
	for (int i = 0; i < n; i++)
	{
		glVertex2f(R * sin(2 * pi / n * i) + 0.3, R * cos(2 * pi / n * i) - 0.3);
	}
	glEnd();

	// draw a blue circle
	glBegin(GL_POLYGON);
	glColor3f(0.0, 0.0, 1.0);	// blue
	for (int i = 0; i < n; i++)
	{
		glVertex2f(R * sin(2 * pi / n * i) + 0.0, R * cos(2 * pi / n * i) + 0.3);
	}
	glEnd();

	glFlush();
}

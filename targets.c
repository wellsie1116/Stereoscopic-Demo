/* viewer.c
 * Kevin Wells
 */

#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>

#include <glib/gmacros.h>

#include <stdio.h>
#include <malloc.h>

#include <string.h>
#include <stdbool.h>

#include <math.h>

#include "tex.h"
#include "vect.h"
#include "shader.h"

#define DEBUG 0
#if DEBUG
#define AXIS_LENGTH 30
#endif

#define INITIAL_WIDTH 800
#define INITIAL_HEIGHT 800

int width;
int height;

GLint progTarget;

Point3 cam = {0.0, 0.0, 0.0};
float rotX = 0.0f;
float rotY = 0.0f;

bool stereobuffer = false;

GLfloat fogColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

bool mouseCaptured = false;
int mouseButtonDown;
int mouseDownX;
int mouseDownY;

float convergence = 0.35f;

int
load_texture(const char* path, GLuint* tex)
{
	tga_data_t* dat = tga_data_load((char*)path);

	if (!dat)
	{
		printf("WARNING: Failed to load texture: %s\n", path);
		return 1;
	}

	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dat->w, dat->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, dat->data);

	free(dat);

	return 0;
}

void
render_scene()
{
#if DEBUG
	glDisable(GL_DEPTH_TEST);
	glPushMatrix();
	{
		glBegin(GL_LINES);
		{
			glColor3f(0.5f, 0.0f, 0.0f);
			glVertex3i(-AXIS_LENGTH, 0, 0);
			glVertex3i(0, 0, 0);
			glColor3f(0.0f, 0.5f, 0.0f);
			glVertex3i(0, -AXIS_LENGTH, 0);
			glVertex3i(0, 0, 0);
			glColor3f(0.0f, 0.0f, 0.5f);
			glVertex3i(0, 0, -AXIS_LENGTH);
			glVertex3i(0, 0, 0);

			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex3i(0, 0, 0);
			glVertex3i(AXIS_LENGTH, 0, 0);
			glColor3f(0.0f, 1.0f, 0.0f);
			glVertex3i(0, 0, 0);
			glVertex3i(0, AXIS_LENGTH, 0);
			glColor3f(0.0f, 0.0f, 1.0f);
			glVertex3i(0, 0, 0);
			glVertex3i(0, 0, AXIS_LENGTH);
		}
		glEnd();
	}
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);

	glBegin(GL_QUADS);
	{
		glVertex3f(0.0f, 0.0f, 20.0f);
		glVertex3f(10.0f, 0.0f, 20.0f);
		glVertex3f(10.0f, 10.0f, 20.0f);
		glVertex3f(0.0f, 10.0f, 20.0f);
	}
	glEnd();
#endif

	glColor3f(0.7, 0.7, 0.7);

	int i;
	for (i = 0; i < 21; i++)
	{
		glBegin(GL_LINE_LOOP);
		{
			glVertex3f(-10.0f, 10.0f, i * 5.0f);
			glVertex3f(-10.0f, -10.0f, i * 5.0f);
			glVertex3f(10.0f, -10.0f, i * 5.0f);
			glVertex3f(10.0f, 10.0f, i * 5.0f);
		}
		glEnd();
	}
	for (i = 0; i < 11; i++)
	{
		glBegin(GL_LINES);
		{
			float step = (i - 5) * 2;
			glVertex3f(step, 10.0f, 0.0f);
			glVertex3f(step, 10.0f, 100.0f);

			glVertex3f(step, -10.0f, 0.0f);
			glVertex3f(step, -10.0f, 100.0f);

			glVertex3f(10.0f, step, 0.0f);
			glVertex3f(10.0f, step, 100.0f);

			glVertex3f(-10.0f, step, 0.0f);
			glVertex3f(-10.0f, step, 100.0f);

			glVertex3f(-10.0f, step, 100.0f);
			glVertex3f(10.0f, step, 100.0f);

			glVertex3f(step, -10.0f, 100.0f);
			glVertex3f(step, 10.0f, 100.0f);

		}
		glEnd();
	}

	srand(9001);

	for (i = 0; i < 15; i++)
	{
		float height = (float)rand() / RAND_MAX * 100.0f;
		float x = (float)rand() / RAND_MAX * 16.0f - 10.0f + 2.0f;
		float y = (float)rand() / RAND_MAX * 16.0f - 10.0f + 2.0f;
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_LINES);
		{
			glVertex3f(x, y, height);
			glVertex3f(x, y, 100.0f);
		}
		glEnd();

		glPushMatrix();
		{
			glUseProgram(progTarget);
			glColor3f(1.0f, 0.0f, 0.0f);
			glTranslatef(x, y, height);
			glBegin(GL_QUADS);
			{
				glVertex2f(-2.0f, 2.0f);
				glVertex2f(-2.0f, -2.0f);
				glVertex2f(2.0f, -2.0f);
				glVertex2f(2.0f, 2.0f);
			}
			glEnd();

			glUseProgram(0);
		}
		glPopMatrix();
	}
	rand();

}

void display()
{
	Vector3 lookDir = {sin(rotX * M_PI_2), sin(rotY * M_PI_2), 0.5f};
	vect_normalize(&lookDir);

	if (stereobuffer)
		glDrawBuffer(GL_BACK_LEFT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	Point3 camLeft = {cam.x + convergence, cam.y, cam.z};
	gluLookAt(	camLeft.x, camLeft.y, camLeft.z,
				camLeft.x + lookDir.x, camLeft.y + lookDir.y, camLeft.z + lookDir.z + 4.0f,
				0.0f, 1.0f, 0.0f);
	if (!stereobuffer)
		glViewport(width / 2, 0, width / 2, height);
	render_scene();

	if (stereobuffer)
	{
		glDrawBuffer(GL_BACK_RIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	glLoadIdentity();
	Point3 camRight = {cam.x - convergence, cam.y, cam.z};
	gluLookAt(	camRight.x, camRight.y, camRight.z,
				camRight.x + lookDir.x, camRight.y + lookDir.y, camRight.z + lookDir.z + 4.0f,
				0.0f, 1.0f, 0.0f);
	if (!stereobuffer)
			glViewport(0, 0, width / 2, height);
	render_scene();

	// Swap the back and front buffer
	glutSwapBuffers();
}

void mouse(int btn, int state, int x, int y)
{
	if (!mouseCaptured)
	{
		if (state == GLUT_DOWN)
		{
			mouseCaptured = true;
			mouseButtonDown = btn;
			mouseDownX = x;
			mouseDownY = y;
			glutSetCursor(GLUT_CURSOR_NONE);
		}

		//if (btn == GLUT_LEFT_BUTTON && state == GLUT_UP)
	}
	else if (state == GLUT_UP)
	{
		mouseCaptured = false;
		glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
	}
}

void mouseMotion(int x, int y)
{
	int dx = x - mouseDownX;
	int dy = y - mouseDownY;

	switch (mouseButtonDown)
	{
	case GLUT_LEFT_BUTTON:
		cam.x -= (float)dx / 10.0f;
		cam.y -= (float)dy / 10.0f;
		cam.x = CLAMP(cam.x, -10.0f + 0.5f, 10.0f - 0.5f);
		cam.y = CLAMP(cam.y, -10.0f + 0.5f, 10.0f - 0.5f);
		break;
	case GLUT_MIDDLE_BUTTON:
		cam.z -= (float)dy / 10.0f;
		cam.z = CLAMP(cam.z, 0.0f, 80.0f);
		break;
	case GLUT_RIGHT_BUTTON:
		rotX += -(float)dx / 30.0f;
		rotY += -(float)dy / 30.0f;
		rotX = CLAMP(rotX, -1.0f, 1.0f);
		rotY = CLAMP(rotY, -1.0f, 1.0f);
		break;
	default:
		return;
	}
	glutPostRedisplay();
	mouseDownX = x;
	mouseDownY = y;
	//glutWarpPointer(mouseDownX, mouseDownY);
}

void mousePassive(int x, int y)
{

}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 13: //Enter
		break;
	case 27: //Escape
		exit(0);
		break;
	case 'f':
		glutFullScreen();
		break;
	case 'a':
		convergence += 0.05;
		printf("Convergence: %.2f\n", convergence);fflush(stdout);
		break;
	case 'd':
		convergence -= 0.05f;
		printf("Convergence: %.2f\n", convergence);fflush(stdout);
		break;
	default:
		return;
	}
	glutPostRedisplay();
}

void
resize(int w, int h)
{
	width = w;
	height = h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (float)width / 2.0f / (float)height, 0.1, 105.0);
	glMatrixMode(GL_MODELVIEW);

	if (stereobuffer)
		glViewport(0, 0, width, height);
}

/**
 * Setup some gl stuff
 */
void gfxinit()
{
	resize(width, height);

	//setup fog
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogf(GL_FOG_DENSITY, 0.9f);
	glFogf(GL_FOG_START, 2.0f);
	glFogf(GL_FOG_END, 105.0f);
	glEnable(GL_FOG);

	glEnable(GL_DEPTH_TEST);

	//set blend function
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	//load shaders
	progTarget = shader_init("shaders/target.vp", "shaders/target.fp", NULL);
}

int main(int argc, char **argv)
{
	// Initialize glut
	glutInit(&argc, argv);

	int i;
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-s"))
		{
			stereobuffer = true;
		}
	}

	width = INITIAL_WIDTH;
	height = INITIAL_HEIGHT;
	if (!stereobuffer)
		width *= 2;
	glutInitWindowSize(width, height);
	glutInitWindowPosition(50, 100);
	//glutInitWindowPosition(1925, 100);
	int mode = GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH;
	if (stereobuffer)
		mode |= GLUT_STEREO;
	glutInitDisplayMode(mode);
	glutCreateWindow("Targets");
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(resize);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	glutPassiveMotionFunc(mousePassive);

	GLenum err;
	err = glewInit();
	if (err != GLEW_OK)
	{
		printf("GLEW error\n");
		exit(1);
	}
	printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	if (glewGetExtension("GL_ARB_fragment_shader") != GL_TRUE
		|| glewGetExtension("GL_ARB_vertex_shader") != GL_TRUE
		|| glewGetExtension("GL_ARB_shader_objects") != GL_TRUE
		|| glewGetExtension("GL_ARB_shading_language_100") != GL_TRUE
		)
	{
		printf("Driver does not support OpenGL Shading Language\n");
		exit(1);
	}


	gfxinit();

	fflush(stdout);

	glutMainLoop();

	return 0;
}

/*
 * shader.c
 *
 *  Created on: Oct 31, 2009
 *      Author: Kevin Wells
 */

#include "shader.h"

#include <GL/glew.h>
#include <GL/gl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// shader reader - creates null terminated string from file
char*
readShaderSource(const char* shaderFile)
{
	if (!shaderFile)
		return NULL;

	struct stat statBuf;
	FILE* fp = fopen(shaderFile, "r");
	if (!fp)
		return NULL;

	char* buf;

	stat(shaderFile, &statBuf);
	buf = (char*)calloc(sizeof(char), statBuf.st_size + 1);
	memset(buf, 0, statBuf.st_size + 1);
	fread(buf, 1, statBuf.st_size, fp);
	fclose(fp);

	return buf;
}

static void
printShaderInfoLog(GLuint shader, int isShader)
{
  int infologLen = 0;
  int charsWritten = 0;
  GLchar *infoLog;

  if (isShader)
	  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);
  else
	  glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &infologLen);
  //printOpenGLError();

  if (infologLen > 0) {
    infoLog = (GLchar *)malloc(infologLen);
    if (isShader)
    	glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog);
    else
    	glGetProgramInfoLog(shader, infologLen, &charsWritten, infoLog);
    printf("%s\n\n", infoLog);
    free(infoLog);
  }
  //printOpenGLError();
}

// error printing function
static int
checkError(GLint shader, GLint status, const char *msg, int isShader)
{
	if (!status)
	{
		printf("%s\n", msg);
		if (shader)
			printShaderInfoLog(shader, isShader);
		return 1;
	}
	return 0;
}

// shader initialization
int
shader_init(const char* vShaderFile, const char* fShaderFile, void (*initCallback)(int program))
{
	GLint program = 0;

	int error = 0;

	GLint vShader = 0;
	GLint fShader = 0;
	GLint status = 0;

	// read shader files
	char* vSource = readShaderSource(vShaderFile);
	char* fSource = readShaderSource(fShaderFile);

	if ((!vSource && vShaderFile) || (!fSource && fShaderFile))
	{
		if (!vSource && vShaderFile)
			printf("Vertex shader file (%s) not found\n", vShaderFile);
		if (!fSource && fShaderFile)
			printf("Fragment shader file (%s) not found\n", fShaderFile);
		return 0;
	}

	// create program and shader objects
	if (vSource)
		vShader = glCreateShader(GL_VERTEX_SHADER);
	if (fSource)
		fShader = glCreateShader(GL_FRAGMENT_SHADER);

	program = glCreateProgram();

	// attach shaders to the program object
	if (vSource)
		glAttachShader(program, vShader);
	if (fSource)
		glAttachShader(program, fShader);

	// read shaders
	if (vSource)
		glShaderSource(vShader, 1, (const char**)&vSource, NULL);
	if (fSource)
		glShaderSource(fShader, 1, (const char**)&fSource, NULL);

	// compile shaders
	if (vSource)
		glCompileShader(vShader);
	if (fSource)
		glCompileShader(fShader);

	// error check
	if (vSource)
	{
		glGetShaderiv(vShader, GL_COMPILE_STATUS, &status);
		error += checkError(vShader, status, "Failed to compile the vertex shader:", 1);
	}
	if (fSource)
	{
		glGetShaderiv(fShader, GL_COMPILE_STATUS, &status);
		error += checkError(fShader, status, "Failed to compile the fragment shader:", 1);
	}

	// link
	if (!error)
	{
		glLinkProgram(program);

		glGetProgramiv(program, GL_LINK_STATUS, &status);
		error += checkError(program, status, "Failed to link the shader program object:", 0);

		if (!error)
		{
			// use program object
			glUseProgram(program);

			// setup uniform parameters
			if (initCallback)
				initCallback(program);

			// turn off shaders
			glUseProgram(0);
		}
	}

	free(vSource);
	free(fSource);

	return error ? 0 : program;
}

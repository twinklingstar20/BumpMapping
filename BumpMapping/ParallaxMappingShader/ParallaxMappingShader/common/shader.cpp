

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <GL/glew.h>

#include "shader.h"

static char* readShaderSource(const char *fileName)
{
	FILE *fp;
	char *content = NULL;
	int count = 0;
	if (!fileName)
		return NULL;
	fp = fopen(fileName, "rt");
	if (!fp)
		return NULL;
	fseek(fp, 0, SEEK_END);
	count = ftell(fp);
	rewind(fp);
	if (count > 0)
	{
		content = (char *)malloc(sizeof(char)* (count + 1));
		count = fread(content, sizeof(char), count, fp);
		content[count] = NULL;
	}
	fclose(fp);

	return content;
}

static GLuint genShader(GLenum type, const char* fileName)
{	
	char* log;
	// create a shader object.
	GLuint shader = glCreateShader(type);
	// read the shader code.
	char* shaderSource = readShaderSource(fileName);
	if (!shaderSource)
		return 0;
	const char* ptrShaderSource = shaderSource;
	// bind the code to the shader object.
	glShaderSource(shader, 1, &ptrShaderSource, NULL);
	free(shaderSource);
	// compile the shader.
	glCompileShader(shader);
	GLint status = 0;
	// query the status of the compiling.
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	GLint length;
	// read the length of the log.
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
	log = (GLchar*)malloc(length);
	// get the log info.
	glGetShaderInfoLog(shader, length, &length, log);
#ifdef _DEBUG
	printf("%s\n", log);
#endif
	free(log);
	if (!status)
	{
		// delete shader.
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

static GLuint linkProgram(GLuint* shader, int shaderNum)
{
	char* log;
	// create a program
	GLuint program = glCreateProgram();
	int i;
	//attach the shaders to the program.
	for (i = 0; i < shaderNum; i++)
		glAttachShader(program, shader[i]);
	//link the shaders.
	glLinkProgram(program);
	GLint status;
	// query the status of the result.
	glGetProgramiv(program, GL_LINK_STATUS, &status);

	GLint length;
	// get the length of the log.
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
	log = (GLchar*)malloc(length);
	// get the log.
	glGetProgramInfoLog(program, length, &length, log);
#ifdef _DEBUG
	printf("%s\n", log);
#endif
	free(log);
	if (!status)
	{
		//delete the program.
		glDeleteProgram(program);
		return 0;
	}
	return program;
}

GLuint GenProgram(const char* vertFile, const char* fragFile)
{

	//create a vertex shader
	GLuint vertexShader = genShader(GL_VERTEX_SHADER, vertFile);
	if (!vertexShader)
		return 0;

	//create a fragment shader.
	GLuint fragmentShader = genShader(GL_FRAGMENT_SHADER, fragFile);
	if (!fragmentShader)
	{
		glDeleteShader(vertexShader);
		return 0;
	}
		

	//Link the vertex shader and fragment shader.
	GLuint shader[2] = { vertexShader, fragmentShader };
	GLuint program = linkProgram(shader, 2);
	if (!program)
	{
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return 0;
	}
	return program;
}
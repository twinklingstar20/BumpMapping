/************************************************************************
\link	www.twinklingstar.cn
\author Twinkling Star
\date	2013/12/03
****************************************************************************/
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glut32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glaux.lib")
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <gl/glaux.h>
#include <vector>
#include <glm/glm.hpp>
#include "common/shader.h"
#include "common/objloader.hpp"
#include "common/vboindexer.hpp"


void computeTangentBasis(
	// inputs
	std::vector<glm::vec3> & vertices,
	std::vector<glm::vec2> & uvs,
	std::vector<glm::vec3> & normals,
	// outputs
	std::vector<glm::vec3> & tangents,
	std::vector<glm::vec3> & bitangents)
{

	for (unsigned int i = 0; i<vertices.size(); i += 3)
	{

		// Shortcuts for vertices
		glm::vec3 & v0 = vertices[i + 0];
		glm::vec3 & v1 = vertices[i + 1];
		glm::vec3 & v2 = vertices[i + 2];

		// Shortcuts for UVs
		glm::vec2 & uv0 = uvs[i + 0];
		glm::vec2 & uv1 = uvs[i + 1];
		glm::vec2 & uv2 = uvs[i + 2];

		// Edges of the triangle : postion delta
		glm::vec3 deltaPos1 = v1 - v0;
		glm::vec3 deltaPos2 = v2 - v0;

		// UV delta
		glm::vec2 deltaUV1 = uv1 - uv0;
		glm::vec2 deltaUV2 = uv2 - uv0;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
		glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;

		// Set the same tangent for all three vertices of the triangle.
		// They will be merged later, in vboindexer.cpp
		tangents.push_back(tangent);
		tangents.push_back(tangent);
		tangents.push_back(tangent);

		// Same thing for binormals
		bitangents.push_back(bitangent);
		bitangents.push_back(bitangent);
		bitangents.push_back(bitangent);

	}

	// See "Going Further"
	for (unsigned int i = 0; i<vertices.size(); i += 1)
	{
		glm::vec3 & n = normals[i];
		glm::vec3 & t = tangents[i];
		glm::vec3 & b = bitangents[i];

		// Gram-Schmidt orthogonalize
		t = glm::normalize(t - n * glm::dot(n, t));

		// Calculate handedness
		if (glm::dot(glm::cross(n, t), b) < 0.0f)
		{
			t = t * -1.0f;
		}
		b = glm::normalize(glm::cross(n, t));
	}
}

GLuint genNormalTexture(const char* filename)
{
	AUX_RGBImageRec *ptrImage = NULL;
	GLuint textureID = 0;
	if (ptrImage = auxDIBImageLoad(filename))
	{
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ptrImage->sizeX, ptrImage->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, ptrImage->data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	if (ptrImage)
	{
		if (ptrImage->data)
			delete ptrImage->data;
		delete ptrImage;
		ptrImage = NULL;
	}

	return textureID;
}

GLuint	gProgramId = 0;
GLuint	gShaderMVPMat;
GLuint	gShaderMVMat3x3;
GLuint	gShaderViewMat;
GLuint	gShaderModelMat;

GLuint	gShaderDiffTex;
GLuint	gShaderNormTex;


GLuint	gShaderLight;

GLuint	gDiffTex;
GLuint	gNormTex;
GLuint	gSpecTex;


glm::mat4 gProjMat;
glm::mat4 gViewMat;
glm::mat4 gModelMat;
glm::mat4 gMVMat;
glm::mat3 gMVMat3x3;
glm::mat4 gMVPMat;


GLuint gVertexBuffer;
GLuint gUVBuffer;
GLuint gNormBuffer;
GLuint gTangentBuffer;
GLuint gBitangentBuffer;
GLuint gElementBuffer;

std::vector<unsigned short> indices;

float gData[] = {
	//顶点坐标， 纹理坐标，法向量
	0.0f, 0.0f, 0.0f,	0.0f, 0.0f,	 0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f,	1.0f, 0.0f,	 0.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f,	0.0f, 1.0f,  0.0f, 0.0f, 1.0f,

	1.0f, 0.0f, 0.0f,	1.0f, 0.0f,	 0.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 0.0f,	1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f,	0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
};

void init()
{
	// Create and compile our GLSL program from the shaders
	gProgramId = GenProgram("NormalMapping.vert", "NormalMapping.frag");

	// Get a handle for our "ModelViewProj" uniform
	gShaderMVPMat = glGetUniformLocation(gProgramId, "ModelViewProj");
	gShaderViewMat = glGetUniformLocation(gProgramId, "View");
	gShaderModelMat = glGetUniformLocation(gProgramId, "Model");
	gShaderMVMat3x3 = glGetUniformLocation(gProgramId, "MV3x3");

	// Load the texture
	gDiffTex = genNormalTexture("circle_diff.bmp");
	gNormTex = genNormalTexture("circle_norm.bmp");

	// Get a handle for our "myTextureSampler" uniform
	gShaderDiffTex = glGetUniformLocation(gProgramId, "DiffTexSampler");
	gShaderNormTex = glGetUniformLocation(gProgramId, "NormTexSampler");

	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	int i;
	for (i = 0; i < 6; i++)
	{
		float *bride = gData + i * 8;
		vertices.push_back(glm::vec3(*(bride + 0), *(bride + 1), *(bride + 2)));
		uvs.push_back(glm::vec2(*(bride + 3), *(bride + 4)));
		normals.push_back(glm::vec3(*(bride + 5), *(bride + 6), *(bride + 7)));
	}

	std::vector<glm::vec3> tangents;
	std::vector<glm::vec3> bitangents;
	computeTangentBasis(vertices, uvs, normals, tangents, bitangents);

	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	std::vector<glm::vec3> indexed_tangents;
	std::vector<glm::vec3> indexed_bitangents;
	indexVBO_TBN(
		vertices, uvs, normals, tangents, bitangents,
		indices, indexed_vertices, indexed_uvs, indexed_normals, indexed_tangents, indexed_bitangents);

	// Load it into a VBO
	glGenBuffers(1, &gVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &gUVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, gUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &gNormBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, gNormBuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &gTangentBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, gTangentBuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_tangents.size() * sizeof(glm::vec3), &indexed_tangents[0], GL_STATIC_DRAW);

	glGenBuffers(1, &gBitangentBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, gBitangentBuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_bitangents.size() * sizeof(glm::vec3), &indexed_bitangents[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well
	glGenBuffers(1, &gElementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gElementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
}


void changeSize(int w, int h)
{
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)
		h = 1;

	float ratio = 1.0* w / h;

	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(45, ratio, 1, 1000);

	glGetFloatv(GL_PROJECTION_MATRIX, &gProjMat[0][0]);

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 0.0);
}

void renderScene(void)
{
	glMatrixMode(GL_MODELVIEW);
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	glTranslatef(-0.5f, -0.5f, -1.5f);
	gViewMat = glm::mat4(1.0f);
	glGetFloatv(GL_MODELVIEW_MATRIX, &gModelMat[0][0]);
	glGetFloatv(GL_MODELVIEW_MATRIX, &gMVMat[0][0]);
	gMVMat3x3 = glm::mat3(gMVMat);
	gMVPMat = gProjMat * gViewMat * gModelMat;

	// Use our shader
	glUseProgram(gProgramId);

	// Send our transformation to the currently bound shader, 
	glUniformMatrix4fv(gShaderMVPMat, 1, GL_FALSE, &gMVPMat[0][0]);
	glUniformMatrix4fv(gShaderModelMat, 1, GL_FALSE, &gModelMat[0][0]);
	glUniformMatrix4fv(gShaderViewMat, 1, GL_FALSE, &gViewMat[0][0]);
	glUniformMatrix3fv(gShaderMVMat3x3, 1, GL_FALSE, &gMVMat3x3[0][0]);

	glm::vec3 lightPos = glm::vec3(0, 0, 4);
	// Get a handle for our "LightPosition" uniform
	gShaderLight = glGetUniformLocation(gProgramId, "LightPos");
	glUniform3f(gShaderLight, lightPos.x, lightPos.y, lightPos.z);

	// Bind our diffuse texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gDiffTex);
	// Set our "DiffuseTextureSampler" sampler to user Texture Unit 0
	glUniform1i(gShaderDiffTex, 0);

	// Bind our normal texture in Texture Unit 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormTex);
	// Set our "Normal	TextureSampler" sampler to user Texture Unit 0
	glUniform1i(gShaderNormTex, 1);


	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
	glVertexAttribPointer(
		0,                  // attribute
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, gUVBuffer);
	glVertexAttribPointer(
		1,                                // attribute
		2,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	// 3rd attribute buffer : normals
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, gNormBuffer);
	glVertexAttribPointer(
		2,                                // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	// 4th attribute buffer : tangents
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, gTangentBuffer);
	glVertexAttribPointer(
		3,                                // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	// 5th attribute buffer : bitangents
	glEnableVertexAttribArray(4);
	glBindBuffer(GL_ARRAY_BUFFER, gBitangentBuffer);
	glVertexAttribPointer(
		4,                                // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	// Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gElementBuffer);

	// Draw the triangles !
	glDrawElements(
		GL_TRIANGLES,      // mode
		indices.size(),    // count
		GL_UNSIGNED_SHORT, // type
		(void*)0           // element array buffer offset
		);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(4);

	glutSwapBuffers();
}

void normalKeys(unsigned char key, int x, int y)
{
	if (key == 27)
		exit(0);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(320, 320);
	glutCreateWindow("Normal Texture Demo");

	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	glutKeyboardFunc(normalKeys);

	if (glewInit() != GLEW_OK)
	{
		printf("No glew supports!\n");
		exit(0);
	}
	//判断是否支持GLSL
	if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader)
	{
		printf("Ready for GLSL\n");
	}
	else
	{
		printf("No GLSL supports\n");
		exit(1);
	}

	init();

	//GLuint program = GenProgram("toon.vert", "toon.frag");
	//glUseProgram(program);

	glutMainLoop();

	return 0;
}


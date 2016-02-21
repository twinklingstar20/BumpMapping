/************************************************************************       
\link   www.twinklingstar.cn
\author Twinkling Star
\date   2015/09/27
\file   main.cpp
****************************************************************************/
#pragma comment(lib, "glaux.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glut32.lib")
#pragma comment(lib, "glew32.lib")

#include <gl/glew.h>
#include <gl/glut.h> 
#include <gl/glaux.h>
#include <stdio.h>
#include <math.h>

//Normal map
GLuint gNormalMap;

//Decal texture
GLuint gDecalTexture;

//Normalisation cube map
GLuint gNormCubeMap;

GLfloat	gXrot = 0;
GLfloat	gYrot = 0;
GLfloat gXspeed = 0;
GLfloat gYspeed = 0;
GLfloat	gZ = -5.0f;

bool gDrawBumps = true;
bool gDrawColor = true;

GLfloat gLightAmbient[]	= { 0.2f, 0.2f, 0.2f};
GLfloat gLightDiffuse[]	= { 1.0f, 1.0f, 1.0f};
GLfloat gLightPosition[] = { 10.0f, 10.0f, 10.0f};

// Data Contains The Faces For The Cube In Format 2xTexCoord, 3xVertex;
// Note That The Tesselation Of The Cube Is Only Absolute Minimum.
GLfloat gData[]= {
	// FRONT FACE
	0.0f, 0.0f,		-1.0f, -1.0f, +1.0f,
	1.0f, 0.0f,		+1.0f, -1.0f, +1.0f,
	1.0f, 1.0f,		+1.0f, +1.0f, +1.0f,
	0.0f, 1.0f,		-1.0f, +1.0f, +1.0f,
	// BACK FACE
	1.0f, 0.0f,		-1.0f, -1.0f, -1.0f,
	1.0f, 1.0f,		-1.0f, +1.0f, -1.0f,
	0.0f, 1.0f,		+1.0f, +1.0f, -1.0f,
	0.0f, 0.0f,		+1.0f, -1.0f, -1.0f,
	// Top Face
	0.0f, 1.0f,		-1.0f, +1.0f, -1.0f,
	0.0f, 0.0f,		-1.0f, +1.0f, +1.0f,
	1.0f, 0.0f,		+1.0f, +1.0f, +1.0f,
	1.0f, 1.0f,		+1.0f, +1.0f, -1.0f,
	// Bottom Face
	1.0f, 1.0f,		-1.0f, -1.0f, -1.0f,
	0.0f, 1.0f,		+1.0f, -1.0f, -1.0f,
	0.0f, 0.0f,		+1.0f, -1.0f, +1.0f,
	1.0f, 0.0f,		-1.0f, -1.0f, +1.0f,
	// Right Face
	1.0f, 0.0f,		+1.0f, -1.0f, -1.0f,
	1.0f, 1.0f,		+1.0f, +1.0f, -1.0f,
	0.0f, 1.0f,		+1.0f, +1.0f, +1.0f,
	0.0f, 0.0f,		+1.0f, -1.0f, +1.0f,
	// Left Face
	0.0f, 0.0f,		-1.0f, -1.0f, -1.0f,
	1.0f, 0.0f,		-1.0f, -1.0f,  1.0f,
	1.0f, 1.0f,		-1.0f,  1.0f,  1.0f,
	0.0f, 1.0f,		-1.0f,  1.0f, -1.0f
};

bool isExtSupported( char* szTargetExtension )
{
	const unsigned char *pszExtensions = NULL;
	const unsigned char *pszStart;
	unsigned char *pszWhere, *pszTerminator;

	// Extension names should not have spaces
	pszWhere = (unsigned char *) strchr( szTargetExtension, ' ' );
	if( pszWhere || *szTargetExtension == '\0' )
		return false;

	// Get Extensions String
	pszExtensions = glGetString(GL_EXTENSIONS);

	// Search The Extensions String For An Exact Copy
	pszStart = pszExtensions;
	for(;;)
	{
		pszWhere = (unsigned char *) strstr( (const char *) pszStart, szTargetExtension );
		if( !pszWhere )
			break;
		pszTerminator = pszWhere + strlen( szTargetExtension );
		if( pszWhere == pszStart || *( pszWhere - 1 ) == ' ' )
			if( *pszTerminator == ' ' || *pszTerminator == '\0' )
				return true;
		pszStart = pszTerminator;
	}
	return false;
}

bool checkExts()
{
	if(!isExtSupported("GL_ARB_multitexture"))
	{
		printf("ARB_multitexture is not supported!\n");
		return false;
	}
	if(!isExtSupported("GL_ARB_texture_cube_map"))
	{
		printf("GL_ARB_texture_cube_map is not supported!\n");
		return false;
	}
	if(!isExtSupported("GL_ARB_texture_env_combine"))
	{
		printf("GL_ARB_texture_env_combine is not supported!\n");
		return false;
	}
	if(!isExtSupported("GL_ARB_texture_env_dot3"))
	{
		printf("GL_ARB_texture_env_dot3 is not supported!\n");
		return false;
	}
	return true;
}

class Vector3D
{
public:
	float x, y, z;

public:
	inline float magnitude() const
	{
		return sqrt(x * x + y * y + z * z);
	}

	inline float normalize()
	{
		float m = magnitude();
		if (m)
		{
			const float il =  float(1.0) / m;
			x *= il;
			y *= il;
			z *= il;
		}
		return m;
	}
};

bool genNormCubeMap()
{
	unsigned char * data=new unsigned char[32*32*3];
	if(!data)
	{
		printf("Unable to allocate memory for texture data for cube map\n");
		return false;
	}

	glGenTextures(1, &gNormCubeMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, gNormCubeMap);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	//some useful variables
	int size = 32;
	float offset = 0;//0.5f;
	float halfSize = 16.0f;
	Vector3D tmp;
	unsigned char * bytePtr;

	//positive x
	bytePtr = data;
	for(int j=0; j<size; j++)
	{
		for(int i=0; i<size; i++)
		{
			
			tmp.x = halfSize;
			tmp.y = -(j + offset - halfSize);
			tmp.z = -(i + offset - halfSize);

			tmp.normalize();
			
			tmp.x = tmp.x * 0.5f + 0.5f;
			tmp.y = tmp.y * 0.5f + 0.5f;
			tmp.z = tmp.z * 0.5f + 0.5f;

			bytePtr[0]=(unsigned char)(tmp.x * 255);
			bytePtr[1]=(unsigned char)(tmp.y * 255);
			bytePtr[2]=(unsigned char)(tmp.z * 255);

			bytePtr += 3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	//negative x
	bytePtr = data;
	for(int j=0; j<size; j++)
	{
		for(int i=0; i<size; i++)
		{
			tmp.x = -halfSize;
			tmp.y = -(j + offset - halfSize);
			tmp.z =  (i + offset - halfSize);

			tmp.normalize();

			tmp.x = tmp.x * 0.5f + 0.5f;
			tmp.y = tmp.y * 0.5f + 0.5f;
			tmp.z = tmp.z * 0.5f + 0.5f;

			bytePtr[0]=(unsigned char)(tmp.x * 255);
			bytePtr[1]=(unsigned char)(tmp.y * 255);
			bytePtr[2]=(unsigned char)(tmp.z * 255);

			bytePtr+=3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	//positive y
	bytePtr=data;
	for(int j=0; j<size; j++)
	{
		for(int i=0; i<size; i++)
		{
			tmp.x = i + offset - halfSize;
			tmp.y = halfSize;
			tmp.z = j + offset - halfSize;

			tmp.normalize();

			tmp.x = tmp.x * 0.5f + 0.5f;
			tmp.y = tmp.y * 0.5f + 0.5f;
			tmp.z = tmp.z * 0.5f + 0.5f;

			bytePtr[0]=(unsigned char)(tmp.x * 255);
			bytePtr[1]=(unsigned char)(tmp.y * 255);
			bytePtr[2]=(unsigned char)(tmp.z * 255);

			bytePtr+=3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	//negative y
	bytePtr = data;
	for(int j = 0; j<size; j++)
	{
		for(int i=0; i<size; i++)
		{
			tmp.x = i + offset - halfSize;
			tmp.y = -halfSize;
			tmp.z = -(j + offset - halfSize);

			tmp.normalize();

			tmp.x = tmp.x * 0.5f + 0.5f;
			tmp.y = tmp.y * 0.5f + 0.5f;
			tmp.z = tmp.z * 0.5f + 0.5f;

			bytePtr[0] = (unsigned char)(tmp.x * 255);
			bytePtr[1] = (unsigned char)(tmp.y * 255);
			bytePtr[2] = (unsigned char)(tmp.z * 255);

			bytePtr+=3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	//positive z
	bytePtr=data;

	for(int j=0; j<size; j++)
	{
		for(int i=0; i<size; i++)
		{
			tmp.x = i + offset - halfSize;
			tmp.y = -(j + offset - halfSize);
			tmp.z = halfSize;

			tmp.normalize();

			tmp.x = tmp.x * 0.5f + 0.5f;
			tmp.y = tmp.y * 0.5f + 0.5f;
			tmp.z = tmp.z * 0.5f + 0.5f;

			bytePtr[0]=(unsigned char)(tmp.x * 255);
			bytePtr[1]=(unsigned char)(tmp.y * 255);
			bytePtr[2]=(unsigned char)(tmp.z * 255);

			bytePtr+=3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	//negative z
	bytePtr = data;
	for(int j=0; j<size; j++)
	{
		for(int i=0; i<size; i++)
		{
			tmp.x = -(i + offset - halfSize);
			tmp.y = -(j + offset - halfSize);
			tmp.z = -halfSize;

			tmp.normalize();

			tmp.x = tmp.x * 0.5f + 0.5f;
			tmp.y = tmp.y * 0.5f + 0.5f;
			tmp.z = tmp.z * 0.5f + 0.5f;

			bytePtr[0]=(unsigned char)(tmp.x * 255);
			bytePtr[1]=(unsigned char)(tmp.y * 255);
			bytePtr[2]=(unsigned char)(tmp.z * 255);

			bytePtr+=3;
		}
	}
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA8, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	delete [] data;

	return true;
}

bool genNormalTexture()
{
	bool status = true;
	AUX_RGBImageRec *ptrImage = NULL;

	if (ptrImage = auxDIBImageLoad("Data/NormalMap.bmp")) 
	{											
		glGenTextures(1, &gNormalMap);
		glBindTexture(GL_TEXTURE_2D, gNormalMap);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ptrImage->sizeX, ptrImage->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, ptrImage->data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else 
	{
		status = false;
	}
	if (ptrImage) 
	{
		if (ptrImage->data) 
			delete ptrImage->data;
		delete ptrImage;
		ptrImage = NULL;
	}

	return status;
}

bool genDecalTexture()
{
	bool status = true;
	AUX_RGBImageRec *ptrImage = NULL;

	if (ptrImage = auxDIBImageLoad("Data/decal.bmp")) 
	{											
		glGenTextures(1, &gDecalTexture);
		glBindTexture(GL_TEXTURE_2D, gDecalTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ptrImage->sizeX, ptrImage->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, ptrImage->data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else 
	{
		status = false;
	}
	if (ptrImage) 
	{
		if (ptrImage->data) 
			delete ptrImage->data;
		delete ptrImage;
		ptrImage = NULL;
	}

	return status;
}

bool genTextures()
{
	if(!genNormalTexture() || !genDecalTexture() || !genNormCubeMap())
		return false;

	return true;
}

bool initGL()
{
	if(!checkExts())
		return false;

	//Shading states
	glShadeModel(GL_SMOOTH);
	glClearColor(0.2f, 0.4f, 0.2f, 0.0f);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	//Depth states
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	if(!genTextures())
	{
		printf("Generate texture fails!\n");
		exit(0);
	}

	//Initialize the lights.
	//glLightfv( GL_LIGHT1, GL_AMBIENT, gLightAmbient);
	//glLightfv( GL_LIGHT1, GL_DIFFUSE, gLightDiffuse);	
	//glLightfv( GL_LIGHT1, GL_POSITION, gLightPosition);

	//glEnable(GL_LIGHT1);

	
	return true;
}

void matMultVector(GLfloat *M, GLfloat *v) 
{// Calculates v=vM, M Is 4x4 In Column-Major, v Is 4dim. Row (i.e. "Transposed")
	GLfloat res[3];
	res[0] = M[ 0]*v[0] + M[ 1] * v[1] + M[ 2] * v[2] + M[ 3] * v[3];
	res[1] = M[ 4]*v[0] + M[ 5] * v[1] + M[ 6] * v[2] + M[ 7] * v[3];
	res[2] = M[ 8]*v[0] + M[ 9] * v[1] + M[10] * v[2] + M[11] * v[3];

	v[0] = res[0];
	v[1] = res[1];
	v[2] = res[2];
	v[3] = M[15];											// Homogenous Coordinate
}


void setUpBumps(GLfloat *s, GLfloat *t, GLfloat *n, GLfloat *l, GLfloat *c) 
{
	GLfloat v[3];							// Vertex From Current Position To Light			
	// Calculate v From Current Vector c To Lightposition And Normalize v	
	v[0] = l[0] - c[0];		
	v[1] = l[1] - c[1];		
	v[2] = l[2] - c[2];

	c[0] = v[0] * s[0] + v[1] * s[1] + v[2] * s[2];
	c[1] = v[0] * t[0] + v[1] * t[1] + v[2] * t[2];
	c[2] = v[0] * n[0] + v[1] * n[1] + v[2] * n[2];
}


void doCube (void) 
{
	int i;
	glBegin(GL_QUADS);
	// Front Face
	glNormal3f(0.0f, 0.0f, +1.0f);
	for (i = 0; i < 4; i++) 
	{
		glTexCoord2f(gData[5*i], gData[5*i+1]);
		glVertex3f(gData[5*i+2], gData[5*i+3], gData[5*i+4]);
	}
	// Back Face
	glNormal3f( 0.0f, 0.0f,-1.0f);
	for (i = 4; i < 8; i++) 
	{
		glTexCoord2f(gData[5*i], gData[5*i+1]);
		glVertex3f(gData[5*i+2], gData[5*i+3], gData[5*i+4]);
	}
	// Top Face
	glNormal3f(0.0f, 1.0f, 0.0f);
	for (i = 8; i < 12; i++) 
	{
		glTexCoord2f(gData[5*i], gData[5*i+1]);
		glVertex3f(gData[5*i+2], gData[5*i+3], gData[5*i+4]);
	}
	// Bottom Face
	glNormal3f(0.0f,-1.0f, 0.0f);
	for (i = 12; i < 16; i++) 
	{
		glTexCoord2f(gData[5*i], gData[5*i+1]);
		glVertex3f(gData[5*i+2], gData[5*i+3], gData[5*i+4]);
	}
	// Right face
	glNormal3f(1.0f, 0.0f, 0.0f);
	for (i = 16; i < 20; i++) 
	{
		glTexCoord2f(gData[5*i], gData[5*i+1]);
		glVertex3f(gData[5*i+2], gData[5*i+3], gData[5*i+4]);
	}
	// Left Face
	glNormal3f(-1.0f, 0.0f, 0.0f);
	for (i = 20; i < 24; i++) 
	{
		glTexCoord2f(gData[5*i], gData[5*i+1]);
		glVertex3f(gData[5*i+2], gData[5*i+3], gData[5*i+4]);
	}
	glEnd();	
}


void draw(void)
{
	//set the modelview matrix stack. GL_MODELVIEW
	GLfloat invMat[16];	

	glMatrixMode(GL_MODELVIEW);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	
	// Get the inverse of the model-view matrix.
	glLoadIdentity();
	glRotatef(-gYrot, 0.0f, 1.0f, 0.0f);
	glRotatef(-gXrot, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, 0.0f, -gZ);
	glGetFloatv(GL_MODELVIEW_MATRIX,invMat);
	
	// Set the model-view matrix.
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, gZ);

	glRotatef(gXrot, 1.0f, 0.0f, 0.0f);
	glRotatef(gYrot, 0.0f, 1.0f, 0.0f);
	
	if(gDrawBumps)
	{
		glEnable(GL_TEXTURE_2D);

		//Bind normal map to texture unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gNormalMap);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);

		//Bind normalisation cube map to texture unit 1
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, gNormCubeMap);
		glEnable(GL_TEXTURE_CUBE_MAP);

		//Set up texture environment to do (tex0 dot tex1)*color
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);/**/
		
		GLfloat c[4] = {0.0f, 0.0f, 0.0f, 1.0f};
		GLfloat n[4] = {0.0f, 0.0f, 0.0f, 1.0f};					// Normalized Normal Of Current Surface		
		GLfloat s[4] = {0.0f, 0.0f, 0.0f, 1.0f};					// s-Texture Coordinate Direction, Normalized
		GLfloat t[4] = {0.0f, 0.0f, 0.0f, 1.0f};					// t-Texture Coordinate Direction, Normalized
		GLfloat l[4];												// Holds Our Lightposition To Be Transformed Into Object Space
		int i;

		l[0] = gLightPosition[0];
		l[1] = gLightPosition[1];
		l[2] = gLightPosition[2];
		l[3] = 1.0f;
		matMultVector(invMat, l);
		l[0] /= l[3];
		l[1] /= l[3];
		l[2] /= l[3];
		
		glBegin(GL_QUADS);	
		// Front Face	
		s[0] = 1.0f;		s[1] = 0.0f;		s[2] = 0.0f;
		t[0] = 0.0f;		t[1] = 1.0f;		t[2] = 0.0f;
		n[0] = 0.0f;		n[1] = 0.0f;		n[2] = 1.0f;
		for (i = 0; i < 4; i++) 
		{
			c[0] = gData[5 * i + 2];		
			c[1] = gData[5 * i + 3];
			c[2] = gData[5 * i + 4];
			setUpBumps(s, t, n, l, c);
			glMultiTexCoord2f(GL_TEXTURE0,gData[5*i], gData[5*i+1]);
			glMultiTexCoord3f(GL_TEXTURE1,c[0], c[1], c[2]);
			glVertex3f(gData[5*i+2], gData[5*i+3], gData[5*i+4]);
		}
		// Back Face
		s[0] = -1.0f;		s[1] = 0.0f;		s[2] = 0.0f;
		t[0] = 0.0f;		t[1] = 1.0f;		t[2] = 0.0f;
		n[0] = 0.0f;		n[1] = 0.0f;		n[2] = -1.0f;	
		for (i = 4; i < 8; i++) 
		{
			c[0] = gData[5*i+2];		
			c[1] = gData[5*i+3];
			c[2] = gData[5*i+4];
			setUpBumps(s, t, n, l, c);
			glMultiTexCoord2f(GL_TEXTURE0,gData[5*i], gData[5*i+1]);
			glMultiTexCoord3f(GL_TEXTURE1,c[0], c[1], c[2]);
			glVertex3f(gData[5*i+2], gData[5*i+3], gData[5*i+4]);
		}
		// Top Face	
		s[0] = 1.0f;		s[1] = 0.0f;		s[2] = 0.0f;
		t[0] = 0.0f;		t[1] = 1.0f;		t[2] = 0.0f;
		n[0] = 0.0f;		n[1] = 0.0f;		n[2] = 1.0f;
		for (i = 8; i < 12; i++) 
		{	
			c[0] = gData[5*i+2];		
			c[1] = gData[5*i+3];
			c[2] = gData[5*i+4];
			setUpBumps(s, t, n, l, c);
			glMultiTexCoord2f(GL_TEXTURE0,gData[5*i], gData[5*i+1]);
			glMultiTexCoord3f(GL_TEXTURE1,c[0], c[1], c[2]);
			glVertex3f(gData[5*i+2], gData[5*i+3], gData[5*i+4]);
		}
		// Bottom Face
		s[0] = -1.0f;		s[1] = 0.0f;		s[2] = 0.0f;
		t[0] = 0.0f;		t[1] = 1.0f;		t[2] = 0.0f;
		n[0] = 0.0f;		n[1] = 0.0f;		n[2] = -1.0f;	
		for (i = 12; i < 16; i++) 
		{	
			c[0] = gData[5*i+2];		
			c[1] = gData[5*i+3];
			c[2] = gData[5*i+4];
			setUpBumps(s, t, n, l, c);
			glMultiTexCoord2f(GL_TEXTURE0,gData[5*i], gData[5*i+1]);
			glMultiTexCoord3f(GL_TEXTURE1,c[0], c[1], c[2]);
			glVertex3f(gData[5*i+2], gData[5*i+3], gData[5*i+4]);
		}

		// Right Face	
		s[0] = 1.0f;		s[1] = 0.0f;		s[2] = 0.0f;
		t[0] = 0.0f;		t[1] = 1.0f;		t[2] = 0.0f;
		n[0] = 0.0f;		n[1] = 0.0f;		n[2] = 1.0f;
		for (i = 16; i < 20; i++) 
		{	
			c[0] = gData[5*i+2];		
			c[1] = gData[5*i+3];
			c[2] = gData[5*i+4];
			setUpBumps(s, t, n, l, c);
			glMultiTexCoord2f(GL_TEXTURE0,gData[5*i], gData[5*i+1]);
			glMultiTexCoord3f(GL_TEXTURE1,c[0], c[1], c[2]);
			glVertex3f(gData[5*i+2], gData[5*i+3], gData[5*i+4]);
		}
		// Left Face
		s[0] = -1.0f;		s[1] = 0.0f;		s[2] = 0.0f;
		t[0] = 0.0f;		t[1] = 1.0f;		t[2] = 0.0f;
		n[0] = 0.0f;		n[1] = 0.0f;		n[2] = -1.0f;
		for (i = 20; i<24; i++) 
		{	
			c[0] = gData[5*i+2];		
			c[1] = gData[5*i+3];
			c[2] = gData[5*i+4];
			setUpBumps(s, t, n, l, c);
			glMultiTexCoord2f(GL_TEXTURE0,gData[5*i], gData[5*i+1]);
			glMultiTexCoord3f(GL_TEXTURE1,c[0], c[1], c[2]);
			glVertex3f(gData[5*i+2], gData[5*i+3], gData[5*i+4]);
		}
		glEnd();

		//Disable textures
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_CUBE_MAP);

		glActiveTexture(GL_TEXTURE1);
		glActiveTexture(GL_TEXTURE0);

		//Return to standard modulate texenv
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
	//If we are drawing both passes, enable blending to multiply them together
	/*if(gDrawBumps && gDrawColor)
	{
		//Enable multiplicative blending
		glBlendFunc(GL_DST_COLOR, GL_ZERO);
		glEnable(GL_BLEND);
	}

	if(gDrawColor)
	{
		glEnable(GL_TEXTURE_2D);
		//Bind decal texture
		glBindTexture(GL_TEXTURE_2D, gDecalTexture);
		glEnable(GL_TEXTURE_2D);

		doCube();

		//Disable texture
		glDisable(GL_TEXTURE_2D);
	}*/
	glutPostRedisplay();

	gXrot += gXspeed;
	gYrot += gYspeed;
	if (gXrot > 360.0f) 
		gXrot -= 360.0f;
	if (gXrot < 0.0f) 
		gXrot += 360.0f;
	if (gYrot > 360.0f) 
		gYrot -= 360.0f;
	if (gYrot < 0.0f) 
		gYrot += 360.0f;
}

void reshape(GLsizei w,GLsizei h)
{
	//set viewport
	glViewport(0,0,w,h);
	//Set the projection stack, GL_PROJECTION
	glMatrixMode(GL_PROJECTION);
	//set the unit matrix.
	glLoadIdentity();
	gluPerspective(45, w / h, 1.0f, 100.0f);
}

void keyboard(unsigned char key, int x, int y)
{
	//If escape is pressed, exit
	if(key == 27)
	{
		exit(0);
	}		
	else if (key == 'Z' || key == 'z')
	{
		gZ -= 0.02f;
	}
	else if (key == 'X' || key == 'x')
	{
		gZ += 0.02f;
	}
	else if (key == 'W' || key == 'w')
	{
		gXspeed -= 0.01f;
	}
	else if (key == 'S' || key == 's')
	{
		gXspeed += 0.01f;
	}
	else if (key == 'A' || key == 'a')
	{
		gYspeed += 0.01f;
	}
	else if (key == 'D' || key == 'd')
	{
		gYspeed -= 0.01f;
	}
}

int main(int argc,char ** argv)
{
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(400,400);
	glutCreateWindow("Normal Bump Mapping");
	
	glewInit();
	if(!initGL())
	{
		printf("Required Extension Unsupported\n");
		exit(0);
	}
	
	glutReshapeFunc(reshape);
	glutDisplayFunc(draw);
	glutKeyboardFunc(keyboard);

	glutMainLoop();
	return(0);
}
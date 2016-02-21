/************************************************************************       
\link   www.twinklingstar.cn
\author TwinklingStar
\date   2015/09/27
\file   main.cpp
****************************************************************************/
#pragma comment(lib, "glaux.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glut32.lib")

#include <gl\glut.h>
#include <gl\glaux.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAX_EMBOSS ((GLfloat)0.008f)		// Maximum Emboss-Translate. Increase To Get Higher Immersion

bool	gEmboss = false;												// Emboss Only, No Basetexture?
bool    gBumps = true;													// Do Bumpmapping?

GLfloat	gXrot;
GLfloat	gYrot;
GLfloat gXspeed;
GLfloat gYspeed;
GLfloat	gZ = -5.0f;

GLuint	gFilter = 1;
GLuint	gTexture[3];
GLuint  gBump[3];
GLuint  gInvbump[3];

GLfloat gLightAmbient[]	= { 0.2f, 0.2f, 0.2f};
GLfloat gLightDiffuse[]	= { 1.0f, 1.0f, 1.0f};
GLfloat gLightPosition[] = { 0.0f, 0.0f, 2.0f};

GLfloat gGray[]= {0.5f,0.5f,0.5f,1.0f};

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


int loadTextures()
{
	bool status = true;
	AUX_RGBImageRec *Image=NULL;
	char *alpha = NULL;

	// Load The Tile-Bitmap For Base-Texture
	if (Image = auxDIBImageLoad("Data/Base.bmp")) 
	{											
		glGenTextures(3, gTexture);

		// Create Nearest Filtered Texture
		glBindTexture(GL_TEXTURE_2D, gTexture[0]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image->sizeX, Image->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Image->data);
		//                             ========
		// Use GL_RGB8 Instead Of "3" In glTexImage2D. Also Defined By GL: GL_RGBA8 Etc.
		// NEW: Now Creating GL_RGBA8 Textures, Alpha Is 1.0f Where Not Specified By Format.

		// Create Linear Filtered Texture
		glBindTexture(GL_TEXTURE_2D, gTexture[1]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image->sizeX, Image->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Image->data);

		// Create MipMapped Texture
		glBindTexture(GL_TEXTURE_2D, gTexture[2]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, Image->sizeX, Image->sizeY, GL_RGB, GL_UNSIGNED_BYTE, Image->data);
	}
	else 
	{
		status = false;
	}
	if (Image) 
	{
		if (Image->data) 
			delete Image->data;
		delete Image;
		Image = NULL;
	}

	// Load The Bumpmaps
	if (Image = auxDIBImageLoad("Data/Bump.bmp")) 
	{			
		glPixelTransferf(GL_RED_SCALE,0.5f);		
		glPixelTransferf(GL_GREEN_SCALE,0.5f);
		glPixelTransferf(GL_BLUE_SCALE,0.5f);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR, gGray);

		glGenTextures(3, gBump);

		// Create Nearest Filtered Texture
		glBindTexture(GL_TEXTURE_2D, gBump[0]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image->sizeX, Image->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Image->data);

		// Create Linear Filtered Texture
		glBindTexture(GL_TEXTURE_2D, gBump[1]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image->sizeX, Image->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Image->data);

		// Create MipMapped Texture
		glBindTexture(GL_TEXTURE_2D, gBump[2]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, Image->sizeX, Image->sizeY, GL_RGB, GL_UNSIGNED_BYTE, Image->data);

		for (int i = 0; i < 3 * Image->sizeX * Image->sizeY; i++)
			Image->data[i] = 255 - Image->data[i];

		glGenTextures(3, gInvbump);

		// Create Nearest Filtered Texture
		glBindTexture(GL_TEXTURE_2D, gInvbump[0]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image->sizeX, Image->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Image->data);

		// Create Linear Filtered Texture
		glBindTexture(GL_TEXTURE_2D, gInvbump[1]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image->sizeX, Image->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, Image->data);

		// Create MipMapped Texture
		glBindTexture(GL_TEXTURE_2D, gInvbump[2]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, Image->sizeX, Image->sizeY, GL_RGB, GL_UNSIGNED_BYTE, Image->data);

		glPixelTransferf(GL_RED_SCALE,1.0f);	
		glPixelTransferf(GL_GREEN_SCALE,1.0f);			
		glPixelTransferf(GL_BLUE_SCALE,1.0f);
	}
	else
	{
		status = false;
	}
	if (Image) 
	{
		if (Image->data) 
			delete Image->data;
		delete Image;
	}	

	return status;
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

void matMultVector(GLfloat *M, GLfloat *v) 
{// Calculates v=vM, M Is 4x4 In Column-Major, v Is 4dim. Row (i.e. "Transposed")
	GLfloat res[3];
	res[0] = M[ 0]*v[0] + M[ 1] * v[1] + M[ 2] * v[2] + M[ 3] * v[3];
	res[1] = M[ 4]*v[0] + M[ 5] * v[1] + M[ 6] * v[2] + M[ 7] * v[3];
	res[2] = M[ 8]*v[0] + M[ 9] * v[1] + M[10] * v[2] + M[11] * v[3];;	
	v[0] = res[0];
	v[1] = res[1];
	v[2] = res[2];
	v[3] = M[15];											// Homogenous Coordinate
}

void setUpBumps(GLfloat *n, GLfloat *c, GLfloat *l, GLfloat *s, GLfloat *t) 
{
	GLfloat v[3];							// Vertex From Current Position To Light	
	GLfloat lenQ;							// Used To Normalize		
	// Calculate v From Current Vector c To Lightposition And Normalize v	
	v[0] = l[0] - c[0];		
	v[1] = l[1] - c[1];		
	v[2] = l[2] - c[2];		
	lenQ=(GLfloat) sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] /= lenQ;		v[1] /= lenQ;		v[2] /= lenQ;
	// Project v Such That We Get Two Values Along Each Texture-Coordinat Axis.
	c[0] = (s[0] * v[0] + s[1] * v[1] + s[2] * v[2]) * MAX_EMBOSS;
	c[1] = (t[0] * v[0] + t[1] * v[1] + t[2] * v[2]) * MAX_EMBOSS;	
}

bool doMesh1TexelUnits(void) 
{
	GLfloat c[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	GLfloat n[4] = {0.0f, 0.0f, 0.0f, 1.0f};					// Normalized Normal Of Current Surface		
	GLfloat s[4] = {0.0f, 0.0f, 0.0f, 1.0f};					// s-Texture Coordinate Direction, Normalized
	GLfloat t[4] = {0.0f, 0.0f, 0.0f, 1.0f};					// t-Texture Coordinate Direction, Normalized
	GLfloat l[4];										// Holds Our Lightposition To Be Transformed Into Object Space
	GLfloat Minv[16];									// Holds The Inverted Modelview Matrix To Do So.
	int i;								

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Build Inverse Modelview Matrix First. This Substitutes One Push/Pop With One glLoadIdentity();
	// Simply Build It By Doing All Transformations Negated And In Reverse Order.
	glLoadIdentity();								
	glRotatef(-gYrot, 0.0f, 1.0f, 0.0f);
	glRotatef(-gXrot, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, 0.0f, -gZ);
	glGetFloatv(GL_MODELVIEW_MATRIX,Minv);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, gZ);

	glRotatef(gXrot, 1.0f, 0.0f, 0.0f);
	glRotatef(gYrot, 0.0f, 1.0f, 0.0f);	

	// Transform The Lightposition Into Object Coordinates:
	l[0] = gLightPosition[0];
	l[1] = gLightPosition[1];
	l[2] = gLightPosition[2];
	l[3] = 1.0f;

	matMultVector(Minv, l);

	/*	PASS#1: Use Texture "Bump"
	No Blend
	No Lighting
	No Offset Texture-Coordinates */
	glBindTexture(GL_TEXTURE_2D, gBump[gFilter]);
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
	doCube();

	/* PASS#2:	Use Texture "Invbump"
	Blend GL_ONE To GL_ONE
	No Lighting
	Offset Texture Coordinates 
	*/
	glBindTexture(GL_TEXTURE_2D, gInvbump[gFilter]);
	glBlendFunc(GL_ONE,GL_ONE);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);	

	glBegin(GL_QUADS);	
	// Front Face	
	n[0] = 0.0f;		n[1] = 0.0f;		n[2] = 1.0f;			
	s[0] = 1.0f;		s[1] = 0.0f;		s[2] = 0.0f;
	t[0] = 0.0f;		t[1] = 1.0f;		t[2] = 0.0f;
	for (i = 0; i < 4; i++) 
	{	
		c[0] = gData[5*i+2];		
		c[1] = gData[5*i+3];
		c[2] = gData[5*i+4];
		setUpBumps(n, c, l, s, t);
		glTexCoord2f(gData[5*i] + c[0], gData[5*i+1] + c[1]); 
		glVertex3f(gData[5*i+2], gData[5*i+3], gData[5*i+4]);
	}
	// Back Face	
	n[0] = 0.0f;		n[1] = 0.0f;		n[2] = -1.0f;	
	s[0] = -1.0f;		s[1] = 0.0f;		s[2] = 0.0f;
	t[0] = 0.0f;		t[1] = 1.0f;		t[2] = 0.0f;
	for (i = 4; i < 8; i++) 
	{	
		c[0] = gData[5*i+2];		
		c[1] = gData[5*i+3];
		c[2] = gData[5*i+4];
		setUpBumps(n, c, l, s, t);
		glTexCoord2f(gData[5*i] + c[0], gData[5*i+1] + c[1]); 
		glVertex3f(gData[5*i+2], gData[5*i+3], gData[5*i+4]);
	}
	// Top Face	
	n[0] = 0.0f;		n[1] = 1.0f;		n[2] = 0.0f;		
	s[0] = 1.0f;		s[1] = 0.0f;		s[2] = 0.0f;
	t[0] = 0.0f;		t[1] = 0.0f;		t[2] = -1.0f;
	for (i = 8; i < 12; i++) 
	{	
		c[0] = gData[5*i+2];		
		c[1] = gData[5*i+3];
		c[2] = gData[5*i+4];
		setUpBumps(n, c, l, s, t);
		glTexCoord2f(gData[5*i] + c[0], gData[5*i+1] + c[1]); 
		glVertex3f(gData[5*i+2], gData[5*i+3], gData[5*i+4]);
	}
	// Bottom Face
	n[0] = 0.0f;		n[1] = -1.0f;		n[2] = 0.0f;		
	s[0] = -1.0f;		s[1] = 0.0f;		s[2] = 0.0f;
	t[0] = 0.0f;		t[1] = 0.0f;		t[2] = -1.0f;
	for (i = 12; i < 16; i++) 
	{	
		c[0] = gData[5*i+2];		
		c[1] = gData[5*i+3];
		c[2] = gData[5*i+4];
		setUpBumps(n, c, l, s, t);
		glTexCoord2f(gData[5*i] + c[0], gData[5*i+1] + c[1]); 
		glVertex3f(gData[5*i+2], gData[5*i+3], gData[5*i+4]);
	}
	// Right Face	
	n[0] = 1.0f;		n[1] = 0.0f;		n[2] = 0.0f;		
	s[0] = 0.0f;		s[1] = 0.0f;		s[2] = -1.0f;
	t[0] = 0.0f;		t[1] = 1.0f;		t[2] = 0.0f;
	for (i = 16; i < 20; i++) 
	{	
		c[0] = gData[5*i+2];		
		c[1] = gData[5*i+3];
		c[2] = gData[5*i+4];
		setUpBumps(n, c, l, s, t);
		glTexCoord2f(gData[5*i] + c[0], gData[5*i+1] + c[1]); 
		glVertex3f(gData[5*i+2], gData[5*i+3], gData[5*i+4]);
	}
	// Left Face
	n[0] = -1.0f;		n[1] = 0.0f;		n[2] = 0.0f;		
	s[0] = 0.0f;		s[1] = 0.0f;		s[2] = 1.0f;
	t[0] = 0.0f;		t[1] = 1.0f;		t[2] = 0.0f;
	for (i=20; i<24; i++) 
	{	
		c[0] = gData[5*i+2];		
		c[1] = gData[5*i+3];
		c[2] = gData[5*i+4];
		setUpBumps(n, c, l, s, t);
		glTexCoord2f(gData[5*i] + c[0], gData[5*i+1] + c[1]); 
		glVertex3f(gData[5*i+2], gData[5*i+3], gData[5*i+4]);
	}		
	glEnd();

	/* PASS#3:	Use Texture "Base"
	Blend GL_DST_COLOR To GL_SRC_COLOR (Multiplies By 2)
	Lighting Enabled
	No Offset Texture-Coordinates
	*/
	if (!gEmboss) 
	{
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D, gTexture[gFilter]);
		glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);	
		glEnable(GL_LIGHTING);
		doCube();
	}

	return true;
}

bool doMeshNoBumps(void) 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, gZ);

	glRotatef(gXrot, 1.0f, 0.0f, 0.0f);
	glRotatef(gYrot, 0.0f, 1.0f, 0.0f);	
	glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D,gTexture[gFilter]);	
	glBlendFunc(GL_DST_COLOR,GL_SRC_COLOR);
	glEnable(GL_LIGHTING);

	doCube();
	return true;
}

void init()
{
	if (!loadTextures()) 
		return;

	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	//Initialize the lights.
	glLightfv( GL_LIGHT1, GL_AMBIENT, gLightAmbient);
	glLightfv( GL_LIGHT1, GL_DIFFUSE, gLightDiffuse);	
	glLightfv( GL_LIGHT1, GL_POSITION, gLightPosition);

	glEnable(GL_LIGHT1);
}

void draw(void)
{
	if (gBumps) 
	{
		doMesh1TexelUnits();	
	}
	else
	{
		doMeshNoBumps();
	}

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

	glutPostRedisplay();
}

void reshape(GLsizei w,GLsizei h)
{
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)w/(GLfloat)h,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y)
{
	//If escape is pressed, exit
	if(key == 27)
	{
		exit(0);
	}
	else if (key == 'E' || key == 'e')
	{
		gEmboss =! gEmboss;
	}						
	else if (key == 'B' || key == 'b')
	{
		gBumps =! gBumps;
	}				
	else if (key == 'F' || key == 'f')
	{
		gFilter = (gFilter + 1) % 3;
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

	glutCreateWindow("Emboss Bump Mapping");

	init();

	glutReshapeFunc(reshape);
	glutDisplayFunc(draw);
	glutKeyboardFunc(keyboard);

	glutMainLoop();
	return(0);
}

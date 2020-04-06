//=================================================================================
// Name			:	glutcallbacks.cpp
// Author		:	Christian Thurow
// Date			:	March 30th 2020
// Description	:	Example Implementation for FontLibrary
//					Teaches usage of all functions exposed by the CFontLibrary 
//					class. Look for DrawFontStressTest() and DrawOnScreenDisplay()
//					specifically.
//=================================================================================

#include "stdafx.h"
#include <windows.h>
#include "glew\include\GL\glew.h"
#include "glutcallbacks.h"
#include <sstream>
#include <math.h>
#include "glut\glut.h"
#include <vector>
#include <iostream>
#include "HighPerformanceCounter.h"
#include "FontLibrary/FontLibrary.h"

extern int m_width;
extern int m_height;

extern CFontLibrary* m_fontLibrary;

float white[4] = { 1, 1, 1, 1 };
float black[4] = { 0, 0, 0, 1 };

float red[4] = { 1, 0, 0, 1};
float green[4] = {0, 1, 0, 1 };
float blue[4] = { 0, 0, 1, 1 };

float yellow[4] = { 1, 1, 0, 1 };
float pink[4] = { 1, 0, 1, 1 };
float cyan[4] = { 0, 1, 1, 1 };

float BGCOLOR_4F[4] = {0.15, 0.15, 0.15, 1.0};

unsigned int cubeDisplayListID = 0;

void* font = GLUT_BITMAP_8_BY_13;

LARGE_INTEGER m_frameCounter[100];
LARGE_INTEGER m_freq;
int m_frame = 0;

using namespace std;

float m_glNear = 1.0;
float m_glFar = 100.0f;
double m_fps = 0.0;
double m_glFov = 60;

int m_lbutton = 0; //is left mouse button pressed
int m_lastMouseX = 0;
int m_lastMouseY = 0;

float m_rotX = 0.0;
float m_rotY = 0.0;

float m_rotSpeedX = 0.01;
float m_rotSpeedY = 0.015;

bool m_stressTest = false;
bool m_displayLists = true;
bool m_dyntext = false;
bool m_sdf = false;

CHighPerformanceCounter m_fpsCounter;

unsigned int m_fovDisplayListID = 0;
unsigned int m_escDisplayListID = 0;
unsigned int m_stressDisplayListID = 0;
unsigned int m_dispDisplayListID = 0;
unsigned int m_fpsDisplayListID = 0;
unsigned int m_loremDisplayListID = 0;
unsigned int m_rotXDisplayListID = 0;
unsigned int m_rotYDisplayListID = 0;
unsigned int m_dynDisplayListID = 0;
unsigned int m_leelaDisplayListID = 0;
unsigned int m_leela2DisplayListID = 0;
unsigned int m_sdfDisplayListID = 0;

void SetVSync(bool sync)
{
	// Function pointer for the wgl extention function we need to enable/disable
	// vsync
	typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALPROC)( int );
	PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = 0;

	const char *extensions = (char*)glGetString( GL_EXTENSIONS );

	if( strstr( extensions, "WGL_EXT_swap_control" ) == 0 )
	{
		std::cout << "\nVSync extension not found.";
		return;
	}
	else
	{
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALPROC)wglGetProcAddress( "wglSwapIntervalEXT" );

		if(wglSwapIntervalEXT)
		{
			wglSwapIntervalEXT(sync);
			std::cout << "\nVSync set to: " << sync << "\n";
		}
	}
}

void Init(void)
{
	QueryPerformanceFrequency(&m_freq);

	GLenum err = glewInit();

	SetVSync(false);

	// usual shading
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// clear background to black and clear depth buffer
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// enable depth test (z-buffer)
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// enable normalization of vertex normals
	glEnable(GL_NORMALIZE);

	// initial view definitions
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// perspective projection
	gluPerspective(m_glFov, 1.0, m_glNear, m_glFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_POINT_SMOOTH);

	if(m_fontLibrary)
	{
		bool success = m_fontLibrary->InitGLFonts();
		if(!success)
			return; //ERROR!
	}
}

void DrawFontStressTest()
{
	if(!m_stressTest)
		return;

	if(!m_fontLibrary)
		return; //ERROR!

	CString bigStaticTextWithLineBreaks = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec aliquet urna congue, scelerisque augue id, scelerisque turpis. Suspendisse potenti. Proin efficitur, justo eu ultricies accumsan, leo nunc placerat lacus, eu suscipit odio erat ac ante. Nunc euismod sit amet mi vel mollis. Nullam molestie dolor quis placerat iaculis. Quisque ut lacus faucibus tellus blandit tincidunt. Praesent lobortis eros nunc, non volutpat eros aliquet sed. Fusce non erat leo. Aenean a orci vel ante congue gravida sit amet quis nisi. Proin pulvinar, est eu posuere mattis, velit magna egestas arcu, at tincidunt risus dolor sed felis. Donec dapibus egestas hendrerit. Mauris maximus in ipsum non vulputate. Ut sagittis a leo et placerat. Donec dictum erat sed mi varius, nec euismod massa sodales. Pellentesque in lorem a urna tempor vehicula sed id justo. Donec dictum dapibus risus eget porta. Phasellus at libero blandit, pellentesque felis nec, aliquet orci. Nam eu aliquam nunc, ut imperdiet augue. Nunc viverra venenatis lacinia. Fusce elementum dignissim turpis, sed efficitur arcu feugiat at. Nam rutrum, odio ac sodales semper, magna arcu ullamcorper odio, ut fermentum mi lectus quis orci. Cras ultricies arcu ante, non consequat ex pulvinar et. Quisque in blandit quam. Quisque vitae feugiat elit, eu congue mauris. Phasellus suscipit ligula a est dignissim consequat. Quisque viverra ex metus, fringilla elementum metus lobortis quis. Mauris vitae convallis ipsum. Nullam faucibus ornare dui eu consectetur. Donec aliquam sem quis ligula ultricies, ac fringilla arcu congue. Pellentesque molestie diam in maximus imperdiet. Integer cursus sit amet sapien ac porta. In blandit aliquet consequat. Vivamus ultrices pretium facilisis. Morbi eu turpis id mauris malesuada fermentum in nec ipsum. Vestibulum congue dapibus est, a vulputate erat congue a. Phasellus aliquet blandit lectus. Mauris tincidunt luctus ullamcorper. Curabitur diam turpis, aliquet ac sagittis nec, gravida eget justo. Duis mattis diam ac lacus hendrerit mattis ut ut diam. Praesent eu velit nec nisi hendrerit tempor. Nunc dapibus mauris ante, eu congue magna sollicitudin eget. Fusce viverra egestas quam. Vivamus porttitor est vitae vestibulum tristique. Suspendisse non lacinia tortor. Suspendisse facilisis, dolor sit amet dignissim efficitur, orci mi lobortis urna, vel posuere urna mi a sem. Morbi a libero bibendum, dictum dui eu, finibus sem. Aenean viverra malesuada nulla, efficitur laoreet nulla egestas sit amet. Phasellus ut imperdiet lorem, vel aliquam mauris. Donec sit amet est id ex dapibus cursus nec vel lacus. Morbi ac velit tellus. Quisque eget rutrum felis. Donec tempor ex turpis, vel pretium lectus ultrices ut. Ut eget est ac nulla lacinia viverra ut a felis. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Quisque ornare elit in magna lobortis dapibus. Sed sapien est, tincidunt nec leo eu, consequat accumsan mi. Etiam quis dignissim enim. Praesent porta erat a diam posuere porta.";
	CString dynamicText1;
	dynamicText1.Format("Cube Rotation Angle x: %.6f", m_rotX);
	CString dynamicText2;
	dynamicText2.Format("Cube Rotation Angle y: %.6f", m_rotY);

	CString leelaTest = "Font rendered:";
	if(m_sdf)
		leelaTest.Append(" with SDF Shader.");
	else
		leelaTest.Append(" conventionally.");

	int top = m_height-60;

	if(m_displayLists)
	{
		if(!m_dyntext)
		{
			if(m_loremDisplayListID == 0)
				m_loremDisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawStringWithLineBreaks(m_loremDisplayListID, bigStaticTextWithLineBreaks, 30, top, white, GLFONT_ARIAL20, false, 1.0, m_width/2.0 - 30, 25);

			if(m_leelaDisplayListID == 0)
				m_leelaDisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_leelaDisplayListID, leelaTest + " Scale: 1.0", 30, 120, green, GLFONT_DINNEXTLTPROMED_SDF, m_sdf, 1.0);

			float lHeightSDF = m_fontLibrary->GetLineHeight(GLFONT_DINNEXTLTPROMED_SDF)*2.0;
			if(m_leela2DisplayListID == 0)
				m_leela2DisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_leela2DisplayListID, leelaTest + " Scale: 2.0", 30, 120 - lHeightSDF, green, GLFONT_DINNEXTLTPROMED_SDF, m_sdf, 2.0);

		}
		else
		{
			if(m_rotXDisplayListID == 0)
				m_rotXDisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_rotXDisplayListID, dynamicText1, m_width/2 + 30, top, white, GLFONT_ARIAL20, false);

			if(m_rotYDisplayListID == 0)
				m_rotYDisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_rotYDisplayListID, dynamicText2, m_width/2 + 30, top-20, white, GLFONT_ARIAL20, false);
		}
	}
	else
	{
		if(!m_dyntext)
		{
			m_fontLibrary->DrawStringWithLineBreaks(bigStaticTextWithLineBreaks, 30, top, white, GLFONT_ARIAL20, false, 1.0, m_width/2.0 - 30, 25);

			m_fontLibrary->DrawString(leelaTest + " Scale: 1.0", 30, 120, green, GLFONT_DINNEXTLTPROMED_SDF, m_sdf, 1.0);
			float lHeightSDF = m_fontLibrary->GetLineHeight(GLFONT_DINNEXTLTPROMED_SDF)*2.0;
			m_fontLibrary->DrawString(leelaTest + " Scale: 2.0", 30, 120 - lHeightSDF, green, GLFONT_DINNEXTLTPROMED_SDF, m_sdf, 2.0);
		}
		else
		{
			m_fontLibrary->DrawString(dynamicText1, m_width/2 + 30, top, white, GLFONT_ARIAL20, false);
			m_fontLibrary->DrawString(dynamicText2, m_width/2 + 30, top - 20, white, GLFONT_ARIAL20, false);
		}
	}
}

void DrawOnScreenDisplay()
{
	if(!m_fontLibrary)
		return; //ERROR!

	CString fps;
	fps.Format("fps: %.1f", m_fps);

	CString fov;
	fov.Format("fovH: %.1f", m_glFov);

	CString esc = "esc: exit";
	CString stress = "enter: Stress Test";
	CString disp = "l: Toggle Display Lists";
	if(m_displayLists)
		disp.Append(" (enabled)");
	else
		disp.Append(" (disabled)");

	if(m_stressTest)
		stress.Append(" (enabled)");
	else
		stress.Append(" (disabled)");

	CString dyn = "d: Toggle static vs. dynamic text";

	if(m_dyntext)
		dyn.Append(" (enabled)");
	else
		dyn.Append(" (disabled)");

	CString s = "s: toggle sdf";

	unsigned int lHeight = m_fontLibrary->GetLineHeight(GLFONT_ARIAL20);
	unsigned int boarderPadding = 10;
	float strWidth = m_fontLibrary->GetWidthOfString(esc, GLFONT_ARIAL20);
	float dynStrWidth = m_fontLibrary->GetWidthOfString(dyn, GLFONT_ARIAL20);

	if(m_displayLists)
	{
		if(m_escDisplayListID == 0)
			m_escDisplayListID = m_fontLibrary->GetNewDrawStringID();
		m_fontLibrary->DrawString(m_escDisplayListID, esc, m_width - strWidth - boarderPadding, m_height - lHeight, white, GLFONT_ARIAL20, false);

		if(m_stressDisplayListID == 0)
			m_stressDisplayListID = m_fontLibrary->GetNewDrawStringID();
		m_fontLibrary->DrawString(m_stressDisplayListID, stress, boarderPadding, m_height - lHeight, m_stressTest ? red : white, GLFONT_ARIAL20, false);

		if(m_dispDisplayListID == 0)
			m_dispDisplayListID = m_fontLibrary->GetNewDrawStringID();
		m_fontLibrary->DrawString(m_dispDisplayListID, disp, boarderPadding, m_height - 2*lHeight, white, GLFONT_ARIAL20, false);

		if(m_stressTest)
		{
			if(m_dynDisplayListID == 0)
				m_dynDisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_dynDisplayListID, dyn, m_width/2.0 - dynStrWidth/2.0, m_height - lHeight, m_dyntext ? red : white, GLFONT_ARIAL20, false);

			if(m_sdfDisplayListID == 0)
				m_sdfDisplayListID = m_fontLibrary->GetNewDrawStringID();
			float lHeightSDF = m_fontLibrary->GetLineHeight(GLFONT_DINNEXTLTPROMED_SDF);
			m_fontLibrary->DrawString(m_sdfDisplayListID, s, 30, 120 + lHeightSDF, m_dyntext ? red : white, GLFONT_ARIAL20, false);
		}

		if(m_fpsDisplayListID == 0)
			m_fpsDisplayListID = m_fontLibrary->GetNewDrawStringID();
		m_fontLibrary->DrawString(m_fpsDisplayListID, fps, boarderPadding, lHeight, white, GLFONT_ARIAL20, false);

		if(m_fovDisplayListID == 0)
			m_fovDisplayListID = m_fontLibrary->GetNewDrawStringID();
		m_fontLibrary->DrawString(m_fovDisplayListID, fov, boarderPadding, 2*lHeight, white, GLFONT_ARIAL20, false);
	}
	else
	{
		m_fontLibrary->DrawString(esc, m_width - strWidth - boarderPadding, m_height - lHeight, white, GLFONT_ARIAL20, false);
		m_fontLibrary->DrawString(stress, boarderPadding, m_height - lHeight, m_stressTest ? red : white, GLFONT_ARIAL20, false);
		m_fontLibrary->DrawString(disp, boarderPadding, m_height - 2*lHeight, red, GLFONT_ARIAL20, false);

		if(m_stressTest)
		{
			m_fontLibrary->DrawString(dyn, m_width/2.0 - dynStrWidth/2.0, m_height - lHeight, m_dyntext ? red : white, GLFONT_ARIAL20, false);

			float lHeightSDF = m_fontLibrary->GetLineHeight(GLFONT_DINNEXTLTPROMED_SDF);
			m_fontLibrary->DrawString(s, 30, 120 + lHeightSDF, m_dyntext ? red : white, GLFONT_ARIAL20, false);
		}

		m_fontLibrary->DrawString(fps, boarderPadding, lHeight, white, GLFONT_ARIAL20, false);
		m_fontLibrary->DrawString(fov, boarderPadding, 2*lHeight, white, GLFONT_ARIAL20, false);
	}
}

void CalculateFrameRate(LARGE_INTEGER newVal)
{
	if(m_frame >= 100)
		m_frame = 0;

	m_frameCounter[m_frame] = newVal;

	int frameDiff = m_frame+1;
	if(frameDiff > 99)
		frameDiff = 0;

	LARGE_INTEGER head = m_frameCounter[frameDiff];
	LARGE_INTEGER tail = m_frameCounter[m_frame];

	double elapsedTime = ((double)tail.QuadPart - (double)head.QuadPart)/(double)m_freq.QuadPart;

	double temp = elapsedTime / 100.0;

	m_fps = 1.0/temp;

	m_frame++;
}

void DrawCubeEightColor()
{
	glDisable(GL_TEXTURE_2D);
	//cube centered around zero with size:
	float size = 1.0;
	float sizeHalf = size / 2.0;
	float z = -2.0;

	glPushMatrix();

	glTranslatef(0.0, 0.0, z);

	glRotatef(m_rotX, 0.0, 1.0, 0.0);
	glRotatef(m_rotY, 0.0, 0.0, 1.0);

	if(!cubeDisplayListID)
	{
		glEnable(GL_MULTISAMPLE);

		GLuint id = glGenLists(1);
		glNewList(id, GL_COMPILE);

		glColor4fv(white);

		glBegin(GL_QUADS);
			//face 1 - front face
			glColor4fv(red);
			glVertex3f(-sizeHalf, sizeHalf, sizeHalf); //top left
			glColor4fv(pink);
			glVertex3f(sizeHalf, sizeHalf, sizeHalf); //top right
			glColor4fv(blue);
			glVertex3f(sizeHalf, -sizeHalf, sizeHalf); //bottom right
			glColor4fv(cyan);
			glVertex3f(-sizeHalf, -sizeHalf, sizeHalf); //bottom left

			//face 3 - left face
			glColor4fv(red);
			glVertex3f(-sizeHalf, sizeHalf, sizeHalf); //front top
			glColor4fv(yellow);
			glVertex3f(-sizeHalf, sizeHalf, -sizeHalf); //back top
			glColor4fv(green);
			glVertex3f(-sizeHalf, -sizeHalf, -sizeHalf); //back bottom
			glColor4fv(cyan);
			glVertex3f(-sizeHalf, -sizeHalf, sizeHalf); //front bottom

			//face 4 - right face
			glColor4fv(pink);
			glVertex3f(sizeHalf, sizeHalf, sizeHalf); //front top
			glColor4fv(white);
			glVertex3f(sizeHalf, sizeHalf, -sizeHalf); //back top
			glColor4fv(black);
			glVertex3f(sizeHalf, -sizeHalf, -sizeHalf); //back bottom
			glColor4fv(blue);
			glVertex3f(sizeHalf, -sizeHalf, sizeHalf); //front bottom
		
			//face 5 - bottom face
			glColor4fv(green);
			glVertex3f(-sizeHalf, -sizeHalf, -sizeHalf); //back left
			glColor4fv(black);
			glVertex3f(sizeHalf, -sizeHalf, -sizeHalf); //back right
			glColor4fv(blue);
			glVertex3f(sizeHalf, -sizeHalf, sizeHalf); //front right
			glColor4fv(cyan);
			glVertex3f(-sizeHalf, -sizeHalf, sizeHalf); //front left

			//face 6 - top face
			glColor4fv(yellow);
			glVertex3f(-sizeHalf, sizeHalf, -sizeHalf); //back left
			glColor4fv(white);
			glVertex3f(sizeHalf, sizeHalf, -sizeHalf); //back right
			glColor4fv(pink);
			glVertex3f(sizeHalf, sizeHalf, sizeHalf); //front right
			glColor4fv(red);
			glVertex3f(-sizeHalf, sizeHalf, sizeHalf); //front left

			//face 2 - back face
			glColor4fv(yellow);
			glVertex3f(-sizeHalf, sizeHalf, -sizeHalf); //top left
			glColor4fv(white);
			glVertex3f(sizeHalf, sizeHalf, -sizeHalf); //top right
			glColor4fv(black);
			glVertex3f(sizeHalf, -sizeHalf, -sizeHalf); //bottom right
			glColor4fv(green);
			glVertex3f(-sizeHalf, -sizeHalf, -sizeHalf); //bottom left
		glEnd();

		glDisable(GL_MULTISAMPLE);

		glEndList();
		cubeDisplayListID = id;
	}
	glCallList(cubeDisplayListID);

	glPopMatrix();
}

void Rotate()
{
	m_rotX += m_rotSpeedX;
	m_rotY += m_rotSpeedY;

	if(m_rotX >= 360.0)
		m_rotX = 0.0;

	if(m_rotY >= 360.0)
		m_rotY = 0.0;
}

// display callback for GLUT
void Display(void)
{
	//initial view definitions
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(m_glFov, (float)m_width/(float)m_height, m_glNear, m_glFar);

	//perspective projection
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(BGCOLOR_4F[0], BGCOLOR_4F[1], BGCOLOR_4F[2], BGCOLOR_4F[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DrawCubeEightColor();

	//set to 2D orthogonal projection
	glMatrixMode(GL_PROJECTION);        //switch to projection matrix
	glLoadIdentity();                   //reset projection matrix
	gluOrtho2D(0, m_width, 0, m_height);//set to orthogonal projection

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//on screen infos
	DrawOnScreenDisplay();

	DrawFontStressTest();

	// display back buffer
	glutSwapBuffers();

	LARGE_INTEGER curr;
	QueryPerformanceCounter(&curr);
	CalculateFrameRate(curr);

	Rotate();
}

// reshape-Callback for GLUT
void Reshape(int w, int h)
{
	// reshaped window aspect ratio
	if(h <= 0)
		h = 1;

	float aspect = (float) w / (float) h;
	m_width = w;
	m_height = h;

	// viewport
	glViewport(0,0, (GLsizei) m_width, (GLsizei) m_height);

	// clear background and depth buffer
	glClearColor(0.1,0.1,0.1,1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// restore view definition after window reshape
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// perspective projection
	gluPerspective(m_glFov, aspect, 1.0, m_glFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	Display();
}

// keyboard callback
void Keyboard(unsigned char key, int x, int y)
{
	// rotate selected node around 
	// x,y and z axes with keypresses
	switch( key )
	{
		case 27: //Escape
			exit(0);
			break;

		case 13:
			m_stressTest = !m_stressTest;
			break;

		case 'l':
			m_displayLists = !m_displayLists;
			break;

		case 'd':
			m_dyntext = !m_dyntext;
			break;

		case 's':
			m_sdf = !m_sdf;
			break;

		default:
			break;
	}
}

// the right button mouse menu
void MouseMenu(int id)
{
	//switch (id)
	//{
	//	case 1: exit(0);
	//	default:
	//		break;
	//}
}

// mouse motion
void MouseMotion(int x, int y)
{
	// rotate selected node when left mouse button is pressed
	if(m_lbutton)
	{
		m_rotY += (float) (y - m_lastMouseY);
		m_rotX -= (float) (x - m_lastMouseX);
		m_lastMouseX = x;
		m_lastMouseY = y;
	}
}

// mouse callback
void Mouse(int btn, int state, int x, int y)
{
	if(btn == GLUT_LEFT)
	{
		if(state == GLUT_UP)
		{
			m_lbutton = 0;
		}
		else if(state == GLUT_DOWN)
		{
			m_lbutton = 1;
			m_lastMouseX = x;
			m_lastMouseY = y;
		}
	}
	else if(btn == GLUT_WHEEL_UP)
	{
		m_glFov += 2.0f;
		if(m_glFov > 179.0f)
			m_glFov = 179.0f;
	}
	else if(btn == GLUT_WHEEL_DOWN)
	{
		m_glFov -= 2.0f;
		if(m_glFov < 1.0f)
			m_glFov = 1.0f;
	}
}

// register callbacks with GLUT
void RegisterCallbacks(void)
{
	glutDisplayFunc(Display);
	glutIdleFunc(Display);
	glutKeyboardFunc(Keyboard);
	glutReshapeFunc(Reshape);
	glutMotionFunc(MouseMotion);
	glutMouseFunc(Mouse);
	//glutCreateMenu(MouseMenu);
		//glutAddMenuEntry("quit",1);
	//glutAttachMenu(GLUT_RIGHT_BUTTON);
	return;
}
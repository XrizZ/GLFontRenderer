//=================================================================================
// Name			:	glutcallbacks.cpp
// Author		:	Christian Thurow
// Date			:	March 30th 2020
// Description	:	Example Implementation for FontLibrary
//					Teaches usage of all functions exposed by the CFontLibrary 
//					class. Look for DrawFontStressTest() and DrawOnScreenDisplay()
//					specifically.
//=================================================================================

#include <windows.h>
#include <glew.h>
#include "glutcallbacks.h"
#include <sstream>
#include <math.h>
#include <glut.h>
#include <vector>
#include <iostream>
#include "HighPerformanceCounter.h"
#include <FontLibrary.h>

// Include GLM
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

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

LARGE_INTEGER m_frameCounter[100];
LARGE_INTEGER m_freq;
int m_frame = 0;

using namespace std;

float m_glNear = 1.0;
float m_glFar = 100.0f;
double m_fps = 0.0;
float m_glFov = 60;

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
unsigned int m_sdf0DisplayListID = 0;
unsigned int m_sdf1DisplayListID = 0;
unsigned int m_sdf2DisplayListID = 0;
unsigned int m_sdf3DisplayListID = 0;
unsigned int m_sdf4DisplayListID = 0;
unsigned int m_sdf5DisplayListID = 0;
unsigned int m_sdf6DisplayListID = 0;
unsigned int m_sdf7DisplayListID = 0;
unsigned int m_sdf8DisplayListID = 0;
unsigned int m_sdf9DisplayListID = 0;
unsigned int m_sdf10DisplayListID = 0;
unsigned int m_sdf11DisplayListID = 0;
unsigned int m_sdf12DisplayListID = 0;
unsigned int m_sdf13DisplayListID = 0;
unsigned int m_sdf14DisplayListID = 0;
unsigned int m_sdf15DisplayListID = 0;
unsigned int m_sdf16DisplayListID = 0;

GLuint m_cubeVertexArrayID = 0;

static const GLfloat g_vertex_buffer_data[] =
{
    -1.0f,-1.0f,-1.0f, //V1
    -1.0f,-1.0f, 1.0f, //V2
    -1.0f, 1.0f, 1.0f, //V3
    1.0f, 1.0f,-1.0f, //V4
    -1.0f,-1.0f,-1.0f, //V1
    -1.0f, 1.0f,-1.0f, //V5
    1.0f,-1.0f, 1.0f, //V6
    -1.0f,-1.0f,-1.0f, //V1
    1.0f,-1.0f,-1.0f, //V7
    1.0f, 1.0f,-1.0f, //V4
    1.0f,-1.0f,-1.0f, //V7
    -1.0f,-1.0f,-1.0f, //V1
    -1.0f,-1.0f,-1.0f, //V1
    -1.0f, 1.0f, 1.0f, //V3
    -1.0f, 1.0f,-1.0f, //V5
    1.0f,-1.0f, 1.0f, //V6
    -1.0f,-1.0f, 1.0f, //V2
    -1.0f,-1.0f,-1.0f, //V1
    -1.0f, 1.0f, 1.0f, //V3
    -1.0f,-1.0f, 1.0f, //V2
    1.0f,-1.0f, 1.0f, //V6
    1.0f, 1.0f, 1.0f, //V8
    1.0f,-1.0f,-1.0f, //V7
    1.0f, 1.0f,-1.0f, //V4
    1.0f,-1.0f,-1.0f, //V7
    1.0f, 1.0f, 1.0f, //V8
    1.0f,-1.0f, 1.0f, //V6
    1.0f, 1.0f, 1.0f, //V8
    1.0f, 1.0f,-1.0f, //V4
    -1.0f, 1.0f,-1.0f, //V5
    1.0f, 1.0f, 1.0f, //V8
    -1.0f, 1.0f,-1.0f, //V5
    -1.0f, 1.0f, 1.0f, //V3
    1.0f, 1.0f, 1.0f, //V8
    -1.0f, 1.0f, 1.0f, //V3
    1.0f,-1.0f, 1.0f //V6
};

// One color for each vertex. They were generated randomly.
static const GLfloat g_color_buffer_data[] =
{
    1.0f, 0.0f, 0.0f, //V1
    0.0f, 1.0f, 0.0f, //V2
    0.0f, 0.0f, 1.0f, //V3
    1.0f, 1.0f, 0.0f, //V4
    1.0f, 0.0f, 0.0f, //V1
    1.0f, 0.0f, 1.0f, //V5
    0.0f, 1.0f, 1.0f, //V6
    1.0f, 0.0f, 0.0f, //V1
    0.0f, 0.0f, 0.0f, //V7
    1.0f, 1.0f, 0.0f, //V4
    0.0f, 0.0f, 0.0f, //V7
    1.0f, 0.0f, 0.0f, //V1
    1.0f, 0.0f, 0.0f, //V1
    0.0f, 0.0f, 1.0f, //V3
    1.0f, 0.0f, 1.0f, //V5
    0.0f, 1.0f, 1.0f, //V6
    0.0f, 1.0f, 0.0f, //V2
    1.0f, 0.0f, 0.0f, //V1
    0.0f, 0.0f, 1.0f, //V3
    0.0f, 1.0f, 0.0f, //V2
    0.0f, 1.0f, 1.0f, //V6
    1.0f, 1.0f, 1.0f, //V8
    0.0f, 0.0f, 0.0f, //V7
    1.0f, 1.0f, 0.0f, //V4
    0.0f, 0.0f, 0.0f, //V7
    1.0f, 1.0f, 1.0f, //V8
    0.0f, 1.0f, 1.0f, //V6
    1.0f, 1.0f, 1.0f, //V8
    1.0f, 1.0f, 0.0f, //V4
    1.0f, 0.0f, 1.0f, //V5
    1.0f, 1.0f, 1.0f, //V8
    1.0f, 0.0f, 1.0f, //V5
    0.0f, 0.0f, 1.0f, //V3
    1.0f, 1.0f, 1.0f, //V8
    0.0f, 0.0f, 1.0f, //V3
    0.0f, 1.0f, 1.0f, //V6
};

GLuint m_vertexbufferID = 0;
GLuint m_colorbufferID = 0;

CGLShaderProgram* m_cubeProgram = nullptr;

glm::mat4 m_projectionMatrix;
glm::mat4 m_ViewMatrix = glm::lookAt(
								glm::vec3(4,3,-3), // Camera is at (4,3,-3), in World Space
								glm::vec3(0,0,0), // and looks at the origin
								glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
						);
glm::mat4 m_modelMatrix = glm::mat4(1.0f);
glm::mat4 MVP;

void SetVSync(bool sync)
{
	// Function pointer for the wgl extention function we need to enable/disable
	// vsync
	typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALPROC)( int );
	PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = 0;

	const char *extensions = (char*)glGetString( GL_EXTENSIONS );

	if( strstr( extensions, "WGL_EXT_swap_control" ) == 0 )
	{
		std::cout << "VSync extension not found.";
		return;
	}
	else
	{
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALPROC)wglGetProcAddress( "wglSwapIntervalEXT" );

		if(wglSwapIntervalEXT)
		{
			wglSwapIntervalEXT(sync);
			std::cout << "VSync set to: " << sync << "\n";
		}
	}
}

void Init(void)
{
	QueryPerformanceFrequency(&m_freq);

	GLenum err = glewInit();

	SetVSync(false);

	// clear background to black and clear depth buffer
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// enable depth test (z-buffer)
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_POINT_SMOOTH);

	glGenVertexArrays(1, &m_cubeVertexArrayID);
	glBindVertexArray(m_cubeVertexArrayID);

	glGenBuffers(1, &m_vertexbufferID);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexbufferID); // This will talk about our 'vertexbuffer' buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW); // Give our vertices to OpenGL.

	glGenBuffers(1, &m_colorbufferID);
	glBindBuffer(GL_ARRAY_BUFFER, m_colorbufferID); // This will talk about our 'vertexbuffer' buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW); // Give our vertices to OpenGL.

	std::string vertexShaderCode =
	#include "Cube.vert"
	std::string fragmentShaderCode =
	#include "Cube.frag"

	m_cubeProgram = new CGLShaderProgram();

	if (!m_cubeProgram || !m_cubeProgram->InitFromString(vertexShaderCode, fragmentShaderCode))
	{
		std::string err = "FontLibrary failed to load default shader.";
		std::cout << err;
	}

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	m_projectionMatrix = glm::perspective(glm::radians(m_glFov), 4.0f / 3.0f, 0.1f, 100.0f);
	MVP = m_projectionMatrix * m_ViewMatrix * m_modelMatrix; // Remember, matrix multiplication is the other way around

	if(m_fontLibrary)
	{
		bool compressNonSDFTextures = true;
		bool success = m_fontLibrary->InitGLFonts(compressNonSDFTextures);
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

	std::string bigStaticTextWithLineBreaks = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec aliquet urna congue, scelerisque augue id, scelerisque turpis. Suspendisse potenti. Proin efficitur, justo eu ultricies accumsan, leo nunc placerat lacus, eu suscipit odio erat ac ante. Nunc euismod sit amet mi vel mollis. Nullam molestie dolor quis placerat iaculis. Quisque ut lacus faucibus tellus blandit tincidunt. Praesent lobortis eros nunc, non volutpat eros aliquet sed. Fusce non erat leo. Aenean a orci vel ante congue gravida sit amet quis nisi. Proin pulvinar, est eu posuere mattis, velit magna egestas arcu, at tincidunt risus dolor sed felis. Donec dapibus egestas hendrerit. Mauris maximus in ipsum non vulputate. Ut sagittis a leo et placerat. Donec dictum erat sed mi varius, nec euismod massa sodales. Pellentesque in lorem a urna tempor vehicula sed id justo. Donec dictum dapibus risus eget porta. Phasellus at libero blandit, pellentesque felis nec, aliquet orci. Nam eu aliquam nunc, ut imperdiet augue. Nunc viverra venenatis lacinia. Fusce elementum dignissim turpis, sed efficitur arcu feugiat at. Nam rutrum, odio ac sodales semper, magna arcu ullamcorper odio, ut fermentum mi lectus quis orci. Cras ultricies arcu ante, non consequat ex pulvinar et. Quisque in blandit quam. Quisque vitae feugiat elit, eu congue mauris. Phasellus suscipit ligula a est dignissim consequat. Quisque viverra ex metus, fringilla elementum metus lobortis quis. Mauris vitae convallis ipsum. Nullam faucibus ornare dui eu consectetur. Donec aliquam sem quis ligula ultricies, ac fringilla arcu congue. Pellentesque molestie diam in maximus imperdiet. Integer cursus sit amet sapien ac porta. In blandit aliquet consequat. Vivamus ultrices pretium facilisis. Morbi eu turpis id mauris malesuada fermentum in nec ipsum. Vestibulum congue dapibus est, a vulputate erat congue a. Phasellus aliquet blandit lectus. Mauris tincidunt luctus ullamcorper. Curabitur diam turpis, aliquet ac sagittis nec, gravida eget justo. Duis mattis diam ac lacus hendrerit mattis ut ut diam. Praesent eu velit nec nisi hendrerit tempor. Nunc dapibus mauris ante, eu congue magna sollicitudin eget. Fusce viverra egestas quam. Vivamus porttitor est vitae vestibulum tristique. Suspendisse non lacinia tortor. Suspendisse facilisis, dolor sit amet dignissim efficitur, orci mi lobortis urna, vel posuere urna mi a sem. Morbi a libero bibendum, dictum dui eu, finibus sem. Aenean viverra malesuada nulla, efficitur laoreet nulla egestas sit amet. Phasellus ut imperdiet lorem, vel aliquam mauris. Donec sit amet est id ex dapibus cursus nec vel lacus. Morbi ac velit tellus. Quisque eget rutrum felis. Donec tempor ex turpis, vel pretium lectus ultrices ut. Ut eget est ac nulla lacinia viverra ut a felis. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Quisque ornare elit in magna lobortis dapibus. Sed sapien est, tincidunt nec leo eu, consequat accumsan mi. Etiam quis dignissim enim. Praesent porta erat a diam posuere porta.";

	char dynamicText1[50];
	sprintf_s(dynamicText1, "Cube Rotation Angle x: %.6f", m_rotX);

	char dynamicText2[50];
	sprintf_s(dynamicText2, "Cube Rotation Angle y: %.6f", m_rotY);

	std::string explanation1 = "native scale  - RGBA texture | SDF texture | Multi-Channel SDF texture";
	std::string explanation2 = "scale: 2.0    - RGBA texture | SDF texture | Multi-Channel SDF texture";
	std::string explanation3 = "scale: 4.0    - RGBA texture | SDF texture | Multi-Channel SDF texture";
	std::string explanation4 = "scale: 8.0    -     SDF texture        |         Multi-Channel SDF texture";
	std::string sample = "a";
	std::string outlineS = "Outlined";
	std::string outlineM = "Outlined";

	int top = m_height-60;

	if(m_displayLists)
	{
		if(!m_dyntext)
		{
			if(m_loremDisplayListID == 0)
				m_loremDisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawStringWithLineBreaks(m_loremDisplayListID, bigStaticTextWithLineBreaks, 30, top, white, GLFONT_ARIAL20, m_width, m_height, 1.0, m_width/2.0 - 30, 22);

			if(m_sdf0DisplayListID == 0)
				m_sdf0DisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_sdf0DisplayListID, explanation1, m_width/2 + 10, top, yellow, GLFONT_ARIAL20, m_width, m_height, 1.0);

			if(m_sdf1DisplayListID == 0)
				m_sdf1DisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_sdf1DisplayListID, sample, m_width/2 + 100, top - 50, green, GLFONT_COURIERNEW100, m_width, m_height, 1.0);

			if(m_sdf2DisplayListID == 0)
				m_sdf2DisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_sdf2DisplayListID, sample, m_width/2 + 250, top - 50, red, GLFONT_COURIERNEW_SDF, m_width, m_height, 0.275);

			if(m_sdf3DisplayListID == 0)
				m_sdf3DisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_sdf3DisplayListID, sample, m_width/2 + 400, top - 50, blue, GLFONT_COURIERNEW_MSDF, m_width, m_height, 1.0);

			if(m_sdf4DisplayListID == 0)
				m_sdf4DisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_sdf4DisplayListID, explanation2, m_width/2 + 10, top-100, yellow, GLFONT_ARIAL20, m_width, m_height, 1.0);

			if(m_sdf5DisplayListID == 0)
				m_sdf5DisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_sdf5DisplayListID, sample, m_width/2 + 70, top - 190, green, GLFONT_COURIERNEW100, m_width, m_height, 2.0);

			if(m_sdf6DisplayListID == 0)
				m_sdf6DisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_sdf6DisplayListID, sample, m_width/2 + 220, top - 190, red, GLFONT_COURIERNEW_SDF, m_width, m_height, 0.55);

			if(m_sdf7DisplayListID == 0)
				m_sdf7DisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_sdf7DisplayListID, sample, m_width/2 + 370, top - 190, blue, GLFONT_COURIERNEW_MSDF, m_width, m_height, 2.0);

			if(m_sdf8DisplayListID == 0)
				m_sdf8DisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_sdf8DisplayListID, explanation3, m_width/2 + 10, top-220, yellow, GLFONT_ARIAL20, m_width, m_height, 1.0);

			if(m_sdf9DisplayListID == 0)
				m_sdf9DisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_sdf9DisplayListID, sample, m_width/2, top - 400, green, GLFONT_COURIERNEW100, m_width, m_height, 4.0);

			if(m_sdf10DisplayListID == 0)
				m_sdf10DisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_sdf10DisplayListID, sample, m_width/2 + 190, top - 400, red, GLFONT_COURIERNEW_SDF, m_width, m_height, 1.10);

			if(m_sdf11DisplayListID == 0)
				m_sdf11DisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_sdf11DisplayListID, sample, m_width/2 + 390, top - 400, blue, GLFONT_COURIERNEW_MSDF, m_width, m_height, 4.0);

			if(m_sdf12DisplayListID == 0)
				m_sdf12DisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_sdf12DisplayListID, explanation4, m_width/2 + 10, top-440, yellow, GLFONT_ARIAL20, m_width, m_height, 1.0);

			if(m_sdf13DisplayListID == 0)
				m_sdf13DisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_sdf13DisplayListID, sample, m_width/2 - 200, top - 800, red, GLFONT_COURIERNEW_SDF, m_width, m_height, 2.2);

			if(m_sdf14DisplayListID == 0)
				m_sdf14DisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_sdf14DisplayListID, sample, m_width/2 + 190, top - 800, blue, GLFONT_COURIERNEW_MSDF, m_width, m_height, 8.0);

			//OUTLINE DEMONSTRATION:
			if(m_sdf15DisplayListID == 0)
				m_sdf15DisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_sdf15DisplayListID, outlineS, 30, top - 500, white, GLFONT_COURIERNEW_SDF, m_width, m_height, 0.281, 1, red);

			if(m_sdf16DisplayListID == 0)
				m_sdf16DisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_sdf16DisplayListID, outlineM, 30, top - 580, white, GLFONT_COURIERNEW_MSDF, m_width, m_height, 1.0, 1, red);
		}
		else
		{
			if(m_rotXDisplayListID == 0)
				m_rotXDisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_rotXDisplayListID, dynamicText1, m_width/2 + 30, top, white, GLFONT_ARIAL20, m_width, m_height);

			if(m_rotYDisplayListID == 0)
				m_rotYDisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_rotYDisplayListID, dynamicText2, m_width/2 + 30, top-20, white, GLFONT_ARIAL20, m_width, m_height);
		}
	}
	else
	{
		if(!m_dyntext)
		{
			m_fontLibrary->DrawStringWithLineBreaks(bigStaticTextWithLineBreaks, 30, top, white, GLFONT_ARIAL20, m_width, m_height, 1.0, m_width/2.0 - 30, 22);

			m_fontLibrary->DrawString(explanation1, m_width/2 + 10, top, yellow, GLFONT_ARIAL20, m_width, m_height, 1.0);
			m_fontLibrary->DrawString(sample, m_width/2 + 100, top - 50, green, GLFONT_COURIERNEW100, m_width, m_height, 1.0);
			m_fontLibrary->DrawString(sample, m_width/2 + 250, top - 50, red, GLFONT_COURIERNEW_SDF, m_width, m_height, 0.275);
			m_fontLibrary->DrawString(sample, m_width/2 + 400, top - 50, blue, GLFONT_COURIERNEW_MSDF, m_width, m_height, 1.0);
			m_fontLibrary->DrawString(explanation2, m_width/2 + 10, top-100, yellow, GLFONT_ARIAL20, m_width, m_height, 1.0);
			m_fontLibrary->DrawString(sample, m_width/2 + 70, top - 190, green, GLFONT_COURIERNEW100, m_width, m_height, 2.0);
			m_fontLibrary->DrawString(sample, m_width/2 + 220, top - 190, red, GLFONT_COURIERNEW_SDF, m_width, m_height, 0.55);
			m_fontLibrary->DrawString(sample, m_width/2 + 370, top - 190, blue, GLFONT_COURIERNEW_MSDF, m_width, m_height, 2.0);
			m_fontLibrary->DrawString(explanation3, m_width/2 + 10, top-220, yellow, GLFONT_ARIAL20, m_width, m_height, 1.0);
			m_fontLibrary->DrawString(sample, m_width/2, top - 400, green, GLFONT_COURIERNEW100, m_width, m_height, 4.0);
			m_fontLibrary->DrawString(sample, m_width/2 + 190, top - 400, red, GLFONT_COURIERNEW_SDF, m_width, m_height, 1.10);
			m_fontLibrary->DrawString(sample, m_width/2 + 390, top - 400, blue, GLFONT_COURIERNEW_MSDF, m_width, m_height, 4.0);
			m_fontLibrary->DrawString(explanation4, m_width/2 + 10, top-440, yellow, GLFONT_ARIAL20, m_width, m_height, 1.0);
			m_fontLibrary->DrawString(sample, m_width/2 - 200, top - 800, red, GLFONT_COURIERNEW_SDF, m_width, m_height, 2.2);
			m_fontLibrary->DrawString(sample, m_width/2 + 190, top - 800, blue, GLFONT_COURIERNEW_MSDF, m_width, m_height, 8.0);
		}
		else
		{
			m_fontLibrary->DrawString(dynamicText1, m_width/2 + 30, top, white, GLFONT_ARIAL20, m_width, m_height);
			m_fontLibrary->DrawString(dynamicText2, m_width/2 + 30, top - 20, white, GLFONT_ARIAL20, m_width, m_height);
		}
	}
}

void DrawOnScreenDisplay()
{
	if(!m_fontLibrary)
		return; //ERROR!

	char fps[20];
	sprintf_s(fps, "fps: %.1f", m_fps);

	char fov[20];
	sprintf_s(fov, "fovH: %.1f", m_glFov);

	std::string esc = "esc: exit";
	std::string stress = "enter: Stress Test";
	std::string disp = "l: Toggle Font Lookup Lists";
	if(m_displayLists)
		disp.append(" (enabled)");
	else
		disp.append(" (disabled)");

	if(m_stressTest)
		stress.append(" (enabled)");
	else
		stress.append(" (disabled)");

	std::string dyn = "d: Toggle static vs. dynamic text";

	if(m_dyntext)
		dyn.append(" (enabled)");
	else
		dyn.append(" (disabled)");

	unsigned int lHeight = m_fontLibrary->GetLineHeight(GLFONT_ARIAL20);
	unsigned int boarderPadding = 10;
	float strWidth = m_fontLibrary->GetWidthOfString(esc, GLFONT_ARIAL20);
	float dynStrWidth = m_fontLibrary->GetWidthOfString(dyn, GLFONT_ARIAL20);

	if(m_displayLists)
	{
		if(m_escDisplayListID == 0)
			m_escDisplayListID = m_fontLibrary->GetNewDrawStringID();
		m_fontLibrary->DrawString(m_escDisplayListID, esc, m_width - strWidth - boarderPadding, m_height - lHeight, white, GLFONT_ARIAL20, m_width, m_height);

		if(m_stressDisplayListID == 0)
			m_stressDisplayListID = m_fontLibrary->GetNewDrawStringID();
		m_fontLibrary->DrawString(m_stressDisplayListID, stress, boarderPadding, m_height - lHeight, m_stressTest ? red : white, GLFONT_ARIAL20, m_width, m_height);

		if(m_dispDisplayListID == 0)
			m_dispDisplayListID = m_fontLibrary->GetNewDrawStringID();
		m_fontLibrary->DrawString(m_dispDisplayListID, disp, boarderPadding, m_height - 2*lHeight, white, GLFONT_ARIAL20, m_width, m_height);

		if(m_stressTest)
		{
			if(m_dynDisplayListID == 0)
				m_dynDisplayListID = m_fontLibrary->GetNewDrawStringID();
			m_fontLibrary->DrawString(m_dynDisplayListID, dyn, m_width/2.0 - dynStrWidth/2.0, m_height - lHeight, m_dyntext ? red : white, GLFONT_ARIAL20, m_width, m_height);
		}

		if(m_fpsDisplayListID == 0)
			m_fpsDisplayListID = m_fontLibrary->GetNewDrawStringID();
		m_fontLibrary->DrawString(m_fpsDisplayListID, fps, boarderPadding, lHeight, white, GLFONT_ARIAL20, m_width, m_height);

		if(m_fovDisplayListID == 0)
			m_fovDisplayListID = m_fontLibrary->GetNewDrawStringID();
		m_fontLibrary->DrawString(m_fovDisplayListID, fov, boarderPadding, 2*lHeight, white, GLFONT_ARIAL20, m_width, m_height);
	}
	else
	{
		m_fontLibrary->DrawString(esc, m_width - strWidth - boarderPadding, m_height - lHeight, white, GLFONT_ARIAL20, m_width, m_height);
		m_fontLibrary->DrawString(stress, boarderPadding, m_height - lHeight, m_stressTest ? red : white, GLFONT_ARIAL20, m_width, m_height);
		m_fontLibrary->DrawString(disp, boarderPadding, m_height - 2*lHeight, red, GLFONT_ARIAL20, m_width, m_height);

		if(m_stressTest)
		{
			m_fontLibrary->DrawString(dyn, m_width/2.0 - dynStrWidth/2.0, m_height - lHeight, m_dyntext ? red : white, GLFONT_ARIAL20, m_width, m_height);
		}

		m_fontLibrary->DrawString(fps, boarderPadding, lHeight, white, GLFONT_ARIAL20, m_width, m_height);
		m_fontLibrary->DrawString(fov, boarderPadding, 2*lHeight, white, GLFONT_ARIAL20, m_width, m_height);
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
	if(!m_cubeProgram)
		return;

	glBindVertexArray(m_cubeVertexArrayID);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexbufferID);
	glVertexAttribPointer(
		0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// 2nd attribute buffer : colors
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, m_colorbufferID);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// Use our shader
	glUseProgram(m_cubeProgram->GetProgramID());

	// Get a handle for our "MVP" uniform
	GLint matrixLoc = glGetUniformLocation(m_cubeProgram->GetProgramID(), "MVP");
	if(matrixLoc >= 0)
		glUniformMatrix4fv(matrixLoc, 1, GL_FALSE, &MVP[0][0]); // Send our transformation to the currently bound shader, in the "MVP" uniform

	// Draw the Cube!
	glDrawArrays(GL_TRIANGLES, 0, 12*3); // 12*3 indices starting at 0 -> 12 triangles

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void Rotate()
{
	m_rotX += m_rotSpeedX;
	m_rotY += m_rotSpeedY;

	if(m_rotX >= 360.0)
		m_rotX = 0.0;

	if(m_rotY >= 360.0)
		m_rotY = 0.0;

	glm::mat4 rotMat = glm::rotate(m_modelMatrix, glm::radians(m_rotX), glm::vec3(1.0, 0.0, 0.0));
	rotMat = glm::rotate(rotMat, glm::radians(m_rotY), glm::vec3(0.0, 1.0, 0.0));
	MVP = m_projectionMatrix * m_ViewMatrix * rotMat; // Remember, matrix multiplication is the other way around
}

// display callback for GLUT
void Display(void)
{
	glClearColor(BGCOLOR_4F[0], BGCOLOR_4F[1], BGCOLOR_4F[2], BGCOLOR_4F[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DrawCubeEightColor();

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
	glViewport(0, 0, (GLsizei)m_width, (GLsizei)m_height);

	// clear background and depth buffer
	glClearColor(BGCOLOR_4F[0],BGCOLOR_4F[1],BGCOLOR_4F[2],BGCOLOR_4F[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Projection matrix : 45° Field of View, aspect ratio, display range : 0.1 unit <-> 100 units
	m_projectionMatrix = glm::perspective(glm::radians(m_glFov), aspect, 0.1f, 100.0f);
	MVP = m_projectionMatrix * m_ViewMatrix * m_modelMatrix; // Remember, matrix multiplication is the other way around

	Display();
}

// keyboard callback
void Keyboard(unsigned char key, int x, int y)
{
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

		m_projectionMatrix = glm::perspective(glm::radians(m_glFov), (float)m_width/(float)m_height, 0.1f, 100.0f);
	}
	else if(btn == GLUT_WHEEL_DOWN)
	{
		m_glFov -= 2.0f;
		if(m_glFov < 1.0f)
			m_glFov = 1.0f;

		m_projectionMatrix = glm::perspective(glm::radians(m_glFov), (float)m_width/(float)m_height, 0.1f, 100.0f);
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
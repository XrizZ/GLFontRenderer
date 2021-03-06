//=================================================================================
// Name			:	demo-01.cpp
// Author		:	Christian Thurow
// Date			:	March 30th 2020
// Description	:	Entry Point for Example Implementation of FontLibrary
//					Teaches usage of CFontLibrary, specifically the creation step.
//					Next look at glutcallbacks.cpp
//=================================================================================

#include <iostream>
#include <FontLibrary.h>
#include <glut.h>
#include "glutcallbacks.h"
#include <tchar.h>

using namespace std;

uint16_t m_width = 1280;
uint16_t m_height = 900;

CFontLibrary* m_fontLibrary = nullptr;

void Render(int argc, char** argv)
{
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowPosition(500, 300);
	glutInitWindowSize(m_width, m_height);
	glutInit(&argc, argv);
	glutCreateWindow("demo-01");

	// register GLUT callbacks (see glutcallbacks.c)
	RegisterCallbacks();
	// initialize OpenGL (see glutcallbacks.c)
	Init();  //this is where we also initialize the font library
	// start GLUT event loop

	cout << "begin rendering" << endl;

	glutMainLoop();
}

int _tmain(int argc, char* argv[])
{
	cout << "Loading Fonts" << endl;

	//load fonts
	std::string fontFileFolder = "..\\FontLibrary\\Fonts";
	
	m_fontLibrary = new CFontLibrary(fontFileFolder);
	if (!m_fontLibrary->ParseAllFontsInFolder())
	{
		delete m_fontLibrary;
		cout << "FontLibrary: , in Fonts folder!\n";
		system("pause");
		return - 1; //no fonts found!
	}

	cout << "Initializing Render Pipeline" << endl;

	Render(argc, argv);

	//cleanup the library
	if (m_fontLibrary)
		delete m_fontLibrary;
	m_fontLibrary = nullptr;

	return 0;
}
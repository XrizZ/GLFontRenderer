//=================================================================================
// Name			:	demo-01.cpp
// Author		:	Christian Thurow
// Date			:	March 30th 2020
// Description	:	Entry Point for Example Implementation of FontLibrary
//					Teaches usage of CFontLibrary, specifically the creation step.
//					Next look at glutcallbacks.cpp
//=================================================================================

#include "stdafx.h"
#include <iostream>
#include "FontLibrary/FontLibrary.h"
#include "glut\glut.h"
#include "glutcallbacks.h"

void Render(int argc, _TCHAR** argv);

using namespace std;

int m_width = 1024;
int m_height = 768;

CFontLibrary* m_fontLibrary = nullptr;

int _tmain(int argc, _TCHAR* argv[])
{
	cout << "begin rendering" << endl;

	//load fonts
	CString fontFileFolder = "..\\demo-01\\FontLibrary\\Fonts";
	
	m_fontLibrary = new CFontLibrary(fontFileFolder);
	if (!m_fontLibrary->ParseAllFontsInFolder())
	{
		delete m_fontLibrary;
		cout << "FontLibrary: No Fonts found in Fonts folder!\n";
		system("pause");
		return - 1; //no fonts found!
	}

	Render(argc, argv);

	//cleanup the library
	if (m_fontLibrary)
		delete m_fontLibrary;
	m_fontLibrary = nullptr;

	return 0;
}

void Render(int argc, _TCHAR** argv)
{
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowPosition(500, 300);
	glutInitWindowSize(m_width, m_height);
	glutInit(&argc, (char**)argv);
	glutCreateWindow("demo-01");

	// register GLUT callbacks (see glutcallbacks.c)
	RegisterCallbacks();
	// initialize OpenGL (see glutcallbacks.c)
	Init();  //this is where we also initialize the font library
	// start GLUT event loop
	glutMainLoop();
}
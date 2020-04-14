//=================================================================================
// Name			:	FontLibrary.h
// Author		:	Christian Thurow
// Date			:	March 30th 2020
// Description	:	Header file for CFontLibrary
// this file denotes the basic structure of the font library
// this class uses files produced by the AngelCode - BitmapFontGenerator,
// which produces a config file(*.fnt) and a bitmap file(*.tga) for each font type.
//=================================================================================

#pragma once
#include "Font.hpp"
#include <Windows.h>
#include "..\glew\include\GL\glew.h"
#include "GLShaderProgram.hpp"
#include <map>

//font type defines, string must match the filename without extension in the "\Fonts" folder!

#define GLFONT_ARIAL20 "Arial20"
#define GLFONT_DINNEXTLTPROMED_SDF "DINNextLTProMED_SDF"

//ADD your own here!


//==================================================================================================

class CGLQuad2D
{
public:
	int topLeftX = 0;
	int topLeftY = 0;
	int topRightX = 0;
	int topRightY = 0;
	int bottomRightX = 0;
	int bottomRightY = 0;
	int bottomLeftX = 0;
	int bottomLeftY = 0;

	float textureTopLeftX = 0;
	float textureTopLeftY = 0;
	float textureTopRightX = 0;
	float textureTopRightY = 0;
	float textureBottomRightX = 0;
	float textureBottomRightY = 0;
	float textureBottomLeftX = 0;
	float textureBottomLeftY = 0;
};

class CDrawString
{
public:
	CDrawString()
	{
		m_color[0] = 0.0f;
		m_color[1] = 0.0f;
		m_color[2] = 0.0f;
		m_color[3] = 0.0f;
	};

	CDrawString(unsigned int ID, std::string textToDraw, int x, int y, float color[4], bool sdf, float scale)
	{
		m_ID = ID;
		m_text = textToDraw;
		m_x = x;
		m_y = y;
		m_color[0] = color[0];
		m_color[1] = color[1];
		m_color[2] = color[2];
		m_color[3] = color[3];
		m_scale = scale;
		m_drawListID = 0;
		m_lineWidth = 0;
		m_maxLines = 0;
		m_sdf = sdf;
	};

	~CDrawString(){};

	unsigned int	m_ID = 0;
	GLint			m_drawListID = 0;
	std::string		m_text;
	int				m_x = 0;
	int				m_y = 0;
	float			m_color[4];
	float			m_scale = 1.0;
	int				m_lineWidth = 0;
	int				m_maxLines = 0;
	bool			m_sdf = false;
};

class CFontLibrary
{
public:
	//constructor + destructor:
	CFontLibrary(std::string folder);
	~CFontLibrary(void);

	//variables:
	std::map<std::string, CGLFont*> m_fontList;
	std::string m_fontFolder;

	//functions:
	bool ParseAllFontsInFolder();
	bool InitGLFonts();
	void DrawString(std::string textToDraw, int x, int y, float color[4], std::string font, bool sdf, float scale = 1.0f);
	void DrawString(unsigned int ID, std::string textToDraw, int x, int y, float color[4], std::string font, bool sdf, float scale = 1.0f);
	void DrawStringWithLineBreaks(std::string textToDraw, int x, int y, float color[4], std::string font, bool sdf, float scale, int lineWidth, int maxLines);
	void DrawStringWithLineBreaks(unsigned int ID, std::string textToDraw, int x, int y, float color[4], std::string font, bool sdf, float scale, int lineWidth, int maxLines);
	unsigned int GetNewDrawStringID();
	float GetWidthOfString(std::string textToDraw, std::string font, float scale = 1.0f);
	unsigned int GetLineHeight(std::string font);

private:
	//functions:
	CGLFont* ParseFont(std::string fileName);
	CGLQuad2D* TextToQuadList(std::string font, std::string textToDraw, int x, int y, float scale);
	void DrawQuadList(std::string font, float color[4], CGLQuad2D* quadList, std::string textToDraw, bool sdf);
	CGLFont* GetFontPointer(std::string fontName);
	int GetTextChar(std::string textToDraw, int pos);
	unsigned int GetWidthOfChar(char ch, std::string font);

	//variables:
	CGLShaderProgram* m_sdfShaderProgram = nullptr;
	std::map<unsigned int, CDrawString*> m_glStringList;
	unsigned int m_IDCounter = 1;
};
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
#include <afxtempl.h>
#include <atlstr.h>
#include "GLShaderProgram.hpp"

//font type defines, string must match the filename without extension in the "\Fonts" folder!

#define GLFONT_ARIAL20 "Arial20"
#define GLFONT_DINNEXTLTPROMED_SDF "DINNextLTProMED_SDF"

//ADD your own here!


//==================================================================================================

class CGLQuad2D
{
public:
	int topLeftX;
	int topLeftY;
	int topRightX;
	int topRightY;
	int bottomRightX;
	int bottomRightY;
	int bottomLeftX;
	int bottomLeftY;

	float textureTopLeftX;
	float textureTopLeftY;
	float textureTopRightX;
	float textureTopRightY;
	float textureBottomRightX;
	float textureBottomRightY;
	float textureBottomLeftX;
	float textureBottomLeftY;
};

class CDrawString
{
public:
	CDrawString()
	{
		m_ID = 0;
		m_drawListID = 0;
		m_x = 0;
		m_y = 0;
		m_color[0] = 0;
		m_color[1] = 0;
		m_color[2] = 0;
		m_color[3] = 0;
		m_scale = 1.0;
		m_lineWidth = 0;
		m_maxLines = 0;
		m_sdf = false;
	};

	CDrawString(unsigned int ID, CString textToDraw, int x, int y, float color[4], bool sdf, float scale)
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

	unsigned int	m_ID;
	GLint			m_drawListID;
	CString			m_text;
	int				m_x;
	int				m_y;
	float			m_color[4];
	float			m_scale;
	int				m_lineWidth;
	int				m_maxLines;
	bool			m_sdf;
};

class CFontLibrary
{
public:
	//constructor + destructor:
	CFontLibrary(CString folder);
	~CFontLibrary(void);

	//variables:
	CMap<CString, LPCSTR, CGLFont*, CGLFont*&> m_fontList;
	CString m_fontFolder;

	//functions:
	bool ParseAllFontsInFolder();
	bool InitGLFonts();
	void DrawString(CString textToDraw, int x, int y, float color[4], CString font, bool sdf, float scale = 1.0f);
	void DrawString(unsigned int ID, CString textToDraw, int x, int y, float color[4], CString font, bool sdf, float scale = 1.0f);
	void DrawStringWithLineBreaks(CString textToDraw, int x, int y, float color[4], CString font, bool sdf, float scale, int lineWidth, int maxLines);
	void DrawStringWithLineBreaks(unsigned int ID, CString textToDraw, int x, int y, float color[4], CString font, bool sdf, float scale, int lineWidth, int maxLines);
	unsigned int GetNewDrawStringID();
	float GetWidthOfString(CString textToDraw, CString font, float scale = 1.0f);
	unsigned int GetLineHeight(CString font);

private:
	//functions:
	CGLFont* ParseFont(CString fileName);
	CGLQuad2D* TextToQuadList(CString font, CString textToDraw, int x, int y, float scale);
	void DrawQuadList(CString font, float color[4], CGLQuad2D* quadList, CString textToDraw, bool sdf);
	CGLFont* GetFontPointer(CString fontName);
	int GetTextChar(CString textToDraw, int pos);
	unsigned int GetWidthOfChar(CString ch, CString font);

	//variables:
	CGLShaderProgram* m_sdfShaderProgram = nullptr;
	CMap<unsigned int, unsigned int, CDrawString*, CDrawString*&> m_glStringList;
	unsigned int m_IDCounter;
};
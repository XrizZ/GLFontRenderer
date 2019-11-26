#include "StdAfx.h"
#include "Font.h"
#include <windows.h>
#include <gl\GL.h>

CGLFont::CGLFont()
{
	m_size = 0;
	m_face.Empty();
	m_bitmapFileName.Empty();
	m_bold = false;
	m_italic = false;
	m_unicode = false;
	m_fontCharInfo = NULL;
	m_highestASCIIChar = 0;
	m_kernings = NULL;
	m_textureH = 0;
	m_textureW = 0;
	m_highestKerningFirst = 0;
	m_highestKerningSecond = 0;
	m_fontTextures = 0;
	m_base = 0;
}

CGLFont::~CGLFont()
{
	if(m_fontCharInfo)
	{
		delete[] m_fontCharInfo;
		m_fontCharInfo = NULL;
	}

	if(m_kernings)
	{
		for(int i = 0; i <= m_highestKerningFirst; i++)
		{
			delete[] m_kernings[i];
		}
		delete[] m_kernings;
		m_kernings = NULL;
	}


	glDeleteTextures(1, &m_fontTextures);
	m_fontTextures = NULL;
}

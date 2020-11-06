//=================================================================================
// Name			:	Font.cpp
// Author		:	Christian Thurow
// Date			:	March 30th 2020
// Description	:	Implementation file for CGLFont
//					this file denotes the basic structure of the font
//					Is used by he Font Library
//=================================================================================

#include "Font.h"
#include <windows.h>
#include <glew.h>

CGLFont::~CGLFont()
{
	if(m_fontCharInfo)
	{
		delete[] m_fontCharInfo;
		m_fontCharInfo = nullptr;
	}

	if(m_kernings)
	{
		for(uint32_t i = 0; i <= m_highestKerningFirst; i++)
		{
			delete[] m_kernings[i];
		}
		delete[] m_kernings;
		m_kernings = nullptr;
	}

	glDeleteTextures(1, &m_fontTextures);
	m_fontTextures = 0;
}

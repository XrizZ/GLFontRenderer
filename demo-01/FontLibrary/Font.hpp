//=================================================================================
// Name			:	Font.h
// Author		:	Christian Thurow
// Date			:	March 30th 2020
// Description	:	Header file for CGLFont
//					this file denotes the basic structure of the font
//					Is used by he Font Library
//=================================================================================

#pragma once
#include <string>

class CCharInfo
{
public:
	unsigned int m_ID = 0;
	unsigned int m_textureX = 0;
	unsigned int m_textureY = 0;
	unsigned int m_textureHeight = 0;
	unsigned int m_textureWidth = 0;
	int m_xoffset = 0;
	int m_yoffset = 0;
	int m_xadvance = 0;
	//unsigned int m_page = 0; //unused, kept for future development
	//unsigned int m_channel = 0; //unused, kept for future development
};

class CKerning
{
public:
	unsigned int m_first = 0;
	unsigned int m_second = 0;
	int m_amount = 0;
};

class CGLFont
{
public:
	//constructor + destructor:
	CGLFont(){};
	~CGLFont(void);

	//variables:
	std::string m_face;
	unsigned int m_size = 0;
	bool	m_bold = false;
	bool	m_italic = false;
	bool	m_unicode = false;
	std::string m_bitmapFileName;
	CCharInfo** m_fontCharInfo = nullptr;
	unsigned int m_highestASCIIChar = 0;
	unsigned int m_textureH = 0;
	unsigned int m_textureW = 0;
	int**	m_kernings = nullptr;
	unsigned int m_highestKerningFirst = 0;
	unsigned int m_highestKerningSecond = 0;
	unsigned int m_fontTextures = 0;
	int		m_base = 0;
};
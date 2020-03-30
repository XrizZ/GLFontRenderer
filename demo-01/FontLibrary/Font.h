//=================================================================================
// Name			:	Font.h
// Author		:	Christian Thurow
// Date			:	March 30th 2020
// Description	:	Header file for CGLFont
//					this file denotes the basic structure of the font
//					Is used by he Font Library
//=================================================================================

#pragma once

class CCharInfo
{
public:
	unsigned int m_ID;
	unsigned int m_textureX;
	unsigned int m_textureY;
	unsigned int m_textureHeight;
	unsigned int m_textureWidth;
	int m_xoffset;
	int m_yoffset;
	int m_xadvance;
	//unsigned int m_page;
	//unsigned int m_channel;
};

class CKerning
{
public:
	unsigned int m_first;
	unsigned int m_second;
	int m_amount;
};

class CGLFont
{
public:
	//constructor + destructor:
	CGLFont(){};
	~CGLFont(void);

	//variables:
	CString m_face;
	unsigned int m_size = 0;
	bool	m_bold = false;
	bool	m_italic = false;
	bool	m_unicode = false;
	CString m_bitmapFileName;
	CCharInfo** m_fontCharInfo = nullptr;
	unsigned int m_highestASCIIChar = 0;
	unsigned int m_textureH = 0;
	unsigned int m_textureW = 0;
	int** m_kernings = nullptr;
	unsigned int m_highestKerningFirst = 0;
	unsigned int m_highestKerningSecond = 0;
	unsigned int m_fontTextures = 0;
	int		m_base = 0;
};
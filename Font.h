//this file denotes the basic structure of the font

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
	CGLFont();
	~CGLFont(void);

	//variables:
	CString m_face;
	unsigned int m_size;
	bool	m_bold;
	bool	m_italic;
	bool	m_unicode;
	CString m_bitmapFileName;
	CCharInfo** m_fontCharInfo;
	unsigned int m_highestASCIIChar;
	unsigned int m_textureH;
	unsigned int m_textureW;
	int** m_kernings;
	unsigned int m_highestKerningFirst;
	unsigned int m_highestKerningSecond;

	unsigned int m_fontTextures;
	int		m_base;
};
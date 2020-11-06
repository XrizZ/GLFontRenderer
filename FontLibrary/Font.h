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

enum SDFTYPE
{
	SDF_NO = 0,
	SDF_SINGLE = 1,
	SDF_MULTI = 2
};

class CCharInfo
{
public:
	uint32_t m_ID = 0;
	uint16_t m_textureX = 0;
	uint16_t m_textureY = 0;
	uint16_t m_textureHeight = 0;
	uint16_t m_textureWidth = 0;
	int16_t m_xoffset = 0;
	int16_t m_yoffset = 0;
	uint16_t m_xadvance = 0;
	//uint16_t m_page = 0; //unused, kept for future development
	//uint16_t m_channel = 0; //unused, kept for future development
};

class CKerning
{
public:
	uint16_t m_first = 0;
	uint16_t m_second = 0;
	float m_amount = 0.0;
};

class CGLFont
{
public:
	//constructor + destructor:
	CGLFont(){};
	~CGLFont(void);

	//variables:
	std::string m_face;
	uint16_t m_size = 0;
	bool	m_bold = false;
	bool	m_italic = false;
	bool	m_unicode = false;
	std::string m_bitmapFileName;
	CCharInfo** m_fontCharInfo = nullptr;
	uint32_t m_highestASCIIChar = 0;
	uint16_t m_textureH = 0;
	uint16_t m_textureW = 0;
	float**	m_kernings = nullptr;
	uint32_t m_highestKerningFirst = 0;
	uint32_t m_highestKerningSecond = 0;
	uint32_t m_fontTextures = 0;
	uint16_t	m_base = 0;
	SDFTYPE m_sdfType = SDF_NO;
	uint8_t	m_sdfRange = 4; //only used for multichannel SDFs
};
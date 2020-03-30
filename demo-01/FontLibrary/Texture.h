//=================================================================================
// Name			:	Texture.h
// Author		:	Christian Thurow
// Date			:	March 30th 2020
// Description	:	Header file for CGLTexture, used by the FontLibrary
//=================================================================================

#include <Windows.h>
#include <gl/GL.h>
#include <string>

struct CRawTexture
{
	CRawTexture()
	{
		m_channels = 0;
		m_sizeX = 0;
		m_sizeY = 0;
		m_data = nullptr;
	}
	~CRawTexture()
	{
		if (m_data)
		{
			delete[] m_data;
			m_data = nullptr;
		}
	}
	int m_channels;
	int m_sizeX;
	int m_sizeY;
	unsigned char *m_data;
};

class CGLTexture
{
public:
	CGLTexture(){};
	~CGLTexture(){};

	static bool LoadTextureFromFile(std::string strFileName, unsigned int& texID);
	static bool GetTextureSizeFromFile(std::string strFileName, float &w, float &h);
	
private:
	// This Loads And Returns The TGA Image Data
	static CRawTexture *LoadTGA(std::string strFileName);
};
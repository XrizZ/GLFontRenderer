//=================================================================================
// Name			:	Texture.h
// Author		:	Christian Thurow
// Date			:	March 30th 2020
// Description	:	Header file for CGLTexture, used by the FontLibrary
//=================================================================================

#include <Windows.h>
#include <glew.h>
#include <string>

struct CRawTexture
{
	CRawTexture(){}
	~CRawTexture()
	{
		if (m_data)
		{
			delete[] m_data;
			m_data = nullptr;
		}
	}

	int m_channels = 0;
	int m_sizeX = 0;
	int m_sizeY = 0;
	unsigned char *m_data = nullptr;
};

class CGLTexture
{
public:
	CGLTexture(){};
	~CGLTexture(){};

	static bool LoadTextureFromFile(std::string strFileName, unsigned int& texID, bool compressTexture =false);
	static bool GetTextureSizeFromFile(std::string strFileName, float &w, float &h);
	
private:
	// This Loads And Returns The TGA Image Data
	static CRawTexture *LoadTGA(std::string strFileName);
	static CRawTexture *LoadPNG(std::string strFileName);
};
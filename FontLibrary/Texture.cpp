//=================================================================================
// Name			:	Texture.cpp
// Author		:	Christian Thurow
// Date			:	March 30th 2020
// Description	:	Implementation file for CGLTexture, used by the FontLibrary
//=================================================================================

#include "Texture.h"
#include <lodepng.h>

/////////////////////////////////////////////////////////////////////////////////////////////////
//										TEXTURE LOADER
/////////////////////////////////////////////////////////////////////////////////////////////////

bool CGLTexture::GetTextureSizeFromFile(const std::string& strFileName, float &w, float &h)
{
	if(!strFileName.length())
		return false;

	CRawTexture *pImage = nullptr;

	// Load TGA Image
	if(strFileName.find(".tga") != std::string::npos)
	{
		pImage = LoadTGA(strFileName);
	}
	else if(strFileName.find(".png") != std::string::npos)
	{
		pImage = LoadPNG(strFileName);
	}
	else
		return false; //unsupported file type

	if(!pImage)
		return false;

	w = (float)pImage->m_sizeX;
	h = (float)pImage->m_sizeY;

	if(pImage)
	{
		delete pImage;
	}

	return true;
}

bool CGLTexture::LoadTextureFromFile(const std::string& strFileName, unsigned int &texture, bool compressTexture /*=false*/)
{
	if(!strFileName.length())
		return false;

	CRawTexture *pImage = nullptr;

	// Load TGA Image
	if(strFileName.find(".tga") != std::string::npos)
	{
		pImage = LoadTGA(strFileName);
	}
	else if(strFileName.find(".png") != std::string::npos)
	{
		pImage = LoadPNG(strFileName);
	}
	else
		return false; //unsupported file type

	if(!pImage)
		return false;

	glGenTextures(1, &texture);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glBindTexture(GL_TEXTURE_2D, texture);

	int textureType = GL_RGBA;

	if(pImage->m_channels == 3)
		textureType = GL_RGB;

	bool mipmapping = true;

	//set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmapping ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	if(!compressTexture)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, pImage->m_channels == 3 ? GL_RGB8 : GL_RGBA8, pImage->m_sizeX, pImage->m_sizeY, 0 , textureType, GL_UNSIGNED_BYTE, pImage->m_data);
	}
	else
	{
		GLenum format = pImage->m_channels == 3 ? GL_COMPRESSED_RGB_S3TC_DXT1_EXT : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		glTexImage2D(GL_TEXTURE_2D, 0, format, pImage->m_sizeX, pImage->m_sizeY, 0, textureType, GL_UNSIGNED_BYTE, pImage->m_data);

		GLint compressed = 0;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_ARB, &compressed);
		if(!compressed)
		{
			//could not load texture with DXT compression
			return false;
		}
	}

	if(mipmapping)
		glGenerateMipmapEXT(GL_TEXTURE_2D);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	if(pImage)
	{
		delete pImage;
	}

	if(texture)
		return true;
	else
		return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//										PNG LOADER
/////////////////////////////////////////////////////////////////////////////////////////////////

CRawTexture* CGLTexture::LoadPNG(const std::string& strFileName)
{
	unsigned int width = 0;
	unsigned int height = 0;
	std::vector<unsigned char> image;
	unsigned int error = lodepng::decode(image, width, height, strFileName);

	if(error || image.empty())
		return nullptr; //ERROR!

	CRawTexture *pImageData = new CRawTexture();
	pImageData->m_data = new unsigned char[image.size()];
	for(unsigned int i=0; i<image.size(); i++)
		pImageData->m_data[i] = image.at(i);

	pImageData->m_channels = 4;
	pImageData->m_sizeX    = width;
	pImageData->m_sizeY    = height;

	return pImageData;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//										TGA LOADER
/////////////////////////////////////////////////////////////////////////////////////////////////
#define TGA_RGB		 2
#define TGA_A		 3
#define TGA_RLE		10

CRawTexture* CGLTexture::LoadTGA(const std::string& strFileName)
{
	CRawTexture *pImageData = nullptr;
	WORD width = 0, height = 0;
	byte length = 0;
	byte imageType = 0;
	byte bits = 0;
	FILE *pFile = nullptr;
	int channels = 0;
	int stride = 0;
	int i = 0;

	if ((fopen_s(&pFile, strFileName.c_str(), "rb")))
	{
		//error, can't load file
		return nullptr;
	}

	pImageData = new CRawTexture();

	fread(&length, sizeof(byte), 1, pFile);

	fseek(pFile, 1, SEEK_CUR);

	fread(&imageType, sizeof(byte), 1, pFile);

	fseek(pFile, 9, SEEK_CUR);

	fread(&width,  sizeof(WORD), 1, pFile);
	fread(&height, sizeof(WORD), 1, pFile);
	fread(&bits,   sizeof(byte), 1, pFile);

	fseek(pFile, length + 1, SEEK_CUR);

	if(imageType != TGA_RLE)
	{
		if(bits == 24 || bits == 32)
		{
			channels = bits / 8;
			stride = channels * width;
			pImageData->m_data = new unsigned char[stride*height];
			
			for(int y = 0; y < height; y++)
			{
				unsigned char *pLine = &(pImageData->m_data[stride * y]);

				fread(pLine, stride, 1, pFile);

				for(i = 0; i < stride; i += channels)
				{
					int temp     = pLine[i];
					pLine[i]     = pLine[i + 2];
					pLine[i + 2] = temp;
				}
			}
		}
		else if(bits == 16)
		{
			unsigned short pixels = 0;
			int r=0, g=0, b=0;
			
			channels = 3;
			stride = channels * width;
			pImageData->m_data = new unsigned char[stride*height];

			for(int i = 0; i < width*height; i++)
			{
				fread(&pixels, sizeof(unsigned short), 1, pFile);

				b = (pixels & 0x1f) << 3;
				g = ((pixels >> 5) & 0x1f) << 3;
				r = ((pixels >> 10) & 0x1f) << 3;
				
				pImageData->m_data[i * 3 + 0] = r;
				pImageData->m_data[i * 3 + 1] = g;
				pImageData->m_data[i * 3 + 2] = b;
			}
		}
		else
		{
			delete pImageData;
			pImageData = nullptr;
			return nullptr;
		}
	}
	else
	{
		byte rleID = 0;
		int colorsRead = 0;
		channels = bits / 8;
		stride = channels * width;
		unsigned int size = stride*height;

		pImageData->m_data = new unsigned char[size];
		byte *pColors = new byte[channels];

		while(i < width*height)
		{
			fread(&rleID, sizeof(byte), 1, pFile);

			if(rleID < 128)
			{
				rleID++;

				while(rleID)
				{
					fread(pColors, sizeof(byte) * channels, 1, pFile);
					
					pImageData->m_data[(colorsRead + 0)] = pColors[2];
					pImageData->m_data[(colorsRead + 1)] = pColors[1];
					pImageData->m_data[(colorsRead + 2)] = pColors[0];
					
					if(bits == 32)
						pImageData->m_data[(colorsRead + 3)] = pColors[3];

					i++;
					rleID--;
					colorsRead += channels;
				}
			}
			else
			{
				rleID -= 127;

				fread(pColors, sizeof(byte) * channels, 1, pFile);

				while(rleID)
				{
					pImageData->m_data[(colorsRead + 0)] = pColors[2];
					pImageData->m_data[(colorsRead + 1)] = pColors[1];
					pImageData->m_data[(colorsRead + 2)] = pColors[0];

					if(bits == 32)
						pImageData->m_data[(colorsRead + 3)] = pColors[3];

					i++;
					rleID--;
					colorsRead += channels;
				}
			}
		}
		delete []pColors;

		//flip image vertically to get correct orientation
		unsigned char* line = new unsigned char[width*channels];
		for(int i=0; i<height/2; i++)
		{
			memcpy(line, &pImageData->m_data[i*width*channels], width*channels);
			memcpy(&pImageData->m_data[i*width*channels], &pImageData->m_data[size - (i+1)*width*channels], width*channels);
			memcpy(&pImageData->m_data[size - (i+1)*width*channels], line, width*channels);
		}
		delete[]line;
	}

	fclose(pFile);

	pImageData->m_channels = channels;
	pImageData->m_sizeX    = width;
	pImageData->m_sizeY    = height;

	return pImageData;
}
//=================================================================================
// Name			:	FontLibrary.cpp
// Author		:	Christian Thurow
// Date			:	March 30th 2020
// Description	:	Main Implementation file CFontLibrary
//=================================================================================

#include "FontFileParser.h"
#include "FontLibrary.h"
#include "Font.h"
#include "Texture.h"
#include <acutil_unicode.h>
#include <windows.h>
#include <fstream>
#include <iostream>
#include <experimental/filesystem>

CFontLibrary::CFontLibrary(const std::string& folder)
{
	m_fontFolder = folder;
}

//needs the gl context, otherwise can't cleanup correctly!
CFontLibrary::~CFontLibrary()
{
	std::cout << "CFontLibrary Destructor \n";

	m_fontList.clear();
	for(uint32_t i=0; i<m_glStringList.size(); i++)
	{
		if(m_glStringList.at(i)->m_vertexbuffer)
			glDeleteBuffers(1, &m_glStringList.at(i)->m_vertexbuffer);

		if(m_glStringList.at(i)->m_uvBuffer)
			glDeleteBuffers(1, &m_glStringList.at(i)->m_uvBuffer);
	}
	m_glStringList.clear();

	if(m_sdfShaderProgram)
		delete m_sdfShaderProgram;
	m_sdfShaderProgram = nullptr;

	if(m_defaultShaderProgram)
		delete m_defaultShaderProgram;
	m_defaultShaderProgram = nullptr;

	glDeleteVertexArrays(1, &m_vertexArrayID);
}

bool CFontLibrary::ParseAllFontsInFolder()
{
	//get a list of all fonts in font directory
	std::forward_list<std::string> fileNameList;
	//this is OS specific, change this Windows method for Linux or other method of your liking
	{
		//the following code was made available by C++17. Using Visual Studio 2017 you will need to use the epxerimental namespace,
		//later versions may be able to use std::filesystem directly
		std::string path = m_fontFolder;
		path.append("\\");
		std::string ext(".fnt");
		for(auto& p: std::experimental::filesystem::recursive_directory_iterator(path))
		{
			if(p.path().extension() == ext)
				fileNameList.push_front(p.path().filename().u8string());
		}
	}

	//parse each font individually and add the parsed font to the global font library list
	uint32_t i=0;
	for (auto it = fileNameList.cbegin(); it != fileNameList.cend(); it++, i++)
	{
		std::string curr = *it;
		char* currFileName = new char[m_fontFolder.length() + 2 + curr.length()];
		sprintf_s(currFileName, m_fontFolder.size() + curr.size() + 2, "%s\\%s", m_fontFolder.c_str(), curr.c_str());

		CGLFont* newFont = ParseFont(currFileName);
		//trim to correct name
		std::string fontName = curr;
		if(fontName.length() > 5)
			fontName = fontName.substr(0, fontName.length()-4);
		if(newFont != nullptr)
			m_fontList.emplace(fontName, newFont); //NOTE: only gets added if the font name is unique

		delete[] currFileName;
	}

	//check if we found at least one font
	if(i)
		return true;
	else
		return false;
}

CGLFont* CFontLibrary::ParseFont(const std::string& fileName)
{
	//Here we assume basic text format set by BMFont, or XML but not json

	CFontFileParser* parser = new CFontFileParser(fileName);
	if(!parser || !parser->IsInitialized())
		return nullptr; //ERROR

	CGLFont* newFont = new CGLFont();
	if(!newFont)
		return nullptr; //ERROR

	//now load font config file(*.fnt) -> see fileName parameter
	//define strings we are searching for
	std::string faceString("face=");
	std::string sizeString("size=");
	std::string boldString("bold=");
	std::string italicString("italic=");
	std::string unicodeString("unicode=");
	std::string textureWString("scaleW=");
	std::string textureHString("scaleH=");
	std::string textureNameString("file=");
	std::string baseString("base=");

	//==========================
	//load general font infos
	//==========================

	//load face name
	std::string faceVal;
	parser->GetValueFromBufferOfFirst(faceString, &faceVal);
	newFont->m_face = faceVal;

	//load font size
	std::string sizeVal;
	parser->GetValueFromBufferOfFirst(sizeString, &sizeVal);
	newFont->m_size = atoi(sizeVal.c_str());

	//load bold parameter
	std::string boldVal;
	parser->GetValueFromBufferOfFirst(boldString, &boldVal);
	newFont->m_bold = (bool)atoi(boldVal.c_str());

	//load italic parameter
	std::string italicVal;
	parser->GetValueFromBufferOfFirst(italicString, &italicVal);
	newFont->m_italic = (bool)atoi(italicVal.c_str());

	//load unicode parameter
	std::string unicodeVal;
	parser->GetValueFromBufferOfFirst(unicodeString, &unicodeVal);
	newFont->m_unicode = (bool)atoi(unicodeVal.c_str());

	//load texture height
	std::string textureHVal;
	parser->GetValueFromBufferOfFirst(textureHString, &textureHVal);
	newFont->m_textureH = atoi(textureHVal.c_str());

	//load texture width
	std::string textureWVal;
	parser->GetValueFromBufferOfFirst(textureWString, &textureWVal);
	newFont->m_textureW = atoi(textureWVal.c_str());

	//load texture width
	std::string textureNameVal;
	parser->GetValueFromBufferOfFirst(textureNameString, &textureNameVal);

	char* currFileName = new char[m_fontFolder.length() + 2 + textureNameVal.length()];
	sprintf_s(currFileName, m_fontFolder.size() + textureNameVal.size() + 2, "%s\\%s", m_fontFolder.c_str(), textureNameVal.c_str());
	newFont->m_bitmapFileName = currFileName;

	//load base
	std::string baseVal;
	parser->GetValueFromBufferOfFirst(baseString, &baseVal);
	newFont->m_base = atoi(baseVal.c_str());

	//==========================
	//load distance field infos
	//==========================
	std::string fieldTypeString("fieldType=");

	std::string fieldVal;
	parser->GetValueFromBufferOfFirst(fieldTypeString, &fieldVal);
	if(fieldVal.compare("msdf") == 0)
		newFont->m_sdfType = SDF_MULTI;
	else if(fieldVal.compare("sdf") == 0)
		newFont->m_sdfType = SDF_SINGLE;
	//else its not an SDF, which is the default, so no need to set it again here

	if(newFont->m_sdfType == SDF_MULTI)
	{
		std::string distanceRangeString("distanceRange=");
		std::string rangeVal;
		parser->GetValueFromBufferOfFirst(distanceRangeString, &rangeVal);
		newFont->m_sdfRange = atoi(rangeVal.c_str());
	}

	//==========================
	//load char infos
	//==========================
	std::string distanceRangeString("chars count=");
	std::string numCharsVal;
	parser->GetValueFromBufferOfFirst(distanceRangeString, &numCharsVal);
	int32_t length = atoi(numCharsVal.c_str());

	if(length <= 0)
	{
		delete parser;
		delete newFont;
		return nullptr;
	}
	uint32_t highestChar = parser->GetHighestSupportedChar();
	newFont->m_fontCharInfo = new CCharInfo*[highestChar+1];
	newFont->m_highestASCIIChar = highestChar;
	parser->LoadCharInfos(newFont);

	//==========================
	//load kerning infos
	//==========================

	newFont->m_kernings = nullptr;
	uint32_t first = parser->GetKerningsFirstHighest();
	uint32_t second = parser->GetKerningsSecondHighest();
	newFont->m_highestKerningFirst = first;
	newFont->m_highestKerningSecond = second;
	if(first <= 0 || second <=0)
	{
		delete parser;
		return newFont;
	}

	//create 2-dimensional kernings array
	newFont->m_kernings = new float*[first+1];
	for(uint16_t i=0; i <= first; i++)
		newFont->m_kernings[i] = new float[second+1];

	//now lets fill the kernins array with correct values
	parser->LoadKernings(newFont);

	//==========================
	//done loading parameters
	//==========================

	//clear the parser memory
	delete parser;
	return newFont;
}

//returns false on errors, true otherwise
//only call this function once per instance!
bool CFontLibrary::InitGLFonts(bool compressNonSDFTextures/* = false*/)
{
	bool success = true;

	//go through all fonts
	for(auto& it: m_fontList)
	{
		//load texture for current font
		CGLFont* currFont = it.second;
		if(!currFont)
		{
			success = false;
			continue;
		}

		bool compressCurrTex = currFont->m_sdfType == 0 ? compressNonSDFTextures : false;

		if(!CGLTexture::LoadTextureFromFile(currFont->m_bitmapFileName, currFont->m_fontTextures, compressCurrTex))
		{
			std::string err = "Font not found or could not load font texture.";
			std::cout << err;
			success = false;
			continue;
		}
	}

	std::string vertexShaderCode =
	#include "Default_Font.vert"
	std::string fragmentShaderCode =
	#include "Default_Font.frag"

	m_defaultShaderProgram = new CGLShaderProgram();

	if (!m_defaultShaderProgram || !m_defaultShaderProgram->InitFromString(vertexShaderCode, fragmentShaderCode))
	{
		std::string err = "FontLibrary failed to load default shader.";
		std::cout << err;
		success = false;
	}

	vertexShaderCode =
	#include "SDF_Font.vert"
	fragmentShaderCode =
	#include "SDF_Font.frag"

	m_sdfShaderProgram = new CGLShaderProgram();

	if (!m_sdfShaderProgram || !m_sdfShaderProgram->InitFromString(vertexShaderCode, fragmentShaderCode))
	{
		std::string err = "FontLibrary failed to load sdf shader.";
		std::cout << err;
		success = false;
	}

	glGenVertexArrays(1, &m_vertexArrayID); // Create our Vertex Array Object

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return success;
}

void CFontLibrary::DrawString(const std::string& textToDraw, int16_t x, int16_t y, GLfloat color[4], const std::string& font, uint16_t winW, uint16_t winH, float scale, uint16_t outline, GLfloat outlineColor[4], GLfloat bgColor[4])
{
	CDrawString* volatileString = new CDrawString(0, font, textToDraw, x, y, color, scale);
	volatileString->m_winH = winH;
	volatileString->m_winW = winW;
	if(bgColor)
	{
		volatileString->m_bgColor[0] = bgColor[0];
		volatileString->m_bgColor[1] = bgColor[1];
		volatileString->m_bgColor[2] = bgColor[2];
		volatileString->m_bgColor[3] = bgColor[3];
	}
	PopulateVertexBuffers(volatileString);

	DrawTriangles(font, color, volatileString, outline, outlineColor, bgColor);
	delete volatileString;
}

void CFontLibrary::PopulateVertexBuffers(CDrawString* stringObject)
{
	if(!stringObject)
		return; //ERROR

	if(!stringObject->m_needsChange)
		return;

	uint16_t lineHeight = GetLineHeight(stringObject->m_font);
	uint16_t line = 0;

	uint32_t numQuads = stringObject->m_text.length();
	CGLQuad2D* quadList = new CGLQuad2D[numQuads];
	uint32_t qi = 0;
	for(uint32_t index = 0; index < numQuads;)
	{
		if(stringObject->m_maxLines > 0 && line >= stringObject->m_maxLines) //lets only draw the number of lines defind my "maxLines"
			break;

		float textWidth = 0.0f;
		std::string subStringToDraw;
		if(stringObject->m_maxLines == 0) //we don't care about line breaks
			subStringToDraw = stringObject->m_text;

		while(textWidth < stringObject->m_lineWidth && index < stringObject->m_text.length())
		{
			subStringToDraw.push_back(stringObject->m_text.at(index++));
			textWidth = GetWidthOfString(subStringToDraw, stringObject->m_font, stringObject->m_scale);
		}

		CGLQuad2D* tempQuadList = TextToQuadList(stringObject->m_font, subStringToDraw, stringObject->m_x, stringObject->m_y - (int16_t)(line++*lineHeight*stringObject->m_scale), stringObject->m_scale);

		for(uint32_t l = 0; l<subStringToDraw.size(); l++)
		{
			quadList[qi++] = tempQuadList[l];
		}

		delete[] tempQuadList;

		if(stringObject->m_maxLines == 0) //we don't care about line breaks
			break;
	}

	uint16_t numVerticies = numQuads*6;

	GLfloat* vertexBufferPositions = new GLfloat[numVerticies*2]; //each quad has 4 verticies á 2 coordinate values: x,y, meaning 8 entries per char total
	GLfloat* vertexBufferUV = new GLfloat[numVerticies*2]; //each quad has four UV coordinates per vertex: u,v meaning 8 entries per char total

	uint16_t winW = stringObject->m_winW;
	uint16_t winH = stringObject->m_winH;
	
	for(uint16_t i=0; i<numQuads; i++)
	{
		uint16_t vertexIndex = i*12;
		uint16_t uvIndex = i*12;

		////////first triangle of quad

		//bottom left vertex
		vertexBufferPositions[vertexIndex] = ((quadList[i].bottomLeftX)/(float)winW)*2.0f - 1.0f;
		vertexBufferPositions[vertexIndex+1] = ((quadList[i].bottomLeftY)/(float)winH)*2.0f - 1.0f;
		vertexBufferUV[uvIndex] = quadList[i].textureBottomLeftX;
		vertexBufferUV[uvIndex+1] = quadList[i].textureBottomLeftY;

		//top left vertex
		vertexBufferPositions[vertexIndex+2] = ((quadList[i].topLeftX)/(float)winW)*2.0f - 1.0f;
		vertexBufferPositions[vertexIndex+2+1] = ((quadList[i].topLeftY)/(float)winH)*2.0f - 1.0f;
		vertexBufferUV[uvIndex+2] = quadList[i].textureTopLeftX;
		vertexBufferUV[uvIndex+2+1] = quadList[i].textureTopLeftY;

		//bottom right vertex
 		vertexBufferPositions[vertexIndex+4] = ((quadList[i].bottomRightX)/(float)winW)*2.0f - 1.0f;
 		vertexBufferPositions[vertexIndex+4+1] = ((quadList[i].bottomRightY)/(float)winH)*2.0f - 1.0f;
 		vertexBufferUV[uvIndex+4] = quadList[i].textureBottomRightX;
 		vertexBufferUV[uvIndex+4+1] = quadList[i].textureBottomRightY;

		////////second triangle of quad

		//top left vertex
		vertexBufferPositions[vertexIndex+6] = ((quadList[i].topLeftX)/(float)winW)*2.0f - 1.0f;
		vertexBufferPositions[vertexIndex+6+1] = ((quadList[i].topLeftY)/(float)winH)*2.0f - 1.0f;
		vertexBufferUV[uvIndex+6] = quadList[i].textureTopLeftX;
		vertexBufferUV[uvIndex+6+1] = quadList[i].textureTopLeftY;

		//bottom right vertex
		vertexBufferPositions[vertexIndex+8] = ((quadList[i].bottomRightX)/(float)winW)*2.0f - 1.0f;
		vertexBufferPositions[vertexIndex+8+1] = ((quadList[i].bottomRightY)/(float)winH)*2.0f - 1.0f;
		vertexBufferUV[uvIndex+8] = quadList[i].textureBottomRightX;
		vertexBufferUV[uvIndex+8+1] = quadList[i].textureBottomRightY;

		//top right vertex
		vertexBufferPositions[vertexIndex+10] = ((quadList[i].topRightX)/(float)winW)*2.0f - 1.0f;
		vertexBufferPositions[vertexIndex+10+1] = ((quadList[i].topRightY)/(float)winH)*2.0f - 1.0f;
		vertexBufferUV[uvIndex+10] = quadList[i].textureTopRightX;
		vertexBufferUV[uvIndex+10+1] = quadList[i].textureTopRightY;
	}
 
	stringObject->m_numVerticies = numVerticies;

	glBindVertexArray(m_vertexArrayID); // Bind our Vertex Array Object so we can use it

	if(!stringObject->m_vertexbuffer)
		glGenBuffers(1, &stringObject->m_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, stringObject->m_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, numVerticies*2*sizeof(GLfloat), vertexBufferPositions, GL_DYNAMIC_DRAW); // Give our vertices to OpenGL.

	if(!stringObject->m_uvBuffer)
		glGenBuffers(1, &stringObject->m_uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, stringObject->m_uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, numVerticies*2*sizeof(GLfloat), vertexBufferUV, GL_DYNAMIC_DRAW); // Give our uv's to OpenGL.

	stringObject->m_needsChange = false;

	delete[] vertexBufferPositions;
	delete[] vertexBufferUV;
	delete[] quadList;
}

void CFontLibrary::DrawString(uint16_t ID, const std::string& textToDraw, int16_t x, int16_t y, GLfloat color[4], const std::string& font, uint16_t winW, uint16_t winH, float scale, uint16_t outline , GLfloat outlineColor[4], GLfloat bgColor[4])
{
	std::map<uint16_t, CDrawString*>::const_iterator found = m_glStringList.find(ID);
	CDrawString* savedString = nullptr;
	if(found != m_glStringList.end() && found->second)
	{
		savedString = found->second;

		if(savedString->m_text.compare(textToDraw.substr(0, MAX_STRING_LENGTH)) != 0 || 
			(savedString->m_x != x || savedString->m_y != y) ||
			(color[0] != savedString->m_color[0] || color[1] != savedString->m_color[1] || color[2] != savedString->m_color[2] || color[3] != savedString->m_color[3]) ||
			(bgColor && (bgColor[0] != savedString->m_bgColor[0] || bgColor[1] != savedString->m_bgColor[1] || bgColor[2] != savedString->m_bgColor[2] || bgColor[3] != savedString->m_bgColor[3])) ||
			(scale != savedString->m_scale) ||
			(winW != savedString->m_winW) || 
			(winH != savedString->m_winH) || 
			(outline != savedString->m_outline) && (outlineColor[0] != savedString->m_outlineColor[0] || outlineColor[1] != savedString->m_outlineColor[1] || outlineColor[2] != savedString->m_outlineColor[2] || outlineColor[3] != savedString->m_outlineColor[3])
			)
		{
			savedString->m_color[0] = color[0];
			savedString->m_color[1] = color[1];
			savedString->m_color[2] = color[2];
			savedString->m_color[3] = color[3];
			if(bgColor)
			{
				savedString->m_bgColor[0] = bgColor[0];
				savedString->m_bgColor[1] = bgColor[1];
				savedString->m_bgColor[2] = bgColor[2];
				savedString->m_bgColor[3] = bgColor[3];
			}
			savedString->m_outline = outline;
			if(outline)
			{
				savedString->m_outlineColor[0] = outlineColor[0];
				savedString->m_outlineColor[1] = outlineColor[1];
				savedString->m_outlineColor[2] = outlineColor[2];
				savedString->m_outlineColor[3] = outlineColor[3];
			}
			savedString->m_x = x;
			savedString->m_y = y;
			savedString->m_scale = scale;
			savedString->m_text = textToDraw;
			savedString->m_winW = winW;
			savedString->m_winH = winH;
			savedString->m_font = font;
			savedString->m_needsChange = true;
		}
	}
	else
	{
		savedString = new CDrawString(ID, font, textToDraw, x, y, color, scale);
		savedString->m_winH = winH;
		savedString->m_winW = winW;

		if(bgColor)
		{
			savedString->m_bgColor[0] = bgColor[0];
			savedString->m_bgColor[1] = bgColor[1];
			savedString->m_bgColor[2] = bgColor[2];
			savedString->m_bgColor[3] = bgColor[3];
		}
		if(outline)
		{
			savedString->m_outline = outline;
			savedString->m_outlineColor[0] = outlineColor[0];
			savedString->m_outlineColor[1] = outlineColor[1];
			savedString->m_outlineColor[2] = outlineColor[2];
			savedString->m_outlineColor[3] = outlineColor[3];
		}
	}

	if(found == m_glStringList.end() && savedString)
		m_glStringList.emplace(ID, savedString);

	PopulateVertexBuffers(savedString);

	DrawTriangles(font, color, savedString, outline, outlineColor, bgColor);
}

//draws the string until lineWidth(pixels) then cuts it off there and draws the rest underneath and so forth till the last character in the string has been drawn
//draws only the text up to the specified line, if maxLines parameter is zero, it means there is no limit
void CFontLibrary::DrawStringWithLineBreaks(uint16_t ID, const std::string& textToDraw, int16_t x, int16_t y, GLfloat color[4], const std::string& font, uint16_t winW, uint16_t winH, float scale, uint16_t lineWidth, uint16_t maxLines, uint16_t outline, GLfloat outlineColor[4])
{
	std::map<uint16_t, CDrawString*>::const_iterator found = m_glStringList.find(ID);
	CDrawString* savedString = nullptr;
	if(found != m_glStringList.end() && found->second)
	{
		savedString = found->second;

		if(savedString->m_text.compare(textToDraw.substr(0, MAX_STRING_LENGTH)) != 0 || 
			(savedString->m_x != x || savedString->m_y != y) ||
			(color[0] != savedString->m_color[0] || color[1] != savedString->m_color[1] || color[2] != savedString->m_color[2] || color[3] != savedString->m_color[3]) ||
			(scale != savedString->m_scale) ||
			(maxLines != savedString->m_maxLines) ||
			(lineWidth != savedString->m_lineWidth) ||
			(winW != savedString->m_winW) || 
			(winH != savedString->m_winH) ||
			(outline != savedString->m_outline) && (outlineColor[0] != savedString->m_outlineColor[0] || outlineColor[1] != savedString->m_outlineColor[1] || outlineColor[2] != savedString->m_outlineColor[2] || outlineColor[3] != savedString->m_outlineColor[3])
			)
		{
			savedString->m_outline = outline;
			if(outline)
			{
				savedString->m_outlineColor[0] = outlineColor[0];
				savedString->m_outlineColor[1] = outlineColor[1];
				savedString->m_outlineColor[2] = outlineColor[2];
				savedString->m_outlineColor[3] = outlineColor[3];
			}
			savedString->m_color[0] = color[0];
			savedString->m_color[1] = color[1];
			savedString->m_color[2] = color[2];
			savedString->m_color[3] = color[3];
			savedString->m_x = x;
			savedString->m_y = y;
			savedString->m_scale = scale;
			savedString->m_text = textToDraw;
			savedString->m_lineWidth = lineWidth;
			savedString->m_maxLines = maxLines;
			savedString->m_winW = winW;
			savedString->m_winH = winH;
			savedString->m_font = font;
			savedString->m_needsChange = true;
		}
	}
	else
	{
		savedString = new CDrawString(ID, font, textToDraw, x, y, color, scale);
		savedString->m_lineWidth = lineWidth;
		savedString->m_maxLines = maxLines;
		if(outline)
		{
			savedString->m_outline = outline;
			savedString->m_outlineColor[0] = outlineColor[0];
			savedString->m_outlineColor[1] = outlineColor[1];
			savedString->m_outlineColor[2] = outlineColor[2];
			savedString->m_outlineColor[3] = outlineColor[3];
		}
	}

	if(found == m_glStringList.end() && savedString)
		m_glStringList.emplace(ID, savedString);

	PopulateVertexBuffers(savedString);

	DrawTriangles(font, color, savedString, outline, outlineColor);
}

//draws the string until lineWidth(pixels) then cuts it off there and draws the rest underneath and so forth till the last character in the string has been drawn
//draws only the text up to the specified line, if maxLines parameter is zero, it means there is no limit
void CFontLibrary::DrawStringWithLineBreaks(const std::string& textToDraw, int16_t x, int16_t y, GLfloat color[4], const std::string& font, uint16_t winW, uint16_t winH, float scale, uint16_t lineWidth, uint16_t maxLines, uint16_t outline, GLfloat outlineColor[4])
{
	CDrawString* volatileString = new CDrawString(0, font, textToDraw, x, y, color, scale);
	volatileString->m_lineWidth = lineWidth;
	volatileString->m_maxLines = maxLines;
	volatileString->m_winH = winH;
	volatileString->m_winW = winW;

	PopulateVertexBuffers(volatileString);

	DrawTriangles(font, color, volatileString);
	delete volatileString;
}

uint16_t CFontLibrary::GetLineHeight(const std::string& font)
{
	uint16_t retVal = 0;

	CGLFont* currfont = GetFontPointer(font);

	if(!currfont)
	{
		std::cout << "CFontLibrary::Font not found! " << font;
		return 0;
	}

	char ascii = 'W'; //very tall character, we use this as representative for the entire font

	if(currfont->m_fontCharInfo[ascii])
		retVal = currfont->m_fontCharInfo[ascii]->m_textureWidth + 2; //very tall character + 2 comes line height very close

	return retVal;
}

// For internal use only
float CFontLibrary::AdjustForKerningPairs(const std::string& font, const std::string& textToDraw, uint16_t first, char second, float scale)
{
	uint32_t ascii1 = GetTextChar(textToDraw, first);
	uint32_t ascii2 = GetTextChar(textToDraw, second);
	if( ascii1 <= 0 || ascii2 <= 0 ) return 0.0f;

	CGLFont* currFont = GetFontPointer(font);
	if(!currFont) return 0.0f;

	if(ascii1 <= currFont->m_highestKerningFirst && ascii2 <= currFont->m_highestKerningSecond)
		return currFont->m_kernings[ascii1][ascii2];

	return 0.0f;
}

float CFontLibrary::GetWidthOfString(const std::string& textToDraw, const std::string& font, float scale, bool ignoreKerning)
{
	float retVal = 0.0f;

	for(unsigned int i=0; i<textToDraw.length(); i++)
	{
		retVal+=GetWidthOfChar(textToDraw.at(i), font)*scale;

		if(!ignoreKerning)
			if(i<textToDraw.length()-1)
				retVal += AdjustForKerningPairs(font, textToDraw, i, i+1, scale);
	}

	return retVal;
}

uint16_t CFontLibrary::GetWidthOfChar(char ch, const std::string& font)
{
	uint16_t retVal = 0;

	//get the pointer to the selected font
	CGLFont* currfont = GetFontPointer(font);

	if(!currfont)
	{
		std::cout << "CFontLibrary::Font not found! " << font;
		return 0;
	}

	std::string stringToTest;
	stringToTest.push_back(ch);

	uint32_t ascii = GetTextChar(stringToTest, 0);
	if(ascii >= 0 && ascii < (int)currfont->m_highestASCIIChar) //sanity check
		retVal = currfont->m_fontCharInfo[ascii]->m_xadvance;

	return retVal;
}

CGLQuad2D* CFontLibrary::TextToQuadList(const std::string& font, const std::string& textToDraw, int16_t x, int16_t y, float scale)
{
	//get pointer to the correct font
	CGLFont* currfont = GetFontPointer(font);

	if(!currfont)
	{
		std::cout << "CFontLibrary::Font not found! " << font;
		return nullptr;
	}

	CGLQuad2D* quadList = new CGLQuad2D[textToDraw.length()];

	int32_t defaultChar = 63; //'?', default for unknown characters is the question mark

	float cursor = 0;

	for(uint32_t i=0; i<textToDraw.length(); i++)
	{
		uint32_t ascii = GetTextChar(textToDraw, i);

		//check if current character is in our font
		// if not, then set it to the default char
		if(ascii <= currfont->m_highestASCIIChar)
		{
			if(currfont->m_fontCharInfo[ascii] == nullptr)
				ascii = defaultChar;
		}
		else
			ascii = defaultChar;

		//get some infos about the current char
		uint16_t charX = currfont->m_fontCharInfo[ascii]->m_textureX;
		uint16_t charY = currfont->m_fontCharInfo[ascii]->m_textureY;
		uint16_t width = currfont->m_fontCharInfo[ascii]->m_textureWidth;
		uint16_t height = currfont->m_fontCharInfo[ascii]->m_textureHeight;
		int16_t offsetX = currfont->m_fontCharInfo[ascii]->m_xoffset;
		int16_t offsetY = currfont->m_fontCharInfo[ascii]->m_yoffset;
		uint16_t base = currfont->m_base;

		//top left
 		quadList[i].textureTopLeftX = (float) charX / (float) currfont->m_textureW;
 		quadList[i].textureTopLeftY = (float) charY / (float) currfont->m_textureH;
		quadList[i].topLeftX = cursor + ((offsetX)*scale) + x;
		quadList[i].topLeftY = y + ((base - offsetY)*scale);
 
 		//top right
 		quadList[i].textureTopRightX = (float) (charX+width) / (float) currfont->m_textureW;
 		quadList[i].textureTopRightY = (float) charY / (float) currfont->m_textureH;
		quadList[i].topRightX = ((width + offsetX)*scale) + x + cursor;
		quadList[i].topRightY = y + ((base - offsetY)*scale);
 
 		//bottom right
 		quadList[i].textureBottomRightX = (float) (charX+width) / (float) currfont->m_textureW;
 		quadList[i].textureBottomRightY = (float) (charY+height) / (float) currfont->m_textureH;
		quadList[i].bottomRightX = ((width + offsetX)*scale) + x + cursor;
		quadList[i].bottomRightY = y + ((base - offsetY - height)*scale);
 
 		//bottom left
 		quadList[i].textureBottomLeftX = (float) charX / (float) currfont->m_textureW;
 		quadList[i].textureBottomLeftY = (float) (charY+height) / (float) currfont->m_textureH;
		quadList[i].bottomLeftX = ((offsetX)*scale) + x + cursor;
		quadList[i].bottomLeftY = y + ((base - offsetY - height)*scale);

		//calc kerning depending on current and next character in the string to draw
		float kerning = 0.0f;
		if(i>0 && i<textToDraw.length()-1)
		{
			if(ascii <= currfont->m_highestKerningFirst)
			{
				uint32_t asciiNext = textToDraw[i+1];
				if(asciiNext <= currfont->m_highestKerningSecond)
					kerning = currfont->m_kernings[ascii][asciiNext];
			}
		}

		//get cursor position in x-direction for next character in string
		cursor += ((float)currfont->m_fontCharInfo[ascii]->m_xadvance + kerning)*scale;
	}

	return quadList;
}

uint32_t CFontLibrary::GetTextChar(const std::string& textToDraw, uint32_t pos)
{
	uint32_t ch = (int32_t)textToDraw.at(pos);
	if(ch < 0) //UTF8 conversion did not work, we will attempt UTF16 conversion
	{
		uint32_t len = 0;
		char text = textToDraw.at(pos);
		ch = acUtility::DecodeUTF16(&text, &len);
	}

	return ch;
}

void CFontLibrary::DrawTriangles(const std::string& font, float color[4], CDrawString* stringObject, uint16_t outline/* = 0*/, GLfloat outlineColor[4]/* = {0}*/, GLfloat bgColor[4]/* = {0}*/)
{
	CGLFont* currFont = GetFontPointer(font);
	if (!currFont || !stringObject)
		return;

	glDisable(GL_DEPTH_TEST);

	uint32_t textureID = currFont->m_fontTextures;

	if(textureID > 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
	}

	glBindVertexArray(m_vertexArrayID); // Bind our Vertex Array Object so we can use it

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, stringObject->m_vertexbuffer); // This will talk about our 'vertexbuffer' buffer

	glVertexAttribPointer(
		0,					// attribute 0. No particular reason for 0, but must match the layout in the shader.
		2,					// size
		GL_FLOAT,			// type
		GL_FALSE,			// normalized?
		0,					// stride, can be 0 for tightly packed array, or user specified: 2*sizeof(GLfloat)
		(void*)0			// array buffer offset
	);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, stringObject->m_uvBuffer); // This will talk about our 'vertexbuffer' buffer

	glVertexAttribPointer(
		1,					// attribute 1. No particular reason for 1, but must match the layout in the shader.
		2,					// size
		GL_FLOAT,			// type
		GL_FALSE,			// normalized?
		0,					// stride, can be 0 for tightly packed array, or user specified: 2*sizeof(GLfloat)
		(void*)0			// array buffer offset
	);

	GLuint programID = 0;

	if(currFont->m_sdfType != SDF_NO && m_sdfShaderProgram && m_sdfShaderProgram->GetProgramID() > 0)
	{
		programID = m_sdfShaderProgram->GetProgramID();
		glUseProgram(programID);

		GLint textureLocation = glGetUniformLocation(programID, "u_FontTexture");
		if(textureLocation >= 0)
			glUniform1i(textureLocation, 0);

		GLint sdfLoc = glGetUniformLocation(programID, "sdfType");
		if(sdfLoc >= 0)
			glUniform1i(sdfLoc, currFont->m_sdfType);

		GLint fgColorLoc = glGetUniformLocation(programID, "fgColor");
		if(fgColorLoc >= 0)
			glUniform4f(fgColorLoc, color[0], color[1], color[2], color[3]);

		GLint outlineLoc = glGetUniformLocation(programID, "outline");
		if(outlineLoc >= 0)
			glUniform1i(outlineLoc, outline);

		if(outline)
		{
			GLint outlineColorLoc = glGetUniformLocation(programID, "outlineColor");
			if(outlineColorLoc >= 0)
				glUniform4f(outlineColorLoc, outlineColor[0], outlineColor[1], outlineColor[2], outlineColor[3]);
		}

		if(currFont->m_sdfType == SDF_MULTI)
		{
			GLint pxLoc = glGetUniformLocation(programID, "pxRange");
			if(pxLoc >= 0)
				glUniform1f(pxLoc, (float)currFont->m_sdfRange);

			GLint bgColorLoc = glGetUniformLocation(programID, "bgColor");
			if(bgColorLoc >= 0)
			{
				if(bgColor)
					glUniform4f(bgColorLoc, bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
				else
					glUniform4f(bgColorLoc, 0.0, 0.0, 0.0, 0.0);
			}
		}
	}
	else
	{
		programID = m_defaultShaderProgram->GetProgramID();
		glUseProgram(programID);
		
		GLint textureLocation = glGetUniformLocation(programID, "u_FontTexture");
		if(textureLocation >= 0)
			glUniform1i(textureLocation, 0);

		GLint fgColorLoc = glGetUniformLocation(programID, "fgColor");
		if(fgColorLoc >= 0)
			glUniform4f(fgColorLoc, color[0], color[1], color[2], color[3]);
	}

	// Draw the thing
	glDrawArrays(GL_TRIANGLES, 0, stringObject->m_numVerticies); // Starting from vertex 0 to vertices total

	//cleanup
	glUseProgram(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0); // Unbind our Vertex Array Object

	if(textureID > 0)
		glBindTexture(GL_TEXTURE_2D, 0); //unbind texture

	glEnable(GL_DEPTH_TEST);
}

CGLFont* CFontLibrary::GetFontPointer(const std::string& fontName)
{
	std::map<std::string, CGLFont*>::const_iterator found = m_fontList.find(fontName);
	if (found != m_fontList.end())
		return found->second;
	else
		return nullptr;
}

uint16_t CFontLibrary::GetNewDrawStringID()
{
	return m_IDCounter++;
}
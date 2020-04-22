//=================================================================================
// Name			:	FontLibrary.cpp
// Author		:	Christian Thurow
// Date			:	March 30th 2020
// Description	:	Main Implementation file CFontLibrary
//=================================================================================

#include "FontFileParser.hpp"
#include "FontLibrary.hpp"
#include "Font.hpp"
#include "Texture.hpp"
#include "acutil_unicode.hpp"
#include <windows.h>
#include <fstream>
#include <iostream>
#include <experimental/filesystem>

CFontLibrary::CFontLibrary(std::string folder)
{
	m_fontFolder = folder;
}

CFontLibrary::~CFontLibrary()
{
	std::cout << "CFontLibrary Destructor \n";

	m_fontList.clear();
	m_glStringList.clear();

	if(m_sdfShaderProgram)
		delete m_sdfShaderProgram;
	m_sdfShaderProgram = nullptr;
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
	int i=0;
	for (auto it = fileNameList.cbegin(); it != fileNameList.cend(); it++, i++)
	{
		std::string curr = *it;
		char* currFileName = new char[m_fontFolder.length() + 2 + curr.length()];
		sprintf(currFileName, "%s\\%s", m_fontFolder.c_str(), curr.c_str());

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

CGLFont* CFontLibrary::ParseFont(std::string fileName)
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
	sprintf(currFileName, "%s\\%s", m_fontFolder.c_str(), textureNameVal.c_str());
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
	unsigned int length = atoi(numCharsVal.c_str());

	if(length <= 0)
	{
		delete parser;
		delete newFont;
		return nullptr;
	}
	unsigned int highestChar = parser->GetHighestSupportedChar();
	newFont->m_fontCharInfo = new CCharInfo*[highestChar+1];
	newFont->m_highestASCIIChar = highestChar;
	parser->LoadCharInfos(newFont);

	//==========================
	//load kerning infos
	//==========================

	newFont->m_kernings = nullptr;
	int first = parser->GetKerningsFirstHighest();
	int second = parser->GetKerningsSecondHighest();
	newFont->m_highestKerningFirst = first;
	newFont->m_highestKerningSecond = second;
	if(first <= 0 || second <=0)
	{
		delete parser;
		return newFont;
	}

	//create 2-dimensional kernings array
	newFont->m_kernings = new int*[first+1];
	for(int i=0; i <= first; i++)
		newFont->m_kernings[i] = new int[second+1];

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
bool CFontLibrary::InitGLFonts()
{
	bool error = false;
	//go through all fonts
	for(auto& it: m_fontList)
	{
		//load texture for current font
		CGLFont* currFont = it.second;

		if(!currFont || !CGLTexture::LoadTextureFromFile(currFont->m_bitmapFileName, currFont->m_fontTextures))
		{
			//TODO: log error
			error = true;
			continue;
		}
	}

	std::string vertexShaderCode =
	#include "SDF_Font.vert"
	std::string fragmentShaderCode =
	#include "SDF_Font.frag"

	m_sdfShaderProgram = new CGLShaderProgram();

	if (!m_sdfShaderProgram || !m_sdfShaderProgram->InitFromString(vertexShaderCode, fragmentShaderCode))
	{
		std::string err = "FontLibrary failed to load sdf shader.";
		//TODO: log error
		error = true;
	}

	return error;
}

void CFontLibrary::DrawString(std::string textToDraw, int x, int y, float color[4], std::string font, float scale)
{
	CGLQuad2D* quadList = TextToQuadList(font, textToDraw, x, y, scale);
	if(quadList)
	{
		DrawQuadList(font, color, quadList, textToDraw);
		delete[] quadList;
	}
}

void CFontLibrary::DrawString(unsigned int ID, std::string textToDraw, int x, int y, float color[4], std::string font, float scale)
{
	std::map<unsigned int, CDrawString*>::const_iterator found = m_glStringList.find(ID);
	CDrawString* savedString = nullptr;
	if(found != m_glStringList.end() && found->second)
	{
		savedString = found->second;

		if(savedString->m_text.compare(textToDraw) != 0 || 
			(savedString->m_x != x || savedString->m_y != y) ||
			(color[0] != savedString->m_color[0] || color[1] != savedString->m_color[1] || color[2] != savedString->m_color[2] || color[3] != savedString->m_color[3]) ||
			(scale != savedString->m_scale)
			)
		{
			if(savedString->m_drawListID)
				glDeleteLists(savedString->m_drawListID, 1);
			savedString->m_drawListID = 0;

			savedString->m_color[0] = color[0];
			savedString->m_color[1] = color[1];
			savedString->m_color[2] = color[2];
			savedString->m_color[3] = color[3];
			savedString->m_x = x;
			savedString->m_y = y;
			savedString->m_scale = scale;
			savedString->m_text = textToDraw;
		}
	}
	else
	{
		savedString = new CDrawString(ID, textToDraw, x, y, color, scale);
	}

	if(savedString && savedString->m_drawListID == 0)
	{
		GLuint id = glGenLists(1);
		glNewList(id, GL_COMPILE);

		DrawString(textToDraw, x, y, color, font, scale);

		glEndList();
		savedString->m_drawListID = id;
	}
	if(savedString && savedString->m_drawListID)
		glCallList(savedString->m_drawListID);

	if(found == m_glStringList.end() && savedString)
		m_glStringList.emplace(ID, savedString);
}

//draws the string until lineWidth(pixels) then cuts it off there and draws the rest underneath and so forth till the last character in the string has been drawn
//draws only the text up to the specified line, if maxLines parameter is zero, it means there is no limit
void CFontLibrary::DrawStringWithLineBreaks(unsigned int ID, std::string textToDraw, int x, int y, float color[4], std::string font, float scale, int lineWidth, int maxLines)
{
	std::map<unsigned int, CDrawString*>::const_iterator found = m_glStringList.find(ID);
	CDrawString* savedString = nullptr;
	if(found != m_glStringList.end() && found->second)
	{
		savedString = found->second;

		if(savedString->m_text.compare(textToDraw) != 0 || 
			(savedString->m_x != x || savedString->m_y != y) ||
			(color[0] != savedString->m_color[0] || color[1] != savedString->m_color[1] || color[2] != savedString->m_color[2] || color[3] != savedString->m_color[3]) ||
			(scale != savedString->m_scale) ||
			(maxLines != savedString->m_maxLines) ||
			(lineWidth != savedString->m_lineWidth))
		{
			if(savedString->m_drawListID)
				glDeleteLists(savedString->m_drawListID, 1);
			savedString->m_drawListID = 0;

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
		}
	}
	else
	{
		savedString = new CDrawString(ID, textToDraw, x, y, color, scale);
		savedString->m_lineWidth = lineWidth;
		savedString->m_maxLines = maxLines;
	}

	if(savedString && savedString->m_drawListID == 0)
	{
		GLuint id = glGenLists(1);
		glNewList(id, GL_COMPILE);

		DrawStringWithLineBreaks(textToDraw, x, y, color, font, scale, lineWidth, maxLines);

		glEndList();
		savedString->m_drawListID = id;
	}
	if(savedString && savedString->m_drawListID)
		glCallList(savedString->m_drawListID);

	if(found == m_glStringList.end() && savedString)
		m_glStringList.emplace(ID, savedString);
}

//draws the string until lineWidth(pixels) then cuts it off there and draws the rest underneath and so forth till the last character in the string has been drawn
//draws only the text up to the specified line, if maxLines parameter is zero, it means there is no limit
void CFontLibrary::DrawStringWithLineBreaks(std::string textToDraw, int x, int y, float color[4], std::string font, float scale, int lineWidth, int maxLines)
{
	unsigned int lineHeight = GetLineHeight(font);
	unsigned int line = 0;
	for(int index = 0; index < textToDraw.length();)
	{
		if(maxLines > 0 && line >= maxLines) //lets only draw the number of lines defind my "maxLines"
			return;

		std::string subStringToDraw;
		int textWidth = 0;
		while(textWidth < lineWidth && index < textToDraw.length())
		{
			subStringToDraw.push_back(textToDraw.at(index++));
			textWidth = GetWidthOfString(subStringToDraw, font, scale);
		}

		DrawString(subStringToDraw, x, y - line++*lineHeight*scale, color, font, scale);
	}
}

unsigned int CFontLibrary::GetLineHeight(std::string font)
{
	unsigned int retVal = 0;

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

float CFontLibrary::GetWidthOfString(std::string textToDraw, std::string font, float scale)
{
	float retVal = 0;

	for(int i=0; i<textToDraw.length(); i++)
	{
		retVal+=GetWidthOfChar(textToDraw.at(i), font);

		//TODO: account for kernings!
	}

	retVal *= scale;

	return retVal;
}

unsigned int CFontLibrary::GetWidthOfChar(char ch, std::string font)
{
	unsigned int retVal = 0;

	//get the pointer to the selected font
	CGLFont* currfont = GetFontPointer(font);

	if(!currfont)
	{
		std::cout << "CFontLibrary::Font not found! " << font;
		return 0;
	}

	std::string stringToTest;
	stringToTest.push_back(ch);

	int ascii = GetTextChar(stringToTest, 0);
	if(ascii >= 0 && ascii < currfont->m_highestASCIIChar) //sanity check
		retVal = currfont->m_fontCharInfo[ascii]->m_xadvance;

	return retVal;
}

CGLQuad2D* CFontLibrary::TextToQuadList(std::string font, std::string textToDraw, int x, int y, float scale)
{
	//get pointer to the correct font
	CGLFont* currfont = GetFontPointer(font);

	if(!currfont)
	{
		std::cout << "CFontLibrary::Font not found! " << font;
		return nullptr;
	}

	CGLQuad2D* quadList = new CGLQuad2D[textToDraw.length()];

	int defaultChar = 63; //'?', default for unknown characters is the question mark

	int cursor = 0;

	for(int i=0; i<textToDraw.length(); i++)
	{
		int ascii = GetTextChar(textToDraw, i);

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
		int charX = currfont->m_fontCharInfo[ascii]->m_textureX;
		int charY = currfont->m_fontCharInfo[ascii]->m_textureY;
		int width = currfont->m_fontCharInfo[ascii]->m_textureWidth;
		int height = currfont->m_fontCharInfo[ascii]->m_textureHeight;
		int offsetX = currfont->m_fontCharInfo[ascii]->m_xoffset;
		int offsetY = currfont->m_fontCharInfo[ascii]->m_yoffset;
		int base = currfont->m_base;

		//top left
		quadList[i].textureTopLeftX = (float) charX / (float) currfont->m_textureW;
		quadList[i].textureTopLeftY = (float) charY / (float) currfont->m_textureH;
		quadList[i].topLeftX = cursor + (offsetX)*scale + x;
		quadList[i].topLeftY = y + (base - offsetY)*scale;

		//top right
		quadList[i].textureTopRightX = (float) (charX+width) / (float) currfont->m_textureW;
		quadList[i].textureTopRightY = (float) charY / (float) currfont->m_textureH;
		quadList[i].topRightX = (width + offsetX)*scale + x + cursor;
		quadList[i].topRightY = y + (base - offsetY)*scale;

		//bottom right
		quadList[i].textureBottomRightX = (float) (charX+width) / (float) currfont->m_textureW;
		quadList[i].textureBottomRightY = (float) (charY+height) / (float) currfont->m_textureH;
		quadList[i].bottomRightX = (width + offsetX)*scale + x + cursor;
		quadList[i].bottomRightY = y + (base - offsetY - height)*scale;

		//bottom left
		quadList[i].textureBottomLeftX = (float) charX / (float) currfont->m_textureW;
		quadList[i].textureBottomLeftY = (float) (charY+height) / (float) currfont->m_textureH;
		quadList[i].bottomLeftX = (offsetX)*scale + x + cursor;
		quadList[i].bottomLeftY = y + (base - offsetY - height)*scale;

		//calc kerning depending on current and next character in the string to draw
		int kerning = 0;
		if(i>0 && i<textToDraw.length()-1)
		{
			if(ascii <= currfont->m_highestKerningFirst)
			{
				int asciiNext = textToDraw[i+1];
				if(asciiNext <= currfont->m_highestKerningSecond)
					kerning = currfont->m_kernings[ascii][asciiNext];
			}
		}

		//get cursor position in x-direction for next character in string
		cursor += (currfont->m_fontCharInfo[ascii]->m_xadvance + kerning)*scale;
	}

	return quadList;
}

int CFontLibrary::GetTextChar(std::string textToDraw, int pos)
{
	int ch = (int)textToDraw.at(pos);
	if(ch < 0) //UTF8 conversion did not work, we will attempt UTF16 conversion
	{
		unsigned int len = 0;
		char text = textToDraw.at(pos);
		ch = acUtility::DecodeUTF16(&text, &len);
	}

	return ch;
}

void CFontLibrary::DrawQuadList(std::string font, float color[4], CGLQuad2D* quadList, std::string textToDraw)
{
	if(textToDraw.length() <= 0)
		return;

	CGLFont* currFont = GetFontPointer(font);
	if (!currFont)
		return;

	glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT | GL_TEXTURE_BIT); //lighting and color mask
	glDisable(GL_LIGHTING);     //need to disable lighting for proper text color
	glDisable(GL_DEPTH_TEST);

	unsigned int textureID = currFont->m_fontTextures;
	if(textureID > 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textureID);
	}

	if(currFont->m_sdfType != SDF_NO && m_sdfShaderProgram && m_sdfShaderProgram->GetProgramID() > 0)
	{
		GLuint programID = m_sdfShaderProgram->GetProgramID();
		glUseProgram(programID);

		GLint textureLocation = glGetUniformLocation(programID, "u_FontTexutre");
		glUniform1i(textureLocation, 0);

		GLint sdfLoc = glGetUniformLocation(programID, "sdfType");
		if(sdfLoc >= 0)
			glUniform1i(sdfLoc, currFont->m_sdfType);

		if(currFont->m_sdfType == SDF_MULTI)
		{
			GLint pxLoc = glGetUniformLocation(programID, "pxRange");
			if(pxLoc >= 0)
				glUniform1f(pxLoc, (float)currFont->m_sdfRange);

			GLint fgColorLoc = glGetUniformLocation(programID, "fgColor");
			if(fgColorLoc >= 0)
				glUniform4f(fgColorLoc, color[0], color[1], color[2], color[3]);

			GLint bgColorLoc = glGetUniformLocation(programID, "bgColor");
			if(bgColorLoc >= 0)
				glUniform4f(bgColorLoc, 0.0, 0.0, 0.0, 0.0); //TODO
		}

		glBegin(GL_QUADS);
			for (int i = 0; i<textToDraw.length(); i++)
			{
				glMultiTexCoord2f(GL_TEXTURE0, quadList[i].textureTopLeftX, quadList[i].textureTopLeftY);
				glVertex2f(quadList[i].topLeftX, quadList[i].topLeftY);			//top left vertex
				glMultiTexCoord2f(GL_TEXTURE0, quadList[i].textureBottomLeftX, quadList[i].textureBottomLeftY);
				glVertex2f(quadList[i].bottomLeftX, quadList[i].bottomLeftY);	//bottom left vertex
				glMultiTexCoord2f(GL_TEXTURE0, quadList[i].textureBottomRightX, quadList[i].textureBottomRightY);
				glVertex2f(quadList[i].bottomRightX, quadList[i].bottomRightY);	//bottom right vertex
				glMultiTexCoord2f(GL_TEXTURE0, quadList[i].textureTopRightX, quadList[i].textureTopRightY);
				glVertex2f(quadList[i].topRightX, quadList[i].topRightY);		//top right vertex
			}
		glEnd();

		glUseProgram(0);
	}
	else
	{
		glColor4fv(color);	//set text color

		glBegin(GL_QUADS);
			for(int i=0; i<textToDraw.length(); i++)
			{
				glTexCoord2f(quadList[i].textureTopLeftX, quadList[i].textureTopLeftY);
				glVertex2i(quadList[i].topLeftX, quadList[i].topLeftY);			//top left vertex
				glTexCoord2f(quadList[i].textureBottomLeftX, quadList[i].textureBottomLeftY);
				glVertex2i(quadList[i].bottomLeftX, quadList[i].bottomLeftY);	//bottom left vertex
				glTexCoord2f(quadList[i].textureBottomRightX, quadList[i].textureBottomRightY);
				glVertex2i(quadList[i].bottomRightX, quadList[i].bottomRightY);	//bottom right vertex
				glTexCoord2f(quadList[i].textureTopRightX, quadList[i].textureTopRightY);
				glVertex2i(quadList[i].topRightX, quadList[i].topRightY);		//top right vertex
			}
		glEnd();
	}

	if(textureID > 0)
		glBindTexture(GL_TEXTURE_2D, 0); //unbind texture

	glEnable(GL_DEPTH_TEST);
	glPopAttrib();
}

CGLFont* CFontLibrary::GetFontPointer(std::string fontName)
{
	std::map<std::string, CGLFont*>::const_iterator found = m_fontList.find(fontName);
	if (found != m_fontList.end())
		return found->second;
	else
		return nullptr;
}

unsigned int CFontLibrary::GetNewDrawStringID()
{
	return m_IDCounter++;
}
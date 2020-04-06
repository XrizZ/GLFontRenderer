//=================================================================================
// Name			:	FontLibrary.cpp
// Author		:	Christian Thurow
// Date			:	March 30th 2020
// Description	:	Main Implementation file CFontLibrary
//=================================================================================

#include "stdafx.h"
#include "FontFileParser.h"
#include "FontLibrary.h"
#include "Font.h"
#include "Texture.h"
#include "acutil_unicode.h"
#include <windows.h>

CFontLibrary::CFontLibrary(CString folder)
{
	m_fontFolder = folder;
	m_IDCounter = 1;
}

CFontLibrary::~CFontLibrary()
{
	TRACE("CFontLibrary Destructor \n");

	//loop through all fonts
	POSITION pos = m_fontList.GetStartPosition();
	while(pos)
	{
		//call cleanup for current font
		CString key;
		CGLFont* toDelete = nullptr;
		m_fontList.GetNextAssoc(pos, key, toDelete);
		m_fontList.RemoveKey(key);
		if(toDelete)
			delete toDelete;
		toDelete = nullptr;
	}

	//loop through all the strings that we drew
	pos = m_glStringList.GetStartPosition();
	while(pos)
	{
		//call cleanup for current font
		unsigned int key;
		CDrawString* toDelete = nullptr;
		m_glStringList.GetNextAssoc(pos, key, toDelete);
		m_glStringList.RemoveKey(key);
		if(toDelete)
		{
			glDeleteLists(toDelete->m_drawListID, 1);
		}
		delete toDelete;
		toDelete = nullptr;
	}

	if(m_sdfShaderProgram)
		delete m_sdfShaderProgram;
	m_sdfShaderProgram = 0;
}

bool CFontLibrary::ParseAllFontsInFolder()
{
	//get a list of all fonts in font directory
	CFileFind finder;
	CString filename;
	filename.Format(_T("%s\\*.fnt"), m_fontFolder.GetString());
	BOOL bWorking = finder.FindFile(filename);
	CList<CString, CString&> fileNameList;
	while(bWorking)
	{
		bWorking = finder.FindNextFile();
		fileNameList.AddTail(finder.GetFileName());
	}

	//parse each font individually and add the parsed font to the global font library list
	POSITION pos = fileNameList.GetHeadPosition();
	int i=0;
	for(; i<fileNameList.GetCount(); i++)
	{
		CString currFileName;
		CString curr = fileNameList.GetNext(pos);
		currFileName.Format(_T("%s\\%s"), m_fontFolder, curr);
		CGLFont* newFont = ParseFont(currFileName);
		//trim to correct name
		CString fontName = curr;
		if(fontName.GetLength() > 5)
			fontName = fontName.Left(fontName.GetLength()-4);
		if(newFont != nullptr)
			m_fontList.SetAt(fontName, newFont);
	}

	//Cleanup
	fileNameList.RemoveAll();

	//check if we found at least one font
	if(i)
		return true;
	else
		return false;
}

CGLFont* CFontLibrary::ParseFont(CString fileName)
{
	CFontFileParser* parser = new CFontFileParser(fileName);
	if(!parser || !parser->IsInitialized())
		return nullptr; //ERROR

	CGLFont* newFont = new CGLFont();
	if(!newFont)
		return nullptr; //ERROR

	//now load font config file(*.fnt) -> see fileName parameter
	//define strings we are searching for
	CString faceString("face=");
	CString sizeString("size=");
	CString boldString("bold=");
	CString italicString("italic=");
	CString unicodeString("unicode=");
	CString textureWString("scaleW=");
	CString textureHString("scaleH=");
	CString textureNameString("file=");
	CString baseString("base=");

	//==========================
	//load general font infos
	//==========================

	//load face name
	CString faceVal;
	parser->GetValueFromBufferOfFirst(faceString, &faceVal);
	//remove the "
	faceVal.SetAt(0, ' ');
	faceVal.SetAt(faceVal.GetLength()-1, ' ');
	faceVal.TrimLeft();
	faceVal.TrimRight();
	newFont->m_face = faceVal;

	//load font size
	CString sizeVal;
	parser->GetValueFromBufferOfFirst(sizeString, &sizeVal);
	newFont->m_size = atoi(sizeVal.GetString());

	//load bold parameter
	CString boldVal;
	parser->GetValueFromBufferOfFirst(boldString, &boldVal);
	newFont->m_bold = (bool)atoi(boldVal);

	//load italic parameter
	CString italicVal;
	parser->GetValueFromBufferOfFirst(italicString, &italicVal);
	newFont->m_italic = (bool)atoi(italicVal);

	//load unicode parameter
	CString unicodeVal;
	parser->GetValueFromBufferOfFirst(unicodeString, &unicodeVal);
	newFont->m_unicode = (bool)atoi(unicodeVal);

	//load texture height
	CString textureHVal;
	parser->GetValueFromBufferOfFirst(textureHString, &textureHVal);
	newFont->m_textureH = atoi(textureHVal);

	//load texture width
	CString textureWVal;
	parser->GetValueFromBufferOfFirst(textureWString, &textureWVal);
	newFont->m_textureW = atoi(textureWVal);

	//load texture width
	CString textureNameVal;
	parser->GetValueFromBufferOfFirst(textureNameString, &textureNameVal);
	//remove the "
	textureNameVal.SetAt(0, ' ');
	textureNameVal.SetAt(textureNameVal.GetLength()-1, ' ');
	textureNameVal.TrimLeft();
	textureNameVal.TrimRight();
	newFont->m_bitmapFileName.Format("%s\\%s", m_fontFolder, textureNameVal);

	//load base
	CString baseVal;
	parser->GetValueFromBufferOfFirst(baseString, &baseVal);
	newFont->m_base = atoi(baseVal);

	//==========================
	//load char infos
	//==========================
	unsigned int length = parser->GetNumberOfSupportedChars(); //determine max length of charInfo array
	if(length <= 0)
	{
		delete parser;
		delete newFont;
		return nullptr;
	}
	newFont->m_fontCharInfo = new CCharInfo*[length+1];
	newFont->m_highestASCIIChar = length;
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
	POSITION pos = m_fontList.GetStartPosition();
	while(pos)
	{
		//load texture for current font
		CString key;
		CGLFont* currFont = nullptr;
		m_fontList.GetNextAssoc(pos, key, currFont);

		if(!currFont || !CGLTexture::LoadTextureFromFile(std::string(currFont->m_bitmapFileName.GetString(), currFont->m_bitmapFileName.GetLength()), currFont->m_fontTextures))
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
		CString desc;
		desc.Format(_T("FontLibrary failed to load sdf shader."));
		//TODO: log error
		error = true;
	}

	return error;
}

void CFontLibrary::DrawString(CString textToDraw, int x, int y, float color[4], CString font, bool sdf, float scale)
{
	CGLQuad2D* quadList = TextToQuadList(font, textToDraw, x, y, scale);
	if(quadList)
	{
		DrawQuadList(font, color, quadList, textToDraw, sdf);
		delete[] quadList;
	}
}

void CFontLibrary::DrawString(unsigned int ID, CString textToDraw, int x, int y, float color[4], CString font, bool sdf, float scale)
{
	CDrawString* savedString = nullptr;
	bool found = m_glStringList.Lookup(ID, savedString);
	if(found && savedString)
	{
		if(savedString->m_text.Compare(textToDraw) != 0 || 
			(savedString->m_x != x || savedString->m_y != y) ||
			(color[0] != savedString->m_color[0] || color[1] != savedString->m_color[1] || color[2] != savedString->m_color[2] || color[3] != savedString->m_color[3]) ||
			(scale != savedString->m_scale) || (sdf != savedString->m_sdf)
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
			savedString->m_sdf = sdf;
		}
	}
	else
	{
		savedString = new CDrawString(ID, textToDraw, x, y, color, sdf, scale);
	}

	if(savedString->m_drawListID == 0)
	{
		GLuint id = glGenLists(1);
		glNewList(id, GL_COMPILE);

		DrawString(textToDraw, x, y, color, font, sdf, scale);

		glEndList();
		savedString->m_drawListID = id;
	}
	glCallList(savedString->m_drawListID);

	if(!found)
		m_glStringList.SetAt(ID, savedString);
}

//draws the string until lineWidth(pixels) then cuts it off there and draws the rest underneath and so forth till the last character in the string has been drawn
//draws only the text up to the specified line, if maxLines parameter is zero, it means there is no limit
void CFontLibrary::DrawStringWithLineBreaks(unsigned int ID, CString textToDraw, int x, int y, float color[4], CString font, bool sdf, float scale, int lineWidth, int maxLines)
{
	CDrawString* savedString = nullptr;
	bool found = m_glStringList.Lookup(ID, savedString);
	if(found && savedString)
	{
		if(savedString->m_text.Compare(textToDraw) != 0 || 
			(savedString->m_x != x || savedString->m_y != y) ||
			(color[0] != savedString->m_color[0] || color[1] != savedString->m_color[1] || color[2] != savedString->m_color[2] || color[3] != savedString->m_color[3]) ||
			(scale != savedString->m_scale) ||
			(maxLines != savedString->m_maxLines) ||
			(lineWidth != savedString->m_lineWidth) ||
			(sdf != savedString->m_sdf)
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
			savedString->m_lineWidth = lineWidth;
			savedString->m_maxLines = maxLines;
			savedString->m_sdf = sdf;
		}
	}
	else
	{
		savedString = new CDrawString(ID, textToDraw, x, y, color, sdf, scale);
		savedString->m_lineWidth = lineWidth;
		savedString->m_maxLines = maxLines;
	}

	if(savedString->m_drawListID == 0)
	{
		GLuint id = glGenLists(1);
		glNewList(id, GL_COMPILE);

		DrawStringWithLineBreaks(textToDraw, x, y, color, font, sdf, scale, lineWidth, maxLines);

		glEndList();
		savedString->m_drawListID = id;
	}
	glCallList(savedString->m_drawListID);

	if(!found)
		m_glStringList.SetAt(ID, savedString);
}

//draws the string until lineWidth(pixels) then cuts it off there and draws the rest underneath and so forth till the last character in the string has been drawn
//draws only the text up to the specified line, if maxLines parameter is zero, it means there is no limit
void CFontLibrary::DrawStringWithLineBreaks(CString textToDraw, int x, int y, float color[4], CString font, bool sdf, float scale, int lineWidth, int maxLines)
{
	unsigned int lineHeight = GetLineHeight(font);
	unsigned int line = 0;
	for(int index = 0; index < textToDraw.GetLength();)
	{
		if(maxLines > 0 && line >= maxLines) //lets only draw the number of lines defind my "maxLines"
			return;

		CString subStringToDraw;
		int textWidth = 0;
		while(textWidth < lineWidth && index < textToDraw.GetLength())
		{
			subStringToDraw.AppendChar(textToDraw.GetAt(index++));
			textWidth = GetWidthOfString(subStringToDraw, font, scale);
		}

		DrawString(subStringToDraw.Trim(), x, y - line++*lineHeight*scale, color, font, scale);
	}
}

unsigned int CFontLibrary::GetLineHeight(CString font)
{
	unsigned int retVal = 0;

	CGLFont* currfont = GetFontPointer(font);

	if(!currfont)
	{
		TRACE("CFontLibrary::Font not found! '%s'", font);
		return 0;
	}

	char ascii = 'W'; //very tall character

	retVal = currfont->m_fontCharInfo[ascii]->m_textureWidth + 2; //very tall character + 2 comes line height very close

	return retVal;
}

float CFontLibrary::GetWidthOfString(CString textToDraw, CString font, float scale)
{
	float retVal = 0;

	for(int i=0; i<textToDraw.GetLength(); i++)
	{
		CString charToDraw;
		charToDraw.Format(_T("%c"), textToDraw.GetAt(i));
		retVal+=GetWidthOfChar(charToDraw, font);

		//TODO: account for kernings!
	}

	retVal *= scale;

	return retVal;
}

unsigned int CFontLibrary::GetWidthOfChar(CString ch, CString font)
{
	unsigned int retVal = 0;

	//get the pointer to the selected font
	CGLFont* currfont = GetFontPointer(font);

	if(!currfont)
	{
		TRACE(_T("CFontLibrary::Font not found! '%s'", font));
		return 0;
	}

	int ascii = GetTextChar(ch, 0);
	if(ascii >= 0 && ascii < currfont->m_highestASCIIChar) //sanity check
		retVal = currfont->m_fontCharInfo[ascii]->m_xadvance;

	return retVal;
}

CGLQuad2D* CFontLibrary::TextToQuadList(CString font, CString textToDraw, int x, int y, float scale)
{
	//get pointer to the correct font
	CGLFont* currfont = GetFontPointer(font);

	if(!currfont)
	{
		TRACE(_T("CFontLibrary::Font not found! '%s'", font));
		return nullptr;
	}

	CGLQuad2D* quadList = new CGLQuad2D[textToDraw.GetLength()];

	int defaultChar = 63; //'?'

	int cursor = 0;

	for(int i=0; i<textToDraw.GetLength(); i++)
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

		//calc kerning depending in current and next character in the string to draw
		int kerning = 0;
		if(i>0 && i<textToDraw.GetLength()-1)
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

int CFontLibrary::GetTextChar(CString textToDraw, int pos)
{
	int UTF8 = 1;
	int UTF16 = 2;

	int encoding = 1;
	if((int)textToDraw.GetAt(pos) < 0)
	{
		encoding = 2;
	}

	int ch;
	unsigned int len;
	if( encoding == UTF8 )
	{
		ch = (int)textToDraw.GetAt(pos);
	}
	else if( encoding == UTF16 )
	{
		CString fakeText;
		fakeText.Format(_T("%c"), textToDraw.GetAt(pos)); //creates a null terminated string with the utf16 character
		ch = acUtility::DecodeUTF16(fakeText.GetString(), &len);
	}

	return ch;
}

void CFontLibrary::DrawQuadList(CString font, float color[4], CGLQuad2D* quadList, CString textToDraw, bool sdf)
{
	if(textToDraw.GetLength() <= 0)
		return;

	CGLFont* currFont = GetFontPointer(font);

	if (!currFont)
		return;

	unsigned int textureID = currFont->m_fontTextures;

	glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT | GL_TEXTURE_BIT); //lighting and color mask
	glDisable(GL_LIGHTING);     //need to disable lighting for proper text color
	glDisable(GL_DEPTH_TEST);

	if(textureID > 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textureID);
	}

	glColor4fv(color);	//set text color

	if(sdf && m_sdfShaderProgram && m_sdfShaderProgram->GetProgramID() > 0)
	{
		GLuint programID = m_sdfShaderProgram->GetProgramID();
		glUseProgram(programID);
		GLuint textureLocation = glGetUniformLocation(programID, "u_FontTexutre");
		GLuint step = glGetUniformLocation(programID, "u_Step");
		glActiveTexture(GL_TEXTURE0);

		glUniform1i(textureLocation, 0);
		glUniform1f(step, 0.0f);

		glBegin(GL_QUADS);
			for (int i = 0; i<textToDraw.GetLength(); i++)
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
		glBegin(GL_QUADS);
			for(int i=0; i<textToDraw.GetLength(); i++)
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

CGLFont* CFontLibrary::GetFontPointer(CString fontName)
{
	CGLFont* retVal;
	bool success = m_fontList.Lookup(fontName, retVal);
	if(success)
		return retVal;
	else
		return nullptr;
}

unsigned int CFontLibrary::GetNewDrawStringID()
{
	return m_IDCounter++;
}
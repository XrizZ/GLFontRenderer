//=================================================================================
// Name			:	FontFileParser.cpp
// Author		:	Christian Thurow
// Date			:	March 30th 2020
// Description	:	Implementation file for CFontFileParser, used by the FontLibrary
//=================================================================================

#include "stdafx.h"
#include "FontFileParser.hpp"
#include <iostream>
#include <fstream>

CFontFileParser::CFontFileParser(CString fileName)
{
	m_isInitialized = false;
	m_buffer.clear();
	m_seperator = _T(" ");

	//open file stream and write it into a buffer
	std::ifstream file;
	file.open(fileName, std::ios::in | std::ios::binary);

	if(!file.fail())
	{
		while(!file.eof())
		{
			char currChar;
			file.get(currChar);
			m_buffer.push_back(currChar);
		}
		file.close();
	}

	if(m_buffer.size() > 0)
		m_isInitialized = true;

	VectorToString(&m_buffer, &m_bufferString);
}

CFontFileParser::~CFontFileParser()
{
	m_buffer.clear();
}

bool CFontFileParser::IsInitialized()
{
	return m_isInitialized;
}

void CFontFileParser::VectorToString(std::vector<char>* src, CString* dst)
{
	dst->Empty();
	for(std::vector<char>::iterator it = src->begin(); it != src->end(); ++it)
	{
		char curr = *it;
		dst->AppendChar(curr);
	}
}

bool CFontFileParser::GetValueFromBufferOfFirst(CString string, CString* value)
{
	int firstPos = m_bufferString.Find(string);
	if(firstPos < 0)
		return false;

	int seperatorPos1 = m_bufferString.Find(m_seperator, firstPos+string.GetLength());
	int seperatorPos2 = m_bufferString.Find("\n", firstPos+string.GetLength());
	int seperatorPos3 = m_bufferString.Find("\r", firstPos+string.GetLength());

	int seperatorPos = seperatorPos1 >= seperatorPos2 ? seperatorPos2 : seperatorPos1;
	seperatorPos = seperatorPos3 >= seperatorPos ? seperatorPos : seperatorPos3;

	int beginValue = firstPos + string.GetLength();
	//retrieve string between beginValue and seperatorPos
	value->Empty();
	for(int i=beginValue; i<seperatorPos; i++)
	{
		CString curr;
		curr.Format(_T("%c"), m_bufferString.GetAt(i));
		value->Append(curr);
	}

	return true;
}

int CFontFileParser::GetValueFromBufferStartingAt(CString string, CString* value, int startPos)
{
	int firstPos = m_bufferString.Find(string, startPos);
	if(firstPos < 0)
		return -1;

	int seperatorPos1 = m_bufferString.Find(m_seperator, firstPos+string.GetLength());
	int seperatorPos2 = m_bufferString.Find("\n", firstPos+string.GetLength());
	int seperatorPos3 = m_bufferString.Find("\r", firstPos+string.GetLength());

	int seperatorPos = seperatorPos1 >= seperatorPos2 ? seperatorPos2 : seperatorPos1;
	seperatorPos = seperatorPos3 >= seperatorPos ? seperatorPos : seperatorPos3;

	int beginValue = firstPos + string.GetLength();
	//retrieve string between beginValue and seperatorPos
	GetCStringBetween(beginValue, seperatorPos, value, m_bufferString);

	return seperatorPos;
}

bool CFontFileParser::GetValueFromBufferOfAll(CString string, CList<CString, CString&>* values)
{
	values->RemoveAll();

	int currPos = m_bufferString.Find(string);
	if(currPos < 0)
		return false;

	while(currPos > 0)
	{
		int beginValue = currPos + string.GetLength();

		int seperatorPos1 = m_bufferString.Find(m_seperator, beginValue);
		int seperatorPos2 = m_bufferString.Find("\n", beginValue);
		int seperatorPos3 = m_bufferString.Find("\r", beginValue);

		int seperatorPos = seperatorPos1 >= seperatorPos2 ? seperatorPos2 : seperatorPos1;
		seperatorPos = seperatorPos3 >= seperatorPos ? seperatorPos : seperatorPos3;

		//retrieve string between beginValue and seperatorPos
		CString currValue;
		for(int i=beginValue; i<seperatorPos; i++)
		{
			CString curr;
			curr.Format(_T("%c"), m_bufferString.GetAt(i));
			currValue.Append(curr);
		}
		values->AddTail(currValue);

		currPos = m_bufferString.Find(string, beginValue + currValue.GetLength());
	}

	return true;
}

//returns the highest number(ascii) of char, supported by the font
//-1 if failure
int CFontFileParser::GetNumberOfSupportedChars()
{
	int retVal = -1;

	if(m_buffer.size() <= 0)
		return -1;

	//parse the char id's
	CList<CString, CString&> values;
	GetValueFromBufferOfAll("char id=", &values);

	//go through all values that were found and extract the highest value
	//this will denote the length of our char list
	POSITION pos = values.GetHeadPosition();
	while(pos)
	{
		CString currString = values.GetNext(pos);
		int currAsciiID = atoi(currString);
		if(currAsciiID > retVal)
			retVal = currAsciiID;
	}

	return retVal;
}

void CFontFileParser::GetCStringBetween(int begin, int end, CString* value, CString src)
{
	value->Empty();
	for(int i=begin; i<end; i++)
	{
		CString curr;
		curr.Format(_T("%c"), src.GetAt(i));
		value->Append(curr);
	}
}

CKerning* CFontFileParser::GetNextKerning(int *startSearchPos)
{
	CKerning* newKerning = new CKerning();

	CString firstString("first=");
	CString secondString("second=");
	CString amountString("amount=");

	int start = *startSearchPos;

	//-------------
	//Get first ascii
	//-------------
	CString firstVal;
	*startSearchPos = GetValueFromBufferStartingAt(firstString, &firstVal, start);
	newKerning->m_first = atoi(firstVal);

	//-------------
	//Get second ascii
	//-------------
	CString secondVal;
	*startSearchPos = GetValueFromBufferStartingAt(secondString, &secondVal, start);
	newKerning->m_second = atoi(secondVal);

	//-------------
	//Get kerning amount
	//-------------
	CString amountVal;
	*startSearchPos = GetValueFromBufferStartingAt(amountString, &amountVal, start);
	newKerning->m_amount = atoi(amountVal);

	return newKerning;
}

CCharInfo* CFontFileParser::GetNextCharInfo(int *startSearchPos)
{
	CCharInfo* newCharInfo = new CCharInfo();

	CString idString("char id=");
	CString xString("x=");
	CString yString("y=");
	CString widthString("width=");
	CString heightString("height=");
	CString xoffsetString("xoffset=");
	CString yoffsetString("yoffset=");
	CString xadvanceString("xadvance=");
	//CString pageString;
	//CString channelString;

	int start = *startSearchPos;

	//-------------
	//Get ID
	//-------------
	CString idVal;
	*startSearchPos = GetValueFromBufferStartingAt(idString, &idVal, start);
	newCharInfo->m_ID = atoi(idVal);

	//-------------
	//Get x
	//-------------
	CString xVal;
	*startSearchPos = GetValueFromBufferStartingAt(xString, &xVal, start);
	newCharInfo->m_textureX = atoi(xVal);

	//-------------
	//Get y
	//-------------
	CString yVal;
	*startSearchPos = GetValueFromBufferStartingAt(yString, &yVal, start);
	newCharInfo->m_textureY = atoi(yVal);

	//-------------
	//Get width
	//-------------
	CString widthVal;
	*startSearchPos = GetValueFromBufferStartingAt(widthString, &widthVal, start);
	newCharInfo->m_textureWidth = atoi(widthVal);

	//-------------
	//Get height
	//-------------
	CString heightVal;
	*startSearchPos = GetValueFromBufferStartingAt(heightString, &heightVal, start);
	newCharInfo->m_textureHeight = atoi(heightVal);

	//-------------
	//Get xoffset
	//-------------
	CString xoffsetVal;
	*startSearchPos = GetValueFromBufferStartingAt(xoffsetString, &xoffsetVal, start);
	newCharInfo->m_xoffset = atoi(xoffsetVal);

	//-------------
	//Get yoffset
	//-------------
	CString yoffsetVal;
	*startSearchPos = GetValueFromBufferStartingAt(yoffsetString, &yoffsetVal, start);
	newCharInfo->m_yoffset = atoi(yoffsetVal);

	//-------------
	//Get xadvance
	//-------------
	CString xadvanceVal;
	*startSearchPos = GetValueFromBufferStartingAt(xadvanceString, &xadvanceVal, start);
	newCharInfo->m_xadvance = atoi(xadvanceVal);

	return newCharInfo;
}

bool CFontFileParser::LoadCharInfos(CGLFont* newFont)
{
	bool success = true;

	//init the array with null
	for(int i=0; i <= newFont->m_highestASCIIChar; i++)
		newFont->m_fontCharInfo[i] = nullptr;

	CString totalNumberOfCharInfosString;
	GetValueFromBufferOfFirst("chars count=", &totalNumberOfCharInfosString);
	int totalNumberOfCharInfos = atoi(totalNumberOfCharInfosString);

	//find all char info lines and save them at the current CGLfont
	int startSearchPos = 0;
	for(int i=0; i<totalNumberOfCharInfos; i++)
	{
		CCharInfo* currCharInfo = GetNextCharInfo(&startSearchPos);
		if(currCharInfo != nullptr)
		{
			newFont->m_fontCharInfo[currCharInfo->m_ID] = currCharInfo;
			success = true;
		}
		else
			return false;
	}

	return success;
}

int CFontFileParser::GetKerningsFirstHighest()
{
	int retVal = 0;

	CList<CString, CString&> values;
	GetValueFromBufferOfAll("first=", &values);

	POSITION pos = values.GetHeadPosition();
	while(pos)
	{
		CString currString = values.GetNext(pos);
		int currFirst = atoi(currString);
		if(currFirst > retVal)
			retVal = currFirst;
	}

	return retVal;
}

int CFontFileParser::GetKerningsSecondHighest()
{
	int retVal = 0;

	CList<CString, CString&> values;
	GetValueFromBufferOfAll("second=", &values);

	POSITION pos = values.GetHeadPosition();
	while(pos)
	{
		CString currString = values.GetNext(pos);
		int currSecond = atoi(currString);
		if(currSecond > retVal)
			retVal = currSecond;
	}

	return retVal;
}

bool CFontFileParser::LoadKernings(CGLFont* newFont)
{
	bool success = true;

	//init the array with null
	for(int f=0; f<=newFont->m_highestKerningFirst; f++)
		for(int s=0; s<=newFont->m_highestKerningSecond; s++)
			newFont->m_kernings[f][s] = 0;

	CString totalNumberOfKerningsString;
	GetValueFromBufferOfFirst("kernings count=", &totalNumberOfKerningsString);
	int totalNumberOfKernings = atoi(totalNumberOfKerningsString);

	//find all char info lines and save them at the current CGLfont
	int startSearchPos = 0;
	for(int i=0; i<totalNumberOfKernings; i++)
	{
		CKerning* currKerning = GetNextKerning(&startSearchPos);
		if(currKerning != nullptr)
		{
			newFont->m_kernings[currKerning->m_first][currKerning->m_second] = currKerning->m_amount;
			success = true;
		}
		else
			return false;
	}

	return success;
}

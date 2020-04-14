//=================================================================================
// Name			:	FontFileParser.cpp
// Author		:	Christian Thurow
// Date			:	March 30th 2020
// Description	:	Implementation file for CFontFileParser, used by the FontLibrary
//=================================================================================

#include "FontFileParser.hpp"
#include <iostream>
#include <fstream>

CFontFileParser::CFontFileParser(std::string fileName)
{
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

void CFontFileParser::VectorToString(std::vector<char>* src, std::string* dst)
{
	dst->clear();
	for(std::vector<char>::iterator it = src->begin(); it != src->end(); ++it)
	{
		char curr = *it;
		dst->push_back(curr);
	}
}

bool CFontFileParser::GetValueFromBufferOfFirst(std::string string, std::string* value)
{
	size_t firstPos = m_bufferString.find(string);
	if(firstPos == std::string::npos)
		return false; //we have not found the value we are looking for in the given buffer

	size_t seperatorPos1 = m_bufferString.find(m_seperator, firstPos+string.length());
	size_t seperatorPos2 = m_bufferString.find("\n", firstPos+string.length());
	size_t seperatorPos3 = m_bufferString.find("\r", firstPos+string.length());

	int seperatorPos = seperatorPos1 >= seperatorPos2 ? seperatorPos2 : seperatorPos1;
	seperatorPos = seperatorPos3 >= seperatorPos ? seperatorPos : seperatorPos3;

	int beginValue = firstPos + string.length();

	//retrieve string between beginValue and seperatorPos
	value->clear();
	for(int i=beginValue; i<seperatorPos; i++)
	{
		value->push_back(m_bufferString.at(i));
	}

	return true;
}

int CFontFileParser::GetValueFromBufferStartingAt(std::string string, std::string* value, int startPos)
{
	size_t firstPos = m_bufferString.find(string, startPos);
	if(firstPos == std::string::npos)
		return -1; //we have not found the value we are looking for in the given buffer

	size_t seperatorPos1 = m_bufferString.find(m_seperator, firstPos+string.length());
	size_t seperatorPos2 = m_bufferString.find("\n", firstPos+string.length());
	size_t seperatorPos3 = m_bufferString.find("\r", firstPos+string.length());

	int seperatorPos = seperatorPos1 >= seperatorPos2 ? seperatorPos2 : seperatorPos1;
	seperatorPos = seperatorPos3 >= seperatorPos ? seperatorPos : seperatorPos3;

	int beginValue = firstPos + string.length();
	//retrieve string between beginValue and seperatorPos
	GetStringBetween(beginValue, seperatorPos, value, m_bufferString);

	return seperatorPos;
}

bool CFontFileParser::GetValueFromBufferOfAll(std::string string, std::forward_list<std::string>* values)
{
	values->clear();

	size_t currPos = m_bufferString.find(string);
	if(currPos == std::string::npos)
		return false; //we have not found the value we are looking for in the given buffer

	while(currPos > 0 && currPos < m_bufferString.length())
	{
		int beginValue = currPos + string.length();

		size_t seperatorPos1 = m_bufferString.find(m_seperator, beginValue);
		size_t seperatorPos2 = m_bufferString.find("\n", beginValue);
		size_t seperatorPos3 = m_bufferString.find("\r", beginValue);

		int seperatorPos = seperatorPos1 >= seperatorPos2 ? seperatorPos2 : seperatorPos1;
		seperatorPos = seperatorPos3 >= seperatorPos ? seperatorPos : seperatorPos3;

		//retrieve string between beginValue and seperatorPos
		std::string currValue;
		for(int i=beginValue; i<seperatorPos; i++)
		{
			currValue.push_back(m_bufferString.at(i));
		}
		values->push_front(currValue);

		currPos = m_bufferString.find(string, beginValue + currValue.length());
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
	std::forward_list<std::string> values;
	GetValueFromBufferOfAll("char id=", &values);

	//go through all values that were found and extract the highest value
	//this will denote the length of our char list
	for (auto it = values.cbegin(); it != values.cend(); it++)
	{
		std::string currString = *it;
		int currAsciiID = atoi(currString.c_str());
		if(currAsciiID > retVal)
			retVal = currAsciiID;
	}

	return retVal;
}

void CFontFileParser::GetStringBetween(int begin, int end, std::string* value, std::string src)
{
	value->clear();
	for(int i=begin; i<end; i++)
	{
		value->push_back(src.at(i));
	}

	//TODO
	//*value = src.substr(begin, end-begin);
}

CKerning* CFontFileParser::GetNextKerning(int *startSearchPos)
{
	CKerning* newKerning = new CKerning();

	std::string firstString("first=");
	std::string secondString("second=");
	std::string amountString("amount=");

	int start = *startSearchPos;

	//-------------
	//Get first ascii
	//-------------
	std::string firstVal;
	*startSearchPos = GetValueFromBufferStartingAt(firstString, &firstVal, start);
	newKerning->m_first = atoi(firstVal.c_str());

	//-------------
	//Get second ascii
	//-------------
	std::string secondVal;
	*startSearchPos = GetValueFromBufferStartingAt(secondString, &secondVal, start);
	newKerning->m_second = atoi(secondVal.c_str());

	//-------------
	//Get kerning amount
	//-------------
	std::string amountVal;
	*startSearchPos = GetValueFromBufferStartingAt(amountString, &amountVal, start);
	newKerning->m_amount = atoi(amountVal.c_str());

	return newKerning;
}

CCharInfo* CFontFileParser::GetNextCharInfo(int *startSearchPos)
{
	CCharInfo* newCharInfo = new CCharInfo();

	std::string idString("char id=");
	std::string xString("x=");
	std::string yString("y=");
	std::string widthString("width=");
	std::string heightString("height=");
	std::string xoffsetString("xoffset=");
	std::string yoffsetString("yoffset=");
	std::string xadvanceString("xadvance=");
	//std::string pageString;
	//std::string channelString;

	int start = *startSearchPos;

	//-------------
	//Get ID
	//-------------
	std::string idVal;
	*startSearchPos = GetValueFromBufferStartingAt(idString, &idVal, start);
	newCharInfo->m_ID = atoi(idVal.c_str());

	//-------------
	//Get x
	//-------------
	std::string xVal;
	*startSearchPos = GetValueFromBufferStartingAt(xString, &xVal, start);
	newCharInfo->m_textureX = atoi(xVal.c_str());

	//-------------
	//Get y
	//-------------
	std::string yVal;
	*startSearchPos = GetValueFromBufferStartingAt(yString, &yVal, start);
	newCharInfo->m_textureY = atoi(yVal.c_str());

	//-------------
	//Get width
	//-------------
	std::string widthVal;
	*startSearchPos = GetValueFromBufferStartingAt(widthString, &widthVal, start);
	newCharInfo->m_textureWidth = atoi(widthVal.c_str());

	//-------------
	//Get height
	//-------------
	std::string heightVal;
	*startSearchPos = GetValueFromBufferStartingAt(heightString, &heightVal, start);
	newCharInfo->m_textureHeight = atoi(heightVal.c_str());

	//-------------
	//Get xoffset
	//-------------
	std::string xoffsetVal;
	*startSearchPos = GetValueFromBufferStartingAt(xoffsetString, &xoffsetVal, start);
	newCharInfo->m_xoffset = atoi(xoffsetVal.c_str());

	//-------------
	//Get yoffset
	//-------------
	std::string yoffsetVal;
	*startSearchPos = GetValueFromBufferStartingAt(yoffsetString, &yoffsetVal, start);
	newCharInfo->m_yoffset = atoi(yoffsetVal.c_str());

	//-------------
	//Get xadvance
	//-------------
	std::string xadvanceVal;
	*startSearchPos = GetValueFromBufferStartingAt(xadvanceString, &xadvanceVal, start);
	newCharInfo->m_xadvance = atoi(xadvanceVal.c_str());

	return newCharInfo;
}

bool CFontFileParser::LoadCharInfos(CGLFont* newFont)
{
	bool success = true;

	//init the array with null
	for(int i=0; i <= newFont->m_highestASCIIChar; i++)
		newFont->m_fontCharInfo[i] = nullptr;

	std::string totalNumberOfCharInfosString;
	GetValueFromBufferOfFirst("chars count=", &totalNumberOfCharInfosString);
	int totalNumberOfCharInfos = atoi(totalNumberOfCharInfosString.c_str());

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

	std::forward_list<std::string> values;
	GetValueFromBufferOfAll("first=", &values);

	for (auto it = values.cbegin(); it != values.cend(); it++)
	{
		std::string currString = *it;
		int currFirst = atoi(currString.c_str());
		if(currFirst > retVal)
			retVal = currFirst;
	}

	return retVal;
}

int CFontFileParser::GetKerningsSecondHighest()
{
	int retVal = 0;

	std::forward_list<std::string> values;
	GetValueFromBufferOfAll("second=", &values);

	for (auto it = values.cbegin(); it != values.cend(); it++)
	{
		std::string currString = *it;
		int currSecond = atoi(currString.c_str());
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

	std::string totalNumberOfKerningsString;
	GetValueFromBufferOfFirst("kernings count=", &totalNumberOfKerningsString);
	int totalNumberOfKernings = atoi(totalNumberOfKerningsString.c_str());

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

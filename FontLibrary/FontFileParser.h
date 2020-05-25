//=================================================================================
// Name			:	FontFileParser.h
// Author		:	Christian Thurow
// Date			:	March 30th 2020
// Description	:	Header file for CFontFileParser, used by the FontLibrary
//=================================================================================

#pragma once
#include <vector>
#include "Font.h"
#include <forward_list>

class CFontFileParser
{
public:
	//constructor + destructor:
	CFontFileParser(std::string fileName);
	~CFontFileParser(void);

	//functions:
	bool IsInitialized();
	bool LoadCharInfos(CGLFont* newFont);
	bool LoadKernings(CGLFont* newFont);
	int GetHighestSupportedChar();
	bool GetValueFromBufferOfFirst(std::string string, std::string* value);
	bool GetValueFromBufferOfAll(std::string string, std::forward_list<std::string>* values);
	int GetValueFromBufferStartingAt(std::string string, std::string* value, int startPos);
	int GetKerningsFirstHighest();
	int GetKerningsSecondHighest();

private:
	//functions:
	void GetStringBetween(int begin, int end, std::string* value, std::string src);
	void VectorToString(std::vector<char>* src, std::string* dst);
	CCharInfo* GetNextCharInfo(int *startSearchPos);
	CKerning* GetNextKerning(int *startSearchPos);

	//variables:
	std::vector<std::string> m_seperators = {" ", "/"}; //separators: whitespace (for standard text file) and slash (for XML)
	bool m_isInitialized = false;
	std::vector<char> m_buffer;
	std::string m_bufferString;
};
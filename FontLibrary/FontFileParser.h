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
	CFontFileParser(const std::string& fileName);
	~CFontFileParser(void);

	//functions:
	bool IsInitialized();
	bool LoadCharInfos(CGLFont* newFont);
	bool LoadKernings(CGLFont* newFont);
	uint32_t GetHighestSupportedChar();
	bool GetValueFromBufferOfFirst(const std::string& string, std::string* value);
	bool GetValueFromBufferOfAll(const std::string& string, std::forward_list<std::string>* values);
	uint32_t GetValueFromBufferStartingAt(const std::string& string, std::string* value, uint32_t startPos);
	uint32_t GetKerningsFirstHighest();
	uint32_t GetKerningsSecondHighest();

private:
	//functions:
	void GetStringBetween(uint32_t begin, uint32_t end, std::string* value, const std::string& src);
	void VectorToString(std::vector<char>* src, std::string* dst);
	CCharInfo* GetNextCharInfo(uint32_t *startSearchPos);
	CKerning* GetNextKerning(uint32_t *startSearchPos);

	//variables:
	std::vector<std::string> m_seperators = {" ", "/"}; //separators: whitespace (for standard text file) and slash (for XML)
	bool m_isInitialized = false;
	std::vector<char> m_buffer;
	std::string m_bufferString;
};
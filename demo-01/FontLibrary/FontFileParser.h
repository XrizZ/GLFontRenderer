//=================================================================================
// Name			:	FontFileParser.h
// Author		:	Christian Thurow
// Date			:	March 30th 2020
// Description	:	Header file for CFontFileParser, used by the FontLibrary
//=================================================================================

#pragma once
#include <vector>
#include "Font.h"
#include <atlstr.h>

class CFontFileParser
{
public:
	//constructor + destructor:
	CFontFileParser(CString fileName);
	~CFontFileParser(void);

	//variables:

	//functions:
	bool IsInitialized();
	bool LoadCharInfos(CGLFont* newFont);
	bool LoadKernings(CGLFont* newFont);
	int GetNumberOfSupportedChars();
	bool GetValueFromBufferOfFirst(CString string, CString* value);
	bool GetValueFromBufferOfAll(CString string, CList<CString, CString&>* values);
	int GetValueFromBufferStartingAt(CString string, CString* value, int startPos);
	int GetKerningsFirstHighest();
	int GetKerningsSecondHighest();

private:
	//functions:
	void GetCStringBetween(int begin, int end, CString* value, CString src);
	void VectorToString(std::vector<char>* src, CString* dst);
	CCharInfo* GetNextCharInfo(int *startSearchPos);
	CKerning* GetNextKerning(int *startSearchPos);

	//variables:
	CString m_seperator;
	bool m_isInitialized;
	std::vector<char> m_buffer;
	CString m_bufferString;
};
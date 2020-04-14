//=================================================================================
// Name			:	HighPerformanceCounter.h
// Author		:	Christian Thurow
// Date			:	March 30th 2020
// Description	:	Header file for CHighPerformanceCounter, used by the Example App
//=================================================================================

#pragma once
#include <windows.h>

class CHighPerformanceCounter
{
private:
	ULONG_PTR m_threadAffMask = 0;
	LARGE_INTEGER m_cpuFrequency = {0};
	LARGE_INTEGER m_lastTick = {0};
	
public:

	//constructor/destructor
	CHighPerformanceCounter();
	~CHighPerformanceCounter();

	void Tick();
	double GetDeltaInMS();
};
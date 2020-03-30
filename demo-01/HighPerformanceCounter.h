//=================================================================================
// Name			:	HighPerformanceCounter.h
// Author		:	Christian Thurow
// Date			:	March 30th 2020
// Description	:	Header file for CHighPerformanceCounter, used by the Example App
//=================================================================================

#pragma once
#include <windows.h>

#ifndef HIGHPERFORMANCECOUNTER_H
#define HIGHPERFORMANCECOUNTER_H

class CHighPerformanceCounter
{
	
private:
	DWORD_PTR m_threadAffMask;
	LARGE_INTEGER m_cpuFrequency;
	LARGE_INTEGER m_lastTick;
	
public:

	//constructor/destructor
	CHighPerformanceCounter();
	~CHighPerformanceCounter();

	void Tick();
	double GetDeltaInMS();
};

#endif

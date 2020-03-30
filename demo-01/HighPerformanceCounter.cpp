//=================================================================================
// Name			:	HighPerformanceCounter.cpp
// Author		:	Christian Thurow
// Date			:	March 30th 2020
// Description	:	Implementation file for CHighPerformanceCounter, used by the Example App
//=================================================================================

#include "StdAfx.h"
#include "HighPerformanceCounter.h"
#include <iostream>

//constructor
CHighPerformanceCounter::CHighPerformanceCounter()
{
	//Make sure we are using the same core on a multi core machine, otherwise timings will be off
	//save the previous core mask
	//set the current mask to cpu 1
	//m_threadAffMask = SetThreadAffinityMask(GetCurrentThread(), 1);
 
	m_lastTick.QuadPart = 0;
	m_cpuFrequency.QuadPart = 0;

	if(!QueryPerformanceFrequency(&m_cpuFrequency) || !QueryPerformanceCounter(&m_lastTick))
		std::cout << "HighPerformanceCounter, timer not supported.";
 
	//restore the previous core mask to the thread
	//SetThreadAffinityMask(GetCurrentThread(), m_threadAffMask);
}

CHighPerformanceCounter::~CHighPerformanceCounter()
{}

//call this function to renew the m_lastTick
void CHighPerformanceCounter::Tick()
{
	//save the previous core mask
	//set the current mask to cpu 1
	//m_threadAffMask = SetThreadAffinityMask(GetCurrentThread(), 1);
 
	QueryPerformanceCounter(&m_lastTick);
 
	//restore the previous core mask to the thread
	//SetThreadAffinityMask(GetCurrentThread(), m_threadAffMask);
}

//returns the elapsed time since the m_lastTick in milliseconds as double
double CHighPerformanceCounter::GetDeltaInMS()
{
	LARGE_INTEGER currTick;
	QueryPerformanceCounter(&currTick);
	//QueryPerformanceFrequency(&m_cpuFrequency); //not sure if its good to call this more than once, but it would account for changing frequency (e.g. on mobile cpus)
	double elapsedTime = ((double)currTick.QuadPart - (double)m_lastTick.QuadPart)/(double)m_cpuFrequency.QuadPart;

	return elapsedTime * 1000.0;
}
#pragma once

#include <stdint.h>

class PerfTimer
{
public:

	void Start();
	double ReadMs() const;
	uint64_t ReadTicks() const;
	double ReadSec() const;

	static double TicksToSeconds(uint64_t ticks);
	static double TicksToMs(uint64_t ticks);
	static double MsToSeconds(double ms);
	static double SecondsToMs(double seconds);
	static uint64_t MsToTicks(double ms);
	static uint64_t SecondsToTicks(double seconds);

private:
	
	static const double InvFrequency();

private:

	uint64_t startTime = 0;

};
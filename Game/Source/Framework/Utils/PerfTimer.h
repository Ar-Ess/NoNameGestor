#pragma once

#include <stdint.h>

class PerfTimer
{
public:

	void Start();
	double ReadMs() const;
	uint64_t ReadTicks() const;
	double ReadSec() const;

private:
	
	static const double InvFrequency();

private:

	uint64_t startTime = 0;

};
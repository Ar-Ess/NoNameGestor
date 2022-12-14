#ifndef __PERFTIMER_H__
#define __PERFTIMER_H__

#include "Defs.h"

class PerfTimer
{
public:

	// Constructor
	PerfTimer();

	void Start();

	double ReadMs() const;

	uint64 ReadTicks() const;

	float ReadSec() const;

private:

	uint32	startedAt = 0;

	uint64 startTime = 0;

	static uint64 frequency;
};

#endif //__PERFTIMER_H__
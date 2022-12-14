#ifndef __TIMER_H__
#define __TIMER_H__

#include "Defs.h"

class Timer
{
public:

	// Constructor
	Timer();

	void Start();

	uint32 Read() const;

	float ReadSec() const;

private:

	uint32 startTime = 0;
};

#endif //__TIMER_H__
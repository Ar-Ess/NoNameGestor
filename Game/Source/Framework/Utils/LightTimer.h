#pragma once

#include <stdint.h>

class LightTimer
{
public:

	void Start();
	uint32_t ReadTicks() const;
	double ReadSec() const;
	double ReadMs() const;

private:

	uint32_t startTime = 0;

};
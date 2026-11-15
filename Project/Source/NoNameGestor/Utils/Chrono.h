#pragma once

#include "Framework/Data/Vector.h"
#include "Framework/Time/PerfTimer.h"

class Chrono
{

	enum class ChronoState
	{
		STOP,
		RUN,
		PAUSE
	};

	struct PauseStamp
	{
		PauseStamp(uint64_t start, uint64_t end) :
			start(start), end(end)
		{
		}

		bool IsCompleted() const
		{
			return start != 0 && end != 0;
		}

		uint64_t ElapsedTime() const
		{
			return end - start;
		}

		uint64_t start = 0, end = 0;
	};

public:

	Chrono();

	void Start();

	void Stop();

	void Pause();

	void Resume();

	bool ChronoTicks(uint64_t ticks);

	bool ChronoSec(double seconds);

	bool ChronoMs(double ms);

	void ChronoStop();

	uint64_t ReadTicks() const;

	float ReadSec() const;

	float ReadMs() const;

private:

	PerfTimer timer;
	Vector<PauseStamp> stamps;
	ChronoState state = ChronoState::STOP;

};
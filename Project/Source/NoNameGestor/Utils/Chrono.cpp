#include "Chrono.h"

#include "Framework/External/SDL/include/SDL_timer.h"
#include <assert.h>
	
Chrono::Chrono()
{
}

void Chrono::Start()
{
	if (state != ChronoState::STOP) return;

	timer.Start();
	state = ChronoState::RUN;
}

void Chrono::Stop()
{
	if (state == ChronoState::STOP) return;

	timer = PerfTimer();
	state = ChronoState::STOP;
	stamps.Clear();
}

void Chrono::Pause()
{
	if (state != ChronoState::RUN) return;

	stamps.EmplaceBack(timer.ReadTicks(), 0);
	state = ChronoState::PAUSE;
}

void Chrono::Resume()
{
	if (state != ChronoState::PAUSE) return;

	stamps.Back().end = timer.ReadTicks();
	state = ChronoState::RUN;
}

bool Chrono::ChronoTicks(uint64_t ticks)
{
	bool stop = false;

	switch (state)
	{
	case ChronoState::STOP:

		state = ChronoState::RUN;
		timer.Start();

		break;

	case ChronoState::RUN:

		if (timer.ReadTicks() >= ticks)
		{
			stop = true;
			ChronoStop();
		}

		break;
	}

	return stop;
}

bool Chrono::ChronoSec(double seconds)
{
	bool stop = false;

	switch (state)
	{
	case ChronoState::STOP:

		state = ChronoState::RUN;
		timer.Start();

		break;

	case ChronoState::RUN:

		if (timer.ReadSec() >= seconds)
		{
			stop = true;
			ChronoStop();
		}

		break;
	}

	return stop;
}

bool Chrono::ChronoMs(double ms)
{
	bool stop = false;

	switch (state)
	{
	case ChronoState::STOP:

		state = ChronoState::RUN;
		timer.Start();

		break;

	case ChronoState::RUN:

		if (timer.ReadMs() >= ms)
		{
			stop = true;
			ChronoStop();
		}

		break;
	}

	return stop;
}

void Chrono::ChronoStop()
{
	if (state == ChronoState::STOP) return;

	timer = PerfTimer();
	state = ChronoState::STOP;
}

uint64_t Chrono::ReadTicks() const
{
	if (stamps.IsEmpty())
		return timer.ReadTicks();

	uint64_t elapsedPauseTime = 0;
	stamps.Iterate([&](const PauseStamp& s) { if (s.IsCompleted()) elapsedPauseTime += s.ElapsedTime(); });

	return timer.ReadTicks() - elapsedPauseTime;
}

float Chrono::ReadSec() const
{
	return PerfTimer::TicksToSeconds(ReadTicks());
}

float Chrono::ReadMs() const
{
	return PerfTimer::TicksToMs(ReadTicks());
}
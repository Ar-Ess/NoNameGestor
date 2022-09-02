// ----------------------------------------------------
// Fast timer with milisecons precision
// ----------------------------------------------------

#include "Chrono.h"
#include "SDL\include\SDL_timer.h"
#include <assert.h>

// L07: DONE 1: Fill Start(), Read(), ReadSec() methods
// they are simple, one line each!
	
Chrono::Chrono()
{
}

void Chrono::Start()
{
	if (state != ChronoState::STOP) return;

	startTime = SDL_GetTicks();
	state = ChronoState::RUN;
}

void Chrono::Stop()
{
	if (state == ChronoState::STOP) return;

	startTime = 0;
	state = ChronoState::STOP;
	stamps.clear();
	stamps.shrink_to_fit();

}

void Chrono::Pause()
{
	if (state != ChronoState::RUN) return;

	stamps.push_back(PauseStamp(SDL_GetTicks(), 0));
	state = ChronoState::PAUSE;
}

void Chrono::Resume()
{
	if (state != ChronoState::PAUSE) return;

	stamps.back().end = SDL_GetTicks();
	state = ChronoState::RUN;
}

bool Chrono::ChronoStart(unsigned int seconds)
{
	unsigned int time = SDL_GetTicks();
	bool stop = false;
	switch (state)
	{
	case ChronoState::STOP:
		state = ChronoState::RUN;
		startTime = time;
		break;

	case ChronoState::RUN:
		if (((time - startTime) / 1000.0f) >= seconds)
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

	startTime = 0;
	state = ChronoState::STOP;
}

unsigned int Chrono::Read() const
{
	unsigned int readTime = SDL_GetTicks();

	if (stamps.empty()) return readTime - startTime;

	unsigned int elapsedPauseTime = 0;
	for (PauseStamp pS : stamps)
	{
		if (pS.IsCompleted()) elapsedPauseTime += pS.GetElapsedTime();
	}

	return readTime - startTime - elapsedPauseTime;
}

float Chrono::ReadSec() const
{
	unsigned int readTime = SDL_GetTicks();

	if (stamps.empty()) return float(readTime - startTime) / 1000;

	unsigned int elapsedPauseTime = 0;
	for (PauseStamp pS : stamps)
	{
		if (pS.IsCompleted()) elapsedPauseTime += pS.GetElapsedTime();
	}

	return float(readTime - startTime - elapsedPauseTime) / 1000;
}
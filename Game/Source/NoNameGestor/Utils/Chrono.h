#pragma once

#include <vector>

class Chrono
{
public:

	Chrono();

	void Start();

	void Stop();

	void Pause();

	void Resume();

	bool ChronoStart(unsigned int seconds);

	void ChronoStop();

	unsigned int Read() const;

	float ReadSec() const;

private:

	unsigned int startTime = 0;
	unsigned int pausedAt = 0;

	enum class ChronoState
	{
		STOP,
		RUN,
		PAUSE
	};

	struct PauseStamp
	{
		PauseStamp()
		{

		}

		PauseStamp(unsigned int start, unsigned int end)
		{
			this->start = start;
			this->end = end;
		}

		bool IsCompleted() const
		{
			return (start != 0 && end != 0);
		}

		unsigned int GetElapsedTime() const
		{
			return end - start;
		}

		unsigned int start = 0, end = 0;
	};

	std::vector<PauseStamp> stamps;

	ChronoState state = ChronoState::STOP;
};
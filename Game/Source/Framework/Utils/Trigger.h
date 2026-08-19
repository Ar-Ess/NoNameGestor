#pragma once

class Trigger
{
public:

	// Activate the trigger
	void Activate() noexcept
	{
		trigger = true;
	}

	// Check if the trigger has been activated
	[[nodiscard]] bool Check()
	{
		if (!trigger)
			return false;

		trigger = false;
		return true;
	}

private:

	bool trigger = false;

};
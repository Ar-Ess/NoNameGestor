#pragma once

class Trigger
{
public:

	// Operator to check if the trigger is not active
	// Is the trigger off?
	// Usage: if (myTrigger()) return;
	bool operator()()
	{
		if (!trigger) return true;

		trigger = false;
		return false;
	}

	// Operator to activate the trigger
	// Usage: !myTrigger;
	void operator!()
	{
		trigger = true;
	}

private:

	bool trigger = false;

};
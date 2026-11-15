#pragma once

#include "Framework/Engine/Module.h"
#include "Framework/Data/Map.h"
#include "Framework/Engine/ID.h"
#include "Framework/Coroutine/Coroutine.h"

class CoroutineManager : public Module<CoroutineManager>
{
	friend class App;

public:

	// Create a function that returns a Coroutine
	// Use co_yield with nullptr to skip a frame
	// Use co_yield with the yield options on "Yield::" namespace
	// Finally, send the function in here
	static const ID Begin(Coroutine&& c);

	static bool End(const ID& id);

private:

	CoroutineManager();

	virtual ~CoroutineManager();

	bool Update(float dt) override;

private:

	Map<ID, Coroutine> coroutines;
};
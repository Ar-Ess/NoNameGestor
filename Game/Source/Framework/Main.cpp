#include "App.h"

#include "Defs.h"
#include "SDL/include/SDL.h"
#include <stdlib.h>

enum class MainState
{
	CREATE = 1,
	START,
	LOOP,
	CLEAN,
	FAIL,
	EXIT
};

App* app = nullptr;

int main(int argc, char* args[])
{
	LOG("Engine starting ...");
	int result = 0;

	MainState state = MainState::CREATE;

	while(state != MainState::EXIT)
	{
		switch(state)
		{
			case MainState::CREATE:
			LOG("======================== CREATION PHASE ========================");

			app = new App(argc, args);
			state = (app != nullptr) ? MainState::START : MainState::FAIL;
			break;

			case MainState::START:
			LOG("======================== START PHASE ========================");

			if(app->Start())
			{
				state = MainState::LOOP;
				LOG("======================== UPDATE PHASE ========================");
			}
			else
			{
				state = MainState::FAIL;
				LOG("ERROR: Start failed");
			}
			break;

			case MainState::LOOP:

			if(!app->Update()) state = MainState::CLEAN;
			break;

			case MainState::CLEAN:
			LOG("======================== CLEANUP PHASE ========================");

			if(app->CleanUp())
			{
				RELEASE(app);
				state = MainState::EXIT;
			}
			else state = MainState::FAIL;
			break;

			case MainState::FAIL:
			LOG("Exiting with errors");
			result = -1;
			state = MainState::EXIT;
			break;
		}
	}

	// Exit with errors
	return result;
}
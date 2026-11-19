#pragma once

#include "NoNameGestor/Audio/ProgramState.h"
struct Intent;
class VoiceNode;

struct CommandContext
{
	// Intent of sentence
	const Intent& intent;
	// Current node where the context is located
	VoiceNode* node;
	// Program State when the sentence arrived, before Cancel()
	ProgramState  state;
	// Is this action resumable
	bool resumable;
};

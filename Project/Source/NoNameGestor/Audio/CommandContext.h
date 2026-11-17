#pragma once

struct Intent;
class VoiceNode;

struct CommandContext
{
	const Intent& intent;
	VoiceNode* node;
};

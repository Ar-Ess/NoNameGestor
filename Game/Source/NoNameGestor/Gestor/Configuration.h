#pragma once

struct Configuration
{
	// User Program Preferences
	bool showContainerType = true;
	bool showFutureUnasigned = false;
	float textFieldSize = 150.f;

	// User File Preferences
	const char* comboCurrency[6] = { "EUR", "USD", "MXN", "COP", "ARS", "PEN" };
	int currency[4] = { 0, 0, 0, 0};

};

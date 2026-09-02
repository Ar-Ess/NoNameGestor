#pragma once

#include "Framework/Data/Vector.h"
#include "Framework/Data/String.h"

struct Configuration
{
	// User Program Preferences
	bool showContainerType = true;
	bool showFutureUnassigned = false;
	float textFieldSize = 150.f;
	bool backupUnsavedFiles = false;
	String backupDirectory = "";
	String defaultDialogDirectory = "";
	bool autoOpenFileBackup = false;
	bool autoCloseFileBackup = false;
	bool autoCloseAppBackup = false;
	bool autoIntervalAppBackup = false;
	const char* comboIntervalText[7] = { "Every 5 minutes", "Every 10 minutes", "Every 15 minutes", "Every 20 minutes", "Every 30 minutes", "Every 1 hour", "Every 2 hours"};
	int comboIntervalValue[7] = { 5 * 60, 10 * 60, 15 * 60, 20 * 60, 30 * 60, 60 * 60, 120 * 60 };
	int autoIntervalAppValue = 3;

	// User File Preferences
	const char* comboCurrency[6] = { "EUR", "USD", "MXN", "COP", "ARS", "PEN" };
	int currency[4] = { 0, 0, 0, 0};

	// Internal Data
	Vector<String> recentFiles;

};

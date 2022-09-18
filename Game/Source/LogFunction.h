#pragma once

#define LOG(format, ...) LogFunc(__FILE__, __LINE__, format, __VA_ARGS__)

void LogFunc(const char file[], int line, const char* format, ...);
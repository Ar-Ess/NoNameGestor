#pragma once

#include "Framework/Data/String.h"
#include <source_location>
#include <type_traits>

class Debug
{

	static const bool DebugMode;

public:
	// Logs a comment to Output
	// format: text to output. To add variables, use %d (ints), %s(const char*), %f(floats)...
	// args: arguments of the variables formatted in the text
	template<typename... Args>
	static void Log(const char* format, Args... args, std::source_location loc = std::source_location::current())
	{
		if (format == nullptr) return;

		static_assert((std::is_trivially_copyable_v<Args> && ...),
			"Only trivial types allowed in printf-style Log");

		if (sizeof...(Args) == 0)
			LogInternal(format, loc);
		else
			LogInternal(String::Format(format, args...).Str(), loc);
	}

	// Logs a comment to Output
	// condition: the condition to assert
	// format: text to output. To add variables, use %d (ints), %s(const char*), %f(floats)...
	// args: arguments of the variables formatted in the text
	template<typename... Args>
	static void Assert(bool condition, const char* format, Args... args, std::source_location loc = std::source_location::current())
	{
		if (condition)
			return;

		String output;
		if (format != nullptr)
		{
			static_assert((std::is_trivially_copyable_v<Args> && ...),
				"Only trivial types allowed in printf-style Log");

			if (sizeof...(Args) == 0)
				AssertInternal(format, loc, true);
			else
				AssertInternal(String::Format(format, args...).Str(), loc, true);
		}
		else
			AssertInternal(nullptr, loc, false);
	}

	// Logs a comment to Output
	// condition: the condition to assert
	// format: text to output
	static void Assert(bool condition, const char* format = nullptr, std::source_location loc = std::source_location::current())
	{
		if (condition)
			return;
		
		AssertInternal(format, loc, true);
	}

private:

	static void LogInternal(const char* msg, std::source_location loc);

	static void AssertInternal(const char* msg, std::source_location loc, bool format);

	static void RegisterOnFile(const char* msg, const char* msgTag);

};
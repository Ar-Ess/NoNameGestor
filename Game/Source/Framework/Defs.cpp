#include "Defs.h"
#include <windows.h>
#include <fstream>

void LOG(const char* format, ...)
{
	static char tmpString1[4096];
	static va_list ap;
	static std::string out;

	// Construct the string from variable arguments
	va_start(ap, format);
	vsprintf_s(tmpString1, 4096, format, ap);
	va_end(ap);

	std::string file = __FILE__;
	file.erase(file.begin(), file.begin() + file.find_last_of('\\') + 1);

	out += "\nDEBUG | ";
	out += file.c_str();
	out += '(';
	out += std::to_string(__LINE__);
	out += "): ";
	out += tmpString1;
	out += '\n';

	OutputDebugString(out.c_str());
	out.clear();
}

bool SameString(std::string a, std::string b)
{
	bool ret = true;

	if (a.size() != b.size()) return false;

	for (unsigned int i = 0; i < a.size(); i++)
	{
		ret = (a[i] == b[i]);
		if (!ret) return false;
	}

	return true;
}

bool FileExists(const char* fileName)
{
	std::ifstream infile(fileName);
	return infile.good();
}

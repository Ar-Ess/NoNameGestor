#pragma once

#include "Framework/Engine/ID.h"

#include <stdint.h>
#include <functional>

class String;
struct Point;

class Hash
{
	friend class ID;

public:

	static const Hash Empty;

	static Hash From(const bool& x);

	static Hash From(const int& x);

	static Hash From(const uint64_t& x);

	static Hash From(const float& x);

	static Hash From(const double& x);

	static Hash From(const ID& x);

	static Hash From(const String& x);

	static Hash From(const char* x);

	static Hash From(const char& x);

	static Hash From(const Point& x);

	template<typename T>
	static Hash From(const T& value)
	{
		static_assert(std::is_default_constructible_v<T>, "From(const T&) requires T to be default constructible.");
		return BitDiffusion(std::hash<T>{}(value));
	}

    template<typename... Rest>
		requires (std::same_as<std::remove_cvref_t<Rest>, Hash> && ...)
	static Hash Combine(const Hash& first, const Hash& second, const Rest&... rest)
    {
		auto mix = [](Hash& a, const Hash& b)
			{
				if (b.IsEmpty())
					return;

				if (a.IsEmpty())
				{
					a = b;
					return;
				}

				a.value ^= b.value + 0x9e3779b97f4a7c15ULL + (a.value << 6) + (a.value >> 2);
			};

		Hash result = first;
		mix(result, second);
		((mix(result, rest)), ...);

        return result.IsEmpty() ? result : Hash::BitDiffusion(result.value);
    }

	bool IsEmpty() const;

	uint64_t Value() const;

	ID ToID() const;

private:

	Hash(uint64_t hash, bool empty = false);

	static Hash BitDiffusion(uint64_t x);

private:

	uint64_t value = 0;
	bool empty = false;

};


#pragma once

#include <stdint.h>

// 64 slots bit-wise manager
class Flag
{
public:

	Flag() = default;

	// Use the 0b0 notation
	// Example: 0b1001 means flag 1 and 4 are true, all the others are false
	Flag(uint64_t flag);

	const static Flag AllFalse;
	
	const static Flag AllTrue;

	// Returns the binary flags in a number of 8 bits
	uint64_t Binary() const;

	// Set an specific flag from slot 0 to 63
	void Set(unsigned int index, bool state);

	// Set the whole flag
	// Use the 0b0 notation
	// Example: 0b1001 means flag 1 and 4 are true, all the others are false
	void Set(uint64_t flag);

	// Get an specific flag
	bool Get(unsigned int index) const;

	// Get and specific flag. If the flag is true, it sets it to false
	bool Trigger(unsigned int index);

	// Inverts the state of all the flags
	void Invert();

	bool IsAnyTrue() const;

	// Clear the flag.
	// state: if false sets all the flags to false, if true sets all flags to true
	void Clear(bool state = false);

	bool operator[](unsigned int index) const;

	void operator()(unsigned int index, bool state);

	static constexpr uint8_t maxCapacity = 64;

private:

	uint64_t flag = 0;
};


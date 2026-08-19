#pragma once

#include <stdint.h>

class Hash;

class ID
{

	friend class Hash;

public:

	ID(const ID& id);
	
	ID(ID&& id) noexcept;

	static const ID Empty;
	static const ID New();

	bool IsEmpty() const;

	bool operator==(const ID& id) const;

	void operator=(const ID& id);

	void operator=(ID&& id) = delete;

private:

	ID();

	ID(uint64_t id);

private:

	uint64_t id = 0;
	bool empty = false;
};

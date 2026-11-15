#pragma once

class bool2
{
public:

	bool2();

	bool2(const bool2& other);

	bool2(bool2&& other) noexcept;

	bool2(bool v);

	bool2(bool x, bool y);

	// One or the other is true
	bool Or() const;

	// One and the other is true
	bool And() const;

	// Nor one or the other is true
	bool None() const;

public:

	bool2& operator=(const bool2& other);

	bool2& operator=(bool2&& other) noexcept;

	bool2 operator==(const bool2& other) const;
	
	bool2 operator!=(const bool2& other) const;

	bool2 operator==(const bool& other) const;

	bool2 operator!=(const bool& other) const;

	bool operator[](int index) const;

	bool2 operator!() const;

public:

	bool x = false, y = false;

};

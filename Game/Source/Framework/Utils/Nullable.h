#pragma once

#include "Framework/Engine/Debug.h"

#include <type_traits>
#include <utility>

template<class T, bool Owns = false>
class Nullable
{
	template<class, bool>
	friend class Nullable;

	static_assert(!Owns || std::is_pointer_v<T>, "Null<T, true> only allowed when T is a pointer mode with heap allocated memmory.");

public:

	constexpr Nullable() :
		exists(false)
	{ }

	Nullable(const T& value) :
		exists(true)
	{
		static_assert(std::is_copy_constructible_v<T>, "Nullable(const T&) requires T to be copy constructible.");
		new(Ptr()) T(value);
	}

	Nullable(T&& value)
		: exists(true)
	{
		static_assert(std::is_move_constructible_v<T>, "Nullable(T&&) requires T to be move constructible.");

		if constexpr (Owns && std::is_pointer_v<T>)
		{
			new (Ptr()) T(value);
			value = nullptr;
		}
		else
		{
			new (Ptr()) T(std::move(value));
		}
	}

	constexpr Nullable(std::nullptr_t) :
		exists(false)
	{

	}

	template<bool OtherOwns>
	Nullable(const Nullable<T, OtherOwns>& other)
	{
		static_assert(std::is_copy_constructible_v<T>, "Nullable copy constructor requires T to be copy constructible.");
		static_assert(!(Owns && OtherOwns), "Copying from a Nullable<T, true> to another Nullable<T, true> is forbidden.");
		static_assert(!(Owns && !OtherOwns), "Cannot create a Nullable<T, true> from a Nullable<T, false>.");
		
		if (!other.exists)
		{
			exists = false;
			return;
		}

		new(Ptr()) T(*other.Ptr());
		exists = true;
	}

	Nullable(const Nullable& other)
		requires (!Owns)
	{
		static_assert(std::is_copy_constructible_v<T>, "Nullable copy constructor requires T to be copy constructible.");

		if (!other.exists)
		{
			exists = false;
			return;
		}

		new (Ptr()) T(*other.Ptr());
		exists = true;
	}

	Nullable(const Nullable& other)
		requires (Owns)
	= delete;

	Nullable(Nullable&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
	{
		static_assert(std::is_move_constructible_v<T>, "Nullable move constructor requires T to be move constructible.");

		if (!other.exists)
		{
			exists = false;
			return;
		}

		new (Ptr()) T(std::move(*other.Ptr()));
		exists = true;

		other.Ptr()->~T();
		other.exists = false;
	}

	~Nullable()
	{
		Clear();
	}

	T& Value()
	{
		Debug::Assert(exists, "Accessing a Nullable value that doesn't exist!");
		return *Ptr();
	}

	const T& Value() const
	{
		Debug::Assert(exists, "Accessing a Nullable value that doesn't exist!");
		return *Ptr();
	}

	[[nodiscard]]
	bool HasValue() const
	{
		return exists;
	}

	void Clear()
	{
		if (!exists)
			return;

		DestroyValue(*Ptr());
		Ptr()->~T();

		exists = false;
	}

	[[nodiscard]]
	T ValueOr(const T& fallback) const
	{
		static_assert(std::is_copy_constructible_v<T>, "Nullable::ValueOr() requires T to be copy constructible.");
		return exists ? *Ptr() : fallback;
	}

	template<class... Args>
	T& Emplace(Args&&... args)
	{
		static_assert(std::is_constructible_v<T, Args&&...>, "Nullable::Emplace() requires T to be constructible from the provided arguments.");

		Clear();

		new (Ptr()) T(std::forward<Args>(args)...);
		exists = true;

		return *Ptr();
	}

public:

	Nullable& operator=(std::nullptr_t)
	{
		Clear();
		return *this;
	}

	Nullable& operator=(const T& value)
	{
		static_assert(std::is_copy_constructible_v<T>, "Nullable::operator=(const T&) requires T to be copy constructible.");
		static_assert(std::is_copy_assignable_v<T>, "Nullable::operator=(const T&) requires T to be copy assignable.");

		if (exists)
		{
			DestroyValue(*Ptr());
			*Ptr() = value;
		}
		else
			new (Ptr()) T(value);

		exists = true;

		return *this;
	}

	Nullable& operator=(T&& value)
	{
		static_assert(std::is_move_constructible_v<T>, "Nullable::operator=(T&&) requires T to be move constructible.");
		static_assert(std::is_move_assignable_v<T>, "Nullable::operator=(T&&) requires T to be move assignable.");

		if (exists)
		{
			if constexpr (Owns && std::is_pointer_v<T>)
			{
				DestroyValue(*Ptr());
				*Ptr() = value;
				value = nullptr;
			}
			else
			{
				*Ptr() = std::move(value);
			}
		}
		else
		{
			if constexpr (Owns && std::is_pointer_v<T>)
			{
				new (Ptr()) T(value);
				value = nullptr;
			}
			else
			{
				new (Ptr()) T(std::move(value));
			}

			exists = true;
		}

		return *this;
	}

	template<bool OtherOwns>
	Nullable& operator=(const Nullable<T, OtherOwns>& other)
	{
		static_assert(std::is_copy_constructible_v<T>, "Nullable copy assignment requires T to be copy constructible.");
		static_assert(std::is_copy_assignable_v<T>, "Nullable copy assignment requires T to be copy assignable.");
		static_assert(!(Owns && OtherOwns), "Copying from a Nullable<T, true> to another Nullable<T, true> is forbidden.");
		static_assert(!(Owns && !OtherOwns), "Cannot create a Nullable<T, true> from a Nullable<T, false>.");

		if (!other.exists)
			Clear();
		else
		{
			if (exists)
			{
				DestroyValue(*Ptr());
				*Ptr() = *other.Ptr();
			}
			else
				new(Ptr()) T(*other.Ptr());

			exists = true;
		}
		return *this;
	}

	Nullable& operator=(const Nullable& other)
		requires (Owns)
	= delete;

	T& operator*()
	{
		Debug::Assert(exists, "Dereferencing an empty Nullable.");
		return *Ptr();
	}

	const T& operator*() const
	{
		Debug::Assert(exists, "Dereferencing an empty Nullable.");
		return *Ptr();
	}

	T* operator->()
	{
		Debug::Assert(exists, "Dereferencing an empty Nullable.");
		return Ptr();
	}

	const T* operator->() const
	{
		Debug::Assert(exists, "Dereferencing an empty Nullable.");
		return Ptr();
	}

	Nullable& operator=(Nullable&& other) noexcept(
		std::is_nothrow_move_constructible_v<T>&&
		std::is_nothrow_move_assignable_v<T>)
	{
		static_assert(std::is_move_constructible_v<T>, "Nullable move assignment requires T to be move constructible.");
		static_assert(std::is_move_assignable_v<T>, "Nullable move assignment requires T to be move assignable.");

		if (this == &other)
			return *this;

		if (!other.exists)
		{
			Clear();
			return *this;
		}

		if (exists)
		{
			DestroyValue(*Ptr());
			*Ptr() = std::move(*other.Ptr());
		}
		else
		{
			new (Ptr()) T(std::move(*other.Ptr()));
			exists = true;
		}


		other.Ptr()->~T();
		other.exists = false;

		return *this;
	}

	Nullable& operator=(const Nullable& other)
		requires (!Owns)
	{
		static_assert(std::is_copy_constructible_v<T>, "Nullable copy assignment requires T to be copy constructible.");
		static_assert(std::is_copy_assignable_v<T>, "Nullable copy assignment requires T to be copy assignable.");

		if (this == &other)
			return *this;

		if (!other.exists)
		{
			Clear();
			return *this;
		}

		if (exists)
			*Ptr() = *other.Ptr();
		else
			new (Ptr()) T(*other.Ptr());

		exists = true;
		return *this;
	}

	explicit operator bool() const
	{
		return exists;
	}

	bool operator==(const Nullable& other) const
	{
		static_assert(requires(const T & a, const T & b)
		{
			{ a == b } -> std::convertible_to<bool>;
		},
			"Nullable::operator== requires T to support operator==.");

		if (!exists && !other.exists)
			return true;

		if (exists != other.exists)
			return false;

		return *Ptr() == *other.Ptr();
	}

	bool operator!=(const Nullable& other) const
	{
		return !(*this == other);
	}

	bool operator==(const T& value) const
	{
		static_assert(requires(const T & a, const T & b)
		{
			{ a == b } -> std::convertible_to<bool>;
		},
			"Nullable::operator== requires T to support operator==.");

		if (!exists)
			return false;

		return *Ptr() == value;
	}

	bool operator!=(const T& value) const
	{
		return !(*this == value);
	}

	friend bool operator==(const T& value, const Nullable& nullable)
	{
		return nullable == value;
	}

	friend bool operator!=(const T& value, const Nullable& nullable)
	{
		return nullable != value;
	}

	bool operator==(std::nullptr_t) const
	{
		return !exists;
	}

	bool operator!=(std::nullptr_t) const
	{
		return exists;
	}

	friend bool operator==(std::nullptr_t, const Nullable& nullable)
	{
		return !nullable.exists;
	}

	friend bool operator!=(std::nullptr_t, const Nullable& nullable)
	{
		return nullable.exists;
	}

	bool operator<(const Nullable& other) const
	{
		static_assert(requires(const T & a, const T & b)
		{
			{ a < b } -> std::convertible_to<bool>;
		},
			"Nullable::operator< requires T to support operator<.");

		Debug::Assert(exists && other.exists,
			"Nullable::operator< cannot compare empty Nullable values.");

		return *Ptr() < *other.Ptr();
	}

	bool operator>(const Nullable& other) const
	{
		static_assert(requires(const T & a, const T & b)
		{
			{ a > b } -> std::convertible_to<bool>;
		},
			"Nullable::operator> requires T to support operator>.");

		Debug::Assert(exists && other.exists,
			"Nullable::operator> cannot compare empty Nullable values.");

		return *Ptr() > *other.Ptr();
	}

	bool operator<=(const Nullable& other) const
	{
		static_assert(requires(const T & a, const T & b)
		{
			{ a <= b } -> std::convertible_to<bool>;
		},
			"Nullable::operator<= requires T to support operator<=.");

		Debug::Assert(exists && other.exists,
			"Nullable::operator<= cannot compare empty Nullable values.");

		return *Ptr() <= *other.Ptr();
	}

	bool operator>=(const Nullable& other) const
	{
		static_assert(requires(const T & a, const T & b)
		{
			{ a >= b } -> std::convertible_to<bool>;
		},
			"Nullable::operator>= requires T to support operator>=.");

		Debug::Assert(exists && other.exists,
			"Nullable::operator>= cannot compare empty Nullable values.");

		return *Ptr() >= *other.Ptr();
	}

	bool operator<(const T& value) const
	{
		static_assert(requires(const T & a, const T & b)
		{
			{ a < b } -> std::convertible_to<bool>;
		},
			"Nullable::operator< requires T to support operator<.");

		Debug::Assert(exists,
			"Nullable::operator< Cannot compare an empty Nullable.");

		return *Ptr() < value;
	}

	bool operator>(const T& value) const
	{
		static_assert(requires(const T & a, const T & b)
		{
			{ a > b } -> std::convertible_to<bool>;
		},
			"Nullable::operator> requires T to support operator>.");

		Debug::Assert(exists,
			"Nullable::operator> Cannot compare an empty Nullable.");

		return *Ptr() > value;
	}

	bool operator<=(const T& value) const
	{
		static_assert(requires(const T & a, const T & b)
		{
			{ a <= b } -> std::convertible_to<bool>;
		},
			"Nullable::operator<= requires T to support operator<=.");

		Debug::Assert(exists,
			"Nullable::operator<= Cannot compare an empty Nullable.");

		return *Ptr() <= value;
	}

	bool operator>=(const T& value) const
	{
		static_assert(requires(const T & a, const T & b)
		{
			{ a >= b } -> std::convertible_to<bool>;
		},
			"Nullable::operator>= requires T to support operator>=.");

		Debug::Assert(exists,
			"Nullable::operator>= Cannot compare an empty Nullable.");

		return *Ptr() >= value;
	}

private:

	T* Ptr()
	{
		return reinterpret_cast<T*>(storage);
	}

	const T* Ptr() const
	{
		return reinterpret_cast<const T*>(storage);
	}

	void DestroyValue(T& value)
	{
		if constexpr (Owns && std::is_pointer_v<T>)
		{
			delete value;
		}
	}

private:

	alignas(T) unsigned char storage[sizeof(T)];
	bool exists;

};
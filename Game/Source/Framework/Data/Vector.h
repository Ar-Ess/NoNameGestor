#pragma once

#include "Framework/Engine/Debug.h"
#include "Framework/Data/Array.h"

#include <exception>

// Vector with index-based access, iteration, search utilities, and callable-based operations (Find / Iterate / Sort).
// Default: Vector<T> is Vector<T, false>, which means that does not own memory if heap allocated and will be only a container for it.
// If T is not a pointer, the data can't be owned. Vector will automatically erase it when destroy.
// Set Vector<T, true> to own the data and destroy it automatically. Do not set true for non-heap-allocated pointers.
template<class T, bool Owns = false>
class Vector
{
	template<class, bool>
	friend class Vector;

	static_assert(!Owns || std::is_pointer_v<T>, "Vector<T, true> only allowed when T is a pointer mode");
	static_assert(std::is_destructible_v<T>, "Vector<T>: T must be destructible.");

private:

public:

	Vector() :
		data(nullptr), size(0), capacity(0)
	{

	}

	Vector(int reserve) :
		data(nullptr), size(0), capacity(reserve)
	{
		ReserveInternal(reserve);
	}

	// Initialize an amount by initValue, or default construct it if can't be copy-constructed
	Vector(int amount, const T& initValue) :
		data(nullptr), size(0), capacity(0)
	{
		static_assert(std::is_copy_constructible_v<T> || std::is_default_constructible_v<T>, "To use Vector<T>(int amount, const T& initValue), mode T must be either copy constructible or default constructible");

		if (amount > 0)
		{
			ReserveInternal(amount);

			for (int i = 0; i < amount; ++i)
			{
				T* ptr = data + i;
				if constexpr (std::is_copy_constructible_v<T>)
				{
					new (ptr) T(initValue);
				}
				else if constexpr (std::is_default_constructible_v<T>)
				{
					Debug::Log("Vector(amount, initValue) couldn't copy construct with initValue, so default constructor is used.");
					new (ptr) T();
				}
			}
			size = amount;
		}
	}

	Vector(const Vector& other) requires (!Owns && std::is_copy_constructible_v<T>)
	{
		static_assert(
			std::is_copy_constructible_v<T>,
			"Vector::Vector(const Vector&): T must be copy constructible."
			);

		other.CopyTo(*this);
	}

	Vector(const Vector&) requires (Owns) = delete;

	Vector(Vector&& other) noexcept
	{
		data = other.data;
		size = other.size;
		capacity = other.capacity;

		other.data = nullptr;
		other.size = 0;
		other.capacity = 0;
	}

	// Creates another instance of the Vector pointing to the same values but never owning them
	void CopyTo(Vector<T>& out) const requires (std::is_copy_constructible_v<T>)
	{
		static_assert(
			std::is_copy_constructible_v<T>,
			"Vector::CopyTo(): T must be copy constructible."
			);

		if (reinterpret_cast<const void*>(&out) == reinterpret_cast<const void*>(this))
			return;

		out.Clear();

		if (size == 0)
			return;

		out.ReserveInternal(size);

		for (int i = 0; i < size; ++i)
		{
			out.PlaceValue(out.data + i, data[i], true);
		}

		out.size = size;
	}

	void CopyTo(Vector<T, true>&) const = delete;

	~Vector()
	{
		Clear();
	}

	int Size() const
	{
		return size;
	}

	bool IsEmpty() const
	{
		return size == 0;
	}

	// Removes all elements and capacity of the vector 
	// If "Vector<T, true>", it will delete the heap allocated data.
	void Clear()
	{
		// data is a raw info array, only size amount from capacity has real T data to delete
		for (int i = 0; i < size; ++i)
		{
			DestroyValue(data[i]);
			DestroyPlace(data[i]);
		}

		// operator to delete raw data (delete[] does not work!)
		operator delete(data);

		data = nullptr;
		size = 0;
		capacity = 0;
	}

	// Removes all the elements but keeps the allocated capacity
	// If "Vector<T, true>", it will delete the heap allocated data.
	void Reset()
	{
		// data is a raw info array, only size amount from capacity has real T data to delete
		for (int i = 0; i < size; ++i)
		{
			DestroyValue(data[i]);
			DestroyPlace(data[i]);
		}

		size = 0;
	}

	void PushBack(const T& value)
	{
		if (size == capacity)
			ReserveInternal();

		PlaceValue(data + size, value, true);
		++size;
	}

	void PushBack(T&& value)
	{
		if (size == capacity)
			ReserveInternal();

		PlaceValue(data + size, std::move(value), true);
		++size;
	}

	// Emplaces directly an object to avoid copyes
	// Ex: Class(int a, int b). Vector<Class>.EmplaceBack(1, 2); instead of creating and copying
	// Does not work for Pointer T
	template<typename... Args>
		requires (!std::is_pointer_v<T> && std::is_constructible_v<T, Args...>)
	void EmplaceBack(Args&&... args)
	{
		static_assert(
			!std::is_pointer_v<T>,
			"Vector::EmplaceBack(): EmplaceBack cannot be used when T is a pointer. "
			"Use PushBack(pointer) instead."
			);

		static_assert(
			std::is_constructible_v<T, Args...>,
			"Vector::EmplaceBack(): T cannot be constructed from the supplied arguments. "
			"Check that T has a matching constructor."
			);

		if (size == capacity)
			ReserveInternal();

		new (data + size) T(std::forward<Args>(args)...);
		++size;
	}

	bool Insert(const T& value, int index)
	{
		if (index < 0 || index > size)
			return false;

		if (size == capacity)
			ReserveInternal();

		if (index == size)
		{
			PushBack(value);
			return true;
		}

		PlaceValue(data + size, std::move(data[size - 1]), true);

		for (int i = size - 1; i > index; --i)
		{
			PlaceValue(data + i, std::move(data[i - 1]), false);
		}

		PlaceValue(data + index, value, false);

		++size;
		return true;
	}

	bool Insert(T&& value, int index)
	{
		if (index < 0 || index > size)
			return false;

		if (size == capacity)
			ReserveInternal();

		if (index == size)
		{
			PushBack(std::move(value));
			return true;
		}

		PlaceValue(data + size, std::move(data[size - 1]), true);

		for (int i = size - 1; i > index; --i)
		{
			PlaceValue(data + i, std::move(data[i - 1]), false);
		}

		PlaceValue(data + index, std::move(value), false);

		++size;
		return true;
	}

	bool Erase(int index)
	{
		if (index < 0 || index >= size)
			return false;

		DestroyValue(data[index]);

		for (int i = index; i < size - 1; ++i)
		{
			PlaceValue(data + i, std::move(data[i + 1]), false);
		}

		DestroyPlace(data[size - 1]);

		--size;
		return true;
	}

	bool Erase(const Array<int>& indexs)
	{
		if (size == 0)
			return true;

		if (indexs.Size() == 1)
			return Erase(indexs[0]);

		if (indexs.Size() == size)
			Reset();

		bool* toDelete = new bool[size]();

		bool allGood = true;

		indexs.Iterate([&](int idx)
			{
				if (idx < 0 || idx >= size)
				{
					allGood = false;
					return;
				}
				toDelete[idx] = true;
			}
		);

		for (int i = 0; i < size; ++i)
		{
			if (toDelete[i])
				DestroyValue(data[i]);
		}

		int write = 0;
		for (int read = 0; read < size; ++read)
		{
			if (!toDelete[read])
			{
				if (write != read)
					PlaceValue(data + write, std::move(data[read]), false);

				++write;
			}
		}

		for (int i = write; i < size; ++i)
			DestroyPlace(data[i]);

		size = write;
		delete[] toDelete;

		return allGood;
	}

	T& At(int index)
	{
		Debug::Assert(!IsEmpty(), "This Vector is empty!");
		Debug::Assert(index >= 0 && index < size, "The index is out of bounds!");
		return data[index];
	}

	const T& At(int index) const
	{
		Debug::Assert(!IsEmpty(), "This Vector is empty!");
		Debug::Assert(index >= 0 && index < size, "The index is out of bounds!");
		return data[index];
	}

	bool TryAt(int index, OutParameter<T> out)
	{
		if (index < 0 || index >= size)
			return false;

		out = &data[index];
		return true;
	}

	bool TryAt(int index, OutParameter<const T> out) const
	{
		if (index < 0 || index >= size)
			return false;

		out = &data[index];
		return true;
	}

	T& Front()
	{
		Debug::Assert(!IsEmpty(), "This Vector is empty!");
		return data[0];
	}

	const T& Front() const
	{
		Debug::Assert(!IsEmpty(), "This Vector is empty!");
		return data[0];
	}

	T& Back()
	{
		Debug::Assert(!IsEmpty(), "This Vector is empty!");
		return data[size - 1];
	}

	const T& Back() const
	{
		Debug::Assert(!IsEmpty(), "This Vector is empty!");
		return data[size - 1];
	}

	bool PopBack()
	{
		if (IsEmpty())
			return false;

		DestroyValue(data[size - 1]);
		DestroyPlace(data[size - 1]);
		--size;

		return true;
	}

	bool PopFront()
	{
		if (IsEmpty())
			return false;

		DestroyValue(data[0]);

		for (int i = 0; i < size - 1; ++i)
		{
			PlaceValue(data + i, std::move(data[i + 1]), false);
		}

		DestroyPlace(data[size - 1]);

		--size;
		return true;
	}

	bool Reserve(int amount)
	{
		if (amount <= capacity)
			return false;

		ReserveInternal(amount);
		return true;
	}

	bool ShrinkToFit()
	{
		if (size >= capacity)
			return false;

		if (IsEmpty())
			Clear();
		else
			ReserveInternal(size);

		return true;
	}

	int Capacity() const
	{
		return capacity;
	}

	T* Data()
	{
		return data;
	}

	const T* Data() const
	{
		return data;
	}

	// Assigns a new value to a specific index
	// If Owns is true, it will attempt to delete the old value if heap allocated
	bool Assign(const T& value, int index)
	{
		if (index < 0 || index >= size)
			return false;

		DestroyValue(data[index]);
		PlaceValue(data + index, value, false);
		return true;
	}

	// Assigns a new value to a specific index
	// If Owns is true, it will attempt to delete the old value if heap allocated
	bool Assign(T&& value, int index)
	{
		if (index < 0 || index >= size)
			return false;

		DestroyValue(data[index]);
		PlaceValue(data + index, std::move(value), false);
		return true;
	}

	// Iterates all elements. Set reverse to true to iterate backwards.
	// Callable can be:
	//  - void(T&) or bool(T&) -> return false to stop iteration
	//  - void(T&, int) or bool(T&, int) -> gives the current index as a parameter
	//  - void(T&, int, int) or bool(T&, int, int) -> gives the LinkedList size as a parameter
	// Use const T& in the function for a const version of Iterate
	template<typename Callable>
	void Iterate(Callable&& func, bool reverse = false)
	{
		IterateInternal(*this, std::forward<Callable>(func), reverse);
	}

	// Iterates all elements. Set reverse to true to iterate backwards.
	// Callable can be:
	//  - void(T&) or bool(T&) -> return false to stop iteration
	//  - void(T&, int) or bool(T&, int) -> gives the current index as a parameter
	//  - void(T&, int, int) or bool(T&, int, int) -> gives the LinkedList size as a parameter
	// Use T& in the function for a non-const version of Iterate
	template<typename Callable>
	void Iterate(Callable&& func, bool reverse = false) const
	{
		IterateInternal(*this, std::forward<Callable>(func), reverse);
	}

	template<typename U>
		requires Comparable<T, U>
	int Find(const U& value) const
	{
		static_assert(Comparable<T, U>, "Vector::Find(const U&): T and U must be comparable using operator==."
			"Check that:\n"
			"  - T defines operator==(const U&) or U defines operator==(const T&).\n"
			"  - The operator accepts const references.\n"
			"  - The operator returns bool (or a type convertible to bool).");

		for (int i = 0; i < size; ++i)
		{
			if constexpr (std::is_pointer_v<T>)
			{
				if (data[i] == nullptr || value == nullptr)
				{
					if (data[i] == value)
						return i;
				}
				else if constexpr (DerefComparable<T, U>)
				{
					if (*data[i] == *value)
					{
						return i;
					}
				}
				else
				{
					if (data[i] == value)
						return i;
				}
			}
			else
			{
				if (data[i] == value)
					return i;
			}
		}

		return -1;
	}

	template<typename Callable>
		requires Predicate<Callable, T>
	int Find(Callable&& predicate) const
	{
		static_assert(
			Predicate<Callable, T>,
			"Vector::Find(Callable&&): The callable must be invocable with a 'const T&' "
			"and return a value convertible to bool. Check that:\n"
			"  - The parameter type matches 'const T&' (or is compatible).\n"
			"  - The callable returns bool (or a type convertible to bool).\n"
			"  - operator() is marked const if you are passing a const functor."
			);

		for (int i = 0; i < size; ++i)
		{
			if (predicate(data[i]))
				return i;
		}

		return -1;
	}

	template<typename U>
		requires Comparable<T, U>
	Array<int> FindAll(const U& value) const
	{
		static_assert(Comparable<T, U>, "Vector::FindAll(const U&): T and U must be comparable using operator==."
			"Check that:\n"
			"  - T defines operator==(const U&) or U defines operator==(const T&).\n"
			"  - The operator accepts const references.\n"
			"  - The operator returns bool (or a type convertible to bool).");

		int* result = new int[size];

		int count = 0;
		for (int i = 0; i < size; ++i)
		{
			if constexpr (std::is_pointer_v<T>)
			{
				if (data[i] == nullptr || value == nullptr)
				{
					if (data[i] == value)
						result[count++] = i;
				}
				else if constexpr (DerefComparable<T, U>)
				{
					if (*data[i] == *value)
						result[count++] = i;
				}
				else if (data[i] == value)
				{
					result[count++] = i;
				}
			}
			else
			{
				if (data[i] == value)
					result[count++] = i;
			}
		}

		Array ret = Array<int>(result, count);
		delete[] result;

		return ret;
	}

	template<typename Callable>
		requires Predicate<Callable, T>
	Array<int> FindAll(Callable&& predicate) const
	{
		static_assert(
			Predicate<Callable, T>,
			"Vector::FindAll(Callable&&): The callable must be invocable with a 'const T&' "
			"and return a value convertible to bool. Check that:\n"
			"  - The parameter type matches 'const T&' (or is compatible).\n"
			"  - The callable returns bool (or a type convertible to bool).\n"
			"  - operator() is marked const if you are passing a const functor."
			);

		int* result = new int[size];

		int count = 0;
		for (int i = 0; i < size; ++i)
		{
			if (predicate(data[i]))
				result[count++] = i;
		}

		Array ret = Array<int>(result, count);
		delete[] result;

		return ret;
	}

	template<typename U>
		requires Comparable<T, U>
	bool Contains(const U& value) const
	{
		static_assert(Comparable<T, U>, "Vector::Contains(const U&): T and U must be comparable using operator==."
			"Check that:\n"
			"  - T defines operator==(const U&) or U defines operator==(const T&).\n"
			"  - The operator accepts const references.\n"
			"  - The operator returns bool (or a type convertible to bool).");
		return Find(value) != -1;
	}

	template<typename Callable>
		requires Predicate<Callable, T>
	bool Contains(Callable&& predicate) const
	{
		static_assert(
			Predicate<Callable, T>,
			"Vector::Contains(Callable&&): The callable must be invocable with a 'const T&' "
			"and return a value convertible to bool. Check that:\n"
			"  - The parameter type matches 'const T&' (or is compatible).\n"
			"  - The callable returns bool (or a type convertible to bool).\n"
			"  - operator() is marked const if you are passing a const functor."
			);

		return Find(std::forward<Callable>(predicate)) != -1;
	}

	// Retrieves first element matching predicate into 'ret'. Returns false if not found.
	// OutPatameter: Input a T pointer (T*) to get the result
	template<typename Callable>
		requires Predicate<Callable, T>
	bool Retrieve(Callable&& predicate, OutParameter<T> out)
	{
		static_assert(
			Predicate<Callable, T>,
			"Vector::Retrieve(Callable&&, OutParameter<T>): The callable must be invocable with a 'const T&' "
			"and return a value convertible to bool. Check that:\n"
			"  - The parameter type matches 'const T&' (or is compatible).\n"
			"  - The callable returns bool (or a type convertible to bool).\n"
			"  - operator() is marked const if you are passing a const functor."
			);

		for (int i = 0; i < size; ++i)
		{
			if (predicate(data[i]))
			{
				if constexpr (std::is_pointer_v<T>)
					out = data[i];
				else
					out = &data[i];
				return true;
			}
		}

		return false;
	}

	// Retrieves first element matching predicate into 'ret'. Returns false if not found.
	// OutPatameter: Input a T pointer (T*) to get the result
	template<typename Callable>
		requires Predicate<Callable, T>
	bool Retrieve(Callable&& predicate, OutParameter<const T> out) const
	{
		static_assert(
			Predicate<Callable, T>,
			"Vector::Retrieve(Callable&&, OutParameter<T>): The callable must be invocable with a 'const T&' "
			"and return a value convertible to bool. Check that:\n"
			"  - The parameter type matches 'const T&' (or is compatible).\n"
			"  - The callable returns bool (or a type convertible to bool).\n"
			"  - operator() is marked const if you are passing a const functor."
			);

		for (int i = 0; i < size; ++i)
		{
			if (predicate(data[i]))
			{
				if constexpr (std::is_pointer_v<T>)
					out = data[i];
				else
					out = &data[i];
				return true;
			}
		}

		return false;
	}

	template<typename Callable>
		requires Predicate<Callable, T>
	bool RetrieveAll(Callable&& predicate, Array<OutParameterList<T>>& ret) const
	{
		static_assert(
			Predicate<Callable, T>,
			"Vector::RetrieveAll(Callable&&, Array<OutParameterList<T>>&): The callable must be invocable with a 'const T&' "
			"and return a value convertible to bool. Check that:\n"
			"  - The parameter type matches 'const T&' (or is compatible).\n"
			"  - The callable returns bool (or a type convertible to bool).\n"
			"  - operator() is marked const if you are passing a const functor."
			);

		T* result = new T[size];

		int count = 0;
		bool found = false;
		for (int i = 0; i < size; ++i)
		{
			if (predicate(data[i]))
			{
				if constexpr (std::is_pointer_v<T>)
					result[count++] = data[i];
				else
					result[count++] = &data[i];
				found = true;
			}
		}

		ret.Clear();
		ret = Array<OutParameterList<T>>(result, count);
		delete[] result;

		return found;
	}

	bool Swap(int a, int b)
	{
		if (a < 0 || b < 0 || a >= size || b >= size)
			return false;

		if (a == b)
			return true;

		std::swap(data[a], data[b]);
		return true;
	}

	template<typename Callable>
		requires Comparator<T, U>
	void Sort(Callable&& comp)
	{
		static_assert(
			Comparator<Callable, T>,
			"Vector::Sort(comparator): The comparator must be invocable with "
			"two 'const T&' parameters and return a value convertible to bool. "
			"Check that:\n"
			"  - The comparator signature is compatible with "
			"'bool(const T&, const T&)'.\n"
			"  - The comparator returns bool (or a type convertible to bool).\n"
			"  - operator() is marked const if you are passing a const functor.\n"
			"  - The comparator defines a strict weak ordering (like std::less)."
			);

		if (size <= 1)
			return;

		if (size <= 20)
			InsertionSort(std::forward<Callable>(comp));
		else
			QuickSort(0, size - 1, std::forward<Callable>(comp));
	}

	Array<T> ToArray() const
	{
		if (IsEmpty())
			return Array<T, false>();

		Array<T, false> ret = Array<T, false>(size);
		for (int i = 0; i < size; ++i)
			ret[i] = data[i];

		return ret;
	}

	template<typename U>
		requires Comparable<T, U>
	bool Remove(const U& value)
	{
		static_assert(Comparable<T, U>, "Vector::Remove(const U&): T and U must be comparable using operator==."
			"Check that:\n"
			"  - T defines operator==(const U&) or U defines operator==(const T&).\n"
			"  - The operator accepts const references.\n"
			"  - The operator returns bool (or a type convertible to bool).");

		int index = Find(value);

		if (index == -1)
			return false;

		return Erase(index);
	}

	template<typename Callable>
		requires Predicate<Callable, T>
	bool Remove(Callable&& predicate)
	{
		static_assert(
			Predicate<Callable, T>,
			"Vector::Remove(Callable&&,): The callable must be invocable with a 'const T&' "
			"and return a value convertible to bool. Check that:\n"
			"  - The parameter type matches 'const T&' (or is compatible).\n"
			"  - The callable returns bool (or a type convertible to bool).\n"
			"  - operator() is marked const if you are passing a const functor."
			);

		int index = Find(std::forward<Callable>(predicate));

		if (index == -1)
			return false;

		return Erase(index);
	}

	template<typename U>
		requires Comparable<T, U>
	bool RemoveAll(const U& value)
	{
		static_assert(Comparable<T, U>, "Vector::RemoveAll(const U&): T and U must be comparable using operator==."
			"Check that:\n"
			"  - T defines operator==(const U&) or U defines operator==(const T&).\n"
			"  - The operator accepts const references.\n"
			"  - The operator returns bool (or a type convertible to bool).");

		Array<int> indexes = FindAll(value);

		if (indexes.IsEmpty())
			return false;

		Erase(indexes);
		return true;
	}

	template<typename Callable>
		requires Predicate<Callable, T>
	bool RemoveAll(Callable&& predicate)
	{
		static_assert(
			Predicate<Callable, T>,
			"Vector::Remove(Callable&&,): The callable must be invocable with a 'const T&' "
			"and return a value convertible to bool. Check that:\n"
			"  - The parameter type matches 'const T&' (or is compatible).\n"
			"  - The callable returns bool (or a type convertible to bool).\n"
			"  - operator() is marked const if you are passing a const functor."
			);

		Array<int> indexes = FindAll(std::forward<Callable>(predicate));

		if (indexes.IsEmpty())
			return false;

		Erase(indexes);
		return true;
	}

public:

	T& operator[](int index)
	{
		return data[index];
	}

	const T& operator[](int index) const
	{
		return data[index];
	}

	Vector& operator=(const Vector& other) requires (!Owns)
	{
		static_assert(
			std::is_copy_constructible_v<T>,
			"Vector::operator=(const Vector&): T must be copy constructible."
			);

		if (this == &other)
			return *this;

		other.CopyTo(*this);

		return *this;
	}

	Vector& operator=(const Vector&) requires (Owns) = delete;

	Vector& operator=(Vector&& other) noexcept
	{
		if (this == &other)
			return *this;

		Clear();

		data = other.data;
		size = other.size;
		capacity = other.capacity;

		other.data = nullptr;
		other.size = 0;
		other.capacity = 0;

		return *this;
	}

private:

	void ReserveInternal(int amount = 0)
	{
		if (amount == 0)
			amount = (capacity == 0) ? 1 : capacity * 2;

		T* newData = static_cast<T*>(operator new(sizeof(T) * amount));
		T* oldData = data;

		int i = 0;

		try
		{
			for (; i < size; ++i)
				PlaceValue(newData + i, std::move(oldData[i]), true);
		}
		catch (const std::exception& e)
		{
			for (int j = 0; j < i; ++j)
				newData[j].~T();

			operator delete(newData);

			Debug::Assert(false, e.what());
			throw;
		}
		catch (...)
		{
			for (int j = 0; j < i; ++j)
				newData[j].~T();

			operator delete(newData);

			Debug::Assert(false, "Unknown exception during Vector reallocation");
			throw;
		}

		for (int i = 0; i < size; ++i)
			DestroyPlace(oldData[i]);

		operator delete(oldData);

		data = newData;
		capacity = amount;
	}

	void DestroyPlace(T& value)
	{
		if constexpr (!std::is_pointer_v<T>)
		{
			value.~T();
		}
	}

	void DestroyValue(T& value)
	{
		if constexpr (Owns && std::is_pointer_v<T>)
		{
			delete value;
			value = nullptr;
		}
	}

	void PlaceValue(T* ptr, const T& value, bool forceConstruct)
	{
		if (forceConstruct)
		{
			if constexpr (std::is_copy_constructible_v<T>)
			{
				new (ptr) T(value);
			}
			else
			{
				static_assert(
					std::is_copy_assignable_v<T> || std::is_copy_constructible_v<T>,
					"Vector: T must be copy assignable or copy constructible. "
					"Check that T implements a copy constructor or copy assignment operator."
					);
			}
		}
		else
		{
			if constexpr (std::is_copy_assignable_v<T>)
			{
				*ptr = value;
			}
			else if constexpr (std::is_copy_constructible_v<T>)
			{
				ptr->~T();
				new (ptr) T(value);
			}
			else
			{
				static_assert(
					std::is_copy_assignable_v<T> || std::is_copy_constructible_v<T>,
					"Vector: T must be copy assignable or copy constructible. "
					"Check that T implements a copy constructor or copy assignment operator."
					);
			}
		}
	}

	void PlaceValue(T* ptr, T&& value, bool forceConstruct)
	{
		if (forceConstruct)
		{
			if constexpr (std::is_move_constructible_v<T>)
			{
				new (ptr) T(std::move(value));
			}
			else
			{
				Debug::Assert(false, "Type is not assignable or constructible");
			}
		}
		else
		{
			if constexpr (std::is_move_assignable_v<T>)
			{
				*ptr = std::move(value);
			}
			else if constexpr (std::is_move_constructible_v<T>)
			{
				ptr->~T();
				new (ptr) T(std::move(value));
			}
			else if constexpr (std::is_copy_assignable_v<T>)
			{
				*ptr = value;
			}
			else if constexpr (std::is_copy_constructible_v<T>)
			{
				ptr->~T();
				new (ptr) T(value);
			}
			else
			{
				static_assert(
					std::is_move_assignable_v<T> ||
					std::is_move_constructible_v<T> ||
					std::is_copy_assignable_v<T> ||
					std::is_copy_constructible_v<T>,
					"Vector: T cannot be relocated. "
					"Check that T is move assignable, move constructible, "
					"copy assignable or copy constructible."
					);
			}
		}
	}

	template<typename Self, typename Callable>
	void IterateInternal(Self& self, Callable&& func, bool reverse) const
	{
		if (self.IsEmpty())
			return;

		using ValueType = std::conditional_t<
			std::is_const_v<Self>,
			const T&,
			T&
		>;

		int index = reverse ? self.size - 1 : 0;

		if (reverse)
		{
			for (int i = self.size - 1; i >= 0; --i)
			{
				ValueType value = self.data[i];

				bool shouldContinue = true;

				if constexpr (requires { func(value, index, self.size); })
				{
					if constexpr (std::is_convertible_v<decltype(func(value, index, self.size)), bool>)
						shouldContinue = func(value, index, self.size);
					else
						func(value, index, self.size);
				}
				else if constexpr (requires { func(value, index); })
				{
					if constexpr (std::is_convertible_v<decltype(func(value, index)), bool>)
						shouldContinue = func(value, index);
					else
						func(value, index);
				}
				else if constexpr (requires { func(value); })
				{
					if constexpr (std::is_convertible_v<decltype(func(value)), bool>)
						shouldContinue = func(value);
					else
						func(value);
				}
				else
				{
					static_assert(
						sizeof(Callable) == 0,
						"Vector::Iterate(): Invalid callable.\n"
						"Expected one of:\n"
						"  void(T&)\n"
						"  bool(T&)\n"
						"  void(T&, int)\n"
						"  bool(T&, int)\n"
						"  void(T&, int, int)\n"
						"  bool(T&, int, int)\n"
						"Use const T& instead of T& when iterating a const Vector."
						);
				}

				if (!shouldContinue)
					return;

				--index;
			}
		}
		else
		{
			for (int i = 0; i < self.size; ++i)
			{
				ValueType value = self.data[i];

				bool shouldContinue = true;

				if constexpr (requires { func(value, index, self.size); })
				{
					if constexpr (std::is_convertible_v<decltype(func(value, index, self.size)), bool>)
						shouldContinue = func(value, index, self.size);
					else
						func(value, index, self.size);
				}
				else if constexpr (requires { func(value, index); })
				{
					if constexpr (std::is_convertible_v<decltype(func(value, index)), bool>)
						shouldContinue = func(value, index);
					else
						func(value, index);
				}
				else if constexpr (requires { func(value); })
				{
					if constexpr (std::is_convertible_v<decltype(func(value)), bool>)
						shouldContinue = func(value);
					else
						func(value);
				}
				else
				{
					static_assert(
						sizeof(Callable) == 0,
						"Vector::Iterate(): Invalid callable.\n"
						"Expected one of:\n"
						"  void(T&)\n"
						"  bool(T&)\n"
						"  void(T&, int)\n"
						"  bool(T&, int)\n"
						"  void(T&, int, int)\n"
						"  bool(T&, int, int)\n"
						"Use const T& instead of T& when iterating a const Vector."
						);
				}

				if (!shouldContinue)
					return;

				++index;
			}
		}
	}

	template<typename Callable>
	void InsertionSort(Callable&& comp)
	{
		for (int i = 1; i < size; ++i)
		{
			T key = std::move(data[i]);
			int j = i - 1;

			while (j >= 0 && comp(key, data[j]))
			{
				data[j + 1] = std::move(data[j]);
				--j;
			}

			data[j + 1] = std::move(key);
		}
	}

	template<typename Callable>
	void QuickSort(int left, int right, Callable&& comp)
	{
		if (left >= right)
			return;

		int pivotIndex = Partition(left, right, std::forward<Callable>(comp));

		QuickSort(left, pivotIndex - 1, comp);
		QuickSort(pivotIndex + 1, right, comp);
	}

	template<typename Callable>
	int Partition(int left, int right, Callable&& comp)
	{
		T pivot = std::move(data[right]);

		int i = left - 1;

		for (int j = left; j < right; ++j)
		{
			if (comp(data[j], pivot))
			{
				++i;
				std::swap(data[i], data[j]);
			}
		}

		std::swap(data[i + 1], data[right]);
		return i + 1;
	}

private:

	T* data = nullptr;
	int size = 0;
	int capacity = 0;
};
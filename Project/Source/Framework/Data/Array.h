#pragma once

#include <concepts>
#include <utility>
#include <type_traits>
#include <algorithm>

template<typename Callable, typename T>
concept Predicate =
	requires(const Callable & f, const T & t)
{
	{ f(t) } -> std::convertible_to<bool>;
};

template<typename X, typename Y>
concept DerefComparable =
std::is_pointer_v<X> && std::is_pointer_v<Y> &&
	requires(const std::remove_pointer_t<X>&a, const std::remove_pointer_t<Y>&b)
{
	{ a == b } -> std::convertible_to<bool>;
};

template<typename T, typename U>
concept Comparable =
(std::is_pointer_v<T> == std::is_pointer_v<U>) &&
(
	(!std::is_pointer_v<T> &&
		requires(const T & a, const U & b)
{
	{ a == b } -> std::convertible_to<bool>;
})
||
(std::is_pointer_v<T> &&
	(
		DerefComparable<T, U> ||
		requires(const T & a, const U & b)
{
	{ a == b } -> std::convertible_to<bool>;
}
)
)
);

template<typename T>
using OutParameter = std::conditional_t<
	std::is_pointer_v<T>,
	T&,   // si T és punter → T&
	T*&   // si no → T*&
>;

template<typename T>
using OutParameterList = std::conditional_t<
	std::is_pointer_v<T>,
	T,   // si T és punter → T
	T*   // si no → T*
>;

template<typename Callable, typename T>
concept Comparator =
	requires(const Callable & comp, const T & a, const T & b)
{
	{ comp(a, b) } -> std::convertible_to<bool>;
};

// Array with index-based access, iteration, search utilities, and callable-based operations (Find / Iterate / Sort).
// Default: Array<T> is Array<T, false>, which means that does not own memory if heap allocated and will be only a container for it.
// If T is not a pointer, the data can't be owned. Vector will automatically erase it when destroy.
// Set Array<T, true> to own the data and destroy it automatically. Do not set true for non-heap-allocated pointers.
template<class T, bool Owns = false>
class Array
{
	template<class, bool>
	friend class Array;

	static_assert(!Owns || std::is_pointer_v<T>, "Array<T, true> only allowed when T is a pointer mode");
	static_assert(std::is_destructible_v<T>, "Array<T>: T must be destructible.");

public:

	Array() :
		data(nullptr), size(0)
	{ }

	Array(int amount) :
		data(nullptr), size(amount)
	{
		if (amount > 0)
			data = new T[amount]();
	}

	Array(int amount, const T& initValue) :
		data(nullptr), size(amount)
	{
		static_assert(
			std::is_default_constructible_v<T>,
			"Array(int, const T&): T must be default constructible because the array is first default-constructed."
			);

		static_assert(
			std::is_copy_assignable_v<T>,
			"Array(int, const T&): T must be copy assignable because each element is assigned from initValue."
			);

		if (amount > 0)
		{
			data = new T[amount];
			for (int i = 0; i < size; ++i)
				data[i] = initValue;
		}
	}

	Array(T* initBuffer, int bufferSize) :
		data(nullptr), size(bufferSize)
	{
		static_assert(
			std::is_default_constructible_v<T>,
			"Array(T*, int): T must be default constructible because the internal array is default-constructed."
			);

		static_assert(
			std::is_copy_assignable_v<T> || std::is_move_assignable_v<T>,
			"Array(T*, int): T must be copy or move assignable because elements are copied/moved from the source buffer."
			);

		if (bufferSize <= 0 || initBuffer == nullptr)
			return;

		data = new T[size];
		for (int i = 0; i < size; ++i)
		{
			if constexpr (std::is_copy_assignable_v<T>)
				data[i] = initBuffer[i];
			else
				data[i] = std::move(initBuffer[i]);

		}
	}

	Array(const Array& other) requires (!Owns && std::is_copy_constructible_v<T>)
	{
		static_assert(
			std::is_copy_constructible_v<T>,
			"Array::Array(const Vector&): T must be copy constructible."
			);

		other.CopyTo(*this);
	}

	Array(const Array&) requires (Owns) = delete;

	Array(Array&& other) noexcept :
		data(other.data), size(other.size)
	{
		other.data = nullptr;
		other.size = 0;
	}

	// Creates another instance of the Array pointing to the same values but never owning them
	void CopyTo(Array<T>& out) const requires (std::is_copy_constructible_v<T>)
	{
		static_assert(
			std::is_default_constructible_v<T>,
			"Array::CopyTo(): T must be default constructible."
			);

		static_assert(
			std::is_copy_assignable_v<T>,
			"Array::CopyTo(): T must be copy assignable."
			);

		if (size == 0 || reinterpret_cast<const void*>(&out) == reinterpret_cast<const void*>(this))
			return;
		out.Clear();
		out = Array<T, false>(data, size);
	}

	void CopyTo(Array<T, true>&) const = delete; 

	~Array()
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
	// If "Array<T, true>", it will delete the heap allocated data.
	void Clear()
	{
		for (int i = 0; i < size; ++i)
			DestroyValue(data[i]);

		delete[] data;
		data = nullptr;
		size = 0;
	}

	T& At(int index)
	{
		return data[index];
	}

	const T& At(int index) const
	{
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

		out = data[index];
		return true;
	}

	// Assigns a new value to a specific index
	// If Owns is true, it will attempt to delete the old value if heap allocated
	bool Assign(const T& value, int index)
	{
		static_assert(
			std::is_copy_assignable_v<T>,
			"Array::Assign(const T&): T must be copy assignable."
			);

		if (IsEmpty() || index < 0 || index >= size)
			return false;

		DestroyValue(data[index]);
		data[index] = value;
		return true;
	}

	// Assigns a new value to a specific index
	// If Owns is true, it will attempt to delete the old value if heap allocated
	bool Assign(T&& value, int index)
	{
		static_assert(
			std::is_move_assignable_v<T>,
			"Array::Assign(T&&): T must be move assignable."
			);

		if (IsEmpty() || index < 0 || index >= size)
			return false;

		DestroyValue(data[index]);
		data[index] = std::move(value);
		return true;
	}

	template<typename Callable>
	void Iterate(Callable&& func, bool reverse = false)
	{
		IterateInternal(*this, std::forward<Callable>(func), reverse);
	}

	template<typename Callable>
	void Iterate(Callable&& func, bool reverse = false) const
	{
		IterateInternal(*this, std::forward<Callable>(func), reverse);
	}

	template<typename U>
		requires Comparable<T, U>
	int Find(const U& value) const
	{
		static_assert(Comparable<T, U>, "Array::Find(const U&): T and U must be comparable using operator==."
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
					// If the pointed types are comparable, prefer dereferenced comparison
					if (*data[i] == *value)
						return i;
				}
				else
				{
					// Fallback to pointer identity
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
			"Array::Find(Callable&&): The callable must be invocable with a 'const T&' "
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
		static_assert(Comparable<T, U>, "Array::FindAll(const U&): T and U must be comparable using operator==."
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
				else if (*data[i] == *value)
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
			"Array::FindAll(Callable&&): The callable must be invocable with a 'const T&' "
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

		Array<int> ret = Array<int>(result, count);
		delete[] result;

		return ret;
	}

	template<typename U>
		requires Comparable<T, U>
	bool Contains(const U& value) const
	{
		static_assert(Comparable<T, U>, 
			"Array::Contains(const U&): T and U must be comparable using operator==."
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
			"Array::Contains(Callable&&): The callable must be invocable with a 'const T&' "
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
			"Array::Retrieve(Callable&&, OutParameter<T>): The callable must be invocable with a 'const T&' "
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
			"Array::Retrieve(Callable&&, OutParameter<const T>): The callable must be invocable with a 'const T&' "
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
			"Array::RetrieveAll(Callable&&, Array<OutParameterList<T>>&): The callable must be invocable with a 'const T&' "
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
		static_assert(
			std::is_swappable_v<T>,
			"Array::Swap(): T must be swappable. Check that std::swap(T&, T&) is valid."
			);

		if (a < 0 || b < 0 || a >= size || b >= size)
			return false;

		if (a == b)
			return true;

		std::swap(data[a], data[b]);
		return true;
	}

	template<typename Callable>
		requires Comparator<Callable, T>
	void Sort(Callable&& comp)
	{
		static_assert(
			Comparator<Callable, T>,
			"Array::Sort(comparator): The comparator must be invocable with "
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

public:

	T& operator[](int index)
	{
		return data[index];
	}

	const T& operator[](int index) const
	{
		return data[index];
	}

	Array& operator=(const Array& other) requires (!Owns)
	{
		if (this == &other)
			return *this;

		other.CopyTo(*this);

		return *this;
	}

	Array& operator=(const Array&) requires (Owns) = delete;

	Array& operator=(Array&& other) noexcept
	{
		if (this == &other)
			return *this;

		Clear();

		data = other.data;
		size = other.size;

		other.data = nullptr;
		other.size = 0;

		return *this;
	}

private:

	void DestroyValue(T& value)
	{
		if constexpr (Owns && std::is_pointer_v<T>)
		{
			delete value;
			value = nullptr;
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
						"Array::Iterate(): Invalid callable.\n"
						"Expected one of the following signatures:\n"
						"  void(T&)\n"
						"  bool(T&)\n"
						"  void(T&, int)\n"
						"  bool(T&, int)\n"
						"  void(T&, int, int)\n"
						"  bool(T&, int, int)\n"
						"Const Array uses const T& instead of T&."
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
						"Array::Iterate(): Invalid callable.\n"
						"Expected one of the following signatures:\n"
						"  void(T&)\n"
						"  bool(T&)\n"
						"  void(T&, int)\n"
						"  bool(T&, int)\n"
						"  void(T&, int, int)\n"
						"  bool(T&, int, int)\n"
						"Const Array uses const T& instead of T&."
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
		static_assert(
			std::is_move_constructible_v<T>,
			"Array::Sort(): T must be move constructible."
			);

		static_assert(
			std::is_move_assignable_v<T>,
			"Array::Sort(): T must be move assignable."
			);

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
		static_assert(
			std::is_move_assignable_v<T>,
			"Array::Sort(): T must be move assignable."
			);

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

		if (i + 1 != right)
		{
			data[right] = std::move(data[i + 1]);
			data[i + 1] = std::move(pivot);
		}
		else
		{
			data[right] = std::move(pivot);
		}

		return i + 1;
	}

private:

	T* data = nullptr;
	int size = 0;
};
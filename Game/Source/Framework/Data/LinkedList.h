#pragma once

#include "Framework/Engine/Debug.h"
#include "Framework/Data/Array.h"

// Doubly linked list with index-based access, iteration, search utilities, and callable-based operations (Find / Iterate / Sort).
// Default: LinkedList<T> is LinkedList<T, false>, which means that does not own memory if heap allocated and will be only a container for it.
// If T is not a pointer, the data can't be owned. Vector will automatically erase it when destroy.
// Set LinkedList<T, true> to own the data and destroy it automatically. Do not set true for non-heap-allocated pointers.
template<class T, bool Owns = false>
class LinkedList
{
	template<class, bool>
	friend class LinkedList;

	template<std::equality_comparable, class, bool>
	friend class Map;

	static_assert(!Owns || std::is_pointer_v<T>, "LinkedList<T, true> only allowed when T is a pointer mode");

	struct Node
	{
		Node(const T& value) :
			value(value), next(nullptr), prev(nullptr)
		{
		}

		Node(T&& value) :
			value(std::move(value)), next(nullptr), prev(nullptr)
		{
		}

		T value;
		Node* next = nullptr;
		Node* prev = nullptr;
	};

public: // Functions

	// Creates an empty list
	LinkedList() :
		start(nullptr), final(nullptr), size(0)
	{
	}

	LinkedList(const LinkedList& other) requires (!Owns && std::is_copy_constructible_v<T>)
	{
		static_assert(
			std::is_copy_constructible_v<T>,
			"LinkedList::LinkedList(const LinkedList&): T must be copy constructible."
			);
		other.CopyTo(*this);
	}

	LinkedList(const LinkedList&) requires (Owns) = delete;

	LinkedList(LinkedList&& other) noexcept
	{
		start = other.start;
		final = other.final;
		size = other.size;

		other.start = nullptr;
		other.final = nullptr;
		other.size = 0;
	}

	// Creates another instance of the LinkedList pointing to the same values but never owning them
	void CopyTo(LinkedList<T>& out) const requires (std::is_copy_constructible_v<T>)
	{
		if (reinterpret_cast<const void*>(&out) == reinterpret_cast<const void*>(this))
			return;

		out.Clear();

		Node* current = start;

		while (current)
		{
			out.Add(current->value); // shallow copy
			current = current->next;
		}
	}

	void CopyTo(LinkedList<T, true>&) const = delete;

	// Destroys all nodes and clears memory
	~LinkedList()
	{
		Clear();
	}

	// Adds element at the end of the list
	void Add(const T& value)
	{
		Node* node = new Node(value);
		LinkNode(node, nullptr);
	}

	// Moves element into the list (avoids copy if possible)
	void Add(T&& value)
	{
		Node* node = new Node(std::move(value));
		LinkNode(node, nullptr);
	}

	// Removes all elements from the list + the heap allocated data if this is "LinkedList<T, true>"
	void Clear()
	{
		while (start)
		{
			Node* next = start->next;
			DestroyValue(start->value);
			delete start;
			start = next;
		}
		final = nullptr;
		size = 0;
	}

	// Returns true if list has no elements
	bool IsEmpty() const
	{
		return size == 0;
	}

	// Returns element at index (assert bounds check, TryAt for safety)
	T& At(int index)
	{
		Debug::Assert(!IsEmpty(), "This LinkedList is empty!");
		Debug::Assert(index >= 0 && index < size, "The index is out of bounds!");
		return GetNode(index)->value;
	}

	// Const version of At()
	const T& At(int index) const
	{
		Debug::Assert(!IsEmpty(), "This LinkedList is empty!");
		Debug::Assert(index >= 0 && index < size, "The index is out of bounds!");
		return GetNode(index)->value;
	}

	// Safe access. Returns false if index is invalid.
	// Parameter out is a ptr of type T
	bool TryAt(int index, OutParameter<T> out)
	{
		if (index < 0 || index >= size)
			return false;
		Node* node = GetNode(index);
		out = &node->value;
		return true;
	}

	// Safe access. Returns false if index is invalid.
	// Parameter out is a ptr of type T
	bool TryAt(int index, OutParameter<const T> out) const
	{
		if (index < 0 || index >= size)
			return false;
		Node* node = GetNode(index);
		out = &node->value;
		return true;
	}

	// Removes element at index. Returns false if invalid index.
	bool Erase(int index)
	{
		if (index < 0 || index >= size)
			return false;

		Node* node = GetNode(index);
		DisconnectNode(node);
		DestroyValue(node->value);
		delete node;
		--size;
		return true;
	}

	bool Erase(const Array<int>& indexes)
	{
		if (size == 0)
			return true;

		bool* toDelete = new bool[size]();
		bool allGood = true;

		// marcar
		indexes.Iterate([&](int idx)
			{
				if (idx < 0 || idx >= size)
				{
					allGood = false;
					return;
				}

				toDelete[idx] = true;
			});

		// eliminar sobre la marxa
		int i = 0;
		Node* current = start;

		while (current != nullptr)
		{
			Node* next = current->next;

			if (toDelete[i])
			{
				DisconnectNode(current);
				DestroyValue(current->value);
				delete current;
				--size;
			}

			current = next;
			++i;
		}

		delete[] toDelete;
		return allGood;
	}

	// Inserts element before index. Returns false if index invalid.
	// Example: Insert(x, 0) inserts at front.
	bool Insert(const T& value, int index)
	{
		if (index < 0 || index > size)
			return false;

		Node* target = (index == size) ? nullptr : GetNode(index);
		Node* node = new Node(value);

		LinkNode(node, target);

		return true;
	}

	// Move version of Insert
	bool Insert(T&& value, int index)
	{
		if (index < 0 || index > size)
			return false;

		Node* target = (index == size) ? nullptr : GetNode(index);
		Node* node = new Node(std::move(value));

		LinkNode(node, target);

		return true;
	}

	// Assigns a new value to a specific index
	// If Owns is true, it will attempt to delete the old value if heap allocated
	bool Assign(const T& value, int index)
	{
		if (index < 0 || index >= size)
			return false;

		Node* node = GetNode(index);
		DestroyValue(node->value);
		node->value = value;
		return true;
	}

	// Assigns a new value to a specific index
	// If Owns is true, it will attempt to delete the old value if heap allocated
	bool Assign(T&& value, int index)
	{
		if (index < 0 || index >= size)
			return false;

		Node* node = GetNode(index);

		DestroyValue(node->value);
		node->value = std::move(value);
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

	// Returns number of elements in the list
	int Size() const
	{
		return size;
	}

	// Finds first element matching value.
	// Returns index or -1 if not found.
	template<typename U>
		requires Comparable<T, U>
	int Find(const U& value) const
	{
		static_assert(
			Comparable<T, U>,
			"LinkedList::Find(const U&): T and U must be comparable using operator==.\n"
			"Check that:\n"
			"  - T implements operator==(const U&), or\n"
			"  - U implements operator==(const T&), or\n"
			"  - if T/U are pointers, the pointed types are comparable."
			);

		int index = 0;

		for (Node* current = start; current != nullptr; current = current->next, ++index)
		{
			if constexpr (std::is_pointer_v<T>)
			{
				if (current->value == nullptr || value == nullptr)
				{
					if (current->value == value)
						return index;
				}
				else if (*current->value == *value)
				{
					return index;
				}
			}
			else
			{
				if (current->value == value)
					return index;
			}
		}

		return -1;
	}

	// Finds first element matching predicate.
	// Returns index or -1 if not found.
	template<typename Callable>
		requires Predicate<Callable, T>
	int Find(Callable&& predicate) const
	{
		static_assert(
			Predicate<Callable, T>,
			"LinkedList::Find(predicate): predicate must be callable with "
			"const T& and return bool (or a type convertible to bool).\n"
			"Expected signature:\n"
			"    bool(const T&)\n"
			"or\n"
			"    bool(T)"
			);

		int index = 0;

		for (Node* current = start; current != nullptr; current = current->next, ++index)
		{
			if (predicate(current->value))
				return index;
		}

		return -1;
	}

	// Returns all indices matching value
	template<typename U>
		requires Comparable<T, U>
	Array<int> FindAll(const U& value) const
	{
		static_assert(
			Comparable<T, U>,
			"LinkedList::FindAll(const U&): T and U must be comparable using operator==.\n"
			"Check that:\n"
			"  - T implements operator==(const U&), or\n"
			"  - U implements operator==(const T&), or\n"
			"  - if T/U are pointers, the pointed types are comparable."
			);

		int* result = new int[size];

		int count = 0, index = 0;
		for (Node* current = start; current != nullptr; current = current->next, ++index)
		{
			if constexpr (std::is_pointer_v<T>)
			{
				if (current->value == nullptr || value == nullptr)
				{
					if (current->value == value)
						result[count++] = index;
				}
				else if (*current->value == *value)
				{
					result[count++] = index;
				}
			}
			else
			{
				if (current->value == value)
					result[count++] = index;
			}
		}

		Array ret = Array<int>(result, count);
		delete[] result;

		return ret;
	}

	// Returns indices of all elements matching predicate.
	template<typename Callable>
		requires Predicate<Callable, T>
	Array<int> FindAll(Callable&& predicate) const
	{
		static_assert(
			Predicate<Callable, T>,
			"LinkedList::FindAll(predicate): predicate must accept const T& "
			"and return bool."
			);

		int* result = new int[size];

		int count = 0, index = 0;
		for (Node* current = start; current != nullptr; current = current->next, ++index)
		{
			if (predicate(current->value))
				result[count++] = index;
		}

		Array ret = Array<int>(result, count);
		delete[] result;

		return ret;
	}

	// Returns true if value exists in list
	template<typename U>
		requires Comparable<T, U>
	bool Contains(const U& value) const
	{
		static_assert(
			Comparable<T, U>,
			"LinkedList::Contains(): T and U must be comparable using operator==."
			);
		return Find(value) != -1;
	}

	// Returns true if any element matches predicate.
	template<typename Callable>
		requires Predicate<Callable, T>
	bool Contains(Callable&& predicate) const
	{
		static_assert(
			Predicate<Callable, T>,
			"LinkedList::Contains(predicate): predicate must accept const T& "
			"and return bool."
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
			"LinkedList::Retrieve(predicate): predicate must accept const T& "
			"and return bool."
			);

		for (Node* current = start; current != nullptr; current = current->next)
		{
			if (predicate(current->value))
			{
				if constexpr (std::is_pointer_v<T>)
					out = current->value;
				else
					out = &current->value;
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
			"LinkedList::Retrieve(predicate): predicate must accept const T& "
			"and return bool."
			);

		for (Node* current = start; current != nullptr; current = current->next)
		{
			if (predicate(current->value))
			{
				if constexpr (std::is_pointer_v<T>)
					out = current->value;
				else
					out = &current->value;
				return true;
			}
		}

		return false;
	}

	// Retrieves all elements matching predicate into 'ret'.
	template<typename Callable>
		requires Predicate<Callable, T>
	bool RetrieveAll(Callable&& predicate, Array<OutParameterList<T>>& ret) const
	{
		static_assert(
			Predicate<Callable, T>,
			"LinkedList::RetrieveAll(predicate): predicate must accept const T& "
			"and return bool."
			);

		T* result = new T[size];

		int count = 0;
		bool found = false;
		for (Node* current = start; current != nullptr; current = current->next)
		{
			if (predicate(current->value))
			{
				if constexpr (std::is_pointer_v<T>)
					result[count++] = current->value;
				else
					result[count++] = &current->value;
				found = true;
			}
		}

		ret.Clear();
		ret = Array<OutParameterList<T>>(result, count);
		delete[] result;

		return found;
	}

	// Removes the first element matching value.
	// Returns false if nothing removed.
	template<typename U>
		requires Comparable<T, U>
	bool Remove(const U& value)
	{
		static_assert(
			Comparable<T, U>,
			"LinkedList::Remove(const U&): T and U must be comparable using operator==.\n"
			"Check that:\n"
			"  - T implements operator==(const U&), or\n"
			"  - U implements operator==(const T&), or\n"
			"  - if T/U are pointers, the pointed types are comparable."
			);

		for (Node* current = start; current != nullptr; current = current->next)
		{
			if constexpr (std::is_pointer_v<T>)
			{
				bool remove = false;
				if (current->value == nullptr || value == nullptr)
					remove = (current->value == value);
				else if (*current->value == *value)
					remove = true;

				// Delete T pointer
				if (remove)
				{
					DisconnectNode(current);
					DestroyValue(current->value);
					delete current;
					--size;
					return true;
				}
			}
			else
			{
				// Delete T Object
				if (current->value == value)
				{
					DisconnectNode(current);
					delete current;
					--size;
					return true;
				}
			}
		}

		return false;
	}

	// Removes the first element matching the predicate.
	// Returns false if nothing removed.
	template<typename Callable>
		requires Predicate<Callable, T>
	bool Remove(Callable&& predicate)
	{
		static_assert(
			Predicate<Callable, T>,
			"LinkedList::Retrieve(predicate): predicate must accept const T& "
			"and return bool."
			);

		for (Node* current = start; current != nullptr; current = current->next)
		{
			if (predicate(current->value))
			{
				DisconnectNode(current);
				DestroyValue(current->value);
				delete current;
				--size;
				return true;
			}
		}

		return false;
	}

	// Removes all matching elements to value
	// Returns false if nothing removed.
	template<typename U>
		requires Comparable<T, U>
	bool RemoveAll(const U& value)
	{
		static_assert(
			Comparable<T, U>,
			"LinkedList::RemoveAll(const U&): T and U must be comparable using operator==.\n"
			"Check that:\n"
			"  - T implements operator==(const U&), or\n"
			"  - U implements operator==(const T&), or\n"
			"  - if T/U are pointers, the pointed types are comparable."
			);

		bool ret = false;
		for (Node* current = start; current != nullptr;)
		{
			Node* next = current->next;
			if constexpr (std::is_pointer_v<T>)
			{
				bool remove = false;
				if (current->value == nullptr || value == nullptr)
					remove = (current->value == value);
				else if (*current->value == *value)
					remove = true;

				// Delete T pointer
				if (remove)
				{
					DisconnectNode(current);
					DestroyValue(current->value);
					delete current;
					--size;
					ret = true;
				}
			}
			else
			{
				// Delete T Object
				if (current->value == value)
				{
					DisconnectNode(current);
					delete current;
					--size;
					ret = true;
				}
			}
			current = next;
		}

		return ret;
	}

	// Removes all matching elements the predicate
	// Returns false if nothing removed.
	template<typename Callable>
		requires Predicate<Callable, T>
	bool RemoveAll(Callable&& predicate)
	{
		static_assert(
			Predicate<Callable, T>,
			"LinkedList::RemoveAll(predicate): predicate must accept const T& "
			"and return bool."
			);

		bool ret = false;
		for (Node* current = start; current != nullptr;)
		{
			Node* next = current->next;
			if (predicate(current->value))
			{
				DisconnectNode(current);
				DestroyValue(current->value);
				delete current;
				--size;
			}
			current = next;
		}

		return ret;
	}

	// Sorts list using comparator:
	// comp(a, b) -> true if a should come before b
	template<typename Callable>
		requires Comparator<Callable, T>
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

		// simple adaptive choice (optional, but fine)
		if (size <= 20)
			InsertionSort(std::forward<Callable>(comp));
		else
			QuickSort(start, final, std::forward<Callable>(comp));
	}

	// Swaps elements at indices a and b
	bool Swap(int a, int b)
	{
		if (a < 0 || b < 0 || a >= (int)size || b >= (int)size)
			return false;

		Node* A = GetNode(a);
		Node* B = GetNode(b);

		if (!A || !B)
			return false;

		SwapNodes(A, B);
		return true;
	}

	T& First()
	{
		return start->value;
	}

	const T& First() const
	{
		return start->value;
	}

	T& Last()
	{
		return final->value;
	}

	const T& Last() const
	{
		return final->value;
	}

	Array<T> ToArray() const
	{
		if (IsEmpty())
			return Array<T, false>();

		Array<T, false> ret = Array<T, false>(size);
		int i = 0;
		for (Node* current = start; current != nullptr; current = current->next)
			ret[i++] = current->value;

		return ret;
	}

public: // Operators

	// Direct access (unsafe if index invalid)
	T& operator[](int index)
	{
		return GetNode(index)->value;
	}

	// Const version of operator[]
	const T& operator[](int index) const
	{
		return GetNode(index)->value;
	}

	LinkedList& operator=(const LinkedList& other) requires (!Owns)
	{
		if (this == &other)
			return *this;

		other.CopyTo(*this);

		return *this;
	}

	LinkedList& operator=(const LinkedList&) requires (Owns) = delete;

	LinkedList& operator=(LinkedList&& other) noexcept
	{
		if (this == &other)
			return *this;

		Clear(); // destroy current data

		start = other.start;
		final = other.final;
		size = other.size;

		other.start = nullptr;
		other.final = nullptr;
		other.size = 0;

		return *this;
	}

private:

	void LinkNode(Node* node, Node* target)
	{
		Node* prev = target != nullptr ? target->prev : final;

		node->prev = prev;
		node->next = target;

		if (prev != nullptr)
			prev->next = node;
		else
			start = node;

		if (target != nullptr)
			target->prev = node;
		else
			final = node;

		++size;
	}

	void DestroyValue(T& value)
	{
		if constexpr (Owns && std::is_pointer<T>::value)
		{
			delete value;
			value = nullptr;
		}
	}

	void DisconnectNode(Node* node)
	{
		if (node->prev != nullptr)
			node->prev->next = node->next;
		else
			start = node->next;

		if (node->next != nullptr)
			node->next->prev = node->prev;
		else
			final = node->prev;
	}

	Node* GetNode(int index) const
	{
		if (IsEmpty())
			return nullptr;

		if (index == 0)
			return start;

		if (index == size - 1)
			return final;

		Node* current = nullptr;
		if (index < size / 2)
		{
			current = start;
			for (unsigned int i = 0; i != index; ++i, current = current->next);
		}
		else
		{
			current = final;
			for (unsigned int i = size - 1; i != index; --i, current = current->prev);
		}

		return current;
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

		auto* current = reverse ? self.final : self.start;

		while (current != nullptr)
		{
			ValueType value = current->value;
			current = reverse ? current->prev : current->next;

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
				static_assert(sizeof(Callable) == 0, "Invalid callable for Iterate");
			}

			if (!shouldContinue)
				return;

			reverse ? --index : ++index;
		}
	}

	template<typename Callable>
	void QuickSort(Node* left, Node* right, Callable&& comp)
	{
		if (!left || !right || left == right)
			return;

		Node* pivot = Partition(left, right, std::forward<Callable>(comp));

		if (pivot && left != pivot)
			QuickSort(left, pivot->prev, comp);

		if (pivot && right != pivot)
			QuickSort(pivot->next, right, comp);
	}

	template<typename Callable>
	typename Node* Partition(Node* left, Node* right, Callable&& comp)
	{
		T pivotValue = right->value;

		Node* i = left->prev;

		for (Node* j = left; j != right; j = j->next)
		{
			if (comp(j->value, pivotValue))
			{
				i = (i == nullptr) ? left : i->next;
				SwapNodes(i, j);
			}
		}

		i = (i == nullptr) ? left : i->next;
		SwapNodes(i, right);

		return i;
	}

	template<typename Callable>
	void InsertionSort(Callable&& comp)
	{
		if (!start)
			return;

		for (Node* i = start->next; i != nullptr; i = i->next)
		{
			Node* key = i;
			Node* j = i->prev;

			while (j && comp(key->value, j->value))
			{
				j = j->prev;
			}

			// move node into correct place if needed
			if (key != j->next)
			{
				// detach
				key->prev->next = key->next;
				if (key->next)
					key->next->prev = key->prev;

				// reinsert
				if (!j)
				{
					key->next = start;
					key->prev = nullptr;
					start->prev = key;
					start = key;
				}
				else
				{
					key->next = j->next;
					key->prev = j;
					if (j->next)
						j->next->prev = key;
					j->next = key;
				}
			}
		}

		// fix final
		final = start;
		while (final && final->next)
			final = final->next;
	}

	void SwapNodes(Node* a, Node* b)
	{
		if (a == b)
			return;

		// adjacent case handling is naturally supported

		Node* aPrev = a->prev;
		Node* aNext = a->next;
		Node* bPrev = b->prev;
		Node* bNext = b->next;

		// reconnect a
		if (aPrev) aPrev->next = b;
		if (aNext) aNext->prev = b;

		// reconnect b
		if (bPrev) bPrev->next = a;
		if (bNext) bNext->prev = a;

		std::swap(a->prev, b->prev);
		std::swap(a->next, b->next);

		// fix head/tail
		if (start == a) start = b;
		else if (start == b) start = a;

		if (final == a) final = b;
		else if (final == b) final = a;
	}

private:

	Node* start = nullptr;
	Node* final = nullptr;
	int size = 0;

};

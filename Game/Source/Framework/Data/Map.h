#pragma once

#include "Framework/Data/Vector.h"
#include "Framework/Data/LinkedList.h"
#include "Framework/Engine/Hash.h"

template<std::equality_comparable Key, class Value, bool Owns = false>
class Map
{
	template<class, bool>
	friend class Vector;

	static_assert(!Owns || std::is_pointer_v<Value>, "Map<Key, Value, true> only allowed when Value is a pointer mode");
	static_assert(!std::is_pointer_v<Key>, "Map Key cannot be a pointer mode");
	static_assert(std::is_copy_constructible_v<Key>, "Map Key must be constructible by copy");
	// Value must be either movable or copyable so Map can rehash / move entries around.
	static_assert(std::is_move_constructible_v<Value> || std::is_copy_constructible_v<Value>,
	              "Map Value must be move- or copy-constructible");

	struct Entry
	{
		Entry(const Key& key, const Value& value) :
			key(key), value(value) {}

		Entry(Key&& key, Value&& value) :
			key(std::move(key)), value(std::move(value)) {}

		Entry(Key&& key, const Value& value) :
			key(std::move(key)), value(value) {
		}

		Entry(const Key& key, Value&& value) :
			key(key), value(std::move(value)) {
		}

		Entry(const Entry&) = default;
		Entry(Entry&&) noexcept = default;

		Entry& operator=(const Entry&) = default;
		Entry& operator=(Entry&&) noexcept = default;

		Key key;
		Value value;

		bool operator==(const Entry& entry) const
		{
			return key == entry.key;
		}
	};

	struct ValueProxy
	{
		Map& map;
		Key key;

		ValueProxy(Map& map, const Key& key)
			: map(map), key(key) {
		}

		ValueProxy& operator=(const Value& value)
		{
			map.Assign(key, value);
			return *this;
		}

		ValueProxy& operator=(Value&& value)
		{
			map.Assign(key, std::move(value));
			return *this;
		}

		operator Value& ()
		{
			Entry* e = map.FindOrAdd(key);
			return e->value;
		}

		operator const Value& () const
		{
			const Entry* e = nullptr;

			if (!map.RetrieveEntry(key, e) || e == nullptr)
			{
				Debug::Assert(false, "Key does not exist in const access");
			}

			return e->value;
		}
	};

public: // Functions

	Map() :
		buckets(Vector<LinkedList<Entry>>(8, LinkedList<Entry>())), size(0)
	{ }

	// Only enable copy constructor when Value is copy-constructible
	Map(const Map& other) requires (!Owns && std::is_copy_constructible_v<Value>)
	{
		other.CopyTo(*this);
	}

	Map(const Map&) requires (Owns) = delete;

	Map(Map&& other) noexcept
		: buckets(std::move(other.buckets)),
		size(other.size)
	{
		other.size = 0;
	}

	~Map()
	{
		Clear();
	}

	bool IsEmpty() const
	{
		return size == 0;
	}

	int Size() const
	{
		return size;
	}

	void Clear()
	{
		if constexpr (Owns)
		{
			buckets.Iterate([this](LinkedList<Entry>& bucket)
				{
					bucket.Iterate(
						[this](Entry& entry)
						{
							DestroyValue(entry.value);
						}
					);
					bucket.Clear();
				});
		}
		else
		{
			buckets.Iterate([](LinkedList<Entry>& bucket)
				{
					bucket.Clear();
				});
		}

		buckets.Clear();
		buckets = std::move(Vector(8, LinkedList<Entry>()));
		size = 0;
	}

	void Reset()
	{
		if constexpr (Owns)
		{
			buckets.Iterate([this](LinkedList<Entry>& bucket)
				{
					bucket.Iterate(
						[this](const Entry& entry)
						{
							DestroyValue(entry.value);
						}
					);
					bucket.Clear();
				});
		}
		else
		{
			buckets.Iterate([](LinkedList<Entry>& bucket)
				{
					bucket.Clear();
				});
		}

		size = 0;
	}

	// Creates another instance of the Map pointing to the same values but never owning them
	void CopyTo(Map<Key, Value, false>& out) const
		requires (std::is_copy_constructible_v<Value>)
	{
		out.Clear();
		out.Reserve(size);

		Iterate([&out](const Entry& entry)
			{
				out.Add(entry.key, entry.value);
			});
	}

	void CopyTo(Map<Key, Value, true>&) const = delete;

	Value& Get(const Key& key)
	{
		Entry* e = nullptr;
		Debug::Assert(RetrieveEntry(key, e), "Error with hashing and the Map bucket system!");
		Debug::Assert(e != nullptr, "The inputted key doesn't exist within the map!");
		return e->value;
	}
	const Value& Get(const Key& key) const
	{
		const Entry* e = nullptr;
		Debug::Assert(RetrieveEntry(key, e), "Error with hashing and the Map bucket system!");
		Debug::Assert(e != nullptr, "The inputted key doesn't exist within the map!");
		return e->value;
	}

	bool TryGet(const Key& key, OutParameter<Value> out)
	{
		Entry* e = nullptr;
		if (!RetrieveEntry(key, e) || e == nullptr)
			return false;

		if constexpr (std::is_pointer_v<Value>)
			out = e->value;
		else
			out = &e->value;

		return true;
	}
	bool TryGet(const Key& key, OutParameter<const Value> out) const
	{
		const Entry* e = nullptr;
		if (!RetrieveEntry(key, e) || e == nullptr)
			return false;

		if constexpr (std::is_pointer_v<Value>)
			out = e->value;
		else
			out = &e->value;

		return true;
	}

	bool Add(const Key&  key, const  Value& value)
	{
		return AddEntry(key, value);
	}
	bool Add(const Key&  key,       Value&& value)
	{
		return AddEntry(key, std::move(value));
	}

	bool Assign(const Key& key, const Value& value)
	{
		Entry* e = nullptr;
		if (!RetrieveEntry(key, e))
			return false;

		bool ret = true;
		if (e == nullptr)
			ret = AddEntry(key, value);
		else
		{
			DestroyValue(e->value);
			e->value = value;
		}

		return ret;
	}
	bool Assign(const Key& key, Value&& value)
	{
		Entry* e = nullptr;
		if (!RetrieveEntry(key, e))
			return false;

		bool ret = true;
		if (e == nullptr)
			ret = AddEntry(key, std::move(value));
		else
		{
			DestroyValue(e->value);
			e->value = std::move(value);
		}

		return ret;
	}

	bool Remove(const Key& key)
	{
		LinkedList<Entry>* bucket = GetBucket(key);

		if (bucket == nullptr)
			return false;

		Entry* entry = nullptr;
		bool ret = false;
		if (bucket->Retrieve(
			[&](const Entry& entry)
			{
				return entry.key == key;
			}, 
			entry
		))
		{
			DestroyValue(entry->value);
			bucket->Remove(*entry);
			--size;
			ret = true;
		}

		return ret;
	}

	bool Contains(const Key& key) const
	{
		const Entry* e = nullptr;
		RetrieveEntry(key, e);
		return e != nullptr;
	}

	// Iterates all elements. Safe to Map::Remove(const Key&) elements inside func.
	// Callable can be:
	//  - void(Value&) or bool(Value&) -> return false to stop iteration
	//  - void(Value&, Key&) or bool(Value&, Key&) -> gives also the key of the entry
	// Use const Value& in the function for a const version of Iterate
	template<typename Callable>
	void Iterate(Callable&& func)
	{
		IterateInternal(*this, std::forward<Callable>(func));
	}

	// Iterates all elements. Safe to Map::Remove(const Key&) elements inside func.
	// Callable can be:
	//  - void(Value&) or bool(Value&) -> return false to stop iteration
	//  - void(Value&, Key&) or bool(Value&, Key&) -> gives also the key of the entry
	// Use Value& in the function for a non-const version of Iterate
	template<typename Callable>
	void Iterate(Callable&& func) const
	{
		IterateInternal(*this, std::forward<Callable>(func));
	}

	void Reserve(int capacity)
	{
		Rehash(static_cast<int>(capacity / LoadFactor) + 1);
	}

	Array<Value> ToArray() const
	{
		if (IsEmpty())
			return Array<Value, false>();

		int i = 0;
		Array<Value, false> ret = Array<Value, false>(size);
		buckets.Iterate(
			[&ret, &i](LinkedList<Entry>& bucket)
			{
				bucket.Iterate(
					[&ret, &i](const Entry& entry)
					{
						ret[i++] = entry.value;
					}
				);
			}
		);

		return ret;
	}

	template<typename U>
		requires Comparable<Value, U>
	bool ContainsValue(const U& value) const
	{
		return buckets.Contains(
			[&](const LinkedList<Entry>& bucket)
			{
				return bucket.Contains(
					[&](const Entry& entry)
					{
						if constexpr (std::is_pointer_v<Value>)
						{
							if (entry.value == nullptr || value == nullptr)
							{
								return entry.value == value;
							}
							else if constexpr (DerefComparable<Value, U>)
							{
								return *entry.value == *value;
							}
							else
							{
								return entry.value == value;
							}
						}
						else
						{
							return entry.value == value;
						}
						return false;
					}
				);
			}
		);
	}

	//// Retrieves first element matching predicate into 'ret'. Returns false if not found.
	//// OutPatameter: Input a T pointer (T*) to get the result
	template<typename Callable>
		requires Predicate<Callable, Value>
	bool Retrieve(Callable&& predicate, OutParameter<Value> out)
	{
		for (int i = 0; i < buckets.Size(); ++i)
		{
			auto& bucket = buckets[i];
			for (int j = 0; j < bucket.Size(); ++j)
			{
				auto& entry = bucket[j];
				if (predicate(entry.value))
				{
					if constexpr (std::is_pointer_v<Value>)
						out = entry.value;
					else
						out = &entry.value;
					return true;
				}
			}
		}

		return false;
	}

	//// Retrieves first element matching predicate into 'ret'. Returns false if not found.
	//// OutPatameter: Input a T pointer (T*) to get the result
	template<typename Callable>
		requires Predicate<Callable, Value>
	bool Retrieve(Callable&& predicate, OutParameter<const Value> out) const
	{
		for (int i = 0; i < buckets.Size(); ++i)
		{
			const auto& bucket = buckets[i];
			for (int j = 0; j < bucket.Size(); ++j)
			{
				const auto& entry = bucket[j];
				if (predicate(entry.value))
				{
					if constexpr (std::is_pointer_v<Value>)
						out = entry.value;
					else
						out = &entry.value;
					return true;
				}
			}
		}

		return false;
	}

	template<typename Callable>
		requires Predicate<Callable, Value>
	bool RetrieveAll(Callable&& predicate, Array<OutParameterList<Value>>& ret) const
	{
		Value* result = new Value[size];

		int count = 0;
		bool found = false;
		for (int i = 0; i < buckets.Size(); ++i)
		{
			const auto& bucket = buckets[i];
			for (int j = 0; j < bucket.Size(); ++j)
			{
				const auto& entry = bucket[j];
				if (predicate(entry.value))
				{
					if constexpr (std::is_pointer_v<Value>)
						result[count++] = entry.value;
					else
						result[count++] = &entry.value;
					found = true;
				}
			}
		}

		ret.Clear();
		ret = Array<OutParameterList<Value>>(result, count);
		delete[] result;

		return found;
	}

public: // Operators

	//Map requires a Value type that is default constructible to use the [] operator to asign a value
	ValueProxy operator[](const Key& key)
		requires std::is_default_constructible_v<Value>
	{
		return ValueProxy(*this, key);
	}

	const Value& operator[](const Key& key) const
	{
		const Entry* e = nullptr;
		Debug::Assert(RetrieveEntry(key, e), "Error in map");
		Debug::Assert(e != nullptr, "Key does not exist");
		return e->value;
	}

	Map& operator=(const Map& other) requires (!Owns)
	{
		if (this == &other)
			return *this;

		other.CopyTo(*this);

		return *this;
	}

	Map& operator=(const Map&) requires (Owns) = delete;

	Map& operator=(Map&& other) noexcept
	{
		if (this == &other)
			return *this;

		Clear();

		buckets = std::move(other.buckets);
		size = other.size;

		other.size = 0;

		return *this;
	}

private: // Functions

	LinkedList<Entry>* GetBucket(const Key& key)
	{
		const int index = Hash::From(key).Value() % buckets.Size();

		LinkedList<Entry>* bucket = nullptr;
		if (!buckets.TryAt(index, bucket))
			return nullptr;

		return bucket;
	}
	const LinkedList<Entry>* GetBucket(const Key& key) const
	{
		const int index = Hash::From(key).Value() % buckets.Size();

		const LinkedList<Entry>* bucket = nullptr;
		if (!buckets.TryAt(index, bucket))
			return nullptr;

		return bucket;
	}

	bool RetrieveEntry(const Key& key, Entry*& out)
	{
		LinkedList<Entry>* bucket = GetBucket(key);

		if (bucket == nullptr)
			return false;

		if (!bucket->Retrieve([&key](const Entry& e)
			{ return e.key == key; }, out))
			out = nullptr;

		return true;
	}
	bool RetrieveEntry(const Key& key, const Entry*& out) const
	{
		const LinkedList<Entry>* bucket = GetBucket(key);

		if (bucket == nullptr)
			return false;

		if (!bucket->Retrieve([&key](const Entry& e)
			{ return e.key == key; }, out))
			out = nullptr;

		return true;
	}

	bool AddEntry(const Key& key, const Value&  value)
	{
		Entry* dummy = nullptr;
		return AddEntry(key, value, dummy);
	}
	bool AddEntry(const Key& key,       Value&& value)
	{
		if (static_cast<float>(size + 1) / buckets.Size() > LoadFactor)
			Rehash();

		LinkedList<Entry>* bucket = GetBucket(key);

		if (bucket == nullptr)
			return false;

		if (bucket->Contains([&key](const Entry& e)
			{ return e.key == key; }))
			return false;

		bucket->Add(Entry{ key, std::move(value) });
		++size;
		return true;
	}
	bool AddEntry(const Key& key, const Value&  value, Entry*& out)
	{
		if (static_cast<float>(size + 1) / buckets.Size() > LoadFactor)
			Rehash();

		LinkedList<Entry>* bucket = GetBucket(key);

		if (bucket == nullptr)
			return false;

		if (bucket->Contains([&key](const Entry& e)
			{ return e.key == key; }))
			return false;

		bucket->Add(Entry{ key, value });
		++size;

		out = &bucket->Last();

		return true;
	}

	void DestroyValue(Value& value)
	{
		if constexpr (Owns && std::is_pointer_v<Value>)
		{
			delete value;
			value = nullptr;
		}
	}

	void Rehash(int bucketAmount = 0)
	{
		if (bucketAmount <= 0)
			bucketAmount = buckets.Size() * 2;
		
		if (bucketAmount <= buckets.Size())
			return;

		Vector<LinkedList<Entry>> newBuckets = Vector(bucketAmount, LinkedList<Entry>());
		buckets.Iterate(
			[&newBuckets](LinkedList<Entry>& bucket)
			{
				bucket.Iterate(
					[&newBuckets](Entry& entry)
					{
						const int index = Hash::From(entry.key).Value() % newBuckets.Size();

						LinkedList<Entry>* bucket = nullptr;
						Debug::Assert(newBuckets.TryAt(index, bucket) && bucket != nullptr, "Reserve rehash error");

						if constexpr (std::is_move_constructible_v<Entry>)
							bucket->Add(std::move(entry));
						else
							bucket->Add(entry);
					}
				);
			}
		);

		buckets = std::move(newBuckets);
	}

	Entry* FindOrAdd(const Key& key)
	{
		Entry* e = nullptr;

		if (!RetrieveEntry(key, e))
		{
			Debug::Assert(false, "Error in map structure");
			return nullptr;
		}

		if (e != nullptr)
			return e;

		bool ret = true;

		if constexpr (std::is_pointer_v<Value>)
			ret = AddEntry(key, nullptr, e);
		else
			ret = AddEntry(key, Value{}, e);

		Debug::Assert(ret, "Error inserting in FindOrAdd");

		return e;
	}

	template<typename Self, typename Callable>
	void IterateInternal(Self& self, Callable&& func)
	{
		if (IsEmpty())
			return;

		using ValueType = std::conditional_t<
			std::is_const_v<Self>,
			const Value&,
			Value&
		>;

		buckets.Iterate(
			[&](LinkedList<Entry>& bucket)
			{
				bucket.Iterate(
					[&](Entry& entry)
					{
						bool shouldContinue = true;
						ValueType value = entry.value;
						const Key& key = entry.key;
						if constexpr (requires { func(value, key); })
						{
							if constexpr (std::is_convertible_v<decltype(func(value, key)), bool>)
								shouldContinue = func(value, key);
							else
								func(value, key);
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

						return shouldContinue;
					},
					true
				);
			}
		);
	}

	template<typename Self, typename Callable>
	void IterateInternal(Self& self, Callable&& func) const
	{
		if (IsEmpty())
			return;

		buckets.Iterate(
			[&](const LinkedList<Entry>& bucket)
			{
				bucket.Iterate(
					[&](const Entry& entry)
					{
						bool shouldContinue = true;
						const Value& value = entry.value;
						const Key& key = entry.key;
						if constexpr (requires { func(value, key); })
						{
							if constexpr (std::is_convertible_v<decltype(func(value, key)), bool>)
								shouldContinue = func(value, key);
							else
								func(value, key);
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

						return shouldContinue;
					},
					true
				);
			}
		);
	}

private: // Variables

	Vector<LinkedList<Entry>> buckets;
	int size;
	const float LoadFactor = 0.75f;

};

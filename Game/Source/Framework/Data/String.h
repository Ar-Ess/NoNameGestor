#pragma once

#include <stdio.h>
#include "Framework/Data/Array.h"

class StringView;

class String
{
    friend class StringView;

public: // Constructors

    String();

    String(const char* string);

    String(const String& string);

    String(const StringView& string);

    String(const char& character);

    String(String&& other) noexcept;

    ~String();

    static const char EmptyChar;

    static const String Empty;

    template<typename... Args>
    static String Format(const char* format, Args... args)
    {
        if (IsNullOrEmpty(format))
            return nullptr;

        if constexpr (sizeof...(Args) == 0)
            return String(format);

        int length = snprintf(nullptr, 0, format, args...);
        if (length <= 0) nullptr;

        String result;
        result.Allocate(length);

        snprintf(result.data, length + 1, format, args...);

        return result;
    }

public: // Methods

    // Returns whether string is nullptr
    bool IsNull() const;

    // Returns whether string is nullptr or empty string
    bool IsNullOrEmpty() const;

    // Returns the length of the string
    unsigned int Length() const;
    
    // Checks if this string is the same as "other"
    bool Same(const char* other) const;

    // Checks if this string is the same as "other"
    bool Same(const String& other) const;

    // Checks if this string is the same as "other"
    bool Same(const StringView& other) const;

    // Checks if this string is the same as "other"
    bool Same(const char& other) const;

    // Returns the character at the given index
    const char& At(unsigned int index) const;

    // Finds the index of the first instance of "string"
    // Returns -1 if "string" is not inside
    int Find(const char* string) const;

    // Finds the index of the first instance of "string"
    // Returns -1 if "string" is not inside
    int Find(const String& string) const;

    // Finds the index of the first instance of "string"
    // Returns -1 if "string" is not inside
    int Find(const StringView& string) const;

    // Finds the index of the first instance of "character"
    // Returns -1 if "character" is not inside
    int Find(const char& character) const;

    // Finds all the indices of "string", returns an empty array if not found
    // If "overlapping" is true, it will find all the indices of the string, even if they overlap
    // Example: FindAll("aa", "aaaaa", true) will return [0, 1, 2, 3], while FindAll("aa", "aaaaa", false) will return [0, 2]
    Array<int> FindAll(const char* string, bool overlapping) const;

    // Finds all the indices of "string", returns an empty array if not found
    // If "overlapping" is true, it will find all the indices of the string, even if they overlap
    // Example: FindAll("aa", "aaaaa", true) will return [0, 1, 2, 3], while FindAll("aa", "aaaaa", false) will return [0, 2]
    Array<int> FindAll(const String& string, bool overlapping) const;

    // Finds all the indices of "string", returns an empty array if not found
    // If "overlapping" is true, it will find all the indices of the string, even if they overlap
    // Example: FindAll("aa", "aaaaa", true) will return [0, 1, 2, 3], while FindAll("aa", "aaaaa", false) will return [0, 2]
    Array<int> FindAll(const StringView& string, bool overlapping) const;

    // Finds all the indices of "character", returns an empty array if not found
    Array<int> FindAll(const char& character) const;

    // Finds the index of the last instance of "string"
    // Returns -1 if "string" is not inside
    int FindLast(const char* string) const;

    // Finds the index of the last instance of "string"
    // Returns -1 if "string" is not inside
    int FindLast(const String& string) const;

    // Finds the index of the last instance of "string"
    // Returns -1 if "string" is not inside
    int FindLast(const StringView& string) const;

    // Finds the index of the last instance of "character"
    // Returns -1 if "character" is not inside
    int FindLast(const char& character) const;

    bool Contains(const char* string) const;

    bool Contains(const String& string) const;

    bool Contains(const StringView& string) const;

    bool Contains(const char& character) const;

    bool Remove(const char* string);

    bool Remove(const String& string);
    
    bool Remove(const StringView& string);

    bool Remove(const char& character);

    bool RemoveAll(const char* string);

    bool RemoveAll(const String& string);
    
    bool RemoveAll(const StringView& string);

    bool RemoveAll(const char& character);

    bool RemoveLast(const char* string);

    bool RemoveLast(const String& string);
    
    bool RemoveLast(const StringView& string);

    bool RemoveLast(const char& character);

    const char* Str() const;

    bool StartsWith(const char* string) const;

    bool StartsWith(const String& string) const;

    bool StartsWith(const StringView& string) const;

    bool StartsWith(const char& character) const;

    bool EndsWith(const char* string) const;

    bool EndsWith(const String& string) const;

    bool EndsWith(const StringView& string) const;

    bool EndsWith(const char& character) const;

    String Substring(unsigned int startIndex) const;

    String Substring(unsigned int startIndex, unsigned int length) const;

    bool Insert(const char* string, unsigned int index);

    bool Insert(const String& string, unsigned int index);

    bool Insert(const StringView& string, unsigned int index);

    bool Insert(const char& character, unsigned int index);

    bool Replace(const char* target, const char* replace);

    bool Replace(const String& target, const String& replace);
    
    bool Replace(const StringView& target, const StringView& replace);

    bool Replace(const char& target, const char& replace);

    bool Replace(const String& target, const char* replace);
    
    bool Replace(const StringView& target, const char* replace);

    bool Replace(const String& target, const char& replace);
    
    bool Replace(const StringView& target, const char& replace);

    bool Replace(const char* target, const String& replace);
    
    bool Replace(const char* target, const StringView& replace);

    bool Replace(const char* target, const char& replace);

    bool Replace(const char& target, const char* replace);

    bool Replace(const char& target, const String& replace);
    
    bool Replace(const char& target, const StringView& replace);

    Array<String> Split(const char* separator) const;

    Array<String> Split(const String& separator) const;
    
    Array<String> Split(const StringView& separator) const;

    Array<String> Split(const char& separator) const;

    bool Clear();

    // Handles -1 indexes
    bool Erase(int startIndex);

    // Handles -1 indexes
    bool Erase(int startIndex, int endIndex);

    // Handles -1 indexes
    bool EraseByLength(int startIndex, int length);

public: // Static

    static int LengthOf(const char* string);

    static bool Same(const char* string1, const char* string2);

    static bool Same(const char& character, const char* string);

    static bool Same(const char* string, const char& character);

    static bool IsNullOrEmpty(const char* string);

    // Finds the index of the first instance of "string" inside "in"
    // Returns -1 if "string" is not inside "in"
    static int Find(const char* string, const char* in);

    // Finds the index of the first instance of "string" inside "in"
    // Returns -1 if "string" is not inside "in"
    static int Find(const String& string, const char* in);

    // Finds the index of the first instance of "string" inside "in"
    // Returns -1 if "string" is not inside "in"
    static int Find(const StringView& string, const char* in);

    // Finds the index of the first instance of "character" inside "in"
    // Returns -1 if "character" is not inside "in"
    static int Find(const char& character, const char* in);

    static int FindLast(const char* string, const char* in);

    static int FindLast(const String& string, const char* in);
    
    static int FindLast(const StringView& string, const char* in);

    static int FindLast(const char& character, const char* in);

    // "in" is the string where "string" is searched in
    // If collision is true, it will find all the indices of the string, even if they overlap
    // Example: FindAll("aa", "aaaaa", true) will return [0, 1, 2, 3], while FindAll("aa", "aaaaa", false) will return [0, 2]
    static Array<int> FindAll(const char* string, const char* in, bool collision);

    // "in" is the string where "string" is searched in
    // If collision is true, it will find all the indices of the string, even if they overlap
    // Example: FindAll("aa", "aaaaa", true) will return [0, 1, 2, 3], while FindAll("aa", "aaaaa", false) will return [0, 2]
    static Array<int> FindAll(const String& string, const char* in, bool collision);

    // "in" is the string where "string" is searched in
    // If collision is true, it will find all the indices of the string, even if they overlap
    // Example: FindAll("aa", "aaaaa", true) will return [0, 1, 2, 3], while FindAll("aa", "aaaaa", false) will return [0, 2]
    static Array<int> FindAll(const StringView& string, const char* in, bool collision);

    // "in" is the string where "character" is searched in
    static Array<int> FindAll(const char& character, const char* in);

    static bool Contains(const char* string, const char* in);

    static bool Contains(const String& string, const char* in);
    
    static bool Contains(const StringView& string, const char* in);

    static bool Contains(const char& character, const char* in);

    static bool StartsWith(const char* string, const char* in);

    static bool StartsWith(const String& string, const char* in);
    
    static bool StartsWith(const StringView& string, const char* in);

    static bool StartsWith(const char& character, const char* in);

    static bool EndsWith(const char* string, const char* in);

    static bool EndsWith(const String& string, const char* in);
    
    static bool EndsWith(const StringView& string, const char* in);

    static bool EndsWith(const char& character, const char* in);

    static String Substring(const char* string, unsigned int startIndex);

    static String Substring(const char* string, unsigned int startIndex, unsigned int length);

    static String Join(const Array<String>& array, const char* separator);

    static String Join(const Array<String>& array, const String& separator);
    
    static String Join(const Array<String>& array, const StringView& separator);

    static String Join(const Array<String>& array, const char& separator);

    static String Join(const Array<StringView>& array, const char* separator);

    static String Join(const Array<StringView>& array, const String& separator);
    
    static String Join(const Array<StringView>& array, const StringView& separator);

    static String Join(const Array<StringView>& array, const char& separator);

    static Array<String> Split(const char* input, const char& separator);

    static Array<String> Split(const char* input, const char* separator);

    static Array<String> Split(const char* input, const String& separator);
    
    static Array<String> Split(const char* input, const StringView& separator);

    static String FromUInt(unsigned int value);

    static String FromInt(int value);

    static String FromBool(bool value);

    static String FromFloat(float value, unsigned int decimals = 2);

    static String FromDouble(double value, unsigned int decimals = 2);

public: // Operators

    bool operator==(const String& other) const;
    
    bool operator==(const StringView& other) const;

    bool operator==(const char* other) const;

    bool operator==(const char& other) const;

    bool operator!=(const String& other) const;
    
    bool operator!=(const StringView& other) const;

    bool operator!=(const char* other) const;

    bool operator!=(const char& other) const;

    String& operator=(const String& string);
    
    String& operator=(const StringView& string);

    String& operator=(const char* string);

    String& operator=(const char& character);

    String& operator=(String&& other) noexcept;

    String operator+(const String& string) const;
    
    String operator+(const StringView& string) const;

    String operator+(const char* string) const;

    String operator+(const char& character) const;

    String operator+(int value) const;

    String operator+(float value) const;

    String operator+(double value) const;

    String operator+(bool value) const;

    const String& operator+=(const String& string);
    
    const String& operator+=(const StringView& string);

    const String& operator+=(const char* string);

    const String& operator+=(const char& character);

    const String& operator+=(int value);

    const String& operator+=(float value);

    const String& operator+=(double value);

    const String& operator+=(bool value);

    char& operator[](unsigned int index);

    const char& operator[](unsigned int index) const;

private:

    void Allocate(unsigned int length);

    // SameInternal only works if both strings are the same length and are not null
    static bool SameInternal(const char* string1, const char* string2);

    static int FindInternal(const char* string, const char* in, unsigned int& situation);

    static int* FindAllInternal(const char* string, const char* in, bool collision, unsigned int& situation, unsigned int& stringLength, int& count);

    static int FindLastInternal(const char* string, const char* in, unsigned int& situation);

    bool RemoveInternal(const char* string, char* in);

    bool RemoveAllInternal(const char* string, char* in);

    bool RemoveLastInternal(const char* string, char* in);

    static bool StartsWithInternal(const char* with, const char* string);

    static bool EndsWithInternal(const char* string, const char* in);

    bool InsertInternal(const char* string, unsigned int stringLength, unsigned int index);

    bool ReplaceInternal(const char* target, const char* replace, const unsigned int replaceLength);

private:

    char* data;
    unsigned int length;

};

class StringView
{

    friend class String;

public: // Constructors

    StringView();

    StringView(const char* string);

    StringView(const String& string);

    StringView(const StringView& string);

    StringView(StringView&& other) noexcept;

    ~StringView();

public: // Methods

    // Returns whether string is nullptr
    bool IsNull() const;

    // Returns whether string is nullptr or empty string
    bool IsNullOrEmpty() const;

    // Returns the length of the string
    unsigned int Length() const;

    // Checks if this string is the same as "other"
    bool Same(const char* other) const;

    // Checks if this string is the same as "other"
    bool Same(const String& other) const;

    // Checks if this string is the same as "other"
    bool Same(const StringView& other) const;

    // Checks if this string is the same as "other"
    bool Same(const char& other) const;

    // Returns the character at the given index
    const char& At(unsigned int index) const;

    // Finds the index of the first instance of "string"
    // Returns -1 if "string" is not inside
    int Find(const char* string) const;

    // Finds the index of the first instance of "string"
    // Returns -1 if "string" is not inside
    int Find(const String& string) const;

    // Finds the index of the first instance of "string"
    // Returns -1 if "string" is not inside
    int Find(const StringView& string) const;

    // Finds the index of the first instance of "character"
    // Returns -1 if "character" is not inside
    int Find(const char& character) const;

    // Finds all the indices of "string", returns an empty array if not found
    // If "overlapping" is true, it will find all the indices of the string, even if they overlap
    // Example: FindAll("aa", "aaaaa", true) will return [0, 1, 2, 3], while FindAll("aa", "aaaaa", false) will return [0, 2]
    Array<int> FindAll(const char* string, bool overlapping) const;

    // Finds all the indices of "string", returns an empty array if not found
    // If "overlapping" is true, it will find all the indices of the string, even if they overlap
    // Example: FindAll("aa", "aaaaa", true) will return [0, 1, 2, 3], while FindAll("aa", "aaaaa", false) will return [0, 2]
    Array<int> FindAll(const String& string, bool overlapping) const;

    // Finds all the indices of "string", returns an empty array if not found
    // If "overlapping" is true, it will find all the indices of the string, even if they overlap
    // Example: FindAll("aa", "aaaaa", true) will return [0, 1, 2, 3], while FindAll("aa", "aaaaa", false) will return [0, 2]
    Array<int> FindAll(const StringView& string, bool overlapping) const;

    // Finds all the indices of "character", returns an empty array if not found
    Array<int> FindAll(const char& character) const;

    // Finds the index of the last instance of "string"
    // Returns -1 if "string" is not inside
    int FindLast(const char* string) const;

    // Finds the index of the last instance of "string"
    // Returns -1 if "string" is not inside
    int FindLast(const String& string) const;

    // Finds the index of the last instance of "string"
    // Returns -1 if "string" is not inside
    int FindLast(const StringView& string) const;

    // Finds the index of the last instance of "character"
    // Returns -1 if "character" is not inside
    int FindLast(const char& character) const;

    bool Contains(const char* string) const;

    bool Contains(const String& string) const;

    bool Contains(const StringView& string) const;

    bool Contains(const char& character) const;

    const char* Data() const;

    bool StartsWith(const char* string) const;

    bool StartsWith(const String& string) const;

    bool StartsWith(const StringView& string) const;

    bool StartsWith(const char& character) const;

    bool EndsWith(const char* string) const;

    bool EndsWith(const String& string) const;

    bool EndsWith(const StringView& string) const;

    bool EndsWith(const char& character) const;

    StringView Substring(unsigned int startIndex) const;

    StringView Substring(unsigned int startIndex, unsigned int length) const;

    Array<StringView> Split(const char* separator) const;

    Array<StringView> Split(const String& separator) const;

    Array<StringView> Split(const StringView& separator) const;

    Array<StringView> Split(const char& separator) const;

    bool Clear();

public: // Static

    static StringView Substring(const char* string, unsigned int startIndex);

    static StringView Substring(const char* string, unsigned int startIndex, unsigned int length);

    static Array<StringView> Split(const char* input, const char& separator);

    static Array<StringView> Split(const char* input, const char* separator);

    static Array<StringView> Split(const char* input, const String& separator);
    
    static Array<StringView> Split(const char* input, const StringView& separator);

public: // Operators

    bool operator==(const String& other) const;
    
    bool operator==(const StringView& other) const;

    bool operator==(const char* other) const;

    bool operator==(const char& other) const;

    bool operator!=(const String& other) const;
    
    bool operator!=(const StringView& other) const;

    bool operator!=(const char* other) const;

    bool operator!=(const char& other) const;

    StringView& operator=(const String& string);
    
    StringView& operator=(const StringView& string);

    StringView& operator=(const char* string);

    StringView& operator=(StringView&& other) noexcept;

    String operator+(const String& string) const;
    
    String operator+(const StringView& string) const;

    String operator+(const char* string) const;

    String operator+(const char& character) const;

    String operator+(int value) const;

    String operator+(float value) const;

    String operator+(double value) const;

    String operator+(bool value) const;

    char& operator[](unsigned int index);

    const char& operator[](unsigned int index) const;

private:

    // SameInternal only works if both strings are the same length and are not null
    static bool SameInternal(const char* string1, const char* string2);

    static int FindInternal(const char* string, const char* in, unsigned int& situation);

    static int* FindAllInternal(const char* string, const char* in, bool collision, unsigned int& situation, unsigned int& stringLength, int& count);

    static int FindLastInternal(const char* string, const char* in, unsigned int& situation);

    static bool StartsWithInternal(const char* with, const char* string);

    static bool EndsWithInternal(const char* string, const char* in);

private:

    const char* data;
    unsigned int length;

};

inline String operator+(const char& lhs, const String& rhs)
{
    return String(lhs) + rhs;
}

inline String operator+(const char& lhs, const StringView& rhs)
{
    return String(lhs) + rhs;
}

inline String operator+(const char* lhs, const String& rhs)
{
    return String(lhs) + rhs;
}

inline String operator+(const char* lhs, const StringView& rhs)
{
    return String(lhs) + rhs;
}

inline String operator+(int lhs, const String& rhs)
{
    return String::FromInt(lhs) + rhs;
}

inline String operator+(int lhs, const StringView& rhs)
{
    return String::FromInt(lhs) + rhs;
}

inline String operator+(unsigned int lhs, const String& rhs)
{
    return String::FromUInt(lhs) + rhs;
}

inline String operator+(unsigned int lhs, const StringView& rhs)
{
    return String::FromUInt(lhs) + rhs;
}

inline String operator+(bool lhs, const String& rhs)
{
    return String::FromBool(lhs) + rhs;
}

inline String operator+(bool lhs, const StringView& rhs)
{
    return String::FromBool(lhs) + rhs;
}

inline String operator+(float lhs, const String& rhs)
{
    return String::FromFloat(lhs) + rhs;
}

inline String operator+(float lhs, const StringView& rhs)
{
    return String::FromFloat(lhs) + rhs;
}

inline String operator+(double lhs, const String& rhs)
{
    return String::FromDouble(lhs) + rhs;
}

inline String operator+(double lhs, const StringView& rhs)
{
    return String::FromDouble(lhs) + rhs;
}

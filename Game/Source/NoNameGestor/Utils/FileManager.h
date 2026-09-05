#pragma once

#include "Framework/Data/String.h"
#include "Framework/External/JSON/json.hpp"

#include "NoNameGestor/Utils/DateTime.h"

#include <fstream>

//TODO: Framework: Add this in the framework
class FileManager
{

	struct FileInfo
	{
		DateTime creationDate;
		DateTime lastAccessDate;
		DateTime lastWriteDate;
		uint64_t fileSize;
		bool isReadonly;
		bool isHidden;
		bool isFromSystem;
		bool isArchive;
		bool isCompressed;
		bool isEncrypted;
		bool isTemporary;
	};

public:

	class FileNode
	{
	public:

		FileNode() = default;
		~FileNode() = default;
		FileNode(const FileNode&) = default;
		FileNode& operator=(const FileNode&) = default;
		FileNode(FileNode&& other) noexcept;
		FileNode& operator=(FileNode&& other) noexcept;

		/// <summary>
		/// Safely reads the value stored under the specified field name.
		/// </summary>
		/// <typeparam name="T">Type of the value to retrieve.</typeparam>
		/// <param name="name">Name of the field.</param>
		/// <param name="value">Output parameter receiving the value.</param>
		/// <returns>False if the field does not exist or the value cannot be converted to the requested type; otherwise true.</returns>
		template<typename T>
		bool Read(StringView name, T& value) const
		{
			AssertFile("File Read Error: File not isNewFile.");

			if (!data->contains(name.Data()))
				return false;

			try
			{
				const auto& a = data->at(name.Data());

				if constexpr (std::is_same_v<std::remove_cvref_t<T>, String>)
					value = a.get<std::string>().c_str();
				else
					value = a.get<T>();
			}
			catch (const nlohmann::json::type_error&)
			{
				return false;
			}

			return true;
		}

		/// <summary>
		/// Safely reads the value stored under this node.
		/// </summary>
		/// <typeparam name="T">Type of the value to retrieve.</typeparam>
		/// <param name="value">Output parameter receiving the value.</param>
		/// <returns>False if the value cannot be converted to the requested type; otherwise true.</returns>
		template<typename T>
		bool Read(T& value) const
		{
			AssertFile("File Read Error: File not isNewFile.");

			try
			{
				if constexpr (std::is_same_v<std::remove_cvref_t<T>, String>)
					value = data->get<std::string>().c_str();
				else
					value = data->get<T>();
			}
			catch (const nlohmann::json::type_error&)
			{
				return false;
			}

			return true;
		}

		/// <summary>
		/// Quickly reads the value stored under the specified field name.
		/// Throws if the field does not exist or the value cannot be converted to the requested type.
		/// </summary>
		/// <typeparam name="T">Type of the value to retrieve.</typeparam>
		/// <param name="name">Name of the field.</param>
		/// <returns>The value stored under the specified field name.</returns>
		template<typename T>
		T Read(StringView name) const
		{
			AssertFile("File Read Error: File not isNewFile.");

			if (!data->contains(name.Data()))
				throw std::runtime_error("JSON key not found");

			const auto& a = data->at(name.Data());

			if constexpr (std::is_same_v<std::remove_cvref_t<T>, String>)
				return a.get<std::string>().c_str();
			else
				return a.get<T>();
		}

		/// <summary>
		/// Quickly reads the value stored under this node.
		/// Throws if the value cannot be converted to the requested type; otherwise true.
		/// </summary>
		/// <typeparam name="T">Type of the value to retrieve.</typeparam>
		/// <returns>The value stored under this node.</returns>
		template<typename T>
		T Read() const
		{
			AssertFile("File Read Error: File not isNewFile.");

			if constexpr (std::is_same_v<std::remove_cvref_t<T>, String>)
				return data->get<std::string>().c_str();
			else
				return data->get<T>();
		}

		/// <summary>
		/// Safely reads an element of this node, which must be a JSON array.
		/// </summary>
		/// <typeparam name="T">Type of the value to retrieve.</typeparam>
		/// <param name="index">Zero-based index of the array element.</param>
		/// <param name="value">Output parameter receiving the value.</param>
		/// <returns>False if this node is not an array, the index is out of bounds, or the element cannot be retrieved; otherwise true.</returns>
		template<typename T>
		bool Read(unsigned int index, T& value) const
		{
			AssertFile("File Read Error: File not isNewFile.");

			if (!data->is_array() || data->size() <= index)
				return false;

			try
			{
				const auto& a = data->at(index);

				if constexpr (std::is_same_v<std::remove_cvref_t<T>, String>)
					value = a.get<std::string>().c_str();
				else
					value = a.get<T>();
			}
			catch (const nlohmann::json::type_error&)
			{
				return false;
			}

			return true;
		}

		/// <summary>
		/// Quickly reads  an element of this node, which must be a JSON array.
		/// Throws if this node is not an array or the index is out of bounds.
		/// </summary>
		/// <typeparam name="T">Type of the value to retrieve.</typeparam>
		/// <param name="index">Zero-based index of the array element.</param>
		/// <returns>The value stored under the specified field name.</returns>
		template<typename T>
		T Read(unsigned int index) const
		{
			AssertFile("File Read Error: File not isNewFile.");

			if (!data->is_array())
				throw std::runtime_error("Current FileNode is not an array!");

			if (data->size() <= index)
				throw std::runtime_error("Index out of bounds!");

			const auto& a = data->at(index);

			if constexpr (std::is_same_v<std::remove_cvref_t<T>, String>)
				return a.get<std::string>().c_str();
			else
				return a.get<T>();
		}

		/// <summary>
		/// Writes a value to the specified field.
		/// Overwrites the existing value if the field already exists.
		/// </summary>
		/// <typeparam name="T">Type of the value to write.</typeparam>
		/// <param name="name">Name of the field.</param>
		/// <param name="value">Value to write.</param>
		/// <returns>False if the value cannot be serialized; otherwise true.</returns>
		template<typename T>
		bool Write(const char* name, const T& value)
		{
			AssertFile("File Write Error: File not isNewFile.");

			try
			{
				if constexpr (std::is_same_v<std::remove_cvref_t<T>, String>)
					(*data)[name] = value.Str();
				else if constexpr (std::is_same_v<std::remove_cvref_t<T>, StringView>)
					(*data)[name] = value.Data();
				else
					(*data)[name] = value;
			}
			catch (const nlohmann::json::type_error&)
			{
				return false;
			}

			return true;
		}

		/// <summary>
		/// Writes a value to the specified field.
		/// Overwrites the existing value if the field already exists.
		/// </summary>
		/// <typeparam name="T">Type of the value to write.</typeparam>
		/// <param name="name">Name of the field.</param>
		/// <param name="value">Value to write.</param>
		/// <returns>False if the value cannot be serialized; otherwise true.</returns>
		template<typename T>
		bool Write(StringView name, const T& value)
		{
			AssertFile("File Write Error: File not isNewFile.");

			try
			{
				if constexpr (std::is_same_v<std::remove_cvref_t<T>, String>)
					(*data)[name.Data()] = value.Str();
				else if constexpr (std::is_same_v<std::remove_cvref_t<T>, StringView>)
					(*data)[name.Data()] = value.Data();
				else
					(*data)[name.Data()] = value;
			}
			catch (const nlohmann::json::type_error&)
			{
				return false;
			}

			return true;
		}

		/// <summary>
		/// Writes a value to this node.
		/// Overwrites the existing value if it has one.
		/// </summary>
		/// <typeparam name="T">Type of the value to write.</typeparam>
		/// <param name="value">Value to write.</param>
		/// <returns>False if the value cannot be serialized; otherwise true.</returns>
		template<typename T>
		bool Write(const T& value)
		{
			AssertFile("File Write Error: File not isNewFile.");

			try
			{
				if constexpr (std::is_same_v<std::remove_cvref_t<T>, String>)
					(*data) = value.Str();
				else if constexpr (std::is_same_v<std::remove_cvref_t<T>, StringView>)
					(*data) = value.Data();
				else
					(*data) = value;
			}
			catch (const nlohmann::json::type_error&)
			{
				return false;
			}

			return true;
		}

		/// <summary>
		/// Writes a value to an element of this node, which must be a JSON array.
		/// Overwrites the existing element at the specified index.
		/// </summary>
		/// <typeparam name="T">Type of the value to write.</typeparam>
		/// <param name="index">Zero-based index of the array element.</param>
		/// <param name="value">Value to write.</param>
		/// <returns>False if this node is not an array, the index is out of bounds, or the value cannot be serialized; otherwise true.</returns>
		template<typename T>
		bool Write(unsigned int index, const T& value)
		{
			AssertFile("File Write Error: File not isNewFile.");

			if (!data->is_array() || data->size() <= index)
				return false;

			try
			{
				if constexpr (std::is_same_v<std::remove_cvref_t<T>, String>)
					(*data)[index] = value.Str();
				else if constexpr (std::is_same_v<std::remove_cvref_t<T>, StringView>)
					(*data)[index] = value.Data();
				else
					(*data)[index] = value;
			}
			catch (const nlohmann::json::type_error&)
			{
				return false;
			}

			return true;
		}

		/// <summary>
		/// Appends a value to an array stored under the specified field.
		/// If the field does not exist, tt creates a new JSON array for it.
		/// </summary>
		/// <typeparam name="T">Type of the value to append.</typeparam>
		/// <param name="name">Name of the array field.</param>
		/// <param name="value">Value to append.</param>
		/// <returns>-1 if the field is not a JSON array or the value cannot be serialized; otherwise the new pushed index.</returns>
		template<typename T>
		int Push(const char* name, const T& value)
		{
			AssertFile("File Write Error: File not isNewFile.");

			if (!data->contains(name))
				(*data)[name] = nlohmann::json::array();
			else if (!(*data)[name].is_array())
				return -1;

			auto& a = (*data)[name];
			try
			{
				if constexpr (std::is_same_v<std::remove_cvref_t<T>, String>)
					a.push_back(value.Str());
				else if constexpr (std::is_same_v<std::remove_cvref_t<T>, StringView>)
					a.push_back(value.Data());
				else
					a.push_back(value);
			}
			catch (const nlohmann::json::type_error&)
			{
				return -1;
			}

			return a.size() - 1;
		}

		/// <summary>
		/// Appends a value to an array stored under the specified field.
		/// If the field does not exist, tt creates a new JSON array for it.
		/// </summary>
		/// <typeparam name="T">Type of the value to append.</typeparam>
		/// <param name="name">Name of the array field.</param>
		/// <param name="value">Value to append.</param>
		/// <returns>-1 if the field is not a JSON array or the value cannot be serialized; otherwise the new pushed index.</returns>
		template<typename T>
		int Push(StringView name, const T& value)
		{
			AssertFile("File Write Error: File not isNewFile.");

			if (!data->contains(name.Data()))
				(*data)[name.Data()] = nlohmann::json::array();
			else if (!(*data)[name.Data()].is_array())
				return -1;

			auto& a = (*data)[name.Data()];
			try
			{
				if constexpr (std::is_same_v<std::remove_cvref_t<T>, String>)
					a.push_back(value.Str());
				else if constexpr (std::is_same_v<std::remove_cvref_t<T>, StringView>)
					a.push_back(value.Data());
				else
					a.push_back(value);
			}
			catch (const nlohmann::json::type_error&)
			{
				return -1;
			}

			return a.size() - 1;
		}

		/// <summary>
		/// Pushes back a value to this node, which must be a JSON array.
		/// Creates a new JSON array if the field does not exist.
		/// </summary>
		/// <typeparam name="T">Type of the value to append.</typeparam>
		/// <param name="value">Value to append.</param>
		/// <returns>-1 if the value cannot be serialized; otherwise the new pushed index.</returns>
		template<typename T>
		int Push(const T& value)
		{
			AssertFile("File Write Error: File not isNewFile.");

			if (!data->is_array())
				return -1;

			try
			{
				if constexpr (std::is_same_v<std::remove_cvref_t<T>, String>)
					data->push_back(value.Str());
				else if constexpr (std::is_same_v<std::remove_cvref_t<T>, StringView>)
					data->push_back(value.Data());
				else
					data->push_back(value);
			}
			catch (const nlohmann::json::type_error&)
			{
				return -1;
			}

			return data->size() - 1;
		}

		/// <summary>
		/// Safely accesses a child node by name.
		/// </summary>
		/// <param name="name">Name of the child node.</param>
		/// <param name="node">Output parameter receiving the child node.</param>
		/// <returns>False if the child node does not exist or cannot be accessed; otherwise true.</returns>
		bool Access(const char* name, FileNode& node) const;

		/// <summary>
		/// Quickly accesses a child node by name.
		/// Throws if the child node does not exist or cannot be accessed.
		/// </summary>
		/// <param name="name">Name of the child node.</param>
		/// <returns>The child node stored under the specified name.</returns>
		FileNode Access(StringView name) const;

		/// <summary>
		/// Safely accesses an element of this node, which must be a JSON array.
		/// </summary>
		/// <param name="index">Zero-based index of the array element.</param>
		/// <param name="node">Output parameter receiving the array element.</param>
		/// <returns>False if this node is not an array, the index is out of bounds, or the element cannot be accessed; otherwise true.</returns>
		bool Access(unsigned int index, FileNode& node) const;

		/// <summary>
		/// Quickly accesses an element of this node, which must be a JSON array.
		/// Throws if this node is not an array or the index is out of bounds.
		/// </summary>
		/// <param name="index">Zero-based index of the array element.</param>
		/// <returns>The array element at the specified index.</returns>
		FileNode Access(unsigned int index) const;

		/// <summary>
		/// Safely accesses an element of the node "name", which must be a JSON array.
		/// </summary>
		/// <param name="name">Name of the array in this node.</param>
		/// <param name="index">Zero-based index of the array element.</param>
		/// <param name="node">Output parameter receiving the array element.</param>
		/// <returns>False if "name" node is not an array, the index is out of bounds, or the element cannot be accessed; otherwise true.</returns>
		bool Access(const char* name, unsigned int index, FileNode& node) const;

		/// <summary>
		/// Safely accesses an element of the node "name", which must be a JSON array.
		/// </summary>
		/// <param name="name">Name of the array in this node.</param>
		/// <param name="index">Zero-based index of the array element.</param>
		/// <param name="node">Output parameter receiving the array element.</param>
		/// <returns>False if "name" node is not an array, the index is out of bounds, or the element cannot be accessed; otherwise true.</returns>
		bool Access(StringView name, unsigned int index, FileNode& node) const;

		/// <summary>
		/// Quickly accesses an element of the node "name", which must be a JSON array.
		/// Throws if the node "name" is not an array or the index is out of bounds.
		/// </summary>
		/// <param name="name">Name of the array in this node.</param>
		/// <param name="index">Zero-based index of the array element.</param>
		/// <returns>The array element at the specified index.</returns>
		FileNode Access(const char* name, unsigned int index) const;

		/// <summary>
		/// Quickly accesses an element of the node "name", which must be a JSON array.
		/// Throws if the node "name" is not an array or the index is out of bounds.
		/// </summary>
		/// <param name="name">Name of the array in this node.</param>
		/// <param name="index">Zero-based index of the array element.</param>
		/// <returns>The array element at the specified index.</returns>
		FileNode Access(StringView name, unsigned int index) const;

		/// <summary>
		/// Removes the field with the specified name.
		/// </summary>
		/// <param name="name">Name of the field to remove.</param>
		/// <returns>False if the field does not exist; otherwise true.</returns>
		bool Remove(StringView name);

		/// <summary>
		/// Removes an element from the array stored under the specified field.
		/// </summary>
		/// <param name="name">Name of the array field.</param>
		/// <param name="index">Zero-based index of the array element to remove.</param>
		/// <returns>False if the field does not contain an array, the index is out of bounds, or the element cannot be removed; otherwise true.</returns>
		bool Remove(const char* name, unsigned int index);

		/// <summary>
		/// Removes an element from this node, which it must be a JSON array.
		/// </summary>
		/// <param name="index">Zero-based index of the array element to remove.</param>
		/// <returns>False if this node is not an array, the index is out of bounds, or the element cannot be removed; otherwise true.</returns>
		bool Remove(unsigned int index);

		/// <summary>
		/// Returns the amount of elements in the current json node.
		/// </summary>
		/// <returns></returns>
		int Length() const;

	protected:

		FileNode(nlohmann::json&& data);

		FileNode(nlohmann::json& json);

		void AssertFile(const char* error) const;

	protected:

		nlohmann::json* data = nullptr;

	};

	class File : public FileNode
	{

		friend class FileManager;

	public:

		static const nlohmann::json Array;
		static const nlohmann::json Object;

		File(bool isBackup = false);
		virtual ~File();
		File(const File&) = delete;
		File& operator=(const File&) = delete;
		File(File&& other) noexcept;
		File& operator=(File&& other) noexcept;

		/// <summary>
		/// Returns if the file is valid, by checking if the internal data is created.
		/// </summary>
		bool IsValid() const;

		/// <summary>
		/// Returns if the file is new, which means it has a name but not a path or directory yet.
		/// </summary>
		bool IsNew() const;

		/// <summary>
		/// Returns if the file is a backup, which means it can't be saved to itself, only SavedAs.
		/// </summary>
		virtual bool IsBackup() const;

		/// <summary>
		/// Eliminates the current data without changing the file.
		/// </summary>
		void Clear();

		/// <summary>
		/// Saves the current file to the current stored path.
		/// </summary>
		/// <returns>False if the file is saved correctly; otherwise true.</returns>
		bool Save();
		/// <summary>
		/// Saves the current file to the inputted path.
		/// </summary>
		/// <returns>False if the file is saved correctly; otherwise true.</returns>
		bool SaveAs(const char* path);
		/// <summary>
		/// Saves the current file to the inputted path.
		/// </summary>
		/// <returns>False if the file is saved correctly; otherwise true.</returns>
		bool SaveAs(StringView path);

		/// <summary>
		/// Resets the current file into a new one without path.
		/// </summary>
		/// <param name="name">Optional file name.</param>
		void New(const char* fileName);
		/// <summary>
		/// Resets the current file into a new one without path.
		/// </summary>
		/// <param name="name">Optional file name.</param>
		void New(StringView fileName);

		/// <summary>
		/// Returns the file path (directory + file name + extension).
		/// </summary>
		StringView Path() const;
		/// <summary>
		/// Returns the file name (file name + extension).
		/// </summary>
		StringView Name() const;
		/// <summary>
		/// Returns the file directory (directory).
		/// </summary>
		StringView Directory() const;

		const FileInfo& Info() const;

	private:

		File(nlohmann::json&& data, const String& path, const FileInfo& info);

		void GenerateFileInfo(const String& path);

	protected:

		String path;
		String name;
		String directory;
		bool isNewFile = false; // Is the file a new file? (no path, only file name)
		bool isBackup = false;
		FileInfo info;
	};

public:

	/// <summary>
	/// Transforms a file into a backup file.
	/// </summary>
	/// <param name="file">File to transform.</param>
	static void ToBackup(File& file);

	/// <summary>
	/// Returns if the inputted path has a valid format. 
	/// This format requires at least one '\\' and ending with .'any extension')
	/// </summary>
	/// <param name="path">Path to validate.</param>
	/// <returns>Returns true if the path format is valid.</returns>
	static bool PathFormatValid(const char* path);
	/// <summary>
	/// Returns if the inputted path has a valid format. 
	/// This format requires at least one '\\' and ending with .'any extension')
	/// </summary>
	/// <param name="path">Path to validate.</param>
	/// <returns>Returns true if the path format is valid.</returns>
	static bool PathFormatValid(StringView path);

	/// <summary>
	/// Returns if the inputted path is valid and points to a file.
	/// </summary>
	/// <param name="path">Path to the file.</param>
	/// <returns>Returns if the inputted path is valid and points to a file.</returns>
	static bool FileExists(const char* path);
	/// <summary>
	/// Returns if the inputted path is valid and points to a file.
	/// </summary>
	/// <param name="path">Path to the file.</param>
	/// <returns>Returns if the inputted path is valid and points to a file.</returns>
	static bool FileExists(StringView path);

	/// <summary>
	/// Returns if the inputted directory is valid and points to a folder.
	/// </summary>
	/// <param name="directory">Directory to check.</param>
	/// <returns>Returns if the inputted directory is valid and points to a folder.</returns>
	static bool DirectoryExists(const char* directory);
	/// <summary>
	/// Returns if the inputted directory is valid and points to a folder.
	/// </summary>
	/// <param name="directory">Directory to check.</param>
	/// <returns>Returns if the inputted directory is valid and points to a folder.</returns>
	static bool DirectoryExists(StringView directory);

	/// <summary>
	/// Creates the inputted directory.
	/// </summary>
	/// <param name="directory">Directory to create.</param>
	/// <returns>Returns if the inputted directory has been correctly created.</returns>
	static bool DirectoryCreate(const char* directory);
	/// <summary>
	/// Creates the inputted directory.
	/// </summary>
	/// <param name="directory">Directory to create.</param>
	/// <returns>Returns if the inputted directory has been correctly created.</returns>
	static bool DirectoryCreate(StringView directory);

	/// <summary>
	/// Returns if the inputted path to a file has the expected extension. Does not check for a valid path nor file.
	/// </summary>
	/// <param name="path">Path to the file.</param>
	/// <param name="extension">Extension to check.</param>
	/// <returns>Returns if the inputted path to a file has the expected extension.</returns>
	static bool FileHasExtension(const char* path, const char* extension);
	/// <summary>
	/// Returns if the inputted path to a file has the expected extension. Does not check for a valid path nor file.
	/// </summary>
	/// <param name="path">Path to the file.</param>
	/// <param name="extension">Extension to check.</param>
	/// <returns>Returns if the inputted path to a file has the expected extension.</returns>
	static bool FileHasExtension(StringView path, const char* extension);

	/// <summary>
	/// Returns the directory of the inputted path.
	/// </summary>
	/// <param name="path">The path to convert into directory.</param>
	/// <returns>The directory of the inputted path.</returns>
	static String ToDirectory(StringView path);

	/// <summary>
	/// Returns the file name of the inputted path.
	/// </summary>
	/// <param name="path">The path to convert into file name.</param>
	/// <returns>The file name of the inputted path.</returns>
	static String ToFileName(StringView path);

	/// <summary>
	/// Opens a file for I/O.
	/// </summary>
	/// <param name="path">Path to the file to open.</param>
	/// <param name="create">In case the file does not exist, should the function create a new file?</param>
	/// <returns>The opened file.</returns>
	static File OpenFile(const char* path, bool create = false);
	/// <summary>
	/// Opens a file for I/O.
	/// </summary>
	/// <param name="path">Path to the file to open.</param>
	/// <param name="create">In case the file does not exist, should the function create a new file?</param>
	/// <returns>The opened file.</returns>
	static File OpenFile(StringView path, bool create = false);
	/// <summary>
	/// Opens a file for I/O. (Safe Version)
	/// </summary>
	/// <param name="path">Path to the file to open.</param>
	/// <param name="output">The output file.</param>
	/// <param name="create">In case the file does not exist, should the function create a new file?</param>
	/// <returns>False if there has been an error on file opening; otherwise true.</returns>
	static bool OpenFile(const char* path, File& output, bool create = false);
	/// <summary>
	/// Opens a file for I/O. (Safe Version)
	/// </summary>
	/// <param name="path">Path to the file to open.</param>
	/// <param name="output">The output file.</param>
	/// <param name="create">In case the file does not exist, should the function create a new file?</param>
	/// <returns>False if there has been an error on file opening; otherwise true.</returns>
	static bool OpenFile(StringView path, File& output, bool create = false);

	/// <summary>
	/// Finds a file without loading the internal json data.
	/// </summary>
	/// <param name="path">Path to the file to find.</param>
	/// <param name="create">In case the file does not exist, should the function create a new file?</param>
	/// <returns>The found file.</returns>
	static File FindFile(const char* path, bool create = false);
	/// <summary>
	/// Finds a file without loading the internal json data.
	/// </summary>
	/// <param name="path">Path to the file to find.</param>
	/// <param name="create">In case the file does not exist, should the function create a new file?</param>
	/// <returns>The found file.</returns>
	static File FindFile(StringView path, bool create = false);
	/// <summary>
	/// Finds a file without loading the internal json data.
	/// </summary>
	/// <param name="path">Path to the file to find.</param>
	/// <param name="output">The output file.</param>
	/// <param name="create">In case the file does not exist, should the function create a new file?</param>
	/// <returns>False if there has been an error on file finding; otherwise true.</returns>
	static bool FindFile(const char* path, File& output, bool create = false);
	/// <summary>
	/// Finds a file without loading the internal json data.
	/// </summary>
	/// <param name="path">Path to the file to find.</param>
	/// <param name="output">The output file.</param>
	/// <param name="create">In case the file does not exist, should the function create a new file?</param>
	/// <returns>False if there has been an error on file finding; otherwise true.</returns>
	static bool FindFile(StringView path, File& output, bool create = false);

	static bool RemoveFile(const char* path);

	static bool RemoveFolder(const char* directory, bool removeItself = false);

private:

	static bool GetFileInfo(const char* path, FileInfo& info);

};

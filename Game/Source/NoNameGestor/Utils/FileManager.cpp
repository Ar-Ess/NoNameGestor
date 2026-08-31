#include "FileManager.h"

#include "Framework/Engine/Debug.h"
#include "Framework/Engine/App.h"

#include <iostream>
#include <filesystem>

FileManager::FileNode::FileNode(FileNode&& other) noexcept :
    data(other.data)
{
    other.data = nullptr;
}

FileManager::FileNode& FileManager::FileNode::operator=(FileNode&& other) noexcept
{
    if (this == &other)
        return *this;

    data = other.data;
    other.data = nullptr;

    return *this;
}

bool FileManager::FileNode::Access(const char* name, FileNode& node) const
{
    AssertFile("File Access Error: File not isNewFile.");

    if (!data->contains(name))
        return false;

    try
    {
        node = FileNode(data->at(name));
    }
    catch (const nlohmann::json::type_error&)
    {
        return false;
    }

    return true;
}

FileManager::FileNode FileManager::FileNode::Access(StringView name) const
{
    AssertFile("File Access Error: File not isNewFile.");

    if (!data->contains(name.Data()))
        throw std::runtime_error("JSON key not found");

    return FileNode(data->at(name.Data()));
}

bool FileManager::FileNode::Access(unsigned int index, FileNode& node) const
{
    AssertFile("File Access Error: File not isNewFile.");

    if (!data->is_array() || data->size() <= index)
        return false;

    try
    {
        node = FileNode(data->at(index));
    }
    catch (const nlohmann::json::type_error&)
    {
        return false;
    }

    return true;
}

FileManager::FileNode FileManager::FileNode::Access(unsigned int index) const
{
    AssertFile("File Access Error: File not isNewFile.");

    if (!data->is_array())
        throw std::runtime_error("Current FileNode is not an array!");

    if (data->size() <= index)
        throw std::runtime_error("Index out of bounds!");

    return FileNode(data->at(index));
}

bool FileManager::FileNode::Access(const char* name, unsigned int index, FileNode& node) const
{
    AssertFile("File Access Error: File not isNewFile.");

    if (!data->contains(name))
        return false;

    auto& a = (*data)[name];
    if (!a.is_array() || a.size() <= index)
        return false;

    try
    {
        node = FileNode(a.at(index));
    }
    catch (const nlohmann::json::type_error&)
    {
        return false;
    }

    return true;
}

bool FileManager::FileNode::Access(StringView name, unsigned int index, FileNode& node) const
{
    AssertFile("File Access Error: File not isNewFile.");

    if (!data->contains(name.Data()))
        return false;

    auto& a = (*data)[name.Data()];
    if (!a.is_array() || a.size() <= index)
        return false;

    try
    {
        node = FileNode(a.at(index));
    }
    catch (const nlohmann::json::type_error&)
    {
        return false;
    }

    return true;
}

FileManager::FileNode FileManager::FileNode::Access(const char* name, unsigned int index) const
{
    AssertFile("File Access Error: File not isNewFile.");

    if (!data->contains(name))
        throw std::runtime_error("There is not an element with the inputted name!");

    auto& a = (*data)[name];
    if (!a.is_array())
        throw std::runtime_error("Current FileNode is not an array!");

    if (a.size() <= index)
        throw std::runtime_error("Index out of bounds!");

    return FileNode(a.at(index));
}

FileManager::FileNode FileManager::FileNode::Access(StringView name, unsigned int index) const
{
    AssertFile("File Access Error: File not isNewFile.");

    if (!data->contains(name.Data()))
        throw std::runtime_error("There is not an element with the inputted name!");

    auto& a = (*data)[name.Data()];
    if (!a.is_array())
        throw std::runtime_error("Current FileNode is not an array!");

    if (a.size() <= index)
        throw std::runtime_error("Index out of bounds!");

    return FileNode(a.at(index));
}

bool FileManager::FileNode::Remove(StringView name)
{
    AssertFile("File Remove Error: File not isNewFile.");

    return data->erase(name.Data()) != 0;
}

bool FileManager::FileNode::Remove(const char* name, unsigned int index)
{
    AssertFile("File Remove Error: File not isNewFile.");

    auto& node = (*data)[name];

    if (!node.is_array() || node.size() <= index)
        return false;

    node.erase(index);
    return true;
}

bool FileManager::FileNode::Remove(unsigned int index)
{
    AssertFile("File Remove Error: File not isNewFile.");

    if (!data->is_array() || data->size() <= index)
        return false;

    data->erase(index);
    return true;
}

int FileManager::FileNode::Length() const
{
    AssertFile("File Length Error: File not isNewFile.");
    return data->size();
}

FileManager::FileNode::FileNode(nlohmann::json&& data) :
    data(new nlohmann::json(std::move(data)))
{ }

FileManager::FileNode::FileNode(nlohmann::json& data) :
    data(&data)
{
}

void FileManager::FileNode::AssertFile(const char* error) const
{
    Debug::Assert(data != nullptr, error);
}

const nlohmann::json FileManager::File::Array = nlohmann::json::array();

const nlohmann::json FileManager::File::Object = nlohmann::json::object();

FileManager::File::~File()
{
    delete data;
    data = nullptr;
}

FileManager::File::File(File&& other) noexcept :
    FileNode(std::move(other)),
    path(std::move(other.path)),
    name(std::move(other.name)),
    directory(std::move(other.directory)),
    isNewFile(other.isNewFile)
{
    other.path = nullptr;
    other.name = nullptr;
    other.directory = nullptr;
    other.isNewFile = false;
}

FileManager::File& FileManager::File::operator=(File&& other) noexcept
{
    if (this == &other)
        return *this;

    delete data;

    FileNode::operator=(std::move(other));

    path = std::move(other.path);
    name = std::move(other.name);
    directory = std::move(other.directory);
    isNewFile = other.isNewFile;

    other.path = nullptr;
    other.name = nullptr;
    other.directory = nullptr;
    other.isNewFile = false;

    return *this;
}

bool FileManager::File::IsValid() const
{
    return data != nullptr;
}

bool FileManager::File::IsNew() const
{
    return isNewFile;
}

void FileManager::File::Clear()
{
    AssertFile("File Clear Error: File not isNewFile.");

    *data = nlohmann::json::object();
}

bool FileManager::File::Save()
{
    AssertFile("File Save Error: File not isNewFile.");

    Debug::Assert(!isNewFile, "File Save Error: Can't Save() a new file, it does not have a path. Use SaveAs(path) instead.");

    return SaveAs(path);
}

bool FileManager::File::SaveAs(const char* path)
{
    return SaveAs(StringView(path));
}

bool FileManager::File::SaveAs(StringView path)
{
    AssertFile("File SaveAs Error: File not isNewFile.");

    std::ofstream file(path.Data(), std::ios::out | std::ios::trunc);

    if (!file.is_open())
        return false;

    file << data->dump(App::DebugMode ? 4 : -1);

    if (!file.good())
        return false;

    GenerateFileInfo(path);

    return true;
}

void FileManager::File::New(const char* fileName)
{
    return New(StringView(fileName));
}

void FileManager::File::New(StringView fileName)
{
    if (!IsValid()) data = new nlohmann::json();
    isNewFile = true;
    name = fileName;
    path = nullptr;
    directory = nullptr;
    Clear();
}

StringView FileManager::File::Path() const
{
    return path;
}

StringView FileManager::File::Name() const
{
    return name;
}

StringView FileManager::File::Directory() const
{
    return directory;
}

FileManager::File::File(nlohmann::json&& data, const String& path) :
    FileNode(std::move(data))
{
    GenerateFileInfo(path);
}

void FileManager::File::GenerateFileInfo(const String& p)
{
    path = p;
    int a = path.FindLast('\\') + 1;
    directory = path.Substring(unsigned int(0), a);
    name = path.Substring(a, path.Length());
    isNewFile = false;
}

bool FileManager::PathFormatValid(const char* path)
{
    if (String::IsNullOrEmpty(path))
        return false;

    return PathFormatValid(StringView(path));
}

bool FileManager::PathFormatValid(StringView path)
{
    if (path.IsNullOrEmpty())
        return false;

    int a = path.FindLast('.');
    if (a == -1) return false;

    int b = path.FindLast('\\');
    if (b == -1) return false;

    return a > b;
}

bool FileManager::FileExists(const char* path)
{
    std::error_code ec;
	return std::filesystem::is_regular_file(path, ec);
}

bool FileManager::FileExists(StringView path)
{
    std::error_code ec;
    return std::filesystem::is_regular_file(path.Data(), ec);
}

bool FileManager::DirectoryExists(const char* directory)
{
    std::error_code ec;
    return std::filesystem::is_directory(directory, ec);
}

bool FileManager::DirectoryExists(StringView directory)
{
    std::error_code ec;
    return std::filesystem::is_directory(directory.Data(), ec);
}

bool FileManager::DirectoryCreate(const char* directory)
{
    std::error_code ec;
    return std::filesystem::create_directories(directory, ec);
}

bool FileManager::DirectoryCreate(StringView directory)
{
    std::error_code ec;
    return std::filesystem::create_directories(directory.Data(), ec);
}

bool FileManager::FileHasExtension(const char* path, const char* extension)
{
    if (String::IsNullOrEmpty(path))
        return false;

    return FileHasExtension(StringView(path), extension);
}

bool FileManager::FileHasExtension(StringView path, const char* extension)
{
    if (path.IsNullOrEmpty() || String::IsNullOrEmpty(extension))
        return false;

    std::filesystem::path file_path(path.Data());

    StringView ext = extension;
    if (ext.StartsWith('.'))
        ext = ext.Substring(1);

    return ext == (file_path.extension().string().c_str() + 1);
}

String FileManager::ToDirectory(StringView path)
{
    if (!PathFormatValid(path))
        return String::Empty;

    int a = path.FindLast('\\') + 1;

    return path.Substring(unsigned int(0), a);
}

String FileManager::ToFileName(StringView path)
{
    if (!PathFormatValid(path))
        return String::Empty;

    int a = path.FindLast('\\') + 1;

    return path.Substring(a, path.Length());
}

FileManager::File FileManager::OpenFile(const char* path, bool create)
{
    if (String::IsNullOrEmpty(path))
        return File();

    nlohmann::json data;

    if (create && !FileExists(path))
    {
        std::ofstream file(path);

        if (!file.is_open())
            return File();

        file << "{}";

        if (!file.good())
            return File();

        data = nlohmann::json::object();

    }
    else
    {
        std::ifstream file(path);

        if (!file.good())
            return File();

        data = nlohmann::json::parse(file, nullptr, false);
    }

    if (data.is_discarded())
        return File();

    return File(std::move(data), path);
}

FileManager::File FileManager::OpenFile(StringView path, bool create)
{
    if (path.IsNullOrEmpty())
        return File();

    nlohmann::json data;

    if (create && !FileExists(path))
    {
        std::ofstream file(path.Data());

        if (!file.is_open())
            return File();

        file << "{}";

        if (!file.good())
            return File();

        data = nlohmann::json::object();

    }
    else
    {
        std::ifstream file(path.Data());

        if (!file.good())
            return File();

        data = nlohmann::json::parse(file, nullptr, false);
    }

    if (data.is_discarded())
        return File();

    return File(std::move(data), path);
}

bool FileManager::OpenFile(const char* path, File& output, bool create)
{
    if (String::IsNullOrEmpty(path))
        return false;

    nlohmann::json data;

    if (create && !FileExists(path))
    {
        std::ofstream file(path);

        if (!file.is_open())
            return false;

        file << "{}";

        if (!file.good())
            return false;

        data = nlohmann::json::object();

    }
    else
    {
        std::ifstream file(path);

        if (!file.good())
            return false;

        data = nlohmann::json::parse(file, nullptr, false);
    }

    if (data.is_discarded())
        return false;

    output = File(std::move(data), path);
    return true;
}

bool FileManager::OpenFile(StringView path, File& output, bool create)
{
    if (path.IsNullOrEmpty())
        return false;

    nlohmann::json data;

    if (create && !FileExists(path))
    {
        std::ofstream file(path.Data());

        if (!file.is_open())
            return false;

        file << "{}";

        if (!file.good())
            return false;

        data = nlohmann::json::object();

    }
    else
    {
        std::ifstream file(path.Data());

        if (!file.good())
            return false;

        data = nlohmann::json::parse(file, nullptr, false);
    }

    if (data.is_discarded())
        return false;

    output = File(std::move(data), path);
    return true;
}


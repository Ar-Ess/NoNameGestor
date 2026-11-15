#include "FileManager.h"

#include "Framework/Engine/Debug.h"
#include "Framework/Engine/App.h"

#include <iostream>
#include <filesystem>
#include <windows.h>

FileManager::FileInfo::FileInfo(FileInfo&& other) noexcept :
    expectedRunoutDate(std::move(other.expectedRunoutDate)),
    lastAccessDate(std::move(other.lastAccessDate)),
    lastWriteDate(std::move(other.lastWriteDate)),
    fileSize(other.fileSize),
    isFile(std::move(other.isFile)),
    path(std::move(other.path)),
    name(std::move(other.name)),
    directory(std::move(other.directory)),
    isValid(other.isValid)
{
    other.expectedRunoutDate = DateTime::BaseEpoch;
    other.lastAccessDate = DateTime::BaseEpoch;
    other.lastWriteDate = DateTime::BaseEpoch;
    other.fileSize = 0;
    other.isFile = Flag::AllFalse;
    other.path = String::Empty;
    other.name = String::Empty;
    other.directory = String::Empty;
    other.isValid = false;
}

FileManager::FileInfo& FileManager::FileInfo::operator=(FileInfo&& other) noexcept
{
    if (this == &other)
        return *this;

    expectedRunoutDate   = std::move(other.expectedRunoutDate);
    lastAccessDate = std::move(other.lastAccessDate);
    lastWriteDate  = std::move(other.lastWriteDate);
    fileSize       = other.fileSize;
    isFile         = std::move(other.isFile);
    path           = std::move(other.path);
    name           = std::move(other.name);
    directory      = std::move(other.directory);
    isValid        = other.isValid;

    other.fileSize = 0;
    other.isValid = false;

    return *this;
}

bool FileManager::FileInfo::IsValid() const
{
    return isValid;
}

StringView FileManager::FileInfo::Path() const
{
    return path;
}

StringView FileManager::FileInfo::Name() const
{
    return name;
}

StringView FileManager::FileInfo::Directory() const
{
    return directory;
}

bool FileManager::FileInfo::IsFile(FileFlag flag) const
{
    return isFile[(int)flag];
}

uint64_t FileManager::FileInfo::Size() const
{
    return fileSize;
}

DateTime FileManager::FileInfo::CreationDate() const
{
    return expectedRunoutDate;
}

DateTime FileManager::FileInfo::LastAccessDate() const
{
    return lastAccessDate;
}

DateTime FileManager::FileInfo::LastWriteDate() const
{
    return lastWriteDate;
}

bool FileManager::FileInfo::GenerateFileInfo(StringView p)
{
    WIN32_FILE_ATTRIBUTE_DATA fileData;

    if (!GetFileAttributesExA(p.Data(), GetFileExInfoStandard, &fileData))
        return false;

    expectedRunoutDate = DateTime::From::WindowsFileTime(fileData.ftCreationTime);
    lastAccessDate = DateTime::From::WindowsFileTime(fileData.ftLastAccessTime);
    lastWriteDate = DateTime::From::WindowsFileTime(fileData.ftLastWriteTime);
    fileSize = ((uint64_t)fileData.nFileSizeHigh << 32) | fileData.nFileSizeLow;
    isFile = Flag(
        (bool)(fileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY),
        {
            (bool)(fileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY),
            (bool)(fileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN),
            (bool)(fileData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM),
            (bool)(fileData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE),
            (bool)(fileData.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED),
            (bool)(fileData.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED),
            (bool)(fileData.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY)
        }
    );

    path = p;
    int a = p.FindLast('\\') + 1;
    directory = p.Substring(unsigned int(0), a);
    name = p.Substring(a, p.Length());

    isValid = true;

    return true;
}

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
    AssertFile("File Access Error: File not valid.");

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
    AssertFile("File Access Error: File not valid.");

    if (!data->contains(name.Data()))
        throw std::runtime_error("JSON key not found");

    return FileNode(data->at(name.Data()));
}

bool FileManager::FileNode::Access(unsigned int index, FileNode& node) const
{
    AssertFile("File Access Error: File not valid.");

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
    AssertFile("File Access Error: File not valid.");

    if (!data->is_array())
        throw std::runtime_error("Current FileNode is not an array!");

    if (data->size() <= index)
        throw std::runtime_error("Index out of bounds!");

    return FileNode(data->at(index));
}

bool FileManager::FileNode::Access(const char* name, unsigned int index, FileNode& node) const
{
    AssertFile("File Access Error: File not valid.");

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
    AssertFile("File Access Error: File not valid.");

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
    AssertFile("File Access Error: File not valid.");

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
    AssertFile("File Access Error: File not valid.");

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
    AssertFile("File Remove Error: File not valid.");

    return data->erase(name.Data()) != 0;
}

bool FileManager::FileNode::Remove(const char* name, unsigned int index)
{
    AssertFile("File Remove Error: File not valid.");

    auto& node = (*data)[name];

    if (!node.is_array() || node.size() <= index)
        return false;

    node.erase(index);
    return true;
}

bool FileManager::FileNode::Remove(unsigned int index)
{
    AssertFile("File Remove Error: File not valid.");

    if (!data->is_array() || data->size() <= index)
        return false;

    data->erase(index);
    return true;
}

int FileManager::FileNode::Length() const
{
    AssertFile("File Length Error: File not valid.");
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

FileManager::File::File(bool isBackup) :
    FileNode(), FileInfo(), isBackup(isBackup)
{
}

FileManager::File::~File()
{
    delete data;
    data = nullptr;
}

FileManager::File::File(File&& other) noexcept :
    FileNode(std::move(other)),
    FileInfo(std::move(other)),
    isNewFile(other.isNewFile),
    isBackup(other.isBackup)
{
    other.isNewFile = false;
    other.isBackup = false;
}

FileManager::File& FileManager::File::operator=(File&& other) noexcept
{
    if (this == &other)
        return *this;

    delete data;

    FileInfo::operator=(std::move(other));
    FileNode::operator=(std::move(other));

    isNewFile = other.isNewFile;
    isBackup = other.isBackup;

    other.isNewFile = false;
    other.isBackup = false;

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

bool FileManager::File::IsBackup() const
{
    return isBackup;
}

void FileManager::File::Clear()
{
    AssertFile("File Clear Error: File not valid.");

    *data = nlohmann::json::object();
}

bool FileManager::File::Save()
{
    AssertFile("File Save Error: File not valid.");

    Debug::Assert(!isNewFile, "File Save Error: Can't Save() a new file, it does not have a path. Use SaveAs(path) instead.");
    Debug::Assert(!isBackup, "File Save Error: Can't Save() a backup. Use SaveAs(path) instead.");

    return SaveAs(path);
}

bool FileManager::File::SaveAs(const char* path)
{
    return SaveAs(StringView(path));
}

bool FileManager::File::SaveAs(StringView path)
{
    AssertFile("File SaveAs Error: File not valid.");

    std::ofstream file(path.Data(), std::ios::out | std::ios::trunc);

    if (!file.is_open())
        return false;

    file << data->dump(App::DebugMode ? 4 : -1);

    if (!file.good())
        return false;

    if (!GenerateFileInfo(path))
        return false;

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

FileManager::File::File(nlohmann::json&& data, const FileInfo& info) :
    FileNode(std::move(data)), FileInfo(info)
{
    isNewFile = false;
}

void FileManager::ToBackup(File& file)
{
    file.AssertFile("FileManager ToBackup Error: File not valid.");

    file.isBackup = true;
    file.isNewFile = true;
    file.path = nullptr;
    file.directory = nullptr;
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

    FileInfo info;
    if (!info.GenerateFileInfo(path))
        return File();

    return File(std::move(data), info);
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

    FileInfo info;
    if (!info.GenerateFileInfo(path))
        return File();

    return File(std::move(data), info);
}

bool FileManager::OpenFile(const char* path, File& output, bool create)
{
    output = File();

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

    FileInfo info;
    if (!info.GenerateFileInfo(path))
        return false;

    output = File(std::move(data), info);
    return true;
}

bool FileManager::OpenFile(StringView path, File& output, bool create)
{
    output = File();

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

    FileInfo info;
    if (!info.GenerateFileInfo(path))
        return false;

    output = File(std::move(data), info);
    return true;
}

FileManager::File FileManager::OpenFile(const FileInfo& info)
{
    if (!info.isValid)
        return File();

    nlohmann::json data;

    std::ifstream file(info.path.Str());

    if (!file.good())
        return File();

    data = nlohmann::json::parse(file, nullptr, false);

    if (data.is_discarded())
        return File();

    return File(std::move(data), info);
}

bool FileManager::OpenFile(const FileInfo& info, FileManager::File& output)
{
    output = File();

    if (!info.isValid)
        return false;

    nlohmann::json data;

    std::ifstream file(info.path.Str());

    if (!file.good())
        return false;

    data = nlohmann::json::parse(file, nullptr, false);

    if (data.is_discarded())
        return false;

    output = File(std::move(data), info);
    return true;
}

FileManager::FileInfo FileManager::FindFile(const char* path, bool create)
{
    if (String::IsNullOrEmpty(path))
        return FileInfo();

    if (create && !FileExists(path))
    {
        std::ofstream file(path);

        if (!file.is_open())
            return FileInfo();

        file << "{}";

        if (!file.good())
            return FileInfo();
    }
    else
    {
        std::ifstream file(path);

        if (!file.good())
            return FileInfo();
    }

    FileInfo info;
    if (!info.GenerateFileInfo(path))
        return FileInfo();

    return info;
}

FileManager::FileInfo FileManager::FindFile(StringView path, bool create)
{
    if (path.IsNullOrEmpty())
        return FileInfo();

    if (create && !FileExists(path))
    {
        std::ofstream file(path.Data());

        if (!file.is_open())
            return FileInfo();

        file << "{}";

        if (!file.good())
            return FileInfo();
    }
    else
    {
        std::ifstream file(path.Data());

        if (!file.good())
            return FileInfo();
    }

    FileInfo info;
    if (!info.GenerateFileInfo(path))
        return FileInfo();

    return info;
}

bool FileManager::FindFile(const char* path, FileInfo& output, bool create)
{
    output = FileInfo();

    if (String::IsNullOrEmpty(path))
        return false;

    if (create && !FileExists(path))
    {
        std::ofstream file(path);

        if (!file.is_open())
            return false;

        file << "{}";

        if (!file.good())
            return false;
    }
    else
    {
        std::ifstream file(path);

        if (!file.good())
            return false;
    }

    if (!output.GenerateFileInfo(path))
        return false;

    return true;
}

bool FileManager::FindFile(StringView path, FileInfo& output, bool create)
{
    output = FileInfo();

    if (path.IsNullOrEmpty())
        return false;

    if (create && !FileExists(path))
    {
        std::ofstream file(path.Data());

        if (!file.is_open())
            return false;

        file << "{}";

        if (!file.good())
            return false;
    }
    else
    {
        std::ifstream file(path.Data());

        if (!file.good())
            return false;
    }

    if (!output.GenerateFileInfo(path))
        return false;

    return true;
}

Array<FileManager::FileInfo> FileManager::FindFiles(const char* directory)
{
    if (String::IsNullOrEmpty(directory) || !DirectoryExists(directory))
        return Array<FileInfo>();

    std::vector<std::string> infos = std::vector<std::string>();

    std::filesystem::path path(directory);
    for (auto& entry : std::filesystem::directory_iterator(path))
    {
        if (entry.is_directory())
            continue;

        infos.push_back(path.string());
    }

    if (infos.empty())
        return Array<FileInfo>();

    auto ret = Array<FileInfo>(infos.size());

    for (int i = 0; i < ret.Size(); ++i)
    {
        FileInfo info = FileInfo();
        if (info.GenerateFileInfo(infos[i].c_str()))
            ret[i] = std::move(info);
    }

    return ret;
}

Array<FileManager::FileInfo> FileManager::FindFiles(StringView directory)
{
    if (directory.IsNullOrEmpty() || !DirectoryExists(directory))
        return Array<FileInfo>();

    std::vector<std::string> infos = std::vector<std::string>();

    std::filesystem::path path(directory.Data());
    for (auto& entry : std::filesystem::directory_iterator(path))
    {
        if (entry.is_directory())
            continue;

        infos.push_back(entry.path().string());
    }

    if (infos.empty())
        return Array<FileInfo>();

    auto ret = Array<FileInfo>(infos.size());

    for (int i = 0; i < ret.Size(); ++i)
    {
        FileInfo info = FileInfo();
        if (info.GenerateFileInfo(infos[i].c_str()))
            ret[i] = std::move(info);
    }

    return ret;
}

bool FileManager::FindFiles(const char* directory, Array<FileInfo>& output)
{
    output = Array<FileInfo>();

    if (String::IsNullOrEmpty(directory) || !DirectoryExists(directory))
        return false;

    std::vector<std::string> infos = std::vector<std::string>();

    std::filesystem::path path(directory);
    for (auto& entry : std::filesystem::directory_iterator(path))
    {
        if (entry.is_directory())
            continue;

        infos.push_back(path.string());
    }

    if (infos.empty())
        return false;

    output = Array<FileInfo>(infos.size());

    for (int i = 0; i < output.Size(); ++i)
    {
        FileInfo info = FileInfo();
        if (info.GenerateFileInfo(infos[i].c_str()))
            output[i] = std::move(info);
    }

    return true;
}

bool FileManager::FindFiles(StringView directory, Array<FileInfo>& output)
{
    output = Array<FileInfo>();

    if (directory.IsNullOrEmpty() || !DirectoryExists(directory))
        return false;

    std::vector<std::string> infos = std::vector<std::string>();

    std::filesystem::path path(directory.Data());
    for (auto& entry : std::filesystem::directory_iterator(path))
    {
        if (entry.is_directory())
            continue;

        infos.push_back(path.string());
    }

    if (infos.empty())
        return false;

    output = Array<FileInfo>(infos.size());

    for (int i = 0; i < output.Size(); ++i)
    {
        FileInfo info = FileInfo();
        if (info.GenerateFileInfo(infos[i].c_str()))
            output[i] = std::move(info);
    }

    return true;
}

bool FileManager::RemoveFile(const char* path)
{
    if (String::IsNullOrEmpty(path) || !FileExists(path))
        return false;

    return std::filesystem::remove(path);
}

bool FileManager::RemoveFile(FileInfo& file)
{
    if (!file.isValid)
        return false;

    bool ret = std::filesystem::remove(file.path.Str());
    if (ret) FileInfo temp = std::move(file);

    return ret;
}

int FileManager::RemoveFiles(Array<FileInfo>& files)
{
    if (files.IsEmpty())
        return 0;

    int count = 0;
    files.Iterate(
        [&](FileInfo& file)
        {
            if (RemoveFile(file))
                ++count;
            return true;
        }
    );

    return count;
}

bool FileManager::RemoveFolder(const char* directory, bool removeItself)
{
    if (String::IsNullOrEmpty(directory) || !DirectoryExists(directory))
        return false;

    int ret = 0;
    if (removeItself)
        ret = std::filesystem::remove_all(directory);
    else
    {
        std::filesystem::path path(directory);
        for (auto& entry : std::filesystem::directory_iterator(path))
            std::filesystem::remove_all(entry.path());
    }

    return ret != 0;
}


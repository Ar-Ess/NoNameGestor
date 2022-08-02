#ifndef __FILE_MANAGER_H__
#define __FILE_MANAGER_H__

#include <fstream>
#include <assert.h>

class FileManager
{
private:

	enum class Access
	{
		OPEN,
		EDIT,
		VIEW
	};

	struct Editor
	{
	private:

		struct Variables
		{
			Variables(const char* name, const char* variable, Access access)
			{
				this->name = name;
				this->variable = variable;
				this->access = access;
			}

			Editor Number(double number)
			{
				std::fstream file;

				file.open(name, std::ios::app | std::ios::ate);

				assert(file.is_open()); // File is not open

				file << variable << " " << number << "," << std::endl;

				file.close();

				return Editor(name, access);
			}

			Editor Number(float number)
			{
				std::fstream file;

				file.open(name, std::ios::app | std::ios::ate);

				assert(file.is_open()); // File is not open

				file << variable << " " << number << "," << std::endl;

				file.close();

				return Editor(name, access);
			}

			Editor Number(int number)
			{
				std::fstream file;

				file.open(name, std::ios::app | std::ios::ate);

				assert(file.is_open()); // File is not open

				file << variable << " " << number << "," << std::endl;

				file.close();

				return Editor(name, access);
			}

			Editor String(std::string string)
			{
				std::fstream file;

				file.open(name, std::ios::app | std::ios::ate);

				assert(file.is_open()); // File is not open

				file << variable << " " << string.c_str() << "," << std::endl;

				file.close();

				return Editor(name, access);
			}

		private:

			const char* name = nullptr;
			const char* variable = nullptr;
			Access access = Access::OPEN;
		};

		struct Returns
		{
			Returns(const char* name, const char* variable, Access access, Editor* editor)
			{
				this->name = name;
				this->variable = variable;
				this->access = access;
				this->editor = editor;
			}

			Editor AsInt(int& number)
			{
				std::fstream file;

				file.open(name, std::ios::in);

				assert(file.is_open()); // File is not open

				if (editor->jumpLines > 0) editor->InternalJumpLines(&file);

				bool variableExists = false;
				while (!file.eof())
				{
					std::string line;
					std::getline(file, line, ' ');
					if (editor->InternalSameString(variable, line))
					{
						variableExists = true;
						line.clear();
						line.shrink_to_fit();
						break;
					}
					line.clear();
					line.shrink_to_fit();

					std::getline(file, line, '\n');
					line.clear();
					line.shrink_to_fit();
				}

				// The variable inputted does not exist in the document
				assert(variableExists);

				double value = 0.0;
				file >> value;

				number = int(ceil(value));

				file.close();

				return *editor;
			}

			Editor AsFloat(float& number)
			{
				std::fstream file;

				file.open(name, std::ios::in);

				assert(file.is_open()); // File is not open

				if (editor->jumpLines > 0) editor->InternalJumpLines(&file);

				bool variableExists = false;
				while (!file.eof())
				{
					std::string line;
					std::getline(file, line, ' ');
					if (editor->InternalSameString(variable, line))
					{
						variableExists = true;
						line.clear();
						line.shrink_to_fit();
						break;
					}
					line.clear();
					line.shrink_to_fit();

					std::getline(file, line, '\n');
					line.clear();
					line.shrink_to_fit();
				}

				// The variable inputted does not exist in the document
				assert(variableExists);

				double value = 0.0;
				file >> value;

				number = float(value);

				file.close();

				return *editor;
			}

			Editor AsDouble(double& number)
			{
				std::fstream file;

				file.open(name, std::ios::in);

				assert(file.is_open()); // File is not open

				if (editor->jumpLines > 0) editor->InternalJumpLines(&file);

				bool variableExists = false;
				while (!file.eof())
				{
					std::string line;
					std::getline(file, line, ' ');
					if (editor->InternalSameString(variable, line))
					{
						variableExists = true;
						line.clear();
						line.shrink_to_fit();
						break;
					}
					line.clear();
					line.shrink_to_fit();

					std::getline(file, line, '\n');
					line.clear();
					line.shrink_to_fit();
				}

				// The variable inputted does not exist in the document
				assert(variableExists);

				double value = 0.0;
				file >> value;

				number = value;

				file.close();

				return *editor;
			}

			Editor AsString(std::string& string)
			{
				std::fstream file;

				file.open(name, std::ios::in);

				assert(file.is_open()); // File is not open

				if (editor->jumpLines > 0) editor->InternalJumpLines(&file);

				bool variableExists = false;
				while (!file.eof())
				{
					std::string line;
					std::getline(file, line, ' ');
					if (editor->InternalSameString(variable, line))
					{
						variableExists = true;
						line.clear();
						line.shrink_to_fit();
						break;
					}
					line.clear();
					line.shrink_to_fit();

					std::getline(file, line, '\n');
					line.clear();
					line.shrink_to_fit();
				}

				// The variable inputted does not exist in the document
				assert(variableExists);

				std::getline(file, string, ',');

				file.close();

				return *editor;
			}

		private:

			const char* name = nullptr;
			const char* variable = nullptr;
			Editor* editor = nullptr;
			Access access = Access::OPEN;
		};

	public:

		Editor(const char* name, Access access, int jumpLines = 0) 
		{
			this->name = name;
			this->access = access;
			this->jumpLines = jumpLines;
		}

		~Editor() {}

		Variables Write(const char* variable)
		{
			// You are trying to write a file in view mode. Access with Open or Edit Mode.
			assert(access != Access::VIEW);

			return Variables(name.c_str(), variable, access);
		}

		Returns Read(const char* variable)
		{
			// You are trying to read an open file. Opened files are empty. Access with Edit or View Mode
			assert(access != Access::OPEN);

			return Returns(name.c_str(), variable, access, this);
		}

	private:

		friend class Returns;
		Editor() {}

		bool InternalSameString(std::string a, std::string b)
		{
			bool ret = true;

			if (a.size() != b.size()) return false;

			for (unsigned int i = 0; i < a.size(); i++)
			{
				ret = (a[i] == b[i]);
				if (!ret) return false;
			}

			return true;
		}

		void InternalJumpLines(std::fstream* file)
		{
			for (unsigned int i = 0; i < jumpLines; ++i)
			{
				std::string line;
				std::getline(*file, line, '\n');
			}
		}

		std::string name;
		Access access = Access::OPEN;
		int jumpLines = 0;
	};

public:

	FileManager(const char* extension = nullptr) 
	{
		SetExtension(extension);
	}

	~FileManager() {}

	Editor OpenFile(const char* name)
	{
		name = InternalExtensionCheck(name);

		if (InternalExists(name))
		{
			std::ofstream erase;
			erase.open(name, std::ofstream::out | std::ofstream::trunc);
			erase.close();
		}

		return Editor(name, Access::OPEN);
	}

	Editor EditFile(const char* name)
	{
		name = InternalExtensionCheck(name);

		assert(InternalExists(name));

		return Editor(name, Access::EDIT);
	}

	Editor ViewFile(const char* name, int jumpLines = 0)
	{
		name = InternalExtensionCheck(name);

		assert(InternalExists(name));

		return Editor(name, Access::VIEW, jumpLines);
	}

	bool Exists(const char* name) const
	{
		std::string fileName = name;
		fileName += extension;
		name = fileName.c_str();
		std::fstream file;

		// In mode (read)
		file.open(name, std::ios::in);

		bool ret = file.is_open();
		file.close();

		return ret;
	}

	void SetExtension(const char* extension)
	{
		if (extension)
		{
			this->extension.clear();
			this->extension.shrink_to_fit();
			this->extension = extension;
		}
	}

private:

	bool InternalExists(const char* name) const
	{
		std::fstream file;

		// In mode (read)
		file.open(name, std::ios::in);

		bool ret = file.is_open();
		file.close();

		return ret;
	}

	const char* InternalExtensionCheck(const char* name)
	{
		if (extension.empty()) return name;

		std::string check = name;
		check.erase(check.begin(), check.end() - extension.size());
		if (SameString(extension.c_str(), check.c_str())) return name;

		std::string fileName = name;
		fileName += extension;
		return fileName.c_str();
	}

	std::string extension;
};

#endif // !__FILE_MANAGER_H__


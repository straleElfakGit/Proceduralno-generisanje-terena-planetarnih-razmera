#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>
#include <fstream>
#include <sstream>
#include <cerrno>
#include <unordered_set>

#include "Logging/Logger.h"

class FileManager
{
public:
	static std::string get_file_contents_processed(const std::string& filepath);

private:
	static std::string GetDirectoryPath(const std::string& filepath);
	static std::string ParseIncludesInternal(const std::string& filepath, std::unordered_set<std::string>& includedFiles);
};

#endif // !FILE_MANAGER_H

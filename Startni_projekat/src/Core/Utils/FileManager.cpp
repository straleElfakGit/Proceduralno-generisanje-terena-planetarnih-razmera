#include "FileManager.h"

std::string FileManager::GetDirectoryPath(const std::string& filepath)
{
    size_t pos = filepath.find_last_of("/\\");
    if (pos != std::string::npos)
        return filepath.substr(0, pos + 1);

    return "";
}

std::string FileManager::ParseIncludesInternal(const std::string& filepath, std::unordered_set<std::string>& includedFiles)
{
    if (includedFiles.find(filepath) != includedFiles.end())
        return "";

    includedFiles.insert(filepath);

    std::ifstream file(filepath);
    if (!file.is_open())
    {
        LOG_ERR("ERROR::SHADER::FILE_NOT_FOUND: {}", filepath);
        return "";
    }

    std::stringstream finalCode;
    std::string line;
    std::string directory = FileManager::GetDirectoryPath(filepath);

    while (std::getline(file, line))
    {
        size_t includePos = line.find("#include");
        if (includePos != std::string::npos)
        {
            size_t firstQuote = line.find('\"', includePos);
            size_t lastQuote = line.find_last_of('\"');

            if (firstQuote != std::string::npos && lastQuote != std::string::npos && firstQuote < lastQuote)
            {
                std::string includeFileName = line.substr(firstQuote + 1, lastQuote - firstQuote - 1);
                std::string fullIncludePath = directory + includeFileName;

                finalCode << "// --- BEGIN INCLUDE: " << includeFileName << " ---\n";
                finalCode << ParseIncludesInternal(fullIncludePath, includedFiles) << "\n";
                finalCode << "// --- END INCLUDE: " << includeFileName << " ---\n";
                continue;
            }
        }

        finalCode << line << "\n";
    }

    return finalCode.str();
}

std::string FileManager::get_file_contents_processed(const std::string& filepath)
{
    std::unordered_set<std::string> includedFiles;
    return ParseIncludesInternal(filepath, includedFiles);
}
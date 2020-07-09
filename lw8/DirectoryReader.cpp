#include "DirectoryReader.h"

std::vector<std::string> DirectoryReader::ReadDirectory(std::string pth)
{
    std::vector<std::string> resultFiles;

    for (const auto& entry : fs::directory_iterator(pth))
    {
        resultFiles.push_back(entry.path().string());
    }

    return resultFiles;
}
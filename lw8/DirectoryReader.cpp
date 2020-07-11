#include "DirectoryReader.h"

std::vector<std::string> DirectoryReader::ReadDirectory(std::string pth)
{
    std::vector<std::string> filePaths;

    std::vector<std::string> resultFiles;

    for (const auto& entry : fs::directory_iterator(pth))
    {
        filePaths.push_back(entry.path().string());
    }

    for (std::string filePath : filePaths)
    {
        if (fs::path(filePath).extension() == ".bmp")
        {
            resultFiles.push_back(filePath);
        }
    }

    return resultFiles;
}
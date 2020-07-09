#pragma once
#include<string>
#include<vector>
#include <filesystem>

namespace fs = std::filesystem;

class DirectoryReader
{
public:
	std::vector<std::string> ReadDirectory(std::string pth);
};
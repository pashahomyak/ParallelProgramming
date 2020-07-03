#pragma once
#include <string>
#include <fstream>

class LogFileWriter
{
public:
	LogFileWriter(std::string outputFileName);

	void Write(std::string data);
private:
	std::ofstream output;
};


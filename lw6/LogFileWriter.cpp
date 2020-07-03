#include "LogFileWriter.h"

LogFileWriter::LogFileWriter(std::string outputFileName):
	output(std::ofstream(outputFileName))
{
}

void LogFileWriter::Write(std::string data)
{
	output << data << std::endl;
}

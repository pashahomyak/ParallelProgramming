#pragma once
#include<windows.h>
#include "LinkedList.h"
#include "LogFileWriter.h"
#include<exception>

class LogBuffer
{
public:
	LogBuffer(LogFileWriter* logFileWriter);

	void PushData(std::string data);

	~LogBuffer();

private:
	LogFileWriter* logWriter;
	LinkedList linkedList;
	CRITICAL_SECTION criticalSection;
	HANDLE monitoringHandle;
	HANDLE eventHandle;

	void StartMonitoringThread();
	static DWORD WINAPI LogSizeMonitoringThread(const LPVOID lpParameter);

	const size_t MAX_BUFFER_SIZE = 100;
};
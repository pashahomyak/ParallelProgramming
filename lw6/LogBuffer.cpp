#include "LogBuffer.h"

LogBuffer::LogBuffer(LogFileWriter* logFileWriter):
	logWriter(logFileWriter)
{
	if (!InitializeCriticalSectionAndSpinCount(&criticalSection, 0x00000400))
	{
		throw std::exception("Critical section error");
	}

	eventHandle = CreateEvent(nullptr, true, false, TEXT("Event"));
	
	StartMonitoringThread();
}

void LogBuffer::StartMonitoringThread()
{
	monitoringHandle = CreateThread(NULL, 0, &LogSizeMonitoringThread, (void*)this, 0, NULL);
}

void LogBuffer::PushData(std::string data)
{
	EnterCriticalSection(&criticalSection);

	linkedList.Push(data);
	if (linkedList.GetSize() >= MAX_BUFFER_SIZE)
	{
		SetEvent(eventHandle);

		if (WaitForSingleObject(monitoringHandle, INFINITE) == WAIT_OBJECT_0)
		{
			ResetEvent(eventHandle);

			StartMonitoringThread();
		}
	}

	LeaveCriticalSection(&criticalSection);
}

DWORD WINAPI LogBuffer::LogSizeMonitoringThread(const LPVOID lpParameter)
{
	LogBuffer* logBuffer = (LogBuffer*)lpParameter;

	if (WaitForSingleObject(logBuffer->eventHandle, INFINITE) == WAIT_OBJECT_0)
	{
		size_t linkedListSize = logBuffer->linkedList.GetSize();

		for (int i = 0; i < linkedListSize; i++)
		{
			logBuffer->logWriter->Write(logBuffer->linkedList.GetHeadData());

			logBuffer->linkedList.Pop();
		}
	}

	ExitThread(0);
}

LogBuffer::~LogBuffer()
{
	CloseHandle(eventHandle);
	CloseHandle(monitoringHandle);

	DeleteCriticalSection(&criticalSection);
}
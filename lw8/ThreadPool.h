#pragma once
#include<string>
#include<windows.h>
#include<vector>
#include "ITask.h"
#include "WorkerThread.h"

class ThreadPool
{
public:
	ThreadPool(std::vector<ITask*> tasks, int poolThreadSize);

	static DWORD WINAPI WorkerThreadProc(CONST LPVOID lpParam);

	~ThreadPool();

private:
	int numberOfThreads;
	size_t inputSize;
	bool stopState;

	std::vector<ITask*> tasks;

	std::vector<HANDLE> workerThreads;
};
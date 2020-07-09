#pragma once
#include<string>
#include<windows.h>

class ThreadPool
{
public:
	ThreadPool(HANDLE* handles, int poolThreadSize, int handleSize);

	void Run();

private:
	int numberOfThreads;
	int inputSize;

	HANDLE* handles;
};
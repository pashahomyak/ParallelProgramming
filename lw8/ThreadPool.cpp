#include "ThreadPool.h"

ThreadPool::ThreadPool(HANDLE* handles, int poolThreadSize, int handleSize):
	handles(handles),
	numberOfThreads(poolThreadSize),
	inputSize(handleSize)
{
}

void ThreadPool::Run()
{
	int handleCounter = 0;

	for (int i = 0; i < inputSize; i++)
	{
		ResumeThread(handles[i]);
		handleCounter++;

		if (handleCounter == numberOfThreads)
		{
			WaitForMultipleObjects(i + 1, handles, true, INFINITE);

			handleCounter = 0;
		}
	}

	WaitForMultipleObjects(inputSize, handles, true, INFINITE);
}
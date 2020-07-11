#include "ThreadPool.h"

ThreadPool::ThreadPool(std::vector<ITask*> tasks, int poolThreadSize) :
	tasks(tasks),
	numberOfThreads(poolThreadSize),
	inputSize(tasks.size()),
	stopState(false)
{
	for (int i = 0; i < numberOfThreads; i++)
	{
		HANDLE worker = CreateThread(nullptr, 0, &WorkerThreadProc, this, CREATE_SUSPENDED, nullptr);
		workerThreads.push_back(worker);
	}
}

ThreadPool::~ThreadPool()
{
	stopState = true;

	for (auto worker : workerThreads)
	{
		ResumeThread(worker);
	}

	WaitForMultipleObjects((DWORD)workerThreads.size(), &workerThreads.front(), true, INFINITE);
}

DWORD __stdcall ThreadPool::WorkerThreadProc(const LPVOID lpParam)
{
	ThreadPool* data = (ThreadPool*)(lpParam);

	while (true)
	{
		if (data->stopState && data->tasks.empty())
		{
			ExitThread(0);
		}

		if (!data->tasks.empty())
		{
			ITask* task = data->tasks.front();
			WorkerThread workThread;
			workThread.ExecuteTask(task);

			data->tasks.erase(data->tasks.begin());
		}
	}
}
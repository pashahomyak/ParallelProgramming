#pragma once

#include "IWorker.h"
#include "ThreadPool.h"
#include<windows.h>

class WorkerThread: public IWorker
{
public:
	bool ExecuteTask(ITask* task) override;
	bool IsBusy() override;

	~WorkerThread();

private:
	HANDLE handle;
};
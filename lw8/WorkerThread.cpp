#include "WorkerThread.h"

bool WorkerThread::ExecuteTask(ITask* task)
{
	if (IsBusy())
	{
		return false;
	}

	handle = CreateThread(NULL, 0, &ThreadProc, task, 0, NULL);

	return true;
}

bool WorkerThread::IsBusy()
{
	LPDWORD statusCode = new DWORD;
	GetExitCodeThread(handle, statusCode);

	if (*statusCode == STILL_ACTIVE)
	{
		return true;
	}

	return false;
}

WorkerThread::~WorkerThread()
{
	WaitForSingleObject(handle, INFINITE);
}

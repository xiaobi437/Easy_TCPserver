/*
* 简易的任务系统
* 
*/
#ifndef CELL_TASK_H
#define CELL_TASK_H

#include <thread>
#include <functional>
#include <mutex>
#include <list>
#include <functional>
#include "Memory_pool.h"
#include "Cell_Thread.h"



//执行任务的服务类型
class CellTaskServer
{
private:
	typedef std::function<void()> Cell_Task;
	//任务数据
	std::list<Cell_Task> _tasks;
	//任务数据缓存区
	std::list<Cell_Task> _taskBuf;
	//锁，改变共享数据需要加锁
	std::mutex _mutex;
	
	Cell_Thread _thread;		//线程
	int _id;
public:
	//添加任务
	void addTask(Cell_Task task)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_taskBuf.push_back(task);
	}
	//启动工作线程
	void Start(int id = 0)
	{
			_id = id;
			_thread.Start(nullptr, [this](Cell_Thread* pThread) {
				onRun(pThread);
				}, nullptr);
	}
	void Close()
	{
			printf("Cell_Task%d.Close() begin\n", _id);
			_thread.Close();
			printf("Cell_Task%d.Close() end\n", _id);		
	}
protected:
	//工作函数
	void onRun(Cell_Thread* pThread)
	{
		printf("Cell_Task%d.onRun() begin\n", _id);
		while (pThread->isRun())	//谁调用就用谁的
		{
			if (!_taskBuf.empty())
			{
				//从缓冲区取出数据
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pTask : _taskBuf)
				{
					_tasks.push_back(pTask);
				}
				_taskBuf.clear();
			}
			//如果有任务处理
			if (_tasks.empty())
			{
				std::chrono::milliseconds t(1);		//如果任务多就可以考虑不睡眠
				std::this_thread::sleep_for(t);
				continue;
			}

			for (auto pTask : _tasks)
			{
				pTask();
			}
			_tasks.clear();
		}
		for (auto pTask : _taskBuf)
		{
			pTask();
		}
		_taskBuf.clear();
		printf("Cell_Task%d.onRun() end\n", _id);
	}
};

#endif // !CELLTASK_H



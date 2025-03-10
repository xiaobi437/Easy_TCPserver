/*
* 简易线程任务
* 用于创建线程
*/
#ifndef CELL_THREAD_H
#define CELL_THREAD_H

#include <thread>
#include <mutex>
#include <functional>
#include "Cell_Semaphore.h"

class Cell_Thread
{
private:
	typedef std::function<void(Cell_Thread*)> EventCall;
public:
	//启动线程
	void Start(EventCall onCreate = nullptr, EventCall onRun = nullptr, EventCall onDestory = nullptr)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (!_isRun)
		{
			_isRun = true;

			if (onCreate)	_onCreate = onCreate;
			if (onRun)		_onRun = onRun;
			if (onDestory)	_onDestory = onDestory;
			//线程
			std::thread t(std::mem_fn(&Cell_Thread::OnWork), this);
			t.detach();
		}
	}
	//关闭线程
	void Close() 
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			_isRun = false;
			_sem.wait();
		}	
	}
	//在工作中函数退出线程,不需要使用信号量，如果使用会死锁
	void Exit()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			_isRun = false;
		}
	}
	//线程是否运行中
	bool isRun()
	{
		return _isRun;
	}
private:
	//线程的工作函数
	void OnWork() 
	{
		if (_onCreate)	_onCreate(this);
		if (_onRun)		_onRun(this);
		if (_onDestory)	_onDestory(this);
		_sem.wakeup();
	}

private:
	EventCall _onCreate;
	EventCall _onRun;
	EventCall _onDestory;
	Cell_Semaphore _sem;		//信号
	std::mutex _mutex;			//改变数据时要加锁
	bool _isRun = false;		//启动标志位
};

#endif // !CELL_THREAD_H


/*
* 用条件变量实现的简单信号量
* 
*/
#ifndef CELL_SEMAPHORE_H
#define CELL_SEMAPHORE_H

#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

class Cell_Semaphore
{
public:
	Cell_Semaphore() {}
	~Cell_Semaphore() {}
	void wait() 
	{
		std::unique_lock<std::mutex> lock(_mutex);
		if (--_wait < 0)
		{
			//阻塞等待
			_cond.wait(lock, [this]()->bool {
				 return _wakeup > 0;
				});
			--_wakeup;
		}
	}
	void wakeup() 
	{
		std::unique_lock<std::mutex> lock(_mutex);
		if (++_wait <= 0)
		{
			++_wakeup;
			_cond.notify_one();
		}
		
	}

private:
	std::mutex _mutex;
	std::condition_variable _cond;		//阻塞等待，条件变量
	int _wait = 0;			//等待计数
	int _wakeup = 0;			//唤醒计数
};

#endif // !CELL_SEMAPHORE_H


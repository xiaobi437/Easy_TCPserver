/**
*高精度计时器，依靠C++11特性写出
* 
**/
#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <chrono>		//C++11标准头文件


class Cell_Time
{
public:
	
	//获取当前时间戳（毫秒）
	static time_t getNowMilliSec() {
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	}
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> _begin;
};

class CELLTimestamp
{
public:
	CELLTimestamp()
	{
		update();
	}

	~CELLTimestamp()
	{
	}
	void update() {
		//获取时间
		_begin = std::chrono::high_resolution_clock::now();
	}
	//获取秒
	double getElapsedSecond() {
		return getElapsedMicroSec() * 0.000001;
	}
	//获取毫秒
	double getElapsedMilliSec() {
		return getElapsedMicroSec() * 0.001;
	}
	//获取微秒
	long long  getElapsedMicroSec() {
		//再次获取的时间 - _begin,的值转换为微秒
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - _begin).count();
	}
	
private:
	//尽量少使用命名空间以免发生冲突
	std::chrono::time_point<std::chrono::high_resolution_clock> _begin;
};



#endif // !CELLTIME_H


#include <stdlib.h>
#include <iostream>
#include <thread>
#include <mutex>//锁
#include "Timestamp.h"
#include "Memory_pool.h"

//重载new、delete运算符
void* operator new(size_t nSize)
{
	return MemoryMgr::Instance().allocMem(nSize);
}
void* operator new[](size_t nSize)
{
	return MemoryMgr::Instance().allocMem(nSize);
}

void operator delete(void* p)
{
	MemoryMgr::Instance().freeMem(p);
}
void operator delete[](void* p)
{
	MemoryMgr::Instance().freeMem(p);
}
//重写malloc、free
void* mem_alloc(size_t size)
{
	return malloc(size);
}
void mem_free(void* p)
{
	free(p);
}



#if 0
//原子操作   原子 分子 
std::mutex m;
const int tCount = 8;
const int mCount = 100000;
const int nCount = mCount / tCount;
void workFun(int index)
{
	char* data[nCount];
	for (size_t i = 0; i < nCount; i++)
	{
		data[i] = new char[(rand() % 128) + 1];
	}
	for (size_t i = 0; i < nCount; i++)
	{
		delete[] data[i];
	}
	/*
	for (int n = 0; n < nCount; n++)
	{
		//自解锁
		//lock_guard<mutex> lg(m);
		//临界区域-开始
		//m.lock();

		//m.unlock();
		//临界区域-结束
	}//线程安全 线程不安全
	 //原子操作 计算机处理命令时最小的操作单位
	 */
}//抢占式

int main()
{
	//std::thread t[tCount];
	//for (int n = 0; n < tCount; n++)
	//{
	//	t[n] = std::thread(workFun, n);
	//}
	//CELLTimestamp tTime;
	//for (int n = 0; n < tCount; n++)
	//{
	//	t[n].join();
	//	//t[n].detach();
	//}
	//std::cout << tTime.getElapsedMilliSec() << std::endl;
	//std::cout << "Hello,main thread." << std::endl;
	int n = 0;
	char* data = nullptr;
	while (1)
	{
		printf("input:\n");
		std::cin >> n;
		switch (n)
		{
		case 1:
			data = new char[100000000];
			continue;
		case 2:
			delete []data;
			continue;
		default:
			continue;
		}
	}
	return 0;
}
#endif
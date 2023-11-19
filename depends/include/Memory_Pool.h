/*
*	内存池底层实现 
*/
#ifndef MEMORY_POOL_BASE_H
#define MEMORY_POOL_BASE_H

//#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <mutex>		//锁
#include <memory>		//智能指针所需头文件
#include <assert.h>		//断言，调试时使用


#define MAX_MEMORY_SIZE 128
#ifdef _DEBUG
	#ifndef xPrintf
	#include<stdio.h>
	#define xPrintf(...) printf(__VA_ARGS__)
#endif // !xPrintf
#else
	#ifndef xPrintf
	#define xPrintf(...)
	#endif // !xPrintf
#endif // _DEBUG
/******************************************************/
//内存块 最小单元
class MemoryAlloc;
class MemoryBlock
{
public:
	int nID;				//内存编号
	int nRef;				//引用次数(目前有多少对象在使用这个块内存)
	MemoryAlloc* pAlloc;	//所属所属大内存块（池）
	MemoryBlock* pNext;		//下一块内存位置
	//size_t nSize;				//内存池大小
	bool bPool;				//是否在内存池中

	MemoryBlock() {
		nID = 0;
		nRef = 0;
		pAlloc = nullptr;
		pNext = nullptr;
		bPool = true;
		memset(c,0,sizeof(c));
	}
	~MemoryBlock() {}
private:
	char c[3];			//预留，因为在64位中是8字节对齐，实际大小为32，计算25缺7
};
//int a = sizeof(MemoryBlock);
//内存池
class MemoryAlloc
{
public:
	MemoryAlloc() {
		_pBuf = nullptr;
		_pHeader = nullptr;
		_nSize = 0;
		_nBlockLen = 0;
	}
	~MemoryAlloc() {
		if (_pBuf)
		{
			free(_pBuf);
		}
	}
	//申请内存
	void* allocMemory(size_t nSize)
	{
		//加锁
		std::lock_guard<std::mutex> lg(_mutex);
		if (!_pBuf)
		{
			initMemory();
		}
		MemoryBlock* pReturn = nullptr;
		//当池子没有空间的时候
		if (_pHeader == nullptr)
		{
			pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			pReturn->bPool = false;
			//pReturn->nSize = _nSize;
			pReturn->nID = -1;
			pReturn->nRef = 0;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
			//xPrintf("allocPoolMem: %llx, nID：%d, size：%d\n", pReturn, (int)pReturn->nID, (int)nSize);
		}
		else {
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}
		xPrintf("allocPoolMem: %llx, nID：%d, size：%d\n", pReturn, (int)pReturn->nID, (int)nSize);
		//返回内存地址（头的地址+MemoryBlock就是可用内存的地址）
		return ((char*)pReturn + sizeof(MemoryBlock));
	}
	//释放内存
	int freeMemory(void* pMem)
	{
		{
			//传进来的地址，只是可用内存的地址，我们需要块的地址
			MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
			assert(1 == pBlock->nRef);
			if (pBlock->bPool)		//是否在池中
			{
				std::lock_guard<std::mutex> gl(_mutex);		//加锁
				if (--pBlock->nRef != 0)
				{
					return -1;		//多人在引用就直接返回不释放
				}
				pBlock->pNext = _pHeader;
				_pHeader = pBlock;
			}
			else
			{
				if (--pBlock->nRef != 0)
				{
					return -1;		//多人在引用就直接返回不释放
				}
				free(pBlock);
			}
			return 0;
		}
	}
	//初始化内存池
	int initMemory()
	{
		std::cout << "initMemory()" << std::endl;
		xPrintf("initPoolMemory:_nSzie=%zd,_nBlockSzie=%zd\n", _nSize, _nBlockLen);

		assert(nullptr == _pBuf);		//断言
		if (_pBuf)
			return -1;

		//向系统申请池内存
		size_t bufSize = (_nSize + sizeof(MemoryBlock)) * _nBlockLen;	//计算内存池大小
		_pBuf = (char*)malloc(bufSize);			//申请内存池
		if (!_pBuf)
		{
			std::cout << "malloc failed" << std::endl;
			return -1;
		}
		//初始化内存池
		_pHeader = (MemoryBlock*)_pBuf;
		_pHeader->bPool = true;
		//_pHeader->nSize = _nSize;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pAlloc = this;
		_pHeader->pNext = nullptr;

		//遍历块初始化
		//我们只知道头和头->next，不知道头->next->next
		MemoryBlock* pTemp1 = _pHeader;			//当前位置   头
		for (size_t n = 1; n < _nBlockLen; n++) {
			MemoryBlock* pTemp2 = (MemoryBlock*)(_pBuf + (n * (_nSize + sizeof(MemoryBlock))));	//计算得出next
			pTemp2->bPool = true;
			//pTemp2->nSize = _nSize;
			pTemp2->nID = (int)n;
			pTemp2->nRef = 0;
			pTemp2->pAlloc = this;
			pTemp2->pNext = nullptr;		//next->next=nullptr；
			pTemp1->pNext = pTemp2;			//当前位置的next=next；
			pTemp1 = pTemp2;				//当前位置=next；
		}
		return 0;
	}
protected:
	char* _pBuf;			//内存池地址
	MemoryBlock* _pHeader;	//头部内存单元的地址
	size_t _nSize;			//内存单元的大小
	size_t _nBlockLen;		//内存单元的数量
	
	std::mutex _mutex;		//锁
};
template<size_t nSize, size_t nBlockLen>//模板类
class MemoryAlloctor :public MemoryAlloc
{
public:
	MemoryAlloctor()
	{
		size_t n = sizeof(void*);		//获取字节对其数，64位8，32位4
		//补齐对齐数,防止输入参数不足对齐。
		//if = (61/8)*8 + (61%8 ? 8:0) -> 7*8 + 8 = 64
		_nSize = (nSize / n) * n + (nSize % n ? n : 0);
		_nBlockLen = nBlockLen;
	}
};
//内存管理工具
class MemoryMgr
{
private:	//使用单例模式设计，构造、析构、拷贝、重载私有化，只能通过特定接口获取
	MemoryMgr() 
	{	
		init_szAlloc(0,64,&_mem_64);			//输入0-64的内存，就会定位到64字节的内存池，池内有100个块
		init_szAlloc(65, 128, &_mem_128);	//输入56-128的内存，就会定位到128字节的内存池，池内有100个块
		//init_szAlloc(129, 256, &_mem_256);
		//init_szAlloc(257, 512, &_mem_512);
		//init_szAlloc(513, 1024, &_mem_1024);
	}
	~MemoryMgr() {
		/*
		for (size_t n = 0; n < MAX_MEMORY_SIZE; n++) {
			freeMem(_szAlloc[n]);
		}
		*/
	}
	MemoryMgr(MemoryMgr& Mgr) {}
	void operator=(MemoryMgr& Mgr) {}
public:
	static MemoryMgr& Instance()
	{
		//std::cout << " MemoryMgr& Instance()" << std::endl;
		static MemoryMgr mgr;
		return mgr;
	}
	//申请内存
	void* allocMem(size_t nSize)
	{
		if (nSize <= MAX_MEMORY_SIZE) {		
			return _szAlloc[nSize]->allocMemory(nSize);
		}
		else {
	
			MemoryBlock* pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			if (nullptr != pReturn)
			{
				pReturn->bPool = false;
				pReturn->nID = -1;
				pReturn->nRef = 1;
				pReturn->pAlloc = nullptr;
				pReturn->pNext = nullptr;
				//printf("allocPoolMem: %llx, size：%d\n", pReturn, (int)nSize);
				//xPrintf("allocPoolMem: %llx, nID：%d, size：%d\n", pReturn, (int)pReturn->nID, (int)nSize);
				//Memory_Info("allocPoolMem: %llx, size：%d\n", pReturn, (int)nSize);
				return ((char*)pReturn + sizeof(MemoryBlock));
				
			}
			else {
				printf("Memory pool allocPoolMem %ld Failed!\n", nSize);
				return NULL;
			}
			
		}			
	}
	//释放内存
	int freeMem(void* pMem) 
	{
		if (!pMem)
			return -1;
		//传进来的地址，只是可用内存的地址，我们需要块的地址
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		//Memory_Info("freeMem: %llx, nID：%d\n", pBlock, pBlock->nID);
		xPrintf("freeMem: %llx, nID：%d\n", pBlock, pBlock->nID);
		if (pBlock->bPool)		//是否在池中
		{
			pBlock->pAlloc->freeMemory(pMem);
		}
		else
		{
			//printf("freeMem: %llx, nID：%d\n", pBlock, pBlock->nID);
			if(--pBlock->nRef == 0)
				free(pBlock);
		}
		return 0;
	}
	//增加内存块的引用计数
	void addRef(void* pMem) {
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		++pBlock->nRef;
	}
private:
	//初始化内存池映射数组
	void init_szAlloc(int nBegin, int nEnd, MemoryAlloc* pMemA)
	{
		//open_log_file();
		for (int n = nBegin; n <= nEnd; n++) 
		{
			_szAlloc[n] = pMemA;
		}
	}
	/* //写入日志文件
	//打开文件
	bool open_log_file()
	{
		if (_MemorylogFile)
		{
			printf("memory_pool:: _logErrorFile !=nullptr \n");
			fclose(_MemorylogFile);
			_MemorylogFile = nullptr;
		}
		const char* FilePath = "C:\\Users\\Xiaob\\Desktop\\Server_Log\\Memory_pool_Log.txt";
		_MemorylogFile = fopen(FilePath, "a+");
		if (!_MemorylogFile)
		{
			printf("memory_pool:: %s open a+ failed\n", FilePath);
			return false;
		}
		return true;
	}
	//写入文件
	template<typename ...Args>
	void Memory_Info(const char* pStr, Args ... args)
	{
		if (_MemorylogFile)
		{
			fprintf(_MemorylogFile, "<Warning>: ");
			fprintf(_MemorylogFile, pStr, args...);
			fflush(_MemorylogFile);
		}
	}
	*/
private:
	MemoryAlloctor<64,2000000> _mem_64;			//1000000=64m的内存
	MemoryAlloctor<128, 1000000> _mem_128;		//1000000=128m的内存
	//MemoryAlloctor<256, 100000> _mem_256;		//1000000=256m的内存
	//MemoryAlloctor<512, 100000> _mem_512;		//1000000=512m的内存
	//MemoryAlloctor<1024, 100000> _mem_1024;		//1000000=1G的内存
	MemoryAlloc* _szAlloc[MAX_MEMORY_SIZE + 1];

	//FILE* _MemorylogFile = nullptr;
};

//重载new、delete运算符
void* operator new(size_t nSize);
void* operator new[](size_t nSize);
void operator delete(void* p);
void operator delete[](void* p);
//重写malloc、free
void* mem_alloc(size_t size);
void mem_free(void* p);
#endif // !MEMORY_POOL_H

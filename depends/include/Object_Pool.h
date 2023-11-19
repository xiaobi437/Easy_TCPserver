/*
*对象池
*使用方式：继承ObjectPoolBase
*/
#ifndef OBJECT_POOL_H_
#define OBJECT_POOL_H_

#include "Public_Include.h"
#include <mutex>
#include <assert.h>		//断言，调试时使用
#include "Memory_Pool.h"		//内存池

//#include "Cell_Log.h"

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

//对象块
class NodeHeader
{
public:
	NodeHeader* pNext;		//下一块对象位置
	int nID;				//对象编号
	unsigned char nRef;				//引用次数(目前有多少对象在使用这个块对象池中的引用次数)
	bool bPool;				//是否在对象池中

	NodeHeader() {
		pNext = nullptr;
		nID = 0;
		nRef = 0;
		bPool = true;
		memset(c, 0, sizeof(c));
	}
	~NodeHeader() {}
private:
	char c[2];			//预留，因为在64位中是8字节对齐，实际大小为16，计算14缺2
};
//对象池管理
template<class Type,size_t nPoolLen>
class ObjectPool
{
public:
	ObjectPool() {
		_pHeader = nullptr;
		_pBuf = nullptr;
		initObjectPool();
	}
	~ObjectPool() {
		if(_pBuf)
			delete[] _pBuf;
	}
	//申请对象内存
	void* allocObject(size_t nSize) {
		//加锁
		std::lock_guard<std::mutex> lg(_mutex);
		NodeHeader* pReturn = nullptr;
		//当池子没有空间的时候
		if (_pHeader == nullptr)
		{
			pReturn = (NodeHeader*)new char[nSize + sizeof(NodeHeader)];
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pNext = nullptr;
			xPrintf("allocObject: %llx, nID：%d, size：%d\n", pReturn, (int)pReturn->nID, (int)nSize);
		}
		else {
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}
		//xPrintf("allocObject: %llx, nID：%d, size：%d\n", pReturn, (int)pReturn->nID, (int)nSize);
		//printf("allocObject: %llx, nID：%d, size：%d\n", pReturn, (int)pReturn->nID, (int)nSize);
		//返回内存地址（头的地址+MemoryBlock就是可用内存的地址）
		return ((char*)pReturn + sizeof(NodeHeader));
	}
	//释放对象内存
	int freeObjectMemory(void* pMem) {
		//传进来的地址，只是可用内存的地址，我们需要块的地址
		NodeHeader* pBlock = (NodeHeader*)((char*)pMem - sizeof(NodeHeader));
		//printf("freeObject:%llx,id=%d\n", pBlock, pBlock->nID);
		assert(1 == pBlock->nRef);
		if (pBlock->bPool)		//是否在池中
		{
			std::lock_guard<std::mutex> gl(_mutex);		//加锁
			//xPrintf("freeObject:%llx,id=%d\n", pBlock, pBlock->nID);
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
			delete pBlock;
		}
		return 0;
	}
	//初始化对象池
	int initObjectPool()
	{
		xPrintf("initObjectPool:TypeSzie=%zd,_nBlockSzie=%zd\n", sizeof(Type), nPoolLen);
		assert(nullptr == _pBuf);		//断言
		if (_pBuf)
			return -1;
		size_t n = nPoolLen * (sizeof(Type) + sizeof(NodeHeader));	//计算大小
		_pBuf = new char[n];	//优先会向内存池申请，没有在向系统

		//初始化对象池
		_pHeader = (NodeHeader*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pNext = nullptr;

		//遍历块初始化
		//我们只知道头和头->next，不知道头->next->next
		NodeHeader* pTemp1 = _pHeader;			//当前位置   头
		for (size_t n = 1; n < nPoolLen; n++) {
			NodeHeader* pTemp2 = (NodeHeader*)(_pBuf + (n * (sizeof(Type) + sizeof(NodeHeader))));	//计算得出next
			pTemp2->bPool = true;
			pTemp2->nID = n;
			pTemp2->nRef = 0;
			pTemp2->pNext = nullptr;		//next->next=nullptr；
			pTemp1->pNext = pTemp2;			//当前位置的next=next；
			pTemp1 = pTemp2;				//当前位置=next；
		}
		return 0;
	}

private:
	NodeHeader* _pHeader;	
	char* _pBuf;			//对象池地址
	std::mutex _mutex;		//锁
};

//对象池对外的接口
template<class Type,size_t nPoolLen>
class ObjectPoolBase
{
private:
	typedef ObjectPool<Type, nPoolLen> classTypePool;
	static classTypePool& objectPool() 
	{
		static classTypePool sPool;	//静态的对象池对象
		return sPool;
	}
public:
	void* operator new(size_t nSize) {
		return objectPool().allocObject(nSize);
	}
	void operator delete(void* p) {
		objectPool().freeObjectMemory(p);
	}
	template<typename ...Args>	//不定参数 可变参数
	//对外开放的另一个接口，如果创建对象的时候有其他事情需要处理的话，可以使用这个接口，没有可以直接使用new
	static Type* createObject(Args ... args) 
	{
		Type* obj = new Type(args ...);
		//可以做点其他事情
		return obj;
	}
	static void destroyObject(Type* obj) {
		delete obj;
	}
};

#endif // !OBJECTPOOL_H_



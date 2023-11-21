#include "Memory_Pool.h"

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



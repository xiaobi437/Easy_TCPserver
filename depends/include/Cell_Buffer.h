/*
* 消息缓存区
* 处理收发
*/
#ifndef CELL_BUFFER_H
#define	CELL_BUFFER_H

#include "Public_Include.h"
#include "Memory_pool.h"
#include "Message_Header.h"

class Cell_Buffer
{
public:
	Cell_Buffer(int nSize = 8192) 
	{
		_pBuff = new char[nSize];
		_nLast = 0;
		_nSize = nSize;
		_sendBuffFullCount = 0;
	}
	~Cell_Buffer() 
	{
		if(_pBuff)
			delete[] _pBuff;
	}
	bool push(char* pData, int nLen)
	{
		if (!pData)
			return false;
		//业务拓展，数据超过缓冲区，动态增加缓冲区大小
		/*
		if (_nLast + nLen > _nSize)
		{
			//需要写入的数据大小
			//写入大量数据不一定要写入内存中，可以写入数据库
			int n = (_nLast + nLen) - _nSize;
			//拓展buff
			if (n < 8192)
				n = 8192;
			char* buff = new char[_nSize+n];
			memcpy(buff, _pBuff, _nLast);
			delete[] _pBuff;
			_pBuff = buff;
		}
		*/
		//当数据未达到缓冲区的大小时，就存入缓冲区
		if (_nLast + nLen <= _nSize)	
		{
			//拷贝数据
			memcpy(_pBuff + _nLast, pData, nLen);
			_nLast += nLen;
			if (_nLast == _nSize)
			{
				_sendBuffFullCount++;
			}
			//printf("delete login_result sizeof:%d,%llx\n", sizeof(LoginResult), pData);
			delete pData;
			return true;
		}
		else
		{
			Cell_Log::Info(Error_Msg, "_sendBuffFullCount....\n");
			_sendBuffFullCount++;
			delete pData;
		}
		return false;
	}
	//发送数据给客户端
	int Write_to_Socket(SOCKET sockfd)
	{
		int ret = 0;
		//缓存区有数据
		if (_nLast > 0 && sockfd != INVALID_SOCKET)
		{
			//有多少发多少而不是，全部发送
			ret = send(sockfd, _pBuff, _nLast, 0);
			_nLast = 0;
			//printf("Write_to_Socket: socket:%d, time:%lld,send:%d\n", (int)_sockfd, _dtSend, ret);
			_sendBuffFullCount = 0;
		}
		return ret;
	}
	//接收客户端消息
	int Read_for_Socket(SOCKET sockfd)
	{
		if (_nSize - _nLast > 0)
		{
			//直接把数据拷贝到客户端的缓冲区，就免去了拷贝的步骤
			char* szRecv = _pBuff + _nLast;
			//接收数据存到接收缓冲区，接收大小缓冲区大小（10KB）
			int nLen = (int)recv(sockfd, szRecv, _nSize - _nLast, 0);
			if (nLen <= 0)
			{
				return nLen;
			}
			//记录缓冲区数据最后的位置,当有新的数据接收的时候就可以使用这个位置继续存放数据
			_nLast += nLen;
			return nLen;
		}
		return 0;	
	}
	//判断缓冲区的消息是否够一条
	bool hasMsg()
	{
		//判断一下消息缓存区的数据是由大于消息头,一直循环处理
		if (_nLast >= sizeof(DataHeader))
		{
			DataHeader* data_head = (DataHeader*)_pBuff;
			return _nLast >= data_head->dataLength;
		}
		return false;
	}
	//有数据需要写
	bool needWrite()
	{
		return _nLast > 0;
	}
	//返回消息头
	char* data()
	{
		return _pBuff;
	}
	//移除缓冲区第一条数据
	void pop(int nLen)
	{
		int n = _nLast - nLen;
		if (n > 0)
		{
			//将消息缓冲区剩余未处理数据前移
			memcpy(_pBuff, _pBuff + nLen, n);
		}
		//将第二缓冲区的数据尾部位置往前移
		_nLast = n;
		if (_sendBuffFullCount > 0)
			--_sendBuffFullCount;
	}
private:
	char* _pBuff = nullptr;		   //消息接收缓冲区 动态大小
	//std::list<char*> _BuffList;	//可以使用链表形式	
	int _nLast;					  //记录接收消息缓冲区数据的末尾，用于下次新数据存入使用
	int _nSize;					//缓冲区大小
	int _sendBuffFullCount;		//缓冲区写满计数
};


#endif // !CELL_BUFFER_H


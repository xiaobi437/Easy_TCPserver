/*
* client程序 
*/
#ifndef CLIENT_H
#define CLIENT_H

#include "Public_Include.h"
#include <vector>
#include <cstring>
#include <thread>
#include <atomic>
#include "Message_Header.h"
#include "Timestamp.h"
#include "Cell_NetWork.h"
#include "Cell_Log.h"
#include "Cell_Buffer.h"
#include "Cell_Client.h"
#include "Cell_Task.h"


class Client
{
public:
	Client() {
		_pClient = nullptr;
		Init_sock();
	}
	virtual ~Client() {
		Close();
	}

	//初始化socket
	SOCKET Init_sock()
	{
		Cell_NetWork::Init();
		if (_pClient)
		{
			Close();
		}
		// 创建socket套接字
		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == SOCKET_ERROR)
		{
			Cell_Log::Info(Error_Msg, "Client:: socket failed!\n");
			return -1;
		}
		sockaddr_in sin_client = {};
		sin_client.sin_family = AF_INET;
		sin_client.sin_port = htons(10000);
#ifdef _WIN32
		sin_client.sin_addr.S_un.S_addr = INADDR_ANY;
#else
		sin_server.sin_addr.s_addr = INADDR_ANY;
#endif // _WIN32
		_pClient = new Cell_Client(sin_client, sock);
		return sock;
	}
	//连接服务器
	int Connect(const char* ip, unsigned short port)
	{
		int ret = SOCKET_ERROR;
		if (!_pClient)
		{
			Init_sock();
		}
		// 定义IPV4地址结构体，用于存放服务器信息
		sockaddr_in sin_server = {};
		sin_server.sin_family = AF_INET;
		sin_server.sin_port = htons(port);
#ifdef _WIN32
		sin_server.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		sin_server.sin_addr.s_addr = inet_addr(ip);
#endif // _WIN32
		ret = connect(_pClient->sockfd(), (sockaddr*)&sin_server, sizeof(sin_server));
		if (ret == SOCKET_ERROR)
		{
			Cell_Log::Info(Error_Msg, "Client:: connect<%s:%d> failed!\n", ip, port);
			return ret;
		}
		return ret;
	}
	//关闭socket
	int Close()
	{
		if (_pClient)
		{
			delete _pClient;
			_pClient = nullptr;
		}
		return 0;
	}

	//接收数据 处理粘包、拆包
	int RecvData()
	{
		int nLen = _pClient->RecvData();
		if (nLen <= 0)
			return -1;
		//循环处理 判断一下消息缓存区的数据是否够一条消息
		while (_pClient->hasMsg())
		{
			//处理网络消息
			onNetMsg(_pClient,_pClient->front_msg());
			//移除消息队列（缓冲区）最前的一条数据
			_pClient->pop_front_msg();
		}
		return 0;
	}
	//处理包头
	virtual int onNetMsg(Cell_Client* _pClient, DataHeader* data_head) = 0;
	//发送数据
	int SendData(DataHeader* data_head, int nLen)
	{
		
		return _pClient->sendData(data_head);
	}
	//查询网络消息
	bool onRun()
	{
		if (isRun())
		{
			SOCKET sock = _pClient->sockfd();
			//创建 伯克利socket描述符合集
			fd_set fd_Read;		//读
			FD_ZERO(&fd_Read);	// 初始化,清空

			fd_set fd_Write;		//写
			FD_ZERO(&fd_Write);	// 初始化,清空

			// 设置超时时间,让select变非阻塞
			timeval t = { 0, 1 };
			int ret = 0;

			// 监视是否可操作
			FD_SET(sock, &fd_Read); // 读

			//判断是否有数据写
			if (_pClient->needWrite())
			{
				FD_SET(sock, &fd_Write);
				ret = select(sock + 1, &fd_Read, &fd_Write, nullptr, &t);
			}
			else
			{
				ret = select(sock + 1, &fd_Read,nullptr, nullptr, &t);
			}

			
			// 参数:伯克利socket（socket+1）,
			// nfds 是一个整数值,是指fd_set集合中所有描述符（socket）的范围,而不是数量
			// 既是所有文件描述符最大值+1,在windows中可以写0；
			//int ret = select(sock + 1, &fd_Read, &fd_Write, nullptr, &t);
			if (ret < 0)
			{
				Cell_Log::Info(Error_Msg, "Client:: select<socket:%d> failed!\n", (int)_pClient->sockfd());
				Close();
				return false;
			}
			// 套接字是否发生了读就绪-->说明有服务端有消息
			if (FD_ISSET(sock, &fd_Read))
			{
				if (RecvData() == -1)
				{
					Cell_Log::Info(Error_Msg, "Client:: RecvData()<socket:%d> failed....\n", (int)_pClient->sockfd());
					Close();
					return false;
				}
			}
			if (FD_ISSET(sock, &fd_Write))
			{
				if (_pClient->sendDataReal() == -1)
				{
					Cell_Log::Info(Error_Msg, "Client:: sendDataReal()<socket:%d> failed....\n", (int)_pClient->sockfd());
					Close();
					return false;
				}
			}
		}
		return true;
	}
	bool isRun()
	{
		if (_pClient)
		{
			return true;
		}
		printf("_pClient==nullprt;");
		return false;
	}
	SOCKET get_socket()
	{
		return _pClient->sockfd();
	}
private:
	//高精度计时器
	CELLTimestamp _tTime;
	//接收缓冲区
	Cell_Buffer _recvBuff;
	//发送缓冲区
	Cell_Buffer _sendBuff;

	Cell_Client* _pClient;
};

#endif // ! 




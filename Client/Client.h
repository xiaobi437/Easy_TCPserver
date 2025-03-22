#ifndef CLIENT_H
#define CLIENT_H

#ifdef _WIN32
#define FD_SETSIZE 1024			//要在头文件winsock2.h前定义
#define WIN32_LEAN_AND_MEAN		//这个宏可以尽量避免早期的宏和其他依赖库的使用
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <windows.h>

#else
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <algorithm>		//linux下,vector 中find函数,需要这个头文件

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)

#endif // _WIN32
#include "Message_Header.h"
#include "CELLTime.h"
#include <iostream>
#include <vector>
#include <cstring>
#include <stdio.h>
#include <Thread>
#include <atomic>
#pragma comment(lib,"ws2_32.lib")
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240    //缓冲区最小单元大小 10KB
#endif // !RECV_BUFF_SIZE

std::atomic<int> sendCount;
std::atomic<int> recvCount;

class Client
{
public:
	Client() {
		_sock = INVALID_SOCKET;
		_lastPos = 0;
		_sendConut = 0;
		_isConnect = false;
		memset(_szRecv, 0, sizeof(_szRecv));
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		Init_sock();
	}
	virtual ~Client() {
		Close_sock();
	}

	//初始化socket
	int Init_sock();
	//连接服务器
	int Connect(const char *ip, unsigned short port);
	//关闭socket
	int Close_sock();

	//接收数据 处理粘包、拆包
	int RecvData(SOCKET _cSock);
	//处理包头
	int onNetMsg(DataHeader* data_head);
	//发送数据
	int SendData(DataHeader* data_head, int nLen);
	//查询网络消息
	bool onRun();
	bool isRun();
	//获取socket
	SOCKET getSock() {
		return _sock;
	}
private:
	SOCKET _sock;
	bool _isConnect;		//连接状态

	char _szRecv[RECV_BUFF_SIZE];		//接收缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE*5];		//第二缓冲区，消息缓冲区50KB
	int _lastPos;							//记录消息缓冲区数据的末尾，用于下次新数据存入使用

	//高精度计时器
	CELLTimestamp _tTime;
	int _sendConut;
};

#endif // ! 




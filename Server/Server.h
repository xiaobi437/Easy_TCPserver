#ifndef SERVER_H
#define SERVER_H

#include "Public_Include.h"

#include <stdio.h>
#include <thread>
#include <cstring>
#include <vector>
#include <mutex>
#include <atomic>			//原子操作
#include <functional>		//mem_fun  mem_fn 所需头文件
#include <memory>			//智能指针

#include "Message_Header.h"
#include "Timestamp.h"
#include "Cell_Task.h"
#include "Memory_pool.h"
#include "Object_Pool.h"
#include "Cell_Client.h"
#include "Cell_Server.h"
//网络报文格式:包头+包体
//包头

#define TEXE_SEND 1

class Server :public NetEvent
{
private:
	std::mutex _mutex1;

	SOCKET _sock;		//服务器socket
	std::vector<Cell_Server*> _cellServers;
	std::thread* _pThread;	//线程对象

	int _CellServer_thread_connt;		//细胞线程数量
protected:
	std::atomic<int> _recvMsg;				//客户端接收消息计数
	std::atomic<int> _clientCount;				//客户端计数

	std::atomic<int> _recvCount;				//客户端接收计数
	std::atomic<int> _sendCount;				//客户端发送计数
	//高精度计时器
	CELLTimestamp _tTime;
public:
	Server()//const char* ip, unsigned short port) 
	{
		_sock = INVALID_SOCKET;
		_recvMsg = 0;
		_clientCount = 0;
		_recvCount = 0;
		_sendCount = 0;
		//Init_sock(ip, port);
	}
	virtual ~Server()
	{
		Close();
	}
	SOCKET get_socketfd() {
		return _sock;
	}
	//初始化网络环境
	SOCKET Init_sock(const char* ip, unsigned short port)
	{
#ifdef _WIN32
		//创建版本号,是我们使用的是sock2.2,输入最好也是2.2。参数1:高位字节指定次要版本号,参数2:低位字节主要版本号
		WORD ver = MAKEWORD(2, 2);
		//指向WSADATA数据结构的指针, 该数据结构将接收Windows套接字实现的详细信息。
		WSADATA dat;
		//启动windows网络库
		if (SOCKET_ERROR == WSAStartup(ver, &dat)) {  //启动这个库需要链接静态库,有两个办法
			printf("WSAStartup failed\n");
			return -1;
		}
#endif

		//1、socket 创建套接字,IPV4,面向数据流,TCP协议
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (_sock == SOCKET_ERROR) {
			printf("socket failed\n");
			return INVALID_SOCKET;
		}

		//2、bind 绑定用于客户端连接的端口IP
	//定义IPV4地址结构体,用于存放要绑定的IP,端口号
		sockaddr_in sin = {};
		sin.sin_family = AF_INET;		//IPV4协议
		sin.sin_port = htons(port);	//端口号
#ifdef _WIN32	
		sin.sin_addr.S_un.S_addr = inet_addr(ip);  //ip
#else
		sin.sin_addr.s_addr = inet_addr(ip);  //ip
		int on = 1;
		setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
#endif
		//因为sockaddr这个方式不利用与我们填写,所以要使用相近的类型
		if (SOCKET_ERROR == bind(_sock, (sockaddr*)&sin, sizeof(sin))) {
			printf("bind failed\n");
			return -1;
		}
		else {
			printf("socket :%d, port :%d\n", (int)_sock, port);
			printf("The network 、port is bound successfully....\n");
		}

		return _sock;
	}
	//启动服务器，等待连结
	int Listen(int listen_number = 5)
	{
		//3、listen 监听网络端口,最大监听数5
		if (listen(_sock, listen_number) == SOCKET_ERROR) {
			printf("listen failed\n");
			return -1;
		}
		else {
			printf("Listening to the network port succeeded....\n");
		}

		return 0;
	}
	//接收客户端连接
	SOCKET Accept_clenit()
	{
		//4、accept 等待接受客户端链接
		sockaddr_in clientaddr = {};
		int clientaddr_len = sizeof(clientaddr);
		SOCKET clientsock = INVALID_SOCKET;
#ifdef _WIN32				
		clientsock = accept(_sock, (sockaddr*)&clientaddr, &clientaddr_len);
#else
		clientsock = accept(_sock, (sockaddr*)&clientaddr, (socklen_t*)&clientaddr_len);
#endif
		if (clientsock == SOCKET_ERROR) {
			printf("accept failed\n");
			return INVALID_SOCKET;
		}
		else
		{
			//printf("New client accept<socket:%d, %s:%u>\n", (int)clientsock, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));//inet_ntoa,把网络字节序的IP转字符串IP
			//将新客户端分配给客户端数量最少的cell_Server
			Cell_Client* pClient(new Cell_Client(clientaddr, clientsock));
			addClinetToCellserver(pClient);
		}
		return clientsock;
	}
	//运行状态
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	//运行
	bool onRun()
	{
		//创建 伯克利socket描述符合集
		fd_set fd_Read;		//读
		if (isRun()) {
			//初始化,清空
			FD_ZERO(&fd_Read);
			//监视是否可操作
			FD_SET(_sock, &fd_Read);

			//设置超时时间,让select变非阻塞
			timeval t = { 0,10 };
			//参数:伯克利socket（socket+1）,
			//nfds 是一个整数值,是指fd_set集合中所有描述符（socket）的范围,而不是数量
			//既是所有文件描述符中最大值+1,在windows中可以写0；
			int ret = select(_sock + 1, &fd_Read, NULL, NULL, &t);
			if (ret < 0) {
				printf("<socket:%d>select failed!\n", (int)_sock);
				return false;
			}
			//判断旧的套接字是否发生了读就绪-->说明有客户端连接服务器
			if (FD_ISSET(_sock, &fd_Read)) {
				//清理一下标志位
				FD_CLR(_sock, &fd_Read);
				//printf("[%d]", _client_number);
				Accept_clenit();
			}
			return true;
		}
		return false;
	}
	//关闭socket
	int Close()
	{
		xPrintf("Server.Close()\n");
		if (_sock != INVALID_SOCKET) {
			for (auto iter : _cellServers) {
				delete iter;
			}
			_cellServers.clear();
#ifdef _WIN32
			//6、关闭socket
			closesocket(_sock);
			//清除Windows网络环境，socket。
			WSACleanup();
#else
			close(_sock);
#endif
		}
		_sock = INVALID_SOCKET;
		return 0;
	}
	//往缓冲队列添加客户端
	int addClinetToCellserver(Cell_Client* pclinet)
	{
		//查询客户端最小的cell_server,消息对象
		auto pMinServer = _cellServers[0];
		for (auto pCellserver : _cellServers) {
			if (pMinServer->getClinetCount() > pCellserver->getClinetCount()) {
				pMinServer = pCellserver;
			}
		}
		pMinServer->addClient(pclinet);
		return 0;
	}
	//开启线程处理客户端消息
	void Start(int Cellserver_thread_number)
	{
		_CellServer_thread_connt = Cellserver_thread_number;
		for (int n = 0; n < _CellServer_thread_connt; n++) {
			//创建Cell_Server对象，传入this指针(方便使用网络事件)，sock
			auto ser = new Cell_Server(this, n+1);
			_cellServers.push_back(ser);
			//启动服务线程
			ser->Start();
		}
	}
	//计算包数，带宽
	void time4msg() {
		while (isRun())
		{
			auto time1 = _tTime.getElapsedSecond();
			if (time1 >= 1.0) {
				// B /1024 = MB /1024 = (MB*8)/1024= Gbps/s
				double Gb = (((((double)_recvMsg * 100) / 1024) / 1024) * 8) / 1024;
				double Gb1 = (((((double)_sendCount * 100) / 1024) / 1024) * 8) / 1024;
				printf("thread<%d>,Time<%lf>, socket<%d>, client<%d>, recvCount<%d>, sendCount<%d>，recvMsg<%d>,recv:%0.2fGbps/s,send:%0.2fGbps/s\n", _CellServer_thread_connt, time1, (int)get_socketfd(), (int)_clientCount, (int)(_recvCount / time1), (int)(_sendCount / time1), (int)(_recvMsg / time1), Gb, Gb1);			
				_recvMsg = 0;
				_recvCount = 0;
				_sendCount = 0;
				_tTime.update();
			}
		}
	}
	void show_timeMsg() {
		_pThread = new std::thread(std::mem_fn(&Server::time4msg), this);
	}
	//子类实现客户端事件
	//被一个线程触发 安全
	virtual void OnNetJoin(Cell_Client* pClient) {
		_clientCount++;
	}
	//被多线程触发 不安全
	virtual void OnNetLeave()
	{
		_clientCount--;
	}
	//被多线程触发 不安全
	virtual void onNetMsg(Cell_Server* pCell_Server,Cell_Client* p_clients, DataHeader* data_header)
	{
		_recvMsg++;

	}
	virtual void onRecvCount(Cell_Client* p_clients)
	{
		_recvCount++;
	}
	virtual void onSendCount(Cell_Client* p_clients)
	{
		_sendCount++;
	}

};

#endif // ! 


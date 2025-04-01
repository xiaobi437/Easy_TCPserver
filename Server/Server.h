#ifndef SERVER_H
#define SERVER_H

#ifdef _WIN32    				//判断WIN32环境
#define FD_SETSIZE 2506			//要在头文件winsock2.h前定义
#define WIN32_LEAN_AND_MEAN		//这个宏可以尽量避免早期的宏和其他依赖库的使用
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
//启动windows网络库的两个方法
#pragma comment(lib,"ws2_32.lib")		   //这个方法在在windows下没问题,但是要跨平台就不行
//第二个方法:工程属性->链接器->输入->附件依赖项中添加 ws2_32.lib 这个库（注意,只在debug和release下添加,不要所有）


#include <winsock2.h> //这个要写在windows前面,不然会报错,如果报错就需要添加宏 #define WIN32_LEAN_AND_MEAN
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
#endif
#include <iostream>
#include <stdio.h>
#include <thread>
#include <cstring>
#include <vector>
#include <mutex>
#include <atomic>			//原子操作
#include <functional>		//mem_fun  mem_fn 所需头文件
#include "Message_Header.h"
#include "CELLTime.h"
//网络报文格式:包头+包体
//包头

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240    //缓冲区最小单元大小 10KB
#endif // !RECV_BUFF_SIZE


#ifndef _Cellserver_THREAD_COUNT     //细胞线程数量
#define _Cellserver_THREAD_COUNT 4
#endif // !Cellserver_THREAD_CONNT     //细胞线程数量

#define TEXE_SEND 1


//为每个客户端创建专属的缓冲区
class Client_socket 
{
private:
	SOCKET _sockfd;		//用于存放客户端sock
	sockaddr_in _clientaddr;
	char _szMsgBuf[RECV_BUFF_SIZE * 5]; //消息缓冲区 50KB
	int _lastPos;					  //记录消息缓冲区数据的末尾，用于下次新数据存入使用
public:
	Client_socket(sockaddr_in clientaddr,SOCKET sockfd = INVALID_SOCKET) {
		_sockfd = sockfd;
		_clientaddr = clientaddr;
		_lastPos = 0;
		memset(_szMsgBuf,0,sizeof(_szMsgBuf));
	}
	SOCKET get_socket() {
		return _sockfd;
	}
	char* get_ip() {
		return inet_ntoa(_clientaddr.sin_addr);
	}
	int get_port() {
		return ntohs(_clientaddr.sin_port);
	}
	int get_lastPos() {
		return _lastPos;
	}
	char* msgBuf(){
		return _szMsgBuf;
	}

	void set_lastPos(int pos) {
		_lastPos = pos;
	}
	//发送数据
	int sendData(DataHeader* data_header)
	{
		if (data_header != nullptr)
		{
			//向客户端发送包头数据
			int ret = send(_sockfd, (char*)data_header, data_header->dataLength, 0);
			if (ret <= 0) {
				printf("向客户端<socket:%d> 发送失败\n", (int)_sockfd);
				return SOCKET_ERROR;
			}
			return 0;
		}
		return SOCKET_ERROR;
	}
};
//网络事件接口，委托代理，客户端退出（方法不是唯一）
class INetEvent
{
public:
	//使用纯虚函数，让子类去实现
	//客户端加入事件
	virtual void OnNetJoin(Client_socket* pClient) = 0;
	//客户端退出事件
	virtual void OnNetLeave(Client_socket* pClient) = 0;
	//客户端消息事件
	virtual void onNetMsg(Client_socket* p_clients, DataHeader* data_header) = 0;
	//客户端发送消息事件
	virtual void onSendMsg(Client_socket* p_clients) = 0;

private:
};

class Cell_Server 
{
private:
	SOCKET _sock;		//服务器socket
	std::mutex _mutex;			//针对缓冲队列的锁
	std::thread *_pThread;	//线程对象
	char _szRecv[RECV_BUFF_SIZE];		//接收缓冲区

	std::vector<Client_socket*> _clients;			//正式客户端队列
	std::vector<Client_socket*> _clientsBuff;			//缓存客户端队列
	INetEvent* _pINetEvent;							//网络事件
public:
	Cell_Server(INetEvent* pINetEvent,SOCKET sock = INVALID_SOCKET)
	{
		_sock = sock;
		_pThread = nullptr;
		_pINetEvent = pINetEvent;
		memset(_szRecv, 0, sizeof(_szRecv));
	}
	~Cell_Server() 
	{
		delete _pThread;
		Close_sock();
		_sock = INVALID_SOCKET;
	}

	//接收数据 处理粘包 拆分包
	int recvData(Client_socket* p_clients) 
	{
		//接收数据存到接收缓冲区，接收大小缓冲区大小（10KB）
		int nLen = (int)recv(p_clients->get_socket(), _szRecv, RECV_BUFF_SIZE, 0);
		if (nLen <= 0) {
			//printf("接收客户端<socket:%d>消息失败，客户端已退出....\n", (int)p_clients->get_socket());
			return -1;
		}
		//把接收缓冲区的数据拷贝到消息缓冲区,接收到多少就拷贝多少
		memcpy(p_clients->msgBuf() + p_clients->get_lastPos(), _szRecv, nLen);
		//记录缓冲区数据最后的位置,当有新的数据接收的时候就可以使用这个位置继续存放数据
		p_clients->set_lastPos(p_clients->get_lastPos() + nLen);
		//判断一下消息缓存区的数据是由大于消息头,一直循环处理
		while (p_clients->get_lastPos() >= sizeof(DataHeader))
		{
			DataHeader* data_head = (DataHeader*)p_clients->msgBuf();
			//判断消息缓冲区的数据长度是否大于消息长度
			if (p_clients->get_lastPos() >= data_head->dataLength) {
				//得到第二缓冲区剩余未处理的数据长度，原本长度-处理长度=剩余长度
				int nSize = p_clients->get_lastPos() - data_head->dataLength;
				//处理网络消息
	//			printf("收到客户端<socket:%d>命令:%s, 数据长度:%d, ", client_sock, cmd_buf[data_head->cmd], data_head->dataLength);
				onNetMsg(p_clients, data_head);
				//将消息缓冲区剩余未处理数据前移
				memcpy(p_clients->msgBuf(), p_clients->msgBuf() + data_head->dataLength, nSize);
				//将第二缓冲区的数据尾部位置往前移
				p_clients->set_lastPos(nSize);
			}
			else {
				//消息缓冲区剩余数据不够完整一条信息
				break;
			}
		}
		return 0;
	}
	// 处理包头
	int onNetMsg(Client_socket* p_clients, DataHeader* data_header)
	{
		_pINetEvent->onNetMsg(p_clients, data_header);
		return 0;
	}
	//运行状态
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	//运行
	bool onRun() 
	{
		while (isRun()) {

			if (_clientsBuff.size() > 0) {
				//从缓冲队列里取出客户数据
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff) {
					_clients.push_back(pClient);
				}
				_clientsBuff.clear();
			}
			//当没有需要处理的客户端，就跳过
			if (_clients.empty()) {
				std::chrono::milliseconds t(1);
				//让线程睡眠1ms
				std::this_thread::sleep_for(t);
				continue;
			}

			//创建 伯克利socket描述符合集
			fd_set fd_Read;		//读
			//初始化,清空
			FD_ZERO(&fd_Read);
			SOCKET MAX_sock = _clients[0]->get_socket();
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				FD_SET(_clients[n]->get_socket(), &fd_Read);
				//判断最大的文件描述符
				if (MAX_sock < _clients[n]->get_socket())
				{
					MAX_sock = _clients[n]->get_socket();
				}
			}

			int ret = select(MAX_sock + 1, &fd_Read, nullptr, nullptr, nullptr);
			if (ret < 0) {
				printf("select failed!\n");
				Close_sock();
				return false;
			}
			for (int n = (int)_clients.size() - 1; n >= 0; n--) {

				if (FD_ISSET(_clients[n]->get_socket(), &fd_Read)) 
				{
					if (recvData(_clients[n]) == -1) 
					{
						//寻找位置
						auto iter = _clients.begin() + n;
						if (iter != _clients.end()) 
						{
							if (_pINetEvent)
								_pINetEvent->OnNetLeave(_clients[n]);
							//printf("<socket:%d>退出 Close\n", (int)_clients[n]->get_socket());
#ifdef _WIN32
							closesocket(_clients[n]->get_socket());
							delete _clients[n];

#else
							close(_clients[n]->get_socket());
							delete _clients[n];
#endif // _WIN32		
							_clients.erase(iter);		//从vector中删除
						}
					}
				}
			}
		}
		return true;
	}
	//关闭socket
	int Close_sock() 
	{
		if (_sock != INVALID_SOCKET) {
#ifdef _WIN32
			for (int i = 0; i <= (int)_clients.size(); i++) {
				closesocket(_clients[i]->get_socket());
				if (_clients[i] != nullptr)
					delete _clients[i];
			}
#else
			for (int i = 0; i <= (int)_clients.size(); i++) {
				close(_clients[i]->get_socket());
				if (_clients[i] != nullptr)
					delete _clients[i];
			}
#endif
		}
		_clients.clear();
		return 0;
	}

	//添加客户端socket
	int addClient(Client_socket* pClient)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_clientsBuff.push_back(pClient);
		return 0;
	}
	//获取客户端数量
	int getClinetCount() 
	{
		//返回正式客户端队列数量+未处理的客户端数量
		return (int)_clients.size() + (int)_clientsBuff.size();
	}
	void Start() 
	{
		//std::mem_fn不是std成员，需要头文件functional
		//mem_fn把类成员函数转换为函数对象（仿函数），使用对象指针和对象（引用）进行绑定。
		_pThread = new std::thread(std::mem_fn(&Cell_Server::onRun), this);
	}
};

class Server:public INetEvent
{
private:
	std::mutex _mutex1;

	SOCKET _sock;		//服务器socket
	std::vector<Cell_Server*> _cellservers;
	std::thread* _pThread;	//线程对象
protected:
	std::atomic<int> _recvCount;				//客户端接收消息计数
	std::atomic<int> _sendCount;				//客户端发送消息计数
	std::atomic<int> _clientCount;				//客户端计数
	//高精度计时器
	CELLTimestamp _tTime;
public:
	Server()//const char* ip, unsigned short port) 
	{
		_sock = INVALID_SOCKET;
		_clientCount = 0;
		_recvCount = 0;
		_sendCount = 0;
		//Init_sock(ip, port);
	}
	virtual ~Server() 
	{
		Close_sock();
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
			addClinetToCellserver(new Client_socket(clientaddr,clientsock));
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
		int ret;
		//创建 伯克利socket描述符合集
		fd_set fd_Read;		//读
		if (isRun()) {
			//time4msg();
			//初始化,清空
			FD_ZERO(&fd_Read);
			//监视是否可操作
			FD_SET(_sock, &fd_Read);

			//设置超时时间,让select变非阻塞
			timeval t = { 0,10 };
			//参数:伯克利socket（socket+1）,
			//nfds 是一个整数值,是指fd_set集合中所有描述符（socket）的范围,而不是数量
			//既是所有文件描述符中最大值+1,在windows中可以写0；
			ret = select(_sock + 1, &fd_Read, NULL, NULL, &t);
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
	int Close_sock()
	{
		if (_sock != INVALID_SOCKET) {
#ifdef _WIN32
			//6、关闭socket
			closesocket(_sock);
			//清除Windows网络环境，socket。
			WSACleanup();
#else
			close(_sock);
#endif
		}
		return 0;
	}
	//往缓冲队列添加客户端
	int addClinetToCellserver(Client_socket* pclinet)
	{
		//查询客户端最小的cell_server,消息对象
		auto pMinServer = _cellservers[0];
		for (auto pCellserver : _cellservers) {
			if (pMinServer->getClinetCount() > pCellserver->getClinetCount()) {
				pMinServer = pCellserver;
			}
		}
		pMinServer->addClient(pclinet);
		OnNetJoin(pclinet);
		return 0;
	}
	//开启线程处理消息
	void Start(int Cellserver_thread_number)
	{
		for (int n = 0; n < Cellserver_thread_number; n++) {
			//创建Cell_Server对象，传入this指针(方便使用网络事件)，sock
			auto ser = new Cell_Server(this,_sock);
			_cellservers.push_back(ser);
			//启动服务线程
			ser->Start();
		}
	}
	//计算包数，带宽
	void time4msg() {
		while(isRun())
		{
			auto time1 = _tTime.getElapsedSecond();
			if (time1 >= 1.0) {
				// B /1024 = MB /1024 = (MB*8)/1024= Gbps/s
				double Gb = (((((double)_recvCount * 100) / 1024) / 1024) * 8) / 1024;
				double Gb1 = (((((double)_sendCount * 100) / 1024) / 1024) * 8) / 1024;
				printf("thread<%d>,Time<%lf>, socket<%d>, client<%d>, recvCount<%d>, sendCount<%d> ,recv:%0.2fGbps/s,send:%0.2fGbps/s\n", _Cellserver_THREAD_COUNT, time1, (int)get_socketfd(), (int)_clientCount, (int)(_recvCount / time1), (int)(_sendCount / time1), Gb, Gb1);
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
	virtual void OnNetJoin(Client_socket* pClient) {
		_clientCount++;
	}
	//被多线程触发 不安全
	virtual void OnNetLeave(Client_socket* pClient)
	{
		_clientCount--;
	}
	//被多线程触发 不安全
	virtual void onNetMsg(Client_socket* p_clients, DataHeader* data_header)
	{
		_recvCount++;

	}
	virtual void onSendMsg(Client_socket* p_clients)
	{
		_sendCount++;
	}

};

#endif // ! 

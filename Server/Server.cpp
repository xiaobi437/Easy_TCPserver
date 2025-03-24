#define WIN32_LEAN_AND_MEAN		//这个宏可以尽量避免早期的宏和其他依赖库的使用
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h> //这个要写在windows前面,不然会报错,如果报错就需要添加宏 #define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <cstring>
#include <vector>


//启动windows网络库的两个方法
#pragma comment(lib,"ws2_32.lib")		   //这个方法在在windows下没问题，但是要跨平台就不行
//第二个方法：工程属性->链接器->输入->附件依赖项中添加 ws2_32.lib 这个库（注意，只在debug和release下添加，不要所有）

//网络报文格式：包头+包体
//包头
enum CMD {
	CMD_LOGIN,
	CMD_LOGIN_RET,
	CMD_LOGINOUT,
	CMD_LOGINOUT_RET,
	CMD_ERROR
};
struct DataHeader {
	short cmd;
	short dataLength;
};
//包体
struct Login :public DataHeader
{
	Login() {
		cmd = CMD_LOGIN;
		dataLength = sizeof(Login);
	}
	char userName[32];
	char PassWord[32];
};
struct LoginResult :public DataHeader	//登录返回数据
{
	LoginResult() {
		cmd = CMD_LOGIN_RET;
		dataLength = sizeof(LoginResult);
	}
	int result;
};
struct LoginOut :public DataHeader		//登出
{
	LoginOut() {
		cmd = CMD_LOGINOUT;
		dataLength = sizeof(LoginOut);
	}
	char userName[32];
};
struct LoginOutResult :public DataHeader	//登出返回数据
{
	LoginOutResult() {
		cmd = CMD_LOGINOUT_RET;
		dataLength = sizeof(LoginOutResult);
	}
	int result;
};

using namespace std;

vector<SOCKET> g_client;

int processor(const SOCKET client_sock) {
	int ret;
	char cmd_buf[5][20] = { "CMD_LOGIN","CMD_LOGIN_RET","CMD_LOGINOUT","CMD_LOGINOUT_RET","CMD_ERROR" };
	//5、通信接收客户端的包头
	DataHeader data_head = {};
	ret = recv(client_sock, (char*)&data_head, sizeof(DataHeader), 0);
	if (ret <= 0) {
		cout << "client is down" << endl;
		return -1;
	}
	cout << "收到命令:" << cmd_buf[data_head.cmd] << "数据长度：" << data_head.dataLength << endl;
	switch (data_head.cmd) {
		case CMD_LOGIN:			//登录成功就返回结果
		{
			Login login = {};
			//接收客户端发送的数据,因为发过来的一个包，前面已经接收一次了包头的数据，第二次接收的时候没有这么长了，
			//所以要偏移一个包头的大小去接收数据
			ret = recv(client_sock, (char*)&login + sizeof(DataHeader), sizeof(Login) - sizeof(DataHeader), 0);
			if (ret <= 0) {
				cout << "client is down" << endl;
				return -1;break;
			}
			printf("接收：name:%s,passworld:%s\n", login.userName, login.PassWord);
			if (strcmp(login.userName, "root") == 0 && strcmp(login.PassWord, "root") == 0) {
				LoginResult login_result = {};
				//cout << "登陆成功" << endl;
				login_result.result = 1;
				//向客户端发送包头数据
				ret = send(client_sock, (char*)&login_result, sizeof(LoginResult), 0);
				if (ret <= 0) {
					cout << "client is down" << endl;
					return -1; break;
				}
				break;
			}
			else {
			//	cout << "用户密码错误" << endl;
				LoginResult login_result = {};
				login_result.result = -1;
				login_result.cmd = CMD_ERROR;
				login_result.dataLength = 0;
				//向客户端发送包头数据
				ret = send(client_sock, (char*)&login_result, sizeof(LoginResult), 0);
				if (ret <= 0) {
					cout << "client is down" << endl;
					return -1; break;
				}
				break;
			}
			break;
		}
		case CMD_LOGINOUT:		//退出成功就返回结果
		{
			LoginOut loginout = {};
			//接收客户端发送的数据
			ret = recv(client_sock, (char*)&loginout + sizeof(DataHeader), sizeof(LoginOut) - sizeof(DataHeader), 0);
			if (ret <= 0) {
				cout << "client is down" << endl;
				return -1; break;
			}
			if (strcmp(loginout.userName, "root") == 0) {
				LoginOutResult loginout_result = {};
				//cout << "退出成功" << endl;
				loginout_result.result = 1;
				//向客户端发送包头数据
				ret = send(client_sock, (char*)&loginout_result, sizeof(LoginOutResult), 0);
				if (ret <= 0) {
					cout << "client is down" << endl;
					return -1; break;
				}
				break;
			}
			else {
				//cout << "退出失败，用户名错误" << endl;
				LoginOutResult loginout_result = {};
				loginout_result.result = -1;
				loginout_result.cmd = CMD_ERROR;
				loginout_result.dataLength = 0;
				//向客户端发送包头数据
				ret = send(client_sock, (char*)&loginout_result, sizeof(LoginOutResult), 0);
				if (ret <= 0) {
					cout << "client is down" << endl;
					return -1; break;
				}
				break;
			}


		}
		default:
			data_head.cmd = CMD_ERROR;
			data_head.dataLength = 0;
			ret = send(client_sock, (char*)&data_head, sizeof(DataHeader), 0);
			if (ret <= 0) {
				cout << "client is down" << endl;
				return -1; break;
			}
			break;
	}
	return 0;
}

int main(int argc, char* argv[]) {

	int ret;
	//创建版本号，是我们使用的是sock2.2，输入最好也是2.2。参数1：高位字节指定次要版本号，参数2：低位字节主要版本号
	WORD ver = MAKEWORD(2, 2);
	//指向WSADATA数据结构的指针, 该数据结构将接收Windows套接字实现的详细信息。
	WSADATA dat;
	//启动windows网络库
	if (SOCKET_ERROR == WSAStartup(ver, &dat)) {  //启动这个库需要链接静态库，有两个办法
		cout << "WSAStartup failed" << endl;
		return -1;
	}

	//1、socket 创建套接字，IPV4，面向数据流，TCP协议
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_sock == SOCKET_ERROR) {
		cout << "socket failed" << endl;
		return -1;
	}
	//2、bind 绑定用于客户端连接的端口IP
	//定义IPV4地址结构体，用于存放要绑定的IP，端口号
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;		//IPV4协议
	_sin.sin_port = htons(10001);	//端口号
	_sin.sin_addr.S_un.S_addr = inet_addr("192.168.31.240");  //ip

	//因为sockaddr这个方式不利用与我们填写，所以要使用相近的类型
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin))) {
		cout << "bind failed" << endl;
		return -1;
	}else {
		cout << "The network 、port is bound successfully...." << endl;
	}

	//3、listen 监听网络端口,最大监听数5
	if (listen(_sock, 5) == SOCKET_ERROR) {
		cout << "listen failed" << endl;
		return -1;
	}else {
		cout << "Listening to the network port succeeded...." << endl;
	}
	
	//创建 伯克利socket描述符合集
	fd_set fd_Read;		//读
	fd_set fd_Write;	//写
	fd_set fd_Excep;	//异常
	//初始化，清空
	FD_ZERO(&fd_Read);
	FD_ZERO(&fd_Write);
	FD_ZERO(&fd_Excep);

	//创建vector指针，指向g_client开头
	vector<SOCKET>::iterator it = g_client.begin();

	while (1) {

		//监视是否可操作
		FD_SET(_sock, &fd_Read);
		FD_SET(_sock, &fd_Write);
		FD_SET(_sock, &fd_Excep);

		for (int i = 0; i < (int)g_client.size(); i++) {
			FD_SET(g_client[i], &fd_Read);
		}
		//设置超时时间，让select变非阻塞
		timeval t = { 0,0 };
		//参数：伯克利socket（socket+1），
		//nfds 是一个整数值，是指fd_set集合中所有描述符（socket）的范围，而不是数量
		//既是所有文件描述符最大值+1，在windows中可以写0；
		ret = select(_sock + 1, &fd_Read, &fd_Write, &fd_Excep, &t);
		if (ret < 0){
			printf("select failed!\n");
			break;
		}
		//判断旧的套接字是否发生了读就绪-->说明有客户端连接服务器
		if (FD_ISSET(_sock, &fd_Read)) {
			//清理一下标志位
			FD_CLR(_sock, &fd_Read);

			//4、accept 等待接受客户端链接
			sockaddr_in _clientaddr = {};
			int _clientaddr_len = sizeof(_clientaddr);
			SOCKET _clientsock = INVALID_SOCKET;
			_clientsock = accept(_sock, (sockaddr*)&_clientaddr, &_clientaddr_len);
			if (_clientsock == SOCKET_ERROR) {
				cout << "accept failed" << endl;
				return -1;
			}
			printf("New client accept:IP = %s, Port:%u ，socket：%d\n", inet_ntoa(_clientaddr.sin_addr), ntohs(_sin.sin_port),_clientsock);//inet_ntoa,把网络字节序的IP转字符串IP
			//把新客户端的socket插进vector中
			g_client.push_back(_clientsock);
		}
		for (int i = 0; i <= (int)fd_Read.fd_count && (int)fd_Read.fd_count != 0; i++) {
			if (processor(fd_Read.fd_array[i]) == -1) {
				//寻找位置
				it = find(g_client.begin(), g_client.end(), fd_Read.fd_array[i]);
				if (it != g_client.end()) {
					printf("socket：%d退出\n", *it);
					//删除
					g_client.erase(it);		//从vector中删除
					FD_CLR(fd_Read.fd_array[i], &fd_Read);		//从合集中删除
				}
			}
		}	
		//cout << "空余时间，处理其他业务" << endl;


	}

	//6、关闭socket
	closesocket(_sock);
	for (int i = 0; i <= (int)g_client.size(); i++) {
		closesocket(g_client[i]);
	}
	

	//关闭windows网络库
	WSACleanup();

	return 0;
}

#define WIN32_LEAN_AND_MEAN		//这个宏可以尽量避免早期的宏和其他依赖库的使用
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h> //这个要写在windows前面,不然会报错,如果报错就需要添加宏 #define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>

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

int processor(const SOCKET _sock) {
	
	int ret;
	char cmd_buf[5][20] = { "CMD_LOGIN","CMD_LOGIN_RET","CMD_LOGINOUT","CMD_LOGINOUT_RET","CMD_ERROR" };

	DataHeader data_head = {};
	ret = recv(_sock, (char*)&data_head, sizeof(DataHeader), 0);
	if (ret <= 0) {
		cout << "server is down" << endl;
		return -1;
	}
	switch (data_head.cmd) {
	case CMD_LOGIN:
	{
		break;
	}
	case CMD_LOGIN_RET:
	{
		//接收服务器发送的包头数据
		LoginResult login_result = {};
		ret = recv(_sock, (char*)&login_result + sizeof(DataHeader), sizeof(LoginResult) - sizeof(DataHeader), 0);
		if (ret <= 0) {
			cout << "server is down" << endl;
			break;
		}
		printf("收到命令：%s，数据长度：%d，登录结果：%d\n", cmd_buf[login_result.cmd], login_result.dataLength, login_result.result);
	}
	case CMD_LOGINOUT:
	{
		break;
	}
	case CMD_LOGINOUT_RET:
	{
		//接收服务器发送的包头数据
		LoginOutResult loginout_result = {};
		ret = recv(_sock, (char*)&loginout_result + sizeof(DataHeader), sizeof(LoginOutResult) - sizeof(DataHeader), 0);
		if (ret <= 0) {
			cout << "client is down" << endl;
			break;
		}
		printf("收到命令：%s，数据长度：%d，登录结果：%d\n", cmd_buf[loginout_result.cmd], loginout_result.dataLength, loginout_result.result);
		break;
	}
	default:
		break;
	}
	return 0;
}
int main(int argc, char* argv[]) {

	int ret;
	char cmd_buf[5][20] = { "CMD_LOGIN","CMD_LOGIN_RET","CMD_LOGINOUT","CMD_LOGINOUT_RET","CMD_ERROR" };
	//创建版本号，是我们使用的是sock2.2，输入最好也是2.2。参数1：高位字节指定次要版本号，参数2：低位字节主要版本号
	WORD ver = MAKEWORD(2, 2);
	//指向WSADATA数据结构的指针, 该数据结构将接收Windows套接字实现的详细信息。
	WSADATA dat;
	//启动windows网络库
	if (SOCKET_ERROR == WSAStartup(ver, &dat)) {		//启动这个库需要链接静态库，有两个办法
		cout << "WSAStartup failed" << endl;
		return -1;
	}

	//1、socket 创建套接字，IPV4，面向数据流，TCP协议
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_sock == SOCKET_ERROR) {
		cout << "socket failed" << endl;
		return -1;
	}

	//3、connect 连结服务端
	//定义IPV4地址结构体，用于存放服务端的IP，端口号
	sockaddr_in _sin_server = {};
	_sin_server.sin_family = AF_INET;		//IPV4协议
	_sin_server.sin_port = htons(10001);	//端口号
	_sin_server.sin_addr.S_un.S_addr = inet_addr("192.168.31.240");  //ip

	if (connect(_sock, (sockaddr*)&_sin_server, sizeof(_sin_server)) == SOCKET_ERROR) {
		cout << "connect failed" << endl;
		return -1;
	}else {
		cout << "Connecting to the server succeeded...." << endl;
	}

	char buf[128] = {};
	//创建 伯克利socket描述符合集
	fd_set fd_Read;		//读
	fd_set fd_Write;	//写
	fd_set fd_Excep;	//异常
	//初始化，清空
	FD_ZERO(&fd_Read);
	FD_ZERO(&fd_Write);
	FD_ZERO(&fd_Excep);

	while (1) {
		//监视是否可操作
		FD_SET(_sock, &fd_Read);		//读
		FD_SET(_sock, &fd_Write);
		FD_SET(_sock, &fd_Excep);

		//设置超时时间，让select变非阻塞
		timeval t = { 0,0 };
		//参数：伯克利socket（socket+1），
		//nfds 是一个整数值，是指fd_set集合中所有描述符（socket）的范围，而不是数量
		//既是所有文件描述符最大值+1，在windows中可以写0；
		ret = select(_sock + 1, &fd_Read, NULL, NULL, &t);
		if (ret < 0) {
			printf("select failed!\n");
			break;
		}
		//套接字是否发生了读就绪-->说明有服务端有消息
		if (FD_ISSET(_sock, &fd_Read)) {
			FD_CLR(_sock, &fd_Read);
			if (processor(_sock) == -1) {
				cout << "与服务器断开连接" << endl;
				break;
			}
		}
		//cout << "空余时间，处理其他业务" << endl;
		int a=rand() % 2;
		switch(a)
		{
			case 1: 
			{
				Login login = {};
				strcpy(login.userName, "root");
				strcpy(login.PassWord, "root");

				//向服务器发送数据
				ret = send(_sock, (char*)&login, sizeof(Login), 0);	//数据
				if (ret <= 0) {
					cout << "server is down" << endl;
					break;
				}
				break;
			}
			case 2:
			{
				LoginOut loginout = {};
				strcpy(loginout.userName, "root");

				//向服务器发送数据
				ret = send(_sock, (char*)&loginout, sizeof(LoginOut), 0);
				if (ret <= 0) {
					cout << "server is down" << endl;
					break;
				}
				break;
			}
			default:
				break;
		}
		Sleep(1);
	}
	
	//5、关闭socket
	closesocket(_sock);

	//关闭windows网络库
	WSACleanup();

	return 0;
}

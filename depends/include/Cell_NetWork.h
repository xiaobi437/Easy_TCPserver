/*
* 网络环境的启动与关闭
* 
*/
#ifndef CELL_NET_WORK_H
#define CELL_NET_WORK_H

#include "Public_Include.h"
#include "Cell_Log.h"

class Cell_NetWork
{
public:
	static void Init()
	{
		static Cell_NetWork obj;
	}
private:
	Cell_NetWork()
	{
#ifdef _WIN32
		//创建版本号,是我们使用的是sock2.2,输入最好也是2.2。参数1:高位字节指定次要版本号,参数2:低位字节主要版本号
		WORD ver = MAKEWORD(2, 2);
		//指向WSADATA数据结构的指针, 该数据结构将接收Windows套接字实现的详细信息。
		WSADATA dat;
		//启动windows网络库
		if (SOCKET_ERROR == WSAStartup(ver, &dat)) {  //启动这个库需要链接静态库,有两个办法
			Cell_Log::Info(Error_Msg, "Cell_NetWork::WSAStartup failed\n");
		}
#endif

#ifndef _WIN32	//非WIN32环境
		//if(signal(SIGPIPE,SIG_IGN) == SIG_ERR)
		//return (1);
		signal(SIGPIPE, SIG_IGN);
#endif
	}
	~Cell_NetWork() 
	{
#ifdef _WIN32
		//清除Windows网络环境，socket。
		WSACleanup();
#endif
	}

private:

};

#endif // !CELL_NET_WORK_H


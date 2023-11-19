/*
*存放公共头文件、声明
*
*/
#ifndef PUBLIC_INCLUDE_H
#define PUBLIC_INCLUDE_H

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
#include <signal.h>

#include "Memory_Pool.h"		//内存池
#include "Object_Pool.h"		//对象池


class Cell_Server;

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 8192    //接收缓冲区最小单元大小 10KB
#endif // !RECV_BUFF_SIZE

#ifndef SEND_BUFF_SIZE
#define SEND_BUFF_SIZE 10240	//发送缓冲区最小单元大小 50KB
#endif // !RECV_BUFF_SIZE


#endif // !PUBLIC_INCLUDE_H


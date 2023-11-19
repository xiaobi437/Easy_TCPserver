#include "client.h"

SOCKET Client::Init_sock()
{
	Cell_NetWork::Init();
	if (_sock != INVALID_SOCKET)
	{
		Close();
	}
	// 创建socket套接字
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_sock == SOCKET_ERROR)
	{
		Cell_Log::Info(Error_Msg,"socket failed!\n");
		return -1;
	}
	return 0;
}

int Client::Connect(const char *ip, unsigned short port)
{
	int ret;
	if (_sock == INVALID_SOCKET)
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
	ret = connect(_sock, (sockaddr*)&sin_server, sizeof(sin_server));
	if (ret == SOCKET_ERROR)
	{
		Cell_Log::Info(Error_Msg, "connect<%s:%d> failed!\n", ip, port);
		return -1;
	}
	return ret;
}

int Client::Close_sock()
{
	if (_sock != INVALID_SOCKET)
	{
#ifdef _WIN32
		closesocket(_sock);
#else
		close(_sock);
#endif // _WIN32
		_sock = INVALID_SOCKET;
	}
	return 0;
}

// 接收数据 处理粘包 拆分包
int Client::RecvData(SOCKET cSock)
{
	// 直接把数据拷贝到客户端的缓冲区，就免去了拷贝的步骤
	char *szRecv = _szMsgBuf + _lastPos;
	// 接收数据存到接收缓冲区，接收大小缓冲区大小（50KB）
	int nLen = recv(cSock, szRecv, (RECV_BUFF_SIZE * 5) - _lastPos, 0);
	if (nLen <= 0)
	{
		printf("接收服务器<socket:%d>消息失败....\n", (int)cSock);
		return -1;
	}
	// 把接收缓冲区的数据拷贝到消息缓冲区,接收到多少就拷贝多少
	// memcpy(_szMsgBuf + _lastPos, _szRecv, nLen);
	// 记录缓冲区数据最后的位置,当有新的数据接收的时候就可以使用这个位置继续存放数据
	_lastPos += nLen;
	// 判断一下消息缓存区的数据是由大于消息头,一直循环处理
	while (_lastPos >= sizeof(DataHeader))
	{
		DataHeader *data_head = (DataHeader *)_szMsgBuf;
		// 判断消息缓冲区的数据长度是否大于消息长度
		if (_lastPos >= data_head->dataLength)
		{
			// 得到第二缓冲区剩余未处理的数据长度，原本长度-处理长度=剩余长度
			int nSize = _lastPos - data_head->dataLength;
			// 处理网络消息
			onNetMsg(data_head);
			// 将消息缓冲区剩余未处理数据前移
			memcpy(_szMsgBuf, _szMsgBuf + data_head->dataLength, nSize);
			// 将第二缓冲区的数据尾部位置往前移
			_lastPos = nSize;
		}
		else
		{
			// 消息缓冲区剩余数据不够完整一条信息
			break;
		}
	}
	return 0;
}

int Client::onNetMsg(DataHeader *data_head)
{
	char cmd_buf[6][20] = {"CMD_LOGIN", "CMD_LOGIN_RET", "CMD_LOGINOUT", "CMD_LOGINOUT_RET", "CMD_NEW_LOGIN", "CMD_ERROR"};
	switch (data_head->cmd)
	{
	case CMD_LOGIN:
	{
		break;
	}
	case CMD_LOGIN_RET:
	{
		LoginResult *login_result = (LoginResult *)data_head;
		//		printf("收到服务器<socket:%d>命令:%s, 数据长度:%d, 结果:%d\n", _sock, cmd_buf[data_head->cmd], data_head->dataLength, login_result->result);
		break;
	}
	case CMD_LOGINOUT:
	{
		break;
	}
	case CMD_LOGINOUT_RET:
	{
		LoginOutResult *loginout_result = (LoginOutResult *)data_head;
		printf("收到服务器<socket:%d>命令:%s, 数据长度:%d, 结果:%d\n", (int)_sock, cmd_buf[data_head->cmd], data_head->dataLength, loginout_result->result);
		break;
	}
	case CMD_NEW_LOGIN:
	{
		NewLogin *new_login = (NewLogin *)data_head;
		// printf("收到服务器<socket:%d>命令:%s, 数据长度:%d, 消息:", _sock, cmd_buf[data_head->cmd], data_head->dataLength);
		// printf("新客户端连接<socket:%d，%s:%d>\n", new_login->sock, new_login->IP, new_login->Port);
		break;
	}
	case CMD_ERROR:
	{
		printf("收到服务器<socket:%d>命令:%s, 数据长度:%d\n", (int)_sock, cmd_buf[data_head->cmd], data_head->dataLength);
		break;
	}
	default:
		printf("收到服务器<socket:%d>未知数据, 数据长度:%d\n", (int)_sock, data_head->dataLength);
		break;
	}
	return 0;
}

int Client::SendData(DataHeader *data_head, int nLen)
{
	int ret;
	if (isRun() && data_head != nullptr)
	{
		// 向服务器发送数据
		ret = send(_sock, (char *)data_head, nLen, 0); // 数据
		if (ret <= 0)
		{
			printf("向服务器<socket:%d>发送消息失败....\n", (int)_sock);
			return ret;
		}
	}

	return ret;
}

bool Client::onRun()
{
	if (isRun())
	{
		int ret;
		// 创建 伯克利socket描述符合集
		fd_set fd_Read; // 读
		// 初始化,清空
		FD_ZERO(&fd_Read);
		// 监视是否可操作
		FD_SET(_sock, &fd_Read); // 读

		// 设置超时时间,让select变非阻塞
		timeval t = {0, 0};
		// 参数:伯克利socket（socket+1）,
		// nfds 是一个整数值,是指fd_set集合中所有描述符（socket）的范围,而不是数量
		// 既是所有文件描述符最大值+1,在windows中可以写0；
		ret = select(_sock + 1, &fd_Read, NULL, NULL, &t);
		if (ret < 0)
		{
			printf("select<socket:%d> failed!\n", (int)_sock);
			Close_sock();
			return false;
		}
		// 套接字是否发生了读就绪-->说明有服务端有消息
		if (FD_ISSET(_sock, &fd_Read))
		{
			if (RecvData(_sock) == -1)
			{
				printf("服务器退出，任务结束\n");
				Close_sock();
				return false;
			}
		}
	}

	return true;
}

bool Client::isRun()
{
	if (_sock != INVALID_SOCKET)
	{
		return true;
	}
	else
	{
		return false;
	}
}

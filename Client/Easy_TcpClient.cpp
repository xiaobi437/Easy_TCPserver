#include "client.h"


std::atomic<int> readyCount;
std::atomic<int> sendCount;
std::atomic<int> recvCount;
const int tCount = 4; // 线程数量
const int cCount = 100; // 客户端数量
bool flag = true;	  // 结束标志位

class MyClient:public Client
{
public:
	//处理包头
	virtual void onNetMsg(DataHeader* data_head)
	{
		char cmd_buf[6][20] = { "CMD_LOGIN", "CMD_LOGIN_RET", "CMD_LOGINOUT", "CMD_LOGINOUT_RET", "CMD_NEW_LOGIN", "CMD_ERROR" };
		switch (data_head->cmd)
		{
		case CMD_LOGIN:
		{
			break;
		}
		case CMD_LOGIN_RET:
		{
			LoginResult* login_result = (LoginResult*)data_head;
			//Cell_Log::Info(Info_Msg,"收到服务器<socket:%d>命令:%s, 数据长度:%d, 结果:%d\n", (int)_pClient->sockfd(), cmd_buf[data_head->cmd], data_head->dataLength, login_result->result);
			break;
		}
		case CMD_LOGINOUT:
		{
			break;
		}
		case CMD_LOGINOUT_RET:
		{
			LoginOutResult* loginout_result = (LoginOutResult*)data_head;
			//Cell_Log::Info(Info_Msg, "收到服务器<socket:%d>命令:%s, 数据长度:%d, 结果:%d\n", (int)_pClient->sockfd(), cmd_buf[data_head->cmd], data_head->dataLength, loginout_result->result);
			break;
		}
		case CMD_NEW_LOGIN:
		{
			NewLogin* new_login = (NewLogin*)data_head;
			//Cell_Log::Info(Info_Msg,"收到服务器<socket:%d>命令:%s, 数据长度:%d, 消息:", _pClient->sockfd(), cmd_buf[data_head->cmd], data_head->dataLength);
			//Cell_Log::Info(Info_Msg,"新客户端连接<socket:%d，%s:%d>\n", new_login->sock, new_login->IP, new_login->Port);
			break;
		}
		case CMD_ERROR:
		{
			//Cell_Log::Info(Error_Msg, "收到服务器<socket:%d>命令:%s, 数据长度:%d\n", (int)_pClient->sockfd(), cmd_buf[data_head->cmd], data_head->dataLength);
			break;
		}
		default:
			Cell_Log::Info(Error_Msg, "收到服务器<socket:%d>未知数据, 数据长度:%d\n", (int)_pClient->sockfd(), data_head->dataLength);
			break;
		}
	}
};

int cmd_flag()
{
	char buf[128] = {};
	while (true)
	{
		memset(buf, 0, sizeof(buf));
		std::cin >> buf;
		if (strcmp(buf, "exit") == 0)
		{
			flag = false;
			break;
		}
		else
		{
			printf("order error!\n");
			continue;
		}
	}
	printf("cmd_flag() Exit\n");
	return 0;
}

void recv_Thread(Client* clients[], int begin, int end, int id)
{
	printf("thread_recv() %d <begin=%d,end=%d> start\n", id, begin, end);
	CELLTimestamp t;
	while (flag)
	{

		for (int n = begin; n < end; n++)
		{
			if (t.getElapsedSecond() > 3.0 && n == begin)
				continue;
			if (!clients[n]->onRun())
			{
				flag = false;
				printf("recv_Thread() %d onRun failed\n", id);
			}
			recvCount++;
		}
	}
	printf("recv_Thread() %d Exit\n", id);
}
void send_Thread(Client* clients[], const int cCount, int id)
{
	int cnumber = cCount / tCount;
	int begin = (id - 1) * cnumber; // 从零开始
	int end = id * cnumber;

	for (int n = begin; n < end; n++)
	{
		clients[n] = new MyClient();
	}

	for (int n = begin; n < end; n++)
	{
#if 1
		clients[n]->Connect("192.168.31.240", 10000);
#else
		clients[n]->Connect("192.168.31.241", 10001);
#endif
	}
	printf("thread_send() %d  Connect<begin=%d,end=%d>\n", id, begin, end);
	readyCount++;
	while (readyCount < tCount)
	{
		std::chrono::milliseconds t(10);
		std::this_thread::sleep_for(t);
	}
	// 启动接收线程
	std::thread t1(recv_Thread, clients, begin, end, id);
	//t1.detach();
/*
	Login login[1] = {};
	for (int n = 0; n < 1; n++)
	{
		strcpy(login[n].userName, "root");
		strcpy(login[n].PassWord, "root");
	}
	const int nLen = sizeof(login);
*/
	
	while (flag)
	{

		for (int n = begin; n < end; n++)
		{
			Login* login = new Login;
			strcpy(login->userName, "root");
			strcpy(login->PassWord, "root");
			if (clients[n]->SendData(login, login->dataLength) == -1)
			{
				//flag = false;
				//printf("send_Thread() %d onRun failed\n", id);
				continue;
			}
			sendCount++;
		}
	}
	t1.join();
	for (int n = begin; n < end; n++)
	{
		delete clients[n];
	}
	printf("send_Thread() %d Exit\n", id);
}

int main(int argc, char* argv[])
{
	char* log_path = new char[256];
	char* log_path1 = new char[256];
	//拼接字符串
#ifdef _WIN32
	sprintf(log_path, "C:\\Users\\Xiaob\\Desktop\\Server_Log\\Client_Log_%s.txt", Cell_Log::Instance().get_Now_Date());
	sprintf(log_path1, "C:\\Users\\Xiaob\\Desktop\\Server_Log\\Client_Error_Log_%s.txt", Cell_Log::Instance().get_Now_Date());
#else
	sprintf(log_path, "./Client_Log_%s.txt", Cell_Log::Instance().get_Now_Date());
	sprintf(log_path1, "./Client_Error_Log_%s.txt", Cell_Log::Instance().get_Now_Date());
#endif // _WIN32
	Cell_Log::Instance().setLogPath(log_path, "a+");
	Cell_Log::Instance().setErrorLogPath(log_path1, "a+");
	delete[] log_path;
	delete[] log_path1;

	Client* clients[cCount];
	//std::vector<Client*> pClients;
	readyCount = 0;
	sendCount = 0;
	recvCount = 0;

	std::chrono::seconds time_s_1(1);

	// 启动发送线程
	for (int i = 0; i < tCount; i++)
	{
		std::thread t1(send_Thread, clients,cCount, i + 1);
		t1.detach();
	}

	// 启动UI线程
	std::thread t1(cmd_flag);
	t1.detach();

	// 计时器
	CELLTimestamp tTime;
	while (readyCount < tCount)
	{
		std::this_thread::sleep_for(time_s_1);
	}
	while (flag)
	{
		auto t = tTime.getElapsedSecond();
		if (t >= 1.0)
		{
			printf("thread<%d>,client<%d>,Time<%lf>, sendCount<%d>, recvCount<%d>\n", tCount, cCount, t, (int)(sendCount / t), (int)(recvCount / t));
			tTime.update();
			sendCount = 0;
			recvCount = 0;
		}

		std::this_thread::sleep_for(time_s_1);
	}

	printf("服务器退出任务结束\n");
	std::this_thread::sleep_for(time_s_1);
	return 0;
}
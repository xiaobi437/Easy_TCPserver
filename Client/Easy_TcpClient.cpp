#include "client.h"
#include"CELLStream.hpp"
#include "CELLMsgStream.hpp"
#include "Cell_Thread.h"


std::atomic<int> readyCount;
std::atomic<int> sendCount;
std::atomic<int> recvCount;
const int tCount = 4; // 线程数量
const int cCount = 1000; // 客户端数量
bool flag = true;	  // 结束标志位
#if 1
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
			CELLReadStream r(data_head);		//传入消息的大小，120
			//读取消息长度
			auto n1 = r.ReadInt16();			//读取长度2
			//读取消息命令
			auto n2 = r.getNetCmd();			//读取长度2
			//读取消息结果
			int32_t result = -1;
			auto n3 = r.ReadInt32(result);
			//读取消息附带数据
			char readta[92] = {};
			auto n4 = r.ReadArray(readta, 93);
			//Cell_Log::Info(Info_Msg,"收到服务器<socket:%d>命令:%s, 数据长度:%d, 结果:%d\n", (int)_pClient->sockfd(), cmd_buf[n2], n1, n3);
			break;
		}
		case CMD_LOGINOUT:
		{
			break;
		}
		case CMD_LOGINOUT_RET:
		{
			
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
#endif


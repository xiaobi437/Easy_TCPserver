#include "Server.h"
#include "CELLMsgStream.hpp"


#if 1
int main(int argc, char* argv[])
{
	char* log_path = new char[256];
	char* log_path1 = new char[256];
	//拼接字符串
#ifdef _WIN32
	sprintf(log_path, "C:\\Users\\Xiaob\\Desktop\\Server_Log\\Serve_Log_%s.txt", Cell_Log::Instance().get_Now_Date());
	sprintf(log_path1, "C:\\Users\\Xiaob\\Desktop\\Server_Log\\Serve_Error_Log_%s.txt", Cell_Log::Instance().get_Now_Date());
#else
	sprintf(log_path, "./Serve_Log_%s.txt", Cell_Log::Instance().get_Now_Date());
	sprintf(log_path1, "./Serve_Error_Log_%s.txt", Cell_Log::Instance().get_Now_Date());
#endif // _WIN32
	Cell_Log::Instance().setLogPath(log_path,"a+");
	Cell_Log::Instance().setErrorLogPath(log_path1, "a+");
	delete[] log_path;
	delete[] log_path1;

#ifdef _WIN32
	MyServer serverA;
	serverA.Init_sock("2.2.2.200", 10000);
	serverA.Listen(5);
	serverA.Start(4);
#else
	MyServer serverA();
	serverA.Init_sock("2.2.2.201", 10001);
	serverA.Listen(5);
	serverA.Start(4);
#endif // _WIN32


	//启动UI线程
	//std::thread t1(cmdThread, &g_bRun);
	//t1.detach();
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			printf("退出cmdThread线程\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "client"))
		{
			printf("总客户端数量%d\n", serverA.get_Client_number());
			continue;
		}
		else {
			printf("不支持的命令。\n");
		}

	}
	serverA.Close();
	printf("服务器已退出。\n");

	Cell_Thread::Sleep(2000);

	return 0;
}
#endif
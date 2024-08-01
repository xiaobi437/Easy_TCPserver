#include "Server.h"
#include "CELLMsgStream.hpp"

class MyServer :virtual public Server
{
public:
	//被多线程触发 不安全
	virtual void onNetMsg(Cell_Server *pCell_Server,Cell_Client* p_clients, DataHeader* data_header)
	{
		Server::onNetMsg(pCell_Server,p_clients, data_header);
		switch (data_header->cmd) {
		case CMD_LOGIN:			//登录成功就返回结果
		{
			p_clients->resetDtHeart();
			CELLReadStream r(data_header);		//传入消息的大小，120
			//读取消息长度
			auto n1 = r.ReadInt16();			//读取长度2
			//读取消息命令
			auto n2 = r.getNetCmd();			//读取长度2
			char name[32] = {};		//读取用户名  读取长度4+32
			auto n3 = r.ReadArray(name, 33);
			char pw[32] = {};		//读取密码		读取长度4+32
			auto n4 = r.ReadArray(pw, 33);
			int ata[10] = {};		//读取附带数据	读取长度4*10
			auto n5 = r.ReadArray(ata, 11);

			//printf("收到客户端<socket:%d>数据, %d:%s:%s\n", (int)p_clients->sockfd(), n2, name, pw);
			int32_t result = -1;
			if (strcmp(name, "root") == 0 && strcmp(pw, "root") == 0)
			{
				result = 1;
			}

#if TEXE_SEND
			CELLWriteStream s(128);
			s.setNetCmd(CMD_LOGIN_RET);
			s.WriteInt32(result);
			char readta[92] = {};
			s.WriteArray(readta,92);
			s.finsh();
			p_clients->sendData(s.data(), s.length());
			Server::onSendCount(p_clients);
#endif
			break;
		}
		case CMD_LOGINOUT:		//退出成功就返回结果
		{
			p_clients->resetDtHeart();
			CELLReadStream r(data_header);
			//读取消息长度
			r.ReadInt16();
			//读取消息命令
			r.getNetCmd();
			auto n1 = r.ReadInt8();
			auto n2 = r.ReadInt16();
			auto n3 = r.ReadInt32();
			auto n4 = r.ReadFloat();
			auto n5 = r.ReadDouble();
			uint32_t n = 0;
			r.onlyRead(n);
			char name[32] = {};
			auto n6 = r.ReadArray(name, 33);
			char pw[32] = {};
			auto n7 = r.ReadArray(pw, 33);
			int ata[10] = {};
			auto n8 = r.ReadArray(ata, 11);
			printf("收到客户端<socket:%d>数据, %s:%s\n",  name, pw);
#if TEXE_SEND
			CELLWriteStream s(128);
			s.setNetCmd(CMD_LOGINOUT_RET);
			s.WriteInt8(n1);
			s.WriteInt16(n2);
			s.WriteInt32(n3);
			s.WriteFloat(n4);
			s.WriteDouble(n5);
			s.WriteArray(name, n6);
			s.WriteArray(pw, n7);
			s.WriteArray(ata, n8);
			s.finsh();
			p_clients->sendData(s.data(), s.length());
			Server::onSendCount(p_clients);
#endif
			break;
		}
		case CMD_HEART_C2S:
		{
			p_clients->resetDtHeart();		//重置心跳
			Heart_S2C* heart_S2C = new Heart_S2C();
			if (SOCKET_ERROR == p_clients->sendData(heart_S2C))
			{
				Cell_Log::Instance().Info(Warning_Msg, "sockte<%d> send full\n", p_clients->sockfd());
				//发送缓冲区满了，消息没发出去。（处理方式看业务）
			}
			//pCell_Server->addSendTask(p_clients, (DataHeader*)heart_S2C);
			Server::onSendCount(p_clients);
			break;
		}
		default:
		{
			printf("收到客户端<socket:%d>未知数据, 数据长度:%d\n", (int)p_clients->sockfd(), data_header->dataLength);
			//DataHeader data_head = {};
			//sendData(client_sock, &data_head);
			break;
		}
		}

	}
};
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
	serverA.Init_sock("192.168.31.241", 10001);
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
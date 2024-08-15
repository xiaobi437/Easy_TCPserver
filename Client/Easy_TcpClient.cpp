#include "client.h"
#include"CELLStream.hpp"
#include "CELLMsgStream.hpp"
#include "Cell_Thread.h"


std::atomic<int> readyCount;
std::atomic<int> sendCount;
std::atomic<int> recvCount;
const int tCount = 4; // �߳�����
const int cCount = 1000; // �ͻ�������
bool flag = true;	  // ������־λ
#if 1
class MyClient:public Client
{
public:
	//�����ͷ
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
			CELLReadStream r(data_head);		//������Ϣ�Ĵ�С��120
			//��ȡ��Ϣ����
			auto n1 = r.ReadInt16();			//��ȡ����2
			//��ȡ��Ϣ����
			auto n2 = r.getNetCmd();			//��ȡ����2
			//��ȡ��Ϣ���
			int32_t result = -1;
			auto n3 = r.ReadInt32(result);
			//��ȡ��Ϣ��������
			char readta[92] = {};
			auto n4 = r.ReadArray(readta, 93);
			//Cell_Log::Info(Info_Msg,"�յ�������<socket:%d>����:%s, ���ݳ���:%d, ���:%d\n", (int)_pClient->sockfd(), cmd_buf[n2], n1, n3);
			break;
		}
		case CMD_LOGINOUT:
		{
			break;
		}
		case CMD_LOGINOUT_RET:
		{
			
			//Cell_Log::Info(Info_Msg, "�յ�������<socket:%d>����:%s, ���ݳ���:%d, ���:%d\n", (int)_pClient->sockfd(), cmd_buf[data_head->cmd], data_head->dataLength, loginout_result->result);
			break;
		}
		case CMD_NEW_LOGIN:
		{
			NewLogin* new_login = (NewLogin*)data_head;
			//Cell_Log::Info(Info_Msg,"�յ�������<socket:%d>����:%s, ���ݳ���:%d, ��Ϣ:", _pClient->sockfd(), cmd_buf[data_head->cmd], data_head->dataLength);
			//Cell_Log::Info(Info_Msg,"�¿ͻ�������<socket:%d��%s:%d>\n", new_login->sock, new_login->IP, new_login->Port);
			break;
		}
		case CMD_ERROR:
		{
			//Cell_Log::Info(Error_Msg, "�յ�������<socket:%d>����:%s, ���ݳ���:%d\n", (int)_pClient->sockfd(), cmd_buf[data_head->cmd], data_head->dataLength);
			break;
		}
		default:
			Cell_Log::Info(Error_Msg, "�յ�������<socket:%d>δ֪����, ���ݳ���:%d\n", (int)_pClient->sockfd(), data_head->dataLength);
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
	int begin = (id - 1) * cnumber; // ���㿪ʼ
	int end = id * cnumber;

	for (int n = begin; n < end; n++)
	{
		clients[n] = new MyClient();
	}

	for (int n = begin; n < end; n++)
	{
#if 1
		clients[n]->Connect("2.2.2.200", 10000);
#else
		clients[n]->Connect("2.2.2.201", 10001);
#endif
	}
	printf("thread_send() %d  Connect<begin=%d,end=%d>\n", id, begin, end);
	readyCount++;
	while (readyCount < tCount)
	{
		Cell_Thread::Sleep(1000);
	}
	// ���������߳�
	std::thread t1(recv_Thread, clients, begin, end, id);
	//t1.detach();

	
	while (flag)
	{
		for (int n = begin; n < end; n++)
		{
			CELLWriteStream s(128);				//ռ2���ֽ�
			s.setNetCmd(CMD_LOGIN);				//ռ2���ֽ�
			char name[] = "root";		//д���û���
			char pw[] = "root";		//д������
			int ata[10] = { 0,2,3,4,5,6,9,999,60 };		//д�븽������
			auto a1 = s.WriteArray(name, 32);		//ռ4+32=36
			auto a2 = s.WriteArray(pw, 32);			//ռ4+32=36
			auto a3 = s.WriteArray(ata, 10);		//ռ4+4*10=44
			s.finsh();
			if (clients[n]->SendData(s.data(), s.length()) == -1)
			{
				//flag = false;
				//printf("send_Thread() %d onRun failed\n", id);
				continue;
			}
			//Cell_Thread::Sleep(1);
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
	//ƴ���ַ���
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

	// ���������߳�
	for (int i = 0; i < tCount; i++)
	{
		std::thread t1(send_Thread, clients,cCount, i + 1);
		t1.detach();
	}

	// ����UI�߳�
	std::thread t1(cmd_flag);
	t1.detach();

	// ��ʱ��
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

	printf("�������˳��������\n");
	std::this_thread::sleep_for(time_s_1);
	return 0;
}
#endif


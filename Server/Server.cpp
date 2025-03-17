#include "server.h"
#include "CELLTime.h"

class MyServer :virtual public Server
{
public:
	//被多线程触发 不安全
	virtual void onNetMsg(Cell_Server *pCell_Server, Client_socket* p_clients, DataHeader* data_header)
	{
		Server::onNetMsg(pCell_Server,p_clients, data_header);
		switch (data_header->cmd) {
		case CMD_LOGIN:			//登录成功就返回结果
		{
			Login* login = (Login*)data_header;
			//printf("recv: ip:%s，port：%d，name:%s, passworld:%s\n", p_clients->get_ip(), p_clients->get_port(), login->userName, login->PassWord);
			if (strcmp(login->userName, "root") == 0 && strcmp(login->PassWord, "root") == 0) {
				LoginResult* login_result = new LoginResult();
				login_result->result = 1;
#if TEXE_SEND
				//向客户端发送包头数据
				pCell_Server->addSendTask(p_clients, login_result);
				Server::onSendCount(p_clients);
#endif
				break;
			}
			else {
				//	cout << "用户密码错误" << endl;
				LoginResult* login_result = new LoginResult();
				login_result->result = -1;
#if TEXE_SEND
				//向客户端发送包头数据
				pCell_Server->addSendTask(p_clients, login_result);
				Server::onSendCount(p_clients);
#endif
				break;
			}
			break;
		}
		case CMD_LOGINOUT:		//退出成功就返回结果
		{
			LoginOut* loginout = (LoginOut*)data_header;
			if (strcmp(loginout->userName, "root") == 0) {
				LoginOutResult* loginout_result = new LoginOutResult();
				loginout_result->result = 1;
#if TEXE_SEND
				//向客户端发送包头数据
				pCell_Server->addSendTask(p_clients, loginout_result);
				Server::onSendCount(p_clients);
#endif
				break;
			}
			else {
				LoginOutResult* loginout_result = new LoginOutResult();
				loginout_result->result = -1;
#if TEXE_SEND
				//向客户端发送包头数据
				pCell_Server->addSendTask(p_clients, loginout_result);
				Server::onSendCount(p_clients);
#endif
				break;
			}
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
	//线程：显示吞吐数据
	void show()
	{
		show_timeMsg();
	}
private:
	
protected:


};
void cmdThread(bool* g_bRun)
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			*g_bRun = false;
			printf("退出cmdThread线程\n");
			break;
		}
		else {
			printf("不支持的命令。\n");
		}
	}
}
int main(int argc, char* argv[])
{
	bool g_bRun = true;
	int max = 0;

#ifdef _WIN32
	MyServer serverA;
	serverA.Init_sock("192.168.31.240", 10000);
	serverA.Listen(5);
	serverA.Start(4);
#else
	MyServer serverA;
	serverA.Init_sock("192.168.31.14", 10001);
	serverA.Listen(5);
	serverA.Start(4);
#endif // _WIN32

	//启动流量监视线程
	serverA.show();
	//启动UI线程
	std::thread t1(cmdThread, &g_bRun);
	t1.detach();
	while (g_bRun) {

		serverA.onRun();
	}

	serverA.Close_sock();
	printf("已退出。\n");
	return 0;
}
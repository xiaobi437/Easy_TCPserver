#include "server.h"
#include "CELLTime.h"

class MyServer :virtual public Server
{
public:

	//子类实现客户端事件
//被一个线程触发 安全
	virtual void OnNetJoin(Client_socket* pClient) {
		_clientCount++;
		//printf("new client join:<ip:%s, port:%d, socket:%d>\n", pClient->get_ip(), pClient->get_port(), (int)pClient->get_socket());
	}
	//被多线程触发 不安全
	virtual void OnNetLeave(Client_socket* pClient)
	{
		_clientCount--;
		//printf("client Leave:<ip:%s, port:%d, socket:%d>\n", pClient->get_ip(), pClient->get_port(),(int)pClient->get_socket());
	}
	//被多线程触发 不安全
	virtual void onNetMsg(Client_socket* p_clients, DataHeader* data_header)
	{
		_recvCount++;
		switch (data_header->cmd) {
		case CMD_LOGIN:			//登录成功就返回结果
		{
			Login* login = (Login*)data_header;
			//printf("recv: ip:%s，port：%d，name:%s, passworld:%s\n", p_clients->get_ip(), p_clients->get_port(), login->userName, login->PassWord);
			if (strcmp(login->userName, "root") == 0 && strcmp(login->PassWord, "root") == 0) {
				LoginResult login_result = {};
				login_result.result = 1;
#if TEXE_SEND
				//向客户端发送包头数据
				p_clients->sendData(&login_result);
				onSendMsg(p_clients);
#endif
				break;
			}
			else {
				//	cout << "用户密码错误" << endl;
				LoginResult login_result = {};
				login_result.result = -1;
#if TEXE_SEND
				//向客户端发送包头数据
				p_clients->sendData(&login_result);
				onSendMsg(p_clients);
#endif
				break;
			}
			break;
		}
		case CMD_LOGINOUT:		//退出成功就返回结果
		{
			LoginOut* loginout = (LoginOut*)data_header;
			if (strcmp(loginout->userName, "root") == 0) {
				LoginOutResult loginout_result = {};
				loginout_result.result = 1;
#if TEXE_SEND
				//向客户端发送包头数据
				p_clients->sendData(&loginout_result);
				onSendMsg(p_clients);
#endif
				break;
			}
			else {
				LoginOutResult loginout_result = {};
				loginout_result.result = -1;
#if TEXE_SEND
				//向客户端发送包头数据
				p_clients->sendData(&loginout_result);
				onSendMsg(p_clients);
#endif
				break;
			}
		}
		default:
		{
			printf("收到客户端<socket:%d>未知数据, 数据长度:%d\n", (int)p_clients->get_socket(), data_header->dataLength);
			//DataHeader data_head = {};
			//sendData(client_sock, &data_head);
			break;
		}
		}

	}

	virtual void onSendMsg(Client_socket* p_clients)
	{
		_sendCount++;
	}
	//线程：显示吞吐数据
	void show()
	{
		show_timeMsg();
	}
private:
	
protected:


};
int main(int argc, char* argv[])
{
	bool flag = true;
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

	Login login = {};
	strcpy(login.userName, "root");
	strcpy(login.PassWord, "root");

	serverA.show();
	while (serverA.isRun()) {

		serverA.onRun();
	}


	serverA.Close_sock();

	return 0;
}
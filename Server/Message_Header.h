/*
* 消息包结构的声明
* 
*/
#ifndef MESSAGE_HEADER_H
#define MESSAGE_HEADER_H

#ifdef _WIN32
#include <windows.h>
#else
#define SOCKET int
#endif // _WIN32
#include <memory>


enum CMD {
	CMD_LOGIN,
	CMD_LOGIN_RET,
	CMD_LOGINOUT,
	CMD_LOGINOUT_RET,
	CMD_NEW_LOGIN,
	CMD_HEART_S2C,
	CMD_HEART_C2S,
	CMD_ERROR
};
struct DataHeader {
	DataHeader() {
		dataLength = sizeof(DataHeader);
		cmd = CMD_ERROR;
	}
	short dataLength;
	short cmd;

};
//包体
struct Login :public DataHeader
{
	Login() {
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];
	char data[32];
};
struct LoginResult :public DataHeader	//登录返回数据
{
	LoginResult() {
		cmd = CMD_LOGIN_RET;
		dataLength = sizeof(LoginResult);
	}
	int result;
	char data[92];
};
struct LoginOut :public DataHeader		//登出
{
	LoginOut() {
		cmd = CMD_LOGINOUT;
		dataLength = sizeof(LoginOut);
	}
	char userName[32];
};
struct LoginOutResult :public DataHeader	//登出返回数据
{
	LoginOutResult() {
		cmd = CMD_LOGINOUT_RET;
		dataLength = sizeof(LoginOutResult);
	}
	int result;
};
struct NewLogin :public DataHeader	//新用户登入
{
	NewLogin() {
		cmd = CMD_NEW_LOGIN;
		dataLength = sizeof(NewLogin);
	}
	char IP[16];
	SOCKET sock;
	int Port;
};
struct Heart_S2C :public DataHeader	//心跳 服务端发送给客户端
{
	Heart_S2C() {
		cmd = CMD_HEART_S2C;
		dataLength = sizeof(Heart_S2C);
	}
};
struct Heart_C2S :public DataHeader	//心跳 客户端发送给服务端
{
	Heart_C2S() {
		cmd = CMD_HEART_C2S;
		dataLength = sizeof(Heart_C2S);
	}
};
typedef std::shared_ptr<DataHeader> DataHeaderPtr;
#endif

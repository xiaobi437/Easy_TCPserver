/*
* Cell_Client 客户端类
* 
*/
#ifndef CELL_CLIENT_H
#define CELL_CLIENT_H

#include <memory>
#include "Public_Include.h"
#include "Message_Header.h"
#include "Cell_Buffer.h"
#include "Object_Pool.h"		//对象池
#include "Cell_Log.h"

//客户端心跳检测时间 60秒
#define CLIENT_HREAT_DEAD_TIME 60000
//定时发送 200ms
#define CLIENT_SEND_BUFF_TIME 200

//为每个客户端创建专属的缓冲区
class Cell_Client : public ObjectPoolBase<Cell_Client, 1000>
{
public:
	int id;
	int serverId = -1;
	Cell_Client(sockaddr_in clientaddr, SOCKET sockfd = INVALID_SOCKET):
		_sendBuff(SEND_BUFF_SIZE),
		_recvBuff(RECV_BUFF_SIZE)
	{
		static int n = 1;			//类似全局变量，只在这个作用域内生效
		id = n++;
		_sockfd = sockfd;
		_clientaddr = clientaddr;
		resetDtHeart();
		resetDtSend();
	}
	~Cell_Client()
	{
		if (_sockfd != INVALID_SOCKET)
		{
#ifdef _WIN32

		closesocket(_sockfd);
#else
		close(_sockfd);
#endif
		}
	}
	//获取socket
	SOCKET sockfd() {
		return _sockfd;
	}
	//获取ip
	char* get_ip() {
		return inet_ntoa(_clientaddr.sin_addr);
	}
	//获取端口
	int get_port() {
		return ntohs(_clientaddr.sin_port);
	}
	//接收数据
	int RecvData()
	{
		return _recvBuff.Read_for_Socket(_sockfd);
	}
	//是否有数据可读
	bool hasMsg()
	{
		return _recvBuff.hasMsg();
	}
	//返回消息头指令
	DataHeader* front_msg()
	{
		return (DataHeader*)_recvBuff.data();
	}
	//移除缓冲区第一条数据
	void pop_front_msg()
	{
		if(hasMsg())
			return _recvBuff.pop(front_msg()->dataLength);
	}
	//有数据需要写
	bool needWrite()
	{
		return _sendBuff.needWrite();
	}
	//发送数据
	int sendDataReal()
	{
		resetDtSend();
		return _sendBuff.Write_to_Socket(_sockfd);
	}
	//发送数据存入缓冲区
	int sendData(DataHeader* data_header)
	{
		int ret = SOCKET_ERROR;
		if (_sendBuff.push((char *)data_header, data_header->dataLength))	//=49.5+1个数据
		{
			return data_header->dataLength;
		}
		return ret;
	}

	//心跳重置
	void resetDtHeart()
	{
		_dtHeart = 0;
	}
	//心跳检测
	bool checkHeart(time_t dt)
	{
		_dtHeart += dt;
		if (_dtHeart >= CLIENT_HREAT_DEAD_TIME)
		{
			Cell_Log::Info(Info_Msg,"checkHeart dead: socket:%d, time:%ld\n",(int)_sockfd, _dtHeart);
			return true;
		}
		return false;
	}
	//定时发送计时
	void resetDtSend()
	{
		_dtSend = 0;
	}
	//定时发送
	bool checkSend(time_t dt)
	{
		_dtSend += dt;
		if (_dtSend >= CLIENT_SEND_BUFF_TIME)
		{
			//立即将缓冲区的数据发送出去
			sendDataReal();
			//重置发送计时
		}
		return false;
	}
private:
	Cell_Buffer _sendBuff;	//发送缓冲区
	Cell_Buffer _recvBuff; //消息接收缓冲区 50KB

	sockaddr_in _clientaddr; 

	time_t _dtHeart;	//死亡心跳计时
	time_t _dtSend;		//上一次发送计时

	SOCKET _sockfd;		//用于存放客户端sock
};
typedef std::shared_ptr<Cell_Client> Cell_ClientPtr;
//int aa = sizeof(Cell_Client);
#endif // !CELL_CLIENT_H


/*
* 网络事件
* 
*/
#ifndef NET_EVENT_H
#define NET_EVENT_H

#include "Cell_Client.h"


//网络事件接口，委托代理，客户端退出（方法不是唯一）
class NetEvent
{
public:
	//使用纯虚函数，让子类去实现
	//客户端加入事件
	virtual void OnNetJoin(Cell_Client* pClient) = 0;
	//客户端退出事件
	virtual void OnNetLeave() = 0;
	//客户端消息事件
	virtual void onNetMsg(Cell_Server* pCell_Server, Cell_Client* p_clients, DataHeader* data_header) = 0;

	//测试recv和send的极限
	virtual void onRecvCount(Cell_Client* p_clients) = 0;
	virtual void onSendCount(Cell_Client* p_clients) = 0;
private:
};

#endif // !NET_EVENT_H
